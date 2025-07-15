#ifndef __ANCTOOL_H__
#define __ANCTOOL_H__

#include "typedef.h"

struct anctool_data {
    void (*send_packet)(u8 *data, u16 size);
    void (*recv_packet)(u8 *data, u16 size);
};

void anctool_api_rx_data(u8 *buf, u16 len);
u8 *anctool_api_write_alloc(u16 len);
void anctool_api_set_active(u8 active);
u16 anctool_api_write(u8 *buf, u16 length);
void anctool_api_init(const struct anctool_data *arg);
void anctool_api_uninit(void);

#endif
