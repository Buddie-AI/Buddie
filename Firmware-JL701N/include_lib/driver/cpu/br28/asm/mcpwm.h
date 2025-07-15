#ifndef _MCPWM_H_
#define _MCPWM_H_


#include "typedef.h"


/* 对齐方式选择 */
typedef enum {
    pwm_edge_aligned,                                   ///< 边沿对齐模式
    pwm_center_aligned,                                 ///< 中心对齐模式
} pwm_aligned_mode_type;

/* pwm通道选择 */
typedef enum {
    pwm_ch0,
    pwm_ch1,
    pwm_ch2,
    pwm_ch3,
    pwm_ch_max,
} pwm_ch_num_type;

/* MCPWM TIMER寄存器 */
typedef struct _pwm_timer_reg {
    volatile u32 tmr_con;
    volatile u32 tmr_cnt;
    volatile u32 tmr_pr;
} PWM_TIMER_REG;

/* MCPWM通道寄存器 */
typedef struct _pwm_ch_reg {
    volatile u32 ch_con0;
    volatile u32 ch_con1;
    volatile u32 ch_cmph;
    volatile u32 ch_cmpl;
} PWM_CH_REG;

/* 初始化要用的参数结构体 */
struct pwm_platform_data {
    pwm_aligned_mode_type pwm_aligned_mode;             ///< PWM对齐方式选择
    pwm_ch_num_type pwm_ch_num;                         ///< 选择pwm通道号
    u32 frequency;                               		///< 初始共同频率，CH0, CH, CH2,,,,,,
    u16 duty;                                           ///< 初始占空比，0~10000 对应 0%~100% 。每个通道可以有不同的占空比。互补模式的占空比体现在高引脚的波形上。
    u8 h_pin;                                           ///< 一个通道的H引脚。
    u8 l_pin;                                           ///< 一个通道的L引脚，不需要则填-1
    u8 complementary_en;                                ///< 该通道的两个引脚输出的波形。0: 同步， 1: 互补，互补波形的占空比体现在H引脚上
};



void mcpwm_set_frequency(pwm_ch_num_type ch, pwm_aligned_mode_type align, u32 frequency);
void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty);
void mctimer_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable);
void mcpwm_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable);
void mcpwm_open(pwm_ch_num_type pwm_ch);
void mcpwm_close(pwm_ch_num_type pwm_ch);
void mcpwm_init(struct pwm_platform_data *arg);
void mcpwm_test(void);


void set_io_ext_interrupt_cbfun(void (*cbfun)(u8 index));
void io_ext_interrupt_init(u8 index, u8 port, u8 trigger_mode);
void io_ext_interrupt_close(u8 index, u8 port);
void io_ext_interrupt_test(void);


void timer_pwm_init(JL_TIMER_TypeDef *JL_TIMERx, u32 pwm_io, u32 fre, u32 duty);
void set_timer_pwm_duty(JL_TIMER_TypeDef *JL_TIMERx, u32 duty);
void timer_pwm_test(void);


#endif


