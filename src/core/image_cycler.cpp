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

namespace lip_sync::pipe {
ImageCycler::ImageCycler(const std::string &imageDir, size_t maxCacheSize)
    : forward(true), currentPos(0), taskCount(0), directionChangeCount(0),
      lastChangePos(0) {

  for (const auto &entry : std::filesystem::directory_iterator(imageDir)) {
    if (entry.is_regular_file()) {
      auto ext = entry.path().extension().string();
      if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
        imagePaths.push_back(entry.path().string());
      }
    }
  }

  std::sort(imagePaths.begin(), imagePaths.end(),
            [](const std::string &a, const std::string &b) {
              auto getNumber = [](const std::string &path) {
                auto pos = path.find_last_of('/');
                auto filename = path.substr(pos + 1);
                std::string number;
                std::copy_if(filename.begin(), filename.end(),
                             std::back_inserter(number), isdigit);
                return number.empty() ? 0 : std::stoll(number);
              };
              return getNumber(a) < getNumber(b);
            });

  cache = std::make_unique<ImageCache>(imagePaths, maxCacheSize);
}

void ImageCycler::predictAndPreload() {
  if (forward && currentPos >= imagePaths.size() - 3) {
    cache->prepareDirectionChange(currentPos, false);
  } else if (!forward && currentPos <= 2) {
    cache->prepareDirectionChange(currentPos, true);
  }
}

std::shared_ptr<cv::Mat> ImageCycler::getNextImage() {
  predictAndPreload();

  int currentIndex = currentPos;
  taskCount++;

  bool directionChanged = false;

  if (forward) {
    currentPos++;
    if (currentPos >= imagePaths.size()) {
      forward = false;
      currentPos = imagePaths.size() - 1;
      directionChanged = true;
      directionChangeCount++;
      lastChangePos = currentPos;
    }
  } else {
    currentPos--;
    if (currentPos < 0) {
      forward = true;
      currentPos = 0;
      directionChanged = true;
      directionChangeCount++;
      lastChangePos = currentPos;
    }
  }

  try {
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
