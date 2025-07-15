#ifndef __RCSP_ADV_TWS_SYNC_H__
#define __RCSP_ADV_TWS_SYNC_H__

#include "classic/tws_api.h"

#define TWS_FUNC_ID_ADV_SETTING_SYNC \
	TWS_FUNC_ID('R', 'C', 'S', 'P')
#define TWS_FUNC_ID_TIME_STAMP_SYNC \
	TWS_FUNC_ID('R' + 'C' + 'S' + 'P', \
			    'A' + 'D' + 'V', \
				'T' + 'I' + 'M' + 'E', \
				'S' + 'T' + 'A' + 'M' + 'P')
#endif
