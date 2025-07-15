#ifndef __ADV_WORK_SETTING_H__
#define __ADV_WORK_SETTING_H__


#include "le_rcsp_adv_module.h"

#if RCSP_ADV_WORK_SET_ENABLE

void set_work_setting(u8 work_setting_info);
u8 get_work_setting(void);

#endif
#endif
