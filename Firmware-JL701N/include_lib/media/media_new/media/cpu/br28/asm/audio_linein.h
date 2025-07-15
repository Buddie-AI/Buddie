#ifndef _AUDIO_LINEIN_H_
#define _AUDIO_LINEIN_H_

#include "generic/typedef.h"
#include "asm/audio_adc.h"

/*
 *ch:
 *amux_en:
 */
int audio_linein0_open(u8 ch, u8 amux_en);
int audio_linein0_close(u8 ch, u8 amux_en);
int audio_linein1_open(u8 ch, u8 amux_en);
int audio_linein1_close(u8 ch, u8 amux_en);
int audio_linein2_open(u8 ch, u8 amux_en);
int audio_linein2_close(u8 ch, u8 amux_en);
/*
 *
 */
void audio_linein_mute(u8 mute);
/*
 *
 */
void audio_linein_gain(u8 gain);

void audio_linein_bias(u8 ch, u8 en);
void audio_linein_amux_bias(u8 amux_bias);
/*
 *
 *
 */
void audio_linein_ch_combine(u8 LR_2_L, u8 LR_2_R);

#endif
