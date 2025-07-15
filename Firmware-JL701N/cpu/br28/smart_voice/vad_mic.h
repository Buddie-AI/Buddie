/*****************************************************************
>file name : vad_mic.h
>author : lichao
>create time : Mon 01 Nov 2021 05:07:50 PM CST
*****************************************************************/
#ifndef _P11_VAD_H_
#define _P11_VAD_H_
#include "includes.h"
#include "asm/power/p11.h"
#include "audio_adc.h"

//--------------------- p11 VAD参数配置 ---------------------------------//
struct avad_config {
    int avad_quantile_p;
    int avad_th;
    int avad_gain_db;
    int avad_compare_v;
};

struct dvad_config {
    int d_low_con_th;
    int d_high_con_th;
    int d2a_th_db;
    int d2a_frame_con;
    int dvad_gain_id;
    int d_frame_con;
    int d_stride1;
    int d_stride2;
};

#define P11_VAD_IRQ_ENABLE()    \
    P11_SYSTEM->M2P_INT_IE |= BIT(M2P_VAD_INDEX);

//===========================================================================//
//                          Master to P11 EVENT POST                         //
//===========================================================================//
enum M2P_VAD_CMD_TABLE {
    M2P_VAD_CMD_INIT = 0x55,
    M2P_VAD_CMD_FRAME,
    M2P_VAD_CMD_CLOSE,
    M2P_VAD_CMD_TEST,
};

//===========================================================================//
//                          P11 VAD EVENT CMD                                //
//===========================================================================//
enum P2M_VAD_CMD_TABLE {
    P2M_VAD_TRIGGER_START = 0xAA,
    P2M_VAD_TRIGGER_DMA,
    P2M_VAD_TRIGGER_STOP,
};

static inline void audio_vad_m2p_event_post(enum M2P_VAD_CMD_TABLE cmd)
{
    M2P_MESSAGE_VAD_CMD = cmd;

    P11_M2P_INT_SET = BIT(M2P_VAD_INDEX);
}

#define AUDIO_VAD_DMA_READ_UPDATE(n) \
    M2P_MESSAGE_VAD_CBUF_RPTR = (n); \
    audio_vad_m2p_event_post(M2P_VAD_CMD_FRAME);

void audio_vad_clock_trim(void);

int lp_vad_mic_data_init(struct vad_mic_platform_data *mic_data);

void *lp_vad_mic_open(void *priv, int (*dma_output)(void *priv, s16 *data, int len));

void lp_vad_mic_close(void *vad);

void lp_vad_mic_test(void);

void lp_vad_mic_disable(void);
#endif
