#ifndef HOWLING_API_H
#define HOWLING_API_H

#include "media/howling_pitchshifter_api.h"
// #include "shiftPhase_api.h"
#include "media/pemafrow_api.h"
#include "media/audio_stream.h"

typedef struct s_howling_para {
    float notch_Q;//陷波器 Q值
    float notch_gain;//陷波器gain值
    int fade_time; //fade time
    float threshold;  //初始化阈值
    // int depth;  //陷波器深度
    // int bandwidth;//陷波器带宽
    int sample_rate;
    int channel;
    // float mode;//陷波器是否降采样
} HOWLING_PARM_SET;

//啸叫抑制 NotchHowling:
typedef struct _NotchHowlingParam {
    float Q;    	//Q值
    float gain;		//增益
    int fade_n;		//启动释放时间
    float threshold;
} NotchHowlingParam;



typedef struct _HOWLING_API_STRUCT_ {
    HOWLING_PARM_SET 	parm;  //陷波参数
    void				*ptr;    //运算buf指针
    HOWLING_PITCHSHIFT_PARM 	parm_2;  //移频参数
    HOWLING_PITCHSHIFT_FUNC_API *func_api;           //移频函数指针
    struct audio_stream_entry entry;	// 音频流入口
    s16 *pre_buf;
    u32 sample_rate;
    u8 run_en;
    u8 mode;
    u8 update;
} HOWLING_API_STRUCT;

// int get_howling_buf(void);
// void howling_init(void *workbuf, int threshold, int depth, int bandwidth, int attackTime, int releaseTime, int Noise_threshold, int low_th_gain, int sampleRate, int channel);
// int howling_run(void *workbuf, short *in, short *out, int len);
int get_howling_buf(int sampleRate);
// void howling_init(void *workbuf, int fade_n,float notch_gain,float notch_Q, int mode,  int sampleRate);
// void howling_init(void *workbuf, int fade_time, float notch_gain, float notch_Q,  int sampleRate);
void howling_init(void *workbuf, int fade_time, float notch_gain, float notch_Q, float threshold, int sampleRate);
// void howling_run(void *workbuf, short *in, short *out, int len);
int howling_run(void *workbuf, short *in, short *out, int len);

void SetHowlingDetection(void *workbuf, int OnlyDetection);
float *getHowlingFreq(void *workbuf, int *num);

#endif
