#ifndef _AUDIO_AEC_ONLINE_H_
#define _AUDIO_AEC_ONLINE_H_

#include "generic/typedef.h"

/*
 *0x30xx:单mic降噪ANS
 *0x31xx:双mic降噪ANS
 *0x32xx:单mic降噪DNS
 *0x33xx:双mic降噪DNS
 *0x34xx:双mic话务耳机ANS
 *0x35xx:双mic话务耳机DNS
 *如果有版本更新，通过更新后两位来区分，比如：0x3001
 */
#define AEC_CFG_SMS		                    0x3000
#define AEC_CFG_DMS		                    0x3100
#define AEC_CFG_SMS_DNS                     0x3200
#define AEC_CFG_DMS_DNS                     0x3300
#define AEC_CFG_DMS_FLEXIBLE                0x3400
#define AEC_CFG_DMS_FLEXIBLE_DNS            0x3500

//GENERAL_CONFIG:0x0000~0x0FFF
enum {
    GENERAL_DAC = 0x0000,
    GENERAL_MIC,
    /*app端在线调试的enablebit;*/
    GENERAL_ModuleEnable,
    GENERAL_UL_EQ,
    GENERAL_Global_MinSuppress,

    GENERAL_MIC_1,

    /*pc端在线调试的enablebit;*/
    GENERAL_PC_ModuleEnable,
};

//AEC_CONFIG:0x1000~0x2FFF
enum {
    //dms
    AEC_ProcessMaxFreq = 0x1000,
    AEC_ProcessMinFreq,
    AEC_AF_Lenght,
    //sms
    AEC_DT_AGGRESS = 0x2000,
    AEC_REFENGTHR,
};

//NLP_CONFIG:0x3000~0x4FFF
enum {
    //dms
    NLP_ProcessMaxFreq = 0x3000,
    NLP_ProcessMinFreq,
    NLP_OverDrive,
    //sms
    NLP_AGGRESS_FACTOR = 0x4000,
    NLP_MIN_SUPPRESS,
};

//ANS_CONFIG:0x5000~0x6FFF
enum {
    //dms
    ANS_AggressFactor = 0x5000,
    ANS_MinSuppress,
    ANS_MicNoiseLevel,
    //sms
    ANS_AGGRESS = 0x6000,
    ANS_SUPPRESS,
    //DNS
    DNS_GainFloor = 0x6100,
    DNS_OverDrive,
};

//AGC_CONFIG(common):0x7000~0x7FFF
enum {
    AGC_NDT_FADE_IN = 0x7000,
    AGC_NDT_FADE_OUT,
    AGC_DT_FADE_IN,
    AGC_DT_FADE_OUT,
    AGC_NDT_MAX_GAIN,
    AGC_NDT_MIN_GAIN,
    AGC_NDT_SPEECH_THR,
    AGC_DT_MAX_GAIN,
    AGC_DT_MIN_GAIN,
    AGC_DT_SPEECH_THR,
    AGC_ECHO_PRESENT_THR,
};

//ENC_CONFIG(only dms):0x8000~0x8FFF
enum {
    ENC_Process_MaxFreq = 0x8000,
    ENC_Process_MinFreq,
    ENC_SIR_MaxFreq,
    ENC_MIC_Distance,
    ENC_Target_Signal_Degradation,
    ENC_AggressFactor,

    ENC_MinSuppress,

    ENC_Suppress_Pre,
    ENC_Suppress_Post,
    ENC_Disconverge_Thr,
};

typedef struct {
    int id;	//参数id号
    union {//参数值(整形或者浮点)
        float val_float;
        int val_int;
    };
} aec_online_t;

int aec_cfg_online_init();
int aec_cfg_online_exit();
int aec_cfg_online_update(int root_cmd, void *cfg);
int aec_cfg_online_update_fill(void *cfg, u16 len);
int get_aec_config(u8 *buf, int version);

#endif/*_AUDIO_AEC_ONLINE_H_*/
