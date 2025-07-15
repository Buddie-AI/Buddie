#ifndef _UIMIDIGI_
#define _UIMIDIGI_

#include "typedef.h"
#include "system/event.h"

void ldo_port_wakeup_to_cmessage(void);
void umidigi_chargestore_message_callback(void (*app_umidigi_chargetore_message_deal)(u16 message));

/*
 * 收集数据定时器采集周期根据充电舱发送波形设置
 * 充电舱发送波形有40ms/bit和20ms/bit两种
 *
 * #define _20MS_BIT			21
 * #define _40MS_BIT			41
 * #define TIMER2CMESSAGE 		_20MS_BIT
*/

#endif
