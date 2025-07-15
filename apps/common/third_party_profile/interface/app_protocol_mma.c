#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_protocol_api.h"
#include "system/includes.h"
#include "audio_config.h"
#include "app_power_manage.h"
#include "user_cfg.h"
#include "bt_tws.h"
#include "custom_cfg.h"

#if APP_PROTOCOL_MMA_CODE

#if 1
#define APP_MMA_LOG       printf
#define APP_MMA_DUMP      put_buf
#else
#define APP_MMA_LOG(...)
#define APP_MMA_DUMP(...)
#endif
//*********************************************************************************//
//                                 MMA初始化                                       //
//*********************************************************************************//

void mma_prev_init()
{
    u16 pid = 0x210c;
    u16 vid = 0x05d6;
    u16 ver = 0x1100;
#if APP_PROTOCOL_READ_CFG_EN
    vid = get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);
    pid = get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG);
    ver = get_vid_pid_ver_from_cfg_file(GET_VER_FROM_EX_CFG);
#endif
    APP_MMA_LOG("mma pid:%x, vid:%x, ver:%x\n", pid, vid, ver);
    app_protocol_set_vendor_id(MMA_HANDLER_ID, vid);
    app_protocol_set_product_id(MMA_HANDLER_ID, pid);
    app_protocol_set_local_version(MMA_HANDLER_ID, ver);
}

//*********************************************************************************//
//                                 MMA提示音                                       //
//*********************************************************************************//
const char *mma_notice_tab[APP_RROTOCOL_TONE_MAX] = {
    [APP_RROTOCOL_TONE_SPEECH_KEY_START]	    	= TONE_NORMAL,
};

//*********************************************************************************//
//                                 MMA私有消息处理                                 //
//*********************************************************************************//

#if 1
extern int le_controller_get_mac(void *addr);

#ifdef CONFIG_NEW_BREDR_ENABLE
void tws_host_get_common_addr(u8 *remote_addr, u8 *common_addr, char channel)
#else
void tws_host_get_common_addr(u8 *local_addr, u8 *remote_addr, u8 *common_addr, char channel)
#endif
{
    APP_MMA_LOG(">>>>>>>>>tws_host_get_common_addr ch:%c \n", channel);

    if (channel == 'L') {
        memcpy(common_addr, bt_get_mac_addr(), 6);
    } else {
        memcpy(common_addr, remote_addr, 6);
    }
    APP_MMA_LOG(">>>local mac: ");
    APP_MMA_DUMP(bt_get_mac_addr(), 6);
    APP_MMA_LOG("remote_addr: ");
    APP_MMA_DUMP((const u8 *)remote_addr, 6);
    APP_MMA_LOG("common_addr: ");
    APP_MMA_DUMP((const u8 *)common_addr, 6);
}

static void mma_tws_get_data_analysis(u16 cmd, u8 *data, u16 len)
{
}

static void mma_update_ble_addr()
{
    u8 ble_old_addr[6];
    u8 ble_new_addr[6];
    u8 comm_addr[6];
    tws_api_get_local_addr(comm_addr);
    le_controller_get_mac(ble_old_addr);
    void ble_update_address_tws_paired(u8 * comm_addr);
    ble_update_address_tws_paired(comm_addr);
    le_controller_get_mac(ble_new_addr);
    if (is_tws_master_role() && memcmp(ble_old_addr, ble_new_addr, 6)) {
        app_protocol_disconnect(NULL);
        app_protocol_ble_adv_switch(0);
        app_protocol_ble_adv_switch(1);
    }
}

static void mma_bt_tws_event_handler(struct bt_event *bt)
{
    int role = bt->args[0];
    int phone_link_connection = bt->args[1];
    int reason = bt->args[2];

    switch (bt->event) {
    case TWS_EVENT_CONNECTED:
        mma_update_ble_addr();
        break;
    case TWS_EVENT_REMOVE_PAIRS:
        mma_update_ble_addr();
        break;
    }
}


static int mma_bt_status_event_handler(struct bt_event *bt)
{
    return 0;
}

int mma_sys_event_deal(struct sys_event *event)
{
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            mma_bt_status_event_handler(&event->u.bt);
        }
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            mma_bt_tws_event_handler(&event->u.bt);
        }
#endif
        break;

    }

    return 0;

}

struct app_protocol_private_handle_t mma_private_handle = {
    /* .tws_rx_from_siblling = mma_tws_get_data_analysis, */
    .sys_event_handler = mma_sys_event_deal,
};
#endif
#endif

