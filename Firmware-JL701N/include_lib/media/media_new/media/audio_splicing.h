
#ifndef _AUDIO_SPLICING_H_
#define _AUDIO_SPLICING_H_

#include "generic/typedef.h"

void pcm_single_to_dual(void *out, void *in, u16 len);
void pcm_single_to_qual(void *out, void *in, u16 len);
void pcm_dual_to_qual(void *out, void *in, u16 len);
void pcm_dual_mix_to_dual(void *out, void *in, u16 len);
void pcm_dual_to_single(void *out, void *in, u16 len);
void pcm_qual_to_single(void *out, void *in, u16 len);
void pcm_single_l_r_2_dual(void *out, void *in_l, void *in_r, u16 in_len);
void pcm_fl_fr_rl_rr_2_qual(void *out, void *in_fl, void *in_fr, void *in_rl, void *in_rr, u16 in_len);
void pcm_flfr_rlrr_2_qual(void *out, void *in_flfr, void *in_rlrr, u16 in_len);
void pcm_fill_single_2_qual(void *out, void *in, u16 in_len, u8 idx);
void pcm_fill_flfr_2_qual(void *out, void *in_flfr, u16 in_len);
void pcm_fill_rlrr_2_qual(void *out, void *in_rlrr, u16 in_len);
void pcm_mix_buf(s32 *obuf, s16 *ibuf, u16 len);
void pcm_mix_buf_limit(s16 *obuf, s32 *ibuf, u16 len);
// 四声道转双声道（q0+q2,q1+q3）
void pcm_qual_to_dual(void *out, void *in, u16 len);

#endif/*_AUDIO_SPLICING_H_*/
