#ifndef AUDIO_ADC_H
#define AUDIO_ADC_H

#include "generic/typedef.h"
#include "generic/list.h"
#include "generic/atomic.h"
#include "system/spinlock.h"

/*无电容电路*/
#define SUPPORT_MIC_CAPLESS          1

#define BR22_ADC_BIT_FLAG_FADE_ON	BIT(31)

#define FADE_OUT_IN           	1
#define FADE_OUT_TIME_MS		2

#define LADC_STATE_INIT			1
#define LADC_STATE_OPEN      	2
#define LADC_STATE_START     	3
#define LADC_STATE_STOP      	4

#define FPGA_BOARD          0

#define LADC_MIC                0
#define LADC_LINEIN             1

#define SOURCE_MONO_LEFT        0
#define SOURCE_MONO_RIGHT       1
#define SOURCE_MONO_LEFT_RIGHT  2

#define ADC_DEFAULT_PNS         128

/* 通道选择 */
#define AUDIO_ADC_MIC_0					    BIT(0)  // PA1
#define AUDIO_ADC_MIC_1					    BIT(1)  // PA3
#define AUDIO_ADC_MIC_2					    BIT(2)  // PG8
#define AUDIO_ADC_MIC_3					    BIT(3)  // PG6
#define AUDIO_ADC_LINE0 					BIT(0)  // PA1
#define AUDIO_ADC_LINE1  					BIT(1)  // PA3
#define AUDIO_ADC_LINE2  					BIT(2)  // PG8
#define AUDIO_ADC_LINE3  					BIT(3)  // PG6
#define PLNK_MIC		            		BIT(6)	//PDM MIC
#define ALNK_MIC				            BIT(7)	//IIS MIC


/*mic_mode 工作模式定义*/
#define AUDIO_MIC_CAP_MODE          0   //单端隔直电容模式
#define AUDIO_MIC_CAP_DIFF_MODE     1   //差分隔直电容模式
#define AUDIO_MIC_CAPLESS_MODE      2   //单端省电容模式

struct adc_platform_data {
    u8 mic_ldo_vsel : 3;//0:1.3v 1:1.4v 2:1.5v 3:2.0v 4:2.2v 5:2.4v 6:2.6v 7:2.8v
    u8 mic_ldo_isel : 2; //MIC通道电流档位选择
    u8 reserved: 3;
    u8 mic_capless;  //MIC免电容方案
    //MIC免电容方案和mic bias供电方案需要设置，影响MIC的偏置电压 1:16K 2:7.5K 3:5.1K 4:6.8K 5:4.7K 6:3.5K 7:2.9K  8:3K  9:2.5K 10:2.1K 11:1.9K  12:2K  13:1.8K 14:1.6K  15:1.5K 16:1K 31:0.6K
    u8 mic_bias_res;
    u8 mic1_bias_res;
    u8 mic2_bias_res;
    u8 mic3_bias_res;

    u32 mic_mode : 3;            //MIC0工作模式
    u32 mic1_mode : 3;           //MIC1工作模式
    u32 mic2_mode : 3;           //MIC2工作模式
    u32 mic3_mode : 3;           //MIC3工作模式
    u32 mic_bias_inside : 1;     //MIC0电容隔直模式使用内部mic偏置  PA2
    u32 mic1_bias_inside : 1;    //MIC1电容隔直模式使用内部mic偏置  PA4
    u32 mic2_bias_inside : 1;    //MIC2电容隔直模式使用内部mic偏置  PG7
    u32 mic3_bias_inside : 1;    //MIC3电容隔直模式使用内部mic偏置  PG5
    u32 mic_ldo_pwr : 1;         // MICLDO供电到PAD(PA0)控制使能
    u32 lowpower_lvl : 2;		 //ADC低功耗等级

    u8 mic_ldo_state;
    u8 mic_dcc;                  //mic的去直流dcc寄存器配置值,可配0到15,数值越大,其高通转折点越低
    u8 anc_adt_en;
    u8 anc_adt_mic_ch_num;
    u8 adc_max_ch_en;
    u8 max_channel_num;
};

/*
 * p11系统VCO ADC mic模拟配置数据
 * 仅支持mic0，故应和主系统mic0对应
 *
 */
struct vco_adc_platform_data {
    u8 mic_mode;
    u8 mic_ldo_vsel;
    u8 mic_ldo_isel;
    u8 mic_bias_en;
    u8 mic_bias_res;
    u8 mic_bias_inside;
    u8 mic_ldo2PAD_en;
    u8 adc_rbs;
    u8 adc_rin;
    u8 adc_test;        //VCO ADC测试使能，该配置用作ADC测试，届时AVAD、DVAD算法将无效
};

/*
 * p11 VAD电源驱动参数和模拟电源域参数配置
 * 先放在adc模拟部分管理
 */
struct vad_power_data {
    u8 ldo_vs;
    u8 ldo_is;
    u8 lowpower;
    u8 clock;
    u8 clock_x2ds_disable;
    u8 acm_select;
};

struct vad_mic_platform_data {
    struct vco_adc_platform_data mic_data;
    struct vad_power_data power_data;
};

struct capless_low_pass {
    u16 bud; //快调边界
    u16 count;
    u16 pass_num;
    u16 tbidx;
};

struct audio_adc_attr {
    u8 gain;
    u16 sample_rate;
    u16 irq_time;
    u16 irq_points;
};

struct adc_stream_ops {
    void (*buf_reset)(void *priv);
    void *(*alloc_space)(void *priv, u32 *len);
    void (*output)(void *priv, void *buf, u32 len);
};

struct audio_adc_output_hdl {
    struct list_head entry;
    void *priv;
    void (*handler)(void *, s16 *, int);
};

struct audio_adc_hdl {
    struct list_head head;
    /*void *priv;
    const struct adc_stream_ops *ops;*/
    struct adc_platform_data *pd;
    spinlock_t lock;
    atomic_t ref;
    struct audio_adc_attr attr;
    u16 pause_srp;
    u16 pns;
    u8 fifo_full;
    u8 channel;
    u8 channel_num;
    u8 ch_sel;
    u8 input;
    u8 state;

    union {
        struct {
            u8 out2dac : 1;
            u8 out_l : 1;
            u8 out_r : 1;
            u8 out_mix : 1;
            u8 cha_idx : 2;
        } ladc;
        u8 param;
    } u;
#if FADE_OUT_IN
    u8 fade_on;
    volatile u8 ref_gain;
    volatile int fade_timer;
#endif
#if SUPPORT_MIC_CAPLESS
    struct capless_low_pass lp;
#endif
};


struct adc_mic_ch {
    struct audio_adc_hdl *adc;
    u8 gain;
    u8 gain1;
    u8 gain2;
    u8 gain3;
    u8 buf_num;
    u16 buf_size;
    s16 *bufs;
    u16 sample_rate;
    void (*handler)(struct adc_mic_ch *, s16 *, u16);
};

struct adc_linein_ch {
    struct audio_adc_hdl *adc;
    u8 gain;
    u8 gain1;
    u8 gain2;
    u8 gain3;
    u8 buf_num;
    u16 buf_size;
    s16 *bufs;
    u16 sample_rate;
    void (*handler)(struct adc_mic_ch *, s16 *, u16);
};
//MIC相关管理结构
typedef struct {
    u8 en;				//MIC使能
    u8 type;			//FF_MIC使能
    u8 gain;			//MIC增益
    u8 mult_flag;		//通话复用标志
} audio_adc_mic_mana_t;

void audio_adc_init(struct audio_adc_hdl *, struct adc_platform_data *);

void audio_adc_add_output_handler(struct audio_adc_hdl *, struct audio_adc_output_hdl *);

void audio_adc_del_output_handler(struct audio_adc_hdl *, struct audio_adc_output_hdl *);

void audio_adc_irq_handler(struct audio_adc_hdl *adc);

/*
*********************************************************************
*                  Audio ADC Mic Open
* Description: 打开mic采样通道
* Arguments  : mic	mic操作句柄
*			   ch	mic通道索引
*			   adc  adc模块操作句柄
* Return	 : 0 成功	其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_adc_mic_open(struct adc_mic_ch *mic, int ch, struct audio_adc_hdl *adc);
int audio_adc_mic1_open(struct adc_mic_ch *mic, int ch, struct audio_adc_hdl *adc);
int audio_adc_mic2_open(struct adc_mic_ch *mic, int ch, struct audio_adc_hdl *adc);
int audio_adc_mic3_open(struct adc_mic_ch *mic, int ch, struct audio_adc_hdl *adc);

/*
*********************************************************************
*                  Audio ADC Mic Gain
* Description: 设置mic增益
* Arguments  : mic	mic操作句柄
*			   gain	mic增益
* Return	 : 0 成功	其他 失败
* Note(s)    : MIC增益范围：0(-8dB)~19(30dB),step:2dB,level(4)=0dB
*********************************************************************
*/
int audio_adc_mic_set_gain(struct adc_mic_ch *mic, int gain);
int audio_adc_mic1_set_gain(struct adc_mic_ch *mic, int gain);
int audio_adc_mic2_set_gain(struct adc_mic_ch *mic, int gain);
int audio_adc_mic3_set_gain(struct adc_mic_ch *mic, int gain);

/*
*********************************************************************
*                  Audio ADC Mic Pre_Gain
* Description: 设置mic第一级/前级增益
* Arguments  : en 前级增益使能(0:6dB 1:0dB)
* Return	 : None.
* Note(s)    : 前级增益只有0dB和6dB两个档位
*********************************************************************
*/
void audio_adc_mic_0dB_en(bool en);
void audio_adc_mic1_0dB_en(bool en);
void audio_adc_mic2_0dB_en(bool en);
void audio_adc_mic3_0dB_en(bool en);

/*
*********************************************************************
*                  Audio ADC linein Open
* Description: 打开linein采样通道
* Arguments  : linein	linein操作句柄
*			   ch	linein通道索引
*			   adc  adc模块操作句柄
* Return	 : 0 成功	其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_adc_linein_open(struct adc_linein_ch *linein, int ch, struct audio_adc_hdl *adc);
int audio_adc_linein1_open(struct adc_linein_ch *linein, int ch, struct audio_adc_hdl *adc);
int audio_adc_linein2_open(struct adc_linein_ch *linein, int ch, struct audio_adc_hdl *adc);
int audio_adc_linein3_open(struct adc_linein_ch *linein, int ch, struct audio_adc_hdl *adc);

/*
*********************************************************************
*                  Audio ADC linein Gain
* Description: 设置linein增益
* Arguments  : linein	linein操作句柄
*			   gain	linein增益
* Return	 : 0 成功	其他 失败
* Note(s)    : linein增益范围：0(-8dB)~19(30dB),step:2dB,level(4)=0dB
*********************************************************************
*/
int audio_adc_linein_set_gain(struct adc_linein_ch *linein, int gain);
int audio_adc_linein1_set_gain(struct adc_linein_ch *linein, int gain);
int audio_adc_linein2_set_gain(struct adc_linein_ch *linein, int gain);
int audio_adc_linein3_set_gain(struct adc_linein_ch *linein, int gain);

/*
*********************************************************************
*                  Audio ADC linein Pre_Gain
* Description: 设置linein第一级/前级增益
* Arguments  : en 前级增益使能(0:6dB 1:0dB)
* Return	 : None.
* Note(s)    : 前级增益只有0dB和6dB两个档位
*********************************************************************
*/
void audio_adc_linein_0dB_en(bool en);
void audio_adc_linein1_0dB_en(bool en);
void audio_adc_linein2_0dB_en(bool en);
void audio_adc_linein3_0dB_en(bool en);


/*
*********************************************************************
*                  AUDIO MIC_LDO Control
* Description: mic电源mic_ldo控制接口
* Arguments  : index	ldo索引(MIC_LDO/MIC_LDO_BIAS0/MIC_LDO_BIAS1)
* 			   en		使能控制
*			   pd		audio_adc模块配置
* Return	 : 0 成功 其他 失败
* Note(s)    : (1)MIC_LDO输出不经过上拉电阻分压
*				  MIC_LDO_BIAS输出经过上拉电阻分压
*			   (2)打开一个mic_ldo示例：
*				audio_mic_ldo_en(MIC_LDO,1,&adc_data);
*			   (2)打开多个mic_ldo示例：
*				audio_mic_ldo_en(MIC_LDO | MIC_LDO_BIAS,1,&adc_data);
*********************************************************************
*/
/*MIC LDO index输出定义*/
#define MIC_LDO					BIT(0)	//PA0输出原始MIC_LDO
#define MIC_LDO_BIAS0			BIT(1)	//PA2输出经过内部上拉电阻分压的偏置
#define MIC_LDO_BIAS1			BIT(2)	//PA4输出经过内部上拉电阻分压的偏置
#define MIC_LDO_BIAS2			BIT(3)	//PG7输出经过内部上拉电阻分压的偏置
#define MIC_LDO_BIAS3			BIT(4)	//PG5输出经过内部上拉电阻分压的偏置
int audio_adc_mic_ldo_en(u8 index, u8 en, struct adc_platform_data *pd);

int audio_adc_mic_set_sample_rate(struct adc_mic_ch *mic, int sample_rate);

int audio_adc_mic_set_buffs(struct adc_mic_ch *mic, s16 *bufs, u16 buf_size, u8 buf_num);

int audio_adc_mic_set_output_handler(struct adc_mic_ch *mic,
                                     void (*handler)(struct adc_mic_ch *, s16 *, u16));

int audio_adc_mic_start(struct adc_mic_ch *mic);

int audio_adc_mic_close(struct adc_mic_ch *mic);

int audio_adc_linein_start(struct adc_linein_ch *linein);

int audio_adc_linein_close(struct adc_linein_ch *linein);

void audio_anc_mic_gain(audio_adc_mic_mana_t *mic_param, u8 mult_gain_en);

void audio_anc_mic_open(audio_adc_mic_mana_t *mic_param, u8 trans_en, u8 adc_ch);

void audio_anc_mic_close(audio_adc_mic_mana_t *mic_param);

int audio_adc_linein_set_sample_rate(struct adc_linein_ch *linein, int sample_rate);

int audio_adc_linein_set_buffs(struct adc_linein_ch *linein, s16 *bufs, u16 buf_size, u8 buf_num);
#endif
