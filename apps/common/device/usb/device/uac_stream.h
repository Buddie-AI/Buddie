/*****************************************************************
>file name : usb_audio.h
>author : lichao
>create time : Wed 22 May 2019 10:39:35 AM CST
*****************************************************************/
#ifndef _UAC_STREAM_H_
#define _UAC_STREAM_H_
#include "typedef.h"

enum uac_event {
    USB_AUDIO_PLAY_OPEN = 0x0,
    USB_AUDIO_PLAY_CLOSE,
    USB_AUDIO_MIC_OPEN,
    USB_AUDIO_MIC_CLOSE,
    // USB_AUDIO_MUTE,
    USB_AUDIO_SET_PLAY_VOL,
    USB_AUDIO_SET_MIC_VOL,
};


void uac_speaker_stream_buf_clear(void);
u32 uac_speaker_stream_length();
u32 uac_speaker_stream_size();
void set_uac_speaker_rx_handler(void *priv, void (*rx_handler)(int, void *, int));
void set_uac_mic_tx_handler(void *priv, int (*tx_handler)(int, void *, int));
int uac_speaker_stream_sample_rate(void);

int uac_speaker_read(void *priv, void *data, u32 len);
u32 uac_speaker_get_alive();
void uac_speaker_set_alive(u8 alive);
#endif
