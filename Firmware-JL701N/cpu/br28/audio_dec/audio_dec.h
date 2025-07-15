
#ifndef _AUDIO_DEC_H_
#define _AUDIO_DEC_H_

#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "asm/audio_src.h"
// #include "audio_digital_vol.h"
/*#include "application/audio_eq_drc_apply.h"*/


#ifndef RB16
#define RB16(b)    (u16)(((u8 *)b)[0] << 8 | (((u8 *)b))[1])
#endif

extern struct audio_decoder_task decode_task;
extern struct audio_mixer mixer;
extern struct audio_mixer recorder_mixer;

u32 audio_output_rate(int input_rate);
u32 audio_output_channel_num(void);
int audio_output_set_start_volume(u8 state);

int audio_output_start(u32 sample_rate, u8 reset_rate);
void audio_output_stop(void);

struct audio_src_handle *audio_hw_resample_open(void *priv, int (*output_handler)(void *, void *, int),
        u8 channel, u16 input_sample_rate, u16 output_sample_rate);
void audio_hw_resample_close(struct audio_src_handle *hdl);

void audio_resume_all_decoder(void);



//////////////////////////////////////////////////////////////////////////////
void audio_sbc_enc_inbuf_resume(void);

int audio_dec_init();

void set_source_sample_rate(u16 sample_rate);
u16 get_source_sample_rate();
#if USER_DIGITAL_VOLUME_ADJUST_ENABLE != 0
void a2dp_user_digital_volume_set(u8 vol);
u8 a2dp_user_audio_digital_volume_get();
void a2dp_user_digital_volume_tab_set(u16 *user_vol_tab, u8 user_vol_max);

void linein_user_digital_volume_set(u8 vol);
u8 linein_user_audio_digital_volume_get();
void linein_user_digital_volume_tab_set(u16 *user_vol_tab, u8 user_vol_max);

void fm_user_digital_volume_set(u8 vol);
u8 fm_user_audio_digital_volume_get();
void fm_user_digital_volume_tab_set(u16 *user_vol_tab, u8 user_vol_max);

void file_user_digital_volume_set(u8 vol);
u8 file_user_audio_digital_volume_get();
void file_user_digital_volume_tab_set(u16 *user_vol_tab, u8 user_vol_max);

void pc_user_digital_volume_set(u8 vol);
u8 pc_user_audio_digital_volume_get();
void pc_user_digital_volume_tab_set(u16 *user_vol_tab, u8 user_vol_max);

void spdif_user_digital_volume_set(u8 vol);
u8 spdif_user_audio_digital_volume_get();
void spdif_user_digital_volume_tab_set(u16 *user_vol_tab, u8 user_vol_max);


void reverb_user_digital_volume_set(u8 vol);
u8 reverb_user_audio_digital_volume_get();
void reverb_user_digital_volume_tab_set(u16 *user_vol_tab, u8 user_vol_max);
#endif
void reverb_set_dodge_threshold(int threshold_in, int threshold_out, u8 fade_tar, u8 dodge_en);


void a2dp_dec_output_set_start_bt_time(u32 bt_tmr);
/*void mix_out_high_bass(u32 cmd, struct high_bass *hb);
void mix_out_high_bass_dis(u32 cmd, u8 dis);*/

//////////////////////////////////////////////////////////////////////////////
static inline void audio_pcm_mono_to_dual(s16 *dual_pcm, s16 *mono_pcm, int points)
{
    s16 *mono = mono_pcm;
    int i = 0;
    u8 j = 0;

    for (i = 0; i < points; i++, mono++) {
        *dual_pcm++ = *mono;
        *dual_pcm++ = *mono;
    }
}


//////////////////////////////////////////////////////////////////////////////
#include "audio_dec_file.h"


#endif

