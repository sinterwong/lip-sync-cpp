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
#include "audio/audio_processor.hpp"
#include "core/types.hpp"
#include "logger/logger.hpp"
#include "utils/time_utils.hpp"
#include "wav_lip_manager.hpp"
#include <opencv2/core/types.hpp>
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

  imageCycler = std::make_unique<pipe::ImageCycler>(
      config.frameDir, config.faceInfoPath, config.maxCacheSize);

  workers.start(config.numWorkers);
  // 初始化模型实例，数量与线程数相同
  for (int i = 0; i < config.numWorkers; ++i) {
    auto model = std::make_unique<ModelInstance>(
        AlgoBase{.name = "wavlip-" + std::to_string(i),
                 .modelPath = config.wavLipModelPath});

    if (!model->initialize()) {
      LOGGER_ERROR("Failed to initialize wav to lip model {}", i);
      return ErrorCode::INITIALIZATION_FAILED;
    }
    modelInstances.push_back(std::move(model));
  }

  isRunning.store(true);
  inputProcessThread = std::thread(&LipSyncSDKImpl::inputProcessLoop, this);

  // 启动工作线程池，每个线程执行processLoop
  for (int i = 0; i < config.numWorkers; ++i) {
    workers.submit([this]() { processLoop(); });
  }

  faceProcessor =
      std::make_unique<FaceProcessor>(config.faceSize, config.facePad);

  isRunning.store(true);
  return ErrorCode::SUCCESS;
}

ErrorCode LipSyncSDKImpl::startProcess(const InputPacket &input) {
  if (!isRunning) {
    return ErrorCode::INVALID_STATE;
  }
  inputQueue.push(input);
  return ErrorCode::SUCCESS;
}

ErrorCode LipSyncSDKImpl::terminate() {
  if (!isRunning.exchange(false)) {
    return ErrorCode::SUCCESS;
  }

  // 等待输入处理线程结束
  if (inputProcessThread.joinable()) {
    inputProcessThread.join();
  }

  // 清理所有队列
  inputQueue.clear();
  processingQueue.clear();
  taskQueue.clear();
  outputQueue.clear();

  // 停止工作线程池
  workers.stop();

  return ErrorCode::SUCCESS;
}

ErrorCode LipSyncSDKImpl::tryGetNext(OutputPacket &result) {
  result = outputQueue.wait_pop();
  return ErrorCode::SUCCESS;
}

void LipSyncSDKImpl::inputProcessLoop() {
  while (isRunning) {
    InputPacket input = inputQueue.wait_pop();
    auto audioChunks = processAudioInput(input.audioPath);
    if (audioChunks.empty())
      continue;

    for (size_t i = 0; i < audioChunks.size(); ++i) {
      ProcessUnit unit;
      unit.uuid = input.uuid;
      unit.sequence = i;
      unit.audioChunk = audioChunks[i];
      unit.isLastChunk = i == audioChunks.size() - 1;
      unit.timestamp = utils::getCurrentTimestamp();

      auto [image, bbox] = imageCycler->getNextImage();
      unit.faceData = faceProcessor->preProcess(
          *image,
          cv::Rect{bbox[0], bbox[1], bbox[2] - bbox[0], bbox[3] - bbox[1]});

      unit.originImage = image;

      // 分配模型实例并创建任务
      size_t modelIndex = i % modelInstances.size();
      Task task{.unit = std::move(unit), .modelIndex = modelIndex};
      taskQueue.push(std::move(task));
    }
  }
}

void LipSyncSDKImpl::processLoop() {
  while (isRunning) {
    Task task = taskQueue.wait_pop();

    // 尝试获取预期的模型实例
    bool acquired = false;
    size_t actualModelIndex = task.modelIndex;

    // 如果预期的模型实例忙，尝试其他模型实例
    for (size_t i = 0; i < modelInstances.size(); ++i) {
      actualModelIndex = (task.modelIndex + i) % modelInstances.size();
      if (modelInstances[actualModelIndex]->tryAcquire()) {
        acquired = true;
        break;
      }
    }

    if (!acquired) {
      // 如果所有模型都忙，将任务重新放回队列
      taskQueue.push(std::move(task));
      std::this_thread::yield(); // 让出CPU
      continue;
    }

    auto modelGuard = std::unique_ptr<void, std::function<void(void *)>>(
        nullptr, [this, actualModelIndex](void *) {
          modelInstances[actualModelIndex]->release();
        });

    auto *model = modelInstances[actualModelIndex]->get();

    // 执行推理
    AlgoInput algoInput;
    WeNetInput wenetInput{.audioFeature = task.unit.audioChunk,
                          .image = task.unit.faceData.xData};
    algoInput.setParams(wenetInput);

    AlgoOutput algoOutput;
    WeNetOutput wenetOutput;
    algoOutput.setParams(wenetOutput);

    if (!model->infer(algoInput, algoOutput)) {
      LOGGER_ERROR("Failed to run wav to lip inference");
      continue;
    }

    auto *output = algoOutput.getParams<WeNetOutput>();
    if (!output) {
      LOGGER_ERROR("Failed to get wav to lip output");
      continue;
    }

    cv::Mat postProcessedFrame = faceProcessor->postProcess(
        output->mel, task.unit.faceData, *task.unit.originImage);

    OutputPacket outputPacket;
    outputPacket.uuid = task.unit.uuid;
    outputPacket.sequence = task.unit.sequence;
    outputPacket.timestamp = task.unit.timestamp;
    outputPacket.width = faceProcessor->getInputSize();
    outputPacket.height = faceProcessor->getInputSize();
    outputPacket.audioData = {};
    outputPacket.sampleRate = 16000;
    outputPacket.channels = 1;

    outputQueue.push(std::move(outputPacket));
  }
}

std::vector<cv::Mat>
LipSyncSDKImpl::processAudioInput(const std::string &audioPath) {
  audio::AudioProcessor audioProcessor;
  auto audio = audioProcessor.readAudio(audioPath);
  if (audio.empty()) {
    return {};
  }

  auto preprocessedAudio = audioProcessor.preprocess(audio);
  auto fbankFeatures = featureExtractor->computeFbank(preprocessedAudio);
  auto wenetFeatures = featureExtractor->extractWenetFeatures(fbankFeatures);
  return featureExtractor->convertToChunks(wenetFeatures);
}

} // namespace lip_sync
