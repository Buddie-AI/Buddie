#ifndef  __LP_TOUCH_KEY_ALOG_H__
#define  __LP_TOUCH_KEY_ALOG_H__


#include "typedef.h"


void TouchAlgo_Init(u8 ch, u16 min, u16 max);
void TouchAlgo_Update(u8 ch, u16 x);
void TouchAlgo_Reset(u8 ch, u16 min, u16 max);

u16 TouchAlgo_GetRange(u8 ch, u8 *valid);
void TouchAlgo_SetRange(u8 ch, u16 range);

s32 TouchAlgo_GetSigma(u8 ch);
void TouchAlgo_SetSigma(u8 ch, s32 sigma);


#endif  /*LP_TOUCH_KEY_ALOG_H*/

