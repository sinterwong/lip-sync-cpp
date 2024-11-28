/**
 * @file audio_processor.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "audio_processor.hpp"
#include "logger/logger.hpp"
#include <algorithm>
#include <sndfile.h>

namespace lip_sync::audio {

AudioProcessor::AudioProcessor(const AudioConfig &config) : config_(config) {}

std::vector<float> AudioProcessor::readAudio(const std::string &filePath) {
  SF_INFO sfinfo{};
  SNDFILE *sndfile = sf_open(filePath.c_str(), SFM_READ, &sfinfo);

  if (!sndfile) {
    LOGGER_ERROR("Could not open audio file: {}", sf_strerror(sndfile));
    return {};
  }

  std::vector<float> audioData(sfinfo.frames);
  sf_readf_float(sndfile, audioData.data(), sfinfo.frames);
  sf_close(sndfile);

  return audioData;
}

std::vector<float> AudioProcessor::preprocess(const std::vector<float> &audio) {
  std::vector<float> paddedAudio;
  paddedAudio.resize(config_.padding30Frames + audio.size() +
                     config_.padding31Frames);

  // Fill beginning with zeros
  std::fill(paddedAudio.begin(), paddedAudio.begin() + config_.padding30Frames,
            0.0f);

  // Convert and copy audio data
  std::transform(
      audio.begin(), audio.end(), paddedAudio.begin() + config_.padding30Frames,
      [this](float x) {
        int16_t scaled = static_cast<int16_t>(x * config_.amplitudeScale);
        return static_cast<float>(scaled);
      });

  // Fill end with zeros
  std::fill(paddedAudio.begin() + config_.padding30Frames + audio.size(),
            paddedAudio.end(), 0.0f);

  return paddedAudio;
}
} // namespace lip_sync::audio