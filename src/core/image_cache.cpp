/**
 * @file image_cache.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-01
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "image_cache.hpp"
#include <memory>

namespace lip_sync::pipe {
ImageCache::ImageCache(const std::vector<std::string> &paths, size_t maxMemSize)
    : imagePaths(paths), maxMemorySize(maxMemSize), totalImages(paths.size()) {

  auto firstImage = std::make_shared<cv::Mat>(cv::imread(paths[0]));
  estimatedImageSize = calculateImageSize(firstImage);

  int totalWindowSize =
      std::max(3, static_cast<int>(maxMemorySize / estimatedImageSize));
  forwardWindowSize = static_cast<int>(totalWindowSize * 0.7);
  backwardWindowSize = totalWindowSize - forwardWindowSize;

  preloadWindow(0, true);
}

size_t ImageCache::calculateImageSize(const std::shared_ptr<cv::Mat> &img) {
  return img->total() * img->elemSize();
}

void ImageCache::adjustWindowSizes() {
  int totalWindowSize = forwardWindowSize + backwardWindowSize;
  if (isForward) {
    forwardWindowSize = static_cast<int>(totalWindowSize * 0.7);
    backwardWindowSize = totalWindowSize - forwardWindowSize;
  } else {
    backwardWindowSize = static_cast<int>(totalWindowSize * 0.7);
    forwardWindowSize = totalWindowSize - backwardWindowSize;
  }
}

void ImageCache::redistributeMemory() {
  size_t targetMemory = isForward ? static_cast<size_t>(maxMemorySize * 0.7)
                                  : static_cast<size_t>(maxMemorySize * 0.3);

  auto &primaryCache = isForward ? forwardCache : backwardCache;
  auto &secondaryCache = isForward ? backwardCache : forwardCache;

  while (currentMemorySize > targetMemory && !secondaryCache.empty()) {
    removeOldestEntry(secondaryCache);
  }
}

void ImageCache::updateHotspots(int pos, bool directionChanged) {
  auto now = std::chrono::system_clock::now().time_since_epoch().count();

  bool found = false;
  for (auto &hotspot : hotspots) {
    if (std::abs(hotspot.position - pos) <= 2) {
      hotspot.accessCount++;
      hotspot.lastAccessTime = now;
      found = true;
      break;
    }
  }

  if (!found) {
    if (hotspots.size() >= MAX_HOTSPOTS) {
      auto oldest =
          std::min_element(hotspots.begin(), hotspots.end(),
                           [](const HotSpot &a, const HotSpot &b) {
                             return a.lastAccessTime < b.lastAccessTime;
                           });
      *oldest = {pos, 1, directionChanged, now};
    } else {
      hotspots.push_back({pos, 1, directionChanged, now});
    }
  }
}

void ImageCache::optimizeForHotspots() {
  for (const auto &hotspot : hotspots) {
    if (hotspot.isDirectionChange || hotspot.accessCount > 5) {
      ensureCached(hotspot.position);
    }
  }
}

void ImageCache::removeOldestEntry(std::unordered_map<int, CacheEntry> &cache) {
  if (cache.empty())
    return;

  auto oldest = cache.begin();
  for (auto it = cache.begin(); it != cache.end(); ++it) {
    if (it->second.lastAccessTime < oldest->second.lastAccessTime) {
      oldest = it;
    }
  }

  currentMemorySize -= oldest->second.size;
  cache.erase(oldest);
}

void ImageCache::ensureCached(int position) {
  auto &targetCache = isForward ? forwardCache : backwardCache;
  if (targetCache.find(position) == targetCache.end()) {
    loadImage(position);
  }
}

void ImageCache::smartPreload(int currentPos) {
  int preloadDistance =
      isForward ? std::min(forwardWindowSize, totalImages - currentPos)
                : std::min(backwardWindowSize, currentPos);

  float memoryUsageRatio =
      static_cast<float>(currentMemorySize) / maxMemorySize;
  if (memoryUsageRatio < 0.7) {
    preloadDistance = static_cast<int>(preloadDistance * 1.5);
  }

  if (isForward) {
    for (int i = currentPos; i < currentPos + preloadDistance; ++i) {
      if (i < totalImages)
        loadImage(i);
    }
  } else {
    for (int i = currentPos; i > currentPos - preloadDistance; --i) {
      if (i >= 0)
        loadImage(i);
    }
  }
}

void ImageCache::prepareDirectionChange(int currentPos, bool nextDirection) {
  isForward = nextDirection;
  adjustWindowSizes();
  redistributeMemory();
  smartPreload(currentPos);
}

void ImageCache::preloadWindow(int startPos, bool forward) {
  isForward = forward;
  windowStart = startPos;

  std::set<int> newWindowIndices;
  int newWindowEnd = forward
                         ? std::min(startPos + forwardWindowSize, totalImages)
                         : std::max(startPos - backwardWindowSize, 0);

  if (forward) {
    for (int i = startPos; i < newWindowEnd; ++i) {
      newWindowIndices.insert(i);
    }
  } else {
    for (int i = startPos; i > newWindowEnd; --i) {
      newWindowIndices.insert(i);
    }
  }

  auto &primaryCache = forward ? forwardCache : backwardCache;
  auto &secondaryCache = forward ? backwardCache : forwardCache;

  auto it = primaryCache.begin();
  while (it != primaryCache.end()) {
    if (newWindowIndices.find(it->first) == newWindowIndices.end()) {
      currentMemorySize -= it->second.size;
      it = primaryCache.erase(it);
    } else {
      ++it;
    }
  }

  for (int index : newWindowIndices) {
    if (primaryCache.find(index) == primaryCache.end()) {
      loadImage(index);
    }
  }

  optimizeForHotspots();
}

void ImageCache::loadImage(int index) {
  auto &targetCache = isForward ? forwardCache : backwardCache;
  if (targetCache.find(index) != targetCache.end())
    return;

  auto newImage = std::make_shared<cv::Mat>(cv::imread(imagePaths[index]));
  size_t newImageSize = calculateImageSize(newImage);

  while (currentMemorySize + newImageSize > maxMemorySize) {
    auto &cacheToClean = targetCache.empty()
                             ? (isForward ? backwardCache : forwardCache)
                             : targetCache;
    removeOldestEntry(cacheToClean);
  }

  auto now = std::chrono::system_clock::now().time_since_epoch().count();
  targetCache[index] = {newImage, newImageSize, 1, now};
  currentMemorySize += newImageSize;
}

std::shared_ptr<cv::Mat> ImageCache::getImage(int index) {
  if (index < 0 || index >= imagePaths.size()) {
    throw std::out_of_range("Image index out of range");
  }

  auto &primaryCache = isForward ? forwardCache : backwardCache;
  auto &secondaryCache = isForward ? backwardCache : forwardCache;

  auto primaryIt = primaryCache.find(index);
  if (primaryIt != primaryCache.end()) {
    primaryIt->second.accessCount++;
    primaryIt->second.lastAccessTime =
        std::chrono::system_clock::now().time_since_epoch().count();
    return primaryIt->second.image;
  }

  auto secondaryIt = secondaryCache.find(index);
  if (secondaryIt != secondaryCache.end()) {
    secondaryIt->second.accessCount++;
    secondaryIt->second.lastAccessTime =
        std::chrono::system_clock::now().time_since_epoch().count();
    return secondaryIt->second.image;
  }

  loadImage(index);
  updateHotspots(index, false);
  return isForward ? forwardCache[index].image : backwardCache[index].image;
}

} // namespace lip_sync::pipe