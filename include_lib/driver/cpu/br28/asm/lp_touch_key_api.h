#ifndef _LP_TOUCH_KEY_API_
#define _LP_TOUCH_KEY_API_

#include "typedef.h"
#include "asm/lp_touch_key_hw.h"


/**************************************************USER配置************************************************************************/
//长按开机时间:
#define CFG_M2P_CTMU_SOFTOFF_LONG_TIME 			    1000	//单位: ms

//触摸按键长按复位时间配置
#define CTMU_RESET_TIME_CONFIG			            8000	//长按复位时间(ms), 配置为0关闭

//debug
#define CFG_CH0_DEBUG_ENABLE                        0
#define CFG_CH1_DEBUG_ENABLE                        0
#define CFG_CH2_DEBUG_ENABLE                        0
#define CFG_CH3_DEBUG_ENABLE                        0
#define CFG_CH4_DEBUG_ENABLE                        0

#define CFG_CHx_DEBUG_ENABLE    ((CFG_CH4_DEBUG_ENABLE<<4) | (CFG_CH3_DEBUG_ENABLE<<3) | (CFG_CH2_DEBUG_ENABLE<<2) | (CFG_CH1_DEBUG_ENABLE<<1) | (CFG_CH0_DEBUG_ENABLE))

#define CFG_DISABLE_KEY_EVENT                       0

#define TWS_BT_SEND_KEY_CH_RES_DATA_ENABLE         	0

#define TWS_BT_SEND_EARTCH_RES_DATA_ENABLE         	0

#define TWS_BT_SEND_EVENT_ENABLE                    0


#if TWS_BT_SEND_KEY_CH_RES_DATA_ENABLE
#if !CFG_CHx_DEBUG_ENABLE
#undef CFG_CHx_DEBUG_ENABLE
#define CFG_CHx_DEBUG_ENABLE                        0b11111
#endif
#endif


#if (CFG_CH0_DEBUG_ENABLE || CFG_CH1_DEBUG_ENABLE || CFG_CH2_DEBUG_ENABLE || CFG_CH3_DEBUG_ENABLE || CFG_CH4_DEBUG_ENABLE)
#if !CFG_DISABLE_KEY_EVENT
#undef CFG_DISABLE_KEY_EVENT
#define CFG_DISABLE_KEY_EVENT                       1
#endif
#endif


#define ERATCH_KEY_MSG_LOCK_TIME                    3000 //ms 入耳检测状态发生改变时，多长时间内不能发送按键消息


/*************************************************** Lpctmu API **********************************************************/
#define LP_CTMU_CHANNEL_SIZE                        5

enum LP_TOUCH_SOFTOFF_MODE {
    LP_TOUCH_SOFTOFF_MODE_LEGACY  = 0, //普通关机
    LP_TOUCH_SOFTOFF_MODE_ADVANCE = 1, //带触摸关机
};

enum ctmu_key_event {
    CTMU_KEY_NULL,
    CTMU_KEY_SHORT_CLICK,
    CTMU_KEY_LONG_CLICK,
    CTMU_KEY_HOLD_CLICK,
};


enum {
    TOUCH_KEY_EVENT_SLIDE_UP,
    TOUCH_KEY_EVENT_SLIDE_DOWN,
    TOUCH_KEY_EVENT_SLIDE_LEFT,
    TOUCH_KEY_EVENT_SLIDE_RIGHT,
    TOUCH_KEY_EVENT_MAX,
};


struct ctmu_ch_cfg {
    u8 enable;
    u8 wakeup_enable;
    u8 key_value;
    u8 port;
    u8 sensitivity;
};

struct lp_touch_key_platform_data {
    u8 slide_mode_en;
    u8 slide_mode_key_value;
    u8 eartch_en;
    u8 eartch_ch;
    u8 eartch_ref_ch;
    u16 eartch_soft_inear_val;
    u16 eartch_soft_outear_val;
    struct ctmu_ch_cfg ch[LP_CTMU_CHANNEL_SIZE];
};

enum ch1_event_list {
    EAR_IN,
    EAR_OUT,
};

enum {
    EPD_IN,
    EPD_OUT,
    EPD_STATE_NO_CHANCE
};

struct ch_ana_cfg {
    u8 isel;
    u8 vhsel;
    u8 vlsel;
};

struct ch_adjust_table {
    u16 cfg0;
    u16 cfg1;
    u16 cfg2;
};

struct ctmu_key {
    u8 init;
    u8 softoff_mode;
    u8 slide_dir;
    u8 click_cnt[LP_CTMU_CHANNEL_SIZE];
    u8 last_key[LP_CTMU_CHANNEL_SIZE];
    u16 short_timer[LP_CTMU_CHANNEL_SIZE];

    u8 key_ch_msg_lock;
    u16 key_ch_msg_lock_timer;

    u8 eartch_inear_ok;
    u8 eartch_last_state;
    u8 eartch_trim_flag;
    u16 eartch_trim_value;

    const struct lp_touch_key_platform_data *config;
};

extern struct ctmu_key _ctmu_key;


void lp_touch_key_init(const struct lp_touch_key_platform_data *config);

u8 lp_touch_key_power_on_status();

void lp_touch_key_disable(void);

void lp_touch_key_enable(void);

u8 lp_touch_key_alog_range_display(u8 *display_buf);

void lp_touch_key_send_cmd(enum CTMU_M2P_CMD cmd);

void set_lpkey_active(u8 set);

u8 lp_touch_key_alog_range_display(u8 *display_buf);

#endif /* #ifndef _LP_TOUCH_KEY_API_ */
