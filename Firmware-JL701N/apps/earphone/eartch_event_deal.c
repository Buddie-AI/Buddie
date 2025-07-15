//===========================================================//
// 					LP TOUCH EAR EVENT HANDLE  				 //
//===========================================================//
//1.入耳事件产生 --> sync state --> state 回调中处理播放暂停, 通话转换
#include "includes.h"
#include "key_event_deal.h"
#include "btstack/avctp_user.h"
#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif /* #if TCFG_USER_TWS_ENABLE */
#include "tone_player.h"
#include "app_config.h"
#include "classic/tws_api.h"

#define LOG_TAG_CONST       EARTCH_EVENT_DEAL
#define LOG_TAG             "[EARTCH_EVENT_DEAL]"
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

/* #define SUPPORT_MS_EXTENSIONS */
/* #ifdef SUPPORT_MS_EXTENSIONS */
/* #pragma bss_seg(	".eartch_event_deal_bss") */
/* #pragma data_seg(".eartch_event_deal_data") */
/* #pragma const_seg(".eartch_event_deal_const") */
/* #pragma code_seg(".eartch_event_deal_code") */
/* #endif */

#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
//========================================================================================//
//入耳检测UI控制流程:
//1.控制音乐播放条件:
//	1)两只取下, 音乐暂停, 15s内佩戴耳机, 控制音乐为播放状态;
//	2)一只取下, 音乐暂停, 没有时间限制, 控制音乐为播放状态;
//2.控制音乐暂停条件: 当音乐播放时, 取下任意一个耳机, 音乐暂停;
//3.控制通话语音转移到远端: 在通话状态下, 当两个耳机都取下, 语音由耳机端转移到远端;
//4.控制通话语音转移到耳机端: 在通话状态下, 佩戴任意一个耳机, 语音由手机端端转移到耳机端;
//========================================================================================//

bool __attribute__((weak)) get_tws_sibling_connect_state(void)
{
    return FALSE;
}

#define TWS_FUNC_ID_EARTCH_SYNC		TWS_FUNC_ID('E', 'A', 'T', 'H')


#define TCFG_EARTCH_MUSIC_CTL_TIMEOUT_ENABLE 		0 //对耳不在耳, 15s超时后入耳控制音乐播放无效
#define TCFG_EARTCH_MUSIC_CTL_A2DP_CONNECT 			0 //对耳不在耳, 断开a2dp链路, 音乐转移到手机扬声器
#define TCFG_EARTCH_CALL_CTL_SCO_CONNECT 			1 //对耳不在耳, 断开SCO链路, 通话转移到手机扬声器
#define TCFG_EARTCH_AUTO_CHANGE_MASTER 				1
#define TCFG_EARTCH_SWITCH_CFG_ENABLE 				1 //入耳消息处理使能开关, 配置项保存到vm, 支持动态开关

struct eartch_control {
    u8 eartch_event_deal_en;
    u8 local_state: 1;
    u8 remote_state: 1;
    u8 last_state: 1;
    u8 event_source: 1;
    u8 a2dp_connect_state: 1;
    u8 a2dp_send_flag: 1;
    u8 music_ctrl_en: 1;
    u8 state_check_cnt;
    u8 a2dp_play_state_cnt;
    u8 a2dp_stop_state_cnt;
    u16 state_check_timer;
    u16 music_ctrl_en_timer;
    u16 a2dp_monitor_timer;
};

static struct eartch_control _eartch = {
    .local_state = EARTCH_STATE_OUT,
    .remote_state = EARTCH_STATE_OUT,
    .last_state = EARTCH_STATE_OUT,
};

#define __this 		(&_eartch)

//============= 关于配置项保存信息 ==========//
#define EARTCH_SWITCH_CFG_LEN 		1 //配置项长度

enum EARTCH_TWS_SYNC_CMD {
    EARTCH_TWS_STATE_IN = 0x00,
    EARTCH_TWS_STATE_OUT = 0x01,
    EARTCH_TWS_STATE_TRIM_OK = 0x02,
    EARTCH_TWS_STATE_TRIM_ERR = 0x03,
    EARTCH_TWS_CFG_SAVE_EVENT_DEAL_ENABLE = 0x10,
    EARTCH_TWS_CFG_SAVE_EVENT_DEAL_DISABLE = 0x20,
};

u8 earin_status = 0;
u8 last_status  = 0;
u16 earin_lock_time = 0;

static void eartch_post_event(u8 event)
{
    struct sys_event e;
    user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
    e.type = SYS_DEVICE_EVENT;
    e.arg = (void *)DEVICE_EVENT_FROM_EARTCH;
    e.u.ear.value = event;
    log_info("notify event: %d", event);
    sys_event_notify(&e);
}

static void eartch_send_bt_ctrl_cmd(u8 cmd)
{
    static u8 last_cmd = 0;
    static u32 last_jiffies = 0;
    u32 cur_jiffies = jiffies;
    if (last_cmd == cmd) {
        if (jiffies_to_msecs(cur_jiffies - last_jiffies) < 1000) {
            log_debug("same cmd: %d", cmd);
            return;
        }
    }
    log_info("send cmd: %d", cmd);
    last_cmd = cmd;
    last_jiffies = cur_jiffies;
    user_send_cmd_prepare(cmd, 0, NULL);
}

static void eartch_music_state_check_timer_del(void)
{
    log_debug("timer del");
    if (__this->state_check_timer) {
        usr_timer_del(__this->state_check_timer);
    }
    __this->state_check_timer = 0;
    __this->state_check_cnt = 0;
}

static void eartch_music_state_check(void *priv)
{
    struct eartch_control *eartch = (struct eartch_control *)priv;

    if (get_total_connect_dev() == 0) {
        log_info("no connect dev");
        eartch_music_state_check_timer_del();
        return;
    }

    u8 a2dp_state;

    log_debug("music_check_state cnt: %d, a2dp_connect_state = %d", __this->state_check_cnt, __this->a2dp_connect_state);
    if (__this->a2dp_connect_state) {
        a2dp_state = a2dp_get_status();
        log_debug("a2dp_state = %d", a2dp_state);
        if (__this->last_state == EARTCH_STATE_IN) {
            if (__this->music_ctrl_en) {
                if (a2dp_state != BT_MUSIC_STATUS_STARTING) {
                    log_info("SEND PLAY");
                    eartch_send_bt_ctrl_cmd(USER_CTRL_AVCTP_OPID_PLAY);
                    __this->a2dp_send_flag = 0;
                    eartch_music_state_check_timer_del();
                }
            }
        } else {
            if (a2dp_state == BT_MUSIC_STATUS_STARTING) {
                log_info("SEND PAUSE");
                eartch_send_bt_ctrl_cmd(USER_CTRL_AVCTP_OPID_PAUSE);
                __this->a2dp_send_flag = 0;
                eartch_music_state_check_timer_del();
            }
        }
    }

    __this->state_check_cnt++;
    if (__this->state_check_cnt > 15) {
        eartch_music_state_check_timer_del();
    }
}

static void eartch_music_state_check_timer_add(void)
{
    if (__this->state_check_timer != 0) {
        eartch_music_state_check_timer_del();
    }

    log_debug("timer add");

    __this->state_check_timer = usr_timer_add((void *)__this, eartch_music_state_check, 200, 1);
}

static void eartch_a2dp_status_callback_handle(u8 conn_flag)
{
    log_debug("update a2dp conflag: %d", conn_flag);
    if (conn_flag) {
        __this->a2dp_connect_state = 1;
        log_debug("a2dp_send_flag: %d, event_deal_en: %d", __this->a2dp_send_flag, __this->eartch_event_deal_en);
        if (__this->eartch_event_deal_en) {
            if (__this->a2dp_send_flag) {
                if (__this->music_ctrl_en) {
                    eartch_music_state_check_timer_add();
                }
            }
        }
    } else {
        __this->a2dp_connect_state = 0;
    }
}



static void eartch_sync_tws_state(u8 state)
{
    if (get_tws_sibling_connect_state() == TRUE) {
        tws_api_send_data_to_sibling(&state, 1, TWS_FUNC_ID_EARTCH_SYNC);
    }
}


static void eartch_sync_tws_state_deal(void *_data, u16 len, bool rx)
{
    u8 *data = (u8 *)_data;
    u8 state = data[0];
    if (rx) {
        if (state <= EARTCH_TWS_STATE_OUT) {
            __this->remote_state = state & BIT(0);
            __this->event_source = 0;
            log_debug("sync remote_state: %d", __this->remote_state);
        }
    }
    //TODO: post event
    log_debug("state: %d, local_state: %d, remote_state = %d", state, __this->local_state, __this->remote_state);
    eartch_post_event(state);
}

static void eartch_music_ctrl_timeout_handle(void *priv)
{
    log_info("Disnable music ctrl");
    __this->music_ctrl_en = 0;
    __this->music_ctrl_en_timer = 0;
}


static void eartch_music_ctrl_timeout_del(void)
{
    if (__this->music_ctrl_en_timer) {
        usr_timer_del(__this->music_ctrl_en_timer);
        __this->music_ctrl_en_timer = 0;
    }
}

static void eartch_music_ctrl_timeout_add(void)
{
    if (__this->music_ctrl_en_timer) {
        eartch_music_ctrl_timeout_del();
    }
    __this->music_ctrl_en_timer = usr_timeout_add((void *)__this, eartch_music_ctrl_timeout_handle, 15000, 1);
}

#define A2DP_PLAY_STATE_CNT 		2
#define A2DP_STOP_STATE_CNT 		2
static void eartch_a2dp_status_monitor(void *priv)
{
    if (get_total_connect_dev() == 0) {
        __this->a2dp_stop_state_cnt = 0;
        __this->a2dp_play_state_cnt = 0;
        return;
    }
    u8 a2dp_state = a2dp_get_status();
    if (a2dp_state == BT_MUSIC_STATUS_STARTING) { //在播歌
        __this->a2dp_stop_state_cnt = 0;
        __this->a2dp_play_state_cnt++;
        if (__this->a2dp_play_state_cnt > A2DP_PLAY_STATE_CNT) {
            if (__this->music_ctrl_en == 0) {
                log_info("Enable music ctrl");
                __this->music_ctrl_en = 1;
            }
        }
    } else {
        __this->a2dp_play_state_cnt = 0;
        __this->a2dp_stop_state_cnt++;
    }
}


static void eartch_a2dp_status_monitor_init(u8 init)
{
    if (__this->a2dp_monitor_timer) {
        sys_timer_del(__this->a2dp_monitor_timer);
        __this->a2dp_monitor_timer = 0;
    }
    if (init) {
        __this->a2dp_monitor_timer = sys_timer_add(NULL, eartch_a2dp_status_monitor, 500);
        __this->a2dp_stop_state_cnt = 0;
        __this->a2dp_play_state_cnt = 0;
    }
}

extern void test_esco_role_switch(u8 flag);
//master: local out
static void eartch_in_bt_control_handle(void)
{
    u8 call_status = get_call_status();
    u8 tws_con = get_tws_sibling_connect_state();
    u8 role;
    u8 a2dp_state;
    if (tws_con) {
        role = tws_api_get_role();
    }

    if (call_status != BT_CALL_HANGUP) {
        //通话中
#if TCFG_EARTCH_CALL_CTL_SCO_CONNECT
        if (call_status == BT_CALL_ACTIVE) {//通话中
            if ((tws_con && ((__this->local_state != EARTCH_STATE_IN) || (__this->remote_state != EARTCH_STATE_IN))) //对耳戴上一只
                || (!tws_con)) { //单耳且入耳
                log_info("USER_CTRL_CONN_SCO\n");
                eartch_send_bt_ctrl_cmd(USER_CTRL_CONN_SCO);
            }
        }
#endif /* #if TCFG_EARTCH_CALL_CTL_SCO_CONNECT */
#if TCFG_EARTCH_AUTO_CHANGE_MASTER
        if (tws_con && ((role == TWS_ROLE_MASTER) && ((__this->local_state == EARTCH_STATE_OUT) && (__this->remote_state == EARTCH_STATE_IN)))) {//主机不在耳，从机在耳，切换主从
            log_info("master no inside, start change role");
            tws_api_auto_role_switch_disable();
            test_esco_role_switch(1); //主机调用
        }
#endif /* #if TCFG_EARTCH_AUTO_CHANGE_MASTER */
    } else {
#if TCFG_EARTCH_MUSIC_CTL_A2DP_CONNECT
        log_info("a2dp_connect_state: %d", __this->a2dp_connect_state);
        if (__this->a2dp_connect_state == 0) {
            /* if ((tws_con && (((__this->local_state == EARTCH_STATE_OUT) && (__this->remote_state == EARTCH_STATE_IN)) || */
            /* ((__this->local_state == EARTCH_STATE_IN) && (__this->remote_state == EARTCH_STATE_OUT)))) ||  //对耳一只戴上 */
            /* (!tws_con)) { //单耳 */
            log_info("CONN_A2DP");
            eartch_send_bt_ctrl_cmd(USER_CTRL_CONN_A2DP); //链接链路
            __this->a2dp_send_flag = 1;
            /* } */
        } else
#endif /* #if TCFG_EARTCH_MUSIC_CTL_A2DP_CONNECT */
        {
            log_info("music ctrl en: %d", __this->music_ctrl_en);
            if (__this->music_ctrl_en) {
                a2dp_state = a2dp_get_status();
                log_info("a2dp_state: %d", a2dp_state);
                if (a2dp_state != BT_MUSIC_STATUS_STARTING) { //没有播放
                    log_info("SEND PLAY");
                    eartch_send_bt_ctrl_cmd(USER_CTRL_AVCTP_OPID_PLAY);
                } else {
                    eartch_music_state_check_timer_add();
                }
            }
        }
    }
}

static void eartch_out_bt_control_handle(void)
{
    u8 call_status = get_call_status();
    u8 tws_con = get_tws_sibling_connect_state();
    u8 role;
    u8 a2dp_state;
    if (tws_con) {
        role = tws_api_get_role();
    }

    if (call_status != BT_CALL_HANGUP) {//通话中
        if (call_status == BT_CALL_ACTIVE) {//通话中
            if ((tws_con && ((__this->local_state == EARTCH_STATE_OUT) && (__this->remote_state == EARTCH_STATE_OUT))) //对耳且都不在耳朵上
                || (!tws_con)) { //单耳且不在耳朵
#if TCFG_EARTCH_CALL_CTL_SCO_CONNECT
                log_info("USER_CTRL_DISCONN_SCO\n");
                eartch_send_bt_ctrl_cmd(USER_CTRL_DISCONN_SCO);
#endif /* #if TCFG_EARTCH_CALL_CTL_SCO_CONNECT */
            }
#if TCFG_EARTCH_AUTO_CHANGE_MASTER
            if (tws_con && ((role == TWS_ROLE_MASTER) && ((__this->local_state == EARTCH_STATE_OUT) && (__this->remote_state == EARTCH_STATE_IN)))) {//主机不在耳，从机在耳，切换主从
                log_info("master no inside, start change role");
                tws_api_auto_role_switch_disable();
                test_esco_role_switch(1); //主机调用
            }
#endif /* #if TCFG_EARTCH_AUTO_CHANGE_MASTER */
        }
    } else {
        log_info("a2dp_connect_state: %d", __this->a2dp_connect_state);
        if (__this->a2dp_connect_state) {
#if TCFG_EARTCH_MUSIC_CTL_A2DP_CONNECT
            if ((tws_con && ((__this->local_state == EARTCH_STATE_OUT) && (__this->remote_state == EARTCH_STATE_OUT))) || //对耳出耳, 断开链路
                (!tws_con)) { //单耳出耳
                log_info("DISCON A2DP");
                eartch_send_bt_ctrl_cmd(USER_CTRL_DISCONN_A2DP);
            } else
#endif /* #if TCFG_EARTCH_MUSIC_CTL_A2DP_CONNECT */
            {
                a2dp_state = a2dp_get_status();
                log_info("a2dp_state: %d", a2dp_state);
                if (a2dp_state == BT_MUSIC_STATUS_STARTING) { //在播放
                    log_info("SEND PAUSE");
                    eartch_send_bt_ctrl_cmd(USER_CTRL_AVCTP_OPID_PAUSE);
                } else {
                    eartch_music_state_check_timer_add();
                }
            }
        }
    }
}
//-------------测试盒流程-----------------//
u8 testbox_in_ear_detect_test_flag_get(void);
void lp_touch_key_testbox_inear_trim(u8 flag);

enum {
    IN_EAR_TRIM_SUCC_KEY = 0xf4,
    IN_EAR_DETECT_IN,
    IN_EAR_DETECT_OUT,
    IN_EAR_TRIM_FAIL_KEY,
};

void test_in_ear_detect_state_notify(u8 key)
{
    user_send_cmd_prepare(USER_CTRL_TEST_KEY, 1, &key); //
}

__attribute__((weak))
void lp_touch_key_testbox_inear_trim(u8 flag)
{

}

void eartch_testbox_flag(u8 flag)
{
    lp_touch_key_testbox_inear_trim(flag);
}

static void eartch_bt_event_control_handle(u8 event)
{
    u8 call_status;
    if (get_total_connect_dev() == 0) {
        log_info("no connect dev");
        return;
    }

    u8 tws_con = get_tws_sibling_connect_state();

    if (event == EARTCH_STATE_IN) {
        eartch_music_ctrl_timeout_del();
        eartch_in_bt_control_handle();
    } else {
#if TCFG_EARTCH_MUSIC_CTL_TIMEOUT_ENABLE
        if ((tws_con && ((__this->local_state == EARTCH_STATE_OUT) && (__this->remote_state == EARTCH_STATE_OUT))) //对耳且都不在耳朵上
            || (!tws_con)) { //单耳且不在耳朵
            if (__this->music_ctrl_en) {
                eartch_music_ctrl_timeout_add();
            }
        }
#endif /* #if TCFG_EARTCH_MUSIC_CTL_TIMEOUT_ENABLE */
        eartch_out_bt_control_handle();

    }
}

static void eartch_event_deal_en_cfg_save(u8 cfg_data)
{
    u8 rdata = 0;
    int ret = 0;

    ret = syscfg_read(CFG_EARTCH_ENABLE_ID, &rdata, EARTCH_SWITCH_CFG_LEN);
    if (ret == EARTCH_SWITCH_CFG_LEN) {
        if (rdata == cfg_data) {
            log_debug("cfg same: %d", rdata);
            return;
        }
    }

    rdata = cfg_data;
    log_debug("cfg write: %d", rdata);
    ret = syscfg_write(CFG_EARTCH_ENABLE_ID, &rdata, EARTCH_SWITCH_CFG_LEN);
    if (ret != EARTCH_SWITCH_CFG_LEN) {
        log_error("cfg write err!!!");
    }
}


void eartch_state_update(u8 state)
{
    switch (state) {
    case EARTCH_TWS_STATE_IN:
    case EARTCH_TWS_STATE_OUT:
        __this->local_state = state;
        __this->event_source = 1;
        log_debug("update local_state: %d", __this->local_state);

        if (__this->eartch_event_deal_en == 0) {
            log_debug("eartch event deal en 0");
            return;
        }
        break;
    case EARTCH_TWS_STATE_TRIM_OK:
        if (testbox_in_ear_detect_test_flag_get()) {
            test_in_ear_detect_state_notify(IN_EAR_TRIM_SUCC_KEY);
        }
        return;

    case EARTCH_TWS_STATE_TRIM_ERR:
        if (testbox_in_ear_detect_test_flag_get()) {
            test_in_ear_detect_state_notify(IN_EAR_TRIM_FAIL_KEY);
        }
        return;
    }

    if (get_tws_sibling_connect_state() == TRUE) {
        eartch_sync_tws_state(state);
    } else {
        eartch_post_event(state);
    }

}

extern void eartch_hardware_suspend(void);
extern void eartch_hardware_recover(void);

__attribute__((weak))
void eartch_hardware_suspend(void)
{
    return;
}

__attribute__((weak))
void eartch_hardware_recover(void)
{
    return;
}

void eartch_event_deal_enable(void)
{
    __this->eartch_event_deal_en = 1;
#if TCFG_EARTCH_MUSIC_CTL_TIMEOUT_ENABLE
    eartch_a2dp_status_monitor_init(1);
#endif
    eartch_hardware_recover();
}

void eartch_event_deal_disable()
{
    __this->eartch_event_deal_en = 0;
#if TCFG_EARTCH_MUSIC_CTL_TIMEOUT_ENABLE
    eartch_a2dp_status_monitor_init(0);
#endif
    eartch_hardware_suspend();
}

void eartch_event_deal_enable_cfg_save(u8 en)
{
#if TCFG_EARTCH_SWITCH_CFG_ENABLE
    if (en) {
        eartch_event_deal_enable();
        eartch_event_deal_en_cfg_save(1);
    } else {
        eartch_event_deal_disable();
        eartch_event_deal_en_cfg_save(0);
    }
#endif /* #if TCFG_EARTCH_SWITCH_CFG_ENABLE */
}

void eartch_handle()
{
    if (earin_status == EARTCH_STATE_IN) {
        if (get_tws_sibling_connect_state() == TRUE) {
            if ((__this->remote_state == EARTCH_STATE_OUT) && (__this->local_state == EARTCH_STATE_IN)) {
                log_info("tone_play1");
                tone_play(TONE_NORMAL, 1);
            }
        } else {
            log_info("tone_play2");
            tone_play(TONE_NORMAL, 1);
        }

        if (testbox_in_ear_detect_test_flag_get()) {
            test_in_ear_detect_state_notify(IN_EAR_DETECT_IN);
        }
    } else {
        if (testbox_in_ear_detect_test_flag_get()) {
            test_in_ear_detect_state_notify(IN_EAR_DETECT_OUT);
        }

    }


    eartch_bt_event_control_handle(earin_status);
    __this->last_state = earin_status;
    sys_timer_del(earin_lock_time);
    earin_lock_time = 0;
}

void eartch_event_handle(u8 event)
{
    switch (event) {
    case EARTCH_TWS_STATE_IN:
    case EARTCH_TWS_STATE_OUT:
        last_status = event;
        if (earin_lock_time == 0) {
            earin_status = event;
            earin_lock_time = sys_timeout_add(NULL, eartch_handle, 500);
        } else {
            if (earin_status != last_status) {
                earin_status = last_status;
                sys_timer_modify(earin_lock_time, 500);
            }
        }
        break;

#if TCFG_EARTCH_SWITCH_CFG_ENABLE
    case EARTCH_TWS_CFG_SAVE_EVENT_DEAL_ENABLE:
        eartch_event_deal_enable_cfg_save(1);
        break;

    case EARTCH_TWS_CFG_SAVE_EVENT_DEAL_DISABLE:
        eartch_event_deal_enable_cfg_save(0);
        break;
#endif /* #if TCFG_EARTCH_SWITCH_CFG_ENABLE */

    default:
        break;
    }
}

void a2dp_connect_status_register(void (*cbk)(u8 conn_flag));
int eartch_event_deal_init(void)
{
    u8 rdata = 0;
    int ret = 0;

#if TCFG_EARTCH_SWITCH_CFG_ENABLE
    ret = syscfg_read(CFG_EARTCH_ENABLE_ID, &rdata, EARTCH_SWITCH_CFG_LEN);
    if (ret == EARTCH_SWITCH_CFG_LEN) {
        __this->eartch_event_deal_en = rdata;
        log_debug("switch cfg: %d", rdata);
    } else
#endif /* #if TCFG_EARTCH_SWITCH_CFG_ENABLE */
    {
        log_debug("switch cfg not write, default enable");
        __this->eartch_event_deal_en = 1;
    }

#if TCFG_EARTCH_MUSIC_CTL_A2DP_CONNECT
    a2dp_connect_status_register(eartch_a2dp_status_callback_handle);
#else
    __this->a2dp_connect_state = 1;
#endif /* #if TCFG_EARTCH_MUSIC_CTL_A2DP_CONNECT */
#if TCFG_EARTCH_MUSIC_CTL_TIMEOUT_ENABLE
    if (__this->eartch_event_deal_en) {
        eartch_a2dp_status_monitor_init(1);
    }
#else
    __this->music_ctrl_en = 1;
#endif /* #if TCFG_EARTCH_MUSIC_CTL_TIMEOUT_ENABLE */
    return __this->eartch_event_deal_en;
}


void eartch_event_deal_enable_cfg_tws_save(u8 en)
{
    if (en) {
        eartch_state_update(EARTCH_TWS_CFG_SAVE_EVENT_DEAL_ENABLE);
    } else {
        eartch_state_update(EARTCH_TWS_CFG_SAVE_EVENT_DEAL_DISABLE);
    }
}


REGISTER_TWS_FUNC_STUB(ear_lptch_sync) = {
    .func_id = TWS_FUNC_ID_EARTCH_SYNC,
    .func    = eartch_sync_tws_state_deal,
};

#endif /* #if TCFG_EARTCH_EVENT_HANDLE_ENABLE */
