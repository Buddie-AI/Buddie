#ifndef _IN_EAR_MANAGE_H
#define _IN_EAR_MANAGE_H

#include "typedef.h"

#define INEAR_ANC_UI		 	0	//switch anc mode when in_out_ear

enum {
    EAR_DETECT_EVENT_NULL = 0,
    EAR_DETECT_EVENT_IN,
    EAR_DETECT_EVENT_OUT,
    EAR_DETECT_EVENT_IN_DEAL,
    EAR_DETECT_EVENT_OUT_DEAL,
};

enum {
    MUSIC_STATE_NULL = 0,
    MUSIC_STATE_PLAY, //音乐播放
    MUSIC_STATE_PAUSE, //音乐暂停
};

enum {
    CMD_EAR_DETECT_MUSIC_PLAY = 0,
    CMD_EAR_DETECT_MUSIC_PAUSE,
    CMD_EAR_DETECT_SCO_CONN,
    CMD_EAR_DETECT_SCO_DCONN,
};

enum {
    DETECT_IDLE = 0,
    DETECT_CHECKING,
};

#define EAR_DETECT_BY_TOUCH     0  //触摸入耳
#define EAR_DETECT_BY_IR     	1  //光感入耳

struct ear_detect_platform_data {
    u8 ear_det_music_ctl_en;				//音乐控制使能
    u16 ear_det_music_ctl_ms;				//音乐暂停之后，入耳检测控制暂停播放的时间
    u8 ear_det_in_music_sta;				// 0：入耳播歌  1：入耳不播歌
    const u16 ear_det_key_delay_time;   	//入耳后按键起效时间ms ( 0 : OFF )
    u8 ear_det_in_cnt; 						//戴上消抖
    u8 ear_det_out_cnt;   					//拿下消抖
    u16 ear_det_ir_enable_time; 			//使能时长
    u16 ear_det_ir_disable_time;   			//休眠时长
    u8 ear_det_ir_compensation_en;			//防太阳光干扰
    u8 ear_det_music_play_cnt;       		//音乐播放检测时间
    u8 ear_det_music_pause_cnt;    		    //音乐暂停检测时间
};

struct ear_detect_d {
    u8 toggle        : 1; //入耳功能开关
    u8 music_en      : 1;
    u8 pre_music_sta : 2;
    u8 pre_state     : 1;
    u8 cur_state     : 1;
    u8 tws_state     : 1;
    u8 bt_init_ok    : 1;
    u16 music_check_timer;
    u16 music_sta_cnt;
    u16 change_master_timer;
    u16 key_enable_timer;
    u8 key_delay_able;
    //TCFG_EAR_DETECT_MUSIC_CTL_EN
    u8 play_cnt;
    u8 stop_cnt;
    u16 music_ctl_timeout;
    u16 a2dp_det_timer;
    u8 music_regist_en;
    //cfg
    const struct ear_detect_platform_data *cfg;
};

struct ear_detect_t {
    //TCFG_EAR_DETECT_TIMER_MODE
    u8 is_idle;
    u16 in_cnt;
    u16 out_cnt;
    u16 s_hi_timer;
    volatile u8 check_status;
    //cfg
    const struct ear_detect_platform_data *cfg;
};

// ------------------- fun -----------
extern void ear_detect_event_handle(u8 state);
extern void ear_detect_phone_active_deal();
extern void ear_detect_call_chg_master_deal();
extern void ear_detect_change_master_timer_del();
extern void tws_sync_ear_detect_state(u8 need_do);
extern void ear_detect_init(const struct ear_detect_platform_data *cfg);
extern void ear_detect_change_state_to_event(u8 state);
extern u8 is_ear_detect_state_in(void);
extern void ear_touch_edge_wakeup_handle(u8 index, u8 gpio);
extern u8 ear_detect_get_key_delay_able(void);

#endif
