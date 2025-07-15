/*****************************************************************
>file name : audio_codec_clock.h
>create time : Thu 03 Jun 2021 09:36:25 AM CST
*****************************************************************/
#ifndef _AUDIO_CODEC_CLOCK_H_
#define _AUDIO_CODEC_CLOCK_H_
#include "audio_base.h"

enum {
    AUDIO_A2DP_MODE = 0,
    AUDIO_ESCO_MODE,
    AUDIO_TONE_MODE,
    AUDIO_FILE_MODE,
    AUDIO_PC_MODE,
    AUDIO_USB_MIC_MODE,
    SMART_VOICE_MODE,
    SPATIAL_EFFECT_MODE,
    SPEAK_TO_CHAT_MODE,
    ANC_EAR_ADAPTIVE_MODE,
    ANC_DUT_SZ_FFT_MODE,
    AUDIO_MAX_MODE,
};


/*************************************************************************
 * 音频编解码时钟设置
 *
 * Input    :  mode - 音频模式，coding_type - 模式下的解码格式
 *             preemption - 打断/叠加.
 * Output   :  0 - 成功, 非0 - 出错.
 * Notes    :  目前在耳机SDK用于A2DP/ESCO/TONE三个模式的时钟切换.
 * History  :  2021/06/03 初始版本
 *=======================================================================*/
int audio_codec_clock_set(u8 mode, u32 coding_type, u8 preemption);

/*************************************************************************
 * 音频编解码时钟删除
 *
 * Input    :  mode - 音频模式
 * Output   :  无.
 * Notes    :  需和audio_codec_clock_set结对使用.
 * History  :  2021/06/03 初始版本
 *=======================================================================*/
void audio_codec_clock_del(u8 mode);

/*************************************************************************
 * 音频编解码时钟检查
 *
 * Input    :  无
 * Output   :  无.
 * Notes    :  检查音频使用中是否有时钟被修改的情况并报错
 * History  :  2021/11/18
 *=======================================================================*/
void audio_codec_clock_check(void);
#endif
