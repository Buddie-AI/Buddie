#include "default_event_handler.h"
#include "app_action.h"
#include "bt_background.h"
#include "earphone.h"
#include "dev_manager/dev_manager.h"
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "bt_tws.h"
#include "app_main.h"
#include "app_music.h"
#include "key_event_deal.h"
#include "asm/charge.h"
#include "app_sd_music.h"
#include "usb/otg.h"
#include "update.h"

#if (TCFG_APP_MUSIC_EN || TCFG_PC_ENABLE)
#include "app_task.h"

#if TCFG_DEC2TWS_ENABLE
#include "audio_dec/audio_dec_file.h"
#endif

extern u8 key_table[KEY_NUM_MAX][KEY_EVENT_MAX];
extern int pc_device_event_handler(struct sys_event *event);
extern u32 timer_get_ms(void);
extern u16 dev_update_check(char *logo);

int switch_to_music_app(int action)
{
    struct intent it;

    if (action == ACTION_MUSIC_MAIN) {
        /* 卡不在线不切到music模式,
         * 防止TWS之间所处的模式不同导致A2DP播歌不同步
         */
#if TCFG_DEV_MANAGER_ENABLE
        if (!music_app_check()) {
            return -ENODEV;
        }
#else
        if (!dev_online("sd0")) {
            return -ENODEV;
        }
#endif
    }
    init_intent(&it);
    it.name = "music";
    it.action = action;
    return start_app(&it);
}

void switch_to_prev_app()
{
    struct intent it;

    init_intent(&it);
    it.action = ACTION_BACK;
    start_app(&it);
}

void switch_to_earphone_app()
{
    struct intent it;

    init_intent(&it);
    it.action = ACTION_BACK;
    start_app(&it);
}

void switch_to_pc_app()
{
    struct intent it;

    /* 退出bt和music模式 */
    if (bt_in_background()) {
        init_intent(&it);
        it.action = ACTION_BACK;
        start_app(&it);
    }
    init_intent(&it);
    it.action = ACTION_BACK;
    start_app(&it);


    /* 切到PC模式 */
    init_intent(&it);
    it.name = "pc";
    it.action = ACTION_PC_MAIN;
    start_app(&it);
}

static void sd_event_handler(struct device_event *evt)
{
    //add 设备的返回值, 0成功
    int err = 0;
    if (evt->event == DEVICE_EVENT_IN) {
        if (app_var.have_mass_storage == 0) {
            app_var.have_mass_storage = 1;
            syscfg_write(CFG_HAVE_MASS_STORAGE, &app_var.have_mass_storage, 1);
        }
#if TCFG_DEC2TWS_ENABLE
        if (bt_in_background()) {
            switch_to_music_app(ACTION_MUSIC_MAIN);
        }
#elif TWFG_APP_POWERON_IGNORE_DEV
        err = dev_manager_add((char *)evt->value);
        if ((err == 0) && (!get_charge_online_flag()) && (usb_otg_online(0) != SLAVE_MODE)) {	//充电状态只add设备，不切模式
            ///检查设备升级
            if (dev_update_check((char *)evt->value) == UPDATA_READY) {
                return;
            }
#if TCFG_APP_MUSIC_EN
            if ((timer_get_ms() - app_var.start_time) > TWFG_APP_POWERON_IGNORE_DEV) {
                app_task_switch_to(APP_MUSIC_TASK, NULL_VALUE);
            }
#endif
        }

#else
        err = dev_manager_add((char *)evt->value);
        if ((err == 0) && (!get_charge_online_flag()) && (usb_otg_online(0) != SLAVE_MODE)) {	//充电状态只add设备，不切模式
            ///检查设备升级
            if (dev_update_check((char *)evt->value) == UPDATA_READY) {
                return;
            }
#if TCFG_APP_MUSIC_EN
            app_task_switch_to(APP_MUSIC_TASK, NULL_VALUE);
#endif
        }
#endif
    } else if (evt->event == DEVICE_EVENT_OUT) {
        update_clear_result();
#if (!TCFG_DEC2TWS_ENABLE)
        dev_manager_del((char *)evt->value);
#endif
    }
}

static int default_earphone_key_event_handler(struct sys_event *event)
{
    struct key_event *key = &event->u.key;
    u8 key_event = key_table[key->value][key->event];

    switch (key_event) {
    case KEY_MODE_SWITCH:
        puts("key_mode_switch\n");
        app_task_switch_next();
        break;
    }

    return false;
}

#if ((TCFG_APP_MUSIC_EN || TCFG_APP_PC_EN) && TCFG_DEC2TWS_ENABLE)
void tws_loacl_media_channe_start(struct bt_event *evt)
{
    int role = tws_api_get_role();
    int state = tws_api_get_tws_state();

    if (role == TWS_ROLE_MASTER) {
        if (state & TWS_STA_PHONE_CONNECTED) {
            tws_conn_switch_role();
        }
        if (state & (TWS_STA_ESCO_OPEN | TWS_STA_SBC_OPEN)) {
            return;
        }
    }

    if (file_dec_get_source() == FILE_FROM_LOCAL) {
        if (role == TWS_ROLE_MASTER) {
            return;
        }
        app_music_exit();
    }

    switch_to_music_app(ACTION_MUSIC_TWS_RX);
    tws_local_media_dec_open(evt->args[0], evt->args + 2);
    sys_auto_shut_down_disable();
}

void tws_loacl_media_channe_stop(struct bt_event *evt)
{
    tws_local_media_dec_close(evt->args[0]);

    sys_auto_shut_down_enable();

    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        struct application *app = get_current_app();
        if (app && app->action != ACTION_PC_MAIN) {
            bt_switch_to_foreground(ACTION_DO_NOTHING, 1);
        }
    }
}

static int default_tws_event_handler(struct bt_event *evt)
{
#if TCFG_DEC2TWS_ENABLE
    int role = evt->args[0];
    int phone_link_connection = evt->args[1];
    int state = evt->args[2];

    switch (evt->event) {
    case TWS_EVENT_CONNECTED:
        tws_api_auto_role_switch_disable();
        if (state & (TWS_STA_ESCO_OPEN | TWS_STA_SBC_OPEN)) {
            if (bt_in_background()) {
                bt_switch_to_foreground(ACTION_DO_NOTHING, 1);
            }
            break;
        }
        if (file_dec_get_source() == FILE_FROM_LOCAL) {
            send_local_media_dec_open_cmd();
        } else {
            if (role == TWS_ROLE_MASTER) {
                if (switch_to_music_app(ACTION_MUSIC_MAIN) == 0) {
                    break;
                }
            }
            if (bt_in_background()) {
                bt_switch_to_foreground(ACTION_DO_NOTHING, 1);
            }
        }
        break;
    case TWS_EVENT_CONNECTION_TIMEOUT:
        if (file_dec_get_source() == -EINVAL) {
            switch_to_music_app(ACTION_MUSIC_MAIN);
        }
        break;
    case TWS_EVENT_MONITOR_START:
        if (role == TWS_ROLE_MASTER) {
            if (!app_var.have_mass_storage || file_dec_get_source() == FILE_FROM_TWS) {
                tws_conn_switch_role();
            }
        }
        break;
    case TWS_EVENT_CONNECTION_DETACH:
        break;
    case TWS_EVENT_DATA_TRANS_OPEN:
        printf("TWS_EVENT_DATA_TRANS_OPEN: %d, %d\n", evt->args[0], evt->args[1]);
        break;
    case TWS_EVENT_DATA_TRANS_START:
        printf("TWS_EVENT_DATA_TRANS_START: %d, %d\n", evt->args[0], evt->args[1]);
        if (evt->args[1] == TWS_DTC_LOCAL_MEDIA) {
            tws_loacl_media_channe_start(evt);
        }
        break;
    case TWS_EVENT_DATA_TRANS_STOP:
        printf("TWS_EVENT_DATA_TRANS_STOP: %d, %d\n", evt->args[0], evt->args[1]);
        if (evt->args[1] == TWS_DTC_LOCAL_MEDIA) {
            tws_loacl_media_channe_stop(evt);
        }
        break;
    case TWS_EVENT_DATA_TRANS_CLOSE:
        break;
    default:
        return false;
    }

    return true;
#endif
}
#endif/* #if ((TCFG_APP_MUSIC_EN || TCFG_APP_PC_EN) && TCFG_DEC2TWS_ENABLE) */

void default_event_handler(struct sys_event *event)
{
    printf("@@@default_event_handler event->type = %d\n", event->type);
    if (bt_in_background()) {
        bt_in_background_event_handler(event);
    }
    switch (event->type) {
    case SYS_KEY_EVENT:
        if (!bt_in_background()) {
            default_earphone_key_event_handler(event);
        }
        break;
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            struct bt_event *bt = &event->u.bt;
            if (bt->event == BT_STATUS_PHONE_HANGUP || (bt->event == BT_STATUS_VOICE_RECOGNITION && !bt->value)) {
                r_printf("BT_STATUS_SCO_STATUS_CHANGE");
                /* if (bt->value == 0xff)  */
                {
                    app_core_back_to_prev_app();
                    if (!app_core_back_to_prev_app_over_check()) {
                        app_task_switch_to(APP_BT_TASK, ACTION_DO_NOTHING);
                    }
                }
            }
        }
#if TCFG_USER_TWS_ENABLE && TCFG_DEC2TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            default_tws_event_handler(&event->u.bt);
        }
#endif
        break;

    case SYS_DEVICE_EVENT:
        /* 系统设备事件处理 */
#if (TCFG_ONLINE_ENABLE || TCFG_CFG_TOOL_ENABLE)
        if ((u32)event->arg == DEVICE_EVENT_FROM_CFG_TOOL) {
            extern int app_cfg_tool_event_handler(struct cfg_tool_event * cfg_tool_dev);
            app_cfg_tool_event_handler(&event->u.cfg_tool);
            break;
        }
#endif

#if TCFG_APP_MUSIC_EN
        if ((u32)event->arg == DRIVER_EVENT_FROM_SD0) {
            sd_event_handler(&event->u.dev);
            break;
        }
#endif

#if TCFG_PC_ENABLE
        int ret = pc_device_event_handler(event);
        if (ret) {
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                app_task_switch_to(APP_PC_TASK, NULL_VALUE);
            }
        }
#endif
        break;
    default:
        break;
    }

}

#else

void default_event_handler(struct sys_event *event)
{
    /*puts("default_event_handler\n");*/
    struct intent it;
    switch (event->type) {
    case SYS_DEVICE_EVENT:
        /* 系统设备事件处理 */
#if TCFG_PC_ENABLE
        extern int pc_device_event_handler(struct sys_event * event);
        int ret = pc_device_event_handler(event);
        if (ret) {
            if (event->u.dev.event == DEVICE_EVENT_IN) {
                /* 退出bt和music模式 */
                if (bt_in_background()) {
                    init_intent(&it);
                    it.action = ACTION_BACK;
                    start_app(&it);
                }
                init_intent(&it);
                it.action = ACTION_BACK;
                start_app(&it);

                /* 切到PC模式 */
                init_intent(&it);
                it.name = "pc";
                it.action = ACTION_PC_MAIN;
                start_app(&it);
            } else {
                cpu_reset();
            }
        }
#endif
        break;
    default:
        break;
    }


}

#endif

