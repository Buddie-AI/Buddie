#ifndef CLOCK_CFG_H
#define CLOCK_CFG_H

#include "typedef.h"
/*

 */


enum {

    BT_IDLE_CLOCK = 1,
    MUSIC_IDLE_CLOCK,
    FM_IDLE_CLOCK,
    LINEIN_IDLE_CLOCK,
    PC_IDLE_CLOCK,
    REC_IDLE_CLOCK,
    RTC_IDLE_CLOCK,
    SPDIF_IDLE_CLOCK,
    BOX_IDLE_CLOCK,


    DEC_SBC_CLK	,
    DEC_AAC_CLK	,
    DEC_MSBC_CLK,
    DEC_CVSD_CLK,

    AEC8K_CLK	,
    AEC8K_ADV_CLK,
    AEC16K_CLK	,
    AEC16K_ADV_CLK,
    AEC8K_SPX_CLK,
    AEC16K_SPX_CLK,

    DEC_TONE_CLK,
    DEC_MP3_CLK	,
    DEC_WAV_CLK	,	/// 10
    DEC_G729_CLK,
    DEC_G726_CLK,
    DEC_PCM_CLK	,
    DEC_MTY_CLK	,
    DEC_WMA_CLK	,

    DEC_APE_CLK	,
    DEC_FLAC_CLK,
    DEC_AMR_CLK	,
    DEC_DTS_CLK	,

    DEC_M4A_CLK	,    ///20
    DEC_ALAC_CLK,
    DEC_FM_CLK	,
    DEC_LINE_CLK,
    DEC_TWS_SBC_CLK,
    SPDIF_CLK	,

    ENC_RECODE_CLK,
    ENC_SBC_CLK	,
    ENC_WAV_CLK	,
    ENC_G726_CLK,
    ENC_MP3_CLK	,
    ENC_TWS_SBC_CLK,

    ENC_MSBC_CLK,      //////30
    ENC_CVSD_CLK,
    SYNC_CLK	,
    AUTOMUTE_CLK	,
    FINDF_CLK	,
    FM_INSIDE_CLK,
    BT_CONN_CLK	,

    EQ_CLK	,
    EQ_DRC_CLK	,
    EQ_ONLINE_CLK,
    REVERB_CLK	,
    REVERB_HOWLING_CLK,
    REVERB_PITCH_CLK,

    DEC_MP3PICK_CLK	,
    DEC_WMAPICK_CLK	,
    DEC_M4APICK_CLK	,
    DEC_MIX_CLK,

    DEC_IIS_CLK,
    DEC_UI_CLK,
    DEC_MIDI_CLK,

    DEC_3D_CLK,
    DEC_VBASS_CLK,
    DEC_LOUDNES_CLK,

    DONGLE_ENC_CLK,

    LCD_UI_CLK,
    SCAN_DISK_CLK,
    SPECTRUM_CLK,

    LOCALTWS_CLK,

    AI_SPEECH_CLK,
    SMARTBOX_ACTION_CLK,

    ADAPTER_PROCESS_CLK,

    ENUM_MAX_CLK = 63	,
};

void clock_pause_play(u8 mode);


void clock_idle(u32 type);
void clock_add(u32 type);
void clock_remove(u32 type);
void clock_set_cur(void);
void clock_add_set(u32 type);
void clock_remove_set(u32 type);


#endif
