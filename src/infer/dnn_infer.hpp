/**
 * @file dnn_infer.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __ONNXRUNTIME_INFERENCE_H_
#define __ONNXRUNTIME_INFERENCE_H_

#include "types.hpp"
#include <onnxruntime_cxx_api.h>

namespace lip_sync::infer::dnn {
class AlgoInference {
public:
  AlgoInference(const AlgoBase &_param) : mParams(_param) {}

  virtual ~AlgoInference() { terminate(); }

  /**
   * @brief initialize the network
   *
   * @return true
   * @return false
   */
  virtual bool initialize();

  /**
   * @brief Runs the inference engine with input of void*
   *
   * @return true
   * @return false
   */
  virtual bool infer(AlgoInput &input, AlgoOutput &output) = 0;

  /**
   * @brief Get the Model Info object
   *
   */
  virtual void getModelInfo(ModelInfo &);

  virtual void terminate();

protected:
  AlgoBase mParams;

  std::vector<Ort::Value> inputTensors;
  std::vector<Ort::Value> outputTensors;

  std::vector<std::string> inputNames;
  std::vector<std::vector<int64_t>> inputShape;

  std::vector<std::string> outputNames;
  std::vector<std::vector<int64_t>> outputShapes;

  // infer engine
  std::unique_ptr<Ort::Env> env;
  std::unique_ptr<Ort::Session> session;
  std::unique_ptr<Ort::MemoryInfo> memoryInfo;
};
} // namespace lip_sync::infer::dnn
#endif
