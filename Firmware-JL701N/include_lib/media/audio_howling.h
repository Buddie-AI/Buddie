#ifndef CONFIG_EFFECT_CORE_V2_ENABLE
#include "application/audio_howling.h"
#else

#ifndef _AUDIO_HOWLING_API_H
#define _AUDIO_HOWLING_API_H

#include "media/howling_api.h"


#define TRAP_MODE                       0
#define SHIFT_PITCH_MODE                1
#define PEMAFROW_MODE                   2
#define SHIFT_PHASE_MODE                3

#define HOWLING_MODE                   SHIFT_PITCH_MODE

#if (HOWLING_MODE == TRAP_MODE)
#define REVERB_RUN_POINT_NUM		160	//固定160个点
#elif (HOWLING_MODE == SHIFT_PHASE_MODE) //
#define REVERB_RUN_POINT_NUM		256	//固定256个点
#elif (HOWLING_MODE == PEMAFROW_MODE)
#define REVERB_RUN_POINT_NUM		128	//固定128
#else
#define REVERB_RUN_POINT_NUM		0	//无限制
#endif


/*----------------------------------------------------------------------------*/
/**@brief    啸叫抑制模块打开
   @param    howl_para:陷波模式时为啸叫抑制参数 ,pemafrow模式为DAC输出数据buf，；
   			 sample_rate:输入数据采样率；
			 channels: 输入数据通道数；
			 mode: 0 陷波模式；1 移频模式 2 pemafrow模式
   @return   啸叫抑制句柄，
   @note     陷波模式 要外部保证每次运算数据为160点，pemafrow 为128个点
   			对应模式需外部使能,默认只开 移频模式
   			const int config_howling_enable_pemafrow_mode = 1;
			const int config_howling_enable_trap_mode     = 1;
			const int config_howling_enable_pitchps_mode  = 1;
*/
/*----------------------------------------------------------------------------*/
HOWLING_API_STRUCT *open_howling(void *howl_para, u16 sample_rate, u8 channel, u8 mode);
/*
 * 啸叫抑制模块数据处理
 */
void run_howling(HOWLING_API_STRUCT *howling_hdl, short *in, short *out, int len);
/*
 * 啸叫抑制模块关闭
 */
void close_howling(HOWLING_API_STRUCT *holing_hdl);
/*
 * 啸叫抑制模块暂停处理
 */
void pause_howling(HOWLING_API_STRUCT *holing_hdl, u8 run_mark);

void howling_update_bypass(HOWLING_API_STRUCT *hdl, u8 bypass);

void update_howling_parm(HOWLING_API_STRUCT *howling_hdl, void *parm);
#endif

#endif /*CONFIG_EFFECT_CORE_V2_ENABLE*/
