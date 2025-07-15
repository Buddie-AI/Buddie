#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "adv_high_low_vol.h"

#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"

#if (RCSP_ADV_EN)
#if RCSP_ADV_HIGH_LOW_SET

static struct _HIGL_LOW_VOL high_low_vol = {
    .low_vol  = 12,
    .high_vol = 12,
};

extern int get_bt_tws_connect_status();
extern int audio_out_eq_spec_set_gain(u8 idx, int gain);

void get_high_low_vol_info(u8 *vol_gain_param)
{
    memcpy(vol_gain_param, &high_low_vol, sizeof(struct _HIGL_LOW_VOL));
}

void set_high_low_vol_info(u8 *vol_gain_param)
{
    memcpy(&high_low_vol, vol_gain_param, sizeof(struct _HIGL_LOW_VOL));
}

static void update_high_low_vol_vm_value(u8 *vol_gain_param)
{
    syscfg_write(CFG_RCSP_ADV_HIGH_LOW_VOL, vol_gain_param, sizeof(struct _HIGL_LOW_VOL));
}

static void high_low_vol_setting_sync(u8 *vol_gain_param)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_HIGH_LOW_VOL));
    }
#endif
}

static void high_low_vol_state_update(void)
{
#if (TCFG_AUDIO_OUT_EQ_ENABLE != 0)
    static int low_vol = 0;
    static int high_vol = 0;
    if (low_vol != high_low_vol.low_vol) {
        audio_out_eq_spec_set_gain(0, high_low_vol.low_vol - 12);
        low_vol = high_low_vol.low_vol;
    }
    if (high_vol != high_low_vol.high_vol) {
        audio_out_eq_spec_set_gain(1, high_low_vol.high_vol - 12);
        high_vol = high_low_vol.high_vol;
    }
#endif
}

void deal_high_low_vol(u8 *vol_gain_data, u8 write_vm, u8 tws_sync)
{
    int vol_gain_param[2] = {0};
    if (!vol_gain_data) {
        get_high_low_vol_info((u8 *)vol_gain_param);
    } else {
        memcpy((u8 *)&vol_gain_param, vol_gain_data, sizeof(vol_gain_param));
        set_high_low_vol_info(vol_gain_data);
        printf("low %d, high %d\n", vol_gain_param[0], vol_gain_param[1]);
    }
    if (write_vm) {
        update_high_low_vol_vm_value((u8 *)vol_gain_param);
    }
    if (tws_sync) {
        high_low_vol_setting_sync((u8 *)vol_gain_param);
    }
    high_low_vol_state_update();
}

#endif
#endif
