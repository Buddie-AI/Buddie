#ifndef A2DP_MEDIA_CODEC_H
#define A2DP_MEDIA_CODEC_H


#include "generic/typedef.h"

#define seqn_after(a, b)        ((s16)((s16)(b) - (s16)(a)) < 0)
#define seqn_before(a, b)       seqn_after(b, a)

extern int a2dp_media_get_packet(u8 **frame);

extern int a2dp_media_try_get_packet(u8 **frame);

extern int a2dp_media_get_remain_buffer_size();

extern int a2dp_media_get_remain_play_time(u8 include_tws);

extern int a2dp_media_get_total_data_len();

extern int a2dp_media_get_packet_num();

extern int a2dp_media_clear_packet_before_seqn(u16 seqn_number);

extern void *a2dp_media_fetch_packet(int *len, void *prev_packet);

extern void *a2dp_media_fetch_packet_and_wait(int *len, void *prev_packet, int msec);

extern void a2dp_media_free_packet(void *_packet);

extern int a2dp_media_channel_exist(void);

extern int a2dp_media_is_clearing_frame(void);

extern int a2dp_media_get_codec_type();

extern u32 a2dp_media_dump_rx_time(u8 *frame);
















#endif
