
#ifndef _AUDIO_ANC_FADE_CTR_H_
#define _AUDIO_ANC_FADE_CTR_H_

#include "generic/typedef.h"

//ANC全部通道
#define AUDIO_ANC_FDAE_CH_ALL  AUDIO_ANC_FADE_CH_LFF | AUDIO_ANC_FADE_CH_LFB | AUDIO_ANC_FADE_CH_RFF | AUDIO_ANC_FADE_CH_RFB

enum anc_fade_mode_t {
    ANC_FADE_MODE_RESET = 0,		//复位
    ANC_FADE_MODE_SWITCH,			//ANC模式切换
    ANC_FADE_MODE_MUSIC_DYNAMIC,	//音乐动态增益
    ANC_FADE_MODE_SCENE_ADAPTIVE,	//ANC场景噪声自适应
    ANC_FADE_MODE_WIND_NOISE,		//ANC风噪检测
    ANC_FADE_MODE_SUSPEND,			//ANC挂起
    //可再此继续添加模式
};


/*
   ANC淡入淡出增益设置
	param:  mode 		场景模式
			ch  		设置目标通道(支持多通道)
			gain 设置增益
	notes: ch 支持配置多个通道，但mode 必须与 ch配置一一对应;
			当设置gain = 16384, 会自动删除对应模式
 */
void audio_anc_fade_ctr_set(enum anc_fade_mode_t mode, u8 ch, u16 gain);

//删除fade mode
void audio_anc_fade_ctr_del(enum anc_fade_mode_t mode);

//fade ctr 初始化
void audio_anc_fade_ctr_init(void);


#endif/*_AUDIO_ANC_FADE_CTR_H_*/

