#include "asm/mcpwm.h"
#include "asm/clock.h"
#include "asm/gpio.h"

#define MCPWM_DEBUG_ENABLE  	1
#if MCPWM_DEBUG_ENABLE
#define mcpwm_debug(fmt, ...) printf("[MCPWM] "fmt, ##__VA_ARGS__)
#else
#define mcpwm_debug(...)
#endif

#define MCPWM_CLK   clk_get("mcpwm")


PWM_TIMER_REG *get_pwm_timer_reg(pwm_ch_num_type index)
{
    PWM_TIMER_REG *reg = NULL;
    switch (index) {
    case pwm_ch0:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR0_CON));
        break;
    case pwm_ch1:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR1_CON));
        break;
    case pwm_ch2:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR2_CON));
        break;
    case pwm_ch3:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR3_CON));
        break;
    default:
        break;
    }
    return reg;
}

PWM_CH_REG *get_pwm_ch_reg(pwm_ch_num_type index)
{
    PWM_CH_REG *reg = NULL;
    switch (index) {
    case pwm_ch0:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH0_CON0));
        break;
    case pwm_ch1:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH1_CON0));
        break;
    case pwm_ch2:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH2_CON0));
        break;
    case pwm_ch3:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH3_CON0));
        break;
    default:
        break;
    }
    return reg;
}

static u8 clk_in_io = IO_PORTC_02;
/*
 * @brief 更改MCPWM的频率
 * @parm frequency 频率
 */
void mcpwm_set_frequency(pwm_ch_num_type ch, pwm_aligned_mode_type align, u32 frequency)
{
    PWM_TIMER_REG *reg = get_pwm_timer_reg(ch);
    if (reg == NULL) {
        return;
    }

    reg->tmr_con = 0;
    reg->tmr_cnt = 0;
    reg->tmr_pr = 0;

    u32 i = 0;
    u32 mcpwm_div_clk = 0;
    u32 mcpwm_tmr_pr = 0;
    u32 mcpwm_fre_min = 0;
    u32 clk = MCPWM_CLK;
    for (i = 0; i < 16; i++) {
        mcpwm_fre_min = clk / (65536 * (1 << i));
        if ((frequency >= mcpwm_fre_min) || (i == 15)) {
            break;
        }
    }
    reg->tmr_con |= (i << 3); //div 2^i
    mcpwm_div_clk = clk / (1 << i);
    if (frequency == 0) {
        mcpwm_tmr_pr = 0;
    } else {
        if (align == pwm_center_aligned) { //中心对齐
            mcpwm_tmr_pr = mcpwm_div_clk / (frequency * 2) - 1;
        } else {
            mcpwm_tmr_pr = mcpwm_div_clk / frequency - 1;
        }
    }
    reg->tmr_pr = mcpwm_tmr_pr;
    //timer mode
    if (align == pwm_center_aligned) { //中心对齐
        reg->tmr_con |= 0b10;
    } else {
        reg->tmr_con |= 0b01;
    }
}

static u32 old_mcpwm_clk = 0;
static void clock_critical_enter(void)
{
    old_mcpwm_clk = clk_get("mcpwm");
}
static void clock_critical_exit(void)
{
    u32 new_mcpwm_clk = clk_get("mcpwm");
    if (new_mcpwm_clk == old_mcpwm_clk) {
        return;
    }
    PWM_CH_REG *pwm_reg = NULL;
    PWM_TIMER_REG *timer_reg = NULL;
    for (u8 ch = 0; ch < 4; ch++) {
        if (JL_MCPWM->MCPWM_CON0 & BIT(ch + 8)) {
            pwm_reg = get_pwm_ch_reg(ch);
            timer_reg = get_pwm_timer_reg(ch);
            if (new_mcpwm_clk > old_mcpwm_clk) {
                timer_reg->tmr_pr = timer_reg->tmr_pr * (new_mcpwm_clk / old_mcpwm_clk);
                pwm_reg->ch_cmpl = pwm_reg->ch_cmpl * (new_mcpwm_clk / old_mcpwm_clk);
            } else {
                timer_reg->tmr_pr = timer_reg->tmr_pr / (old_mcpwm_clk / new_mcpwm_clk);
                pwm_reg->ch_cmpl = pwm_reg->ch_cmpl / (old_mcpwm_clk / new_mcpwm_clk);
            }
            pwm_reg->ch_cmph = pwm_reg->ch_cmpl;
        }
    }
}
CLOCK_CRITICAL_HANDLE_REG(mcpwm, clock_critical_enter, clock_critical_exit)

/*
 * @brief 设置一个通道的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm duty 占空比：0 ~ 10000 对应 0% ~ 100%
 */
void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);

    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmpl = timer_reg->tmr_pr * duty / 10000;
        pwm_reg->ch_cmph = pwm_reg->ch_cmpl;
        timer_reg->tmr_cnt = 0;
        timer_reg->tmr_con |= 0b01;
        if (duty == 10000) {
            timer_reg->tmr_cnt = 0;
            timer_reg->tmr_con &= ~(0b11);
        } else if (duty == 0) {
            timer_reg->tmr_cnt = pwm_reg->ch_cmpl;
            timer_reg->tmr_con &= ~(0b11);
        }
    }
}

/*
 * @brief 打开或者关闭一个时基
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mctimer_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable)
{
    if (pwm_ch > pwm_ch_max) {
        return;
    }
    if (enable) {
        JL_MCPWM->MCPWM_CON0 |= BIT(pwm_ch + 8); //TnEN
    } else {
        JL_MCPWM->MCPWM_CON0 &= (~BIT(pwm_ch + 8)); //TnDIS
    }
}


/*
 * @brief 打开或者关闭一个通道
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mcpwm_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable)
{
    if (pwm_ch >= pwm_ch_max) {
        return;
    }
    if (enable) {
        JL_MCPWM->MCPWM_CON0 |= BIT(pwm_ch); //PWMnEN
    } else {
        JL_MCPWM->MCPWM_CON0 &= (~BIT(pwm_ch)); //PWMnDIS
    }
}

/*
 * @brief 关闭MCPWM模块
 */
void mcpwm_open(pwm_ch_num_type pwm_ch)
{
    if (pwm_ch >= pwm_ch_max) {
        return;
    }
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    pwm_reg->ch_con1 &= ~(0b111 << 8);
    pwm_reg->ch_con1 |= (pwm_ch << 8); //sel mctmr
    mcpwm_ch_open_or_close(pwm_ch, 1);
    mctimer_ch_open_or_close(pwm_ch, 1);
}


/*
 * @brief 关闭MCPWM模块
 */
void mcpwm_close(pwm_ch_num_type pwm_ch)
{
    mctimer_ch_open_or_close(pwm_ch, 0);
    mcpwm_ch_open_or_close(pwm_ch, 0);
}


void log_pwm_info(pwm_ch_num_type pwm_ch)
{
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    mcpwm_debug("tmr%d con0 = 0x%x", pwm_ch, timer_reg->tmr_con);
    mcpwm_debug("tmr%d pr = 0x%x", pwm_ch, timer_reg->tmr_pr);
    mcpwm_debug("pwm ch%d_con0 = 0x%x", pwm_ch, pwm_reg->ch_con0);
    mcpwm_debug("pwm ch%d_con1 = 0x%x", pwm_ch, pwm_reg->ch_con1);
    mcpwm_debug("pwm ch%d_cmph = 0x%x, pwm ch%d_cmpl = 0x%x", pwm_ch, pwm_reg->ch_cmph, pwm_ch, pwm_reg->ch_cmpl);
    mcpwm_debug("MCPWM_CON0 = 0x%x", JL_MCPWM->MCPWM_CON0);
    mcpwm_debug("mcpwm clk = %d", MCPWM_CLK);
}

void mcpwm_init(struct pwm_platform_data *arg)
{
    //set output IO
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(arg->pwm_ch_num);
    if (pwm_reg == NULL) {
        return;
    }

    //set mctimer frequency
    mcpwm_set_frequency(arg->pwm_ch_num, arg->pwm_aligned_mode, arg->frequency);

    pwm_reg->ch_con0 = 0;

    if (arg->complementary_en) {            //是否互补
        pwm_reg->ch_con0 &= ~(BIT(5) | BIT(4));
        pwm_reg->ch_con0 |= BIT(5); //L_INV
    } else {
        pwm_reg->ch_con0 &= ~(BIT(5) | BIT(4));
    }

    mcpwm_open(arg->pwm_ch_num); 	 //mcpwm enable
    //set duty
    mcpwm_set_duty(arg->pwm_ch_num, arg->duty);

    //H:
    if (arg->h_pin < IO_MAX_NUM) {      //任意引脚
        pwm_reg->ch_con0 |= BIT(2);     //H_EN
        gpio_och_sel_output_signal(arg->h_pin, OUTPUT_CH_SIGNAL_MC_PWM0_H + 2 * arg->pwm_ch_num);
        gpio_set_direction(arg->h_pin, 0); //DIR output
    }
    //L:
    if (arg->l_pin < IO_MAX_NUM) {      //任意引脚
        pwm_reg->ch_con0 |= BIT(3);     //L_EN
        gpio_och_sel_output_signal(arg->l_pin, OUTPUT_CH_SIGNAL_MC_PWM0_L + 2 * arg->pwm_ch_num);
        gpio_set_direction(arg->l_pin, 0); //DIR output
    }

    log_pwm_info(arg->pwm_ch_num);
}


///////////// for test code //////////////////
void mcpwm_test(void)
{
#define PWM_CH0_ENABLE 		1
#define PWM_CH1_ENABLE 		1
#define PWM_CH2_ENABLE 		1
#define PWM_CH3_ENABLE 		1

    struct pwm_platform_data pwm_p_data;

#if PWM_CH0_ENABLE
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch0;                        //通道号
    pwm_p_data.frequency = 1000;                            //1KHz
    pwm_p_data.duty = 5000;                                 //占空比50%
    pwm_p_data.h_pin = IO_PORTA_06;                         //任意引脚
    pwm_p_data.l_pin = -1;                                  //任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
#endif
#if PWM_CH1_ENABLE
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch1;                        //通道号
    pwm_p_data.frequency = 1000;                            //1KHz
    pwm_p_data.duty = 2500;                                 //占空比25%
    pwm_p_data.h_pin = IO_PORTA_08;                         //任意引脚
    pwm_p_data.l_pin = IO_PORTA_09;                         //任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 1;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
#endif
#if PWM_CH2_ENABLE
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch2;                        //通道号
    pwm_p_data.frequency = 10000;                           //10KHz
    pwm_p_data.duty = 5000;                                 //占空比50%
    pwm_p_data.h_pin = IO_PORTA_03;                         //任意引脚
    pwm_p_data.l_pin = -1;                                  //任意引脚,不需要就填-1
    mcpwm_init(&pwm_p_data);
#endif
#if PWM_CH3_ENABLE
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch3;                        //通道号
    pwm_p_data.frequency = 10000;                           //10KHz
    pwm_p_data.duty = 7500;                                 //占空比75%
    pwm_p_data.h_pin = IO_PORTA_04;                         //任意引脚
    pwm_p_data.l_pin = IO_PORTA_05;                         //任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
#endif

    extern void clk_out(u8 gpio, enum CLK_OUT_SOURCE clk);
    clk_out(IO_PORTA_07, LSB_CLK_OUT);
    extern void wdt_clear();
    while (1) {
        wdt_clear();
    }
}



/*******************************  外部引脚中断参考代码  ***************************/

void (*io_isr_cbfun)(u8 index) = NULL;
void set_io_ext_interrupt_cbfun(void (*cbfun)(u8 index))
{
    io_isr_cbfun = cbfun;
}
___interrupt
void io_interrupt(void)
{
    u32 io_index = -1;
    if (JL_MCPWM->CH0_CON1 & BIT(15)) {
        JL_MCPWM->CH0_CON1 |= BIT(14);
        io_index = 0;
    } else if (JL_MCPWM->CH1_CON1 & BIT(15)) {
        JL_MCPWM->CH1_CON1 |= BIT(14);
        io_index = 1;
    } else if (JL_MCPWM->CH2_CON1 & BIT(15)) {
        JL_MCPWM->CH2_CON1 |= BIT(14);
        io_index = 2;
    } else if (JL_MCPWM->CH3_CON1 & BIT(15)) {
        JL_MCPWM->CH3_CON1 |= BIT(14);
        io_index = 3;
    } else {
        return;
    }
    if (io_isr_cbfun) {
        io_isr_cbfun(io_index);
    }
}
void io_ext_interrupt_init(u8 index, u8 port, u8 trigger_mode)
{
    if (port > IO_PORT_MAX) {
        return;
    }
    gpio_set_die(port, 1);
    gpio_set_direction(port, 1);
    if (trigger_mode) {
        gpio_set_pull_up(port, 1);
        gpio_set_pull_down(port, 0);
        JL_MCPWM->FPIN_CON &= ~BIT(16 + index);//下降沿触发
    } else {
        gpio_set_pull_up(port, 0);
        gpio_set_pull_down(port, 1);
        JL_MCPWM->FPIN_CON |=  BIT(16 + index);//上升沿触发
    }
    JL_MCPWM->FPIN_CON |=  BIT(8 + index);//开启滤波
    JL_MCPWM->FPIN_CON |= (0b111111 << 0); //滤波时间 = 16 * 64 / hsb_clk (单位：s)

    gpio_ich_sel_iutput_signal(port, INPUT_CH_SIGNAL_MC_FPIN0 + index, INPUT_CH_TYPE_GP_ICH);
    request_irq(IRQ_MCPWM_CHX_IDX, 3, io_interrupt, 0);   //注册中断函数
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(index);
    pwm_reg->ch_con1 = BIT(14) | BIT(11) | BIT(4) | (index << 0);
    JL_MCPWM->MCPWM_CON0 |= BIT(index);

    printf("JL_MCPWM->CH%d_CON1  = 0x%x\n", index, pwm_reg->ch_con1);
    printf("JL_MCPWM->FPIN_CON   = 0x%x\n", JL_MCPWM->FPIN_CON);
    printf("JL_MCPWM->MCPWM_CON0 = 0x%x\n", JL_MCPWM->MCPWM_CON0);
    printf("JL_IOMAP->ICH_CON0   = 0x%x\n", JL_IOMAP->ICH_CON0);
    printf("JL_IOMAP->ICH_CON1   = 0x%x\n", JL_IOMAP->ICH_CON1);
    printf("JL_IOMAP->ICH_CON2   = 0x%x\n", JL_IOMAP->ICH_CON2);
    printf("JL_IOMAP->ICH_CON3   = 0x%x\n", JL_IOMAP->ICH_CON3);
    printf("JL_IMAP->FI_GP_ICH0  = %d\n", JL_IMAP->FI_GP_ICH0);
    printf("JL_IMAP->FI_GP_ICH1  = %d\n", JL_IMAP->FI_GP_ICH1);
    printf("JL_IMAP->FI_GP_ICH2  = %d\n", JL_IMAP->FI_GP_ICH2);
    printf("JL_IMAP->FI_GP_ICH3  = %d\n", JL_IMAP->FI_GP_ICH3);
    printf("JL_IMAP->FI_GP_ICH4  = %d\n", JL_IMAP->FI_GP_ICH4);
    printf("JL_IMAP->FI_GP_ICH5  = %d\n", JL_IMAP->FI_GP_ICH5);
    printf("JL_IMAP->FI_GP_ICH6  = %d\n", JL_IMAP->FI_GP_ICH6);
    printf("JL_IMAP->FI_GP_ICH7  = %d\n", JL_IMAP->FI_GP_ICH7);
    printf("JL_IMAP->FI_GP_ICH8  = %d\n", JL_IMAP->FI_GP_ICH8);
    printf("JL_IMAP->FI_GP_ICH9  = %d\n", JL_IMAP->FI_GP_ICH9);
}
void io_ext_interrupt_close(u8 index, u8 port)
{
    if (port > IO_PORT_MAX) {
        return;
    }
    gpio_set_die(port, 0);
    gpio_set_direction(port, 1);
    gpio_set_pull_up(port, 0);
    gpio_set_pull_down(port, 0);
    gpio_ich_disable_iutput_signal(port, INPUT_CH_SIGNAL_MC_FPIN0 + index, INPUT_CH_TYPE_GP_ICH);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(index);
    pwm_reg->ch_con1 = BIT(14);
}

///////////// 使用举例如下 //////////////////
void my_io_isr_cbfun(u8 index)
{
    printf("io index --> %d  Hello world !\n", index);
}
void io_ext_interrupt_test(void)
{
    set_io_ext_interrupt_cbfun(my_io_isr_cbfun);
    io_ext_interrupt_init(0, IO_PORTA_04, 1);
    io_ext_interrupt_init(1, IO_PORTB_01, 1);
    io_ext_interrupt_init(2, IO_PORTC_02, 1);
    io_ext_interrupt_init(3, IO_PORTG_01, 1);
    extern void wdt_clear();
    while (1) {
        wdt_clear();
    }
}


/*******************************  用timer做pwm的参考代码  ***************************/

/**
 * @param JL_TIMERx : JL_TIMER0/1/2/3
 * @param pwm_io : JL_PORTA_01, JL_PORTB_02,,,等等，支持任意普通IO
 * @param fre : 频率，单位Hz
 * @param duty : 初始占空比，0~10000对应0~100%
 */
void timer_pwm_init(JL_TIMER_TypeDef *JL_TIMERx, u32 pwm_io, u32 fre, u32 duty)
{
    switch ((u32)JL_TIMERx) {
    case (u32)JL_TIMER0 :
        gpio_och_sel_output_signal(pwm_io, OUTPUT_CH_SIGNAL_TIMER0_PWM);
        break;
    case (u32)JL_TIMER1 :
        gpio_och_sel_output_signal(pwm_io, OUTPUT_CH_SIGNAL_TIMER1_PWM);
        break;
    case (u32)JL_TIMER2 :
        gpio_och_sel_output_signal(pwm_io, OUTPUT_CH_SIGNAL_TIMER2_PWM);
        break;
    case (u32)JL_TIMER3 :
        bit_clr_ie(IRQ_TIME3_IDX);
        gpio_och_sel_output_signal(pwm_io, OUTPUT_CH_SIGNAL_TIMER3_PWM);
        break;
    case (u32)JL_TIMER4 :
        gpio_och_sel_output_signal(pwm_io, OUTPUT_CH_SIGNAL_TIMER4_PWM);
        break;
    case (u32)JL_TIMER5 :
        gpio_och_sel_output_signal(pwm_io, OUTPUT_CH_SIGNAL_TIMER5_PWM);
        break;
    default:
        return;
    }
    u32 u_clk = 24000000;
    //初始化timer
    JL_TIMERx->CON = 0;
    JL_TIMERx->CON |= (0b110 << 10);					//时钟源选择STD_24M时钟源
    JL_TIMERx->CON |= (0b0001 << 4);					//时钟源再4分频
    JL_TIMERx->CNT = 0;									//清计数值
    JL_TIMERx->PRD = u_clk / (4 * fre);				    //设置周期
    JL_TIMERx->PWM = (JL_TIMERx->PRD * duty) / 10000;	//设置初始占空比，0~10000对应0~100%
    JL_TIMERx->CON |= (0b01 << 0);						//计数模式
    JL_TIMERx->CON |= BIT(8);							//PWM使能
    //设置引脚状态
    gpio_set_die(pwm_io, 1);
    gpio_set_pull_up(pwm_io, 0);
    gpio_set_pull_down(pwm_io, 0);
    gpio_set_direction(pwm_io, 0);

    printf("JL_TIMERx->PRD = 0x%x\n", JL_TIMERx->PRD);
    printf("JL_TIMERx->CON = 0x%x\n", JL_TIMERx->CON);
}

void set_timer_pwm_duty(JL_TIMER_TypeDef *JL_TIMERx, u32 duty)
{
    JL_TIMERx->PWM = (JL_TIMERx->PRD * duty) / 10000;	//0~10000对应0~100%
}

void timer_pwm_test(void)
{
    timer_pwm_init(JL_TIMER2, IO_PORTC_03, 10000, 7000);
    timer_pwm_init(JL_TIMER3, IO_PORTG_04, 1000, 2000);
    extern void wdt_clear();
    while (1) {
        wdt_clear();
    }
}



