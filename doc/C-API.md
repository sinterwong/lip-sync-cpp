# LipSync SDK C 语言接口文档

本接口文档基于 `lip_sync_sdk.h` 和 `lip_sync_types.h` 文件，描述了 LipSync SDK 的 C 语言接口使用方法。

## 1. 概述

LipSync SDK 是一个提供唇音同步功能的 SDK，可以将输入的音频数据或音频文件，与视频帧进行同步处理，生成唇形匹配的视频帧数据。本接口为 C 语言封装版本，底层实现依赖于 C++ 版本。

## 2. 数据结构

### 2.1. SDKConfig

`SDKConfig` 结构体用于配置 LipSync SDK 的初始化参数。

| 字段                | 类型          | 说明                                       |
| ------------------- | ------------- | ------------------------------------------ |
| `numWorkers`        | `uint32_t`    | 工作线程数量，默认为 1                      |
| `wavLipModelPath`   | `char*`       | 唇音同步模型路径 (需要手动释放)          |
| `encoderModelPath`  | `char*`       | 音频编码模型路径 (需要手动释放)          |
| `frameDir`          | `char*`       | 帧路径 (需要手动释放)                    |
| `frameRate`         | `uint32_t`    | 帧率                                       |
| `faceInfoPath`      | `char*`       | 人脸信息路径 (需要手动释放)              |
| `maxCacheSize`      | `size_t`      | 图片最大缓存大小（字节）                   |
| `faceSize`          | `uint32_t`    | 人脸图片尺寸                               |
| `facePad`           | `uint32_t`    | 人脸图片填充                               |

**注意：** `char*` 类型的字段，在使用完后需要用户**手动释放内存**。

### 2.2. InputPacket

`InputPacket` 结构体用于传递输入数据到 SDK。

| 字段          | 类型                | 说明                                       |
| ------------- | ------------------- | ------------------------------------------ |
| `audioData`   | `float*`          | 音频数据 (使用完后需要手动释放)              |
| `audioDataSize` | `size_t`          | 音频数据大小                               |
| `audioPath`   | `char*`           | 音频文件路径 (使用完后需要手动释放)          |
| `uuid`        | `char*`           | 数据唯一标识符 (使用完后需要手动释放)      |

**注意：** `float*` 和 `char*` 类型的字段，在使用完后需要用户**手动释放内存**。

### 2.3. OutputPacket

`OutputPacket` 结构体用于接收 SDK 的输出数据。

| 字段            | 类型                | 说明                                 |
| --------------- | ------------------- | ------------------------------------ |
| `uuid`          | `char*`           | 数据唯一标识符 (使用完后需要手动释放)      |
| `frameData`     | `uint8_t*`        | 生成的视频帧数据 (使用完后需要手动释放)      |
| `frameDataSize` | `size_t`          | 视频帧数据大小                           |
| `width`         | `uint32_t`          | 帧宽度                               |
| `height`        | `uint32_t`          | 帧高度                               |
| `audioData`     | `float*`          | 对应的音频段数据 (使用完后需要手动释放)      |
| `audioDataSize` | `size_t`          | 音频数据大小                           |
| `sampleRate`    | `uint32_t`          | 音频采样率                           |
| `channels`      | `uint32_t`          | 音频通道数                           |
| `timestamp`     | `int64_t`           | 时间戳（微秒）                       |
| `sequence`      | `int64_t`           | 序列号                               |
| `isLastChunk`   | `bool`              | 是否是最后一帧                       |

**注意：** `char*`、`uint8_t*` 和 `float*` 类型的字段，在使用完后需要用户**手动释放内存**。

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

### 3.1. `LipSyncSDK_Create`

**功能:** 创建 LipSync SDK 实例。

```c
LipSyncSDKHandle LipSyncSDK_Create();
```

**返回值:**

-   `LipSyncSDKHandle`: LipSync SDK 实例句柄，如果创建失败，返回 `NULL`。

### 3.2. `LipSyncSDK_Destroy`

**功能:** 销毁 LipSync SDK 实例，释放资源。

```c
void LipSyncSDK_Destroy(LipSyncSDKHandle handle);
```

**参数:**

-   `handle`: LipSync SDK 实例句柄。

### 3.3. `LipSyncSDK_Initialize`

**功能:** 初始化 SDK。

```c
lip_sync::ErrorCode LipSyncSDK_Initialize(LipSyncSDKHandle handle, const lip_sync::SDKConfig *config);
```

**参数:**

-   `handle`: LipSync SDK 实例句柄。
-   `config`: 指向 `SDKConfig` 结构体的指针，用于配置 SDK。

**返回值:**

-   `lip_sync::ErrorCode`: 返回 `ErrorCode` 枚举值，表示初始化结果。

### 3.4. `LipSyncSDK_StartProcess`

**功能:** 开始处理输入数据。

```c
lip_sync::ErrorCode LipSyncSDK_StartProcess(LipSyncSDKHandle handle, const lip_sync::InputPacket *input);
```

**参数:**

-   `handle`: LipSync SDK 实例句柄。
-   `input`: 指向 `InputPacket` 结构体的指针，包含输入数据。

**返回值:**

-   `lip_sync::ErrorCode`: 返回 `ErrorCode` 枚举值，表示处理结果。

### 3.5. `LipSyncSDK_Terminate`

**功能:** 终止当前处理任务。

```c
lip_sync::ErrorCode LipSyncSDK_Terminate(LipSyncSDKHandle handle);
```

**参数:**

-   `handle`: LipSync SDK 实例句柄。

**返回值:**

-   `lip_sync::ErrorCode`: 返回 `ErrorCode` 枚举值，表示终止结果。

### 3.6. `LipSyncSDK_TryGetNext`

**功能:** 尝试获取下一个处理结果。

```c
lip_sync::ErrorCode LipSyncSDK_TryGetNext(LipSyncSDKHandle handle, lip_sync::OutputPacket *result);
```

**参数:**

-   `handle`: LipSync SDK 实例句柄。
-   `result`: 指向 `OutputPacket` 结构体的指针，用于接收处理结果。

**返回值:**

-   `lip_sync::ErrorCode`: 返回 `ErrorCode` 枚举值，表示获取结果的状态。`ErrorCode.TRY_GET_NEXT_OVERTIME` 表示超时，说明当前还没有可用的结果，需要继续调用。

### 3.7. `LipSyncSDK_GetVersion`

**功能:** 获取 SDK 版本号。

```c
const char *LipSyncSDK_GetVersion();
```

**返回值:**

-   `const char *`: SDK 版本号字符串，**需要用户手动释放内存**。

### 3.8. `LipSyncSDK_GetVersion_Callback`

**功能:** 获取 SDK 版本号 (回调函数方式)。

```c
void LipSyncSDK_GetVersion_Callback(void (*callback)(const char *));
```

**参数:**

-   `callback`: 回调函数指针，接收版本号字符串作为参数，该字符串内存由SDK内部管理，**不需要用户释放**。

## 4. 使用流程

1. **创建实例:** 使用 `LipSyncSDK_Create` 函数创建 SDK 实例。
2. **初始化:**
    *   创建 `SDKConfig` 结构体并配置参数，对于字符串类型参数，需要使用 `malloc` 等函数分配内存并复制字符串内容。
    *   调用 `LipSyncSDK_Initialize` 函数初始化 SDK。
3. **数据处理:**
    *   创建 `InputPacket` 结构体并填充数据，对于指针类型参数，需要使用 `malloc` 等函数分配内存并复制数据内容。
    *   调用 `LipSyncSDK_StartProcess` 函数开始处理数据。
    *   循环调用 `LipSyncSDK_TryGetNext` 函数获取处理结果，直到返回 `ErrorCode.TRY_GET_NEXT_OVERTIME` 或 标志为最后一帧。
    *   可以使用 `LipSyncSDK_Terminate` 函数中止处理。
    *   处理完 `OutputPacket` 中的数据后，需要手动释放 `uuid`、`frameData`、`audioData` 指针指向的内存。
4. **获取版本号:**
    *   调用 `LipSyncSDK_GetVersion` 函数获取 SDK 版本号，使用完后需要手动释放返回的字符串内存。
    *   或者调用 `LipSyncSDK_GetVersion_Callback` 函数，并通过回调函数获取版本号。
5. **销毁实例:** 当不再需要 SDK 时，调用 `LipSyncSDK_Destroy` 函数销毁实例，释放资源。

**注意：** 使用 C 语言接口时，需要特别注意内存管理，`SDKConfig`，`InputPacket` 和 `OutputPacket` 中的指针类型字段，以及`LipSyncSDK_GetVersion`返回的字符串都需要用户手动管理内存，并在使用完后释放，以避免内存泄漏。
