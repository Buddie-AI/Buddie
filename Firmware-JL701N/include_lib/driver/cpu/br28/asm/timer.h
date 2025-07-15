#ifndef __TIMER_H__
#define __TIMER_H__

#include "typedef.h"

void udelay(u32 us);
void mdelay(u32 us);
void delay_2ms(int cnt);
u32 timer_get_sec(void);
u32 timer_get_ms(void);

#endif
