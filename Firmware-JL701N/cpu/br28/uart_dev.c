/**
 * @file uart.c
 * @author JL
 * @brief 串口UART模块C文件
 * @version 1.2
 * @date 2018-11-22
 */

#include "asm/clock.h"
#include "asm/uart_dev.h"
#include "asm/cpu.h"
#include "generic/gpio.h"
#include "spinlock.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".uart_bss")
#pragma data_seg(".uart_data")
#pragma const_seg(".uart_const")
#pragma code_seg(".uart_code")
#endif

#define UART_CLK  clk_get("uart")
#define UART_OT_CLK  clk_get("lsb")



static uart_bus_t uart0;
static uart_bus_t uart1;
static uart_bus_t uart2;

/* _WEAK_ */
/* extern */
const u32 CONFIG_UART0_ENABLE = 1;

/* _WEAK_ */
/* extern */
const u32 CONFIG_UART1_ENABLE  = 1;

/* _WEAK_ */
/* extern */
const u32 CONFIG_UART2_ENABLE  = 1;

/* _WEAK_ */
/* extern */
const u32 CONFIG_UART0_ENABLE_TX_DMA =  1;

/* _WEAK_ */
/* extern */
const u32 CONFIG_UART1_ENABLE_TX_DMA =  1;

/* _WEAK_ */
/* extern */
const u32 CONFIG_UART2_ENABLE_TX_DMA =  1;

static u32 kfifo_get(KFIFO *kfifo, u8 *buffer, u32 len)
{
    unsigned int i;
    len = MIN(len, kfifo->buf_in - kfifo->buf_out);

    i = MIN(len, kfifo->buf_size - (kfifo->buf_out & (kfifo->buf_size - 1)));

    memcpy(buffer, kfifo->buffer + (kfifo->buf_out & (kfifo->buf_size - 1)), i);

    memcpy(buffer + i, kfifo->buffer, len - i);

    kfifo->buf_out += len;
    return len;

}
static u32 kfifo_length(KFIFO *kfifo)
{
    return kfifo->buf_in - kfifo->buf_out;
}
/**
 * @brief ut0发送一个byte
 *
 * @param a 要发送的字节
 */
static void UT0_putbyte(char a)
{
    if (JL_UART0->CON0 & BIT(0)) {
        JL_UART0->BUF = a;
        __asm__ volatile("csync");
        while ((JL_UART0->CON0 & BIT(15)) == 0);
        JL_UART0->CON0 |= BIT(13);
    }
}
/**
 * @brief ut0接收一个byte
 *
 * @param buf 字节存放地址
 * @param timeout 接收超时时间，单位1ms
 * @return 返回0：接收失败；返回1：接收成功
 */
static u8 UT0_getbyte(u8 *buf, u32 timeout)
{
    u32 _timeout, _t_sleep;
    timeout = ut_msecs_to_jiffies(timeout);
    if (JL_UART0->CON0 & BIT(6)) {
        //DMA_MODE
        if (!kfifo_length(&uart0.kfifo)) {
            UT_OSSemPend(&uart0.sem_rx, timeout);
        }
        UT_OSSemSet(&uart0.sem_rx, 0);

        return kfifo_get(&uart0.kfifo, buf, 1);
    } else {
        _timeout = timeout + ut_get_jiffies();
        _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
        while (!(JL_UART0->CON0 & BIT(14))) {
            if (timeout && time_before(_timeout, ut_get_jiffies())) {
                return 0;
            }
            if (time_before(_t_sleep, ut_get_jiffies())) {
                ut_sleep();
                _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
            }
        }
        *buf = JL_UART0->BUF;
        JL_UART0->CON0 |= BIT(12);
        __asm__ volatile("csync");  //make RX_PND_CLR taking effect
    }
    return 1;
}
/**
 * @brief ut0中断函数
 */
SET_INTERRUPT
static void UT0_isr_fun(void)
{
    u32 rx_len = 0;
    if ((JL_UART0->CON0 & BIT(2)) && (JL_UART0->CON0 & BIT(15))) {
        JL_UART0->CON0 |= BIT(13);
        UT_OSSemPost(&uart0.sem_tx);
        if (uart0.isr_cbfun) {
            uart0.isr_cbfun(&uart0, UT_TX);
        }
    }
    if ((JL_UART0->CON0 & BIT(3)) && (JL_UART0->CON0 & BIT(14))) {
        JL_UART0->CON0 |= BIT(7);                     //DMA模式
        JL_UART0->CON0 |= BIT(12);           //清RX PND
        rx_len = JL_UART0->HRXCNT;             //读当前串口接收数据的个数
        /* uart0.kfifo.buf_in += uart0.frame_length; //每满frame_length字节则产生一次中断 */
        uart0.kfifo.buf_in += rx_len;
        UT_OSSemPost(&uart0.sem_rx);
        if (uart0.isr_cbfun) {
            uart0.isr_cbfun(&uart0, UT_RX);
        }
    }
    if ((JL_UART0->CON0 & BIT(5)) && (JL_UART0->CON0 & BIT(11))) {
        //OTCNT PND
        JL_UART0->CON0 |= BIT(7);                     //DMA模式
        JL_UART0->CON0 |= BIT(10);                    //清OTCNT PND
        JL_UART0->CON0 |= BIT(12);                    //清RX PND(这里的顺序不能改变，这里要清一次)
        rx_len = JL_UART0->HRXCNT;             //读当前串口接收数据的个数

        if (rx_len) {
            uart0.kfifo.buf_in += rx_len;
            /* printf("%s() %d\n", __func__, __LINE__); */
            UT_OSSemPost(&uart0.sem_rx);
            if (uart0.isr_cbfun) {
                uart0.isr_cbfun(&uart0, UT_RX_OT);
            }
        }
    }
}
/**
 * @brief ut0接收字符串
 *
 * @param buf 字符串存放首地址
 * @param len 预接收长度
 * @param timeout 接收超时时间，单位1ms
 * @return 返回实际接收长度
 */
static u32 UT0_read_buf(u8 *buf, u32 len, u32 timeout)
{
    u32 i;
    u32 _timeout, _t_sleep;
    if (len == 0) {
        return 0;
    }
    timeout = ut_msecs_to_jiffies(timeout);
    if (JL_UART0->CON0 & BIT(6)) {

        if (!kfifo_length(&uart0.kfifo)) {
            UT_OSSemPend(&uart0.sem_rx, timeout);
        }
        UT_OSSemSet(&uart0.sem_rx, 0);

        return kfifo_get(&uart0.kfifo, buf, len);
    } else {
        _timeout = timeout + ut_get_jiffies();
        _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
        for (i = 0; i < len; i++) {
            while (!(JL_UART0->CON0 & BIT(14))) {
                if (timeout && time_before(_timeout, ut_get_jiffies())) {
                    return i;
                }
                if (time_before(_t_sleep, ut_get_jiffies())) {
                    ut_sleep();
                    _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
                }
            }
            *(buf + i) = JL_UART0->BUF;
            JL_UART0->CON0 |= BIT(12);
            __asm__ volatile("csync");  //make RX_PND_CLR taking effect
        }
    }
    return len;
}
/**
 * @brief ut0发送字符串
 *
 * @param buf 字符串首地址
 * @param len 发送的字符串长度
 */
static void UT0_write_buf(const u8 *buf, u32 len)
{
    u32 i;
    if (len == 0) {
        return;
    }
    if (CONFIG_UART0_ENABLE_TX_DMA) {
        UT_OSSemSet(&uart0.sem_tx, 0);
        JL_UART0->CON0 |= BIT(13);
        JL_UART0->CON0 |= BIT(2);
        JL_UART0->TXADR = (u32)buf;
        JL_UART0->TXCNT = len;
        UT_OSSemPend(&uart0.sem_tx, 0);
        JL_UART0->CON0 &= ~BIT(2);
    } else {
        for (i = 0; i < len; i ++) {
            UT0_putbyte(*(buf + i));
        }
    }
}
/**
 * @brief ut0配置波特率
 *
 * @param baud 波特率值
 */
static void UT0_set_baud(u32 baud)
{
    JL_UART0->CON0 &= ~(BIT(0) | BIT(1));
    JL_UART0->CON0 |= BIT(13) | BIT(12) | BIT(10);
    JL_UART0->BAUD = ((UART_CLK + baud / 2) / baud) / 4 - 1;
    if (JL_UART0->CON0 & BIT(5)) {
        if (uart0.rx_timeout > 10) {
            JL_UART0->OTCNT = (uart0.rx_timeout / 10) * (UART_OT_CLK / 10) / 10;
        } else {
            JL_UART0->OTCNT = uart0.rx_timeout * UART_OT_CLK / 1000;
        }
    }
    JL_UART0->CON0 |= BIT(13) | BIT(12) | BIT(10) | BIT(0) | BIT(1);
}

/**
*获取串口数据长度
*/
static u32 uart1_get_data_len()
{
    return kfifo_length(&uart1.kfifo);
}

/**
 * @brief ut0使能
 */
static void UT0_open(u32 baud, u32 is_9bit, void *cbuf, u32 cbuf_size, u32 rx_cnt, u32 ot)
{
    JL_UART0->CON0 = BIT(13) | BIT(12) | BIT(10);
    UT_OSSemCreate(&uart0.sem_rx, 0);
    UT_OSSemCreate(&uart0.sem_tx, 0);
    request_irq(IRQ_UART0_IDX, 3, UT0_isr_fun, 0);
    if (cbuf) {
        uart0.kfifo.buffer = cbuf;
        uart0.kfifo.buf_size = cbuf_size;
        uart0.kfifo.buf_in = 0;
        uart0.kfifo.buf_out = 0;
        uart0.frame_length = rx_cnt;
        uart0.rx_timeout = ot;
        JL_UART0->RXSADR = (u32)uart0.kfifo.buffer;
        JL_UART0->RXEADR = (u32)(uart0.kfifo.buffer + uart0.kfifo.buf_size);
        JL_UART0->RXCNT = uart0.frame_length;
        JL_UART0->CON0 |= BIT(6) | BIT(5) | BIT(3);
    }
    if (is_9bit) {
        JL_UART0->CON2 |= BIT(0);
    } else {
        JL_UART0->CON2 &= ~BIT(0);
    }
    UT0_set_baud(baud);
}
/**
 * @brief ut0关闭，注销
 */
static void UT0_close(void)
{
    UT_OSSemClose(&uart0.sem_rx);
    UT_OSSemClose(&uart0.sem_tx);
    irq_disable(IRQ_UART0_IDX);
    JL_UART0->CON0 = BIT(13) | BIT(12) | BIT(10);
}
/**
 * @brief ut1发送一个byte
 *
 * @param a 要发送的字节
 */
static void UT1_putbyte(char a)
{
    if (JL_UART1->CON0 & BIT(0)) {
        JL_UART1->BUF = a;
        __asm__ volatile("csync");
        while ((JL_UART1->CON0 & BIT(15)) == 0);
        JL_UART1->CON0 |= BIT(13);
    }
}
/**
 * @brief ut1接收一个byte
 *
 * @param buf 字节存放地址
 * @param timeout 接收超时时间，单位1ms
10ms¥；返回1：接收成功
 */
static u8 UT1_getbyte(u8 *buf, u32 timeout)
{
    u32 _timeout, _t_sleep;
    timeout = ut_msecs_to_jiffies(timeout);
    if (JL_UART1->CON0 & BIT(6)) {
        if (!kfifo_length(&uart1.kfifo)) {
            UT_OSSemPend(&uart1.sem_rx, timeout);
        }
        UT_OSSemSet(&uart1.sem_rx, 0);

        return kfifo_get(&uart1.kfifo, buf, 1);
    } else {
        _timeout = timeout + ut_get_jiffies();
        _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
        while (!(JL_UART1->CON0 & BIT(14))) {
            if (timeout && time_before(_timeout, ut_get_jiffies())) {
                return 0;
            }
            if (time_before(_t_sleep, ut_get_jiffies())) {
                ut_sleep();
                _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
            }
        }
        *buf = JL_UART1->BUF;
        JL_UART1->CON0 |= BIT(12);
        __asm__ volatile("csync");  //make RX_PND_CLR taking effect
    }
    return 1;
}
/**
 * @brief ut1中断函数
 */
SET_INTERRUPT
static void UT1_isr_fun(void)
{
    u32 rx_len = 0;
    if ((JL_UART1->CON0 & BIT(2)) && (JL_UART1->CON0 & BIT(15))) {
        JL_UART1->CON0 |= BIT(13);
        UT_OSSemPost(&uart1.sem_tx);
        if (uart1.isr_cbfun) {
            uart1.isr_cbfun(&uart1, UT_TX);
        }
    }
    if ((JL_UART1->CON0 & BIT(3)) && (JL_UART1->CON0 & BIT(14))) {
        JL_UART1->CON0 |= BIT(7);                     //DMA模式
        JL_UART1->CON0 |= BIT(12);           //清RX PND
        rx_len = JL_UART1->HRXCNT;             //读当前串口接收数据的个数
        /* uart1.kfifo.buf_in += uart1.frame_length; //每满32字节则产生一次中断 */
        uart1.kfifo.buf_in += rx_len;
        UT_OSSemPost(&uart1.sem_rx);
        if (uart1.isr_cbfun) {
            uart1.isr_cbfun(&uart1, UT_RX);
        }
    }

    if ((JL_UART1->CON0 & BIT(5)) && (JL_UART1->CON0 & BIT(11))) {
        //OTCNT PND
        JL_UART1->CON0 |= BIT(7);                     //DMA模式
        JL_UART1->CON0 |= BIT(10);                    //清OTCNT PND
        JL_UART1->CON0 |= BIT(12);                    //清RX PND(这里的顺序不能改变，这里要清一次)
        rx_len = JL_UART1->HRXCNT;             //读当前串口接收数据的个数

        if (rx_len) {
            uart1.kfifo.buf_in += rx_len;
            /* printf("%s() %d\n", __func__, __LINE__); */
            UT_OSSemPost(&uart1.sem_rx);
            if (uart1.isr_cbfun) {
                uart1.isr_cbfun(&uart1, UT_RX_OT);
            }
        }
    }
}
/**
 * @brief ut1接收字符串
 *
 * @param buf 字符串存放首地址
 * @param len 预接收长度
 * @param timeout 接收超时时间，单位1ms
 * @return 返回实际接收的长度
 */
static u32 UT1_read_buf(u8 *buf, u32 len, u32 timeout)
{
    u32 i;
    u32 _timeout, _t_sleep;
    if (len == 0) {
        return 0;
    }
    timeout = ut_msecs_to_jiffies(timeout);
    if (JL_UART1->CON0 & BIT(6)) {
        if (!kfifo_length(&uart1.kfifo)) {
            UT_OSSemPend(&uart1.sem_rx, timeout);
        }
        UT_OSSemSet(&uart1.sem_rx, 0);

        return kfifo_get(&uart1.kfifo, buf, len);
    } else {
        _timeout = timeout + ut_get_jiffies();
        _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
        for (i = 0; i < len; i++) {
            while (!(JL_UART1->CON0 & BIT(14))) {
                if (timeout && time_before(_timeout, ut_get_jiffies())) {
                    return i;
                }
                if (time_before(_t_sleep, ut_get_jiffies())) {
                    ut_sleep();
                    _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
                }
            }
            *(buf + i) = JL_UART1->BUF;
            JL_UART1->CON0 |= BIT(12);
            __asm__ volatile("csync");  //make RX_PND_CLR taking effect
        }
    }
    return len;
}
/**
 * @brief ut1发送字符串
 *
 * @param buf 字符串首地址
 * @param len 发送的字符串长度
 * @param timeout 发送超时时间，单位10ms
 */
static void UT1_write_buf(const u8 *buf, u32 len)
{
    u32 i;
    if (len == 0) {
        return;
    }
    if (CONFIG_UART1_ENABLE_TX_DMA) {
        UT_OSSemSet(&uart1.sem_tx, 0);
        JL_UART1->CON0 |= BIT(13);
        JL_UART1->CON0 |= BIT(2);
        JL_UART1->TXADR = (u32)buf;
        JL_UART1->TXCNT = len;
        UT_OSSemPend(&uart1.sem_tx, 0);
        JL_UART1->CON0 &= ~BIT(2);
    } else {
        for (i = 0; i < len; i ++) {
            UT1_putbyte(*(buf + i));
        }
    }
}
/**
 * @brief ut1配置波特率
 *
 * @param baud 波特率值
 */
static void UT1_set_baud(u32 baud)
{
    JL_UART1->CON0 &= ~(BIT(0) | BIT(1));
    JL_UART1->CON0 |= BIT(13) | BIT(12) | BIT(10);
    JL_UART1->BAUD = ((UART_CLK + baud / 2) / baud) / 4 - 1;
    if (JL_UART1->CON0 & BIT(5)) {
        if (uart1.rx_timeout > 10) {
            JL_UART1->OTCNT = (uart1.rx_timeout / 10) * (UART_OT_CLK / 10) / 10;
        } else {
            JL_UART1->OTCNT = uart1.rx_timeout * UART_OT_CLK / 1000;
        }
    }
    JL_UART1->CON0 |= BIT(13) | BIT(12) | BIT(10) | BIT(0) | BIT(1);
}
/**
 * @brief ut1使能
 */
static void UT1_open(u32 baud, u32 is_9bit, void *cbuf, u32 cbuf_size, u32 rx_cnt, u32 ot)
{
    JL_UART1->CON0 = BIT(13) | BIT(12) | BIT(10);
    UT_OSSemCreate(&uart1.sem_rx, 0);
    UT_OSSemCreate(&uart1.sem_tx, 0);
    request_irq(IRQ_UART1_IDX, 3, UT1_isr_fun, 0);
    if (cbuf) {
        uart1.kfifo.buffer = cbuf;
        uart1.kfifo.buf_size = cbuf_size;
        uart1.kfifo.buf_in = 0;
        uart1.kfifo.buf_out = 0;
        uart1.frame_length = rx_cnt;
        uart1.rx_timeout = ot;
        JL_UART1->RXSADR = (u32)uart1.kfifo.buffer;
        JL_UART1->RXEADR = (u32)(uart1.kfifo.buffer + uart1.kfifo.buf_size);
        JL_UART1->RXCNT = uart1.frame_length;
        JL_UART1->CON0 |= BIT(6) | BIT(5) | BIT(3);
    }
    if (is_9bit) {
        JL_UART1->CON2 |= BIT(0);
    } else {
        JL_UART1->CON2 &= ~BIT(0);
    }
    UT1_set_baud(baud);
}
/**
 * @brief ut1关闭，注销
 */
static void UT1_close(void)
{
    UT_OSSemClose(&uart1.sem_rx);
    UT_OSSemClose(&uart1.sem_tx);
    irq_disable(IRQ_UART1_IDX);
    JL_UART1->CON0 = BIT(13) | BIT(12) | BIT(10);
}
/**
 * @brief ut2发送一个byte
 *
 * @param a 要发送的字节
 */
static void UT2_putbyte(char a)
{
    if (JL_UART2->CON0 & BIT(0)) {
        JL_UART2->BUF = a;
        __asm__ volatile("csync");
        while ((JL_UART2->CON0 & BIT(15)) == 0);
        JL_UART2->CON0 |= BIT(13);
    }
}
/**
 * @brief ut2接收一个byte
 *
 * @param buf 字节存放地址
 * @param timeout 接收超时时间，单位1ms
 * @return 返回0：接收失败；返回1：接收成功
 */
static u8 UT2_getbyte(u8 *buf, u32 timeout)
{
    u32 _timeout, _t_sleep;
    timeout = ut_msecs_to_jiffies(timeout);

    if (JL_UART2->CON0 & BIT(6)) {
        if (!kfifo_length(&uart2.kfifo)) {
            UT_OSSemPend(&uart2.sem_rx, timeout);
        }
        UT_OSSemSet(&uart2.sem_rx, 0);

        return kfifo_get(&uart2.kfifo, buf, 1);
    } else {
        _timeout = timeout + ut_get_jiffies();
        _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
        while (!(JL_UART2->CON0 & BIT(14))) {
            if (timeout && time_before(_timeout, ut_get_jiffies())) {
                return 0;
            }
            if (time_before(_t_sleep, ut_get_jiffies())) {
                ut_sleep();
                _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
            }
        }
        *buf = JL_UART2->BUF;
        JL_UART2->CON0 |= BIT(12);
        __asm__ volatile("csync");  //make RX_PND_CLR taking effect
    }

    return 1;
}
/**
 * @brief ut2中断函数
 */
SET_INTERRUPT
static void UT2_isr_fun(void)
{
    //JL_UART2->CON0 |= (BIT(13) | BIT(12) | BIT(10));
    u32 rx_len = 0;
    if ((JL_UART2->CON0 & BIT(2)) && (JL_UART2->CON0 & BIT(15))) {
        JL_UART2->CON0 |= BIT(13);
        UT_OSSemPost(&uart2.sem_tx);
        if (uart2.isr_cbfun) {
            uart2.isr_cbfun(&uart2, UT_TX);
        }
    }
    if ((JL_UART2->CON0 & BIT(3)) && (JL_UART2->CON0 & BIT(14))) {
        JL_UART2->CON0 |= BIT(7);                     //DMA模式
        JL_UART2->CON0 |= BIT(12);           //清RX PND
        rx_len = JL_UART2->HRXCNT;             //读当前串口接收数据的个数
        /* uart2.kfifo.buf_in += uart2.frame_length; //每满32字节则产生一次中断 */
        uart2.kfifo.buf_in += rx_len;
        UT_OSSemPost(&uart2.sem_rx);
        if (uart2.isr_cbfun) {
            uart2.isr_cbfun(&uart2, UT_RX);
        }
    }

    if ((JL_UART2->CON0 & BIT(5)) && (JL_UART2->CON0 & BIT(11))) {
        //OTCNT PND
        JL_UART2->CON0 |= BIT(7);                     //DMA模式
        JL_UART2->CON0 |= BIT(10);                    //清OTCNT PND
        JL_UART2->CON0 |= BIT(12);                    //清RX PND(这里的顺序不能改变，这里要清一次)
        rx_len = JL_UART2->HRXCNT;             //读当前串口接收数据的个数

        if (rx_len) {
            uart2.kfifo.buf_in += rx_len;
            /* printf("%s() %d\n", __func__, __LINE__); */
            UT_OSSemPost(&uart2.sem_rx);
            if (uart2.isr_cbfun) {
                uart2.isr_cbfun(&uart2, UT_RX_OT);
            }
        }
    }
}

/**
 * @brief ut1接收字符串
 *
 * @param buf 字符串存放首地址
 * @param len 预接收长度
 * @param timeout 接收超时时间，单位1ms
 * @return 返回实际接收的长度
 */
static u32 UT2_read_buf(u8 *buf, u32 len, u32 timeout)
{
    u32 i;
    u32 _timeout, _t_sleep;
    if (len == 0) {
        return 0;
    }

    timeout = ut_msecs_to_jiffies(timeout);
    if (JL_UART2->CON0 & BIT(6)) {
        if (!kfifo_length(&uart2.kfifo)) {
            UT_OSSemPend(&uart2.sem_rx, timeout);
        }
        UT_OSSemSet(&uart2.sem_rx, 0);
        return kfifo_get(&uart2.kfifo, buf, len);
    } else {
        _timeout = timeout + ut_get_jiffies();
        _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
        for (i = 0; i < len; i ++) {
            while (!(JL_UART2->CON0 & BIT(14))) {
                if (timeout && time_before(_timeout, ut_get_jiffies())) {
                    return i;
                }
                if (time_before(_t_sleep, ut_get_jiffies())) {
                    ut_sleep();
                    _t_sleep = ut_msecs_to_jiffies(10) + ut_get_jiffies();
                }
            }
            *(buf + i) = JL_UART2->BUF;
            JL_UART2->CON0 |= BIT(12);
            __asm__ volatile("csync");  //make RX_PND_CLR taking effect
        }
    }
    return len;
}
/**
 * @brief ut2发送字符串
 *
 * @param buf 字符串首地址
 * @param len 发送的字符串长度
 * @param timeout 发送超时时间，单位10ms
 */
static void UT2_write_buf(const u8 *buf, u32 len)
{
    u32 i;
    if (len == 0) {
        return;
    }
    if (CONFIG_UART2_ENABLE_TX_DMA) {
        UT_OSSemSet(&uart2.sem_tx, 0);
        JL_UART2->CON0 |= BIT(13);
        JL_UART2->CON0 |= BIT(2);
        JL_UART2->TXADR = (u32)buf;
        JL_UART2->TXCNT = len;
        UT_OSSemPend(&uart2.sem_tx, 0);
        JL_UART2->CON0 &= ~BIT(2);
    } else {
        for (i = 0; i < len; i++) {
            UT2_putbyte(*(buf + i));
        }
    }
}
/**
 * @brief ut2配置波特率
 *
 * @param baud 波特率值
 */
static void UT2_set_baud(u32 baud)
{
    JL_UART2->CON0 &= ~(BIT(0) | BIT(1));
    JL_UART2->CON0 |= BIT(13) | BIT(12) | BIT(10);
    JL_UART2->BAUD = ((UART_CLK + baud / 2) / baud) / 4 - 1;
    if (JL_UART2->CON0 & BIT(5)) {
        if (uart2.rx_timeout > 10) {
            JL_UART2->OTCNT = (uart2.rx_timeout / 10) * (UART_OT_CLK / 10) / 10;
        } else {
            JL_UART2->OTCNT = uart2.rx_timeout * UART_OT_CLK / 1000;
        }
    }
    JL_UART2->CON0 |= BIT(13) | BIT(12) | BIT(10) | BIT(0) | BIT(1);
}

/**
 * @brief ut2使能
 */
static void UT2_open(u32 baud, u32 is_9bit, void *cbuf, u32 cbuf_size, u32 rx_cnt, u32 ot)
{
    JL_UART2->CON0 = BIT(13) | BIT(12) | BIT(10);
    UT_OSSemCreate(&uart2.sem_rx, 0);
    UT_OSSemCreate(&uart2.sem_tx, 0);
    request_irq(IRQ_UART2_IDX, 3, UT2_isr_fun, 0);
    if (cbuf) {
        uart2.kfifo.buffer = cbuf;
        uart2.kfifo.buf_size = cbuf_size;
        uart2.kfifo.buf_in = 0;
        uart2.kfifo.buf_out = 0;
        uart2.frame_length = rx_cnt;
        uart2.rx_timeout = ot;
        JL_UART2->RXSADR = (u32)uart2.kfifo.buffer;
        JL_UART2->RXEADR = (u32)(uart2.kfifo.buffer + uart2.kfifo.buf_size);
        JL_UART2->RXCNT = uart2.frame_length;
        JL_UART2->CON0 |= BIT(6) | BIT(5) | BIT(3);
    }
    if (is_9bit) {
        JL_UART2->CON2 |= BIT(0);
    } else {
        JL_UART2->CON2 &= ~BIT(0);
    }
    UT2_set_baud(baud);
}

/**
 * @brief ut2关闭，注销
 */
static void UT2_close(void)
{
    UT_OSSemClose(&uart2.sem_rx);
    UT_OSSemClose(&uart2.sem_tx);
    irq_disable(IRQ_UART2_IDX);
    JL_UART2->CON0 = BIT(13) | BIT(12) | BIT(10);
}

static u32 uart_is_idle(u32 ut_num)
{
    switch (ut_num) {
    case 0 :
        return !(JL_UART0->CON0 & BIT(0));
    case 1 :
        return !(JL_UART1->CON0 & BIT(0));
    case 2:
        return !(JL_UART2->CON0 & BIT(0));
    default :
        break;
    }
    return 0;
}

static u8 uart_config(const struct uart_platform_data_t *arg, u8 tx_ch, enum PFI_TABLE rx_pfun)
{
    if (!(arg->tx_pin < IO_PORT_MAX || arg->rx_pin < IO_PORT_MAX)) {
        return -1;
    }

    if (arg->tx_pin < IO_PORT_MAX) {
        gpio_direction_output(arg->tx_pin, 1);
        gpio_set_fun_output_port(arg->tx_pin, tx_ch, 1, 1, LOW_POWER_FREE);
    }
    if (arg->rx_pin < IO_PORT_MAX) {
        gpio_direction_input(arg->rx_pin);
        gpio_set_pull_up(arg->rx_pin, 1);
        gpio_set_die(arg->rx_pin, 1);
        gpio_set_fun_input_port(arg->rx_pin, rx_pfun, LOW_POWER_FREE);
    }
    return 0;
}

/**
 * @brief ut模块初始const struct uart_platform_data_t *arg化函数，供外部调用
 *
 * @param arg 传入uartconst struct uart_platform_data_t *arg_argment型结构体指针
 * @return 返回uart_buconst struct uart_platform_data_t *args_t型结构体指针
 */
__attribute__((noinline))
const uart_bus_t *uart_dev_open(const struct uart_platform_data_t *arg)
{
    u8 ut_num;
    uart_bus_t *ut = NULL;
    u8 gpio_input_channle_flag = 0;

    if (uart_is_idle(0)) {
        ut_num = 0;
    } else if (uart_is_idle(1)) {
        ut_num = 1;
    } else if ((uart_is_idle(2))) {
        ut_num = 2;
    } else {
        return NULL;
    }
    if (arg->rx_cbuf) {
        if ((arg->rx_cbuf_size == 0) || (arg->rx_cbuf_size & (arg->rx_cbuf_size - 1))) {
            return NULL;
        }
    }

    if (CONFIG_UART0_ENABLE && ut_num == 0) {
        //gpio_set_uart0(ut_ch);
        if (uart_config(arg, FO_UART0_TX, PFI_UART0_RX)) {
            return NULL;
        }

        uart0.argv = arg->argv;
        uart0.isr_cbfun = arg->isr_cbfun;
        uart0.putbyte = UT0_putbyte;
        uart0.getbyte = UT0_getbyte;
        uart0.read = UT0_read_buf;
        uart0.write = UT0_write_buf;
        uart0.set_baud = UT0_set_baud;
        UT0_open(arg->baud, arg->is_9bit,
                 arg->rx_cbuf, arg->rx_cbuf_size,
                 arg->frame_length, arg->rx_timeout);
        ut = &uart0;
    } else if (CONFIG_UART1_ENABLE && ut_num == 1) {
        //gpio_set_uart1(ut_ch);
        if (uart_config(arg, FO_UART1_TX, PFI_UART1_RX)) {
            return NULL;
        }
        uart1.argv = arg->argv;
        uart1.isr_cbfun = arg->isr_cbfun;
        uart1.putbyte = UT1_putbyte;
        uart1.getbyte = UT1_getbyte;
        uart1.read    = UT1_read_buf;
        uart1.write   = UT1_write_buf;
        uart1.set_baud = UT1_set_baud;
        uart1.get_data_len = uart1_get_data_len;
        UT1_open(arg->baud, arg->is_9bit,
                 arg->rx_cbuf, arg->rx_cbuf_size,
                 arg->frame_length, arg->rx_timeout);
        ut = &uart1;
    } else if (CONFIG_UART2_ENABLE && ut_num == 2) {
        //gpio_set_uart2(ut_ch);
        if (uart_config(arg, FO_UART2_TX, PFI_UART2_RX)) {
            return NULL;
        }
        uart2.argv = arg->argv;
        uart2.isr_cbfun = arg->isr_cbfun;
        uart2.putbyte = UT2_putbyte;
        uart2.getbyte = UT2_getbyte;
        uart2.read    = UT2_read_buf;
        uart2.write   = UT2_write_buf;
        uart2.set_baud = UT2_set_baud;
        UT2_open(arg->baud, arg->is_9bit,
                 arg->rx_cbuf, arg->rx_cbuf_size,
                 arg->frame_length, arg->rx_timeout);
        ut = &uart2;
    } else {
        return NULL;
    }
    return ut;
}
u32 uart_dev_close(uart_bus_t *ut)
{
    UT_OSSemClose(&ut->sem_rx);
    UT_OSSemClose(&ut->sem_tx);
    if (&uart0 == ut) {
        UT0_close();
        return gpio_close_uart0();
    } else if (&uart1 == ut) {
        UT1_close();
        return gpio_close_uart1();
    } else {
        UT2_close();
        return gpio_close_uart2();
    }
    return 0;
}

void uart_disable_for_ota()
{
    JL_UART0->CON0 = BIT(13) | BIT(12) | BIT(10);
    JL_UART1->CON0 = BIT(13) | BIT(12) | BIT(10);
    JL_UART2->CON0 = BIT(13) | BIT(12) | BIT(10);
}

static u8 _rts_io = -1;
static u8 _cts_io = -1;
void uart1_flow_ctl_init(u8 rts_io, u8 cts_io)
{
    JL_UART1->CON1 = 0;
    //RTS
    if (rts_io < IO_PORT_MAX) {
        _rts_io = rts_io;
        gpio_set_die(rts_io, 1);
        gpio_set_direction(rts_io, 0);
        gpio_set_pull_up(rts_io, 0);
        gpio_set_pull_down(rts_io, 0);
        gpio_write(rts_io, 0);
        gpio_set_fun_output_port(rts_io, FO_UART1_RTS, 1, 1, LOW_POWER_FREE);
        JL_UART1->CON1 |= BIT(13) | BIT(0);
    }
    //CTS
    if (cts_io < IO_PORT_MAX) {
        _cts_io = cts_io;
        gpio_set_die(cts_io, 1);
        gpio_set_direction(cts_io, 1);
        gpio_set_pull_up(cts_io, 0);
        gpio_set_pull_down(cts_io, 1);
        gpio_set_fun_input_port(cts_io, PFI_UART1_CTS, LOW_POWER_FREE);
        JL_UART1->CON1 |= BIT(14) | BIT(2);
    }
}

void uart1_flow_ctl_rts_suspend(void)
{
    if (!(JL_UART1->CON1 & BIT(0))) {
        gpio_write(_rts_io, 1);      //告诉对方，自己忙碌
    }
    JL_UART1->CON1 |= BIT(4);   //硬件停止接收
}

void uart1_flow_ctl_rts_resume(void)
{
    JL_UART1->CON1 &= ~BIT(4);  //硬件可以接收
    if (JL_UART1->CON1 & BIT(0)) {
        JL_UART1->CON1 |= BIT(13);
    } else {
        gpio_write(_rts_io, 0);      //表示可以继续接收数据
    }
}

