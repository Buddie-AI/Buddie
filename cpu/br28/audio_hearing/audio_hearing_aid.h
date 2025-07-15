#ifndef _AUD_HEARING_AID_H_
#define _AUD_HEARING_AID_H_

#include "generic/typedef.h"
#include "board_config.h"


/*************************************************************************
 *						可选功能模块配置(Optinal Module Define)
 ************************************************************************/
#define DHA_DATA_EXPORT_ENABLE			0	//数据导出使能, 需要先开APP_PCM_DEBUG
#define DHA_RUN_TIME_TRACE_ENABLE		0	//运行时间log跟踪
#define DHA_RUN_TIME_IO_DEBUG_ENABLE	0	//运行时间IO跟踪
#ifdef DHA_RUN_TIME_IO_DEBUG_ENABLE
#define IO_IDX	7
#define IO_INTERVAL_IDX	4		/*DHA节奏，唤醒间隔*/
#define DHA_IO_DEBUG_INIT() 	JL_PORTA->DIR &= ~BIT(IO_IDX)
#define DHA_IO_DEBUG_1()  		JL_PORTA->OUT |= BIT(IO_IDX)
#define DHA_IO_DEBUG_0()  		JL_PORTA->OUT &= ~BIT(IO_IDX)
#define DHA_IO_INTERVAL()  		{JL_PORTA->DIR &= ~BIT(IO_INTERVAL_IDX);JL_PORTA->OUT ^= BIT(IO_INTERVAL_IDX);}
#else
#define DHA_IO_DEBUG_INIT(...)
#define DHA_IO_DEBUG_1(...)
#define DHA_IO_DEBUG_0(...)
#define DHA_IO_INTERVAL(...)
#endif/*DHA_RUN_TIME_IO_DEBUG_ENABLE*/

/*************************************************************************
 *						可选功能模块配置(Optinal Module Define)
 ************************************************************************/
#define DHA_AND_MEDIA_MUTEX_ENABLE		    1	//辅听功能和多媒体播放互斥
#define DHA_SRC_USE_HW_ENABLE               0   //SRC选择，1:使用硬件SRC，0:使用软件SRC
#define DHA_MIC_DATA_CBUF_ENABLE            0   //是否使用cbuf缓存mic的数据
#define DHA_DAC_OUTPUT_ENHANCE_ENABLE       0   //DAC输出音量增强使能：用来提高辅听的动态范围
#define DHA_TDE_ENABLE   				    0   //辅听信号延时估计
#define DHA_IN_LOUDNESS_TRACE_ENABLE		0	//跟踪获取当前mic输入幅值
#define DHA_OUT_LOUDNESS_TRACE_ENABLE		0	//跟踪获取算法输出幅值
#define DHA_USE_WDRC_ENABLE		            1	//DRC选择，1:使用WDRC，0:使用普通限幅器DRC

/*************************************************************************
 *					    验配功能定义(DHA Fitting Define)
 ************************************************************************/

/*听力验配版本号*/
#define DHA_FITTING_VERSION			0x01
/*验配通道数*/
#define DHA_FITTING_CHANNEL_MAX		6
/*通道频率*/
#define DHA_CH0_FREQ	250
#define DHA_CH1_FREQ	500
#define DHA_CH2_FREQ	1000
#define DHA_CH3_FREQ	2000
#define DHA_CH4_FREQ	4000
#define DHA_CH5_FREQ	6000

/*
 * 1. PayLoad Format
 * +---------+---------------+------+
 * |cmd[8bit]|data_len[16bit]| data |
 * +---------+---------------+------+
 * 2.DHA Fitting Commands
 */


/*(1)验配信息交互：版本、通道数、通道频率*/
#define DHA_FITTING_CMD_INFO		0x50
typedef struct {
    u8 version;								/*版本号：DHA_FITTING_VERSION*/
    u8 ch_num;								/*通道数：DHA_FITTING_CHANNEL_MAX*/
    u16 ch_freq[DHA_FITTING_CHANNEL_MAX];	/*通道频率组：获取通道对应的freq*/
} dha_fitting_info_t;

/*(2)通道验配*/
#define DHA_FITTING_CMD_ADJUST		0x51
typedef struct {
    u8 channel: 1;								/*左右声道标识:0=左声道 1=右声道*/
    u8 sine: 2;                                 /*开关验配单频音的标志: BIT(0) = 1 左耳播单频音，BIT(0) = 0 左耳静音
                                                                        BIT(1) = 1 右耳播单频音，BIT(1) = 0 右耳静音*/
    u8 reserve0: 5;                             /*保留*/
    u8 reserve1;						    	 /*保留*/
    u16 freq;								/*通道频率*/
    float gain;								/*通道增益*/
} dha_fitting_adjust_t;

/*(3)验配结果保存更新*/
/*
+---------+------------+--------------------+-------------+---------------+
|cmd[8bit]|len[16bit]  | type(8bit)左/右/双耳|N个左耳[float]| N个右耳[float]  |
+---------+------------+--------------------+-------------+---------------+
type(8bit) 0:左耳数据，1:右耳数据，2:双耳数据
*/
#define DHA_FITTING_CMD_UPDATE		0x52

/*(4)左右耳打开辅听的状态*/
#define DHA_FITTING_CMD_STATE       0x53
typedef struct {
    u8 state_left: 1;
    u8 state_right: 1;
    u8 reserve: 6;
} dha_fitting_state_t;

/*************************************************************************
 *						辅听耳机接口声明(DHA APIs Declaration)
 ************************************************************************/
int audio_hearing_aid_open(void);
int audio_hearing_aid_close(void);
void audio_hearing_aid_demo(void);
void audio_hearing_aid_resume(void);
void audio_hearing_aid_suspend(void);
int hearing_aid_fitting_parse(u8 *data, u16 len);
int hearing_aid_fitting_start(u8 en);
int get_hearing_aid_fitting_info(u8 *data);
u8 get_hearing_aid_state(void);
u8 get_hearing_aid_fitting_state(void);
u8 set_hearing_aid_fitting_state(u8 state);
void audio_dha_fitting_sync_close(void);
int get_hearing_aid_state_cmd_info(u8 *data);

/*************************************************************************
 *						其他引用(Other Reference)
 ************************************************************************/
extern int aec_uart_open(u8 nch, u16 single_size);
extern int aec_uart_fill(u8 ch, void *buf, u16 size);
extern void aec_uart_write(void);
extern int aec_uart_close(void);

#endif/*_AUD_HEARING_AID_H_*/
