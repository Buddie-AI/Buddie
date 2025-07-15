#include "asm/iic_hw.h"
#include "system/generic/gpio.h"
#include "system/generic/log.h"
#include "asm/clock.h"
#include "asm/cpu.h"

/*
    [[  注意!!!  ]]
    * 适用于带cfg_done的硬件IIC，另一种硬件IIC另作说明
    * 硬件IIC的START / ACK(NACK)必须在发送或接收字节cfg_done前设置，且不能
      接cfg_done单独发送；而STOP则应在发送或接收字节cfg_done后设置，必须接
      cfg_done单独发送
*/

static JL_IIC_TypeDef *const iic_regs[IIC_HW_NUM] = {
    JL_IIC,
};

#define iic_get_id(iic)         (iic)

#define iic_info_port(iic, x)   (hw_iic_cfg[iic_get_id(iic)].port[x])
#define iic_info_baud(iic)      (hw_iic_cfg[iic_get_id(iic)].baudrate)
#define iic_info_hdrive(iic)    (hw_iic_cfg[iic_get_id(iic)].hdrive)
#define iic_info_io_filt(iic)   (hw_iic_cfg[iic_get_id(iic)].io_filter)
#define iic_info_io_pu(iic)     (hw_iic_cfg[iic_get_id(iic)].io_pu)
#define iic_info_role(iic)      (hw_iic_cfg[iic_get_id(iic)].role)

static inline u32 iic_get_scl(hw_iic_dev iic)
{
    u8 port = iic_info_port(iic, 0);
    return port;
}

static inline u32 iic_get_sda(hw_iic_dev iic)
{
    u8 port = iic_info_port(iic, 1);
    return port;
}

static int iic_port_init(hw_iic_dev iic)
{
    u32 reg;
    int ret = 0;
    u8 id = iic_get_id(iic);
    u32 scl, sda;
    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    if (id == 0) {
        gpio_set_fun_output_port(scl, FO_IIC_SCL, 1, 1, LOW_POWER_FREE);
        gpio_set_fun_output_port(sda, FO_IIC_SDA, 1, 1, LOW_POWER_FREE);
        gpio_set_fun_input_port(scl, PFI_IIC_SCL, LOW_POWER_FREE);
        gpio_set_fun_input_port(sda, PFI_IIC_SDA, LOW_POWER_FREE);
        if (iic_info_hdrive(iic)) {
            gpio_set_hd(scl, 1);
            gpio_set_hd(sda, 1);
        } else {
            gpio_set_hd(scl, 0);
            gpio_set_hd(sda, 0);
        }
        if (iic_info_io_pu(iic)) {
            gpio_set_pull_up(scl, 1);
            gpio_set_pull_up(sda, 1);
        } else {
            gpio_set_pull_up(scl, 0);
            gpio_set_pull_up(sda, 0);
        }
    } else {
        ret = -EINVAL;
    }
    return ret;
}

int hw_iic_set_baud(hw_iic_dev iic, u32 baud)
{
    //f_iic = f_sys / ((IIC_BAUD + 1) * 2)
    //=> IIC_BAUD = f_sys / (2 * f_iic) - 1
    u32 sysclk;
    u8 id = iic_get_id(iic);

    sysclk = clk_get("lsb");
    if (sysclk < 2 * baud) {
        return -EINVAL;
    }
    iic_baud_reg(iic_regs[id]) = sysclk / (2 * baud) - 1;
    return 0;
}

static void hw_iic_set_die(hw_iic_dev iic, u8 en)
{
    u8 id = iic_get_id(iic);
    u32 scl, sda;
    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    if (id == 0) {
        gpio_set_die(scl, en);
        gpio_set_die(sda, en);
    } else {
        //undefined
    }
}

void hw_iic_suspend(hw_iic_dev iic)
{
    hw_iic_set_die(iic, 0);
}

void hw_iic_resume(hw_iic_dev iic)
{
    hw_iic_set_die(iic, 1);
}

int hw_iic_init(hw_iic_dev iic)
{
    int ret;
    u8 id = iic_get_id(iic);

    if ((ret = iic_port_init(iic))) {
        log_e("invalid hardware iic port\n");
        return ret;
    }
    hw_iic_set_die(iic, 1);
    if (iic_info_role(iic) == IIC_MASTER) {
        iic_role_host(iic_regs[id]);
    } else {
        iic_role_slave(iic_regs[id]);
        iic_si_mode_en(iic_regs[id]);
    }
    if (iic_info_io_filt(iic)) {
        iic_isel_filter(iic_regs[id]);
    } else {
        iic_isel_direct(iic_regs[id]);
    }
    if ((ret = hw_iic_set_baud(iic, iic_info_baud(iic)))) {
        log_e("iic baudrate is invalid\n");
        return ret ;
    }
    iic_pnd_clr(iic_regs[id]);
    iic_end_pnd_clr(iic_regs[id]);
    iic_start_pnd_clr(iic_regs[id]);
    iic_enable(iic_regs[id]);
#if 0
    printf("info->scl = %d\n", iic_get_scl(iic));
    printf("info->sda = %d\n", iic_get_sda(iic));
    printf("info->baudrate = %d\n", iic_info_baud(iic));
    printf("info->hdrive = %d\n", iic_info_hdrive(iic));
    printf("info->io_filter = %d\n", iic_info_io_filt(iic));
    printf("info->io_pu = %d\n", iic_info_io_pu(iic));
    printf("info->role = %d\n", iic_info_role(iic));
    printf("IIC_CON0 0x%04x\n", iic_regs[id]->CON0);
    printf("IIC_CON1 0x%04x\n", iic_regs[id]->CON1);
    printf("IIC_BAUD 0x%02x\n", iic_regs[id]->BAUD);
    //printf("IIC_BUF %02x\n", iic_regs[id]->BUF);
    printf("IOMC1 0x%08x\n", JL_IOMAP->CON1);
#endif
    return 0;
}

void hw_iic_uninit(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    u32 scl, sda;

    scl = iic_get_scl(iic);
    sda = iic_get_sda(iic);
    hw_iic_set_die(iic, 0);
    if (id == 0) {
        gpio_set_hd(scl, 0);
        gpio_set_hd(sda, 0);
        gpio_set_pull_up(scl, 0);
        gpio_set_pull_up(sda, 0);
    }
    iic_disable(iic_regs[id]);
}

void hw_iic_start(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    iic_preset_restart(iic_regs[id]);
    //don't add iic_cfg_done() here, it must be used with send byte
}

void hw_iic_stop(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);
    iic_preset_end(iic_regs[id]);
    iic_cfg_done(iic_regs[id]);
}

u8 hw_iic_tx_byte(hw_iic_dev iic, u8 byte)
{
    u8 id = iic_get_id(iic);
    iic_dir_out(iic_regs[id]);
    iic_buf_reg(iic_regs[id]) = byte;
    iic_cfg_done(iic_regs[id]);
    /* putchar('a'); */
    while (!iic_pnd(iic_regs[id]));
    iic_pnd_clr(iic_regs[id]);
    /* putchar('b'); */
    return iic_send_is_ack(iic_regs[id]);
}

u8 hw_iic_rx_byte(hw_iic_dev iic, u8 ack)
{
    u8 id = iic_get_id(iic);
    iic_dir_in(iic_regs[id]);
    if (ack) {
        iic_recv_ack(iic_regs[id]);
    } else {
        iic_recv_nack(iic_regs[id]);
    }
    iic_buf_reg(iic_regs[id]) = 0xff;
    iic_cfg_done(iic_regs[id]);
    /* putchar('c'); */
    while (!iic_pnd(iic_regs[id]));
    iic_pnd_clr(iic_regs[id]);
    /* putchar('d'); */
    return iic_buf_reg(iic_regs[id]);
}

int hw_iic_read_buf(hw_iic_dev iic, void *buf, int len)
{
    u8 id = iic_get_id(iic);
    int i;

    if (!buf || !len) {
        return -1;
    }
    iic_dir_in(iic_regs[id]);
    iic_recv_ack(iic_regs[id]);
    for (i = 0; i < len; i++) {
        if (i == len - 1) {
            iic_recv_nack(iic_regs[id]);
        }
        iic_buf_reg(iic_regs[id]) = 0xff;
        iic_cfg_done(iic_regs[id]);
        while (!iic_pnd(iic_regs[id]));
        iic_pnd_clr(iic_regs[id]);
        ((u8 *)buf)[i] = iic_buf_reg(iic_regs[id]);
    }
    return len;
}

int hw_iic_write_buf(hw_iic_dev iic, const void *buf, int len)
{
    u8 id = iic_get_id(iic);
    int i = 0;

    if (!buf || !len) {
        return -1;
    }
    iic_dir_out(iic_regs[id]);
    for (i = 0; i < len; i++) {
        iic_buf_reg(iic_regs[id]) = ((u8 *)buf)[i];
        iic_cfg_done(iic_regs[id]);
        while (!iic_pnd(iic_regs[id]));
        iic_pnd_clr(iic_regs[id]);
        if (!iic_send_is_ack(iic_regs[id])) {
            break;
        }
    }
    return i;
}

void hw_iic_set_ie(hw_iic_dev iic, u8 en)
{
    u8 id = iic_get_id(iic);

    if (en) {
        iic_set_ie(iic_regs[id]);
    } else {
        iic_clr_ie(iic_regs[id]);
    }
}

u8 hw_iic_get_pnd(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);

    return !!iic_pnd(iic_regs[id]);
}

void hw_iic_clr_pnd(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);

    iic_pnd_clr(iic_regs[id]);
}

void hw_iic_set_end_ie(hw_iic_dev iic, u8 en)
{
    u8 id = iic_get_id(iic);

    if (en) {
        iic_set_end_ie(iic_regs[id]);
    } else {
        iic_clr_end_ie(iic_regs[id]);
    }
}

u8 hw_iic_get_end_pnd(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);

    return !!iic_end_pnd(iic_regs[id]);
}

void hw_iic_clr_end_pnd(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);

    iic_end_pnd_clr(iic_regs[id]);
}

void hw_iic_slave_set_addr(hw_iic_dev iic, u8 addr, u8 addr_ack)
{
    u8 id = iic_get_id(iic);

    iic_baud_reg(iic_regs[id]) = (addr & 0xfe) | !!addr_ack;
}

void hw_iic_slave_rx_prepare(hw_iic_dev iic, u8 ack)
{
    u8 id = iic_get_id(iic);

    iic_dir_in(iic_regs[id]);
    if (ack) {
        iic_recv_ack(iic_regs[id]);
    } else {
        iic_recv_nack(iic_regs[id]);
    }
    iic_buf_reg(iic_regs[id]) = 0xff;
    iic_cfg_done(iic_regs[id]);
}

u8 hw_iic_slave_rx_byte(hw_iic_dev iic, bool *is_start_addr)
{
    u8 id = iic_get_id(iic);
    if (iic_start_pnd(iic_regs[id])) {
        iic_start_pnd_clr(iic_regs[id]);
        is_start_addr ? (*is_start_addr = 1) : 0;
    } else {
        is_start_addr ? (*is_start_addr = 0) : 0;
    }
    return iic_buf_reg(iic_regs[id]);
}

void hw_iic_slave_tx_byte(hw_iic_dev iic, u8 byte)
{
    u8 id = iic_get_id(iic);

    iic_dir_out(iic_regs[id]);
    iic_buf_reg(iic_regs[id]) = byte;
    iic_cfg_done(iic_regs[id]);
}

u8 hw_iic_slave_tx_check_ack(hw_iic_dev iic)
{
    u8 id = iic_get_id(iic);

    return iic_send_is_ack(iic_regs[id]);
}
void iic_disable_for_ota()
{
    JL_IIC->CON0 = 0;
}

