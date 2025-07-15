#ifndef __SPP_CONFIG_H__
#define __SPP_CONFIG_H__

#include "typedef.h"

bool bt_3th_spp_fw_ready(void *priv);
s32 bt_3th_spp_send(void *priv, void *data, u16 len);
void bt_3th_spp_callback_set(void (*resume)(void), void (*recieve)(void *, void *, u16), void (*status)(u8));
u8 bt_3th_get_jl_spp_status(void);
void bt_3th_spp_status_callback(u8 status);
int bt_3th_spp_data_send(void *priv, u8 *buf, u16 len);
void bt_3th_spp_init(void);
void bt_3th_spp_get_operation_table(void);

#endif//__SPP_CONFIG_H__
