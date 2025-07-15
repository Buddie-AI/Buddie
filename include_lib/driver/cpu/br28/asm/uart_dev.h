#ifndef _UART_DEV_H_
#define _UART_DEV_H_

#include "typedef.h"
#include "os/os_api.h"
#include "jiffies.h"
#include "irq.h"
/* #include "jtime.h" */

#define	CONFIG_ENABLE_UART_SEM	1
#define     SET_INTERRUPT   ___interrupt
#define irq_disable(x)  bit_clr_ie(x)
#define irq_enable(x)  bit_set_ie(x)

#ifndef time_after
#define time_after(a,b)  (((long)(b) - (long)(a)) < 0)
#endif

#ifndef time_before
#define time_before(a,b) time_after(b,a)
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

static inline u32 ut_get_jiffies(void)
{
#if 1
    return jiffies;
#endif
#if 0
    return Jtime_updata_jiffies();
#endif
}

static inline u32 ut_msecs_to_jiffies(u32 msecs)
{
    if (msecs >= 10) {
        msecs /= 10;
    } else if (msecs) {
        msecs = 1;
    }
    return msecs;
}

#if CONFIG_ENABLE_UART_SEM
typedef OS_SEM UT_Semaphore ;
static inline void UT_OSSemCreate(UT_Semaphore *sem, u32 count)
{
    os_sem_create(sem, count);
}
static inline void UT_OSSemPost(UT_Semaphore *sem)
{
    os_sem_post(sem);
}
static inline u32 UT_OSSemPend(UT_Semaphore *sem, u32 timeout)
{
    return os_sem_pend(sem, timeout);
}
static inline void UT_OSSemSet(UT_Semaphore *sem, u32 count)
{
    os_sem_set(sem, count);
}
static inline void UT_OSSemClose(UT_Semaphore *sem)
{

}
static inline void ut_sleep()
{
    os_time_dly(1);
}

#else
typedef volatile u32 UT_Semaphore;
static inline void UT_OSSemCreate(UT_Semaphore *sem, u32 count)
{
    *sem = count;
}
static inline void UT_OSSemPost(UT_Semaphore *sem)
{
    (*sem)++;
}
static inline u32 UT_OSSemPend(UT_Semaphore *sem, u32 timeout)
{
    u32 _timeout = timeout + ut_get_jiffies();
    extern void clr_wdt();
    while (1) {
        if (*sem) {
            (*sem) --;
            break;
        }
        if ((timeout != 0) && time_before(_timeout, ut_get_jiffies())) {
            return -1;
        }
        clr_wdt();
    }
    return 0;
}
static inline void UT_OSSemSet(UT_Semaphore *sem, u32 count)
{
    *sem = count;
}
static inline void UT_OSSemClose(UT_Semaphore *sem)
{

}
static inline void ut_sleep()
{
    extern void clr_wdt();
    clr_wdt();
}
#endif


typedef void (*ut_isr_cbfun)(void *ut_bus, u32 status);
struct uart_platform_data_t {
    u8 tx_pin;                                          ///< 作为发送引脚的引脚号，可从参考gpio.h枚举中选，当引脚为空时，则填 -1
    u8 rx_pin;                                          ///< 作为接收引脚的引脚号，可从参考gpio.h枚举中选，当引脚为空时，则填 -1
    void *rx_cbuf;                                      ///< 如果使用中断DMA接收，则写入循环buf的首地址，ut中断使能；如果不使用，则写入NULL，无中断
    u32 rx_cbuf_size;                                   ///< 循环buf的大小,必须为2的多少几次幂，如果不用循环buf，该值无效，可写NULL
    u32 frame_length;                                   ///< 产生RT中断的字节数，如无中断，该值无效
    u32 rx_timeout;                                     ///< 产生OT中断的时间值，单位ms，如无中断，该值无效
    ut_isr_cbfun isr_cbfun; 			                ///< ut中断的回调函数句柄，不用回调函数则写入NULL，如无中断，句柄无效
    void *argv;                                         ///< ut中断的回调函数的一个扩展形参，可供用户设定，如无回调函数，此参数无效
    u32 is_9bit: 1;                                     ///< ut九位模式使能位，0：关闭；1：使能
    u32 baud: 24;                                       ///< ut的波特率
};

/**
 * @brief 循环buf结构体类型定义
 */
typedef struct {
    u8 *buffer;                                         ///<循环buf的首地址
    u32 buf_size;                                       ///<循环buf的大小
    u32 buf_in;                                         ///<循环buf的写偏移量
    u32 buf_out;                                        ///<循环buf的读偏移量
} KFIFO;

enum {
    UT_TX = 1,
    UT_RX,
    UT_RX_OT
};

/**
 * @brief ut初始化函数的返回结构体，含各函数指针，供外部使用
 */
typedef struct {
    ut_isr_cbfun isr_cbfun;                             ///< ut中断的回调函数句柄，不用回调函数则写入NULL，如无中断，句柄无效
    void *argv;                                         ///< ut中断的回调函数的一个扩展形参,在此返回
    void (*putbyte)(char a);                            ///< ut发送一个byte
    u8(*getbyte)(u8 *buf, u32 timeout);                 ///< ut接收一个byte，buf：字节存放地址；timeout：超时时间，单位ms；返回0：失败；返回1：成功
    u32(*read)(u8 *inbuf, u32 len, u32 timeout);        ///< ut接收一个字符串，inbuf：字符串存放首地址；len：预接收长度；timeout：超时时间，单位ms；返回实际接收的长度
    void (*write)(const u8 *outbuf, u32 len);           ///< ut发送一个字符串，outbuf：字符串首地址；len：发送的字符串长度；
    void (*set_baud)(u32 baud);                         ///< ut设置波特率，baud：波特率值
    u32 frame_length;
    u32 rx_timeout;
    KFIFO kfifo;                                        ///< ut用的循环buf结构体的指针
    UT_Semaphore  sem_rx;
    UT_Semaphore  sem_tx;
    u32(*get_data_len)(void);
} uart_bus_t;


const uart_bus_t *uart_dev_open(const struct uart_platform_data_t *arg);
u32 uart_dev_close(uart_bus_t *ut);

////////////////////////////////////////////////////////////////////////////////
#endif


