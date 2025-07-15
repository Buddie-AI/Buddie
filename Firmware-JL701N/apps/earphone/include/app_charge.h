#ifndef _APP_CHARGE_H_
#define _APP_CHARGE_H_

#include "typedef.h"
#include "system/event.h"

extern void charge_close_deal(void);
extern void charge_start_deal(void);
extern void ldo5v_keep_deal(void);
extern void charge_full_deal(void);
extern void charge_ldo5v_in_deal(void);
extern void charge_ldo5v_off_deal(void);
extern u8 get_charge_full_flag(void);

extern int app_charge_event_handler(struct device_event *dev);

#endif    //_APP_CHARGE_H_
