/*
 ****************************************************************
 *				   Audio Hearing-Aid
 * File  : audio_hearing_aid.c
 * By    :
 * Notes : DHA = Digital Hearing-Aid
 * Usage :
 ****************************************************************
 */

#include "generic/typedef.h"
#include "board_config.h"
#include "media/includes.h"
#include "audio_config.h"
#include "sound_device.h"
#include "audio_hearing_aid.h"
#include "howling_api.h"
#include "audio_codec_clock.h"
#include "application/eq_config.h"
#include "application/audio_drc.h"
#include "audio_dec_eff.h"
#include "audio_enc.h"
#include "audio_llns.h"
#include "audio_dvol.h"
#include "circular_buf.h"
#include "audio_gain_process.h"
#include "bt_tws.h"
#include "audio_noise_gate.h"
#include "amplitude_statistic.h"
#include "audio_hearing_aid_lp.h"
#include "media/audio_howling.h"
#if TCFG_AUTO_SHUT_DOWN_TIME
#include "bt_edr_fun.h"
#endif /*TCFG_AUTO_SHUT_DOWN_TIME*/


#if TCFG_AUDIO_HEARING_AID_ENABLE

extern struct audio_dac_hdl dac_hdl;
extern struct audio_adc_hdl adc_hdl;

#define HEARING_AID_TASK_NAME     		"HearingAid"

//*********************************************************************************//
//           数字信号处理配置(Digital Hearing_Aid[DHA] Process Config)             //
//*********************************************************************************//
/*mic声音淡入使能*/
#define DHA_FADEIN_ENABLE			1

/*均衡器*/
#define DHA_EQ_ENABLE				1

/*均衡器2*/
#define DHA_EQ2_ENABLE				0

/*动态范围控制*/
#define DHA_DRC_ENABLE				1

/*[啸叫抑制-移频法]Howling Suppress - FreqShift*/
#define DHA_HS_FS_ENABLE			1

/*[啸叫抑制-陷波法]Howling Suppress - Notch*/
#define DHA_HS_NOTCH_ENABLE			1

/*噪声抑制Noise Suppress*/
#define DHA_NS_ENABLE				0

/*变采样模块：如果输入输出采样率一致，条件编译自动关闭使能*/
#define DHA_SRC_ENABLE				1

/*音量控制模块*/
#define DHA_VOLUME_ENABLE			1

/*Noise-Gate噪声门限控制*/
#define DHA_NOISE_GATE_ENABLE		0

//*********************************************************************************//
//          						数据结构定义  					           	   //
//*********************************************************************************//
typedef struct {
    u16 mic_ch_sel;
    u16 sample_rate;
    u16 adc_irq_points;
    u16 adc_buf_num;
    u16	dac_delay;
    u16 mic_gain;
} hearing_adda_param_t;

/*Microphone采样率和采样点数配置*/
#if DHA_NS_ENABLE
#define HEARING_AID_CLK     	(160 * 1000000L)
#define DHA_MIC_SAMPLE_RATE		TCFG_AUDIO_DHA_MIC_SAMPLE_RATE
#define DHA_MIC_SAMPLE_POINT	(DHA_MIC_SAMPLE_RATE / 200)
#else
#define HEARING_AID_CLK     	(160 * 1000000L)
#define DHA_MIC_SAMPLE_RATE		TCFG_AUDIO_DHA_MIC_SAMPLE_RATE
#define DHA_MIC_SAMPLE_POINT	64
#endif/*DHA_NS_ENABLE*/
#if (DHA_MIC_SAMPLE_RATE == TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE)
/*输入输出采样率一致，条件编译自动关闭SRC使能*/
#undef DHA_SRC_ENABLE
#define DHA_SRC_ENABLE			0
#endif/*DHA_MIC_SAMPLE_RATE*/
static const hearing_adda_param_t hearing_adda_param = {
    .mic_ch_sel        = TCFG_AD2DA_LOW_LATENCY_MIC_CHANNEL,
    .mic_gain          = 12,
    .sample_rate       = DHA_MIC_SAMPLE_RATE,//采样率
    .adc_irq_points    = DHA_MIC_SAMPLE_POINT,//一次处理数据的数据单元， 单位点 4对齐(要配合mic起中断点数修改)
    .adc_buf_num       = 3,
#if (TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE == 44100)
    .dac_delay         = (DHA_MIC_SAMPLE_POINT *TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE / DHA_MIC_SAMPLE_RATE) /
    (TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE / 1000) + 5,//dac硬件混响延时， 单位ms 要大于 point_unit*2

#else
    .dac_delay         = (DHA_MIC_SAMPLE_POINT *TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE / DHA_MIC_SAMPLE_RATE) /
    (TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE / 1000) + 3,//dac硬件混响延时， 单位ms 要大于 point_unit*2
#endif
};


//*********************************************************************************//
//           mic能量检测处理配置             //
//*********************************************************************************//

typedef struct {
    struct audio_dac_channel dac_ch;
    struct adc_mic_ch mic_ch;
    struct audio_adc_output_hdl adc_output;
    s16 *adc_dma_buf;
    OS_SEM sem;
#if DHA_MIC_DATA_CBUF_ENABLE
    s16 *run_buf;
    u8 *data_buf;
    cbuffer_t mic_cbuf;
#else
    s16 *mic_data_addr;
    s32  mic_data_len;
#endif /*DHA_MIC_DATA_CBUF_ENABLE*/
    u8 task_release;
    u8 task_exit;
    volatile u8 task_busy;
    u16 fade_in_gain;
#if DHA_HS_FS_ENABLE
    HOWLING_API_STRUCT 		*hs_fs;
    HOWLING_API_STRUCT 		*hs_fs_1;
#endif /*DHA_HS_FS_ENABLE*/
#if DHA_HS_NOTCH_ENABLE
    HOWLING_API_STRUCT 		*hs_notch;
    HOWLING_API_STRUCT 		*hs_notch_1;
#endif /*DHA_HS_NOTCH_ENABLE*/
#if (DHA_EQ_ENABLE || DHA_DRC_ENABLE)
    struct dec_eq_drc       *hearing_eq_drc;
#endif /*(DHA_EQ_ENABLE || DHA_DRC_ENABLE)*/
#if (DHA_EQ2_ENABLE)
    struct audio_eq         *hearing_eq2;
#endif /*(DHA_EQ2_ENABLE)*/

#if DHA_NS_ENABLE
    void *llns;
    void *llns_1;
#endif /*DHA_NS_ENABLE*/
#if DHA_SRC_ENABLE
#if DHA_SRC_USE_HW_ENABLE
    struct audio_src_handle *hw_src;
#else
    s16 src_out_data[480];
#endif/*DHA_SRC_USE_HW_ENABLE*/
#endif /*DHA_SRC_ENABLE*/

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
    s16 mono_to_dual[(DHA_MIC_SAMPLE_POINT * TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE / DHA_MIC_SAMPLE_RATE) * 2 + 8];
#endif/*TCFG_AUDIO_DAC_CONNECT_MODE*/
    /*双声道处理双变单，单变双使用*/
    s16 *tmp_buf;
    s16 *tmp_buf_1;
    u8 channel;

} hearing_aid_t;
static hearing_aid_t *hearing_hdl = NULL;
static u32 start_ts = 0;
static u32 end_ts = 0;

enum {
    DHA_STATE_CLOSE,
    DHA_STATE_OPEN,
    DHA_STATE_SUSPEND,
};
typedef struct {
    volatile u8 state;  /*辅听状态*/
    volatile u8 fitting_state;  /*验配状态*/
    u8 fitting;			/*听力验配模式标识*/
    u8 volume;
    u16 timer;
} hearing_aid_schedule_t;
hearing_aid_schedule_t dha_schedule = {0};

/*保留EQ段，做自定义滤波使用*/
#define DHA_REV0_FREQ	100
#define DHA_REV1_FREQ	16000
#define  MIC_EQ_SECTION  8
static struct eq_seg_info audio_mic_eq_tab[MIC_EQ_SECTION] = {
    {0, EQ_IIR_TYPE_BAND_PASS, DHA_CH0_FREQ, 0, 1.0f},
    {1, EQ_IIR_TYPE_BAND_PASS, DHA_CH1_FREQ, 0, 1.0f},
    {2, EQ_IIR_TYPE_BAND_PASS, DHA_CH2_FREQ, 0, 1.4f},
    {3, EQ_IIR_TYPE_BAND_PASS, DHA_CH3_FREQ, 0, 1.4f},
    {4, EQ_IIR_TYPE_BAND_PASS, DHA_CH4_FREQ, 0, 1.4f},
    {5, EQ_IIR_TYPE_BAND_PASS, DHA_CH5_FREQ, 0, 2.0f},
    {6, EQ_IIR_TYPE_HIGH_PASS, DHA_REV0_FREQ, 0, 1.0f},
    {7, EQ_IIR_TYPE_LOW_PASS,  DHA_REV1_FREQ, 0, 0.7f},
};

/*听力验配频点列表*/
static u16 sine_freq[] = {
    DHA_CH0_FREQ,
    DHA_CH1_FREQ,
    DHA_CH2_FREQ,
    DHA_CH3_FREQ,
    DHA_CH4_FREQ,
    DHA_CH5_FREQ
};
static u8 sine_idx = 2;

static u16 dac_digital_vol = 0;
int audio_dac_set_digital_vol(struct audio_dac_hdl *dac, u16 vol);
u16 audio_dac_get_digital_vol(void)
{
    u16 vol = 0;
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_L)
    vol = JL_AUDIO->DAC_VL0 & 0x0000ffff;
#else
    vol = (JL_AUDIO->DAC_VL0 >> 16) & 0x0000ffff;
#endif
    return vol;
}

extern void put_float(double fv);
/*
*********************************************************************
*                  Audio Hearing-aid Fitting
* Description: Hearing Level Fitting
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : 听力验配
*********************************************************************
*/
dha_fitting_adjust_t dha_fitting_result[DHA_FITTING_CHANNEL_MAX];
int fitting_param_write(u8 LR, void *buf);
extern u32 hearing_aid_rcsp_response(u8 *data, u16 data_len);
extern u32 hearing_aid_rcsp_notify(u8 *data, u16 data_len);
static int hearing_aid_eq_filter_updata(float *EQGain);
int hearing_aid_fitting_parse(u8 *data, u16 len)
{
    u8 tmp[4];
    u8 cmd = data[0];
    u16 data_len = (data[2] << 8) | data[1];
    printf("cmd %x, data_len %d\n", cmd, data_len);
    switch (cmd) {
    case DHA_FITTING_CMD_INFO:
        break;
    case DHA_FITTING_CMD_ADJUST:
        printf("DHA_FITTING_CMD_ADJUST\n");
        dha_fitting_adjust_t adjust;
        if (data_len > sizeof(dha_fitting_adjust_t)) {
            /*数据长度错误*/
            printf("data_len err !!!");
            return 0;
        }
        memcpy(&adjust, &data[3], data_len);
        hearing_aid_fitting_start(adjust.sine);
        if (adjust.sine) {//开始验配，需要波单频音
            audio_dac_set_digital_vol(&dac_hdl, (u16)(16384 * adjust.gain / 100));
            printf("dval : %d\n", (u16)(16384 * adjust.gain / 100));
#if TCFG_USER_TWS_ENABLE
            enum audio_channel channel = tws_api_get_local_channel() == 'R' ? AUDIO_CH_R : AUDIO_CH_L;
            if ((channel == AUDIO_CH_L) && (adjust.channel == 0)) { //当前是左耳,并且数据是设置左耳时
                for (int i = 0; i < DHA_FITTING_CHANNEL_MAX; i++) {
                    if (audio_mic_eq_tab[i].freq == adjust.freq) {
                        /* audio_mic_eq_tab[i].gain = adjust.gain; */
                        sine_idx = i;
                        printf("L idx %d, freq %d, gain %d\n", sine_idx, audio_mic_eq_tab[i].freq, (int)adjust.gain);
                        break;
                    }
                }
            } else if ((channel == AUDIO_CH_R) && (adjust.channel == 1)) { //当前是右耳，并且数据是设置右耳时
                for (int i = 0; i < DHA_FITTING_CHANNEL_MAX; i++) {
                    if (audio_mic_eq_tab[i].freq == adjust.freq) {
                        /* audio_mic_eq_tab[i].gain = adjust.gain; */
                        sine_idx = i;
                        printf("R idx %d, freq %d, gain %d\n", sine_idx, audio_mic_eq_tab[i].freq, (int)adjust.gain);
                        break;
                    }
                }

            }
#else
            //没有TWS时,不判断左右耳直接设置单频音
            for (int i = 0; i < DHA_FITTING_CHANNEL_MAX; i++) {
                if (audio_mic_eq_tab[i].freq == adjust.freq) {
                    /* audio_mic_eq_tab[i].gain = adjust.gain; */
                    sine_idx = i;
                    printf("L idx %d, freq %d, gain %d\n", sine_idx, audio_mic_eq_tab[i].freq, (int)adjust.gain);
                    break;
                }
            }
#endif /*TCFG_USER_TWS_ENABLE*/
        }
        /*回复辅听状态信息*/
        get_hearing_aid_state_cmd_info(tmp);
        hearing_aid_rcsp_response(tmp, 4);
        break;
    case DHA_FITTING_CMD_UPDATE:
        printf("DHA_FITTING_CMD_UPDATE\n");
        float EQGain[12];
        if (data_len > sizeof(EQGain)) {
            /*数据长度错误*/
            printf("data_len err !!!");
            return 0;
        }
        /*回复辅听状态信息*/
        get_hearing_aid_state_cmd_info(tmp);
        hearing_aid_rcsp_response(tmp, 4);
        /*退出验配状态*/
        dha_schedule.fitting = 0;
        dha_schedule.fitting_state = 0;

        u8 LR = data[3]; //左右耳数据标志
        memcpy(EQGain, &data[4], data_len);
        /*更新验配数据到辅听*/
        hearing_aid_eq_filter_updata(EQGain);

        /*保存验配数据到vm*/
        fitting_param_write(LR, EQGain);

        for (int i = 0; i < data_len / 4; i++) {
            put_float(EQGain[i]);
        }
        /*恢复允许播歌*/
        if (get_hearing_aid_fitting_state() == 0) {
            extern void a2dp_tws_dec_resume(void);
            a2dp_tws_dec_resume();
        }
        break;
    }
    return 0;
}


/*获取辅听左右耳状态信息*/
int get_hearing_aid_state_cmd_info(u8 *data)
{
    dha_fitting_state_t dha_fitting_state;
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) { //对耳已经配对
        dha_fitting_state.state_left = dha_schedule.state == DHA_STATE_OPEN ? 1 : 0;
        dha_fitting_state.state_right = dha_schedule.state == DHA_STATE_OPEN ? 1 : 0;
    } else { /*tws 单只耳时*/
        enum audio_channel channel = tws_api_get_local_channel() == 'R' ? AUDIO_CH_R : AUDIO_CH_L;
        if (channel == AUDIO_CH_L) { //当前是左耳
            dha_fitting_state.state_left = dha_schedule.state == DHA_STATE_OPEN ? 1 : 0;
            dha_fitting_state.state_right = 0;
        } else if (channel == AUDIO_CH_R) { //当前是右耳
            dha_fitting_state.state_left = 0;
            dha_fitting_state.state_right = dha_schedule.state == DHA_STATE_OPEN ? 1 : 0;
        }
    }
#else /*没有开tws单只耳当作左耳*/
    dha_fitting_state.state_left = dha_schedule.state == DHA_STATE_OPEN ? 1 : 0;
    dha_fitting_state.state_right = 0;
#endif /*TCFG_USER_TWS_ENABLE*/
    data[0] = DHA_FITTING_CMD_STATE;
    data[1] = sizeof(dha_fitting_state);
    data[2] = 0;
    data[3] = ((u8 *)&dha_fitting_state)[0];
    put_buf(data, 4);
    return 0;
}

/*获取辅听验配状态*/
u8 get_hearing_aid_fitting_state(void)
{
    return dha_schedule.fitting || dha_schedule.fitting_state ? 1 : 0;
}

/*获取验配信息*/
int get_hearing_aid_fitting_info(u8 *data)
{
    hearing_aid_t *hdl = (hearing_aid_t *)hearing_hdl;
    if (data && hearing_hdl) {
        dha_fitting_info_t dha_info = {
            .version = DHA_FITTING_VERSION,
            .ch_num = DHA_FITTING_CHANNEL_MAX,
            .ch_freq[0] = DHA_CH0_FREQ,
            .ch_freq[1] = DHA_CH1_FREQ,
            .ch_freq[2] = DHA_CH2_FREQ,
            .ch_freq[3] = DHA_CH3_FREQ,
            .ch_freq[4] = DHA_CH4_FREQ,
            .ch_freq[5] = DHA_CH5_FREQ,
        };
        memcpy(data, &dha_info, sizeof(dha_info));
        return sizeof(dha_info);
    } else {
        return 0;
    }
}

/*判断是否播放单频音
 * en:
 * BIT(0) = 1 左耳播单频音，BIT(0) = 0 左耳静音
 * BIT(1) = 1 右耳播单频音，BIT(1) = 0 右耳静音
 * */
int hearing_aid_fitting_start(u8 en)
{
    hearing_aid_t *hdl = (hearing_aid_t *)hearing_hdl;
    printf("fitting en: %d", en);
    if (hdl) {
#if TCFG_USER_TWS_ENABLE
        if (en) { //需要播单频音
            enum audio_channel channel = tws_api_get_local_channel() == 'R' ? AUDIO_CH_R : AUDIO_CH_L;
            if (channel == AUDIO_CH_L) { //当前是左耳
                //判断是否设置左耳
                if (en & BIT(0)) { //设置左耳时左耳播放单频音
                    dha_schedule.fitting = 1;
                } else { //设置右耳时左耳静音
                    dha_schedule.fitting = 2;
                }
            } else if (channel == AUDIO_CH_R) { //当前是右耳
                //判断是否设置右耳
                if (en & BIT(1)) { //设置右耳时右耳播放单频音
                    dha_schedule.fitting = 1;
                } else { //设置左耳时右耳静音
                    dha_schedule.fitting = 2;
                }
            }
        } else { //关闭单频音
            dha_schedule.fitting = en;
        }
#else
        //没有TWS时
        dha_schedule.fitting = en ? 1 : 0;
#endif /*TCFG_USER_TWS_ENABLE*/
        if (dha_schedule.fitting == 0) {
            audio_dac_set_digital_vol(&dac_hdl, dac_digital_vol);
        }
        dha_schedule.fitting_state = dha_schedule.fitting;
        printf("dha_schedule.fitting : %d", dha_schedule.fitting);
        return dha_schedule.fitting;
    }
    return -1;
}
u8 set_hearing_aid_fitting_state(u8 state)
{
    dha_schedule.fitting_state = state;
    return dha_schedule.fitting_state;
}
/*同步关闭辅听验配*/
void audio_dha_fitting_sync_close(void)
{
#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state()) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            printf("[tws_master]fitting close");
            bt_tws_play_tone_at_same_time(SYNC_TONE_DHA_FITTING_CLOSE, 400);
        }
    } else {
        hearing_aid_fitting_start(0);
    }
#else
    hearing_aid_fitting_start(0);
#endif/*TCFG_USER_TWS_ENABLE*/
    set_hearing_aid_fitting_state(0);
}

/*
 * 保存验配参数
 * LR: 0 保存左耳，1 保存右耳，2 保存对耳
 * buf:需要保存的参数
 */
int fitting_param_write(u8 LR, void *buf)
{
    float EQGain[DHA_FITTING_CHANNEL_MAX * 2];
    int ret = 0;
    ret = syscfg_read(CFG_DHA_FITTING_ID, EQGain, DHA_FITTING_CHANNEL_MAX * 2 * 4);
    if (ret != DHA_FITTING_CHANNEL_MAX * 2 * 4) {
        printf("vm fitting param empty\n");
    }
    if (LR == 0) { /*写左耳数据*/
        memcpy(EQGain, (u8 *)buf, DHA_FITTING_CHANNEL_MAX * 4);
    } else if (LR == 1) { /*写右耳数据*/
        memcpy(EQGain + DHA_FITTING_CHANNEL_MAX, (u8 *)buf, DHA_FITTING_CHANNEL_MAX * 4);
    } else if (LR == 2) { /*写对耳数据*/
        memcpy(EQGain, (u8 *)buf, DHA_FITTING_CHANNEL_MAX * 2 * 4);
    }
    syscfg_write(CFG_DHA_FITTING_ID, EQGain, sizeof(EQGain));
    return 0;
}

/*读取当前耳机(左耳/右耳)验配参数*/
int fitting_param_read(void *buf)
{
    float EQGain[DHA_FITTING_CHANNEL_MAX * 2];
    int ret = 0;

    ret = syscfg_read(CFG_DHA_FITTING_ID, EQGain, DHA_FITTING_CHANNEL_MAX * 2 * 4);
    if (ret != DHA_FITTING_CHANNEL_MAX * 2 * 4) {
        printf("vm fitting param read err\n");
        return 0;
    }

    /*判断是否清除过数据,全是0xFF表示是清除过数据*/
    u8 *data = (u8 *)EQGain;
    u8 cnt = 0;
    for (u8 i = 0; i < 4; i++) {
        if (data[i] == 0xFF) {
            cnt++;
        }
    }
    if (cnt >= 4) {
        return 0;
    }

#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) { //对耳已经配对
        enum audio_channel channel = tws_api_get_local_channel() == 'R' ? AUDIO_CH_R : AUDIO_CH_L;
        if (channel == AUDIO_CH_L) { //当前是左耳,使用第一组数据
            memcpy((u8 *)buf, EQGain, DHA_FITTING_CHANNEL_MAX * 4);
        } else if (channel == AUDIO_CH_R) { //当前是右耳,使用第二组数据
            memcpy((u8 *)buf, EQGain + DHA_FITTING_CHANNEL_MAX, DHA_FITTING_CHANNEL_MAX * 4);
        }
    } else
#endif /*TCFG_USER_TWS_ENABLE*/
    {
        //没有配对时，使用第一组数据
        memcpy((u8 *)buf, EQGain, DHA_FITTING_CHANNEL_MAX * 4);
    }
    return ret >> 1;
}

/*清除验配参数
 * LR: 0 清除左耳，1 清除右耳，2 清除对耳
 */
int clear_fitting_patam(u8 LR)
{
    float EQGain[DHA_FITTING_CHANNEL_MAX * 2];
    int ret = 0;
    ret = syscfg_read(CFG_DHA_FITTING_ID, EQGain, DHA_FITTING_CHANNEL_MAX * 2 * 4);
    if (ret != DHA_FITTING_CHANNEL_MAX * 2 * 4) {
        printf("vm fitting param empty\n");
    }
    if (LR == 0) { /*清楚左耳参数*/
        memset(EQGain, 0xFF, DHA_FITTING_CHANNEL_MAX * 4);
    } else if (LR == 1) { /*清楚右耳参数*/
        memset(EQGain + DHA_FITTING_CHANNEL_MAX, 0xFF, DHA_FITTING_CHANNEL_MAX * 4);
    } else if (LR == 2) { /*清楚对耳参数*/
        memset(EQGain, 0xFF, DHA_FITTING_CHANNEL_MAX * 2 * 4);
    }
    syscfg_write(CFG_DHA_FITTING_ID, EQGain, sizeof(EQGain));
    return 0;

}

#if DHA_SRC_ENABLE
#include "Resample_api.h"
static RS_STUCT_API *sw_src_api = NULL;
static u8 *sw_src_buf = NULL;

int hearing_output(hearing_aid_t *hdl, s16 *data, u16 len);
static int hearing_aid_hw_src_output_handler(void *priv, s16 *data, int len);
static int hearing_src_init(hearing_aid_t *hdl, u8 nch, u16 insample, u16 outsample)
{
    printf("hearing_src_init,insr:%d,outsr:%d\n", insample, outsample);
#if DHA_SRC_USE_HW_ENABLE
    if (insample != TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE) {
        printf("hw src in %d, out %d\n", insample, TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE);
        hdl->hw_src = zalloc(sizeof(struct audio_src_handle));
        if (hdl->hw_src) {
            u8 channel = nch;
            audio_hw_src_open(hdl->hw_src, channel, AUDIO_RESAMPLE_SYNC_OUTPUT);
            audio_hw_src_set_rate(hdl->hw_src, insample, TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE);
            audio_src_set_output_handler(hdl->hw_src, hdl, (void (*)(void *, void *, int))hearing_output);
            printf("audio hw src open succ %x", hdl->hw_src);
        } else {
            printf("hdl->hw_src malloc fail !!!\n");
        }
    }
#else
    if (insample != outsample) {
        sw_src_api = get_rs16_context();
        /* sw_src_api = get_rsfast_context(); */
        printf("sw_src_api:0x%x\n", sw_src_api);
        ASSERT(sw_src_api);
        u32 sw_src_need_buf = sw_src_api->need_buf();
        printf("sw_src_buf:%d\n", sw_src_need_buf);
        sw_src_buf = zalloc(sw_src_need_buf);
        ASSERT(sw_src_buf);
        RS_PARA_STRUCT rs_para_obj;
        rs_para_obj.nch = nch;

        rs_para_obj.new_insample = insample;
        rs_para_obj.new_outsample = outsample;
        printf("sw src,in = %d,out = %d\n", rs_para_obj.new_insample, rs_para_obj.new_outsample);
        sw_src_api->open(sw_src_buf, &rs_para_obj);
    }
    return 0;
#endif/*DHA_SRC_USE_HW_ENABLE*/
}

static int hearing_src_run(hearing_aid_t *hdl, s16 *indata, s16 *outdata, u16 len)
{
#if DHA_SRC_USE_HW_ENABLE
    if (hdl->hw_src) {
        return audio_src_resample_write(hdl->hw_src, indata, len);
    }
#else
    int outlen = len;
    if (sw_src_api && sw_src_buf) {
        outlen = sw_src_api->run(sw_src_buf, indata, len >> 1, outdata);
        /* ASSERT(outlen <= (sizeof(outdata) >> 1));  */
        outlen = outlen << 1;
        /* printf("%d\n",outlen); */
    } else {
        memcpy(outdata, indata, len);
    }
    return outlen;
#endif/*DHA_SRC_USE_HW_ENABLE*/
}

static void hearing_src_exit(hearing_aid_t *hdl)
{
#if DHA_SRC_USE_HW_ENABLE
    printf("[HW]hearing_src_exit\n");
    if (hdl->hw_src) {
        audio_hw_src_stop(hdl->hw_src);
        audio_hw_src_close(hdl->hw_src);
        free(hdl->hw_src);
        hdl->hw_src = NULL;
    }
#else
    printf("[SW]hearing_src_exit\n");
    if (sw_src_buf) {
        free(sw_src_buf);
        sw_src_buf = NULL;
        sw_src_api = NULL;
    }
#endif/*DHA_SRC_USE_HW_ENABLE*/
}
#endif /*DHA_SRC_ENABLE*/

#if DHA_SRC_ENABLE
static void hearing_volume_run(s16 *data, int len);
static int hearing_aid_hw_src_output_handler(void *priv, s16 *data, int len)
{
    hearing_aid_t *hdl = (hearing_aid_t *)hearing_hdl;
    if (!hdl) {
        return 0;
    }

    s16 *mic_data = data;
    int data_len = len;
    int wlen = 0;

#if DHA_VOLUME_ENABLE
    hearing_volume_run(mic_data, data_len);
#endif/*DHA_VOLUME_ENABLE*/

#if DHA_RUN_TIME_TRACE_ENABLE
    end_ts = jiffies_msec();
    printf("run_time:%d - %d = %d\n", end_ts, start_ts, (end_ts - start_ts));
#endif/*DHA_RUN_TIME_TRACE_ENABLE*/

    DHA_IO_DEBUG_0();
    // 写入DAC
    wlen = sound_pcm_dev_write(&hdl->dac_ch, data, len);
    hdl->task_busy = 0;
    return len;
}
#endif /*DHA_SRC_ENABLE*/

#if DHA_NS_ENABLE
/*
 * 低延时降噪只支持16k 和32k
 * 16k采样率时,一帧处理80个点的数据(5ms)
 * 32k采样率时 一帧处理160个点的数据(5ms)
 */
static const u16 llns_frame_size[2][2] = {
    {16000, 160},//80点
    {32000, 320},//160点
    /* {44100, 440},//220点 */
    /* {48000, 480},//240点 */
};

typedef struct {
    char *private_buf;
    char *share_buf;
    s16 inbuf[160];
    s16 in_pool[320];
    cbuffer_t in_cbuf;
    int frame_size;
    void *llns;
} llns_hdl_t;
/* static llns_hdl_t *llns_hdl = NULL; */

static int llns_exit(void *hdl);
static void *llns_init(int sr, float gainfloor, float suppress_level)
{
    llns_hdl_t *llns_hdl = zalloc(sizeof(llns_hdl_t));
    if (llns_hdl == NULL) {
        printf("llns_hdl zalloc err !!!!\n");
        return -1;
    }
    cbuf_init(&llns_hdl->in_cbuf, llns_hdl->in_pool, sizeof(llns_hdl->in_pool));

    printf("%s %d", __func__, __LINE__);
    int private_heap_size, share_heap_size;
    audio_llns_heap_query(&share_heap_size, &private_heap_size,  sr);
    printf("private_heap_size:%d,share_heap_size:%d\n", private_heap_size, share_heap_size);
    llns_hdl->private_buf = (char *)zalloc(private_heap_size);
    llns_hdl->share_buf = (char *)zalloc(share_heap_size);

    /*判断是否支持的采样率*/
    llns_hdl->frame_size = 0;
    for (int i = 0; i < ARRAY_SIZE(llns_frame_size); i++) {
        //printf("llns_frame_size[%d] = %d",i,llns_frame_size[i][0]);
        if (llns_frame_size[i][0] == sr) {
            //printf("llns sr math:%d\n",sr);
            llns_hdl->frame_size = llns_frame_size[i][1];
        }
    }
    if (llns_hdl->frame_size == 0) {
        printf("samplerate only support 16k or 32k !!!\n");
        return -1;
    }

    printf("sr: %d, gainfloor: %d/100\n", sr, (int)(gainfloor * 100));
    llns_hdl->llns = audio_llns_init(llns_hdl->private_buf, private_heap_size, llns_hdl->share_buf, share_heap_size, sr, gainfloor, suppress_level);
    ASSERT(llns_hdl->llns);

    printf("llns init ok\n");
    return llns_hdl;
}

static int llns_run(void *hdl, s16 *data, int len)
{
    llns_hdl_t *llns_hdl = (llns_hdl_t *)hdl;
    int llns_outsize = 0, llns_outsize1 = 0;
    if (llns_hdl) {
        if (len != llns_hdl->frame_size) {
            putchar('B');
        }
        cbuf_write(&llns_hdl->in_cbuf, data, len);
        if (cbuf_read(&llns_hdl->in_cbuf, llns_hdl->inbuf, llns_hdl->frame_size) == llns_hdl->frame_size) {
            llns_outsize = audio_llns_run(llns_hdl->llns, llns_hdl->inbuf, llns_hdl->frame_size, data);
            //llns_outsize = len >> 1;
        }
    }
    return llns_outsize << 1;
}

static int llns_exit(void *hdl)
{
    llns_hdl_t *llns_hdl = (llns_hdl_t *)hdl;
    if (llns_hdl) {
        audio_llns_close(llns_hdl->llns);
        llns_hdl->llns = NULL;
        if (llns_hdl->private_buf) {
            free(llns_hdl->private_buf);
            llns_hdl->private_buf = NULL;
        }
        if (llns_hdl->share_buf) {
            free(llns_hdl->share_buf);
            llns_hdl->share_buf = NULL;
        }
        free(llns_hdl);
        llns_hdl = NULL;
    }
    return 0;
}
#endif /*DHA_NS_ENABLE*/


#if (DHA_EQ_ENABLE || DHA_DRC_ENABLE)
static int hearing_aid_eq_get_filter_info(void *eq, int sr, struct audio_eq_filter_info *info)
{
    struct audio_eq *eq_hdl = (struct audio_eq *)eq;
    if (!eq_hdl) {
        return -1;
    }
    local_irq_disable();
    u8 nsection = ARRAY_SIZE(audio_mic_eq_tab);
    if (!eq_hdl->eq_coeff_tab) {
        eq_hdl->eq_coeff_tab = zalloc(sizeof(int) * 5 * nsection);
    }
    for (int i = 0; i < nsection; i++) {
        eq_seg_design(&audio_mic_eq_tab[i], sr, &eq_hdl->eq_coeff_tab[5 * i]);
    }

    local_irq_enable();
    info->L_coeff = info->R_coeff = (void *)eq_hdl->eq_coeff_tab;
    info->L_gain = info->R_gain = 0;
    info->nsection = nsection;
    return 0;
}
/*限幅器drc*/
static float Gain_dB = 1;
static struct drc_ch mic_drc_p = {0};
static int hearing_aid_drc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    float th = -10.0f;//单位:dB 范围:-60dB~0dB,限幅器阈值
    Gain_dB = 7.0f; /*drc压制后要放大的dB数*/
    Gain_dB = powf(10.0f, Gain_dB / 20.0f);

    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768);
    mic_drc_p.nband = 1;
    mic_drc_p.type = 1; //1:限幅器
    mic_drc_p._p.limiter[0].attacktime = 5;
    mic_drc_p._p.limiter[0].releasetime = 500;
    mic_drc_p._p.limiter[0].threshold[0] = threshold;
    mic_drc_p._p.limiter[0].threshold[1] = 32768;
    info->R_pch = info->pch = &mic_drc_p;
    return 0;
}

#define FITTING_EQ_THR  30
static int hearing_aid_eq_filter_updata(float *EQGain)
{
    hearing_aid_t *hdl = (hearing_aid_t *)hearing_hdl;
    if (hdl == NULL) {
        return -1;
    }
    float max = 100 - FITTING_EQ_THR;
    for (int i = 0; i < DHA_FITTING_CHANNEL_MAX; i++) {
        if (EQGain[i] > FITTING_EQ_THR) {
            audio_mic_eq_tab[i].gain = (EQGain[i] - FITTING_EQ_THR) * 12.0f / max;
        } else {
            audio_mic_eq_tab[i].gain = 0;
        }
        put_float(audio_mic_eq_tab[i].gain);
    }

    struct dec_eq_drc *eq_drc = hdl->hearing_eq_drc;
    struct audio_eq *eq = eq_drc->eq;
    if (eq_drc == NULL || eq == NULL) {
        return -1;
    }
    eq->updata = 1;
    return 0;
}

void *hearing_eq_drc_open(u32 sample_rate, u8 ch_num)
{
#if TCFG_EQ_ENABLE

    struct dec_eq_drc *eff = zalloc(sizeof(struct dec_eq_drc));
    /* struct audio_eq_drc *eff = zalloc(sizeof(struct audio_eq_drc));  */
    struct audio_eq_param eq_param = {0};
    /* eff->priv = priv; */
    /* eff->out_cb = eq_output_cb; */

    eq_param.channels = ch_num;
    eq_param.online_en = 1;
    eq_param.mode_en = 0;
    eq_param.remain_en = 0;
    eq_param.no_wait = 0;
    eq_param.out_32bit = 0;
#if TCFG_AUDIO_DHA_FITTING_ENABLE
    eq_param.max_nsection = MIC_EQ_SECTION;
    eq_param.cb = hearing_aid_eq_get_filter_info;
#else
    eq_param.max_nsection = EQ_SECTION_MAX;
    eq_param.cb = eq_get_filter_info;
#endif /*TCFG_AUDIO_DHA_FITTING_ENABLE*/
    eq_param.eq_name = hearing_aid_mode;
    eq_param.sr = sample_rate;
    /* eq_param.priv = eff; */
    /* eq_param.output = eq_output; */
    eff->eq = audio_dec_eq_open(&eq_param);

#if (TCFG_DRC_ENABLE && DHA_DRC_ENABLE)
    struct audio_drc_param drc_param = {0};
    drc_param.sr = sample_rate;
    drc_param.channels = ch_num;
    drc_param.online_en = 1;
    drc_param.remain_en = 0;
    drc_param.out_32bit = 0;
#if DHA_USE_WDRC_ENABLE
    drc_param.cb = drc_get_filter_info;
#else
    drc_param.cb = hearing_aid_drc_get_filter_info;
#endif /*DHA_USE_WDRC_ENABLE*/
    drc_param.drc_name = hearing_aid_mode;
    eff->drc = audio_dec_drc_open(&drc_param);
    eff->async = 0;
#endif //TCFG_DRC_ENABLE

    return eff;
#else
    return NULL;
#endif//TCFG_EQ_ENABLE
}

void hearing_eq_drc_run(struct dec_eq_drc *eq_drc, void *data, u32 len)
{
    struct dec_eq_drc *eff_hdl = (struct dec_eq_drc *)eq_drc;
#if DHA_EQ_ENABLE
    if (eff_hdl->eq) {
        audio_eq_run(eff_hdl->eq, data, len);
    }
#endif/*DHA_EQ_ENABLE*/

#if DHA_DRC_ENABLE
    if (eff_hdl->drc) {
        audio_drc_run(eff_hdl->drc, data, len);
#if (DHA_USE_WDRC_ENABLE == 0)
        GainProcess_16Bit(data, data, Gain_dB, 1, 1, 1, len >> 1);
#endif /*DHA_USE_WDRC_ENABLE == 0*/
    }
#endif/*DHA_DRC_ENABLE*/
}
void hearing_eq_drc_close(struct dec_eq_drc *eq_drc)
{
#if TCFG_EQ_ENABLE
    struct dec_eq_drc *eff_hdl = (struct dec_eq_drc *)eq_drc;
    /* struct audio_eq_drc *eff_hdl = (struct audio_eq_drc *)eq_drc; */
    if (eff_hdl->eq) {
        audio_dec_eq_close(eff_hdl->eq);
        eff_hdl->eq = NULL;
    }

    if (eff_hdl->drc) {
        audio_dec_drc_close(eff_hdl->drc);
        eff_hdl->drc = NULL;
    }

    free(eff_hdl);
#endif
    return;
}
#endif /*(DHA_EQ_ENABLE || DHA_DRC_ENABLE)*/

#if (TCFG_EQ_ENABLE && DHA_EQ2_ENABLE)
void *hearing_eq2_open(u32 sample_rate, u8 ch_num)
{
    struct audio_eq *eq;
    struct audio_eq_param eq_param = {0};
    /* eff->priv = priv; */
    /* eff->out_cb = eq_output_cb; */

    eq_param.channels = ch_num;
    eq_param.online_en = 1;
    eq_param.mode_en = 0;
    eq_param.remain_en = 0;
    eq_param.no_wait = 0;
    eq_param.out_32bit = 0;
    eq_param.max_nsection = EQ_SECTION_MAX;
    eq_param.cb = eq_get_filter_info;
    eq_param.eq_name = hearing_aid_mode2;
    eq_param.sr = sample_rate;
    /* eq_param.priv = eff; */
    /* eq_param.output = eq_output; */
    eq = audio_dec_eq_open(&eq_param);
    return eq;

}

void hearing_eq2_run(struct audio_eq *eq, void *data, u32 len)
{
    if (eq) {
        audio_eq_run(eq, data, len);
    }
}

void hearing_eq2_close(struct audio_eq *eq)
{
    if (eq) {
        audio_dec_eq_close(eq);
        eq = NULL;
    }
}
#endif /*TCFG_EQ_ENABLE && DHA_EQ2_ENABLE*/

/*
*********************************************************************
*                  Audio Hearing-aid Fade In
* Description:
* Arguments  : data
*			   points
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
#define HEARING_AID_FADEIN_SHIFT	(14)
#define HEARING_AID_FADEIN_GAIN_MAX	(1 << HEARING_AID_FADEIN_SHIFT)
static void hearing_fade_in_run(s16 *data, u16 points)
{
    if (hearing_hdl->fade_in_gain < HEARING_AID_FADEIN_GAIN_MAX) {
        s32 tmp_data;
        for (int i = 0; i < points; i++) {
            tmp_data = data[i];
            data[i] = (tmp_data * hearing_hdl->fade_in_gain) >> HEARING_AID_FADEIN_SHIFT;
            if (++hearing_hdl->fade_in_gain >= HEARING_AID_FADEIN_GAIN_MAX) {
                return;
            }
        }
    }
}

#if DHA_VOLUME_ENABLE
/*
*********************************************************************
*                  Audio Hearing-aid Volume
* Description:
* Arguments  : data
*			   points
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
/*数字音量*/
static void hearing_volume_run(s16 *data, int len)
{
#if 0
    static u16 vol_dbg = 0;
    if (vol_dbg++ > 500) {
        vol_dbg = 0;
        printf("[DHA]Volume:%d\n", audio_digital_vol_get(HEARING_DVOL));
    }
#endif
    audio_digital_vol_run(HEARING_DVOL, data, len);
}
/*设置音量*/
static void hearing_volume_set(u8 volume)
{
    dha_schedule.volume = volume;
    audio_digital_vol_set(HEARING_DVOL, volume);
}
/*获取当前音量大小*/
static int hearing_volume_get(void)
{
    return audio_digital_vol_get(HEARING_DVOL);
}
/*打开数字音量*/
static void hearing_volume_open(u8 volume)
{
    dha_schedule.volume = (volume > HEARING_DVOL_MAX) ? HEARING_DVOL_MAX : volume;
    audio_digital_vol_open(HEARING_DVOL, dha_schedule.volume, HEARING_DVOL_MAX, HEARING_DVOL_FS, -1);
}
/*关闭数字音量*/
static void hearing_volume_close(void)
{
    audio_digital_vol_close(HEARING_DVOL);
}
#endif /*DHA_VOLUME_ENABLE*/

/*
*********************************************************************
*                  Audio Hearing-aid TDE
* Description: 全路径延时估计Time Delay Estimation
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/

typedef struct {
    u16 tick;
    u16 delay_points_max;
    u16 delay_time_max;
} hearing_aid_tde_t;
hearing_aid_tde_t dha_dte;

void hearing_tde_run(void)
{
    if (dha_dte.tick ++ > 2000) {
        dha_dte.tick = 0;
        extern int audio_dac_data_len();
        u16 time_unit = DHA_MIC_SAMPLE_RATE / 1000;
        u16 delay_points = audio_dac_data_len();
        u16 delay_time = (delay_points + (time_unit >> 1)) / time_unit;
        delay_time = delay_time + ((DHA_MIC_SAMPLE_POINT * 1000) / DHA_MIC_SAMPLE_RATE);
        if (delay_points > dha_dte.delay_points_max) {
            dha_dte.delay_points_max = delay_points;
        }
        if (delay_time > dha_dte.delay_time_max) {
            dha_dte.delay_time_max = delay_time;
        }
        printf("[DHA]delay_est:%d(points),%d(ms)\n", delay_points, delay_time);
        //printf("[DHA]delay_est_max:%d(points),%d(ms)\n",dha_dte.delay_points_max,dha_dte.delay_time_max);
    }
}

/*
*********************************************************************
*                  Audio Hearing-aid Mic Raw Data Handle
* Description:
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
static void hearing_mic_output_handler(void *priv, s16 *data, int len)
{
    hearing_aid_t *hdl = (hearing_aid_t *)priv;
    DHA_IO_INTERVAL();
    if (hdl == NULL) {
        return;
    }
    len = len * hdl->channel;
#if DHA_MIC_DATA_CBUF_ENABLE
    int wlen = cbuf_write(&hdl->mic_cbuf, data, len);
    if (wlen != len) {
        /* putchar('f'); */
        printf("hearing aid mic cbuf full !!!\n");
        /* audio_codec_clock_check(); */
    }
#else
    hdl->mic_data_addr = data;
    hdl->mic_data_len = len;
    /*判断上一次数据是否已经处理完了*/
    if (hdl->task_busy) {
        putchar('&');
        /* audio_codec_clock_check(); */
        /* printf("hearing aid need to increase sys_clock"); */
    }
#endif /*DHA_MIC_DATA_CBUF_ENABLE*/

    os_sem_set(&hdl->sem, 0);
    os_sem_post(&hdl->sem);
}
static int hearing_aid_mic_en(u8 en, hearing_adda_param_t *param)
{
    hearing_aid_t *hdl = (hearing_aid_t *)hearing_hdl;
    if (hdl == NULL) {
        printf("hearing_aid_mic_en error! hdl == NULL");
        return -1;;
    }
    if (en) {
        printf("dha mic en, ch %d, sr %d, gain %d", param->mic_ch_sel, param->sample_rate, param->mic_gain);
        u16 mic_ch = param->mic_ch_sel;
        u16 mic0_gain = param->mic_gain;
        u16 mic1_gain = param->mic_gain;
        u16 mic2_gain = param->mic_gain;
        u16 mic3_gain = param->mic_gain;

        audio_mic_pwr_ctl(MIC_PWR_ON);
        if (mic_ch & AUDIO_ADC_MIC_0) {
            audio_adc_mic_open(&hdl->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic_set_gain(&hdl->mic_ch, mic0_gain);
        }
        if (mic_ch & AUDIO_ADC_MIC_1) {
            audio_adc_mic1_open(&hdl->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic1_set_gain(&hdl->mic_ch, mic1_gain);
        }
        if (mic_ch & AUDIO_ADC_MIC_2) {
            audio_adc_mic2_open(&hdl->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic2_set_gain(&hdl->mic_ch, mic2_gain);
        }
        if (mic_ch & AUDIO_ADC_MIC_3) {
            audio_adc_mic3_open(&hdl->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic3_set_gain(&hdl->mic_ch, mic3_gain);
        }

        audio_adc_mic_set_sample_rate(&hdl->mic_ch, param->sample_rate);
        printf("adc_dma_buf: %x, size: %d", hdl->adc_dma_buf, param->adc_irq_points * 2 * param->adc_buf_num * hdl->channel);
        audio_adc_mic_set_buffs(&hdl->mic_ch, hdl->adc_dma_buf,
                                param->adc_irq_points * 2, param->adc_buf_num);
        hdl->adc_output.handler = hearing_mic_output_handler;
        hdl->adc_output.priv = hdl;
        audio_adc_add_output_handler(&adc_hdl, &hdl->adc_output);

        audio_adc_mic_start(&hdl->mic_ch);



    } else {
        audio_adc_mic_close(&hdl->mic_ch);
        audio_adc_del_output_handler(&adc_hdl, &hdl->adc_output);
        audio_mic_pwr_ctl(MIC_PWR_OFF);
    }
    return 0;

}

/*
*********************************************************************
*                  Audio Hearing-aid Output
* Description:
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
int hearing_output(hearing_aid_t *hdl, s16 *data, u16 len)
{
    s16 *mic_data = data;
    u16 data_len = len;
    int wlen = 0;
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
    if (hdl->channel == 1) {
        /*单变双*/
        for (u16 i = 0; i < (len >> 1); i++) {
            hdl->mono_to_dual[2 * i] 		= mic_data[i];
            hdl->mono_to_dual[2 * i + 1] 	= mic_data[i];
        }
        mic_data = hdl->mono_to_dual;
        data_len = len + data_len;
    }
#else  /*单声道*/
    if (hdl->channel == 2) {
        /*双变单*/
        for (u16 i = 0; i < (len >> 2); i++) {
            /*左右声道混合成单声道会变成杂音*/
            mic_data[i] = mic_data[2 * i];
            /* mic_data[i] = mic_data[2 * i + 1]; */
            /* mic_data[i] = ((int)mic_data[2 * i] + (int)mic_data[2 * i + 1]) / 2; */
        }
        data_len = len / 2;
    }
#endif/*TCFG_AUDIO_DAC_CONNECT_MODE*/

    // 写入DAC
    //putchar('o');
    int offset = 0;
    int cnt = 0;
    do {
        wlen = sound_pcm_dev_write(&hdl->dac_ch, mic_data + (offset >> 1), data_len - offset);
        offset = offset + wlen;
        cnt ++;
    } while ((offset != data_len) && (cnt < 1000));

    if (offset != data_len) {
        /* putchar('D'); */
        printf("dac write err :%d %d\n", data_len, offset);
    }
    return len;
}

/*
*********************************************************************
*                  Audio Hearing-aid Task
* Description:
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
static LOUDNESS_M_STRUCT dha_in_loudness;
static LOUDNESS_M_STRUCT dha_out_loudness;
static void audio_hearing_aid_task(void *p)
{
    int err = 0;
    int wlen = 0;
    s16 *mic_data = NULL;
    int data_len = 0;
    hearing_aid_t *hdl = (hearing_aid_t *)p;
    while (1) {
        err = os_sem_pend(&hdl->sem, 0);
        if (err || hdl->task_release) {
            break;
        }
#if DHA_RUN_TIME_TRACE_ENABLE
        start_ts = jiffies_msec();
#endif/*DHA_RUN_TIME_TRACE_ENABLE*/
        DHA_IO_DEBUG_1();

        hdl->task_busy = 1;

#if DHA_MIC_DATA_CBUF_ENABLE
        mic_data = hdl->run_buf;
        data_len = cbuf_read(&hdl->mic_cbuf, mic_data, DHA_MIC_SAMPLE_POINT * sizeof(short) * hdl->channel);
#else
        mic_data = hdl->mic_data_addr;
        data_len = hdl->mic_data_len;
#endif /*DHA_MIC_DATA_CBUF_ENABLE*/

        int clk = clk_get("sys");
        if (clk < HEARING_AID_CLK) {
            clk_set("sys", HEARING_AID_CLK);
        }

#if ((defined TCFG_AUDIO_DHA_LOW_POWER_ENABLE) && TCFG_AUDIO_DHA_LOW_POWER_ENABLE)
        audio_hearing_aid_lp_detect(NULL, mic_data, data_len);
#endif /*TCFG_AUDIO_DHA_LOW_POWER_ENABLE*/

#if DHA_IN_LOUDNESS_TRACE_ENABLE
        loudness_meter_short(&dha_in_loudness, mic_data, data_len >> 1);
#endif /*DHA_IN_LOUDNESS_TRACE_ENABLE*/


        //数据导出
#if DHA_DATA_EXPORT_ENABLE
        aec_uart_fill(0, mic_data, data_len);
#endif /*DHA_DATA_EXPORT_ENABLE*/

#if TCFG_AUDIO_DHA_FITTING_ENABLE
        if (dha_schedule.fitting) {
            if (dha_schedule.fitting == 1) {
                /*生成听力验配对应的频点声音*/
                extern void sin_pcm_fill(int fc, int fs, void *buf, u32 len);
                sin_pcm_fill(sine_freq[sine_idx], DHA_MIC_SAMPLE_RATE, mic_data, data_len);
            } else if (dha_schedule.fitting == 2) {
                memset(mic_data, 0, data_len);
            }
        } else
#endif /*TCFG_AUDIO_DHA_FITTING_ENABLE*/
        {
            // 算法处理

            if (hdl->channel == 1) {
#if DHA_HS_FS_ENABLE
                if (hdl->hs_fs) {
                    run_howling(hdl->hs_fs, mic_data, mic_data, data_len >> 1);
                }
#endif/*DHA_HS_FS_ENABLE*/
#if DHA_HS_NOTCH_ENABLE
                if (hdl->hs_notch) {
                    run_howling(hdl->hs_notch, mic_data, mic_data, data_len >> 1);
                }
#endif/*DHA_HS_NOTCH_ENABLE*/
            } else {
                /*双变单*/
                for (int i = 0; i < data_len / 4; i++) {
                    hdl->tmp_buf[i] = mic_data[2 * i];
                    hdl->tmp_buf_1[i] = mic_data[2 * i + 1];
                }
#if DHA_HS_FS_ENABLE
                if (hdl->hs_fs && hdl->hs_fs_1) {
                    run_howling(hdl->hs_fs, hdl->tmp_buf, hdl->tmp_buf, data_len >> 2);
                    run_howling(hdl->hs_fs_1, hdl->tmp_buf_1, hdl->tmp_buf_1, data_len >> 2);
                }
#endif/*DHA_HS_FS_ENABLE*/
#if DHA_HS_NOTCH_ENABLE
                if (hdl->hs_notch && hdl->hs_notch_1) {
                    run_howling(hdl->hs_notch, hdl->tmp_buf, hdl->tmp_buf, data_len >> 2);
                    run_howling(hdl->hs_notch_1, hdl->tmp_buf_1, hdl->tmp_buf_1, data_len >> 2);
                }
#endif/*DHA_HS_NOTCH_ENABLE*/
                /*单变双*/
                for (int i = 0; i < data_len / 4; i++) {
                    mic_data[2 * i] = hdl->tmp_buf[i];
                    mic_data[2 * i + 1] = hdl->tmp_buf_1[i];
                }
            }


#if (DHA_EQ_ENABLE || DHA_DRC_ENABLE)
            if (hdl->hearing_eq_drc) {
                hearing_eq_drc_run(hdl->hearing_eq_drc, mic_data, data_len);
            }
#endif/*DHA_EQ_ENABLE || DHA_DRC_ENABLE*/

#if (TCFG_EQ_ENABLE && DHA_EQ2_ENABLE)
            if (hdl->hearing_eq2) {
                hearing_eq2_run(hdl->hearing_eq2, mic_data, data_len);
            }
#endif /*TCFG_EQ_ENABLE && DHA_EQ2_ENABLE*/

#if DHA_NOISE_GATE_ENABLE
            audio_noise_gate_run(mic_data, mic_data, data_len);
#endif/*DHA_NOISE_GATE_ENABLE*/

#if DHA_NS_ENABLE
            if (hdl->channel == 1) {
                if (hdl->llns) {
                    data_len = llns_run(hdl->llns, mic_data, data_len);
                }
            } else {
                /*双变单*/
                for (int i = 0; i < data_len / 4; i++) {
                    hdl->tmp_buf[i] = mic_data[2 * i];
                    hdl->tmp_buf_1[i] = mic_data[2 * i + 1];
                }
                if (hdl->llns) {
                    llns_run(hdl->llns, hdl->tmp_buf, data_len / 2);
                }
                if (hdl->llns_1) {
                    llns_run(hdl->llns_1, hdl->tmp_buf_1, data_len / 2);
                }
                /*单变双*/
                for (int i = 0; i < data_len / 4; i++) {
                    mic_data[2 * i] = hdl->tmp_buf[i];
                    mic_data[2 * i + 1] = hdl->tmp_buf_1[i];
                }
            }
#endif /*DHA_NS_ENABLE*/

#if DHA_FADEIN_ENABLE
            hearing_fade_in_run(mic_data, (data_len >> 1));
#endif/*DHA_FADEIN_ENABLE*/
        }

#if DHA_VOLUME_ENABLE
        hearing_volume_run(mic_data, data_len);
#endif/*DHA_VOLUME_ENABLE*/

        //数据导出
#if DHA_DATA_EXPORT_ENABLE
        aec_uart_fill(1, mic_data, data_len);
        aec_uart_write();
#endif /*DHA_DATA_EXPORT_ENABLE*/

#if DHA_OUT_LOUDNESS_TRACE_ENABLE
        loudness_meter_short(&dha_out_loudness, mic_data, data_len >> 1);
#endif /*DHA_OUT_LOUDNESS_TRACE_ENABLE*/

#if DHA_SRC_ENABLE
#if DHA_SRC_USE_HW_ENABLE
        hearing_src_run(hdl, mic_data, NULL, data_len);
        continue;
#else
        data_len = hearing_src_run(hdl, mic_data, hdl->src_out_data, data_len);
        if (data_len) {
            mic_data = hdl->src_out_data;
        }
#endif/*DHA_SRC_USE_HW_ENABLE*/
#endif /*DHA_SRC_ENABLE*/

        //result output
        hearing_output(hdl, mic_data, data_len);

#if DHA_TDE_ENABLE
        hearing_tde_run();
#endif/*DHA_TDE_ENABLE*/

#if DHA_RUN_TIME_TRACE_ENABLE
        end_ts = jiffies_msec();
        printf("run_time:%d - %d = %d\n", end_ts, start_ts, (end_ts - start_ts));
#endif/*DHA_RUN_TIME_TRACE_ENABLE*/

        DHA_IO_DEBUG_0();

        hdl->task_busy = 0;
    }
    hdl->task_exit = 1;
    while (1) {
        os_time_dly(100);
    }
}

/*
*********************************************************************
*                  Audio Hearing-aid Open
* Description:
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
int audio_hearing_aid_open(void)
{
    printf("audi_hearing_aid_open\n");
    int err = 0;
    hearing_adda_param_t *param = &hearing_adda_param;
    hearing_aid_t *hdl = hearing_hdl;
    if (hdl != NULL) {
        printf("hdl != NULL!");
        return err;
    }

#if TCFG_AUDIO_DHA_AND_MUSIC_MUTEX
    /*播歌互斥时播歌不开辅听*/
    extern u8 bt_media_is_running(void);
    if (bt_media_is_running()) {
        dha_schedule.state = DHA_STATE_SUSPEND;
        return 0;
    }
#endif /*TCFG_AUDIO_DHA_AND_MUSIC_MUTEX*/
#if TCFG_AUDIO_DHA_AND_CALL_MUTEX
    /*通话互斥时通话不开辅听*/
    extern u8 bt_phone_dec_is_running();
    if (bt_phone_dec_is_running()) {
        dha_schedule.state = DHA_STATE_SUSPEND;
        return 0;
    }
#endif /*TCFG_AUDIO_DHA_AND_CALL_MUTEX*/
#if TCFG_AUDIO_DHA_AND_TONE_MUTEX
    /*提示音互斥时播提示音不开辅听*/
    extern int tone_get_status();
    extern int sine_get_status();
    if (tone_get_status() || sine_get_status()) {
        dha_schedule.state = DHA_STATE_SUSPEND;
        return 0;
    }
#endif /*TCFG_AUDIO_DHA_AND_TONE_MUTEX*/

#if TCFG_AUTO_SHUT_DOWN_TIME
    sys_auto_shut_down_disable();
#endif/*TCFG_AUTO_SHUT_DOWN_TIME*/
    mem_stats();

    hdl = zalloc(sizeof(hearing_aid_t));
    if (hdl == NULL) {
        printf("hdl malloc error!");
        goto __err;
    }
    hearing_hdl = hdl;

    hdl->channel = 0;
    u8 mic_ch = hearing_adda_param.mic_ch_sel;
    if (mic_ch & AUDIO_ADC_MIC_0) {
        hdl->channel++;
    }
    if (mic_ch & AUDIO_ADC_MIC_1) {
        hdl->channel++;
    }
    if (mic_ch & AUDIO_ADC_MIC_2) {
        hdl->channel++;
    }
    if (mic_ch & AUDIO_ADC_MIC_3) {
        hdl->channel++;
    }
    printf(" hdl channel %d", hdl->channel);
    u32 adc_dma_size = param->adc_irq_points * param->adc_buf_num * sizeof(short) * hdl->channel;
    if (adc_dma_size == 0) {
        printf("adc_dma_size == NULL!");
        goto __err;
    }
    hdl->adc_dma_buf = zalloc(adc_dma_size);
    if (hdl->adc_dma_buf == NULL) {
        printf("hdl->adc_dma_buf malloc error!");
        goto __err;
    }
#if DHA_MIC_DATA_CBUF_ENABLE
    hdl->run_buf = zalloc(DHA_MIC_SAMPLE_POINT * sizeof(short) * hdl->channel);
    hdl->data_buf = zalloc(DHA_MIC_SAMPLE_POINT * sizeof(short) * hdl->channel * 3);
    cbuf_init(&hdl->mic_cbuf, hdl->data_buf, DHA_MIC_SAMPLE_POINT * sizeof(short) * hdl->channel * 3);
#endif /*DHA_MIC_DATA_CBUF_ENABLE*/

    if (hdl->channel == 2) {
        hdl->tmp_buf   = zalloc(DHA_MIC_SAMPLE_POINT * sizeof(short));
        hdl->tmp_buf_1 = zalloc(DHA_MIC_SAMPLE_POINT * sizeof(short));
    }
    clk_set("sys", HEARING_AID_CLK);
    /* audio_codec_clock_set(HEARING_AID_MODE, AUDIO_CODING_PCM, 0); */

#if TCFG_AUDIO_DHA_FITTING_ENABLE
    /*读取验配参数*/
    float EQGain[DHA_FITTING_CHANNEL_MAX];
    err = fitting_param_read(EQGain);
    if (err == DHA_FITTING_CHANNEL_MAX * 4) {
        put_buf(EQGain, DHA_FITTING_CHANNEL_MAX * 4);
        /*更新验配数据到辅听*/
        hearing_aid_eq_filter_updata(EQGain);
    } else {
        printf("default fitting param !!!\n");
    }
#endif /*TCFG_AUDIO_DHA_FITTING_ENABLE*/

#if DHA_IN_LOUDNESS_TRACE_ENABLE
    loudness_meter_init(&dha_in_loudness, param->sample_rate, 50, 0);
#endif /*DHA_IN_LOUDNESS_TRACE_ENABLE*/
#if DHA_OUT_LOUDNESS_TRACE_ENABLE
    loudness_meter_init(&dha_out_loudness, param->sample_rate, 50, 1);
#endif /*DHA_IN_LOUDNESS_TRACE_ENABLE*/

    //memcpy(&hdl->param, param, sizeof(struct __ad2da_low_latency_param));

    os_sem_create(&hdl->sem, 0);
    err = task_create(audio_hearing_aid_task, (void *)hdl, HEARING_AID_TASK_NAME);
    if (err != OS_NO_ERR) {
        printf("task create error!");
        goto __err;
    }
    dha_schedule.fitting = 0;
    dha_schedule.fitting_state = 0;
    //mem_stats();

#if ((defined TCFG_AUDIO_DHA_LOW_POWER_ENABLE) && TCFG_AUDIO_DHA_LOW_POWER_ENABLE)
    audio_hearing_aid_lp_open(1, audio_hearing_aid_open, audio_hearing_aid_close);
#endif /*TCFG_AUDIO_DHA_LOW_POWER_ENABLE*/

    //数据导出
#if DHA_DATA_EXPORT_ENABLE
    aec_uart_open(2, param->adc_irq_points * 2);
#endif /*DHA_DATA_EXPORT_ENABLE*/

    // 算法初始化
#if DHA_HS_FS_ENABLE
    ///啸叫抑制初始化
    hdl->hs_fs = open_howling(NULL, param->sample_rate, 0, 1);//mode 1:移频
    printf("open_howling %x\n\n\n", hdl->hs_fs);
    if (hdl->channel == 2) {
        hdl->hs_fs_1 = open_howling(NULL, param->sample_rate, 0, 1);//mode 1:移频
        printf("open_howling_1 %x\n\n\n", hdl->hs_fs_1);
    }
    //mem_stats();
#endif/*DHA_HS_FS_ENABLE*/

#if DHA_HS_NOTCH_ENABLE
    HOWLING_PARM_SET howling_param_default = {
        .threshold = 25,
        .sample_rate = param->sample_rate,
        .channel = 1,
        .fade_time = 10,
        .notch_Q = 2.0f,
        .notch_gain = -20.0f,
    };
    hdl->hs_notch = open_howling(&howling_param_default, param->sample_rate, 0, 0);//mode 0:陷波
    printf("open_notch howling %x\n\n\n", hdl->hs_notch);
    if (hdl->channel == 2) {
        hdl->hs_notch_1 = open_howling(&howling_param_default, param->sample_rate, 0, 0);//mode 0:陷波
        printf("open_notch howling_1 %x\n\n\n", hdl->hs_notch_1);
    }
    //mem_stats();
#endif/*DHA_HS_NOTCH_ENABLE*/

#if DHA_NOISE_GATE_ENABLE
    /*限幅器上限*/
#define LIMITER_THR	 -3000 /*-12000 = -12dB,放大1000倍,(-10000参考)*/
    /*小于CONST_NOISE_GATE的当成噪声处理,防止清0近端声音*/
#define LIMITER_NOISE_GATE  -40000 /*-12000 = -12dB,放大1000倍,(-30000参考)*/
    /*低于噪声门限阈值的增益 */
#define LIMITER_NOISE_GAIN  ((int) (0.2 * (1 << 30))) /*(0~1)*2^30*/
    audio_noise_gate_open(param->sample_rate, LIMITER_THR, LIMITER_NOISE_GATE, LIMITER_NOISE_GAIN);
#endif/*DHA_NOISE_GATE_ENABLE*/

#if (DHA_EQ_ENABLE || DHA_DRC_ENABLE)
    hdl->hearing_eq_drc = hearing_eq_drc_open(param->sample_rate, hdl->channel);
    printf("hearing_eq_drc open %x\n\n\n", hdl->hearing_eq_drc);
    //mem_stats();
#endif/*DHA_EQ_ENABLE || DHA_DRC_ENABLE*/

#if (TCFG_EQ_ENABLE && DHA_EQ2_ENABLE)
    hdl->hearing_eq2 = hearing_eq2_open(param->sample_rate, hdl->channel);
    printf("hearing_eq2 open %x\n\n\n", hdl->hearing_eq2);
#endif /*TCFG_EQ_ENABLE && DHA_EQ2_ENABLE*/

#if DHA_NS_ENABLE
    hdl->llns = llns_init(param->sample_rate, 0.05f, 1.0f);
    if (hdl->channel == 2) {
        hdl->llns_1 = llns_init(param->sample_rate, 0.05f, 1.0f);
    }
    //mem_stats();
#endif /*DHA_NS_ENABLE*/

#if DHA_SRC_ENABLE
    hearing_src_init(hdl, hdl->channel, param->sample_rate, TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE);
    //mem_stats();
#endif /*DHA_SRC_ENABLE*/

    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());
    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, get_max_sys_vol(), 1);

#if DHA_VOLUME_ENABLE
    hearing_volume_open(HEARING_DVOL_MAX);
#endif /*DHA_VOLUME_ENABLE*/

    // DAC 初始化
    audio_dac_new_channel(&dac_hdl, &hdl->dac_ch);
    struct audio_dac_channel_attr attr;
    attr.delay_time = param->dac_delay;
    attr.protect_time = 8;
    attr.write_mode = WRITE_MODE_FORCE;
    audio_dac_channel_set_attr(&hdl->dac_ch, &attr);
    sound_pcm_dev_start(&hdl->dac_ch, TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE, app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
#if DHA_DAC_OUTPUT_ENHANCE_ENABLE
    //DAC输出音量增强使能
    app_audio_dac_vol_mode_set(1);
#endif /*DHA_DAC_OUTPUT_ENHANCE_ENABLE*/

    dac_digital_vol = audio_dac_get_digital_vol();
    printf("dac_digital_vol : %d\n",  dac_digital_vol);

#if DHA_TDE_ENABLE
    memset(&dha_dte, 0, sizeof(dha_dte));
#endif/*DHA_TDE_ENABLE*/

    /* hearing_volume_set(14); */
    DHA_IO_DEBUG_INIT();

    // ADC 初始化
    hearing_aid_mic_en(1, param);
    dha_schedule.state = DHA_STATE_OPEN;
    printf("audio_hearing_aid_open success!");
    mem_stats();
    return 0;
__err:
    if (hdl) {
        if (hdl->adc_dma_buf) {
            free(hdl->adc_dma_buf);
        }
        free(hdl);
    }
    hearing_hdl = NULL;

#if TCFG_AUTO_SHUT_DOWN_TIME
    sys_auto_shut_down_enable();
#endif/*TCFG_AUTO_SHUT_DOWN_TIME*/

    printf("audio_hearing_aid_open error!");
    return -1;
}

void audio_hearing_aid_sync_open(void)
{
#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state()) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            printf("[tws_master]dha open");
            bt_tws_play_tone_at_same_time(SYNC_TONE_HEARING_AID_OPEN, 400);
        }
    } else {
        audio_hearing_aid_open();
    }
#else
    audio_hearing_aid_open();
#endif/*TCFG_USER_TWS_ENABLE*/

}


/*
*********************************************************************
*                  Audio Hearing-aid Close
* Description:
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
int audio_hearing_aid_close(void)
{
    int err = 0;
    hearing_aid_t *hdl = (hearing_aid_t *)hearing_hdl;

    printf("[DHA]audio_hearing_aid_close\n");
    if (hdl == NULL) {
        if (dha_schedule.state == DHA_STATE_SUSPEND) {
            printf("[DHA]hearing-aid suspend now\n");
            dha_schedule.state = DHA_STATE_CLOSE;
            return 0;
        }
        printf("audio_hearing_aid_close error! hdl == NULL");
        return -1;
    }

    // ADC 关闭
    hearing_aid_mic_en(0, NULL);

    hdl->task_release = 1;
    os_sem_set(&hdl->sem, 0);
    os_sem_post(&hdl->sem);

    while (hdl->task_exit == 0) {
        os_time_dly(1);
    }

    err = task_kill(HEARING_AID_TASK_NAME);
    os_sem_del(&hdl->sem, 0);
    // DAC 关闭
    sound_pcm_dev_stop(&hdl->dac_ch);
#if DHA_DAC_OUTPUT_ENHANCE_ENABLE
    app_audio_dac_vol_mode_set(0);
#endif /*DHA_DAC_OUTPUT_ENHANCE_ENABLE*/

#if DHA_HS_FS_ENABLE
    if (hdl->hs_fs) {
        printf("close_howling\n\n\n");
        close_howling(hdl->hs_fs);
    }
    if (hdl->hs_fs_1) {
        printf("close_howling_1\n\n\n");
        close_howling(hdl->hs_fs_1);
    }
#endif/*DHA_HS_FS_ENABLE*/

#if DHA_HS_NOTCH_ENABLE
    if (hdl->hs_notch) {
        printf("close_howling\n\n\n");
        close_howling(hdl->hs_notch);
    }
    if (hdl->hs_notch_1) {
        printf("close_howling\n\n\n");
        close_howling(hdl->hs_notch_1);
    }
#endif/*DHA_HS_NOTCH_ENABLE*/

#if DHA_NOISE_GATE_ENABLE
    audio_noise_gate_close();
#endif/*DHA_NOISE_GATE_ENABLE*/

#if (DHA_EQ_ENABLE || DHA_DRC_ENABLE)
    if (hdl->hearing_eq_drc) {
        hearing_eq_drc_close(hdl->hearing_eq_drc);
    }
#endif/*DHA_EQ_ENABLE || DHA_DRC_ENABLE*/

#if (TCFG_EQ_ENABLE && DHA_EQ2_ENABLE)
    if (hdl->hearing_eq2) {
        hearing_eq2_close(hdl->hearing_eq2);
    }
#endif /*TCFG_EQ_ENABLE && DHA_EQ2_ENABLE*/

#if DHA_NS_ENABLE
    if (hdl->llns) {
        llns_exit(hdl->llns);
        hdl->llns = NULL;
        if (hdl->llns_1) {
            llns_exit(hdl->llns_1);
            hdl->llns_1 = NULL;
        }
    }
#endif /*DHA_NS_ENABLE*/

#if DHA_SRC_ENABLE
    hearing_src_exit(hdl);
#endif /*DHA_SRC_ENABLE*/

#if DHA_VOLUME_ENABLE
    hearing_volume_close();
#endif /*DHA_VOLUME_ENABLE*/

    //数据导出
#if DHA_DATA_EXPORT_ENABLE
    aec_uart_close();
#endif /*DHA_DATA_EXPORT_ENABLE*/

#if DHA_MIC_DATA_CBUF_ENABLE
    if (hdl->run_buf) {
        free(hdl->run_buf);
        hdl->run_buf = NULL;
    }
    if (hdl->data_buf) {
        free(hdl->data_buf);
        hdl->data_buf = NULL;
    }
#endif /*DHA_MIC_DATA_CBUF_ENABLE*/

    if (hdl->adc_dma_buf) {
        free(hdl->adc_dma_buf);
        hdl->adc_dma_buf = NULL;
    }
    if (hdl->tmp_buf) {
        free(hdl->tmp_buf);
        hdl->tmp_buf = NULL;
    }
    if (hdl->tmp_buf_1) {
        free(hdl->tmp_buf_1);
        hdl->tmp_buf_1 = NULL;
    }
    free(hdl);
    hearing_hdl = NULL;
    /* audio_codec_clock_del(HEARING_AID_MODE); */

    // 算法关闭
    dha_schedule.state = DHA_STATE_CLOSE;

#if TCFG_AUDIO_DHA_FITTING_ENABLE
    /*验配过程关闭辅听主动告诉app*/
    extern u8 get_rcsp_connect_status(void);
    if (get_rcsp_connect_status()) {
        /*主动推送辅听状态信息*/
        u8 tmp[4];
        get_hearing_aid_state_cmd_info(tmp);
        hearing_aid_rcsp_notify(tmp, 4);
    }
#endif /*TCFG_AUDIO_DHA_FITTING_ENABLE*/
    dha_schedule.fitting = 0;
    dha_schedule.fitting_state = 0;

#if TCFG_AUTO_SHUT_DOWN_TIME
    sys_auto_shut_down_enable();
#endif/*TCFG_AUTO_SHUT_DOWN_TIME*/

    printf("audio_hearing_aid_close success!\n");
    return 0;
}
char *dha_state_str[] = {
    "close",
    "open",
    "suspend",
    "error",
};
/*TWS配对后，状态同步*/
u8 get_hearing_aid_state(void)
{
    return dha_schedule.state;
}
static void hearing_aid_state_sync_deal(void *param)
{
    printf("[DHA]sync open,cur_state = %s\n", dha_state_str[dha_schedule.state]);
    if (dha_schedule.timer) {
        sys_timer_del(dha_schedule.timer);
        dha_schedule.timer = 0;
    }
    if (dha_schedule.state == DHA_STATE_OPEN) {
        audio_hearing_aid_open();
    }
}
void hearing_aid_state_sync(u8 state)
{
    if (dha_schedule.state != state) {
        dha_schedule.state = state;
        printf("[DHA]open, cur_state= %s,timer:0x%x\n", dha_state_str[dha_schedule.state], dha_schedule.timer);
        if ((dha_schedule.state == DHA_STATE_OPEN) && (dha_schedule.timer == 0)) {
            dha_schedule.timer = sys_timer_add(NULL, hearing_aid_state_sync_deal, 1500);
        }
    }
}

/*
*********************************************************************
*                  Audio Hearing-aid Demo
* Description:
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
void audio_hearing_aid_demo(void)
{
    hearing_aid_t *hdl = (hearing_aid_t *)hearing_hdl;
    printf("audio_hearing_aid_demo,toggle = %x\n", hdl);
    if (hdl == NULL) {
        audio_hearing_aid_sync_open();
    } else {
        /*验配过程中不允许按键关闭辅听*/
        if (get_hearing_aid_fitting_state()) {
            return;
        }
        audio_hearing_aid_close();
    }
}

/*
*********************************************************************
*                  Audio Hearing-aid Schedule
* Description: suspend or resume
* Arguments  : NULL
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/

void audio_hearing_aid_suspend(void)
{
#if DHA_AND_MEDIA_MUTEX_ENABLE
    printf("[DHA]suspend,cur_state = %s\n", dha_state_str[dha_schedule.state]);
    if (dha_schedule.timer) {
        printf("[DHA]schedule timer delete\n");
        sys_timer_del(dha_schedule.timer);
        dha_schedule.timer = 0;
    }
    if (hearing_hdl) {
        audio_hearing_aid_close();
        dha_schedule.state = DHA_STATE_SUSPEND;
    }
#endif/*DHA_AND_MEDIA_MUTEX_ENABLE*/
}

static void hearing_aid_schedule(void *priv)
{
    printf("[DHA]schedule,cur_state = %s\n", dha_state_str[dha_schedule.state]);
    sys_timer_del(dha_schedule.timer);
    dha_schedule.timer = 0;
    if (dha_schedule.state == DHA_STATE_SUSPEND) {
        audio_hearing_aid_sync_open();
    }
}

void audio_hearing_aid_resume(void)
{
#if DHA_AND_MEDIA_MUTEX_ENABLE
    printf("[DHA]resume, cur_state= %s,timer:0x%x\n", dha_state_str[dha_schedule.state], dha_schedule.timer);
    if ((dha_schedule.state == DHA_STATE_SUSPEND) && (dha_schedule.timer == 0)) {
        dha_schedule.timer = sys_timer_add(NULL, hearing_aid_schedule, 2500);
    }
#endif/*DHA_AND_MEDIA_MUTEX_ENABLE*/
}

static u8 hearing_aid_idle_query()
{
#if ((defined TCFG_AUDIO_DHA_LOW_POWER_ENABLE) && TCFG_AUDIO_DHA_LOW_POWER_ENABLE)
    if (audio_hearing_aid_lp_flag()) {
        return 1; //进入低功耗
    } else {
        return 0;  //不能进
    }
#else
    return hearing_hdl ? 0 : 1;
#endif /*TCFG_AUDIO_DHA_LOW_POWER_ENABLE*/
}

REGISTER_LP_TARGET(hearing_aid_lp_target) = {
    .name = "hearing_aid",
    .is_idle = hearing_aid_idle_query,
};
#endif //TCFG_AUDIO_HEARING_AID_ENABLE
