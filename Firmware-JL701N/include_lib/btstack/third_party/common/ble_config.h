#ifndef __BLE_CONFIG_H__
#define __BLE_CONFIG_H__

#include "typedef.h"
#include "ble_user.h"

bool bt_3th_ble_ready(void *priv);
s32 bt_3th_ble_send(void *priv, void *data, u16 len);
void bt_3th_ble_callback_set(void (*resume)(void), void (*recieve)(void *, void *, u16), void (*status)(void *, ble_state_e));
void bt_3th_ble_status_callback(void *priv, ble_state_e status);
int bt_3th_ble_data_send(void *priv, u8 *buf, u16 len);
void bt_3th_ble_get_operation_table(void);
ble_state_e bt_3th_get_jl_ble_status(void);

#endif//__BLE_CONFG_H__
