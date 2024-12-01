/**
 * @file face_processor.cpp
 * @author Sinter Wong (sintercver@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "face_processor.hpp"

namespace lip_sync::infer {
FaceProcessor::FaceProcessor(int inputSize, int padSize)
    : inputSize_(inputSize), padSize_(padSize) {}

ProcessedFaceData FaceProcessor::preProcess(const cv::Mat &frame,
                                            const cv::Rect &faceBbox) {
  ProcessedFaceData result;
  result.boundingBox = faceBbox;

  // Face cropping
  cv::Mat faceCropLarge = frame(faceBbox).clone();

  // Resize with padding
  cv::resize(faceCropLarge, result.faceCropLarge,
             cv::Size(inputSize_ + padSize_ * 2, inputSize_ + padSize_ * 2), 0,
             0, cv::INTER_LINEAR);

  // Extract center crop
  cv::Rect centerRoi(padSize_, padSize_, inputSize_, inputSize_);
  cv::Mat faceCrop = result.faceCropLarge(centerRoi).clone();

  // Create face mask
  cv::Mat faceMask = faceCrop.clone();
  cv::rectangle(faceMask,
                cv::Rect(padSize_ + 1, padSize_ + 1,
                         inputSize_ - padSize_ * 2 - 2,
                         inputSize_ - padSize_ * 3 - 3),
                cv::Scalar(0, 0, 0), -1);

  // Convert to float and normalize
  faceCrop.convertTo(faceCrop, CV_32F, 1.0 / 255.0);
  faceMask.convertTo(faceMask, CV_32F, 1.0 / 255.0);

  // Split channels for both faceCrop and faceMask
  std::vector<cv::Mat> cropChannels, maskChannels;
  cv::split(faceCrop, cropChannels);
  cv::split(faceMask, maskChannels);

  // Create the final 4D tensor with shape (1, 6, inputSize_, inputSize_)
  std::vector<cv::Mat> allChannels;
  // First add all channels from faceCrop
  for (const auto &channel : cropChannels) {
    allChannels.push_back(channel);
  }
  // Then add all channels from faceMask
  for (const auto &channel : maskChannels) {
    allChannels.push_back(channel);
  }

  // Create a single matrix that contains all channels
  // First, ensure all matrices are continuous and reshape them
  std::vector<cv::Mat> reshapedChannels;
  for (auto &channel : allChannels) {
    cv::Mat continuous = cv::Mat(channel.rows * channel.cols, 1, CV_32F);
    channel.reshape(1, channel.rows * channel.cols).copyTo(continuous);
    reshapedChannels.push_back(continuous);
  }

  // Create the final 4D tensor with shape (1, 6, inputSize_, inputSize_)
  int dims[] = {1, 6, inputSize_, inputSize_};
  result.xData = cv::Mat(4, dims, CV_32F);

  // Fill the tensor with data
  float *dataPtr = (float *)result.xData.data;
  for (size_t c = 0; c < reshapedChannels.size(); ++c) {
    memcpy(dataPtr + c * inputSize_ * inputSize_, reshapedChannels[c].data,
           inputSize_ * inputSize_ * sizeof(float));
  }

  return result;
}

cv::Mat FaceProcessor::vectorToMat(const std::vector<float> &prediction) {
  // 预期输入维度为 (1, 3, 160, 160)，总元素数应该是 3*160*160
  CV_Assert(prediction.size() == 3 * inputSize_ * inputSize_);

  // 创建目标Mat，格式为(height, width, channels)
  cv::Mat result(inputSize_, inputSize_, CV_8UC3);

  // 计算单个通道的大小
  size_t channelSize = inputSize_ * inputSize_;

  // 遍历并重新排列数据
  for (int h = 0; h < inputSize_; h++) {
    for (int w = 0; w < inputSize_; w++) {
      for (int c = 0; c < 3; c++) {
        // 从prediction中读取数据，注意通道顺序
        float value = prediction[c * channelSize + h * inputSize_ + w];
        // 转换到0-255范围并存储
        result.at<cv::Vec3b>(h, w)[c] =
            cv::saturate_cast<uchar>(value * 255.0f);
      }
    }
  }

  return result;
}

cv::Mat FaceProcessor::postProcess(const std::vector<float> &prediction,
                                   const ProcessedFaceData &data,
                                   cv::Mat &frame) {
  cv::Mat outputFrame = frame.clone();

  // 1. 将vector转换为Mat并调整格式
  cv::Mat predictionMat = vectorToMat(prediction);

  // 2. 将结果填充到faceCropLarge中
  cv::Mat faceCropLarge = data.faceCropLarge.clone();
  cv::Rect centerRoi(padSize_, padSize_, inputSize_, inputSize_);
  predictionMat.copyTo(faceCropLarge(centerRoi));

  // 3. 将结果缩放回原始人脸框大小
  cv::Mat resizedFace;
  cv::resize(faceCropLarge, resizedFace,
             cv::Size(data.boundingBox.width, data.boundingBox.height), 0, 0,
             cv::INTER_LINEAR);

  // 4. 将处理后的人脸放回原始帧
  resizedFace.copyTo(outputFrame(data.boundingBox));

  return outputFrame;
}
} // namespace lip_sync::infer
