#ifndef __ADV_HIGH_LOW_VOL__
#define __ADV_HIGH_LOW_VOL__

#include "le_rcsp_adv_module.h"

#if RCSP_ADV_HIGH_LOW_SET

struct _HIGL_LOW_VOL {
	int low_vol;
	int high_vol;
};

void get_high_low_vol_info(u8 *vol_gain_param);
void set_high_low_vol_info(u8 *vol_gain_param);
void deal_high_low_vol(u8 *vol_gain_data, u8 write_vm, u8 tws_sync);

#endif
#endif
