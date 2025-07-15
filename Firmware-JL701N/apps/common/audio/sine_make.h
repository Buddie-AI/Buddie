#ifndef __SINE_MAKE_H_
#define __SINE_MAKE_H_

#include "generic/typedef.h"

#define DEFAULT_SINE_SAMPLE_RATE 16000
#define SINE_TOTAL_VOLUME        26843546//16106128//20132660 //26843546

struct sin_param {
    //int idx_increment;
    int freq;
    int points;
    int win;
    int decay;
};

int sin_tone_make(void *_maker, void *data, int len);
void *sin_tone_open(const struct sin_param *param, int num, u8 channel, u8 repeat);
int sin_tone_points(void *_maker);
void sin_tone_close(void *_maker);

/*
*********************************************************************
* Description: 正弦信号生成
* Arguments  : fc               信号中心频率
*              fs               信号采样率
*              buf				数据输出地址
*			   len				数据生成长度，单位是bytes
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void sin_pcm_fill(int fc, int fs, void *buf, u32 len);

/*
*********************************************************************
* Description: 扫频信号生成
* Arguments  : fs               信号采样率
*              buf				数据输出地址
*			   len				数据生成长度，单位是bytes
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void sweepsin_pcm_fill(int fs, void *buf, u32 len);

#endif/*__SINE_MAKE_H_*/
