/**
 * @file image_cycler.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-01
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "image_cache.hpp"
#include <cstddef>
#include <memory>
#include <opencv2/opencv.hpp>

namespace lip_sync::pipe {
class ImageCycler {
private:
  std::vector<std::string> imagePaths;
  std::unique_ptr<ImageCache> cache;
  bool forward;
  int currentPos;
  int taskCount;
  int directionChangeCount;
  int lastChangePos;

  void predictAndPreload();

public:
  ImageCycler(const std::string &imageDir, size_t maxCacheSize);

  std::shared_ptr<cv::Mat> getNextImage();
  int getTaskCount() const { return taskCount; }
  size_t getCacheSize() const { return cache->getCacheSize(); }
  size_t getCachedImageCount() const { return cache->getCacheCount(); }
};
} // namespace lip_sync::pipe
