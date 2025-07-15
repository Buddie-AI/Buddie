#ifndef __ADAPTER_WIRELESS_ENC_H__
#define __ADAPTER_WIRELESS_ENC_H__

#include "asm/includes.h"
#include "generic/typedef.h"
#include "media/includes.h"


int adapter_wireless_enc_write(void *buf, int len);
int adapter_wireless_enc_open(void);
void adapter_wireless_enc_close(void);

#endif//__ADAPTER_WIRELESS_ENC_H__
