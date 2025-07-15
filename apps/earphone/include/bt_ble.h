#ifndef APP_BT_BLE_H
#define APP_BT_BLE_H

#include "typedef.h"

// #define ADV_RSP_PACKET_MAX  31
#define ADV_RAND_MAX      7

// typedef struct {
// u16 pid; //厂家id，默认用杰理的id (0x5d6)
// u8  private_data[17];//厂家自定义数据内容

// } manufacturer_data_t;

enum {
    ICON_TYPE_INQUIRY = 0,
    ICON_TYPE_CONNECTED,
    ICON_TYPE_RECONNECT,
    ICON_TYPE_PRE_DEV,
};


enum {
    ADV_ST_NULL,
    ADV_ST_PRE1,
    ADV_ST_PRE2,
    ADV_ST_INQUIRY,
    ADV_ST_CONN,
    ADV_ST_RECONN,
    ADV_ST_DISMISS,
    ADV_ST_FAST_DISMISS,
    ADV_ST_END,
};

extern void bt_ble_icon_set(u8 *info, u8 len);
extern u8 bt_ble_get_adv_enable(void);
extern void bt_ble_icon_open(u8 type);
extern void bt_ble_icon_close(u8 dismiss_flag);
extern void bt_ble_icon_set_comm_address(u8 *addr);
extern void bt_ble_icon_reset(void);
extern void bt_ble_set_control_en(u8 en);
extern void bt_ble_icon_slave_en(u8 en);
extern u8 bt_ble_icon_get_adv_state(void);
extern void bt_ble_icon_role_switch(u8 role);
extern void bt_ble_icon_state_sniff(u8 state);

#endif

