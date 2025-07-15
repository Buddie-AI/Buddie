#ifndef __ADV_EQ_SETTING_H__
#define __ADV_EQ_SETTING_H__

#include "le_rcsp_adv_module.h"

#if defined(RCSP_ADV_EQ_SET_ENABLE) && RCSP_ADV_EQ_SET_ENABLE

struct _EQ_INFO {
    u8 mode;
    s8 gain_val[10];
};

void set_eq_setting(u8 *eq_setting);
void get_eq_setting(u8 *eq_setting);
void deal_eq_setting(u8 *eq_setting, u8 write_vm, u8 tws_sync);
u8 app_get_eq_info(u8 *get_eq_info);
u8 app_get_eq_all_info(u8 *get_eq_info);

#endif
#endif
