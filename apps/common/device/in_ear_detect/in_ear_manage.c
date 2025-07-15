#include "in_ear_detect/in_ear_manage.h"
#include "in_ear_detect/in_ear_detect.h"
#include "system/includes.h"
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "tone_player.h"

#define LOG_TAG_CONST       EAR_DETECT
#define LOG_TAG             "[EAR_DETECT]"
#define LOG_INFO_ENABLE
#include "debug.h"

//#define log_info(format, ...)       y_printf("[EAR_DETECT] : " format "\r\n", ## __VA_ARGS__)


#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif

#if TCFG_EAR_DETECT_ENABLE

#if INEAR_ANC_UI
#include "audio_anc.h"
u8 inear_tws_ancmode = ANC_OFF;
#endif

static struct ear_detect_d _ear_detect_d = {
    .toggle = 1,
    .music_en = 0, //上电开机默认不使能音乐控制
    .pre_music_sta = MUSIC_STATE_NULL,
    .pre_state = !TCFG_EAR_DETECT_DET_LEVEL, //默认没有入耳
    .cur_state = !TCFG_EAR_DETECT_DET_LEVEL, //默认没有入耳
    .tws_state = !TCFG_EAR_DETECT_DET_LEVEL, //默认没有入耳
    .bt_init_ok = 1,
    .music_check_timer = 0,
    .music_sta_cnt = 0,
    .change_master_timer = 0,
    .key_enable_timer = 0,
    .key_delay_able = 1,
    //TCFG_EAR_DETECT_MUSIC_CTL_EN
    .play_cnt = 0,
    .stop_cnt = 0,
    .music_ctl_timeout = 0,
    .a2dp_det_timer = 0,
    .music_regist_en = 0,
};

#define __this 	(&_ear_detect_d)

//入耳检测公共控制
u8 is_ear_detect_state_in(void)     //入耳返回1
{
    if (0 == __this->toggle) {
        return 1;
    }
    return (__this->cur_state == TCFG_EAR_DETECT_DET_LEVEL);
}

u8 is_ear_detect_tws_state_in(void) //对耳入耳返回1
{
    if (0 == __this->toggle) {
        return 1;
    }
    return (__this->tws_state == TCFG_EAR_DETECT_DET_LEVEL);
}

void ear_detect_music_ctl_delay_deal(void *priv)
{
    //暂停n秒内戴上控制音乐，超过后不能控制
    log_info("%s", __func__);
    __this->music_en = 0;
    __this->music_ctl_timeout = 0;
    __this->music_regist_en = 0;
}

void ear_detect_music_ctl_timer_del()
{
    log_info("%s", __func__);
    if (__this->music_ctl_timeout) {
        sys_timeout_del(__this->music_ctl_timeout);
        __this->music_ctl_timeout = 0;
    }
}

static void ear_detect_music_ctl_en(u8 en)
{
    log_info("%s, %d, music_en:%d, timeout:%d,regist_en:%d", __func__, en, __this->music_en, __this->music_ctl_timeout, __this->music_regist_en);
    if (en) {
        __this->music_en = en;
        if (__this->music_ctl_timeout) {
            log_info("del pause timeout deal");
            sys_timeout_del(__this->music_ctl_timeout);
            __this->music_ctl_timeout = 0;
        }
    } else {
        if (__this->music_regist_en == 0) {
            __this->music_en = 0;
            return;
        }
        if (__this->music_en && (0 == __this->music_ctl_timeout)) {
            log_info("regist pause timeout deal");
            if (__this->cfg->ear_det_music_ctl_ms) {
                __this->music_ctl_timeout = sys_timeout_add(NULL, ear_detect_music_ctl_delay_deal, __this->cfg->ear_det_music_ctl_ms);
            }
        }
    }
}

#define A2DP_PLAY_CNT           2
#define A2DP_STOP_CNT           2
static void ear_detect_a2dp_detech(void *priv)
{
    u8 a2dp_state = 0;
    if (get_call_status() != BT_CALL_HANGUP) {
        return;
    }
    a2dp_state = a2dp_get_status();
    //printf("a2dp: %d", a2dp_state);
    if (a2dp_state == BT_MUSIC_STATUS_STARTING) { //在播歌
        __this->stop_cnt = 0;
        if (__this->play_cnt < A2DP_PLAY_CNT) {
            __this->play_cnt++;
            if (__this->play_cnt == A2DP_PLAY_CNT) {
                __this->pre_music_sta = MUSIC_STATE_PLAY;
                ear_detect_music_ctl_en(1);
            }
        }
    } else { //不在播歌
        __this->play_cnt = 0;
        if (__this->stop_cnt < A2DP_STOP_CNT) {
            __this->stop_cnt++;
            if (__this->stop_cnt == A2DP_STOP_CNT) {
                if (__this->music_en) { //入耳检测已经使能
                    __this->pre_music_sta = MUSIC_STATE_PAUSE;
                    ear_detect_music_ctl_en(0);
                }
            }
        }
    }
}

void ear_detect_a2dp_det_en(u8 en)
{
    if (en) {
        __this->play_cnt = 0;
        __this->stop_cnt = 0;
        if (!__this->a2dp_det_timer) {
            log_info("ear_detect_a2dp_detech timer add");
            __this->a2dp_det_timer = sys_timer_add(NULL, ear_detect_a2dp_detech, 500);
        }
    } else {
        if (__this->a2dp_det_timer) {
            log_info("ear_detect_a2dp_detech timer del");
            sys_timer_del(__this->a2dp_det_timer);
            __this->a2dp_det_timer = 0;
        }
    }
}

static void cmd_post_key_msg(u8 user_msg)
{
    struct sys_event e = {0};
    e.type = SYS_KEY_EVENT;
    e.u.key.event = KEY_EVENT_USER;
    e.u.key.value = user_msg;

    sys_event_notify(&e);
}

int cmd_key_msg_handle(struct sys_event *event)
{
    log_info("%s\n", __func__);
    struct key_event *key = &event->u.key;
    switch (key->value) {
    case CMD_EAR_DETECT_MUSIC_PLAY:
        log_info("CMD_EAR_DETECT_MUSIC_PLAY");
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
        break;
    case CMD_EAR_DETECT_MUSIC_PAUSE:
        log_info("CMD_EAR_DETECT_MUSIC_PAUSE");
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
        break;
    case CMD_EAR_DETECT_SCO_CONN:
        log_info("CMD_EDETECT_SCO_CONN");
        user_send_cmd_prepare(USER_CTRL_CONN_SCO, 0, NULL);
        break;
    case CMD_EAR_DETECT_SCO_DCONN:
        log_info("CMD_EAR_DETECT_SCO_DCONN");
        user_send_cmd_prepare(USER_CTRL_DISCONN_SCO, 0, NULL);
        break;
    default:
        break;
    }
    return 0;
}
static void ear_detect_post_event(u8 event)
{
    struct sys_event e;
    user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
    e.type = SYS_DEVICE_EVENT;
    e.arg = (void *)DEVICE_EVENT_FROM_EAR_DETECT;
    e.u.ear.value = event;
    sys_event_notify(&e);
}

static void cancel_music_state_check()
{
    if (__this->music_check_timer) {
        sys_timer_del(__this->music_check_timer);
        __this->music_check_timer = 0;
        __this->music_sta_cnt = 0;
    }
}

static void music_play_state_check(void *priv)
{
    if (__this->pre_music_sta == MUSIC_STATE_PLAY) {
        if (__this->cfg->ear_det_in_music_sta == 1) {
            cancel_music_state_check();
        } else {
            if (a2dp_get_status() == BT_MUSIC_STATUS_STARTING) {
                __this->music_sta_cnt++;
                log_info("play cnt: %d", __this->music_sta_cnt);
            } else { //期间变成了暂停
                cmd_post_key_msg(CMD_EAR_DETECT_MUSIC_PLAY);
                cancel_music_state_check();
            }
            if (__this->music_sta_cnt >= __this->cfg->ear_det_music_play_cnt) {
                cancel_music_state_check();
            }
        }
    } else {
        if (a2dp_get_status() != BT_MUSIC_STATUS_STARTING) {
            __this->music_sta_cnt++;
            log_info("pause cnt: %d", __this->music_sta_cnt);
        } else { //期间变成了暂停
            cmd_post_key_msg(CMD_EAR_DETECT_MUSIC_PAUSE);
            cancel_music_state_check();
        }
        if (__this->music_sta_cnt >= __this->cfg->ear_det_music_pause_cnt) {
            cancel_music_state_check();
        }
    }
}

static void ear_detect_music_play_ctl(u8 music_state)
{
    log_info("%s", __func__);
#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state() && (__this->pre_music_sta == music_state))  //防止两只耳机陆续入耳，重复操作
#endif
    {
        log_info("tws same or notws . music_state: %d", music_state);
        if (__this->cfg->ear_det_music_ctl_en == 1) {
            return;
        }
    }
    __this->pre_music_sta = music_state;
    log_info("music state:%d music_en:%d", music_state, __this->music_en);
    if (__this->music_en) { //入耳检测控制音乐使能
        if (music_state == MUSIC_STATE_PLAY) { //播放音乐
            log_info("MUSIC_STATE_PLAY");
            if (__this->cfg->ear_det_music_ctl_en == 1) {
                __this->music_regist_en = 0;
            }
            if (__this->cfg->ear_det_in_music_sta == 0) {
                if (a2dp_get_status() != BT_MUSIC_STATUS_STARTING) { //没有播放
                    log_info("-------1");
                    cmd_post_key_msg(CMD_EAR_DETECT_MUSIC_PLAY);
                } else {
                    log_info("-------2");
                    cancel_music_state_check();
                    __this->music_check_timer = sys_timer_add(NULL, music_play_state_check, 100);
                }
            }
        } else { //暂停音乐
            log_info("MUSIC_STATE_PAUSE");
            if (__this->cfg->ear_det_music_ctl_en == 1) {
                __this->music_regist_en = 1;
            }
            if (a2dp_get_status() == BT_MUSIC_STATUS_STARTING) { //正在播放
                log_info("-------3");
                cmd_post_key_msg(CMD_EAR_DETECT_MUSIC_PAUSE);
            } else {
                log_info("-------4");
                cancel_music_state_check();
                __this->music_check_timer = sys_timer_add(NULL, music_play_state_check, 100);
            }
        }
    }
}

static u8 ear_detect_check_online(u8 check_mode)
{
    if (check_mode == 1) {
#if TCFG_USER_TWS_ENABLE
        if ((get_tws_sibling_connect_state() && (__this->cur_state != TCFG_EAR_DETECT_DET_LEVEL) && (__this->tws_state != TCFG_EAR_DETECT_DET_LEVEL)) //对耳且都不在耳朵上
            || (!get_tws_sibling_connect_state() && (__this->cur_state != TCFG_EAR_DETECT_DET_LEVEL)))  //单耳且不在耳朵
#else
        if (__this->cur_state != TCFG_EAR_DETECT_DET_LEVEL)
#endif
        {
            return 1;
        }
        return 0;
    } else if (check_mode == 0) {
#if TCFG_USER_TWS_ENABLE
        if ((get_tws_sibling_connect_state() && ((__this->cur_state == TCFG_EAR_DETECT_DET_LEVEL) || (__this->tws_state == TCFG_EAR_DETECT_DET_LEVEL))) //对耳戴上一只
            || (!get_tws_sibling_connect_state() && (__this->cur_state == TCFG_EAR_DETECT_DET_LEVEL)))  //单耳且入耳
#else
        if (__this->cur_state == TCFG_EAR_DETECT_DET_LEVEL)
#endif
        {
            return 1;
        }
        return 0;
    }
    return 0;
}

void ear_detect_phone_active_deal()
{
    if (0 == __this->toggle) {
        return;
    }
    if (ear_detect_check_online(1) == 1) {
        cmd_post_key_msg(CMD_EAR_DETECT_SCO_DCONN);
    }
}

#if (TCFG_EAR_DETECT_AUTO_CHG_MASTER && TCFG_USER_TWS_ENABLE)
extern void tws_conn_switch_role();
extern void test_esco_role_switch(u8 flag);
void ear_detect_change_master_timeout_deal(void *priv) //主从切换
{
    y_printf("%s", __func__);
    __this->change_master_timer = 0;
    tws_api_auto_role_switch_disable();
    test_esco_role_switch(1); //主机调用
}

void ear_detect_change_master_timer_del()
{
    if (__this->change_master_timer) {
        y_printf("%s", __func__);
        sys_timeout_del(__this->change_master_timer);
        __this->change_master_timer = 0;
    }
    tws_api_auto_role_switch_enable(); //恢复主从自动切换
}

//开始通话时，判断主机是否在耳朵上，不在切换主从
void ear_detect_call_chg_master_deal()
{
    y_printf("self:%d tws:%d\n", __this->cur_state, __this->tws_state);
    if (0 == __this->toggle) {
        return;
    }
    if (get_tws_sibling_connect_state() && (tws_api_get_role() == TWS_ROLE_MASTER) && (__this->cur_state != TCFG_EAR_DETECT_DET_LEVEL) && (__this->tws_state == TCFG_EAR_DETECT_DET_LEVEL)) { //主机不在耳，从机在耳，切换主从
        y_printf("%s", __func__);
        ear_detect_change_master_timeout_deal(NULL);
    }
}
#endif

static void ear_detect_in_deal()   //主机才会执行
{
    log_info("%s", __func__);
    u8 call_status = get_call_status();
    y_printf("[EAR_DETECT] : in self:%d tws:%d call_st:%d\n", __this->cur_state, __this->tws_state, call_status);
    if (0 == __this->toggle) {
        return;
    }
    if (call_status != BT_CALL_HANGUP) {//通话中
#if TCFG_EAR_DETECT_CALL_CTL_SCO
        if (call_status == BT_CALL_ACTIVE) {//通话中
            if (ear_detect_check_online(0) == 1) {
                log_info("CMD_CTRL_CONN_SCO\n");
                cmd_post_key_msg(CMD_EAR_DETECT_SCO_CONN);
            }
        }
#endif
#if (TCFG_EAR_DETECT_AUTO_CHG_MASTER && TCFG_USER_TWS_ENABLE)
        ear_detect_change_master_timer_del();
        if (get_tws_sibling_connect_state() && (__this->cur_state != TCFG_EAR_DETECT_DET_LEVEL) && (__this->tws_state == TCFG_EAR_DETECT_DET_LEVEL)) { //主机不在耳，从机在耳，切换主从
            y_printf("master no inside, start change role");
            __this->change_master_timer = sys_timeout_add(NULL, ear_detect_change_master_timeout_deal, 2000);
        }
#endif
    } else if (get_total_connect_dev()) { //已连接
        if ((__this->cur_state == TCFG_EAR_DETECT_DET_LEVEL) || (__this->tws_state == TCFG_EAR_DETECT_DET_LEVEL)) { //可以自定义控制暂停播放的条件
            ear_detect_music_play_ctl(MUSIC_STATE_PLAY);
        }
    }
}

static void ear_detect_out_deal()   //主机才会执行
{
    log_info("%s", __func__);
    u8 call_status = get_call_status();
    y_printf("[EAR_DETECT] : out self:%d tws:%d call_st:%d\n", __this->cur_state, __this->tws_state, call_status);
    if (0 == __this->toggle) {
        return;
    }
    if (call_status != BT_CALL_HANGUP) {//通话中
        if (call_status == BT_CALL_ACTIVE) {//通话中
            if (ear_detect_check_online(1) == 1) {
#if (TCFG_EAR_DETECT_AUTO_CHG_MASTER && TCFG_USER_TWS_ENABLE)
                ear_detect_change_master_timer_del(); //通话中如果主机摘掉，然后从机也摘掉，不切换主从
#endif
#if TCFG_EAR_DETECT_CALL_CTL_SCO
                cmd_post_key_msg(CMD_EAR_DETECT_SCO_DCONN);
                log_info("CMD_CTRL_DISCONN_SCO\n");
#endif
            }
#if (TCFG_EAR_DETECT_AUTO_CHG_MASTER && TCFG_USER_TWS_ENABLE)
            ear_detect_change_master_timer_del();
            if (get_tws_sibling_connect_state() && (__this->cur_state != TCFG_EAR_DETECT_DET_LEVEL) && (__this->tws_state == TCFG_EAR_DETECT_DET_LEVEL)) { //主机不在耳，从机在耳，切换主从
                y_printf("master no inside, start change role");
                __this->change_master_timer = sys_timeout_add(NULL, ear_detect_change_master_timeout_deal, 2000);
            }
#endif
        }
    } else if (get_total_connect_dev()) { //已连接
        if ((__this->cur_state != TCFG_EAR_DETECT_DET_LEVEL) || (__this->tws_state != TCFG_EAR_DETECT_DET_LEVEL)) { //可以自定义控制暂停播放的条件
            ear_detect_music_play_ctl(MUSIC_STATE_PAUSE);
        }
    }
}

#if TCFG_USER_TWS_ENABLE
void tws_sync_ear_detect_state(u8 need_do)
{
    u8 state = 0;
    state = need_do ? (BIT(7) | __this->cur_state) : __this->cur_state;
    if (!need_do) {
        state |= (__this->music_en << 6);
    }
    r_printf("[EAR_DETECT] : %s: %x , %x ", __func__, state, __this->cur_state);
    tws_api_send_data_to_sibling(&state, 1, TWS_FUNC_ID_EAR_DETECT_SYNC);
}

static void tws_sync_ear_detect_state_deal(void *_data, u16 len, bool rx) //在这个回调里面，不能执行太久，不要调用tws_api_get_role()和user_send_cmd_prepare，会偶尔死机
{
    u8 *data = (u8 *)_data;
    u8 state = data[0];

    if (rx) {
        r_printf("[EAR_DETECT] : %s: %x , rx = %x", __func__, state, rx);
        __this->tws_state = state & BIT(0);
        if ((state & BIT(7))) { //主机并且需要执行 // && (tws_api_get_role() == TWS_ROLE_MASTER)
            if (__this->tws_state == TCFG_EAR_DETECT_DET_LEVEL) {
                ear_detect_post_event(EAR_DETECT_EVENT_IN_DEAL);
            } else {
                ear_detect_post_event(EAR_DETECT_EVENT_OUT_DEAL);
            }
        }
    }
}
REGISTER_TWS_FUNC_STUB(ear_detect_sync) = {
    .func_id = TWS_FUNC_ID_EAR_DETECT_SYNC,
    .func    = tws_sync_ear_detect_state_deal,
};
#endif

void ear_detect_change_state_to_event(u8 state)
{
    u8 event = EAR_DETECT_EVENT_NULL;
    __this->pre_state = __this->cur_state;
    __this->cur_state = state;
    if (__this->cur_state == __this->pre_state) {
        log_info("same state,return");
        return;
    }
    log_info("post event, cur_state:%d, pre_state:%d", __this->cur_state, __this->pre_state);
    event = (state == TCFG_EAR_DETECT_DET_LEVEL) ? EAR_DETECT_EVENT_IN : EAR_DETECT_EVENT_OUT;
    ear_detect_post_event(event);
}

static void __ear_detect_in_dealy_deal(void *priv)
{
    static u16 tone_delay_in_deal = 0;
    if (!tone_get_status()) {
        sys_timer_del(tone_delay_in_deal);
        tone_delay_in_deal = 0;
        ear_detect_in_deal();
    } else {
        if (!tone_delay_in_deal) {
            tone_delay_in_deal = sys_timer_add(NULL, __ear_detect_in_dealy_deal, 100);
        }
    }
}

static void ear_detect_set_key_delay_able(void *priv)
{
    u8 able = (u8)priv;
    __this->key_delay_able = able;
}

u8 ear_detect_get_key_delay_able(void)
{
    return __this->key_delay_able;
}
#if INEAR_ANC_UI
void etch_in_anc(void)
{
    if (get_tws_sibling_connect_state()) {//tws
        if (__this->tws_state && __this->cur_state) {
            if (inear_tws_ancmode == ANC_ON) {
                log_info("switch anc mode\n");
                anc_mode_switch(inear_tws_ancmode, 0);
                inear_tws_ancmode = 1;
            }
        } else if (__this->cur_state && !__this->tws_state) {
            if (inear_tws_ancmode == ANC_ON) {
                anc_mode_switch(ANC_TRANSPARENCY, 0);
            }
        }
    }
}

void etch_out_anc(void)
{
    if (get_tws_sibling_connect_state()) {//tws
        if (anc_mode_get() == ANC_ON) {
            inear_tws_ancmode = ANC_ON;
            if (!__this->cur_state) {
                anc_mode_switch(ANC_OFF, 0);
            } else {
                anc_mode_switch(ANC_TRANSPARENCY, 0);

            }
        } else if (anc_mode_get() == ANC_TRANSPARENCY) {
            if (inear_tws_ancmode == ANC_ON) {
                anc_mode_switch(ANC_OFF, 0);
            }
        }
    }
}
#endif
void tone_play_deal(const char *name, u8 preemption, u8 add_en);
void ear_detect_event_handle(u8 state)
{
    switch (state) {
    case EAR_DETECT_EVENT_NULL:
        log_info("EAR_DETECT_EVENT_NULL");
        break;
    case EAR_DETECT_EVENT_IN:
        log_info("EAR_DETECT_EVENT_IN");
#if INEAR_ANC_UI
        etch_in_anc();
#endif
        log_info("toggle = %d,call_status = %d\n", __this->toggle, get_call_status());
        if (__this->toggle && __this->bt_init_ok && (get_call_status() == BT_CALL_HANGUP)) {
#if TCFG_USER_TWS_ENABLE
            if (get_tws_sibling_connect_state()) { //对耳链接上了，对耳不在耳时播，第一只播
                if (__this->tws_state != TCFG_EAR_DETECT_DET_LEVEL) { //对耳已经戴上了，不播放
                    bt_tws_play_tone_at_same_time(SYNC_TONE_EARDET_IN, 400);
                }
            } else //对耳没连接上，自己决定

#endif
            {
                tone_play(TONE_EAR_CHECK, 1);
            }
        }
#if TCFG_USER_TWS_ENABLE
        if (get_tws_sibling_connect_state()) {
            tws_sync_ear_detect_state(1);
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                __ear_detect_in_dealy_deal(NULL);
            }
        } else
#endif
        {
            __ear_detect_in_dealy_deal(NULL);
        }
        if ((__this->key_enable_timer == 0) && (__this->cfg->ear_det_key_delay_time != 0)) {
            __this->key_enable_timer = sys_timeout_add((void *)1, ear_detect_set_key_delay_able, __this->cfg->ear_det_key_delay_time);
        }
        break;
    case EAR_DETECT_EVENT_OUT:
        log_info("EAR_DETECT_EVENT_OUT");
#if INEAR_ANC_UI
        etch_out_anc();
#endif
#if TCFG_USER_TWS_ENABLE
        if (get_tws_sibling_connect_state()) {
            tws_sync_ear_detect_state(1);
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                ear_detect_out_deal();
            }
        } else
#endif
        {
            ear_detect_out_deal();
        }
        if (__this->cfg->ear_det_key_delay_time != 0) {
            ear_detect_set_key_delay_able(0);
            if (__this->key_enable_timer) {
                sys_timeout_del(__this->key_enable_timer);
                __this->key_enable_timer = 0;
            }
        }
        break;
#if TCFG_USER_TWS_ENABLE
    case EAR_DETECT_EVENT_IN_DEAL:
#if INEAR_ANC_UI
        etch_in_anc();
#endif
        log_info("EAR_DETECT_EVENT_IN_DEAL");
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            __ear_detect_in_dealy_deal(NULL);
        }
        break;
    case EAR_DETECT_EVENT_OUT_DEAL:
        log_info("EAR_DETECT_EVENT_OUT_DEAL");
#if INEAR_ANC_UI
        etch_out_anc();
#endif
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            ear_detect_out_deal();
        }
        break;
#endif
    default:
        break;
    }
}

void ear_touch_edge_wakeup_handle(u8 index, u8 gpio)
{
    u8 io_state = 0;
    ASSERT(gpio == TCFG_EAR_DETECT_DET_IO);

    io_state = gpio_read(TCFG_EAR_DETECT_DET_IO);

    if (io_state == TCFG_EAR_DETECT_DET_LEVEL) {
        log_info("earphone touch in\n");
#if TCFG_KEY_IN_EAR_FILTER_ENABLE
        extern u8 io_key_filter_flag;
        if (gpio_read(IO_PORTB_01) == 0) {
            io_key_filter_flag = 1;
        } else {
            io_key_filter_flag = 0;
        }
#endif
        ear_detect_change_state_to_event(TCFG_EAR_DETECT_DET_LEVEL);
    } else {
        log_info("earphone touch out\n");
        ear_detect_change_state_to_event(!TCFG_EAR_DETECT_DET_LEVEL);
    }

    if (io_state) {
        //current: High
        power_wakeup_set_edge(TCFG_EAR_DETECT_DET_IO, FALLING_EDGE);
    } else {
        //current: Low
        power_wakeup_set_edge(TCFG_EAR_DETECT_DET_IO, RISING_EDGE);
    }
}

static void ear_det_app_event_handler(struct sys_event *event)
{
    //log_info("%s", __func__);
    struct bt_event *bt = &(event->u.bt);
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            switch (bt->event) {
            case BT_STATUS_FIRST_CONNECTED:
            case BT_STATUS_SECOND_CONNECTED:
                log_info("BT_STATUS_CONNECTED\n");
                if (__this->cfg->ear_det_music_ctl_en) {
                    ear_detect_a2dp_det_en(1);
                }
                break;
            case BT_STATUS_FIRST_DISCONNECT:
            case BT_STATUS_SECOND_DISCONNECT:
                log_info("BT_STATUS_DISCONNECT\n");
                if (__this->cfg->ear_det_music_ctl_en) {
                    ear_detect_a2dp_det_en(0);
                }
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
}

void ear_detect_init(const struct ear_detect_platform_data *cfg)
{
    log_info("%s", __func__);
    ASSERT(cfg);
    __this->cfg = cfg;

    if (__this->cfg->ear_det_music_ctl_en) {
        register_sys_event_handler(SYS_BT_EVENT, 0, 0, ear_det_app_event_handler);
    }

#if (TCFG_EAR_DETECT_TYPE == EAR_DETECT_BY_IR)
    ear_detect_ir_init(cfg);
#elif (TCFG_EAR_DETECT_TYPE == EAR_DETECT_BY_TOUCH)
    ear_detect_tch_init(cfg);
#endif

}

#endif

