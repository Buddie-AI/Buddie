#ifndef _AUDIO_NOISE_GATE_H_
#define _AUDIO_NOISE_GATE_H_

#include "generic/typedef.h"


u8 *audio_noise_gate_open(int attack_time, int release_time, float thr, float gain, int sr, int ch);

void audio_noise_gate_run(u8 *runbuf, void *in, void *out, u16 len);

void audio_noise_gate_close(u8 *runbuf);

#endif/*_AUDIO_NOISE_GATE_H_*/
