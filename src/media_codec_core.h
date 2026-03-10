#ifndef MEDIA_CODEC_CORE_H
#define MEDIA_CODEC_CORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEDIA_CODEC_PCMU 0
#define MEDIA_CODEC_PCMA 8
#define MEDIA_CODEC_G722 9

#define MEDIA_CODEC_OK                 0
#define MEDIA_CODEC_ERR_ARG           -1
#define MEDIA_CODEC_ERR_CODEC         -2
#define MEDIA_CODEC_ERR_RATE          -3
#define MEDIA_CODEC_ERR_CHANNELS      -4
#define MEDIA_CODEC_ERR_BITRATE       -5
#define MEDIA_CODEC_ERR_OPTIONS       -6
#define MEDIA_CODEC_ERR_NOMEM         -7
#define MEDIA_CODEC_ERR_OUTBUF_SMALL  -8
#define MEDIA_CODEC_ERR_STATE         -9
#define MEDIA_CODEC_ERR_NOT_DIRECT   -10

typedef struct media_codec_encoder_s media_codec_encoder_t;
typedef struct media_codec_decoder_s media_codec_decoder_t;

/* encoder lifecycle */
media_codec_encoder_t *media_codec_create_encoder(
  int codec_type,
  int sample_rate,
  int channels,
  int bitrate,
  int options);

void media_codec_destroy_encoder(media_codec_encoder_t *encoder);

/* decoder lifecycle */
media_codec_decoder_t *media_codec_create_decoder(
  int codec_type,
  int sample_rate,
  int channels,
  int bitrate,
  int options);

void media_codec_destroy_decoder(media_codec_decoder_t *decoder);

/*
 * encode:
 *   pcm16le      : int16 PCM buffer
 *   pcm_samples  : number of samples, not bytes
 *   encoded_out  : output buffer
 *   encoded_cap  : output buffer capacity in bytes
 *
 * return:
 *   >= 0 : encoded byte count
 *   <  0 : error code
 */
int media_codec_encode(
  media_codec_encoder_t *encoder,
  const int16_t *pcm16le,
  int pcm_samples,
  uint8_t *encoded_out,
  int encoded_cap);

/*
 * decode:
 *   encoded      : encoded input bytes
 *   encoded_len  : input bytes
 *   pcm16le_out  : output PCM buffer
 *   pcm_cap_samples : PCM output capacity in samples
 *
 * return:
 *   >= 0 : decoded sample count
 *   <  0 : error code
 */
int media_codec_decode(
  media_codec_decoder_t *decoder,
  const uint8_t *encoded,
  int encoded_len,
  int16_t *pcm16le_out,
  int pcm_cap_samples);

/* helper methods for Java */
int media_codec_get_pcm_samples_per_20ms(
  int codec_type,
  int sample_rate,
  int channels);

int media_codec_get_encoded_bytes_per_20ms(
  int codec_type,
  int sample_rate,
  int channels,
  int bitrate);

#ifdef __cplusplus
}
#endif

#endif