
#include "app_config.h"
#include "app_action.h"

#include "system/includes.h"
#include "spp_user.h"
#include "string.h"
#include "circular_buf.h"
#include "3th_profile_api.h"
#include "bt_common.h"


#if 1
extern void printf_buf(u8 *buf, u32 len);
#define log_info          printf
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

/* #define DEBUG_ENABLE */
/* #include "debug_log.h" */

/* #include "rcsp_spp_user.h" */
#if (RCSP_ADV_EN)

static struct spp_operation_t *spp_api = NULL;
static u8 spp_state;

int rcsp_spp_send_data(u8 *data, u16 len)
{
    if (spp_api) {
        return spp_api->send_data(NULL, data, len);
    }
    return SPP_USER_ERR_SEND_FAIL;
}

int rcsp_spp_send_data_check(u16 len)
{
    if (spp_api) {
        if (spp_api->busy_state()) {
            return 0;
        }
    }
    return 1;
}

static void rcsp_spp_state_cbk(u8 state)
{
    spp_state = state;
    switch (state) {
    case SPP_USER_ST_CONNECT:
        log_info("SPP_USER_ST_CONNECT ~~~\n");
        set_app_connect_type(TYPE_SPP);
        break;

    case SPP_USER_ST_DISCONN:
        log_info("SPP_USER_ST_DISCONN ~~~\n");
        set_app_connect_type(TYPE_NULL);
        break;

    default:
        break;
    }

}

static void rcsp_spp_send_wakeup(void)
{
    putchar('W');
}

static void rcsp_spp_recieve_cbk(void *priv, u8 *buf, u16 len)
{
    log_info("spp_api_rx ~~~\n");
    log_info_hexdump(buf, len);
}

void rcsp_spp_init(void)
{
    spp_state = 0;
    spp_get_operation_table(&spp_api);
    spp_api->regist_recieve_cbk(0, rcsp_spp_recieve_cbk);
    spp_api->regist_state_cbk(0, rcsp_spp_state_cbk);
    spp_api->regist_wakeup_send(NULL, rcsp_spp_send_wakeup);
}

#endif


