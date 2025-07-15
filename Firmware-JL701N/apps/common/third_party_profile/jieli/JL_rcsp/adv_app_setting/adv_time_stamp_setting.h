#ifndef __ADV_TIME_STAMP_SETTING_H__
#define __ADV_TIME_STAMP_SETTING_H__

void set_adv_time_stamp(u32 time_stamp);
u32 get_adv_time_stamp(void);
void deal_sibling_time_stamp_setting_switch(void *data, u16 len);
extern void sync_setting_by_time_stamp(void);
void deal_adv_setting_gain_time_stamp(void);

#endif
