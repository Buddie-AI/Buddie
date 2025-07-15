
#ifndef AUDIO_ANC_MULT_SCENE_H
#define AUDIO_ANC_MULT_SCENE_H

#include "generic/typedef.h"
#include "asm/anc.h"

typedef enum {
    ANC_MULT_COEFF_FILL_ALL = 0,	//滤波器全部填充
    ANC_MULT_COEFF_FILL_PART,		//滤波器部分填充
} ANC_coeff_fill_t;

int audio_anc_mult_coeff_file_read(void);

void anc_mult_init(audio_anc_t *param);

int anc_mult_scene_set(u16 scene_id);

int audio_anc_mult_coeff_write(ANC_coeff_fill_t type, int *coeff, u16 len);

void audio_anc_mult_scene_coeff_free(void);

void audio_anc_mult_gains_id_set(u8 gain_id, int data);

int audio_anc_mult_scene_id_check(u16 scene_id);

#endif/*AUDIO_ANC_MULT_SCENE_H*/
