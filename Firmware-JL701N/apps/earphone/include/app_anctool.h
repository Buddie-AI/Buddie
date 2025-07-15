#ifndef _APP_ANCTOOL_H_
#define _APP_ANCTOOL_H_

#include "typedef.h"
#include "anctool.h"

u8 app_anctool_spp_rx_data(u8 *packet, u16 size);
void app_anctool_spp_connect(void);
void app_anctool_spp_disconnect(void);
u8 get_app_anctool_spp_connected_flag();

#endif    //_APP_CHARGESTORE_H_

