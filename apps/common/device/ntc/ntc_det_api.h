#ifndef __NTC_DET_API_H__
#define __NTC_DET_API_H__

#include "typedef.h"
#include "app_config.h"

#if NTC_DET_EN
extern void ntc_det_start(void);
extern void ntc_det_stop(void);
extern u16 ntc_det_working();
#endif

#endif //__NTC_DET_API_H__

