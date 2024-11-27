/**
 * @file test_wenet_encoder.cc
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-22
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "audio/fbank.hpp"
#include "infer/types.hpp"
#include "infer/wenet_encoder.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <sndfile.h>

#include <filesystem>

#include "logger/logger.hpp"

namespace fs = std::filesystem;
using namespace lip_sync::audio;
using namespace lip_sync::infer;

const auto initLogger = []() -> decltype(auto) {
  LipSyncLoggerInit(true, true, true, true);
  return true;
}();

std::vector<float> readAudioFile(std::string const &filePath) {
  SF_INFO sfinfo;
  SNDFILE *sndfile = sf_open(filePath.c_str(), SFM_READ, &sfinfo);
  if (!sndfile) {
    fprintf(stderr, "Error: could not open audio file: %s\n",
            sf_strerror(sndfile));
    return {};
  }

  int num_frames = sfinfo.frames;
  std::vector<float> audioData(num_frames);
  sf_readf_float(sndfile, audioData.data(), num_frames);
  sf_close(sndfile);
  return audioData;
}

std::vector<float> preprocessAudio(const std::vector<float> &audio) {
  int empty_frames_30 = 32 * 160; // 32 * 160 samples
  int empty_frames_31 = 35 * 160; // 35 * 160 samples

  std::vector<float> paddedAudio;
  paddedAudio.resize(empty_frames_30 + audio.size() + empty_frames_31);

  // Fill the beginning and end with zeros
  std::fill(paddedAudio.begin(), paddedAudio.begin() + empty_frames_30, 0.0f);
  std::transform(audio.begin(), audio.end(),
                 paddedAudio.begin() + empty_frames_30, [](float x) {
                   int16_t ret = static_cast<int16_t>(x * 32767.0f);
                   return static_cast<float>(ret);
                 });
  std::fill(paddedAudio.begin() + empty_frames_30 + audio.size(),
            paddedAudio.end(), 0.0f);

  return paddedAudio;
}

void visualizeFbank(const std::vector<std::vector<float>> &fbank_feature,
                    const std::string &output_path) {
  if (fbank_feature.empty() || fbank_feature[0].empty()) {
    std::cerr << "Empty feature matrix!" << std::endl;
    return;
  }

  // Get dimensions
  int num_frames = fbank_feature.size();
  int num_mel_bins = fbank_feature[0].size();

  // Find min and max values
  float max_val = fbank_feature[0][0];
  float min_val = fbank_feature[0][0];
  for (const auto &frame : fbank_feature) {
    for (float val : frame) {
      max_val = std::max(max_val, val);
      min_val = std::min(min_val, val);
    }
  }

  // Create image matrix
  cv::Mat image(num_mel_bins, num_frames, CV_8UC1);

  // Normalize and fill the image
  float scale = 255.0f / (max_val - min_val);
  for (int i = 0; i < num_frames; ++i) {
    for (int j = 0; j < num_mel_bins; ++j) {
      // Normalize to [0, 255] range
      float normalized_val = (fbank_feature[i][j] - min_val) * scale;
      // Flip Y axis (num_mel_bins - 1 - j)
      image.at<uchar>(num_mel_bins - 1 - j, i) =
          static_cast<uchar>(std::max(0.0f, std::min(255.0f, normalized_val)));
    }
  }

  // Apply color map
  cv::Mat color_image;
  cv::applyColorMap(image, color_image, cv::COLORMAP_JET);

  // Save image
  cv::imwrite(output_path, color_image);
}

void prettyPrintModelInfos(ModelInfo const &modelInfos) {
  std::cout << "Model Name: " << modelInfos.name << std::endl;
  std::cout << "Inputs:" << std::endl;
  for (const auto &input : modelInfos.inputs) {
    std::cout << "  Name: " << input.name << ", Shape: ";
    for (int64_t dim : input.shape) {
      std::cout << dim << " ";
    }
    std::cout << std::endl;
  }
  std::cout << "Outputs:" << std::endl;
  for (const auto &output : modelInfos.outputs) {
    std::cout << "  Name: " << output.name << ", Shape: ";
    for (int64_t dim : output.shape) {
      std::cout << dim << " ";
    }
    std::cout << std::endl;
  }
}

void printMatInfo(const cv::Mat &mat, const std::string &name) {
  std::cout << name << " info:" << std::endl;
  std::cout << "  Dimensions: " << mat.dims << std::endl;
  std::cout << "  Size: ";
  for (int i = 0; i < mat.dims; ++i) {
    std::cout << mat.size[i] << " ";
  }
  std::cout << std::endl;
  std::cout << "  Total elements: " << mat.total() << std::endl;
  std::cout << "  Continuous: " << (mat.isContinuous() ? "yes" : "no")
            << std::endl;
  std::cout << "  Type: " << mat.type() << std::endl;
  std::cout << "  Element size: " << mat.elemSize() << " bytes" << std::endl;
  std::cout << "  Total size: " << mat.total() * mat.elemSize() << " bytes"
            << std::endl;
}

void printFeatureStats(const std::vector<cv::Mat> &wenetFeatures) {
  std::cout << "=== C++ WenetFeatures Statistics ===" << std::endl;
  std::cout << "Total features: " << wenetFeatures.size() << std::endl;

  if (wenetFeatures.empty()) {
    std::cout << "No features to analyze" << std::endl;
    return;
  }

  // 打印第一个特征的统计信息
  const cv::Mat &firstFeature = wenetFeatures.front();
  std::cout << "\nFirst feature shape: " << firstFeature.rows << "x"
            << firstFeature.cols << std::endl;

  // 打印第一行前5个值
  std::cout << "First feature first 5 values: ";
  for (int i = 0; i < std::min(5, firstFeature.cols); ++i) {
    std::cout << firstFeature.at<float>(0, i) << " ";
  }
  std::cout << std::endl;

  cv::Scalar mean, stddev;
  cv::meanStdDev(firstFeature, mean, stddev);
  double minVal, maxVal;
  cv::minMaxLoc(firstFeature, &minVal, &maxVal);

  std::cout << "First feature mean: " << std::fixed << std::setprecision(6)
            << mean[0] << std::endl;
  std::cout << "First feature std: " << stddev[0] << std::endl;
  std::cout << "First feature min: " << minVal << std::endl;
  std::cout << "First feature max: " << maxVal << std::endl;

  // 打印最后一个特征的统计信息
  const cv::Mat &middleFeature = wenetFeatures[31];
  std::cout << "\nMiddle feature shape: " << middleFeature.rows << "x"
            << middleFeature.cols << std::endl;

  std::cout << "Middle feature first 5 values: ";
  for (int i = 0; i < std::min(5, middleFeature.cols); ++i) {
    std::cout << middleFeature.at<float>(0, i) << " ";
  }
  std::cout << std::endl;

  cv::meanStdDev(middleFeature, mean, stddev);
  cv::minMaxLoc(middleFeature, &minVal, &maxVal);

  std::cout << "Middle feature mean: " << mean[0] << std::endl;
  std::cout << "Middle feature std: " << stddev[0] << std::endl;
  std::cout << "Middle feature min: " << minVal << std::endl;
  std::cout << "Middle feature max: " << maxVal << std::endl;
}

int main(int argc, char **argv) {
  fs::path dataDir = fs::path("data");
  fs::path audioPath = dataDir / "test.wav";
  fs::path modelDir = fs::path("models");
  fs::path wenetEncoderPath = modelDir / "wenet_encoder.onnx";

  // Read and preprocess audio
  std::vector<float> audio = readAudioFile(audioPath.string());
  std::vector<float> preprocessedAudio = preprocessAudio(audio);

  // Setup Fbank computer
  FbankComputer::FbankOptions opts;
  opts.num_mel_bins = 80;
  opts.frame_length = 25;
  opts.frame_shift = 10;
  opts.dither = 0.0;
  opts.energy_floor = 1.0;
  opts.sample_frequency = 16000;
  opts.use_log_fbank = true;
  opts.use_power = true;

  // Compute Fbank features
  FbankComputer fbankComputer(opts);
  auto fbankFeatures = fbankComputer.Compute(preprocessedAudio);
  visualizeFbank(fbankFeatures, "fbank_features.png");

  // Initialize WeNet encoder
  AlgoBase algoBase;
  algoBase.name = "wenet_encoder";
  algoBase.modelPath = wenetEncoderPath.string();

  dnn::WeNetEncoderInference wenetEncoder(algoBase);
  if (!wenetEncoder.initialize()) {
    LOGGER_ERROR("Failed to initialize wenet encoder model");
    return 1;
  }

  // Print model info
  ModelInfo modelInfos;
  wenetEncoder.getModelInfo(modelInfos);
  prettyPrintModelInfos(modelInfos);

  return 0;
}