#ifndef _AUDIO_LIMITER_NOISE_GATE_H_
#define _AUDIO_LIMITER_NOISE_GATE_H_

#include "generic/typedef.h"


int audio_noise_gate_open(u16 sample_rate, int limiter_thr, int noise_gate, int noise_gain);
void audio_noise_gate_run(void *in, void *out, u16 len);
void audio_noise_gate_close();

#endif/*_AUDIO_NOISE_GATE_H_*/
