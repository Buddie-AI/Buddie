/*****************************************************************
>file name : spatial_effect_tws.h
>create time : Fri 08 Apr 2022 07:55:01 PM CST
*****************************************************************/
#ifndef _SPATIAL_EFFECT_TWS_H_
#define _SPATIAL_EFFECT_TWS_H_
#include "system/includes.h"
#include "classic/tws_api.h"

void *spatial_tws_create_connection(int period, void *priv, void (*handler)(void *, void *data, int len));

void spatial_tws_delete_connection(void *conn);

int spatial_tws_audio_data_sync(void *conn, void *buf, int len);

#endif/*_SPATIAL_EFFECT_TWS_H_*/

