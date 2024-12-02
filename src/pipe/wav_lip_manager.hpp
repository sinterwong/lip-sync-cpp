/**
 * @file wav_lip_manager.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __WAV_LIP_MANAGER_HPP__
#define __WAV_LIP_MANAGER_HPP__

#include "core/types.hpp"
#include "core/wavlip.hpp"
#include <atomic>
#include <memory>

namespace lip_sync {
class ModelInstance {
public:
  ModelInstance(const infer::AlgoBase &config)
      : model(std::make_unique<infer::dnn::WavToLipInference>(config)),
        busy(false) {}

  bool initialize() { return model->initialize(); }

  bool tryAcquire() {
    bool expected = false;
    return busy.compare_exchange_strong(expected, true);
  }

  void release() { busy.store(false); }

  infer::dnn::WavToLipInference *get() { return model.get(); }

private:
  std::unique_ptr<infer::dnn::WavToLipInference> model;
  std::atomic<bool> busy;
};

} // namespace lip_sync

#endif