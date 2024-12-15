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
#include <cmath>
#include <opencv2/core/types.hpp>
#include <string>

namespace lip_sync {

using namespace lip_sync::infer;
LipSyncSDKImpl::LipSyncSDKImpl() : isRunning(false) {}

ErrorCode LipSyncSDKImpl::initialize(const SDKConfig &config) {
  WeNetConfig wenetConfig;
  wenetConfig.modelPath = config.encoderModelPath;
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
    AlgoBase algoBase;
    algoBase.name = "wavlip-" + std::to_string(i);
    algoBase.modelPath = config.wavLipModelPath;
    auto model = std::make_unique<ModelInstance>(algoBase);

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

  samplesPerFrame = std::round(audioSampleRate / config.frameRate);

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
  auto ret = outputQueue.wait_pop_for(std::chrono::milliseconds(100));
  if (!ret.has_value()) {
    return ErrorCode::TRY_GET_NEXT_OVERTIME;
  }
  result = std::move(ret.value());
  return ErrorCode::SUCCESS;
}

void LipSyncSDKImpl::inputProcessLoop() {
  while (isRunning) {
    InputPacket input;
    auto result = inputQueue.wait_pop_for(std::chrono::milliseconds(100));
    if (!result) {
      continue;
    }
    input = std::move(*result);
    auto [audio, audioChunks] = input.audioData.empty()
                                    ? processAudioInput(input.audioPath)
                                    : processAudioInput(input.audioData);
    storeAudio(input.uuid, std::move(audio));

    if (audioChunks.empty())
      continue;

    for (size_t i = 0; i < audioChunks.size(); ++i) {
      ProcessUnit unit;
      unit.uuid = input.uuid;
      unit.sequence = i;
      unit.audioChunk = audioChunks[i];
      unit.audioSegment = getAudioSegment(input.uuid, i);
      unit.isLastChunk = i == audioChunks.size() - 1;
      unit.timestamp = utils::getCurrentTimestamp();

      auto [image, bbox] = imageCycler->getNextImage();
      unit.faceData = faceProcessor->preProcess(
          *image,
          cv::Rect{bbox[0], bbox[1], bbox[2] - bbox[0], bbox[3] - bbox[1]});

      unit.originImage = image;

      // 分配模型实例并创建任务
      size_t modelIndex = i % modelInstances.size();
      Task task;
      task.unit = std::move(unit);
      task.modelIndex = modelIndex;
      taskQueue.push(std::move(task));
    }
  }
}

void LipSyncSDKImpl::processLoop() {
  while (isRunning) {
    auto task = taskQueue.wait_pop_for(std::chrono::milliseconds(100));
    if (!task) {
      std::this_thread::yield(); // 让出CPU
      continue;
    }

    bool acquired = false;
    size_t actualModelIndex = task->modelIndex;

    // 如果预期的模型实例忙，尝试其他模型实例
    for (size_t i = 0; i < modelInstances.size(); ++i) {
      actualModelIndex = (task->modelIndex + i) % modelInstances.size();
      if (modelInstances[actualModelIndex]->tryAcquire()) {
        acquired = true;
        break;
      }
    }

    if (!acquired) {
      // 如果所有模型都忙，将任务重新放回队列
      taskQueue.push(std::move(*task));
      std::this_thread::yield(); // 让出CPU
      continue;
    }

    auto *model = modelInstances[actualModelIndex]->get();

    // 执行推理
    AlgoInput algoInput;
    WeNetInput wenetInput;
    wenetInput.audioFeature = task->unit.audioChunk;
    wenetInput.image = task->unit.faceData.xData;
    algoInput.setParams(wenetInput);

    AlgoOutput algoOutput;
    WeNetOutput wenetOutput;
    algoOutput.setParams(wenetOutput);

    if (!model->infer(algoInput, algoOutput)) {
      LOGGER_ERROR("Failed to run wav to lip inference");
      continue;
    }

    // 释放当前模型实例
    modelInstances[actualModelIndex]->release();

    auto *output = algoOutput.getParams<WeNetOutput>();
    if (!output) {
      LOGGER_ERROR("Failed to get wav to lip output");
      continue;
    }

    cv::Mat postProcessedFrame = faceProcessor->postProcess(
        output->mel, task->unit.faceData, *task->unit.originImage);

    OutputPacket outputPacket;
    outputPacket.uuid = task->unit.uuid;
    outputPacket.sequence = task->unit.sequence;
    outputPacket.timestamp = task->unit.timestamp;
    outputPacket.width = faceProcessor->getInputSize();
    outputPacket.height = faceProcessor->getInputSize();
    outputPacket.audioData = task->unit.audioSegment;
    outputPacket.sampleRate = 16000;
    outputPacket.channels = 1;
    cv::imencode(".png", postProcessedFrame, outputPacket.frameData);

    outputQueue.push(std::move(outputPacket));
  }
}

std::pair<std::vector<float>, std::vector<cv::Mat>>
LipSyncSDKImpl::processAudioInput(const std::string &audioPath) {
  audio::AudioProcessor audioProcessor;
  auto audio = audioProcessor.readAudio(audioPath);
  if (audio.empty()) {
    return {};
  }

  auto preprocessedAudio = audioProcessor.preprocess(audio);
  auto fbankFeatures = featureExtractor->computeFbank(preprocessedAudio);
  auto wenetFeatures = featureExtractor->extractWenetFeatures(fbankFeatures);
  return {audio, featureExtractor->convertToChunks(wenetFeatures)};
}

std::pair<std::vector<float>, std::vector<cv::Mat>>
LipSyncSDKImpl::processAudioInput(const std::vector<float> &audio) {
  if (audio.empty()) {
    return {};
  }

  audio::AudioProcessor audioProcessor;
  auto preprocessedAudio = audioProcessor.preprocess(audio);
  auto fbankFeatures = featureExtractor->computeFbank(preprocessedAudio);
  auto wenetFeatures = featureExtractor->extractWenetFeatures(fbankFeatures);
  return {audio, featureExtractor->convertToChunks(wenetFeatures)};
}

void LipSyncSDKImpl::storeAudio(const std::string &uuid,
                                std::vector<float> &&samples) {
  std::lock_guard<std::mutex> lock(audioStorageMutex);
  audioStorage[uuid] =
      AudioData{std::move(samples), uuid, utils::getCurrentTimestamp()};
}

std::vector<float> LipSyncSDKImpl::getAudioSegment(const std::string &uuid,
                                                   size_t startFrame) {
  std::lock_guard<std::mutex> lock(audioStorageMutex);
  size_t startSample = startFrame * samplesPerFrame;

  auto &audioData = audioStorage[uuid];

  if (startSample >= audioData.samples.size()) {
    // 补0返回
    return std::vector<float>(samplesPerFrame, 0.0f);
  }
  return std::vector<float>(
      audioData.samples.begin() + startSample,
      audioData.samples.begin() +
          std::min(startSample + samplesPerFrame, audioData.samples.size()));
}

} // namespace lip_sync
