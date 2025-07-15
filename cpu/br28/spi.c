
#undef LOG_TAG_CONST
#define LOG_TAG     "[SPI]"
#define LOG_ERROR_ENABLE
//#define LOG_DEBUG_ENABLE
#include "system/debug.h"
#include "generic/gpio.h"
#include "asm/clock.h"
#include "asm/spi.h"


#define spi_enable(reg)                     ((reg)->CON |= BIT(0))
#define spi_disable(reg)                    ((reg)->CON &= ~BIT(0))
#define spi_role_slave(reg)                 ((reg)->CON |= BIT(1))
#define spi_role_master(reg)                ((reg)->CON &= ~BIT(1))
#define spi_cs_en(reg)                      ((reg)->CON |= BIT(2))
#define spi_cs_dis(reg)                     ((reg)->CON &= ~BIT(2))
#define spi_bidir(reg)                      ((reg)->CON |= BIT(3))
#define spi_unidir(reg)                     ((reg)->CON &= ~BIT(3))
#define spi_smp_edge_rise(reg)              ((reg)->CON &= ~BIT(4))
#define spi_smp_edge_fall(reg)              ((reg)->CON |= BIT(4))
#define spi_ud_edge_rise(reg)               ((reg)->CON &= ~BIT(5))
#define spi_ud_edge_fall(reg)               ((reg)->CON |= BIT(5))
#define spi_clk_idle_h(reg)                 ((reg)->CON |= BIT(6))
#define spi_clk_idle_l(reg)                 ((reg)->CON &= ~BIT(6))
#define spi_cs_idle_h(reg)                  ((reg)->CON |= BIT(7))
#define spi_cs_idle_l(reg)                  ((reg)->CON &= ~BIT(7))
#define spi_data_width(reg, x)              ((reg)->CON = (reg->CON&~(3<<10))|((x)<<10))
#define spi_dir_in(reg)                     ((reg)->CON |= BIT(12))
#define spi_dir_out(reg)                    ((reg)->CON &= ~BIT(12))
#define spi_ie_en(reg)                      ((reg)->CON |= BIT(13))
#define spi_ie_dis(reg)                     ((reg)->CON &= ~BIT(13))
#define spi_clr_pnd(reg)                    ((reg)->CON |= BIT(14))
#define spi_pnd(reg)                        ((reg)->CON & BIT(15))

#define spi_w_reg_con(reg, val)             ((reg)->CON = (val))
#define spi_r_reg_con(reg)                  ((reg)->CON)
#define spi_w_reg_buf(reg, val)             ((reg)->BUF = (val))
#define spi_r_reg_buf(reg)                  ((reg)->BUF)
#define spi_w_reg_baud(reg, baud)           ((reg)->BAUD = (baud))
#define spi_r_reg_baud(reg)                 ((reg)->BAUD)
#define spi_w_reg_dma_addr(reg, addr)       ((reg)->ADR  = (addr))
#define spi_w_reg_dma_cnt(reg, cnt)         ((reg)->CNT = (cnt))


#define SPI1_ENABLE   1   //是否使能SPI1，使能后需定义spi1_p_data
#define SPI2_ENABLE   0   //是否使能SPI2，使能后需定义spi2_p_data

static JL_SPI_TypeDef *const spi_regs[SPI_MAX_HW_NUM] = {
    NULL,
    JL_SPI1,
    JL_SPI2,
};

static u8 spi_get_info_id(spi_dev spi)
{
    ASSERT(spi < SPI_MAX_HW_NUM);
    return spi;
}

static u8 *spi_get_info_port(spi_dev spi)
{
    u8 *port = NULL;
    u8 id = spi_get_info_id(spi);
    switch (id) {
#if SPI1_ENABLE
    case SPI1:
        port = (u8 *)spi1_p_data.port;
        break;
#endif
#if SPI2_ENABLE
    case SPI2:
        port = (u8 *)spi2_p_data.port;
        break;
#endif
    default:
        break;
    }
    return port;
}

static u8 spi_get_info_mode(spi_dev spi)
{
    u8 mode = (u8) - 1;
    u8 id = spi_get_info_id(spi);
    switch (id) {
#if SPI1_ENABLE
    case SPI1:
        mode = spi1_p_data.mode;
        break;
#endif
#if SPI2_ENABLE
    case SPI2:
        mode = spi2_p_data.mode;
        break;
#endif
    default:
        break;
    }
    return mode;
}

static u8 spi_get_info_role(spi_dev spi)
{
    u8 role = (u8) - 1;
    u8 id = spi_get_info_id(spi);
    switch (id) {
#if SPI1_ENABLE
    case SPI1:
        role = spi1_p_data.role;
        break;
#endif
#if SPI2_ENABLE
    case SPI2:
        role = spi2_p_data.role;
        break;
#endif
    default:
        break;
    }
    return role;
}

static u32 spi_get_info_clock(spi_dev spi)
{
    u32 clock = (u8) - 1;
    u8 id = spi_get_info_id(spi);
    switch (id) {
#if SPI1_ENABLE
    case SPI1:
        clock = spi1_p_data.clk;
        break;
#endif
#if SPI2_ENABLE
    case SPI2:
        clock = spi2_p_data.clk;
        break;
#endif
    default:
        break;
    }
    return clock;
}

static void spi1_iomc_config(spi_dev spi)
{
    u8 *port = spi_get_info_port(spi);
    u8 mode = spi_get_info_mode(spi);
    gpio_set_fun_output_port(port[0], FO_SPI1_CLK, 1, 1, LOW_POWER_FREE);
    gpio_set_fun_output_port(port[1], FO_SPI1_DA0, 1, 1, LOW_POWER_FREE);
    gpio_set_fun_output_port(port[2], FO_SPI1_DA1, 1, 1, LOW_POWER_FREE);
    gpio_set_fun_input_port(port[0], PFI_SPI1_CLK, LOW_POWER_FREE);
    gpio_set_fun_input_port(port[1], PFI_SPI1_DA0, LOW_POWER_FREE);
    gpio_set_fun_input_port(port[2], PFI_SPI1_DA1, LOW_POWER_FREE);
    if (mode == SPI_MODE_UNIDIR_4BIT) {
        gpio_set_fun_output_port(port[3], FO_SPI1_DA2, 1, 1, LOW_POWER_FREE);
        gpio_set_fun_output_port(port[4], FO_SPI1_DA3, 1, 1, LOW_POWER_FREE);
        gpio_set_fun_input_port(port[3], PFI_SPI1_DA2, LOW_POWER_FREE);
        gpio_set_fun_input_port(port[4], PFI_SPI1_DA3, LOW_POWER_FREE);
    }
}

static void spi2_iomc_config(spi_dev spi)
{
    u8 *port = spi_get_info_port(spi);
    u8 mode = spi_get_info_mode(spi);
    gpio_set_fun_output_port(port[0], FO_SPI2_CLK, 1, 1, LOW_POWER_FREE);
    gpio_set_fun_output_port(port[1], FO_SPI2_DA0, 1, 1, LOW_POWER_FREE);
    gpio_set_fun_output_port(port[2], FO_SPI2_DA1, 1, 1, LOW_POWER_FREE);
    gpio_set_fun_input_port(port[0], PFI_SPI2_CLK, LOW_POWER_FREE);
    gpio_set_fun_input_port(port[1], PFI_SPI2_DA0, LOW_POWER_FREE);
    gpio_set_fun_input_port(port[2], PFI_SPI2_DA1, LOW_POWER_FREE);
    if (mode == SPI_MODE_UNIDIR_4BIT) {
        gpio_set_fun_output_port(port[3], FO_SPI2_DA2, 1, 1, LOW_POWER_FREE);
        gpio_set_fun_output_port(port[4], FO_SPI2_DA3, 1, 1, LOW_POWER_FREE);
        gpio_set_fun_input_port(port[3], PFI_SPI2_DA2, LOW_POWER_FREE);
        gpio_set_fun_input_port(port[4], PFI_SPI2_DA3, LOW_POWER_FREE);
    }
}

static void (*pSPI_IOMC_CONFIG[])(spi_dev spi) = {
    NULL,
    spi1_iomc_config,
    spi2_iomc_config,
};


static void spi_io_port_init(u8 port, u8 dir)
{
    if (port != (u8) - 1) {
        if (dir == 1) {
            gpio_set_direction(port, 1);
        } else {
            gpio_set_direction(port, 0);
        }
        gpio_set_die(port, 1);
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 0);
    }
}

static void spi_io_port_uninit(u8 port)
{
    if (port != (u8) - 1) {
        gpio_set_direction(port, 1);
        gpio_set_die(port, 0);
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 0);
    }
}

/*
 * @brief 设置波特率
 * @parm spi  spi句柄
 * @parm baud  波特率
 * @return 0 成功，< 0 失败
 */
int spi_set_baud(spi_dev spi, u32 baud)
{
    //SPICK = sysclk / (SPIx_BAUD + 1)
    //=> SPIx_BAUD = sysclk / SPICK - 1
    u8 id = spi_get_info_id(spi);
    u32 sysclk;

    sysclk = clk_get("spi");
    log_debug("spi clock source freq %lu", sysclk);
    if (sysclk < baud) {
        spi_w_reg_baud(spi_regs[id], 0);
        return -EINVAL;
    }
    spi_w_reg_baud(spi_regs[id], sysclk / baud - 1);
    return 0;
}

/*
 * @brief 获取波特率
 * @parm spi  spi句柄
 * @return  波特率
 */
u32 spi_get_baud(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    return spi_get_info_clock(spi);
}

static int __spi_wait_ok(spi_dev spi, u32 n)
{
    u8 id = spi_get_info_id(spi);
    u32 baud = spi_get_info_clock(spi);
    baud = clk_get("spi") / baud - 1;
    u32 retry = baud * (clk_get("sys") / clk_get("spi")) * 8 * n * 5;  //500% spi baudate

    while (!spi_pnd(spi_regs[id])) {
        __asm__ volatile("nop");
        if (--retry == 0) {
            log_error("spi wait pnd timeout");
            return -EFAULT;
        }
    }
    spi_clr_pnd(spi_regs[id]);
    return 0;
}

/*
 * @brief 发送1个字节
 * @parm spi  spi句柄
 * @parm byte 发送的字节
 * @return 0 成功，< 0 失败
 */
int spi_send_byte(spi_dev spi, u8 byte)
{
    u8 id = spi_get_info_id(spi);

    spi_dir_out(spi_regs[id]);
    spi_w_reg_buf(spi_regs[id], byte);
    return __spi_wait_ok(spi, 1);
}

/*
 * @brief 发送1个字节，不等待pnd，用于中断
 * @parm spi  spi句柄
 * @parm byte 发送的字节
 * @return null
 */
void spi_send_byte_for_isr(spi_dev spi, u8 byte)
{
    u8 id = spi_get_info_id(spi);

    spi_dir_out(spi_regs[id]);
    spi_w_reg_buf(spi_regs[id], byte);
}

/*
 * @brief 接收1个字节
 * @parm spi  spi句柄
 * @parm err  返回错误信息，若err为非空指针，0 成功，< 0 失败，若为空指针，忽略
 * @return 接收的字节
 */
u8 spi_recv_byte(spi_dev spi, int *err)
{
    u8 id = spi_get_info_id(spi);
    int ret;

    spi_dir_in(spi_regs[id]);
    spi_w_reg_buf(spi_regs[id], 0xff);
    ret = __spi_wait_ok(spi, 1);
    if (ret) {
        err != NULL ? *err = ret : 0;
        return 0;
    }
    err != NULL ? *err = 0 : 0;
    return spi_r_reg_buf(spi_regs[id]);
}


/*
 * @brief 接收1个字节，不等待pnd，用于中断
 * @parm spi  spi句柄
 * @return 接收的字节
 */
u8 spi_recv_byte_for_isr(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);

    spi_dir_in(spi_regs[id]);
    return spi_r_reg_buf(spi_regs[id]);
}

/*
 * @brief 发送并接收1个字节，在8个时钟内完成，仅使用于SPI_MODE_BIDIR_1BIT
 * @parm spi  spi句柄
 * @parm byte  发送的字节
 * @parm err  返回错误信息，若err为非空指针，0 成功，< 0 失败，若为空指针，忽略
 * @return 接收的字节
 */
u8 spi_send_recv_byte(spi_dev spi, u8 byte, int *err)
{
    u8 id = spi_get_info_id(spi);
    int ret;

    spi_w_reg_buf(spi_regs[id], byte);
    ret = __spi_wait_ok(spi, 1);
    if (ret) {
        err != NULL ? *err = ret : 0;
        return 0;
    }
    err != NULL ? *err = 0 : 0;
    return spi_r_reg_buf(spi_regs[id]);
}

/*
 * @brief 设置spi[单向/双向，位数]模式
 * @parm spi  spi句柄
 * @parm mode  模式
 * @return null
 */
void spi_set_bit_mode(spi_dev spi, int mode)
{
    u8 id = spi_get_info_id(spi);
    u8 *port = spi_get_info_port(spi);
    u8 role = spi_get_info_role(spi);

    switch (mode) {
    case SPI_MODE_BIDIR_1BIT:
        spi_bidir(spi_regs[id]);
        spi_data_width(spi_regs[id], 0);
        break;
    case SPI_MODE_UNIDIR_1BIT:
        spi_unidir(spi_regs[id]);
        spi_data_width(spi_regs[id], 0);
        break;
    case SPI_MODE_UNIDIR_2BIT:
        spi_unidir(spi_regs[id]);
        spi_data_width(spi_regs[id], 1);
        break;
    case SPI_MODE_UNIDIR_4BIT:
        spi_unidir(spi_regs[id]);
        spi_data_width(spi_regs[id], 2);
        break;
    }

    spi_io_port_init(port[0], role == SPI_ROLE_MASTER ? 0 : 1);
    if (mode == SPI_MODE_BIDIR_1BIT) {
        spi_io_port_init(port[1], 0);
        spi_io_port_init(port[2], 1);
    } else if (mode == SPI_MODE_UNIDIR_2BIT) {
        spi_io_port_init(port[1], role == SPI_ROLE_MASTER ? 0 : 1);
        spi_io_port_init(port[2], role == SPI_ROLE_MASTER ? 0 : 1);
    } else if (mode == SPI_MODE_UNIDIR_4BIT) {
        spi_io_port_init(port[1], role == SPI_ROLE_MASTER ? 0 : 1);
        spi_io_port_init(port[2], role == SPI_ROLE_MASTER ? 0 : 1);
        spi_io_port_init(port[3], role == SPI_ROLE_MASTER ? 0 : 1);
        spi_io_port_init(port[4], role == SPI_ROLE_MASTER ? 0 : 1);
    }
}

/*
 * @brief 打开spi
 * @parm spi  spi句柄
 * @return 0 成功，< 0 失败
 */
int spi_open(spi_dev spi)
{
    int err;
    u8 id = spi_get_info_id(spi);
    u8 mode = spi_get_info_mode(spi);
    u8 role = spi_get_info_role(spi);
    u32 clock = spi_get_info_clock(spi);

    pSPI_IOMC_CONFIG[id](spi);

    spi_set_bit_mode(spi, mode);
    spi_cs_dis(spi_regs[id]);
    if (role == SPI_ROLE_MASTER) {
        spi_role_master(spi_regs[id]);
    } else if (role == SPI_ROLE_SLAVE) {
        spi_role_slave(spi_regs[id]);
    }
    spi_smp_edge_rise(spi_regs[id]);
    spi_ud_edge_fall(spi_regs[id]);
    spi_cs_idle_h(spi_regs[id]);
    spi_clk_idle_l(spi_regs[id]);
    spi_clr_pnd(spi_regs[id]);
    if ((err = spi_set_baud(spi, clock))) {
        log_error("invalid spi baudrate");
        /* return 0; */
    }
    if (role == SPI_ROLE_MASTER) {
        spi_w_reg_buf(spi_regs[id], 0);//设定spi初始化后DO口默认电平为低
    } else if (role == SPI_ROLE_SLAVE) {
        spi_w_reg_buf(spi_regs[id], 0xff);
    }
    spi_enable(spi_regs[id]);

#if 0
    printf("spi%d clk     = %d\n", id, clock);
    printf("spi%d mode    = %d\n", id, mode);
    printf("spi%d role    = %d\n", id, role);
    printf("spi%d clk_pin = %d\n", id, port[0]);
    printf("spi%d do_pin  = %d\n", id, port[1]);
    printf("spi%d di_pin  = %d\n", id, port[2]);
    printf("spi%d CON     = %04x\n", id, spi_r_reg_con(spi_regs[id]));
    printf("spi%d IOMC1   = %08x\n", id, JL_IOMAP->CON1);
#endif
    return 0;
}

/*
 * @brief spi dma接收
 * @parm spi  spi句柄
 * @parm buf  接收缓冲区基地址
 * @parm len  期望接收长度
 * @return 实际接收长度，< 0表示失败
 */
int spi_dma_recv(spi_dev spi, void *buf, u32 len)
{
    u8 id = spi_get_info_id(spi);

    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    spi_dir_in(spi_regs[id]);
    spi_w_reg_dma_addr(spi_regs[id], (u32)buf);
    spi_w_reg_dma_cnt(spi_regs[id], len);
    asm("csync");
    if (__spi_wait_ok(spi, len)) {
        return -EFAULT;
    }
    return len;
}

/*
 * @brief spi dma发送
 * @parm spi  spi句柄
 * @parm buf  发送缓冲区基地址
 * @parm len  期望发送长度
 * @return 实际发送长度，< 0表示失败
 */
int spi_dma_send(spi_dev spi, const void *buf, u32 len)
{
    u8 id = spi_get_info_id(spi);

    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    spi_dir_out(spi_regs[id]);
    spi_w_reg_dma_addr(spi_regs[id], (u32)buf);
    spi_w_reg_dma_cnt(spi_regs[id], len);
    asm("csync");
    if (__spi_wait_ok(spi, len)) {
        return -EFAULT;
    }
    return len;
}

/*
 * @brief spi 配置dma，不等待pnd，用于中断
 * @parm spi  spi句柄
 * @parm buf  缓冲区基地址
 * @parm len  期望长度
 * @parm rw  1 接收 / 0 发送
 * @return null
 */
void spi_dma_set_addr_for_isr(spi_dev spi, void *buf, u32 len, u8 rw)
{
    u8 id = spi_get_info_id(spi);

    /* ASSERT((u32)buf % 4 == 0, "spi dma addr need 4-aligned"); */
    rw ? spi_dir_in(spi_regs[id]) : spi_dir_out(spi_regs[id]);
    spi_w_reg_dma_addr(spi_regs[id], (u32)buf);
    spi_w_reg_dma_cnt(spi_regs[id], len);
}

/*
 * @brief 中断使能
 * @parm spi  spi句柄
 * @parm en  1 使能，0 失能
 * @return null
 */
void spi_set_ie(spi_dev spi, u8 en)
{
    u8 id = spi_get_info_id(spi);
    en ? spi_ie_en(spi_regs[id]) : spi_ie_dis(spi_regs[id]);
}

/*
 * @brief 判断中断标志
 * @parm spi  spi句柄
 * @return 0 / 1
 */
u8 spi_get_pending(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    return spi_pnd(spi_regs[id]) ? 1 : 0;
}

/*
 * @brief 清除中断标志
 * @parm spi  spi句柄
 * @return null
 */
void spi_clear_pending(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    spi_clr_pnd(spi_regs[id]);
}

/*
 * @brief 关闭spi
 * @parm spi  spi句柄
 * @return null
 */
void spi_close(spi_dev spi)
{
    u8 id = spi_get_info_id(spi);
    u8 *port = spi_get_info_port(spi);
    spi_io_port_uninit(port[0]);
    spi_io_port_uninit(port[1]);
    spi_io_port_uninit(port[2]);
    spi_disable(spi_regs[id]);
}
void spi_disable_for_ota()
{
    for (int i = 0; i < 2; i++) {
        spi_disable(spi_regs[i]);
    }
}
