/*****************************************************************
>file name : user_asr.c
>create time : Thu 23 Dec 2021 10:00:58 AM CST
>用户自定义语音识别算法平台接入
*****************************************************************/
#include "system/includes.h"
#include "user_asr.h"
#include "smart_voice.h"
#include "voice_mic_data.h"
#define ASR_FRAME_SAMPLES   160 /*语音识别帧长（采样点）*/

struct user_platform_asr_context {
    void *mic;
    void *core;
    u8 data_enable;
};

extern const int config_user_asr_enable;
static struct user_platform_asr_context *__this = NULL;
/*
 * 算法引擎打开函数
 */
static void *user_asr_core_open(int sample_rate)
{
    return NULL;
}

/*
 * 算法引擎关闭函数
 */
static void user_asr_core_close(void *core)
{

}

/*
 * 算法引擎数据处理
 */
static int user_asr_core_data_handler(void *core, void *data, int len)
{
    return 0;
}

/*
*********************************************************************
*       user asr data handler
* Description: 用户算法平台平台语音识别数据处理
* Arguments  : asr - 语音识别数据管理结构
* Return	 : 0 - 处理成功， 非0 - 处理失败.
* Note(s)    : 该函数通过读取mic数据送入算法引擎完成语音帧.
*********************************************************************
*/
static int user_asr_data_handler(struct user_platform_asr_context *asr)
{
    int result = 0;

    if (!asr->mic) {
        return -EINVAL;
    }

    s16 data[ASR_FRAME_SAMPLES];

    int len = voice_mic_data_read(asr->mic, data, sizeof(data));
    if (len < sizeof(data)) {
        return -EINVAL;
    }

    if (asr->core) {
        result = user_asr_core_data_handler(asr->core, data, sizeof(data));
    }

    return 0;
}

/*
*********************************************************************
*       user_asr core handler
* Description: 用户自定义语音识别处理
* Arguments  : priv - 语音识别私有数据
*              taskq_type - TASK消息类型
*              msg  - 消息存储指针(对应自身模块post的消息)
* Return	 : None.
* Note(s)    : 音频平台资源控制以及ASR主要识别算法引擎.
*********************************************************************
*/
int user_asr_core_handler(void *priv, int taskq_type, int *msg)
{
    struct user_platform_asr_context *asr = (struct user_platform_asr_context *)priv;
    int err = ASR_CORE_STANDBY;

    if (taskq_type != OS_TASKQ) {
        return err;
    }

    switch (msg[0]) {
    case SMART_VOICE_MSG_MIC_OPEN: /*语音识别打开 - MIC打开，算法引擎打开*/
        /* msg[1] - MIC数据源，msg[2] - 音频采样率，msg[3] - mic的数据总缓冲长度*/
        asr->mic = voice_mic_data_open(msg[1], msg[2], msg[3]);
        asr->core = user_asr_core_open(msg[2]);
        asr->data_enable = 1;
        err = ASR_CORE_WAKEUP;
        break;
    case SMART_VOICE_MSG_SWITCH_SOURCE:
        /*这里进行MIC的数据源切换 （主系统MIC或VAD MIC）*/

        break;
    case SMART_VOICE_MSG_MIC_CLOSE: /*语音识别关闭 - MIC关闭，算法引擎关闭*/
        /* msg[2] - 信号量*/
        voice_mic_data_close(asr->mic);
        asr->mic = NULL;
        user_asr_core_close(asr->core);
        asr->core = NULL;
        asr->data_enable = 0;
        os_sem_post((OS_SEM *)msg[1]);
        break;
    case SMART_VOICE_MSG_DMA: /*MIC通路数据DMA消息*/
        err = ASR_CORE_WAKEUP;
        break;
    default:
        break;
    }

    if (asr->data_enable) {
        err = user_asr_data_handler(asr);
        err = err ? ASR_CORE_STANDBY : ASR_CORE_WAKEUP;
    }
    return err;
}

int user_platform_asr_open(void)
{
    if (!config_user_asr_enable) {
        return 0;
    }
    int err = 0;
    struct user_platform_asr_context *asr = (struct user_platform_asr_context *)zalloc(sizeof(struct user_platform_asr_context));

    if (!asr) {
        return -ENOMEM;
    }

    err = smart_voice_core_create(asr);
    if (err != OS_NO_ERR) {
        goto __err;
    }

    /*
     * 推送MIC的资源打开到语音识别主任务
     * V1.1.0版本支持了低功耗VAD MIC的使用，需要可以改为VAD MIC
     *
     */
    smart_voice_core_post_msg(4, SMART_VOICE_MSG_MIC_OPEN, VOICE_VAD_MIC, 16000, 4096);

    __this = asr;
    return 0;
__err:
    if (asr) {
        free(asr);
    }
    return err;
}

void user_platform_asr_close(void)
{
    if (!config_user_asr_enable) {
        return;
    }
    if (__this) {
        OS_SEM *sem = (OS_SEM *)malloc(sizeof(OS_SEM));
        os_sem_create(sem, 0);
        smart_voice_core_post_msg(2, SMART_VOICE_MSG_MIC_CLOSE, (int)sem);
        os_sem_pend(sem, 0);
        free(sem);
        smart_voice_core_free();
        free(__this);
        __this = NULL;
    }
}
