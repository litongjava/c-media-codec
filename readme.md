# c-media-codec

一个轻量级 **JNI 音频编解码库**，提供以下 codec：

* **G.711 PCMU (μ-law)**
* **G.711 PCMA (A-law)**
* **G.722 wideband codec**

特点：

* 纯 **C 实现**
* **无第三方依赖**
* **JNI DirectByteBuffer 零拷贝**
* 可用于 **VoIP / RTP / SIP / WebRTC 相关项目**

---

# Source Code Origin (源码来源)

本项目中的 codec 实现来自 WebRTC 的第三方 codec 目录。

来源仓库：

* WebRTC

具体来源文件：

## G.711

来源：

```
modules/third_party/g711/
```

源码：

* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g711/g711.c](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g711/g711.c)
* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g711/g711.h](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g711/g711.h)

功能：

* μ-law 编码 / 解码
* A-law 编码 / 解码

---

## G.722

来源：

```
modules/third_party/g722/
```

源码：

* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_encode.c](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_encode.c)
* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_decode.c](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_decode.c)
* [https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_enc_dec.h](https://github.com/litongjava/webrtc/blob/main/modules/third_party/g722/g722_enc_dec.h)

功能：

* G.722 encoder
* G.722 decoder
* 支持码率：

```
64000
56000
48000
```

---

# Project Structure

```
c-media-codec
├── CMakeLists.txt
├── jni
│   └── com_litongjava_media_MediaCodec.h
├── readme.me
├── src
│   ├── g711.c
│   ├── g711.h
│   ├── g722_decode.c
│   ├── g722_enc_dec.h
│   ├── g722_encode.c
│   ├── jni_media_codec.c
│   ├── media_codec_core.c
│   ├── media_codec_core.h
```

模块说明：

| 文件                 | 说明         |
| ------------------ | ---------- |
| g711.c / g711.h    | G711 编解码实现 |
| g722_encode.c      | G722 编码器   |
| g722_decode.c      | G722 解码器   |
| g722_enc_dec.h     | G722 头文件   |
| media_codec_core.c | codec 统一封装 |
| jni_media_codec.c  | JNI 接口实现   |

---

# Build

要求：

```
CMake >= 3.27
JDK (用于 JNI)
C compiler
```

---

## 生成 build 目录

```
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
```

---

## 编译

```
cmake --build cmake-build-release --target all
```

---

## 生成结果

Linux:

```
libc_media_codec.so
```

macOS:

```
libc_media_codec.dylib
```

Windows:

```
c_media_codec.dll
```

---

# Java Usage

Java API：

```java
public final class MediaCodec {

  public static final int CODEC_PCMU = 0;
  public static final int CODEC_PCMA = 8;
  public static final int CODEC_G722 = 9;

  public static native long createEncoder(
      int codecType,
      int sampleRate,
      int channels,
      int bitrate,
      int options);

  public static native int encodeDirect(
      long encoder,
      ByteBuffer pcm16le,
      int pcmSamples,
      ByteBuffer encodedOut);

  public static native int decodeDirect(
      long decoder,
      ByteBuffer encoded,
      int encodedLen,
      ByteBuffer pcmOut);
}
```

---

# Codec Parameters

## G711

| 参数         | 值      |
| ---------- | ------ |
| sampleRate | 8000   |
| channels   | 1      |
| bitrate    | ignore |

20ms frame:

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

20ms frame:

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

# JNI Zero Copy

JNI 使用：

```
DirectByteBuffer
```

避免：

```
byte[] copy
```

调用：

```
GetDirectBufferAddress
```

实现：

```
zero-copy audio pipeline
```

适用于：

* RTP
* VoIP
* SIP server
* media gateway

---

# License

第三方 codec 来自 WebRTC 项目。

请参考：

* WebRTC license
* Google BSD-style license

---

# Summary

本项目提供：

```
JNI codec bridge
↓
G711 / G722
↓
pure C implementation
```

适用于：

```
VoIP
SIP
RTP
Media Server
Audio Gateway
```

---

# RTP Payload Size Table

以下表格以 **20ms 音频帧** 为例。

## G.711

G.711 为 8kHz、单声道、8-bit companding。

### 20ms

| Codec | Payload Type | PCM Sample Rate | PCM Samples / 20ms | Encoded Bytes / 20ms | RTP Timestamp Step |
| ----- | -----------: | --------------: | -----------------: | -------------------: | -----------------: |
| PCMU  |            0 |            8000 |                160 |                  160 |                160 |
| PCMA  |            8 |            8000 |                160 |                  160 |                160 |

说明：

* G.711 编码后 **1 sample = 1 byte**
* 20ms 时：

  * `8000 * 20 / 1000 = 160 samples`
  * payload size = `160 bytes`

---

## G.722

G.722 在本项目中按常规使用：

* PCM 输入：`16kHz / 16bit / mono`
* RTP clock rate：`8000`

### 20ms

| Codec | Payload Type | PCM Sample Rate | PCM Samples / 20ms | Bitrate | Encoded Bytes / 20ms | RTP Timestamp Step |
| ----- | -----------: | --------------: | -----------------: | ------: | -------------------: | -----------------: |
| G722  |            9 |           16000 |                320 |   64000 |                  160 |                160 |
| G722  |            9 |           16000 |                320 |   56000 |                  140 |                160 |
| G722  |            9 |           16000 |                320 |   48000 |                  120 |                160 |

说明：

* 20ms PCM sample 数：

  * `16000 * 20 / 1000 = 320`
* payload size：

  * `64000 / 8 * 20 / 1000 = 160 bytes`
  * `56000 / 8 * 20 / 1000 = 140 bytes`
  * `48000 / 8 * 20 / 1000 = 120 bytes`
* **G.722 RTP timestamp step 始终按 8000 clock 计算**

  * `8000 * 20 / 1000 = 160`

---

## Quick Reference

| Codec    | Frame | PCM Samples | PCM Bytes | Encoded Bytes |
| -------- | ----: | ----------: | --------: | ------------: |
| PCMU     |  20ms |         160 |       320 |           160 |
| PCMA     |  20ms |         160 |       320 |           160 |
| G722@64k |  20ms |         320 |       640 |           160 |
| G722@56k |  20ms |         320 |       640 |           140 |
| G722@48k |  20ms |         320 |       640 |           120 |

---

# Java Usage Example

下面给出一个完整的 Java encode/decode 示例，演示：

* 创建 encoder / decoder
* 分配 `DirectByteBuffer`
* 进行 PCM -> encoded -> PCM
* 销毁 native context

---

## MediaCodec.java

```java
package com.litongjava.media;

import java.nio.ByteBuffer;

public final class MediaCodec {
  public static final int CODEC_PCMU = 0;
  public static final int CODEC_PCMA = 8;
  public static final int CODEC_G722 = 9;

  static {
    System.loadLibrary("c_media_codec");
  }

  public static native long createEncoder(int codecType, int sampleRate, int channels, int bitrate, int options);

  public static native void destroyEncoder(long encoder);

  public static native int encodeDirect(long encoder, ByteBuffer pcm16le, int pcmSamples, ByteBuffer encodedOut);

  public static native long createDecoder(int codecType, int sampleRate, int channels, int bitrate, int options);

  public static native void destroyDecoder(long decoder);

  public static native int decodeDirect(long decoder, ByteBuffer encoded, int encodedLen, ByteBuffer pcm16leOut);

  public static native int getPcmSamplesPer20ms(int codecType, int sampleRate, int channels);

  public static native int getEncodedBytesPer20ms(int codecType, int sampleRate, int channels, int bitrate);
}
```

---

## Example: G.722 encode / decode

```java
package com.litongjava.media;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class MediaCodecDemoG722 {

  public static void main(String[] args) {
    int codecType = MediaCodec.CODEC_G722;
    int sampleRate = 16000;
    int channels = 1;
    int bitrate = 64000;
    int options = 0;

    int pcmSamples = MediaCodec.getPcmSamplesPer20ms(codecType, sampleRate, channels);
    int encodedBytes = MediaCodec.getEncodedBytesPer20ms(codecType, sampleRate, channels, bitrate);

    if (pcmSamples <= 0 || encodedBytes <= 0) {
      throw new IllegalStateException("invalid frame size, pcmSamples=" + pcmSamples + ", encodedBytes=" + encodedBytes);
    }

    ByteBuffer pcmIn = ByteBuffer.allocateDirect(pcmSamples * 2).order(ByteOrder.LITTLE_ENDIAN);
    ByteBuffer encoded = ByteBuffer.allocateDirect(encodedBytes);
    ByteBuffer pcmOut = ByteBuffer.allocateDirect(pcmSamples * 2).order(ByteOrder.LITTLE_ENDIAN);

    for (int i = 0; i < pcmSamples; i++) {
      short sample = (short) (Math.sin(i * 2.0 * Math.PI / 32.0) * 10000);
      pcmIn.putShort(i * 2, sample);
    }

    long encoder = MediaCodec.createEncoder(codecType, sampleRate, channels, bitrate, options);
    long decoder = MediaCodec.createDecoder(codecType, sampleRate, channels, bitrate, options);

    if (encoder == 0 || decoder == 0) {
      throw new IllegalStateException("create encoder/decoder failed");
    }

    try {
      int encLen = MediaCodec.encodeDirect(encoder, pcmIn, pcmSamples, encoded);
      System.out.println("G722 encoded bytes = " + encLen);

      if (encLen < 0) {
        throw new IllegalStateException("encode failed: " + encLen);
      }

      int decSamples = MediaCodec.decodeDirect(decoder, encoded, encLen, pcmOut);
      System.out.println("G722 decoded samples = " + decSamples);

      if (decSamples < 0) {
        throw new IllegalStateException("decode failed: " + decSamples);
      }

      for (int i = 0; i < Math.min(decSamples, 10); i++) {
        short value = pcmOut.getShort(i * 2);
        System.out.println("pcmOut[" + i + "] = " + value);
      }
    } finally {
      MediaCodec.destroyEncoder(encoder);
      MediaCodec.destroyDecoder(decoder);
    }
  }
}
```

---

## Example: PCMU encode / decode

```java
package com.litongjava.media;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class MediaCodecDemoPcmu {

  public static void main(String[] args) {
    int codecType = MediaCodec.CODEC_PCMU;
    int sampleRate = 8000;
    int channels = 1;
    int bitrate = 0;
    int options = 0;

    int pcmSamples = MediaCodec.getPcmSamplesPer20ms(codecType, sampleRate, channels);
    int encodedBytes = MediaCodec.getEncodedBytesPer20ms(codecType, sampleRate, channels, bitrate);

    ByteBuffer pcmIn = ByteBuffer.allocateDirect(pcmSamples * 2).order(ByteOrder.LITTLE_ENDIAN);
    ByteBuffer encoded = ByteBuffer.allocateDirect(encodedBytes);
    ByteBuffer pcmOut = ByteBuffer.allocateDirect(pcmSamples * 2).order(ByteOrder.LITTLE_ENDIAN);

    for (int i = 0; i < pcmSamples; i++) {
      short sample = (short) ((i % 40 - 20) * 500);
      pcmIn.putShort(i * 2, sample);
    }

    long encoder = MediaCodec.createEncoder(codecType, sampleRate, channels, bitrate, options);
    long decoder = MediaCodec.createDecoder(codecType, sampleRate, channels, bitrate, options);

    if (encoder == 0 || decoder == 0) {
      throw new IllegalStateException("create encoder/decoder failed");
    }

    try {
      int encLen = MediaCodec.encodeDirect(encoder, pcmIn, pcmSamples, encoded);
      System.out.println("PCMU encoded bytes = " + encLen);

      if (encLen < 0) {
        throw new IllegalStateException("encode failed: " + encLen);
      }

      int decSamples = MediaCodec.decodeDirect(decoder, encoded, encLen, pcmOut);
      System.out.println("PCMU decoded samples = " + decSamples);

      if (decSamples < 0) {
        throw new IllegalStateException("decode failed: " + decSamples);
      }

      for (int i = 0; i < Math.min(decSamples, 10); i++) {
        short value = pcmOut.getShort(i * 2);
        System.out.println("pcmOut[" + i + "] = " + value);
      }
    } finally {
      MediaCodec.destroyEncoder(encoder);
      MediaCodec.destroyDecoder(decoder);
    }
  }
}
```

---

## Example: PCMA encode / decode

```java
package com.litongjava.media;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class MediaCodecDemoPcma {

  public static void main(String[] args) {
    int codecType = MediaCodec.CODEC_PCMA;
    int sampleRate = 8000;
    int channels = 1;
    int bitrate = 0;
    int options = 0;

    int pcmSamples = MediaCodec.getPcmSamplesPer20ms(codecType, sampleRate, channels);
    int encodedBytes = MediaCodec.getEncodedBytesPer20ms(codecType, sampleRate, channels, bitrate);

    ByteBuffer pcmIn = ByteBuffer.allocateDirect(pcmSamples * 2).order(ByteOrder.LITTLE_ENDIAN);
    ByteBuffer encoded = ByteBuffer.allocateDirect(encodedBytes);
    ByteBuffer pcmOut = ByteBuffer.allocateDirect(pcmSamples * 2).order(ByteOrder.LITTLE_ENDIAN);

    for (int i = 0; i < pcmSamples; i++) {
      short sample = (short) ((i % 50 - 25) * 400);
      pcmIn.putShort(i * 2, sample);
    }

    long encoder = MediaCodec.createEncoder(codecType, sampleRate, channels, bitrate, options);
    long decoder = MediaCodec.createDecoder(codecType, sampleRate, channels, bitrate, options);

    if (encoder == 0 || decoder == 0) {
      throw new IllegalStateException("create encoder/decoder failed");
    }

    try {
      int encLen = MediaCodec.encodeDirect(encoder, pcmIn, pcmSamples, encoded);
      System.out.println("PCMA encoded bytes = " + encLen);

      if (encLen < 0) {
        throw new IllegalStateException("encode failed: " + encLen);
      }

      int decSamples = MediaCodec.decodeDirect(decoder, encoded, encLen, pcmOut);
      System.out.println("PCMA decoded samples = " + decSamples);

      if (decSamples < 0) {
        throw new IllegalStateException("decode failed: " + decSamples);
      }

      for (int i = 0; i < Math.min(decSamples, 10); i++) {
        short value = pcmOut.getShort(i * 2);
        System.out.println("pcmOut[" + i + "] = " + value);
      }
    } finally {
      MediaCodec.destroyEncoder(encoder);
      MediaCodec.destroyDecoder(decoder);
    }
  }
}
```

---

## Notes

### 1. 必须使用 DirectByteBuffer

正确：

```java
ByteBuffer.allocateDirect(size)
```

不要用：

```java
ByteBuffer.wrap(new byte[size])
```

---

### 2. PCM 必须是 little-endian 16-bit

建议：

```java
ByteBuffer.allocateDirect(size).order(ByteOrder.LITTLE_ENDIAN)
```

---

### 3. G711 参数

* `sampleRate = 8000`
* `channels = 1`
* `bitrate = 0`
* `options = 0`

---

### 4. G722 参数

常规宽带模式：

* `sampleRate = 16000`
* `channels = 1`
* `bitrate = 64000 / 56000 / 48000`
* `options = 0`

---

### 5. 返回值含义

* `encodeDirect(...)` 返回编码后的字节数
* `decodeDirect(...)` 返回解码后的 PCM sample 数
* 返回负数表示错误

