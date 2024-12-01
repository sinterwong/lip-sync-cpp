/**
 * @file feature_extractor.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __FEATURE_EXTRACTOR_H__
#define __FEATURE_EXTRACTOR_H__

#include "fbank.hpp"
#include "wenet_encoder.hpp"
#include <memory>
#include <opencv2/core.hpp>
#include <vector>

namespace lip_sync::infer {

class FeatureExtractor {
public:
  explicit FeatureExtractor(const FbankConfig &fbankConfig = FbankConfig{},
                            const WeNetConfig &wenetConfig = WeNetConfig{});
  bool initialize();
  std::vector<std::vector<float>> computeFbank(const std::vector<float> &audio);
  std::vector<cv::Mat>
  extractWenetFeatures(const std::vector<std::vector<float>> &fbankFeatures);
  std::vector<cv::Mat>
  convertToChunks(const std::vector<cv::Mat> &featureArray);

private:
  cv::Mat getSlicedFeature(const std::vector<cv::Mat> &feature, int frameIdx);
  cv::Mat
  prepareChunkFeature(const std::vector<std::vector<float>> &fbankFeatures,
                      int start, int end);

  FbankConfig fbankConfig_;
  WeNetConfig wenetConfig_;
  std::unique_ptr<FbankComputer> fbankComputer_;
  std::unique_ptr<dnn::WeNetEncoderInference> wenetEncoder_;
};
} // namespace lip_sync::infer

#endif
