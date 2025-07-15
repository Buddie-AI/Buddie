/*
 ****************************************************************************
 *							ICSD ADT APP API
 *
 *Description	: 智能免摘和anc风噪检测相关接口
 *Notes			:
 ****************************************************************************
 */
#include "app_config.h"
#if ((defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN && \
	 TCFG_AUDIO_ANC_ENABLE)
#include "system/includes.h"
#include "icsd_adt_app.h"
#include "icsd_adt.h"
#include "system/task.h"
#include "asm/audio_adc.h"
#include "audio_anc.h"
#include "timer.h"
#include "icsd_anc.h"
#include "btstack/avctp_user.h"
#include "bt_tws.h"
#include "tone_player.h"
#include "asm/anc.h"
#include "app_anctool.h"
#include "audio_config.h"
#include "icsd_anc_user.h"

#if ICSD_ADT_WIND_INFO_SPP_DEBUG_EN
#include "spp_user.h"
#endif

extern struct audio_dac_hdl dac_hdl;

struct speak_to_chat_t {
    volatile u8 busy;
    volatile u8 state;
    struct icsd_acoustic_detector_libfmt libfmt;//回去需要的资源
    struct icsd_acoustic_detector_infmt infmt;//转递资源
    void *lib_alloc_ptr;//adt申请的内存指针
    u32 timer;//免摘结束定时齐
    u8 sensitivity;
    u8 voice_state;//免摘检测到声音的状态
    u8 adt_resume;//resume的状态
    u8 adt_suspend;//suspend的状态
    u8 adt_resume_timer;//resume定时器
    u8 wind_lvl;//记录风强信息
    u8 a2dp_state;//触发免摘时是否播歌的状态
    u8 speak_to_chat_state;//智能免摘的状态
    u8 last_anc_state;//记录触发免摘前的anc模式
    u8 anc_switch_flag;//是否是内部免摘触发的anc模式切换
    u8 tws_sync_state;//是否已经接收到同步的信息
    u8 adt_wind_lvl;//风噪等级
    u8 adt_tmp_wind_lvl;//风噪等级
    u8 adt_wat_result;//广域点击次数
    u8 adt_param_updata;//是否在resume前更新参数
    u8 adt_wat_ignore_flag;//是否忽略广域点击的响应
#if ICSD_ADT_WIND_INFO_SPP_DEBUG_EN
    struct spp_operation_t *spp_opt;
    u8 spp_connected_state;
#endif
    u32 wind_cnt;
    int sample_rate;
};
struct speak_to_chat_t *speak_to_chat_hdl = NULL;

typedef struct {
    u8 speak_to_chat_state;
    u8 adt_mode;
    u16 speak_to_chat_end_time; //免摘定时结束的时间，单位ms
} adt_info_t;
static adt_info_t adt_info = {
    .speak_to_chat_state = AUDIO_ADT_CLOSE,
    .adt_mode = ADT_MODE_CLOSE,
    .speak_to_chat_end_time = 5000,
};

/*tws同步char状态使用*/
static u8 globle_speak_to_chat_state = 0;
/*用于判断先开anc off再开免摘*/
static u8 adt_open_in_anc = 0;

void audio_icsd_adt_info_sync(u8 *data, int len);
void audio_icsd_adt_info_sync_cb(void *_data, u16 len, bool rx)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    u8 tmp_data[4];
    int mode = ((u8 *)_data)[0];
    int voice_state = 0;
    int wind_lvl    = 0;
    int wat_result  = 0;
    int val = 0;
    int err = 0;
    int adt_mode = 0;
    int anc_fade_gain = 0;
    // printf("audio_icsd_adt_info_sync_cb rx:%d, mode:%d ", rx, mode);
    switch (mode) {
    case SYNC_ICSD_ADT_WIND_LVL_RESULT:
        if (!hdl || !hdl->state) {
            break;
        }
        hdl->busy = 1;
        voice_state = ((u8 *)_data)[1];
        wind_lvl    = ((u8 *)_data)[2];
        wat_result  = ((u8 *)_data)[3];

        err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_WIND_LVL, wind_lvl);

#if TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE
        if ((adt_info.adt_mode & ADT_SPEAK_TO_CHAT_MODE) && voice_state) {
            /* err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_VOICE_STATE, voice_state); */
            set_speak_to_chat_voice_state(1);
        }
#endif /*TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE*/

#if TCFG_AUDIO_WIDE_AREA_TAP_ENABLE
#if (TCFG_USER_TWS_ENABLE && !AUDIO_ANC_WIDE_AREA_TAP_EVENT_SYNC)
        /*广域点击:是否响应另一只耳机的点击*/
        if (!rx)
#endif /*AUDIO_ANC_WIDE_AREA_TAP_EVENT_SYNC*/
        {
            if ((adt_info.adt_mode & ADT_WIDE_AREA_TAP_MODE) && wat_result && !hdl->adt_wat_ignore_flag) {
                err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_WAT_RESULT, wat_result);
            }
        }
#endif /*TCFG_AUDIO_WIDE_AREA_TAP_ENABLE*/
        hdl->busy = 0;
        break;
    case SYNC_ICSD_ADT_VOICE_STATE:
    case SYNC_ICSD_ADT_WIND_LVL_CMP:
    case SYNC_ICSD_ADT_WAT_RESULT:
        if (!hdl || !hdl->state) {
            break;
        }
        hdl->busy = 1;
        voice_state = ((u8 *)_data)[1];
        wind_lvl    = ((u8 *)_data)[2];
        wat_result  = ((u8 *)_data)[3];

#if TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE
        if (adt_info.adt_mode & ADT_WIND_NOISE_DET_MODE) {
            /*从机本地的风噪和接收到的主机风噪比较，取最大值*/
            if (tws_api_get_role() == TWS_ROLE_SLAVE) {
                if (hdl->adt_tmp_wind_lvl < ((u8)wind_lvl)) {
                    hdl->adt_tmp_wind_lvl = (u8)wind_lvl;
                }
                /*比较完后从机同步最终的风噪信息*/
                tmp_data[0] = SYNC_ICSD_ADT_WIND_LVL_RESULT;
                tmp_data[1] = 0;
                tmp_data[2] = hdl->adt_tmp_wind_lvl;
                tmp_data[3] = 0;
                audio_icsd_adt_info_sync(tmp_data, 4);
            }
        }
#endif /*TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE*/

#if TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE
        if ((adt_info.adt_mode & ADT_SPEAK_TO_CHAT_MODE) && voice_state) {
            /* err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_VOICE_STATE, voice_state); */
            set_speak_to_chat_voice_state(1);
        }
#endif /*TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE*/

#if TCFG_AUDIO_WIDE_AREA_TAP_ENABLE
#if (TCFG_USER_TWS_ENABLE && !AUDIO_ANC_WIDE_AREA_TAP_EVENT_SYNC)
        /*广域点击:是否响应另一只耳机的点击*/
        if (!rx)
#endif /*AUDIO_ANC_WIDE_AREA_TAP_EVENT_SYNC*/
        {
            if ((adt_info.adt_mode & ADT_WIDE_AREA_TAP_MODE) && wat_result && !hdl->adt_wat_ignore_flag) {
                err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_WAT_RESULT, wat_result);
            }
        }
#endif /*TCFG_AUDIO_WIDE_AREA_TAP_ENABLE*/

        hdl->busy = 0;
        break;
    case SYNC_ICSD_ADT_SUSPEND:
        audio_speak_to_char_suspend();
        break;
    case SYNC_ICSD_ADT_OPEN:
    case SYNC_ICSD_ADT_CLOSE:
        adt_mode = ((u8 *)_data)[1];
        err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, mode, adt_mode);
        break;
    case SYNC_ICSD_ADT_SET_ANC_FADE_GAIN:
        anc_fade_gain = (((u8 *)_data)[2] << 8) | ((u8 *)_data)[1];
        err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, mode, anc_fade_gain);
        break;
    default:
        break;
    }
}

#define TWS_FUNC_ID_ICSD_ADT_M2S    TWS_FUNC_ID('I', 'A', 'M', 'S')
REGISTER_TWS_FUNC_STUB(audio_icsd_adt_m2s) = {
    .func_id = TWS_FUNC_ID_ICSD_ADT_M2S,
    .func    = audio_icsd_adt_info_sync_cb,
};

void audio_icsd_adt_info_sync(u8 *data, int len)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    /* printf("audio_icsd_adt_info_sync , %d, %d", data[0], data[1]); */
#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state()) {
        int ret = tws_api_send_data_to_sibling(data, len, TWS_FUNC_ID_ICSD_ADT_M2S);
    } else
#endif
    {
        audio_icsd_adt_info_sync_cb(data, len, 0);
    }
}

static void audio_mic_output_handle(void *priv, s16 *data, int len)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        u8 esco_mic_num = audio_adc_file_get_esco_mic_num();
        u8 adt_mic_num = get_icsd_adt_mic_num();
        u8 mic_ch_num = esco_mic_num > adt_mic_num ? esco_mic_num : adt_mic_num;
        /*adt使用3mic模式 或者 通话使用3mic时(adc固定开3个)，如果免摘使用2mic，需要重新排列数据顺序*/
        if (mic_ch_num > hdl->libfmt.mic_num) {
            for (int i = 0; i < len / 2; i++) {
                memcpy(&data[2 * i], &data[3 * i], 4);
            }
        }
        icsd_acoustic_detector_mic_input_hdl(priv, data, len);
    }
}

/*算法结果输出*
 * voice_state : 是否有说话，0 无讲话，1 有讲话
 * wind_lvl : 风噪等级，0 - 10
 * wat_result : 连击次数，2 双击， 3 三击
 */
extern int tws_in_sniff_state();
void audio_acoustic_detector_output_hdl(u8 voice_state, u8 wind_lvl, u8 wat_result)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    u8 data[4] = {SYNC_ICSD_ADT_WIND_LVL_CMP, voice_state, wind_lvl, wat_result};
    static u8 cnt = 0;
    u8 wind_lvl_target_cnt = 0;;

    if (hdl && hdl->state) {
        hdl->busy = 1;
        /*限制少的判断放前面*/
        if ((adt_info.adt_mode & ADT_WIDE_AREA_TAP_MODE) && wat_result && !hdl->adt_wat_ignore_flag) {
#if TCFG_USER_TWS_ENABLE
            if (tws_in_sniff_state()) {
                /*如果在蓝牙siniff下需要退出蓝牙sniff再发送*/
                icsd_adt_tx_unsniff_req();
            }
#endif
            /*没有tws时直接更新状态*/
            data[0] = SYNC_ICSD_ADT_WAT_RESULT;
            audio_icsd_adt_info_sync(data, 4);
        } else if ((adt_info.adt_mode & ADT_SPEAK_TO_CHAT_MODE) && voice_state) {
#if TCFG_USER_TWS_ENABLE
            if (tws_in_sniff_state() && (tws_api_get_role() == TWS_ROLE_MASTER)) {
                /*如果在蓝牙siniff下需要退出蓝牙sniff再发送*/
                icsd_adt_tx_unsniff_req();
            }
            if (get_tws_sibling_connect_state()) {
                if ((tws_api_get_role() == TWS_ROLE_MASTER)) {
                    /*连接tws时，同步主机状态*/
                    data[0] = SYNC_ICSD_ADT_VOICE_STATE;
                    audio_icsd_adt_info_sync(data, 4);
                }
            } else
#endif
            {
                /*没有tws时直接更新状态*/
                data[0] = SYNC_ICSD_ADT_VOICE_STATE;
                audio_icsd_adt_info_sync(data, 4);
            }

        } else if (adt_info.adt_mode & ADT_WIND_NOISE_DET_MODE) {
            /*如果开了风噪检测，需要实时与对耳交流风噪信息*/
#if TCFG_USER_TWS_ENABLE
            if (tws_in_sniff_state()) {
                /*蓝牙sniff下，间隔 发送一次*/
                wind_lvl_target_cnt = 20;
            } else
#endif
            {
                /*非蓝牙sniff下，间隔 发送一次*/
                wind_lvl_target_cnt = 2;
            }
            cnt ++;
#if TCFG_USER_TWS_ENABLE
            if (get_tws_sibling_connect_state()) {
                /*记录本地的风噪强度*/
                hdl->adt_tmp_wind_lvl = wind_lvl;
                /*同步主机风噪和从机风噪比较*/
                if ((tws_api_get_role() == TWS_ROLE_MASTER)) {
                    if (cnt > wind_lvl_target_cnt) {
                        /*间隔wind_lvl_target_cnt次发送一次*/
                        data[0] = SYNC_ICSD_ADT_WIND_LVL_CMP;
                        audio_icsd_adt_info_sync(data, 4);

                        cnt = 0;
                    }
                }
            } else
#endif
            {
                /*没有tws时直接更新状态*/
                hdl->adt_wind_lvl = wind_lvl;
                if (cnt > wind_lvl_target_cnt) {
                    /*间隔wind_lvl_target_cnt次发送一次*/
                    data[0] = SYNC_ICSD_ADT_WIND_LVL_RESULT;
                    audio_icsd_adt_info_sync(data, 4);
                    cnt = 0;
                }
            }
        }

        hdl->busy = 0;
    }
}

//ANC task dma 抄数回调接口
void audio_acoustic_detector_anc_dma_post_msg(void)
{

}
//ANC DMA下采样数据输出回调，用于写卡分析
void audio_acoustic_detector_anc_dma_output_callback(void)
{

}

void audio_icsd_adt_start()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl->last_anc_state == ANC_ON) {
        icsd_acoustic_detector_resume(RESUME_ANCMODE, ADT_ANC_ON);
    } else if (hdl->last_anc_state == ANC_TRANSPARENCY) {
        icsd_acoustic_detector_resume(RESUME_BYPASSMODE, ADT_ANC_OFF);
    } else { /*if (hdl->last_anc_state == ANC_OFF)*/
        icsd_acoustic_detector_resume(RESUME_ANCMODE, ADT_ANC_OFF);
    }
}

void icsd_mic_ch_sel(struct icsd_acoustic_detector_infmt *infmt)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;

    infmt->mic0_type = ICSD_ANC_MIC_NULL;
    infmt->mic1_type = ICSD_ANC_MIC_NULL;
    infmt->mic2_type = ICSD_ANC_MIC_NULL;
    infmt->mic3_type = ICSD_ANC_MIC_NULL;
    u8 talk_mic_ch = icsd_get_talk_mic_ch();
    u8 ff_mic_ch = icsd_get_ref_mic_ch();
    u8 fb_mic_ch = icsd_get_fb_mic_ch();

    /*免摘使用3mic时，talk mic ch读取通话的配置，ff和fb使用anc的配置*/
    if (hdl->libfmt.mic_num == 3) {
        /*判断talk mic*/
        switch (talk_mic_ch) {
        case AUDIO_ADC_MIC_0:
            infmt->mic0_type = ICSD_ANC_TALK_MIC;
            break;
        case AUDIO_ADC_MIC_1:
            infmt->mic1_type = ICSD_ANC_TALK_MIC;
            break;
        case AUDIO_ADC_MIC_2:
            infmt->mic2_type = ICSD_ANC_TALK_MIC;
            break;
        case AUDIO_ADC_MIC_3:
            infmt->mic3_type = ICSD_ANC_TALK_MIC;
            break;
        }
    }

    /*无论tws还是头戴式立体声都是用左通道*/
    /*判断ff mic*/
    switch (ff_mic_ch) {
    case AUDIO_ADC_MIC_0:
        infmt->mic0_type = ICSD_ANC_LFF_MIC;
        break;
    case AUDIO_ADC_MIC_1:
        infmt->mic1_type = ICSD_ANC_LFF_MIC;
        break;
    case AUDIO_ADC_MIC_2:
        infmt->mic2_type = ICSD_ANC_LFF_MIC;
        break;
    case AUDIO_ADC_MIC_3:
        infmt->mic3_type = ICSD_ANC_LFF_MIC;
        break;
    }

    /*判断fb mic*/
    switch (fb_mic_ch) {
    case AUDIO_ADC_MIC_0:
        infmt->mic0_type = ICSD_ANC_LFB_MIC;
        break;
    case AUDIO_ADC_MIC_1:
        infmt->mic1_type = ICSD_ANC_LFB_MIC;
        break;
    case AUDIO_ADC_MIC_2:
        infmt->mic2_type = ICSD_ANC_LFB_MIC;
        break;
    case AUDIO_ADC_MIC_3:
        infmt->mic3_type = ICSD_ANC_LFB_MIC;
        break;
    }

}

void set_icsd_adt_param_updata()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        hdl->adt_param_updata = 1;
    }
}

int audio_acoustic_detector_updata()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        hdl->busy = 1;
        if (hdl->infmt.lfb_coeff) {
            free(hdl->infmt.lfb_coeff);
            hdl->infmt.lfb_coeff = NULL;
        }
        if (hdl->infmt.lff_coeff) {
            free(hdl->infmt.lff_coeff);
            hdl->infmt.lff_coeff = NULL;
        }
        if (hdl->infmt.ltrans_coeff) {
            free(hdl->infmt.ltrans_coeff);
            hdl->infmt.ltrans_coeff = NULL;
        }
        if (hdl->infmt.ltransfb_coeff) {
            free(hdl->infmt.ltransfb_coeff);
            hdl->infmt.ltransfb_coeff = NULL;
        }
        /*获取anc参数和增益*/
        /*获取anc参数和增益*/
        hdl->infmt.gains_l_fbgain = get_anc_gains_l_fbgain();
        hdl->infmt.gains_l_ffgain = get_anc_gains_l_ffgain();
        hdl->infmt.gains_l_transgain = get_anc_gains_l_transgain();
        hdl->infmt.gains_l_transfbgain = get_anc_gains_lfb_transgain();
        hdl->infmt.lfb_yorder     = get_anc_l_fbyorder();
        hdl->infmt.lff_yorder     = get_anc_l_ffyorder();
        hdl->infmt.ltrans_yorder  = get_anc_l_transyorder();
        hdl->infmt.ltransfb_yorder  = get_anc_lfb_transyorder();
        hdl->infmt.trans_alogm    = get_anc_gains_trans_alogm();
        hdl->infmt.alogm          = get_anc_gains_alogm();

        hdl->infmt.lfb_coeff      = zalloc(hdl->infmt.lfb_yorder * 40);
        hdl->infmt.lff_coeff      = zalloc(hdl->infmt.lff_yorder * 40);;
        hdl->infmt.ltrans_coeff      = zalloc(hdl->infmt.ltrans_yorder * 40);;
        hdl->infmt.ltransfb_coeff      = zalloc(hdl->infmt.ltransfb_yorder * 40);;

        memcpy(hdl->infmt.lfb_coeff, get_anc_lfb_coeff(), hdl->infmt.lfb_yorder * 40);
        memcpy(hdl->infmt.lff_coeff, get_anc_lff_coeff(), hdl->infmt.lff_yorder * 40);
        memcpy(hdl->infmt.ltrans_coeff, get_anc_ltrans_coeff(), hdl->infmt.ltrans_yorder * 40);
        memcpy(hdl->infmt.ltransfb_coeff, get_anc_ltrans_fb_coeff(), hdl->infmt.ltransfb_yorder * 40);

        extern u32 get_anc_gains_sign();
        hdl->infmt.gain_sign = get_anc_gains_sign();

        icsd_acoustic_detector_infmt_updata(&hdl->infmt);
        hdl->busy = 0;
    }

    return 0;
}

#if ICSD_ADT_WIND_INFO_SPP_DEBUG_EN
/*记录spp连接状态*/
void icsd_adt_wind_spp_connect_state_cbk(u8 state)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        hdl->spp_connected_state = state;
    }
}
#endif

/*设置talk mic gain和ff mic gain一样*/
void audio_icsd_adt_set_talk_mic_gain(audio_mic_param_t *mic_param)
{
    u8 talk_mic_ch = icsd_get_talk_mic_ch();
    u16 talk_mic_gain = audio_anc_ffmic_gain_get();
    printf("talk_mic_ch %d, gain %d", talk_mic_ch, talk_mic_gain);
    switch (talk_mic_ch) {
    case AUDIO_ADC_MIC_0:
        mic_param->mic0_gain = talk_mic_gain;
        break;
    case AUDIO_ADC_MIC_1:
        mic_param->mic1_gain = talk_mic_gain;
        break;
    case AUDIO_ADC_MIC_2:
        mic_param->mic2_gain = talk_mic_gain;
        break;
    case AUDIO_ADC_MIC_3:
        mic_param->mic3_gain = talk_mic_gain;
        break;
    }
}

void audio_icsd_adt_set_sample(int sample_rate)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl) {
        printf("%s:%d", __func__, __LINE__);
        if (hdl->sample_rate != sample_rate) {
            icsd_adt_set_audio_sample_rate(sample_rate);
        }
        hdl->sample_rate = sample_rate;
    }
}

/*获取免摘需要多少个mic*/
u8 get_icsd_adt_mic_num()
{
    u8 mic_num = 2;
    extern const u8 ADT_PATH_CONFIG;
    if (ADT_PATH_CONFIG & ADT_PATH_3M_EN) {
        mic_num = 3;
    }
    return mic_num;
}

/*打开智能免摘检测*/
int audio_acoustic_detector_open()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl == NULL) {
        return -1;
    }

    /*修改了sniff的唤醒间隔*/
    icsd_set_tws_t_sniff(160);
    icsd_adt_version();
    u8 adt_function = 0;
    if (adt_info.adt_mode == ADT_WIDE_AREA_TAP_MODE) {
        /*只开广域点击*/
        //adt_function = ADT_WAT_MODE;
        adt_function |=	ADT_WAT_EN;
    } else if (adt_info.adt_mode & ADT_WIDE_AREA_TAP_MODE) {
        /*同时开了广域点击 和 智能免摘/风噪检测*/
        //adt_function = ADT_VW_WAT_MODE;
        adt_function |=	ADT_WAT_EN;
        adt_function |=	ADT_VDT_EN;
        adt_function |=	ADT_WDT_EN;
    } else {
        /*只开了智能免摘/风噪检测*/
        //adt_function = ADT_VOICE_WIND_MODE;
        adt_function |=	ADT_VDT_EN;
        adt_function |=	ADT_WDT_EN;
    }
    if (ADT_EAR_IN_EN) {
        adt_function |= ADT_EIN_EN;
    }

    audio_dac_set_samplerate_callback_add(&dac_hdl, audio_icsd_adt_set_sample);

    icsd_acoustic_detector_get_libfmt(&hdl->libfmt, adt_function);

#if ICSD_ADT_WIND_INFO_SPP_DEBUG_EN
    /*获取spp发送句柄*/
    if (adt_info.adt_mode & ADT_WIND_NOISE_DET_MODE) {
        spp_get_operation_table(&hdl->spp_opt);
        if (hdl->spp_opt) {
            /*设置记录spp连接状态回调*/
            hdl->spp_opt->regist_state_cbk(hdl, icsd_adt_wind_spp_connect_state_cbk);
        }
    }
#endif

    printf("mode : %x, adc_len : %d, sr : %d, size : %d", adt_function, hdl->libfmt.adc_isr_len, hdl->libfmt.adc_sr, hdl->libfmt.lib_alloc_size);
    if (hdl->libfmt.lib_alloc_size) {
        hdl->lib_alloc_ptr = zalloc(hdl->libfmt.lib_alloc_size);
    } else {
        return -1;;
    }
    if (hdl->lib_alloc_ptr == NULL) {
        printf("hdl->lib_alloc_ptr malloc fail !!!");
        return -1;
    }

    /*配置TWS还是头戴式耳机*/
#if TCFG_USER_TWS_ENABLE
    hdl->infmt.adt_mode = ADT_TWS;
#else
    hdl->infmt.adt_mode = ADT_HEADSET;
#endif /*TCFG_USER_TWS_ENABLE*/

    /*配置mic通道*/
    icsd_mic_ch_sel(&hdl->infmt);
    /* hdl->infmt.mic0_type = ICSD_ANC_MIC_NULL; */
    /* hdl->infmt.mic1_type = ICSD_ANC_LFF_MIC; */
    /* hdl->infmt.mic2_type = ICSD_ANC_LFB_MIC; */
    /* hdl->infmt.mic3_type = ICSD_ANC_MIC_NULL; */
    printf("mic0_type: %d, mic1_type: %d, mic2_type: %d, mic3_type: %d", hdl->infmt.mic0_type, hdl->infmt.mic1_type, hdl->infmt.mic2_type, hdl->infmt.mic3_type);
    /*检测灵敏度
     * 0 : 正常灵敏度
     * 1 : 智能灵敏度*/
    hdl->infmt.sensitivity = 0;
    /*配置低压/高压DAC*/
#if ((TCFG_AUDIO_DAC_MODE == DAC_MODE_L_DIFF) || (TCFG_AUDIO_DAC_MODE == DAC_MODE_L_SINGLE))
    hdl->infmt.dac_mode = ADT_DACMODE_LOW;
#else
    hdl->infmt.dac_mode = ADT_DACMODE_HIGH;
#endif /*TCFG_AUDIO_DAC_MODE*/
    /*获取anc参数和增益*/
    hdl->infmt.gains_l_fbgain = get_anc_gains_l_fbgain();
    hdl->infmt.gains_l_ffgain = get_anc_gains_l_ffgain();
    hdl->infmt.gains_l_transgain = get_anc_gains_l_transgain();
    hdl->infmt.gains_l_transfbgain = get_anc_gains_lfb_transgain();
    hdl->infmt.lfb_yorder     = get_anc_l_fbyorder();
    hdl->infmt.lff_yorder     = get_anc_l_ffyorder();
    hdl->infmt.ltrans_yorder  = get_anc_l_transyorder();
    hdl->infmt.ltransfb_yorder  = get_anc_lfb_transyorder();
    hdl->infmt.trans_alogm    = get_anc_gains_trans_alogm();
    hdl->infmt.alogm          = get_anc_gains_alogm();

    hdl->infmt.lfb_coeff      = zalloc(hdl->infmt.lfb_yorder * 40);
    hdl->infmt.lff_coeff      = zalloc(hdl->infmt.lff_yorder * 40);;
    hdl->infmt.ltrans_coeff      = zalloc(hdl->infmt.ltrans_yorder * 40);;
    hdl->infmt.ltransfb_coeff      = zalloc(hdl->infmt.ltransfb_yorder * 40);;

    memcpy(hdl->infmt.lfb_coeff, get_anc_lfb_coeff(), hdl->infmt.lfb_yorder * 40);
    memcpy(hdl->infmt.lff_coeff, get_anc_lff_coeff(), hdl->infmt.lff_yorder * 40);
    memcpy(hdl->infmt.ltrans_coeff, get_anc_ltrans_coeff(), hdl->infmt.ltrans_yorder * 40);
    memcpy(hdl->infmt.ltransfb_coeff, get_anc_ltrans_fb_coeff(), hdl->infmt.ltransfb_yorder * 40);

    extern u32 get_anc_gains_sign();
    hdl->infmt.gain_sign = get_anc_gains_sign();

    hdl->infmt.alloc_ptr = hdl->lib_alloc_ptr;
    /*数据输出回调*/
    /* hdl->infmt.output_hdl = audio_acoustic_detector_output_hdl; */
    hdl->infmt.anc_dma_post_msg = audio_acoustic_detector_anc_dma_post_msg;
    hdl->infmt.anc_dma_output_callback = audio_acoustic_detector_anc_dma_output_callback;
    hdl->infmt.ff_gain = audio_anc_ffmic_gain_get();
    hdl->infmt.fb_gain = audio_anc_fbmic_gain_get();
    /* hdl->infmt.sample_rate = 16000;//Raymond debug */
    hdl->infmt.sample_rate = 44100;//Raymond debug
    printf("ff_gain %d, fb_gain %d", hdl->infmt.ff_gain, hdl->infmt.fb_gain);
    icsd_acoustic_detector_set_infmt(&hdl->infmt);
    icsd_adt_task_create();
    icsd_src_task_create();
    icsd_acoustic_detector_open();

    u16 mic_ch = 0;
    /*根据anc使用的mic配置adt的mic*/
    if (TCFG_AUDIO_ANCL_FF_MIC != MIC_NULL) {
        mic_ch |= BIT(TCFG_AUDIO_ANCL_FF_MIC);
    }
    if (TCFG_AUDIO_ANCL_FB_MIC != MIC_NULL) {
        mic_ch |= BIT(TCFG_AUDIO_ANCL_FB_MIC);
    }
    /*头戴式使用左耳*/
    if (hdl->infmt.adt_mode != ADT_HEADSET) {
        if (TCFG_AUDIO_ANCR_FF_MIC != MIC_NULL) {
            mic_ch |= BIT(TCFG_AUDIO_ANCR_FF_MIC);
        }
        if (TCFG_AUDIO_ANCR_FB_MIC != MIC_NULL) {
            mic_ch |= BIT(TCFG_AUDIO_ANCR_FB_MIC);
        }
    }

    if (hdl->libfmt.mic_num == 3) {
        mic_ch |= icsd_get_talk_mic_ch();
    }

    audio_mic_param_t mic_param = {
        .mic_ch_sel        = mic_ch,
        .sample_rate       = hdl->libfmt.adc_sr,//采样率
        .adc_irq_points    = hdl->libfmt.adc_isr_len,//一次处理数据的数据单元， 单位点 4对齐(要配合mic起中断点数修改)
        .adc_buf_num       = 3,
        .mic0_gain          = audio_anc_mic_gain_get(0),
        .mic1_gain          = audio_anc_mic_gain_get(1),
        .mic2_gain          = audio_anc_mic_gain_get(2),
        .mic3_gain          = audio_anc_mic_gain_get(3),
    };
    if (hdl->libfmt.mic_num == 3) {
        /*免摘使用3mic时，设置talk mic gain和ff mic gain一样*/
        audio_icsd_adt_set_talk_mic_gain(&mic_param);
    }
    int err = audio_mic_en(1, &mic_param, audio_mic_output_handle);
    if (err != 0) {
        printf("open mic fail !!!");
        goto err0;
    }
    icsd_adt_start();
    audio_icsd_adt_start();

    /*用于TWS同步adt状态*/
    if (adt_info.speak_to_chat_state == AUDIO_ADT_CHAT) {
        audio_speak_to_chat_voice_state_sync();
        hdl->adt_resume = 0;
    }
    adt_info.speak_to_chat_state = AUDIO_ADT_OPEN;
    hdl->speak_to_chat_state = AUDIO_ADT_OPEN;
    return 0;
err0:
    audio_acoustic_detector_close();
    return -1;
}

/*关闭智能免摘检测*/
int audio_acoustic_detector_close()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl) {
        audio_dac_set_samplerate_callback_del(&dac_hdl, audio_icsd_adt_set_sample);
        audio_mic_en(0, NULL, NULL);
        if (hdl->lib_alloc_ptr) {
            /*需要先挂起再关闭*/
            icsd_acoustic_detector_suspend();
            icsd_acoustic_detector_close();
            icsd_adt_task_kill();
            icsd_src_task_kill();

            if (hdl->infmt.lfb_coeff) {
                free(hdl->infmt.lfb_coeff);
                hdl->infmt.lfb_coeff = NULL;
            }
            if (hdl->infmt.lff_coeff) {
                free(hdl->infmt.lff_coeff);
                hdl->infmt.lff_coeff = NULL;
            }
            if (hdl->infmt.ltrans_coeff) {
                free(hdl->infmt.ltrans_coeff);
                hdl->infmt.ltrans_coeff = NULL;
            }
            if (hdl->infmt.ltransfb_coeff) {
                free(hdl->infmt.ltransfb_coeff);
                hdl->infmt.ltransfb_coeff = NULL;
            }

            free(hdl->lib_alloc_ptr);
            hdl->lib_alloc_ptr = NULL;
        }
    }
    return 0;
}

/*dac数据清零 mute*/
void audio_adt_dac_check_slience_cb(void *buf, int len)
{
    /* memset((u8 *)buf, 0x0, len); */
}
void audio_adt_dac_mute_start(void *priv)
{
    audio_dac_write_callback_add(&dac_hdl, audio_adt_dac_check_slience_cb);
}
void audio_adt_dac_mute_stop(void)
{
    audio_dac_write_callback_del(&dac_hdl, audio_adt_dac_check_slience_cb);
}


void adt_anc_mode_switch(u8 mode, u8 tone_play)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl) {
        anc_mode_switch(ANC_ON, 0);
    }
}


void audio_icsd_adt_suspend()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        printf("%s : %d", __func__, __LINE__);
        icsd_acoustic_detector_suspend();
        hdl->adt_suspend = 0;
        /*删除定时器*/
        if (hdl->timer) {
            sys_s_hi_timeout_del(hdl->timer);
            hdl->timer = 0;
        }
    }
}

/*char 定时结束后从通透同步恢复anc on /anc off*/
void audio_speak_to_char_suspend(void)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    int err = 0;
    if (hdl && hdl->state) {
        printf("%s : %d", __func__, __LINE__);
        /*tws同步suspend时，防止二次调用*/
        if (hdl->adt_suspend) {
            return;
        }
        hdl->busy = 1;
        hdl->adt_suspend = 1;
        printf("%s : %d", __func__, hdl->last_anc_state);
        /*tws同步后，删除定时器*/
        if (hdl->timer) {
            sys_s_hi_timeout_del(hdl->timer);
            hdl->timer = 0;
        }
        hdl->speak_to_chat_state = AUDIO_ADT_OPEN;
        adt_info.speak_to_chat_state = AUDIO_ADT_OPEN;
        icsd_acoustic_detector_suspend();
        /*播放结束提示音的标志*/
        hdl->anc_switch_flag = 1;
        if (hdl->last_anc_state == ANC_ON) {
            if (anc_mode_get() == ANC_ON) {
                icsd_acoustic_detector_resume(RESUME_ANCMODE, ADT_ANC_ON);
#if SPEAK_TO_CHAT_PLAY_TONE_EN
                /*免摘结束退出通透提示音*/
                err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_TONE_PLAY, (int)ICSD_ADT_TONE_NUM0);
#else
                hdl->adt_suspend = 0;
#endif /*SPEAK_TO_CHAT_PLAY_TONE_EN*/
                hdl->anc_switch_flag = 0;
            } else {
                /* anc_mode_switch(ANC_ON, 0); */
                err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 3, ICSD_ANC_MODE_SWITCH, ANC_ON, 0);
            }
        } else if (hdl->last_anc_state == ANC_TRANSPARENCY) {
            if (anc_mode_get() == ANC_TRANSPARENCY) {
                icsd_acoustic_detector_resume(RESUME_BYPASSMODE, ADT_ANC_OFF);
#if SPEAK_TO_CHAT_PLAY_TONE_EN
                /*免摘结束退出通透提示音*/
                err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_TONE_PLAY, (int)ICSD_ADT_TONE_NUM0);
#else
                hdl->adt_suspend = 0;
#endif /*SPEAK_TO_CHAT_PLAY_TONE_EN*/
                hdl->anc_switch_flag = 0;
            } else {
                /* anc_mode_switch(ANC_TRANSPARENCY, 0); */
                err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 3, ICSD_ANC_MODE_SWITCH, ANC_TRANSPARENCY, 0);
            }
        } else { /*if (hdl->last_anc_state == ANC_OFF*/
            if (anc_mode_get() == ANC_OFF) {
                icsd_acoustic_detector_resume(RESUME_ANCMODE, ADT_ANC_OFF);
#if SPEAK_TO_CHAT_PLAY_TONE_EN
                /*免摘结束退出通透提示音*/
                err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_TONE_PLAY, (int)ICSD_ADT_TONE_NUM0);
#else
                hdl->adt_suspend = 0;
#endif /*SPEAK_TO_CHAT_PLAY_TONE_EN*/
                hdl->anc_switch_flag = 0;
            } else {
                adt_open_in_anc = 1;
                /* anc_mode_switch(ANC_OFF, 0); */
                err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 3, ICSD_ANC_MODE_SWITCH, ANC_OFF, 0);
            }
        }

        audio_adt_dac_mute_stop();
        if (hdl->a2dp_state && (bt_a2dp_get_status() != BT_MUSIC_STATUS_STARTING)) {
            printf("send PLAY cmd");
            bt_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
        }
        hdl->a2dp_state = 0;
        hdl->busy = 0;
    }
}

void audio_speak_to_char_sync_suspend(void)
{
    printf("%s", __func__);
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    u8 data[4];
    if (hdl && hdl->state) {
        hdl->adt_suspend = 0;
#if TCFG_USER_TWS_ENABLE
        if (get_tws_sibling_connect_state()) {
            data[0] = SYNC_ICSD_ADT_SUSPEND;
            audio_icsd_adt_info_sync(data, 4);
        } else {
            audio_speak_to_char_suspend();
        }
#else
        audio_speak_to_char_suspend();
#endif/*TCFG_USER_TWS_ENABLE*/
    }
}

void audio_anc_mode_switch_in_adt(u8 anc_mode)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        icsd_acoustic_detector_suspend();
        if (anc_mode == ANC_TRANSPARENCY && hdl->adt_resume) {
            /*说话触发的通透不记录*/
            return;
        }
        hdl->busy = 1;
        printf("%s : %d", __func__, __LINE__);
        /*每次切anc模式都在resume前更新参数*/
        set_icsd_adt_param_updata();
        /*进入免摘后，如果切换anc模式主动退出免摘*/
        if (hdl->speak_to_chat_state == AUDIO_ADT_CHAT) {
            /* hdl->adt_suspend = 0; */
            /*删除定时器*/
            if (hdl->timer) {
                sys_s_hi_timeout_del(hdl->timer);
                hdl->timer = 0;
            }
        }
        if ((anc_mode == ANC_OFF) && (hdl->speak_to_chat_state != AUDIO_ADT_CLOSE)) {
            adt_open_in_anc = 1;
        }
        /*记录外部切换的anc模式*/
        hdl->last_anc_state = anc_mode;
        hdl->busy = 0;
    }
}

void audio_speak_to_chat_timer(void *p)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        printf("%s", __func__);
        if (hdl->timer) {
            sys_s_hi_timeout_del(hdl->timer);
        }
        hdl->timer = 0;
        audio_speak_to_char_sync_suspend();
    }
}

/*切换anc 或者通透完成后，恢复免摘*/
void audio_icsd_adt_resume_timer(void *p)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        hdl->busy = 1;
        printf("%s : %d", __func__, __LINE__);
        /*更新参数*/
        if (hdl->adt_param_updata) {
            hdl->adt_param_updata = 0;
            audio_acoustic_detector_updata();
        }
        /*adt挂起完成，准备退出挂起*/
        hdl->adt_suspend = 0;
        if (hdl->adt_resume) {
            /*智能免摘切换通透挂起恢复*/
            if (anc_mode_get() == ANC_TRANSPARENCY) {
                icsd_acoustic_detector_resume(RESUME_BYPASSMODE, ADT_ANC_OFF);
            }
            hdl->adt_resume = 0;
        } else {
            /*智能免摘定时结束触发的挂起恢复*/
            if (anc_mode_get() == ANC_ON) {
                icsd_acoustic_detector_resume(RESUME_ANCMODE, ADT_ANC_ON);
            } else if (anc_mode_get() == ANC_TRANSPARENCY) {
                icsd_acoustic_detector_resume(RESUME_BYPASSMODE, ADT_ANC_OFF);
            } else { /*if (anc_mode_get() == ANC_OFF*/
                icsd_acoustic_detector_resume(RESUME_ANCMODE, ADT_ANC_OFF);
            }
            hdl->speak_to_chat_state = AUDIO_ADT_OPEN;
            adt_info.speak_to_chat_state = AUDIO_ADT_OPEN;
        }
        hdl->adt_resume_timer = 0;
        hdl->busy = 0;
    }
}
void audio_icsd_adt_resume()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        hdl->busy = 1;
        printf("%s : %d", __func__, __LINE__);
        adt_open_in_anc = 0;
        /*延时1s，等待数据稳定在恢复*/
        if (hdl->adt_resume_timer == 0) {
            hdl->adt_resume_timer = sys_s_hi_timerout_add(NULL, audio_icsd_adt_resume_timer, 500);
        }
        if (hdl->adt_resume) {
            /*智能免摘定时结束触发的挂起恢复*/
            if (anc_mode_get() == ANC_TRANSPARENCY) {
            }
        } else {
            if (hdl->anc_switch_flag) {
#if SPEAK_TO_CHAT_PLAY_TONE_EN
                /*免摘结束退出通透提示音*/
                icsd_adt_tone_play(ICSD_ADT_TONE_NUM0);
#endif /*SPEAK_TO_CHAT_PLAY_TONE_EN*/
                hdl->anc_switch_flag = 0;
            }
        }
        hdl->busy = 0;
    }
}

u8 audio_speak_to_chat_is_running()
{
    return (speak_to_chat_hdl != NULL);
}

/*adt是否在跑*/
u8 audio_icsd_adt_is_running()
{
    return (speak_to_chat_hdl != NULL);
}

/*获取智能免摘的状态*/
u8 get_speak_to_chat_state()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl) {
        return hdl->speak_to_chat_state;
    } else {
        return 0;
    }
}

/*获取adt的模式*/
u8 get_icsd_adt_mode()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    return adt_info.adt_mode;
}

void audio_icsd_adt_state_sync_done(u8 adt_mode, u8 speak_to_chat_state)
{

    int close_adt = ADT_SPEAK_TO_CHAT_MODE | ADT_WIDE_AREA_TAP_MODE | ADT_WIND_NOISE_DET_MODE;
    audio_icsd_adt_res_close(close_adt);

    printf("%s, adt %d, chat %d", __func__, adt_mode, speak_to_chat_state);
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    adt_info.speak_to_chat_state = speak_to_chat_state;
    if (hdl && hdl->state) {
        /*tws同步时重置记录的anc模式*/
        hdl->last_anc_state = anc_mode_get();
    }
    adt_info.adt_mode = adt_mode;
    /*同步动作前已经关闭adt了，这里只需要判断打开，避免没有开adt在anc off里面开了anc*/
    if (adt_info.adt_mode) {
        /*在anc跑完后面执行同步adt状态*/
        adt_open_in_anc = 1;
    }
}

/*同步tws配对时，同步adt的状态*/
static u8 sync_data[5];
void audio_anc_icsd_adt_state_sync(u8 *data)
{
    printf("audio_anc_icsd_adt_state_sync");
    memcpy(sync_data, data, sizeof(sync_data));
    os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_STATE_SYNC, (int)sync_data);
}

/*检测到讲话状态同步*/
void set_speak_to_chat_voice_state(u8 state)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    int err = 0;
    if (hdl && hdl->state) {
        hdl->voice_state = state;
        hdl->tws_sync_state = 0;
        printf("%s", __func__);
        err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, ICSD_ADT_VOICE_STATE, (int)hdl);
    }
}

void audio_speak_to_chat_voice_state_sync(void)
{
    printf("%s", __func__);
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    u8 data[4] = {0};
    if ((hdl->adt_resume == 0) && (hdl->adt_suspend == 0)) {
#if TCFG_USER_TWS_ENABLE
        if (get_tws_sibling_connect_state()) {
            /*hdl->tws_sync_state == 0 : 保证上一次消息还没接收到，不发下一个消息*/
            if (hdl->tws_sync_state == 0) {
                hdl->tws_sync_state = 1;
                data[0] = SYNC_ICSD_ADT_VOICE_STATE;
                audio_icsd_adt_info_sync(data, 4);
            }
        } else {
            set_speak_to_chat_voice_state(1);
        }
#else
        set_speak_to_chat_voice_state(1);
#endif/*TCFG_USER_TWS_ENABLE*/
    }
}


void icsd_adt_task_play_tone_cb(void *priv)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    /*播放完提示音*/
    if (hdl && hdl->state) {
        hdl->adt_suspend = 0;
    }
}

static void audio_icsd_adt_task(void *p)
{
    printf("%s", __func__);
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    int msg[16];
    int res;

    while (1) {
        res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            switch (msg[1]) {
#if TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE
            case ICSD_ADT_VOICE_STATE:
                hdl = speak_to_chat_hdl;
                if (hdl && hdl->state) {
                    hdl->busy = 1;
                    /* hdl->adt_resume == 0 ：上一次检测到语音，挂起切换通透还没完成恢复的时候，不能做下一次动作
                     * hdl->adt_suspend == 0 : 结束定时挂起切换anc模式的时候，还没完成模式切换，不能做检测作动*/
                    if (hdl->voice_state && (hdl->adt_resume == 0) && (hdl->adt_suspend == 0)) {
                        if (hdl->speak_to_chat_state == AUDIO_ADT_CHAT) {
                            printf("hdl->speak_to_chat_state == AUDIO_ADT_CHAT)");
                        } else {
                            printf("%s", __func__);
                            /*检测到语音切换到通透*/
                            if (anc_mode_get() != ANC_TRANSPARENCY) {
                                hdl->adt_resume = 1;/*调用顺序不可改*/
                                hdl->speak_to_chat_state = AUDIO_ADT_OPEN;
                                adt_info.speak_to_chat_state = AUDIO_ADT_OPEN;
                                icsd_acoustic_detector_suspend();
                                set_anc_adt_state(1);
                                anc_mode_switch(ANC_TRANSPARENCY, 0);
                            }

                            /*如果在播歌暂停播歌*/
                            if (bt_a2dp_get_status() == BT_MUSIC_STATUS_STARTING) {
                                printf("send PAUSE cmd");
                                hdl->a2dp_state = 1;
                                bt_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
                            }
#if SPEAK_TO_CHAT_PLAY_TONE_EN
                            /*结束免摘后检测播放提示音*/
                            icsd_adt_tone_play(ICSD_ADT_TONE_NORMAL);
#endif /*SPEAK_TO_CHAT_PLAY_TONE_EN*/
                            /* tone_play_index_with_callback(IDEX_TONE_NORMAL, 1, audio_adt_dac_mute_start, NULL); */
                        }
#if SPEAK_TO_CHAT_TEST_TONE_EN
                        /*每次检测到说话都播放提示音*/
                        icsd_adt_tone_play(ICSD_ADT_TONE_NORMAL);
#endif /*SPEAK_TO_CHAT_TEST_TONE_EN*/

                        /*重新定时*/
                        if (hdl->timer) {
                            sys_s_hi_timeout_del(hdl->timer);
                            hdl->timer = 0;
                        }
                        hdl->timer = sys_s_hi_timerout_add(NULL, audio_speak_to_chat_timer, adt_info.speak_to_chat_end_time);
                        hdl->speak_to_chat_state = AUDIO_ADT_CHAT;
                        adt_info.speak_to_chat_state = AUDIO_ADT_CHAT;
                        hdl->voice_state = 0;
                    }
                    hdl->busy = 0;
                }
                break;
#endif /*TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE*/
#if TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE
            case ICSD_ADT_WIND_LVL:
                hdl = speak_to_chat_hdl;
                if (hdl && hdl->state) {
                    hdl->busy = 1;
                    u8 wind_lvl = (u8)msg[2];
                    /* printf("[task]wind_lvl : %d", wind_lvl); */
                    hdl->adt_wind_lvl = wind_lvl;

#if ICSD_ADT_WIND_INFO_SPP_DEBUG_EN
                    /*蓝牙spp发送风噪值*/
                    static u8 cnt = 0;
                    /*spp是否连接 && spp是否初始化 && spp发送是否初始化 && 每隔5次发送一次数据*/
                    if ((hdl->spp_connected_state == SPP_USER_ST_CONNECT) && hdl->spp_opt && hdl->spp_opt->send_data && (cnt++ > 5)) {
                        cnt = 0;
                        char tmpbuf[25];
                        memset(tmpbuf, 0x20, sizeof(tmpbuf));//填充空格
                        sprintf(tmpbuf, "wind lvl:%d", hdl->adt_wind_lvl);

#if TCFG_USER_TWS_ENABLE
                        if (get_tws_sibling_connect_state() && (tws_api_get_role() == TWS_ROLE_MASTER)) {
                            /*主机发送*/
                            hdl->spp_opt->send_data(NULL, tmpbuf, sizeof(tmpbuf));
                        } else
#endif /*TCFG_USER_TWS_ENABLE*/
                        {
                            hdl->spp_opt->send_data(NULL, tmpbuf, sizeof(tmpbuf));

                        }
                    }
#endif /*ICSD_ADT_WIND_INFO_SPP_DEBUG_EN*/
                    /*风噪处理*/
                    audio_anc_wind_noise_process(hdl->adt_wind_lvl);
                    hdl->busy = 0;
                }
                break;
#endif /*TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE*/
#if TCFG_AUDIO_WIDE_AREA_TAP_ENABLE
            case ICSD_ADT_WAT_RESULT:
                hdl = speak_to_chat_hdl;
                if (hdl && hdl->state) {
                    hdl->busy = 1;
                    u8 wat_result = (u8)msg[2];
                    printf("[task]wat_result : %d", wat_result);
                    hdl->adt_wat_result = wat_result;
                    audio_wat_area_tap_event_handle(hdl->adt_wat_result);
                    hdl->busy = 0;
                }
                break;
#endif /*TCFG_AUDIO_WIDE_AREA_TAP_ENABLE*/
            case ICSD_ADT_TONE_PLAY:
                /*播放提示音,定时器里面需要播放提示音时，可发消息到这里播放*/
                u8 index = (u8)msg[2];
                int err = icsd_adt_tone_play_callback(index, icsd_adt_task_play_tone_cb, NULL);
                if (hdl && (err != 0)) {
                    /*如果播放提示音失败，suspend提前置0*/
                    hdl->adt_suspend = 0;
                }
                break;
            case SPEAK_TO_CHAT_TASK_KILL:
                /*清掉队列消息*/
                os_taskq_flush();
                os_sem_post((OS_SEM *)msg[2]);
                break;
            case ICSD_ANC_MODE_SWITCH:
                anc_mode_switch((u8)msg[2], (u8)msg[3]);
                break;
            case SYNC_ICSD_ADT_OPEN:
                audio_icsd_adt_open((u8)msg[2]);
                break;
            case SYNC_ICSD_ADT_CLOSE:
                audio_icsd_adt_res_close((u8)msg[2]);
                break;
            case SYNC_ICSD_ADT_SET_ANC_FADE_GAIN:
                printf("set anc fade gain : %d", msg[2]);
                icsd_anc_fade_set(msg[2]);
                break;
            case ICSD_ADT_STATE_SYNC:
                printf("ICSD_ADT_STATE_SYNC");
                u8 *s_data = (u8 *)msg[2];
                /*先关闭adt，同步adt状态，然后同步anc，在anc里面做判断开adt*/
                audio_icsd_adt_state_sync_done(s_data[3], s_data[4]);
                anc_mode_sync(s_data);
                break;
            default:
                break;
            }
        }
    }
}

/*return
 * 0 : 不允许打开
 * 1 : 允许打开*/
static int audio_icsd_adt_open_permit()
{
    /*通话的时候不允许打开*/
    if (esco_player_runing()) {
        printf("esco open !!!");
        return 0;
    }
#if TCFG_ANC_TOOL_DEBUG_ONLINE
    /*连接anc spp 工具的时候不允许打开*/
    if (get_app_anctool_spp_connected_flag()) {
        printf("anctool spp connected !!!");
        return 0;
    }
#endif
    return 1;
}

int anc_adt_init()
{
    /*先创建任务，开关adt的操作在任务里面处理*/
    task_create(audio_icsd_adt_task, NULL, SPEAK_TO_CHAT_TASK_NAME);
    return 0;
}

int audio_icsd_adt_init()
{
    printf("%s", __func__);
    struct speak_to_chat_t *hdl = NULL;

    if (speak_to_chat_hdl) {
        printf("speak_to_chat_hdl  is areadly open !!");
        return 0;
    }
    speak_to_chat_hdl = zalloc(sizeof(struct speak_to_chat_t));
    if (speak_to_chat_hdl == NULL) {
        printf("speak_to_chat_hdl malloc fail !!!");
        adt_info.adt_mode = ADT_MODE_CLOSE;
        return -1;
    }
    hdl = speak_to_chat_hdl;

    hdl->last_anc_state = anc_mode_get();
    printf("last_anc_state %d", hdl->last_anc_state);

    /*设置anc使用的adt状态*/
    set_anc_adt_state(1);
    /* task_create(audio_icsd_adt_task, hdl, SPEAK_TO_CHAT_TASK_NAME); */
    audio_acoustic_detector_open();
    icsd_adt_clock_add();
    /*关闭蓝牙sniff*/
#if TCFG_USER_TWS_ENABLE
    icsd_bt_sniff_set_enable(0);
#endif
    hdl->state = 1;
    return 0;
}

int audio_icsd_adt_open(u8 adt_mode)
{
    printf("%s", __func__);
    if (!(~adt_info.adt_mode & adt_mode) && adt_mode) {
        /*判断传进来的模式中有哪些没有打开的*/
        return 0;
    }
    if (adt_info.adt_mode && adt_mode) {
        /*如果当前有其他模块已经打开时，需要关闭在重新打开*/
        adt_info.adt_mode |= adt_mode;
        audio_icsd_adt_res_close(0);
        return 0;
    }
    adt_info.adt_mode |= adt_mode;

    if (anc_mode_get() == ANC_ON) {
        adt_open_in_anc = 0;
        audio_icsd_adt_init();
    } else if (anc_mode_get() == ANC_OFF) {
        adt_open_in_anc = 1;
        anc_mode_switch_in_anctask(ANC_OFF, 0);
    } else if (anc_mode_get() == ANC_TRANSPARENCY) {
        adt_open_in_anc = 0;
        audio_icsd_adt_init();
    }
    return 0;
}

/*同步打开，
 *ag: audio_icsd_adt_sync_open(ADT_SPEAK_TO_CHAT_MODE | ADT_WIDE_AREA_TAP_MODE | ADT_WIND_NOISE_DET_MODE) */
int audio_icsd_adt_sync_open(u8 adt_mode)
{
    printf("%s", __func__);
    u8 data[4] = {0};
    data[0] = SYNC_ICSD_ADT_OPEN;
    data[1] = adt_mode;

    if (audio_icsd_adt_open_permit() == 0) {
        return -1;
    }
#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state()) {
        if ((tws_api_get_role() == TWS_ROLE_MASTER)) {
            /*同步主机状态打开*/
            audio_icsd_adt_info_sync(data, 4);
        }
    } else
#endif
    {
        audio_icsd_adt_info_sync(data, 4);
    }
    return 0;
}

/*同步关闭，
 *ag: audio_icsd_adt_sync_close(ADT_SPEAK_TO_CHAT_MODE | ADT_WIDE_AREA_TAP_MODE | ADT_WIND_NOISE_DET_MODE) */
int audio_icsd_adt_sync_close(u8 adt_mode)
{
    u8 data[4] = {0};
    data[0] = SYNC_ICSD_ADT_CLOSE;
    data[1] = adt_mode;
#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state()) {
        if ((tws_api_get_role() == TWS_ROLE_MASTER)) {
            /*同步主机状态打开*/
            audio_icsd_adt_info_sync(data, 4);
        }
    } else
#endif
    {
        audio_icsd_adt_info_sync(data, 4);
    }
    return 0;
}

/*获取是否需要先开anc再开免摘的状态*/
u8 get_adt_open_in_anc_state()
{
    return adt_open_in_anc;
}

int audio_speak_to_chat_open_in_anc_done()
{
    if (adt_open_in_anc) {
        adt_open_in_anc = 0;
        if (adt_info.adt_mode) {
            audio_icsd_adt_init();
        } else {
            audio_icsd_adt_res_close(0);
        }
    }
    return 0;
}

int audio_icsd_adt_close(u8 adt_mode)
{
    int err = 0;
    err = os_taskq_post_msg(SPEAK_TO_CHAT_TASK_NAME, 2, SYNC_ICSD_ADT_CLOSE, adt_mode);
    return err;
}

int audio_icsd_adt_res_close(u8 adt_mode)
{
    printf("%s", __func__);
    adt_info.adt_mode &= ~adt_mode;
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && hdl->state) {
        hdl->state = 0;
        while (hdl->busy) {
            putchar('w');
            os_time_dly(1);
        }
        audio_acoustic_detector_close();
        if (hdl->timer) {
            sys_s_hi_timeout_del(hdl->timer);
            hdl->timer = 0;
        }
        set_anc_adt_state(0);
        hdl->speak_to_chat_state = AUDIO_ADT_CLOSE;
        adt_info.speak_to_chat_state = AUDIO_ADT_CLOSE;

        /*恢复ANC状态*/
        if (hdl->last_anc_state == ANC_ON) {
            anc_mode_switch(ANC_ON, 0);
        } else if (hdl->last_anc_state == ANC_TRANSPARENCY) {
            anc_mode_switch(ANC_TRANSPARENCY, 0);
        } else {
            adt_open_in_anc = 0;
            //如果全部模式都关闭的时候，需要anc忽略相同模式的切换，关闭假anc off
            anc_mode_switch(ANC_OFF, 0);
        }

        /*关闭dac mute*/
        audio_adt_dac_mute_stop();

        /*恢复播歌状态*/
        if (hdl->a2dp_state && (bt_a2dp_get_status() != BT_MUSIC_STATUS_STARTING)) {
            printf("send PLAY cmd");
            bt_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
        }
        /*打开蓝牙sniff*/
#if TCFG_USER_TWS_ENABLE
        icsd_bt_sniff_set_enable(1);
#endif

        free(hdl);
        speak_to_chat_hdl = NULL;
        icsd_adt_clock_del();

    }
    /*如果adt没有全部关闭，需要重新打开
     *如果是通话时关闭的，直接关闭adt*/
    if (adt_info.adt_mode && !esco_player_runing()) {
        audio_icsd_adt_open(0);
    }
    return 0;
}

/*关闭所有模块*/
int audio_icsd_adt_close_all()
{
    u8 adt_mode = ADT_SPEAK_TO_CHAT_MODE | ADT_WIDE_AREA_TAP_MODE | ADT_WIND_NOISE_DET_MODE;
    audio_icsd_adt_close(adt_mode);
    return 0;
}

/*打开所有模块*/
int audio_icsd_adt_open_all()
{
    u8 adt_mode = ADT_SPEAK_TO_CHAT_MODE | ADT_WIDE_AREA_TAP_MODE | ADT_WIND_NOISE_DET_MODE;
    audio_icsd_adt_sync_open(adt_mode);
    return 0;
}

/*打开智能免摘*/
int audio_speak_to_chat_open()
{
    if (anc_mode_get() == ANC_ON) {
#if !(SPEAK_TO_CHAT_ANC_MODE_ENABLE & ANC_ON_BIT)
        /*不支持anc on下打开免摘*/
        return 0;
#endif
    } else if (anc_mode_get() == ANC_OFF) {
#if !(SPEAK_TO_CHAT_ANC_MODE_ENABLE & ANC_OFF_BIT)
        /*不支持anc off下打开免摘*/
        return 0;
#endif
    } else if (anc_mode_get() == ANC_TRANSPARENCY) {
        /*不支持通透下开免摘*/
#if !(SPEAK_TO_CHAT_ANC_MODE_ENABLE & ANC_TRANS_BIT)
        return 0;
#endif
    }
    printf("%s", __func__);
    u8 adt_mode = ADT_SPEAK_TO_CHAT_MODE;
    return audio_icsd_adt_sync_open(adt_mode);
}

/*关闭智能免摘*/
int audio_speak_to_chat_close()
{
    printf("%s", __func__);
    u8 adt_mode = ADT_SPEAK_TO_CHAT_MODE;
    return audio_icsd_adt_sync_close(adt_mode);
}

/*设置免摘定时结束的时间，单位ms*/
int audio_speak_to_char_end_time_set(u16 time)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && (adt_info.adt_mode & ADT_SPEAK_TO_CHAT_MODE)) {
        adt_info.speak_to_chat_end_time = time;
        return 0;
    } else {
        return -1;
    }
}

/*设置智能免摘检测的灵敏度*/
int audio_speak_to_chat_sensitivity_set(u8 sensitivity)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && (adt_info.adt_mode & ADT_SPEAK_TO_CHAT_MODE)) {
        return 0;
    } else {
        return -1;
    }
}

void audio_speak_to_chat_demo()
{
    printf("%s", __func__);
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (audio_icsd_adt_open_permit() == 0) {
        return;
    }

    /*判断智能免摘是否已经打开*/
    if ((adt_info.adt_mode & ADT_SPEAK_TO_CHAT_MODE) == 0) {
        if (anc_mode_get() == ANC_ON) {
#if !(SPEAK_TO_CHAT_ANC_MODE_ENABLE & ANC_ON_BIT)
            /*不支持anc on下打开免摘*/
            return;
#endif
        } else if (anc_mode_get() == ANC_OFF) {
#if !(SPEAK_TO_CHAT_ANC_MODE_ENABLE & ANC_OFF_BIT)
            /*不支持anc off下打开免摘*/
            return;
#endif
        } else if (anc_mode_get() == ANC_TRANSPARENCY) {
            /*暂不支持通透下开免摘*/
#if !(SPEAK_TO_CHAT_ANC_MODE_ENABLE & ANC_TRANS_BIT)
            return;
#endif
        }
        /*打开提示音*/
        icsd_adt_tone_play(ICSD_ADT_TONE_SPKCHAT_ON);
        audio_speak_to_chat_open();
    } else {
        /*关闭提示音*/
        icsd_adt_tone_play(ICSD_ADT_TONE_SPKCHAT_OFF);
        audio_speak_to_chat_close();
    }
}

/*打开广域点击*/
int audio_wat_click_open()
{
    printf("%s", __func__);
    u8 adt_mode = ADT_WIDE_AREA_TAP_MODE;
    return audio_icsd_adt_sync_open(adt_mode);
}

/*关闭广域点击*/
int audio_wat_click_close()
{
    printf("%s", __func__);
    u8 adt_mode = ADT_WIDE_AREA_TAP_MODE;
    return audio_icsd_adt_sync_close(adt_mode);
}

void audio_wide_area_tap_ingre_flag_timer(void *p)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && (adt_info.adt_mode & ADT_WIDE_AREA_TAP_MODE)) {
        /*恢复响应*/
        printf("wat ingore end");
        hdl->adt_wat_ignore_flag = 0;
    }
}

/*设置是否忽略广域点击
 * ignore: 1 忽略点击，0 响应点击
 * 忽略点击的时间，单位ms*/
int audio_wide_area_tap_ignore_flag_set(u8 ignore, u16 time)
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl && (adt_info.adt_mode & ADT_WIDE_AREA_TAP_MODE)) {
        hdl->adt_wat_ignore_flag = ignore;
        if (hdl->adt_wat_ignore_flag) {
            /*如果忽略点击，定时恢复响应*/
            printf("wat ingore start");
            sys_s_hi_timerout_add(NULL, audio_wide_area_tap_ingre_flag_timer, time);
        }
        return 0;
    } else {
        return -1;
    }
}

/*广域点击使用demo*/
void audio_wat_click_demo()
{
    printf("%s", __func__);
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (audio_icsd_adt_open_permit() == 0) {
        return;
    }

    if ((adt_info.adt_mode & ADT_WIDE_AREA_TAP_MODE) == 0) {
        /*打开提示音*/
        icsd_adt_tone_play(ICSD_ADT_TONE_WCLICK_ON);
        audio_wat_click_open();
    } else {
        /*关闭提示音*/
        icsd_adt_tone_play(ICSD_ADT_TONE_WCLICK_OFF);
        audio_wat_click_close();
    }
}

/*广域点击事件处理*/
void audio_wat_area_tap_event_handle(u8 wat_result)
{
    switch (wat_result) {
    case WIND_AREA_TAP_DOUBLE_CLICK:
        /*音乐暂停播放*/
        if ((bt_get_call_status() == BT_CALL_OUTGOING) ||
            (bt_get_call_status() == BT_CALL_ALERT)) {
            bt_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        } else if (bt_get_call_status() == BT_CALL_INCOMING) {
            bt_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        } else if (bt_get_call_status() == BT_CALL_ACTIVE) {
            bt_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        } else {
            bt_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
        }
        break;
    case WIND_AREA_TAP_THIRD_CLICK:
        /*anc切模式*/
        anc_mode_next();
        break;
    case WIND_AREA_TAP_MULTIPLE_CLICK:
        /* tone_play_index(IDEX_TONE_NUM_4, 0); */
        break;
    }
}


/*打开风噪检测*/
int audio_icsd_wind_detect_open()
{
    printf("%s", __func__);
    u8 adt_mode = ADT_WIND_NOISE_DET_MODE;
    return audio_icsd_adt_sync_open(adt_mode);
}

/*关闭风噪检测*/
int audio_icsd_wind_detect_close()
{
    printf("%s", __func__);
    u8 adt_mode = ADT_WIND_NOISE_DET_MODE;
    return audio_icsd_adt_sync_close(adt_mode);
}

/*获取风噪等级*/
u8 get_audio_icsd_wind_lvl()
{
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (hdl) {
        return hdl->adt_wind_lvl;
    } else {
        return 0;
    }
}

/*风噪检测使用demo*/
void audio_icsd_wind_detect_demo()
{
    printf("%s", __func__);
    struct speak_to_chat_t *hdl = speak_to_chat_hdl;
    if (audio_icsd_adt_open_permit() == 0) {
        return;
    }

    if ((adt_info.adt_mode & ADT_WIND_NOISE_DET_MODE) == 0) {
        /*打开提示音*/
        icsd_adt_tone_play(ICSD_ADT_TONE_WINDDET_ON);
        audio_icsd_wind_detect_open();
    } else {
        /*关闭提示音*/
        icsd_adt_tone_play(ICSD_ADT_TONE_WINDDET_OFF);
        audio_icsd_wind_detect_close();
    }
}

/*风噪输出等级:0~255*/
typedef struct {
    const u16 lvl1_thr;   //阈值1
    const u16 lvl2_thr;   //阈值2
    const u16 lvl3_thr;   //阈值3
    const u16 lvl4_thr;   //阈值4
    const u16 lvl5_thr;   //阈值5
    const u16 dithering_step; //消抖风噪等级间距
    u8 last_lvl;
    u8 cur_lvl;
} wind_lvl_det_t;
wind_lvl_det_t wind_lvl_det_anc = {
    .lvl1_thr = 30,
    .lvl2_thr = 60,
    .lvl3_thr = 90,
    .lvl4_thr = 120,
    .lvl5_thr = 150,
    .dithering_step = 10,
    .last_lvl = 0,
    .cur_lvl = 0,
};
/*划分风噪等级为 6三个等级*/
static u8 get_icsd_anc_wind_noise_lvl(wind_lvl_det_t *wind_lvl_det, u8 wind_lvl)
{

    if ((wind_lvl >= 0) && (wind_lvl <= (wind_lvl_det->lvl1_thr - wind_lvl_det->dithering_step))) {
        /*判断LVL0*/
        wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL0;
        wind_lvl_det->last_lvl = ANC_WIND_NOISE_LVL0;
    } else if ((wind_lvl > (wind_lvl_det->lvl1_thr - wind_lvl_det->dithering_step)) && (wind_lvl <= wind_lvl_det->lvl1_thr)) {
        /*LVL0和LVL1阈值处的消抖处理*/
        if (wind_lvl_det->last_lvl > ANC_WIND_NOISE_LVL0) {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL1;
        } else {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL0;
        }

    } else if ((wind_lvl > wind_lvl_det->lvl1_thr) && (wind_lvl <= (wind_lvl_det->lvl2_thr - wind_lvl_det->dithering_step))) {
        /*判断LVL1*/
        wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL1;
        wind_lvl_det->last_lvl = ANC_WIND_NOISE_LVL1;
    } else if ((wind_lvl > (wind_lvl_det->lvl2_thr - wind_lvl_det->dithering_step)) && (wind_lvl <= wind_lvl_det->lvl2_thr)) {
        /*LVL1和LVL2阈值处的消抖处理*/
        if (wind_lvl_det->last_lvl > ANC_WIND_NOISE_LVL1) {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL2;
        } else {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL1;
        }

    } else if ((wind_lvl > wind_lvl_det->lvl2_thr) && (wind_lvl <= (wind_lvl_det->lvl3_thr - wind_lvl_det->dithering_step))) {
        /*判断LVL2*/
        wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL2;
        wind_lvl_det->last_lvl = ANC_WIND_NOISE_LVL2;
    } else if ((wind_lvl > (wind_lvl_det->lvl3_thr - wind_lvl_det->dithering_step)) && (wind_lvl <= wind_lvl_det->lvl3_thr)) {
        /*LVL2和LVL3阈值处的消抖处理*/
        if (wind_lvl_det->last_lvl > ANC_WIND_NOISE_LVL2) {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL3;
        } else {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL2;
        }

    } else if ((wind_lvl > wind_lvl_det->lvl3_thr) && (wind_lvl <= (wind_lvl_det->lvl4_thr - wind_lvl_det->dithering_step))) {
        /*判断LVL3*/
        wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL3;
        wind_lvl_det->last_lvl = ANC_WIND_NOISE_LVL3;
    } else if ((wind_lvl > (wind_lvl_det->lvl4_thr - wind_lvl_det->dithering_step)) && (wind_lvl <= wind_lvl_det->lvl4_thr)) {
        /*LVL3和LVL4阈值处的消抖处理*/
        if (wind_lvl_det->last_lvl > ANC_WIND_NOISE_LVL3) {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL4;
        } else {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL3;
        }

    } else if ((wind_lvl > wind_lvl_det->lvl4_thr) && (wind_lvl <= (wind_lvl_det->lvl5_thr - wind_lvl_det->dithering_step))) {
        /*判断LVL4*/
        wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL4;
        wind_lvl_det->last_lvl = ANC_WIND_NOISE_LVL4;
    } else if ((wind_lvl > (wind_lvl_det->lvl5_thr - wind_lvl_det->dithering_step)) && (wind_lvl <= wind_lvl_det->lvl5_thr)) {
        /*LVL4和LVL5阈值处的消抖处理*/
        if (wind_lvl_det->last_lvl > ANC_WIND_NOISE_LVL4) {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL5;
        } else {
            wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL4;
        }

    } else {//if ((wind_lvl > MIDDLE_STRONG_WIND_LVL_THR) && (wind_lvl <= 300)) {
        /*判断LVL5*/
        wind_lvl_det->cur_lvl = ANC_WIND_NOISE_LVL5;
        wind_lvl_det->last_lvl = ANC_WIND_NOISE_LVL5;
    }
    return wind_lvl_det->cur_lvl;
}

typedef struct {
    u16 time;//定时器定时计算时间, ms
    u32 fade_timer;//计算定时器
    u32 wind_cnt;//记录单次定时器的风噪帧数
    u32 wind_eng;//记录风噪等级累加数字
    u8 last_lvl;//记录上一次风噪等级
    u8 preset_lvl;//记录当前检测到的风噪等级
    u8 fade_in_cnt;//记录风噪变大的次数
    u8 fade_out_cnt;//记录风噪变小的次数
    u8 wind_process_flag;//是否条件anc增益

    u8 fade_in_time;//设置淡入时间，单位s，误差1s
    u8 fade_out_time;//设置淡出时间，单位s，误差1s
    float ratio_thr;//设置判断阈值百分比，范围：0~1
} wind_info_t;
static wind_info_t wind_info_anc = {
    .time = 1000, //ms
    .fade_timer = 0,
    .wind_cnt = 0,
    .wind_eng = 0,
    .last_lvl = 0,
    .preset_lvl = 0,
    .fade_in_cnt = 0,
    .fade_out_cnt = 0,
    .wind_process_flag = 0,
    .fade_in_time = 4, //s
    .fade_out_time = 10, //s
    .ratio_thr = 0.8f,
};

void audio_adt_wn_process_fade_timer(void *p)
{
    wind_info_t *wind_info = (wind_info_t *)p;

    wind_info->fade_timer = 0;

    /*计算当前计算帧的风噪等级:求当前计算帧内的平均值*/
    wind_info->preset_lvl = (float)((float)wind_info->wind_eng / wind_info->wind_cnt) * (1.0 / wind_info->ratio_thr);
    printf("=========================cnt %d, eng %d, avg %d", wind_info->wind_cnt, wind_info->wind_eng, wind_info->preset_lvl);
    wind_info->wind_cnt = 0;
    wind_info->wind_eng = 0;
    if (wind_info->preset_lvl > wind_info->last_lvl) {
        /*与上一次比较，风噪变大*/
        wind_info->fade_in_cnt ++;
        wind_info->fade_out_cnt = 0;
    } else if (wind_info->preset_lvl < wind_info->last_lvl) {
        /*与上一次比较，风噪变小*/
        wind_info->fade_in_cnt = 0;
        wind_info->fade_out_cnt ++;;
    } else {
        /*与上一次比较，风噪可能是变大，也可能是变小*/
        wind_info->fade_in_cnt ++;
        wind_info->fade_out_cnt ++;;
    }

    /*判断是否到达淡入淡出的时间*/
    if (wind_info->fade_in_cnt >= (wind_info->fade_in_time * 1000 / wind_info->time)) {
        wind_info->fade_in_cnt = 0;
        wind_info->fade_out_cnt = 0;
        wind_info->wind_process_flag = 1;
    } else if (wind_info->fade_out_cnt >= (wind_info->fade_out_time * 1000 / wind_info->time)) {
        wind_info->fade_in_cnt = 0;
        wind_info->fade_out_cnt = 0;
        wind_info->wind_process_flag = 1;
    }

}

int audio_anc_wind_noise_process_fade(wind_info_t *wind_info, u8 anc_wind_noise_lvl)
{
    if (wind_info->fade_timer == 0) {
        wind_info->fade_timer = sys_s_hi_timerout_add(wind_info, audio_adt_wn_process_fade_timer, wind_info->time);
    }

    wind_info->wind_cnt ++;
    wind_info->wind_eng += anc_wind_noise_lvl;
    if (wind_info->wind_process_flag) {
        wind_info->wind_process_flag = 0;
        printf("anc_wind_noise_lvl %x : %d", (int)wind_info, wind_info->preset_lvl);
        wind_info->last_lvl = wind_info->preset_lvl;
        return wind_info->preset_lvl;
    }
    return 0;
}

/*anc风噪检测的处理*/
void audio_anc_wind_noise_process(u8 wind_lvl)
{
    u8 anc_wind_noise_lvl = 0;

    /*anc模式下才改anc增益*/
    if (anc_mode_get() == ANC_ON) {
        /*划分风噪等级*/
        anc_wind_noise_lvl = get_icsd_anc_wind_noise_lvl(&wind_lvl_det_anc, wind_lvl);
        /* printf(" ========== anc_wind_noise_lvl %d", anc_wind_noise_lvl); */

        /*做淡入淡出时间处理，返回0表示不做处理维持原来的增益不变*/
        anc_wind_noise_lvl = audio_anc_wind_noise_process_fade(&wind_info_anc, anc_wind_noise_lvl);
        if (anc_wind_noise_lvl == 0) {
            return;
        }
    } else {
        return;
    }

    u16 anc_fade_gain = 16384;
    /*根据风噪等级改变anc增益*/
    switch (anc_wind_noise_lvl) {
    case ANC_WIND_NOISE_LVL0:
        anc_fade_gain = 16384;
        break;
    case ANC_WIND_NOISE_LVL1:
        anc_fade_gain = 10000;
        break;
    case ANC_WIND_NOISE_LVL2:
        anc_fade_gain = 8000;
        break;
    case ANC_WIND_NOISE_LVL3:
        anc_fade_gain = 6000;
        break;
    case ANC_WIND_NOISE_LVL4:
        anc_fade_gain = 3000;
        break;
    case ANC_WIND_NOISE_LVL5:
        anc_fade_gain = 0;
        break;
    }

    u8 data[4] = {0};
    data[0] = SYNC_ICSD_ADT_SET_ANC_FADE_GAIN;
    data[1] = anc_fade_gain & 0xff;
    data[2] = (anc_fade_gain >> 8) & 0xff;
    audio_icsd_adt_info_sync(data, 4);
}

static u8 audio_speak_to_chat_idle_query()
{
    return speak_to_chat_hdl ? 0 : 1;
}

REGISTER_LP_TARGET(speak_to_chat_lp_target) = {
    .name = "speak_to_chat",
    .is_idle = audio_speak_to_chat_idle_query,
};

#endif /*(defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
