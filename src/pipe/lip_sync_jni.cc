#include "api/lip_sync_jni.h"
#include "lip_sync_sdk.h"
#include "logger/logger.hpp"
#include <android/log.h>
#include <vector>

#define LOG_TAG "LipSyncJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace lip_sync;

// 工具函数：Java String 转 std::string
static std::string jstring2string(JNIEnv *env, jstring jstr) {
  if (!jstr)
    return "";
  const char *str = env->GetStringUTFChars(jstr, nullptr);
  std::string result(str);
  env->ReleaseStringUTFChars(jstr, str);
  return result;
}

// 工具函数：std::string 转 Java String
static jstring string2jstring(JNIEnv *env, const std::string &str) {
  return env->NewStringUTF(str.c_str());
}

// 工具函数：std::vector<uint8_t> 转 byte[]
static jbyteArray vector_uint8_to_jbyteArray(JNIEnv *env,
                                             const std::vector<uint8_t> &data) {
  jbyteArray result = env->NewByteArray(data.size());
  if (result) {
    env->SetByteArrayRegion(result, 0, data.size(),
                            reinterpret_cast<const jbyte *>(data.data()));
  }
  return result;
}

// 工具函数：std::vector<float> 转 float[]
static jfloatArray vector_float_to_jfloatArray(JNIEnv *env,
                                               const std::vector<float> &data) {
  jfloatArray result = env->NewFloatArray(data.size());
  if (result) {
    env->SetFloatArrayRegion(result, 0, data.size(), data.data());
  }
  return result;
}

// 创建 SDK 实例
JNIEXPORT jlong JNICALL
Java_com_example_lipsync_LipSyncSDK_nativeCreate(JNIEnv *env, jclass clazz) {

  const auto initLogger = []() -> decltype(auto) {
    auto ret = mkdir("logs", 0777);
    if (ret == -1) {
      LOGE("Failed to create logs directory");
      return false;
    }
    LipSyncLoggerInit(true, true, true, true);
    return true;
  }();

  if (!initLogger) {
    LOGE("Failed to initialize logger");
    return 0;
  }

  try {
    auto *sdk = new LipSyncSDK();
    return reinterpret_cast<jlong>(sdk);
  } catch (const std::exception &e) {
    LOGE("Failed to create SDK instance: %s", e.what());
    return 0;
  }
}

// 初始化 SDK
JNIEXPORT jint JNICALL Java_com_example_lipsync_LipSyncSDK_nativeInitialize(
    JNIEnv *env, jclass clazz, jlong handle, jobject jconfig) {
  if (!handle)
    return static_cast<jint>(ErrorCode::INVALID_STATE);

  try {
    auto *sdk = reinterpret_cast<LipSyncSDK *>(handle);

    // 获取 SDKConfig 类的字段 ID
    jclass configClass = env->GetObjectClass(jconfig);
    jfieldID numWorkersField = env->GetFieldID(configClass, "numWorkers", "I");
    jfieldID wavLipModelPathField =
        env->GetFieldID(configClass, "wavLipModelPath", "Ljava/lang/String;");
    jfieldID encoderModelPathField =
        env->GetFieldID(configClass, "encoderModelPath", "Ljava/lang/String;");
    jfieldID frameDirField =
        env->GetFieldID(configClass, "frameDir", "Ljava/lang/String;");
    jfieldID frameRateField = env->GetFieldID(configClass, "frameRate", "I");
    jfieldID faceInfoPathField =
        env->GetFieldID(configClass, "faceInfoPath", "Ljava/lang/String;");
    jfieldID maxCacheSizeField =
        env->GetFieldID(configClass, "maxCacheSize", "J");
    jfieldID faceSizeField = env->GetFieldID(configClass, "faceSize", "I");
    jfieldID facePadField = env->GetFieldID(configClass, "facePad", "I");

    // 构建 C++ SDKConfig 对象
    SDKConfig config;
    config.numWorkers = env->GetIntField(jconfig, numWorkersField);
    config.wavLipModelPath = jstring2string(
        env, (jstring)env->GetObjectField(jconfig, wavLipModelPathField));
    config.encoderModelPath = jstring2string(
        env, (jstring)env->GetObjectField(jconfig, encoderModelPathField));
    config.frameDir = jstring2string(
        env, (jstring)env->GetObjectField(jconfig, frameDirField));
    config.frameRate = env->GetIntField(jconfig, frameRateField);
    config.faceInfoPath = jstring2string(
        env, (jstring)env->GetObjectField(jconfig, faceInfoPathField));
    config.maxCacheSize = env->GetLongField(jconfig, maxCacheSizeField);
    config.faceSize = env->GetIntField(jconfig, faceSizeField);
    config.facePad = env->GetIntField(jconfig, facePadField);

    return static_cast<jint>(sdk->initialize(config));
  } catch (const std::exception &e) {
    LOGE("Failed to initialize SDK: %s", e.what());
    return static_cast<jint>(ErrorCode::INITIALIZATION_FAILED);
  }
}

// 开始处理
JNIEXPORT jint JNICALL Java_com_example_lipsync_LipSyncSDK_nativeStartProcess(
    JNIEnv *env, jclass clazz, jlong handle, jobject input) {
  if (!handle)
    return static_cast<jint>(ErrorCode::INVALID_STATE);

  try {
    auto *sdk = reinterpret_cast<LipSyncSDK *>(handle);

    // 获取 InputPacket 类的字段 ID
    jclass inputClass = env->GetObjectClass(input);
    jfieldID audioDataField = env->GetFieldID(inputClass, "audioData", "[F");
    jfieldID audioPathField =
        env->GetFieldID(inputClass, "audioPath", "Ljava/lang/String;");
    jfieldID uuidField =
        env->GetFieldID(inputClass, "uuid", "Ljava/lang/String;");

    // 构建 C++ InputPacket 对象
    InputPacket inputPacket;
    jfloatArray audioData =
        (jfloatArray)env->GetObjectField(input, audioDataField);
    jsize audioDataSize = env->GetArrayLength(audioData);
    inputPacket.audioData.resize(audioDataSize);
    env->GetFloatArrayRegion(audioData, 0, audioDataSize,
                             inputPacket.audioData.data());
    inputPacket.audioPath = jstring2string(
        env, (jstring)env->GetObjectField(input, audioPathField));
    inputPacket.uuid =
        jstring2string(env, (jstring)env->GetObjectField(input, uuidField));

    return static_cast<jint>(sdk->startProcess(inputPacket));
  } catch (const std::exception &e) {
    LOGE("Failed to start process: %s", e.what());
    return static_cast<jint>(ErrorCode::PROCESSING_ERROR);
  }
}

// 终止处理
JNIEXPORT jint JNICALL Java_com_example_lipsync_LipSyncSDK_nativeTerminate(
    JNIEnv *env, jclass clazz, jlong handle) {
  if (!handle)
    return static_cast<jint>(ErrorCode::INVALID_STATE);

  try {
    auto *sdk = reinterpret_cast<LipSyncSDK *>(handle);
    return static_cast<jint>(sdk->terminate());
  } catch (const std::exception &e) {
    LOGE("Failed to terminate SDK: %s", e.what());
    return static_cast<jint>(ErrorCode::PROCESSING_ERROR);
  }
}

// 获取下一帧数据
JNIEXPORT jint JNICALL Java_com_example_lipsync_LipSyncSDK_nativeTryGetNext(
    JNIEnv *env, jclass clazz, jlong handle, jobject output) {
  if (!handle)
    return static_cast<jint>(ErrorCode::INVALID_STATE);

  try {
    auto *sdk = reinterpret_cast<LipSyncSDK *>(handle);

    // 准备 C++ OutputPacket
    OutputPacket outPacket;

    // 尝试获取下一帧
    ErrorCode result = sdk->tryGetNext(outPacket);

    if (result == ErrorCode::SUCCESS) {
      // 获取 OutputPacket 类的字段 ID
      jclass outputClass = env->GetObjectClass(output);
      jfieldID uuidField =
          env->GetFieldID(outputClass, "uuid", "Ljava/lang/String;");
      jfieldID frameDataField = env->GetFieldID(outputClass, "frameData", "[B");
      jfieldID widthField = env->GetFieldID(outputClass, "width", "I");
      jfieldID heightField = env->GetFieldID(outputClass, "height", "I");
      jfieldID audioDataField = env->GetFieldID(outputClass, "audioData", "[F");
      jfieldID sampleRateField =
          env->GetFieldID(outputClass, "sampleRate", "I");
      jfieldID channelsField = env->GetFieldID(outputClass, "channels", "I");
      jfieldID timestampField = env->GetFieldID(outputClass, "timestamp", "J");
      jfieldID sequenceField = env->GetFieldID(outputClass, "sequence", "J");

      // 设置 Java OutputPacket 的字段
      env->SetObjectField(output, uuidField,
                          string2jstring(env, outPacket.uuid));
      env->SetObjectField(output, frameDataField,
                          vector_uint8_to_jbyteArray(env, outPacket.frameData));
      env->SetIntField(output, widthField, outPacket.width);
      env->SetIntField(output, heightField, outPacket.height);
      env->SetObjectField(
          output, audioDataField,
          vector_float_to_jfloatArray(env, outPacket.audioData));
      env->SetIntField(output, sampleRateField, outPacket.sampleRate);
      env->SetIntField(output, channelsField, outPacket.channels);
      env->SetLongField(output, timestampField, outPacket.timestamp);
      env->SetLongField(output, sequenceField, outPacket.sequence);
    }

    return static_cast<jint>(result);
  } catch (const std::exception &e) {
    LOGE("Failed to get next frame: %s", e.what());
    return static_cast<jint>(ErrorCode::PROCESSING_ERROR);
  }
}

// 获取版本信息
JNIEXPORT jstring JNICALL Java_com_example_lipsync_LipSyncSDK_nativeGetVersion(
    JNIEnv *env, jclass clazz) {
  try {
    return string2jstring(env, LipSyncSDK::getVersion());
  } catch (const std::exception &e) {
    LOGE("Failed to get version: %s", e.what());
    return env->NewStringUTF("");
  }
}

// 销毁 SDK 实例
JNIEXPORT void JNICALL Java_com_example_lipsync_LipSyncSDK_nativeDestroy(
    JNIEnv *env, jclass clazz, jlong handle) {
  if (handle) {
    try {
      auto *sdk = reinterpret_cast<LipSyncSDK *>(handle);
      delete sdk;
    } catch (const std::exception &e) {
      LOGE("Failed to destroy SDK: %s", e.what());
    }
  }
}