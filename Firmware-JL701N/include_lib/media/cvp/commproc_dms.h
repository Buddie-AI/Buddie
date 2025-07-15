#ifndef _COMMPROC_DMS_H_
#define _COMMPROC_DMS_H_

#include "generic/typedef.h"
#include "cvp_common.h"

/*降噪版本定义*/
#define DMS_V100    0xA1
#define DMS_V200    0xA2

//dms_cfg:
typedef struct {
    u8 mic_again;			//DAC增益,default:3(0~14)
    u8 dac_again;			//MIC增益,default:22(0~31)
    u8 enable_module;       //使能模块
    u8 ul_eq_en;        	//上行EQ使能,default:enable(disable(0), enable(1))
    /*AGC*/
    float ndt_fade_in;  	//单端讲话淡入步进default: 1.3f(0.1 ~ 5 dB)
    float ndt_fade_out;  	//单端讲话淡出步进default: 0.7f(0.1 ~ 5 dB)
    float dt_fade_in;  		//双端讲话淡入步进default: 1.3f(0.1 ~ 5 dB)
    float dt_fade_out;  	//双端讲话淡出步进default: 0.7f(0.1 ~ 5 dB)
    float ndt_max_gain;   	//单端讲话放大上限,default: 12.f(0 ~ 24 dB)
    float ndt_min_gain;   	//单端讲话放大下限,default: 0.f(-20 ~ 24 dB)
    float ndt_speech_thr;   //单端讲话放大阈值,default: -50.f(-70 ~ -40 dB)
    float dt_max_gain;   	//双端讲话放大上限,default: 12.f(0 ~ 24 dB)
    float dt_min_gain;   	//双端讲话放大下限,default: 0.f(-20 ~ 24 dB)
    float dt_speech_thr;    //双端讲话放大阈值,default: -40.f(-70 ~ -40 dB)
    float echo_present_thr; //单端双端讲话阈值,default:-70.f(-70 ~ -40 dB)
    /*aec*/
    int aec_process_maxfrequency;	//default:8000,range[3000:8000]
    int aec_process_minfrequency;	//default:0,range[0:1000]
    int af_length;					//default:128 range[128:256]
    /*nlp*/
    int nlp_process_maxfrequency;	//default:8000,range[3000:8000]
    int nlp_process_minfrequency;	//default:0,range[0:1000]
    float overdrive;				//default:1,range[0:30]
    /*ans*/
    float aggressfactor;			//default:1.25,range[1:2]
    float minsuppress;				//default:0.04,range[0.01:0.1]
    float init_noise_lvl;			//default:-75dB,range[-100:-30]
    /*enc*/
    int enc_process_maxfreq;		//default:8000,range[3000:8000]
    int enc_process_minfreq;		//default:0,range[0:1000]
    int sir_maxfreq;				//default:3000,range[1000:8000]
    float mic_distance;				//default:0.015,range[0.035:0.015]
    float target_signal_degradation;//default:1,range[0:1]
    float enc_aggressfactor;		//default:4.f,range[0:4]
    float enc_minsuppress;			//default:0.09f,range[0:0.1]
    /*common*/
    float global_minsuppress;		//default:0.0,range[0.0:0.09]
} _GNU_PACKED_ AEC_DMS_CONFIG;

//dms_flexible_cfg:
typedef struct {
    u8 ver;						//Ver:01
    u8 mic_again;				//MIC0增益,default:10(0~14)
    u8 mic1_again;				//MIC1增益,default:3(0~14)
    u8 dac_again;				//DAC增益,default:22(0~31)
    u8 enable_module;       	//使能模块 NS:BIT(2) ENC:BIT(3)  AGC:BIT(4)
    u8 ul_eq_en;        		//上行EQ使能,default:enable(disable(0), enable(1))
    u8 reserved[2];

    /*AGC*/
    float ndt_fade_in;  		//单端讲话淡入步进default: 1.3f(0.1 ~ 5 dB)
    float ndt_fade_out;  		//单端讲话淡出步进default: 0.7f(0.1 ~ 5 dB)
    float dt_fade_in;  			//双端讲话淡入步进default: 1.3f(0.1 ~ 5 dB)
    float dt_fade_out;  		//双端讲话淡出步进default: 0.7f(0.1 ~ 5 dB)
    float ndt_max_gain;   		//单端讲话放大上限,default: 12.f(0 ~ 24 dB)
    float ndt_min_gain;   		//单端讲话放大下限,default: 0.f(-20 ~ 24 dB)
    float ndt_speech_thr;   	//单端讲话放大阈值,default: -50.f(-70 ~ -40 dB)
    float dt_max_gain;   		//双端讲话放大上限,default: 12.f(0 ~ 24 dB)
    float dt_min_gain;   		//双端讲话放大下限,default: 0.f(-20 ~ 24 dB)
    float dt_speech_thr;    	//双端讲话放大阈值,default: -40.f(-70 ~ -40 dB)
    float echo_present_thr; 	//单端双端讲话阈值,default:-70.f(-70 ~ -40 dB)
    /*aec*/
    int aec_process_maxfrequency;	//default:8000,range[3000:8000]
    int aec_process_minfrequency;	//default:0,range[0:1000]
    int af_length;					//default:128 range[128:256]
    /*nlp*/
    int nlp_process_maxfrequency;	//default:8000,range[3000:8000]
    int nlp_process_minfrequency;	//default:0,range[0:1000]
    float overdrive;				//default:1,range[0:30]
    /*ans*/
    float aggressfactor;		//default:1.25,range[1:2]
    float minsuppress;			//default:0.04,range[0.01:0.1]
    float init_noise_lvl;		//default:-75dB,range[-100:-30]
    /*enc*/
    float enc_suppress_pre;		//ENC前级压制，越大越强 default:0.6f,range[0:1]
    float enc_suppress_post;	//ENC后级压制，越大越强 default:0.15f,range[0:1]
    float enc_minsuppress;		//ENC后级压制下限 default:0.09f,range[0:1]
    float enc_disconverge_erle_thr;	//滤波器发散控制阈值，越大控制越强 default:-6.f,range[-20:5]

} DMS_FLEXIBLE_CONFIG;

struct dms_attr {
    u8 ul_eq_en: 1;
    u8 wideband: 1;
    u8 wn_en: 1;
    u8 dly_est : 1;
    u8 aptfilt_only: 1;
    u8 reserved: 3;

    u8 dst_delay;/*延时估计目标延时*/
    u8 EnableBit;
    u8 packet_dump;
    u8 SimplexTail;
    u8 output_sel;/*dms output选择*/
    u16 hw_delay_offset;/*dac hardware delay offset*/
    u16 wn_gain;/*white_noise gain*/
    /*AGC*/
    float AGC_NDT_fade_in_step; 	//in dB
    float AGC_NDT_fade_out_step; 	//in dB
    float AGC_NDT_max_gain;     	//in dB
    float AGC_NDT_min_gain;     	//in dB
    float AGC_NDT_speech_thr;   	//in dB
    float AGC_DT_fade_in_step;  	//in dB
    float AGC_DT_fade_out_step; 	//in dB
    float AGC_DT_max_gain;      	//in dB
    float AGC_DT_min_gain;      	//in dB
    float AGC_DT_speech_thr;    	//in dB
    float AGC_echo_present_thr; 	//In dB
    int AGC_echo_look_ahead;    	//in ms
    int AGC_echo_hold;          	// in ms
    /*AEC*/
    int aec_process_maxfrequency;	//default:8000,range[3000:8000]
    int aec_process_minfrequency;	//default:0,range[0:1000]
    int af_length;					//default:128 range[128:256]
    /*NLP*/
    int nlp_process_maxfrequency;	//default:8000,range[3000:8000]
    int nlp_process_minfrequency;	//default:0,range[0:1000]
    float overdrive;				//default:1,range[0:30]
    /*ANS*/
    float aggressfactor;			//default:1.25,range[1:2]
    float minsuppress;				//default:0.04,range[0.01:0.1]
    float init_noise_lvl;			//default:-75db,range[-100:-30]
    /*ENC*/
    int enc_process_maxfreq;		//default:8000,range[3000:8000]
    int enc_process_minfreq;		//default:0,range[0:1000]
    int sir_maxfreq;				//default:3000,range[1000:8000]
    float mic_distance;				//default:0.015,range[0.035:0.015]
    float target_signal_degradation;//default:1,range[0:1]
    float enc_aggressfactor;		//default:4.f,range[0:4]
    float enc_minsuppress;			//default:0.09f,range[0:0.1]
    /*BCS*/
    int bone_process_maxfreq;
    int bone_process_minfreq;
    float bone_init_noise_lvl;
    int Bone_AEC_Process_MaxFrequency;
    int Bone_AEC_Process_MinFrequency;
    /*common*/
    float global_minsuppress;		//default:0.0,range[0.0:0.09]
    /*data handle*/
    int (*cvp_advanced_options)(void *aec,
                                void *nlp,
                                void *ns,
                                void *enc,
                                void *agc,
                                void *wn,
                                void *mfdt);
    int (*aec_probe)(short *mic0, short *mic2, short *mic3, short *ref, u16 len);
    int (*aec_post)(s16 *dat, u16 len);
    int (*aec_update)(u8 EnableBit);
    int (*output_handle)(s16 *dat, u16 len);

    /*flexible enc*/
    int flexible_af_length;          //default:512 range[128、256、512、1024]
    int sir_minfreq;                 //default:100,range[0:8000]
    int SIR_mean_MaxFrequency;       //default:1000,range[0:8000]
    int SIR_mean_MinFrequency;       //default:100,range[0:8000]
    float ENC_CoheFlgMax_gamma;      //default:0.5f,range[0:1]
    float coheXD_thr;                //default:0.5f,range[0:1]
    float Disconverge_ERLE_Thr;      //default:-6.f,range[-20:5]

    /*WN*/
    float wn_detect_time;           //in second
    float wn_detect_time_ratio_thr; //0-1
    float wn_detect_thr;            //0-1
    float wn_minsuppress;           //0-1

    /*Extended-Parameters*/
    u16 ref_sr;
    u16 ref_channel;    /*参考数据声道数*/
    u16 adc_ref_en;    /*adc回采参考数据使能*/

    /*DNS Parameters*/
    float DNS_highGain;     //EQ强度   range[1.0:3.5]
    float DNS_rbRate;       //混响强度 range[0:0.9];

    /*MFDT Parameters*/
    float detect_time;           // // 检测时间s，影响状态切换的速度
    float detect_eng_diff_thr;   // 0~-90 dB 两个mic能量差异持续大于此阈值超过检测时间则会检测为故障
    float detect_eng_lowerbound; // 0~-90 dB 当处于故障状态时，正常的mic能量大于此阈值才会检测能量差异，避免安静环境下误判切回正常状态
    int MalfuncDet_MaxFrequency;// 检测信号的最大频率成分
    int MalfuncDet_MinFrequency;// 检测信号的最小频率成分
    int OnlyDetect;// 0 -> 故障切换到单mic模式， 1-> 只检测不切换
};

s32 aec_dms_init(struct dms_attr *attr);
s32 aec_dms_exit();
s32 aec_dms_fill_in_data(void *dat, u16 len);
int aec_dms_fill_in_ref_data(void *dat, u16 len);
s32 aec_dms_fill_ref_data(void *dat, u16 len);
void aec_dms_toggle(u8 toggle);
int aec_dms_cfg_update(AEC_DMS_CONFIG *cfg);
int aec_dms_reboot(u8 enablebit);

s32 aec_dms_flexible_init(struct dms_attr *attr);
s32 aec_dms_flexible_exit();
s32 aec_dms_flexible_fill_in_data(void *dat, u16 len);
int aec_dms_flexible_fill_in_ref_data(void *dat, u16 len);
s32 aec_dms_flexible_fill_ref_data(void *dat, u16 len);
void aec_dms_flexible_toggle(u8 toggle);
int aec_dms_flexible_cfg_update(DMS_FLEXIBLE_CONFIG *cfg);
int aec_dms_flexible_reboot(u8 enablebit);

/*获取风噪的检测结果，1：有风噪，0：无风噪*/
int cvp_dms_get_wind_detect_state(void);

/*单双麦切换状态
 * 0: 正常双麦 ;
 * 1: 副麦坏了，触发故障
 * -1: 主麦坏了，触发故障
 */
int cvp_dms_get_malfunc_state(void);

/*
 * 获取mic的能量值，开了MFDT_EN才能用
 * mic: 0 获取主麦能量
 * mic：1 获取副麦能量
 * return：返回能量值，[0~90.3],返回-1表示错误
 */
float cvp_dms_get_mic_energy(u8 mic);

typedef enum {
    DMS_FLEXIBLE_DUAL_MIC = 0,	//双mic
    DMS_FLEXIBLE_USE_TALK_MIC,	//通话mic（主麦）
    DMS_FLEXIBLE_USE_REF_MIC,	//参考mic（副麦）
} DMS_FLEXIBLE_MIC;

/* 话务耳机切换使用的mic
 * DMS_FLEXIBLE_DUAL_MIC ：正常双麦话务耳机
 * DMS_FLEXIBLE_USE_TALK_MIC ：使用主麦
 * DMS_FLEXIBLE_USE_REF_MIC ：使用副麦
 */
int aec_dms_flexible_selete_mic(DMS_FLEXIBLE_MIC mic);

int cvp_dms_read_ref_data(void);
int cvp_dms_flexible_read_ref_data(void);

/*
 * 获取风噪检测信息
 * wd_flag: 0 没有风噪，1 有风噪
 * 风噪强度r: 0~BIT(16)
 * wd_lev: 风噪等级，0：弱风，1：中风，2：强风
 * */
int jlsp_get_wind_detect_info(int *wd_flag, int *wd_val, int *wd_lev);

#endif/*_COMMPROC_DMS_H_*/
