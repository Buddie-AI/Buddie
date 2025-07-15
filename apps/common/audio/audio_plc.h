#ifndef _AUDIO_PLC_H_
#define _AUDIO_PLC_H_

#include "generic/typedef.h"

int audio_plc_open(u16 sr);
void audio_plc_run(s16 *dat, u16 len, u8 repair_flag);
int audio_plc_close(void);

#endif/*_AUDIO_PLC_H_*/
