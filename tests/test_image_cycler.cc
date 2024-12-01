#include "core/image_cycler.hpp"
#include "logger/logger.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
using namespace lip_sync::pipe;

const auto initLogger = []() -> decltype(auto) {
  LipSyncLoggerInit(true, true, true, true);
  return true;
}();

int main() {
  fs::path dataDir = fs::path("data");
  fs::path imageDir = dataDir / "frames";

  if (!fs::exists(imageDir)) {
    std::cerr << "Image directory not found: " << imageDir << std::endl;
    return 1;
  }

  ImageCycler cycler(imageDir.string(), 1024 * 1024 * 100); // 100MB cache

  for (int i = 0; i < 500; ++i) {
    auto image = cycler.getNextImage();
    if (image) {
      std::cout << "Got image " << i
                << ", cache size: " << cycler.getCacheSize()
                << " bytes, cached images: " << cycler.getCachedImageCount()
                << std::endl;
    } else {
      std::cerr << "Failed to get image " << i << std::endl;
    }
  }

  std::cout << "Total tasks: " << cycler.getTaskCount() << std::endl;
  return 0;
}