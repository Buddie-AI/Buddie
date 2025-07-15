#include "system/includes.h"
#include "config/config_interface.h"
#include "system/event.h"
#include "app_online_cfg.h"
#include "asm/uart_dev.h"
#include "app_config.h"
#include "asm/crc16.h"

#define LOG_TAG     "[CI-UART]"
/* #define LOG_ERROR_ENABLE */
/* #define LOG_INFO_ENABLE */
/* #define LOG_DUMP_ENABLE */
#include "debug.h"

#if (TCFG_ONLINE_ENABLE || TCFG_CFG_TOOL_ENABLE)
struct config_uart {
    u32 baudrate;
    int flowcontrol;
    const char *dev_name;
};

struct uart_hdl {

    struct config_uart config;

    uart_bus_t *udev;
    void *dbuf;

    void *pRxBuffer;

    u8 ucRxIndex;
    u8 rx_type;

    u16 data_length;

    void *pTxBuffer;

    void (*packet_handler)(const u8 *packet, int size);
};


typedef struct {
    //head
    u16 preamble;
    u8  type;
    u16 length;
    u8  crc8;
    u16 crc16;
    u8 payload[0];
} _GNU_PACKED_	uart_packet_t;

#define UART_FORMAT_HEAD  sizeof(uart_packet_t)

typedef struct {
    //head
    u16 preamble0;
    u8  preamble1;
    u16 crc16;
    u8  length;
    u8 	type;
    u8  sq;
    u8 	payload[0];
} _GNU_PACKED_	uart_tool_packet_t;

#define UART_TOOL_FORMAT_HEAD  sizeof(uart_tool_packet_t)

static void dummy_handler(const u8 *packet, int size);

#define UART_PREAMBLE       		 0xBED6
#define UART_NEW_TOOL_PREAMBLE0      0xAA5A
#define UART_NEW_TOOL_PREAMBLE1      0xA5

#define UART_RX_SIZE        0x100
#define UART_TX_SIZE        0x30
#define UART_DB_SIZE        0x100
#define UART_BAUD_RATE      115200

#ifdef HAVE_MALLOC
static struct uart_hdl *hdl;
#define __this      (hdl)
#else
static struct uart_hdl hdl;
#define __this      (&hdl)

static u8 pRxBuffer_static[UART_RX_SIZE] __attribute__((aligned(4)));       //rx memory
static u8 pTxBuffer_static[UART_TX_SIZE] __attribute__((aligned(4)));       //tx memory
static u8 devBuffer_static[UART_DB_SIZE] __attribute__((aligned(4)));       //dev DMA memory
#endif

s16 get_ci_tx_size()
{
    return UART_TX_SIZE;
}
static u8 procotol = 0;
void ci_data_rx_handler(u8 type)
{
    u16 crc16;
    uart_tool_packet_t *p_newtool;
    uart_packet_t *p;

    __this->rx_type = type;

    if (type == CI_UART && __this->udev) {
        __this->data_length += __this->udev->read(&__this->pRxBuffer[__this->data_length], (UART_RX_SIZE - __this->data_length), 0);//串口读取buf剩余空间的长度，实际长度比buf长，导致越界改写问题
    }

    /* log_info("Rx : %d", __this->data_length); */
    /* log_info_hexdump(__this->pRxBuffer, __this->data_length); */
    if (__this->data_length > UART_RX_SIZE) {
        log_error("Wired");
    }

    u8 *tmp_buf = NULL;
    tmp_buf = __this->pRxBuffer;
    if (__this->data_length >= 2) {
        unsigned i = 0;
        for (i = 0; i < __this->data_length - 1; ++i) {
            if ((tmp_buf[i] == 0x5A) && (tmp_buf[i + 1] == 0xAA) && (tmp_buf[i + 2] == 0xA5)) {
                procotol = 1;
                break;
            } else if (tmp_buf[i] == 0xD6 && tmp_buf[i + 1] == 0xBE) {
                procotol = 0;
                break;
            }
        }
        if (i != 0) {
            __this->data_length -= i;
            /* printf("__this->data_length %d i %d\n", __this->data_length, i); */
            if (__this->data_length > 0) {
                memmove(&__this->pRxBuffer[0], &tmp_buf[i], __this->data_length);
            }
        }
    }

    if (procotol) {
        if (__this->data_length <= UART_TOOL_FORMAT_HEAD) {
            return;
        }
        p_newtool = __this->pRxBuffer;

        if ((p_newtool->preamble0 != UART_NEW_TOOL_PREAMBLE0) || (p_newtool->preamble1 != UART_NEW_TOOL_PREAMBLE1)) {
            log_error("preamble err\n");
            log_info_hexdump(__this->pRxBuffer, __this->data_length);
            goto reset_buf;
        }

        if (__this->data_length >= p_newtool->length + 6) {
            crc16 = crc_get_16bit(&p_newtool->length, p_newtool->length + 1);
            /* log_info("CRC16 0x%x / 0x%x", crc16, p_newtool->crc16); */
            if (p_newtool->crc16 != crc16) {
                log_error("crc16 err\n");
                goto reset_buf;
            }
            /* log_info_hexdump(p_newtool, p_newtool->length + 6); */
            online_cfg_tool_data_deal(p_newtool, p_newtool->length + 6);
        } else {
            return;
        }

    } else {
        if (__this->data_length <= UART_FORMAT_HEAD) {
            return;
        }
        p = __this->pRxBuffer;

        if (p->preamble != UART_PREAMBLE) {
            log_info("preamble err\n");
            log_info_hexdump(__this->pRxBuffer, __this->data_length);
            goto reset_buf;
        }

        crc16 = crc_get_16bit(__this->pRxBuffer, UART_FORMAT_HEAD - 3);
        /* log_info("CRC8 0x%x / 0x%x", crc16, p->crc8); */
        if (p->crc8 != (crc16 & 0xff)) {
            log_info("crc8 err\n");
            goto reset_buf;
        }
        if (__this->data_length >= p->length + UART_FORMAT_HEAD) {
            /* log_info("Total length : 0x%x / Rx length : 0x%x", __this->data_length, p->length + CI_FORMAT_HEAD); */
            crc16 = crc_get_16bit(p->payload, p->length);
            /* log_info("CRC16 0x%x / 0x%x", crc16, p->crc16); */
            if (p->crc16 != crc16) {
                log_info("crc16 err\n");
                goto reset_buf;
            }
            __this->packet_handler(p->payload, p->length);
        } else {
            return;
        }
    }

reset_buf:
    __this->data_length = 0;
}

static void ci_uart_isr_cb(void *ut_bus, u32 status)
{
    if (status == UT_TX) {
        return ;
    }
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg  = (void *)DEVICE_EVENT_FROM_CI_UART;
    e.u.dev.event = 0;
    e.u.dev.value = 0;
    sys_event_notify(&e);
}

static int ci_uart_init()
{
    struct uart_platform_data_t ut = {0};
    ut.tx_pin = TCFG_ONLINE_TX_PORT;
    ut.rx_pin = TCFG_ONLINE_RX_PORT;
    ut.baud = __this->config.baudrate;
    ut.rx_timeout = 1;
    ut.isr_cbfun = ci_uart_isr_cb;
    ut.rx_cbuf = devBuffer_static;
    ut.rx_cbuf_size = UART_DB_SIZE;
    ut.frame_length = UART_DB_SIZE;
    /* JL_CLOCK->CLK_CON1 |= BIT(11); */
    /* JL_CLOCK->CLK_CON1 &= ~BIT(10); */
    __this->dbuf = devBuffer_static;
    __this->data_length = 0;
    __this->udev = (uart_bus_t *)uart_dev_open(&ut);
    if (__this->udev == NULL) {
        log_error("open uart dev err\n");
        return -1;
    }
    return 0;
}

static void ci_uart_putbyte(char a)
{
    if (__this->udev) {
        __this->udev->putbyte(a);
    }
}

void ci_uart_write(char *buf, u16 len)
{
    if (__this->udev) {
        __this->udev->write(buf, len);
    }
}

static void dummy_handler(const u8 *packet, int size)
{
    log_error("Dummy");
}

static void clock_critical_enter(void)
{

}
static void clock_critical_exit(void)
{
    if (__this->udev) {
        __this->udev->set_baud(__this->config.baudrate);
    }
}
CLOCK_CRITICAL_HANDLE_REG(ci, clock_critical_enter, clock_critical_exit)

static void ci_dev_init(const void *config)
{
#ifdef HAVE_MALLOC
    __this = malloc(sizeof(struct uart_hdl));
    ASSERT(__this, "Fatal error");

    memset(__this, 0x0, sizeof(struct uart_hdl));

    __this->pRxBuffer = malloc(UART_RX_SIZE);
    ASSERT(__this->pRxBuffer, "Fatal error");

    __this->pTxBuffer = malloc(UART_TX_SIZE);
    ASSERT(__this->pTxBuffer, "Fatal error");
#else
    log_info("Static");
    __this->pRxBuffer = pRxBuffer_static;
    __this->pTxBuffer = pTxBuffer_static;
#endif

    __this->packet_handler = dummy_handler;

    ci_transport_config_uart_t *ci_config_uart = (ci_transport_config_uart_t *)config;

    __this->config.baudrate     = ci_config_uart->baudrate_init;
    __this->config.flowcontrol  = ci_config_uart->flowcontrol;
    __this->config.dev_name     = ci_config_uart->device_name;

    log_info("baudrate : %d", __this->config.baudrate);
    log_info("flowcontrol: %d", __this->config.flowcontrol);
}

static int ci_dev_open(void)
{
    ci_uart_init();
    return 0;
}

static int ci_dev_close(void)
{
    return 0;
}

static void ci_dev_register_packet_handler(void (*handler)(const u8 *packet, int size))
{
    __this->packet_handler = handler;
}

static int ci_dev_send_packet(const u8 *packet, int size)
{
    /* dev_stream_out(); */
    int i = 0;
    uart_packet_t *p = (uart_packet_t *)__this->pTxBuffer;

    p->preamble = UART_PREAMBLE;
    p->type     = 0;
    p->length   = size;
    p->crc8     = CRC16(p, UART_FORMAT_HEAD - 3) & 0xff;
    p->crc16    = CRC16(packet, size);

    size += UART_FORMAT_HEAD;
    /* ASSERT(size <= UART_TX_SIZE, "Fatal Error"); */
    if (size > UART_TX_SIZE) {
        log_e("Fatal Error");
        return 0;
    }

    memcpy(p->payload, packet, size);

    /* log_info("Tx : %d", size); */
    /* log_info_hexdump(p, size); */
    if (__this->rx_type == CI_UART) {
#if 0
        while (size--) {
            ci_uart_putbyte(((char *)p)[i++]);
        }
#else
        ci_uart_write(p, size);
#endif
    }

    return 0;
}

static int ci_dev_can_send_packet_now(uint8_t packet_type)
{
    return 0;
}

// get dev api skeletons
static const ci_transport_t ci_transport_uart = {
    /* const char * name; */                                        "CI_UART",
    /* void   (*init) (const void *transport_config); */            &ci_dev_init,
    /* int    (*open)(void); */                                     &ci_dev_open,
    /* int    (*close)(void); */                                    &ci_dev_close,
    /* void   (*register_packet_handler)(void (*handler)(...); */   &ci_dev_register_packet_handler,
    /* int    (*can_send_packet_now)(uint8_t packet_type); */       &ci_dev_can_send_packet_now,
    /* int    (*send_packet)(...); */                               &ci_dev_send_packet,
};

const ci_transport_t *ci_transport_uart_instance(void)
{
    return &ci_transport_uart;
}



#endif

