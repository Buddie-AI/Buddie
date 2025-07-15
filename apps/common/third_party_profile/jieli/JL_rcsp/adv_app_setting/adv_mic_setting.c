#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "le_rcsp_adv_module.h"

#include "adv_mic_setting.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"

#if (RCSP_ADV_EN)
#if RCSP_ADV_MIC_SET_ENABLE

extern void test_esco_role_switch(u8 flag);
extern void tws_conn_switch_role();
extern int get_bt_tws_connect_status();
static void mic_setting_info_deal(u8 mic_info_data)
{
#if TCFG_USER_TWS_ENABLE
    u8 channel = tws_api_get_local_channel();
    printf("%s, %d\n", __FUNCTION__, channel);

    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        printf("-----master-----\n");
    } else {
        printf("-----role-----\n");
    }
    switch (mic_info_data) {
    case  0x01:
        if (get_bt_tws_connect_status()) {
            printf("-------auto--------\n");
            tws_api_auto_role_switch_enable();
        }
        break;
    case  0x02:
        if (get_bt_tws_connect_status()) {
            if (channel != 'L') {
                printf("-------!L--------\n");
                tws_api_auto_role_switch_disable();
                tws_conn_switch_role();
            }
        }
        break;
    case  0x03:
        if (get_bt_tws_connect_status()) {
            if (channel != 'R') {
                printf("-------!R--------\n");
                tws_api_auto_role_switch_disable();
                tws_conn_switch_role();
            }
        }
        break;
    default:
        break;
    }
#endif
}

void set_mic_setting(u8 mic_setting_info)
{
    _s_info.mic_mode = 	mic_setting_info;
}

u8 get_mic_setting(void)
{
    return _s_info.mic_mode;
}

static void update_mic_setting_vm_value(u8 mic_setting_info)
{
    syscfg_write(CFG_RCSP_ADV_MIC_SETTING, &mic_setting_info, 1);
}

static void adv_mic_setting_sync(u8 mic_setting_info)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_MIC_SETTING));
    }
#endif
}

void deal_mic_setting(u8 mic_setting_info, u8 write_vm, u8 tws_sync)
{
    if (!mic_setting_info) {
        mic_setting_info = get_mic_setting();
    } else {
        set_mic_setting(mic_setting_info);
    }
    if (write_vm) {
        update_mic_setting_vm_value(mic_setting_info);
    }
    if (tws_sync) {
        adv_mic_setting_sync(mic_setting_info);
    }
    mic_setting_info_deal(mic_setting_info);
}


//通话时,固定mic的位置,mode--esco state
void rcsp_user_mic_fixed_deal(u8 mode)
{
    u8 channel = tws_api_get_local_channel();

    if (!get_bt_tws_connect_status()) {
        return;
    }

    if (mode == 0) {
        test_esco_role_switch(0);
        return;
    }

    if ((tws_api_get_role() == TWS_ROLE_MASTER)
        && (tws_api_get_tws_state() | TWS_STA_ESCO_OPEN)) {

        switch (get_mic_setting()) {
        case 0x02:
            if (get_bt_tws_connect_status()) {
                if (channel != 'L') {
                    printf("mic_sw_l\n");
                    test_esco_role_switch(1);
                }
            }
            break;

        case 0x03:
            if (get_bt_tws_connect_status()) {
                if (channel != 'R') {
                    printf("mic_sw_r\n");
                    test_esco_role_switch(1);

                }
            }

            break;

        default:
            printf("mic_sw_auto\n");
            test_esco_role_switch(0);
            break;
        }
    }
}

#else

void rcsp_user_mic_fixed_deal(u8 mode)
{
    return;
}

#endif

#endif
