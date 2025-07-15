#ifndef _AEC_USER_H_
#define _AEC_USER_H_

#include "generic/typedef.h"
#include "user_cfg.h"

#define AEC_DEBUG_ONLINE	0
#define AEC_READ_CONFIG		1

/*
 *CVP(清晰语音模式定义)
 */
#define CVP_MODE_NONE		0x00	//清晰语音处理关闭
#define CVP_MODE_NORMAL		0x01	//通用模式
#define CVP_MODE_DMS		0x02	//双mic降噪(ENC)模式
#define CVP_MODE_SMS		0x03	//单mic系统(外加延时估计)
#define CVP_MODE_SMS_TDE	0x04	//单mic系统(内置延时估计)
#define CVP_MODE_SEL		CVP_MODE_NORMAL

#define AEC_EN				BIT(0)
#define NLP_EN				BIT(1)
#define	ANS_EN				BIT(2)
#define	ENC_EN				BIT(3)
#define	AGC_EN				BIT(4)
#define WNC_EN              BIT(5)
#define MFDT_EN             BIT(6)

/*aec module enable bit define*/
#define AEC_MODE_ADVANCE	(AEC_EN | NLP_EN | ANS_EN )
#define AEC_MODE_REDUCE		(NLP_EN | ANS_EN )

#define AEC_MODE_SIMPLEX	(ANS_EN)

/*
 *SMS模式选择
 *(1)SMS模式性能更好，同时也需要更多的ram和mips
 *(2)SMS_NORMAL和SMS_TDE功能一样，只是SMS_TDE内置了延时估计和延时补偿
 *可以更好的兼容延时不固定的场景
 */
#define SMS_DISABLE		0
#define SMS_NORMAL		1
#define SMS_TDE			2
#define TCFG_AUDIO_SMS_ENABLE	SMS_DISABLE

/*
 *DMS版本配置
 *DMS_GLOBAL_V100:第一版双麦算法
 *DMS_GLOBAL_V200:第二版双麦算法，更少的ram和mips
 */
#define TCFG_AUDIO_DMS_GLOBAL_VERSION		DMS_GLOBAL_V100

#if (defined(CONFIG_MEDIA_NEW_ENABLE) || (defined(CONFIG_MEDIA_DEVELOP_ENABLE)))

extern const u8 CONST_AEC_SIMPLEX;

/*兼容SMS和DMS*/
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
#include "commproc_tms.h"
#define aec_open		    aec_tms_init
#define aec_close		    aec_tms_exit
#define aec_in_data		    aec_tms_fill_in_data
#define aec_in_data_ref     aec_tms_fill_in_ref_data
#define aec_in_data_ref_1	aec_tms_fill_in_ref_1_data
#define aec_ref_data	    aec_tms_fill_ref_data

#elif TCFG_AUDIO_DUAL_MIC_ENABLE
#include "commproc_dms.h"
#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
#define aec_open		aec_dms_init
#define aec_close		aec_dms_exit
#define aec_in_data		aec_dms_fill_in_data
#define aec_in_data_ref	aec_dms_fill_in_ref_data
#define aec_ref_data	aec_dms_fill_ref_data
#else
#define aec_open		aec_dms_flexible_init
#define aec_close		aec_dms_flexible_exit
#define aec_in_data		aec_dms_flexible_fill_in_data
#define aec_in_data_ref	aec_dms_flexible_fill_in_ref_data
#define aec_ref_data	aec_dms_flexible_fill_ref_data
#endif/*TCFG_AUDIO_DMS_SEL*/

void aec_cfg_fill(AEC_DMS_CONFIG *cfg);
/*
*********************************************************************
*                  Audio AEC Output Select
* Description: 输出选择
* Arguments  : sel  = Default	默认输出算法处理结果
*					= Master	输出主mic(通话mic)的原始数据
*					= Slave		输出副mic(降噪mic)的原始数据
*			   agc 输出数据要不要经过agc自动增益控制模块
* Return	 : None.
* Note(s)    : 可以通过选择不同的输出，来测试mic的频响和ENC指标
*********************************************************************
*/
void audio_aec_output_sel(CVP_OUTPUT_ENUM sel, u8 agc);

#elif (TCFG_AUDIO_SMS_ENABLE == SMS_TDE)
#include "commproc.h"
#define aec_open		sms_tde_init
#define aec_close		sms_tde_exit
#define aec_in_data		sms_tde_fill_in_data
#define aec_in_data_ref(...)
#define aec_ref_data	sms_tde_fill_ref_data
void aec_cfg_fill(AEC_CONFIG *cfg);
#else
#include "commproc.h"
#define aec_open		aec_init
#define aec_close		aec_exit
#define aec_in_data		aec_fill_in_data
#define aec_in_data_ref(...)
#define aec_ref_data	aec_fill_ref_data
void aec_cfg_fill(AEC_CONFIG *cfg);
#endif

s8 aec_debug_online(void *buf, u16 size);
void aec_input_clear_enable(u8 enable);

int audio_aec_init(u16 sample_rate);
void audio_aec_close(void);
void audio_aec_inbuf(s16 *buf, u16 len);
void audio_aec_inbuf_ref(s16 *buf, u16 len);
void audio_aec_inbuf_ref_1(s16 *buf, u16 len);
void audio_aec_refbuf(s16 *buf, u16 len);
u8 audio_aec_status(void);
void audio_aec_reboot(u8 reduce);
void audio_aec_ref_start(u8 en);
/*
*********************************************************************
*                  Audio AEC Open
* Description: 初始化AEC模块
* Arguments  : sr 			采样率(8000/16000)
*			   enablebit	使能模块(AEC/NLP/AGC/ANS...)
*			   out_hdl		自定义回调函数，NULL则用默认的回调
* Return	 : 0 成功 其他 失败
* Note(s)    : 该接口是对audio_aec_init的扩展，支持自定义使能模块以及
*			   数据输出回调函数
*********************************************************************
*/
int audio_aec_open(u16 sample_rate, s16 enablebit, int (*out_hdl)(s16 *data, u16 len));

/*
*********************************************************************
*                  			Audio CVP IOCTL
* Description: CVP功能配置
* Arguments  : cmd 		操作命令
*		       value 	操作数
*		       priv 	操作内存地址
* Return	 : 0 成功 其他 失败
* Note(s)    : (1)比如动态开关降噪NS模块:
*				audio_cvp_ioctl(CVP_NS_SWITCH,1,NULL);	//降噪关
*				audio_cvp_ioctl(CVP_NS_SWITCH,0,NULL);  //降噪开
*********************************************************************
*/
int audio_cvp_ioctl(int cmd, int value, void *priv);

/*获取风噪的检测结果，1：有风噪，0：无风噪*/
int audio_cvp_dms_wnc_state(void);

/*获取单双麦切换状态
 * 0: 正常双麦 ;
 * 1: 副麦坏了，触发故障
 * -1: 主麦坏了，触发故障
 */
int audio_cvp_dms_malfunc_state();

/*
 * 获取mic的能量值，开了MFDT_EN才能用
 * mic: 0 获取主麦能量
 * mic：1 获取副麦能量
 * return：返回能量值，[0~90.3],返回-1表示错误
 */
float audio_cvp_dms_mic_energy(u8 mic);

/*tri_en: 1 正常3MIC降噪
 *        0 变成2MIC降噪，不使用 FB MIC数据*/
int audio_tms_mode_choose(u8 tri_en);

#else

extern struct aec_s_attr aec_param;
extern const u8 CONST_AEC_SIMPLEX;

struct aec_s_attr *aec_param_init(u16 sr);
s8 aec_debug_online(void *buf, u16 size);
void aec_cfg_fill(AEC_CONFIG *cfg);
void aec_input_clear_enable(u8 enable);

#endif /* #if (defined(CONFIG_MEDIA_NEW_ENABLE) || (defined(CONFIG_MEDIA_DEVELOP_ENABLE))) */


//aec 上行音效
enum  {
    SOUND_ORIGINAL, //原声
    SOUND_UNCLE,    //大叔
    SOUND_GODDESS,  //女神
};

#if defined(AEC_PRESETS_UL_EQ_CONFIG) && AEC_PRESETS_UL_EQ_CONFIG
#ifndef UL_PRESETS_EQ_NSECTION
#define UL_PRESETS_EQ_NSECTION 5
#endif
#if  (UL_PRESETS_EQ_NSECTION > EQ_SECTION_MAX )
#error "EQ_SETCTION_MAX too samll"
#endif
void aec_ul_eq_update();
void ul_presets_eq_set(u8 tab_num);
#endif/*AEC_PRESETS_UL_EQ_CONFIG*/

#endif/*_AEC_USER_H_*/
