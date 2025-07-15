
#include "app_config.h"
#include "app_action.h"

#include "system/includes.h"
#include "spp_user.h"
#include "string.h"
#include "circular_buf.h"
#include "bt_common.h"
#include "online_db_deal.h"
#include "spp_online_db.h"

#if (TCFG_ANC_TOOL_DEBUG_ONLINE && TCFG_AUDIO_ANC_ENABLE)
#include "app_anctool.h"
#endif


#if 1
extern void printf_buf(u8 *buf, u32 len);
#define log_info          printf
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

#if APP_ONLINE_DEBUG

static struct db_online_api_t *db_api;
static struct spp_operation_t *spp_api = NULL;
static u8 spp_state;

int online_spp_send_data(u8 *data, u16 len)
{
    if (spp_api) {
        /* log_info("spp_api_tx(%d) \n", len); */
        /* log_info_hexdump(data, len); */
        return spp_api->send_data(NULL, data, len);
    }
    return SPP_USER_ERR_SEND_FAIL;
}

int online_spp_send_data_check(u16 len)
{
    if (spp_api) {
        if (spp_api->busy_state()) {
            return 0;
        }
    }
    return 1;
}

static void online_spp_state_cbk(u8 state)
{
    spp_state = state;
    switch (state) {
    case SPP_USER_ST_CONNECT:
        log_info("SPP_USER_ST_CONNECT ~~~\n");
        db_api->init(DB_COM_TYPE_SPP);
        db_api->register_send_data(online_spp_send_data);
#if (TCFG_ANC_TOOL_DEBUG_ONLINE && TCFG_AUDIO_ANC_ENABLE)
        app_anctool_spp_connect();
#endif
        break;

    case SPP_USER_ST_DISCONN:
        log_info("SPP_USER_ST_DISCONN ~~~\n");
        db_api->exit();
#if (TCFG_ANC_TOOL_DEBUG_ONLINE && TCFG_AUDIO_ANC_ENABLE)
        app_anctool_spp_disconnect();
#endif
        break;

    default:
        break;
    }

}

static void online_spp_send_wakeup(void)
{
    /* putchar('W'); */
    db_api->send_wake_up();
}

static void online_spp_recieve_cbk(void *priv, u8 *buf, u16 len)
{
    log_info("spp_api_rx(%d) \n", len);
    /* log_info_hexdump(buf, len); */

#if (TCFG_ANC_TOOL_DEBUG_ONLINE && TCFG_AUDIO_ANC_ENABLE)
    if (app_anctool_spp_rx_data(buf, len)) {
        return;
    }
#endif
#if (TCFG_CFG_TOOL_ENABLE || TCFG_ONLINE_ENABLE)
#if (TCFG_COMM_TYPE == TCFG_SPP_COMM)
    extern int cfg_tool_online_parse(u8 * buf, u32 len);
    if ((buf[1] == 0x12) || (buf[1] == 0x05) || (buf[1] == 0x11)) {
        if (cfg_tool_online_parse(buf, len)) {
            return;
        }
    }
#endif
#endif
    /* log_info("spp_rx_channel_handle"); */
    db_api->packet_handle(buf, len);

    //loop send data for test
    /* if (online_spp_send_data_check(len)) { */
    /* online_spp_send_data(buf, len); */
    /* } */
}

void online_spp_init(void)
{
    log_info("spp_file: %s", __FILE__);
    spp_state = 0;
    spp_get_operation_table(&spp_api);
    spp_api->regist_recieve_cbk(0, online_spp_recieve_cbk);
    spp_api->regist_state_cbk(0, online_spp_state_cbk);
    spp_api->regist_wakeup_send(NULL, online_spp_send_wakeup);
    db_api = app_online_get_api_table();
}

#else

void online_spp_init(void)
{
}

#endif

