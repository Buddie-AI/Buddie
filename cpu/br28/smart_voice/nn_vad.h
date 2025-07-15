/*****************************************************************
>file name : nn_vad.h
>create time : Fri 17 Dec 2021 02:36:53 PM CST
*****************************************************************/
#ifndef _AUDIO_NN_VAD_H_
#define _AUDIO_NN_VAD_H_

void *audio_nn_vad_open(void);

int audio_nn_vad_data_handler(void *vad, void *data, int len);

void audio_nn_vad_close(void *vad);

#endif

