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
    // memory used(byte)
    size_t size;
  };

  size_t maxMemorySize;
  size_t currentMemorySize = 0;

  // using sliding window
  std::unordered_map<int, CacheEntry> cache;

  int windowStart = 0;
  int windowSize;
  bool isForward = true;
  int totalImages;

  std::vector<std::string> imagePaths;

  // estimate the average size of a single image
  size_t estimatedImageSize = 0;

  size_t calculateImageSize(const std::shared_ptr<cv::Mat> &img);

public:
  ImageCache(const std::vector<std::string> &paths, size_t maxMemSize);

  void preloadWindow(int startPos, bool forward);

  void loadImage(int index);

  std::shared_ptr<cv::Mat> getImage(int index);

  size_t getCacheSize() const { return currentMemorySize; }

  size_t getCacheCount() const { return cache.size(); }
};
} // namespace lip_sync::pipe
