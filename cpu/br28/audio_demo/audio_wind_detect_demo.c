/*
 ****************************************************************
 *							AUDIO_WIND_DETECT_DEMO
 * File  : audio_wind_detect_demo.c
 * By    :
 * Notes :风噪检测使用，请不要修改本demo，如有需求，请拷贝副
 *		  本，自行修改!
 * Usage :
 *(1)需要打开双麦的宏 TCFG_AUDIO_DUAL_MIC_ENABLE
 *(2)需要使用第二版双麦算法 CONST_DMS_GLOBAL_VERSION = DMS_GLOBAL_V200
 *(3)调用audio_wind_detect_demo_open();使用
 ****************************************************************
 */
#include "audio_wind_detect_demo.h"
#include "system/includes.h"
#include "app_config.h"
#include "audio_adc.h"
#include "audio_enc.h"
#include "aec_user.h"
#include "task.h"
#include "cvp/cvp_common.h"
#include "lib_h/jlsp_ns.h"
#include "online_db_deal.h"
#include "spp_user.h"
#include "overlay_code.h"
#include "app_main.h"


#define WIND_DETECT_TASK_NAME "WindDetect"
/* 通过蓝牙spp发送风噪信息
 * 需要同时打开USER_SUPPORT_PROFILE_SPP和APP_ONLINE_DEBUG*/
#define WIND_DETECT_INFO_SPP_DEBUG_ENABLE  0

#define AUDIO_ADC_BUF_NUM        3	//mic_adc采样buf个数
#define AUDIO_ADC_IRQ_POINTS     256 //mic_adc采样长度（单位：点数）
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
#define AUDIO_ADC_CH			    3	//3mic通话
#elif TCFG_AUDIO_DUAL_MIC_ENABLE
#define AUDIO_ADC_CH			    2	//双mic通话
#else
#define AUDIO_ADC_CH			    1	//单mic通话
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
#define AUDIO_ADC_BUFS_SIZE      (AUDIO_ADC_BUF_NUM * AUDIO_ADC_IRQ_POINTS * AUDIO_ADC_CH)

struct audio_mic_hdl {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    int adc_dump_cnt;
    s16 adc_buf[AUDIO_ADC_BUFS_SIZE];    //align 2Bytes
#if (AUDIO_ADC_CH > 1)
    s16 tmp_buf[AUDIO_ADC_IRQ_POINTS];
#endif/*AUDIO_ADC_CH*/
#if (AUDIO_ADC_CH > 2)
    s16 tmp_buf_1[AUDIO_ADC_IRQ_POINTS];
#endif/*AUDIO_ADC_CH*/
};
static struct audio_mic_hdl *audio_mic = NULL;
extern struct audio_adc_hdl adc_hdl;

struct audio_wind_detect_hdl {
    int wd_flag;
    int wd_val;
    int wd_lev;
#if WIND_DETECT_INFO_SPP_DEBUG_ENABLE
    struct spp_operation_t *spp_opt;
#endif
};
static struct audio_wind_detect_hdl *audio_wd = NULL;

void audio_mic_dump_set(u16 dump_cnt)
{
    printf("adc_dump_cnt:%d\n", dump_cnt);
    if (audio_mic) {
        audio_mic->adc_dump_cnt = dump_cnt;
    }
}
/*adc_mic采样输出接口*/
static void adc_mic_output_handler(void *priv, s16 *data, int len)
{
    if (audio_mic) {
        if (audio_mic->adc_dump_cnt) {
            audio_mic->adc_dump_cnt--;
            //printf("[%d]",audio_mic->adc_dump_cnt);
            memset(data, 0, len);
            return;
        }

#if (AUDIO_ADC_CH == 3)/*3 Mic*/
        s16 *mic0_data = data;
        s16 *mic1_data = data + (len / 2);
        s16 *mic2_data = data + (len / 2) * 2;
        int offset = 0;

        s16 *mic1_data_pos = audio_mic->tmp_buf;
        s16 *mic2_data_pos = audio_mic->tmp_buf_1;
        //printf("mic_data:%x,%x,%d\n",data,mic1_data_pos,len);
        for (u16 i = 0; i < (len >> 1); i++) {
            mic0_data[i] = data[i * 3];
            mic1_data[i] = data[i * 3 + 1];
            mic2_data[i] = data[i * 3 + 2];
        }
        memcpy(mic1_data, mic1_data_pos, len);
        memcpy(mic2_data, mic2_data_pos, len);

        offset = (len / 2) * app_var.talk_ref_mic_ch;
        audio_aec_inbuf_ref(data + offset, len);
        offset = (len / 2) * app_var.talk_fb_mic_ch;
        audio_aec_inbuf_ref_1(data + offset, len);
        offset = (len / 2) * app_var.talk_mic_ch;
        audio_aec_inbuf(data + offset, len);
        return;

#elif (AUDIO_ADC_CH == 2)/*DualMic*/
        s16 *mic0_data = data;
        s16 *mic1_data = audio_mic->tmp_buf;
        s16 *mic1_data_pos = data + (len / 2);
        //printf("mic_data:%x,%x,%d\n",data,mic1_data_pos,len);
        for (u16 i = 0; i < (len >> 1); i++) {
            mic0_data[i] = data[i * 2];
            mic1_data[i] = data[i * 2 + 1];
        }
        memcpy(mic1_data_pos, mic1_data, len);

#if (TCFG_AUDIO_DMS_MIC_MANAGE == DMS_MASTER_MIC0)
        audio_aec_inbuf_ref(mic1_data_pos, len);
        audio_aec_inbuf(data, len);
#else
        audio_aec_inbuf_ref(data, len);
        audio_aec_inbuf(mic1_data_pos, len);
#endif/*TCFG_AUDIO_DMS_MIC_MANAGE*/

#else/*SingleMic*/
        audio_aec_inbuf(data, len);
#endif/*ESCO_ADC_CH*/
    }
}

/*mic初始化*/
static int audio_mic_en(u8 en, u16 sr, u16 mic0_gain, u16 mic1_gain, u16 mic2_gain, u16 mic3_gain)
{
    printf("audio_mic_en:%d\n", en);
    u8 mic_ch = en;
    if (en) {
        if (audio_mic) {
            printf("audio_mic re-malloc error\n");
            return -1;
        }
        audio_mic = zalloc(sizeof(struct audio_mic_hdl));
        if (audio_mic == NULL) {
            printf("audio mic zalloc failed\n");
            return -1;
        }
        audio_mic_pwr_ctl(MIC_PWR_ON);

        audio_mic_dump_set(3);/*处理开mic后在短时间起3次中断影响aec的问题*/
        if (en & AUDIO_ADC_MIC_0) {
            printf("adc_mic0 open,gain:%d\n", mic0_gain);
            audio_adc_mic_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic_set_gain(&audio_mic->mic_ch, mic0_gain);
        }
        if (en & AUDIO_ADC_MIC_1) {
            printf("adc_mic1 open,gain:%d\n", mic1_gain);
            audio_adc_mic1_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic1_set_gain(&audio_mic->mic_ch, mic1_gain);
        }
        if (en & AUDIO_ADC_MIC_2) {
            printf("adc_mic2 open,gain:%d\n", mic2_gain);
            audio_adc_mic2_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic2_set_gain(&audio_mic->mic_ch, mic2_gain);
        }
        if (en & AUDIO_ADC_MIC_3) {
            printf("adc_mic3 open,gain:%d\n", mic3_gain);
            audio_adc_mic3_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic3_set_gain(&audio_mic->mic_ch, mic3_gain);
        }
        audio_adc_mic_set_sample_rate(&audio_mic->mic_ch, sr);
        audio_adc_mic_set_buffs(&audio_mic->mic_ch, audio_mic->adc_buf,
                                AUDIO_ADC_IRQ_POINTS * 2, AUDIO_ADC_BUF_NUM);
        audio_mic->adc_output.handler = adc_mic_output_handler;
        audio_adc_add_output_handler(&adc_hdl, &audio_mic->adc_output);
        audio_adc_mic_start(&audio_mic->mic_ch);
    } else {
        if (audio_mic) {
            audio_adc_mic_close(&audio_mic->mic_ch);
            audio_adc_del_output_handler(&adc_hdl, &audio_mic->adc_output);
            audio_mic_pwr_ctl(MIC_PWR_OFF);
            free(audio_mic);
            audio_mic = NULL;
        }
    }
    return 0;
}

static int audio_wind_detect_output_hdl(s16 *data, int len)
{
    return len;
}

/*打开风噪检测*/
int audio_wind_detect_open(u32 sr, u16 mic0_gain, u16 mic1_gain, u16 mic2_gain, u16 mic3_gain)
{
    int err = 0;
    overlay_load_code(OVERLAY_AEC);
    audio_aec_open(sr, -1, audio_wind_detect_output_hdl);
    /* audio_aec_open(sr, WNC_EN | ENC_EN | AEC_EN, audio_wind_detect_output_hdl); */
    audio_mic_en(TCFG_AUDIO_ADC_MIC_CHA, sr, mic0_gain, mic1_gain, mic2_gain, mic3_gain);
    return 0;
}

/*关闭风噪检测*/
int audio_wind_detect_close()
{
    audio_aec_close();
    audio_mic_en(0, 0, 0, 0, 0, 0);
    return 0;
}

/*风噪检测信息获取任务*/
static void audio_wind_detect_task(void *p)
{
    while (1) {
        os_time_dly(10);
        audio_get_wind_detect_info();
    }
}

/*打开获取风噪信息的任务*/
int audio_wind_info_task_open()
{
    int err = 0;
    if (audio_wd) {
        printf("[err] audio_wd re-malloc !!!");
        return -1;
    }
    audio_wd = zalloc(sizeof(struct audio_wind_detect_hdl));
    if (audio_wd == NULL) {
        printf("[err] audio_wd malloc fail !!!");
        return 0;
    }
#if WIND_DETECT_INFO_SPP_DEBUG_ENABLE
    spp_get_operation_table(&audio_wd->spp_opt);
#endif
    overlay_load_code(OVERLAY_AEC);
    err = os_task_create(audio_wind_detect_task, NULL, 2, 256, 128, WIND_DETECT_TASK_NAME);
    /* err = task_create(audio_wind_detect_task, NULL, WIND_DETECT_TASK_NAME); */
    if (err != OS_NO_ERR) {
        printf("task create error!");
        return -1;
    }
    return 0;
}

/*关闭获取风噪信息的任务*/
int audio_wind_info_task_close(void)
{
    if (audio_wd) {
        /* task_kill(WIND_DETECT_TASK_NAME); */
        os_task_del(WIND_DETECT_TASK_NAME);
        free(audio_wd);
        audio_wd = NULL;
    }
    return 0;
}

/*打开风噪检测的demo*/
int audio_wind_detect_demo_open(void)
{
    /*打开风噪检测*/
    audio_wind_detect_open(16000, 4, 4, 1, 1);
    /*获取风噪的信息*/
    audio_wind_info_task_open();
    return 0;
}

/*关闭风噪检测的demo*/
int audio_wind_detect_demo_close(void)
{
    audio_wind_detect_close();
    audio_wind_info_task_close();
    return 0;
}

/*获取风噪信息*/
int audio_get_wind_detect_info(void)
{
    if (audio_wd) {
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
        jlsp_tms_get_wind_detect_info(&audio_wd->wd_flag, &audio_wd->wd_val, &audio_wd->wd_lev);
#elif TCFG_AUDIO_DUAL_MIC_ENABLE
        jlsp_get_wind_detect_info(&audio_wd->wd_flag, &audio_wd->wd_val, &audio_wd->wd_lev);
#endif
        printf("wd_flag:%d, wd_val:%d, wd_lev:%d", audio_wd->wd_flag, audio_wd->wd_val, audio_wd->wd_lev);

#if WIND_DETECT_INFO_SPP_DEBUG_ENABLE
        if (audio_wd->spp_opt && audio_wd->spp_opt->send_data) {
            char tmpbuf[25];
            memset(tmpbuf, 0x20, sizeof(tmpbuf));
            sprintf(tmpbuf, "falg:%d, val:%d, lev:%d", audio_wd->wd_flag, audio_wd->wd_val, audio_wd->wd_lev);
            audio_wd->spp_opt->send_data(NULL, tmpbuf, sizeof(tmpbuf));
        }
#endif
    }
    return 0;
}

/*获取是否有风，0：无风，1：有风*/
int audio_get_wd_flag(void)
{
    if (audio_wd) {
        return audio_wd->wd_flag;
    }
    return -1;
}
/*获取风强*/
int audio_get_wd_val(void)
{
    if (audio_wd) {
        return audio_wd->wd_val;
    }
    return -1;
}
/*获取预设的风噪等级, 0：弱风，1：中风：2：强风*/
int audio_get_wd_lev(void)
{
    if (audio_wd) {
        return audio_wd->wd_lev;
    }
    return -1;
}

static u8 audio_wind_detect_idle_query()
{
    return (audio_wd == NULL) ? 1 : 0;
}
REGISTER_LP_TARGET(audio_wind_detect_lp_target) = {
    .name = "audio_wind_detect",
    .is_idle = audio_wind_detect_idle_query,
};


