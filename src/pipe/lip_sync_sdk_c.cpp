/**
 * @file lip_sync_sdk_c.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "lip_sync_sdk.h"
#include "lip_sync_sdk.hpp"
#include "lip_sync_types.h"
#include <string.h>

LipSyncSDKHandle LipSyncSDK_Create() {
  lip_sync::LipSyncSDK *sdk = new lip_sync::LipSyncSDK();
  return (LipSyncSDKHandle)sdk;
}

void LipSyncSDK_Destroy(LipSyncSDKHandle handle) {
  if (handle) {
    lip_sync::LipSyncSDK *sdk = (lip_sync::LipSyncSDK *)handle;
    delete sdk;
  }
}

lip_sync::ErrorCode LipSyncSDK_Initialize(LipSyncSDKHandle handle,
                                          const lip_sync::SDKConfig *config) {
  if (!handle || !config) {
    return lip_sync::ErrorCode::INITIALIZATION_FAILED;
  }
  lip_sync::LipSyncSDK *sdk = (lip_sync::LipSyncSDK *)handle;
  return sdk->initialize(*config);
}

lip_sync::ErrorCode
LipSyncSDK_StartProcess(LipSyncSDKHandle handle,
                        const lip_sync::InputPacket *input) {
  if (!handle || !input) {
    return lip_sync::ErrorCode::INVALID_INPUT;
  }
  lip_sync::LipSyncSDK *sdk = (lip_sync::LipSyncSDK *)handle;
  return sdk->startProcess(*input);
}

lip_sync::ErrorCode LipSyncSDK_Terminate(LipSyncSDKHandle handle) {
  if (!handle) {
    return lip_sync::ErrorCode::INVALID_STATE;
  }
  lip_sync::LipSyncSDK *sdk = (lip_sync::LipSyncSDK *)handle;
  return sdk->terminate();
}

lip_sync::ErrorCode LipSyncSDK_TryGetNext(LipSyncSDKHandle handle,
                                          lip_sync::OutputPacket *result) {
  if (!handle || !result) {
    return lip_sync::ErrorCode::INVALID_INPUT;
  }
  lip_sync::LipSyncSDK *sdk = (lip_sync::LipSyncSDK *)handle;
  return sdk->tryGetNext(*result);
}

const char *LipSyncSDK_GetVersion() {
  std::string version = lip_sync::LipSyncSDK::getVersion();
  char *c_version = (char *)malloc(version.length() + 1);
  if (c_version) {
    strcpy(c_version, version.c_str());
  }
  return c_version;
}

void LipSyncSDK_GetVersion_Callback(void (*callback)(const char *)) {
  if (callback) {
    std::string version = lip_sync::LipSyncSDK::getVersion();
    callback(version.c_str());
  }
}