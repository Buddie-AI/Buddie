#ifndef	_IIC_SOFT_H_
#define _IIC_SOFT_H_

#include "generic/typedef.h"

typedef const int soft_iic_dev;

struct soft_iic_config {
    int scl;
    int sda;
    u32 delay;
    u8 io_pu;
};

extern const struct soft_iic_config soft_iic_cfg[];

int soft_iic_init(soft_iic_dev iic);
void soft_iic_uninit(soft_iic_dev iic);
void soft_iic_suspend(soft_iic_dev iic);
void soft_iic_resume(soft_iic_dev iic);
void soft_iic_start(soft_iic_dev iic);
void soft_iic_stop(soft_iic_dev iic);
u8 soft_iic_tx_byte(soft_iic_dev iic, u8 byte);
u8 soft_iic_rx_byte(soft_iic_dev iic, u8 ack);
int soft_iic_read_buf(soft_iic_dev iic, void *buf, int len);
int soft_iic_write_buf(soft_iic_dev iic, const void *buf, int len);
#endif

