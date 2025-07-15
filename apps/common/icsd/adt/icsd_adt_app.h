#ifndef __ICSD_ADT_APP_H_
#define __ICSD_ADT_APP_H_

#include "typedef.h"
#include "icsd_anc_user.h"
// #include "icsd_adt_client_board.h"

#define SPEAK_TO_CHAT_TASK_NAME     "speak_to_chat"

/*智能免摘检测到声音和退出通透是否播放提示音*/
#define SPEAK_TO_CHAT_PLAY_TONE_EN  1

/*智能免摘每次检测到声音的测试提示音*/
#define SPEAK_TO_CHAT_TEST_TONE_EN  0

/* 通过蓝牙spp发送风噪信息
 * 需要同时打开USER_SUPPORT_PROFILE_SPP和APP_ONLINE_DEBUG*/
#define ICSD_ADT_WIND_INFO_SPP_DEBUG_EN  0

enum {
    AUDIO_ADT_CLOSE = 0,    //关闭关闭
    AUDIO_ADT_OPEN,     //打开状态
    AUDIO_ADT_CHAT,   //免摘状态
};

enum {
    ANC_WIND_NOISE_LVL0 = 1,
    ANC_WIND_NOISE_LVL1,
    ANC_WIND_NOISE_LVL2,
    ANC_WIND_NOISE_LVL3,
    ANC_WIND_NOISE_LVL4,
    ANC_WIND_NOISE_LVL5,
};

enum {
    WIND_AREA_TAP_DOUBLE_CLICK = 2,     //双击
    WIND_AREA_TAP_THIRD_CLICK,      //三击
    WIND_AREA_TAP_MULTIPLE_CLICK,   //大于3次多次连击
};

enum {
    ADT_MODE_CLOSE = 0,
    ADT_SPEAK_TO_CHAT_MODE = BIT(0), //智能免摘
    ADT_WIND_NOISE_DET_MODE = BIT(1),//风噪检测
    ADT_WIDE_AREA_TAP_MODE = BIT(2), //广域点击
};

enum {
    /*任务消息*/
    ICSD_ADT_VOICE_STATE = 1,
    ICSD_ADT_WIND_LVL,
    ICSD_ADT_WAT_RESULT,
    ICSD_ADT_TONE_PLAY,
    SPEAK_TO_CHAT_TASK_KILL,
    ICSD_ANC_MODE_SWITCH,
    ICSD_ADT_STATE_SYNC,

    /*对耳信息同步*/
    SYNC_ICSD_ADT_VOICE_STATE,
    SYNC_ICSD_ADT_WIND_LVL_CMP,
    SYNC_ICSD_ADT_WAT_RESULT,
    SYNC_ICSD_ADT_WIND_LVL_RESULT,
    SYNC_ICSD_ADT_SUSPEND,
    SYNC_ICSD_ADT_OPEN,
    SYNC_ICSD_ADT_CLOSE,
    SYNC_ICSD_ADT_SET_ANC_FADE_GAIN,
};

/*打开智能免摘*/
int audio_speak_to_chat_open();

/*关闭智能免摘*/
int audio_speak_to_chat_close();

void audio_speak_to_chat_demo();

/*设置免摘定时结束的时间，单位ms*/
int audio_speak_to_char_end_time_set(u16 time);

/*设置智能免摘检测的灵敏度*/
int audio_speak_to_chat_sensitivity_set(u8 sensitivity);

/*获取是否需要先开anc再开免摘的状态*/
u8 get_adt_open_in_anc_state();

void audio_anc_mode_switch_in_adt(u8 anc_mode);

/*打开声音检测*/
int audio_acoustic_detector_open();

/*关闭声音检测*/
int audio_acoustic_detector_close();

int audio_speak_to_chat_open_in_anc_done();

void audio_icsd_adt_resume();
void audio_icsd_adt_suspend();
/*char 定时结束后从通透同步恢复anc on /anc off*/
void audio_speak_to_char_suspend(void);
void audio_speak_to_char_sync_suspend(void);

/*获取智能免摘是否打开*/
u8 audio_speak_to_chat_is_running();

/*获取智能免摘状态*/
u8 get_speak_to_chat_state();

/*获取adt的模式*/
u8 get_icsd_adt_mode();

/*同步tws配对时，同步adt的状态*/
void audio_anc_icsd_adt_state_sync(u8 *data);
void audio_icsd_adt_state_sync_done(u8 adt_mode, u8 speak_to_chat_state);

/*检测到讲话状态同步*/
void set_speak_to_chat_voice_state(u8 state);
void audio_speak_to_chat_voice_state_sync(void);

int anc_adt_init();

int audio_icsd_adt_open(u8 adt_mode);
int audio_icsd_adt_sync_open(u8 adt_mode);
/*打开所有模块*/
int audio_icsd_adt_open_all();

int audio_icsd_adt_sync_close(u8 adt_mode);
int audio_icsd_adt_close(u8 adt_mode);
int audio_icsd_adt_res_close(u8 adt_mode);
/*关闭所有模块*/
int audio_icsd_adt_close_all();

u8 audio_icsd_adt_is_running();

/*打开广域点击*/
int audio_wat_click_open();

/*关闭广域点击*/
int audio_wat_click_close();

/*设置是否忽略广域点击
 * ingore: 1 忽略点击，0 响应点击
 * 忽略点击的时间，单位ms*/
int audio_wide_area_tap_ignore_flag_set(u8 ignore, u16 time);

/*广域点击开关demo*/
void audio_wat_click_demo();

/*广域点击事件处理*/
void audio_wat_area_tap_event_handle(u8 wat_result);

/*打开风噪检测*/
int audio_icsd_wind_detect_open();

/*关闭风噪检测*/
int audio_icsd_wind_detect_close();

/*获取风噪等级*/
u8 get_audio_icsd_wind_lvl();

/*风噪检测开关demo*/
void audio_icsd_wind_detect_demo();

/*风噪检测处理*/
void audio_anc_wind_noise_process(u8 wind_lvl);

/*参数更新接口*/
int audio_acoustic_detector_updata();
void set_icsd_adt_param_updata();

/*获取免摘需要多少个mic*/
u8 get_icsd_adt_mic_num();

extern void set_anc_adt_state(u8 state);
extern void *get_anc_lfb_coeff();
extern void *get_anc_lff_coeff();
extern void *get_anc_ltrans_coeff();
extern void *get_anc_ltrans_fb_coeff();
extern float get_anc_gains_l_fbgain();
extern float get_anc_gains_l_ffgain();
extern float get_anc_gains_l_transgain();
extern float get_anc_gains_lfb_transgain();
extern u8 get_anc_l_fbyorder();
extern u8 get_anc_l_ffyorder();
extern u8 get_anc_l_transyorder();
extern u8 get_anc_lfb_transyorder();
extern u32 get_anc_gains_alogm();
extern u32 get_anc_gains_trans_alogm();

#endif
