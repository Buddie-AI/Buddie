#ifndef _AUD_DEC_EFF_H
#define _AUD_DEC_EFF_H
#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "classic/tws_api.h"
#include "classic/hci_lmp.h"
#include "media/eq_config.h"
#include "media/audio_surround.h"
#include "app_config.h"
#include "audio_config.h"
#include "app_main.h"
#include "media/audio_vbass.h"
#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif/*TCFG_AUDIO_ANC_ENABLE*/

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
#include "online_db_deal.h"
#endif

struct dec_sur {
#if AUDIO_SURROUND_CONFIG
    surround_hdl *surround;         //环绕音效句柄
    u8 surround_eff;  //音效模式记录
#endif

};

#if (defined(TCFG_AUDIO_OUT_EQ_ENABLE) && (TCFG_AUDIO_OUT_EQ_ENABLE != 0))
#define AUDIO_OUT_EQ_USE_SPEC_NUM		2	// 使用特定的eq段
#else
#define AUDIO_OUT_EQ_USE_SPEC_NUM		0
#endif
#define AUDIO_EQ_FADE_EN  1
#define HIGH_BASS_EQ_FADE_STEP  (1)

#if TCFG_EQ_ENABLE&&TCFG_AUDIO_OUT_EQ_ENABLE
#define AUDIO_OUT_EFFECT_ENABLE			1	// 音频输出时的音效处理
#else
#define AUDIO_OUT_EFFECT_ENABLE			0
#endif//TCFG_AUDIO_OUT_EQ_ENABLE
extern struct audio_mixer mixer;

typedef int (*eq_output_cb)(void *, void *, int);

struct eq_filter_fade {
    u16 tmr;
    int cur_gain[AUDIO_OUT_EQ_USE_SPEC_NUM];
    int use_gain[AUDIO_OUT_EQ_USE_SPEC_NUM];
};
struct dec_eq_drc {
    s16 *eq_out_buf;
    int eq_out_buf_len;
    int eq_out_points;
    int eq_out_total;

    void *priv;
    eq_output_cb  out_cb;
    void *drc_prev;
    void *eq;
    void *spk_eq;
    void *drc;
    u8 async;
    u8 drc_bef_eq;
    struct eq_filter_fade fade;
    u8 remain;
};

struct eq_parm_new {
    u8 in_mode: 2;
    u8 run_mode: 2;
    u8 data_in_mode: 2;
    u8 data_out_mode: 2;
};
void *audio_surround_setup(u8 channel, u8 eff);
void audio_surround_free(void *sur);
void audio_surround_set_ch(void *sur, u8 channel);
void audio_surround_voice(void *sur, u8 en);


vbass_hdl *audio_vbass_setup(u32 sample_rate, u8 channel);
void audio_vbass_free(vbass_hdl *vbass);


void *dec_eq_drc_setup(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en);
void dec_eq_drc_free(void *eff);


void *esco_eq_drc_setup(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en);
void esco_eq_drc_free(void *eff);


void *audio_out_eq_drc_setup(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en);
void audio_out_eq_drc_free(void *eff);
int audio_out_eq_set_gain(void *eff, u8 idx, int gain);

int eq_drc_run(void *priv, void *data, u32 len);

void mix_out_drc_open(u16 sample_rate);
void mix_out_drc_close();
void mix_out_drc_run(s16 *data, u32 len);
/*----------------------------------------------------------------------------*/
/**@brief    mix_out后限幅器系数更新
   @param    threadhold限幅器阈值，-60~0,单位db
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void mix_out_drc_threadhold_update(float threadhold);

struct audio_drc *esco_limiter_open(u16 sample_rate, u16 ch_num);
void esco_limiter_run(struct audio_drc *limiter, s16 *data, u32 len);
void esco_limiter_close(struct audio_drc *limiter);

void spk_eq_seg_update(struct eq_seg_info *seg);
void spk_eq_global_gain_udapte(float global_gain);
void *spk_eq_open(u32 sample_rate, u8 ch_num, u8 bitwide);
void spk_eq_close(void *eq);
void spk_eq_run(void *eq, void *in, void *out, u16 len);
int spk_eq_save_to_vm();
void spk_eq_read_from_vm();
void spk_eq_set_send_data_handler(void (*handler)(u8 seq, u8 *packet, u8 size));
int spk_eq_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size);
int spk_eq_spp_rx_packet(u8 *packet, u8 len);
int spk_eq_read_seg_l(u8 **buf);
int spk_eq_read_seg_r(u8 **buf);

void *audio_bass_boost_open(u32 sample_rate, u8 ch_num);
void audio_bass_boost_run(void *hdl, s16 *data, int len);
void audio_bass_boost_close(void *hdl);

extern struct virtual_bass_tool_set *get_vbass_parm();
#endif
