#ifndef _SPATIAL_EFFECT_H_
#define _SPATIAL_EFFECT_H_
#include "system/includes.h"
#include "generic/typedef.h"
#include "app_config.h"
#include "tech_lib/effect_surTheta_api.h"

#define SPATIAL_AUDIO_EXPORT_DATA       0
#define SPATIAL_AUDIO_EXPORT_MODE       0

/*音效使能*/
#define SPATIAL_AUDIO_EFFECT_ENABLE     1

/*头部跟踪灵敏度：范围 0.001 ~ 1，1：表示即时跟踪，数值越小跟踪越慢*/
#define TRACK_SENSITIVITY     (1.0f)

/*静止角度复位灵敏度：范围 0.001 ~ 1，1：表示立刻复位回正，数值越小回正越慢*/
#define ANGLE_RESET_SENSITIVITY     (0.01f)

/*头部跟踪角度复位时间,单位：秒*/
#define ANGLE_RESET_TIME      (8)

#if SPATIAL_AUDIO_EXPORT_DATA
struct data_export_header {
    u8 magic;
    u8 ch;
    u16 seqn;
    u16 crc;
    u16 len;
    u32 timestamp;
    u32 total_len;
    u8 data[0];
};
void audio_export_task(void *arg);
#endif

struct spatial_audio_context {
    void *sensor;
    void *effect;
    void *calculator;
#if TCFG_USER_TWS_ENABLE
    void *tws_conn;
    int tws_angle;
#endif
    u8 mapping_channel;
    u32 head_tracked;
#if SPATIAL_AUDIO_EXPORT_DATA
    cbuffer_t audio_cbuf;
    cbuffer_t space_cbuf;
    u8 *audio_buf;
    u8 *space_buf;
    void *space_fifo_buf;
    u16 audio_seqn;
    u16 audio_out_seqn;
    u16 space_seqn;
    u32 audio_data_len;
    u32 space_data_len;
    u32 timestamp;
    int space_data_single_len;
    u8 export;
#if SPATIAL_AUDIO_EXPORT_MODE == 0
    void *audio_file;
    void *space_file;
#endif
#endif
    u8 data[0];
};

/*设置并更新混响参数*/
void spatial_effect_online_updata(RP_PARM_CONIFG *params);

/*获取当前混响的参数*/
void get_spatial_effect_reverb_params(RP_PARM_CONIFG *params);

/*打开空间音效*/
void *spatial_audio_open(void);

/*关闭空间音效*/
void spatial_audio_close(void *sa);

/*音效处理*/
int spatial_audio_filter(void *sa, void *data, int len);

/*设置空间音频的输出声道映射*/
int spatial_audio_set_mapping_channel(void *sa, u8 channel);

/*读取传感器的数据*/
int spatial_audio_space_data_read(void *data);

/*头部跟踪使能*/
void spatial_audio_head_tracked_en(struct spatial_audio_context *ctx, u8 en);

/*获取头部跟踪使能状态*/
u8 get_spatial_audio_head_tracked(struct spatial_audio_context *ctx);

#endif
