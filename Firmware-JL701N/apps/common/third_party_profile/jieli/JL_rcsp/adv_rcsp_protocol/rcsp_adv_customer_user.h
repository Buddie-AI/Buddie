#ifndef __JL_ADV_CUSTOMER_USER_H__
#define __JL_ADV_CUSTOMER_USER_H__

#include "app_config.h"
#include "typedef.h"
#include "system/event.h"

#include "le_common.h"
#include "spp_user.h"
#if (RCSP_ADV_EN)

void rcsp_user_recv_cmd_resp(u8 *data, u16 len);
void rcsp_user_recv_resp(u8 *data, u16 len);
u32 rcsp_user_send_resp_cmd(u8 *data, u16 len);

#endif
#endif
