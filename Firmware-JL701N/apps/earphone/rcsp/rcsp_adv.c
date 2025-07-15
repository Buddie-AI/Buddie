#include "app_config.h"

#if RCSP_ADV_EN

#include "earphone.h"
#include "app_main.h"
#include "le_rcsp_adv_module.h"
#include "rcsp_adv_bluetooth.h"
#include "rcsp_bluetooth.h"
#include "adv_mic_setting.h"
#include "JL_rcsp_api.h"
#include "update_tws.h"
#include "update_tws_new.h"
#include "3th_profile_api.h"
#include "adv_time_stamp_setting.h"
#include "adv_anc_voice.h"

#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "bt_tws.h"

#define LOG_TAG             "[RCSP-ADV]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

extern APP_VAR app_var;
extern void rcsp_spp_init();
extern u8 get_app_connect_type(void);
extern void ble_app_disconnect(void);
extern u8 get_app_connect_type(void);
extern void mic_data_sync_tx_deal(void);
extern void set_ble_connect_type(u8 type);
extern void bt_ble_adv_enable(u8 enable);
extern void app_tws_get_data_from_sibling(u8 cmd, u8 *data, u8 len);
extern u8 JL_get_cur_bt_channel_sel(void);
extern void send_version_to_sibling(void);
extern void set_rcsp_dev_type_spp(void);
extern u8 rcsp_get_update_flag(void);

u8 ble_adv_miss_flag = 0;
u8 ble_adv_poweron_flag = 0;

int rcsp_earphone_state_set_page_scan_enable()
{
#if (TCFG_USER_TWS_ENABLE == 0)
    bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
#elif (CONFIG_NO_DISPLAY_BUTTON_ICON || !TCFG_CHARGESTORE_ENABLE)
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        printf("switch_icon_ctl11...\n");
        bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
    }
#endif
    return 0;
}

int rcsp_earphone_state_get_connect_mac_addr()
{
    bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_CONNECTING, 1);
    return 0;
}

int rcsp_earphone_state_cancel_page_scan()
{
#if (TCFG_USER_TWS_ENABLE == 0)
    bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
#elif (CONFIG_NO_DISPLAY_BUTTON_ICON || !TCFG_CHARGESTORE_ENABLE)
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        if (ble_adv_miss_flag) {
            ble_adv_miss_flag = 0;
            puts("ble_adv_miss_flag...\n");
        } else {
            printf("switch_icon_ctl00...\n");
            bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
        }
    }
#endif
    return 0;
}

int rcsp_earphone_state_tws_init(int paired)
{
    ble_adv_poweron_flag = 1;

    if (paired) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            bt_ble_adv_ioctl(BT_ADV_ENABLE, 0, 1);
        } else {
            //slave close
            bt_ble_adv_ioctl(BT_ADV_DISABLE, 0, 1);
        }
    } else {
#if 0 //disable
        //no_pair_adv,last do
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            printf("switch_icon_ctl no_pair...\n");
            bt_ble_set_control_en(1);
            bt_ble_icon_open(ICON_TYPE_INQUIRY);
        } else {
            bt_ble_set_control_en(0);
        }
#endif
    }
    return 0;
}

int rcsp_earphone_state_tws_connected(int first_pair, u8 *comm_addr)
{
#if JL_EARPHONE_APP_EN
#if RCSP_ADV_MUSIC_INFO_ENABLE
    extern void stop_get_music_timer(u8 en) ;
    stop_get_music_timer(0);
#endif
#endif

    if (first_pair) {
        extern void ble_module_enable(u8 en);
        extern void bt_update_mac_addr(u8 * addr);
        extern void lib_make_ble_address(u8 * ble_address, u8 * edr_address);
        /* bt_ble_adv_enable(0); */
        u8 tmp_ble_addr[6] = {0};
        lib_make_ble_address(tmp_ble_addr, comm_addr);
        le_controller_set_mac(tmp_ble_addr);//将ble广播地址改成公共地址
        bt_update_mac_addr(comm_addr);
        /* bt_ble_adv_enable(1); */


        /*新的连接，公共地址改变了，要重新将新的地址广播出去*/
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            printf("\nNew Connect Master!!!\n\n");
            ble_app_disconnect();
            bt_ble_adv_enable(0);
            bt_ble_adv_enable(1);
        } else {
            printf("\nConnect Slave!!!\n\n");
            /*从机ble关掉*/
            ble_app_disconnect();
            bt_ble_adv_enable(0);
        }
    }

    return 0;
}

int rcsp_earphone_state_enter_soft_poweroff()
{
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        u8 adv_cmd = 0x3;
        extern u8 adv_info_device_request(u8 * buf, u16 len);
        extern u8 adv_tws_both_in_charge_box(u8 type);
        if (adv_tws_both_in_charge_box(0)) {
            adv_info_device_request(&adv_cmd, sizeof(adv_cmd));
        }
    }
#endif
    extern void bt_ble_exit(void);
    bt_ble_exit();
    return 0;
}

int rcsp_adv_bt_status_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
    case BT_STATUS_SECOND_CONNECTED:
    case BT_STATUS_FIRST_CONNECTED:
#if TCFG_USER_TWS_ENABLE
        if (tws_api_get_role() == TWS_ROLE_MASTER || JL_get_cur_bt_channel_sel() == RCSP_SPP) {
            bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_CONNECTED, 1);
        } else {
            //maybe slave already open
            bt_ble_adv_ioctl(BT_ADV_DISABLE, 0, 1);
        }
#else
        bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_CONNECTED, 1);
#endif
        break;
    case BT_STATUS_FIRST_DISCONNECT:
    case BT_STATUS_SECOND_DISCONNECT:
        bt_adv_seq_change();
        if (!app_var.goto_poweroff_flag) {
#if TCFG_USER_TWS_ENABLE
            if (tws_api_get_role() == TWS_ROLE_MASTER || JL_get_cur_bt_channel_sel() == RCSP_SPP) {
                bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
            } else {
                //maybe slave already open
                bt_ble_adv_ioctl(BT_ADV_DISABLE, 0, 1);
            }
#else
            bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
#endif
        }
        break;
    case BT_STATUS_SCO_STATUS_CHANGE:
#if TCFG_USER_TWS_ENABLE
        if (bt->value != 0xff) {
            rcsp_user_mic_fixed_deal(1);
        } else {
            rcsp_user_mic_fixed_deal(0);
        }
#endif
        break;

    case BT_STATUS_PHONE_INCOME:
    case BT_STATUS_PHONE_OUT:
    case BT_STATUS_PHONE_ACTIVE:
        printf("BT_STATUS_PHONE_ACTIVE\n");
#if RCSP_ADV_FIND_DEVICE_ENABLE
        extern void send_find_device_stop(void);
        send_find_device_stop();
#endif
        // 更新通话状态
        extern void JL_bt_phone_state_update(void);
        JL_bt_phone_state_update();
        break;
    case BT_STATUS_PHONE_HANGUP:
        printf("BT_STATUS_PHONE_HANGUP\n");
        // 更新通话状态
        extern void JL_bt_phone_state_update(void);
        JL_bt_phone_state_update();
        break;
    }

    return 0;
}


int rcsp_adv_hci_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
    case HCI_EVENT_CONNECTION_COMPLETE:
        switch (bt->value) {
        case ERROR_CODE_PIN_OR_KEY_MISSING:
#if (CONFIG_NO_DISPLAY_BUTTON_ICON && TCFG_CHARGESTORE_ENABLE)
            //已取消配对了, 切换广播
            bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
#endif
            break;
        }
        break;
    }

    return 0;
}

void rcsp_bt_tws_event_handler(struct bt_event *bt)
{
    int role = bt->args[0];
    int phone_link_connection = bt->args[1];
    int reason = bt->args[2];

    switch (bt->event) {
    case TWS_EVENT_CONNECTED:
        //bt_ble_adv_enable(1);
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            //master enable
            log_info("master do icon_open\n");
            if (phone_link_connection) {
                bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_CONNECTED, 1);
            } else {
#if (TCFG_CHARGESTORE_ENABLE && !CONFIG_NO_DISPLAY_BUTTON_ICON)
                bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_CONNECTED, 1);
#else
                if (ble_adv_poweron_flag) { //上电标记
                    if (bt_user_priv_var.auto_connection_counter > 0) {
                        //有回连手机动作
                        /* g_printf("ICON_TYPE_RECONNECT"); */
                        bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_CONNECTING, 1);
                        //没按键配对的话，等回连成功的时候才显示电量。如果在这里显示，手机取消配对后耳机开机，会显示出按键的界面
                    } else {
                        //没有回连，设可连接
                        /* g_printf("ICON_TYPE_INQUIRY"); */
                        bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
                    }

                }
#endif
            }
        } else {
            //slave disable
            printf("\nConnect Slave!!!\n\n");
            /*从机ble关掉*/
            ble_app_disconnect();
            bt_ble_adv_enable(0);
        }
        send_version_to_sibling();
        ble_adv_poweron_flag = 0;

#if RCSP_ADV_ANC_VOICE
        anc_voice_setting_sync();
#endif

        sync_setting_by_time_stamp();

#if (RCSP_ADV_FIND_DEVICE_ENABLE && TCFG_USER_TWS_ENABLE)
        extern void find_decice_tws_connect_handle(u8 flag, u8 * param);
        find_decice_tws_connect_handle(0, NULL);
#endif

        break;
    case TWS_EVENT_PHONE_LINK_DETACH:
        /*
         * 跟手机的链路LMP层已完全断开, 只有tws在连接状态才会收到此事件
         */
        if (reason == 0x0b) {
            //CONNECTION ALREADY EXISTS
            ble_adv_miss_flag = 1;
        } else {
            ble_adv_miss_flag = 0;
        }
        break;
    case TWS_EVENT_CONNECTION_DETACH:
        /*
         * TWS连接断开
         */
        if (app_var.goto_poweroff_flag) {
            break;
        }
        if (get_app_connect_type() == 0) {
            printf("\ntws detach to open ble~~~\n\n");
            bt_ble_adv_enable(1);
        }
        set_ble_connect_type(TYPE_NULL);

        deal_adv_setting_gain_time_stamp();
        break;
    case TWS_EVENT_SYNC_FUN_CMD:
#if JL_EARPHONE_APP_EN
        if (reason == SYNC_CMD_APP_RESET_LED_UI) {
#if RCSP_ADV_LED_SET_ENABLE
            extern void update_led_setting_state(void);
            update_led_setting_state();
#endif
        } else if (reason == SYNC_CMD_MUSIC_INFO) {
#if RCSP_ADV_MUSIC_INFO_ENABLE
            extern void get_music_info(void);
            get_music_info();
#endif
        } else if (reason == SYNC_CMD_RCSP_AUTH_RES) {
            extern void rcsp_tws_auth_sync_deal(void);
            rcsp_tws_auth_sync_deal();
        } else if (reason == SYNC_CMD_MUSIC_PLAYER_STATE) {
#if RCSP_ADV_MUSIC_INFO_ENABLE
            void rcsp_update_player_state(void);
            rcsp_update_player_state();
#endif
        } else if (reason ==  SYNC_CMD_MUSIC_PLAYER_TIEM_EN) {
#if RCSP_ADV_MUSIC_INFO_ENABLE
            extern void reset_player_time_en(void);
            reset_player_time_en();
#endif
        }
#endif
        break;
    case TWS_EVENT_ROLE_SWITCH:
        if (role == TWS_ROLE_MASTER) {	// 切换后触发
#if JL_EARPHONE_APP_EN
#if RCSP_ADV_LED_SET_ENABLE
            extern void update_led_setting_state(void);
            update_led_setting_state();
#endif
#endif
        }
        {
            extern void adv_role_switch_handle(void);
            adv_role_switch_handle();
        }
        break;
    }

#if OTA_TWS_SAME_TIME_ENABLE
    tws_ota_app_event_deal(bt->event);
#endif
}

int rcsp_sys_event_handler_specific(struct sys_event *event)
{
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            rcsp_adv_bt_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            rcsp_adv_hci_event_handler(&event->u.bt);
        }
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            rcsp_bt_tws_event_handler(&event->u.bt);
        }
#endif
#if OTA_TWS_SAME_TIME_ENABLE
        else if (((u32)event->arg == SYS_BT_OTA_EVENT_TYPE_STATUS)) {
            bt_ota_event_handler(&event->u.bt);
        }
#endif
        break;
    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_RCSP) {
            int JL_rcsp_event_handler(struct rcsp_event * rcsp);
            JL_rcsp_event_handler(&event->u.rcsp);
        }
        break;
    }

    return 0;
}



int user_spp_state_specific(u8 packet_type)
{
    u8 tws_need_update = 0;

    switch (packet_type) {
    case 1:
        ble_app_disconnect();
        bt_ble_adv_enable(0);
        set_app_connect_type(TYPE_SPP);
#if (0 == BT_CONNECTION_VERIFY)
        JL_rcsp_auth_reset();
#endif

        set_rcsp_dev_type_spp();
        set_connect_flag(2);
        bt_ble_adv_ioctl(BT_ADV_SET_NOTIFY_EN, 1, 1);
        break;
    case 2:

        set_app_connect_type(TYPE_NULL);
        bt_ble_adv_ioctl(BT_ADV_SET_NOTIFY_EN, 0, 1);

#if TCFG_USER_TWS_ENABLE
        if (syscfg_read(VM_UPDATE_FLAG, &tws_need_update, sizeof(tws_need_update)) == 1) {
            printf("packet_type:%d need:%d\n", packet_type, tws_need_update);
            if (tws_need_update) {         //这里直接return防止升级过程从机断开SPP之后又开广播然后被主机误连
                return 0;
            }
        }
        if (!(tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
            if (!rcsp_get_update_flag()) {
                ble_module_enable(1);
            }
        } else {
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                if (!rcsp_get_update_flag()) {
                    ble_module_enable(1);
                }
            }
        }
#else
        ble_module_enable(1);
#endif

#if (0 == BT_CONNECTION_VERIFY)
        JL_rcsp_auth_reset();
#endif
        break;
    }
    return 0;
}


int rcsp_earphone_state_init()
{
    rcsp_spp_init();
    rcsp_init();
    spp_data_deal_handle_register(user_spp_data_handler);

#if JL_EARPHONE_APP_EN
    extern int jl_phone_app_init();
    jl_phone_app_init();
#endif
    return 0;
}





#endif
