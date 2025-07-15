#ifndef _AUD_SIDETONE_H_
#define _AUD_SIDETONE_H_

#include "generic/typedef.h"
#include "board_config.h"

void audio_sidetone_inbuf(s16 *data, u16 len);
int audio_sidetone_open(void);
int audio_sidetone_close(void);
int audio_sidetone_suspend(void);
#endif

