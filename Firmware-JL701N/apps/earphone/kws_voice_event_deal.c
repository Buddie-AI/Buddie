#include "includes.h"
#include "event.h"
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "media/kws_event.h"

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif
#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif/*TCFG_AUDIO_ANC_ENABLE*/

#define LOG_TAG_CONST       KWS_VOICE_EVENT
#define LOG_TAG             "[KWS_VOICE_EVENT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if ((defined TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE) && (TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE))

struct jl_kws_event_hdl {
    u32 last_event;
    u32 last_event_jiffies;
};

static struct jl_kws_event_hdl kws_hdl = {
    .last_event = 0,
    .last_event_jiffies = 0,
};

#define __this 		(&kws_hdl)


extern void volume_up(u8 inc);
extern void volume_down(u8 inc);

/* ---------------------------------------------------------------------------- */
/**
 * @brief: 关键词唤醒语音事件处理流程
 *
 * @param event: 系统事件
 *
 * @return : true: 处理该事件; false: 不处理该事件, 由
 */
/* ---------------------------------------------------------------------------- */
int jl_kws_voice_event_handle(struct sys_event *event)
{
    struct key_event *key = &(event->u.key);

    if (key->type != KEY_DRIVER_TYPE_VOICE) {
        return false;
    }

    u32 cur_jiffies = jiffies;
    u8 a2dp_state;
    u8 call_state;
    u32 voice_event = key->event;

    log_info("%s: event: %d", __func__, voice_event);

    if (voice_event == __this->last_event) {
        if (jiffies_to_msecs(cur_jiffies - __this->last_event_jiffies) < 1000) {
            log_info("voice event %d same, ignore", voice_event);
            __this->last_event_jiffies = cur_jiffies;
            return true;
        }
    }
    __this->last_event_jiffies = cur_jiffies;
    __this->last_event = voice_event;

    switch (voice_event) {
    case KWS_EVENT_HEY_KEYWORD:
    case KWS_EVENT_XIAOJIE:
        //主唤醒词:
        log_info("send SIRI cmd");
        user_send_cmd_prepare(USER_CTRL_HFP_GET_SIRI_OPEN, 0, NULL);
        break;

    case KWS_EVENT_XIAODU:
        //主唤醒词:
        log_info("send SIRI cmd");
        user_send_cmd_prepare(USER_CTRL_HFP_GET_SIRI_OPEN, 0, NULL);
        break;

    case KWS_EVENT_PLAY_MUSIC:
    case KWS_EVENT_STOP_MUSIC:
    case KWS_EVENT_PAUSE_MUSIC:
        call_state = get_call_status();
        if ((call_state == BT_CALL_OUTGOING) ||
            (call_state == BT_CALL_ALERT)) {
            //user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        } else if (call_state == BT_CALL_INCOMING) {
            //user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        } else if (call_state == BT_CALL_ACTIVE) {
            //user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        } else {
            a2dp_state = a2dp_get_status();
            if (a2dp_state == BT_MUSIC_STATUS_STARTING) {
                if (voice_event == KWS_EVENT_PAUSE_MUSIC) {
                    log_info("send PAUSE cmd");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
                } else if (voice_event == KWS_EVENT_STOP_MUSIC) {
                    log_info("send STOP cmd");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_STOP, 0, NULL);
                }
            } else {
                if (voice_event == KWS_EVENT_PLAY_MUSIC) {
                    log_info("send PLAY cmd");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
                }
            }
        }
        break;

    case KWS_EVENT_VOLUME_UP:
        log_info("volume up");
        volume_up(4); //music: 0 ~ 16, call: 0 ~ 15, step: 25%
        break;

    case KWS_EVENT_VOLUME_DOWN:
        log_info("volume down");
        volume_down(4); //music: 0 ~ 16, call: 0 ~ 15, step: 25%
        break;

    case KWS_EVENT_PREV_SONG:
        log_info("Send PREV cmd");
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PREV, 0, NULL);
        break;

    case KWS_EVENT_NEXT_SONG:
        log_info("Send NEXT cmd");
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_NEXT, 0, NULL);
        break;

    case KWS_EVENT_CALL_ACTIVE:
        if (get_call_status() == BT_CALL_INCOMING) {
            log_info("Send ANSWER cmd");
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        }
        break;

    case KWS_EVENT_CALL_HANGUP:
        log_info("Send HANG UP cmd");
        if ((get_call_status() >= BT_CALL_INCOMING) && (get_call_status() <= BT_CALL_ALERT)) {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        }
        break;
#if TCFG_AUDIO_ANC_ENABLE
    case KWS_EVENT_ANC_ON:
        anc_mode_switch(ANC_ON, 1);
        break;
    case KWS_EVENT_TRANSARENT_ON:
        anc_mode_switch(ANC_TRANSPARENCY, 1);
        break;
    case KWS_EVENT_ANC_OFF:
        anc_mode_switch(ANC_OFF, 1);
        break;
#endif

    case KWS_EVENT_NULL:
        log_info("KWS_EVENT_NULL");
        break;

    default:
        break;
    }

    return true;
}

#else /* #if ((defined TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE) && (TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE)) */

int jl_kws_voice_event_handle(struct key_event *key)
{
    return false;
}

#endif /* #if ((defined TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE) && (TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE)) */
