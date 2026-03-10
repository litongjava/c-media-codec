#include <jni.h>
#include <stdint.h>
#include <stddef.h>

#include "com_litongjava_media_MediaCodec.h"
#include "media_codec_core.h"

static void *jni_get_direct_buffer(JNIEnv *env, jobject buffer, jlong *capacity) {
  void *addr;

  if (buffer == NULL) {
    return NULL;
  }

  addr = (*env)->GetDirectBufferAddress(env, buffer);
  if (addr == NULL) {
    return NULL;
  }

  if (capacity != NULL) {
    *capacity = (*env)->GetDirectBufferCapacity(env, buffer);
  }

  return addr;
}

static media_codec_encoder_t *jlong_to_encoder(jlong value) {
  return (media_codec_encoder_t *)(intptr_t)value;
}

static media_codec_decoder_t *jlong_to_decoder(jlong value) {
  return (media_codec_decoder_t *)(intptr_t)value;
}

static jlong encoder_to_jlong(media_codec_encoder_t *encoder) {
  return (jlong)(intptr_t)encoder;
}

static jlong decoder_to_jlong(media_codec_decoder_t *decoder) {
  return (jlong)(intptr_t)decoder;
}

JNIEXPORT jlong JNICALL
Java_com_litongjava_media_MediaCodec_createEncoder(JNIEnv *env,
                                                   jclass clazz,
                                                   jint codecType,
                                                   jint sampleRate,
                                                   jint channels,
                                                   jint bitrate,
                                                   jint options) {
  media_codec_encoder_t *encoder;
  (void)env;
  (void)clazz;

  encoder = media_codec_create_encoder(
    (int)codecType,
    (int)sampleRate,
    (int)channels,
    (int)bitrate,
    (int)options
  );

  if (encoder == NULL) {
    return (jlong)0;
  }

  return encoder_to_jlong(encoder);
}

JNIEXPORT void JNICALL
Java_com_litongjava_media_MediaCodec_destroyEncoder(JNIEnv *env,
jclass clazz,
  jlong encoderPtr) {
media_codec_encoder_t *encoder;
(void)env;
(void)clazz;

if (encoderPtr == 0) {
return;
}

encoder = jlong_to_encoder(encoderPtr);
media_codec_destroy_encoder(encoder);
}

JNIEXPORT jint JNICALL
Java_com_litongjava_media_MediaCodec_encodeDirect(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong encoderPtr,
                                                  jobject pcm16le,
                                                  jint pcmSamples,
                                                  jobject encodedOut) {
  media_codec_encoder_t *encoder;
  void *pcm_addr;
  void *encoded_addr;
  jlong pcm_cap_bytes = 0;
  jlong encoded_cap_bytes = 0;

  (void)clazz;

  if (encoderPtr == 0 || pcm16le == NULL || encodedOut == NULL || pcmSamples < 0) {
    return (jint)MEDIA_CODEC_ERR_ARG;
  }

  encoder = jlong_to_encoder(encoderPtr);
  if (encoder == NULL) {
    return (jint)MEDIA_CODEC_ERR_STATE;
  }

  pcm_addr = jni_get_direct_buffer(env, pcm16le, &pcm_cap_bytes);
  if (pcm_addr == NULL) {
    return (jint)MEDIA_CODEC_ERR_NOT_DIRECT;
  }

  encoded_addr = jni_get_direct_buffer(env, encodedOut, &encoded_cap_bytes);
  if (encoded_addr == NULL) {
    return (jint)MEDIA_CODEC_ERR_NOT_DIRECT;
  }

  if (pcm_cap_bytes < ((jlong)pcmSamples * (jlong)sizeof(int16_t))) {
    return (jint)MEDIA_CODEC_ERR_OUTBUF_SMALL;
  }

  return (jint)media_codec_encode(
    encoder,
    (const int16_t *)pcm_addr,
    (int)pcmSamples,
    (uint8_t *)encoded_addr,
    (int)encoded_cap_bytes
  );
}

JNIEXPORT jlong JNICALL
Java_com_litongjava_media_MediaCodec_createDecoder(JNIEnv *env,
                                                   jclass clazz,
                                                   jint codecType,
                                                   jint sampleRate,
                                                   jint channels,
                                                   jint bitrate,
                                                   jint options) {
  media_codec_decoder_t *decoder;
  (void)env;
  (void)clazz;

  decoder = media_codec_create_decoder(
    (int)codecType,
    (int)sampleRate,
    (int)channels,
    (int)bitrate,
    (int)options
  );

  if (decoder == NULL) {
    return (jlong)0;
  }

  return decoder_to_jlong(decoder);
}

JNIEXPORT void JNICALL
Java_com_litongjava_media_MediaCodec_destroyDecoder(JNIEnv *env,
jclass clazz,
  jlong decoderPtr) {
media_codec_decoder_t *decoder;
(void)env;
(void)clazz;

if (decoderPtr == 0) {
return;
}

decoder = jlong_to_decoder(decoderPtr);
media_codec_destroy_decoder(decoder);
}

JNIEXPORT jint JNICALL
Java_com_litongjava_media_MediaCodec_decodeDirect(JNIEnv *env,
                                                  jclass clazz,
                                                  jlong decoderPtr,
                                                  jobject encoded,
                                                  jint encodedLen,
                                                  jobject pcm16leOut) {
  media_codec_decoder_t *decoder;
  void *encoded_addr;
  void *pcm_addr;
  jlong encoded_cap_bytes = 0;
  jlong pcm_cap_bytes = 0;
  int pcm_cap_samples;

  (void)clazz;

  if (decoderPtr == 0 || encoded == NULL || pcm16leOut == NULL || encodedLen < 0) {
    return (jint)MEDIA_CODEC_ERR_ARG;
  }

  decoder = jlong_to_decoder(decoderPtr);
  if (decoder == NULL) {
    return (jint)MEDIA_CODEC_ERR_STATE;
  }

  encoded_addr = jni_get_direct_buffer(env, encoded, &encoded_cap_bytes);
  if (encoded_addr == NULL) {
    return (jint)MEDIA_CODEC_ERR_NOT_DIRECT;
  }

  pcm_addr = jni_get_direct_buffer(env, pcm16leOut, &pcm_cap_bytes);
  if (pcm_addr == NULL) {
    return (jint)MEDIA_CODEC_ERR_NOT_DIRECT;
  }

  if (encoded_cap_bytes < (jlong)encodedLen) {
    return (jint)MEDIA_CODEC_ERR_ARG;
  }

  pcm_cap_samples = (int)(pcm_cap_bytes / (jlong)sizeof(int16_t));

  return (jint)media_codec_decode(
    decoder,
    (const uint8_t *)encoded_addr,
    (int)encodedLen,
    (int16_t *)pcm_addr,
    pcm_cap_samples
  );
}

JNIEXPORT jint JNICALL
Java_com_litongjava_media_MediaCodec_getPcmSamplesPer20ms(JNIEnv *env,
                                                          jclass clazz,
                                                          jint codecType,
                                                          jint sampleRate,
                                                          jint channels) {
  (void)env;
  (void)clazz;

  return (jint)media_codec_get_pcm_samples_per_20ms(
    (int)codecType,
    (int)sampleRate,
    (int)channels
  );
}

JNIEXPORT jint JNICALL
Java_com_litongjava_media_MediaCodec_getEncodedBytesPer20ms(JNIEnv *env,
                                                            jclass clazz,
                                                            jint codecType,
                                                            jint sampleRate,
                                                            jint channels,
                                                            jint bitrate) {
  (void)env;
  (void)clazz;

  return (jint)media_codec_get_encoded_bytes_per_20ms(
    (int)codecType,
    (int)sampleRate,
    (int)channels,
    (int)bitrate
  );
}