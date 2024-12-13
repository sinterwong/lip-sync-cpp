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
#include "types.hpp"

namespace lip_sync::infer::dnn {

bool WavToLipInference::infer(AlgoInput &input, AlgoOutput &output) {
  auto *wavToLipInput = input.getParams<WeNetInput>();
  if (!wavToLipInput) {
    // LOGGER_ERROR("Invalid input parameters");
    std::cerr << "Invalid input parameters" << std::endl;
    return false;
  }

  auto *wavToLipOutput = output.getParams<WeNetOutput>();
  if (!wavToLipOutput) {
    // LOGGER_ERROR("Invalid output parameters");
    std::cerr << "Invalid output parameters" << std::endl;
    return false;
  }

  try {
    inputTensors.clear();

    if (wavToLipInput->image.empty() || wavToLipInput->image.type() != CV_32F) {
      // LOGGER_ERROR("Invalid image data");
      std::cerr << "Invalid image data" << std::endl;
      return false;
    }

    std::vector<int64_t> imageShape = inputShape[0]; // Copy the original shape
    if (imageShape[0] == -1) { // Replace dynamic batch size with 1
      imageShape[0] = 1;
    }
    // LOGGER_DEBUG("Actual image tensor shape: {}x{}x{}x{}", imageShape[0],
    //              imageShape[1], imageShape[2], imageShape[3]);

    std::vector<int64_t> audioShape = inputShape[1]; // Copy the original shape
    if (audioShape[0] == -1) { // Replace dynamic batch size with 1
      audioShape[0] = 1;
    }
    // LOGGER_DEBUG("Actual audio tensor shape: {}x{}x{}x{}", audioShape[0],
    //              audioShape[1], audioShape[2], audioShape[3]);

    inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
        *memoryInfo, (float *)wavToLipInput->image.data,
        wavToLipInput->image.total() * wavToLipInput->image.channels(),
        imageShape.data(), imageShape.size()));

    if (wavToLipInput->audioFeature.empty() ||
        wavToLipInput->audioFeature.type() != CV_32F) {
      // LOGGER_ERROR("Invalid audio feature data");
      std::cerr << "Invalid audio feature data" << std::endl;
      return false;
    }

    inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
        *memoryInfo, (float *)wavToLipInput->audioFeature.data,
        wavToLipInput->audioFeature.total(), audioShape.data(),
        audioShape.size()));

    // LOGGER_DEBUG("Number of input tensors created: {}", inputTensors.size());

    std::vector<const char *> inputNamesPtr;
    std::vector<const char *> outputNamesPtr;

    for (const auto &name : inputNames) {
      inputNamesPtr.push_back(name.c_str());
    }
    for (const auto &name : outputNames) {
      outputNamesPtr.push_back(name.c_str());
    }

    outputTensors = session->Run(Ort::RunOptions{nullptr}, inputNamesPtr.data(),
                                 inputTensors.data(), inputTensors.size(),
                                 outputNamesPtr.data(), outputNamesPtr.size());

    if (outputTensors.empty()) {
      // LOGGER_ERROR("No output tensors produced");
      std::cerr << "No output tensors produced" << std::endl;
      return false;
    }

    float *outData = outputTensors[0].GetTensorMutableData<float>();
    size_t outSize =
        outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();

    wavToLipOutput->mel.assign(outData, outData + outSize);

    // LOGGER_DEBUG("Output mel size: {}", wavToLipOutput->mel.size());

    return true;

  } catch (const Ort::Exception &e) {
    // LOGGER_ERROR("ONNX Runtime error during inference: {}", e.what());
    std::cerr << "ONNX Runtime error during inference: " << e.what()
              << std::endl;
    return false;
  } catch (const std::exception &e) {
    // LOGGER_ERROR("Error during inference: {}", e.what());
    std::cerr << "Error during inference: " << e.what() << std::endl;
    return false;
  }
}
} // namespace lip_sync::infer::dnn
