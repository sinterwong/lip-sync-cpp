/**
 * @file feature_extractor.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "feature_extractor.hpp"

namespace lip_sync::infer {
FeatureExtractor::FeatureExtractor(const FbankConfig &fbankConfig,
                                   const WeNetConfig &wenetConfig)
    : fbankConfig_(fbankConfig), wenetConfig_(wenetConfig) {}

bool FeatureExtractor::initialize() {
  FbankComputer::FbankOptions opts;
  opts.num_mel_bins = fbankConfig_.numMelBins;
  opts.frame_length = fbankConfig_.frameLength;
  opts.frame_shift = fbankConfig_.frameShift;
  opts.dither = fbankConfig_.dither;
  opts.energy_floor = fbankConfig_.energyFloor;
  opts.sample_frequency = fbankConfig_.sampleFrequency;
  opts.use_log_fbank = fbankConfig_.useLogFbank;
  opts.use_power = fbankConfig_.usePower;

  fbankComputer_ = std::make_unique<FbankComputer>(opts);

  AlgoBase encoderAlgoBase;
  encoderAlgoBase.name = "wenet_encoder";
  encoderAlgoBase.modelPath = wenetConfig_.modelPath;

  wenetEncoder_ = std::make_unique<dnn::WeNetEncoderInference>(encoderAlgoBase);
  return wenetEncoder_->initialize();
}

std::vector<std::vector<float>>
FeatureExtractor::computeFbank(const std::vector<float> &audio) {
  return fbankComputer_->Compute(audio);
}

std::vector<cv::Mat> FeatureExtractor::extractWenetFeatures(
    const std::vector<std::vector<float>> &fbankFeatures) {
  std::vector<cv::Mat> wenetFeatures;
  const int fbankFeatureLength = fbankFeatures.size();

  cv::Mat attCache = cv::Mat::zeros(3 * 8 * 16 * 128, 1, CV_32F);
  cv::Mat cnnCache = cv::Mat::zeros(3 * 1 * 512 * 14, 1, CV_32F);
  int offset = 100;

  int start = 0;
  int end = 0;

  while (end < fbankFeatureLength) {
    end = start + wenetConfig_.framesStride;

    cv::Mat chunkFeat;
    if (end <= fbankFeatureLength) {
      chunkFeat = cv::Mat(wenetConfig_.framesStride * wenetConfig_.numFeatures,
                          1, CV_32F);

      for (int i = start; i < end; i++) {
        if (i < fbankFeatureLength) {
          std::memcpy(chunkFeat.ptr<float>() +
                          (i - start) * wenetConfig_.numFeatures,
                      fbankFeatures[i].data(),
                      wenetConfig_.numFeatures * sizeof(float));
        } else {
          std::memset(chunkFeat.ptr<float>() +
                          (i - start) * wenetConfig_.numFeatures,
                      0, wenetConfig_.numFeatures * sizeof(float));
        }
      }
    } else {
      chunkFeat = cv::Mat(wenetConfig_.framesStride * wenetConfig_.numFeatures,
                          1, CV_32F, cv::Scalar(0));
      for (int i = start; i < fbankFeatureLength; i++) {
        std::memcpy(
            chunkFeat.ptr<float>() + (i - start) * wenetConfig_.numFeatures,
            fbankFeatures[i].data(), wenetConfig_.numFeatures * sizeof(float));
      }
    }

    WeNetEncoderInput encoderInput;
    encoderInput.chunk = chunkFeat;
    encoderInput.offset = offset;
    encoderInput.attCache = attCache;
    encoderInput.cnnCache = cnnCache;

    AlgoInput input;
    input.setParams(encoderInput);

    AlgoOutput output;
    WeNetEncoderOutput wenetEncoderOutput;
    output.setParams(wenetEncoderOutput);

    if (!wenetEncoder_->infer(input, output)) {
      throw std::runtime_error("Failed to process WeNet encoder");
    }

    auto *encoderOutput = output.getParams<WeNetEncoderOutput>();
    if (encoderOutput) {
      const float *srcData = encoderOutput->data.data();
      cv::Mat outputFeature(16, 512, CV_32F);

      for (int i = 0; i < 16; i++) {
        float *dstRow = outputFeature.ptr<float>(i);
        for (int j = 0; j < 512; j++) {
          dstRow[j] = srcData[i * 512 + j];
        }
      }
      wenetFeatures.push_back(outputFeature);
    }

    start += wenetConfig_.slidingStep;
  }

  return wenetFeatures;
}

cv::Mat FeatureExtractor::prepareChunkFeature(
    const std::vector<std::vector<float>> &fbankFeatures, int start, int end) {

  const int featureLength = fbankFeatures.size();
  cv::Mat chunkFeat(wenetConfig_.framesStride * wenetConfig_.numFeatures, 1,
                    CV_32F, cv::Scalar(0));

  for (int i = start; i < std::min(end, featureLength); i++) {
    std::memcpy(chunkFeat.ptr<float>() + (i - start) * wenetConfig_.numFeatures,
                fbankFeatures[i].data(),
                wenetConfig_.numFeatures * sizeof(float));
  }

  return chunkFeat;
}

cv::Mat FeatureExtractor::getSlicedFeature(const std::vector<cv::Mat> &feature,
                                           int frameIdx) {
  const int windowSize = 8;
  const int left = frameIdx - windowSize;
  const int right = frameIdx + windowSize;
  const int padLeft = std::max(0, -left);
  const int padRight = std::max(0, right - static_cast<int>(feature.size()));

  const int validLeft = std::max(0, left);
  const int validRight = std::min(static_cast<int>(feature.size()), right);

  const int rows = feature[0].rows;
  const int cols = feature[0].cols;
  cv::Mat result(rows * 16, cols, CV_32F);

  int currentRow = 0;

  for (int i = 0; i < padLeft; ++i) {
    cv::Mat zeroMat = cv::Mat::zeros(rows, cols, CV_32F);
    zeroMat.copyTo(result(cv::Rect(0, currentRow, cols, rows)));
    currentRow += rows;
  }

  for (int i = validLeft; i < validRight; ++i) {
    feature[i].copyTo(result(cv::Rect(0, currentRow, cols, rows)));
    currentRow += rows;
  }

  for (int i = 0; i < padRight; ++i) {
    cv::Mat zeroMat = cv::Mat::zeros(rows, cols, CV_32F);
    zeroMat.copyTo(result(cv::Rect(0, currentRow, cols, rows)));
    currentRow += rows;
  }

  return result;
}

std::vector<cv::Mat>
FeatureExtractor::convertToChunks(const std::vector<cv::Mat> &featureArray) {
  std::vector<cv::Mat> audioChunks;
  audioChunks.reserve(featureArray.size());

  for (int i = 0; i < featureArray.size(); ++i) {
    audioChunks.push_back(getSlicedFeature(featureArray, i));
  }

  return audioChunks;
}

} // namespace lip_sync::infer
