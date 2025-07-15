#ifndef __ADAPTER_WIRELESS_DEC_H__
#define __ADAPTER_WIRELESS_DEC_H__

#include "generic/typedef.h"
#include "media/includes.h"

void adapter_wireless_dec_frame_init(void);
void adapter_wireless_dec_frame_close(void);
int adapter_wireless_dec_frame_write(void *data, u16 len);
int adapter_wireless_dec_open(void);
int adapter_wireless_dec_close(void);

#endif//__ADAPTER_WIRELESS_DEC_H__
