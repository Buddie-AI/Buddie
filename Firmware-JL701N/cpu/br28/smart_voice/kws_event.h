/*****************************************************************
>file name : kws_event.h
>author : lichao
>create time : Mon 01 Nov 2021 05:08:48 PM CST
*****************************************************************/
#ifndef _SMART_VOICE_KWS_EVENT_H_
#define _SMART_VOICE_KWS_EVENT_H_

#include "media/kws_event.h"



int smart_voice_kws_event_handler(u8 model, int kws);

void *smart_voice_kws_dump_open(int period_time);

void smart_voice_kws_dump_result_add(void *_ctx, u8 model, int kws);

void smart_voice_kws_dump_close(void *_ctx);

#endif
