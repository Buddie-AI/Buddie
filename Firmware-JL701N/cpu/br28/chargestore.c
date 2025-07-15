#include "generic/typedef.h"
#include "generic/gpio.h"
#include "asm/power/p33.h"
#include "asm/power/p33_app.h"
#include "asm/hwi.h"
#include "asm/gpio.h"
#include "asm/clock.h"
#include "asm/charge.h"
#include "asm/chargestore.h"
#include "update.h"
#include "app_config.h"

#if (TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE)
struct chargestore_handle {
    const struct chargestore_platform_data *data;
    JL_UART_TypeDef *UART;
    u32 baudrate;
};
#define DMA_ISR_LEN 64
#define DMA_BUF_LEN 64
#define __this  (&hdl)
static struct chargestore_handle hdl;
u8 uart_dma_buf[DMA_BUF_LEN] __attribute__((aligned(4)));
volatile u8 send_busy;

//串口时钟和串口超时时钟是分开的
#define UART_SRC_CLK    clk_get("uart")
#define UART_OT_CLK     clk_get("lsb")

enum {
    UPGRADE_NULL = 0,
    UPGRADE_USB_HARD_KEY,
    UPGRADE_USB_SOFTKEY,
    UPGRADE_UART_SOFT_KEY,
    UPGRADE_UART_ONE_WIRE_HARD_KEY,
};
extern void charge_reset_pb5_pd_status(void);
extern void nvram_set_boot_state(u32 state);
void update_param_boot_ram_set(u8 *buf, u16 len);

void chargestore_set_update_ram(void)
{
    //需要补充设置ram
    int tmp;
    __asm__ volatile("%0 =icfg" : "=r"(tmp));
    tmp &= ~(3 << 8);
    __asm__ volatile("icfg = %0" :: "r"(tmp));//GIE1
    local_irq_disable();
#if TCFG_TEST_BOX_ANY_IO_UPDATE
    update_param_boot_ram_set("UART_UPDATE_CUSTOM", sizeof("UART_UPDATE_CUSTOM"));
#else
    nvram_set_boot_state(UPGRADE_UART_SOFT_KEY);
#endif
}

void __attribute__((weak)) chargestore_data_deal(u8 cmd, u8 *data, u8 len)
{

}

void __attribute__((weak)) chargestore_uart_data_deal(u8 *data, u8 len)
{

}

static u8 chargestore_get_f95_det_res(u32 equ_res)
{
    u8 det_res = (equ_res + 50) / 100;
    if (det_res > 0) {
        det_res -= 1;
    }
    if (det_res > 0x0f) {
        det_res = 0x0f;
    }
    return det_res;
}

//br30(PMU数据未知,IC还未给出测试数据,说是和BR23基本一致)
u8 chargestore_get_det_level(u8 chip_type)
{
    u32 res = 1600;
    switch (chip_type) {
    case TYPE_F95:
        if (IS_L5V_LOAD_EN()) {
            res = (GET_L5V_RES_DET_S_SEL() + 1) * 50;
        }
        return chargestore_get_f95_det_res(res);
    case TYPE_NORMAL:
    default:
        return 0x0f;
    }
}

___interrupt
static void uart_isr(void)
{
    u16 i;
    u32 rx_len = 0;
    if (__this->data->io_port == IO_PORT_DP) {
        //测试盒选DP，跟USB功能复用
        local_irq_disable();
    }
    if ((__this->UART->CON0 & BIT(2)) && (__this->UART->CON0 & BIT(15))) {
        __this->UART->CON0 |= BIT(13);
        send_busy = 0;
        chargestore_data_deal(CMD_COMPLETE, NULL, 0);
    }
    if ((__this->UART->CON0 & BIT(3)) && (__this->UART->CON0 & BIT(14))) {
        __this->UART->CON0 |= BIT(12);//清RX PND
        chargestore_data_deal(CMD_RECVDATA, uart_dma_buf, DMA_ISR_LEN);
        memset((void *)uart_dma_buf, 0, sizeof(uart_dma_buf));
        __this->UART->RXSADR = (u32)uart_dma_buf;
        __this->UART->RXEADR = (u32)(uart_dma_buf + DMA_BUF_LEN);
        __this->UART->RXCNT = DMA_ISR_LEN;
    }
    if ((__this->UART->CON0 & BIT(5)) && (__this->UART->CON0 & BIT(11))) {
        //OTCNT PND
        __this->UART->CON0 |= BIT(7);//DMA模式
        __this->UART->CON0 |= BIT(10);//清OTCNT PND
        asm volatile("nop");
        rx_len = __this->UART->HRXCNT;//读当前串口接收数据的个数
        __this->UART->CON0 |= BIT(12);//清RX PND(这里的顺序不能改变，这里要清一次)
        chargestore_data_deal(CMD_RECVDATA, uart_dma_buf, rx_len);
        chargestore_uart_data_deal(uart_dma_buf, rx_len);
        memset((void *)uart_dma_buf, 0, sizeof(uart_dma_buf));
        __this->UART->RXSADR = (u32)uart_dma_buf;
        __this->UART->RXEADR = (u32)(uart_dma_buf + DMA_BUF_LEN);
        __this->UART->RXCNT = DMA_ISR_LEN;
    }
    if (__this->data->io_port == IO_PORT_DP) {
        //测试盒选DP，跟USB功能复用
        local_irq_enable();
    }
}

void chargestore_write(u8 *data, u8 len)
{
    ASSERT(((u32)data) % 4 == 0); //4byte对齐
    send_busy = 1;
    __this->UART->TXADR = (u32)data;
    __this->UART->TXCNT = len;
}

void chargestore_open(u8 mode)
{
    __this->UART->CON0 = BIT(13) | BIT(12) | BIT(10);
    if (mode == MODE_RECVDATA) {
        charge_set_ldo5v_detect_stop(0);
        gpio_direction_input(__this->data->io_port);
        gpio_set_die(__this->data->io_port, 1);
        __this->UART->CON1 &= ~BIT(4);
        if (__this->UART == JL_UART0) {
            gpio_set_fun_input_port(__this->data->io_port, PFI_UART0_RX, LOW_POWER_FREE);
        } else if (__this->UART == JL_UART1) {
            gpio_set_fun_input_port(__this->data->io_port, PFI_UART1_RX, LOW_POWER_FREE);
        } else {
            gpio_set_fun_input_port(__this->data->io_port, PFI_UART2_RX, LOW_POWER_FREE);
        }
        memset((void *)uart_dma_buf, 0, sizeof(uart_dma_buf));
        __this->UART->RXSADR = (u32)uart_dma_buf;
        __this->UART->RXEADR = (u32)(uart_dma_buf + DMA_BUF_LEN);
        __this->UART->RXCNT = DMA_ISR_LEN;
        __this->UART->CON0 |= BIT(6) | BIT(5) | BIT(3);
    } else {
        charge_set_ldo5v_detect_stop(1);
        gpio_direction_output(__this->data->io_port, 1);
        gpio_set_hd(__this->data->io_port, 1);
        __this->UART->CON1 |= BIT(4);
        if (__this->UART == JL_UART0) {
            gpio_set_fun_output_port(__this->data->io_port, FO_UART0_TX, 1, 1, LOW_POWER_FREE);
        } else if (__this->UART == JL_UART1) {
            gpio_set_fun_output_port(__this->data->io_port, FO_UART1_TX, 1, 1, LOW_POWER_FREE);
        } else {
            gpio_set_fun_output_port(__this->data->io_port, FO_UART2_TX, 1, 1, LOW_POWER_FREE);

        }
        __this->UART->CON0 |= BIT(2);
    }
    __this->UART->CON0 |= BIT(13) | BIT(12) | BIT(10) | BIT(1) | BIT(0);
}

void chargestore_close(void)
{
    __this->UART->CON0 = BIT(13) | BIT(12) | BIT(10) | BIT(0);
    gpio_set_pull_down(__this->data->io_port, 0);
    gpio_set_pull_up(__this->data->io_port, 0);
    gpio_set_die(__this->data->io_port, 1);
    gpio_set_hd(__this->data->io_port, 0);
    gpio_direction_input(__this->data->io_port);
    memset((void *)uart_dma_buf, 0, sizeof(uart_dma_buf));
    charge_set_ldo5v_detect_stop(0);
}

void chargestore_set_baudrate(u32 baudrate)
{
    u32 uart_timeout;
    __this->baudrate = baudrate;
    uart_timeout = 20 * 1000000 / __this->baudrate;
    __this->UART->OTCNT = uart_timeout * (UART_OT_CLK / 1000000);
    __this->UART->BAUD = (UART_SRC_CLK / __this->baudrate) / 4 - 1;
}

#if 0
void chargestore_init(const struct chargestore_platform_data *data)
{
    u32 uart_timeout;
    __this->data = (struct chargestore_platform_data *)data;
    ASSERT(data);
    switch (__this->data->uart_irq) {
    case IRQ_UART0_IDX:
        __this->UART = JL_UART0;
        break;
    case IRQ_UART1_IDX:
        __this->UART = JL_UART1;
        break;
    default:
        ASSERT(0, "uart irq(%d) err!\n", __this->data->uart_irq);
        break;
    }
    if (__this->UART->CON0 & BIT(0)) {
        ASSERT(0, "uart used!\n");
    }
    uart_timeout = 20 * 1000000 / __this->data->baudrate;
    __this->UART->CON0 = BIT(13) | BIT(12) | BIT(10);
    __this->UART->OTCNT = uart_timeout * (clk_get("uart") / 1000000);
    __this->UART->BAUD = (clk_get("uart") / __this->data->baudrate) / 4 - 1;
    __this->UART->CON1 = (((clk_get("uart") / __this->data->baudrate) % 4) << 4);
    gpio_set_pull_down(__this->data->io_port, 0);
    gpio_set_pull_up(__this->data->io_port, 0);
    gpio_set_die(__this->data->io_port, 1);
    gpio_direction_input(__this->data->io_port);
    request_irq(__this->data->uart_irq, 2, uart_isr, 0);
    JL_IOMAP->CON1 &= ~(0xf << 8);
    //约定:
    //使用UART0时,使用output channel0 和 input channel 0
    //使用UART1时,使用output channel1 和 input channel 1 //由于irflt需要用到INPUT_CH1,所以串口1输入改成INPUT_CH3
    if (__this->UART == JL_UART0) {
        //不占用IO
        JL_IOMAP->CON3 &= ~BIT(3);
        //100:input channel0
        //101:input channel1
        //110:input channel2
        //111:input channel3
        JL_IOMAP->CON3 &= ~(0b111 << 0);
        JL_IOMAP->CON3 |= (0b100 << 0);
        //[29:24]:input channel 3 io sel
        //[21:16]:input channel 2 io sel
        //[13:08]:input channel 1 io sel
        //[05:00]:input channel 0 io sel
        JL_IOMAP->CON2 &= ~(0b111111 << 0);
        JL_IOMAP->CON2 |= (__this->data->io_port << 0);
        //000:output channel 0 sel UT0_TX
        //001:output channel 0 sel UT1_TX
        JL_IOMAP->CON1 &= ~(0b1111 << 8);
        JL_IOMAP->CON1 |= (0 << 8);//outchl0 -> ut0_tx
    } else {
        //不占用IO
        JL_IOMAP->CON3 &= ~BIT(7);
        //100:input channel0
        //101:input channel1
        //110:input channel2
        //111:input channel3
        JL_IOMAP->CON3 &= ~(0b111 << 4);
        JL_IOMAP->CON3 |= (0b111 << 4);
        //[29:24]:input channel 3 io sel
        //[21:16]:input channel 2 io sel
        //[13:08]:input channel 1 io sel
        //[05:00]:input channel 0 io sel
        JL_IOMAP->CON2 &= ~(0b111111 << 24);
        JL_IOMAP->CON2 |= (__this->data->io_port << 24);
        //000:output channel 1 sel UT0_TX
        //001:output channel 1 sel UT1_TX
        JL_IOMAP->CON3 &= ~(0b1111 << 20);
        JL_IOMAP->CON3 |= (1 << 20);//outchl1 -> ut1_tx
    }
}
#endif

#if 1
void chargestore_init(const struct chargestore_platform_data *data)
{
    u32 uart_timeout;
    __this->data = (struct chargestore_platform_data *)data;
    ASSERT(data);
    if (!(JL_UART0->CON0 & BIT(0))) {
        JL_UART0->CON0 = BIT(13) | BIT(12) | BIT(10);
        request_irq(IRQ_UART0_IDX, 2, uart_isr, 0);
        __this->UART = JL_UART0;
    } else if (!(JL_UART1->CON0 & BIT(0))) {
        JL_UART1->CON0 = BIT(13) | BIT(12) | BIT(10);
        request_irq(IRQ_UART1_IDX, 2, uart_isr, 0);
        __this->UART = JL_UART1;
    } else if (!(JL_UART2->CON0 & BIT(0))) {
        JL_UART2->CON0 = BIT(13) | BIT(12) | BIT(10);
        request_irq(IRQ_UART2_IDX, 2, uart_isr, 0);
        __this->UART = JL_UART2;
    } else {
        ASSERT(0, "uart all used!\n");
    }
    send_busy = 0;
    uart_timeout = 20 * 1000000 / __this->data->baudrate;
    __this->UART->CON0 = BIT(13) | BIT(12) | BIT(10) | BIT(1) | BIT(0);//占用该串口,不被其他模块使用
    __this->UART->OTCNT = uart_timeout * (UART_OT_CLK / 1000000);
    __this->UART->BAUD = (UART_SRC_CLK / __this->data->baudrate) / 4 - 1;
    __this->baudrate = __this->data->baudrate;
    gpio_set_pull_down(__this->data->io_port, 0);
    gpio_set_pull_up(__this->data->io_port, 0);
    gpio_set_die(__this->data->io_port, 1);
    gpio_direction_input(__this->data->io_port);
}
#endif

static void clock_critical_enter(void)
{
    u8 cmp_buf[2] = {0x55, 0xAA};
    //等待数据收完
    extern void *memmem(void *srcmem, int src_len, void *desmem, int des_len);
    while (memmem(uart_dma_buf, sizeof(uart_dma_buf), cmp_buf, sizeof(cmp_buf)));
    //等待数据发完
    while (send_busy);
}
static void clock_critical_exit(void)
{
    u32 uart_timeout;
    if (__this->UART == NULL) {
        return;
    }
    uart_timeout = 20 * 1000000 / __this->baudrate;
    __this->UART->OTCNT = uart_timeout * (UART_OT_CLK / 1000000);
    __this->UART->BAUD = (UART_SRC_CLK / __this->baudrate) / 4 - 1;
}
CLOCK_CRITICAL_HANDLE_REG(chargestore, clock_critical_enter, clock_critical_exit)

#endif

