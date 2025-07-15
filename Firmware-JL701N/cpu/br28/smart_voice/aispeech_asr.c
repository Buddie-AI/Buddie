/*****************************************************************
>file name : aispeech_asr.c
>create time : Thu 23 Dec 2021 10:00:58 AM CST
>思必驰语音识别算法平台接入
*****************************************************************/
#include "system/includes.h"
#include "aispeech_asr.h"
#include "smart_voice.h"
#include "voice_mic_data.h"
#include "key_event_deal.h"
#include "vad_mic.h"

#define ASR_FRAME_SAMPLES   160 /*语音识别帧长（采样点）*/
#define AISPEECH_ASR_SAMPLE_RATE           16000

struct ais_platform_asr_context {
    void *mic;
    void *core;
    u8 data_enable;
};

extern const int config_aispeech_asr_enable;
static struct ais_platform_asr_context *__this = NULL;


#define ASR_CLK (160 * 1000000L) /*模块运行时钟(MaxFre:160MHz)*/

#ifdef AUDIO_PCM_DEBUG
//#define AISPEECH_ASR_AUDIO_DUMP
#endif /*AUDIO_PCM_DEBUG*/

#define AISPEECH_VAD_ASR_MODULE

#ifdef AISPEECH_VAD_ASR_MODULE
#include "vad_asr_demo.h"
#endif /*AISPEECH_VAD_ASR_MODULE*/
#if 1
#include "btstack/avctp_user.h"
#include "audio_anc.h"
#include "anc.h"
#endif

#define MIC_CAPTURE_BUF_SIZE (1024*10)

/*--------------------audio dump--------------------------------*/
#ifdef AISPEECH_ASR_AUDIO_DUMP
#include "cqueue.h"

OS_MUTEX catch_queue_amutex;
CQueue aec_uart_catch_queue;
u8 aec_uart_catch_buf[MIC_CAPTURE_BUF_SIZE];

extern int aec_uart_init();
extern int aec_uart_fill(u8 ch, void *buf, u16 size);
extern void aec_uart_write(void);
extern int aec_uart_close(void);

s16 gasr_exportbuf[256];
void audio_asr_export_demo_task(void *param)
{
    int ret = 0;
    int msg[16];

    while (1) {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (ret == OS_TASKQ) {

            switch (msg[1]) {
            case 0x01:
                int rlen = 512;
                while (LengthOfCQueue(&aec_uart_catch_queue) >= rlen) {
                    os_mutex_pend(&catch_queue_amutex, 0);
                    DeCQueue(&aec_uart_catch_queue, gasr_exportbuf, rlen);
                    os_mutex_post(&catch_queue_amutex);

                    /*         for (int i = 0; i < 256; i++)
                               {
                               gasr_exportbuf[i] = i * 110 + 1;
                               } */
                    aec_uart_fill(0, gasr_exportbuf, rlen);
                    aec_uart_fill(1, gasr_exportbuf, rlen);
                    aec_uart_fill(2, gasr_exportbuf, rlen);
                    aec_uart_fill(3, gasr_exportbuf, rlen);
                    aec_uart_fill(4, gasr_exportbuf, rlen);
                    // putchar('W');
                    aec_uart_write();
                }
                break;
            }
        }
    }

}

int audio_asr_export_demo_init()
{
    int err = 0;

    os_mutex_create(&catch_queue_amutex);
    InitCQueue(&aec_uart_catch_queue, sizeof(aec_uart_catch_buf), (CQItemType *)aec_uart_catch_buf);
    aec_uart_init();
    task_create(audio_asr_export_demo_task, NULL, "audio_asr_export_task");

    return err;

}

void audio_asr_export_demo_deinit()
{
    aec_uart_close();
    os_mutex_del(&catch_queue_amutex, 0);
    task_kill("audio_asr_export_task");
}
#endif /*AISPEECH_ASR_AUDIO_DUMP*/
/*--------------------audio dump end-------------------------------*/

#ifdef AISPEECH_VAD_ASR_MODULE
#if 1
enum ais_asr_cmdid {
    AIS_ASR_CMDID_NULL = 0,

    /*Hey xxx系列关键词*/
    AIS_ASR_CMDID_KEYWORD,
    AIS_ASR_CMDID_PLAY_MUSIC,   /*播放音乐*/
    AIS_ASR_CMDID_STOP_MUSIC,   /*停止播放*/
    AIS_ASR_CMDID_PAUSE_MUSIC,  /*暂停播放*/
    AIS_ASR_CMDID_VOLUME_UP,    /*增大音量*/
    AIS_ASR_CMDID_VOLUME_DOWN,  /*减小音量*/
    AIS_ASR_CMDID_PREV_SONG,    /*上一首*/
    AIS_ASR_CMDID_NEXT_SONG,    /*下一首*/
    AIS_ASR_CMDID_CALL_ACTIVE,  /*接听电话*/
    AIS_ASR_CMDID_CALL_HANGUP,  /*挂断电话*/
    AIS_ASR_CMDID_ANC_OFF,      /*关闭模式*/
    AIS_ASR_CMDID_ANC_ON,       /*降噪模式*/
    AIS_ASR_CMDID_ANC_TRANSPARENCY,  /*通透模式*/

    /*TODO*/

    AIS_ASR_CMDID_MAX,
};

typedef struct Input_CMD {
    enum ais_asr_cmdid  cmdid;
    const char *cmd_str;
    const char *at_cmd;
} Input_CMD_ST;

Input_CMD_ST global_asr_cmd[] = {
    { AIS_ASR_CMDID_KEYWORD,        "xiao ting xiao ting",     "AT+XEVENT=MAJORWAKEUP\r"},
    { AIS_ASR_CMDID_KEYWORD,        "xiao jie tong xue",       "AT+XEVENT=BIXINBIXIN\r"},
    { AIS_ASR_CMDID_STOP_MUSIC,     "zan ting bo fang",        "AT+XEVENT=ZANTINGBOFANG\r"},
    { AIS_ASR_CMDID_PLAY_MUSIC,     "bo fang yin yue",         "AT+XEVENT=BOFANGYINYUE\r"},
    { AIS_ASR_CMDID_PLAY_MUSIC,     "ji xv bo fang",           "AT+XEVENT=BOFANGYINYUE\r"},
    { AIS_ASR_CMDID_NEXT_SONG,      "xia yi shou",             "AT+XEVENT=XIAYISHOU\r"},
    { AIS_ASR_CMDID_PREV_SONG,      "shang yi shou",           "AT+XEVENT=SHANGYISHOU\r"},
    { AIS_ASR_CMDID_VOLUME_UP,      "zeng da yin liang",       "AT+XEVENT=ZENGDAYINLIANG\r"},
    { AIS_ASR_CMDID_VOLUME_UP,      "yin liang da yi dian",    "AT+XEVENT=ZENGDAYINLIANG\r"},
    { AIS_ASR_CMDID_VOLUME_DOWN,    "jian xiao yin liang",     "AT+XEVENT=JIANXIAOYINLIANG\r"},
    { AIS_ASR_CMDID_VOLUME_DOWN,    "yin liang xiao yi dian",  "AT+XEVENT=JIANXIAOYINLIANG\r"},
    { AIS_ASR_CMDID_CALL_ACTIVE,    "jie ting dian hua",       "AT+XEVENT=JIETINGDIANHUA\r"},
    { AIS_ASR_CMDID_CALL_HANGUP,    "gua duan dian hua",       "AT+XEVENT=GUADUANDIANHUA\r"},
    { AIS_ASR_CMDID_ANC_ON,         "jiang zao mo shi",        "AT+XEVENT=QUEDINGQUEDING\r"},
    { AIS_ASR_CMDID_ANC_TRANSPARENCY, "tong tou mo shi",       "AT+XEVENT=QUEDINGQUEDING\r"},
    { AIS_ASR_CMDID_ANC_OFF,        "guan bi jiang zao",       "AT+XEVENT=QUXIAOQUXIAO\r"},
};
#endif

/*
 * 算法引擎输出回调
 */
int aispeech_asr_output_handler(int status, const char *json, int bytes)
{

#if 1
    u8 a2dp_state;
    u8 call_state;
    enum ais_asr_cmdid  cmdid = AIS_ASR_CMDID_NULL;

    char *cmdstr = strstr(json, "rec");
    if (cmdstr != NULL) {
        //printf("cmdstr: %s. \n", cmdstr+6);
        cmdstr = cmdstr + 6;
        for (int i = 0; i < (sizeof(global_asr_cmd) / sizeof(global_asr_cmd[0])); i++) {
            if (memcmp(global_asr_cmd[i].cmd_str, cmdstr, strlen(global_asr_cmd[i].cmd_str)) == 0) {
                cmdid = global_asr_cmd[i].cmdid;

                int ret = user_send_at_cmd_prepare(global_asr_cmd[i].at_cmd, strlen(global_asr_cmd[i].at_cmd));
                //printf("user_send_at_cmd_prepare  :%s %d \n", global_asr_cmd[i].at_cmd, ret);
                break;
            }
        }
    }

    printf("asr_post: %s. bytes:%d\n", json, bytes);


    os_time_dly(1);

    switch (cmdid) {
    case AIS_ASR_CMDID_KEYWORD:
        //主唤醒词:
        printf("send SIRI cmd");
        user_send_cmd_prepare(USER_CTRL_HFP_GET_SIRI_OPEN, 0, NULL);
        break;

    case AIS_ASR_CMDID_PLAY_MUSIC:
    case AIS_ASR_CMDID_STOP_MUSIC:
    case AIS_ASR_CMDID_PAUSE_MUSIC:
        call_state = get_call_status();
        if ((call_state == BT_CALL_OUTGOING) ||
            (call_state == BT_CALL_ALERT)) {
            //user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        } else if (call_state == BT_CALL_INCOMING) {
            //user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        } else if (call_state == BT_CALL_ACTIVE) {
            //user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        } else {
            a2dp_state = a2dp_get_status();
            if (a2dp_state == BT_MUSIC_STATUS_STARTING) {
                if (cmdid == AIS_ASR_CMDID_PAUSE_MUSIC) {
                    printf("send PAUSE cmd");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
                } else if (cmdid == AIS_ASR_CMDID_STOP_MUSIC) {
                    printf("send PLAY cmd to STOP");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
                }
            } else {
                if (cmdid == AIS_ASR_CMDID_PLAY_MUSIC) {
                    printf("send PLAY cmd");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
                }
            }
        }
        break;


    case AIS_ASR_CMDID_VOLUME_UP:
        printf("volume up");
        volume_up(4); //music: 0 ~ 16, call: 0 ~ 15, step: 25%
        break;


    case AIS_ASR_CMDID_VOLUME_DOWN:
        printf("volume down");
        volume_down(4); //music: 0 ~ 16, call: 0 ~ 15, step: 25%
        break;


    case AIS_ASR_CMDID_PREV_SONG:
        printf("Send PREV cmd");
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PREV, 0, NULL);
        break;


    case AIS_ASR_CMDID_NEXT_SONG:
        printf("Send NEXT cmd");
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_NEXT, 0, NULL);
        break;


    case AIS_ASR_CMDID_CALL_ACTIVE:
        if (get_call_status() == BT_CALL_INCOMING) {
            printf("Send ANSWER cmd");
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        }
        break;


    case AIS_ASR_CMDID_CALL_HANGUP:
        printf("Send HANG UP cmd");
        if ((get_call_status() >= BT_CALL_INCOMING) && (get_call_status() <= BT_CALL_ALERT)) {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        }
        break;

    case AIS_ASR_CMDID_ANC_OFF:
        printf("Send ANC_OFF cmd");
#if TCFG_AUDIO_ANC_ENABLE
        anc_mode_switch(ANC_OFF, 1);
#endif
        break;
    case AIS_ASR_CMDID_ANC_ON:
        printf("Send ANC_ON cmd");
#if TCFG_AUDIO_ANC_ENABLE
        anc_mode_switch(ANC_ON, 1);
#endif
        break;
    case AIS_ASR_CMDID_ANC_TRANSPARENCY:
        printf("Send ANC_TRANSPARENCY cmd 11");
#if TCFG_AUDIO_ANC_ENABLE
        anc_mode_switch(ANC_TRANSPARENCY, 1);
#endif
        break;

    case AIS_ASR_CMDID_NULL:
        printf("KWS_EVENT_NULL");
        break;

    default:
        break;
    }
#endif


}
#endif

/*
 * 算法引擎打开函数
 */
static void *aispeech_asr_core_open(int sample_rate)
{
    printf("[%s-%d]-----start\n", __func__, __LINE__);
    clk_set("sys", ASR_CLK);
    printf("aip_asr----clk %d \n", ASR_CLK / 1000000);

    mem_stats();

    void *core = NULL;
#ifdef AISPEECH_VAD_ASR_MODULE
    core =  aispeech_vad_asr_init();
    aispeech_asr_register_handler(aispeech_asr_output_handler);
#endif /*AISPEECH_VAD_ASR_MODULE*/

#ifdef AISPEECH_ASR_AUDIO_DUMP
    audio_asr_export_demo_init();
#endif /*AISPEECH_ASR_AUDIO_DUMP*/

    printf("[%s-%d]-----end\n", __func__, __LINE__);

    return core;
}

/*
 * 算法引擎关闭函数
 */
static void aispeech_asr_core_close(void *core)
{
    printf("[%s-%d]-----start\n", __func__, __LINE__);

#ifdef AISPEECH_VAD_ASR_MODULE
    aispeech_vad_asr_deinit();
#endif /*AISPEECH_VAD_ASR_MODULE*/

#ifdef AISPEECH_ASR_AUDIO_DUMP
    audio_asr_export_demo_deinit();
#endif /*AISPEECH_ASR_AUDIO_DUMP*/

    printf("[%s-%d]-----end\n", __func__, __LINE__);

}

/*
 * 算法引擎数据处理
 */
static u8 asr_maxcosttime = 0;
static u32 asr_meantime = 0;
static u8 asr_printcnt = 0;

static int aispeech_asr_core_data_handler(void *core, void *data, int len)
{
    //printf("[%s-%d]-------len %d \n", __func__, __LINE__, len);
    printf(".");

    int cur_sys_clk = clk_get("sys");
    if (cur_sys_clk < ASR_CLK) {
        //printf("[%s-%d]------- %d M\n", __func__, __LINE__, cur_sys_clk/1000000);
        clk_set("sys", ASR_CLK);
    }

#ifdef AISPEECH_VAD_ASR_MODULE
    unsigned long start = jiffies_msec();
    aispeech_vad_asr_feed((char *)data, len);
    unsigned long end = jiffies_msec();
    u8  costtim = end - start;

    if (costtim > asr_maxcosttime) {
        asr_maxcosttime = costtim;
    }
    asr_meantime += costtim;

    if ((++asr_printcnt) >= 200) {
        printf("200 ais asr cost %d ms MAX %d  \r\n", asr_meantime / asr_printcnt, asr_maxcosttime);
        asr_printcnt = 0;
        asr_meantime = 0;
    }
#endif /*AISPEECH_VAD_ASR_MODULE*/

    return 0;
}

/*
*********************************************************************
*       aispeech asr data handler
* Description: 杰理音频平台语音识别数据处理
* Arguments  : asr - 语音识别数据管理结构
* Return	 : 0 - 处理成功， 非0 - 处理失败.
* Note(s)    : 该函数通过读取mic数据送入算法引擎完成语音帧.
*********************************************************************
*/
static int aispeech_asr_data_handler(struct ais_platform_asr_context *asr)
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
        result = aispeech_asr_core_data_handler(asr->core, data, sizeof(data));
    } else {
        printf("[%s-%d]---len %d \n", __func__, __LINE__, len);
    }
#ifdef AISPEECH_ASR_AUDIO_DUMP
    os_mutex_pend(&catch_queue_amutex, 0);
    EnCQueue(&aec_uart_catch_queue, gasr_datatempbuf, rlen);
    os_mutex_post(&catch_queue_amutex);
    os_taskq_post_msg("audio_asr_export_task", 1, 0x01);
#endif /*AISPEECH_ASR_AUDIO_DUMP*/

#if 0 //def AISPEECH_ASR_AUDIO_DUMP
    EnCQueue(&aec_uart_catch_queue, gasr_datatempbuf, rlen);
    rlen = 512;
    while (LengthOfCQueue(&aec_uart_catch_queue) >= rlen) {
        DeCQueue(&aec_uart_catch_queue, gasr_datatempbuf, rlen);

        /*         for (int i = 0; i < 256; i++)
                   {
                   gasr_datatempbuf[i] = i * 110 + 1;
                   } */
        aec_uart_fill(0, gasr_datatempbuf, rlen);
        aec_uart_fill(1, gasr_datatempbuf, rlen);
        aec_uart_fill(2, gasr_datatempbuf, rlen);
        aec_uart_fill(3, gasr_datatempbuf, rlen);
        aec_uart_fill(4, gasr_datatempbuf, rlen);
        putchar('W');
        aec_uart_write();

    }
#endif

    return 0;
}

/*
*********************************************************************
*       aispeech_asr core handler
* Description: 思必驰语音识别处理
* Arguments  : priv - 语音识别私有数据
*              taskq_type - TASK消息类型
*              msg  - 消息存储指针(对应自身模块post的消息)
* Return	 : None.
* Note(s)    : 音频平台资源控制以及ASR主要识别算法引擎.
*********************************************************************
*/
int aispeech_asr_core_handler(void *priv, int taskq_type, int *msg)
{
    struct ais_platform_asr_context *asr = (struct ais_platform_asr_context *)priv;
    int err = ASR_CORE_STANDBY;

    if (taskq_type != OS_TASKQ) {
        return err;
    }

    switch (msg[0]) {
    case SMART_VOICE_MSG_MIC_OPEN: /*语音识别打开 - MIC打开，算法引擎打开*/
        /* msg[1] - MIC数据源，msg[2] - 音频采样率，msg[3] - mic的数据总缓冲长度*/
        if (asr->data_enable == 0) {
            asr->mic = voice_mic_data_open(msg[1], msg[2], msg[3]);
            asr->core = aispeech_asr_core_open(msg[2]);
            asr->data_enable = 1;
        }
        err = ASR_CORE_WAKEUP;
        break;
    case SMART_VOICE_MSG_SWITCH_SOURCE:
        /*这里进行MIC的数据源切换 （主系统MIC或VAD MIC）*/
        if (asr->mic) {
            voice_mic_data_clear(asr->mic);
            voice_mic_data_switch_source(asr->mic, msg[1], msg[2], msg[3]);
        }
        /* smart_voice_kws_open(sv, msg[4]); */
        break;
    case SMART_VOICE_MSG_MIC_CLOSE: /*语音识别关闭 - MIC关闭，算法引擎关闭*/
        /* msg[2] - 信号量*/
        if (asr->data_enable == 1) {
            voice_mic_data_close(asr->mic);
            asr->mic = NULL;
            aispeech_asr_core_close(asr->core);
            asr->core = NULL;
            asr->data_enable = 0;
        }
        os_sem_post((OS_SEM *)msg[1]);
        break;
    case SMART_VOICE_MSG_WAKE:
        err = ASR_CORE_WAKEUP;
        /*putchar('W');*/
        /* voice_mic_data_debug_start(sv); */
        asr->data_enable = 1;
        break;
    case SMART_VOICE_MSG_STANDBY:
        asr->data_enable = 0;
        if (asr->mic) {
            voice_mic_data_clear(asr->mic);
        }
        /* voice_mic_data_debug_stop(sv); */
        break;
    case SMART_VOICE_MSG_DMA: /*MIC通路数据DMA消息*/
        err = ASR_CORE_WAKEUP;
        break;
    default:
        break;
    }

    if (asr->data_enable) {
        err = aispeech_asr_data_handler(asr);
        err = err ? ASR_CORE_STANDBY : ASR_CORE_WAKEUP;
    }
    return err;
}

int __ais_platform_asr_open(u8 mic)
{
    if (!config_aispeech_asr_enable) {
        return 0;
    }
    int err = 0;

    if (__this) {
        smart_voice_core_post_msg(4, SMART_VOICE_MSG_SWITCH_SOURCE, MIC_CAPTURE_BUF_SIZE, AISPEECH_ASR_SAMPLE_RATE, mic);
        return 0;
    }

    struct ais_platform_asr_context *asr = (struct ais_platform_asr_context *)zalloc(sizeof(struct ais_platform_asr_context));

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
    smart_voice_core_post_msg(4, SMART_VOICE_MSG_MIC_OPEN, mic, AISPEECH_ASR_SAMPLE_RATE, MIC_CAPTURE_BUF_SIZE);

    __this = asr;
    return 0;
__err:
    if (asr) {
        free(asr);
    }
    return err;
}

int ais_platform_asr_open(void)
{
    /* return __ais_platform_asr_open(VOICE_MCU_MIC); */
    return __ais_platform_asr_open(VOICE_VAD_MIC);
}
void ais_platform_asr_close(void)
{
    if (!config_aispeech_asr_enable) {
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

int audio_ais_platform_asr_init(struct vad_mic_platform_data *mic_data)
{
    lp_vad_mic_data_init(mic_data);
    /* __ais_platform_asr_open(VOICE_MCU_MIC); */
    __ais_platform_asr_open(VOICE_VAD_MIC);
    return 0;
}
/*
 * 来电KWS关键词识别
 */
int audio_phone_call_aispeech_asr_start(void)
{
    if (__this) {
        /*通话语音识别，由LP VAD的MIC切到系统主MIC进行识别*/
        smart_voice_core_post_msg(4, SMART_VOICE_MSG_SWITCH_SOURCE, MIC_CAPTURE_BUF_SIZE, AISPEECH_ASR_SAMPLE_RATE, VOICE_MCU_MIC);
        return 0;
    }

    __ais_platform_asr_open(VOICE_MCU_MIC);
    return 0;
}

/*
 * 来电KWS关闭（接通或拒接）
 */
int audio_phone_call_aispeech_asr_close(void)
{
    if (!__this) {
        return 0;
    }
    /*通话语音识别结束后，由系统的主MIC切换回LP VAD的MIC源*/
    smart_voice_core_post_msg(4, SMART_VOICE_MSG_SWITCH_SOURCE, MIC_CAPTURE_BUF_SIZE, AISPEECH_ASR_SAMPLE_RATE, VOICE_VAD_MIC);

    return 0;
}


static u8 aispeech_asr_core_idle_query()
{
    if (__this) {
        return !(__this->data_enable);
    } else {
        return 1;
    }
}
REGISTER_LP_TARGET(aispeech_asr_core_lp_target) = {
    .name = "aispeech_asr_core",
    .is_idle = aispeech_asr_core_idle_query,
};

