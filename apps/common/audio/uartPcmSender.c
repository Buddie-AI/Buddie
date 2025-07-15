#include "app_config.h"

#ifdef AUDIO_PCM_DEBUG
#include "system/includes.h"
#include "asm/uart_dev.h"
#include "uartPcmSender.h"

static u8 uart_cbuf[32] __attribute__((aligned(4)));
const uart_bus_t *uart_bus = NULL;

//设备事件响应demo
void uart_event_handler(struct sys_event *e)
{
    u8 uart_rxbuf[12] = {0};
    u8 uart_txbuf[12] = {0};
    const uart_bus_t *_uart_bus;
    u32 uart_rxcnt = 0;
    u8 i = 0;

    if (0) {//!strcmp(e->arg, "uart_rx_overflow")) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event: %s\n", e->arg);
            _uart_bus = (const uart_bus_t *)e->u.dev.value;
            uart_rxcnt = _uart_bus->read(uart_rxbuf, sizeof(uart_rxbuf), 0);
            if (uart_rxcnt) {
                g_printf("rx:%s", uart_rxbuf);
            }
        }
    }
    if (0) { //(!strcmp(e->arg, "uart_rx_outtime")) {
        if (e->u.dev.event == DEVICE_EVENT_CHANGE) {
            printf("uart event: %s\n", e->arg);
            _uart_bus = (const uart_bus_t *)e->u.dev.value;
            uart_rxcnt = _uart_bus->read(uart_rxbuf, sizeof(uart_rxbuf), 0);
            if (uart_rxcnt) {
                printf("get_buffer:\n");
                for (int i = 0; i < uart_rxcnt; i++) {
                    putbyte(uart_rxbuf[i]);
                    if (i % 16 == 15) {
                        putchar('\n');
                    }
                }
                if (uart_rxcnt % 16) {
                    putchar('\n');
                }
                _uart_bus->write(uart_rxbuf, uart_rxcnt);
            }
            printf("uart out\n");
        }
    }
}
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, uart_event_handler, 0);

void gpio_change(void *priv);
static void uart_isr_hook(void *arg, u32 status)
{
    const uart_bus_t *ubus = arg;
    struct sys_event e;

    //当CONFIG_UARTx_ENABLE_TX_DMA（x = 0, 1）为1时，不要在中断里面调用ubus->write()，因为中断不能pend信号量
    if (status == UT_RX) {
        printf("uart_rx_isr\n");
#if 1//(UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = "uart_rx_overflow";
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_RX_OT) {
        printf("uart_rx_ot_isr\n");
#if 1//(UART_DEV_USAGE_TEST_SEL == 1)
        e.type = SYS_DEVICE_EVENT;
        e.arg = "uart_rx_outtime";
        e.u.dev.event = DEVICE_EVENT_CHANGE;
        e.u.dev.value = (int)ubus;
        sys_event_notify(&e);
#endif
    }
    if (status == UT_TX) {
        /* putchar('T'); */
        /* gpio_change(0); */
    }
}

void uartSendData(void *buf, u16 len) 			//发送数据的接口。
{
    if (uart_bus) {
        uart_bus->write(buf, len);		//把数据写到DMA
    }
}

/* char *TickPage = "uart online"; */
/* void uartTickSend(void *priv){ */
/* if(uartTickSendFlag){ */
/* r_printf("sizeof:%d\n", sizeof(TickPage)); */
/* uartSendData(TickPage, sizeof(TickPage)); */
/* } */
/* } */

void uartSendInit()
{
    struct uart_platform_data_t u_arg = {0};
    u_arg.tx_pin = PCM_UART1_TX_PORT;
    u_arg.rx_pin = PCM_UART1_RX_PORT;
    u_arg.rx_cbuf = uart_cbuf;
    u_arg.rx_cbuf_size = 32;
    u_arg.frame_length = 6;
    u_arg.rx_timeout = 100;
    u_arg.isr_cbfun = uart_isr_hook;
    u_arg.baud = PCM_UART1_BAUDRATE;
    u_arg.is_9bit = 0;


    r_printf("uart_dev_open() ...\n");
    uart_bus = uart_dev_open(&u_arg);
    r_printf("comming %s,%d\n", __func__, __LINE__);
    if (uart_bus != NULL) {
        r_printf("success\n");
        gpio_set_hd(PCM_UART1_TX_PORT, 1);
        gpio_set_hd0(PCM_UART1_TX_PORT, 1);
        //os_task_create(uart_u_task, (void *)uart_bus, 31, 512, 0, "uart_u_task");
    } else {
        r_printf("false\n");
    }
}

#endif
