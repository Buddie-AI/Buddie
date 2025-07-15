#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "le_rcsp_adv_module.h"

#include "adv_work_setting.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"
#include "os/os_api.h"
#include "btstack/avctp_user.h"

#if (RCSP_ADV_EN)
#if RCSP_ADV_WORK_SET_ENABLE

extern int get_bt_tws_connect_status();
extern void bt_set_low_latency_mode(int enable);
void set_work_setting(u8 work_setting_info)
{
    _s_info.work_mode =	work_setting_info;
}

u8 get_work_setting(void)
{
    return _s_info.work_mode;
}

static void adv_work_setting_vm_value(u8 work_setting_info)
{
    /* syscfg_write(CFG_RCSP_ADV_WORK_SETTING, &work_setting_info, 1); */
}

static void adv_work_setting_sync(u8 work_setting_info)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_WORK_MODE));
    }
#endif
}

static void __rcsp_work_set_call(void)
{
    //r_f_printf("__rcsp_work_set_call\n");
    if (1 == _s_info.work_mode) {
        bt_set_low_latency_mode(0);
    } else if (2 == _s_info.work_mode) {
        bt_set_low_latency_mode(1);
    }
}

static void rcsp_work_set_deal(void)
{
    int err;
    int msg[3];
    msg[0] = (int) __rcsp_work_set_call;
    msg[1] = 1;
    msg[2] = 0;

    err = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
    //r_printf("err %x\n",err);


}

static void update_work_setting_state(void)
{
    //r_f_printf("set deal\n");
    rcsp_work_set_deal();
}

void deal_work_setting(u8 work_setting_info, u8 write_vm, u8 tws_sync, u8 poweron)
{
    if (poweron) {
        __rcsp_work_set_call();
    } else {
        if (0 == work_setting_info) {
            work_setting_info = get_work_setting();
        } else {
            set_work_setting(work_setting_info);
        }
        if (write_vm) {
            adv_work_setting_vm_value(work_setting_info);
        }
        if (tws_sync) {
            adv_work_setting_sync(work_setting_info);
        }
        update_work_setting_state();
    }
}


int ble_vendor_is_slow_state(void)
{
    if ((BT_STATUS_PLAYING_MUSIC == get_bt_connect_status()) && (2 == _s_info.work_mode)) {
        return 1;
    } else {
        return 0;
    }
}


#endif
#endif
