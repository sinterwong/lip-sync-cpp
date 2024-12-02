/**
 * @file face_processor.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __FACE_PROCESSOR_H__
#define __FACE_PROCESSOR_H__

#include "types.hpp"

namespace lip_sync::infer {
class FaceProcessor {
public:
  FaceProcessor(int inputSize = 160, int padSize = 4);

  ProcessedFaceData preProcess(const cv::Mat &frame, const cv::Rect &faceBbox);
  cv::Mat postProcess(const std::vector<float> &prediction,
                      const ProcessedFaceData &data, cv::Mat &frame);

  int getInputSize() { return inputSize_; }

private:
  int inputSize_;
  int padSize_;
  cv::Mat vectorToMat(const std::vector<float> &prediction);
};
} // namespace lip_sync::infer
#endif
