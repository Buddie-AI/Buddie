#ifndef _ICSD_ANC_APP_H
#define _ICSD_ANC_APP_H

/*===========接口说明=============================================
一. void sd_anc_init(audio_anc_t *param,u8 mode)
  1.调用该函数启动sd anc训练
  2.param为ANC ON模式下的audio_anc_t参数指针
  3.mode：0. 普通模式   1. htarget 数据上传模式

二. void sd_anc_htarget_data_send(float *h_freq,float *hszpz_out,float *hpz_out, float *htarget_out,int len)
  1.htarget数据上传模式下，当数据准备好后会自动调用该函数

三. void sd_anc_htarget_data_end()
  1.htarget数据上传模式下，当数据上传完成后需要调用该函数进入训练后的ANC ON模式

四. void anc_user_train_tone_play_cb()
  1.当前ANC ON提示音结束时回调该函数进入 SD ANC 训练
  2.SD ANC训练结束后会切换到ANC ON模式

 	1:rdout   + lout		2:rdout   + rerrmic
 	3:rdout   + rrefmic		4:ldout   + lerrmic
 	5:ldout   + lrefmic		6:rerrmic + rrefmic
 	7:lerrmic + lrefmic

	anc_sr, 0: 11.7k  1: 23.4k  2: 46.9k  3: 93.8k  4: 187.5k     5: 375k     6: 750k      7: 1.5M

================================================================*/

#include "asm/anc.h"
#include "asm/dac.h"
#include "app_config.h"
#include "icsd_anc.h"
#include "icsd_anc_client_board.h"

#ifndef ADAPTIVE_CLIENT_BOARD

#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER			8			/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER			5			/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER			8			/*ANC自适应CMP滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_RECORD_FF_ORDER	8			/*ANC自适应耳道记忆FF滤波器阶数，原厂指定*/
#define ANC_FB_TRAIN_OFF            1
#define ANC_FB_TRAIN_NEW            0
#define ANC_EARPHONE_CHECK_EN       0
#define ANC_ADAPTIVE_CMP_EN         1 				//自适应CMP补偿
#define ANC_ADAPTIVE_TONE_DELAY		1570

#endif/*ADAPTIVE_CLIENT_BOARD*/

#define ANC_HEADSET_TONE_00			0
#define ANC_HEADSET_TONE_01		    1
#define ANC_HEADSET_TONE			ANC_HEADSET_TONE_00

#if ANC_HEADSET_TONE == ANC_HEADSET_TONE_00
#define  TONEL_DELAY     			50
#define  TONER_DELAY     			2400
#define  PZL_DELAY       			900
#define  PZR_DELAY       			3300
#else
#define TONEL_DELAY     			50
#define TONER_DELAY     			1000
#define PZL_DELAY       			1900
#define PZR_DELAY       			3200
#endif

#define TEST_EAR_RECORD_EN         	0
#define TEST_ANC_COMBINATION  		TEST_ANC_COMBINATION_OFF
#define TEST_ANC_TRAIN          	TEST_ANC_TRAIN_OFF
#define ICSD_ANC_DEBUG_TYPE			ANC_DEBUG_OFF

#define ANC_USER_TRAIN_EN			1

#if ICSD_ANC_MODE == HEADSET_BYPASS_MODE
#define ANC_USER_TRAIN_TONE_MODE    0
#define BYPASS_MAXTIME              4
#define PZ_MAXTIME                  4

#elif ICSD_ANC_MODE == HEADSET_TONES_MODE
#define ANC_USER_TRAIN_TONE_MODE    1
#define BYPASS_MAXTIME              2
#define PZ_MAXTIME                  3

#elif ICSD_ANC_MODE == TWS_BYPASS_MODE
#define ANC_USER_TRAIN_TONE_MODE    0
#define BYPASS_MAXTIME              4
#define PZ_MAXTIME                  4

#elif ICSD_ANC_MODE == TWS_TONE_BYPASS_MODE
#define ANC_USER_TRAIN_TONE_MODE    1
#define BYPASS_MAXTIME              2
#define PZ_MAXTIME                  3

#elif ICSD_ANC_MODE == TWS_TONE_MODE
#define ANC_USER_TRAIN_TONE_MODE    1
#define BYPASS_MAXTIME              2
#define PZ_MAXTIME                  3

#elif ICSD_ANC_MODE == HEADSET_TONE_BYPASS_MODE
#define ANC_USER_TRAIN_TONE_MODE    1
#define BYPASS_MAXTIME              2
#define PZ_MAXTIME                  3

#else
#define ANC_USER_TRAIN_TONE_MODE    0
#define BYPASS_MAXTIME              4
#define PZ_MAXTIME                  4
#endif

#define ANC_USER_TRAIN_MODE         2//0:FF 1:FB 2:HY
#define ANC_USER_TRAIN_SR_SEL       2//46.9k
#define ANC_EAR_RECORD_EN           1 //耳道记忆
#define EAR_RECORD_MAX  		    5
#define ANC_DFF_AFB_EN              1//默认FF + 自适应FB  1:判断为自适应成功  0:判断为自适应失败
#define ANC_AFF_DFB_EN              1//默认FB + 自适应FF  1:判断为自适应成功  0:判断为自适应失败
#define ANC_SZ_OUT_EN               0//自适应SZ输出使能
#define HEADSET_TONES_MODE_BYPASS_OFF   0

//耳道记忆滤波器保存大小，滤波器组 * 3 + 总增益
#define ANC_VMDATA_FF_RECORD_SIZE	(ANC_ADAPTIVE_RECORD_FF_ORDER * 3) + 1

extern float (*vmdata_FL)[ANC_VMDATA_FF_RECORD_SIZE];//ff fgq
extern float (*vmdata_FR)[ANC_VMDATA_FF_RECORD_SIZE];//ff fgq
extern int *vmdata_num;

#define ANC_USE_RECORD     			1	//ANC自适应失败使用耳道记忆参数
#define ANC_SUCCESS        			2	//ANC自适应成功
#define ANC_USE_DEFAULT    			3	//ANC自适应使用ICSD内部默认参数，目前不用;

//自适应训练结果 u8
#define ANC_ADAPTIVE_RESULT_LFF		BIT(0)	//使用LFF自适应or记忆参数
#define ANC_ADAPTIVE_RESULT_LFB    	BIT(1)	//使用LFB自适应参数
#define ANC_ADAPTIVE_RESULT_LCMP    BIT(2)	//使用LCMP自适应参数
#define ANC_ADAPTIVE_RESULT_RFF     BIT(3)	//使用RFF自适应or记忆参数
#define ANC_ADAPTIVE_RESULT_RFB     BIT(4)	//使用RFB自适应参数
#define ANC_ADAPTIVE_RESULT_RCMP    BIT(5)	//使用RCMP自适应参数

struct icsd_fb_ref {
    float m_value;
    float sen;
    float in_q;
};

struct icsd_ff_ref {
    float fre[75];
    float db[75];
};


extern void icsd_anc_vmdata_init(float (*FL)[ANC_VMDATA_FF_RECORD_SIZE], float (*FR)[ANC_VMDATA_FF_RECORD_SIZE], int *num);

#endif/*_ICSD_ANC_APP_H*/
