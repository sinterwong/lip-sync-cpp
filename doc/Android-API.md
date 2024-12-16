# LipSync SDK Android 接口文档

本接口文档基于 `lip_sync_jni.h`，`lip_sync_sdk.h` 和 `types.h` 文件，讲述 LipSync SDK 在 Android 平台上的 JNI 接口使用方法。

## 1. 概述

LipSync SDK 是一个提供唇音同步功能的 SDK，可以将输入的音频数据或音频文件，与视频帧进行同步处理，生成唇形匹配的视频帧数据。

## 2. 数据结构

### 2.1. SDKConfig

`SDKConfig` 结构体用于配置 LipSync SDK 的初始化参数。

| 字段                | 类型          | 说明                                       |
| ------------------- | ------------- | ------------------------------------------ |
| `numWorkers`        | `uint32_t`    | 工作线程数量，默认为 1                      |
| `wavLipModelPath`   | `std::string` | 唇音同步模型路径                           |
| `encoderModelPath`  | `std::string` | 音频编码模型路径                           |
| `frameDir`          | `std::string` | 帧路径                                     |
| `frameRate`         | `uint32_t`    | 帧率                                       |
| `faceInfoPath`      | `std::string` | 人脸信息路径                               |
| `maxCacheSize`      | `size_t`      | 图片最大缓存大小（字节）                   |
| `faceSize`          | `uint32_t`    | 人脸图片尺寸                               |
| `facePad`           | `uint32_t`    | 人脸图片填充                               |

### 2.2. InputPacket

`InputPacket` 结构体用于传递输入数据到 SDK。

| 字段          | 类型                | 说明             |
| ------------- | ------------------- | ---------------- |
| `audioData`   | `std::vector<float>` | 音频数据         |
| `audioPath`   | `std::string`       | 音频文件路径     |
| `uuid`        | `std::string`       | 数据唯一标识符   |

### 2.3. OutputPacket

`OutputPacket` 结构体用于接收 SDK 的输出数据。

| 字段            | 类型                | 说明                                 |
| --------------- | ------------------- | ------------------------------------ |
| `uuid`          | `std::string`       | 数据唯一标识符                       |
| `frameData`     | `std::vector<uint8_t>` | 生成的视频帧数据                     |
| `width`         | `uint32_t`          | 帧宽度                               |
| `height`        | `uint32_t`          | 帧高度                               |
| `audioData`     | `std::vector<float>` | 对应的音频段数据                     |
| `sampleRate`    | `uint32_t`          | 音频采样率                           |
| `channels`      | `uint32_t`          | 音频通道数                           |
| `timestamp`     | `int64_t`           | 时间戳                       |
| `sequence`      | `int64_t`           | 序列号                               |
| `isLastChunk`   | `bool`              | 是否是最后一帧                       |

### 2.4. ErrorCode

`ErrorCode` 枚举类型定义了 SDK 可能返回的错误码。

| 枚举值                  | 值    | 说明                     |
| ----------------------- | ----- | ------------------------ |
| `SUCCESS`               | 0     | 成功                     |
| `INVALID_INPUT`         | -1    | 无效输入                 |
| `FILE_NOT_FOUND`        | -2    | 文件未找到               |
| `INVALID_FILE_FORMAT`   | -3    | 无效文件格式             |
| `INITIALIZATION_FAILED` | -4    | 初始化失败               |
| `PROCESSING_ERROR`      | -5    | 处理错误                 |
| `INVALID_STATE`         | -6    | 无效状态                 |
| `TRY_GET_NEXT_OVERTIME` | -7    | `tryGetNext`获取结果超时 |

## 3. 接口说明

以下接口均在 Java 类 `com.example.lipsync.LipSyncSDK` 中定义。

### 3.1. `nativeCreate`

**功能:** 创建 LipSync SDK 实例。

**Java 方法:**

```java
public static native long nativeCreate();
```

**参数:**

无

**返回值:**

- `long`: LipSync SDK 实例的句柄，后续接口调用需要使用此句柄。

### 3.2. `nativeInitialize`

**功能:** 初始化 LipSync SDK 实例。

**Java 方法:**

```java
public static native int nativeInitialize(long handle, Object config);
```

**参数:**

- `handle`: `long` 类型，`nativeCreate` 返回的实例句柄。
- `config`: `Object` 类型，对应 `SDKConfig` 结构体，需要使用 Java 对象进行封装。

**返回值:**

- `int`: 返回 `ErrorCode` 枚举值，0 表示成功，非 0 表示失败。

### 3.3. `nativeStartProcess`

**功能:** 开始处理输入数据。

**Java 方法:**

```java
public static native int nativeStartProcess(long handle, Object input);
```

**参数:**

- `handle`: `long` 类型，`nativeCreate` 返回的实例句柄。
- `input`: `Object` 类型，对应 `InputPacket` 结构体，需要使用 Java 对象进行封装。

**返回值:**

- `int`: 返回 `ErrorCode` 枚举值，0 表示成功，非 0 表示失败。

### 3.4. `nativeTerminate`

**功能:** 终止当前处理任务。

**Java 方法:**

```java
public static native int nativeTerminate(long handle);
```

**参数:**

- `handle`: `long` 类型，`nativeCreate` 返回的实例句柄。

**返回值:**

- `int`: 返回 `ErrorCode` 枚举值，0 表示成功，非 0 表示失败。

### 3.5. `nativeTryGetNext`

**功能:** 尝试获取下一个处理结果。

**Java 方法:**

```java
public static native int nativeTryGetNext(long handle, Object output);
```

**参数:**

- `handle`: `long` 类型，`nativeCreate` 返回的实例句柄。
- `output`: `Object` 类型，对应 `OutputPacket` 结构体，需要使用 Java 对象进行封装，用于接收处理结果。

**返回值:**

- `int`: 返回 `ErrorCode` 枚举值，0 表示成功，非 0 表示失败。`ErrorCode.TRY_GET_NEXT_OVERTIME` 表示超时。

### 3.6. `nativeGetVersion`

**功能:** 获取 SDK 版本号。

**Java 方法:**

```java
public static native String nativeGetVersion();
```

**参数:**

无

**返回值:**

- `String`: SDK 版本号字符串。

### 3.7. `nativeDestroy`

**功能:** 销毁 LipSync SDK 实例，释放资源。

**Java 方法:**

```java
public static native void nativeDestroy(long handle);
```

**参数:**

- `handle`: `long` 类型，`nativeCreate` 返回的实例句柄。

**返回值:**

无

## 4. 使用流程

1. **创建实例:** 调用 `nativeCreate` 创建 LipSync SDK 实例，获取实例句柄。
2. **初始化:** 使用 `SDKConfig` 对象配置 SDK 参数，调用 `nativeInitialize` 初始化 SDK 实例。
3. **数据处理:**
    *   使用 `InputPacket` 对象封装输入数据，调用 `nativeStartProcess` 开始处理。
    *   循环调用 `nativeTryGetNext` 获取处理结果，直到返回 `ErrorCode.TRY_GET_NEXT_OVERTIME` 或 标志为最后一帧。
    *   可以使用 `nativeTerminate` 中止处理。
4. **获取版本号:** 调用 `nativeGetVersion` 获取 SDK 版本号。
5. **销毁实例:** 调用 `nativeDestroy` 销毁 SDK 实例，释放资源。
