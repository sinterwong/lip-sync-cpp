#ifndef __LIP_SYNC_TYPES_H__
#define __LIP_SYNC_TYPES_H__
#include <cstdint>
#include <string>
#include <vector>

namespace lip_sync {

struct SDKConfig {
  uint32_t numWorkers{4};       // 工作线程数量
  std::string wavLipModelPath;  // 唇音同步模型路径
  std::string encoderModelPath; // 音频编码模型路径
  std::string frameDir;         // 帧路径
};

struct InputPacket {
  std::string audioPath; // 音频文件
  std::string uuid;      // 数据标识
};

struct OutputPacket {
  std::vector<uint8_t> frameData; // 生成的视频帧数据
  uint32_t width;                 // 帧宽度
  uint32_t height;                // 帧高度
  std::vector<float> audioData;   // 对应的音频段数据
  uint32_t sampleRate;            // 音频采样率
  uint32_t channels;              // 音频通道数
  int64_t timestamp;              // 时间戳(微秒)
  int64_t sequence;               // 序列号
};

enum class ErrorCode {
  SUCCESS = 0,
  INVALID_INPUT = -1,
  FILE_NOT_FOUND = -2,
  INVALID_FILE_FORMAT = -3,
  INITIALIZATION_FAILED = -4,
  PROCESSING_ERROR = -5,
  INVALID_STATE = -6
};
} // namespace lip_sync
#endif