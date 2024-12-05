#ifndef __LIP_SYNC_JNI_H__
#define __LIP_SYNC_JNI_H__

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

// JNI方法声明
JNIEXPORT jlong JNICALL
Java_com_example_lipsync_LipSyncSDK_nativeCreate(JNIEnv *env, jclass clazz);

JNIEXPORT jint JNICALL Java_com_example_lipsync_LipSyncSDK_nativeInitialize(
    JNIEnv *env, jclass clazz, jlong handle, jobject config);

JNIEXPORT jint JNICALL Java_com_example_lipsync_LipSyncSDK_nativeStartProcess(
    JNIEnv *env, jclass clazz, jlong handle, jobject input);

JNIEXPORT jint JNICALL Java_com_example_lipsync_LipSyncSDK_nativeTerminate(
    JNIEnv *env, jclass clazz, jlong handle);

JNIEXPORT jint JNICALL Java_com_example_lipsync_LipSyncSDK_nativeTryGetNext(
    JNIEnv *env, jclass clazz, jlong handle, jobject output);

JNIEXPORT jstring JNICALL
Java_com_example_lipsync_LipSyncSDK_nativeGetVersion(JNIEnv *env, jclass clazz);

JNIEXPORT void JNICALL Java_com_example_lipsync_LipSyncSDK_nativeDestroy(
    JNIEnv *env, jclass clazz, jlong handle);

#ifdef __cplusplus
}
#endif

#endif