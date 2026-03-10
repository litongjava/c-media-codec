#ifndef MEDIA_RESAMPLE_H
#define MEDIA_RESAMPLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct media_resampler_s media_resampler_t;

/**
 * 创建重采样器
 *
 * channels: 声道数，当前建议 1
 * input_rate: 输入采样率，例如 24000
 * output_rate: 输出采样率，例如 8000 / 16000
 * quality: SpeexDSP quality，推荐 5~6
 * options: 预留参数，当前传 0
 *
 * 返回：
 *   非 NULL: 成功
 *   NULL: 失败
 */
media_resampler_t *media_resampler_create(int channels,
                                          int input_rate,
                                          int output_rate,
                                          int quality,
                                          int options);

/**
 * 销毁重采样器
 */
void media_resampler_destroy(media_resampler_t *resampler);

/**
 * 流式重采样
 *
 * 输入/输出都是 16-bit signed PCM little-endian 对应的 int16_t 数据。
 *
 * input_samples_per_channel:
 *   输入每声道 sample 数
 *
 * output_capacity_samples_per_channel:
 *   输出 buffer 可容纳的每声道 sample 数
 *
 * 返回：
 *   >= 0: 实际输出每声道 sample 数
 *   < 0 : 错误码
 */
int media_resampler_process(media_resampler_t *resampler,
                            const int16_t *input,
                            int input_samples_per_channel,
                            int16_t *output,
                            int output_capacity_samples_per_channel);

/**
 * 重置内部状态
 *
 * 返回：
 *   0: 成功
 *   < 0: 错误
 */
int media_resampler_reset(media_resampler_t *resampler);

/**
 * 更新输入/输出采样率
 *
 * 返回：
 *   0: 成功
 *   < 0: 错误
 */
int media_resampler_set_rate(media_resampler_t *resampler,
                             int input_rate,
                             int output_rate);

/**
 * 给定输入每声道 sample 数，估算安全输出容量（每声道）
 *
 * 返回：
 *   >= 0: 建议输出每声道 sample 数
 *   < 0 : 错误
 */
int media_resampler_get_expected_output_samples(media_resampler_t *resampler,
                                                int input_samples_per_channel);

/**
 * 获取声道数
 */
int media_resampler_get_channels(const media_resampler_t *resampler);

#ifdef __cplusplus
}
#endif

#endif