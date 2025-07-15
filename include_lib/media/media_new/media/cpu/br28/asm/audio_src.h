/*****************************************************************
>file name : audio_src.h
>author : lichao
>create time : Fri 14 Dec 2018 03:05:49 PM CST
*****************************************************************/
#ifndef _AUDIO_SRC_H_
#define _AUDIO_SRC_H_
#include "media/audio_stream.h"
#include "audio_resample.h"


#define SRC_DATA_MIX    0//各通道数据交叉存放
#define SRC_DATA_SEP    1//各通道数据连续存放

#define SRC_CHI                     2
#define SRC_FILT_POINTS             24

#define SRC_TYPE_NONE               0
#define SRC_TYPE_RESAMPLE           1
#define SRC_TYPE_AUDIO_SYNC         2

#define AUDIO_ONLY_RESAMPLE         1
#define AUDIO_SYNC_RESAMPLE         2

#define AUDIO_SAMPLE_FMT_16BIT      0
#define AUDIO_SAMPLE_FMT_24BIT      1

#define BIND_AUDSYNC                0x10
#define SET_RESAMPLE_TYPE(fmt, type)    (((fmt) << 4) | (type))
#define RESAMPLE_TYPE_TO_FMT(a)         (((a) >> 4) & 0xf)
#define RESAMPLE_TYPE(a)                ((a) & 0xf)

#define INPUT_FRAME_BITS                            18//20 -- 整数位减少可提高单精度浮点的运算精度
#define RESAMPLE_INPUT_BIT_RANGE                    ((1 << INPUT_FRAME_BITS) - 1)
#define RESAMPLE_INPUT_BIT_NUM                      (1 << INPUT_FRAME_BITS)

#define RESAMPLE_HW_24BIT  1


// *INDENT-OFF*
struct audio_hw_resample_context {
    void *base;
};
// *INDENT-ON*


void *audio_resample_hw_open(u8 channel, int in_rate, int out_rate, u8 type);

int audio_resample_hw_change_config(void *resample, int in_rate, int out_rate);

int audio_resample_hw_set_input_buffer(void *resample, void *addr, int len);

int audio_resample_hw_set_output_buffer(void *resample, void *addr, int len);

int audio_resample_hw_set_output_handler(void *resample, void *, int (*handler)(void *, void *, int));

int audio_resample_hw_write(void *resample, void *data, int len);

int audio_resample_hw_trigger_interrupt(void *resample, void *priv, void (*handler)(void *));

int audio_resample_hw_stop(void *resample);

void audio_resample_hw_close(void *resample);

int audio_resample_hw_push_data_out(void *resample);

int audio_resample_hw_open_v1(struct audio_hw_resample_context *src, u8 channel, u8 type);

void audio_resample_hw_close_v1(struct audio_hw_resample_context *ctx);

#define audio_src_handle                        audio_hw_resample_context
#define audio_hw_src_open                       audio_resample_hw_open_v1
#define audio_hw_src_set_rate                   audio_resample_hw_change_config
#define audio_src_resample_write                audio_resample_hw_write
#define audio_src_set_output_handler            audio_resample_hw_set_output_handler
#define audio_src_set_rise_irq_handler          audio_resample_hw_trigger_interrupt
#define audio_hw_src_set_input_buffer           audio_resample_hw_set_input_buffer
#define audio_hw_src_set_output_buffer          audio_resample_hw_set_output_buffer
#define audio_hw_src_stop                       audio_resample_hw_stop
#define audio_hw_src_close                      audio_resample_hw_close_v1
#define audio_hw_src_trigger_resume             audio_resample_hw_trigger_interrupt
#define audio_hw_src_set_check_running(a, b)    (0)
#define audio_hw_src_active(x)                  (0)


#endif
