/**
 * @file dnn_infer.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "dnn_infer.hpp"
#include "logger/logger.hpp"

namespace lip_sync::infer::dnn {
bool AlgoInference::initialize() {
  try {
    // create environment
    env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING,
                                     mParams.name.c_str());

    // session options
    Ort::SessionOptions sessionOptions;
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(
        GraphOptimizationLevel::ORT_ENABLE_ALL);

    // create session
#ifdef _WIN32
    // Windows: Convert string to wstring for UTF-16 support
    size_t size = mParams.modelPath.length();
    session = std::make_unique<Ort::Session>(
        *env, 
        mParams.modelPath.c_str(),
        size,  // Pass the size of the model path string
        sessionOptions);
#else
    // Non-Windows platforms can use the original code
    session = std::make_unique<Ort::Session>(
        *env, 
        mParams.modelPath.c_str(),
        sessionOptions);
#endif

    // create memory info
    memoryInfo = std::make_unique<Ort::MemoryInfo>(
        Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));

    // get input info
    Ort::AllocatorWithDefaultOptions allocator;
    size_t numInputNodes = session->GetInputCount();
    inputNames.resize(numInputNodes);
    inputShape.resize(numInputNodes);

    for (size_t i = 0; i < numInputNodes; i++) {
      // get input name
      auto inputName = session->GetInputNameAllocated(i, allocator);
      inputNames[i] = inputName.get();

      // get input shape
      auto typeInfo = session->GetInputTypeInfo(i);
      auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
      inputShape[i] = tensorInfo.GetShape();
    }

    // get output info
    size_t numOutputNodes = session->GetOutputCount();
    outputNames.resize(numOutputNodes);
    outputShapes.resize(numOutputNodes);

    for (size_t i = 0; i < numOutputNodes; i++) {
      // get output name
      auto outputName = session->GetOutputNameAllocated(i, allocator);
      outputNames[i] = outputName.get();

      // get output shape
      auto typeInfo = session->GetOutputTypeInfo(i);
      auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
      outputShapes[i] = tensorInfo.GetShape();
    }

    return true;
  } catch (const Ort::Exception &e) {
    LOGGER_ERROR("ONNX Runtime error during initialization: {}", e.what());
    return false;
  } catch (const std::exception &e) {
    LOGGER_ERROR("Error during initialization: {}", e.what());
    return false;
  }
}

void AlgoInference::terminate() {
  try {
    inputTensors.clear();
    outputTensors.clear();

    session.reset();
    env.reset();
    memoryInfo.reset();

    inputNames.clear();
    inputShape.clear();
    outputNames.clear();
    outputShapes.clear();
  } catch (const std::exception &e) {
    LOGGER_ERROR("Error during termination: {}", e.what());
  }
}

std::shared_ptr<ModelInfo> AlgoInference::getModelInfo() {
  if (modelInfo)
    return modelInfo;

  modelInfo = std::make_shared<ModelInfo>();

  modelInfo->name = mParams.name;
  if (!session) {
    LOGGER_ERROR("Session is not initialized");
    return nullptr;
  }

  try {
    Ort::AllocatorWithDefaultOptions allocator;
    size_t numInputNodes = session->GetInputCount();
    modelInfo->inputs.resize(numInputNodes);
    for (size_t i = 0; i < numInputNodes; i++) {
      auto inputName = session->GetInputNameAllocated(i, allocator);
      modelInfo->inputs[i].name = inputName.get();

      auto typeInfo = session->GetInputTypeInfo(i);
      auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();

      modelInfo->inputs[i].shape = tensorInfo.GetShape();

      size_t numOutputNodes = session->GetOutputCount();
      modelInfo->outputs.resize(numOutputNodes);

      for (size_t i = 0; i < numOutputNodes; i++) {
        auto outputName = session->GetOutputNameAllocated(i, allocator);
        modelInfo->outputs[i].name = outputName.get();
        auto typeInfo = session->GetOutputTypeInfo(i);
        auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
        modelInfo->outputs[i].shape = tensorInfo.GetShape();
      }
    }
  } catch (const Ort::Exception &e) {
    LOGGER_ERROR("ONNX Runtime error during getting model info: {}", e.what());
  } catch (const std::exception &e) {
    LOGGER_ERROR("Error during getting model info: {}", e.what());
  }
  return modelInfo;
}

void AlgoInference::prettyPrintModelInfos() {
  if (!modelInfo) {
    modelInfo = getModelInfo();
    if (!modelInfo) {
      return;
    }
  }
  std::cout << "Model Name: " << modelInfo->name << std::endl;
  std::cout << "Inputs:" << std::endl;
  for (const auto &input : modelInfo->inputs) {
    std::cout << "  Name: " << input.name << ", Shape: ";
    for (int64_t dim : input.shape) {
      std::cout << dim << " ";
    }
    std::cout << std::endl;
  }
  std::cout << "Outputs:" << std::endl;
  for (const auto &output : modelInfo->outputs) {
    std::cout << "  Name: " << output.name << ", Shape: ";
    for (int64_t dim : output.shape) {
      std::cout << dim << " ";
    }
    std::cout << std::endl;
  }
}
}; // namespace lip_sync::infer::dnn
