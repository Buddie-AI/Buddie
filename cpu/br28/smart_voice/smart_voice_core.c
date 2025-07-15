/*****************************************************************
>file name : smart_voice_core.c
>author : lichao
>create time : Mon 01 Nov 2021 11:35:15 AM CST
*****************************************************************/
#include "smart_voice.h"
#include "vad_mic.h"
#include "includes.h"
#include "os/os_api.h"
/*#include "jl_kws_platform.h"*/
/*#include "aispeech_platform.h"*/
/*#include "user_platform.h"*/

/*****************************************************************
 * **算法平台接入说明**
 *
 * 1、TCFG_SMART_VOICE_ENABLE 为内置VAD+KWS关键词识别算法，默认在
 * 非通话的模式下使用低功耗VAD MIC作为语音数据MIC。
 *
 * 2、根据宏定义TCFG_AUDIO_ASR_DEVELOP 选择对应的算法开发平台,在开
 * 发框架上实现算法接入。
 *
 * 3、使用系统内置低功耗VAD作为语音数据MIC唤醒以及数据源需要实现
 * 以下三个步骤：
 *  1) 配置TCFG_VAD_LOWPOWER_CLOCK 时钟选择为 VAD_CLOCK_USE_PMU_STD12M
 *  2) 调用lp_vad_mic_data_init()初始化VAD MIC配置
 *  3) 调用voice_mic_data_open()时的数据源选择为VOICE_VAD_MIC
 *
 *****************************************************************/

extern const int config_jl_audio_kws_enable; /*KWS 使能*/
extern const int config_aispeech_asr_enable;
extern const int config_user_asr_enable;
extern const int config_lp_vad_enable;

extern int smart_voice_core_handler(void *priv, int taskq_type, int *msg);
extern int aispeech_asr_core_handler(void *priv, int taskq_type, int *msg);
extern int user_asr_core_handler(void *priv, int taskq_type, int *msg);
/*
 * 智能唤醒和语音识别处理任务
 */
static void smart_voice_core_task(void *arg)
{
    int msg[16];
    int res;
    u8 pend_taskq = 1;
    int err = 0;
    /*struct smart_voice_context *sv = (struct smart_voice_context *)arg;*/
    if (config_lp_vad_enable) {
        audio_vad_clock_trim();
    }

    while (1) {
        if (pend_taskq) {
            res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        } else {
            res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        }

        err = ASR_CORE_STANDBY;

        if (config_jl_audio_kws_enable) {
            err = smart_voice_core_handler(arg, res, &msg[1]);
        }

        if (config_aispeech_asr_enable) {
            err = aispeech_asr_core_handler(arg, res, &msg[1]);
        }

        if (config_user_asr_enable) {
            err = user_asr_core_handler(arg, res, &msg[1]);
        }

        pend_taskq = err ? 1 : 0;
    }
}

int smart_voice_core_create(void *priv)
{
    int err = task_create(smart_voice_core_task, priv, ASR_CORE);
    if (err != OS_NO_ERR) {
        return -EINVAL;
    }
    return 0;
}

int smart_voice_core_free(void)
{
    task_kill(ASR_CORE);
    return 0;
}
