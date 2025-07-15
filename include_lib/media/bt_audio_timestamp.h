/*****************************************************************
>file name : bt_audio_timestamp.h
>create time : Fri 21 May 2021 03:05:35 PM CST
*****************************************************************/
#ifndef _BT_AUDIO_TIMESTAMP_H_
#define _BT_AUDIO_TIMESTAMP_H_
#include "typedef.h"

#define TWS_A2DP_AUDIO      0
#define TWS_ESCO_AUDIO      1
#define TWS_FILE_AUDIO      2
#define TWS_SHARE_AUDIO     3
#define TWS_UNKOWN_AUDIO    8

/*===================================A2DP 音频时间戳接口==================================*/
void *a2dp_audio_timestamp_create(int sample_rate, u32 timestamp, u8 factor);

u32 a2dp_audio_update_timestamp(void *handle, u16 seqn, u32 dts);

void a2dp_audio_delay_offset_update(void *handle, int distance);

int a2dp_audio_sample_rate(void *handle);

bool a2dp_audio_timestamp_is_available(void *handle, u16 seqn, u32 dts, int *drop);

int tws_a2dp_share_timestamp(void *handle);

void a2dp_audio_set_base_time(void *handle, u32 base_time);

void a2dp_audio_timestamp_close(void *handle);

int a2dp_tws_audio_conn_offline(void);

void a2dp_tws_audio_conn_delete(void);

/*========================================================================================*/

/*===================================ESCO 音频时间戳接口==================================*/
void *esco_audio_timestamp_create(u8 frame_clkn, u32 delay_time, u8 factor);

u32 esco_audio_timestamp_update(void *handle, u32 time);

void esco_audio_timestamp_close(void *handle);
/*========================================================================================*/


/*===================================FILE 音频时间戳接口==================================*/
void *file_audio_timestamp_create(u32 magic, int sample_rate, u32 timestamp, u32 delay_time, u8 factor);

bool tws_file_timestamp_available(void *handle);

u32 file_audio_timestamp_update(void *handle, u32 dts);

void file_audio_timestamp_close(void *handle);
/*========================================================================================*/
#endif
