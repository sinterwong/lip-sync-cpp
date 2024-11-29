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

namespace lip_sync {
LipSyncSDKImpl::LipSyncSDKImpl() : isRunning(false) {}

ErrorCode LipSyncSDKImpl::initialize(const SDKConfig &config) {
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
