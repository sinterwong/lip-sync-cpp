#ifndef __LIP_SYNC_SDK_H__
#define __LIP_SYNC_SDK_H__
#include "types.h"
#include <memory>
#include <string>

namespace lip_sync {

class LipSyncSDKImpl;

class LipSyncSDK {
public:
  LipSyncSDK();
  ~LipSyncSDK();

  ErrorCode initialize(const SDKConfig &config);

  ErrorCode startProcess(const InputPacket &input);

  ErrorCode terminate();

  ErrorCode tryGetNext(OutputPacket &result);

  static std::string getVersion();

private:
  std::unique_ptr<LipSyncSDKImpl> impl_;

  LipSyncSDK(const LipSyncSDK &) = delete;
  LipSyncSDK &operator=(const LipSyncSDK &) = delete;
};

} // namespace lip_sync

#endif