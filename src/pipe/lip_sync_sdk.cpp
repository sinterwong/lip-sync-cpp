/**
 * @file lip_sync_sdk.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "lip_sync_sdk.hpp"
#include "lip_sync_sdk_impl.hpp"
#include <string>

namespace lip_sync {

LipSyncSDK::LipSyncSDK() : impl_(std::make_unique<LipSyncSDKImpl>()) {}

LipSyncSDK::~LipSyncSDK() {
  if (impl_) {
    impl_->terminate();
  }
}

ErrorCode LipSyncSDK::initialize(const SDKConfig &config) {
  if (!impl_) {
    return ErrorCode::INITIALIZATION_FAILED;
  }
  return impl_->initialize(config);
}

ErrorCode LipSyncSDK::startProcess(const InputPacket &input) {
  if (!impl_) {
    return ErrorCode::INVALID_STATE;
  }
  return impl_->startProcess(input);
}

ErrorCode LipSyncSDK::terminate() {
  if (!impl_) {
    return ErrorCode::INVALID_STATE;
  }
  return impl_->terminate();
}

ErrorCode LipSyncSDK::tryGetNext(OutputPacket &result) {
  if (!impl_) {
    return ErrorCode::INVALID_STATE;
  }
  return impl_->tryGetNext(result);
}

std::string LipSyncSDK::getVersion() { return "1.0.0"; }

} // namespace lip_sync