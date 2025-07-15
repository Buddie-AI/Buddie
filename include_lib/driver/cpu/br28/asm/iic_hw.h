#ifndef _IIC_HW_H_
#define _IIC_HW_H_

#include "system/generic/typedef.h"

#define IIC_HW_NUM                  1
#define IIC_PORT_GROUP_NUM          4

#define iic_enable(reg)             (reg->CON0 |= BIT(0))
#define iic_disable(reg)            (reg->CON0 &= ~BIT(0))
#define iic_role_host(reg)          (reg->CON0 &= ~BIT(1))
#define iic_role_slave(reg)         (reg->CON0 |= BIT(1))
#define iic_cfg_done(reg)           (reg->CON0 |= BIT(2))
#define iic_dir_out(reg)            (reg->CON0 &= ~BIT(3))
#define iic_dir_in(reg)             (reg->CON0 |= BIT(3))
#define iic_preset_end(reg)         (reg->CON0 |= BIT(4))
#define iic_preset_restart(reg)     (reg->CON0 |= BIT(5))
#define iic_recv_ack(reg)           (reg->CON0 &= ~BIT(6))
#define iic_recv_nack(reg)          (reg->CON0 |= BIT(6))
#define iic_send_is_ack(reg)        (!(reg->CON0 & BIT(7)))
#define iic_isel_direct(reg)        (reg->CON0 &= ~BIT(9))
#define iic_isel_filter(reg)        (reg->CON0 |= BIT(9))
#define iic_si_mode_en(reg)         (reg->CON1 |= BIT(13))
#define iic_si_mode_dis(reg)        (reg->CON1 &= ~BIT(13))

#define iic_set_ie(reg)             (reg->CON0 |= BIT(8))
#define iic_clr_ie(reg)             (reg->CON0 &= ~BIT(8))
#define iic_pnd(reg)                (reg->CON0 & BIT(15))
#define iic_pnd_clr(reg)            (reg->CON0 |= BIT(14))

#define iic_set_end_ie(reg)         (reg->CON0 |= BIT(10))
#define iic_clr_end_ie(reg)         (reg->CON0 &= BIT(10))
#define iic_end_pnd(reg)            (reg->CON0 & BIT(13))
#define iic_end_pnd_clr(reg)        (reg->CON0 |= BIT(12))

#define iic_start_pnd(reg)          (reg->CON1 & BIT(15))
#define iic_start_pnd_clr(reg)      (reg->CON1 |= BIT(14))

#define iic_baud_reg(reg)           (reg->BAUD)
#define iic_buf_reg(reg)            (reg->BUF)


typedef const int hw_iic_dev;

enum {IIC_MASTER, IIC_SLAVE};

struct hw_iic_config {
    u8 port[2];
    u32 baudrate;
    u8 hdrive;
    u8 io_filter;
    u8 io_pu;
    u8 role;
};

extern const struct hw_iic_config hw_iic_cfg[];

int hw_iic_init(hw_iic_dev iic);
void hw_iic_uninit(hw_iic_dev iic);
void hw_iic_suspend(hw_iic_dev iic);
void hw_iic_resume(hw_iic_dev iic);
void hw_iic_start(hw_iic_dev iic);
void hw_iic_stop(hw_iic_dev iic);
u8 hw_iic_tx_byte(hw_iic_dev iic, u8 byte);
u8 hw_iic_rx_byte(hw_iic_dev iic, u8 ack);
int hw_iic_read_buf(hw_iic_dev iic, void *buf, int len);
int hw_iic_write_buf(hw_iic_dev iic, const void *buf, int len);
int hw_iic_set_baud(hw_iic_dev iic, u32 baud);

void hw_iic_set_ie(hw_iic_dev iic, u8 en);
u8 hw_iic_get_pnd(hw_iic_dev iic);
void hw_iic_clr_pnd(hw_iic_dev iic);
void hw_iic_set_end_ie(hw_iic_dev iic, u8 en);
u8 hw_iic_get_end_pnd(hw_iic_dev iic);
void hw_iic_clr_end_pnd(hw_iic_dev iic);
void hw_iic_slave_set_addr(hw_iic_dev iic, u8 addr, u8 addr_ack);
void hw_iic_slave_rx_prepare(hw_iic_dev iic, u8 ack);
u8 hw_iic_slave_rx_byte(hw_iic_dev iic, bool *is_start_addr);
void hw_iic_slave_tx_byte(hw_iic_dev iic, u8 byte);
u8 hw_iic_slave_tx_check_ack(hw_iic_dev iic);

#endif
