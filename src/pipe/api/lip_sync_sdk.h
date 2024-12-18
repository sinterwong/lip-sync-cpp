#ifndef __LIP_SYNC_SDK_H__
#define __LIP_SYNC_SDK_H__
#include "lip_sync_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *LipSyncSDKHandle;

LipSyncSDKHandle LipSyncSDK_Create();
void LipSyncSDK_Destroy(LipSyncSDKHandle handle);
lip_sync::ErrorCode LipSyncSDK_Initialize(LipSyncSDKHandle handle,
                                          const lip_sync::SDKConfig *config);
lip_sync::ErrorCode LipSyncSDK_StartProcess(LipSyncSDKHandle handle,
                                            const lip_sync::InputPacket *input);
lip_sync::ErrorCode LipSyncSDK_Terminate(LipSyncSDKHandle handle);
lip_sync::ErrorCode LipSyncSDK_TryGetNext(LipSyncSDKHandle handle,
                                          lip_sync::OutputPacket *result);

const char *LipSyncSDK_GetVersion();
void LipSyncSDK_GetVersion_Callback(void (*callback)(const char *));

#ifdef __cplusplus
}
#endif

#endif