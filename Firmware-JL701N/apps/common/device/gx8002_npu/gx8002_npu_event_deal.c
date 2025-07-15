#include "includes.h"
#include "app_config.h"
#include "gx8002_npu.h"
#include "gx8002_npu_api.h"
#include "btstack/avctp_user.h"
#include "tone_player.h"

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif /* #if TCFG_USER_TWS_ENABLE */

#if TCFG_GX8002_NPU_ENABLE

#define gx8002_event_info 	printf

bool get_tws_sibling_connect_state(void);

#define TWS_FUNC_ID_VOICE_EVENT_SYNC	TWS_FUNC_ID('V', 'O', 'I', 'C')

struct gx8002_event_hdl {
    u8 last_event;
    u32 last_event_jiffies;
};

static struct gx8002_event_hdl hdl = {0};

#define __this 		(&hdl)

__attribute__((weak))
void volume_down(u8 dec)
{
    return;
}

__attribute__((weak))
void volume_up(u8 inc)
{
    return;
}

__attribute__((weak))
int app_case_voice_event_handle(u8 voice_event)
{
    return 0;
}

static void gx8002_recognize_tone_play(void)
{
    if (get_tws_sibling_connect_state() == TRUE) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            bt_tws_play_tone_at_same_time(SYNC_TONE_VOICE_RECOGNIZE, 200);
        }
    } else {
        tone_play(TONE_NORMAL, 1);
    }
}

static void gx8002_event_handle(u8 voice_event)
{
    u32 cur_jiffies = jiffies;
    u8 a2dp_state;
    u8 call_state;

    if (voice_event == __this->last_event) {
        if (jiffies_to_msecs(cur_jiffies - __this->last_event_jiffies) < 1000) {
            gx8002_event_info("voice event %d same, ignore", voice_event);
            __this->last_event_jiffies = cur_jiffies;
            return;
        }
    }
    __this->last_event_jiffies = cur_jiffies;
    __this->last_event = voice_event;

    gx8002_event_info("%s: %d", __func__, voice_event);

    if (app_case_voice_event_handle(voice_event)) {
        return;
    }

    //播放提示音
    if ((voice_event != VOLUME_UP_VOICE_EVENT) && (voice_event != VOLUME_DOWN_VOICE_EVENT)) {
        //gx8002_recognize_tone_play();
    }

    switch (voice_event) {
    case MAIN_WAKEUP_VOICE_EVENT:
        gx8002_event_info("send SIRI cmd");
        user_send_cmd_prepare(USER_CTRL_HFP_GET_SIRI_OPEN, 0, NULL);
        break;
    case MUSIC_PAUSE_VOICE_EVENT:
    case MUSIC_STOP_VOICE_EVENT:
    case MUSIC_PLAY_VOICE_EVENT:
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
                if (voice_event == MUSIC_PAUSE_VOICE_EVENT) {
                    gx8002_event_info("send PAUSE cmd");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
                } else if (voice_event == MUSIC_STOP_VOICE_EVENT) {
                    gx8002_event_info("send STOP cmd");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_STOP, 0, NULL);
                }
            } else {
                if (voice_event == MUSIC_PLAY_VOICE_EVENT) {
                    gx8002_event_info("send PLAY cmd");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
                }
            }
        }
        break;
    case VOLUME_UP_VOICE_EVENT:
        gx8002_event_info("volume up");
        volume_up(4); //music: 0 ~ 16, call: 0 ~ 15, step: 25%
        //gx8002_recognize_tone_play();
        break;
    case VOLUME_DOWN_VOICE_EVENT:
        gx8002_event_info("volume down");
        volume_down(4); //music: 0 ~ 16, call: 0 ~ 15, step: 25%
        //gx8002_recognize_tone_play();
        break;
    case MUSIC_PREV_VOICE_EVENT:
        gx8002_event_info("volume PREV cmd");
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PREV, 0, NULL);
        break;
    case MUSIC_NEXT_VOICE_EVENT:
        gx8002_event_info("volume NEXT cmd");
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_NEXT, 0, NULL);
        break;
    case CALL_ANSWER_VOICE_EVENT:
        if (get_call_status() == BT_CALL_INCOMING) {
            gx8002_event_info("volume ANSWER cmd");
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        }
        break;
    case CALL_HANG_UP_VOICE_EVENT:
        gx8002_event_info("volume HANG UP cmd");
        if ((get_call_status() >= BT_CALL_INCOMING) && (get_call_status() <= BT_CALL_ALERT)) {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        }
        break;
    default:
        break;
    }
}

static void gx8002_event_sync_tws_state_deal(void *_data, u16 len, bool rx)
{
    u8 *data = (u8 *)_data;
    u8 voice_event = data[0];
    //if (rx) {
    gx8002_event_info("tws event rx sync: %d", voice_event);
    //gx8002_event_handle(voice_event);
    gx8002_voice_event_post_msg(voice_event);
    //}
}

static void gx8002_sync_tws_event(u8 voice_event)
{
    if (get_tws_sibling_connect_state() == TRUE) {
        tws_api_send_data_to_sibling(&voice_event, 1, TWS_FUNC_ID_VOICE_EVENT_SYNC);
    }
}

void gx8002_event_state_update(u8 voice_event)
{
    //tone_play(TONE_NORMAL, 1);
    if (get_tws_sibling_connect_state() == TRUE) {
        gx8002_sync_tws_event(voice_event);
    } else {
        gx8002_event_handle(voice_event);
    }
}


void gx8002_voice_event_handle(u8 voice_event)
{
    gx8002_event_handle(voice_event);
}

REGISTER_TWS_FUNC_STUB(gx8002_voice_event_sync) = {
    .func_id = TWS_FUNC_ID_VOICE_EVENT_SYNC,
    .func    = gx8002_event_sync_tws_state_deal,
};

#endif /* #if TCFG_GX8002_NPU_ENABLE */
