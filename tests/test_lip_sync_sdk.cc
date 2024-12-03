#include "lip_sync/lip_sync_sdk.h"
#include "logger/logger.hpp"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <sstream>

const auto initLogger = []() -> decltype(auto) {
  LipSyncLoggerInit(true, true, true, true);
  return true;
}();

std::string getTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
  return ss.str();
}

void saveFrame(const std::vector<uint8_t> &frameData,
               const std::string &outputDir, const std::string &uuid,
               int64_t sequence) {
  std::filesystem::create_directories(outputDir);
  std::string filename =
      outputDir + "/" + uuid + "_" + std::to_string(sequence) + ".png";

  cv::Mat frame = cv::imdecode(frameData, cv::IMREAD_COLOR);
  if (!frame.empty()) {
    cv::imwrite(filename, frame);
    std::cout << "Saved frame to: " << filename << std::endl;
  } else {
    std::cerr << "Failed to decode frame data for sequence: " << sequence
              << std::endl;
  }
}

int main(int argc, char **argv) {

  LipSyncLoggerSetLevel(2);

  std::cout << "Starting LipSync SDK test at: " << getTimestamp() << std::endl;

  lip_sync::LipSyncSDK sdk;
  lip_sync::SDKConfig config;
  config.numWorkers = 1;
  config.frameDir = "data/frames";
  config.faceInfoPath = "data/face_bboxes.json";
  config.encoderModelPath = "models/wenet_encoder.onnx";
  config.wavLipModelPath = "models/w2l_with_wenet.onnx";
  config.faceSize = 160;
  config.facePad = 4;
  config.maxCacheSize = 1024 * 1024 * 100;
  config.frameRate = 30;

  std::cout << "\nInitializing SDK with config:" << "\n - Workers: "
            << config.numWorkers << "\n - Face size: " << config.faceSize
            << "\n - Face pad: " << config.facePad
            << "\n - Cache size: " << config.maxCacheSize << " bytes"
            << std::endl;

  auto ret = sdk.initialize(config);
  if (ret != lip_sync::ErrorCode::SUCCESS) {
    std::cerr << "Failed to initialize SDK" << std::endl;
    return 1;
  }
  std::cout << "SDK initialized successfully" << std::endl;

  lip_sync::InputPacket input;
  input.audioPath = "data/test.wav";
  input.uuid = "test_" + getTimestamp();

  std::cout << "\nStarting process with:" << "\n - Audio: " << input.audioPath
            << "\n - UUID: " << input.uuid << std::endl;

  ret = sdk.startProcess(input);
  if (ret != lip_sync::ErrorCode::SUCCESS) {
    std::cerr << "Failed to start processing" << std::endl;
    return 1;
  }

  std::string outputDir = "output/" + input.uuid;
  int successCount = 0;
  int failureCount = 0;

  lip_sync::OutputPacket output;
  for (int i = 0; i < 100; ++i) {
    ret = sdk.tryGetNext(output);
    if (ret == lip_sync::ErrorCode::SUCCESS) {
      successCount++;
      std::cout << "\nReceived output packet " << successCount << ":"
                << "\n - Sequence: " << output.sequence
                << "\n - Timestamp: " << output.timestamp
                << "\n - Frame size: " << output.frameData.size() << " bytes"
                << "\n - Dimensions: " << output.width << "x" << output.height
                << std::endl;

      saveFrame(output.frameData, outputDir, input.uuid, output.sequence);
    } else {
      failureCount++;
      std::cout << "Failed to get output packet (attempt " << i + 1 << ")"
                << std::endl;
    }
  }

  std::cout << "\nTest summary:" << "\n - Successful packets: " << successCount
            << "\n - Failed attempts: " << failureCount
            << "\n - Output directory: " << outputDir << std::endl;

  sdk.terminate();
  std::cout << "Test completed at: " << getTimestamp() << std::endl;

  return 0;
}