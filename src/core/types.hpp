/**
 * @file types.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __INFERENCE_TYPES_H__
#define __INFERENCE_TYPES_H__

#include <opencv2/opencv.hpp>
#include <string>
#include <variant>

namespace lip_sync::infer {

struct WeNetInput {
  cv::Mat audioFeature;
  cv::Mat image;
};

struct WeNetEncoderInput {
  cv::Mat chunk;
  int offset;
  cv::Mat attCache;
  cv::Mat cnnCache;
};

struct ModelInfo {
  std::string name;

  struct InputInfo {
    std::string name;
    std::vector<int64_t> shape;
  };

  struct OutputInfo {
    std::string name;
    std::vector<int64_t> shape;
  };

  std::vector<InputInfo> inputs;
  std::vector<OutputInfo> outputs;
};

class AlgoInput {
public:
  using Params = std::variant<WeNetInput, WeNetEncoderInput>;

  template <typename T> void setParams(T params) {
    params_ = std::move(params);
  }

  template <typename Func> void visitParams(Func &&func) {
    std::visit([&](auto &&params) { std::forward<Func>(func)(params); },
               params_);
  }

  template <typename T> T *getParams() { return std::get_if<T>(&params_); }

private:
  Params params_;
};

struct WeNetOutput {
  std::vector<float> mel;
};

struct WeNetEncoderOutput {
  std::vector<float> data;
  std::vector<float> RAttCache;
  std::vector<float> RCNNCache;
};

class AlgoOutput {
public:
  using Params = std::variant<WeNetOutput, WeNetEncoderOutput>;

  template <typename T> void setParams(T params) {
    params_ = std::move(params);
  }

  template <typename Func> void visitParams(Func &&func) {
    std::visit([&](auto &&params) { std::forward<Func>(func)(params); },
               params_);
  }

  template <typename T> T *getParams() { return std::get_if<T>(&params_); }

private:
  Params params_;
};

struct AlgoBase {
  std::string name;
  std::string modelPath;
};

struct ProcessedFaceData {
  cv::Mat xData;
  cv::Mat faceCropLarge;
  cv::Rect boundingBox;
};

struct FbankConfig {
  int numMelBins = 80;
  int frameLength = 25;
  int frameShift = 10;
  bool useLogFbank = true;
  bool usePower = true;
  float dither = 0.0f;
  float energyFloor = 1.0f;
  int sampleFrequency = 16000;
};

struct WeNetConfig {
  int batchSize = 1;
  int framesStride = 67;
  int numFeatures = 80;
  int slidingStep = 5;
  std::string modelPath;
};

struct ProcessUnit {};

} // namespace lip_sync::infer
#endif
