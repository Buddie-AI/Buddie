

#ifndef _3TH_PROFILE_API_H
#define _3TH_PROFILE_API_H

#include<string.h>
#include <stdint.h>
#include "le_common.h"


#define TYPE_NULL        0
#define TYPE_BLE         1
#define TYPE_SPP         2

#define TYPE_MASTER_BLE  3
#define TYPE_SLAVE_BLE   4

#define TYPE_MASTER_SPP  5
#define TYPE_SLAVE_SPP   6

#define SOURCE_TYPE        0
#define SINK_TYPE_MASTER   1
#define SINK_TYPE_SLAVE    2

void set_app_connect_type(u8 type);
u8 get_app_connect_type(void);
u8 get_ble_connect_type(void);
void set_ble_connect_type(u8 type);
void mic_set_data_source(u8 data_type);
u8 mic_get_data_source(void);

int tws_data_to_sibling_send(u8 opcode, u8 *data, u8 len); //发送数据给对耳

#endif
