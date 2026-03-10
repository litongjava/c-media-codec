#include "media_codec_core.h"

#include <stdlib.h>
#include <string.h>

#include "g711.h"
#include "g722_enc_dec.h"

struct media_codec_encoder_s {
  int codec_type;
  int sample_rate;
  int channels;
  int bitrate;
  int options;
  union {
    struct {
      int law; /* 0=u-law, 1=a-law */
    } g711;
    struct {
      G722EncoderState *state;
    } g722;
  } u;
};

struct media_codec_decoder_s {
  int codec_type;
  int sample_rate;
  int channels;
  int bitrate;
  int options;
  union {
    struct {
      int law; /* 0=u-law, 1=a-law */
    } g711;
    struct {
      G722DecoderState *state;
    } g722;
  } u;
};

static int media_codec_is_valid_codec(int codec_type) {
  return codec_type == MEDIA_CODEC_PCMU ||
         codec_type == MEDIA_CODEC_PCMA ||
         codec_type == MEDIA_CODEC_G722;
}

static int media_codec_is_valid_g722_bitrate(int bitrate) {
  return bitrate == 64000 || bitrate == 56000 || bitrate == 48000;
}

static int media_codec_validate_g711_common(int sample_rate, int channels) {
  if (sample_rate != 8000) {
    return MEDIA_CODEC_ERR_RATE;
  }
  if (channels != 1) {
    return MEDIA_CODEC_ERR_CHANNELS;
  }
  return MEDIA_CODEC_OK;
}

static int media_codec_validate_g722_common(int sample_rate, int channels, int bitrate, int options) {
  if (channels != 1) {
    return MEDIA_CODEC_ERR_CHANNELS;
  }
  if (!media_codec_is_valid_g722_bitrate(bitrate)) {
    return MEDIA_CODEC_ERR_BITRATE;
  }

  if ((options & G722_SAMPLE_RATE_8000) != 0) {
    if (sample_rate != 8000) {
      return MEDIA_CODEC_ERR_RATE;
    }
  } else {
    if (sample_rate != 16000) {
      return MEDIA_CODEC_ERR_RATE;
    }
  }

  /* only allow known flags */
  if ((options & ~(G722_SAMPLE_RATE_8000 | G722_PACKED)) != 0) {
    return MEDIA_CODEC_ERR_OPTIONS;
  }

  return MEDIA_CODEC_OK;
}

media_codec_encoder_t *media_codec_create_encoder(
  int codec_type,
  int sample_rate,
  int channels,
  int bitrate,
  int options) {

  media_codec_encoder_t *enc;
  int rc;

  if (!media_codec_is_valid_codec(codec_type)) {
    return NULL;
  }

  enc = (media_codec_encoder_t *)calloc(1, sizeof(*enc));
  if (enc == NULL) {
    return NULL;
  }

  enc->codec_type = codec_type;
  enc->sample_rate = sample_rate;
  enc->channels = channels;
  enc->bitrate = bitrate;
  enc->options = options;

  switch (codec_type) {
    case MEDIA_CODEC_PCMU:
      rc = media_codec_validate_g711_common(sample_rate, channels);
      if (rc != MEDIA_CODEC_OK) {
        free(enc);
        return NULL;
      }
      enc->u.g711.law = 0;
      return enc;

    case MEDIA_CODEC_PCMA:
      rc = media_codec_validate_g711_common(sample_rate, channels);
      if (rc != MEDIA_CODEC_OK) {
        free(enc);
        return NULL;
      }
      enc->u.g711.law = 1;
      return enc;

    case MEDIA_CODEC_G722:
      rc = media_codec_validate_g722_common(sample_rate, channels, bitrate, options);
      if (rc != MEDIA_CODEC_OK) {
        free(enc);
        return NULL;
      }
      enc->u.g722.state = WebRtc_g722_encode_init(NULL, bitrate, options);
      if (enc->u.g722.state == NULL) {
        free(enc);
        return NULL;
      }
      return enc;

    default:
      free(enc);
      return NULL;
  }
}

void media_codec_destroy_encoder(media_codec_encoder_t *encoder) {
  if (encoder == NULL) {
    return;
  }

  if (encoder->codec_type == MEDIA_CODEC_G722 && encoder->u.g722.state != NULL) {
    WebRtc_g722_encode_release(encoder->u.g722.state);
    encoder->u.g722.state = NULL;
  }

  free(encoder);
}

media_codec_decoder_t *media_codec_create_decoder(
  int codec_type,
  int sample_rate,
  int channels,
  int bitrate,
  int options) {

  media_codec_decoder_t *dec;
  int rc;

  if (!media_codec_is_valid_codec(codec_type)) {
    return NULL;
  }

  dec = (media_codec_decoder_t *)calloc(1, sizeof(*dec));
  if (dec == NULL) {
    return NULL;
  }

  dec->codec_type = codec_type;
  dec->sample_rate = sample_rate;
  dec->channels = channels;
  dec->bitrate = bitrate;
  dec->options = options;

  switch (codec_type) {
    case MEDIA_CODEC_PCMU:
      rc = media_codec_validate_g711_common(sample_rate, channels);
      if (rc != MEDIA_CODEC_OK) {
        free(dec);
        return NULL;
      }
      dec->u.g711.law = 0;
      return dec;

    case MEDIA_CODEC_PCMA:
      rc = media_codec_validate_g711_common(sample_rate, channels);
      if (rc != MEDIA_CODEC_OK) {
        free(dec);
        return NULL;
      }
      dec->u.g711.law = 1;
      return dec;

    case MEDIA_CODEC_G722:
      rc = media_codec_validate_g722_common(sample_rate, channels, bitrate, options);
      if (rc != MEDIA_CODEC_OK) {
        free(dec);
        return NULL;
      }
      dec->u.g722.state = WebRtc_g722_decode_init(NULL, bitrate, options);
      if (dec->u.g722.state == NULL) {
        free(dec);
        return NULL;
      }
      return dec;

    default:
      free(dec);
      return NULL;
  }
}

void media_codec_destroy_decoder(media_codec_decoder_t *decoder) {
  if (decoder == NULL) {
    return;
  }

  if (decoder->codec_type == MEDIA_CODEC_G722 && decoder->u.g722.state != NULL) {
    WebRtc_g722_decode_release(decoder->u.g722.state);
    decoder->u.g722.state = NULL;
  }

  free(decoder);
}

static int media_codec_encode_g711(
  media_codec_encoder_t *encoder,
  const int16_t *pcm16le,
  int pcm_samples,
  uint8_t *encoded_out,
  int encoded_cap) {

  int i;

  if (encoder == NULL || pcm16le == NULL || encoded_out == NULL || pcm_samples < 0 || encoded_cap < 0) {
    return MEDIA_CODEC_ERR_ARG;
  }

  /* 1 sample -> 1 byte */
  if (encoded_cap < pcm_samples) {
    return MEDIA_CODEC_ERR_OUTBUF_SMALL;
  }

  if (encoder->u.g711.law == 0) {
    for (i = 0; i < pcm_samples; ++i) {
      encoded_out[i] = linear_to_ulaw(pcm16le[i]);
    }
  } else {
    for (i = 0; i < pcm_samples; ++i) {
      encoded_out[i] = linear_to_alaw(pcm16le[i]);
    }
  }

  return pcm_samples;
}

static int media_codec_decode_g711(
  media_codec_decoder_t *decoder,
  const uint8_t *encoded,
  int encoded_len,
  int16_t *pcm16le_out,
  int pcm_cap_samples) {

  int i;

  if (decoder == NULL || encoded == NULL || pcm16le_out == NULL || encoded_len < 0 || pcm_cap_samples < 0) {
    return MEDIA_CODEC_ERR_ARG;
  }

  if (pcm_cap_samples < encoded_len) {
    return MEDIA_CODEC_ERR_OUTBUF_SMALL;
  }

  if (decoder->u.g711.law == 0) {
    for (i = 0; i < encoded_len; ++i) {
      pcm16le_out[i] = ulaw_to_linear(encoded[i]);
    }
  } else {
    for (i = 0; i < encoded_len; ++i) {
      pcm16le_out[i] = alaw_to_linear(encoded[i]);
    }
  }

  return encoded_len;
}

int media_codec_encode(
  media_codec_encoder_t *encoder,
  const int16_t *pcm16le,
  int pcm_samples,
  uint8_t *encoded_out,
  int encoded_cap) {

  size_t out_len;

  if (encoder == NULL || pcm16le == NULL || encoded_out == NULL || pcm_samples < 0 || encoded_cap < 0) {
    return MEDIA_CODEC_ERR_ARG;
  }

  switch (encoder->codec_type) {
    case MEDIA_CODEC_PCMU:
    case MEDIA_CODEC_PCMA:
      return media_codec_encode_g711(encoder, pcm16le, pcm_samples, encoded_out, encoded_cap);

    case MEDIA_CODEC_G722:
      if (encoder->u.g722.state == NULL) {
        return MEDIA_CODEC_ERR_STATE;
      }
      out_len = WebRtc_g722_encode(
        encoder->u.g722.state,
        encoded_out,
        pcm16le,
        (size_t)pcm_samples
      );
      if ((size_t)encoded_cap < out_len) {
        return MEDIA_CODEC_ERR_OUTBUF_SMALL;
      }
      return (int)out_len;

    default:
      return MEDIA_CODEC_ERR_CODEC;
  }
}

int media_codec_decode(
  media_codec_decoder_t *decoder,
  const uint8_t *encoded,
  int encoded_len,
  int16_t *pcm16le_out,
  int pcm_cap_samples) {

  size_t out_samples;

  if (decoder == NULL || encoded == NULL || pcm16le_out == NULL || encoded_len < 0 || pcm_cap_samples < 0) {
    return MEDIA_CODEC_ERR_ARG;
  }

  switch (decoder->codec_type) {
    case MEDIA_CODEC_PCMU:
    case MEDIA_CODEC_PCMA:
      return media_codec_decode_g711(decoder, encoded, encoded_len, pcm16le_out, pcm_cap_samples);

    case MEDIA_CODEC_G722:
      if (decoder->u.g722.state == NULL) {
        return MEDIA_CODEC_ERR_STATE;
      }
      out_samples = WebRtc_g722_decode(
        decoder->u.g722.state,
        pcm16le_out,
        encoded,
        (size_t)encoded_len
      );
      if ((size_t)pcm_cap_samples < out_samples) {
        return MEDIA_CODEC_ERR_OUTBUF_SMALL;
      }
      return (int)out_samples;

    default:
      return MEDIA_CODEC_ERR_CODEC;
  }
}

int media_codec_get_pcm_samples_per_20ms(
  int codec_type,
  int sample_rate,
  int channels) {

  if (!media_codec_is_valid_codec(codec_type)) {
    return MEDIA_CODEC_ERR_CODEC;
  }
  if (channels != 1) {
    return MEDIA_CODEC_ERR_CHANNELS;
  }

  switch (codec_type) {
    case MEDIA_CODEC_PCMU:
    case MEDIA_CODEC_PCMA:
      if (sample_rate != 8000) {
        return MEDIA_CODEC_ERR_RATE;
      }
      return 8000 / 50; /* 160 */

    case MEDIA_CODEC_G722:
      if (sample_rate == 16000) {
        return 16000 / 50; /* 320 */
      }
      if (sample_rate == 8000) {
        return 8000 / 50; /* special option mode */
      }
      return MEDIA_CODEC_ERR_RATE;

    default:
      return MEDIA_CODEC_ERR_CODEC;
  }
}

int media_codec_get_encoded_bytes_per_20ms(
  int codec_type,
  int sample_rate,
  int channels,
  int bitrate) {

  if (!media_codec_is_valid_codec(codec_type)) {
    return MEDIA_CODEC_ERR_CODEC;
  }
  if (channels != 1) {
    return MEDIA_CODEC_ERR_CHANNELS;
  }

  switch (codec_type) {
    case MEDIA_CODEC_PCMU:
    case MEDIA_CODEC_PCMA:
      if (sample_rate != 8000) {
        return MEDIA_CODEC_ERR_RATE;
      }
      return 8000 / 50; /* 160 bytes */

    case MEDIA_CODEC_G722:
      if (!(sample_rate == 16000 || sample_rate == 8000)) {
        return MEDIA_CODEC_ERR_RATE;
      }
      if (!media_codec_is_valid_g722_bitrate(bitrate)) {
        return MEDIA_CODEC_ERR_BITRATE;
      }
      return (bitrate / 8) / 50;

    default:
      return MEDIA_CODEC_ERR_CODEC;
  }
}