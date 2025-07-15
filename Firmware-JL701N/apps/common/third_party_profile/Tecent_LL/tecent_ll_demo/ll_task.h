#ifndef __LL_TASK_H__
#define __LL_TASK_H__

#include "typedef.h"
#include "os/os_api.h"
#include "circular_buf.h"

typedef struct __LL_PACKET_HEAD {
    u8 packet_channel;
    u16 len;
} LL_PACKET_HEAD_T;

typedef struct __LL_PACKET_CONTROL {
    OS_SEM               ll_sem;
    cbuffer_t            cbuf;
    volatile u8          wait;
    u8                   *tecent_ll_buf;
} LL_PACKET_CONTROL;

#define LL_PACKET_HEAD_LEN              (sizeof(LL_PACKET_HEAD_T))

enum {
    LL_DEVICE_INFO_MSG_CH = 0,
    LL_DATA_MSG_CH,
    LL_OTA_MSG_CH,
};

int  tecent_ll_task_init(void);
void tecent_ll_packet_recieve(void *buf, u16 len);

#endif //__LL_TASK_H__
