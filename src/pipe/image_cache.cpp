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
  windowSize =
      std::max(3, static_cast<int>(maxMemorySize / estimatedImageSize));

  // preload the images of first window
  preloadWindow(0, true);
}

size_t ImageCache::calculateImageSize(const std::shared_ptr<cv::Mat> &img) {
  return img->total() * img->elemSize();
}

void ImageCache::preloadWindow(int startPos, bool forward) {
  cache.clear();
  currentMemorySize = 0;
  windowStart = startPos;
  isForward = forward;

  int endPos = forward ? std::min(startPos + windowSize, totalImages)
                       : std::max(startPos - windowSize, 0);

  if (forward) {
    for (int i = startPos; i < endPos; ++i) {
      loadImage(i);
    }
  } else {
    for (int i = startPos; i > endPos; --i) {
      loadImage(i);
    }
  }
}

void ImageCache::loadImage(int index) {
  if (cache.find(index) != cache.end())
    return;

  auto newImage = std::make_shared<cv::Mat>(cv::imread(imagePaths[index]));
  size_t newImageSize = calculateImageSize(newImage);

  if (currentMemorySize + newImageSize > maxMemorySize) {
    // remove the farthest image if run out of space
    if (isForward) {
      if (!cache.empty()) {
        auto firstKey = windowStart;
        currentMemorySize -= cache[firstKey].size;
        cache.erase(firstKey);
        windowStart++;
      }
    } else {
      if (!cache.empty()) {
        auto lastKey = windowStart;
        currentMemorySize -= cache[lastKey].size;
        cache.erase(lastKey);
        windowStart--;
      }
    }
  }

  cache[index] = {newImage, newImageSize};
  currentMemorySize += newImageSize;
}

std::shared_ptr<cv::Mat> ImageCache::getImage(int index) {
  if (index < 0 || index >= imagePaths.size()) {
    throw std::out_of_range("Image index out of range");
  }

  // check if need to move the window
  if (isForward && (index >= windowStart + windowSize || index < windowStart)) {
    preloadWindow(index, true);
  } else if (!isForward &&
             (index <= windowStart - windowSize || index > windowStart)) {
    preloadWindow(index, false);
  }

  // load it on demand
  if (cache.find(index) == cache.end()) {
    loadImage(index);
  }

  return cache[index].image;
}

} // namespace lip_sync::pipe