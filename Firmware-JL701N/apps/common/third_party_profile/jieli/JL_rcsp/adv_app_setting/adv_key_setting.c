#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "key_event_deal.h"
#include "le_rcsp_adv_module.h"

#include "adv_key_setting.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"

#include "adv_anc_voice_key.h"

#if (RCSP_ADV_EN)
#if RCSP_ADV_KEY_SET_ENABLE

extern int get_bt_tws_connect_status();
static u8 rcsp_adv_key_event_flag = 0x0;
static void enable_adv_key_event(void)
{
    rcsp_adv_key_event_flag = 1;
}

static u8 get_adv_key_event_status(void)
{
    return rcsp_adv_key_event_flag;
}

static void disable_adv_key_event(void)
{
    rcsp_adv_key_event_flag = 0;
}

void set_key_setting(u8 *key_setting_info)
{
    _s_info.key_setting[3 * 0 + 2] = key_setting_info[0];
    _s_info.key_setting[3 * 2 + 2] = key_setting_info[1];
    _s_info.key_setting[3 * 1 + 2] = key_setting_info[2];
    _s_info.key_setting[3 * 3 + 2] = key_setting_info[3];
}

void get_key_setting(u8 *key_setting_info)
{
    key_setting_info[0] = _s_info.key_setting[3 * 0 + 2];
    key_setting_info[1] = _s_info.key_setting[3 * 2 + 2];
    key_setting_info[2] = _s_info.key_setting[3 * 1 + 2];
    key_setting_info[3] = _s_info.key_setting[3 * 3 + 2];
}

static void update_key_setting_vm_value(u8 *key_setting_info)
{
    syscfg_write(CFG_RCSP_ADV_KEY_SETTING, key_setting_info, 4);
}

static void key_setting_sync(u8 *key_setting_info)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_KEY_SETTING));
    }
#endif
}

void deal_key_setting(u8 *key_setting_info, u8 write_vm, u8 tws_sync)
{
    u8 key_setting[4];
    if (!key_setting_info) {
        get_key_setting(key_setting);
    } else {
        memcpy(key_setting, key_setting_info, 4);
    }
    // 1、写入VM
    if (write_vm) {
        update_key_setting_vm_value(key_setting);
    }
    // 2、同步对端
    if (tws_sync) {
        key_setting_sync(key_setting);
    }
    // 3、更新状态
    enable_adv_key_event();
}

#define ADV_POWER_ON_OFF	0
enum ADV_KEY_TYPE {
    ADV_KEY_TYPE_NULL = 0x0,
#if ADV_POWER_ON_OFF
    ADV_KEY_TYPE_POWER_ON,
    ADV_KEY_TYPE_POWER_OFF,
#endif
    ADV_KEY_TYPE_PREV = 0x3,
    ADV_KEY_TYPE_NEXT,
    ADV_KEY_TYPE_PP,
    ADV_KEY_TYPE_ANSWER_CALL,
    ADV_KEY_TYPE_HANG_UP,
    ADV_KEY_TYPE_CALL_BACK,
    ADV_KEY_TYPE_INC_VOICE,
    ADV_KEY_TYPE_DESC_VOICE,
    ADV_KEY_TYPE_TAKE_PHOTO,
    ADV_KEY_TYPE_ANC_VOICE = 0xFF,
};

static u8 get_adv_key_opt(u8 key_action, u8 channel)
{
    u8 opt;
    for (opt = 0; opt < sizeof(_s_info.key_setting); opt += 3) {
        if (_s_info.key_setting[opt] == channel &&
            _s_info.key_setting[opt + 1] == key_action) {
            break;
        }
    }
    if (sizeof(_s_info.key_setting) == opt) {
        return -1;
    }
    switch (_s_info.key_setting[opt + 2]) {
    case ADV_KEY_TYPE_NULL:
        //opt = KEY_NULL;
        opt = ADV_KEY_TYPE_NULL;
        break;
#if ADV_POWER_ON_OFF
    case ADV_KEY_TYPE_POWER_ON:
        opt = KEY_POWER_ON;
        break;
    case ADV_KEY_TYPE_POWER_OFF:
        opt = KEY_RCSP_POWEROFF;
        break;
#endif
    case ADV_KEY_TYPE_PREV:
        opt = KEY_MUSIC_PREV;
        break;
    case ADV_KEY_TYPE_NEXT:
        opt = KEY_MUSIC_NEXT;
        break;
    case ADV_KEY_TYPE_PP:
        opt = KEY_MUSIC_PP;
        break;
    case ADV_KEY_TYPE_ANSWER_CALL:
        opt = KEY_CALL_ANSWER;
        break;
    case ADV_KEY_TYPE_HANG_UP:
        opt = KEY_CALL_HANG_UP;
        break;
    case ADV_KEY_TYPE_CALL_BACK:
        opt = KEY_CALL_LAST_NO;
        break;
    case ADV_KEY_TYPE_INC_VOICE:
        opt = KEY_VOL_UP;
        break;
    case ADV_KEY_TYPE_DESC_VOICE:
        opt = KEY_VOL_DOWN;
        break;
    case ADV_KEY_TYPE_TAKE_PHOTO:
        opt = KEY_HID_CONTROL;
        break;
    case ADV_KEY_TYPE_ANC_VOICE:
        update_anc_voice_key_opt();
        opt = KEY_NULL;
        break;
    }
    return opt;
}

void set_key_event_by_rcsp_info(struct sys_event *event, u8 *key_event)
{
    if (0 == get_adv_key_event_status()) {
        return;
    }
    u8 key_action = 0;
    struct key_event *key = &event->u.key;
    switch (key->event) {
    case 0:
        // 单击
        key_action = 0x1;
        break;
    case 4:
        // 双击
        key_action = 0x2;
        break;
    default:
        return;
    }

#if (TCFG_CHARGESTORE_ENABLE && TCFG_USER_TWS_ENABLE)
    u8 channel = tws_api_get_local_channel();

    if (get_bt_tws_connect_status()) {
        channel = tws_api_get_local_channel();
    }
#else
    u8 channel = 'U';
#endif

    switch (channel) {
    case 'U':
    case 'L':
        channel = (u32) event->arg == KEY_EVENT_FROM_TWS ? 2 : 1;
        break;
    case 'R':
        channel = (u32) event->arg == KEY_EVENT_FROM_TWS ? 1 : 2;
        break;
    default:
        return;
    }

    key_action = get_adv_key_opt(key_action, channel);
    if ((u8) - 1 != key_action) {
        *key_event = key_action ? key_action : KEY_NULL;
    }
}
#else


void set_key_event_by_rcsp_info(struct sys_event *event, u8 *key_event)
{
    return;
}

#endif
#endif
