#include "media_resample.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "media_codec_core.h"
#include "resampler/speex_resampler.h"

struct media_resampler_s {
  SpeexResamplerState *state;
  int channels;
  int input_rate;
  int output_rate;
  int quality;
  int options;
};

static int media_resampler_validate_params(int channels,
                                           int input_rate,
                                           int output_rate,
                                           int quality) {
  if (channels <= 0) {
    return MEDIA_CODEC_ERR_ARG;
  }
  if (input_rate <= 0 || output_rate <= 0) {
    return MEDIA_CODEC_ERR_ARG;
  }
  if (quality < 0 || quality > 10) {
    return MEDIA_CODEC_ERR_ARG;
  }
  return 0;
}

media_resampler_t *media_resampler_create(int channels,
                                          int input_rate,
                                          int output_rate,
                                          int quality,
                                          int options) {
  media_resampler_t *resampler;
  SpeexResamplerState *state;
  int err;

  if (media_resampler_validate_params(channels, input_rate, output_rate, quality) != 0) {
    return NULL;
  }

  state = speex_resampler_init((spx_uint32_t)channels,
                               (spx_uint32_t)input_rate,
                               (spx_uint32_t)output_rate,
                               quality,
                               &err);
  if (state == NULL || err != RESAMPLER_ERR_SUCCESS) {
    if (state != NULL) {
      speex_resampler_destroy(state);
    }
    return NULL;
  }

  resampler = (media_resampler_t *)calloc(1, sizeof(media_resampler_t));
  if (resampler == NULL) {
    speex_resampler_destroy(state);
    return NULL;
  }

  resampler->state = state;
  resampler->channels = channels;
  resampler->input_rate = input_rate;
  resampler->output_rate = output_rate;
  resampler->quality = quality;
  resampler->options = options;

  return resampler;
}

void media_resampler_destroy(media_resampler_t *resampler) {
  if (resampler == NULL) {
    return;
  }

  if (resampler->state != NULL) {
    speex_resampler_destroy(resampler->state);
    resampler->state = NULL;
  }

  free(resampler);
}

int media_resampler_process(media_resampler_t *resampler,
                            const int16_t *input,
                            int input_samples_per_channel,
                            int16_t *output,
                            int output_capacity_samples_per_channel) {
  spx_uint32_t in_len;
  spx_uint32_t out_len;
  int err;

  if (resampler == NULL || resampler->state == NULL) {
    return MEDIA_CODEC_ERR_STATE;
  }
  if (input == NULL || output == NULL) {
    return MEDIA_CODEC_ERR_ARG;
  }
  if (input_samples_per_channel < 0 || output_capacity_samples_per_channel < 0) {
    return MEDIA_CODEC_ERR_ARG;
  }
  if (input_samples_per_channel == 0) {
    return 0;
  }

  in_len = (spx_uint32_t)input_samples_per_channel;
  out_len = (spx_uint32_t)output_capacity_samples_per_channel;

  err = speex_resampler_process_interleaved_int(resampler->state,
                                                (const spx_int16_t *)input,
                                                &in_len,
                                                (spx_int16_t *)output,
                                                &out_len);
  if (err != RESAMPLER_ERR_SUCCESS) {
    return MEDIA_CODEC_ERR_STATE;
  }

  return (int)out_len;
}

int media_resampler_reset(media_resampler_t *resampler) {
  int err;

  if (resampler == NULL || resampler->state == NULL) {
    return MEDIA_CODEC_ERR_STATE;
  }

  err = speex_resampler_reset_mem(resampler->state);
  if (err != RESAMPLER_ERR_SUCCESS) {
    return MEDIA_CODEC_ERR_STATE;
  }

  return 0;
}

int media_resampler_set_rate(media_resampler_t *resampler,
                             int input_rate,
                             int output_rate) {
  int err;

  if (resampler == NULL || resampler->state == NULL) {
    return MEDIA_CODEC_ERR_STATE;
  }
  if (input_rate <= 0 || output_rate <= 0) {
    return MEDIA_CODEC_ERR_ARG;
  }

  err = speex_resampler_set_rate(resampler->state,
                                 (spx_uint32_t)input_rate,
                                 (spx_uint32_t)output_rate);
  if (err != RESAMPLER_ERR_SUCCESS) {
    return MEDIA_CODEC_ERR_STATE;
  }

  resampler->input_rate = input_rate;
  resampler->output_rate = output_rate;
  return 0;
}

int media_resampler_get_expected_output_samples(media_resampler_t *resampler,
                                                int input_samples_per_channel) {
  uint64_t base;
  int latency;

  if (resampler == NULL || resampler->state == NULL) {
    return MEDIA_CODEC_ERR_STATE;
  }
  if (input_samples_per_channel < 0) {
    return MEDIA_CODEC_ERR_ARG;
  }
  if (input_samples_per_channel == 0) {
    return 0;
  }

  /*
   * 安全估算：
   * ceil(input * out_rate / in_rate) + output_latency + 16
   */
  base = ((uint64_t)input_samples_per_channel * (uint64_t)resampler->output_rate
          + (uint64_t)resampler->input_rate - 1ULL)
         / (uint64_t)resampler->input_rate;

  latency = speex_resampler_get_output_latency(resampler->state);
  if (latency < 0) {
    latency = 0;
  }

  base += (uint64_t)latency + 16ULL;

  if (base > 0x7fffffffULL) {
    return MEDIA_CODEC_ERR_ARG;
  }

  return (int)base;
}

int media_resampler_get_channels(const media_resampler_t *resampler) {
  if (resampler == NULL) {
    return 0;
  }
  return resampler->channels;
}