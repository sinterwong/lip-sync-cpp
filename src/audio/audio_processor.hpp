/**
 * @file audio_processor.hpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <string>
#include <vector>

namespace lip_sync::audio {
class AudioProcessor {
public:
  struct AudioConfig {
    int sampleRate;
    int padding30Frames;
    int padding31Frames;
    float amplitudeScale;

    AudioConfig()
        : sampleRate(16000), padding30Frames(32 * 160),
          padding31Frames(35 * 160), amplitudeScale(32767.0f) {}
  };

  explicit AudioProcessor(const AudioConfig &config = AudioConfig());
  ~AudioProcessor() = default;

  std::vector<float> readAudio(const std::string &filePath);
  std::vector<float> preprocess(const std::vector<float> &audio);

private:
  AudioConfig config_;
};
} // namespace lip_sync::audio