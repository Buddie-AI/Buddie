#ifndef __ADV_KEY_SETTING_H__
#define __ADV_KEY_SETTING_H__


#include "le_rcsp_adv_module.h"

#if RCSP_ADV_KEY_SET_ENABLE

void set_key_setting(u8 *key_setting_info);
void get_key_setting(u8 *key_setting_info);

#endif
#endif
