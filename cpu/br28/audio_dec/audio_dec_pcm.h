#ifndef __AUDIO_DEC_PCM_H
#define __AUDIO_DEC_PCM_H

#include "asm/cpu.h"


void pcm_dec_resume(void);
void pcm_dec_set_read_cb(int (*pcm_read_func)(void *, void *, int));
int pcm_audio_play_open(int sr, u8 ch);
void pcm_audio_play_close(void);


extern int sound_pcm_dev_channel_mapping(int ch_num);

#endif

