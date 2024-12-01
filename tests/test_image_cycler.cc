

#include "lip_sync/image_cycler.hpp"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>

using namespace lip_sync::pipe;
namespace fs = std::filesystem;

class TestStats {
private:
  size_t totalAccesses = 0;
  size_t cacheHits = 0;
  double totalTime = 0;
  std::vector<double> accessTimes;

public:
  void recordAccess(bool isHit) {
    totalAccesses++;
    if (isHit)
      cacheHits++;
  }

  void recordTiming(double ms) {
    totalTime += ms;
    accessTimes.push_back(ms);
  }

  void print() const {
    std::cout << "\nTest Statistics:\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total accesses: " << totalAccesses << "\n";
    std::cout << "Cache hits: " << cacheHits << "\n";
    std::cout << "Hit rate: "
              << (totalAccesses > 0 ? (cacheHits * 100.0 / totalAccesses) : 0)
              << "%\n";
    std::cout << "Average access time: "
              << (totalAccesses > 0 ? (totalTime / totalAccesses) : 0)
              << "ms\n";

    if (!accessTimes.empty()) {
      double sum = 0;
      for (double time : accessTimes)
        sum += time;
      double mean = sum / accessTimes.size();

      double sq_sum = 0;
      for (double time : accessTimes) {
        sq_sum += (time - mean) * (time - mean);
      }
      double stddev = std::sqrt(sq_sum / accessTimes.size());

      double min_time =
          *std::min_element(accessTimes.begin(), accessTimes.end());
      double max_time =
          *std::max_element(accessTimes.begin(), accessTimes.end());
      std::cout << "Standard deviation: " << stddev << "ms\n";
      std::cout << "Min access time: " << min_time << "ms\n";
      std::cout << "Max access time: " << max_time << "ms\n";
    }
  }
};

void testWithDifferentCacheSizes(const fs::path &imageDir) {
  std::cout << "Testing with real data from: " << imageDir << std::endl;

  // Count total images and calculate image size
  size_t imageCount = 0;
  size_t singleImageSize = 0;

  // Calculate exact image size from first image
  for (const auto &entry : fs::directory_iterator(imageDir)) {
    if (entry.is_regular_file() && (entry.path().extension() == ".png" ||
                                    entry.path().extension() == ".jpg" ||
                                    entry.path().extension() == ".jpeg")) {
      if (imageCount == 0) {
        cv::Mat sampleImage = cv::imread(entry.path().string());
        singleImageSize = sampleImage.total() * sampleImage.elemSize();
        std::cout << "Single image size: "
                  << (singleImageSize / (1024.0 * 1024.0)) << "MB\n";
      }
      imageCount++;
    }
  }

  size_t totalSize = singleImageSize * imageCount;
  std::cout << "Total images found: " << imageCount << std::endl;
  std::cout << "Total memory needed: " << (totalSize / (1024.0 * 1024.0))
            << "MB\n\n";

  // Test with different numbers of cached images
  std::vector<size_t> cachedImageCounts = {10, 20, 30, 50};

  for (size_t numImages : cachedImageCounts) {
    size_t cacheSize = singleImageSize * numImages;
    std::cout << "\n=== Testing with cache for " << numImages << " images "
              << "(" << (cacheSize / (1024.0 * 1024.0)) << "MB) ===\n";

    TestStats stats;

    try {
      ImageCycler cycler(imageDir.string(), cacheSize);

      // Run one complete cycle (forward and backward)
      std::cout << "Running forward scan...\n";
      for (size_t i = 0; i < imageCount; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        auto img = cycler.getNextImage();
        auto end = std::chrono::high_resolution_clock::now();

        double duration =
            std::chrono::duration<double, std::milli>(end - start).count();
        stats.recordTiming(duration);
        // Adjust threshold based on observed min access time
        stats.recordAccess(duration < 15.0);

        if ((i + 1) % 10 == 0) {
          std::cout << "Processed " << (i + 1) << "/" << imageCount
                    << " images, Cache usage: "
                    << (cycler.getCacheSize() / (1024.0 * 1024.0)) << "MB/"
                    << (cacheSize / (1024.0 * 1024.0)) << "MB"
                    << ", Cached images: " << cycler.getCachedImageCount()
                    << "\n";
        }
      }

      std::cout << "\nRunning backward scan...\n";
      for (size_t i = 0; i < imageCount; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        auto img = cycler.getNextImage();
        auto end = std::chrono::high_resolution_clock::now();

        double duration =
            std::chrono::duration<double, std::milli>(end - start).count();
        stats.recordTiming(duration);
        stats.recordAccess(duration < 15.0);

        if ((i + 1) % 10 == 0) {
          std::cout << "Processed " << (i + 1) << "/" << imageCount
                    << " images, Cache usage: "
                    << (cycler.getCacheSize() / (1024.0 * 1024.0)) << "MB/"
                    << (cacheSize / (1024.0 * 1024.0)) << "MB"
                    << ", Cached images: " << cycler.getCachedImageCount()
                    << "\n";
        }
      }

      stats.print();

    } catch (const std::exception &e) {
      std::cerr << "Test failed: " << e.what() << "\n";
    }
  }
}

int main() {
  fs::path dataDir = fs::path("data");
  fs::path imageDir = dataDir / "frames";

  if (!fs::exists(imageDir)) {
    std::cerr << "Image directory not found: " << imageDir << std::endl;
    return 1;
  }

  std::cout << "Starting Real Data Tests\n";
  testWithDifferentCacheSizes(imageDir);
  std::cout << "\nAll tests completed!\n";
  return 0;
}