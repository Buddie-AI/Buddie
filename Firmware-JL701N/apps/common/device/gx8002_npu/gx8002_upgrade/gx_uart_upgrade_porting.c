#include "includes.h"
#include "asm/uart_dev.h"
#include "gx_uart_upgrade_porting.h"
#include "../gx8002_npu_api.h"
#include "app_config.h"

#if GX8002_UPGRADE_TOGGLE

#define GX_UART_UPGRADE_PORT_DEBUG  	1

#ifdef GX_UART_UPGRADE_PORT_DEBUG
//#define upgrade_debug(fmt, ...) y_printf("[gx_uart_upgrade]: " fmt, ##__VA_ARGS__)
#define gx_uart_upgrade_port_debug 			y_printf
#else
#define gx_uart_upgrade_port_debug(fmt, ...)
#endif

struct gx_upgrade_uart_port {
    const uart_bus_t *uart_bus;
    u32 cur_baud;
};

static u8 uart_upgrade_cbuf[512] __attribute__((aligned(4)));

static struct gx_upgrade_uart_port uart_port = {0};

#define __this 		(&uart_port)

static void gx8002_upgrade_uart_isr_hook(void *arg, u32 status)
{
    if (status != UT_TX) {
        //gx_uart_upgrade_port_debug("UT_RX_OT pending");
    }
}


int gx_upgrade_uart_porting_open(int baud, int databits, int stopbits, int parity)
{
    int ret = -1;
    struct uart_platform_data_t u_arg = {0};

    if (__this->uart_bus) {
        gx_uart_upgrade_port_debug("gx8002 uart upgrade have open");
        return 0;
    }

    u_arg.tx_pin = TCFG_GX8002_NPU_UART_TX_PORT;
    u_arg.rx_pin = TCFG_GX8002_NPU_UART_RX_PORT;
    u_arg.rx_cbuf = uart_upgrade_cbuf;
    u_arg.rx_cbuf_size = sizeof(uart_upgrade_cbuf);
    u_arg.frame_length = 0xFFFF;
    u_arg.rx_timeout = 5;
    u_arg.isr_cbfun = gx8002_upgrade_uart_isr_hook;
    u_arg.baud = baud;
    u_arg.is_9bit = 0;

    __this->uart_bus = uart_dev_open(&u_arg);
    if (__this->uart_bus != NULL) {
        gx_uart_upgrade_port_debug("gx8002 uart upgrade open: baud: %d", baud);
        ret = 0;
    } else {
        gx_uart_upgrade_port_debug("gx8002 uart upgrade open fail");
    }

    return ret;
}


int gx_upgrade_uart_porting_close(void)
{
    if (__this->uart_bus) {
        uart_dev_close(__this->uart_bus);
        gpio_disable_fun_output_port(TCFG_GX8002_NPU_UART_TX_PORT);
        __this->uart_bus = NULL;
    }

    return 0;
}

int gx_upgrade_uart_porting_read(unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int ret = -1;

    if (__this->uart_bus && __this->uart_bus->read) {
        ret = __this->uart_bus->read(buf, len, timeout_ms);
    }

    return ret;
}

//int gx_upgrade_uart_porting_write(const unsigned char *buf, unsigned int len, unsigned int timeout_ms)
int gx_upgrade_uart_porting_write(const unsigned char *buf, unsigned int len)
{
    int ret = len;

    if (__this->uart_bus && __this->uart_bus->write) {
        __this->uart_bus->write(buf, len);
    }

    return ret;
}

int gx_uart_upgrade_porting_wait_reply(const unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int index = 0;
    unsigned char ch;

    while (gx_upgrade_uart_porting_read(&ch, 1, timeout_ms) > 0) {
        //gx_uart_upgrade_port_debug("`0x%02x`\n", ch);
        if (ch == (unsigned char)buf[index]) {
            index++;
        }
        if (index == len) {
            return 0;
        }
    }

    return -1;
}

static void uart_clock_critical_enter(void)
{

}

static void uart_clock_critical_exit(void)
{
    if (__this->uart_bus && __this->uart_bus->set_baud) {
        gx_uart_upgrade_port_debug("clock critical uart set_baud: %d", __this->cur_baud);
        __this->uart_bus->set_baud(__this->cur_baud);
    }
}
CLOCK_CRITICAL_HANDLE_REG(gx8002, uart_clock_critical_enter, uart_clock_critical_exit)


#endif /* #if GX8002_UPGRADE_TOGGLE */
