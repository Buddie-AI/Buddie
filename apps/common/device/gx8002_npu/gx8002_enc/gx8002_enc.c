#include "includes.h"
#include "app_config.h"
#include "gx8002_enc.h"

#if TCFG_GX8002_ENC_ENABLE

#define GX8002_DEBUG_ENABLE 	1

#define gx8002_info 	printf
#if GX8002_DEBUG_ENABLE
#define gx8002_debug 	printf
#define gx8002_put_buf  put_buf
#else
#define gx8002_debug(...)
#define gx8002_put_buf(...)
#endif /* #if GX8002_DEBUG_ENABLE */


//====================================================================//
//                     AC897N GX8002 ENC模型                          //
/*
  ________                    ________
 |        |      PCM 2CH     |        |
 |      TX|----------------->|RX      |
 |  UART  |                  |  UART  |
 |        |      PCM 1CH     |        |
 |      RX|<-----------------|TX      |
 |________|                  |________|
   BT_CHIP                    GX8002

NOTE:
*/

//====================================================================//
#define UART_RX_BUF_POINTS 		(256 * 4) // buf_len = UART_RX_BUF_POINTS x sizeof(16)
#define UART_RX_BUF_FRAME_LEN 	(256 * sizeof(s16) * 2) // buf_len = UART_RX_BUF_FRAME_LEN
#define UART_TX_BUF_POINTS 		(256 * 2) //buf_len = UART_TX_BUF_POINTS x sizeof(16)
#define UART_TRANSPORT_BUAD 	1000000   //发送波特率

#define THIS_TASK_NAME 		"gx8002_enc"

struct gx8002_enc_t {
    u8 task_init;
    u8 state;
    const uart_bus_t *uart_bus;
    OS_SEM rx_sem;
    int uart_rxbuf[(UART_RX_BUF_POINTS * sizeof(s16)) / sizeof(int)]; //uart硬件RX dma缓存, 要求buf长度为2的n次幂, 4字节对齐
    int uart_txbuf[(UART_TX_BUF_POINTS * sizeof(s16)) / sizeof(int)]; //uart硬件TX dma缓存, 要求buf4字节对齐
    int uart_rx_frame_buf[UART_RX_BUF_FRAME_LEN / sizeof(int)]; //用于读取Rx缓存中的数据, 临时buf
};

enum {
    GX8002_ENC_STATE_CLOSE = 0,
    GX8002_ENC_STATE_OPEN,
    GX8002_ENC_STATE_RUN,
};


static struct gx8002_enc_t *gx8002_enc = NULL;

//============== extern function =================//
/* extern void gx8002_vddio_power_ctrl(u8 on); */
/* extern void gx8002_core_vdd_power_ctrl(u8 on); */

//for make
__attribute__((weak))
void gx8002_board_port_suspend(void)
{
}

__attribute__((weak))
void gx8002_vddio_power_ctrl(u8 on)
{
}

__attribute__((weak))
void gx8002_core_vdd_power_ctrl(u8 on)
{
}


static void gx8002_device_power_on(void)
{
    gx8002_vddio_power_ctrl(1);
    os_time_dly(2); //vddio先上电 --> wait 20ms --> core_vdd上电
    gx8002_core_vdd_power_ctrl(1);
}

static void gx8002_device_power_off(void)
{
    gx8002_vddio_power_ctrl(0);
    gx8002_core_vdd_power_ctrl(0);
}

static void gx8002_enc_data_output(void)
{
    int ret = 0;
    s16 *rxbuf = NULL;
    s16 magic_code = 0;
    static u8 cnt = 0;
    if (gx8002_enc && (gx8002_enc->state == GX8002_ENC_STATE_RUN)) {
        if (gx8002_enc->uart_bus && gx8002_enc->uart_bus->read) {
            rxbuf = (s16 *)(gx8002_enc->uart_rx_frame_buf);
            putchar('r');
            ret = gx8002_enc->uart_bus->read((u8 *)rxbuf, UART_RX_BUF_FRAME_LEN, 0);
#if 1
            //数据校验
            magic_code = rxbuf[0];
            for (int i = 1; i < ret / sizeof(s16); i++) {
                if (rxbuf[i] != magic_code) {
                    if (rxbuf[i] != (magic_code + 1)) {
                        gx8002_debug("data err: magic_code = 0x%x, rxbuf[%d] = 0x%x", magic_code, i, rxbuf[i]);
                        printf("buf_in = %d", gx8002_enc->uart_bus->kfifo.buf_in);
                        printf("buf_out = %d", gx8002_enc->uart_bus->kfifo.buf_out);
                        put_buf(gx8002_enc->uart_bus->kfifo.buffer, gx8002_enc->uart_bus->kfifo.buf_size);
                        ASSERT(0);
                        break;
                        wdt_clear();
                    } else {
                        magic_code++;
                        gx8002_debug("increase: 0x%x", magic_code);
                    }
                }
            }
#endif
        }
    }
}


static void gx8002_enc_uart_isr_hook(void *arg, u32 status)
{
    if (status != UT_TX) {
        //RX pending
        os_sem_post(&(gx8002_enc->rx_sem));
    }
}


static void gx8002_enc_uart_porting_config(u32 baud)
{
    struct uart_platform_data_t u_arg = {0};

    u_arg.tx_pin = TCFG_GX8002_ENC_UART_TX_PORT;
    u_arg.rx_pin = TCFG_GX8002_ENC_UART_RX_PORT;
    u_arg.rx_cbuf = gx8002_enc->uart_rxbuf;
    u_arg.rx_cbuf_size = sizeof(gx8002_enc->uart_rxbuf);
    u_arg.frame_length = UART_RX_BUF_FRAME_LEN;
    u_arg.rx_timeout = 100;
    u_arg.isr_cbfun = gx8002_enc_uart_isr_hook;
    u_arg.baud = baud;
    u_arg.is_9bit = 0;

    gx8002_enc->uart_bus = uart_dev_open(&u_arg);

    if (gx8002_enc->uart_bus) {
        gx8002_info("gx8002 uart init succ");
    } else {
        gx8002_info("gx8002 uart init fail");
        ASSERT(gx8002_enc->uart_bus);
    }
}

static void gx8002_enc_uart_porting_close(void)
{
    if (gx8002_enc && gx8002_enc->uart_bus) {
        uart_dev_close((uart_bus_t *)(gx8002_enc->uart_bus));
        gx8002_board_port_suspend();
        gx8002_enc->uart_bus = NULL;
    }
}

static void __gx8002_enc_run(void)
{
    //TODO: add gx8002 post start msg
    gx8002_debug("%s", __func__);

    while (1) {
        os_sem_pend(&(gx8002_enc->rx_sem), 0);

        if (gx8002_enc->state != GX8002_ENC_STATE_RUN) {
            break;
        }

        gx8002_enc_data_output();
    }
}

static void __gx8002_enc_close(int priv)
{
    if (gx8002_enc) {
        if (gx8002_enc->state == GX8002_ENC_STATE_CLOSE) {
            gx8002_device_power_off();
            gx8002_enc_uart_porting_close();
            if (priv) {
                os_sem_post((OS_SEM *)priv);
            }
        }
    }
}

static void gx8002_enc_task(void *priv)
{
    int msg[16];
    int res;

    os_sem_create(&(gx8002_enc->rx_sem), 0);

    gx8002_enc_uart_porting_config(UART_TRANSPORT_BUAD);
    gx8002_device_power_on();

    gx8002_enc->state = GX8002_ENC_STATE_OPEN;

    if (priv) {
        os_sem_post((OS_SEM *)priv);
    }

    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case GX8002_ENC_MSG_RUN:
                __gx8002_enc_run();
                break;
            case GX8002_ENC_MSG_CLOSE:
                __gx8002_enc_close(msg[2]);
                break;
            default:
                break;
            }
        }
    }
}




//====================================================================//
//                       GX8002 ENC API                               //
//====================================================================//
/*----------------------------------------------------------------------------*/
/**@brief   打开gx8002_enc流程, 创建gx8002线程
   @param   void
   @return  void
   @note    不能在中断中调用, 需要在线程中调用;
*/
/*----------------------------------------------------------------------------*/
void gx8002_enc_open(void)
{
    gx8002_debug("%s", __func__);

    if (gx8002_enc == NULL) {
        gx8002_enc = zalloc(sizeof(struct gx8002_enc_t));
        if (gx8002_enc == NULL) {
            return;
        }
    } else {
        return;
    }

    OS_SEM sem_wait;
    os_sem_create(&sem_wait, 0);
    gx8002_enc->task_init = 1;
    task_create(gx8002_enc_task, (void *)&sem_wait, THIS_TASK_NAME);

    os_sem_pend(&sem_wait, 0);
}

/*----------------------------------------------------------------------------*/
/**@brief   启动gx8002_enc流程
   @param   void
   @return  void
   @note
*/
/*----------------------------------------------------------------------------*/
void gx8002_enc_start(void)
{
    gx8002_debug("%s", __func__);

    if (gx8002_enc && (gx8002_enc->state == GX8002_ENC_STATE_OPEN)) {
        gx8002_enc->state = GX8002_ENC_STATE_RUN;
        os_taskq_post_msg(THIS_TASK_NAME, 1, GX8002_ENC_MSG_RUN);
    }

    return;
}

/*----------------------------------------------------------------------------*/
/**@brief   关闭gx8002_enc流程, 释放资源
   @param   void
   @return  void
   @note    不能在中断中调用, 需要在线程中调用;
*/
/*----------------------------------------------------------------------------*/
void gx8002_enc_close(void)
{
    OS_SEM sem_wait;

    gx8002_debug("%s", __func__);
    if (gx8002_enc) {
        if (gx8002_enc->task_init) {
            os_sem_create(&sem_wait, 0);
            gx8002_enc->state = GX8002_ENC_STATE_CLOSE;
            os_sem_post(&(gx8002_enc->rx_sem));
            os_taskq_post_msg(THIS_TASK_NAME, 2, GX8002_ENC_MSG_CLOSE, (u32)&sem_wait);
            os_sem_pend(&sem_wait, 0);
            gx8002_enc->task_init = 0;
            task_kill(THIS_TASK_NAME);
        }

        free(gx8002_enc);
        gx8002_enc = NULL;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   gx8002_enc数据输入, 支持1~2通道数据输入
   @param   ch0_buf: 通道0数据buf
   @param   ch1_buf: 通道1数据buf
   @param   points: ch0和ch1 buf中点数
   @return
   @note    1)不能在中断中调用, 需要在线程中调用;
   @note    2)如果只有1通道数据, 把ch0_buf/ch1_buf传参为NULL即可;
   @note    3)ch0_buf和ch1_buf数据宽度为16bit;
*/
/*----------------------------------------------------------------------------*/
u32 gx8002_enc_data_input(s16 *ch0_buf, s16 *ch1_buf, u32 points)
{
    u32 send_points = 0;
    s16 *txbuf = NULL;
    u32 remain_points = points;
    u32 txbuf_points = 0;
    u8 ch_num = 0;

    if (gx8002_enc && (gx8002_enc->state == GX8002_ENC_STATE_RUN)) {
        if (gx8002_enc->uart_bus && gx8002_enc->uart_bus->write) {
            if (ch0_buf && ch1_buf) {
                //2ch
                ch_num = 2;
                txbuf_points = (sizeof(gx8002_enc->uart_txbuf) / sizeof(s16) / 2);
            } else {
                ch_num = 1;
                if (ch0_buf == NULL) {
                    ch0_buf = ch1_buf;
                }
                txbuf_points = (sizeof(gx8002_enc->uart_txbuf) / sizeof(s16));
            }

            txbuf = (s16 *)(gx8002_enc->uart_txbuf);
            while (remain_points) {
                send_points = remain_points <=  txbuf_points ? remain_points : txbuf_points;
                //gx8002_debug("send_points: %d", send_points);
                if (ch_num == 2) {
                    for (int i = 0; i < send_points; i++) {
                        txbuf[2 * i] = ch0_buf[i];
                        txbuf[2 * i + 1] = ch1_buf[i];
                    }
                } else {
                    for (int i = 0; i < send_points; i++) {
                        txbuf[i] = ch0_buf[i];
                    }
                }

                gx8002_enc->uart_bus->write((const u8 *)txbuf, send_points * sizeof(s16) * ch_num);
                remain_points -= send_points;
            }

            return points * sizeof(s16) * ch_num;
        }
    }

    return 0;
}

//====================================================================//
//                       GX8002 ENC TEST                              //
//====================================================================//

static void gx8002_enc_input_test(void *priv)
{

    if (gx8002_enc == NULL) {
        putchar('i');
        return;
    }
#define BUF_TEST_POINTS 	300
    static s16 cnt = 0x0;
    s16 tmp_buf[BUF_TEST_POINTS];
    putchar('w');
    cnt++;
    for (int i = 0; i < ARRAY_SIZE(tmp_buf); i++) {
        tmp_buf[i] = cnt;
    }
    //gx8002_enc_data_input(tmp_buf, NULL, BUF_TEST_POINTS); //1ch data
    gx8002_enc_data_input(tmp_buf, tmp_buf, BUF_TEST_POINTS); //two ch data

    return ;
}

static void gx8002_enc_close_test(void *priv)
{
    gx8002_enc_close();
}

static int gx8002_enc_test(void)
{
    gx8002_enc_open();
    gx8002_enc_start();

    sys_timer_add(NULL, gx8002_enc_input_test, 50);
    sys_timer_add(NULL, gx8002_enc_close_test, 1000 * 60 * 3);

    return 0;
}
//late_initcall(gx8002_enc_test);


#endif /* #if TCFG_GX8002_ENC_ENABLE */
