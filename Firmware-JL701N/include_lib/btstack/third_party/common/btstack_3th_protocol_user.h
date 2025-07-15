#ifndef __BTSTACK_3TH_PROTOCAL_H__
#define __BTSTACK_3TH_PROTOCAL_H__

#include "spp_config.h"
#include "ble_config.h"
#include "JL_rcsp_api.h"
#include "JL_rcsp_protocol.h"
#include "JL_rcsp_packet.h"
#include "attr.h"


#define APP_TYPE_RCSP                0x01
#define APP_TYPE_DUEROS              0x02
#define APP_TYPE_BTSTACK             0xFF


#define BT_CONFIG_BLE        BIT(0)
#define BT_CONFIG_SPP        BIT(1)

typedef enum {
    BT_3TH_EVENT_COMMON_INIT,
    BT_3TH_EVENT_COMMON_BLE_STATUS,
    BT_3TH_EVENT_COMMON_SPP_STATUS,

    BT_3TH_EVENT_RCSP_DEV_SELECT = 100,




    BT_3TH_EVENT_DUEROS_CONNECT = 200,


} BT_3TH_EVENT_TYPE;

enum {
    RCSP_BLE,
    RCSP_SPP,
};


typedef struct __BT_3TH_USER_CB {
    int type;
    int bt_config;
    JL_PRO_CB bt_3th_handler;
    void (*BT_3TH_event_handler)(u16 opcode, u8 *packet, int size);
    void (*BT_3TH_data_handler)(u8 *packet, int size);
    int (*BT_3TH_spp_state_specific)(u8 type);
} BT_3TH_USER_CB;

typedef struct __BT_3TH_PROTOCOL_CB {
    int type;
    void (*BT_3TH_type_dev_select)(u8 type);
    void (*resume)(void);
    void (*recieve)(void *, void *, u16);
} BT_3TH_PROTOCOL_CB;

int btstack_3th_protocol_user_init(BT_3TH_USER_CB *protocol_callback);
int btstack_3th_protocol_lib_init(BT_3TH_PROTOCOL_CB *protocol_lib_callback);

JL_PRO_CB *bt_3th_get_spp_callback(void);
void bt_3th_set_spp_callback_priv(void *priv);
JL_PRO_CB *bt_3th_get_ble_callback(void);
void bt_3th_set_ble_callback_priv(void *priv);
void bt_3th_dev_type_spp(void);
void bt_3th_type_dev_select(u8 type);
u8 bt_3th_get_cur_bt_channel_sel(void);
void bt_3th_spp_state_handle(u8 type);
void bt_3th_event_send_to_user(u16 opcode, u8 *packet, int size);
void bt_3th_data_send_to_user(u8 *packet, int size);
#endif// __BTSTACK_3TH_PROTOCAL_H__


