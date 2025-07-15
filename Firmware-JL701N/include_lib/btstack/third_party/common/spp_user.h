#ifndef __SPP_USER_H__
#define __SPP_USER_H__

#include "typedef.h"

extern void (*spp_state_cbk)(u8 state);
extern void (*spp_recieve_cbk)(void *priv, u8 *buf, u16 len);
extern void user_spp_data_handler(u8 packet_type, u16 ch, u8 *packet, u16 size);

struct spp_operation_t {
    int(*disconnect)(void *priv);
    int(*send_data)(void *priv, void *buf, u16 len);
    int(*regist_wakeup_send)(void *priv, void *cbk);
    int(*regist_recieve_cbk)(void *priv, void *cbk);
    int(*regist_state_cbk)(void *priv, void *cbk);
    int(*busy_state)(void);
};

enum {
    SPP_USER_ERR_NONE = 0x0,
    SPP_USER_ERR_SEND_BUFF_BUSY,
    SPP_USER_ERR_SEND_OVER_LIMIT,
    SPP_USER_ERR_SEND_FAIL,
};

enum {
    SPP_USER_ST_NULL = 0x0,
    SPP_USER_ST_CONNECT,
    SPP_USER_ST_DISCONN,
    SPP_USER_ST_WAIT_DISC,
    SPP_USER_ST_CONNECT_OTA,
    SPP_USER_ST_DISCONN_OTA,
};

void spp_get_operation_table(struct spp_operation_t **interface_pt);

#endif//__SPP_USER_H__
