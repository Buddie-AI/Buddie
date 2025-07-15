#ifndef APP_POWER_MANAGE_H
#define APP_POWER_MANAGE_H

#include "typedef.h"
#include "system/event.h"

#define LOW_POWER_SHUTDOWN      300  //低电直接关机电压-拔出不开机-开盖不开机
#define LOW_POWER_OFF_VAL   	330  //低电关机电压
#define LOW_POWER_WARN_VAL   	340  //低电提醒电压
#define LOW_POWER_WARN_TIME   	(60 * 1000)  //低电提醒时间

#define DEVICE_EVENT_FROM_POWER		(('P' << 24) | ('O' << 16) | ('W' << 8) | '\0')

enum {
    POWER_EVENT_POWER_NORMAL,
    POWER_EVENT_POWER_WARNING,
    POWER_EVENT_POWER_LOW,
    POWER_EVENT_POWER_CHANGE,
    POWER_EVENT_SYNC_TWS_VBAT_LEVEL,
    POWER_EVENT_POWER_CHARGE,
};

int app_power_event_handler(struct device_event *dev);
void check_power_on_voltage(void);
u16 get_vbat_level(void);
u8 get_vbat_percent(void);
void vbat_check_init(void);
void vbat_timer_update(u32 msec);
void vbat_timer_delete(void);
void tws_sync_bat_level(void);
u8 get_tws_sibling_bat_level(void);
u8 get_tws_sibling_bat_persent(void);
bool get_vbat_need_shutdown(void);
u8  get_self_battery_level(void);

void app_power_set_tws_sibling_bat_level(u8 vbat, u8 percent);

#endif

