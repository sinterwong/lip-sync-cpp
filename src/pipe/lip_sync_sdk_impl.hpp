/**
 * @file lip_sync_sdk_impl.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __LIP_SYNC_SDK_IMPL_HPP__
#define __LIP_SYNC_SDK_IMPL_HPP__

#include "core/face_processor.hpp"
#include "core/feature_extractor.hpp"
#include "core/image_cycler.hpp"
#include "core/types.hpp"
#include "lip_sync_types.h"
#include "utils/thread_pool.hpp"
#include "utils/thread_safe_queue.hpp"
#include "wav_lip_manager.hpp"
#include <atomic>
#include <memory>

namespace lip_sync {

using namespace utils;

struct OutputPacketComparator {
  bool operator()(const OutputPacket &a, const OutputPacket &b) {
    if (a.sequence == b.sequence) {
      return a.timestamp > b.timestamp;
    }
    return a.sequence > b.sequence;
  }
};

class LipSyncSDKImpl {
private:
  // 第一级队列：接收输入数据
  ThreadSafeQueue<InputPacket> inputQueue;

  // 第二级队列：特征处理后的数据
  ThreadSafeQueue<infer::ProcessUnit> processingQueue;

  // 第三级队列：优先队列，尽可能保证输出顺序
  ThreadSafePriorityQueue<OutputPacket, OutputPacketComparator> outputQueue;

  // 工作线程池
  thread_pool workers;

  // 特征提取器
  std::unique_ptr<infer::FeatureExtractor> featureExtractor;

  // 模型实例池
  std::vector<std::unique_ptr<ModelInstance>> modelInstances;

  // 状态控制
  std::atomic<bool> isRunning;

  // 图片周期管理器
  std::unique_ptr<pipe::ImageCycler> imageCycler;

  // 人脸处理器
  std::unique_ptr<infer::FaceProcessor> faceProcessor;

  // 单独的输入处理线程
  std::thread inputProcessThread;

  // 线程池任务队列
  struct Task {
    infer::ProcessUnit unit;
    size_t modelIndex;
  };
  ThreadSafeQueue<Task> taskQueue;

  struct AudioData {
    std::vector<float> samples;
    std::string uuid;
    int64_t timestamp;
  };
  std::map<std::string, AudioData> audioStorage;
  std::mutex audioStorageMutex;

  // 音频采样率
  float audioSampleRate = 16000.0f;

  // 每帧对应的音频采样点数
  size_t samplesPerFrame;

public:
  LipSyncSDKImpl();
  ErrorCode initialize(const SDKConfig &config);
  ErrorCode startProcess(const InputPacket &input);
  ErrorCode terminate();
  ErrorCode tryGetNext(OutputPacket &result);

private:
  void inputProcessLoop();
  void processLoop();
  std::pair<std::vector<float>, std::vector<cv::Mat>>
  processAudioInput(const std::string &audioPath);

  std::pair<std::vector<float>, std::vector<cv::Mat>>
  processAudioInput(const std::vector<float> &audio);

  void storeAudio(const std::string &uuid, std::vector<float> &&samples);

  std::vector<float> getAudioSegment(const std::string &uuid,
                                     size_t startFrame);
};

} // namespace lip_sync

#endif