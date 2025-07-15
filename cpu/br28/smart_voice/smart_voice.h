/*****************************************************************
>file name : smart_voice.h
>create time : Thu 17 Jun 2021 02:07:32 PM CST
*****************************************************************/
#ifndef _SMART_VOICE_H_
#define _SMART_VOICE_H_
#include "media/includes.h"

#define SMART_VOICE_MSG_WAKE              0 /*唤醒*/
#define SMART_VOICE_MSG_STANDBY           1 /*待机*/
#define SMART_VOICE_MSG_DMA               2 /*语音数据DMA传输*/
#define SMART_VOICE_MSG_MIC_OPEN          3 /*MIC打开*/
#define SMART_VOICE_MSG_SWITCH_SOURCE     4 /*MIC切换*/
#define SMART_VOICE_MSG_MIC_CLOSE         5 /*MIC关闭*/

#define ASR_CORE            "audio_vad"

#define ASR_CORE_WAKEUP     0
#define ASR_CORE_STANDBY    1
/*
*********************************************************************
*       audio smart voice detect create
* Description: 创建智能语音检测引擎
* Arguments  : model         - KWS模型
*			   task_name     - VAD引擎task
*			   mic           - MIC的选择（低功耗MIC或主控MIC）
*			   buffer_size   - MIC的DMA数据缓冲大小
* Return	 : 0 - 创建成功，非0 - 失败.
* Note(s)    : None.
*********************************************************************
*/
int audio_smart_voice_detect_create(u8 model, u8 mic, int buffer_size);

/*
*********************************************************************
*       audio smart voice detect open
* Description: 智能语音检测打开接口
* Arguments  : mic           - MIC的选择（低功耗MIC或主控MIC）
* Return	 : void.
* Note(s)    : None.
*********************************************************************
*/
void audio_smart_voice_detect_open(u8 mic);

/*
*********************************************************************
*       audio smart voice detect close
* Description: 智能语音检测关闭接口
* Arguments  : void.
* Return	 : void.
* Note(s)    : 关闭引擎的所有资源（无论使用哪个mic）.
*********************************************************************
*/
void audio_smart_voice_detect_close(void);

/*
*********************************************************************
*       audio smart voice detect init
* Description: 智能语音检测配置初始化
* Arguments  : mic_data - P11 mic初始化配置.
* Return	 : void.
* Note(s)    : None.
*********************************************************************
*/
int audio_smart_voice_detect_init(struct vad_mic_platform_data *mic_data);

/*
*********************************************************************
*       audio phone call kws start
* Description: 启动通话来电关键词识别
* Arguments  : void.
* Return	 : 0 - 成功，非0 - 失败.
* Note(s)    : 接口会将VAD的低功耗mic切换至通话使用的主控mic.
*********************************************************************
*/
int audio_phone_call_kws_start(void);

/*
*********************************************************************
*       audio phone call kws close
* Description: 关闭通话来电关键词识别
* Arguments  : void.
* Return	 : 0 - 成功，非0 - 出错.
* Note(s)    : 关闭来电关键词识别，通常用于接通后或挂断/拒接后.
*********************************************************************
*/
int audio_phone_call_kws_close(void);


#define smart_voice_core_post_msg(num, ...)     os_taskq_post_msg(ASR_CORE, num, __VA_ARGS__)

int smart_voice_core_create(void *priv);

int smart_voice_core_free(void);
#endif
