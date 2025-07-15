#ifndef __ADV_BT_NAME_SETTING_H__
#define __ADV_BT_NAME_SETTING_H__

#include "le_rcsp_adv_module.h"

#if RCSP_ADV_NAME_SET_ENABLE

void set_bt_name_setting(u8 *bt_name_setting);
void get_bt_name_setting(u8 *bt_name_setting);

#endif
#endif
