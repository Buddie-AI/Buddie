/*
 ****************************************************************
 *							AUDIO ANC
 * File  : audio_anc.c
 * By    :
 * Notes : ref_mic = 参考mic
 *		   err_mic = 误差mic
 *
 ****************************************************************
 */
#include "system/includes.h"
#include "audio_anc.h"
#include "system/task.h"
#include "timer.h"
#include "asm/clock.h"
#include "asm/power/p33.h"
#include "online_db_deal.h"
#include "app_config.h"
#include "tone_player.h"
#include "audio_adc.h"
#include "audio_enc.h"
#include "asm/dac.h"
#include "audio_anc_coeff.h"
#include "btstack/avctp_user.h"
#include "app_main.h"
#include "audio_decoder.h"
#include "adv_adaptive_noise_reduction.h"
#include "audio_codec_clock.h"
#if ANC_MUSIC_DYNAMIC_GAIN_EN
#include "amplitude_statistic.h"
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#include "icsd_adt.h"
#include "icsd_adt_app.h"
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif/*TCFG_USER_TWS_ENABLE*/
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".anc_user_bss")
#pragma data_seg(".anc_user_data")
#pragma const_seg(".anc_user_const")
#pragma code_seg(".anc_user_code")
#endif/*SUPPORT_MS_EXTENSIONS*/

#define AT_VOLATILE_RAM_CODE        AT(.volatile_ram_code)

#if INEAR_ANC_UI
extern u8 inear_tws_ancmode;
#endif/*INEAR_ANC_UI*/

#if TCFG_AUDIO_ANC_ENABLE

#if 1
#define user_anc_log	printf
#else
#define user_anc_log(...)
#endif/*log_en*/

/*************************ANC增益配置******************************/
#define ANC_DAC_GAIN			13			//ANC Speaker增益
#define ANC_REF_MIC_GAIN	 	8			//ANC参考Mic增益
#define ANC_ERR_MIC_GAIN	    8			//ANC误差Mic增益
#define ANC_FADE_GAIN_DEFAULT	16384		//ANC默认淡入淡出增益，16384 = 0dB
/*****************************************************************/
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
const u8 CONST_ANC_ADT_EN = 1;
#else
const u8 CONST_ANC_ADT_EN = 0;
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

#if ANC_EAR_ADAPTIVE_EN
const u8 CONST_ANC_EAR_ADAPTIVE_EN = 1;
#else
const u8 CONST_ANC_EAR_ADAPTIVE_EN = 0;
#endif/*ANC_EAR_ADAPTIVE_EN*/

#if TCFG_ANC_SELF_DUT_GET_SZ
const u8 CONST_ANC_DUT_SZ_EN = 1;
#else
const u8 CONST_ANC_DUT_SZ_EN = 0;
#endif/*TCFG_ANC_SELF_DUT_GET_SZ*/

#if ANC_ADAPTIVE_EN
/*自适应增益等级系数表*/
anc_adap_param_t anc_adap_param[] = {
    {
        .pow_thr = 0XFFFFFFFF,		//当前等级能量高阈值
        .default_flag = 1,			//默认等级标志
        .trigger_cnt = 20,			//当前等级检测时间*500ms */
        .gain = 1024,               //当前等级对应增益 */
        .fade_lvl = 10,             //当前等级增益淡入时间*100ms */
        /*
         此阈值对应MIC模拟增益为4, 对应差值倍数为1.259,
         如MIC模拟增益为3, 该阈值 = 500*(1.259^(3-4)) = 397
         如MIC模拟增益为6, 该阈值 = 500*(1.259^(6-4)) = 792
         */
        .pow_lthr = 700,			//当前等级能量低阈值
    },
    {
        .pow_thr = 200,				//此阈值对应模拟增益为4,计算方法如上
        .trigger_cnt = 20,
        .gain = 350,
        .fade_lvl = 10,
        .pow_lthr = 0,
    },
};

void audio_anc_pow_adap_fade(u16 gain);
#endif/*ANC_ADAPTIVE_EN*/



#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
float anc_trans_lfb_gain = 0.0625f;

float anc_trans_rfb_gain = 0.0625f;

const double anc_trans_lfb_coeff[] = {
    0.195234751212410628795623779296875,
    0.1007948522455990314483642578125,
    0.0427739980514161288738250732421875,
    -1.02504855208098888397216796875,
    0.36385215423069894313812255859375,
    /*
    0.0508266850956715643405914306640625,
    -0.1013386586564593017101287841796875,
    0.0505129448720254004001617431640625,
    -1.99860572628676891326904296875,
    0.9986066981218755245208740234375,
    */
};

const double anc_trans_rfb_coeff[] = {
    0.195234751212410628795623779296875,
    0.1007948522455990314483642578125,
    0.0427739980514161288738250732421875,
    -1.02504855208098888397216796875,
    0.36385215423069894313812255859375,
};
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/


static void anc_mix_out_audio_drc_thr(float thr);
static void anc_fade_in_timeout(void *arg);
static void anc_mode_switch_deal(u8 mode);
static void anc_timer_deal(void *priv);
static int anc_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size);
static anc_coeff_t *anc_cfg_test(u8 coeff_en, u8 gains_en);
static void anc_fade_in_timer_add(audio_anc_t *param);
static void audio_anc_adap_tone_play(int ref_lvl, int err_lvl);
void anc_dmic_io_init(audio_anc_t *param, u8 en);
u8 anc_btspp_train_again(u8 mode, u32 dat);
static void anc_tone_play_and_mode_switch(u8 index, u8 preemption, u8 cb_sel);
void anc_mode_enable_set(u8 mode_enable);
void audio_anc_post_msg_drc(void);
void audio_anc_post_msg_debug(void);
void audio_anc_post_msg_user_train_run(void);
void audio_anc_adc_ch_set(void);
void audio_anc_music_dynamic_gain_process(void);

static void audio_anc_sz_fft_dac_check_slience_cb(void *buf, int len);
void audio_anc_post_msg_sz_fft_run(void);

#if ANC_EAR_ADAPTIVE_EN
int audio_anc_adaptive_data_save(anc_adaptive_iir_t *iir);
int audio_anc_adaptive_data_read(anc_adaptive_iir_t *iir);
void audio_anc_ear_adaptive_param_init(audio_anc_t *param);

void audio_anc_param_map(u8 coeff_en, u8 gain_en);
void audio_anc_adaptive_poweron_catch_data(anc_adaptive_iir_t *iir);
void audio_anc_adaptive_data_format(anc_adaptive_iir_t *iir);
#endif/*ANC_EAR_ADAPTIVE_EN*/


extern u8 bt_phone_dec_is_running();
extern u8 bt_media_is_running(void);
extern void anc_user_train_tone_play_cb();
extern void esco_adc_mic_set_buffs(void);

#define ESCO_MIC_DUMP_CNT	10
extern void esco_mic_dump_set(u16 dump_cnt);
extern struct audio_dac_hdl dac_hdl;


typedef struct {
    u8 mode_num;					/*模式总数*/
    u8 mode_enable;					/*使能的模式*/
    u8 anc_parse_seq;
    u8 suspend;						/*挂起*/
    u8 ui_mode_sel; 				/*ui菜单降噪模式选择*/
    u8 new_mode;					/*记录模式切换最新的目标模式*/
    u8 last_mode;					/*记录模式切换的上一个模式*/
    u8 mic_dy_state;        		/*动态MIC增益状态*/
    char mic_diff_gain;				/*动态MIC增益差值*/
    u16 scene_id;					/*多场景滤波器-场景ID*/
    u8 scene_max;					/*多场景滤波器-最大场景个数*/
    u16 ui_mode_sel_timer;
    u16 fade_in_timer;
    u16 fade_gain;
    u16 mic_resume_timer;			/*动态MIC增益恢复定时器id*/
    float drc_ratio;				/*动态MIC增益，对应DRC增益比例*/
    volatile u8 state;				/*ANC状态*/
    volatile u8 mode_switch_lock;	/*切模式锁存，1表示正在切ANC模式*/
    volatile u8 sync_busy;
    audio_anc_t param;				/*ANC lib句柄*/
    audio_adc_mic_mana_t dy_mic_param[4];	/*MIC动态增益参数管理*/
#if ANC_EAR_ADAPTIVE_EN
    u8 ear_adaptive_tws_sync;		/*耳道自适应-左右耳平衡使能*/
    u8 ear_adaptive_seq;
    u8 dac_check_slience_flag;		/*耳道自适应-DAC静音检测标志*/
    u8 ear_adaptive_data_from;		/*耳道自适应-滤波器数据来源*/
    u8 ear_adaptive;				/*耳道自适应-模式标志*/
    u8 ear_adaptive_busy;			/*耳道自适应-检测繁忙标记*/
    u8 ear_adaptive_dec_wait;		/*耳道自适应-音乐播放打断标志*/
    int last_sys_clk;					/*上一次系统时钟*/
    anc_adaptive_iir_t adaptive_iir;/*耳道自适应*/
    struct audio_res_wait adaptive_wait;	/*耳道自适应，解码任务hdl*/
#endif/*ANC_EAR_ADAPTIVE_EN*/

#if ANC_MUSIC_DYNAMIC_GAIN_EN
    char loud_dB;					/*音乐动态增益-当前音乐幅值*/
    s16 loud_nowgain;				/*音乐动态增益-当前增益*/
    s16 loud_targetgain;			/*音乐动态增益-目标增益*/
    u16 loud_timeid;				/*音乐动态增益-定时器ID*/
    LOUDNESS_M_STRUCT loud_hdl;		/*音乐动态增益-操作句柄*/
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/

} anc_t;
static anc_t *anc_hdl = NULL;
anc_packet_data_t *anc_adaptive_data = NULL;

const u8 anc_tone_tab[3] = {IDEX_TONE_ANC_OFF, IDEX_TONE_ANC_ON, IDEX_TONE_TRANSPARCNCY};

u32 get_anc_gains_sign()
{
    if (anc_hdl) {
        return anc_hdl->param.gains.gain_sign;
    } else {
        return 0;
    }
}

u32 get_anc_gains_alogm()
{
    if (anc_hdl) {
        return anc_hdl->param.gains.alogm;
    } else {
        return 0;
    }
}

u32 get_anc_gains_trans_alogm()
{
    if (anc_hdl) {
        return anc_hdl->param.gains.trans_alogm;
    } else {
        return 0;
    }
}

void *get_anc_lfb_coeff()
{
    if (anc_hdl) {
        return anc_hdl->param.lfb_coeff;
    } else {
        return NULL;
    }
}

float get_anc_gains_l_fbgain()
{
    if (anc_hdl) {
        return anc_hdl->param.gains.l_fbgain;
    } else {
        return 0;
    }
}

u8 get_anc_l_fbyorder()
{
    if (anc_hdl) {
        return anc_hdl->param.lfb_yorder;
    } else {
        return 0;
    }
}



void *get_anc_lff_coeff()
{
    if (anc_hdl) {
        printf("anc hdl lff coeff:%x\n", (int)anc_hdl->param.lff_coeff);
        return anc_hdl->param.lff_coeff;
    } else {
        printf("anc hdl NULL~~~~~~~~\n");
        return NULL;
    }
}

float get_anc_gains_l_ffgain()
{
    if (anc_hdl) {
        return anc_hdl->param.gains.l_ffgain;
    } else {
        return 0;
    }
}

u8 get_anc_l_ffyorder()
{
    if (anc_hdl) {
        return anc_hdl->param.lff_yorder;
    } else {
        return 0;
    }
}

void *get_anc_ltrans_coeff()
{
    if (anc_hdl) {
        return anc_hdl->param.ltrans_coeff;
    } else {
        return NULL;
    }
}

float get_anc_gains_l_transgain()
{
    if (anc_hdl) {
        return anc_hdl->param.gains.l_transgain;
    } else {
        return 0;
    }
}

u8 get_anc_l_transyorder()
{
    if (anc_hdl) {
        return anc_hdl->param.ltrans_yorder;
    } else {
        return 0;
    }
}

void *get_anc_ltrans_fb_coeff()
{
    if (anc_hdl) {
        return anc_hdl->param.ltrans_fb_coeff;
    } else {
        return NULL;
    }
}

float get_anc_gains_lfb_transgain()
{
    if (anc_hdl) {
        return anc_hdl->param.ltrans_fbgain;
    } else {
        return 0;
    }
}

u8 get_anc_lfb_transyorder()
{
    if (anc_hdl) {
        return anc_hdl->param.ltrans_fb_yorder;
    } else {
        return 0;
    }
}

void set_anc_adt_state(u8 state)
{
    if (anc_hdl) {
        anc_hdl->param.adt_state = state;
    }
}

static void anc_task(void *p)
{
    int res;
    int anc_ret = 0;
    int msg[16];
    u8 speak_to_chat_flag = 0;
    u8 adt_flag = 0;
    u8 cur_anc_mode;
    u32 pend_timeout = portMAX_DELAY;
    user_anc_log(">>>ANC TASK<<<\n");
    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case ANC_MSG_TRAIN_OPEN:/*启动训练模式*/
                audio_mic_pwr_ctl(MIC_PWR_ON);
                anc_dmic_io_init(&anc_hdl->param, 1);
                user_anc_log("ANC_MSG_TRAIN_OPEN");
                audio_anc_train(&anc_hdl->param, 1);
                os_time_dly(1);
                anc_train_close();
                audio_mic_pwr_ctl(MIC_PWR_OFF);
                break;
            case ANC_MSG_TRAIN_CLOSE:/*训练关闭模式*/
                user_anc_log("ANC_MSG_TRAIN_CLOSE");
                anc_train_close();
                break;
            case ANC_MSG_RUN:
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
                if (anc_hdl->param.gains.trans_alogm != 5) {
                    ASSERT(0, "ERR!!ANC ADT trans_alogm %d != 5\n", anc_hdl->param.gains.trans_alogm);
                }
                audio_anc_mode_switch_in_adt(anc_hdl->param.mode);
                if (get_adt_open_in_anc_state() && anc_hdl->param.mode == ANC_OFF) {
                    user_anc_log("[ADT] ANC_MSG_RUN:%s \n", anc_mode_str[anc_hdl->param.mode]);
                    anc_hdl->param.anc_fade_gain = 0;
                    anc_hdl->param.mode = ANC_ON;
                    speak_to_chat_flag = 1;
                } else {
                    anc_hdl->param.anc_fade_gain = 16384;
                    speak_to_chat_flag = 0;
                }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

                cur_anc_mode = anc_hdl->param.mode;/*保存当前anc模式，防止切换过程anc_hdl->param.mode发生切换改变*/

#if ANC_MULT_ORDER_ENABLE
                if (cur_anc_mode != ANC_OFF) {
#if ANC_EAR_ADAPTIVE_EN
                    if (anc_hdl->param.anc_coeff_mode == ANC_COEFF_MODE_ADAPTIVE) {
                        audio_anc_coeff_adaptive_set(ANC_COEFF_MODE_ADAPTIVE, 0, 0);
                    } else {
                        anc_mult_scene_set(anc_hdl->scene_id);
                    }
#else
                    anc_mult_scene_set(anc_hdl->scene_id);
#endif/*ANC_EAR_ADAPTIVE_EN*/
                }
#endif/*ANC_MULT_ORDER_ENABLE*/

                user_anc_log("ANC_MSG_RUN:%s \n", anc_mode_str[cur_anc_mode]);
#if ANC_MUSIC_DYNAMIC_GAIN_EN
                audio_anc_music_dynamic_gain_reset(0);	/*音乐动态增益，切模式复位状态*/
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/

#if TCFG_AUDIO_DYNAMIC_ADC_GAIN
                if (anc_hdl->mic_resume_timer) {
                    sys_timer_del(anc_hdl->mic_resume_timer);
                    anc_hdl->mic_resume_timer = 0;
                }
#endif/*TCFG_AUDIO_DYNAMIC_ADC_GAIN*/
#if (TCFG_AUDIO_ADC_MIC_CHA == LADC_CH_PLNK)
                esco_mic_dump_set(ESCO_MIC_DUMP_CNT);
#endif/*LADC_CH_PLNK*/
                if (anc_hdl->state == ANC_STA_INIT) {
                    audio_mic_pwr_ctl(MIC_PWR_ON);
                    anc_dmic_io_init(&anc_hdl->param, 1);
#if ANC_MODE_SYSVDD_EN
                    clock_set_lowest_voltage(SYSVDD_VOL_SEL_105V);	//进入ANC时提高SYSVDD电压
#endif/*ANC_MODE_SYSVDD_EN*/
                    anc_ret = audio_anc_run(&anc_hdl->param);
                    if (anc_ret == 0) {
                        anc_hdl->state = ANC_STA_OPEN;
                    } else {
                        /*
                         *-EPERM(-1):不支持ANC(非差分MIC，或混合馈使用4M的flash)
                         *-EINVAL(-22):参数错误(anc_coeff.bin参数为空, 或不匹配)
                         */
                        user_anc_log("audio_anc open Failed:%d\n", anc_ret);
                        anc_hdl->mode_switch_lock = 0;
                        anc_hdl->state = ANC_STA_INIT;
                        anc_hdl->param.mode = ANC_OFF;
                        anc_hdl->new_mode = ANC_OFF;
                        /*ANC OFF && esco_dec idle*/
                        if (bt_phone_dec_is_running() == 0) {
                            audio_mic_pwr_ctl(MIC_PWR_OFF);
                        }
                        break;
                    }
                } else {
                    audio_anc_run(&anc_hdl->param);
                    if (cur_anc_mode == ANC_OFF) {
                        anc_hdl->state = ANC_STA_INIT;
                        anc_dmic_io_init(&anc_hdl->param, 0);
                        /*ANC OFF && esco_dec idle*/
                        if (bt_phone_dec_is_running() == 0) {
                            audio_mic_pwr_ctl(MIC_PWR_OFF);
                        }
                    }
                }
                if (cur_anc_mode == ANC_OFF) {
#if ANC_MODE_SYSVDD_EN
                    clock_set_lowest_voltage(SYSVDD_VOL_SEL_084V);	//退出ANC恢复普通模式
#endif/*ANC_MODE_SYSVDD_EN*/
                    /*anc关闭，如果没有连接蓝牙，倒计时进入自动关机*/
                    /* extern u8 get_total_connect_dev(void); */
                    /* if (get_total_connect_dev() == 0) {    //已经没有设备连接 */
                    /* sys_auto_shut_down_enable(); */
                    /* } */
                }
#if TCFG_AUDIO_DYNAMIC_ADC_GAIN
                if (bt_phone_dec_is_running() && (anc_hdl->state == ANC_STA_OPEN)) {
                    if (anc_hdl->mic_dy_state == ANC_MIC_DY_STA_START) {
                        audio_anc_drc_ctrl(&anc_hdl->param, anc_hdl->drc_ratio);
                    } else if (anc_hdl->mic_dy_state == ANC_MIC_DY_STA_INIT) {
                        anc_dynamic_micgain_start(app_var.aec_mic_gain);
                    }
                }
#endif/*TCFG_AUDIO_DYNAMIC_ADC_GAIN*/
                anc_hdl->mode_switch_lock = 0;
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
                if (speak_to_chat_flag) {
                    cur_anc_mode = ANC_OFF;
                    anc_hdl->param.mode = ANC_OFF;
                    speak_to_chat_flag = 0;
                }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
                if ((anc_hdl->param.mode != ANC_OFF) && anc_hdl->param.anc_fade_en) {
                    os_time_dly(6);	//延时避免切模式反馈有哒哒声
                }
                audio_anc_lvl_set(anc_hdl->param.anc_lvl, 0);
                anc_fade_in_timer_add(&anc_hdl->param);

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
                audio_icsd_adt_resume();
                audio_speak_to_chat_open_in_anc_done();
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
                anc_hdl->last_mode = cur_anc_mode;
#if ANC_MULT_ORDER_ENABLE
                audio_anc_mult_scene_coeff_free();
#endif/*ANC_MULT_ORDER_ENABLE*/
                break;
            case ANC_MSG_MODE_SYNC:
                user_anc_log("anc_mode_sync:%d", msg[2]);
                anc_mode_switch(msg[2], 0);
                break;
#if TCFG_USER_TWS_ENABLE
            case ANC_MSG_TONE_SYNC:
                user_anc_log("anc_tone_sync_play:%d", msg[2]);
#if !ANC_MODE_EN_MODE_NEXT_SW
                if (anc_hdl->mode_switch_lock) {
                    user_anc_log("anc mode switch lock\n");
                    break;
                }
                anc_hdl->mode_switch_lock = 1;
#endif/*ANC_MODE_EN_MODE_NEXT_SW*/
                if (msg[2] == SYNC_TONE_ANC_OFF) {
                    anc_hdl->param.mode = ANC_OFF;
                } else if (msg[2] == SYNC_TONE_ANC_ON) {
                    anc_hdl->param.mode = ANC_ON;
                } else {
                    anc_hdl->param.mode = ANC_TRANSPARENCY;
                }
                if (anc_hdl->suspend) {
                    anc_hdl->param.tool_enablebit = 0;
                }
                anc_hdl->new_mode = anc_hdl->param.mode;	//确保准备切的新模式，与最后一个提示音播放的模式一致
                anc_tone_play_and_mode_switch(anc_tone_tab[anc_hdl->param.mode - 1], ANC_TONE_PREEMPTION, ANC_TONE_END_MODE_SW);
                //anc_mode_switch_deal(anc_hdl->param.mode);
                anc_hdl->sync_busy = 0;
                break;
#endif/*TCFG_USER_TWS_ENABLE*/
            case ANC_MSG_FADE_END:
                break;
            case ANC_MSG_DRC_TIMER:
                audio_anc_drc_process();
                break;
            case ANC_MSG_DEBUG_OUTPUT:
                audio_anc_debug_data_output();
                break;
#if ANC_ADAPTIVE_EN
            case ANC_MSG_ADAPTIVE_SYNC:
                user_anc_log("anc_pow_adap_sync:%d %d", msg[2], msg[3]);
                audio_anc_adap_tone_play((u8)msg[2], (u8)msg[3]);
                break;
#endif/*ANC_ADAPTIVE_EN*/

#if ANC_MUSIC_DYNAMIC_GAIN_EN
            case ANC_MSG_MUSIC_DYN_GAIN:
                audio_anc_music_dynamic_gain_process();
                break;
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
            case ANC_MSG_ADT:
                icsd_adt_anctask_handle((u8)msg[2]);
                break;
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
#if ANC_EAR_ADAPTIVE_EN
            case ANC_MSG_ICSD_ANC_CMD:
                icsd_anc_anctask_cmd_handle((u8)msg[2]);
                break;
            case ANC_MSG_USER_TRAIN_INIT:
                /* anc_hdl->last_sys_clk = clk_get("sys"); */
                /* user_anc_log("sys clk:%d\n", anc_hdl->last_sys_clk); */
                /* sys_clk_set(SYS_160M);	//自适应跑最高的时钟 */
                audio_codec_clock_set(ANC_EAR_ADAPTIVE_MODE, AUDIO_CODING_PCM, 0);
                //icsd_anc_init(&anc_hdl->param,0);
                if (++anc_hdl->ear_adaptive_seq == 0xff) {
                    anc_hdl->ear_adaptive_seq = 0;
                }
                icsd_anc_init(&anc_hdl->param, 1, anc_hdl->ear_adaptive_seq, ANC_ADAPTIVE_TONE_DELAY, anc_hdl->ear_adaptive_tws_sync);
                break;
            case ANC_MSG_USER_TRAIN_RUN:
                icsd_anc_run();
                break;
            case ANC_MSG_USER_TRAIN_SETPARAM:
                icsd_anc_setparam();
                break;
            case ANC_MSG_USER_TRAIN_TIMEOUT:
                icsd_anc_timeout_handler();
                break;
            case ANC_MSG_USER_TRAIN_END:
                icsd_anc_end(&anc_hdl->param);
                /* sys_clk_set(anc_hdl->last_sys_clk);	//自适应结束恢复时钟 */
                audio_codec_clock_del(ANC_EAR_ADAPTIVE_MODE);
                break;
#endif/*ANC_EAR_ADAPTIVE_EN*/
            case ANC_MSG_MIC_DATA_GET:
                /* extern void anc_spp_mic_data_get(audio_anc_t *param); */
                /* anc_spp_mic_data_get(&anc_hdl->param); */
                break;
            case ANC_MSG_RESET:
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
                adt_flag = audio_icsd_adt_is_running();
                if (adt_flag) {	////免摘挂起
                    audio_icsd_adt_suspend();
                }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
                if (msg[2] && anc_hdl->param.anc_fade_en) {
                    int alogm = (anc_hdl->param.mode == ANC_TRANSPARENCY) ? anc_hdl->param.gains.trans_alogm : anc_hdl->param.gains.alogm;
                    int dly = audio_anc_fade_dly_get(anc_hdl->param.anc_fade_gain, alogm);
                    audio_anc_fade_ctr_set(ANC_FADE_MODE_RESET, AUDIO_ANC_FDAE_CH_ALL, 0);
                    os_time_dly(dly / 10 + 1);
                    audio_anc_reset(&anc_hdl->param, 1);
                    //避免param.anc_fade_gain 被修改，这里使用固定值
                    audio_anc_fade_ctr_set(ANC_FADE_MODE_RESET, AUDIO_ANC_FDAE_CH_ALL, ANC_FADE_GAIN_DEFAULT);
                } else {
                    audio_anc_reset(&anc_hdl->param, 0);
                }

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
                if (adt_flag) { //免摘恢复, 更新免摘参数coeff gain
                    set_icsd_adt_param_updata();
                    audio_icsd_adt_resume();
                }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
#if ANC_MULT_ORDER_ENABLE
                audio_anc_mult_scene_coeff_free();		//释放空间
#endif/*ANC_MULT_ORDER_ENABLE*/
                break;
#if TCFG_ANC_SELF_DUT_GET_SZ
            case ANC_MSG_SZ_FFT_OPEN:
                audio_codec_clock_set(ANC_DUT_SZ_FFT_MODE, AUDIO_CODING_PCM, 0);
                anc_hdl->param.mode = ANC_TRAIN;
                anc_hdl->new_mode = ANC_TRAIN;
                anc_hdl->param.test_type = ANC_TEST_TYPE_FFT;	//设置当前的测试类型
                anc_hdl->param.sz_fft.alogm = ANC_ALOGM3;		//设置当前测试模式采样率
                anc_hdl->param.sz_fft.data_sel = msg[2];		//传入目标数据通道
                anc_hdl->param.sz_fft.check_flag = 1;			//检查DAC有效数据段
                audio_anc_train(&anc_hdl->param, 1);			//打开训练模式
                audio_dac_write_callback_add(&dac_hdl, audio_anc_sz_fft_dac_check_slience_cb);	//注册DAC回调接口-静音检测
                break;
            case ANC_MSG_SZ_FFT_RUN:
                audio_anc_sz_fft_run(&anc_hdl->param);			//SZ_FFT 频响计算主体
                break;
            case ANC_MSG_SZ_FFT_CLOSE:
                audio_anc_sz_fft_stop(&anc_hdl->param);			//SZ_FFT 停止计算
                anc_train_close();								//关闭ANC，模式设置为ANC_OFF
                break;
#endif/*TCFG_ANC_SELF_DUT_GET_SZ*/
            case ANC_MSG_MODE_SWITCH_IN_ANCTASK:
                user_anc_log("anc_mode_in_anctask:%d, %d", msg[2], msg[3]);
                anc_mode_switch(msg[2], msg[3]);
                break;
            }
        } else {
            user_anc_log("res:%d,%d", res, msg[1]);
        }
    }
}

void anc_param_gain_t_printf(u8 cmd)
{
    char *str[2] = {
        "ANC_CFG_READ",
        "ANC_CFG_WRITE"
    };
    anc_gain_param_t *gains = &anc_hdl->param.gains;
    user_anc_log("-------------anc_param anc_gain_t-%s--------------\n", str[cmd - 1]);
    user_anc_log("developer_mode %d\n",    gains->developer_mode);
    user_anc_log("dac_gain %d\n", gains->dac_gain);
    user_anc_log("l_ffmic_gain %d\n", gains->l_ffmic_gain);
    user_anc_log("l_fbmic_gain %d\n", gains->l_fbmic_gain);
    user_anc_log("r_ffmic_gain %d\n", gains->r_ffmic_gain);
    user_anc_log("r_fbmic_gain %d\n", gains->r_fbmic_gain);
    user_anc_log("cmp_en %d\n",		  gains->cmp_en);
    user_anc_log("drc_en %d\n",  	  gains->drc_en);
    user_anc_log("ahs_en %d\n",		  gains->ahs_en);
    user_anc_log("ff_1st_dcc %d\n",	  gains->ff_1st_dcc);
    user_anc_log("fb_1st_dcc %d\n",	  gains->fb_1st_dcc);
    user_anc_log("ff_2nd_dcc %d\n",	  gains->ff_2nd_dcc);
    user_anc_log("fb_2nd_dcc %d\n",	  gains->fb_2nd_dcc);
    user_anc_log("drc_ff_2dcc %d\n",  gains->drc_ff_2dcc);
    user_anc_log("drc_fb_2dcc %d\n",  gains->drc_fb_2dcc);
    user_anc_log("drc_dcc_det_time %d\n",  gains->drc_dcc_det_time);
    user_anc_log("drc_dcc_res_time %d\n",  gains->drc_dcc_res_time);

    user_anc_log("gain_sign 0X%x\n",  gains->gain_sign);
    user_anc_log("noise_lvl 0X%x\n",  gains->noise_lvl);
    user_anc_log("fade_step 0X%x\n",  gains->fade_step);
    user_anc_log("alogm %d\n",		  gains->alogm);
    user_anc_log("trans_alogm %d\n",  gains->trans_alogm);

    user_anc_log("ancl_ffgain %d.%d\n", ((int)(gains->l_ffgain * 100.0)) / 100, ((int)(gains->l_ffgain * 100.0)) % 100);
    user_anc_log("ancl_fbgain %d.%d\n", ((int)(gains->l_fbgain * 100.0)) / 100, ((int)(gains->l_fbgain * 100.0)) % 100);
    user_anc_log("ancl_trans_gain %d.%d\n", ((int)(gains->l_transgain * 100.0)) / 100, ((int)(gains->l_transgain * 100.0)) % 100);
    user_anc_log("ancl_cmpgain %d.%d\n", ((int)(gains->l_cmpgain * 100.0)) / 100, ((int)(gains->l_cmpgain * 100.0)) % 100);
    user_anc_log("ancr_ffgain %d.%d\n", ((int)(gains->r_ffgain * 100.0)) / 100, ((int)(gains->r_ffgain * 100.0)) % 100);
    user_anc_log("ancr_fbgain %d.%d\n", ((int)(gains->r_fbgain * 100.0)) / 100, ((int)(gains->r_fbgain * 100.0)) % 100);
    user_anc_log("ancr_trans_gain %d.%d\n", ((int)(gains->r_transgain * 100.0)) / 100, ((int)(gains->r_transgain * 100.0)) % 100);
    user_anc_log("ancr_cmpgain %d.%d\n", ((int)(gains->r_cmpgain * 100.0)) / 100, ((int)(gains->r_cmpgain * 100.0)) % 100);

    user_anc_log("drcff_zero_det %d\n",    gains->drcff_zero_det);
    user_anc_log("drcff_dat_mode %d\n",    gains->drcff_dat_mode);
    user_anc_log("drcff_lpf_sel %d\n",     gains->drcff_lpf_sel);
    user_anc_log("drcfb_zero_det %d\n",    gains->drcfb_zero_det);
    user_anc_log("drcfb_dat_mode %d\n",    gains->drcfb_dat_mode);
    user_anc_log("drcfb_lpf_sel %d\n",     gains->drcfb_lpf_sel);

    user_anc_log("drcff_low_thr %d\n",     gains->drcff_lthr);
    user_anc_log("drcff_high_thr %d\n",    gains->drcff_hthr);
    user_anc_log("drcff_low_gain %d\n",    gains->drcff_lgain);
    user_anc_log("drcff_high_gain %d\n",   gains->drcff_hgain);
    user_anc_log("drcff_nor_gain %d\n",    gains->drcff_norgain);

    user_anc_log("drcfb_low_thr %d\n",     gains->drcfb_lthr);
    user_anc_log("drcfb_high_thr %d\n",    gains->drcfb_hthr);
    user_anc_log("drcfb_low_gain %d\n",    gains->drcfb_lgain);
    user_anc_log("drcfb_high_gain %d\n",   gains->drcfb_hgain);
    user_anc_log("drcfb_nor_gain %d\n",    gains->drcfb_norgain);

    user_anc_log("drctrans_low_thr %d\n",     gains->drctrans_lthr);
    user_anc_log("drctrans_high_thr %d\n",    gains->drctrans_hthr);
    user_anc_log("drctrans_low_gain %d\n",    gains->drctrans_lgain);
    user_anc_log("drctrans_high_gain %d\n",   gains->drctrans_hgain);
    user_anc_log("drctrans_nor_gain %d\n",    gains->drctrans_norgain);

    user_anc_log("ahs_dly %d\n",     gains->ahs_dly);
    user_anc_log("ahs_tap %d\n",     gains->ahs_tap);
    user_anc_log("ahs_wn_shift %d\n", gains->ahs_wn_shift);
    user_anc_log("ahs_wn_sub %d\n",  gains->ahs_wn_sub);
    user_anc_log("ahs_shift %d\n",   gains->ahs_shift);
    user_anc_log("ahs_u %d\n",       gains->ahs_u);
    user_anc_log("ahs_gain %d\n",    gains->ahs_gain);
    user_anc_log("ahs_nlms_sel %d\n",    gains->ahs_nlms_sel);

    user_anc_log("audio_drc_thr %d/10\n", (int)(gains->audio_drc_thr * 10.0));

    user_anc_log("adaptive_ref_en %d\n", gains->adaptive_ref_en);
    user_anc_log("adaptive_ref_fb_f %d/10\n", (int)(gains->adaptive_ref_fb_f * 10.0));
    user_anc_log("adaptive_ref_fb_g %d/10\n", (int)(gains->adaptive_ref_fb_g * 10.0));
    user_anc_log("adaptive_ref_fb_q %d/10\n", (int)(gains->adaptive_ref_fb_q * 10.0));

}

/*ANC配置参数填充*/
void anc_param_fill(u8 cmd, anc_gain_t *cfg)
{
    if (cmd == ANC_CFG_READ) {
        anc_gain_t *db_gain = (anc_gain_t *)anc_db_get(ANC_DB_GAIN, &anc_hdl->param.gains_size);
        cfg->gains = db_gain->gains;	//直接读flash区域
        /* cfg->gains = anc_hdl->param.gains; */
    } else if (cmd == ANC_CFG_WRITE) {
        anc_hdl->param.gains = cfg->gains;
        anc_hdl->param.gains.hd_en = 0;
        anc_hdl->param.gains.hd_corr_thr = 225;
        anc_hdl->param.gains.hd_corr_dly = 13;
        anc_hdl->param.gains.hd_corr_gain = 256;
        anc_hdl->param.gains.hd_pwr_rate = 2;
        anc_hdl->param.gains.hd_pwr_ctl_gain_en = 0;
        anc_hdl->param.gains.hd_pwr_ctl_ahsrst_en = 0;
        anc_hdl->param.gains.hd_pwr_thr = 500;
        anc_hdl->param.gains.hd_pwr_ctl_gain = 1638;

        anc_hdl->param.gains.hd_pwr_ref_ctl_en = 0;
        anc_hdl->param.gains.hd_pwr_ref_ctl_hthr = 2000;
        anc_hdl->param.gains.hd_pwr_ref_ctl_lthr1 = 1000;
        anc_hdl->param.gains.hd_pwr_ref_ctl_lthr2 = 200;

        anc_hdl->param.gains.hd_pwr_err_ctl_en = 0;
        anc_hdl->param.gains.hd_pwr_err_ctl_hthr = 2000;
        anc_hdl->param.gains.hd_pwr_err_ctl_lthr1 = 1000;
        anc_hdl->param.gains.hd_pwr_err_ctl_lthr2 = 200;

        anc_hdl->param.gains.developer_mode = anc_hdl->param.developer_mode;

        audio_anc_param_normalize(&anc_hdl->param);

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        if (anc_hdl->param.gains.trans_alogm != anc_hdl->param.gains.alogm) {
            user_anc_log("===========================\n\n");
            // anc_hdl->param.gains.trans_alogm = anc_hdl->param.gains.alogm;
            user_anc_log("err : anc_hdl->param.gains.trans_alogm != anc_hdl->param.gains.alogm !!!");
            user_anc_log("\n\n===========================");
        }
#endif

#if ANC_EAR_ADAPTIVE_EN
        audio_anc_param_map(0, 1);
        if (anc_hdl->param.anc_coeff_mode == ANC_COEFF_MODE_ADAPTIVE) {
            audio_anc_ear_adaptive_param_init(&anc_hdl->param);
        }

#endif/*ANC_EAR_ADAPTIVE_EN*/
    }
    /* anc_param_gain_t_printf(cmd); */
}

//ANC配置文件读取并更新
int audio_anc_db_cfg_read(void)
{
    /*读取ANC增益配置*/
    anc_gain_t *db_gain = (anc_gain_t *)anc_db_get(ANC_DB_GAIN, &anc_hdl->param.gains_size);
    if (db_gain) {
        user_anc_log("anc_gain_db get succ,len:%d\n", anc_hdl->param.gains_size);
        if (audio_anc_gains_version_verify(&anc_hdl->param, db_gain)) {	//检查版本差异
            user_anc_log("The anc gains version is older,0X%x\n", db_gain->gains.version);
            db_gain = (anc_gain_t *)anc_db_get(ANC_DB_GAIN, &anc_hdl->param.gains_size);
        }
        anc_param_fill(ANC_CFG_WRITE, db_gain);
    } else {
        user_anc_log("anc_gain_db get failed");
        /* anc_param_gain_t_printf(ANC_CFG_READ); */
        return 1;
    }
    /*读取ANC滤波器系数*/
#if ANC_MULT_ORDER_ENABLE
    return audio_anc_mult_coeff_file_read();
#endif/*ANC_MULT_ORDER_ENABLE*/
    anc_coeff_t *db_coeff = (anc_coeff_t *)anc_db_get(ANC_DB_COEFF, &anc_hdl->param.coeff_size);

    if (!anc_coeff_fill(db_coeff)) {
        user_anc_log("anc_coeff_db get succ,len:%d", anc_hdl->param.coeff_size);
    } else {
        user_anc_log("anc_coeff_db get failed");
        return 1;
#if 0	//测试数据写入是否正常，或写入自己想要的滤波器数据
        extern anc_coeff_t *anc_cfg_test(audio_anc_t *param, u8 coeff_en, u8 gains_en);
        anc_coeff_t *test_coeff = anc_cfg_test(&anc_hdl->param, 1, 1);
        if (test_coeff) {
            int ret = anc_db_put(&anc_hdl->param, NULL, test_coeff);
            if (ret == 0) {
                user_anc_log("anc_db put test succ\n");
            } else {
                user_anc_log("anc_db put test err\n");
            }
        }
        anc_coeff_t *test_coeff1 = (anc_coeff_t *)anc_db_get(ANC_DB_COEFF, &anc_hdl->param.coeff_size);
        anc_coeff_fill(test_coeff1);
#endif
    }
#if ANC_EAR_ADAPTIVE_EN
    audio_anc_param_map(1, 0);
#endif/*ANC_EAR_ADAPTIVE_EN*/
    return 0;

}



/*ANC初始化*/
void anc_init(void)
{
    anc_hdl = zalloc(sizeof(anc_t));
    user_anc_log("anc_hdl size:%lu\n", sizeof(anc_t));
    ASSERT(anc_hdl);
    audio_anc_param_init(&anc_hdl->param);
    anc_debug_init(&anc_hdl->param);
    audio_anc_fade_ctr_init();
#if TCFG_ANC_TOOL_DEBUG_ONLINE
    app_online_db_register_handle(DB_PKT_TYPE_ANC, anc_app_online_parse);
#endif/*TCFG_ANC_TOOL_DEBUG_ONLINE*/
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    anc_hdl->param.adt = zalloc(sizeof(anc_adt_param_t));
    ASSERT(anc_hdl->param.adt);
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
#if ANC_EAR_ADAPTIVE_EN
    anc_hdl->param.adaptive = zalloc(sizeof(anc_ear_adaptive_param_t));
    ASSERT(anc_hdl->param.adaptive);
#endif/*ANC_EAR_ADAPTIVE_EN*/

#if ANC_MULT_ORDER_ENABLE
    anc_mult_init(&anc_hdl->param);
    anc_hdl->scene_id = ANC_MULT_ORDER_NORMAL_ID;	//默认使用场景1滤波器
    anc_hdl->param.mult_gain_set = audio_anc_mult_gains_id_set;
#endif/*ANC_MULT_ORDER_ENABLE*/

    anc_hdl->param.post_msg_drc = audio_anc_post_msg_drc;
    anc_hdl->param.post_msg_debug = audio_anc_post_msg_debug;
    anc_hdl->mode_enable = ANC_MODE_ENABLE;
    anc_mode_enable_set(anc_hdl->mode_enable);
    anc_hdl->param.cfg_online_deal_cb = anc_cfg_online_deal;
    anc_hdl->param.adc_set_buffs_cb = esco_adc_mic_set_buffs;
    audio_anc_adc_ch_set();
    anc_hdl->param.mode = ANC_OFF;
    anc_hdl->new_mode = anc_hdl->param.mode;
    anc_hdl->param.production_mode = 0;
    anc_hdl->param.developer_mode = ANC_DEVELOPER_MODE_EN;
    anc_hdl->param.anc_fade_en = ANC_FADE_EN;/*ANC淡入淡出，默认开*/
    anc_hdl->param.anc_fade_gain = 16384;/*ANC淡入淡出增益,16384 (0dB) max:32767*/
    anc_hdl->param.tool_enablebit = ANC_TRAIN_MODE;
    anc_hdl->param.online_busy = 0;		//在线调试繁忙标志位
    //36
    anc_hdl->param.dut_audio_enablebit = ANC_DUT_CLOSE;
    anc_hdl->param.fade_time_lvl = ANC_MODE_FADE_LVL;
    anc_hdl->param.enablebit = ANC_TRAIN_MODE;
    anc_hdl->param.adaptive_mode = ANC_ADAPTIVE_MODE;
    anc_hdl->param.ch = ANC_CH;
    anc_hdl->param.mic_type[0] = ANCL_FF_MIC;
    anc_hdl->param.mic_type[1] = ANCL_FB_MIC;
    anc_hdl->param.mic_type[2] = ANCR_FF_MIC;
    anc_hdl->param.mic_type[3] = ANCR_FB_MIC;

    anc_hdl->param.lff_en = ANC_CONFIG_LFF_EN;
    anc_hdl->param.lfb_en = ANC_CONFIG_LFB_EN;
    anc_hdl->param.rff_en = ANC_CONFIG_RFF_EN;
    anc_hdl->param.rfb_en = ANC_CONFIG_RFB_EN;
    anc_hdl->param.trans_default_coeff = (double *)trans_coeff_test;	//推荐通透滤波器
    anc_hdl->param.debug_sel = 0;
    anc_hdl->param.gains.version = ANC_GAINS_VERSION;
    anc_hdl->param.gains.dac_gain = 3;
    anc_hdl->param.gains.l_ffmic_gain = 4;
    anc_hdl->param.gains.l_fbmic_gain = 4;
    anc_hdl->param.gains.r_ffmic_gain = 4;
    anc_hdl->param.gains.r_fbmic_gain = 4;
    anc_hdl->param.gains.cmp_en = 1;
    anc_hdl->param.gains.drc_en = 0;
    anc_hdl->param.gains.ahs_en = 0;
    anc_hdl->param.gains.ff_1st_dcc = 8;
    anc_hdl->param.gains.ff_2nd_dcc = 4;
    anc_hdl->param.gains.fb_1st_dcc = 8;
    anc_hdl->param.gains.fb_2nd_dcc = 1;
    anc_hdl->param.gains.gain_sign = 0;
    anc_hdl->param.gains.alogm = ANC_ALOGM6;
    anc_hdl->param.gains.trans_alogm = ANC_ALOGM4;
    anc_hdl->param.gains.l_ffgain = 1.0f;
    anc_hdl->param.gains.l_fbgain = 1.0f;
    anc_hdl->param.gains.l_cmpgain = 1.0f;
    anc_hdl->param.gains.l_transgain = 1.0f;
    anc_hdl->param.gains.r_ffgain = 1.0f;
    anc_hdl->param.gains.r_fbgain = 1.0f;
    anc_hdl->param.gains.r_cmpgain = 1.0f;
    anc_hdl->param.gains.r_transgain = 1.0f;

    anc_hdl->param.gains.drcff_zero_det = 0;
    anc_hdl->param.gains.drcff_dat_mode = 0;//hbit control l_gain, lbit control h_gain
    anc_hdl->param.gains.drcff_lpf_sel  = 0;
    anc_hdl->param.gains.drcfb_zero_det = 0;
    anc_hdl->param.gains.drcfb_dat_mode = 0;//hbit control l_gain, lbit control h_gain
    anc_hdl->param.gains.drcfb_lpf_sel  = 0;

    anc_hdl->param.gains.drcff_lthr = 0;
    anc_hdl->param.gains.drcff_hthr = 6000 ;
    anc_hdl->param.gains.drcff_lgain = 0;
    anc_hdl->param.gains.drcff_hgain = 512;
    anc_hdl->param.gains.drcff_norgain = 1024 ;

    anc_hdl->param.gains.drctrans_lthr = 0;
    anc_hdl->param.gains.drctrans_hthr = 32767 ;
    anc_hdl->param.gains.drctrans_lgain = 0;
    anc_hdl->param.gains.drctrans_hgain = 1024;
    anc_hdl->param.gains.drctrans_norgain = 1024 ;

    anc_hdl->param.gains.drcfb_lthr = 0;
    anc_hdl->param.gains.drcfb_hthr = 6000;
    anc_hdl->param.gains.drcfb_lgain = 0;
    anc_hdl->param.gains.drcfb_hgain = 512;
    anc_hdl->param.gains.drcfb_norgain = 1024;

    anc_hdl->param.gains.ahs_dly = 1;
    anc_hdl->param.gains.ahs_tap = 100;
    anc_hdl->param.gains.ahs_wn_shift = 9;
    anc_hdl->param.gains.ahs_wn_sub = 1;
    anc_hdl->param.gains.ahs_shift = 210;
    anc_hdl->param.gains.ahs_u = 4000;
    anc_hdl->param.gains.ahs_gain = -1024;
    anc_hdl->param.gains.audio_drc_thr = -6.0f;

    anc_hdl->param.gains.ahs_nlms_sel = 0;

    anc_hdl->param.gains.drc_ff_2dcc = 0;
    anc_hdl->param.gains.drc_fb_2dcc = 0;
    anc_hdl->param.gains.drc_dcc_det_time = 300;
    anc_hdl->param.gains.drc_dcc_res_time = 20;
    anc_hdl->param.gains.hd_en = 0;
    anc_hdl->param.gains.hd_corr_thr = 232;
    anc_hdl->param.gains.hd_corr_gain = 256;
    anc_hdl->param.gains.hd_corr_dly = 13;
    anc_hdl->param.gains.hd_pwr_rate = 2;
    anc_hdl->param.gains.hd_pwr_ctl_gain_en = 0;
    anc_hdl->param.gains.hd_pwr_ctl_ahsrst_en = 0;
    anc_hdl->param.gains.hd_pwr_thr = 18000;
    anc_hdl->param.gains.hd_pwr_ctl_gain = 1638;

    anc_hdl->param.gains.hd_pwr_ref_ctl_en = 0;
    anc_hdl->param.gains.hd_pwr_ref_ctl_hthr = 2000;
    anc_hdl->param.gains.hd_pwr_ref_ctl_lthr1 = 1000;
    anc_hdl->param.gains.hd_pwr_ref_ctl_lthr2 = 200;

    anc_hdl->param.gains.hd_pwr_err_ctl_en = 0;
    anc_hdl->param.gains.hd_pwr_err_ctl_hthr = 2000;
    anc_hdl->param.gains.hd_pwr_err_ctl_lthr1 = 1000;
    anc_hdl->param.gains.hd_pwr_err_ctl_lthr2 = 200;

    anc_hdl->param.gains.adaptive_ref_en = 0;
    anc_hdl->param.gains.adaptive_ref_fb_f = 120.0;
    anc_hdl->param.gains.adaptive_ref_fb_g = 15.0;
    anc_hdl->param.gains.adaptive_ref_fb_q = 0.4;

#if ANC_COEFF_SAVE_ENABLE
    //1.先读取ANC默认参数
    anc_db_init();
    audio_anc_db_cfg_read();	//读取anc_gains.bin anc_coeff.bin，更新ANC参数
#endif/*ANC_COEFF_SAVE_ENABLE*/

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    anc_hdl->param.adt->dma_done_cb = icsd_adt_dma_done;
    anc_hdl->param.ltrans_fbgain = anc_trans_lfb_gain;
    anc_hdl->param.rtrans_fbgain = anc_trans_rfb_gain;
    anc_hdl->param.ltrans_fb_coeff = anc_trans_lfb_coeff;
    anc_hdl->param.rtrans_fb_coeff = anc_trans_rfb_coeff;

    anc_hdl->param.ltrans_fb_yorder = sizeof(anc_trans_lfb_coeff) / 40;
    anc_hdl->param.rtrans_fb_yorder = sizeof(anc_trans_rfb_coeff) / 40;
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

    anc_hdl->param.biquad2ab = icsd_biquad2ab_out;

#if ANC_EAR_ADAPTIVE_EN
    //2.再读取ANC自适应参数, 若存在则覆盖默认参数
    anc_hdl->param.adaptive->ff_yorder = ANC_ADAPTIVE_FF_ORDER;
    anc_hdl->param.adaptive->fb_yorder = ANC_ADAPTIVE_FB_ORDER;
    anc_hdl->param.adaptive->cmp_yorder = ANC_ADAPTIVE_CMP_ORDER;
    anc_hdl->param.adaptive->dma_done_cb = icsd_anc_dma_done;
    audio_anc_adaptive_data_read(&anc_hdl->adaptive_iir);
#if ANC_EAR_RECORD_EN
    icsd_anc_vmdata_init(anc_hdl->adaptive_iir.record_FL, anc_hdl->adaptive_iir.record_FR, &anc_hdl->adaptive_iir.record_num);
#endif/*ANC_EAR_RECORD_EN*/

#endif/*ANC_EAR_ADAPTIVE_EN*/

    /* sys_timer_add(NULL, anc_timer_deal, 5000); */

#if ANC_ADAPTIVE_EN
    anc_pow_adap_init(&anc_hdl->param, &anc_adap_param, &anc_adap_param, \
                      sizeof(anc_adap_param) / sizeof(anc_adap_param_t), \
                      sizeof(anc_adap_param) / sizeof(anc_adap_param_t));

    anc_hdl->param.pow_adap_fade = audio_anc_pow_adap_fade;
#endif/*ANC_ADAPTIVE_EN*/


#if TCFG_ANC_SELF_DUT_GET_SZ
    anc_hdl->param.sz_fft.dma_run_hdl = audio_anc_post_msg_sz_fft_run;
#endif/*TCFG_ANC_SELF_DUT_GET_SZ*/

    audio_anc_mic_management(&anc_hdl->param);
    anc_mix_out_audio_drc_thr(anc_hdl->param.gains.audio_drc_thr);
    task_create(anc_task, NULL, "anc");
    anc_hdl->state = ANC_STA_INIT;

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    anc_adt_init();
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
    user_anc_log("anc_init ok");
}

void anc_train_open(u8 mode, u8 debug_sel)
{
    user_anc_log("ANC_Train_Open\n");
    local_irq_disable();
    if (anc_hdl && (anc_hdl->state == ANC_STA_INIT)) {
        /*防止重复打开训练模式*/
        if (anc_hdl->param.mode == ANC_TRAIN) {
            local_irq_enable();
            return;
        }
        /*anc工作，退出sniff*/
        user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
        /*anc工作，关闭自动关机*/
        /* sys_auto_shut_down_disable(); */

        anc_hdl->param.debug_sel = debug_sel;
        anc_hdl->param.mode = mode;
        anc_hdl->new_mode = anc_hdl->param.mode;
        anc_hdl->param.test_type = ANC_TEST_TYPE_PCM;
        /*训练的时候，申请buf用来保存训练参数*/
        local_irq_enable();
        os_taskq_post_msg("anc", 1, ANC_MSG_TRAIN_OPEN);
        user_anc_log("%s ok\n", __FUNCTION__);
        return;
    }
    local_irq_enable();
}

void anc_train_close(void)
{
    int ret = 0;
    if (anc_hdl == NULL) {
        return;
    }
    if (anc_hdl && (anc_hdl->param.mode == ANC_TRAIN)) {
        anc_hdl->param.train_para.train_busy = 0;
        anc_hdl->param.mode = ANC_OFF;
        anc_hdl->new_mode = anc_hdl->param.mode;
        anc_hdl->state = ANC_STA_INIT;
        audio_anc_train(&anc_hdl->param, 0);
        user_anc_log("anc_train_close ok\n");
    }
}

/*查询当前ANC是否处于训练状态*/
int anc_train_open_query(void)
{
    if (anc_hdl) {
#if ANC_EAR_ADAPTIVE_EN
        if (anc_hdl->ear_adaptive_busy) {
            return 1;
        }
#endif/*ANC_EAR_ADAPTIVE_EN*/
        if (anc_hdl->param.mode == ANC_TRAIN) {
            return 1;
        }
    }
    return 0;
}

extern void audio_dump();
static void anc_timer_deal(void *priv)
{
    u8 dac_again_l = JL_ADDA->DAA_CON1 & 0xF;
    u8 dac_again_r = (JL_ADDA->DAA_CON1 >> 4) & 0xF;
    u32 dac_dgain_l = JL_AUDIO->DAC_VL0 & 0xFFFF;
    u32 dac_dgain_r = (JL_AUDIO->DAC_VL0 >> 16) & 0xFFFF;
    u8 mic0_gain = (JL_ADDA->ADA_CON8) & 0x1F;
    u8 mic1_gain = (JL_ADDA->ADA_CON8 >> 5) & 0x1F;
    u8 mic2_gain = (JL_ADDA->ADA_CON8 >> 10) & 0x1F;
    u8 mic3_gain = (JL_ADDA->ADA_CON8 >> 15) & 0x1F;

    user_anc_log("MIC_G:%d,%d,%d,%d,DAC_AG:%d,%d,DAC_DG:%d,%d\n", mic0_gain, mic1_gain, mic2_gain, mic3_gain, dac_again_l, dac_again_r, dac_dgain_l, dac_dgain_r);
    /* mem_stats(); */
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        printf("%c, master\n", bt_tws_get_local_channel());
    } else {
        printf("%c, slave\n", bt_tws_get_local_channel());
    }
#endif/*TCFG_USER_TWS_ENABLE*/
    /* audio_dump(); */
}

#if ANC_EAR_ADAPTIVE_EN
static void anc_tone_play_pre(u8 next_mode)
{
    user_anc_log("anc_tone_play_pre:%d %d %d\n", anc_hdl->new_mode, next_mode, anc_hdl->ear_adaptive);
    /* if (anc_hdl->new_mode == next_mode) { */
    if (next_mode == ANC_ON && anc_hdl->ear_adaptive) {
        icsd_anc_tone_play_start();
    }
    /* } */
}
#endif/*ANC_EAR_ADAPTIVE_EN*/

static void anc_tone_play_cb(void *priv, int break_flag)
{
    u8 next_mode = (u8)priv;
    user_anc_log("anc_tone_play_cb,anc_mode:%d,%d,new_mode:%d, break %d\n", next_mode, anc_hdl->param.mode, anc_hdl->new_mode, break_flag);
    /*
     *当最新的目标ANC模式和即将切过去的模式一致，才进行模式切换实现。
     *否则表示，模式切换操作（比如快速按键切模式）new_mode领先于ANC模式切换实现next_mode
     *则直接在切模式的时候，实现最新的目标模式
     */
    if (anc_hdl->new_mode == next_mode) {
#if ANC_EAR_ADAPTIVE_EN
        if (next_mode == ANC_ON && anc_hdl->ear_adaptive) {
            anc_hdl->ear_adaptive = 0;
            if (break_flag) {	//	提示音被打断，自适应强制停止
                icsd_anc_forced_exit();
            }
            anc_user_train_tone_play_cb();
        } else
#endif/*ANC_EAR_ADAPTIVE_EN*/
        {
            anc_mode_switch_deal(next_mode);
        }
    } else {
        anc_hdl->mode_switch_lock = 0;
    }
}

/*
*********************************************************************
*                  Audio ANC Tone Play And Mode Switch
* Description: ANC提示音播放和模式切换
* Arguments  : index		提示音索引
*			   preemption	提示音抢断播放标识
*			   cb_sel	 	切模式方式选择
* Return	 : None.
* Note(s)    : 通过cb_sel选择是在播放提示音切模式，还是播提示音同时切
*			   模式
*********************************************************************
*/
static void anc_tone_play_and_mode_switch(u8 index, u8 preemption, u8 cb_sel)
{
    if (!bt_media_is_running() && !bt_phone_dec_is_running()) {	//后台没有音频，提示音默认打断播放
        preemption = 1;
    }
    if (cb_sel) {
#if ANC_EAR_ADAPTIVE_EN
        if (anc_hdl->ear_adaptive) {	//非ANC模式下切自适应，需要提前开MIC，ADC需要稳定时间
            audio_mic_pwr_ctl(MIC_PWR_ON);
            audio_anc_dac_open(anc_hdl->param.gains.dac_gain, anc_hdl->param.gains.dac_gain);
            anc_hdl->param.adc_set_buffs_cb();
            audio_anc_mic_open(anc_hdl->param.mic_param, 0, anc_hdl->param.adc_ch);
            index = (index == IDEX_TONE_ANC_ON) ? IDEX_TONE_ANC_ADAPTIVE : index;
        }
#if ANC_USER_TRAIN_TONE_MODE
        anc_tone_play_pre(anc_hdl->param.mode);	//播放提示音时，先进行自适应训练
#endif/*ANC_USER_TRAIN_TONE_MODE*/
#endif/*ANC_EAR_ADAPTIVE_EN*/
        tone_play_index_with_callback(index, preemption, (void(*)(void *))anc_tone_play_cb, (void *)anc_hdl->param.mode);
        /*ANC打开情况下，播提示音的同时，anc效果淡出*/
        if (anc_hdl->state == ANC_STA_OPEN) {
            audio_anc_fade_ctr_set(ANC_FADE_MODE_SWITCH, AUDIO_ANC_FDAE_CH_ALL, 0);
        }
    } else {
        tone_play_index(index, preemption);
        anc_mode_switch_deal(anc_hdl->param.mode);
    }
}

static void anc_tone_stop(void)
{
    if (anc_hdl && anc_hdl->mode_switch_lock) {
        tone_play_stop();
    }
}

/*
*********************************************************************
*                  anc_fade_in_timer
* Description: ANC增益淡入函数
* Arguments  : None.
* Return	 : None.
* Note(s)    :通过定时器的控制，使ANC的淡入增益一点一点叠加
*********************************************************************
*/
static void anc_fade_in_timer(void *arg)
{
    anc_hdl->fade_gain += (anc_hdl->param.anc_fade_gain / anc_hdl->param.fade_time_lvl);
    if (anc_hdl->fade_gain > anc_hdl->param.anc_fade_gain) {
        anc_hdl->fade_gain = anc_hdl->param.anc_fade_gain;
        usr_timer_del(anc_hdl->fade_in_timer);
    }
    audio_anc_fade_ctr_set(ANC_FADE_MODE_SWITCH, AUDIO_ANC_FDAE_CH_ALL, anc_hdl->fade_gain);
}

static void anc_fade_in_timer_add(audio_anc_t *param)
{
    u32 alogm, dly;
    if (param->anc_fade_en) {
        alogm = (param->mode == ANC_TRANSPARENCY) ? param->gains.trans_alogm : param->gains.alogm;
        dly = audio_anc_fade_dly_get(param->anc_fade_gain, alogm);
        anc_hdl->fade_gain = 0;
        if (param->mode == ANC_ON) {		//可自定义模式，当前仅在ANC模式下使用
            user_anc_log("anc_fade_time_lvl  %d\n", param->fade_time_lvl);
            anc_hdl->fade_gain = (param->anc_fade_gain / param->fade_time_lvl);
            audio_anc_fade_ctr_set(ANC_FADE_MODE_SWITCH, AUDIO_ANC_FDAE_CH_ALL, anc_hdl->fade_gain);
            if (param->fade_time_lvl > 1) {
                anc_hdl->fade_in_timer = usr_timer_add((void *)0, anc_fade_in_timer, dly, 1);
            }
        } else if (param->mode != ANC_OFF) {
            audio_anc_fade_ctr_set(ANC_FADE_MODE_SWITCH, AUDIO_ANC_FDAE_CH_ALL, param->anc_fade_gain);
        }
    }
}
static void anc_fade_out_timeout(void *arg)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_RUN);
}

static void anc_fade(u32 gain)
{
    u32	alogm, dly;
    if (anc_hdl) {
        alogm = (anc_hdl->last_mode == ANC_TRANSPARENCY) ? anc_hdl->param.gains.trans_alogm : anc_hdl->param.gains.alogm;
        dly = audio_anc_fade_dly_get(anc_hdl->param.anc_fade_gain, alogm);
        user_anc_log("anc_fade:%d,dly:%d", gain, dly);
        audio_anc_fade_ctr_set(ANC_FADE_MODE_SWITCH, AUDIO_ANC_FDAE_CH_ALL, gain);
        if (anc_hdl->param.anc_fade_en) {
            usr_timeout_del(anc_hdl->fade_in_timer);
            usr_timeout_add((void *)0, anc_fade_out_timeout, dly, 1);
        } else {/*不淡入淡出，则直接切模式*/
            os_taskq_post_msg("anc", 1, ANC_MSG_RUN);
        }
    }
}

/*
 *mode:降噪/通透/关闭
 *tone_play:切换模式的时候，是否播放提示音
 */
static void anc_mode_switch_deal(u8 mode)
{
#if TCFG_ANCLED_ENABLE
    if ((mode == ANC_ON) || (mode == ANC_TRANSPARENCY)) {
        gpio_set_pull_up(TCFG_ANCLED_PIN, 1);//头戴耳机ANC指示灯
        gpio_set_pull_down(TCFG_ANCLED_PIN, 0);
        gpio_set_direction(TCFG_ANCLED_PIN, 0);
        gpio_set_output_value(TCFG_ANCLED_PIN, 1);
    } else {
        gpio_set_pull_up(TCFG_ANCLED_PIN, 0);
        gpio_set_pull_down(TCFG_ANCLED_PIN, 0);
        gpio_set_direction(TCFG_ANCLED_PIN, 1);
        gpio_set_die(TCFG_ANCLED_PIN, 0);
    }
#endif
    user_anc_log("anc switch,state:%s", anc_state_str[anc_hdl->state]);
    anc_hdl->mode_switch_lock = 1;

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    audio_anc_mode_switch_in_adt(anc_hdl->param.mode);
#endif
    if (anc_hdl->state == ANC_STA_OPEN) {
        user_anc_log("anc open now,switch mode:%d", mode);
        anc_fade(0);//切模式，先fade_out
    } else if (anc_hdl->state == ANC_STA_INIT) {
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        if ((anc_hdl->param.mode != ANC_OFF) || get_adt_open_in_anc_state()) {
#else
        if (anc_hdl->param.mode != ANC_OFF) {
#endif/*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
            /*anc工作，关闭自动关机*/
            /* sys_auto_shut_down_disable(); */
            os_taskq_post_msg("anc", 1, ANC_MSG_RUN);
        } else {
            user_anc_log("anc close now,new mode is ANC_OFF\n");
            anc_hdl->mode_switch_lock = 0;
        }
    } else {
        user_anc_log("anc state err:%d\n", anc_hdl->state);
        anc_hdl->mode_switch_lock = 0;
    }
}

void anc_gain_app_value_set(int app_value)
{
    static int anc_gain_app_value = -1;
    if (-1 == app_value && -1 != anc_gain_app_value) {
        /* anc_hdl->param.anc_ff_gain = anc_gain_app_value; */
    }
    anc_gain_app_value = app_value;
}

static void anc_user_train_timeout(void *arg)
{
#if ANC_EAR_ADAPTIVE_EN
    anc_user_train_tone_play_cb();
#endif/*ANC_EAR_ADAPTIVE_EN*/
}

#define TWS_ANC_SYNC_TIMEOUT	600 //ms
void anc_mode_switch(u8 mode, u8 tone_play)
{
    u8 ignore_same_mode = 0;
    u8 tws_sync_en = 1;
    if (anc_hdl == NULL) {
        return;
    }
    /*模式切换超出范围*/
    if ((mode > ANC_BYPASS) || (mode < ANC_OFF)) {
        user_anc_log("anc mode switch err:%d", mode);
        return;
    }

    /*模式切换同一个*/
#if ANC_EAR_ADAPTIVE_EN
    if (anc_hdl->ear_adaptive) {
        ignore_same_mode = 1;
        if (!anc_hdl->ear_adaptive_tws_sync) {
            tws_sync_en = 0;	//自适应状态关闭左右平衡，则不需要同步切模式
        }
    }
#endif/*ANC_EAR_ADAPTIVE_EN*/
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    /*判断adt是否在关闭过程中切anc*/
    u8 adt_off_state = (audio_icsd_adt_is_running() && !get_icsd_adt_mode()) ? 1 : 0;
    /*需要在切anc模式是开adt || adt关闭时切anc off*/
    if ((get_adt_open_in_anc_state() && get_icsd_adt_mode()) || (adt_off_state && (mode == ANC_OFF))) {
        ignore_same_mode = 1;
    }
#endif/*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
    if (anc_hdl->param.mode == mode && (!ignore_same_mode)) {

        user_anc_log("anc mode switch err:same mode");
        return;
    }
#if ANC_MODE_EN_MODE_NEXT_SW
    if (anc_hdl->mode_switch_lock) {
        user_anc_log("anc mode switch lock\n");
        return;
    }
    anc_hdl->mode_switch_lock = 1;
#endif/*ANC_MODE_EN_MODE_NEXT_SW*/

    anc_hdl->new_mode = mode;/*记录最新的目标ANC模式*/
#if ANC_TONE_END_MODE_SW && (!ANC_MODE_EN_MODE_NEXT_SW)
    anc_tone_stop();
#endif/*ANC_TONE_END_MODE_SW*/
    anc_hdl->param.mode = mode;

    if (anc_hdl->suspend) {
        anc_hdl->param.tool_enablebit = 0;
    }

    /* anc_gain_app_value_set(-1); */		//没有处理好bypass与ANC_ON的关系
    /*
     *ANC模式提示音播放规则
     *(1)根据应用选择是否播放提示音：tone_play
     *(2)tws连接的时候，主机发起模式提示音同步播放
     *(3)单机的时候，直接播放模式提示音
     */
    if (tone_play) {
#if TCFG_USER_TWS_ENABLE
        if (get_tws_sibling_connect_state() && tws_sync_en) {
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                user_anc_log("[tws_master]anc_tone_sync_play");
                anc_hdl->sync_busy = 1;
                if (anc_hdl->param.mode == ANC_ON) {
                    bt_tws_play_tone_at_same_time(SYNC_TONE_ANC_ON, TWS_ANC_SYNC_TIMEOUT);
                } else if (anc_hdl->param.mode == ANC_OFF) {
                    bt_tws_play_tone_at_same_time(SYNC_TONE_ANC_OFF, TWS_ANC_SYNC_TIMEOUT);
                } else {
                    bt_tws_play_tone_at_same_time(SYNC_TONE_ANC_TRANS, TWS_ANC_SYNC_TIMEOUT);
                }
            }
            return;
        } else {
            user_anc_log("anc_tone_play");
            anc_tone_play_and_mode_switch(anc_tone_tab[mode - 1], ANC_TONE_PREEMPTION, ANC_TONE_END_MODE_SW);
        }
#else
        anc_tone_play_and_mode_switch(anc_tone_tab[mode - 1], ANC_TONE_PREEMPTION, ANC_TONE_END_MODE_SW);
#endif/*TCFG_USER_TWS_ENABLE*/
    } else {
#if ANC_EAR_ADAPTIVE_EN	//	不播提示音进入自适应流程
        if (mode == ANC_ON && anc_hdl->ear_adaptive) {
            anc_hdl->ear_adaptive = 0;
            if (anc_hdl->param.anc_fade_en) {
                u32 dly = audio_anc_fade_dly_get(anc_hdl->param.anc_fade_gain, anc_hdl->param.gains.alogm);
                audio_anc_fade_ctr_set(ANC_FADE_MODE_SWITCH, AUDIO_ANC_FDAE_CH_ALL, 0);
                usr_timeout_add((void *)0, anc_user_train_timeout, dly, 1);
            } else {
                anc_user_train_timeout(NULL);
            }
        } else
#endif/*ANC_EAR_ADAPTIVE_EN*/
        {
            anc_mode_switch_deal(mode);
        }
        /* anc_mode_switch_deal(mode); */
    }
}

static void anc_ui_mode_sel_timer(void *priv)
{
    if (anc_hdl->ui_mode_sel && (anc_hdl->ui_mode_sel != anc_hdl->param.mode)) {
        /*
         *提示音不打断
         *tws提示音同步播放完成
         */
        if ((tone_get_status() == 0) && (anc_hdl->sync_busy == 0)) {
            user_anc_log("anc_ui_mode_sel_timer:%d,sync_busy:%d", anc_hdl->ui_mode_sel, anc_hdl->sync_busy);
            anc_mode_switch(anc_hdl->ui_mode_sel, 1);
            sys_timer_del(anc_hdl->ui_mode_sel_timer);
            anc_hdl->ui_mode_sel_timer = 0;
        }
    }
}

/*ANC通过ui菜单选择anc模式,处理快速切换的情景*/
void anc_ui_mode_sel(u8 mode, u8 tone_play)
{
    /*
     *timer存在表示上个模式还没有完成切换
     *提示音不打断
     *tws提示音同步播放完成
     */
    if ((anc_hdl->ui_mode_sel_timer == 0) && (tone_get_status() == 0) && (anc_hdl->sync_busy == 0)) {
        user_anc_log("anc_ui_mode_sel[ok]:%d", mode);
        anc_mode_switch(mode, tone_play);
    } else {
        user_anc_log("anc_ui_mode_sel[dly]:%d,timer:%d,sync_busy:%d", mode, anc_hdl->ui_mode_sel_timer, anc_hdl->sync_busy);
        anc_hdl->ui_mode_sel = mode;
        if (anc_hdl->ui_mode_sel_timer == 0) {
            anc_hdl->ui_mode_sel_timer = sys_timer_add(NULL, anc_ui_mode_sel_timer, 50);
        }
    }
}

/*tws同步播放模式提示音，并且同步进入anc模式*/
void anc_tone_sync_play(int tone_name)
{
#if TCFG_USER_TWS_ENABLE
    if (anc_hdl) {
        user_anc_log("anc_tone_sync_play:%d", tone_name);
        os_taskq_post_msg("anc", 2, ANC_MSG_TONE_SYNC, tone_name);
    }
#endif/*TCFG_USER_TWS_ENABLE*/
}

/*ANC模式同步(tws模式)*/
void anc_mode_sync(u8 *data)
{
    //data[0] mode;
    //data[1] adaptive_seq;
    //data[2] scene_id;
    if (anc_hdl) {
#if ANC_EAR_ADAPTIVE_EN
        anc_hdl->ear_adaptive_seq = data[1];
#endif/*ANC_EAR_ADAPTIVE_EN*/

#if ANC_MULT_ORDER_ENABLE
        anc_hdl->scene_id = data[2];
#endif/*ANC_MULT_ORDER_ENABLE*/
        os_taskq_post_msg("anc", 2, ANC_MSG_MODE_SYNC, data[0]);
    }
}

/*在anc任务里面切换anc模式，避免上一次切换没有完成，这次切换被忽略的情况*/
void anc_mode_switch_in_anctask(u8 mode, u8 tone_play)
{
    if (anc_hdl) {
        os_taskq_post_msg("anc", 3, ANC_MSG_MODE_SWITCH_IN_ANCTASK, mode, tone_play);
    }
}

/*ANC挂起*/
void anc_suspend(void)
{
    if (anc_hdl) {
        user_anc_log("anc_suspend\n");
        anc_hdl->suspend = 1;
        anc_hdl->param.tool_enablebit = 0;	//使能设置为0
        if (anc_hdl->param.anc_fade_en) {	  //挂起ANC增益淡出
            audio_anc_fade_ctr_set(ANC_FADE_MODE_SUSPEND, AUDIO_ANC_FDAE_CH_ALL, 0);
        }
        audio_anc_en_set(0);
    }
}

/*ANC恢复*/
void anc_resume(void)
{
    if (anc_hdl) {
        user_anc_log("anc_resume\n");
        anc_hdl->suspend = 0;
        anc_hdl->param.tool_enablebit = anc_hdl->param.enablebit;	//使能恢复
        audio_anc_en_set(1);
        if (anc_hdl->param.anc_fade_en) {	  //恢复ANC增益淡入
            //避免param.anc_fade_gain 被修改，这里使用固定值
            audio_anc_fade_ctr_set(ANC_FADE_MODE_SUSPEND, AUDIO_ANC_FDAE_CH_ALL, ANC_FADE_GAIN_DEFAULT);
        }
    }
}

/*ANC信息保存*/
void anc_info_save()
{
    if (anc_hdl) {
        anc_info_t anc_info;
        int ret = syscfg_read(CFG_ANC_INFO, &anc_info, sizeof(anc_info));
        if (ret == sizeof(anc_info)) {
#if INEAR_ANC_UI
            if (anc_info.mode == anc_hdl->param.mode && anc_info.inear_tws_mode == inear_tws_ancmode) {
#else
            if (anc_info.mode == anc_hdl->param.mode) {
#endif/*INEAR_ANC_UI*/
                user_anc_log("anc info.mode == cur_anc_mode");
                return;
            }
        } else {
            user_anc_log("read anc_info err");
        }

        user_anc_log("save anc_info");
        anc_info.mode = anc_hdl->param.mode;
#if INEAR_ANC_UI
        anc_info.inear_tws_mode = inear_tws_ancmode;
#endif/*INEAR_ANC_UI*/
        ret = syscfg_write(CFG_ANC_INFO, &anc_info, sizeof(anc_info));
        if (ret != sizeof(anc_info)) {
            user_anc_log("anc info save err!\n");
        }

    }
}

/*系统上电的时候，根据配置决定是否进入上次的模式*/
void anc_poweron(void)
{
    if (anc_hdl) {
#if ANC_INFO_SAVE_ENABLE
        anc_info_t anc_info;
        int ret = syscfg_read(CFG_ANC_INFO, &anc_info, sizeof(anc_info));
        if (ret == sizeof(anc_info)) {
            user_anc_log("read anc_info succ,state:%s,mode:%s", anc_state_str[anc_hdl->state], anc_mode_str[anc_info.mode]);
#if INEAR_ANC_UI
            inear_tws_ancmode = anc_info.inear_tws_mode;
#endif/*INEAR_ANC_UI*/
            if ((anc_hdl->state == ANC_STA_INIT) && (anc_info.mode != ANC_OFF)) {
                anc_mode_switch(anc_info.mode, 0);
            }
        } else {
            user_anc_log("read anc_info err");
        }
#endif/*ANC_INFO_SAVE_ENABLE*/
    }
}

/*ANC poweroff*/
void anc_poweroff(void)
{
    if (anc_hdl) {
#if ANC_POWEOFF_SAVE_ADAPTIVE_DATA && ANC_EAR_ADAPTIVE_EN && (!ANC_DEVELOPER_MODE_EN)
        if (anc_hdl->ear_adaptive_data_from == ANC_ADAPTIVE_DATA_FROM_ALOGM) {	//表示数据已经更新
            audio_anc_adaptive_data_save(&anc_hdl->adaptive_iir);
        }
#endif/*ANC_POWEOFF_SAVE_ADAPTIVE_DATA*/
        user_anc_log("anc_cur_state:%s\n", anc_state_str[anc_hdl->state]);
#if ANC_INFO_SAVE_ENABLE
        anc_info_save();
#endif/*ANC_INFO_SAVE_ENABLE*/
        if (anc_hdl->state == ANC_STA_OPEN) {
            user_anc_log("anc_poweroff\n");
            /*close anc module when fade_out timeout*/
            anc_hdl->param.mode = ANC_OFF;
            anc_hdl->new_mode = anc_hdl->param.mode;
            anc_fade(0);
        }
    }
}

/*模式切换测试demo*/
#define ANC_MODE_NUM	3 /*ANC模式循环切换*/
static const u8 anc_mode_switch_tab[ANC_MODE_NUM] = {
    ANC_OFF,
    ANC_TRANSPARENCY,
    ANC_ON,
};
void anc_mode_next(void)
{
    if (anc_hdl) {
        if (anc_train_open_query()) {
            return;
        }
        u8 next_mode = 0;
        local_irq_disable();
        anc_hdl->param.anc_fade_en = ANC_FADE_EN;	//防止被其他地方清0
        for (u8 i = 0; i < ANC_MODE_NUM; i++) {
            if (anc_mode_switch_tab[i] == anc_hdl->param.mode) {
                next_mode = i + 1;
                if (next_mode >= ANC_MODE_NUM) {
                    next_mode = 0;
                }
                if ((anc_hdl->mode_enable & BIT(anc_mode_switch_tab[next_mode])) == 0) {
                    user_anc_log("anc_mode_filt,next:%d,en:%d", next_mode, anc_hdl->mode_enable);
                    next_mode++;
                    if (next_mode >= ANC_MODE_NUM) {
                        next_mode = 0;
                    }
                }
                //g_printf("fine out anc mode:%d,next:%d,i:%d",anc_hdl->param.mode,next_mode,i);
                break;
            }
        }
        local_irq_enable();
        //user_anc_log("anc_next_mode:%d old:%d,new:%d", next_mode, anc_hdl->param.mode, anc_mode_switch_tab[next_mode]);
        u8 new_mode = anc_mode_switch_tab[next_mode];
        user_anc_log("new_mode:%s", anc_mode_str[new_mode]);

#if ANC_EAR_ADAPTIVE_EN && ANC_EAR_ADAPTIVE_EVERY_TIME		//非通话状态下，每次切模式都自适应
        if ((anc_mode_switch_tab[next_mode] == ANC_ON) && (bt_phone_dec_is_running() == 0)) {
            anc_hdl->param.mode = ANC_ON;
            anc_hdl->new_mode = anc_hdl->param.mode;
            audio_anc_mode_ear_adaptive(1);
        } else
#endif/*ANC_EAR_ADAPTIVE_EVERY_TIME*/
        {
            anc_mode_switch(anc_mode_switch_tab[next_mode], 1);
        }
    }
}

/*设置ANC支持切换的模式*/
void anc_mode_enable_set(u8 mode_enable)
{
    if (anc_hdl) {
        anc_hdl->mode_enable = mode_enable;
        u8 mode_cnt = 0;
        for (u8 i = 1; i < 4; i++) {
            if (mode_enable & BIT(i)) {
                mode_cnt++;
                user_anc_log("%s Select", anc_mode_str[i]);
            }
        }
        user_anc_log("anc_mode_enable_set:%d", mode_cnt);
        anc_hdl->mode_num = mode_cnt;
    }
}

/*获取anc状态，0:空闲，l:忙*/
u8 anc_status_get(void)
{
    u8 status = 0;
    if (anc_hdl) {
        if (anc_hdl->state == ANC_STA_OPEN) {
            status = 1;
        }
    }
    return status;
}

/*获取anc当前模式*/
u8 anc_mode_get(void)
{
    if (anc_hdl) {
        //user_anc_log("anc_mode_get:%s", anc_mode_str[anc_hdl->param.mode]);
        return anc_hdl->param.mode;
    }
    return 0;
}

/*获取anc记录的最新的目标ANC模式*/
u8 anc_new_target_mode_get(void)
{
    if (anc_hdl) {
        //user_anc_log("anc_mode_get:%s", anc_mode_str[anc_hdl->new_mode]);
        return anc_hdl->new_mode;
    }
    return 0;
}

/*获取anc模式，dac左右声道的增益*/
u8 anc_dac_gain_get(u8 ch)
{
    u8 gain = 0;
    if (anc_hdl) {
        gain = anc_hdl->param.gains.dac_gain;
    }
    return gain;
}

/*获取anc模式，ff_mic的增益*/
u8 audio_anc_ffmic_gain_get(void)
{
    u8 gain = 0;
    if (anc_hdl) {
        if (anc_hdl->param.ch & ANC_L_CH) {
            gain = anc_hdl->param.gains.l_ffmic_gain;
        } else {
            gain = anc_hdl->param.gains.r_ffmic_gain;
        }
    }
    return gain;
}

/*获取anc模式，fb_mic的增益*/
u8 audio_anc_fbmic_gain_get(void)
{
    u8 gain = 0;
    if (anc_hdl) {
        if (anc_hdl->param.ch & ANC_L_CH) {
            gain = anc_hdl->param.gains.l_fbmic_gain;
        } else {
            gain = anc_hdl->param.gains.r_fbmic_gain;
        }
    }
    return gain;
}

/*获取anc模式，指定mic的增益, mic_sel:目标MIC通道*/
u8 audio_anc_mic_gain_get(u8 mic_sel)
{
    return anc_hdl->param.mic_param[mic_sel].gain;
}


/*anc coeff读接口*/
int *anc_coeff_read(void)
{
#if ANC_BOX_READ_COEFF
    int *coeff = anc_db_get(ANC_DB_COEFF, &anc_hdl->param.coeff_size);
    if (coeff) {
        coeff = (int *)((u8 *)coeff);
    }
    user_anc_log("anc_coeff_read:0x%x", (u32)coeff);
    return coeff;
#else
    return NULL;
#endif/*ANC_BOX_READ_COEFF*/
}


/*anc coeff写接口*/
int anc_coeff_write(int *coeff, u16 len)
{
    int ret = 0;
    u8 single_type = 0;
    anc_coeff_t *db_coeff = (anc_coeff_t *)coeff;
    anc_coeff_t *tmp_coeff = NULL;
#if ANC_MULT_ORDER_ENABLE
    return -1;
#endif/**/

    user_anc_log("anc_coeff_write:0x%x, len:%d", (u32)coeff, len);
    ret = anc_coeff_check(db_coeff, len);
    if (ret) {
        return ret;
    }
    if (db_coeff->cnt != 1) {
        ret = anc_db_put(&anc_hdl->param, NULL, (anc_coeff_t *)coeff);
    } else {	//保存对应的单项滤波器
        ret = anc_coeff_single_fill(&anc_hdl->param, (anc_coeff_t *)coeff);
    }
    if (ret) {
        user_anc_log("anc_coeff_write err:%d", ret);
        return -1;
    }
    db_coeff = (anc_coeff_t *)anc_db_get(ANC_DB_COEFF, &anc_hdl->param.coeff_size);
    user_anc_log("anc_db coeff_size %d\n", anc_hdl->param.coeff_size);
    anc_coeff_fill(db_coeff);
#if ANC_EAR_ADAPTIVE_EN
    audio_anc_param_map(1, 0);
#endif/*ANC_EAR_ADAPTIVE_EN*/

    if (anc_hdl->param.mode != ANC_OFF) {		//实时更新填入使用
        audio_anc_reset(&anc_hdl->param, 0);
    }
    return 0;
}

static u8 ANC_idle_query(void)
{
    if (anc_hdl) {
        /*ANC训练模式，不进入低功耗*/
        if (anc_train_open_query()) {
            return 0;
        }
    }
    return 1;
}

static enum LOW_POWER_LEVEL ANC_level_query(void)
{
    if (anc_hdl == NULL) {
        return LOW_POWER_MODE_SLEEP;
    }
    /*根据anc的状态选择sleep等级*/
    if (anc_status_get() || anc_hdl->mode_switch_lock) {
        /*anc打开，进入轻量级低功耗*/
        return LOW_POWER_MODE_LIGHT_SLEEP;
    }
    /*anc关闭，进入最优低功耗*/
    return LOW_POWER_MODE_DEEP_SLEEP;
}

/* AT_VOLATILE_RAM_CODE */
AT(.volatile_ram_code)
u8 is_lowpower_anc_active()
{
    if (anc_hdl->state == ANC_STA_OPEN || anc_hdl->mode_switch_lock) {
        /* 若anc 开启，则进入轻量级低功耗时，保持住ANC对应时钟 */
        if ((anc_hdl->param.ch == (ANC_L_CH | ANC_R_CH)) && !anc_hdl->param.lr_lowpower_en) {
            return ANC_CLOCK_USE_PLL;
        } else {
            return ANC_CLOCK_USE_BTOSC;
        }
    }
    return ANC_CLOCK_USE_CLOSE;
}

REGISTER_LP_TARGET(ANC_lp_target) = {
    .name       = "ANC",
    .level      = ANC_level_query,
    .is_idle    = ANC_idle_query,
};

void chargestore_uart_data_deal(u8 *data, u8 len)
{
    /* anc_uart_process(data, len); */
}

#if TCFG_ANC_TOOL_DEBUG_ONLINE
//回复包
void anc_ci_send_packet(u32 id, u8 *packet, int size)
{
    if (DB_PKT_TYPE_ANC == id) {
        app_online_db_ack(anc_hdl->anc_parse_seq, packet, size);
        return;
    }
    /* y_printf("anc_app_spp_tx\n"); */
    ci_send_packet(id, packet, size);
}

//接收函数
static int anc_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    /* y_printf("anc_app_spp_rx\n"); */
    anc_hdl->anc_parse_seq  = ext_data[1];
    anc_spp_rx_packet(packet, size);
    return 0;
}
//发送函数
void anc_btspp_packet_tx(u8 *packet, int size)
{
    app_online_db_send(DB_PKT_TYPE_ANC, packet, size);
}
#endif/*TCFG_ANC_TOOL_DEBUG_ONLINE*/

u8 anc_btspp_train_again(u8 mode, u32 dat)
{
    /* tws_api_detach(TWS_DETACH_BY_POWEROFF); */
    //ANC模式下可直接获取REF+ERRMIC的数据
    /* if((dat == 3) || (dat == 5)) { */
    /* anc_hdl->param.debug_sel = dat; */
    /* os_taskq_post_msg("anc", 1, ANC_MSG_MIC_DATA_GET); */
    /* return 0; */
    /* } */
#if ANC_EAR_ADAPTIVE_EN
    if (anc_hdl->ear_adaptive_busy) {	//ANC自适应切换时，不支持获取DMA
        return 0;
    }
#if TCFG_USER_TWS_ENABLE
    switch (dat) {
    case 2:
        dat = 4;
        break;
    case 3:
        dat = 5;
        break;
    case 6:
        dat = 7;
        break;
    }
#endif/*TCFG_USER_TWS_ENABLE*/
#endif/*ANC_EAR_ADAPTIVE_EN*/
    user_anc_log("anc_btspp_train_again\n");
    audio_anc_close();
    anc_hdl->state = ANC_STA_INIT;
    anc_train_open(mode, (u8)dat);
    return 1;
}

u8 audio_anc_develop_get(void)
{
    return anc_hdl->param.developer_mode;
}

void audio_anc_develop_set(u8 mode)
{
#if (!ANC_DEVELOPER_MODE_EN) && ANC_EAR_ADAPTIVE_EN	//非强制开发者模式跟随工具设置
    if (mode != anc_hdl->param.developer_mode) {
        anc_hdl->param.developer_mode = mode;
        if (anc_hdl->param.mode != ANC_OFF) {
            anc_mode_switch(ANC_OFF, 0);									//避免右耳参数异常，让调试者感受不适
        }
        if (anc_hdl->param.anc_coeff_mode == ANC_COEFF_MODE_ADAPTIVE) {
            return;															//自适应模式不更新参数
        }
        if (mode) { //0 -> 1
            audio_anc_param_map(1, 1);										//映射ANC参数
            audio_anc_adaptive_poweron_catch_data(&anc_hdl->adaptive_iir);	//打包自适应数据,等待工具读取
        } else {    //1 -> 0
            audio_anc_db_cfg_read();										//读取ANC配置区的数据
            if (anc_adaptive_data) {
                free_anc_data_packet(anc_adaptive_data);
                anc_adaptive_data = NULL;
            }
        }
    }
#endif/*ANC_DEVELOPER_MODE_EN*/
}

/*ANC配置在线读取接口*/
int anc_cfg_online_deal(u8 cmd, anc_gain_t *cfg)
{
    /*同步在线更新配置*/
    int ret = 0;
    anc_gain_param_t *gains = &anc_hdl->param.gains;
    anc_param_fill(cmd, cfg);
    if (cmd == ANC_CFG_WRITE) {
        /*实时更新ANC配置*/
        audio_anc_dac_gain(gains->dac_gain, gains->dac_gain);
        audio_anc_mic_management(&anc_hdl->param);
        audio_anc_mic_gain(anc_hdl->param.mic_param, 0);
        anc_mix_out_audio_drc_thr(gains->audio_drc_thr);
        if (anc_hdl->param.mode != ANC_OFF) {		//实时更新填入使用
#if ANC_MULT_ORDER_ENABLE
            anc_mult_scene_set(anc_hdl->scene_id);	//覆盖增益以及增益符号
#endif/*ANC_MULT_ORDER_ENABLE*/
            audio_anc_reset(&anc_hdl->param, 0);		//不可异步，因为后面会更新ANCIF区域内容
#if ANC_MULT_ORDER_ENABLE
            audio_anc_mult_scene_coeff_free();		//释放空间
#endif/*ANC_MULT_ORDER_ENABLE*/
        }
        /*仅修改增益配置*/
        ret = anc_db_put(&anc_hdl->param, cfg, NULL);
        if (ret) {
            user_anc_log("ret %d\n", ret);
            anc_hdl->param.lff_coeff = NULL;
            anc_hdl->param.rff_coeff = NULL;
            anc_hdl->param.lfb_coeff = NULL;
            anc_hdl->param.rfb_coeff = NULL;
        };
    }
    return 0;
}

#if 0
/*ANC数字MIC IO配置*/
static atomic_t dmic_mux_ref;
#define DMIC_SCLK_FROM_PLNK		0
#define DMIC_SCLK_FROM_ANC		1
void dmic_io_mux_ctl(u8 en, u8 sclk_sel)
{
    user_anc_log("dmic_io_mux,en:%d,sclk:%d,ref:%d\n", en, sclk_sel, atomic_read(&dmic_mux_ref));
    if (en) {
        if (atomic_read(&dmic_mux_ref)) {
            user_anc_log("DMIC_IO_MUX open now\n");
            if (sclk_sel == DMIC_SCLK_FROM_ANC) {
                user_anc_log("plink_sclk -> anc_sclk\n");
                gpio_set_fun_output_port(TCFG_AUDIO_PLNK_SCLK_PIN, FO_ANC_MICCK, 0, 1, LOW_POWER_KEEP);
            }
            atomic_inc_return(&dmic_mux_ref);
            return;
        }
        if (sclk_sel == DMIC_SCLK_FROM_ANC) {
            gpio_set_fun_output_port(TCFG_AUDIO_PLNK_SCLK_PIN, FO_ANC_MICCK, 0, 1, LOW_POWER_KEEP);
        } else {
            gpio_set_fun_output_port(TCFG_AUDIO_PLNK_SCLK_PIN, FO_PLNK_SCLK, 0, 1, LOW_POWER_KEEP);
        }
        gpio_set_direction(TCFG_AUDIO_PLNK_SCLK_PIN, 0);
        gpio_set_die(TCFG_AUDIO_PLNK_SCLK_PIN, 0);
        gpio_direction_output(TCFG_AUDIO_PLNK_SCLK_PIN, 1);
#if TCFG_AUDIO_PLNK_DAT0_PIN != NO_CONFIG_PORT
        //anc data0 port init
        gpio_set_pull_down(TCFG_AUDIO_PLNK_DAT0_PIN, 0);
        gpio_set_pull_up(TCFG_AUDIO_PLNK_DAT0_PIN, 1);
        gpio_set_direction(TCFG_AUDIO_PLNK_DAT0_PIN, 1);
        gpio_set_die(TCFG_AUDIO_PLNK_DAT0_PIN, 1);
        gpio_set_fun_input_port(TCFG_AUDIO_PLNK_DAT0_PIN, PFI_PLNK_DAT0, LOW_POWER_KEEP);
#endif/*TCFG_AUDIO_PLNK_DAT0_PIN != NO_CONFIG_PORT*/

#if TCFG_AUDIO_PLNK_DAT1_PIN != NO_CONFIG_PORT
        //anc data1 port init
        gpio_set_pull_down(TCFG_AUDIO_PLNK_DAT1_PIN, 0);
        gpio_set_pull_up(TCFG_AUDIO_PLNK_DAT1_PIN, 1);
        gpio_set_direction(TCFG_AUDIO_PLNK_DAT1_PIN, 1);
        gpio_set_die(TCFG_AUDIO_PLNK_DAT1_PIN, 1);
        gpio_set_fun_input_port(TCFG_AUDIO_PLNK_DAT1_PIN, PFI_PLNK_DAT1, LOW_POWER_KEEP);
#endif/*TCFG_AUDIO_PLNK_DAT1_PIN != NO_CONFIG_PORT*/
        atomic_inc_return(&dmic_mux_ref);
    } else {
        if (atomic_read(&dmic_mux_ref)) {
            atomic_dec_return(&dmic_mux_ref);
            if (atomic_read(&dmic_mux_ref)) {
                if (sclk_sel == DMIC_SCLK_FROM_ANC) {
                    user_anc_log("anc close now,anc_sclk->plnk_sclk\n");
                    gpio_set_fun_output_port(TCFG_AUDIO_PLNK_SCLK_PIN, FO_PLNK_SCLK, 0, 1, LOW_POWER_KEEP);
                } else {
                    user_anc_log("plnk close,anc_plnk open\n");
                }
            } else {
                user_anc_log("dmic all close,disable plnk io_mapping output\n");
                gpio_disable_fun_output_port(TCFG_AUDIO_PLNK_SCLK_PIN);
#if TCFG_AUDIO_PLNK_DAT0_PIN != NO_CONFIG_PORT
                gpio_disable_fun_input_port(TCFG_AUDIO_PLNK_DAT0_PIN);
#endif/*TCFG_AUDIO_PLNK_DAT0_PIN != NO_CONFIG_PORT*/
#if TCFG_AUDIO_PLNK_DAT1_PIN != NO_CONFIG_PORT
                gpio_disable_fun_input_port(TCFG_AUDIO_PLNK_DAT1_PIN);
#endif/*TCFG_AUDIO_PLNK_DAT1_PIN != NO_CONFIG_PORT*/
            }
        } else {
            user_anc_log("dmic_mux_ref NULL\n");
        }
    }
}
void anc_dmic_io_init(audio_anc_t *param, u8 en)
{
    if (en) {
        int i;
        for (i = 0; i < 4; i++) {
            if ((param->mic_type[i] > A_MIC1) && (param->mic_type[i] != MIC_NULL)) {
                user_anc_log("anc_dmic_io_init %d:%d\n", i, param->mic_type[i]);
                dmic_io_mux_ctl(1, DMIC_SCLK_FROM_ANC);
                break;
            }
        }
    } else {
        dmic_io_mux_ctl(0, DMIC_SCLK_FROM_ANC);
    }
}
#else
void anc_dmic_io_init(audio_anc_t *param, u8 en)
{
}
#endif

extern void mix_out_drc_threadhold_update(float threadhold);
static void anc_mix_out_audio_drc_thr(float thr)
{
    if (thr > 0.0 || thr < -6.0 || anc_hdl->param.enablebit == ANC_FB_EN) {
        thr = 0.0;
    }
    mix_out_drc_threadhold_update(thr);
}

#if TCFG_AUDIO_DYNAMIC_ADC_GAIN
void anc_dynamic_micgain_start(u8 audio_mic_gain)
{
    int anc_mic_gain, i, diff_gain;
    u8 flag = 0;
    float ffgain;
    float multiple = 1.259;
    if (!anc_status_get()) {
        anc_hdl->mic_dy_state = ANC_MIC_DY_STA_INIT;
        return;
    }
    memcpy(anc_hdl->dy_mic_param, anc_hdl->param.mic_param, sizeof(audio_adc_mic_mana_t) * 4);

    for (i = 0; i < 4; i++) {   //动态MIC增益调整时，忽略FBmic
        if ((anc_hdl->dy_mic_param[i].type % 2) == 1) {
            anc_hdl->dy_mic_param[i].mult_flag = 0;
            printf("clean anc_hdl->dy_mic_param[%d].mult_flag\n", i);
        }
    }
    anc_hdl->drc_ratio = 1.0;
    ffgain = (float)(anc_hdl->param.mode == ANC_TRANSPARENCY ? anc_hdl->param.gains.drctrans_norgain : anc_hdl->param.gains.drcff_norgain);
    for (i = 0; i < 4; i++) {
        if (anc_hdl->dy_mic_param[i].mult_flag) {
            anc_mic_gain = anc_hdl->dy_mic_param[i].gain;
            anc_hdl->dy_mic_param[i].gain = audio_mic_gain;
            flag = 1;
            user_anc_log("after mic%d en %d, gain %d, type %d, mult_flag %d\n", i, anc_hdl->dy_mic_param[i].en, \
                         anc_hdl->dy_mic_param[i].gain, anc_hdl->dy_mic_param[i].type, anc_hdl->dy_mic_param[i].mult_flag);
        }
    }
    if ((!flag) || (!(audio_mic_gain - anc_mic_gain))) {
        user_anc_log("There is no common MIC\n");
        anc_hdl->mic_dy_state = ANC_MIC_DY_STA_STOP;
        return;
    }
    anc_hdl->mic_dy_state = ANC_MIC_DY_STA_START;
    if (anc_hdl->mic_resume_timer) {
        sys_timer_del(anc_hdl->mic_resume_timer);
        anc_hdl->mic_resume_timer = 0;
    }
    anc_hdl->mic_diff_gain = audio_mic_gain - anc_mic_gain;
    multiple = (anc_hdl->mic_diff_gain < 0) ? (1.0f / multiple) : multiple;
    diff_gain = (anc_hdl->mic_diff_gain < 0) ? 0 - anc_hdl->mic_diff_gain : anc_hdl->mic_diff_gain;
    for (i = 0; i < diff_gain; i++) {
        ffgain /= multiple;
        anc_hdl->drc_ratio *= multiple;
    }
    audio_anc_drc_ctrl(&anc_hdl->param, anc_hdl->drc_ratio);
    audio_anc_mic_gain(anc_hdl->dy_mic_param, 1);
    g_printf("dynamic mic:audio_g %d,anc_g %d, diff_g %d, ff_gain %d, mult %d/100\n", audio_mic_gain, anc_mic_gain, anc_hdl->mic_diff_gain, (s16)ffgain, (int)(multiple * 100.0f));
}

void anc_dynamic_micgain_resume_timer(void *priv)
{
    float multiple = 1.259;
    audio_adc_mic_mana_t mic_param[4];
    if (anc_hdl->mic_diff_gain > 0) {
        anc_hdl->mic_diff_gain--;
        anc_hdl->drc_ratio /= multiple;
        if (!anc_hdl->mic_diff_gain) {
            anc_hdl->drc_ratio = 1.0;
        }
        for (int i = 0; i < 4; i++) {
            if (anc_hdl->dy_mic_param[i].mult_flag) {
                anc_hdl->dy_mic_param[i].gain--;
            }
        }
    } else if (anc_hdl->mic_diff_gain < 0) {
        anc_hdl->mic_diff_gain++;
        anc_hdl->drc_ratio *= multiple;
        if (!anc_hdl->mic_diff_gain) {
            anc_hdl->drc_ratio = 1.0;
        }
        for (int i = 0; i < 4; i++) {
            if (anc_hdl->dy_mic_param[i].mult_flag) {
                anc_hdl->dy_mic_param[i].gain++;
            }
        }
    } else {
        sys_timer_del(anc_hdl->mic_resume_timer);
        anc_hdl->mic_resume_timer = 0;
    }
    audio_anc_drc_ctrl(&anc_hdl->param, anc_hdl->drc_ratio);
    audio_anc_mic_gain(anc_hdl->dy_mic_param, 1);
}

void anc_dynamic_micgain_stop(void)
{
    if ((anc_hdl->mic_dy_state == ANC_MIC_DY_STA_START) && anc_status_get()) {
        anc_hdl->mic_dy_state = ANC_MIC_DY_STA_STOP;
        anc_hdl->mic_resume_timer = sys_timer_add(NULL, anc_dynamic_micgain_resume_timer, 10);
    }
}
#endif/*TCFG_AUDIO_DYNAMIC_ADC_GAIN*/

void audio_anc_post_msg_drc(void)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_DRC_TIMER);
}

void audio_anc_post_msg_debug(void)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_DEBUG_OUTPUT);
}

void audio_anc_post_msg_icsd_anc_cmd(u8 cmd)
{
    os_taskq_post_msg("anc", 2, ANC_MSG_ICSD_ANC_CMD, cmd);
}

void audio_anc_post_msg_user_train_init(u8 mode)
{
    if (mode == ANC_ON) {
        os_taskq_post_msg("anc", 1, ANC_MSG_USER_TRAIN_INIT);
    }
}

void audio_anc_post_msg_user_train_run(void)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_USER_TRAIN_RUN);
}

void audio_anc_post_msg_user_train_setparam(void)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_USER_TRAIN_SETPARAM);
}

void audio_anc_post_msg_user_train_timeout(void)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_USER_TRAIN_TIMEOUT);
}

void audio_anc_post_msg_user_train_end(void)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_USER_TRAIN_END);
}

#if TCFG_ANC_SELF_DUT_GET_SZ
void audio_anc_post_msg_sz_fft_open(u8 sel)
{
    os_taskq_post_msg("anc", 2, ANC_MSG_SZ_FFT_OPEN, sel);
}

void audio_anc_post_msg_sz_fft_run(void)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_SZ_FFT_RUN);
}

void audio_anc_post_msg_sz_fft_close(void)
{
    os_taskq_post_msg("anc", 1, ANC_MSG_SZ_FFT_CLOSE);
}

static void audio_anc_sz_fft_dac_check_slience_cb(void *buf, int len)
{
    if (anc_hdl->param.sz_fft.check_flag) {
        s16 *check_buf = (s16 *)buf;
        for (int i = 0; i < len / 2; i++) {
            if (check_buf[i]) {
                anc_hdl->param.sz_fft.check_flag = 0;
                audio_anc_sz_fft_trigger();
                audio_dac_write_callback_del(&dac_hdl, audio_anc_sz_fft_dac_check_slience_cb);
                break;
            }
        }
    }
}

//SZ_FFT测试函数
void audio_anc_sz_fft_test(void)
{
#if 0
    static u8 cnt = 0;
    g_printf("cnt %d\n", cnt);
    switch (cnt) {
    case 0:
        audio_anc_post_msg_sz_fft_open(4);
        break;
    case 1:
        audio_anc_sz_fft_trigger();
        /* play_tone_file(get_tone_files()->normal); */
        break;
    case 2:
        audio_anc_post_msg_sz_fft_close();
        break;
    case 3:
        audio_anc_sz_fft_outbuf_release();
        break;
    }
    if (++cnt > 3) {
        cnt = 0;
    }
#endif
}

#endif/* TCFG_ANC_SELF_DUT_GET_SZ*/


void audio_anc_dut_enable_set(u8 enablebit)
{
    anc_hdl->param.dut_audio_enablebit = enablebit;
}

/*
   ANC结构设置, 下一次切模式或复位ANC时生效
	param: enablebit ANC结构配置，如(ANC_FF_EN/ANC_FB_EN/ANC_HYBRID_EN)
		   update_flag 非ANC_OFF时，即时更新效果: 0 不更新；1 更新
 */
void audio_anc_enable_set(u8 enablebit, u8 update_flag)
{
    anc_hdl->param.tool_enablebit = enablebit;
    if ((anc_hdl->param.mode != ANC_OFF) && update_flag) {		//实时更新填入使用
        os_taskq_post_msg("anc", 2, ANC_MSG_RESET, 1);
    }
}

void audio_anc_post_msg_adt(u8 cmd)
{
    os_taskq_post_msg("anc", 2, ANC_MSG_ADT, cmd);
}

void audio_anc_mic_mana_set_gain(audio_anc_t *param, u8 num, u8 type)
{
    int i;
    switch (type) {
    case 0:
        param->mic_param[num].gain = param->gains.l_ffmic_gain;
        break;
    case 1:
        param->mic_param[num].gain = param->gains.l_fbmic_gain;
        break;
    case 2:
        param->mic_param[num].gain = param->gains.r_ffmic_gain;
        break;
    case 3:
        param->mic_param[num].gain = param->gains.r_fbmic_gain;
        break;
    }
    if ((param->ch == (ANC_L_CH | ANC_R_CH)) && param->mic_param[num].mult_flag) {
        for (i = 0; i < 4; i++) {
            if ((param->mic_param[i].type + 1) % 2) {
                param->mic_param[i].mult_flag = 1;
            }
        }
    }
}

/*设置fb  mic为复用mic*/
void audio_anc_mic_mana_fb_mult_set(u8 mult_flag)
{
    if (anc_hdl) {
        audio_anc_t *param = &anc_hdl->param;
        for (int i = 0; i < 4; i++) {
            if ((param->mic_param[i].type == ANC_MIC_TYPE_LFB) || (param->mic_param[i].type == ANC_MIC_TYPE_RFB)) {
                param->mic_param[i].mult_flag = mult_flag;
            }
        }
    }
}

void audio_anc_mic_management(audio_anc_t *param)
{
    int i;
    u8 double_mult_flag = 0;
    for (i = 0; i < 4; i++) {
        if (param->mic_type[i] == A_MIC0) {
            param->mic_param[0].en = 1;
            param->mic_param[0].type = i;
#if TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_0
            param->mic_param[0].mult_flag = 1;
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_0*/
            audio_anc_mic_mana_set_gain(param, 0, i);
        }
        if (param->mic_type[i] == A_MIC1) {
            param->mic_param[1].en = 1;
            param->mic_param[1].type = i;
#if TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_1
            param->mic_param[1].mult_flag = 1;
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_1*/
            audio_anc_mic_mana_set_gain(param, 1, i);
        }
        if (param->mic_type[i] == A_MIC2) {
            param->mic_param[2].en = 1;
            param->mic_param[2].type = i;
#if TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_2
            param->mic_param[2].mult_flag = 1;
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_2*/
            audio_anc_mic_mana_set_gain(param, 2, i);
        }
        if (param->mic_type[i] == A_MIC3) {
            param->mic_param[3].en = 1;
            param->mic_param[3].type = i;
#if TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_3
            param->mic_param[3].mult_flag = 1;
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_3*/
            audio_anc_mic_mana_set_gain(param, 3, i);
        }
    }
    for (i = 0; i < 4; i++) {
        printf("mic%d en %d, gain %d, type %d, mult_flag %d\n", i, param->mic_param[i].en, \
               param->mic_param[i].gain, param->mic_param[i].type, param->mic_param[i].mult_flag);
    }
}

void audio_anc_adc_ch_set(void)
{
#if TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_0
    anc_hdl->param.adc_ch = (anc_hdl->param.adc_ch << 1) | 1;
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_0*/
#if TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_1
    anc_hdl->param.adc_ch = (anc_hdl->param.adc_ch << 1) | 1;
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_1*/
#if TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_2
    anc_hdl->param.adc_ch = (anc_hdl->param.adc_ch << 1) | 1;
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_2*/
#if TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_3
    anc_hdl->param.adc_ch = (anc_hdl->param.adc_ch << 1) | 1;
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_3*/
    if (!anc_hdl->param.adc_ch) {	//通话没有使用模拟MIC，则adc_ch默认赋1
        anc_hdl->param.adc_ch = 1;
    }

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    /* 开免摘功能后，根据通话mic和免摘mic个数，取最大值来打开ADC的数字通道，保证免摘ADC取数正常 */
    u8 esco_mic_num = audio_adc_file_get_esco_mic_num();
    u8 adt_mic_num = get_icsd_adt_mic_num();
    u8 mic_num = esco_mic_num > adt_mic_num ? esco_mic_num : adt_mic_num;
    if (mic_num == 2) {
        anc_hdl->param.adc_ch = BIT(1) | BIT(0);
    } else {
        anc_hdl->param.adc_ch = BIT(2) | BIT(1) | BIT(0);
    }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

}
#if ANC_ADAPTIVE_EN
static void audio_anc_adap_tone_play(int ref_lvl, int err_lvl)
{
#if ANC_ADAPTIVE_TONE_EN
    if (ref_lvl == 1) {
        tone_play(TONE_ANC_ADAPTIVE_LVL1, 1);
    } else if (ref_lvl == 0) {
        tone_play(TONE_ANC_ADAPTIVE_LVL2, 1);
    }
#endif/*ANC_ADAPTIVE_TONE_EN*/
    anc_pow_adap_fade_timer_add(ref_lvl, err_lvl);
}

void audio_anc_adap_sync(int ref_lvl, int err_lvl, int err)
{
    os_taskq_post_msg("anc", 3, ANC_MSG_ADAPTIVE_SYNC, ref_lvl, err_lvl);
}

extern void bt_tws_anc_adap_sync_send(int ref_lvl, int err_lvl, int msec);
extern int bt_tws_anc_adap_compare_send(int ref_lvl, int err_lvl);

void audio_anc_adap_lvl_compare(int ref_lvl, int err_lvl)
{
#if TCFG_USER_TWS_ENABLE
    int ref_lvl_temp, err_lvl_temp;
    ref_lvl_temp = anc_pow_adap_lvl_get(ANC_FF_EN);
    err_lvl_temp = anc_pow_adap_lvl_get(ANC_FB_EN);
    g_printf("compare ref_lvl_temp %d, %d, lvl %d, %d\n", ref_lvl_temp, err_lvl_temp, ref_lvl, err_lvl);
    if (ref_lvl_temp >= ref_lvl) {
        bt_tws_anc_adap_sync_send(ref_lvl, err_lvl, 150);
    }
    if (err_lvl_temp >= ref_lvl) {
        bt_tws_anc_adap_sync_send(ref_lvl, err_lvl, 150);
    }
#endif/*TCFG_USER_TWS_ENABLE*/
}

int audio_anc_pow_adap_deal(int ref_lvl, int err_lvl)
{
#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state()) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            user_anc_log("[master]anc_pow_adap send %d %d\n", ref_lvl, err_lvl);
        } else {
            user_anc_log("[slave]anc_pow_adap send %d %d\n", ref_lvl, err_lvl);
        }
        bt_tws_anc_adap_compare_send(ref_lvl, err_lvl);
    } else {
        user_anc_log("[no tws]anc_pow_adap send %d %d\n", ref_lvl, err_lvl);
        audio_anc_adap_tone_play(ref_lvl, err_lvl);
    }
#else
    user_anc_log("[no tws]anc_pow_adap send %d %d\n", ref_lvl, err_lvl);
    audio_anc_adap_tone_play(ref_lvl, err_lvl);
#endif/*TCFG_USER_TWS_ENABLE*/
    return 0;
}

/*切换增益模式*/
void audio_anc_adaptive_mode_set(u8 mode, u8 lvl)
{
    if (anc_hdl) {
        anc_hdl->param.adaptive_mode = mode;
        audio_anc_pow_export_en((mode == ANC_ADAPTIVE_GAIN_MODE), 0);
        if (anc_hdl->param.mode == ANC_ON) {
            switch (mode) {
            case ANC_ADAPTIVE_MANUAL_MODE:
                user_anc_log("ANC_ADAPTIVE_MANUAL_MODE ,lvl%d\n", lvl);
                audio_anc_pow_adap_stop();
                anc_hdl->param.anc_lvl = lvl;
                audio_anc_lvl_set(lvl, 1);
                break;
            case ANC_ADAPTIVE_GAIN_MODE:
                user_anc_log("ANC_ADAPTIVE_GAIN_MODE\n");
                anc_hdl->param.anc_fade_gain = 16384;
                audio_anc_pow_adap_start();
                break;
            }
        }
    }
}

void audio_anc_pow_adap_fade(u16 gain)
{
    audio_anc_fade_ctr_set(ANC_FADE_MODE_SCENE_ADAPTIVE, AUDIO_ANC_FDAE_CH_ALL, gain);
}

#endif/*ANC_ADAPTIVE_EN*/

#if ANC_MUSIC_DYNAMIC_GAIN_EN

#define ANC_MUSIC_DYNAMIC_GAIN_NORGAIN				1024	/*正常默认增益*/
#define ANC_MUSIC_DYNAMIC_GAIN_TRIGGER_INTERVAL		6		/*音乐响度取样间隔*/

#if ANC_MUSIC_DYNAMIC_GAIN_SINGLE_FB
void audio_anc_music_dyn_gain_resume_timer(void *priv)
{
    if (anc_hdl->loud_nowgain < anc_hdl->loud_targetgain) {
        anc_hdl->loud_nowgain += 5;      ///+5耗时194 * 10ms
        if (anc_hdl->loud_nowgain >= anc_hdl->loud_targetgain) {
            anc_hdl->loud_nowgain = anc_hdl->loud_targetgain;
            sys_timer_del(anc_hdl->loud_timeid);
            anc_hdl->loud_timeid = 0;
        }
    } else if (anc_hdl->loud_nowgain > anc_hdl->loud_targetgain) {
        anc_hdl->loud_nowgain -= 5;      ///-5耗时194 * 10ms
        if (anc_hdl->loud_nowgain <= anc_hdl->loud_targetgain) {
            anc_hdl->loud_nowgain = anc_hdl->loud_targetgain;
            sys_timer_del(anc_hdl->loud_timeid);
            anc_hdl->loud_timeid = 0;
        }
    }
    audio_anc_fbgain_set(anc_hdl->loud_nowgain, anc_hdl->loud_nowgain);
    /* user_anc_log("fb resume anc_hdl->loud_nowgain %d\n", anc_hdl->loud_nowgain); */
}

#endif/*ANC_MUSIC_DYNAMIC_GAIN_SINGLE_FB*/

void audio_anc_music_dynamic_gain_process(void)
{
    if (anc_hdl->loud_nowgain != anc_hdl->loud_targetgain) {
//        user_anc_log("low_nowgain %d, targergain %d\n", anc_hdl->loud_nowgain, anc_hdl->loud_targetgain);
#if ANC_MUSIC_DYNAMIC_GAIN_SINGLE_FB
        if (!anc_hdl->loud_timeid) {
            anc_hdl->loud_timeid = sys_timer_add(NULL, audio_anc_music_dyn_gain_resume_timer, 10);
        }
#else
        anc_hdl->loud_nowgain = anc_hdl->loud_targetgain;
        audio_anc_fade_ctr_set(ANC_FADE_MODE_MUSIC_DYNAMIC, AUDIO_ANC_FDAE_CH_ALL, anc_hdl->loud_targetgain << 4);
#endif/*ANC_MUSIC_DYNAMIC_GAIN_SINGLE_FB*/
    }
}

void audio_anc_music_dynamic_gain_reset(u8 effect_reset_en)
{
    if (anc_hdl->loud_targetgain != ANC_MUSIC_DYNAMIC_GAIN_NORGAIN) {
        /* user_anc_log("audio_anc_music_dynamic_gain_reset\n"); */
        anc_hdl->loud_targetgain = ANC_MUSIC_DYNAMIC_GAIN_NORGAIN;
        if (anc_hdl->loud_timeid) {
            sys_timer_del(anc_hdl->loud_timeid);
            anc_hdl->loud_timeid = 0;
        }
        if (effect_reset_en && anc_hdl->param.mode == ANC_ON) {
            os_taskq_post_msg("anc", 1, ANC_MSG_MUSIC_DYN_GAIN);
        } else {
            anc_hdl->loud_nowgain = ANC_MUSIC_DYNAMIC_GAIN_NORGAIN;
            anc_hdl->loud_dB = 1;	//清除上次保留的dB值, 避免切模式后不触发的情况
            anc_hdl->loud_hdl.peak_val = ANC_MUSIC_DYNAMIC_GAIN_THR;
        }
    }
}

void audio_anc_music_dynamic_gain_dB_get(int dB)
{
    float mult = 1.122f;
    float target_mult = 1.0f;
    int cnt, i, gain;
#if ANC_MUSIC_DYNAMIC_GAIN_TRIGGER_INTERVAL > 1 //区间变化，避免频繁操作
    if (dB > ANC_MUSIC_DYNAMIC_GAIN_THR) {
        int temp_dB = dB / ANC_MUSIC_DYNAMIC_GAIN_TRIGGER_INTERVAL;
        dB = temp_dB * ANC_MUSIC_DYNAMIC_GAIN_TRIGGER_INTERVAL;
    }
#endif/*ANC_MUSIC_DYNAMIC_GAIN_TRIGGER_INTERVAL*/
    if (anc_hdl->loud_dB != dB) {
        anc_hdl->loud_dB = dB;
        if (dB > ANC_MUSIC_DYNAMIC_GAIN_THR) {
            cnt = dB - ANC_MUSIC_DYNAMIC_GAIN_THR;
            for (i = 0; i < cnt; i++) {
                target_mult *= mult;
            }
            gain = (int)((float)ANC_MUSIC_DYNAMIC_GAIN_NORGAIN / target_mult);
//            user_anc_log("cnt %d, dB %d, gain %d\n", cnt, dB, gain);
            anc_hdl->loud_targetgain = gain;
            os_taskq_post_msg("anc", 1, ANC_MSG_MUSIC_DYN_GAIN);
        } else {
            audio_anc_music_dynamic_gain_reset(1);
        }
    }
}

void audio_anc_music_dynamic_gain_det(s16 *data, int len)
{
    /* putchar('l'); */
    if (anc_hdl->param.mode == ANC_ON) {
        loudness_meter_short(&anc_hdl->loud_hdl, data, len >> 1);
        audio_anc_music_dynamic_gain_dB_get(anc_hdl->loud_hdl.peak_val);
    }
}


void audio_anc_music_dynamic_gain_init(int sr)
{
    /* user_anc_log("audio_anc_music_dynamic_gain_init %d\n", sr); */
    anc_hdl->loud_nowgain = ANC_MUSIC_DYNAMIC_GAIN_NORGAIN;
    anc_hdl->loud_targetgain = ANC_MUSIC_DYNAMIC_GAIN_NORGAIN;
    anc_hdl->loud_dB = 1;     //清除上次保留的dB值
    loudness_meter_init(&anc_hdl->loud_hdl, sr, 50, 0);
    anc_hdl->loud_hdl.peak_val = ANC_MUSIC_DYNAMIC_GAIN_THR;
}
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/

#if ANC_EAR_ADAPTIVE_EN
extern struct audio_decoder_task decode_task;
/* 解码任务回调,执行自适应切换 */

void audio_anc_dac_check_slience_cb(void *buf, int len)
{
    if (anc_hdl->dac_check_slience_flag) {
        s16 *check_buf = (s16 *)buf;
        for (int i = 0; i < len / 2; i++) {
            if (check_buf[i]) {
                anc_hdl->dac_check_slience_flag = 0;
                icsd_anctone_dacon(i, dac_hdl.sample_rate);
                audio_dac_write_callback_del(&dac_hdl, audio_anc_dac_check_slience_cb);
                break;
            }
        }
    }
}

void audio_anc_ear_adaptive_open(void)
{
#if ANC_MULT_ORDER_ENABLE
    //载入场景参数-提供自适应训练过程使用
    u8 scene_id = ANC_MULT_ADPTIVE_TRAIN_USE_ID ? ANC_MULT_ADPTIVE_TRAIN_USE_ID : anc_hdl->scene_id;
    anc_mult_scene_set(scene_id);
#endif/*ANC_MULT_ORDER_ENABLE*/
    anc_hdl->ear_adaptive = 1;
    anc_hdl->param.anc_coeff_mode = ANC_COEFF_MODE_ADAPTIVE;	//切到自适应的参数
    anc_hdl->ear_adaptive_busy = 1;
    audio_anc_ear_adaptive_param_init(&anc_hdl->param);
#if ANC_USER_TRAIN_TONE_MODE
    anc_hdl->dac_check_slience_flag = 1;
    audio_dac_write_callback_add(&dac_hdl, audio_anc_dac_check_slience_cb);
#endif/*ANC_USER_TRAIN_TONE_MODE*/
    /*自适应时需要挂起adt*/
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    audio_anc_mode_switch_in_adt(ANC_ON);
#endif
    anc_mode_switch(ANC_ON, ANC_USER_TRAIN_TONE_MODE);
}

static int audio_anc_adaptive_wait_handler(struct audio_res_wait *wait, int event)
{
    if (event == AUDIO_RES_GET) {
        if (anc_hdl->ear_adaptive_dec_wait == ANC_ADAPTIVE_DEC_WAIT_START) {
            //避免重复进入
            anc_hdl->ear_adaptive_dec_wait = ANC_ADAPTIVE_DEC_WAIT_RUN;
            audio_anc_ear_adaptive_open();
        }
    } else {
        g_printf("audio_anc.c AUDIO_RES_PUT\n");
    }
    return 0;
}

/* 自适应解码任务注册 */
static int audio_anc_decoder_task_add(int parm)
{
    anc_hdl->adaptive_wait.priority = 2;				//优先级
    anc_hdl->adaptive_wait.preemption = 1;			//打断方式
    anc_hdl->adaptive_wait.handler = audio_anc_adaptive_wait_handler;//解码任务回调
    anc_hdl->ear_adaptive_dec_wait = ANC_ADAPTIVE_DEC_WAIT_START;
    audio_decoder_task_add_wait(&decode_task, &anc_hdl->adaptive_wait);
    return 0;
}

/*
   自适应模式-重新检测
   param: tws_sync_en	 	1 TWS同步自适应，支持TWS降噪平衡，需左右耳一起调用此接口
   							0 单耳自适应, 不支持TWS降噪平衡，可TWS状态下单耳自适应
 */
int audio_anc_mode_ear_adaptive(u8 tws_sync_en)
{
    //不支持重入自适应模式，仅在ANC模式，非通话下进入自适应模式
    anc_hdl->ear_adaptive_tws_sync = tws_sync_en;

    if (anc_hdl->param.mode == ANC_ON && !anc_hdl->ear_adaptive_busy \
        && (bt_phone_dec_is_running() == 0) && !anc_hdl->mode_switch_lock
       ) {
        if (bt_media_is_running()) {	//当前处于音乐播放状态, 注册解码任务打断，进入自适应
            /* if (0) {	//当前处于音乐播放状态, 注册解码任务打断，进入自适应 */
            if (strcmp(os_current_task(), "app_core") != 0) {
                int *ptr = malloc(2 * sizeof(int));
                ptr[0] = 0;
                ptr[1] = 0;
                OS_SEM *sem = malloc(sizeof(OS_SEM) + 4);
                int *err = (int *)(sem + 1);
                ASSERT(sem);
                int msg[8];
                msg[0] = (int)audio_anc_decoder_task_add;
                msg[1] = 1 | BIT(8) | BIT(9);
                msg[2] = (int)ptr;
                msg[3] = (int)err;
                msg[4] = (int)sem;

                os_sem_create(sem, 0);
                int ret_err;
                while (1) {
                    int ret_err = os_taskq_post_type("app_core", Q_CALLBACK, 5, msg);
                    if (ret_err == OS_ERR_NONE) {
                        os_sem_pend(sem, 0);
                        ret_err = *err;
                        break;
                    }
                    if (ret_err != OS_Q_FULL) {
                        break;
                    }
                    os_time_dly(2);
                }
                free(sem);
                free(ptr);
                g_printf("switch to app_core paly tone ok\n");
                return 0;
            } else {
                audio_anc_decoder_task_add(0);
                return 0;
            }
        }
        audio_anc_ear_adaptive_open();
        return 0;
    }
#if (RCSP_ADV_EN && RCSP_ADV_ANC_VOICE && RCSP_ADV_ADAPTIVE_NOISE_REDUCTION)
    set_adaptive_noise_reduction_reset_callback(0);		//	无法进入自适应，返回失败结果
#endif
    return 1;
}


/*自适应结束回调函数*/
void anc_user_train_cb(u8 mode, u8 result, u8 forced_exit)
{
    user_anc_log("anc_adaptive end result 0x%x , coeff_use 0x%x\n", result, anc_hdl->adaptive_iir.result);
    audio_anc_t *param = &anc_hdl->param;
    anc_hdl->ear_adaptive_busy = 0;

    if (forced_exit == 1) {
        //强制打断，自适应定义为失败，使用默认参数
        anc_hdl->adaptive_iir.result = 0;
        //删除自适应DAC回调接口
        audio_dac_write_callback_del(&dac_hdl, audio_anc_dac_check_slience_cb);
    }

    if (anc_hdl->adaptive_iir.result) {	//自适应成功或部分成功
        if (anc_hdl->ear_adaptive_data_from == ANC_ADAPTIVE_DATA_FROM_VM) {		//释放掉原来VM使用的资源
            if (anc_hdl->param.adaptive->lff_coeff) {
                free(anc_hdl->param.adaptive->lff_coeff);
            };
            if (anc_hdl->param.adaptive->lfb_coeff) {
                free(anc_hdl->param.adaptive->lfb_coeff);
            };
            if (anc_hdl->param.adaptive->lcmp_coeff) {
                free(anc_hdl->param.adaptive->lcmp_coeff);
            };
            if (anc_hdl->param.adaptive->rff_coeff) {
                free(anc_hdl->param.adaptive->rff_coeff);
            };
            if (anc_hdl->param.adaptive->rfb_coeff) {
                free(anc_hdl->param.adaptive->rfb_coeff);
            };
            if (anc_hdl->param.adaptive->rcmp_coeff) {
                free(anc_hdl->param.adaptive->rcmp_coeff);
            };
        }
        anc_hdl->ear_adaptive_data_from = ANC_ADAPTIVE_DATA_FROM_ALOGM;	//自适应参数更新
#if (ANC_CH & ANC_L_CH)
        param->adaptive->lff_coeff = param->lff_coeff;
        param->adaptive->lfb_coeff = param->lfb_coeff;
#if ANC_EAR_ADAPTIVE_CMP_EN
        param->adaptive->lcmp_coeff = param->lcmp_coeff;
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*(ANC_CH & ANC_L_CH)*/
#if (ANC_CH & ANC_R_CH)
        param->adaptive->rff_coeff = param->rff_coeff;
        param->adaptive->rfb_coeff = param->rfb_coeff;
#if ANC_EAR_ADAPTIVE_CMP_EN
        param->adaptive->rcmp_coeff = param->rcmp_coeff;
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*(ANC_CH & ANC_R_CH)*/
        if (result != ANC_SUCCESS) {	//非FF+FB同时成功
            g_printf("no all success");
            audio_anc_coeff_adaptive_set(ANC_COEFF_MODE_NORMAL, 0, 0);	//先覆盖默认参数
            audio_anc_coeff_adaptive_set(ANC_COEFF_MODE_ADAPTIVE, 0, 0);	//根据自适应结果选择覆盖参数
        }

#if (!ANC_POWEOFF_SAVE_ADAPTIVE_DATA) || ANC_DEVELOPER_MODE_EN
        audio_anc_adaptive_data_save(&anc_hdl->adaptive_iir);
#endif/*ANC_POWEOFF_SAVE_ADAPTIVE_DATA*/
    } else {	//自适应失败使用默认的参数
        audio_anc_coeff_adaptive_set(ANC_COEFF_MODE_NORMAL, 0, 0);
    }
    if (anc_hdl->ear_adaptive_dec_wait) {
        anc_hdl->ear_adaptive_dec_wait = ANC_ADAPTIVE_DEC_WAIT_STOP;
        audio_decoder_task_del_wait(&decode_task, &anc_hdl->adaptive_wait);
    }
    anc_mode_switch_deal(mode);
}

/*ANC滤波器模式循环切换*/
int audio_anc_coeff_adaptive_switch()
{
    if (anc_hdl->param.mode == ANC_ON && !anc_hdl->ear_adaptive_busy && !anc_hdl->mode_switch_lock) {
        anc_hdl->param.anc_coeff_mode ^= 1;
        audio_anc_coeff_adaptive_set(anc_hdl->param.anc_coeff_mode, 1, 1);
    }
    return 0;
}

/*当前ANC滤波器模式获取 0:普通参数 1:自适应参数*/
int audio_anc_coeff_mode_get(void)
{
    if (!anc_hdl) {
        return 0;
    }
    return anc_hdl->param.anc_coeff_mode;
}

/*
   自适应/普通参数切换
	param:  mode 		0 使用普通参数; 1 使用自适应参数
			tone_play 	0 不播放提示音；1 播放提示音
			update_flag 0 不更新效果；	1 即时更新效果
 */
int audio_anc_coeff_adaptive_set(u32 mode, u8 tone_play, u8 update_flag)
{
    audio_anc_t *param = &anc_hdl->param;
    u8 result = anc_hdl->adaptive_iir.result;
    if (anc_hdl->ear_adaptive_busy) {
        return 0;
    }
    anc_hdl->param.anc_coeff_mode = mode;
    switch (mode) {
    case 0:
        if (tone_play) {
            tone_play_index(IDEX_TONE_ANC_NORMAL_COEFF, ANC_TONE_PREEMPTION);
        }
#if ANC_COEFF_SAVE_ENABLE
#if ANC_MULT_ORDER_ENABLE
        //自适应切默认参数，回到当前场景
        anc_mult_scene_set(anc_hdl->scene_id);
#else
        if (audio_anc_db_cfg_read()) {
            return 1;
        }
#endif/*ANC_MULT_ORDER_ENABLE*/
#endif/*ANC_COEFF_SAVE_ENABLE*/
        break;
    case 1:
        if (((!anc_hdl->param.adaptive->lff_coeff) && anc_hdl->param.lff_en) || \
            ((!anc_hdl->param.adaptive->lfb_coeff) && anc_hdl->param.lfb_en) || \
            ((!anc_hdl->param.adaptive->rff_coeff) && anc_hdl->param.rff_en) || \
            ((!anc_hdl->param.adaptive->rfb_coeff) && anc_hdl->param.rfb_en)) {
            g_printf("anc no adaptive data\n");
            return 1;
        }
        if (tone_play) {
            tone_play_index(IDEX_TONE_ANC_ADAPTIVE_COEFF, ANC_TONE_PREEMPTION);
        }
#if ANC_MULT_ORDER_ENABLE
        //载入场景参数-提供自适应部分成功时，匹配使用。
        u8 scene_id = ANC_MULT_ADPTIVE_MATCH_USE_ID ? ANC_MULT_ADPTIVE_MATCH_USE_ID : anc_hdl->scene_id;
        anc_mult_scene_set(scene_id);
#endif/*ANC_MULT_ORDER_ENABLE*/
        audio_anc_ear_adaptive_param_init(&anc_hdl->param);	//初始化自适应固定配置参数
#if ANC_CH & ANC_L_CH
        if (result & ANC_ADAPTIVE_RESULT_LFF) {
            user_anc_log("adaptive coeff set lff\n");
            anc_hdl->param.gains.l_ffgain = (anc_hdl->adaptive_iir.lff_gain < 0) ? (0 - anc_hdl->adaptive_iir.lff_gain) : anc_hdl->adaptive_iir.lff_gain;
            anc_hdl->param.lff_yorder = ANC_ADAPTIVE_FF_ORDER;
            anc_hdl->param.lff_coeff = anc_hdl->param.adaptive->lff_coeff;
        }
        if (result & ANC_ADAPTIVE_RESULT_LFB) {
            user_anc_log("adaptive coeff set lfb\n");
            anc_hdl->param.gains.l_fbgain = (anc_hdl->adaptive_iir.lfb_gain < 0) ? (0 - anc_hdl->adaptive_iir.lfb_gain) : anc_hdl->adaptive_iir.lfb_gain;
            anc_hdl->param.lfb_yorder = ANC_ADAPTIVE_FB_ORDER;
            anc_hdl->param.lfb_coeff = anc_hdl->param.adaptive->lfb_coeff;
        }
#if ANC_EAR_ADAPTIVE_CMP_EN
        if (result & ANC_ADAPTIVE_RESULT_LCMP) {
            user_anc_log("adaptive coeff set lcmp\n");
            anc_hdl->param.gains.l_cmpgain = (anc_hdl->adaptive_iir.lcmp_gain < 0) ? (0 - anc_hdl->adaptive_iir.lcmp_gain) : anc_hdl->adaptive_iir.lcmp_gain;
            anc_hdl->param.lcmp_coeff = anc_hdl->param.adaptive->lcmp_coeff;
            anc_hdl->param.lcmp_yorder = ANC_ADAPTIVE_CMP_ORDER;
        }
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CH*/

#if ANC_CH & ANC_R_CH
        if (result & ANC_ADAPTIVE_RESULT_RFF) {
            user_anc_log("adaptive coeff set rff\n");
            anc_hdl->param.gains.r_ffgain = (anc_hdl->adaptive_iir.rff_gain < 0) ? (0 - anc_hdl->adaptive_iir.rff_gain) : anc_hdl->adaptive_iir.rff_gain;
            anc_hdl->param.rff_yorder = ANC_ADAPTIVE_FF_ORDER;
            anc_hdl->param.rff_coeff = anc_hdl->param.adaptive->rff_coeff;
        }
        if (result & ANC_ADAPTIVE_RESULT_RFB) {
            user_anc_log("adaptive coeff set rfb\n");
            anc_hdl->param.gains.r_fbgain = (anc_hdl->adaptive_iir.rfb_gain < 0) ? (0 - anc_hdl->adaptive_iir.rfb_gain) : anc_hdl->adaptive_iir.rfb_gain;
            anc_hdl->param.rfb_yorder = ANC_ADAPTIVE_FB_ORDER;
            anc_hdl->param.rfb_coeff = anc_hdl->param.adaptive->rfb_coeff;
        }
#if ANC_EAR_ADAPTIVE_CMP_EN
        if (result & ANC_ADAPTIVE_RESULT_RCMP) {
            user_anc_log("adaptive coeff set rcmp\n");
            anc_hdl->param.gains.r_cmpgain = (anc_hdl->adaptive_iir.rcmp_gain < 0) ? (0 - anc_hdl->adaptive_iir.rcmp_gain) : anc_hdl->adaptive_iir.rcmp_gain;
            anc_hdl->param.rcmp_coeff = anc_hdl->param.adaptive->rcmp_coeff;
            anc_hdl->param.rcmp_yorder = ANC_ADAPTIVE_CMP_ORDER;
        }
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CH*/

        break;
    default:
        return 1;
    }
    if ((anc_hdl->param.mode == ANC_ON) && update_flag) {		//实时更新填入使用
        os_taskq_post_msg("anc", 2, ANC_MSG_RESET, 1);
        /* audio_anc_reset(&anc_hdl->param); */
    }

    //put_buf(param->lff_coeff, ANC_ADAPTIVE_FF_ORDER * 5 * 8);
    //put_buf(param->lfb_coeff, ANC_ADAPTIVE_FB_ORDER * 5 * 8);
    //put_buf(param->lcmp_coeff, ANC_ADAPTIVE_CMP_ORDER * 5 * 8);
    return 0;
}

void audio_anc_adaptive_data_format(anc_adaptive_iir_t *iir)
{
    if (anc_hdl->ear_adaptive_data_from == ANC_ADAPTIVE_DATA_EMPTY) {
        anc_hdl->ear_adaptive_data_from = ANC_ADAPTIVE_DATA_FROM_VM;
        if (anc_hdl->param.lff_en) {
            anc_hdl->param.adaptive->lff_coeff = malloc(ANC_ADAPTIVE_FF_ORDER * sizeof(double) * 5);
        }
        if (anc_hdl->param.lfb_en) {
            anc_hdl->param.adaptive->lfb_coeff = malloc(ANC_ADAPTIVE_FB_ORDER * sizeof(double) * 5);
#if ANC_EAR_ADAPTIVE_CMP_EN
            anc_hdl->param.adaptive->lcmp_coeff = malloc(ANC_ADAPTIVE_CMP_ORDER * sizeof(double) * 5);
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
        }
        if (anc_hdl->param.rff_en) {
            anc_hdl->param.adaptive->rff_coeff = malloc(ANC_ADAPTIVE_FF_ORDER * sizeof(double) * 5);
        }
        if (anc_hdl->param.rfb_en) {
            anc_hdl->param.adaptive->rfb_coeff = malloc(ANC_ADAPTIVE_FB_ORDER * sizeof(double) * 5);
#if ANC_EAR_ADAPTIVE_CMP_EN
            anc_hdl->param.adaptive->rcmp_coeff = malloc(ANC_ADAPTIVE_CMP_ORDER * sizeof(double) * 5);
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
        }

        anc_iir_t iir_lib;	//	库与外部指针分离
#if ANC_CONFIG_LFF_EN
        iir_lib.lff = anc_hdl->adaptive_iir.lff;
#endif/*ANC_CONFIG_LFF_EN*/
#if ANC_CONFIG_LFB_EN
        iir_lib.lfb = anc_hdl->adaptive_iir.lfb;
#if ANC_EAR_ADAPTIVE_CMP_EN
        iir_lib.lcmp = anc_hdl->adaptive_iir.lcmp;
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CONFIG_LFB_EN*/
#if ANC_CONFIG_RFF_EN
        iir_lib.rff = anc_hdl->adaptive_iir.rff;
#endif/*ANC_CONFIG_RFF_EN*/
#if ANC_CONFIG_RFB_EN
        iir_lib.rfb = anc_hdl->adaptive_iir.rfb;
#if ANC_EAR_ADAPTIVE_CMP_EN
        iir_lib.rcmp = anc_hdl->adaptive_iir.rcmp;
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CONFIG_RFB_EN*/
        audio_anc_adaptive_data_analsis(&iir_lib);
    }
}

int audio_anc_adaptive_data_save(anc_adaptive_iir_t *iir)
{
    printf("%s\n", __func__);
#if ANC_DEVELOPER_MODE_EN
    //保存前做数据格式处理，避免数据异常导致上电转换数据死机
    anc_hdl->ear_adaptive_data_from = ANC_ADAPTIVE_DATA_EMPTY;
    audio_anc_adaptive_data_format(iir);
#endif/*ANC_DEVELOPER_MODE_EN*/
    int ret = syscfg_write(CFG_ANC_ADAPTIVE_DATA_ID, (u8 *)iir, sizeof(anc_adaptive_iir_t));
    /* printf("vmdata_num %d\n", *vmdata_num); */
    /* put_buf((u8*)vmdata_FL, EAR_RECORD_MAX*25*4); */
    /* put_buf((u8*)vmdata_FR, EAR_RECORD_MAX*25*4); */
    g_printf("ret %d, %d\n", ret, sizeof(anc_adaptive_iir_t));
    return ret;
}


int audio_anc_adaptive_data_read(anc_adaptive_iir_t *iir)
{
    int ret = syscfg_read(CFG_ANC_ADAPTIVE_DATA_ID, (u8 *)iir, sizeof(anc_adaptive_iir_t));
    if (ret == sizeof(anc_adaptive_iir_t)) {	//能读取到参数, 则默认进入自适应
        if (!CRC16((u8 *)iir, sizeof(anc_adaptive_iir_t))) {	//读到的数全为0
            r_printf("adaptive data is emtpy!!!\n");
            return -1;
        }
        g_printf("do adaptive, ret %d\n", ret);

    } else {
        return -1;
        //给予自适应默认参数
    }

    audio_anc_adaptive_poweron_catch_data(iir);	//开机需确认左右耳
    audio_anc_adaptive_data_format(iir);
    audio_anc_coeff_adaptive_set(ANC_COEFF_MODE_ADAPTIVE, 0, 0);
    return 0;
}

/*判断当前是否处于训练中*/
u8 audio_anc_adaptive_busy_get(void)
{
    return anc_hdl->ear_adaptive_busy;
}

/* 自适应默认参数更新 */
void audio_anc_ear_adaptive_param_init(audio_anc_t *param)
{
    param->gains.ff_1st_dcc = icsd_dcc[0];
    param->gains.ff_2nd_dcc = icsd_dcc[1];
    param->gains.fb_1st_dcc = icsd_dcc[2];
    param->gains.fb_2nd_dcc = icsd_dcc[3];
}

void audio_anc_adaptive_fr_fail_fill(u8 *dat, u8 order, u8 result)
{
    u8 fr_fail[13] = {0x2, 0x0,  0x0, 0xc8, 0x42, \
                      0x0,  0x0, 0x0,  0x0, \
                      0x0,  0x0, 0x80, 0x3f
                     };
    if (!result) {
        for (int i = 0; i < order; i++) {
            memcpy(dat + 4 + i * 13, fr_fail, 13);
        }
    }
}

extern void anc_packet_show(anc_packet_data_t *packet);
void audio_anc_adaptive_data_packet(struct icsd_anc_tool_data *TOOL_DATA)
{
    int len = TOOL_DATA->h_len;
    int fb_yorder = TOOL_DATA->yorderb;
    int ff_yorder = TOOL_DATA->yorderf;
    int cmp_yorder = TOOL_DATA->yorderc;

    int ff_dat_len =  sizeof(anc_fr_t) * ff_yorder + 4;
    int fb_dat_len =  sizeof(anc_fr_t) * fb_yorder + 4;
    int cmp_dat_len =  sizeof(anc_fr_t) * cmp_yorder + 4;
    anc_adaptive_iir_t *iir = &anc_hdl->adaptive_iir;
    u8 *ff_dat, *fb_dat, *cmp_dat, *rff_dat, *rfb_dat, *rcmp_dat;
    u8 result = icsd_anc_train_result_get(TOOL_DATA);

    anc_hdl->adaptive_iir.result = result;	//记录当前参数的保存结果;

#if ANC_CONFIG_LFF_EN
    ff_dat = (u8 *)&iir->lff_gain;
    audio_anc_fr_format(ff_dat, TOOL_DATA->data_out5, ff_yorder, ff_iir_type);
    audio_anc_adaptive_fr_fail_fill(ff_dat, ff_yorder, (result & ANC_ADAPTIVE_RESULT_LFF));
#endif/*ANC_CONFIG_LFF_EN*/
#if ANC_CONFIG_LFB_EN
    fb_dat = (u8 *)&iir->lfb_gain;
    audio_anc_fr_format(fb_dat, TOOL_DATA->data_out4, fb_yorder, fb_iir_type);
    audio_anc_adaptive_fr_fail_fill(fb_dat, fb_yorder, (result & ANC_ADAPTIVE_RESULT_LFB));
#if ANC_EAR_ADAPTIVE_CMP_EN
    cmp_dat = (u8 *)&iir->lcmp_gain;
    audio_anc_fr_format(cmp_dat, TOOL_DATA->data_out11, cmp_yorder, cmp_iir_type);
    audio_anc_adaptive_fr_fail_fill(cmp_dat, cmp_yorder, (result & ANC_ADAPTIVE_RESULT_LCMP));
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CONFIG_LFB_EN*/
#if ANC_CONFIG_RFF_EN
    rff_dat = (u8 *)&iir->rff_gain;
    if (!ANC_CONFIG_LFF_EN) { 	//TWS 单R声道使用
        ff_dat = rff_dat;
        audio_anc_fr_format(ff_dat, TOOL_DATA->data_out5, ff_yorder, ff_iir_type);
        audio_anc_adaptive_fr_fail_fill(ff_dat, ff_yorder, (result & ANC_ADAPTIVE_RESULT_LFF));
    } else {
        audio_anc_fr_format(rff_dat, TOOL_DATA->data_out10, ff_yorder, ff_iir_type);
        audio_anc_adaptive_fr_fail_fill(rff_dat, ff_yorder, (result & ANC_ADAPTIVE_RESULT_RFF));
    }
#endif/*ANC_CONFIG_RFF_EN*/
#if ANC_CONFIG_RFB_EN
    rfb_dat = (u8 *)&iir->rfb_gain;
    if (!ANC_CONFIG_LFB_EN) {	//TWS 单R声道使用
        fb_dat = rfb_dat;
        audio_anc_fr_format(fb_dat, TOOL_DATA->data_out4, fb_yorder, fb_iir_type);
        audio_anc_adaptive_fr_fail_fill(fb_dat, fb_yorder, (result & ANC_ADAPTIVE_RESULT_LFB));
    } else {
        audio_anc_fr_format(rfb_dat, TOOL_DATA->data_out9, fb_yorder, fb_iir_type);
        audio_anc_adaptive_fr_fail_fill(rfb_dat, fb_yorder, (result & ANC_ADAPTIVE_RESULT_RFB));
    }
#if ANC_EAR_ADAPTIVE_CMP_EN
    rcmp_dat = (u8 *)&iir->rcmp_gain;
    if (!ANC_CONFIG_LFB_EN) {	//TWS 单R声道使用
        cmp_dat = rcmp_dat;
        audio_anc_fr_format(cmp_dat, TOOL_DATA->data_out11, cmp_yorder, cmp_iir_type);
        audio_anc_adaptive_fr_fail_fill(cmp_dat, cmp_yorder, (result & ANC_ADAPTIVE_RESULT_LCMP));
    } else {
        audio_anc_fr_format(rcmp_dat, TOOL_DATA->data_out12, cmp_yorder, cmp_iir_type);
        audio_anc_adaptive_fr_fail_fill(rcmp_dat, cmp_yorder, (result & ANC_ADAPTIVE_RESULT_RCMP));
    }
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CONFIG_RFB_EN*/

    if (anc_hdl->param.developer_mode) {
        printf("-- len = %d\n", len);
        printf("-- ff_yorder = %d\n", ff_yorder);
        printf("-- fb_yorder = %d\n", fb_yorder);
        printf("-- cmp_yorder = %d\n", cmp_yorder);
        /* 先统一申请空间，因为下面不知道什么情况下调用函数 anc_data_catch 时令参数 init_flag 为1 */
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, NULL, 0, 0, 1);

#if (ANC_CH == (ANC_L_CH | ANC_R_CH))	//头戴式
        r_printf("ANC ear-adaptive send data\n");
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->h_freq, len * 4, ANC_R_ADAP_FRE, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out6, len * 8, ANC_R_ADAP_SZPZ, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out7, len * 8, ANC_R_ADAP_PZ, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out8, len * 8, ANC_R_ADAP_TARGET, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out14, len * 8, ANC_R_ADAP_TARGET_CMP, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)rff_dat, ff_dat_len, ANC_R_FF_IIR, 0);  //R_ff
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)rfb_dat, fb_dat_len, ANC_R_FB_IIR, 0);  //R_fb

        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->h_freq, len * 4, ANC_L_ADAP_FRE, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out1, len * 8, ANC_L_ADAP_SZPZ, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out2, len * 8, ANC_L_ADAP_PZ, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out3, len * 8, ANC_L_ADAP_TARGET, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out13, len * 8, ANC_L_ADAP_TARGET_CMP, 0);
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)ff_dat, ff_dat_len, ANC_L_FF_IIR, 0);  //L_ff
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)fb_dat, fb_dat_len, ANC_L_FB_IIR, 0);  //L_fb
#if ANC_EAR_ADAPTIVE_CMP_EN
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)cmp_dat, cmp_dat_len, ANC_L_CMP_IIR, 0);  //L_cmp
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)rcmp_dat, cmp_dat_len, ANC_R_CMP_IIR, 0);  //R_cmp
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#else  //入耳式
#if TCFG_USER_TWS_ENABLE
        extern char bt_tws_get_local_channel(void);
        if (bt_tws_get_local_channel() == 'R') {
            r_printf("ANC ear-adaptive send data, ch:R\n");
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->h_freq, len * 4, ANC_R_ADAP_FRE, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out1, len * 8, ANC_R_ADAP_SZPZ, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out2, len * 8, ANC_R_ADAP_PZ, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out3, len * 8, ANC_R_ADAP_TARGET, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out13, len * 8, ANC_R_ADAP_TARGET_CMP, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)ff_dat, ff_dat_len, ANC_R_FF_IIR, 0);  //R_ff
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)fb_dat, fb_dat_len, ANC_R_FB_IIR, 0);  //R_fb
#if ANC_EAR_ADAPTIVE_CMP_EN
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)cmp_dat, cmp_dat_len, ANC_R_CMP_IIR, 0);  //R_cmp
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
        } else
#endif/*TCFG_USER_TWS_ENABLE*/
        {
            r_printf("ANC ear-adaptive send data, ch:L\n");
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->h_freq, len * 4, ANC_L_ADAP_FRE, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out1, len * 8, ANC_L_ADAP_SZPZ, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out2, len * 8, ANC_L_ADAP_PZ, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out3, len * 8, ANC_L_ADAP_TARGET, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)TOOL_DATA->data_out13, len * 8, ANC_L_ADAP_TARGET_CMP, 0);
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)ff_dat, ff_dat_len, ANC_L_FF_IIR, 0);  //L_ff
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)fb_dat, fb_dat_len, ANC_L_FB_IIR, 0);  //L_fb
#if ANC_EAR_ADAPTIVE_CMP_EN
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)cmp_dat, cmp_dat_len, ANC_L_CMP_IIR, 0);  //L_cmp
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
        }
#endif  /* ANC_CH == (ANC_L_CH | ANC_R_CH) */

    }

}

//开机自适应数据拼包，预备工具读取
void audio_anc_adaptive_poweron_catch_data(anc_adaptive_iir_t *iir)
{
    if (anc_hdl->param.developer_mode) {
        int i;
        int ff_dat_len =  sizeof(anc_fr_t) * ANC_ADAPTIVE_FF_ORDER + 4;
        int fb_dat_len =  sizeof(anc_fr_t) * ANC_ADAPTIVE_FB_ORDER + 4;
        int cmp_dat_len =  sizeof(anc_fr_t) * ANC_ADAPTIVE_CMP_ORDER + 4;
        u8 *ff_dat, *fb_dat, *cmp_dat, *rff_dat, *rfb_dat, *rcmp_dat;
        audio_anc_t *param = &anc_hdl->param;
#if ANC_CONFIG_LFF_EN
        ff_dat = (u8 *)&iir->lff_gain;
#endif/*ANC_CONFIG_LFF_EN*/
#if ANC_CONFIG_LFB_EN
        fb_dat = (u8 *)&iir->lfb_gain;
#if ANC_EAR_ADAPTIVE_CMP_EN
        cmp_dat = (u8 *)&iir->lcmp_gain;
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CONFIG_LFB_EN*/
#if ANC_CONFIG_RFF_EN
        rff_dat = (u8 *)&iir->rff_gain;
        if (ff_dat == NULL) { //TWS ANC_R_CH使用
            ff_dat = rff_dat;
        }
#endif/*ANC_CONFIG_RFF_EN*/
#if ANC_CONFIG_RFB_EN
        rfb_dat = (u8 *)&iir->rfb_gain;
        if (fb_dat == NULL) {	//TWS ANC_R_CH使用
            fb_dat = rfb_dat;
        }
#if ANC_EAR_ADAPTIVE_CMP_EN
        rcmp_dat = (u8 *)&iir->rcmp_gain;
        if (cmp_dat == NULL) {	//TWS ANC_R_CH使用
            cmp_dat = rcmp_dat;
        }
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CONFIG_RFB_EN*/

        /* 先统一申请空间，因为下面不知道什么情况下调用函数 anc_data_catch 时令参数 init_flag 为1 */
        anc_adaptive_data = anc_data_catch(anc_adaptive_data, NULL, 0, 0, 1);
#if (ANC_CH == (ANC_L_CH | ANC_R_CH))	//头戴式的耳机
        r_printf("ANC ear-adaptive send data\n");
        if (param->lff_en && param->rff_en) {
            if (iir->result & ANC_ADAPTIVE_RESULT_LFF) {
                anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)ff_dat, ff_dat_len, ANC_L_FF_IIR, 0);  //L_ff
            }
            if (iir->result & ANC_ADAPTIVE_RESULT_RFF) {
                anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)rff_dat, ff_dat_len, ANC_R_FF_IIR, 0);  //R_ff
            }
        }
        if (iir->result & ANC_ADAPTIVE_RESULT_LFB) {
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)fb_dat, fb_dat_len, ANC_L_FB_IIR, 0);  //L_fb
        }
        if (iir->result & ANC_ADAPTIVE_RESULT_RFB) {
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)rfb_dat, fb_dat_len, ANC_R_FB_IIR, 0);  //R_fb
        }

#if ANC_EAR_ADAPTIVE_CMP_EN
        if (iir->result & ANC_ADAPTIVE_RESULT_LCMP) {
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)cmp_dat, cmp_dat_len, ANC_L_CMP_IIR, 0);  //L_cmp
        }
        if (iir->result & ANC_ADAPTIVE_RESULT_RCMP) {
            anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)rcmp_dat, cmp_dat_len, ANC_R_CMP_IIR, 0);  //R_cmp
        }
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#else	//入耳式的
#if TCFG_USER_TWS_ENABLE
        extern char bt_tws_get_local_channel(void);
        if (bt_tws_get_local_channel() == 'R') {
            r_printf("ANC ear-adaptive send data, ch:R\n");
            if (param->lff_en || param->rff_en) {
                if (iir->result & ANC_ADAPTIVE_RESULT_RFF) {
                    anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)ff_dat, ff_dat_len, ANC_R_FF_IIR, 0);  //R_ff
                }
            }
            if (iir->result & ANC_ADAPTIVE_RESULT_RFB) {
                anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)fb_dat, fb_dat_len, ANC_R_FB_IIR, 0);  //R_fb
            }
#if ANC_EAR_ADAPTIVE_CMP_EN
            if (iir->result & ANC_ADAPTIVE_RESULT_RCMP) {
                anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)cmp_dat, cmp_dat_len, ANC_R_CMP_IIR, 0);  //R_cmp
            }
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
        } else
#endif/*TCFG_USER_TWS_ENABLE*/
        {
            r_printf("ANC ear-adaptive send data, ch:L\n");
            if (param->lff_en || param->rff_en) {
                if (iir->result & ANC_ADAPTIVE_RESULT_LFF) {
                    anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)ff_dat, ff_dat_len, ANC_L_FF_IIR, 0);  //L_ff
                }
            }
            if (iir->result & ANC_ADAPTIVE_RESULT_LFB) {
                anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)fb_dat, fb_dat_len, ANC_L_FB_IIR, 0);  //L_fb
            }
#if ANC_EAR_ADAPTIVE_CMP_EN
            if (iir->result & ANC_ADAPTIVE_RESULT_LCMP) {
                anc_adaptive_data = anc_data_catch(anc_adaptive_data, (u8 *)cmp_dat, cmp_dat_len, ANC_L_CMP_IIR, 0);  //L_cmp
            }
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
        }
#endif	/* ANC_CH == (ANC_L_CH | ANC_R_CH) */
    }
}

//左右参数映射
void audio_anc_param_map(u8 coeff_en, u8 gain_en)
{
#if TCFG_USER_TWS_ENABLE && (ANC_CH != (ANC_L_CH | ANC_R_CH))
    if (anc_hdl->param.developer_mode) {	//开发者模式左右耳区分使用参数
        if (coeff_en && (!anc_hdl->param.lff_coeff || !anc_hdl->param.rff_coeff || \
                         !anc_hdl->param.lfb_coeff || !anc_hdl->param.rfb_coeff)) {
#if ANC_DEVELOPER_MODE_EN
            ASSERT(0, "(ERROR)ANC adaptive coeff map %x, %x, %x, %x, Maybe anc_coeff.bin is ERR!", \
                   anc_hdl->param.lff_coeff, anc_hdl->param.rff_coeff, \
                   anc_hdl->param.lfb_coeff, anc_hdl->param.rfb_coeff);
#else
            user_anc_log("(ERROR)ANC adaptive coeff map %x, %x, %x, %x, Maybe anc_coeff.bin is ERR!", \
                         anc_hdl->param.lff_coeff, anc_hdl->param.rff_coeff, \
                         anc_hdl->param.lfb_coeff, anc_hdl->param.rfb_coeff);
#endif/*ANC_DEVELOPER_MODE_EN*/
        }
        if ((ANC_CH == ANC_L_CH) && (bt_tws_get_local_channel() == 'R')) {
            user_anc_log("ANC MAP TWS_RIGHT_CHANNEL\n");
            if (gain_en) {
                anc_hdl->param.gains.l_ffgain = anc_hdl->param.gains.r_ffgain;
                anc_hdl->param.gains.l_fbgain = anc_hdl->param.gains.r_fbgain;
                anc_hdl->param.gains.l_transgain = anc_hdl->param.gains.r_transgain;
                anc_hdl->param.gains.l_cmpgain = anc_hdl->param.gains.r_cmpgain;
                anc_hdl->param.gains.l_ffmic_gain = anc_hdl->param.gains.r_ffmic_gain;
                anc_hdl->param.gains.l_fbmic_gain = anc_hdl->param.gains.r_fbmic_gain;
                anc_hdl->param.gains.gain_sign = (anc_hdl->param.gains.gain_sign >> 4);
            }
            if (coeff_en) {
                anc_hdl->param.lff_coeff = anc_hdl->param.rff_coeff;
                anc_hdl->param.lfb_coeff = anc_hdl->param.rfb_coeff;
                anc_hdl->param.ltrans_coeff = anc_hdl->param.rtrans_coeff;
                anc_hdl->param.lcmp_coeff = anc_hdl->param.rcmp_coeff;

                anc_hdl->param.lff_yorder = anc_hdl->param.rff_yorder;
                anc_hdl->param.lfb_yorder = anc_hdl->param.rfb_yorder;
                anc_hdl->param.ltrans_yorder = anc_hdl->param.rtrans_yorder;
                anc_hdl->param.lcmp_yorder = anc_hdl->param.rcmp_yorder;
            }
        } else if ((ANC_CH == ANC_R_CH) && (bt_tws_get_local_channel() == 'L')) {
            user_anc_log("ANC MAP TWS_LEFT_CHANNEL\n");
            if (gain_en) {
                anc_hdl->param.gains.r_ffgain = anc_hdl->param.gains.l_ffgain;
                anc_hdl->param.gains.r_fbgain = anc_hdl->param.gains.l_fbgain;
                anc_hdl->param.gains.r_transgain = anc_hdl->param.gains.l_transgain;
                anc_hdl->param.gains.r_cmpgain = anc_hdl->param.gains.l_cmpgain;
                anc_hdl->param.gains.r_ffmic_gain = anc_hdl->param.gains.l_ffmic_gain;
                anc_hdl->param.gains.r_fbmic_gain = anc_hdl->param.gains.l_fbmic_gain;
                anc_hdl->param.gains.gain_sign = (anc_hdl->param.gains.gain_sign << 4);
            }
            if (coeff_en) {
                anc_hdl->param.rff_coeff = anc_hdl->param.lff_coeff;
                anc_hdl->param.rfb_coeff = anc_hdl->param.lfb_coeff;
                anc_hdl->param.rtrans_coeff = anc_hdl->param.ltrans_coeff;
                anc_hdl->param.rcmp_coeff = anc_hdl->param.lcmp_coeff;

                anc_hdl->param.rff_yorder = anc_hdl->param.lff_yorder;
                anc_hdl->param.rfb_yorder = anc_hdl->param.lfb_yorder;
                anc_hdl->param.rtrans_yorder = anc_hdl->param.ltrans_yorder;
                anc_hdl->param.rcmp_yorder = anc_hdl->param.lcmp_yorder;
            }
        }
    }
#endif/*TCFG_USER_TWS_ENABLE*/

}

//临时接口
void icsd_audio_adc_mic_open(u8 mic_idx, u8 gain, u16 sr, u8 mic_2_dac)
{

}

u8 anc_ear_adaptive_seq_get(void)
{
    if (anc_hdl) {
        return anc_hdl->ear_adaptive_seq;
    }
    return 0;
}

void anc_ear_adaptive_forced_exit(void)
{
    if (anc_hdl) {
        if (anc_hdl->ear_adaptive_busy) {
            icsd_anc_forced_exit();
            anc_tone_stop();
        }
    }
}

//ANC自适应训练失败处理回调函数
void audio_anc_adaptive_fail_callback(u8 anc_err)
{
#if (RCSP_ADV_EN && RCSP_ADV_ANC_VOICE && RCSP_ADV_ADAPTIVE_NOISE_REDUCTION)
    extern void set_adaptive_noise_reduction_reset_callback(u8 result);
    set_adaptive_noise_reduction_reset_callback(!anc_err);
#endif
    if (anc_err == 0) {
        /* user_anc_log("ANC_ADAPTIVE SUCCESS==========================\n"); */
    }
    if (anc_err & ANC_ERR_LR_BALANCE) {
        user_anc_log("ANC_ERR_LR_BALANCE==========================\n");
    }
    if (anc_err & ANC_ERR_SOUND_PRESSURE) {
        user_anc_log("ANC_ERR_SOUND_PRESSURE==========================\n");
    }
    if (anc_err & ANC_ERR_SHAKE) {
        user_anc_log("ANC_ERR_SHAKE==========================\n");
    }
}

#endif/*ANC_EAR_ADAPTIVE_EN*/

#if ANC_MULT_ORDER_ENABLE

void audio_anc_mult_scene_set(u16 scene_id, u8 update_flag)
{
    user_anc_log(" audio_anc_mult scene_id set %d\n", scene_id);
    if (audio_anc_mult_scene_id_check(scene_id)) {
        user_anc_log("anc_mult_scene id set err id = %d", scene_id);
        return;
    }
    anc_hdl->scene_id = scene_id;
#if ANC_EAR_ADAPTIVE_EN
    if (anc_hdl->ear_adaptive_busy) {
        anc_ear_adaptive_forced_exit();
        return;
    }
    //非自适应训练状态，切场景自动切回普通参数
    anc_hdl->param.anc_coeff_mode = ANC_COEFF_MODE_NORMAL;
#endif/*ANC_EAR_ADAPTIVE_EN*/
    if ((anc_hdl->param.mode != ANC_OFF) && update_flag && \
        !anc_hdl->mode_switch_lock) {		//实时更新填入使用
        anc_mult_scene_set(scene_id);
        os_taskq_post_msg("anc", 2, ANC_MSG_RESET, 1);
    }
}

u8 audio_anc_mult_scene_get(void)
{
    return anc_hdl->scene_id;
}

void audio_anc_mult_scene_max_set(u8 scene_cnt)
{
    anc_hdl->scene_max = scene_cnt;
}

u8 audio_anc_mult_scene_max_get(void)
{
    return anc_hdl->scene_max;
}

void audio_anc_mult_scene_switch(u8 tone_flag)
{
    static u8 cnt = 1;
    if (cnt < audio_anc_mult_scene_max_get()) {
        cnt++;
    } else {
        cnt = 1;
    }
    if (tone_flag) {
        tone_play_index(IDEX_TONE_NUM_0 + cnt, 0);
    }
    audio_anc_mult_scene_set(cnt, 1);
}
#endif/*ANC_MULT_ORDER_ENABLE*/

//ANC产测/调试模式
extern struct adc_platform_data adc_data;
void audio_anc_prodution_mode_set(u8 mode)
{
    //产测 or 调试模式 关闭非必要的流程
    if (mode && (!anc_hdl->param.production_mode)) {	//0->1
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        /*关闭ACOUSTIC_DETECTOR所有模块*/
        if (audio_icsd_adt_is_running()) {
            audio_icsd_adt_close_all();
        }
        adc_data.anc_adt_en = 0;	//关闭adc_ch绑定，否则产测使用ADC中断数据会异常
#endif/* TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
    } else if (!mode && (anc_hdl->param.production_mode)) {	//1->0
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        adc_data.anc_adt_en = 1;	//恢复adc_ch绑定
#endif/* TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
    }
    anc_hdl->param.production_mode = mode;
}

#endif/*TCFG_AUDIO_ANC_ENABLE*/
