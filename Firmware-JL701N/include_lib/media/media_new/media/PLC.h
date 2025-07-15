#ifndef _PLC_H_
#define _PLC_H_

#include "generic/typedef.h"

u32 PLC_query(void);
s8 PLC_init(void *pbuf);
void PLC_exit(void);
void PLC_run(s16 *inbuf, s16 *outbuf, u16 point, u8 repair_flag);

#endif
