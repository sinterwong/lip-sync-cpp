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
#include "audio/audio_processor.hpp"
#include "core/face_processor.hpp"
#include "core/fbank.hpp"
#include "core/feature_extractor.hpp"
#include "core/types.hpp"
#include "core/wavlip.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <sndfile.h>

#include <filesystem>

#include "logger/logger.hpp"

namespace fs = std::filesystem;
using namespace lip_sync::audio;
using namespace lip_sync::infer;
using namespace lip_sync;

const auto initLogger = []() -> decltype(auto) {
  LipSyncLoggerInit(true, true, true, true);
  return true;
}();

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

  // Helper function to calculate statistics
  auto calculateStats = [](const cv::Mat &feature) {
    cv::Scalar mean, stddev;
    cv::meanStdDev(feature, mean, stddev);
    double minVal, maxVal;
    cv::minMaxLoc(feature, &minVal, &maxVal);
    return std::make_tuple(mean[0], stddev[0], minVal, maxVal);
  };

  // Print first 5 values of a feature
  auto printFirst5Values = [](const cv::Mat &feature) {
    std::cout << "First 5 values: [";
    const float *ptr = feature.ptr<float>(0); // 获取第一行
    for (int i = 0; i < 5; ++i) {
      std::cout << std::fixed << std::setprecision(6) << ptr[i];
      if (i < 4)
        std::cout << ", ";
    }
    std::cout << "]" << std::endl;
  };

  // First feature statistics
  if (!wenetFeatures.empty()) {
    const cv::Mat &firstFeature = wenetFeatures[0];
    auto [firstMean, firstStd, firstMin, firstMax] =
        calculateStats(firstFeature);

    std::cout << "\nFirst feature shape: [" << firstFeature.rows << ", "
              << firstFeature.cols << "]" << std::endl;
    printFirst5Values(firstFeature);
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "First feature mean: " << firstMean << std::endl;
    std::cout << "First feature std: " << firstStd << std::endl;
    std::cout << "First feature min: " << firstMin << std::endl;
    std::cout << "First feature max: " << firstMax << std::endl;
  }

  // Middle feature statistics
  if (wenetFeatures.size() > 31) {
    const cv::Mat &middleFeature = wenetFeatures[31];
    auto [middleMean, middleStd, middleMin, middleMax] =
        calculateStats(middleFeature);

    std::cout << "\nMiddle feature shape: [" << middleFeature.rows << ", "
              << middleFeature.cols << "]" << std::endl;
    printFirst5Values(middleFeature);
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Middle feature mean: " << middleMean << std::endl;
    std::cout << "Middle feature std: " << middleStd << std::endl;
    std::cout << "Middle feature min: " << middleMin << std::endl;
    std::cout << "Middle feature max: " << middleMax << std::endl;
  }

  // Last feature statistics
  if (!wenetFeatures.empty()) {
    const cv::Mat &lastFeature = wenetFeatures.back();
    auto [lastMean, lastStd, lastMin, lastMax] = calculateStats(lastFeature);

    std::cout << "\nLast feature shape: [" << lastFeature.rows << ", "
              << lastFeature.cols << "]" << std::endl;
    printFirst5Values(lastFeature);
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Last feature mean: " << lastMean << std::endl;
    std::cout << "Last feature std: " << lastStd << std::endl;
    std::cout << "Last feature min: " << lastMin << std::endl;
    std::cout << "Last feature max: " << lastMax << std::endl;
  }
}

int main(int argc, char **argv) {
  LipSyncLoggerSetLevel(2);

  fs::path dataDir = fs::path("data");
  fs::path audioPath = dataDir / "test.wav";
  fs::path imagePath = dataDir / "image.jpg";
  fs::path modelDir = fs::path("models");
  fs::path wenetEncoderPath = modelDir / "wenet_encoder.onnx";
  fs::path wavToLipPath = modelDir / "w2l_with_wenet.onnx";

  // Read and preprocess audio
  AudioProcessor audioProcessor;
  auto audio = audioProcessor.readAudio(audioPath.string());
  auto preprocessedAudio = audioProcessor.preprocess(audio);

  WeNetConfig wenetConfig;
  wenetConfig.modelPath = wenetEncoderPath.string();
  FeatureExtractor featureExtractor(FbankConfig{}, wenetConfig);

  if (!featureExtractor.initialize()) {
    LOGGER_ERROR("Failed to initialize feature extractor");
    return 1;
  }

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

  auto fbankFeatures = featureExtractor.computeFbank(preprocessedAudio);
  visualizeFbank(fbankFeatures, "fbank_feature.png");

  auto wenetFeatures = featureExtractor.extractWenetFeatures(fbankFeatures);
  printFeatureStats(wenetFeatures);

  auto audioChunks = featureExtractor.convertToChunks(wenetFeatures);
  printFeatureStats(audioChunks);

  // Initialize wav to lip engine
  AlgoBase wavToLipAlgoBase;
  wavToLipAlgoBase.name = "wavlip";
  wavToLipAlgoBase.modelPath = wavToLipPath.string();

  dnn::WavToLipInference wavToLip(wavToLipAlgoBase);
  if (!wavToLip.initialize()) {
    LOGGER_ERROR("Failed to initialize wav to lip model");
    return 1;
  }
  wavToLip.prettyPrintModelInfos();

  // load face image
  cv::Mat frame = cv::imread(imagePath.string());
  if (frame.empty()) {
    LOGGER_ERROR("Failed to load image");
    return 1;
  }
  cv::Rect faceBbox(476, 832, 645 - 476, 1001 - 832);

  // Preprocess face
  FaceProcessor processor(160, 4);
  ProcessedFaceData processed = processor.preProcess(frame, faceBbox);

  printMatInfo(processed.xData, "processed.xData");

  WeNetInput wavToLipInput;
  wavToLipInput.image = processed.xData;
  wavToLipInput.audioFeature = audioChunks[0];

  AlgoInput wavToLipInputParam;
  wavToLipInputParam.setParams(wavToLipInput);

  AlgoOutput wavToLipOutputParam;
  WeNetOutput wavToLipOutput;
  wavToLipOutputParam.setParams(wavToLipOutput);

  if (!wavToLip.infer(wavToLipInputParam, wavToLipOutputParam)) {
    LOGGER_ERROR("Failed to run wav to lip inference");
    return 1;
  }

  auto *output = wavToLipOutputParam.getParams<WeNetOutput>();
  if (output) {
    std::cout << "Mel size: " << output->mel.size() << std::endl;
  }

  // post
  cv::Mat postProcessedFrame =
      processor.postProcess(output->mel, processed, frame);
  cv::imwrite("post_processed_frame.png", postProcessedFrame);

  return 0;
}