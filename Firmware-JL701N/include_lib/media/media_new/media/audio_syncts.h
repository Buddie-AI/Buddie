/*****************************************************************
>file name : audio_syncts.h
>create time : Mon 22 Mar 2021 02:41:39 PM CST
>description:
*****************************************************************/

#ifndef _AUDIO_SYNCTS_H_
#define _AUDIO_SYNCTS_H_
#include "typedef.h"


#define PCM_INSIDE_DAC                      0
#define PCM_OUTSIDE_DAC                     1

#define AUDIO_NETWORK_LOCAL                 0
#define AUDIO_NETWORK_BT2_1                 1
#define AUDIO_NETWORK_BLE                   2
#define AUDIO_NETWORK_IPV4                  3

#define TIME_US_FACTOR      32
/*
 * Audio同步变采样参数
 */
struct audio_syncts_params {
    unsigned char network;      /*网络选择*/
    unsigned char pcm_device;   /*PCM设备选择*/
    unsigned char nch;          /*声道数*/
    unsigned char factor;       /*timestamp的整数放大因子*/
    int rin_sample_rate;        /*变采样输入采样率*/
    int rout_sample_rate;       /*变采样输出采样率*/
    void *priv;                 /*私有数据*/
    int (*output)(void *, void *, int);     /*变采样输出callback*/
};

int audio_syncts_open(void **syncts, struct audio_syncts_params *params);

int audio_syncts_next_pts(void *syncts, u32 timestamp);

int audio_syncts_frame_filter(void *syncts, void *data, int len);

void audio_syncts_close(void *syncts);

u32 audio_syncts_get_dts(void *syncts);

int audio_syncts_set_dts(void *syncts, u32 dts);

int audio_syncts_trigger_resume(void *syncts, void *priv, void (*resume)(void *priv));

int audio_syncts_update_sample_rate(void *syncts, int sample_rate);

void audio_syncts_resample_suspend(void *syncts);

void audio_syncts_resample_resume(void *syncts);

int sound_pcm_update_frame_num(void *syncts, int frames);

int sound_pcm_syncts_latch_trigger(void *syncts);

u32 sound_buffered_between_syncts_and_device(void *priv, u8 time_select);

void sound_pcm_enter_update_frame(void *syncts);
#endif
