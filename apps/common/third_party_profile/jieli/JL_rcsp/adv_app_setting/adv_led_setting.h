#ifndef __ADV_LED_SETTING_H__
#define __ADV_LED_SETTING_H__


#include "le_rcsp_adv_module.h"

#if RCSP_ADV_LED_SET_ENABLE

void set_led_setting(u8 *led_setting_info);
void get_led_setting(u8 *led_setting_info);

#endif
#endif
