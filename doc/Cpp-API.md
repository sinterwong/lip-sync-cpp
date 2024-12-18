# LipSync SDK C++ 接口文档

本接口文档基于 `lip_sync_sdk.hpp` 和 `lip_sync_types.h` 文件，描述了 LipSync SDK 的 C++ 接口使用方法。

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
| `timestamp`     | `int64_t`           | 时间戳（微秒）                       |
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

以下接口均在 `lip_sync` 命名空间下的 `LipSyncSDK` 类中定义。

### 3.1. 构造函数

**功能:** 创建 `LipSyncSDK` 对象。

```cpp
LipSyncSDK();
```

### 3.2. 析构函数

**功能:** 销毁 `LipSyncSDK` 对象，释放资源。

```cpp
~LipSyncSDK();
```

### 3.3. `initialize`

**功能:** 初始化 SDK。

```cpp
ErrorCode initialize(const SDKConfig &config);
```

**参数:**

-   `config`: `SDKConfig` 类型的引用，用于配置 SDK。

**返回值:**

-   `ErrorCode`: 返回 `ErrorCode` 枚举值，表示初始化结果。

### 3.4. `startProcess`

**功能:** 开始处理输入数据。

```cpp
ErrorCode startProcess(const InputPacket &input);
```

**参数:**

-   `input`: `InputPacket` 类型的引用，包含输入数据。

**返回值:**

-   `ErrorCode`: 返回 `ErrorCode` 枚举值，表示处理结果。

### 3.5. `terminate`

**功能:** 终止当前处理任务。

```cpp
ErrorCode terminate();
```

**返回值:**

-   `ErrorCode`: 返回 `ErrorCode` 枚举值，表示终止结果。

### 3.6. `tryGetNext`

**功能:** 尝试获取下一个处理结果。

```cpp
ErrorCode tryGetNext(OutputPacket &result);
```

**参数:**

-   `result`: `OutputPacket` 类型的引用，用于接收处理结果。

**返回值:**

-   `ErrorCode`: 返回 `ErrorCode` 枚举值，表示获取结果的状态。`ErrorCode.TRY_GET_NEXT_OVERTIME` 表示超时，说明当前还没有可用的结果，需要继续调用。

### 3.7. `getVersion`

**功能:** 获取 SDK 版本号。

```cpp
static std::string getVersion();
```

**返回值:**

-   `std::string`: SDK 版本号字符串。

## 4. 使用流程

1. **创建对象:** 使用 `LipSyncSDK` 的构造函数创建对象。
2. **初始化:**
    *   创建 `SDKConfig` 对象并配置参数。
    *   调用 `initialize` 方法初始化 SDK。
3. **数据处理:**
    *   创建 `InputPacket` 对象并填充数据。
    *   调用 `startProcess` 方法开始处理数据。
    *   循环调用 `tryGetNext` 方法获取处理结果，直到返回 `ErrorCode.TRY_GET_NEXT_OVERTIME` 或 标志为最后一帧。
    *   可以使用 `terminate` 方法中止处理。
4. **获取版本号:** 调用 `LipSyncSDK::getVersion()` 静态方法获取 SDK 版本号。
5. **销毁对象:** 当不再需要 SDK 时，`LipSyncSDK` 对象会被自动析构，释放资源。
