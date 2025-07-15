#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "rcsp_adv_bluetooth.h"
#include "JL_rcsp_protocol.h"
#include "le_rcsp_adv_module.h"

#include "adv_time_stamp_setting.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"

#if (RCSP_ADV_EN)
extern int get_bt_tws_connect_status();
static u32 adv_time_stamp = 0;
u32 get_adv_time_stamp(void)
{
    return adv_time_stamp;
}

void set_adv_time_stamp(u32 time_stamp)
{
    adv_time_stamp = time_stamp;
}

void sync_setting_by_time_stamp(void)
{
    tws_api_send_data_to_sibling((u8 *)&adv_time_stamp, sizeof(adv_time_stamp), TWS_FUNC_ID_TIME_STAMP_SYNC);
}

void deal_sibling_time_stamp_setting_switch(void *data, u16 len)
{
    u32 time_stamp = *((u32 *)data);
    if (adv_time_stamp > time_stamp) {
        JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_ADV_SETTING_SYNC, NULL, 0);
    }
}

void deal_adv_setting_gain_time_stamp(void)
{
#if TCFG_USER_TWS_ENABLE
    extern u8 adv_info_device_request(u8 * buf, u16 len);
    static u8 tws_pair = 0;
    u8 buf = 2;
    u8 ret = 0;
    if (get_bt_tws_connect_status()) {
        tws_pair = 1;
    } else {
        if (tws_pair) {
            tws_pair = 0;
            ret = adv_info_device_request(&buf, sizeof(buf));
        }
    }
#endif
}

// 1、写入VM
static void update_time_stamp_vm_value(u32 time_stamp)
{
    syscfg_write(CFG_RCSP_ADV_TIME_STAMP, (u8 *)&time_stamp, 4);
}
// 2、同步对端
static void adv_time_stamp_sync(u32 time_stamp)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_TIME_STAMP));
    }
#endif
}

void deal_time_stamp_setting(u32 time_stamp, u8 write_vm, u8 tws_sync)
{
    if (!time_stamp) {
        time_stamp = get_adv_time_stamp();
    } else {
        set_adv_time_stamp(time_stamp);
    }
    if (write_vm) {
        update_time_stamp_vm_value(time_stamp);
    }
    if (tws_sync) {
        adv_time_stamp_sync(time_stamp);
    }
}

#endif
