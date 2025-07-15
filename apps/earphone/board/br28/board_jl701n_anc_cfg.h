#ifndef CONFIG_BOARD_JL701N_ANC_CFG_H
#define CONFIG_BOARD_JL701N_ANC_CFG_H

#include "board_jl701n_anc_global_build_cfg.h"

#ifdef CONFIG_BOARD_JL701N_ANC

#define CONFIG_SDFILE_ENABLE

#define DEVE_BOARD					    0x00	//开发板
#define CUSTOM1_CFG					    0x01
#define CUSTOM2_CFG					    0x02	//高压
#define CUSTOM3_CFG					    0x03
#define CUSTOM4_CFG                     0x04
#define CUSTOM5_CFG						0x05	//高压
#define CUSTOM6_CFG						0x06
#define CUSTOM7_CFG						0x07
#define CUSTOM8_CFG						0x08
#define CUSTOM9_CFG						0x09
#define CUSTOM10_CFG					0x0a
#define CUSTOM11_CFG					0x0b

#define CLIENT_BOARD 				    DEVE_BOARD

//*********************************************************************************//
//                                 配置开始                                        //
//*********************************************************************************//
#define ENABLE_THIS_MOUDLE					1
#define DISABLE_THIS_MOUDLE					0

#define ENABLE								1
#define DISABLE								0

#define NO_CONFIG_PORT						(-1)

//*********************************************************************************//
//                            外置触摸供电配置                                     //
//*********************************************************************************//
#if CLIENT_BOARD == CUSTOM2_CFG || \
    CLIENT_BOARD == CUSTOM8_CFG
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    IO_PORTC_03    //外置触摸供电口

#elif CLIENT_BOARD == CUSTOM3_CFG
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    IO_PORTC_05    //外置触摸供电口

#elif CLIENT_BOARD == CUSTOM4_CFG
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    IO_PORTG_05    //外置触摸供电口

#elif CLIENT_BOARD == CUSTOM5_CFG
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    IO_PORTB_04    //外置触摸供电口

#elif CLIENT_BOARD == CUSTOM11_CFG
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    IO_PORTB_04    //外置触摸供电口

#elif CLIENT_BOARD == CUSTOM6_CFG
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    IO_PORTG_06    //外置触摸供电口

#elif CLIENT_BOARD == CUSTOM7_CFG
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    IO_PORTC_03    //外置触摸供电口

#elif CLIENT_BOARD == CUSTOM9_CFG
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    IO_PORTC_05    //ADC模拟开关供电口

#else
#define TCFG_EXTERNAL_TOUCH_KEY_POWER_PORT		    NO_CONFIG_PORT //外置触摸供电口
#endif

//*********************************************************************************//
//                                 UART配置                                        //
//*********************************************************************************//
#define TCFG_UART0_ENABLE					ENABLE_THIS_MOUDLE                     //串口打印模块使能
#define TCFG_UART0_RX_PORT					NO_CONFIG_PORT                         //串口接收脚配置（用于打印可以选择NO_CONFIG_PORT）
//#define TCFG_UART0_TX_PORT  				IO_PORTA_05                            //串口发送脚配置
#if CLIENT_BOARD == CUSTOM1_CFG || \
    CLIENT_BOARD == CUSTOM2_CFG || \
    CLIENT_BOARD == CUSTOM4_CFG || \
    CLIENT_BOARD == CUSTOM5_CFG || \
    CLIENT_BOARD == CUSTOM11_CFG || \
	CLIENT_BOARD == CUSTOM3_CFG ||\
	CLIENT_BOARD == CUSTOM7_CFG || \
	CLIENT_BOARD == CUSTOM8_CFG || \
	CLIENT_BOARD == CUSTOM6_CFG || \
	CLIENT_BOARD == CUSTOM9_CFG || \
	CLIENT_BOARD == CUSTOM10_CFG


#define TCFG_UART0_TX_PORT  				IO_PORT_DP                            //串口发送脚配置
#else
#define TCFG_UART0_TX_PORT  				IO_PORTB_02                            //串口发送脚配置
#endif
#define TCFG_UART0_BAUDRATE  				1000000                                //串口波特率配置

//*********************************************************************************//
//                                 IIC配置                                        //
//*********************************************************************************//
/*软件IIC设置*/
#define TCFG_SW_I2C0_CLK_PORT               IO_PORTA_09                             //软件IIC  CLK脚选择
#define TCFG_SW_I2C0_DAT_PORT               IO_PORTA_10                             //软件IIC  DAT脚选择
#define TCFG_SW_I2C0_DELAY_CNT              50                                      //IIC延时参数，影响通讯时钟频率

/*硬件IIC端口选择
  SCL         SDA
  'A': IO_PORT_DP   IO_PORT_DM
  'B': IO_PORTA_09  IO_PORTA_10
  'C': IO_PORTA_07  IO_PORTA_08
  'D': IO_PORTA_05  IO_PORTA_06
 */
#define TCFG_HW_I2C0_PORTS                  'B'
#define TCFG_HW_I2C0_CLK                    100000                                  //硬件IIC波特率

//*********************************************************************************//
//                                  SD 配置                                        //
//*********************************************************************************//
#define TCFG_SD0_ENABLE						DISABLE_THIS_MOUDLE                    //SD0模块使能
#define TCFG_SD0_DAT_MODE					1               //线数设置，1：一线模式  4：四线模式
#define TCFG_SD0_DET_MODE					SD_IO_DECT     //SD卡检测方式
#define TCFG_SD0_DET_IO                     IO_PORTB_03      //当检测方式为IO检测可用
#define TCFG_SD0_DET_IO_LEVEL               0               //当检测方式为IO检测可用,0：低电平检测到卡。 1：高电平(外部电源)检测到卡。 2：高电平(SD卡电源)检测到卡。
#define TCFG_SD0_CLK						(3000000 * 4L)  //SD卡时钟频率设置
#define TCFG_SD0_PORT_CMD					IO_PORTB_00
#define TCFG_SD0_PORT_CLK					IO_PORTB_01
#define TCFG_SD0_PORT_DA0					IO_PORTB_02
#define TCFG_SD0_PORT_DA1					NO_CONFIG_PORT  //当选择4线模式时要用
#define TCFG_SD0_PORT_DA2					NO_CONFIG_PORT
#define TCFG_SD0_PORT_DA3					NO_CONFIG_PORT

//*********************************************************************************//
//                                 key 配置                                        //
//*********************************************************************************//
#define KEY_NUM_MAX                        	10
#define KEY_NUM                            	1

#define MULT_KEY_ENABLE						DISABLE 		//是否使能组合按键消息, 使能后需要配置组合按键映射表

#define TCFG_KEY_TONE_EN					DISABLE 		// 按键提示音。

//*********************************************************************************//
//                                 iokey 配置                                      //
//*********************************************************************************//
#if CLIENT_BOARD == CUSTOM1_CFG
#define TCFG_IOKEY_ENABLE					ENABLE_THIS_MOUDLE //是否使能IO按键
#define TCFG_IOKEY_POWER_CONNECT_WAY		ONE_PORT_TO_LOW    //按键一端接低电平一端接IO
#define TCFG_IOKEY_POWER_ONE_PORT			IO_PORTB_02        //IO按键端口

#elif CLIENT_BOARD == CUSTOM2_CFG || \
 	  CLIENT_BOARD == CUSTOM3_CFG || \
 	  CLIENT_BOARD == CUSTOM4_CFG || \
 	  CLIENT_BOARD == CUSTOM5_CFG || \
 	  CLIENT_BOARD == CUSTOM11_CFG || \
 	  CLIENT_BOARD == CUSTOM6_CFG || \
	  CLIENT_BOARD == CUSTOM7_CFG || \
	  CLIENT_BOARD == CUSTOM8_CFG || \
	  CLIENT_BOARD == CUSTOM9_CFG

#define TCFG_IOKEY_ENABLE					ENABLE_THIS_MOUDLE //是否使能IO按键
#define TCFG_IOKEY_POWER_CONNECT_WAY		ONE_PORT_TO_LOW    //按键一端接低电平一端接IO
#define TCFG_IOKEY_POWER_ONE_PORT			IO_PORTB_01        //IO按键端口

#else
#define TCFG_IOKEY_ENABLE					DISABLE_THIS_MOUDLE //是否使能IO按键
#define TCFG_IOKEY_POWER_CONNECT_WAY		ONE_PORT_TO_LOW    //按键一端接低电平一端接IO
#define TCFG_IOKEY_POWER_ONE_PORT			IO_PORTB_01        //IO按键端口
#endif

//*********************************************************************************//
//                                 adkey 配置                                      //
//*********************************************************************************//
#define TCFG_ADKEY_ENABLE                   DISABLE_THIS_MOUDLE//是否使能AD按键
#define TCFG_ADKEY_PORT                     IO_PORT_DM         //AD按键端口(需要注意选择的IO口是否支持AD功能)
/*AD通道选择，需要和AD按键的端口相对应:
    AD_CH_PA1    AD_CH_PA3    AD_CH_PA4    AD_CH_PA5
    AD_CH_PA9    AD_CH_PA1    AD_CH_PB1    AD_CH_PB4
    AD_CH_PB6    AD_CH_PB7    AD_CH_DP     AD_CH_DM
    AD_CH_PB2
*/
#define TCFG_ADKEY_AD_CHANNEL               AD_CH_DM
#define TCFG_ADKEY_EXTERN_UP_ENABLE         ENABLE_THIS_MOUDLE //是否使用外部上拉

#if TCFG_ADKEY_EXTERN_UP_ENABLE
#define R_UP    220                 //22K，外部上拉阻值在此自行设置
#else
#define R_UP    100                 //10K，内部上拉默认10K
#endif

//必须从小到大填电阻，没有则同VDDIO,填0x3ffL
#define TCFG_ADKEY_AD0      (0)                                 //0R
#define TCFG_ADKEY_AD1      (0x3ffL * 30   / (30   + R_UP))     //3k
#define TCFG_ADKEY_AD2      (0x3ffL * 62   / (62   + R_UP))     //6.2k
#define TCFG_ADKEY_AD3      (0x3ffL * 91   / (91   + R_UP))     //9.1k
#define TCFG_ADKEY_AD4      (0x3ffL * 150  / (150  + R_UP))     //15k
#define TCFG_ADKEY_AD5      (0x3ffL * 240  / (240  + R_UP))     //24k
#define TCFG_ADKEY_AD6      (0x3ffL * 330  / (330  + R_UP))     //33k
#define TCFG_ADKEY_AD7      (0x3ffL * 510  / (510  + R_UP))     //51k
#define TCFG_ADKEY_AD8      (0x3ffL * 1000 / (1000 + R_UP))     //100k
#define TCFG_ADKEY_AD9      (0x3ffL * 2200 / (2200 + R_UP))     //220k
#define TCFG_ADKEY_VDDIO    (0x3ffL)

#define TCFG_ADKEY_VOLTAGE0 ((TCFG_ADKEY_AD0 + TCFG_ADKEY_AD1) / 2)
#define TCFG_ADKEY_VOLTAGE1 ((TCFG_ADKEY_AD1 + TCFG_ADKEY_AD2) / 2)
#define TCFG_ADKEY_VOLTAGE2 ((TCFG_ADKEY_AD2 + TCFG_ADKEY_AD3) / 2)
#define TCFG_ADKEY_VOLTAGE3 ((TCFG_ADKEY_AD3 + TCFG_ADKEY_AD4) / 2)
#define TCFG_ADKEY_VOLTAGE4 ((TCFG_ADKEY_AD4 + TCFG_ADKEY_AD5) / 2)
#define TCFG_ADKEY_VOLTAGE5 ((TCFG_ADKEY_AD5 + TCFG_ADKEY_AD6) / 2)
#define TCFG_ADKEY_VOLTAGE6 ((TCFG_ADKEY_AD6 + TCFG_ADKEY_AD7) / 2)
#define TCFG_ADKEY_VOLTAGE7 ((TCFG_ADKEY_AD7 + TCFG_ADKEY_AD8) / 2)
#define TCFG_ADKEY_VOLTAGE8 ((TCFG_ADKEY_AD8 + TCFG_ADKEY_AD9) / 2)
#define TCFG_ADKEY_VOLTAGE9 ((TCFG_ADKEY_AD9 + TCFG_ADKEY_VDDIO) / 2)

#define TCFG_ADKEY_VALUE0                   0
#define TCFG_ADKEY_VALUE1                   1
#define TCFG_ADKEY_VALUE2                   2
#define TCFG_ADKEY_VALUE3                   3
#define TCFG_ADKEY_VALUE4                   4
#define TCFG_ADKEY_VALUE5                   5
#define TCFG_ADKEY_VALUE6                   6
#define TCFG_ADKEY_VALUE7                   7
#define TCFG_ADKEY_VALUE8                   8
#define TCFG_ADKEY_VALUE9                   9

//*********************************************************************************//
//                                 irkey 配置                                      //
//*********************************************************************************//
#define TCFG_IRKEY_ENABLE                   DISABLE_THIS_MOUDLE//是否使能AD按键
#define TCFG_IRKEY_PORT                     IO_PORTA_08        //IR按键端口

//*********************************************************************************//
//                              tocuh key 配置                                     //
//*********************************************************************************//
//*********************************************************************************//
//                             lp tocuh key 配置                                   //
//*********************************************************************************//
#if CLIENT_BOARD == CUSTOM10_CFG
#define TCFG_LP_TOUCH_KEY_ENABLE 			ENABLE_THIS_MOUDLE 		//是否使能触摸总开关
#else
#define TCFG_LP_TOUCH_KEY_ENABLE 			DISABLE_THIS_MOUDLE 		//是否使能触摸总开关
#endif

#define TCFG_LP_TOUCH_KEY0_EN    			0                  		//是否使能触摸按键0 —— PB0
#define TCFG_LP_TOUCH_KEY1_EN    			1                  		//是否使能触摸按键1 —— PB1
#define TCFG_LP_TOUCH_KEY2_EN    			0                  		//是否使能触摸按键2 —— PB2
#define TCFG_LP_TOUCH_KEY3_EN    			0                  		//是否使能触摸按键3 —— PB4
#define TCFG_LP_TOUCH_KEY4_EN    			0                  		//是否使能触摸按键4 —— PB5

#define TCFG_LP_TOUCH_KEY0_WAKEUP_EN        0                  		//是否使能触摸按键0可以软关机低功耗唤醒
#define TCFG_LP_TOUCH_KEY1_WAKEUP_EN        1                  		//是否使能触摸按键1可以软关机低功耗唤醒
#define TCFG_LP_TOUCH_KEY2_WAKEUP_EN        0                  		//是否使能触摸按键2可以软关机低功耗唤醒
#define TCFG_LP_TOUCH_KEY3_WAKEUP_EN        0                  		//是否使能触摸按键3可以软关机低功耗唤醒
#define TCFG_LP_TOUCH_KEY4_WAKEUP_EN        0                  		//是否使能触摸按键4可以软关机低功耗唤醒

//两个按键以上，可以做简单的滑动处理
#define TCFG_LP_SLIDE_KEY_ENABLE            0                       //是否使能触摸按键的滑动功能
#define TCFG_LP_SLIDE_KEY_SHORT_DISTANCE    1                       //两个触摸按键距离是否很短，只支持两个按键，现流行适用于耳机。如果距离很远，则可以两个以上的远距离触摸按键的滑动。

#define TCFG_LP_EARTCH_KEY_ENABLE 			0	 	                //是否使能触摸按键用作入耳检测
#define TCFG_LP_EARTCH_KEY_CH               2                       //带入耳检测流程的按键号：0/1/2/3/4
#define TCFG_LP_EARTCH_KEY_REF_CH           1                       //入耳检测算法需要的另一个按键通道作为参考：0/1/2/3/4, 即入耳检测至少要使能两个触摸通道
#define TCFG_LP_EARTCH_SOFT_INEAR_VAL       3000                    //入耳检测算法需要的入耳阈值，参考文档设置
#define TCFG_LP_EARTCH_SOFT_OUTEAR_VAL      2000                    //入耳检测算法需要的出耳阈值，参考文档设置

//电容检测灵敏度级数配置(范围: 0 ~ 9)
//该参数配置与触摸时电容变化量有关, 触摸时电容变化量跟模具厚度, 触摸片材质, 面积等有关,
//触摸时电容变化量越小, 推荐选择灵敏度级数越大,
//触摸时电容变化量越大, 推荐选择灵敏度级数越小,
//用户可以从灵敏度级数为0开始调试, 级数逐渐增大, 直到选择一个合适的灵敏度配置值.
#define TCFG_LP_TOUCH_KEY0_SENSITIVITY		5 	//触摸按键0电容检测灵敏度配置(级数0 ~ 9)
#define TCFG_LP_TOUCH_KEY1_SENSITIVITY		5 	//触摸按键1电容检测灵敏度配置(级数0 ~ 9)
#define TCFG_LP_TOUCH_KEY2_SENSITIVITY		5 	//触摸按键2电容检测灵敏度配置(级数0 ~ 9)
#define TCFG_LP_TOUCH_KEY3_SENSITIVITY		5 	//触摸按键3电容检测灵敏度配置(级数0 ~ 9)
#define TCFG_LP_TOUCH_KEY4_SENSITIVITY		5 	//触摸按键4电容检测灵敏度配置(级数0 ~ 9)

//内置触摸灵敏度调试工具使能, 使能后可以通过连接PC端上位机通过SPP调试,
//打开该宏需要确保同时打开宏:
//1.USER_SUPPORT_PROFILE_SPP
//2.APP_ONLINE_DEBUG
//可以针对每款样机校准灵敏度参数表(在lp_touch_key.c ch_sensitivity_table), 详细使用方法请参考《低功耗内置触摸介绍》文档.
#define TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE 	DISABLE


#if TCFG_LP_TOUCH_KEY_ENABLE
//如果 IO按键与触摸按键有冲突，则需要关掉 IO按键的 宏
#if 1
//取消外置触摸的一些宏定义
#ifdef TCFG_IOKEY_ENABLE
#undef TCFG_IOKEY_ENABLE
#define TCFG_IOKEY_ENABLE 					DISABLE_THIS_MOUDLE
#endif /* #ifdef TCFG_IOKEY_ENABLE */
#endif

//如果 AD按键与触摸按键有冲突，则需要关掉 AD按键的 宏
#if 1
//取消外置触摸的一些宏定义
#ifdef TCFG_ADKEY_ENABLE
#undef TCFG_ADKEY_ENABLE
#define TCFG_ADKEY_ENABLE 					DISABLE_THIS_MOUDLE
#endif /* #ifdef TCFG_ADKEY_ENABLE */
#endif

#if TCFG_LP_EARTCH_KEY_ENABLE
#ifdef TCFG_EARTCH_EVENT_HANDLE_ENABLE
#undef TCFG_EARTCH_EVENT_HANDLE_ENABLE
#endif
#define TCFG_EARTCH_EVENT_HANDLE_ENABLE 	ENABLE_THIS_MOUDLE 		//入耳检测事件APP流程处理使能

#if TCFG_LP_SLIDE_KEY_ENABLE
#error "入耳检测和滑动功能不能同时打开"
#endif

#endif /* #if TCFG_LP_EARTCH_KEY_ENABLE */


#endif /* #if TCFG_LP_TOUCH_KEY_ENABLE */


//*********************************************************************************//
//                             plcnt tocuh key 配置                                   //
//*********************************************************************************//
#define TCFG_TOUCH_KEY_ENABLE               DISABLE_THIS_MOUDLE             //是否使能plcnt触摸按键
//key0配置
#define TCFG_TOUCH_KEY0_PRESS_DELTA	   	    100//变化阈值，当触摸产生的变化量达到该阈值，则判断被按下，每个按键可能不一样，可先在驱动里加到打印，再反估阈值
#define TCFG_TOUCH_KEY0_PORT 				IO_PORTB_06 //触摸按键key0 IO配置
#define TCFG_TOUCH_KEY0_VALUE 				0x12 		//触摸按键key0 按键值
//key1配置
#define TCFG_TOUCH_KEY1_PRESS_DELTA	   	    100//变化阈值，当触摸产生的变化量达到该阈值，则判断被按下，每个按键可能不一样，可先在驱动里加到打印，再反估阈值
#define TCFG_TOUCH_KEY1_PORT 				IO_PORTB_07 //触摸按键key1 IO配置
#define TCFG_TOUCH_KEY1_VALUE 				0x34        //触摸按键key1 按键值

//*********************************************************************************//
//                                 rdec_key 配置                                      //
//*********************************************************************************//
#define TCFG_RDEC_KEY_ENABLE					DISABLE_THIS_MOUDLE //是否使能RDEC按键
//RDEC0配置
#define TCFG_RDEC0_ECODE1_PORT					IO_PORTA_03
#define TCFG_RDEC0_ECODE2_PORT					IO_PORTA_04
#define TCFG_RDEC0_KEY0_VALUE 				 	0
#define TCFG_RDEC0_KEY1_VALUE 				 	1

//RDEC1配置
#define TCFG_RDEC1_ECODE1_PORT					IO_PORTA_05
#define TCFG_RDEC1_ECODE2_PORT					IO_PORTA_06
#define TCFG_RDEC1_KEY0_VALUE 				 	2
#define TCFG_RDEC1_KEY1_VALUE 				 	3

//RDEC2配置
#define TCFG_RDEC2_ECODE1_PORT					IO_PORTB_00
#define TCFG_RDEC2_ECODE2_PORT					IO_PORTB_01
#define TCFG_RDEC2_KEY0_VALUE 				 	4
#define TCFG_RDEC2_KEY1_VALUE 				 	5

//*********************************************************************************//
//                           Digital Hearing Aid(DHA)辅听耳机配置                      //
//*********************************************************************************//
/*辅听功能使能*/
#define TCFG_AUDIO_HEARING_AID_ENABLE		DISABLE_THIS_MOUDLE
/*听力验配功能*/
#define TCFG_AUDIO_DHA_FITTING_ENABLE		DISABLE
/*辅听功能互斥配置*/
#define TCFG_AUDIO_DHA_AND_MUSIC_MUTEX		ENABLE	//辅听功能和音乐播放互斥(默认互斥，资源有限)
#define TCFG_AUDIO_DHA_AND_CALL_MUTEX		ENABLE	//辅听功能和通话互斥(默认互斥，资源有限)
#define TCFG_AUDIO_DHA_AND_TONE_MUTEX		ENABLE	//辅听功能和提示音互斥
/*辅听功能MIC的采样率*/
#define TCFG_AUDIO_DHA_MIC_SAMPLE_RATE		(32000)

/*混叠DAC输出采样率*/
#define TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE      (32000)
/*低延时 mic 通道*/
#define TCFG_AD2DA_LOW_LATENCY_MIC_CHANNEL      (AUDIO_ADC_MIC_0)

#if TCFG_AUDIO_HEARING_AID_ENABLE
#define TCFG_AUDIO_MUSIC_SAMPLE_RATE        TCFG_AD2DA_LOW_LATENCY_SAMPLE_RATE
#endif

//*********************************************************************************//
//                                  Sidetone配置                                   //
//*********************************************************************************//
#define TCFG_SIDETONE_ENABLE                0 //ENABLE

//*********************************************************************************//
//                                 Audio配置                                       //
//*********************************************************************************//
#define TCFG_AUDIO_ADC_ENABLE				ENABLE_THIS_MOUDLE
#define TCFG_AUDIO_ADC_LINE_CHA				AUDIO_ADC_LINE0
/*MIC LDO电流档位设置：
    0:0.625ua    1:1.25ua    2:1.875ua    3:2.5ua*/
#define TCFG_AUDIO_ADC_LD0_SEL				2

#define TCFG_AUDIO_DAC_ENABLE				ENABLE_THIS_MOUDLE
#define TCFG_AUDIO_DAC_LDO_SEL				1

/*
DAC硬件上的连接方式,可选的配置：
    DAC_OUTPUT_MONO_L               左声道
    DAC_OUTPUT_MONO_R               右声道
    DAC_OUTPUT_LR                   立体声
    DAC_OUTPUT_MONO_LR_DIFF         左右差分 LP-RP

    TCFG_AUDIO_DAC_CONNECT_MODE 决定方案需要使用多少个DAC声道
    TCFG_AUDIO_DAC_MODE 决定DAC是差分输出模式还是单端输出模式

    TCFG_AUDIO_DAC_CONNECT_MODE和TCFG_AUDIO_DAC_MODE不同配置组合实际使用引脚见下表：

+--------------------+-------------------+-------------------+----------------------------+-------------------------+
|                    | DAC_OUTPUT_MONO_L | DAC_OUTPUT_MONO_R |    DAC_OUTPUT_LR           | DAC_OUTPUT_MONO_LR_DIFF |
+--------------------+-------------------+-------------------+----------------------------+-------------------------+
| DAC_MODE_L_DIFF    | 单声道  LP-LN     | 单声道  RP-RN     | 立声道 L:LP-LN   R:RP-RN   |         不支持          |
| DAC_MODE_L_SINGLE  | 单声道  LP-AGND   | 单声道  RP-AGND   | 立声道 L:LP-AGND R:RP-AGND |      单声道  LP-RP      |
| DAC_MODE_H1_DIFF   | 单声道  LP-LN     | 单声道  RP-RN     | 立声道 L:LP-LN   R:RP-RN   |         不支持          |
| DAC_MODE_H1_SINGLE | 单声道  LP-AGND   | 单声道  RP-AGND   | 立声道 L:LP-AGND R:RP-AGND |      单声道  LP-RP      |
| DAC_MODE_H2_DIFF   | 单声道  LP-LN     | 单声道  RP-RN     | 立声道 L:LP-LN   R:RP-RN   |         不支持          |
| DAC_MODE_H2_SINGLE | 单声道  LP-AGND   | 单声道  RP-AGND   | 立声道 L:LP-AGND R:RP-AGND |      单声道  LP-RP      |
+--------------------+-------------------+-------------------+----------------------------+-------------------------+
*/
#if CLIENT_BOARD == CUSTOM5_CFG || \
	CLIENT_BOARD == CUSTOM11_CFG || \
	CLIENT_BOARD == CUSTOM9_CFG
#define TCFG_AUDIO_DAC_CONNECT_MODE         DAC_OUTPUT_LR
#else
#define TCFG_AUDIO_DAC_CONNECT_MODE         DAC_OUTPUT_MONO_L
#endif

/*
         DAC模式选择
#define DAC_MODE_L_DIFF          (0)  // 低压差分模式   , 适用于低功率差分耳机  , 输出幅度 0~2Vpp
#define DAC_MODE_L_SINGLE        (1)  // 低压单端模式   , 主要用于左右差分模式, 差分输出幅度 0~2Vpp
#define DAC_MODE_H1_DIFF         (2)  // 高压1档差分模式, 适用于高功率差分耳机  , 输出幅度 0~3Vpp  , VDDIO >= 3.0V
#define DAC_MODE_H1_SINGLE       (3)  // 高压1档单端模式, 适用于高功率单端PA音箱, 输出幅度 0~1.5Vpp, VDDIO >= 3.0V
#define DAC_MODE_H2_DIFF         (4)  // 高压2档差分模式, 适用于高功率差分PA音箱, 输出幅度 0~5Vpp  , VDDIO >= 3.3V
#define DAC_MODE_H2_SINGLE       (5)  // 高压2档单端模式, 适用于高功率单端PA音箱, 输出幅度 0~2.5Vpp, VDDIO >= 3.3V
*/
#if CLIENT_BOARD == CUSTOM2_CFG ||\
    CLIENT_BOARD == CUSTOM5_CFG ||\
    CLIENT_BOARD == CUSTOM11_CFG ||\
    CLIENT_BOARD == CUSTOM9_CFG
#define TCFG_AUDIO_DAC_MODE         DAC_MODE_H1_DIFF    // DAC_MODE_L_DIFF 低压， DAC_MODE_H1_DIFF 高压
#else
#define TCFG_AUDIO_DAC_MODE         DAC_MODE_L_DIFF    // DAC_MODE_L_DIFF 低压， DAC_MODE_H1_DIFF 高压
#endif

#define TCFG_APP_AUX_EN                    DISABLE_THIS_MOUDLE

/*预留接口，未使用*/
#define TCFG_AUDIO_DAC_PA_PORT				NO_CONFIG_PORT

//*********************************ANC配置******************************************//

#define TCFG_AUDIO_ANC_ENABLE				CONFIG_ANC_ENABLE		//ANC总使能,根据global_bulid_cfg板级定义
#define TCFG_ANC_TOOL_DEBUG_ONLINE 			ENABLE_THIS_MOUDLE		//ANC工具蓝牙spp调试
#define TCFG_ANC_SELF_DUT_GET_SZ			DISABLE_THIS_MOUDLE		//ANC自研产测SZ获取，用于(HYBIRD/FB)产测
/*
 *当通话与ANC共用FF MIC，并且通话要求更大的MIC模拟增益时，使能TCFG_AUDIO_DYNAMIC_ADC_GAIN，
 *则通话MIC模拟增益以配置工具通话参数为准,通话时，ANC使用通话MIC模拟增益
 */
#define TCFG_AUDIO_DYNAMIC_ADC_GAIN			ENABLE_THIS_MOUDLE
#define TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN		DISABLE_THIS_MOUDLE		//ANC耳道自适应使能, 耳道是变量

/* ICSD ADT 相关功能配置 */
#define TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE         DISABLE_THIS_MOUDLE      //智能免摘
#define TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE    DISABLE_THIS_MOUDLE      //ANC风噪检测
#define TCFG_AUDIO_WIDE_AREA_TAP_ENABLE         DISABLE_THIS_MOUDLE      //广域点击

#if (TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE || TCFG_AUDIO_WIDE_AREA_TAP_ENABLE || TCFG_AUDIO_ANC_WIND_NOISE_DET_ENABLE)
#define TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN		ENABLE_THIS_MOUDLE		//声音检测(使能智能免摘需要打开)
#else
#define TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN		DISABLE_THIS_MOUDLE		//声音检测
#endif

#if CLIENT_BOARD == CUSTOM1_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC0					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC3					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型

#elif CLIENT_BOARD == CUSTOM2_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC1					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC2					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型

#elif CLIENT_BOARD == CUSTOM6_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC1					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC2					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型
/*ANC最大滤波器阶数限制，若设置超过10，ANC配置区则占用8K(range: 10-20)*/

#elif CLIENT_BOARD == CUSTOM7_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC2					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC1					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型
/*ANC最大滤波器阶数限制，若设置超过10，ANC配置区则占用8K(range: 10-20)*/

#elif CLIENT_BOARD == CUSTOM4_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC1					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC2					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型

#elif CLIENT_BOARD == CUSTOM3_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC2					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC1					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型

#elif CLIENT_BOARD == CUSTOM8_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC2					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC0					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型

#elif CLIENT_BOARD == CUSTOM5_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH | ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC0					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC2					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							A_MIC1				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							A_MIC3                //ANC右通道FBMIC类型
/*ANC最大滤波器阶数限制，若设置超过10，ANC配置区则占用8K(range: 10-20)*/

#elif CLIENT_BOARD == CUSTOM11_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH | ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC0					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC2					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							A_MIC1				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							A_MIC3                //ANC右通道FBMIC类型
/*ANC最大滤波器阶数限制，若设置超过10，ANC配置区则占用8K(range: 10-20)*/

#elif CLIENT_BOARD == CUSTOM9_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH | ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC0					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC2					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							A_MIC1				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							A_MIC3                //ANC右通道FBMIC类型

#elif CLIENT_BOARD == CUSTOM10_CFG
#define ANC_TRAIN_MODE						ANC_HYBRID_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC1					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							A_MIC2					//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型

#else	/*DEVE_BOARD*/
#define ANC_TRAIN_MODE						ANC_FF_EN				//ANC类型配置：单前馈，单后馈，混合馈
#define ANC_CH 			 					ANC_L_CH //| ANC_R_CH	//ANC通道使能：左通道 | 右通道
#define ANCL_FF_MIC							A_MIC0					//ANC左通道FFMIC类型
#define ANCL_FB_MIC							MIC_NULL				//ANC左通道FBMIC类型
#define ANCR_FF_MIC							MIC_NULL				//ANC右通道FFMIC类型
#define ANCR_FB_MIC							MIC_NULL                //ANC右通道FBMIC类型
#endif
/********************************ANC配置END*****************************************/

/*通话下行(DL:DownLink)通路噪声门配置*/
#define TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE	DISABLE_THIS_MOUDLE
#define ESCO_NOISEGATE_THR		-40.f	//噪声门阈值:-90~0 Unit:dB
#define ESCO_NOISEGATE_GAIN		0.0f	//噪声增益衰减比例:0~1 Ratio

/*通话下行降噪使能*/
#define TCFG_ESCO_DL_NS_ENABLE				DISABLE_THIS_MOUDLE

/*
 *系统音量类型选择
 *软件数字音量是指纯软件对声音进行运算后得到的
 *硬件数字音量是指dac内部数字模块对声音进行运算后输出
 *选择软件数字音量或者硬件数字音量会固定模拟音量，
 *固定的模拟音量值由 MAX_ANA_VOL 决定
 */
#define VOL_TYPE_DIGITAL		0	//软件数字音量
#define VOL_TYPE_AD				2	//联合音量(模拟数字混合调节)
#define VOL_TYPE_DIGITAL_HW		3  	//硬件数字音量
/*注意:ANC使能情况下使用软件数字音量*/
#if (TCFG_AUDIO_ANC_ENABLE || TCFG_AD2DA_LOW_LATENCY_ENABLE || TCFG_AUDIO_HEARING_AID_ENABLE)
#define SYS_VOL_TYPE            VOL_TYPE_DIGITAL
#else
#define SYS_VOL_TYPE            VOL_TYPE_DIGITAL_HW
#endif/*TCFG_AUDIO_ANC_ENABLE*/
/*
 *通话的时候使用数字音量
 *0：通话使用和SYS_VOL_TYPE一样的音量调节类型
 *1：通话使用数字音量调节，更加平滑
 */
#define TCFG_CALL_USE_DIGITAL_VOLUME		0
//注意：数字音量不用使能TCFG_CALL_USE_DIGITAL_VOLUME
#if ((SYS_VOL_TYPE == VOL_TYPE_DIGITAL) || (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW))
#undef TCFG_CALL_USE_DIGITAL_VOLUME
#define TCFG_CALL_USE_DIGITAL_VOLUME		0
#endif/*SYS_VOL_TYPE*/

/*PDM数字MIC配置*/
#define TCFG_AUDIO_PLNK_SCLK_PIN			IO_PORTA_07
#define TCFG_AUDIO_PLNK_DAT0_PIN			IO_PORTA_08
#define TCFG_AUDIO_PLNK_DAT1_PIN			NO_CONFIG_PORT

/*MIC模式配置:单端隔直电容模式/差分隔直电容模式/单端省电容模式*/
/*注意:ANC使能情况下，使用差分mic*/
#define TCFG_AUDIO_MIC_MODE					AUDIO_MIC_CAP_DIFF_MODE
#define TCFG_AUDIO_MIC1_MODE				AUDIO_MIC_CAP_DIFF_MODE
#define TCFG_AUDIO_MIC2_MODE				AUDIO_MIC_CAP_DIFF_MODE
#define TCFG_AUDIO_MIC3_MODE				AUDIO_MIC_CAP_DIFF_MODE

/*
 *>>MIC电源管理:根据具体方案，选择对应的mic供电方式
 *(1)如果是多种方式混合，则将对应的供电方式或起来即可，比如(MIC_PWR_FROM_GPIO | MIC_PWR_FROM_MIC_BIAS)
 *(2)如果使用固定电源供电(比如dacvdd)，则配置成DISABLE_THIS_MOUDLE
 */
#define MIC_PWR_FROM_GPIO		(1UL << 0)	//使用普通IO输出供电
#define MIC_PWR_FROM_MIC_BIAS	(1UL << 1)	//使用内部mic_ldo供电(有上拉电阻可配)
#define MIC_PWR_FROM_MIC_LDO	(1UL << 2)	//使用内部mic_ldo供电
//配置MIC电源
#define TCFG_AUDIO_MIC_PWR_CTL				MIC_PWR_FROM_MIC_LDO

//使用普通IO输出供电:不用的port配置成NO_CONFIG_PORT
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_GPIO)
#define TCFG_AUDIO_MIC_PWR_PORT				IO_PORTC_02
#define TCFG_AUDIO_MIC1_PWR_PORT			NO_CONFIG_PORT
#define TCFG_AUDIO_MIC2_PWR_PORT			NO_CONFIG_PORT
#endif/*MIC_PWR_FROM_GPIO*/

//使用内部mic_ldo供电(有上拉电阻可配)
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_MIC_BIAS)
#define TCFG_AUDIO_MIC0_BIAS_EN				ENABLE_THIS_MOUDLE/*Port:PA2*/
#define TCFG_AUDIO_MIC1_BIAS_EN				ENABLE_THIS_MOUDLE/*Port:PA4*/
#define TCFG_AUDIO_MIC2_BIAS_EN				ENABLE_THIS_MOUDLE/*Port:PG7*/
#define TCFG_AUDIO_MIC3_BIAS_EN				ENABLE_THIS_MOUDLE/*Port:PG5*/
#endif/*MIC_PWR_FROM_MIC_BIAS*/

//使用内部mic_ldo供电(Port:PA0)
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_MIC_LDO)
#define TCFG_AUDIO_MIC_LDO_EN				ENABLE_THIS_MOUDLE
#endif/*MIC_PWR_FROM_MIC_LDO*/
/*>>MIC电源管理配置结束*/

/*
 *第三方清晰语音开发配置:
 *(1)使用杰理自研算法
 *#define TCFG_CVP_DEVELOP_ENABLE             DISABLE_THIS_MOUDLE
 *(2)用户自己开发算法
 *#define TCFG_CVP_DEVELOP_ENABLE             CVP_CFG_USER_DEFINED
 *(3)使用思必驰3-mic通话算法
 *#define TCFG_CVP_DEVELOP_ENABLE             CVP_CFG_AIS_3MIC
 */
#define CVP_CFG_USER_DEFINED	1	//用户自定义开发算法
#define CVP_CFG_AIS_3MIC		2	//思必驰3mic算法
#define TCFG_CVP_DEVELOP_ENABLE             DISABLE_THIS_MOUDLE

#if (TCFG_CVP_DEVELOP_ENABLE == CVP_CFG_AIS_3MIC)
#define CONFIG_BOARD_AISPEECH_NR
#define CONFIG_BOARD_AISPEECH_NR_GAIN
#endif /*TCFG_CVP_DEVELOP_ENABLE*/

#ifdef CONFIG_BOARD_AISPEECH_NR_GAIN
//思必驰三麦MIC增益设置
#define TALK_MIC0_GAIN      13
#define FF_MIC1_GAIN        13
#define FB_MIC2_GAIN        0
#define OTHER_MIC3_GAIN     0
#endif /*CONFIG_BOARD_AISPEECH_NR_GAIN*/

/*通话降噪模式配置*/
#define CVP_ANS_MODE	0	/*传统降噪*/
#define CVP_DNS_MODE	1	/*神经网络降噪*/
#define TCFG_AUDIO_CVP_NS_MODE				CVP_DNS_MODE

/*
 * ENC(双mic降噪)配置
 * 双mic降噪包括DMS_NORMAL和DMS_FLEXIBLE，在使能TCFG_AUDIO_DUAL_MIC_ENABLE
 * 的前提下，根据具体需求，选择对应的DMS模式
 */
/*ENC(双mic降噪)使能*/
#if CLIENT_BOARD == CUSTOM1_CFG || \
    CLIENT_BOARD == CUSTOM2_CFG || \
    CLIENT_BOARD == CUSTOM4_CFG || \
    CLIENT_BOARD == CUSTOM8_CFG || \
    CLIENT_BOARD == CUSTOM10_CFG || \
    CLIENT_BOARD == CUSTOM3_CFG
#define TCFG_AUDIO_DUAL_MIC_ENABLE			ENABLE_THIS_MOUDLE

#else
#define TCFG_AUDIO_DUAL_MIC_ENABLE			DISABLE_THIS_MOUDLE
#endif

/*ENC(3 mic降噪)使能*/
#if (TCFG_CVP_DEVELOP_ENABLE == CVP_CFG_AIS_3MIC)
#define TCFG_AUDIO_TRIPLE_MIC_ENABLE		ENABLE_THIS_MOUDLE
#else
#define TCFG_AUDIO_TRIPLE_MIC_ENABLE		DISABLE_THIS_MOUDLE
#endif /*TCFG_CVP_DEVELOP_ENABLE*/

/*DMS模式选择*/
#define DMS_NORMAL		1	//普通双mic降噪(mic距离固定)
#define DMS_FLEXIBLE	2	//适配mic距离不固定且距离比较远的情况，比如头戴式话务耳机
#define TCFG_AUDIO_DMS_SEL					DMS_NORMAL

/*ENC双mic配置主mic副mic对应的mic port*/
#define DMS_MASTER_MIC0		0 //mic0是主mic
#define DMS_MASTER_MIC1		1 //mic1是主mic
#if CLIENT_BOARD == CUSTOM1_CFG
#define TCFG_AUDIO_DMS_MIC_MANAGE			DMS_MASTER_MIC1

#elif CLIENT_BOARD == CUSTOM2_CFG || \
	  CLIENT_BOARD == CUSTOM3_CFG || \
	  CLIENT_BOARD == CUSTOM10_CFG
#define TCFG_AUDIO_DMS_MIC_MANAGE			DMS_MASTER_MIC0

#else
#define TCFG_AUDIO_DMS_MIC_MANAGE			DMS_MASTER_MIC0
#endif

/*通话CVP 测试使能，配合第三方量产设备测试MIC频响和算法性能*/
#define TCFG_AUDIO_CVP_DUT_ENABLE			DISABLE_THIS_MOUDLE
/*自研通话产测使能, 配合JL π-HAT 上位机，测试MIC频响和算法性能*/
#define AUDIO_ENC_MPT_SELF_ENABLE 		DISABLE_THIS_MOUDLE

//MIC通道配置
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
#define TCFG_AUDIO_ADC_MIC_CHA				(AUDIO_ADC_MIC_0 | AUDIO_ADC_MIC_1 | AUDIO_ADC_MIC_2)

#elif TCFG_AUDIO_DUAL_MIC_ENABLE
#if CLIENT_BOARD == CUSTOM1_CFG || \
    CLIENT_BOARD == CUSTOM4_CFG || \
    CLIENT_BOARD == CUSTOM10_CFG || \
    CLIENT_BOARD == CUSTOM2_CFG
#define TCFG_AUDIO_ADC_MIC_CHA				(AUDIO_ADC_MIC_0 | AUDIO_ADC_MIC_1)

#elif CLIENT_BOARD == CUSTOM3_CFG
#define TCFG_AUDIO_ADC_MIC_CHA				(AUDIO_ADC_MIC_0 | AUDIO_ADC_MIC_2)

#elif CLIENT_BOARD == CUSTOM8_CFG
#define TCFG_AUDIO_ADC_MIC_CHA				(AUDIO_ADC_MIC_1 | AUDIO_ADC_MIC_2)

#else
#define TCFG_AUDIO_ADC_MIC_CHA				AUDIO_ADC_MIC_0
#endif

#else
#define TCFG_AUDIO_ADC_MIC_CHA				AUDIO_ADC_MIC_0
#endif/*TCFG_AUDIO_TRIPLE_MIC_ENABLE*/

/*
 *支持省电容MIC模块
 *(1)要使能省电容mic,首先要支持该模块:TCFG_SUPPORT_MIC_CAPLESS
 *(2)只有支持该模块，才能使能该模块:TCFG_MIC_CAPLESS_ENABLE
 */
#define TCFG_SUPPORT_MIC_CAPLESS			DISABLE_THIS_MOUDLE
//省电容MIC使能
#define TCFG_MIC_CAPLESS_ENABLE				DISABLE_THIS_MOUDLE
//省电容MIC1使能
#define TCFG_MIC1_CAPLESS_ENABLE			DISABLE_THIS_MOUDLE

// AUTOMUTE
#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_AND_MUSIC_MUTEX)
#define AUDIO_OUTPUT_AUTOMUTE   			ENABLE_THIS_MOUDLE
#else
#define AUDIO_OUTPUT_AUTOMUTE   			DISABLE_THIS_MOUDLE
#endif/*TCFG_AUDIO_HEARING_AID_ENABLE*/


// OUTPUT WAY
#define AUDIO_OUT_WAY_TYPE 					AUDIO_WAY_TYPE_DAC

/*Audio数据导出配置:支持BT_SPP\SD\UART载体导出*/
#define AUDIO_DATA_EXPORT_USE_SD	1
#define AUDIO_DATA_EXPORT_USE_SPP 	2
#define AUDIO_DATA_EXPORT_USE_UART 	3
#define TCFG_AUDIO_DATA_EXPORT_ENABLE		DISABLE_THIS_MOUDLE
/*
 *蓝牙spp数据导出的mic 通道，调试双麦ENC时,需要和ENC的mic通道保持一致
 */
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
/*3MIC通话的时候需要导出3路麦克风数据分析*/
#define TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA	(AUDIO_ADC_MIC_0 | AUDIO_ADC_MIC_1 | AUDIO_ADC_MIC_2)
#else
/*2MIC通话的时候需要导出两路麦克风数据分析*/
#define TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA	(AUDIO_ADC_MIC_0 | AUDIO_ADC_MIC_1)
#endif

/*通话清晰语音处理数据导出*/
//#define AUDIO_PCM_DEBUG

/*通话参数在线调试*/
#define TCFG_AEC_TOOL_ONLINE_ENABLE         DISABLE_THIS_MOUDLE

/*麦克风测试和传递函数测试*/
#define TCFG_AUDIO_MIC_DUT_ENABLE           DISABLE_THIS_MOUDLE

//*********************************************************************************//
//                                 Audio Smart Voice                               //
//*********************************************************************************//
#define TCFG_SMART_VOICE_ENABLE 		    DISABLE_THIS_MOUDLE
#define TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE 	TCFG_SMART_VOICE_ENABLE //语音事件处理流程开关
#if TCFG_SMART_VOICE_ENABLE
#define TCFG_VAD_LOWPOWER_CLOCK             VAD_CLOCK_USE_PMU_STD12M
#else
#define TCFG_VAD_LOWPOWER_CLOCK             VAD_CLOCK_USE_LRC
#endif/*TCFG_AUDIO_SMART_VOICE_ENABLE*/
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
#define TCFG_CALL_KWS_SWITCH_ENABLE         DISABLE_THIS_MOUDLE
#else
#define TCFG_CALL_KWS_SWITCH_ENABLE         TCFG_SMART_VOICE_ENABLE
#endif

/*
 *第三方ASR（语音识别）配置
 *(1)用户自己开发算法
 *#define TCFG_AUDIO_ASR_DEVELOP             ASR_CFG_USER_DEFINED
 *(2)使用思必驰ASR算法
 *#define TCFG_AUDIO_ASR_DEVELOP             ASR_CFG_AIS
 */
#define ASR_CFG_USER_DEFINED	1
#define ASR_CFG_AIS				2
#define TCFG_AUDIO_ASR_DEVELOP				DISABLE_THIS_MOUDLE

#if (TCFG_AUDIO_ASR_DEVELOP	== ASR_CFG_AIS)
#undef TCFG_SMART_VOICE_ENABLE
#undef TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE
#undef TCFG_VAD_LOWPOWER_CLOCK
#define TCFG_SMART_VOICE_ENABLE 		    DISABLE_THIS_MOUDLE
#define TCFG_KWS_VOICE_EVENT_HANDLE_ENABLE 	TCFG_SMART_VOICE_ENABLE //语音事件处理流程开关
#define TCFG_VAD_LOWPOWER_CLOCK             VAD_CLOCK_USE_PMU_STD12M

#define CONFIG_BOARD_AISPEECH_VAD_ASR
#endif /*TCFG_AUDIO_ASR_DEVELOP*/

//*********************************************************************************//
//                         Spatial Audio Effect 空间音效配置                       //
//*********************************************************************************//
#define TCFG_AUDIO_SPATIAL_EFFECT_ENABLE           	DISABLE_THIS_MOUDLE
#define TCFG_TWS_SPATIAL_AUDIO_AS_CHANNEL   		'L'
/*空间音效在线调试*/
#define TCFG_SPATIAL_EFFECT_ONLINE_ENABLE           DISABLE_THIS_MOUDLE
/*空间音频传感器是否在解码任务读传感器数据*/
#define TCFG_SENSOR_DATA_READ_IN_DEC_TASK           DISABLE_THIS_MOUDLE
/*传感器数据读取的频率间隔，单位ms*/
#define TCFG_SENSOR_DATA_READ_INTERVAL              100
/*空间音频独立EQ使能*/
#define TCFG_SPATIAL_EFFECT_EQ_ENABLE               DISABLE_THIS_MOUDLE

/*陀螺仪数据导出配置:支持BT_SPP\UART载体导出*/
#define SENSOR_DATA_EXPORT_USE_UART 	1
#define SENSOR_DATA_EXPORT_USE_SPP 	    2
#define TCFG_SENSOR_DATA_EXPORT_ENABLE				DISABLE_THIS_MOUDLE

//*********************************************************************************//
//                                 IIS 配置                                  	   //
//*********************************************************************************//
#define TCFG_AUDIO_INPUT_IIS                DISABLE_THIS_MOUDLE
#define TCFG_AUDIO_OUTPUT_IIS               DISABLE_THIS_MOUDLE

//IIS 输出使用的通道号
#define TCFG_IIS_OUTPUT_DATAPORT_SEL        ALINK_CH0
//IIS 输入使用的通道号
#define TCFG_IIS_INPUT_DATAPORT_SEL         ALINK_CH1

#define TCFG_IIS_MODE                       (0)   //  0:master  1:slave
#define TCFG_IIS_SR                         (44100)

#define TCFG_IIS_MCLK_IO                    IO_PORTA_12
#define TCFG_SCLK_IO                        IO_PORTA_13
#define TCFG_LRCLK_IO                       IO_PORTA_14
#define TCFG_DATA0_IO                       IO_PORTA_15
#define TCFG_DATA1_IO                       IO_PORTA_11
#define TCFG_DATA2_IO                       IO_PORTA_11
#define TCFG_DATA3_IO                       IO_PORTA_11

//*********************************************************************************//
//                                  充电舱配置                                     //
//   充电舱/蓝牙测试盒/ANC测试三者为同级关系,开启任一功能都会初始化PP0通信接口     //
//*********************************************************************************//
#define TCFG_CHARGESTORE_ENABLE				DISABLE_THIS_MOUDLE         //是否支持智能充电舱
#define TCFG_TEST_BOX_ENABLE			    ENABLE_THIS_MOUDLE          //是否支持蓝牙测试盒
#define TCFG_ANC_BOX_ENABLE			        CONFIG_ANC_ENABLE           //是否支持ANC测试盒
#define TCFG_UMIDIGI_BOX_ENABLE				ENABLE_THIS_MOUDLE			//是否支持UMIDIGI充电舱
#if TCFG_UMIDIGI_BOX_ENABLE
#define _20MS_BIT							20							//传输20ms/Bit时使用
#define _40MS_BIT							40							//传输40ms/Bit时使用
#define TIMER2CMESSAGE 						_20MS_BIT					//定时器收集message周期
#endif
#if CLIENT_BOARD == CUSTOM5_CFG  || \
    CLIENT_BOARD == CUSTOM11_CFG || \
    CLIENT_BOARD == CUSTOM9_CFG
#define TCFG_CHARGESTORE_PORT				IO_PORT_DM                 //耳机通讯的IO口
#else
#define TCFG_CHARGESTORE_PORT				IO_PORTP_00                 //耳机通讯的IO口
#endif/*CLIENT_BOARD == CUSTOM5_CFG*/

//*********************************************************************************//
//                                  充电参数配置                                   //
//*********************************************************************************//
//是否支持芯片内置充电
#define TCFG_CHARGE_ENABLE					ENABLE_THIS_MOUDLE
//是否支持开机充电
#define TCFG_CHARGE_POWERON_ENABLE			DISABLE
//是否支持拔出充电自动开机功能
#define TCFG_CHARGE_OFF_POWERON_NE			ENABLE
/*充电截止电压可选配置*/
#define TCFG_CHARGE_FULL_V					CHARGE_FULL_V_4199
/*充电截止电流可选配置*/
#define TCFG_CHARGE_FULL_MA					CHARGE_FULL_mA_10
/*恒流充电电流可选配置*/
#define TCFG_CHARGE_MA						CHARGE_mA_40
/*涓流充电电流配置*/
#define TCFG_CHARGE_TRICKLE_MA              CHARGE_mA_10

//*********************************************************************************//
//                                  LED 配置                                       //
//*********************************************************************************//
#if CLIENT_BOARD == CUSTOM3_CFG
#define TCFG_PWMLED_ENABLE					ENABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTC_03					//LED使用的IO口
#elif CLIENT_BOARD == CUSTOM4_CFG
#define TCFG_PWMLED_ENABLE					ENABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTB_05					//LED使用的IO口

#elif CLIENT_BOARD == CUSTOM6_CFG
#define TCFG_PWMLED_ENABLE					ENABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTB_05					//LED使用的IO口

#elif CLIENT_BOARD == CUSTOM8_CFG
#define TCFG_PWMLED_ENABLE					ENABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTC_04					//LED使用的IO口

#elif CLIENT_BOARD == CUSTOM10_CFG
#define TCFG_PWMLED_ENABLE					ENABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTG_05 				//LED使用的IO口

#else
#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTB_06					//LED使用的IO口
#endif
//*********************************************************************************//
//                                  时钟配置                                       //
//*********************************************************************************//
#define TCFG_CLOCK_SYS_SRC					SYS_CLOCK_INPUT_PLL_BT_OSC   //系统时钟源选择
//#define TCFG_CLOCK_SYS_HZ					12000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					16000000                     //系统时钟设置
#define TCFG_CLOCK_SYS_HZ					24000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					32000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					48000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					54000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					64000000                     //系统时钟设置
#define TCFG_CLOCK_OSC_HZ					24000000                     //外界晶振频率设置
#define TCFG_CLOCK_MODE                     CLOCK_MODE_ADAPTIVE

//*********************************************************************************//
//                                  电源配置                                       //
//*********************************************************************************//
#define TCFG_LOWPOWER_POWER_SEL				PWR_DCDC15        	//电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                   //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL			1  				    //低功耗使能，蓝牙&&系统空闲可进入低功耗
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_28V		//vddiom等级
#define TCFG_LOWPOWER_OSC_TYPE              OSC_TYPE_LRC		//低功耗晶振类型，btosc/lrc
#if (TCFG_AUDIO_ASR_DEVELOP && TCFG_CVP_DEVELOP_ENABLE)
#define TCFG_LOWPOWER_RAM_SIZE				0	                // 低功耗掉电ram大小，单位：128K，可设置值：0、2、3
#elif (TCFG_SMART_VOICE_ENABLE || TCFG_AUDIO_ASR_DEVELOP || TCFG_CVP_DEVELOP_ENABLE || TCFG_AUDIO_TRIPLE_MIC_ENABLE\
      || TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN || TCFG_ANC_SELF_DUT_GET_SZ || TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN || AUDIO_DUT_FRE_RESPONE_ENABLE)
#define TCFG_LOWPOWER_RAM_SIZE				2	                // 低功耗掉电ram大小，单位：128K，可设置值：0、2、3
#else
#define TCFG_LOWPOWER_RAM_SIZE				3	                // 低功耗掉电ram大小，单位：128K，可设置值：0、2、3
#endif

//*********************************************************************************//
//                                  EQ配置                                         //
//*********************************************************************************//
//EQ配置，使用在线EQ时，EQ文件和EQ模式无效。有EQ文件时，使能TCFG_USE_EQ_FILE,默认不用EQ模式切换功能
#define TCFG_EQ_ENABLE                            0     //支持EQ功能,EQ总使能
// #if TCFG_EQ_ENABLE
//支持在线EQ调试,如果使用蓝牙串口调试，需要打开宏 APP_ONLINE_DEBUG，否则，默认使用uart调试(二选一)
#define TCFG_EQ_ONLINE_ENABLE                     0
#define TCFG_BT_MUSIC_EQ_ENABLE                   0     //支持蓝牙音乐EQ
#define TCFG_PHONE_EQ_ENABLE                      0     //支持通话近端EQ
#define TCFG_AUDIO_OUT_EQ_ENABLE                  0     //mix out级，增加EQ高低音接口
#define TCFG_AEC_DCCS_EQ_ENABLE           		  0     // AEC DCCS
#define TCFG_AEC_UL_EQ_ENABLE           		  0     // AEC UL
#define TCFG_MUSIC_MODE_EQ_ENABLE                 0     //支持音乐模式EQ
#define TCFG_PC_MODE_EQ_ENABLE                    0     //支持pc模式EQ

#define TCFG_USE_EQ_FILE                          1    	//离线EQ使用配置文件还是默认系数表 1：使用文件  0 使用默认系数表
#if !TCFG_USE_EQ_FILE
#define TCFG_USER_EQ_MODE_NUM                     7    	//EQ默认系数表的模式个数，默认是7个
#endif

#define EQ_SECTION_MAX                            10   	//EQ段数，最大20
#define TCFG_CALL_DL_EQ_SECTION                   3 //通话下行EQ段数,小于等于EQ_SECTION_MAX ，最大10段
#define TCFG_CALL_UL_EQ_SECTION                   3  //通话上行EQ段数,小于等于EQ_SECTION_MAX ,最大10段

// #endif//TCFG_EQ_ENABLE

/*省电容mic通过eq模块实现去直流滤波*/
#if (TCFG_SUPPORT_MIC_CAPLESS && (TCFG_MIC_CAPLESS_ENABLE || TCFG_MIC1_CAPLESS_ENABLE))
#if ((TCFG_EQ_ENABLE == 0) || (TCFG_AEC_DCCS_EQ_ENABLE == 0))
#error "MicCapless enable,Please enable TCFG_EQ_ENABLE and TCFG_AEC_DCCS_EQ_ENABLE"
#endif
#endif

#if TCFG_AUDIO_HEARING_AID_ENABLE
#define TCFG_DRC_ENABLE							  1	  /*DRC 总使能*/
#else
#define TCFG_DRC_ENABLE							  0	  	//DRC 总使能
#endif /*TCFG_AUDIO_HEARING_AID_ENABLE*/
// #if TCFG_DRC_ENABLE
#define TCFG_BT_MUSIC_DRC_ENABLE            	  0     //支持蓝牙音乐DRC
#define TCFG_MUSIC_MODE_DRC_ENABLE                0     //支持音乐模式DRC
#define TCFG_PC_MODE_DRC_ENABLE                   0     //支持PC模式DRC
#define TCFG_CVP_UL_DRC_ENABLE           		  0     //支持通话上行DRC
// #endif

//*********************************************************************************//
//                          新配置工具 && 调音工具                             //
//*********************************************************************************//
#define TCFG_CFG_TOOL_ENABLE				DISABLE		  	//是否支持在线配置工具
#define TCFG_EFFECT_TOOL_ENABLE				DISABLE		  	//是否支持在线音效调试,使能该项还需使能EQ总使能TCFG_EQ_ENABL,
#define TCFG_NULL_COMM						0				//不支持通信
#define TCFG_UART_COMM						1				//串口通信
#define TCFG_USB_COMM						2				//USB通信
#define TCFG_SPP_COMM						3				//蓝牙SPP通信
#if (TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE)
#define TCFG_COMM_TYPE						TCFG_USB_COMM	//通信方式选择
#else
#define TCFG_COMM_TYPE						TCFG_NULL_COMM
#endif
#define TCFG_ONLINE_TX_PORT					IO_PORT_DP      //UART模式调试TX口选择
#define TCFG_ONLINE_RX_PORT					IO_PORT_DM      //UART模式调试RX口选择
#define TCFG_ONLINE_ENABLE                  (TCFG_EFFECT_TOOL_ENABLE)    //是否支持音效在线调试功能

/***********************************非用户配置区***********************************/
#include "usb_std_class_def.h"
#if (TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE)
#if (TCFG_COMM_TYPE == TCFG_USB_COMM)
#define TCFG_USB_CDC_BACKGROUND_RUN         ENABLE
#endif
#if (TCFG_COMM_TYPE == TCFG_UART_COMM)
#define TCFG_USB_CDC_BACKGROUND_RUN         DISABLE
#endif
#endif
/**********************************************************************************/

/***********************************非用户配置区***********************************/
#if TCFG_EQ_ONLINE_ENABLE
#if (TCFG_USE_EQ_FILE == 0)
#undef TCFG_USE_EQ_FILE
#define TCFG_USE_EQ_FILE                    1    //开在线调试时，打开使用离线配置文件宏定义
#endif
#if TCFG_AUDIO_OUT_EQ_ENABLE
#undef TCFG_AUDIO_OUT_EQ_ENABLE
#define TCFG_AUDIO_OUT_EQ_ENABLE            0    //开在线调试时，关闭高低音
#endif
#endif

/**********************************************************************************/

//*********************************************************************************//
//                                  g-sensor配置                                   //
//*********************************************************************************//
#define TCFG_GSENSOR_ENABLE                       0     //gSensor使能
#define TCFG_DA230_EN                             0
#define TCFG_SC7A20_EN                            0
#define TCFG_STK8321_EN                           0
#define TCFG_IRSENSOR_ENABLE                      0
#define TCFG_JSA1221_ENABLE                       0
#define TCFG_GSENOR_USER_IIC_TYPE                 0     //0:软件IIC  1:硬件IIC

//*********************************************************************************//
//                                  imu-sensor配置                                   //
//*********************************************************************************//
#define TCFG_IMUSENSOR_ENABLE                	1    //imu Sensor使能
//mpu6887 cfg
#define TCFG_MPU6887P_ENABLE                  	0
#define TCFG_MPU6887P_INTERFACE_TYPE          	0 //0:iic, 1:spi
#define TCFG_MPU6887P_USER_IIC_TYPE           	0 //iic有效:1:硬件iic, 0:软件iic
#define TCFG_MPU6887P_USER_IIC_INDEX          	0 //IIC 序号
#define TCFG_MPU6887P_DETECT_IO               	(-1) //传感器中断io
#define TCFG_MPU6887P_AD0_SELETE_IO             IO_PORTC_03 //iic地址选择io
//mpu9250 cfg
#define TCFG_TP_MPU9250_ENABLE                	0
#define TCFG_MPU9250_INTERFACE_TYPE           	0 //不支持.0:iic, 1:spi
#define TCFG_MPU9250_USER_IIC_TYPE            	0 //iic有效:1:硬件iic, 0:软件iic
#define TCFG_MPU9250_USER_IIC_INDEX           	0 //IIC 序号
#define TCFG_MPU9250_DETECT_IO              	IO_PORTB_03 //传感器中断io
//sh3001 cfg
#define TCFG_SH3001_ENABLE                    	0
#define TCFG_SH3001_INTERFACE_TYPE            	0 //0:iic, 1:spi
#define TCFG_SH3001_USER_IIC_TYPE		      	0 //1:硬件iic, 0:软件iic
#define TCFG_SH3001_USER_IIC_INDEX            	0 //IIC 序号
#define TCFG_SH3001_DETECT_IO                 	IO_PORTB_03 //传感器中断io
//qmi8658 cfg
#define TCFG_QMI8658_ENABLE                     1
#define TCFG_QMI8658_INTERFACE_TYPE             0 //0:iic, 1:spi, 2:i3c
#define TCFG_QMI8658_USER_IIC_TYPE              0 //1:硬件iic, 0:软件iic
#define TCFG_QMI8658_USER_IIC_INDEX          	0 //IIC 序号
#define TCFG_QMI8658_DETECT_IO               	(-1) //传感器中断io
#define TCFG_QMI8658_AD0_SELETE_IO              (-1) //iic地址选择io
//lsm6dsl cfg
#define TCFG_LSM6DSL_ENABLE                     0
#define TCFG_LSM6DSL_INTERFACE_TYPE             0 //0:iic, 1:spi
#define TCFG_LSM6DSL_USER_IIC_TYPE              0 //1:硬件iic, 0:软件iic
#define TCFG_LSM6DSL_USER_IIC_INDEX          	0 //IIC 序号
#define TCFG_LSM6DSL_DETECT_IO               	(-1) //传感器中断io
//mpu6050 cfg
#define TCFG_MPU6050_EN                     	0
//qmc5883 cfg

/*
 *imu-sensor power manager
 *不用独立IO供电，则配置 NO_CONFIG_PORT
 */
#define TCFG_IMU_SENSOR_PWR_PORT				IO_PORTG_05


/*空间音效和传感器的依赖*/
#if (TCFG_AUDIO_SPATIAL_EFFECT_ENABLE == 0)
#undef TCFG_IMUSENSOR_ENABLE
#undef TCFG_MPU6887P_ENABLE
#undef TCFG_QMI8658_ENABLE
#undef TCFG_LSM6DSL_ENABLE
#undef TCFG_TP_MPU9250_ENABLE
#undef TCFG_SH3001_ENABLE
#undef TCFG_MPU6050_EN
#define TCFG_IMUSENSOR_ENABLE              		0
#define TCFG_MPU6887P_ENABLE                  	0
#define TCFG_QMI8658_ENABLE                     0
#define TCFG_LSM6DSL_ENABLE                     0
#define TCFG_TP_MPU9250_ENABLE                	0
#define TCFG_SH3001_ENABLE                    	0
#define TCFG_MPU6050_EN                     	0
#endif/*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

//*********************************************************************************//
//                                  系统配置                                         //
//*********************************************************************************//
#define TCFG_AUTO_SHUT_DOWN_TIME		          180   //没有蓝牙连接自动关机时间
#define TCFG_SYS_LVD_EN						      1     //电量检测使能
#define TCFG_POWER_ON_NEED_KEY				      0	    //是否需要按按键开机配置

//*********************************************************************************//
//                                  蓝牙配置                                       //
//*********************************************************************************//

/*头戴式关闭TWS*/
#if CLIENT_BOARD == CUSTOM5_CFG || \
    CLIENT_BOARD == CUSTOM11_CFG || \
    CLIENT_BOARD == CUSTOM9_CFG
#define TCFG_USER_TWS_ENABLE                      0   //tws功能使能
#else
#define TCFG_USER_TWS_ENABLE                      1   //tws功能使能
#endif

#define TCFG_USER_BLE_ENABLE                      0   //BLE功能使能
#define TCFG_BT_SUPPORT_AAC                       1   //AAC格式支持
#define TCFG_BT_SUPPORT_LDAC                      0   //LDAC格式支持

#if TCFG_BT_SUPPORT_LDAC
#define CONFIG_BT_RX_BUFF_SIZE  (46 * 1024)
#else
#define CONFIG_BT_RX_BUFF_SIZE  (22 * 1024)
#endif

#define USER_SUPPORT_PROFILE_SPP    0
#define USER_SUPPORT_PROFILE_HFP    1
#define USER_SUPPORT_PROFILE_A2DP   1
#define USER_SUPPORT_PROFILE_AVCTP  1
#define USER_SUPPORT_PROFILE_HID    1
#define USER_SUPPORT_PROFILE_PNP    1
#define USER_SUPPORT_PROFILE_PBAP   0

#if TCFG_USER_TWS_ENABLE
#define TCFG_BD_NUM						          1   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME             0   //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#else
#define TCFG_BD_NUM						          2   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME             0 //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#endif

#define BT_INBAND_RINGTONE                        1   //是否播放手机自带来电铃声
#define BT_PHONE_NUMBER                           0   //是否播放来电报号
#define BT_SUPPORT_DISPLAY_BAT                    1   //是否使能电量检测
#define BT_SUPPORT_MUSIC_VOL_SYNC                 1   //是否使能音量同步

/*spp数据导出配置*/
#if ((TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP) || TCFG_AUDIO_MIC_DUT_ENABLE)
#undef TCFG_USER_TWS_ENABLE
#undef TCFG_USER_BLE_ENABLE
#undef TCFG_BD_NUM
#undef USER_SUPPORT_PROFILE_SPP
#undef USER_SUPPORT_PROFILE_A2DP
#define TCFG_USER_TWS_ENABLE        0//spp数据导出，关闭tws
#define TCFG_USER_BLE_ENABLE        0//spp数据导出，关闭ble
#define TCFG_BD_NUM					1//连接设备个数配置
#define USER_SUPPORT_PROFILE_SPP	1
#define USER_SUPPORT_PROFILE_A2DP   0
#define APP_ONLINE_DEBUG            1//通过spp导出数据
#else
#define APP_ONLINE_DEBUG            0//在线APP调试,发布默认不开
#endif/*TCFG_AUDIO_DATA_EXPORT_ENABLE*/

/*以下功能需要打开SPP和在线调试功能
 *1、回音消除参数在线调试
 *2、通话CVP_DUT 产测模式
 *3、ANC工具蓝牙spp调试
 *4、配置/调音工具spp调试
 *5、空间音频在线工具spp调试
 */
#if (TCFG_AEC_TOOL_ONLINE_ENABLE || TCFG_AUDIO_CVP_DUT_ENABLE || TCFG_ANC_TOOL_DEBUG_ONLINE \
     || (TCFG_SENSOR_DATA_EXPORT_ENABLE == SENSOR_DATA_EXPORT_USE_SPP) \
     || TCFG_SPATIAL_EFFECT_ONLINE_ENABLE )||(TCFG_COMM_TYPE == TCFG_SPP_COMM) \
     || (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE)
#undef USER_SUPPORT_PROFILE_SPP
#undef APP_ONLINE_DEBUG
#define USER_SUPPORT_PROFILE_SPP	1
#define APP_ONLINE_DEBUG            1
#endif/*TCFG_AEC_TOOL_ONLINE_ENABLE*/

#if ((TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE) && (TCFG_COMM_TYPE == TCFG_SPP_COMM))
#if ((!USER_SUPPORT_PROFILE_SPP) || (!APP_ONLINE_DEBUG))
#error "Please enable USER_SUPPORT_PROFILE_SPP and APP_ONLINE_DEBUG ! !"
#endif
#endif

//*********************************************************************************//
//                           编解码格式配置(CodecFormat)                           //
//*********************************************************************************//
/*解码格式使能*/
#define TCFG_DEC_MP3_ENABLE                 DISABLE
#define TCFG_DEC_WTGV2_ENABLE				ENABLE
#define TCFG_DEC_G729_ENABLE                DISABLE
#define TCFG_DEC_WMA_ENABLE					DISABLE
#define TCFG_DEC_WAV_ENABLE					DISABLE
#define TCFG_DEC_FLAC_ENABLE				DISABLE
#define TCFG_DEC_APE_ENABLE					DISABLE
#define TCFG_DEC_M4A_ENABLE					DISABLE
#define TCFG_DEC_ALAC_ENABLE				DISABLE
#define TCFG_DEC_AMR_ENABLE					DISABLE
#define TCFG_DEC_DTS_ENABLE					DISABLE
#define TCFG_DEC_G726_ENABLE			    DISABLE
#define TCFG_DEC_MIDI_ENABLE			    DISABLE
#define TCFG_DEC_MTY_ENABLE					DISABLE
#define TCFG_DEC_SBC_ENABLE					ENABLE
#define TCFG_DEC_PCM_ENABLE					DISABLE
#define TCFG_DEC_CVSD_ENABLE				DISABLE
#define TCFG_DEC_LC3_ENABLE                 DISABLE
#define TCFG_DEC_AAC_ENABLE					DISABLE
#define TCFG_DEC_LDAC_ENABLE				DISABLE

/*编码格式使能*/
#define TCFG_ENC_MSBC_ENABLE				ENABLE
#define TCFG_ENC_CVSD_ENABLE				ENABLE

/*提示音叠加配置*/
#define TCFG_WAV_TONE_MIX_ENABLE			DISABLE
#define TCFG_MP3_TONE_MIX_ENABLE			DISABLE
#define TCFG_AAC_TONE_MIX_ENABLE			DISABLE//叠加AAC解码需要使用malloc
#define TCFG_WTG_TONE_MIX_ENABLE			ENABLE
#define TCFG_WTS_TONE_MIX_ENABLE			ENABLE

//*********************************************************************************//
//                                  fm emitter 配置 (不支持)                                    //
//*********************************************************************************//
#define TCFG_APP_FM_EMITTER_EN              DISABLE_THIS_MOUDLE

//*********************************************************************************//
//                                  AI配置                                       //
//*********************************************************************************//
#if TCFG_USER_BLE_ENABLE
#define    DUEROS_DMA_EN             0
#define    TRANS_DATA_EN             0
#define    BLE_HID_EN                0

#if (DUEROS_DMA_EN)
#define    BT_MIC_EN             1
#else
#define    BT_MIC_EN             0
#endif

#if (DUEROS_DMA_EN || TRANS_DATA_EN || TUYA_DEMO_EN || LL_SYNC_EN)
#define    BT_FOR_APP_EN             1
#else
#define    BT_FOR_APP_EN             0
#endif

#if(DUEROS_DMA_EN)
#define USE_DMA_TONE  0  //使用DMA提示音
#else
#define USE_DMA_TONE  0
#endif

#if (DUEROS_DMA_EN && TRANS_DATA_EN)
#error "they can not enable at the same time!"
#endif
#endif

//*********************************************************************************//
//                                 环绕音效使能
//*********************************************************************************//
#define AUDIO_SURROUND_CONFIG     0//3d环绕

#define AUDIO_VBASS_CONFIG        0//虚拟低音,虚拟低音不支持四声道

#define AEC_PITCHSHIFTER_CONFIG   0   //通话上行变声,左耳触发变大叔声，右耳触发变女声

#define AEC_PRESETS_UL_EQ_CONFIG  0 //通话上行预设eq系数表使能，使能后不使用效果文件的系数表

#define AUDIO_SPK_EQ_CONFIG       0//SPK_EQ 使能
#if AUDIO_SPK_EQ_CONFIG
#if (EQ_SECTION_MAX < 10)
#undef EQ_SECTION_MAX
#define EQ_SECTION_MAX 10
#endif
#if !APP_ONLINE_DEBUG
#undef  APP_ONLINE_DEBUG
#define APP_ONLINE_DEBUG 1
#endif
#if (!TCFG_EQ_ENABLE)
#error "需使能总eq宏TCFG_EQ_ENABLE"
#endif
#endif/*AUDIO_SPK_EQ_CONFIG*/



//*********************************************************************************//
//                                 电源切换配置                                    //
//*********************************************************************************//

#define CONFIG_PHONE_CALL_USE_LDO15	    0


//*********************************************************************************//
//                                 时钟切换配置                                    //
//*********************************************************************************//

#define CONFIG_BT_NORMAL_HZ	            (24 * 1000000L)
#define CONFIG_BT_CONNECT_HZ            (48 * 1000000L)

#if TCFG_BT_MUSIC_EQ_ENABLE
#define CONFIG_BT_A2DP_HZ	        	(96 * 1000000L)
#else
#define CONFIG_BT_A2DP_HZ	        	(48 * 1000000L)
#endif

#define CONFIG_MUSIC_DEC_CLOCK			(48 * 1000000L)
#define CONFIG_MUSIC_IDLE_CLOCK		    (48 * 1000000L)

#define CONFIG_BT_CALL_HZ		        (64 * 1000000L)
#define CONFIG_BT_CALL_ADVANCE_HZ       (64 * 1000000L)
#define CONFIG_BT_CALL_16k_HZ	        (80 * 1000000L)
#define CONFIG_BT_CALL_16k_ADVANCE_HZ   (96 * 1000000L)
#define CONFIG_BT_CALL_DNS_HZ           (96 * 1000000L)

#if (TCFG_EQ_ONLINE_ENABLE && (!TCFG_EFFECT_TOOL_ENABLE) || (!TCFG_EQ_ONLINE_ENABLE && (TCFG_EFFECT_TOOL_ENABLE)))
#error "音效在线调试需使能调音工具宏TCFG_EFFECT_TOOL_ENABLE"
#endif

#if TCFG_AUDIO_OUTPUT_IIS && (SYS_VOL_TYPE != VOL_TYPE_DIGITAL)
#error "iis输出需要使用数字音量"
#endif

//*********************************************************************************//
//                                 配置结束                                        //
//*********************************************************************************//


#endif //CONFIG_BOARD_JL701N_ANC
#endif //CONFIG_BOARD_JL701N_ANC_CFG_H
