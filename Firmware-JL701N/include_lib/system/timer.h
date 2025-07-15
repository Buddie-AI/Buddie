#ifndef SYS_TIMER_H
#define SYS_TIMER_H


#include "typedef.h"
#include "generic/list.h"


struct static_sys_timer {
    void (*func)(void *priv);
    void *priv;
    u32 msec;
    u32 jiffies;
};

struct sys_usec_timer {
    void (*func)(void *priv);
    void *priv;
    const char *owner;
    struct sys_cpu_timer *timer;
};


#define SYS_HI_TIMER_ADD(_func, _priv, _msec) \
	static struct static_sys_timer hi_timer sec(.hi_timer) = { \
		.func = _func, \
		.priv = _priv, \
		.msec = _msec, \
	}

extern struct static_sys_timer static_hi_timer_begin[];
extern struct static_sys_timer static_hi_timer_end[];

#define list_for_each_static_hi_timer(p) \
	for (p=static_hi_timer_begin; p<static_hi_timer_end; p++)



struct sys_cpu_timer {
    u8 busy;
    void *priv;
    void (*set)(u32 usec);
    void (*unset)();
};

#define DEFINE_SYS_CPU_TIMER(t) \
    struct sys_cpu_timer t sec(.sys_cpu_timer);

#define REGISTER_SYS_CPU_TIMER(t) \
    struct sys_cpu_timer t sec(.sys_cpu_timer)


extern struct sys_cpu_timer sys_cpu_timer_begin[];
extern struct sys_cpu_timer sys_cpu_timer_end[];

#define list_for_each_cpu_usec_timer(p) \
    for (p = sys_cpu_timer_begin; p < sys_cpu_timer_end; p++)


/*
 *  System Timer
 */

/**
 * @brief 添加ms级系统非timeout类型定时任务。
 *
 * @param [in] priv 定时任务处理函数的输入参数
 * @param [in] func 定时任务处理函数
 * @param [in] msec 定时时间
 *
 * @return 定时器分配的ID
 *
 * @note
 * 1、系统会进入低功耗，节拍不会丢失，定时结束会唤醒系统；
 * 2、sys_timer由systimer线程提供时基，属于同步接口，
 *    也就是说在哪个线程add的sys_timer，当定时时基到了,
 *    systimer线程会发事件通知对应的add线程响应（回调函数被执行）；
 * 3、与sys_timer_del成对使用。
 */
u16 sys_timer_add(void *priv, void (*func)(void *priv), u32 msec);

/**
 * @brief 删除ms级非timeout类型定时任务。
 *
 * @param [in] id 要删除的任务ID
 *
 * @note
 * 1、与sys_timer_add成对使用。
 */
void sys_timer_del(u16);

/**
 * @brief 添加ms级系统timeout类型定时任务。
 *
 * @param [in] priv 定时任务处理函数的输入参数
 * @param [in] func 定时任务处理函数
 * @param [in] msec 定时时间
 *
 * @return 定时器分配的id号
 *
 * @note
 * 1、系统会进入低功耗，节拍不会丢失，定时结束会唤醒系统；
 * 2、sys_timeout由systimer线程提供时基，属于同步接口，
 *    也就是说在哪个线程add的sys_timeout，定时时基到了
 *    systimer线程会发事件通知对应的add线程响应（回调函数被执行）；
 * 3、timeout回调只会被执行一次；
 * 4、与sys_timerout_del成对使用。
 */
u16 sys_timeout_add(void *priv, void (*func)(void *priv), u32 msec);

/**
 * @brief 删除ms级timeout类型定时任务。
 *
 * @param [in] id 要删除的任务ID
 *
 * @note
 * 1、与sys_timerout_add成对使用。
 */
void sys_timeout_del(u16);

/**
 * @brief 重置ms级定时任务定时时间。
 *
 * @param [in] id 要修改的任务ID
 *
 * @note
 * 1、重置之后重新计时。
 */
void sys_timer_re_run(u16 id);

/**
 * @brief 修改ms级定时任务处理函数的输入参数。
 *
 * @param [in] id 要修改的任务ID
 * @param [in] priv 处理函数的输入参数
 */
void sys_timer_set_user_data(u16 id, void *priv);

/**
 * @brief 获取ms级定时任务处理函数的输入参数的值。
 *
 * @param [in] id 要获取处理函数的输入参数值的任务ID
 *
 * @return 返回add时的私有参数
 *
 * @note
 * 1、如果有通过sys_timer_set_user_data重新设置私有参数,
 *    则返回的是设置后的私有参数。
 */
void *sys_timer_get_user_data(u16 id);

/*-----------------------------------------------------------*/

/*
 *  System Usec Timer
 */
int sys_timer_modify(u16 id, u32 msec);

int sys_usec_timer_add(void *priv, void (*func)(void *priv), u32 usec);

void sys_usec_timer_schedule(struct sys_cpu_timer *);

void sys_usec_timer_set(int _t, u32 usec);

void sys_usec_timer_del(int);

void sys_timer_dump_time(void);

u32 sys_timer_get_ms(void);

/*-----------------------------------------------------------*/

/*
 *  Usr Timer
 */

void usr_timer_schedule();

/**
 * @brief usr_timer定时扫描增加接口
 *
 * @param [in] priv 私有参数
 * @param [in] func 定时扫描回调函数
 * @param [in] msec 定时时间， 单位：毫秒
 * @param [in] priority 优先级，范围：0/1
 *
 * @return 定时器分配的id号
 *
 * @note
 * 1、usr_timer的参数priority（优先级）为1，使用该类定时器，系统无法进入低功耗；
 * 2、usr_timer的参数priority（优先级）为0，使用该类定时器，系统低功耗会忽略该节拍，节拍不会丢失，但是定时周期会变；
 * 3、usr_timer属于异步接口， add的时候注册的扫描函数将在硬件定时器中时基到时候被调用;
 * 4、对应释放接口usr_timer_del。
*/
u16 usr_timer_add(void *priv, void (*func)(void *priv), u32 msec, u8 priority);

/**
 * @brief usr_timer超时增加接口
 *
 * @param [in] priv 私有参数
 * @param [in] func 超时回调函数
 * @param [in] msec 定时时间， 单位：毫秒
 * @param [in] priority 优先级,范围：0/1
 *
 * @return 定时器分配的id号
 *
 * @note
 * 1、usr_timerout的参数priority（优先级）为1，使用该类定时器，系统无法进入低功耗；
 * 2、usr_timerout的参数priority（优先级）为0，使用该类定时器，系统低功耗会忽略该节拍，节拍不会丢失，但是定时周期会变；
 * 3、usr_timerout属于异步接口， add的时候注册的扫描函数将在硬件定时器中时基到时候被调用；
 * 4、对应释放接口usr_timerout_del；
 * 5、timeout回调只会被执行一次。
*/
u16 usr_timeout_add(void *priv, void (*func)(void *priv), u32 msec, u8 priority);

/**
 * @brief usr_timer修改定时扫描时间接口
 *
 * @param [in] id usr_timer_add时分配的id号
 * @param [in] msec 定时时间，单位：毫秒
*/
int usr_timer_modify(u16 id, u32 msec);

/**
 * @brief usr_timerout修改超时时间接口
 *
 * @param [in] id usr_timerout_add时分配的id号
 * @param [in] msec 定时时间，单位：毫秒
*/
int usr_timeout_modify(u16 id, u32 msec);

/**
 * @brief usr_timer删除接口
 *
 * @param [in] id usr_timer_add时分配的id号
 *
 * @note
 * 1、注意与usr_timer_add成对使用。
*/
void usr_timer_del(u16 id);

/**
 * @brief usr_timeout删除接口
 *
 * @param [in] id usr_timerout_add时分配的id号
 *
 * @note
 * 1、注意与usr_timerout_add成对使用。
*/
void usr_timeout_del(u16 id);

/**
 * @brief usr_time输出调试信息
 *
 * @note
 * 1.调试时可用；
 * 2.将输出所有被add定时器的id及其时间(msec)。
*/
void usr_timer_dump(void);

/*-----------------------------------------------------------*/

/*
 *  For Compatible
 */
#define sys_hi_timer_schedule()\
    usr_timer_schedule()

#define sys_hi_timer_add(a, b, c)\
    usr_timer_add(a, b, c, 1)

#define sys_hi_timeout_add(a, b, c)\
    usr_timeout_add(a, b, c, 1)

#define sys_hi_timer_modify(a, b)\
    usr_timer_modify(a, b)

#define sys_hi_timeout_modify(a, b)\
    usr_timeout_modify(a, b)

#define sys_hi_timer_del(a)\
    usr_timer_del(a)

#define sys_hi_timeout_del(a)\
    usr_timeout_del(a)

#define sys_s_hi_timer_add(a, b, c)\
    usr_timer_add(a, b, c, 0)

#define sys_s_hi_timerout_add(a, b, c)\
    usr_timeout_add(a, b, c, 0)

#define sys_s_hi_timer_modify(a, b)\
    usr_timer_modify(a, b)

#define sys_s_hi_timeout_modify(a, b)\
    usr_timeout_modify(a, b)

#define sys_s_hi_timer_del(a)\
    usr_timer_del(a)

#define sys_s_hi_timeout_del(a)\
    usr_timeout_del(a)


#endif


