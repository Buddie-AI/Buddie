#ifndef TWS_DATA_TRANS_H
#define TWS_DATA_TRANS_H

#include "generic/typedef.h"

enum {
    TWS_DATA_TRANS_SOURCE,
    TWS_DATA_TRANS_SINK,
};

enum tws_data_trans_attr {
    TWS_DTC_LOCAL_MEDIA,
    TWS_DTC_MIC_REC,
};

u8 tws_api_data_trans_open(u8 channel, enum tws_data_trans_attr attr, u16 buf_size);


int tws_api_data_trans_start(u8 channel, u8 *arg, u8 len);


int tws_api_data_trans_stop(u8 channel);


int tws_api_data_trans_close(u8 channel);


void tws_api_data_trans_auto_drop(u8 channel, int enable);

int tws_api_data_trans_send(u8 channel, u8 *buf, int len);

void *tws_api_data_trans_buf_alloc(u8 channel, int len);

int tws_api_data_trans_push(u8 channel, void *_frame, int len);

void *tws_api_data_trans_pop(u8 channel, int *len);

void tws_api_data_trans_free(u8 channel, void *_frame);

void *tws_api_data_trans_fetch(u8 channel, void *_prev, int *len);

void tws_api_data_trans_clear(u8 channel);

int tws_api_data_trans_check(u8 channel, u16 *ready_len, u16 *total_len);

#endif

