#ifndef __ADV_MIC_SETTING_H__
#define __ADV_MIC_SETTING_H__


#include "le_rcsp_adv_module.h"

#if RCSP_ADV_MIC_SET_ENABLE

void set_mic_setting(u8 mic_setting_info);
u8 get_mic_setting(void);
#endif

void rcsp_user_mic_fixed_deal(u8 mode);

#endif
