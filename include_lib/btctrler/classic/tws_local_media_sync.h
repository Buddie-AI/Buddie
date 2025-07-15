#ifndef TWS_LOCAL_MEDIA_SYNC_H
#define TWS_LOCAL_MEDIA_SYNC_H

#include "generic/list.h"
#include "generic/typedef.h"



void tws_api_local_media_trans_start();

void tws_api_local_media_trans_set_buf(void *buf, int size);

void *tws_api_local_media_trans_alloc(int len);

void tws_api_local_media_trans_free(void *frame);

void tws_api_local_media_trans_push(void *frame, int len);

void *tws_api_local_media_trans_pop(int *len);

void *tws_api_local_media_trans_fetch(void *prev, int *len);

void tws_api_local_media_trans_stop();

void tws_api_local_media_trans_packet_del(void *_frame);
int tws_api_local_media_trans_check_total(u8 head);
int tws_api_local_media_trans_check_ready_total(void);
void tws_api_local_media_trans_clear(void);
int tws_api_local_media_packet_cnt(u8 *rx_packet_cnt, u8 *wait_send_pcaket_cnt);

// 填数超过了一定值才可以发送
void tws_api_local_media_set_limit_size(int size);

int tws_api_local_media_trans_bulk_push(u8 *buf, int len);

int tws_api_local_media_push_with_sequence(u8 *buf, int len, u16 seqn);

int tws_api_local_media_push_with_timestamp(u8 *buf, int len, u32 timestamp);

int tws_api_local_media_trans_open(u16 buf_size);

int tws_api_local_media_dec_start(u8 *arg, u8 len);

int tws_api_local_media_dec_stop();

void tws_api_auto_drop_frame_enable(int enable);
#endif

