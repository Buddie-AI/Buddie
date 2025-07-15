#include "system/app_core.h"
#include "system/includes.h"

#include "app_config.h"
#include "app_action.h"

#include "earphone.h"
#include "app_main.h"
#include "update_tws.h"
#include "3th_profile_api.h"

#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "bt_tws.h"

#include "user_cfg.h"
#include "vm.h"
#include "app_power_manage.h"
#include "btcontroller_modules.h"
#include "app_chargestore.h"
#include "bt_common.h"
#include "le_common.h"

#if (BLE_HID_EN)

/* #define LOG_TAG_CONST       BLE_HID */
/* #define LOG_TAG             "[BLE_HID]" */
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
/* #define LOG_INFO_ENABLE */
/* #define LOG_CLI_ENABLE */
/* #include "debug.h" */

#if 1
#define log_info(x, ...)    printf("[BLE-HID]" x " ", ## __VA_ARGS__)
#define log_info_hexdump    put_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif


#define HID_TRACE_FUNC()   log_info("func:%s ,line:%d\n",__FUNCTION__,__LINE__)

//----------------------------------
static const u8 bt_hidkey_report_map[] = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x09, 0xCD,        //   Usage (Play/Pause)
    0x09, 0xE2,        //   Usage (Mute)
    0x09, 0xB6,        //   Usage (Scan Previous Track)
    0x09, 0xB5,        //   Usage (Scan Next Track)
    0x09, 0xB3,        //   Usage (Fast Forward)
    0x09, 0xB4,        //   Usage (Rewind)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    // 35 bytes
};

// consumer key
#define CONSUMER_VOLUME_INC             0x0001
#define CONSUMER_VOLUME_DEC             0x0002
#define CONSUMER_PLAY_PAUSE             0x0004
#define CONSUMER_MUTE                   0x0008
#define CONSUMER_SCAN_PREV_TRACK        0x0010
#define CONSUMER_SCAN_NEXT_TRACK        0x0020
#define CONSUMER_SCAN_FRAME_FORWARD     0x0040
#define CONSUMER_SCAN_FRAME_BACK        0x0080

int ble_hid_is_connected(void);
int ble_hid_data_send(u8 report_id, u8 *data, u16 len);
void le_hogp_set_icon(u16 class_type);
void le_hogp_set_ReportMap(u8 *map, u16 size);
void le_hogp_regiest_get_battery(u8(*get_battery_cbk)(void));
u8 get_vbat_percent(void);
void ble_module_enable(u8 en);
void bt_update_mac_addr(u8 *addr);
void lib_make_ble_address(u8 *ble_address, u8 *edr_address);

static void ble_hid_test_timer(void *priv)
{
    if (ble_hid_is_connected()) {
        log_info("PP key for test!!!\n");
        u16 key = CONSUMER_PLAY_PAUSE;
        ble_hid_data_send(1, &key, 2);
        key = 0;
        ble_hid_data_send(1, &key, 2);
    }
}

int ble_hid_earphone_state_set_page_scan_enable()
{
    return 0;
}

int ble_hid_earphone_state_get_connect_mac_addr()
{
    return 0;
}

int ble_hid_earphone_state_cancel_page_scan()
{
    return 0;
}

int ble_hid_earphone_state_tws_init(int paired)
{
    return 0;
}

int ble_hid_earphone_state_tws_connected(int first_pair, u8 *comm_addr)
{
    if (first_pair) {
        u8 tmp_ble_addr[6] = {0};
        lib_make_ble_address(tmp_ble_addr, comm_addr);
        le_controller_set_mac(tmp_ble_addr);//将ble广播地址改成公共地址
        bt_update_mac_addr(comm_addr);

        /*新的连接，公共地址改变了，要重新将新的地址广播出去*/
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            log_info("New Connect Master!!!\n");
            ble_module_enable(0);
            ble_module_enable(1);
        } else {
            log_info("Connect Slave!!!\n\n");
            /*从机ble关掉*/
            ble_module_enable(0);
        }
    }
    return 0;
}

int ble_hid_earphone_state_enter_soft_poweroff()
{
    extern void bt_ble_exit(void);
    bt_ble_exit();
    return 0;
}

int ble_hid_bt_status_event_handler(struct bt_event *bt)
{
    return 0;
}

int ble_hid_hci_event_handler(struct bt_event *bt)
{
    return 0;
}

void ble_hid_bt_tws_event_handler(struct bt_event *bt)
{
    int role = bt->args[0];
    int phone_link_connection = bt->args[1];
    int reason = bt->args[2];

    log_info("tws_msg: %d\n", bt->event);

    switch (bt->event) {
    case TWS_EVENT_CONNECTED:
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            //master enable
            log_info("Connect Slave!!!\n");
            /*从机ble关掉*/
            ble_module_enable(0);
        }
        break;

    case TWS_EVENT_CONNECTION_TIMEOUT:
        break;

    case TWS_EVENT_PHONE_LINK_DETACH:
        /*
         * 跟手机的链路LMP层已完全断开, 只有tws在连接状态才会收到此事件
         */
        break;
    case TWS_EVENT_CONNECTION_DETACH:
        /*
         * TWS连接断开
         */
        if (app_var.goto_poweroff_flag) {
            break;
        }

        /* if (get_app_connect_type() == 0) { */
        /* log_info("tws detach to open ble~~~\n"); */
        /* ble_module_enable(1); */
        /* } */
        /* set_ble_connect_type(TYPE_NULL); */

        break;

    case TWS_EVENT_SYNC_FUN_CMD:
        break;

    case TWS_EVENT_ROLE_SWITCH:
        break;
    }

#if OTA_TWS_SAME_TIME_ENABLE
    /* tws_ota_app_event_deal(bt->event); */
#endif
}

int ble_hid_sys_event_handler_specific(struct sys_event *event)
{
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {

        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {

        }
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            trans_data_bt_tws_event_handler(&event->u.bt);
        }
#endif
#if OTA_TWS_SAME_TIME_ENABLE
        else if (((u32)event->arg == SYS_BT_OTA_EVENT_TYPE_STATUS)) {
            bt_ota_event_handler(&event->u.bt);
        }
#endif
        break;
    case SYS_DEVICE_EVENT:
        break;
    }

    return 0;
}


int ble_hid_earphone_state_init()
{
    le_hogp_set_icon(BLE_APPEARANCE_HID_KEYBOARD);
    le_hogp_set_ReportMap(bt_hidkey_report_map, sizeof(bt_hidkey_report_map));
    le_hogp_regiest_get_battery(get_vbat_percent);

    sys_timer_add(0, ble_hid_test_timer, 6000);
    return 0;
}

#endif



