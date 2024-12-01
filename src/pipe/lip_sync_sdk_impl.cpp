/**
 * @file lip_sync_sdk_impl.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "lip_sync_sdk_impl.hpp"
#include "core/wavlip.hpp"
#include "logger/logger.hpp"
#include <string>

namespace lip_sync {

using namespace lip_sync::infer;
LipSyncSDKImpl::LipSyncSDKImpl() : isRunning(false) {}

ErrorCode LipSyncSDKImpl::initialize(const SDKConfig &config) {
  WeNetConfig wenetConfig{.modelPath = config.encoderModelPath};
  featureExtractor =
      std::make_unique<FeatureExtractor>(FbankConfig{}, wenetConfig);
  if (!featureExtractor->initialize()) {
    LOGGER_ERROR("Failed to initialize feature extractor");
    return ErrorCode::INITIALIZATION_FAILED;
  }

  imageCycler =
      std::make_unique<pipe::ImageCycler>(config.frameDir, config.maxCacheSize);

  workers.start(config.numWorkers);
  for (int i = 0; i < config.numWorkers; ++i) {
    // init model instance
    modelInstances.emplace_back(std::make_unique<dnn::WavToLipInference>(
        AlgoBase{.name = "wavlip-" + std::to_string(i),
                 .modelPath = config.wavLipModelPath}));
    if (!modelInstances.back()->initialize()) {
      LOGGER_ERROR("Failed to initialize wav to lip model");
      return ErrorCode::INITIALIZATION_FAILED;
    }
  }

  isRunning.store(true);
  return ErrorCode::SUCCESS;
}

ErrorCode LipSyncSDKImpl::startProcess(const InputPacket &input) {
  return ErrorCode::SUCCESS;
}

ErrorCode LipSyncSDKImpl::terminate() { return ErrorCode::SUCCESS; }

ErrorCode LipSyncSDKImpl::tryGetNext(OutputPacket &result) {
  return ErrorCode::PROCESSING_ERROR;
}
} // namespace lip_sync
