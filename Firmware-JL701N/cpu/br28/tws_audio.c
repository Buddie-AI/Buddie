/*****************************************************************
>file name : tws_audio.c
>create time : Wed 08 Dec 2021 01:52:02 PM CST
>处理tws与audio模块相关的事件等技术交叉点
*****************************************************************/
#include "app_config.h"
#include "media/includes.h"
#include "system/includes.h"

#if TCFG_USER_TWS_ENABLE
#include "btstack/avctp_user.h"
#include "bt_tws.h"

extern int CONFIG_BTCTLER_TWS_ENABLE;
extern struct audio_dac_hdl dac_hdl;

static void tws_audio_event_handler(struct sys_event *event)
{
    if (!CONFIG_BTCTLER_TWS_ENABLE) {
        return;
    }

    if (((u32)event->arg != SYS_BT_EVENT_FROM_TWS)) {
        return;
    }

    struct bt_event *e = &event->u.bt;
    int state;
    if (e->event == TWS_EVENT_CONNECTED) {
        state = e->args[2];
        /*
         * 当TWS连接时，并且确认要播放提示音，则提前进行DAC上电
         * 原因: DAC上电有较大延迟，会导致连接提示音的错位
         */
        if (!get_bt_tws_discon_dly_state() && (get_call_status() == BT_CALL_HANGUP) && !(state & TWS_STA_SBC_OPEN)) {
//考虑开了DEC2TWS_ENABLE不播提示音是情况
#if !TCFG_DEC2TWS_ENABLE
            audio_dac_try_power_on(&dac_hdl);
#endif/*!TCFG_DEC2TWS_ENABLE*/
        }
    }
}

SYS_EVENT_HANDLER(SYS_BT_EVENT, tws_audio_event_handler, 4);
#endif
