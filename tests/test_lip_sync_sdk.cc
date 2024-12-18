#include "audio/audio_processor.hpp"
#include "lip_sync/lip_sync_sdk.hpp"
#include "logger/logger.hpp"
#include "sndfile.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <sstream>

const auto initLogger = []() -> decltype(auto) {
  LipSyncLoggerInit(true, true, true, true);
  return true;
}();

class VideoWriter {
public:
  static void saveVideoWithAudio(const std::vector<cv::Mat> &frames,
                                 const std::vector<float> &audioData,
                                 const std::string &outputPath,
                                 int frameRate = 24, int sampleRate = 16000) {
    if (frames.empty()) {
      throw std::runtime_error("No frames to write");
    }

    // 创建临时目录
    std::string tempDir = "temp_frames_" + std::to_string(std::time(nullptr));
    std::filesystem::create_directories(tempDir);

    // 保存音频文件
    std::string audioPath = tempDir + "/audio.raw";
    FILE *audioFile = fopen(audioPath.c_str(), "wb");
    if (audioFile) {
      fwrite(audioData.data(), sizeof(float), audioData.size(), audioFile);
      fclose(audioFile);
    }

    // 保存帧
    for (size_t i = 0; i < frames.size(); ++i) {
      std::stringstream ss;
      ss << tempDir << "/frame_" << std::setw(6) << std::setfill('0') << i
         << ".png";
      cv::imwrite(ss.str(), frames[i]);
      std::cout << "Saved frame " << i + 1 << "/" << frames.size() << std::endl;
    }

    // 使用 mjpeg 编码器的 FFmpeg 命令，它支持高分辨率
    std::string ffmpeg_cmd =
        "ffmpeg -y "
        "-framerate " +
        std::to_string(frameRate) +
        " "
        "-i " +
        tempDir +
        "/frame_%06d.png "
        "-f f32le -ar " +
        std::to_string(sampleRate) +
        " -ac 1 "
        "-i " +
        audioPath +
        " "
        "-vcodec mjpeg "           // 使用 MJPEG 编码器
        "-qscale:v 2 "             // 质量设置 (1-31，1 最好)
        "-vf \"format=yuvj420p\" " // 使用完整范围的 YUV 颜色空间
        "-c:a aac "
        "-b:a 128k "
        "-shortest "
        "\"" +
        outputPath + "\"";

    std::cout << "Running FFmpeg command: " << ffmpeg_cmd << std::endl;

    int ret = system(ffmpeg_cmd.c_str());
    if (ret != 0) {
      // 如果 MJPEG 失败，尝试使用 VP8 编码器
      ffmpeg_cmd = "ffmpeg -y "
                   "-framerate " +
                   std::to_string(frameRate) +
                   " "
                   "-i " +
                   tempDir +
                   "/frame_%06d.png "
                   "-f f32le -ar " +
                   std::to_string(sampleRate) +
                   " -ac 1 "
                   "-i " +
                   audioPath +
                   " "
                   "-c:v vp8 " // 使用 VP8 编码器
                   "-b:v 2M "
                   "-vf \"format=yuv420p\" "
                   "-c:a aac "
                   "-b:a 128k "
                   "-shortest "
                   "\"" +
                   outputPath + "\"";

      std::cout << "Retrying with VP8 encoder..." << std::endl;
      std::cout << "Running FFmpeg command: " << ffmpeg_cmd << std::endl;

      ret = system(ffmpeg_cmd.c_str());
      if (ret != 0) {
        std::cerr << "FFmpeg command failed with return code: " << ret
                  << std::endl;
      }
    }

    // // 清理临时文件
    // try {
    //   std::filesystem::remove_all(tempDir);
    // } catch (const std::exception &e) {
    //   std::cerr << "Error cleaning up temporary files: " << e.what()
    //             << std::endl;
    // }
  }
};

std::string getTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
  return ss.str();
}

void saveFrame(const std::vector<uint8_t> &frameData,
               const std::string &outputDir, const std::string &uuid,
               int64_t sequence) {
  std::filesystem::create_directories(outputDir);
  std::string filename =
      outputDir + "/" + uuid + "_" + std::to_string(sequence) + ".png";

  cv::Mat frame = cv::imdecode(frameData, cv::IMREAD_COLOR);
  if (!frame.empty()) {
    cv::imwrite(filename, frame);
    std::cout << "Saved frame to: " << filename << std::endl;
  } else {
    std::cerr << "Failed to decode frame data for sequence: " << sequence
              << std::endl;
  }
}

bool writeAudioFile(const std::vector<float> &audioData,
                    const std::string &filename, int sampleRate = 16000,
                    int channels = 1) {
  SF_INFO sfInfo;
  sfInfo.channels = channels;
  sfInfo.samplerate = sampleRate;
  sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

  SNDFILE *outfile = sf_open(filename.c_str(), SFM_WRITE, &sfInfo);
  if (!outfile) {
    std::cerr << "无法创建音频文件: " << sf_strerror(NULL) << std::endl;
    return false;
  }

  sf_count_t count =
      sf_write_float(outfile, audioData.data(), audioData.size());

  if (count != audioData.size()) {
    std::cerr << "写入音频数据时发生错误" << std::endl;
    sf_close(outfile);
    return false;
  }

  sf_close(outfile);
  return true;
}

int main(int argc, char **argv) {

  LipSyncLoggerSetLevel(2);

  std::cout << "Starting LipSync SDK test at: " << getTimestamp() << std::endl;

  lip_sync::LipSyncSDK sdk;
  lip_sync::SDKConfig config;
  config.numWorkers = 1;
  config.frameDir = "data/frames";
  config.faceInfoPath = "data/face_bboxes.json";
  config.encoderModelPath = "models/wenet_encoder.onnx";
  config.wavLipModelPath = "models/w2l_with_wenet.onnx";
  config.faceSize = 160;
  config.facePad = 4;
  config.maxCacheSize = 1024 * 1024 * 100;
  config.frameRate = 20;

  std::cout << "\nInitializing SDK with config:" << "\n - Workers: "
            << config.numWorkers << "\n - Face size: " << config.faceSize
            << "\n - Face pad: " << config.facePad
            << "\n - Cache size: " << config.maxCacheSize << " bytes"
            << std::endl;

  auto ret = sdk.initialize(config);
  if (ret != lip_sync::ErrorCode::SUCCESS) {
    std::cerr << "Failed to initialize SDK" << std::endl;
    return 1;
  }
  std::cout << "SDK initialized successfully" << std::endl;

  lip_sync::InputPacket input;
  auto inputAudioPath = "data/test.wav";
  lip_sync::audio::AudioProcessor audioProcessor;
  input.audioData = audioProcessor.readAudio(inputAudioPath);
  input.uuid = "test_" + getTimestamp();

  std::cout << "\nStarting process with:" << "\n - Audio: " << input.audioPath
            << "\n - UUID: " << input.uuid << std::endl;

  ret = sdk.startProcess(input);
  if (ret != lip_sync::ErrorCode::SUCCESS) {
    std::cerr << "Failed to start processing" << std::endl;
    return 1;
  }

  std::string outputDir = "output";
  int successCount = 0;
  int failureCount = 0;

  std::vector<cv::Mat> frames;   // 存储所有帧
  std::vector<float> audioDatas; // 存储音频数据

  lip_sync::OutputPacket output;
  for (int i = 0; i < 200; ++i) {
    ret = sdk.tryGetNext(output);
    if (ret == lip_sync::ErrorCode::SUCCESS) {
      cv::Mat frame = cv::imdecode(output.frameData, cv::IMREAD_COLOR);
      if (!frame.empty()) {
        frames.push_back(frame);
      }

      audioDatas.insert(audioDatas.end(), output.audioData.begin(),
                        output.audioData.end());
    }
  }
  std::string outputPath = outputDir + "/output.mp4";
  try {
    VideoWriter::saveVideoWithAudio(frames, audioDatas, outputPath);
    std::cout << "Successfully saved video to: " << outputPath << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error saving video: " << e.what() << std::endl;
  }

  // save audio
  std::string audioPath = outputDir + "/output.wav";
  if (!writeAudioFile(audioDatas, audioPath)) {
    std::cerr << "Failed to save audio to: " << audioPath << std::endl;
  } else {
    std::cout << "Successfully saved audio to: " << audioPath << std::endl;
  }

  std::cout << "\nTest summary:" << "\n - Successful packets: " << successCount
            << "\n - Failed attempts: " << failureCount
            << "\n - Output directory: " << outputDir << std::endl;

  sdk.terminate();
  std::cout << "Test completed at: " << getTimestamp() << std::endl;

  return 0;
}