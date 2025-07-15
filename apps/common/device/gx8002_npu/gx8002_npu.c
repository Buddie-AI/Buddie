#include "includes.h"
#include "app_config.h"
#include "gx8002_npu.h"
#include "gx8002_npu_api.h"
#include "asm/pwm_led.h"
#include "btstack/avctp_user.h"

#if TCFG_GX8002_NPU_ENABLE

#define GX8002_DEBUG_ENABLE 	0

#define gx8002_info 	g_printf
#if GX8002_DEBUG_ENABLE
#define gx8002_debug 	g_printf
#define gx8002_put_buf  put_buf
#else
#define gx8002_debug(...)
#define gx8002_put_buf(...)
#endif /* #if GX8002_DEBUG_ENABLE */



//===========================================================//
//                 GX8002 NPU LOGIC LAYER                    //
//===========================================================//
///// 分配内存 /////
#define OS_MALLOC    malloc

///// 释放内存 /////
#define OS_FREE    free

static const char msg_rcv_magic[MSG_MAGIC_LEN] = {
    MSG_RCV_MAGIC0, MSG_RCV_MAGIC1,
    MSG_RCV_MAGIC2, MSG_RCV_MAGIC3
};

static unsigned char msg_seq = 0;
static unsigned char initialized = 0;
static struct message prev_snd_msg;
static struct message prev_rcv_msg;
static STREAM_READ  stream_read = NULL;
static STREAM_WRITE stream_write = NULL;
static STREAM_EMPTY stream_is_empty = NULL;

struct msg_header {
    unsigned int magic;
    unsigned short cmd;
    unsigned char  seq;
    unsigned char  flags;
    unsigned short length;
    unsigned int crc32;
} __attribute__((packed));

static const unsigned int crc32tab[] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
    0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
    0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
    0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
    0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
    0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
    0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
    0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
    0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
    0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
    0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
    0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
    0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
    0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
    0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
    0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
    0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
    0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
    0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
    0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
    0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
    0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
    0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
    0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
    0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
    0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
    0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
    0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
    0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
    0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
    0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
    0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
    0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
    0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
    0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
    0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
    0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
    0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
    0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
    0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
    0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
    0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
    0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
    0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
    0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
    0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
    0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
    0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
    0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
    0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
    0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
    0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

static unsigned int crc32(const unsigned char *buf, unsigned int size)
{
    unsigned int i;
    unsigned int crc = 0xFFFFFFFF;

    for (i = 0; i < size; i++) {
        crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;
}

static inline int stream_getc(char *ch)
{
    int ret = stream_read((unsigned char *)ch, 1);
    return ret;
}

static inline int msg_byte_match(char c)
{
    char ch = 0;

    return (stream_getc(&ch) && (ch == c));
}

static int msg_find_magic(void)
{
    int i;
    int offset = 0;

    while (offset < MSG_LEN_MAX) {
        for (i = 0; i < MSG_MAGIC_LEN; i++) {
            offset++;
            if (!msg_byte_match(msg_rcv_magic[i])) {
                break;
            }

            if ((i + 1) == MSG_MAGIC_LEN) {
                return 0;
            }
        }
    }
    return -1;
}

static unsigned char msg_get_new_seq(unsigned char seq)
{
    unsigned char next_seq;

    next_seq = (seq + 1) % MSG_SEQ_MAX;

    return next_seq == 0 ? next_seq + 1 : next_seq;
}

static int nc_message_receive(struct message *msg)
{
    struct msg_header msg_header;
    unsigned int newcrc32 = 0;
    unsigned int oldcrc32 = 0;
    unsigned short bodylen;
    unsigned char *pheader = (unsigned char *)&msg_header;

    if (!msg || !initialized) {
        line_inf;
        return -1;
    }

    if (stream_is_empty()) {
        line_inf;
        return -1;
    }

    /* Read stream until find mseeage magic */
    if (msg_find_magic() != 0) {
        line_inf;
        return -1;
    }

    memset(&msg_header, 0, sizeof(struct msg_header));
    msg_header.magic = MSG_MAGIC;

    /* Read the rest of message header */
    stream_read(pheader + MSG_MAGIC_LEN,
                sizeof(struct msg_header) - MSG_MAGIC_LEN);

    /* Check message header integrity */
    newcrc32 = crc32((unsigned char *)&msg_header,
                     sizeof(struct msg_header) - MSG_HEAD_CHECK_LEN);
    if (newcrc32 != msg_header.crc32) {
        line_inf;
        return -1;
    }

    /* Read message body */
    if (msg_header.length > 0) {

        msg->body = OS_MALLOC(msg_header.length);
        if (msg->body == NULL) {
            line_inf;
            return -1;
        }

        stream_read(msg->body, msg_header.length);
        if (MSG_NEED_BCHECK(msg_header.flags)) {
            bodylen = msg_header.length - MSG_BODY_CHECK_LEN;
            newcrc32 = crc32(msg->body, bodylen);
            memcpy(&oldcrc32, msg->body + bodylen, MSG_BODY_CHECK_LEN);

            if (oldcrc32 != newcrc32) {
                line_inf;
                return -1;
            }
        } else {
            bodylen = msg_header.length;
        }
    } else {
        bodylen = 0;
    }

    msg->cmd = msg_header.cmd;
    msg->seq = msg_header.seq;
    msg->flags = msg_header.flags;
    msg->bodylen = bodylen;

    prev_rcv_msg.cmd = msg->cmd;
    prev_rcv_msg.seq = msg->seq;

    return bodylen;
}

static int nc_message_send(struct message *msg)
{
    struct msg_header msg_header;
    unsigned int body_crc32 = 0;

    if (!msg || !initialized) {
        return -1;
    }

    memset(&msg_header, 0, sizeof(struct msg_header));

    msg_header.magic = MSG_MAGIC;
    msg_header.cmd = msg->cmd;
    msg_header.flags = msg->flags;

    if ((msg->cmd == prev_snd_msg.cmd) &&
        (msg->seq == prev_snd_msg.seq) &&
        (MSG_TYPE(msg->cmd)) == MSG_TYPE_REQ) {
        msg_seq = prev_snd_msg.seq;
    } else {
        msg_seq = msg_get_new_seq(msg_seq);
    }

    msg_header.seq = msg_seq;

    if (!msg->body) {
        msg_header.length = 0;
    } else {
        if (MSG_NEED_BCHECK(msg->flags)) {
            body_crc32 = crc32((unsigned char *)msg->body, msg->bodylen);
            msg_header.length = msg->bodylen + MSG_BODY_CHECK_LEN;
        } else {
            msg_header.length = msg->bodylen;
        }
    }

    msg_header.crc32 = crc32((unsigned char *)&msg_header,
                             sizeof(struct msg_header) - MSG_BODY_CHECK_LEN);

    stream_write((unsigned char *)&msg_header, sizeof(struct msg_header));

    if (msg->body) {
        stream_write(msg->body, msg->bodylen);
        if (MSG_NEED_BCHECK(msg->flags)) {
            stream_write((unsigned char *)&body_crc32, MSG_BODY_CHECK_LEN);
        }
    }

    prev_snd_msg.cmd = msg->cmd;
    prev_snd_msg.seq = msg_seq;
    msg->seq = msg_seq;

    return msg->bodylen;
}



static int nc_message_init(STREAM_READ read, STREAM_WRITE write, STREAM_EMPTY is_empty)
{
    if (!read || !write || !is_empty) {
        return -1;
    }

    stream_read = read;
    stream_write = write;
    stream_is_empty = is_empty;

    msg_seq = 0;
    memset(&prev_rcv_msg, 0xFF, sizeof(prev_rcv_msg));
    memset(&prev_snd_msg, 0xFF, sizeof(prev_snd_msg));
    initialized = 1;

    return 0;
}

static int gx8002_uart_agent_query_event(unsigned short cmd)
{
    struct message snd_msg = {0};
    gx8002_info("gx8002 task query msg");

    //snd_msg.cmd = MSG_REQ_VOICE_EVENT;
    snd_msg.cmd = cmd;
    snd_msg.flags = 0;
    snd_msg.body = NULL;
    snd_msg.bodylen = 0;

    return nc_message_send(&snd_msg);
}



//===========================================================//
//                 GX8002 NPU PORT LAYER                     //
//===========================================================//

#define THIS_TASK_NAME 		"gx8002"

enum GX8002_STATE {
    GX8002_STATE_WORKING = 1,
    GX8002_STATE_SUSPEND,
    GX8002_STATE_UPGRADING,
};


struct gx8002_handle {
    u8 state;
    u8 idle; //0: busy, 1: idle
    OS_SEM rx_sem;
    const uart_bus_t *uart_bus;
};

static struct gx8002_handle hdl;

#define __this (&hdl)
static u8 uart_cbuf[32] __attribute__((aligned(4)));
u8 gx_self_info[6] = {0, 0, 0, 0, //version
                      0,  //mic status
                      0
                     }; //gsensor status

//============== extern function =================//
extern void gx8002_board_port_resumed(void);
extern void gx8002_board_port_suspend(void);
extern void gx8002_vddio_power_ctrl(u8 on);
extern void gx8002_core_vdd_power_ctrl(u8 on);

static void gx8002_uart_isr_hook(void *arg, u32 status)
{
    if (status != UT_TX) {
        gx8002_info("gx8002 uart RX pnding");
        if (__this->state == GX8002_STATE_WORKING) {
            os_sem_post(&(__this->rx_sem));
            __this->idle = 1;
        }
    }
}

static void gx8002_uart_porting_config(u32 baud)
{
    struct uart_platform_data_t u_arg = {0};

    if (__this->uart_bus) {
        gx8002_info("gx8002 uart have init");
        return;
    }
    u_arg.tx_pin = TCFG_GX8002_NPU_UART_TX_PORT;
    u_arg.rx_pin = TCFG_GX8002_NPU_UART_RX_PORT;
    u_arg.rx_cbuf = uart_cbuf;
    u_arg.rx_cbuf_size = sizeof(uart_cbuf);
    u_arg.frame_length = 0xFFFF;
    u_arg.rx_timeout = 100;
    u_arg.isr_cbfun = gx8002_uart_isr_hook;
    u_arg.baud = baud;
    u_arg.is_9bit = 0;

    __this->uart_bus = uart_dev_open(&u_arg);
    if (__this->uart_bus) {
        gx8002_info("gx8002 uart init succ");
    } else {
        gx8002_info("gx8002 uart init fail");
        ASSERT(__this->uart_bus);
    }
}


static void gx8002_uart_porting_close(void)
{
    if (__this->uart_bus) {
        uart_dev_close(__this->uart_bus);
        gpio_disable_fun_output_port(TCFG_GX8002_NPU_UART_TX_PORT);
        gx8002_board_port_suspend();
        __this->uart_bus = NULL;
    }
}


static int gx8002_uart_stream_read(unsigned char *buf, int len)
{
    int ret = 0;

    ASSERT(__this->uart_bus && __this->uart_bus->read);
    if (__this->uart_bus && __this->uart_bus->read) {
        ret = __this->uart_bus->read(buf, len, 1000);
        gx8002_debug("stream read len %d", len);
        gx8002_put_buf(buf, len);
    }

    return ret;
}

static int gx8002_uart_stream_write(const unsigned char *buf, int len)
{
    int ret = 0;

    ASSERT(__this->uart_bus && __this->uart_bus->write);
    if (__this->uart_bus && __this->uart_bus->write) {
        __this->uart_bus->write(buf, len);
        ret = len;
        gx8002_debug("stream write len %d", len);
        gx8002_put_buf(buf, len);
    }

    return ret;
}

static int gx8002_uart_stream_is_empty(void)
{
    return 0;
}

static int gx8002_uart_porting_poll(u32 timeout)
{
    return 1;
}


static void gx8002_power_off(void)
{
    //由板级提供
    gx8002_board_port_suspend();
    gx8002_vddio_power_ctrl(0);
    gx8002_core_vdd_power_ctrl(0);
}

static void gx8002_power_off_keep_vddio(void)
{
    //由板级提供
    gx8002_board_port_suspend();
    //gx8002_vddio_power_ctrl(0); //通话是会影响mic配置, 不关MIC_LDO
    gx8002_core_vdd_power_ctrl(0);
}

//======================================//
// GX8002上电注意事项:
// 1) 正常上电需要先将uart IO初始化为确认电平, 否则gx8002会进入升级模式;
// 2) vddio先上电, core_vdd需要等vddio上电后之后3ms后上电;
//======================================//
static void gx8002_normal_power_on(void)
{
    gx8002_board_port_resumed();
    //由板级提供
    //gx8002_core_vdd_power_ctrl(0);
    gx8002_vddio_power_ctrl(1);
    os_time_dly(2);
    gx8002_core_vdd_power_ctrl(1);
}

//进升级模式
static void gx8002_upgrade_power_on(void)
{
    //由板级提供
    //gx8002_core_vdd_power_ctrl(0);
    gx8002_vddio_power_ctrl(1);
    os_time_dly(2);
    gx8002_core_vdd_power_ctrl(1);
    gx8002_board_port_resumed();
}

static void gx8002_first_power_on(void)
{
    //由板级提供
    //gx8002_core_vdd_power_ctrl(0);
    gx8002_vddio_power_ctrl(1);
    os_time_dly(10);
    gx8002_core_vdd_power_ctrl(1);

    //gx8002_board_port_resumed();
}


static void __gx8002_module_suspend(u8 keep_vddio)
{
    gx8002_info("%s", __func__);
    if (keep_vddio) {
        gx8002_power_off_keep_vddio();
    } else {
        gx8002_power_off();
    }

    if (__this->state == GX8002_STATE_UPGRADING) {
        return;
    }
    __this->state = GX8002_STATE_SUSPEND;
}

static void __gx8002_module_resumed()
{
    gx8002_info("%s", __func__);
    gx8002_normal_power_on();

    if (__this->state == GX8002_STATE_UPGRADING) {
        return;
    }
    __this->state = GX8002_STATE_WORKING;
}

void gx8002_module_suspend(u8 keep_vddio)
{
    os_taskq_post_msg(THIS_TASK_NAME, 2, GX8002_MSG_SUSPEND, keep_vddio);
}

void gx8002_module_resumed()
{
    os_taskq_post_msg(THIS_TASK_NAME, 1, GX8002_MSG_RESUMED);
}

void gx8002_normal_cold_reset(void)
{
    gx8002_power_off();
    os_time_dly(50);
    gx8002_normal_power_on();
}

void gx8002_upgrade_cold_reset(void)
{
    gx8002_power_off();
    os_time_dly(50);
    gx8002_upgrade_power_on();
}

int gx8002_uart_spp_ota_init(void);
static void gx8002_app_event_handler(struct sys_event *event)
{
    gx8002_info("%s", __func__);
    struct bt_event *bt = &(event->u.bt);
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            switch (bt->event) {
            case BT_STATUS_FIRST_CONNECTED:
#if GX8002_UPGRADE_SPP_TOGGLE
                gx8002_uart_spp_ota_init();
#endif /* #if GX8002_UPGRADE_SPP_TOGGLE */
                break;
#if 0
            case BT_STATUS_SCO_STATUS_CHANGE:
                if (bt->value != 0xff) {
                    gx8002_module_suspend();
                } else {
                    gx8002_module_resumed();
                }
#endif
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
}

static int gx8002_wakeup_handle(unsigned short cmd)
{
    int err;
    struct message recv_msg = {0};
    unsigned short event;
    int ret = 0;

    if (__this->state != GX8002_STATE_WORKING) {
        return -2;
    }

    if (gx8002_uart_porting_poll(1000) > 0) {
        os_sem_set(&(__this->rx_sem), 0);
        gx8002_uart_agent_query_event(cmd);

        err = os_sem_pend(&(__this->rx_sem), 100);
        if (err != OS_NO_ERR) {
            gx8002_info("rx sem err: %d", err);
            __this->idle = 1;
            return -1;
        }
        gx8002_info("gx8002 task receive >>>>");
        // 有串口数据
        memset(&recv_msg, 0, sizeof(recv_msg));
        if (nc_message_receive(&recv_msg) < 0) {
            if (recv_msg.body) {
                OS_FREE(recv_msg.body);
            }
            gx8002_info("recv_msg NULL\n");
            return -1;
        }

        switch (recv_msg.cmd) {
        case MSG_RSP_VOICE_EVENT:
            //语音事件
            event = recv_msg.body[1] << 8 | recv_msg.body[0];
            //处理语音事件
            ret = event;
            gx8002_event_state_update(event);
            break;

        case MSG_RSP_MIC_EVENT:
            event = recv_msg.body[1] << 8 | recv_msg.body[0];
            ret = event;
            break;
        case MSG_RSP_GSENSOR_EVENT:
            event = recv_msg.body[1] << 8 | recv_msg.body[0];
            ret = event;
            break;

        case MSG_RSP_VERSION_EVENT:
            gx8002_info(">> gx version");
            memcpy(gx_self_info, recv_msg.body, recv_msg.bodylen);
            put_buf(gx_self_info, sizeof(gx_self_info));
            break;

        default:
            break;
        }

        if (recv_msg.body) {
            OS_FREE(recv_msg.body);
        }
    }

    return ret;
}

static void gx8002_self_test_handle()
{
    int ret = 0;
    int mic_test_cnt = 0;
    int gsensor_test_cnt = 0;

    //int self_test_result = 0;

    gx8002_info("gx8002 self test start >>>>");
__gsensor_test_retry:
    __this->idle = 0;
    gx8002_info("gsensor_test_cnt = %d", gsensor_test_cnt);
    ret = gx8002_wakeup_handle(MSG_REQ_GSENSOR_EVENT);
    if (ret == 0) {
        gx8002_info("gx8002 gsensor test OK");
        //self_test_result |= (0xA << 4);
        gx_self_info[5] = 0xA;
    } else {
        gsensor_test_cnt++;
        if (gsensor_test_cnt < 10) {
            gx8002_info("gx8002 gsensor no OK try again");
            os_time_dly(500 / 10);
            goto __gsensor_test_retry;
        }
        gx_self_info[5] = 1;
    }

//MIC状态查询
__mic_test_retry:
    __this->idle = 0;
    gx8002_info("mic_test_cnt = %d", mic_test_cnt);
    ret = gx8002_wakeup_handle(MSG_REQ_MIC_EVENT);
    if (ret == 0) {
        gx8002_info("gx8002 mic test OK");
        //self_test_result |= 0xA;
        gx_self_info[4] = 0xA;
    } else {
        mic_test_cnt++;
        if (mic_test_cnt < 10) {
            gx8002_info("gx8002 mic no OK try again");
            os_time_dly(500 / 10);
            goto __mic_test_retry;
        }
        gx_self_info[4] = 1;
    }

    __this->idle = 1;

    //put_buf(gx_self_info,sizeof(gx_self_info));
    gx8002_info("gx_self_info[mic]:%x,gx_self_info[g]:%x", gx_self_info[4], gx_self_info[5]);
}


static void gx8002_sdfile_update_timer(void *priv)
{
    gx8002_info("gx8002 post update msg >>>>");
    os_taskq_post_msg(THIS_TASK_NAME, 2, GX8002_MSG_UPDATE, GX8002_UPDATE_TYPE_SDFILE);
}

void gx8002_update_end_post_msg(u8 flag)
{
    os_taskq_post_msg(THIS_TASK_NAME, 2, GX8002_MSG_UPDATE_END, flag);
}


static void gx8002_upgrade_start(int update_type)
{
#if GX8002_UPGRADE_TOGGLE
    gx8002_uart_porting_close();
    __this->state = GX8002_STATE_UPGRADING;

    switch (update_type) {
    case GX8002_UPDATE_TYPE_SDFILE:
        gx8002_uart_sdfile_ota_init();
        break;
    case GX8002_UPDATE_TYPE_SPP:
        gx8002_uart_spp_ota_init();
        break;
    case GX8002_UPDATE_TYPE_APP:
        gx8002_uart_app_ota_init();
        break;
    default:
        break;
    }
#endif /* #if GX8002_UPGRADE_TOGGLE */
}

static void gx8002_upgrade_end(u8 flag)
{
    gx8002_uart_porting_config(115200);
    gx8002_normal_power_on();
    __this->state = GX8002_STATE_WORKING;
}

void gx8002_voice_event_post_msg(u8 msg)
{
    os_taskq_post_msg(THIS_TASK_NAME, 2, GX8002_MSG_VOICE_EVENT, msg);
}

void gx8002_npu_version_check(void *priv)
{
    if (__this->state == GX8002_STATE_WORKING) {
        __this->idle = 0;
        os_taskq_post_msg(THIS_TASK_NAME, 1, GX8002_MSG_GET_VERSION);
    } else {
        __this->idle = 1;
    }
}


static void gx8002_uart_recv_task(void *param)
{
    int msg[16];
    int res;

    gx8002_info("gx8002 task init");
#if 1
    //串口初始化
    gx8002_uart_porting_config(115200);

    gx8002_first_power_on();

    // 设置回调
    nc_message_init(gx8002_uart_stream_read, gx8002_uart_stream_write, gx8002_uart_stream_is_empty);
#endif

    os_sem_create(&(__this->rx_sem), 0);

    __this->state = GX8002_STATE_WORKING;
    __this->idle = 1;
#if GX8002_UPGRADE_SPP_TOGGLE
    register_sys_event_handler(SYS_BT_EVENT, 0, 0, gx8002_app_event_handler);
#endif /* #if GX8002_UPGRADE_SPP_TOGGLE */

    /* if (GX8002_UPGRADE_SDFILE_TOGGLE) { */
    /* sys_timeout_add(NULL, gx8002_sdfile_update_timer, 10000); */
    /* } */

#if GX8002_UPGRADE_APP_TOGGLE
    gx8002_uart_app_ota_update_register_handle();
#endif /* #if GX8002_UPGRADE_APP_TOGGLE */

    //gx版本查询
    sys_timeout_add(NULL, gx8002_npu_version_check, 1000);

    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case GX8002_MSG_WAKEUP:
                gx8002_wakeup_handle(MSG_REQ_VOICE_EVENT);
                break;
            case GX8002_MSG_VOICE_EVENT:
                gx8002_voice_event_handle(msg[2]);
                break;
            case GX8002_MSG_UPDATE:
                gx8002_upgrade_start(msg[2]);
                break;
            case GX8002_MSG_UPDATE_END:
                gx8002_upgrade_end(msg[2]);
                break;
            case GX8002_MSG_SUSPEND:
                __gx8002_module_suspend(msg[2]);
                break;
            case GX8002_MSG_RESUMED:
                __gx8002_module_resumed();
                break;
            case GX8002_MSG_SELF_TEST:
                gx8002_self_test_handle();
                break;
            case GX8002_MSG_GET_VERSION:
                gx8002_wakeup_handle(MSG_REQ_VERSION_EVENT);
                break;
            default:
                break;
            }
        }
    }
}


int gx8002_npu_init(void)
{
#if 0 //升级GX8002固件
    gpio_set_pull_down(TCFG_GX8002_NPU_UART_TX_PORT, 0);
    gpio_set_pull_up(TCFG_GX8002_NPU_UART_TX_PORT, 0);
    gpio_set_die(TCFG_GX8002_NPU_UART_TX_PORT, 0);
    gpio_set_dieh(TCFG_GX8002_NPU_UART_TX_PORT, 0);
    gpio_set_direction(TCFG_GX8002_NPU_UART_TX_PORT, 1);

    gpio_set_pull_down(TCFG_GX8002_NPU_UART_RX_PORT, 0);
    gpio_set_pull_up(TCFG_GX8002_NPU_UART_RX_PORT, 0);
    gpio_set_die(TCFG_GX8002_NPU_UART_RX_PORT, 0);
    gpio_set_dieh(TCFG_GX8002_NPU_UART_RX_PORT, 0);
    gpio_set_direction(TCFG_GX8002_NPU_UART_RX_PORT, 1);
    return 0;
#endif
    //创建recv任务
    task_create(gx8002_uart_recv_task, 0, THIS_TASK_NAME);
    return 0;
}

void gx8002_npu_int_edge_wakeup_handle(u8 index, u8 gpio)
{
    gx8002_info("gx8002 wakeup, cut_state: %d", __this->state);
    if (__this->state == GX8002_STATE_WORKING) {
        __this->idle = 0;
        os_taskq_post_msg(THIS_TASK_NAME, 1, GX8002_MSG_WAKEUP);
    } else {
        __this->idle = 1;
    }
}

void gx8002_npu_mic_gsensor_self_test(void)
{
    gx8002_info("gx8002 self test %d", __this->state);
    if (__this->state == GX8002_STATE_WORKING) {
        __this->idle = 0;
        os_taskq_post_msg(THIS_TASK_NAME, 1, GX8002_MSG_SELF_TEST);
    } else {
        __this->idle = 1;
    }
}

static u8 gx8002_npu_idle_query(void)
{
    //gx8002_debug("__this->idle = %d", __this->idle);
    //return 1;
    if (__this->state == GX8002_STATE_UPGRADING) {
        return 0;
    }
    return __this->idle;
}

static enum LOW_POWER_LEVEL gx8002_npu_level_query(void)
{
    if (__this->state != GX8002_STATE_SUSPEND) {
        return LOW_POWER_MODE_LIGHT_SLEEP;
    } else {
        return LOW_POWER_MODE_DEEP_SLEEP;
    }
}

REGISTER_LP_TARGET(gx8002_lp_target) = {
    .name = "gx8002",
    .level = gx8002_npu_level_query,
    .is_idle = gx8002_npu_idle_query,
};

#endif /* #if TCFG_GX8002_NPU_ENABLE */
