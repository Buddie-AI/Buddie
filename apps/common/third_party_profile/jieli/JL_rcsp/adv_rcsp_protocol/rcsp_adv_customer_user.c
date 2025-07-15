#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "string.h"
#include "JL_rcsp_api.h"
#include "JL_rcsp_protocol.h"
#include "JL_rcsp_packet.h"
#include "spp_user.h"
#include "btstack/avctp_user.h"
#include "system/timer.h"
#include "app_core.h"
#include "user_cfg.h"
#include "asm/pwm_led.h"
#include "ui_manage.h"
#include "key_event_deal.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "classic/tws_api.h"
#include "event.h"
#include "bt_tws.h"
#include "custom_cfg.h"
#include "app_config.h"


#if (RCSP_ADV_EN)

/* #define RCSP_USER_DEBUG_EN */
#ifdef RCSP_USER_DEBUG_EN
#define rcsp_user_putchar(x)                putchar(x)
#define rcsp_user_printf                    printf
#define rcsp_user_printf_buf(x,len)         put_buf(x,len)
#else
#define rcsp_user_putchar(...)
#define rcsp_user_printf(...)
#define rcsp_user_printf_buf(...)
#endif

/// 发送需要回复的自定义命令
u32 rcsp_user_send_resp_cmd(u8 *data, u16 len)
{
    u32 ret = 0;
    rcsp_user_printf("send resp cmd\n");
    rcsp_user_printf_buf(data, len);
    ret = JL_CMD_send(JL_OPCODE_CUSTOMER_USER, data, len, 1);
    return ret;
}

void rcsp_user_recv_resp(u8 *data, u16 len)
{
    ///收到固件发出去,APP的回复
    rcsp_user_printf("resp resp\n");
    rcsp_user_printf_buf(data, len);
}


void rcsp_user_recv_cmd_resp(u8 *data, u16 len)
{
    ///收到app主动发过来的需要回复的自定义命令
    ///回复在其他地方已完成，开发不需要单独回复
    rcsp_user_printf("user recv cmd:\n");
    rcsp_user_printf_buf(data, len);
    /* rcsp_user_send_resp_cmd(data, len); */
}






#endif
