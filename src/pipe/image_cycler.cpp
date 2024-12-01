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

#include "image_cycler.hpp"
#include "logger/logger.hpp"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace lip_sync::pipe {
ImageCycler::ImageCycler(const std::string &imageDir, size_t maxCacheSize)
    : forward(true), currentPos(0), taskCount(0) {

  // get all image paths
  fs::directory_iterator it(imageDir);
  fs::directory_iterator end;
  for (; it != end; ++it) {
    if (it->is_regular_file() && it->path().extension() == ".png" ||
        it->path().extension() == ".jpg" || it->path().extension() == ".jpeg") {
      imagePaths.push_back(it->path().string());
    }
  }

  // 图片文件名一定是数字，根据文件名的数字大小进行排序
  std::sort(
      imagePaths.begin(), imagePaths.end(),
      [](const std::string &a, const std::string &b) {
        // extract numbers from filenames
        size_t a_pos = a.find_last_of('/');
        size_t b_pos = b.find_last_of('/');
        std::string a_num_str = a.substr(a_pos + 1);
        std::string b_num_str = b.substr(b_pos + 1);
        a_num_str.erase(std::remove_if(a_num_str.begin(), a_num_str.end(),
                                       [](char c) { return !isdigit(c); }),
                        a_num_str.end());
        b_num_str.erase(std::remove_if(b_num_str.begin(), b_num_str.end(),
                                       [](char c) { return !isdigit(c); }),
                        b_num_str.end());

        // convert to numbers and compare
        if (a_num_str.empty() || b_num_str.empty()) {
          return a < b;
        }
        return std::stoll(a_num_str) < std::stoll(b_num_str);
      });
  cache = std::make_unique<ImageCache>(imagePaths, maxCacheSize);
}

std::shared_ptr<cv::Mat> ImageCycler::getNextImage() {
  int currentIndex = currentPos;
  taskCount++;

  bool directionChanged = false;

  if (forward) {
    currentPos++;
    if (currentPos >= imagePaths.size()) {
      forward = false;
      currentPos = imagePaths.size() - 1;
      directionChanged = true;
    }
  } else {
    currentPos--;
    if (currentPos < 0) {
      forward = true;
      currentPos = 0;
      directionChanged = true;
    }
  }

  try {
    // notify the cache if the direction change
    if (directionChanged) {
      cache->preloadWindow(currentPos, forward);
    }
    return cache->getImage(currentIndex);
  } catch (const std::exception &e) {
    LOGGER_ERROR("Error loading image: {}", e.what());
    throw;
  }
}
} // namespace lip_sync::pipe
