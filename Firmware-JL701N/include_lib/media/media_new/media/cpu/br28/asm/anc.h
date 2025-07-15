#ifndef _ANC_H_
#define _ANC_H_

#include "generic/typedef.h"
#include "system/task.h"
#include "media/audio_def.h"
#include "asm/audio_adc.h"

enum {
    ANCDB_ERR_CRC = 1,
    ANCDB_ERR_TAG,
    ANCDB_ERR_PARAM,
    ANCDB_ERR_STATE,
};

typedef enum {
    ANC_OFF = 1,		/*关闭模式*/
    ANC_ON,				/*降噪模式*/
    ANC_TRANSPARENCY,	/*通透模式*/
    ANC_BYPASS,			/*训练模式*/
    ANC_TRAIN,			/*训练模式*/
    ANC_TRANS_TRAIN,	/*通透训练模式*/
} ANC_mode_t;

/*ANC状态*/
enum {
    ANC_STA_CLOSE = 0,
    ANC_STA_INIT,
    ANC_STA_OPEN,
};

typedef enum {
    TRANS_ERR_TIMEOUT = 1,	//训练超时
    TRANS_ERR_MICERR,		//训练中MIC异常
    TRANS_ERR_FORCE_EXIT	//强制退出训练
} ANC_trans_errmsg_t;

/*ANC MIC 类型*/
enum {
    ANC_MIC_TYPE_LFF = 0,
    ANC_MIC_TYPE_LFB,
    ANC_MIC_TYPE_RFF,
    ANC_MIC_TYPE_RFB,
};

typedef enum {
    //OLD
    ANC_L_FF_IIR  = 0x0,		//左FF滤波器ID
    ANC_L_FB_IIR  = 0x1,		//左FB滤波器ID
    ANC_L_CMP_IIR = 0x2,		//左音乐补偿滤波器ID
    ANC_L_TRANS_IIR = 0x3,		//左通透滤波器ID
    ANC_L_FIR = 0x4,			//(保留位，暂时不用)
    // ANC_L_TRANS2_IIR = 0x5,

    //NEW
    ANC_L_FF_FGQ  = 0x6,		//左FF滤波器FGQ
    ANC_L_FB_FGQ  = 0x7,		//左FB滤波器FGQ
    ANC_L_CMP_FGQ = 0x8,		//左音乐补偿滤波器FGQ
    ANC_L_TRANS_FGQ = 0x9,		//左通透滤波器FGQ
    ANC_L_FF_PARM  = 0xA,		//左FF参数ID
    ANC_L_FB_PARM  = 0xB,		//左FB参数ID
    ANC_L_CMP_PARM = 0xC,		//左音乐补偿参数ID
    ANC_L_TRANS_PARM = 0xD,		//左通透参数ID

    //OLD
    ANC_R_FF_IIR  = 0x10,		//右FF滤波器ID
    ANC_R_FB_IIR  = 0x11,       //右FB滤波器ID
    ANC_R_CMP_IIR = 0x12,       //右音乐补偿滤波器ID
    ANC_R_TRANS_IIR = 0x13,     //右通透滤波器ID
    ANC_R_FIR = 0x14,            //(保留位，暂时不用)
    // ANC_R_TRANS2_IIR = 0x15,

    //NEW
    ANC_R_FF_FGQ  = 0x16,		//右FF滤波器FGQ
    ANC_R_FB_FGQ  = 0x17,       //右FB滤波器FGQ
    ANC_R_CMP_FGQ = 0x18,       //右音乐补偿滤波器FGQ
    ANC_R_TRANS_FGQ = 0x19,     //右通透滤波器FGQ
    ANC_R_FF_PARM  = 0x1A,		//右FF参数ID
    ANC_R_FB_PARM  = 0x1B,       //右FB参数ID
    ANC_R_CMP_PARM = 0x1C,       //右音乐补偿参数ID
    ANC_R_TRANS_PARM = 0x1D,     //右通透参数ID

    //OTHER
    ANC_L_ADAP_FRE = 0x20,
    ANC_L_ADAP_PZ = 0x21,
    ANC_L_ADAP_SZPZ = 0x22,
    ANC_L_ADAP_TARGET = 0x23,
    ANC_L_ADAP_GOLD_CURVE = 0x24,//自适应金机曲线
    ANC_L_ADAP_TARGET_CMP = 0x25,
    ANC_R_ADAP_FRE = 0x30,
    ANC_R_ADAP_PZ = 0x31,
    ANC_R_ADAP_SZPZ = 0x32,
    ANC_R_ADAP_TARGET = 0x33,
    ANC_R_ADAP_GOLD_CURVE = 0x34,
    ANC_R_ADAP_TARGET_CMP = 0x35,
} ANC_config_seg_id_t;

//新增iir滤波器 type 枚举
typedef enum {
    ANC_IIR_HIGH_PASS = 0,
    ANC_IIR_LOW_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
    ANC_IIR_LOW_SHELF,
} ANC_iir_type;

typedef enum {
    ANC_ALOGM1  = 0,
    ANC_ALOGM2,
    ANC_ALOGM3,
    ANC_ALOGM4,
    ANC_ALOGM5,
    ANC_ALOGM6,
    ANC_ALOGM7,
    ANC_ALOGM8,
} ANC_alogm_t;		//ANC算法模式

typedef enum {
    A_MIC0 = 0x0,			//模拟MIC0 对应IO-PA1 PA2
    A_MIC1,                 //模拟MIC1 对应IO-PA3 PA4
    A_MIC2,                 //模拟MIC2 对应IO-PG7 PG8
    A_MIC3,                 //模拟MIC3 对应IO-PG5 PG6
    D_MIC0,                 //数字MIC0(plnk_dat0_pin-上升沿采样)
    D_MIC1,                 //数字MIC1(plnk_dat1_pin-上升沿采样)
    D_MIC2,                 //数字MIC2(plnk_dat0_pin-下降沿采样)
    D_MIC3,                 //数字MIC3(plnk_dat1_pin-下降沿采样)
    MIC_NULL = 0XFF,		//没有定义相关的MIC
} ANC_mic_type_t;

typedef enum {
    ANC_POW_SEL_R_DAC_REF =	0x02,	//功率输出R DAC+REFMIC
    ANC_POW_SEL_R_DAC_ERR,			//功率输出R DAC+ERRMIC
    ANC_POW_SEL_L_DAC_REF,			//功率输出L DAC+REFMIC
    ANC_POW_SEL_L_DAC_ERR,			//功率输出L DAC+ERRMIC
} ANC_pow_sel_t;

typedef enum {
    ANC_COEFF_MODE_NORMAL =	0x0,		//自适应普通模式
    ANC_COEFF_MODE_ADAPTIVE = 0x1,		//自适应滤波器模式
} ANC_coeff_mode_t;

//自适应解码任务打断状态
typedef enum {
    ANC_ADAPTIVE_DEC_WAIT_STOP = 0x0, //停止
    ANC_ADAPTIVE_DEC_WAIT_START,	  //启动
    ANC_ADAPTIVE_DEC_WAIT_RUN,		  //运行中
} ANC_adaptive_dec_wait_t;

//自适应滤波器数据状态
typedef enum {
    ANC_ADAPTIVE_DATA_EMPTY = 0x0, 	//数据为空
    ANC_ADAPTIVE_DATA_FROM_VM,	  	   	//来源VM
    ANC_ADAPTIVE_DATA_FROM_ALOGM,		//来源算法
} ANC_adaptive_data_from_t;

// ANC fade gain通道
typedef enum {
    AUDIO_ANC_FADE_CH_LFF = 0x1,
    AUDIO_ANC_FADE_CH_LFB = 0x2,
    AUDIO_ANC_FADE_CH_RFF = 0x4,
    AUDIO_ANC_FADE_CH_RFB = 0x8,
} ANC_fade_ch_t;

/*ANC模式使能位*/
#define ANC_OFF_BIT				BIT(1)	/*降噪关闭使能*/
#define ANC_ON_BIT				BIT(2)	/*降噪模式使能*/
#define ANC_TRANS_BIT			BIT(3)	/*通透模式使能*/

//ANC Ch ---eg:双声道 ANC_L_CH|ANC_R_CH
#define ANC_L_CH 					BIT(0)	//通道左
#define ANC_R_CH 	 				BIT(1)	//通道右

//ANC各类增益对应符号位
#define ANCL_FF_SIGN				BIT(0)	//左声道FF增益符号
#define ANCL_FB_SIGN				BIT(1)	//左声道FB增益符号
#define ANCL_TRANS_SIGN				BIT(2)  //左声道通透增益符号
#define ANCL_CMP_SIGN				BIT(3)  //左声道音乐补偿增益符号
#define ANCR_FF_SIGN				BIT(4)	//右声道FF增益符号
#define ANCR_FB_SIGN				BIT(5)	//右声道FB增益符号
#define ANCR_TRANS_SIGN				BIT(6)  //右声道通透增益符号
#define ANCR_CMP_SIGN				BIT(7)  //右声道音乐补偿增益符号

//DRC中断标志
#define ANCL_REF_DRC_ISR			BIT(0)
#define ANCL_ERR_DRC_ISR			BIT(1)
#define ANCR_REF_DRC_ISR			BIT(2)
#define ANCR_ERR_DRC_ISR			BIT(3)
#define ANC_ERR_DRC_ISR				ANCL_ERR_DRC_ISR | ANCR_ERR_DRC_ISR
#define ANC_REF_DRC_ISR				ANCL_REF_DRC_ISR | ANCR_REF_DRC_ISR

//DRC模块使能标志
#define ANC_DRC_FF_EN				BIT(0)	//DRC_FF使能标志
#define ANC_DRC_FB_EN				BIT(1)	//DRC_FB使能标志
#define ANC_DRC_TRANS_EN			BIT(2)	//DRC_TRANS使能标志

//ANC DUT音频模块使能控制
#define ANC_DUT_ANC_ENABLE			BIT(0)	//ANC_DUT ANC使能
#define ANC_DUT_ADC_ENABLE			BIT(1)	//ANC_DUT ADC使能
#define ANC_DUT_DAC_ENABLE			BIT(2)	//ANC_DUT DAC使能
#define ANC_DUT_CLOSE				ANC_DUT_ANC_ENABLE | ANC_DUT_ADC_ENABLE | ANC_DUT_DAC_ENABLE	//ANC_DUT模块关闭,正常开启所有模块

//ANC自适应模式
#define ANC_ADAPTIVE_MANUAL_MODE	0		//自适应手动模式-手动切换等级
#define ANC_ADAPTIVE_GAIN_MODE		1		//自适应增益模式-自动切换等级

/*ANC训练方式*/
#define ANC_AUTO_UART  	   			0
#define ANC_AUTO_BT_SPP    			1
#define ANC_TRAIN_WAY 				ANC_AUTO_BT_SPP

/*SZ频响获取默认算法模式*/
#define ANC_SZ_EXPORT_DEF_ALOGM		ANC_ALOGM3

/*ANC 测试模式类型*/
#define ANC_TEST_TYPE_PCM			0	//MIC_SPK数据以PCM导出
#define ANC_TEST_TYPE_FFT			1	//SZ数据累加FFT导出

/*ANC芯片版本定义(只读)*/
#define ANC_CHIP_VERSION			ANC_VERSION_BR28

/*ANC CFG读写标志*/
#define ANC_CFG_READ		 		0x01
#define ANC_CFG_WRITE				0x02

typedef struct {
    u8  mode;				//当前训练步骤
    u8  train_busy;			//训练繁忙位 0-空闲 1-训练中 2-训练结束测试中
    u8	train_step;			//训练系数
    u8  ret_step;	 		//自适应训练系数值
    u8  auto_ts_en; 		//自动步进微调使能位
    u8  enablebit; 			//训练模式使能标志位 前馈|混合馈|通透
    u8 tool_time_flag;		//工具修改时间标志
    u8	mic_dma_export_en;	//从ANC DAM拿数，会消耗4*4K
    u8 	only_mute_train_en;	//只跑静音训练保持sz,fz系数
    u16 timerout_id;		//训练超时定时器ID
    s16 fb0_gain;
    s16 fb0sz_dly_en;
    u16 fb0sz_dly_num;
    int sz_gain;			//静音训练sz 增益
    u32 sz_lower_thr;		//误差MIC下限能量阈值
    u32 fz_lower_thr;   	//参考MIC下限能量阈值
    u32 sz_noise_thr;		//误差MIC底噪阈值
    u32 fz_noise_thr;   	//参考MIC底噪阈值
    u32 sz_adaptive_thr;	//误差MIC自适应收敛阈值
    u32 fz_adaptive_thr;	//参考MIC自适应收敛阈值
    u32 wz_train_thr;		//噪声训练阈值
} anc_train_para_t;

typedef struct {
    u8 default_flag;	//初始状态标志
    u8 fade_lvl;		//淡入时间fade_lvl*100ms
    u8 trigger_cnt;		//触发计数 trigger_cnt*100ms
    u16	gain;			//目标增益
    u32 pow_lthr;		//低阈值
    u32 pow_thr;		//高阈值
} anc_adap_param_t;

//ANC耳道自适应相关结构体
typedef struct {
    u8 ff_yorder;				//ANC耳道自适应FF IIR num
    u8 fb_yorder;				//ANC耳道自适应FB IIR num
    u8 cmp_yorder;				//ANC耳道自适应CMP IIR num
    double *lff_coeff;			/*耳道自适应-滤波器LFF暂存指针*/
    double *lfb_coeff;			/*耳道自适应-滤波器LFB暂存指针*/
    double *lcmp_coeff;			/*耳道自适应-滤波器LCMP暂存指针*/
    double *rff_coeff;			/*耳道自适应-滤波器RFF暂存指针*/
    double *rfb_coeff;			/*耳道自适应-滤波器RFB暂存指针*/
    double *rcmp_coeff;			/*耳道自适应-滤波器RCMP暂存指针*/
    u8  *ref_ff_curve;			/*耳道自适应-FF金机参考曲线*/
    int ref_ff_curve_len;  		/*耳道自适应-FF金机参考曲线长度*/
    void (*dma_done_cb)(void);
} anc_ear_adaptive_param_t;

struct anc_sz_fft_t {
    volatile u8 state;	//状态
    u8 data_sel;		//目标数据通道
    u8 alogm;			//采样率信息
    u8 check_flag;		//检查DAC有效数据标志
    u8 packet_cnt;		//overlap拼包标志
    u8 packet_lcnt;		//long buf overlap拼包标志
    volatile u8 busy;	//run 繁忙标志
    int all_sample_cnt;	//总样本计数
    int spk_sample_cnt[5];//spk区间样本计数
    int err_sample_cnt[5];//err区间样本次数
    s16 *packet_buf;	//overlap 拼包buf
    s16 *packet_lbuf;	//long buf overlap 拼包buf
    int *inbuf;			//输入数据buf
    float *out0_buf;	//输出数据buf0
    float *out1_buf;	//输出数据buf1
    float *out0_lbuf;	//输出数据lbuf0
    float *out1_lbuf;	//输出数据lbuf1
    void (*dma_run_hdl)(void);
};

typedef struct {
    void (*dma_done_cb)(void);
} anc_adt_param_t;

typedef struct {
    u8 mic_errmsg;
    u8 status;
    u32 dat[4];  //ff_num/ff_dat/fb_num/fb_dat
    u32 pow;
    u32 temp_pow;
    u32 mute_pow;
} anc_ack_msg_t;

#define ANC_GAINS_VERSION 		0X7012	//结构体版本号信息
typedef struct {
//cfg
    u16 version;		//当前结构体版本号
    u8 dac_gain;		//dac模拟增益 			range 0-15;  default 13
    u8 l_ffmic_gain;	//ANCL FFmic增益 		range 0-19;  default 10
    u8 l_fbmic_gain;	//ANCL FBmic增益		range 0-19;  default 10
    u8 cmp_en;			//音乐补偿使能			range 0-1;   default 1
    u8 drc_en;		    //DRC使能				range 0-7;   default 0
    u8 ahs_en;		    //AHS使能				range 0-1;   default 1
    u8 ff_1st_dcc;		//FF 1阶DCC档位 		range 0-8;   default 8
    u8 gain_sign; 		//ANC各类增益的符号     range 0-255; default 0
    u8 noise_lvl;		//训练的噪声等级		range 0-255; default 0
    u8 fade_step;		//淡入淡出步进			range 0-15;  default 1
    u32 alogm;			//ANC算法因素
    u32 trans_alogm;    //通透算法因素
    float l_ffgain;	    //ANCL FF增益			range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    float l_fbgain;	    //ANCL FB增益			range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    float l_transgain;  //ANCL 通透增益			range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    float l_cmpgain;	//ANCL 音乐补偿增益		range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    float r_ffgain;	    //ANCR FF增益			range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    float r_fbgain;	    //ANCR FB增益			range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    float r_transgain;  //ANCR 通透增益			range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    float r_cmpgain;	//ANCR 音乐补偿增益		range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)

    u8 drcff_zero_det;	  //DRCFF过零检测使能	range 0-1; 	   default 0;
    u8 drcff_dat_mode;	  //DRCFF_DAT模式		range 0-3; 	   default 0;
    u8 drcff_lpf_sel;	  //DRCFF_LPF档位		range 0-3; 	   default 0;
    u8 drcfb_zero_det;	  //DRCFB过零检测使		range 0-1; 	   default 0;
    u8 drcfb_dat_mode;    //DRCFB_DAT模式		range 0-3;	   default 0;
    u8 drcfb_lpf_sel;     //DRCFB_LPF档位		range 0-3;	   default 0;

    u16 drcff_lthr;		  //DRCFF_LOW阈值		range 0-32767; default 0;
    u16 drcff_hthr;       //DRCFF_HIGH阈值		range 0-32767; default 32767;
    s16 drcff_lgain;      //DRCFF_LOW增益		range 0-32767; default 0;
    s16 drcff_hgain;	  //DRCFF_HIGH增益		range 0-32767; default 1024;
    s16 drcff_norgain;    //DRCFF_NOR增益		range 0-32767; default 1024;

    u16 drcfb_lthr;       //DRCFB_LOW阈值		range 0-32767; default 0;
    u16 drcfb_hthr;       //DRCFB_HIGH阈值		range 0-32767; default 32767;
    s16 drcfb_lgain;	  //DRCFB_LOW增益		range 0-32767; default 0;
    s16 drcfb_hgain;  	  //DRCFB_HIGH增益		range 0-32767; default 1024;
    s16 drcfb_norgain;    //DRCFB_NOR增益		range 0-32767; default 1024;

    u16 drctrans_lthr;    //DRCTRANS_LOW阈值	range 0-32767; default 0;
    u16 drctrans_hthr;    //DRCTRANS_HIGH阈值	range 0-32767; default 32767;
    s16 drctrans_lgain;   //DRCTRANS_LOW增益	range 0-32767; default 0;
    s16 drctrans_hgain;   //DRCTRANS_HIGH增益	range 0-32767; default 1024;
    s16 drctrans_norgain; //DRCTRANS_NOR增益	range 0-32767; default 1024;

    u8 ahs_dly;			  //AHS_DLY				range 0-15;	   default 1;
    u8 ahs_tap;			  //AHS_TAP				range 0-255;   default 100;
    u8 ahs_wn_shift;	  //AHS_WN_SHIFT		range 0-15;	   default 9;
    u8 ahs_wn_sub;	  	  //AHS_WN_SUB  		range 0-1;	   default 1;
    u16 ahs_shift;		  //AHS_SHIFT   		range 0-65536; default 210;
    u16 ahs_u;			  //AHS步进				range 0-65536; default 4000;
    s16 ahs_gain;		  //AHS增益				range -32767-32767;default -1024;
    u8 ahs_nlms_sel;	  //AHS_NLMS			range 0-1;	   default 0;
    u8 developer_mode;	  //GAIN开发者模式		range 0-1;	   default 0;

    float audio_drc_thr;  //Audio DRC阈值       range -6.0-0;  default -6.0dB;

    u8 r_ffmic_gain;	  //ANCR FFmic增益		range 0-19;	   default 10;
    u8 r_fbmic_gain;	  //ANCR FBmic增益		range 0-19;	   default 10;
    u8 fb_1st_dcc;	  	  //FB 1阶DCC档位 		range 0-8;     default 8;
    u8 ff_2nd_dcc;	  	  //FF 2阶DCC档位 		range 0-15;    default 4;
    u8 fb_2nd_dcc;	      //FB 2阶DCC档位 		range 0-15;    default 1;
    u8 drc_ff_2dcc;		  //DRC FF动态DCC目标值 range 0-15;    default 0;
    u8 drc_fb_2dcc;		  //DRC FB动态DCC目标值 range 0-15;    default 0;

    u8 adaptive_ref_en;	  //耳道自适应-金机曲线参考使能
    u16 drc_dcc_det_time; //DRC DCC检测时间ms	range 0-32767; default 300;
    u16 drc_dcc_res_time; //DRC DCC恢复时间ms	range 0-32767; default 20;

    float adaptive_ref_fb_f;	//FB参考F最深点位置中心值   range 80-200; default 135
    float adaptive_ref_fb_g;	//FB参考G最深点深度   range 12-22; default 18
    float adaptive_ref_fb_q;	//FB参考Q最深点降噪宽度  range 0.4-1.2; default 0.6

    //相关性检测 HD param
    u8 hd_en;				//啸叫检测使能		range 0-1;	   default 1;
    u8 hd_corr_thr;			//相关性检测阈值	range 0-255;   default 232;
    u16 hd_corr_gain;		//相关性预设增益	range 0-1024	default 512;
    u8 hd_corr_dly;			//相关性检测延时	range 2-30;	   default 30;

    //功率检测全频带，增益可预设
    u8 hd_pwr_rate;			//功率检测回音处理前后比例		 	range 0-3; 	   default 2;
    u8 hd_pwr_ctl_gain_en;	//功率检测是否使用预设的增益使能 	range 0-1; 	   default 0;
    u8 hd_pwr_ctl_ahsrst_en;//功率检测是否复位AHS 			 	range 0-1; 	   default 1;
    u16 hd_pwr_thr;			//功率检测阈值设置,增益自动 	 	range 0-32767; default 18000;
    u16 hd_pwr_ctl_gain;	//功率检测预设增益 				 	range 0-16384; default 1638;

    //可选带宽 增益自动调节
    u8 hd_pwr_ref_ctl_en;	//参考功率检测自动调增益使能 		range 0-1; 	   default 0;
    u8 hd_pwr_err_ctl_en;	//误差功率检测自动调增益使能 		range 0-1;     default 0;
    u16 hd_pwr_ref_ctl_hthr;	//参考功率检测H_THR,触发中断 	range 0-32767; default 2000;
    u16 hd_pwr_ref_ctl_lthr1;	//参考功率检测L1_THR 		 	range 0-32767; default 1000;
    u16 hd_pwr_ref_ctl_lthr2;	//参考功率检测L2_THR,小于L1_THR range 0-32767; default 200;

    u16 hd_pwr_err_ctl_hthr;	//误差功率检测H_THR,触发中断 	range 0-32767; default 2000;
    u16 hd_pwr_err_ctl_lthr1;	//误差功率检测L1_THR 		 	range 0-32767; default 1000;
    u16 hd_pwr_err_ctl_lthr2;	//误差功率检测L2_THR,小于L1_THR range 0-32767; default 200;
    u16 reserve_2;

} anc_gain_param_t;

/*ANCIF配置区滤波器系数gains*/
typedef struct {
    anc_gain_param_t gains;
    u8 reserve[236 - 156];	//236 + 20byte(header)
    // u8 reserve[236 - 128];	//236 + 20byte(header)
} anc_gain_t;

typedef struct {
    u8 start;                       //ANC状态
    u8 mode;                        //ANC模式：降噪关;降噪开;通透...
    u8 production_mode;             //量产模式
    u8 developer_mode;				//开发者模式
    u8 anc_fade_en;                 //ANC淡入淡出使能
    u8 tool_enablebit;				//ANC动态使能，使用过程中可随意控制
    u8 debug_sel;					//ANCdebug data输出源
    u8 lff_en;						//左耳FF使能
    u8 lfb_en;						//左耳FB使能
    u8 rff_en;						//右耳FF使能
    u8 rfb_en;						//右耳FB使能
    u8 online_busy;					//在线调试繁忙标志位
    u8 fade_time_lvl;				//淡入时间等级，越大时间越长
    u8 mic_type[4];					//ANC mic类型控制位
    u8 dut_audio_enablebit;			//ANC_DUT 音频使能控制,用于分离模块功耗
    u8 drc_dcc_en;					//drc动态调整DCC使能
    u8 adc_ch;						//adc数字通道
    u8 adaptive_mode;				//ANC场景自适应模式
    u8 anc_lvl;						//ANC等级
    u8 lff_yorder;					//LFF IIR NUM
    u8 lfb_yorder;					//LFB IIR NUM
    u8 lcmp_yorder;					//LTRANS IIR NUM
    u8 ltrans_yorder;               //LCMP IIR NUM
    u8 rff_yorder;					//RFF IIR NUM
    u8 rfb_yorder;                  //RFB IIR NUM
    u8 rcmp_yorder;					//RTRANS IIR NUM
    u8 rtrans_yorder;               //RCMP IIR NUM
    u8 lr_lowpower_en;				//ANCLR(立体声)配置省功耗使能
    u8 anc_coeff_mode;				/*ANC coeff模式-0 普通coeff; 1 自适应coeff*/
    u8 test_type;					//ANC测试模式类型，获取SZ频响 or 获取SPK_MIC PCM数据
    u16 anc_fade_gain;				//ANC淡入淡出增益
    u16 drc_fade_gain;				//DRC淡出增益
    u16 drc_time_id;				//DRCtimeout定时器ID
    u16	enablebit;					//ANC模式:FF,FB,HYBRID
    u16 pow_export_en;				//ANC功率输出使能
    u32 coeff_size;					//ANC 读滤波器总长度
    u32 write_coeff_size;			//ANC 写滤波器总长度
    int train_err;					//训练结果 0:成功 other:失败
    u32 gains_size;					//ANC 滤波器长度

    int *debug_buf;					//测试数据流，最大65536byte(堆)
    s32 *lfir_coeff;				//FZ补偿滤波器表
    s32 *rfir_coeff;				//FZ补偿滤波器表
    double *lff_coeff;				//左耳FF滤波器
    double *lfb_coeff;				//左耳FB滤波器
    double *lcmp_coeff;				//左耳cmp滤波器
    double *ltrans_coeff;			//左耳通透滤波器
    double *rff_coeff;				//右耳FF滤波器
    double *rfb_coeff;  	    	//右耳FB滤波器
    double *rcmp_coeff;				//右耳cmp滤波器
    double *rtrans_coeff;			//右耳通透滤波器
    double *trans_default_coeff;	//通透默认滤波器
    volatile u8 ch;					//ANC通道选择 ： ANC_L_CH | ANC_R_CH
    /*开智能免摘开通透需要开fb*/
    float ltrans_fbgain;  //ANCL FB通透增益			range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    float rtrans_fbgain;  //ANCR FB通透增益			range 0.0316(-30dB) - 31.622(+30dB); default 1.0(0dB)
    double *ltrans_fb_coeff;			//左耳FB通透滤波器
    double *rtrans_fb_coeff;			//右耳FB通透滤波器
    u8 ltrans_fb_yorder;               //LCMP IIR NUM
    u8 rtrans_fb_yorder;               //RCMP IIR NUM

    anc_gain_param_t gains;
    audio_adc_mic_mana_t mic_param[4];	//ANC MIC 控制参数
    anc_train_para_t train_para;//训练参数结构体
    anc_ear_adaptive_param_t *adaptive;
    anc_adt_param_t *adt;
    struct anc_sz_fft_t sz_fft;

    void (*train_callback)(u8, u8);
    void (*pow_callback)(anc_ack_msg_t *msg_t, u8 setp);
    int (*cfg_online_deal_cb)(u8, anc_gain_t *);
    void (*post_msg_drc)(void);
    void (*post_msg_debug)(void);
    void (*adc_set_buffs_cb)(void);
    void (*biquad2ab)(float gain, float f, float fs, float q, double *a0, double *a1, double *a2, double *b0, double *b1, double *b2, int type);
    void (*mult_gain_set)(u8 gain_id, int data);
    void (*pow_adap_fade)(u16 gain);
    u8 adt_state;//智能免摘开启状态
} audio_anc_t;


#define ANC_DB_HEAD_LEN		20/*ANC配置区数据头长度*/
#define ANCIF_GAIN_TAG_01	"ANCGAIN01"
#define ANCIF_COEFF_TAG_01	"ANCCOEF01"
#define ANCIF_GAIN_TAG		ANCIF_GAIN_TAG_01	//当前增益配置版本
#define ANCIF_COEFF_TAG		ANCIF_COEFF_TAG_01	//当前系数配置版本
#define ANCIF_HEADER_LEN	10
typedef struct {
    u32 total_len;	//4 后面所有数据加起来长度
    u16 group_crc;	//6 group_type开始做的CRC，更新数据后需要对应更新CRC
    u16 group_type;	//8
    u16 group_len;  //10 header开始到末尾的长度
    char header[ANCIF_HEADER_LEN];//20
    int coeff[0];
} anc_db_head_t;

#define _PACKED __attribute__((packed))
struct anc_param_head_t {
    u16 id;
    u16 offset;
    u16 len;
} _PACKED;

/*ANCIF配置区滤波器系数coeff*/
typedef struct {
    u16 version;
    u16 cnt;
    u8 dat[0];	//小心double访问非对齐异常
} anc_coeff_t;

/* ANC 数据拼接结构体 */
typedef struct {
    u16 version;
    u16 cnt;
    u16 last_len;
    u16 last_total_len;
    u16 dat_len;	//记录数据包dat的长度
    u8 *dat;		//指向数据包的地址
} anc_packet_data_t;

typedef struct {
    u8 type;
    float a[3];
} _PACKED anc_fr_t;

typedef struct {
    float iir_gain;
} _PACKED anc_mult_gain_t;

typedef struct {
    anc_fr_t *lff;
    anc_fr_t *lfb;
    anc_fr_t *lcmp;
    anc_fr_t *rff;
    anc_fr_t *rfb;
    anc_fr_t *rcmp;
} anc_iir_t;


int audio_anc_train(audio_anc_t *param, u8 en);

/*
 *Description 	: audio_anc_run
 *Arguements  	: param is audio anc param
 *Returns	 	: 0  open success
 *		   		: -EPERM 不支持ANC
 *		   		: -EINVAL 参数错误
 *Notes			:
 */
int audio_anc_run(audio_anc_t *param);

int audio_anc_close();

/*
*********************************************************************
*                  	audio_anc_fade_cfg_set
* Description: ANC fade 基础配置
* Arguments  : en 淡入淡出使能;
			   step 默认1，淡入步进，越大越快越容易有杂音
			   slow 默认0，缓慢步进，越大越慢
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_anc_fade_cfg_set(u8 en, u8 step, u8 slow);

/*
*********************************************************************
*                  	audio_anc_fade
* Description: ANC fade 增益配置
* Arguments  : ch 目标通道(enum ANC_fade_ch_t);
			   gain 目标fade增益
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_anc_fade(u8 ch, u16 gain);

/*
*********************************************************************
*                  	audio_anc_fade_dly_get
* Description: ANC根据采样率计算，淡出淡入时间
* Arguments  : target_value 淡入/出目标值, alogm 当前采样率
* Return	 : dly 延时时间(ms)
* Note(s)    : None.
*********************************************************************
*/
int audio_anc_fade_dly_get(int target_value, int alogm);

void anc_coeff_online_update(audio_anc_t *param, u8 hd_reset_en);

int anc_coeff_size_count(audio_anc_t *param);

/*ANC配置id*/
enum {
    ANC_DB_COEFF,	//ANC系数
    ANC_DB_GAIN,	//ANC增益
    ANC_DB_MAX,
};

typedef struct {
    u8 state;		//ANC配置区状态
    u16 total_size;	//ANC配置区总大小
} anc_db_t;

/*anc配置区初始化*/
int anc_db_init(void);
/*根据配置id获取不同的anc配置*/
int *anc_db_get(u8 id, u32 *len);
/*
 *gain:增益配置,没有的话传NULL
 *coeff:系数配置,没有的话传NULL
 */
int anc_db_put(audio_anc_t *param, anc_gain_t *gain, anc_coeff_t *coeff);
/*anc 音乐补偿模块在线开关*/
void audio_anc_cmp_en(audio_anc_t *param, u8 en);

int anc_coeff_single_fill(audio_anc_t *param, anc_coeff_t *target_coeff);

void audio_anc_dcc_set(u8 ref_1_dcc, u8 err_1_dcc, u8 ref_2_dcc, u8 err_2_dcc);

void audio_anc_gain_sign(audio_anc_t *param);

void audio_anc_drc_ctrl(audio_anc_t *param, float drc_ratio);

void audio_anc_en_set(u8 en);

void audio_anc_norgain(s16 ffgain, s16 fbgain);
/*anc ff增益设置API*/
void audio_anc_ffgain_set(s16 l_gain, s16 r_gain);
/*anc fb增益设置API*/
void audio_anc_fbgain_set(s16 l_gain, s16 r_gain);

void audio_anc_tool_en_ctrl(audio_anc_t *param, u8 enablebit);

int audio_anc_gains_version_verify(audio_anc_t *param, anc_gain_t *cfg_target);

void audio_anc_drc_process(void);

void audio_anc_max_yorder_verify(audio_anc_t *param);

void audio_anc_param_normalize(audio_anc_t *param);

void audio_anc_param_init(audio_anc_t *param);

int anc_cfg_online_deal(u8 cmd, anc_gain_t *cfg);

void audio_anc_drc_process(void);

void audio_anc_debug_data_output(void);

void audio_anc_debug_cbuf_sel_set(u8 sel);

void audio_anc_debug_extern_trigger(u8 flag);

/*播歌状态下特殊处理API*/
void audio_anc_mix_process(u8 en);

/*ANC模块复位*/
void audio_anc_reset(audio_anc_t *param, u8 fade_en);

/********************工具交互API*************************/

/*设置结果回调函数*/
void anc_api_set_callback(void (*callback)(u8, u8), void (*pow_callback)(anc_ack_msg_t *, u8));

/*设置淡入淡出使能*/
void anc_api_set_fade_en(u8 en);

u8 anc_fade_ctr_ch_check(u8 ch);

/*获取ANC淡入淡出使能*/
u8 anc_api_get_fade_en(void);

/*获取参数结构体*/
anc_train_para_t *anc_api_get_train_param(void);

/*获取步进*/
u8 anc_api_get_train_step(void);

/*设置通透目标能量值*/
void anc_api_set_trans_aim_pow(u32 pow);

/*ANC在线调试标志设置*/
void anc_online_busy_set(u8 en);

/*ANC在线调试标志获取*/
u8 anc_online_busy_get(void);

/*ANC滤波器大小设置*/
void anc_coeff_size_set(u8 mode, int len);

/*ANC滤波器大小获取*/
int anc_coeff_size_get(u8 mode);

/*ANC滤波器参数填充*/
int anc_coeff_fill(anc_coeff_t *db_coeff);

/*更新DRC 动态DCC使能设置*/
void anc_drc_dcc_en_set(audio_anc_t *param);

/*ANC功率数据获取，长度为8byte*/
void audio_anc_pow_get(u16 *pow);

/*ANC功率输出使能控制*/
void audio_anc_pow_export_en(u8 en, u8 tool_en);

/*ANC滤波器格式校验*/
int anc_coeff_check(anc_coeff_t *db_coeff, u16 len);

/*ANC量产模式设置*/
void anc_production_mode_set(u8 en);

void audio_anc_fr_format(u8 *out_dat, float *fr, u8 order, const u8 *type);

void audio_anc_adaptive_data_analsis(anc_iir_t *iir);

anc_packet_data_t *anc_data_catch(anc_packet_data_t *row_data_packet, u8 *buf, u16 len, u16 id, u8 init_flag);

int anc_adaptive_ff_ref_data_get(u8 **buf);

void anc_adaptive_ff_ref_data_free(void);

int anc_adaptive_fb_ref_data_get(u8 **buf);

void anc_adaptive_ff_ref_data_save(u8 *buf, int len);

void free_anc_data_packet(anc_packet_data_t *anc_data_packet);

/*******************ANC增益自适应************************/
/*ANC自适应初始化*/
void anc_pow_adap_init(audio_anc_t *param, anc_adap_param_t *ref_p, anc_adap_param_t *err_p, \
                       u8 ref_max_lvl, u8 err_max_lvl);

/*ANC自适应启动*/
void audio_anc_pow_adap_start();

/*ANC自适应关闭*/
void audio_anc_pow_adap_stop(void);

/*ANC自适应增益切换定时器注册*/
void anc_pow_adap_fade_timer_add(int ref_lvl, int err_lvl);

/*ANC自适应当前增益等级获取*/
int anc_pow_adap_lvl_get(u8 anc_type);

/*设置当前的ANC增益等级, 只有普通增益模式才可以设置*/
void audio_anc_lvl_set(u8 lvl, u8 ex_en);


void audio_anc_sz_fft_start(audio_anc_t *param, int len);
void audio_anc_sz_fft_run(audio_anc_t *param);
void audio_anc_sz_fft_stop(audio_anc_t *param);
int audio_anc_sz_fft_outbuf_get(u8 **p);
void audio_anc_sz_fft_outbuf_release(void);
void audio_anc_sz_fft_trigger(void);

#endif/*_ANC_H_*/
