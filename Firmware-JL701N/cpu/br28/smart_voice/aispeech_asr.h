/*****************************************************************
>file name : aispeech_asr.h
>create time : Thu 23 Dec 2021 11:53:40 AM CST
*****************************************************************/
#ifndef _AISPEECH_ASR_H_
#define _AISPEECH_ASR_H_
#include "media/includes.h"

int ais_platform_asr_open(void);

void ais_platform_asr_close(void);

int aispeech_asr_core_handler(void *priv, int taskq_type, int *msg);

int audio_ais_platform_asr_init(struct vad_mic_platform_data *mic_data);

int audio_phone_call_aispeech_asr_start(void);

int audio_phone_call_aispeech_asr_close(void);
#endif

