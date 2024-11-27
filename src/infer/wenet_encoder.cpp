/**
 * @file wenet_encoder.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-22
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "wenet_encoder.hpp"
#include "logger/logger.hpp"

namespace lip_sync::infer::dnn {

bool WeNetEncoderInference::infer(AlgoInput &input, AlgoOutput &output) {
  // Get input parameters
  auto *encoderInput = input.getParams<WeNetEncoderInput>();
  if (!encoderInput) {
    return false;
  }

  auto *encoderOutput = output.getParams<WeNetEncoderOutput>();
  if (!encoderOutput) {
    return false;
  }

  try {
    // Prepare input tensors
    std::vector<float> chunkData;
    cv::Mat chunk = encoderInput->chunk;
    if (chunk.empty() || chunk.type() != CV_32F) {
      LOGGER_ERROR("Invalid chunk data");
      return false;
    }

    // Convert Mat to float vector using total() for all dimensions
    chunkData.assign((float *)chunk.data, (float *)chunk.data + chunk.total());

    LOGGER_DEBUG("Chunk data size: {}", chunkData.size());

    std::vector<float> attCacheData;
    cv::Mat attCache = encoderInput->attCache;
    if (!attCache.empty()) {
      attCacheData.assign((float *)attCache.data,
                          (float *)attCache.data + attCache.total());
    }
    LOGGER_DEBUG("AttCache data size: {}", attCacheData.size());

    std::vector<float> cnnCacheData;
    cv::Mat cnnCache = encoderInput->cnnCache;
    if (!cnnCache.empty()) {
      cnnCacheData.assign((float *)cnnCache.data,
                          (float *)cnnCache.data + cnnCache.total());
    }
    LOGGER_DEBUG("CNNCache data size: {}", cnnCacheData.size());

    // Create input tensors
    inputTensors.clear();

    // Print shape information for debugging
    LOGGER_DEBUG("Input shape 0 size: {}", inputShape[0].size());
    for (size_t i = 0; i < inputShape[0].size(); ++i) {
      LOGGER_DEBUG("Input shape 0[{}]: {}", i, inputShape[0][i]);
    }

    // Add chunk tensor
    inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
        *memoryInfo, chunkData.data(), chunkData.size(), inputShape[0].data(),
        inputShape[0].size()));

    // Add offset tensor
    std::vector<int64_t> offsetData = {encoderInput->offset};
    inputTensors.emplace_back(Ort::Value::CreateTensor<int64_t>(
        *memoryInfo, offsetData.data(), offsetData.size(), inputShape[1].data(),
        inputShape[1].size()));

    // Add att_cache tensor if present
    if (!attCacheData.empty()) {
      inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
          *memoryInfo, attCacheData.data(), attCacheData.size(),
          inputShape[2].data(), inputShape[2].size()));
    }

    // Add cnn_cache tensor if present
    if (!cnnCacheData.empty()) {
      inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
          *memoryInfo, cnnCacheData.data(), cnnCacheData.size(),
          inputShape[3].data(), inputShape[3].size()));
    }

    LOGGER_DEBUG("Number of input tensors created: {}", inputTensors.size());

    // Convert input/output names to const char* array
    std::vector<const char *> input_names_ptr;
    std::vector<const char *> output_names_ptr;

    for (const auto &name : inputNames) {
      input_names_ptr.push_back(name.c_str());
    }
    for (const auto &name : outputNames) {
      output_names_ptr.push_back(name.c_str());
    }

    // Run inference
    outputTensors = session->Run(
        Ort::RunOptions{nullptr}, input_names_ptr.data(), inputTensors.data(),
        inputTensors.size(), output_names_ptr.data(), output_names_ptr.size());

    // Process output tensors
    if (outputTensors.size() != 3) {
      LOGGER_ERROR("Unexpected number of output tensors: {}",
                   outputTensors.size());
      return false;
    }

    // Get output data
    float *outData = outputTensors[0].GetTensorMutableData<float>();
    size_t outSize =
        outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
    encoderOutput->data.assign(outData, outData + outSize);

    // Get attention cache
    float *attCacheOut = outputTensors[1].GetTensorMutableData<float>();
    size_t attCacheSize =
        outputTensors[1].GetTensorTypeAndShapeInfo().GetElementCount();
    encoderOutput->RAttCache.assign(attCacheOut, attCacheOut + attCacheSize);

    // Get CNN cache
    float *cnnCacheOut = outputTensors[2].GetTensorMutableData<float>();
    size_t cnnCacheSize =
        outputTensors[2].GetTensorTypeAndShapeInfo().GetElementCount();
    encoderOutput->RCNNCache.assign(cnnCacheOut, cnnCacheOut + cnnCacheSize);

    return true;
  } catch (const Ort::Exception &e) {
    LOGGER_ERROR("ONNX Runtime error during inference: {}", e.what());
    return false;
  } catch (const std::exception &e) {
    LOGGER_ERROR("Error during inference: {}", e.what());
    return false;
  }
}

} // namespace lip_sync::infer::dnn
