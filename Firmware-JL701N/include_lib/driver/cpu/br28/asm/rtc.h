
#ifndef __RTC_H__
#define __RTC_H__

#include "typedef.h"
#include "system/sys_time.h"


struct rtc_dev_platform_data {
    const struct sys_time *default_sys_time;
    const struct sys_time *default_alarm;
    void (*cbfun)(u8);
    u8 clk_sel;
    u8 x32xs;
};

#define RTC_DEV_PLATFORM_DATA_BEGIN(data) \
	const struct rtc_dev_platform_data data = {

#define RTC_DEV_PLATFORM_DATA_END()  \
    .x32xs = 0, \
};

extern const struct device_operations rtc_dev_ops;

int rtc_init(const struct rtc_dev_platform_data *arg);
int rtc_ioctl(u32 cmd, u32 arg);
void set_alarm_ctrl(u8 set_alarm);
void write_sys_time(struct sys_time *curr_time);
void read_sys_time(struct sys_time *curr_time);
void write_alarm(struct sys_time *alarm_time);
void read_alarm(struct sys_time *alarm_time);

u16 month_to_day(u16 year, u8 month);
void day_to_ymd(u16 day, struct sys_time *sys_time);
u16 ymd_to_day(struct sys_time *time);
u8 caculate_weekday_by_time(struct sys_time *r_time);

#endif // __RTC_API_H__
