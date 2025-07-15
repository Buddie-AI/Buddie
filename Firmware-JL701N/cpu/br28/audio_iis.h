/*****************************************************************
>file name : audio_iis.h
>create time : Fri 25 Feb 2022 04:35:37 PM CST
*****************************************************************/
#ifndef _AUDIO_IIS_H_
#define _AUDIO_IIS_H_
#include "system/includes.h"
#include "sound/pcm.h"
#include "asm/iis.h"
#include "audio_link.h"

struct audio_iis_hdl {
    u8 state;
    u8 hw_ch;
    u8 irq_trigger;
    u8 input_mapping;
    short delay_time;
    short prepared_time;
    short prepared_frames;
    short reserved_frames;
    short underrun_time;
    short underrun_pns;
    void *underrun_data;
    void (*underrun_feedback)(void *);
    void *trigger_data;
    void (*trigger_handler)(void *);
    int sample_rate;
    struct list_head sync_list;
    ALINK_PARM *alink0_param;
};

int audio_iis_pcm_tx_open(struct audio_iis_hdl *iis, u8 ch, int smaple_rate);

void audio_iis_pcm_tx_close(struct audio_iis_hdl *iis);

int audio_iis_pcm_remapping(struct audio_iis_hdl *iis, u8 mapping);

int audio_iis_pcm_write(struct audio_iis_hdl *iis, void *data, int len);

int audio_iis_dma_start(struct audio_iis_hdl *iis);

int audio_iis_dma_stop(struct audio_iis_hdl *iis);

int audio_iis_set_delay_time(struct audio_iis_hdl *iis, int prepared_time, int delay_time);

int audio_iis_set_underrun_params(struct audio_iis_hdl *iis, int time, void *priv, void (*feedback)(void *));

int audio_iis_trigger_interrupt(struct audio_iis_hdl *iis, int time, void *priv, void (*callback)(void *));

void audio_iis_add_syncts_handle(struct audio_iis_hdl *iis, void *syncts);

void audio_iis_remove_syncts_handle(struct audio_iis_hdl *iis, void *syncts);

void audio_iis_syncts_update_frame(struct audio_iis_hdl *iis);

int audio_iis_buffered_frames(struct audio_iis_hdl *iis);

int audio_iis_buffered_time(struct audio_iis_hdl *iis);

int audio_iis_pcm_channel_num(struct audio_iis_hdl *iis);

int audio_iis_pcm_sample_rate(struct audio_iis_hdl *iis);

int audio_aec_ref_src_data_fill(void *p, s16 *data, int len);
int audio_aec_ref_src_open(u32 insr, u32 outsr);
void audio_aec_ref_src_close();
#endif

