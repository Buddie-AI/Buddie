#include "jl_kws_common.h"
#include "btstack/avctp_user.h"
#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif /* #if TCFG_USER_TWS_ENABLE */

#if TCFG_KWS_VOICE_RECOGNITION_ENABLE

//=========================================================//
//                      KWS 事件处理                       //
//=========================================================//

struct jl_kws_event {
    u32 last_event_jiffies;
    u8 last_event;
};

static struct jl_kws_event __kws_event = {0};

static void kws_event_handle(u8 voice_event)
{
    u32 cur_jiffies = jiffies;

    if (voice_event == __kws_event.last_event) {
        if (jiffies_to_msecs(cur_jiffies - __kws_event.last_event_jiffies) < 1000) {
            kws_info("voice event %d same, ignore", voice_event);
            __kws_event.last_event_jiffies = cur_jiffies;
            return;
        }
    }
    __kws_event.last_event_jiffies = cur_jiffies;
    __kws_event.last_event = voice_event;

    kws_info("%s: %d", __func__, voice_event);

    switch (voice_event) {
    case KWS_VOICE_EVENT_YES:
        kws_info("send ANSWER cmd");
        user_send_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
        break;
    case KWS_VOICE_EVENT_NO:
        kws_info("send HANGUP cmd");
        user_send_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
        break;
    default:
        break;
    }

    return;
}

//==========================================================//
// 				       TWS 消息同步                         //
//==========================================================//
#if TCFG_USER_TWS_ENABLE
extern bool get_tws_sibling_connect_state(void);

#define TWS_FUNC_ID_KWS_EVENT_SYNC	TWS_FUNC_ID('K', 'W', 'S', 'V')

static void kws_event_sync_tws_state_deal(void *_data, u16 len, bool rx)
{
    u8 *data = (u8 *)_data;
    u8 voice_event = data[0];
    kws_info("tws event rx sync: %d", voice_event);
    kws_event_handle(voice_event);
}

static void kws_sync_tws_event(u8 voice_event)
{
    if (get_tws_sibling_connect_state() == TRUE) {
        tws_api_send_data_to_sibling(&voice_event, 1, TWS_FUNC_ID_KWS_EVENT_SYNC);
    }
}

REGISTER_TWS_FUNC_STUB(kws_voice_event_sync) = {
    .func_id = TWS_FUNC_ID_KWS_EVENT_SYNC,
    .func    = kws_event_sync_tws_state_deal,
};
#endif /* #if TCFG_USER_TWS_ENABLE */

//==========================================================//
// 				    JL_KWS EVENT API                        //
//==========================================================//
int jl_kws_event_init(void)
{
    return JL_KWS_ERR_NONE;
}

void jl_kws_event_stop(void)
{
    return;
}


void jl_kws_event_close(void)
{
    return;
}

void jl_kws_event_state_update(u8 voice_event)
{
#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state() == TRUE) {
        kws_sync_tws_event(voice_event);
    } else
#endif /* #if TCFG_USER_TWS_ENABLE */
    {
        kws_event_handle(voice_event);
    }
}


#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
