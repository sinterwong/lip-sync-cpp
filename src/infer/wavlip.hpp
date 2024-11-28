/**
 * @file wavlip.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __ONNXRUNTIME_INFERENCE_WAV_TO_LIP_H_
#define __ONNXRUNTIME_INFERENCE_WAV_TO_LIP_H_

#include "dnn_infer.hpp"

namespace lip_sync::infer::dnn {
class WavToLipInference : public AlgoInference {
public:
  explicit WavToLipInference(const AlgoBase &param) : AlgoInference(param) {}

  bool infer(AlgoInput &input, AlgoOutput &output) override;
};
} // namespace lip_sync::infer::dnn
#endif
