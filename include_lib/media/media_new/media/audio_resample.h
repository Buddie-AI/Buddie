/*****************************************************************
>file name : include_lib/media/media_develop/media/audio_resample.h
>author : lichao
>create time : Fri 07 Aug 2020 11:32:47 AM CST
*****************************************************************/
#ifndef _AUDIO_RESAMPLE_H_
#define _AUDIO_RESAMPLE_H_
#include "typedef.h"
#include "media/audio_stream.h"

#define AUDIO_HARDWARE_RESAMPLER    0
#define AUDIO_SOFTWARE_RESAMPLER    1

struct audio_resampler;

struct audio_resample_context {
    u8 channel;
    int in_sample_rate;
    int out_sample_rate;
    void *output_buff;
    void *remain_addr;
    u16 output_size;
    u16 remain_len;
    u16 process_len;
    const struct audio_resampler *resampler;
    void *priv;
    struct audio_stream_entry entry;
    int (*data_flush)(struct audio_resample_context *ctx, void *data, int len);
    void *output_priv;
    int (*output_handler)(void *priv, void *data, int len);
};


struct audio_resample_data;

struct audio_resampler {
    u8 type;
    int (*open)(struct audio_resample_context *ctx, u8 channel);
    int (*resample)(struct audio_resample_context *ctx,
                    struct audio_resample_data *in,
                    struct audio_resample_data *out);
    int (*write)(struct audio_resample_context *ctx, void *data, int len);
    int (*reset_sample_rate)(struct audio_resample_context *ctx, int in_sample_rate, int out_sample_rate);
    int (*stop)(struct audio_resample_context *ctx);
    void (*close)(struct audio_resample_context *ctx);
};

/*
 * 变采样输入输出数据
 */
struct audio_resample_data {
    void *buffer;       //变采样数据缓冲地址
    int sample_rate;    //采样率
    int sample_num;     //缓冲内样点个数
    int offset;         //已处理的样点偏移
    u8 ch_num;          //样点的通道个数
};

struct audio_resample_context *audio_resample_open(u8 ch_num,
        u8 hardware_first,
        void *output_priv,
        int (*output_handler)(void *priv, void *data, int len));

int audio_resample_set_sample_rate(struct audio_resample_context *ctx, int in_sample_rate, int out_sample_rate);

int audio_resample_set_sample_sync(struct audio_resample_context *ctx, int in_correct_rate, int out_correct_rate);

int audio_resample(struct audio_resample_context *ctx,
                   struct audio_resample_data *in,
                   struct audio_resample_data *out);

int audio_resample_stream_write(struct audio_resample_context *ctx, void *data, int len);

void audio_resample_close(struct audio_resample_context *ctx);

extern struct audio_resampler audio_resampler_begin[];
extern struct audio_resampler audio_resampler_end[];

#define list_for_each_resampler(p) \
    for (p = audio_resampler_begin; p < audio_resampler_end; p++)

#define REGISTER_AUDIO_RESAMPLER(resampler) \
    const struct audio_resampler resampler SEC(.audio_resampler) = {

#endif
