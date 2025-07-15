#ifndef _PWM_LED_H_
#define _PWM_LED_H_

/*******************************************************************
*   本文件为LED灯配置的接口头文件
*
*	约定:
*		1)两盏灯为单IO双LED接法;
*		2)LED0: RLED, 蓝色, 低电平亮灯;
*		3)LED1: BLED, 红色, 高电平亮灯;
*********************************************************************/

// LED实现的效果有:
// 1.两盏LED全亮;
// 2.LED单独亮灭;
// 3.LED单独慢闪和快闪;
// 4.LED 5s内单独闪一次和两次;
// 5.LED交替快闪和慢闪;
// 6.LED单独呼吸;
// 7.LED交替呼吸

/*
 * 	LED各个效果可供配置以下参数, 请按照参数后面的注释说明的范围进行配置
 */

//#define PWM_LED_TWO_IO_SUPPORT					//定义该宏会支持两个IO推灯模式, 默认关闭

#define CFG_LED0_LIGHT						200  	//10 ~ 500, 值越大, (红灯)亮度越高
#define CFG_LED1_LIGHT						200  	//10 ~ 500, 值越大, (蓝灯)亮度越高

#define CFG_SINGLE_FAST_FLASH_FREQ			500		//LED单独快闪速度, ms闪烁一次(100 ~ 1000)
#define CFG_SINGLE_FAST_LIGHT_TIME 			100  	//单灯快闪灯亮持续时间, 单位ms

#define CFG_SINGLE_SLOW_FLASH_FREQ			2000	//LED单独慢闪速度, ms闪烁一次(1000 ~ 20000)
#define CFG_SINGLE_SLOW_LIGHT_TIME 			100  	//单灯慢闪灯亮持续时间, 单位ms

#define CFG_DOUBLE_FAST_FLASH_FREQ			500		//LED交替快闪速度, ms闪烁一次(100 ~ 1000)
#define CFG_DOUBLE_SLOW_FLASH_FREQ			2000	//LED交替慢闪速度, ms闪烁一次(1000 ~ 20000)

/***************** LED0/LED1单独每隔5S单闪时, 可供调节参数 ********************/
#define CFG_LED_5S_FLASH_LIGHT_TIME			100		//LED 5S 闪烁时灯亮持续时间, 单位ms

/***************** 呼吸模式配置参数, 可供调节参数 ********************/
#define CFG_LED_BREATH_TIME 				1000		//呼吸时间灭->亮->灭, 单位ms
#define CFG_LED0_BREATH_BRIGHT 				300			//呼吸亮度, 范围: 0 ~ 500
#define CFG_LED1_BREATH_BRIGHT 				300			//呼吸亮度, 范围: 0 ~ 500
#define CFG_LED_BREATH_BLINK_TIME 			1000		//灭灯延时, 单位ms


enum pwm_led_clk_source {
    PWM_LED_CLK_RESERVED0,  //PWM_LED_CLK_OSC32K, no use
    PWM_LED_CLK_RC32K, 		//use
    PWM_LED_CLK_RESERVED1,  //PWM_LED_CLK_BTOSC_12M, no use
    PWM_LED_CLK_RESERVED2, 	//PWM_LED_CLK_RCLK_250K, no use
    PWM_LED_CLK_BTOSC_24M,  //use
};

enum pwm_led_mode {
    PWM_LED_MODE_START,

    PWM_LED_ALL_OFF,         		//mode1: 全灭
    PWM_LED_ALL_ON,              	//mode2: 全亮

    PWM_LED0_ON,             		//mode3: 蓝亮
    PWM_LED0_OFF,            		//mode4: 蓝灭
    PWM_LED0_SLOW_FLASH,           	//mode5: 蓝慢闪
    PWM_LED0_FAST_FLASH,           	//mode6: 蓝快闪
    PWM_LED0_DOUBLE_FLASH_5S,  		//mode7: 蓝灯5秒连闪两下
    PWM_LED0_ONE_FLASH_5S,    		//mode8: 蓝灯5秒连闪1下

    PWM_LED1_ON,             		//mode9:  红亮
    PWM_LED1_OFF,            		//mode10: 红灭
    PWM_LED1_SLOW_FLASH,           	//mode11: 红慢闪
    PWM_LED1_FAST_FLASH,            //mode12: 红快闪
    PWM_LED1_DOUBLE_FLASH_5S,  		//mode13: 红灯5秒连闪两下
    PWM_LED1_ONE_FLASH_5S,    		//mode14: 红灯5秒闪1下

    PWM_LED0_LED1_FAST_FLASH,   	//mode15: 红蓝交替闪（快闪）
    PWM_LED0_LED1_SLOW_FLASH, 		//mode16: 红蓝交替闪（慢闪）

    PWM_LED0_BREATHE,				//mode17: 蓝灯呼吸灯模式
    PWM_LED1_BREATHE,				//mode18: 红灯呼吸灯模式
    PWM_LED0_LED1_BREATHE,			//mode19: 红蓝交替呼吸灯模式

    PWM_LED_MODE_END,

    PWM_LED1_FLASH_THREE,           //自定义状态，不能通过pmd_led_mode去设置
    PWM_LED0_FLASH_THREE,           //自定义状态，不能通过pmd_led_mode去设置

    PWM_LED_USER_DEFINE_BEGIN = 0x50,
    PWM_LED_USER_DEFINE_MODE0,  	//用户自定义模式0:
    PWM_LED_USER_DEFINE_END,

    PWM_LED_NULL = 0xFF,
};

struct pwm_led_two_io_mode {
    u8 two_io_mode_enable;
    u8 led0_pin;
    u8 led1_pin;
};
enum led_io_mode {
    LED_ONE_IO_MODE,
    LED_TWO_IO_MODE,
};

struct one_io_cfg {
    u8 pin;
};

struct two_io_cfg {
    u8 pin0;
    u8 pin1;
};

union io_mode_cfg {
    struct one_io_cfg one_io;
    struct two_io_cfg two_io;
};

struct led_platform_data {
    enum led_io_mode io_mode;
    union io_mode_cfg io_cfg;
};

#define LED_PLATFORM_DATA_BEGIN(data) \
		const struct led_platform_data data = {

#define LED_PLATFORM_DATA_END() \
};

/*********************** LED 初始化 ******************************/
void pwm_led_init(const struct led_platform_data *user_data);

/********************** LED 闪烁模式切换 ************************/
void pwm_led_mode_set(u8 fre_mode);

/*****************************************************************
	LED时钟源切换, support:
		PWM_LED_CLK_RC32K
		PWM_LED_CLK_BTOSC_24M
*********************************************************************/
void pwm_led_clk_set(enum pwm_led_clk_source src);

/***************** 闪烁状态复位, 重新开始一个周期 ******************/
void pwm_led_display_mode_reset(void);

/***************** 获取led当前的闪烁模式 ***************************/
enum pwm_led_mode pwm_led_display_mode_get(void);

/********************************************************************
	修改LED灯IO口驱动能力, 挡位: 0 ~ 3
		0: 2.4mA(8mA mos + 120Ωres)
		1: 8mA(8mA mos)
		2: 18.4mA(24mA mos + 120Ωres)
		3: 24mA(24mA mos)
*********************************************************************/
void pwm_led_io_max_drive_set(u8 strength);

/******************* PWM 模块开关 *********************/
void pwm_led_set_on(void);
void pwm_led_set_off(void);
void led_module_on();
void led_module_off();


/******************* PWM 模块是否开启 *********************/
u8 is_pwm_led_on(void);
bool is_led_module_on();

//=================================================================================//
//@brief: 自定义设置单灯闪状态
//@input: void
//		led_index: 0: led0, 1:led1, 2:led0 & led1(互闪)
//		led0_bright, LED0亮度: 0 ~ 500
// 		led1_bright, LED1亮度: 0 ~ 500
// 		led1_bright: led1亮度,
// 		period: 闪灯周期(ms), 多少ms闪一下,
// 		start_light_time: 在周期中开始亮灯的时间, -1: 周期最后亮灯
// 		light_time: 灯亮持续时间,
//@return: void
//@note:
//=================================================================================//
void pwm_led_one_flash_display(u8 led_index, u16 led0_bright, u16 led1_bright,
                               u32 period, u32 start_light_time, u32 light_time);

//=================================================================================//
//@brief: 自定义设置单灯双闪状态
//@input:
// 		led_index: 0: led0, 1:led1, 3:led0 & led1(互闪)
//		led0_bright, LED0亮度: 0 ~ 500
// 		led1_bright, LED1亮度: 0 ~ 500
// 		period: 闪灯周期(ms), 多少ms闪一下
//  	first_light_time: 第一次亮灯持续时间,
// 		second_light_time: 第二次亮灯持续时间,
// 		gap_time: 两次亮灯时间间隔,
//@return: void
//@note:
//=================================================================================//
void pwm_led_double_flash_display(u8 led_index, u16 led0_bright, u16 led1_bright,
                                  u32 period, u32 first_light_time, u32 gap_time, u32 second_light_time);


//=================================================================================//
//@brief: 自定义设置呼吸模式
//@input:
//		led_index: 0: led0, 1:led1, 2:led0 & led1(交互呼吸)
//		breathe_time: 呼吸周期(灭->最亮->灭), 设置范围: 500ms以上;
// 		led0_bright: led0呼吸到最亮的亮度(0 ~ 500);
// 		led1_bright: led1呼吸到最亮的亮度(0 ~ 500);
// 		led0_light_delay_time: led0最高亮度延时(0 ~ 100ms);
// 		led1_light_delay_time: led1最高亮度延时(0 ~ 100ms);
// 		led_blink_delay_time: led0和led1灭灯延时(0 ~ 20000ms), 0 ~ 20S;
//@return: void
//@note:
//=================================================================================//
void pwm_led_breathe_display(u8 led_index, u16 breathe_time, u16 led0_bright, u16 led1_bright,
                             u32 led0_light_delay_time, u32 led1_light_delay_time, u32 led_blink_delay_time);

//=================================================================================//
//@brief: 注册LED周期中断函数, 每个LED周期结束后会调用一次, 可以统计指定状态闪烁多少次
//@input:
//@return: void
//@note:
//=================================================================================//
void pwm_led_register_irq(void (*func)(void));

void led_module_enter_sniff_mode();
void led_module_exit_sniff_mode();

#endif //_PWM_LED_H_


