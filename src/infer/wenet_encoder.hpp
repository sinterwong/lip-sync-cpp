/**
 * @file wenet_encoder.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-22
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __ONNXRUNTIME_INFERENCE_WENET_ENCODER_H_
#define __ONNXRUNTIME_INFERENCE_WENET_ENCODER_H_

#include "dnn_infer.hpp"

namespace lip_sync::infer::dnn {
class WeNetEncoderInference : public AlgoInference {
public:
  explicit WeNetEncoderInference(const AlgoBase &param)
      : AlgoInference(param) {}

  bool infer(AlgoInput &input, AlgoOutput &output) override;
};
} // namespace lip_sync::infer::dnn
#endif
