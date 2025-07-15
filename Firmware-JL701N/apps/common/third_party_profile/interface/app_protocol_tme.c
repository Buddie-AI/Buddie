#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_protocol_api.h"
#include "system/includes.h"
#include "audio_config.h"
#include "app_power_manage.h"
#include "user_cfg.h"
#include "btstack/avctp_user.h"
#include "bt_tws.h"

#if APP_PROTOCOL_TME_CODE

extern void TME_set_edr_connected(u8 flag);

//*********************************************************************************//
//                                 TME认证信息                                     //
//*********************************************************************************//

//*********************************************************************************//
//                                 TME提示音                                       //
//*********************************************************************************//
const char *tme_notice_tab[APP_RROTOCOL_TONE_MAX] = {
    [APP_RROTOCOL_TONE_SPEECH_KEY_START]	    	= TONE_NORMAL,
};

//*********************************************************************************//
//                                 TME弱函数实现                                   //
//*********************************************************************************//

//*********************************************************************************//
//                                 TME私有消息处理                                 //
//*********************************************************************************//
#if 1
extern int le_controller_set_mac(void *addr);
extern int le_controller_get_mac(void *addr);

#if TCFG_USER_TWS_ENABLE
static void tme_tws_rx_from_sibling(u16 cmd, u8 *data, u16 len)
{
#if TCFG_USER_TWS_ENABLE
    switch (cmd) {
    }
#endif
}

static void tme_update_ble_addr()
{
    u8 ble_old_addr[6];
    u8 ble_new_addr[6];
    u8 comm_addr[6];

    printf("%s\n", __func__);

    tws_api_get_local_addr(comm_addr);
    le_controller_get_mac(ble_old_addr);
    lib_make_ble_address(ble_new_addr, comm_addr);
    le_controller_set_mac(ble_new_addr); //地址发生变化，更新地址
    if (is_tws_master_role() && memcmp(ble_old_addr, ble_new_addr, 6)) {
        app_protocol_disconnect(NULL);
        app_protocol_ble_adv_switch(0);
        app_protocol_ble_adv_switch(1);
    }
}

static void tme_bt_tws_event_handler(struct bt_event *bt)
{
    int role = bt->args[0];
    int phone_link_connection = bt->args[1];
    int reason = bt->args[2];

    switch (bt->event) {
    case TWS_EVENT_CONNECTED:
        tme_update_ble_addr();
        if (BT_STATUS_WAITINT_CONN != get_bt_connect_status()) {
            TME_set_edr_connected(1);
        }
        break;
    case TWS_EVENT_REMOVE_PAIRS:
        tme_update_ble_addr();
        break;
    }

}
#endif

static int tme_bt_status_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
    case BT_STATUS_SECOND_CONNECTED:
    case BT_STATUS_FIRST_CONNECTED:
        TME_set_edr_connected(1); //连接过经典蓝牙标志
        break;
    }
    return 0;
}

int tme_sys_event_deal(struct sys_event *event)
{
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            tme_bt_status_event_handler(&event->u.bt);
        }
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            tme_bt_tws_event_handler(&event->u.bt);
        }
#endif
        break;

    }

    return 0;

}

struct app_protocol_private_handle_t tme_private_handle = {
    /* .tws_rx_from_siblling = tme_tws_rx_from_sibling, */
    .sys_event_handler = tme_sys_event_deal,
};
#endif

#endif

