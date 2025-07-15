/*****************************************************************
>file name : smart_voice_config.c
>author : lichao
>create time : Mon 01 Nov 2021 11:18:03 AM CST
*****************************************************************/
#include "typedef.h"
#include "app_config.h"

#if (TCFG_SMART_VOICE_ENABLE)
const int config_lp_vad_enable = 1;
const int config_jl_audio_kws_enable = 1; /*KWS 使能*/
const int config_aispeech_asr_enable = 0;
const int config_user_asr_enable = 0;
const int config_audio_kws_event_enable = 1;
#elif ((defined TCFG_AUDIO_ASR_DEVELOP) && (TCFG_AUDIO_ASR_DEVELOP == ASR_CFG_AIS))
const int config_lp_vad_enable = 1;
const int config_jl_audio_kws_enable = 0;
const int config_aispeech_asr_enable = 1;
const int config_user_asr_enable = 0;
const int config_audio_kws_event_enable = 1;
#elif ((defined TCFG_AUDIO_ASR_DEVELOP) && (TCFG_AUDIO_ASR_DEVELOP == ASR_CFG_USER_DEFINED))
const int config_lp_vad_enable = 1;
const int config_jl_audio_kws_enable = 0;
const int config_aispeech_asr_enable = 0;
const int config_user_asr_enable = 1;
const int config_audio_kws_event_enable = 1;
#else
const int config_lp_vad_enable = 0;
const int config_jl_audio_kws_enable = 0;
const int config_aispeech_asr_enable = 0;
const int config_user_asr_enable = 0;
const int config_audio_kws_event_enable = 0;
#endif

const int config_audio_nn_vad_enable = 0;
