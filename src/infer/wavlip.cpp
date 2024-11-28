/**
 * @file wavlip.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "wavlip.hpp"
#include "logger/logger.hpp"
#include "types.hpp"

namespace lip_sync::infer::dnn {

bool WavToLipInference::infer(AlgoInput &input, AlgoOutput &output) {
  // Get input parameters
  auto *wavToLipInput = input.getParams<WeNetInput>();
  if (!wavToLipInput) {
    LOGGER_ERROR("Invalid input parameters");
    return false;
  }

  auto *wavToLipOutput = output.getParams<WeNetOutput>();
  if (!wavToLipOutput) {
    LOGGER_ERROR("Invalid output parameters");
    return false;
  }

  try {
    // Prepare input tensors
    inputTensors.clear();

    // Process image data
    if (wavToLipInput->image.empty() || wavToLipInput->image.type() != CV_32F) {
      LOGGER_ERROR("Invalid image data");
      return false;
    }

    // Create shape vectors for actual inference
    std::vector<int64_t> imageShape = inputShape[0]; // Copy the original shape
    if (imageShape[0] == -1) { // Replace dynamic batch size with 1
      imageShape[0] = 1;
    }
    LOGGER_DEBUG("Actual image tensor shape: {}x{}x{}x{}", imageShape[0],
                 imageShape[1], imageShape[2], imageShape[3]);

    std::vector<int64_t> audioShape = inputShape[1]; // Copy the original shape
    if (audioShape[0] == -1) { // Replace dynamic batch size with 1
      audioShape[0] = 1;
    }
    LOGGER_DEBUG("Actual audio tensor shape: {}x{}x{}x{}", audioShape[0],
                 audioShape[1], audioShape[2], audioShape[3]);

    // Create input tensors with corrected shapes
    inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
        *memoryInfo, (float *)wavToLipInput->image.data,
        wavToLipInput->image.total() * wavToLipInput->image.channels(),
        imageShape.data(), imageShape.size()));

    // Process audio feature data
    if (wavToLipInput->audioFeature.empty() ||
        wavToLipInput->audioFeature.type() != CV_32F) {
      LOGGER_ERROR("Invalid audio feature data");
      return false;
    }

    inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
        *memoryInfo, (float *)wavToLipInput->audioFeature.data,
        wavToLipInput->audioFeature.total(), audioShape.data(),
        audioShape.size()));

    LOGGER_DEBUG("Number of input tensors created: {}", inputTensors.size());

    // Convert input/output names to const char* array
    std::vector<const char *> inputNamesPtr;
    std::vector<const char *> outputNamesPtr;

    for (const auto &name : inputNames) {
      inputNamesPtr.push_back(name.c_str());
    }
    for (const auto &name : outputNames) {
      outputNamesPtr.push_back(name.c_str());
    }

    // Run inference
    outputTensors = session->Run(Ort::RunOptions{nullptr}, inputNamesPtr.data(),
                                 inputTensors.data(), inputTensors.size(),
                                 outputNamesPtr.data(), outputNamesPtr.size());

    // Process output tensors
    if (outputTensors.empty()) {
      LOGGER_ERROR("No output tensors produced");
      return false;
    }

    // Get output data
    float *outData = outputTensors[0].GetTensorMutableData<float>();
    size_t outSize =
        outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();

    // Assign output data to mel vector
    wavToLipOutput->mel.assign(outData, outData + outSize);

    LOGGER_DEBUG("Output mel size: {}", wavToLipOutput->mel.size());

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
