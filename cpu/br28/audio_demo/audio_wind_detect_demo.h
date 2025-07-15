#ifndef __AUDIO_WIND_DETECT_DEMO_H_
#define __AUDIO_WIND_DETECT_DEMO_H_
#include "generic/typedef.h"

/*打开风噪检测的demo*/
int audio_wind_detect_demo_open(void);

/*关闭风噪检测的demo*/
int audio_wind_detect_demo_close(void);

/*打开风噪检测*/
int audio_wind_detect_open(u32 sr, u16 mic0_gain, u16 mic1_gain, u16 mic2_gain, u16 mic3_gain);

/*关闭风噪检测*/
int audio_wind_detect_close();

/*打开获取风噪信息的任务*/
int audio_wind_info_task_open();

/*获取风噪信息*/
int audio_get_wind_detect_info(void);

/*关闭获取风噪信息的任务*/
int audio_wind_info_task_close(void);

/*获取是否有风，0：无风，1：有风*/
int audio_get_wd_flag(void);

/*获取风强*/
int audio_get_wd_val(void);

/*获取预设的风噪等级, 0：弱风，1：中风：2：强风*/
int audio_get_wd_lev(void);

#endif /* __AUDIO_WIND_DETECT_DEMO_H_*/
