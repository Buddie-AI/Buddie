#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "key_event_deal.h"
#include "le_rcsp_adv_module.h"

#include "adv_anc_voice_key.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"

#include "adv_anc_voice.h"

#if (RCSP_ADV_EN)
/* #if (RCSP_ADV_KEY_SET_ENABLE && TCFG_AUDIO_ANC_ENABLE) */
#if (RCSP_ADV_KEY_SET_ENABLE)

#include "audio_anc.h"

#define COMMON_FUNCTION    		0xFF
#define ANC_VOICE_TYPE_MAX		3

static u8 g_anc_voice_key_mode[4] = {0, 0, 0, BIT(ANC_VOICE_TYPE_MAX) - 1};

extern int get_bt_tws_connect_status();

void set_anc_voice_key_mode(u8 *anc_voice_mode)
{
    memcpy(g_anc_voice_key_mode, anc_voice_mode, sizeof(g_anc_voice_key_mode));
}

void get_anc_voice_key_mode(u8 *anc_voice_mode)
{
    memcpy(anc_voice_mode, g_anc_voice_key_mode, sizeof(g_anc_voice_key_mode));
}

static void update_anc_voice_key_vm_value(u8 *anc_voice_mode)
{
    syscfg_write(CFG_RCSP_ADV_ANC_VOICE_KEY, anc_voice_mode, 4);
}

static void anc_voice_key_sync(u8 *key_setting_info)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_ANC_VOICE_KEY));
    }
#endif
}

void deal_anc_voice_key_setting(u8 *anc_key_setting, u8 write_vm, u8 tws_sync)
{
    u8 anc_voice_key_mode[4] = {0};
    if (!anc_key_setting) {
        get_anc_voice_key_mode(anc_voice_key_mode);
    } else {
        set_anc_voice_key_mode(anc_key_setting);
        memcpy(anc_voice_key_mode, g_anc_voice_key_mode, sizeof(g_anc_voice_key_mode));
    }
    if (write_vm) {
        update_anc_voice_key_vm_value(anc_voice_key_mode);
    }
    if (tws_sync) {
        anc_voice_key_sync(anc_voice_key_mode);
    }
}

int update_anc_voice_key_opt(void)
{
    int ret = 0;
#if (RCSP_ADV_ANC_VOICE)
    u32 mask = g_anc_voice_key_mode[0] << 24 | g_anc_voice_key_mode[1] << 16 | g_anc_voice_key_mode[2] << 8 | g_anc_voice_key_mode[3];
    // 获取当前处于什么模式
#if TCFG_AUDIO_ANC_ENABLE
    u8 cur_mode = anc_mode_get();
#else
    u8 cur_mode = ANC_OFF;
#endif
    u8 next_mode = cur_mode % ANC_VOICE_TYPE_MAX;
    for (; next_mode < ANC_VOICE_TYPE_MAX; next_mode++) {
        if (mask & BIT(next_mode)) {
            rcsp_adv_voice_mode_update(next_mode % ANC_VOICE_TYPE_MAX);
            return ret;
        }
    }
    if (mask & BIT(next_mode % ANC_VOICE_TYPE_MAX)) {
        rcsp_adv_voice_mode_update(next_mode % ANC_VOICE_TYPE_MAX);
    }
#endif
    return ret;
}

#endif
#endif
