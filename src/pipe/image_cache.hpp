/**
 * @file image_cache.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-01
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <cstddef>
#include <memory>
#include <opencv2/opencv.hpp>

namespace lip_sync::pipe {
class ImageCache {
private:
  struct CacheEntry {
    std::shared_ptr<cv::Mat> image;
    size_t size;
    int accessCount;
    long long lastAccessTime;
  };

  struct HotSpot {
    int position;
    int accessCount;
    bool isDirectionChange;
    long long lastAccessTime;
  };

  // Cache configurations
  size_t maxMemorySize;
  size_t currentMemorySize = 0;
  size_t estimatedImageSize = 0;

  // Dual direction caches
  std::unordered_map<int, CacheEntry> forwardCache;
  std::unordered_map<int, CacheEntry> backwardCache;

  // Window management
  int windowStart = 0;
  int forwardWindowSize;
  int backwardWindowSize;
  int totalImages;
  bool isForward = true;

  // Hotspot management
  std::vector<HotSpot> hotspots;
  const size_t MAX_HOTSPOTS = 10;

  std::vector<std::string> imagePaths;

  // Private methods
  size_t calculateImageSize(const std::shared_ptr<cv::Mat> &img);
  void adjustWindowSizes();
  void redistributeMemory();
  void updateHotspots(int pos, bool directionChanged);
  void optimizeForHotspots();
  void removeOldestEntry(std::unordered_map<int, CacheEntry> &cache);
  void ensureCached(int position);
  void smartPreload(int currentPos);

public:
  ImageCache(const std::vector<std::string> &paths, size_t maxMemSize);

  void prepareDirectionChange(int currentPos, bool nextDirection);
  void preloadWindow(int startPos, bool forward);
  void loadImage(int index);
  std::shared_ptr<cv::Mat> getImage(int index);

  size_t getCacheSize() const { return currentMemorySize; }
  size_t getCacheCount() const {
    return forwardCache.size() + backwardCache.size();
  }
};
} // namespace lip_sync::pipe
