/*****************************************************************
>file name : a2dp_sample_detect.h
>create time : Mon 23 Jul 2021 11:26:34 AM CST
*****************************************************************/
#ifndef _A2DP_SAMPLE_DETECT_H_
#define _A2DP_SAMPLE_DETECT_H_
#include "typedef.h"
#include "audio_base.h"

void *a2dp_sample_detect_open(int sample_rate, u32 coding_type);

int a2dp_frame_sample_detect_start(void *hdl, u32 time);

int a2dp_frame_sample_detect(void *hdl, void *data, int len, u32 time);

void a2dp_sample_detect_close(void *hdl);
#endif
