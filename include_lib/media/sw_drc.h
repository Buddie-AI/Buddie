#ifndef CONFIG_EFFECT_CORE_V2_ENABLE
#include "media/media_develop/media/sw_drc.h"
#else

#ifndef __SW_DRC_H
#define __SW_DRC_H

#include "generic/typedef.h"
#include "generic/list.h"
#include "os/os_api.h"
#include "drc_api.h"


//分频器 crossover:
typedef struct _CrossOverParam_TOOL_SET {
    int way_num;        //段数
    int N;				//阶数
    int low_freq;	    //低中分频点
    int high_freq;      //高中分频点
} CrossOverParam_TOOL_SET;


struct threshold_group {
    float in_threshold;
    float out_threshold;
};

struct wdrc_struct {
    u16 attacktime;   //启动时间
    u16 releasetime;  //释放时间
    struct threshold_group threshold[6];
    float inputgain;	   //wdrc输入数据放大或者缩小的db数（正数放大，负数缩小）, 例如配2表示 总体增加2dB
    float outputgain;	   //wdrc输出数据放大或者缩小的db数（正数放大，负数缩小）
    u8 threshold_num;
    u8 rms_time;
    u8 algorithm;//0:PEAK  1:RMS
    u8 mode;//0:PERPOINT  1:TWOPOINT
    u16 holdtime; //预留位置
    u8 reverved[2];//预留位置
};

//动态范围控制 DRC:
typedef struct _wdrc_struct_TOOL_SET {
    int is_bypass;          // 1-> byass 0 -> no bypass
    struct wdrc_struct parm;
} wdrc_struct_TOOL_SET;



/* //对耳wdrc处理，区分左右声道 */
// #define L_wdrc   0x10
// #define LL_wdrc  0x20
// #define R_wdrc   0x40
/* #define RR_wdrc  0x80 */

struct drc_ch {
    CrossOverParam_TOOL_SET crossover;
    union {
        struct wdrc_struct      wdrc[4];//[0]low [1]mid [2]high  [3]多带之后，再做一次全带
    } _p;
};


struct sw_drc {
    void *work_buf[4];    //drc内部驱动句柄
    void *crossoverBuf;   //分频器句柄
    int *run_tmp[3];      //多带限幅器或压缩器，运行buf
    int run_tmp_len;      //多带限幅器或压缩器，运行buf 长度
    u8 nband;             //max<=3，1：全带 2：两段 3：三段
    u8 channel;           //通道数
    u8 run32bit;          //drc处理输入输出数据位宽是否是32bit 1:32bit  0:16bit
    struct audio_eq *crossover[3];//多带分频器eq句柄
    u32 sample_rate;      //采样率
    u8 nsection;          //分频器eq段数
    u8 other_band_en;     //多带限幅器之后，是否需要再做一次全带的限幅器  1：需要，0：不需要
    u8 bypass[4];
};
extern void *get_low_sosmatrix();
extern void *get_high_sosmatrix();
extern void *get_band_sosmatrix();
extern int get_crossover_nsection();


void *audio_sw_drc_open(void *crossover, void *wdrc, u32 sample_rate, u8 channel, u8 run32bit, u8 other_band_en);
extern void audio_sw_drc_close(void *hdl);
int audio_sw_drc_update(void *hdl, void *crossover, void *wdrc, u32 sample_rate, u8 channel);
extern int audio_sw_drc_run(void *hdl, s16 *in_buf, s16 *out_buf, int npoint_per_channel);
void sw_drc_set_bypass(struct sw_drc *drc, u8 tar, u8 bypass);

void audio_hw_crossover_open(struct sw_drc *drc, int (*L_coeff)[3], u8 nsection);
void audio_hw_crossover_close(struct sw_drc *drc);
void audio_hw_crossover_run(struct sw_drc *drc, s16 *data, int len);
void audio_hw_crossover_update(struct sw_drc *drc, int (*L_coeff)[3], u8 nsection);

void  band_merging_32bit(int *in0, int *in1, int *in2, s32 *out_buf_tmp, int points, u8 nband);
void  band_merging_16bit(short *in0, short *in1, short *in2, short *out_buf_tmp, int points, u8 nband);
#endif
#endif /*CONFIG_EFFECT_CORE_V2_ENABLE*/
