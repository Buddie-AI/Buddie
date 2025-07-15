/*****************************************************************
>file name : sound_device.h
>create time : Sat 25 Feb 2022 02:25:50 PM CST
*****************************************************************/
#ifndef _SOUND_PCM_DEVICE_H_
#define _SOUND_PCM_DEVICE_H_
#include "media/sound/sound.h"

int sound_pcm_driver_init(void);

int sound_pcm_sync_device_select(void);

void sound_pcm_dev_try_power_on(void);

void sound_pcm_dev_set_analog_gain(s16 gain);

void sound_pcm_set_underrun_params(int time_ms, void *priv, void (*callback)(void *));

int sound_pcm_dev_buffered_time(void);

int sound_pcm_dev_buffered_frames(void);

int sound_pcm_dev_channel_mapping(int ch_num);

int sound_pcm_dev_write(void *private_data, void *data, int len);

int sound_pcm_dev_start(void *private_data, int sample_rate, s16 volume);

int sound_pcm_dev_stop(void *private_data);

int sound_pcm_match_sample_rate(int sample_rate);

int sound_pcm_dev_set_delay_time(int prepared_time, int delay_time);

int sound_pcm_trigger_resume(int time_ms, void *priv, void (*callback)(void *priv));

void sound_pcm_dev_channel_mute(u8 ch, u8 mute);

int sound_pcm_dev_set_delay_time(int prepared_time, int delay_time);

int sound_pcm_dev_is_running(void);

void sound_pcm_dev_add_syncts(void *priv);

void sound_pcm_dev_remove_syncts(void *handle);

void *get_iis_alink_param(void);
#endif


