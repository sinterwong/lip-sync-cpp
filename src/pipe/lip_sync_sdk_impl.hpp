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

#include "api/types.h"
#include "core/dnn_infer.hpp"
#include "core/feature_extractor.hpp"
#include "core/image_cycler.hpp"
#include "core/types.hpp"
#include "utils/thread_pool.hpp"
#include "utils/thread_safe_queue.hpp"
#include <atomic>
#include <memory>

namespace lip_sync {

using namespace utils;
class LipSyncSDKImpl {
private:
  // 第一级队列：接收输入数据
  ThreadSafeQueue<InputPacket> inputQueue;

  // 第二级队列：特征处理后的数据
  ThreadSafeQueue<infer::ProcessUnit> processingQueue;

  // 第三级队列：优先队列，确保输出顺序
  ThreadSafePriorityQueue<OutputPacket> outputQueue;

  // 工作线程池
  thread_pool workers;

  // 特征提取器
  std::unique_ptr<infer::FeatureExtractor> featureExtractor;

  // 模型实例池
  std::vector<std::unique_ptr<infer::dnn::AlgoInference>> modelInstances;

  // 状态控制
  std::atomic<bool> isRunning;

  // 图片周期管理器
  std::unique_ptr<pipe::ImageCycler> imageCycler;

public:
  LipSyncSDKImpl();
  ErrorCode initialize(const SDKConfig &config);
  ErrorCode startProcess(const InputPacket &input);
  ErrorCode terminate();
  ErrorCode tryGetNext(OutputPacket &result);

private:
  // workflow functions
};

} // namespace lip_sync

#endif