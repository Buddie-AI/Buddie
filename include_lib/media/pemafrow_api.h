#ifndef PEMAFROW_API_H
#define PEMAFROW_API_H

#include "media/audio_stream.h"

#define PEMAFROW_RUN_POINT_NUM		128	//每次run固定点数
int getPemafrowBuf();
void pemafrowInit(void *workBuf);
//int pemafrowRun(void *workBuf, short *in, short *out, int len);
int pemafrowRun(void *workBuf, short *in, short *preOut, short *out);

typedef struct _PEMAFROW_API_STRUCT_ {
    void				*ptr;    //运算buf指针
    s16 pre_data[PEMAFROW_RUN_POINT_NUM];
    s16 data_buf[PEMAFROW_RUN_POINT_NUM];
    struct audio_stream_entry entry;	// 音频流入口
    int out_len;
    int process_len;
    u16 data_len;
    u8 run_en;
} PEMAFROW_API_STRUCT;

#endif
