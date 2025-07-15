#ifndef _AUDIO_ENC_H_
#define _AUDIO_ENC_H_

#include "generic/typedef.h"

enum enc_source {
    ENCODE_SOURCE_MIX = 0x0,
    ENCODE_SOURCE_MIC,
    ENCODE_SOURCE_LINE0_LR,
    ENCODE_SOURCE_LINE1_LR,
    ENCODE_SOURCE_LINE2_LR,
    ENCODE_SOURCE_USER,
};

enum {
    MIC_PWR_INIT = 1,	/*开机状态*/
    MIC_PWR_ON,			/*工作状态*/
    MIC_PWR_OFF,		/*空闲状态*/
    MIC_PWR_DOWN,	/*低功耗状态*/

};

int esco_enc_open(u32 coding_type, u8 frame_len);
void esco_enc_close();
void audio_mic_pwr_ctl(u8 state);
u8 get_master_mic_gain(u8 master);
void esco_mic_reset(void);
void esco_mic_cfg_set(u16 sr, u16 mic0_gain, u16 mic1_gain, u16 mic2_gain, u16 mic3_gain);
#endif/*_AUDIO_ENC_H_*/
