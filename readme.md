# c-media-codec

一个轻量级 **JNI 音频编解码与重采样库**，提供：

* **G.711 PCMU (μ-law)**
* **G.711 PCMA (A-law)**
* **G.722 wideband codec**
* **SpeexDSP Resampler（高质量音频重采样）**

该库面向 **VoIP / RTP / SIP / WebRTC / Media Server** 场景设计，核心目标是：

* **纯 C 实现**
* **无额外运行时依赖**
* **JNI DirectByteBuffer 零拷贝**
* **高性能实时音频处理**

---

# Features

* G.711 PCMU / PCMA 编解码
* G.722 宽带音频编解码
* 高质量 SpeexDSP 重采样
* JNI DirectByteBuffer **zero-copy pipeline**
* 支持 **实时流式处理**
* 跨平台构建

支持平台：

```
Linux
macOS
Windows
```

适用于：

```
VoIP
SIP Server
RTP Gateway
WebRTC Gateway
Media Server
Audio Processing Pipeline
```

---

# Project Structure

```
c-media-codec
├── CMakeLists.txt
├── jni
│   └── com_litongjava_media_MediaCodec.h
├── readme.md
├── src
│   ├── codec
│   │   ├── g711.c
│   │   ├── g711.h
│   │   ├── g722_decode.c
│   │   ├── g722_enc_dec.h
│   │   ├── g722_encode.c
│   │
│   ├── resampler
│   │   ├── arch.h
│   │   ├── os_support.h
│   │   ├── resample.c
│   │   ├── resampler_compat.h
│   │   ├── speex_resampler.h
│   │   ├── speexdsp_types.h
│   │
│   ├── jni_media_codec.c
│   ├── media_codec_core.c
│   ├── media_codec_core.h
│   ├── media_resample.c
│   └── media_resample.h
```

模块说明：

| 模块               | 说明                |
| ---------------- | ----------------- |
| codec            | G711 / G722 编解码实现 |
| resampler        | SpeexDSP 重采样实现    |
| media_codec_core | codec 统一封装        |
| media_resample   | 重采样封装             |
| jni_media_codec  | JNI 接口            |

---

# Source Code Origin

## G.711 / G.722

来源：

WebRTC third_party codec 实现

```
modules/third_party/g711
modules/third_party/g722
```

原始代码：

* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g711/g711.c](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g711/g711.c)
* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g711/g711.h](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g711/g711.h)
* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_encode.c](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_encode.c)
* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_decode.c](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_decode.c)
* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_enc_dec.h](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_enc_dec.h)

功能：

```
G711 μ-law / A-law encode / decode
G722 encoder / decoder
```

支持码率：

```
64000
56000
48000
```

---

## Resampler

来源：

SpeexDSP

```
https://github.com/xiph/speexdsp
```

使用文件：

```
resample.c
speex_resampler.h
arch.h
os_support.h
resampler_compat.h
speexdsp_types.h
```

提供：

```
高质量音频重采样
低延迟
流式处理
```

---

# Build

要求：

```
CMake >= 3.15
JDK (JNI headers)
C compiler
```

---

## 创建 build 目录

```
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
```

---

## 编译

```
cmake --build cmake-build-release --target all
```

---

## 输出文件

Linux：

```
libc_media_codec.so
```

macOS：

```
libc_media_codec.dylib
```

Windows：

```
c_media_codec.dll
```

---

# Java API

Java 类：

```
com.litongjava.media.MediaCodec
```

包含两部分：

```
Codec API
Resampler API
```

---

# Codec API

```java
public static native long createEncoder(
    int codecType,
    int sampleRate,
    int channels,
    int bitrate,
    int options);

public static native void destroyEncoder(long encoder);

public static native int encodeDirect(
    long encoder,
    ByteBuffer pcm16le,
    int pcmSamples,
    ByteBuffer encodedOut);

public static native long createDecoder(
    int codecType,
    int sampleRate,
    int channels,
    int bitrate,
    int options);

public static native void destroyDecoder(long decoder);

public static native int decodeDirect(
    long decoder,
    ByteBuffer encoded,
    int encodedLen,
    ByteBuffer pcmOut);
```

支持 codec：

| Codec | PayloadType |
| ----- | ----------- |
| PCMU  | 0           |
| PCMA  | 8           |
| G722  | 9           |

---

# Resampler API

```java
public static native long createResampler(
    int channels,
    int inputRate,
    int outputRate,
    int quality,
    int options);

public static native void destroyResampler(long resampler);

public static native int resampleDirect(
    long resampler,
    ByteBuffer pcm16leIn,
    int inputSamplesPerChannel,
    ByteBuffer pcm16leOut);

public static native int resetResampler(long resampler);

public static native int setResamplerRate(
    long resampler,
    int inputRate,
    int outputRate);

public static native int getResamplerExpectedOutputSamples(
    long resampler,
    int inputSamplesPerChannel);
```

---

# Codec Parameters

## G711

| 参数         | 值      |
| ---------- | ------ |
| sampleRate | 8000   |
| channels   | 1      |
| bitrate    | ignore |

20ms frame：

```
PCM samples = 160
encoded bytes = 160
```

---

## G722

| 参数         | 值                     |
| ---------- | --------------------- |
| sampleRate | 16000                 |
| channels   | 1                     |
| bitrate    | 64000 / 56000 / 48000 |

20ms frame：

| bitrate | encoded size |
| ------- | ------------ |
| 64000   | 160 bytes    |
| 56000   | 140 bytes    |
| 48000   | 120 bytes    |

PCM：

```
320 samples
```

---

# RTP Payload Size Table

以下以 **20ms 音频帧**为例。

## G711

| Codec | PayloadType | SampleRate | PCM Samples | Encoded Bytes | RTP Timestamp Step |
| ----- | ----------- | ---------- | ----------- | ------------- | ------------------ |
| PCMU  | 0           | 8000       | 160         | 160           | 160                |
| PCMA  | 8           | 8000       | 160         | 160           | 160                |

说明：

```
8000 * 20ms = 160 samples
```

G711 编码：

```
1 sample = 1 byte
```

---

## G722

| Codec | PayloadType | PCM SampleRate | PCM Samples | Bitrate | Encoded Bytes | RTP Timestamp |
| ----- | ----------- | -------------- | ----------- | ------- | ------------- | ------------- |
| G722  | 9           | 16000          | 320         | 64000   | 160           | 160           |
| G722  | 9           | 16000          | 320         | 56000   | 140           | 160           |
| G722  | 9           | 16000          | 320         | 48000   | 120           | 160           |

说明：

PCM sample：

```
16000 * 20ms = 320
```

RTP timestamp：

```
8000 * 20ms = 160
```

---

# Resampler Example

示例：

```
24000 Hz → 8000 Hz
```

```java
int inRate = 24000;
int outRate = 8000;

long rs = MediaCodec.createResampler(
    1,
    inRate,
    outRate,
    5,
    0);

ByteBuffer input = ByteBuffer.allocateDirect(960 * 2);
ByteBuffer output = ByteBuffer.allocateDirect(320 * 2);

int outSamples = MediaCodec.resampleDirect(
    rs,
    input,
    960,
    output);
```

---

# JNI Zero Copy

JNI 使用：

```
DirectByteBuffer
```

避免：

```
byte[] copy
```

JNI 获取地址：

```
GetDirectBufferAddress
```

实现：

```
zero-copy audio pipeline
```

适用于：

```
RTP
VoIP
SIP Server
Media Gateway
Audio Processing
```

---

# Important Notes

### 1 必须使用 DirectByteBuffer

正确：

```
ByteBuffer.allocateDirect(size)
```

错误：

```
ByteBuffer.wrap(byte[])
```

---

### 2 PCM 格式

必须为：

```
16-bit signed PCM
little-endian
```

建议：

```java
ByteBuffer.allocateDirect(size)
          .order(ByteOrder.LITTLE_ENDIAN)
```

---

### 3 Resampler 输入输出

格式：

```
PCM 16bit little-endian
```

支持：

```
任意采样率转换
流式处理
```

---

# License

第三方代码来自：

* WebRTC
* SpeexDSP

许可证：

```
Google BSD-style license
BSD-style license
```

请参考对应项目 License 文件。

---

# Summary

c-media-codec 提供：

```
JNI Audio Codec Bridge
        +
G711 / G722
        +
SpeexDSP Resampler
        +
Zero-copy DirectByteBuffer pipeline
```

适用于：

```
VoIP
RTP Server
SIP Server
WebRTC Gateway
Media Server
Audio Gateway
```
