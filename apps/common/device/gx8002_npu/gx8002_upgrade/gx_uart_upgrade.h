#ifndef __GX_URAT_UPGRADE_H__
#define __GX_URAT_UPGRADE_H__

#include "gx_upgrade_def.h"

typedef struct {
    int (*open)(int baud, int databits, int stopbits, int parity);
    int (*close)(void);
    int (*wait_reply)(const unsigned char *buf, unsigned int len, unsigned int timeout);
    int (*send)(const unsigned char *buf, unsigned int len);
} upgrade_uart_t;

int gx_uart_upgrade_init(upgrade_uart_t *uart, fw_stream_t *fw_stream, upgrade_status_cb status_cb);
int gx_uart_upgrade_deinit(void);
int gx_uart_upgrade_proc(void);

#endif
