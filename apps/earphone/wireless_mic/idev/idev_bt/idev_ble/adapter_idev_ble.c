#include "adapter_idev.h"
#include "adapter_idev_bt.h"
#include "adapter_idev_ble.h"
#include "app_config.h"
#include "btstack/le/att.h"
#include "btstack/le/le_user.h"
#include "ble_user.h"
#include "btcontroller_modules.h"
#include "adapter_process.h"
#include "adapter_wireless_dec.h"
#if TCFG_WIRELESS_MIC_ENABLE

#define  LOG_TAG_CONST       WIRELESSMIC
#define  LOG_TAG             "[IDEV_BLE]"
#define  LOG_ERROR_ENABLE
#define  LOG_DEBUG_ENABLE
#define  LOG_INFO_ENABLE
#define  LOG_DUMP_ENABLE
#define  LOG_CLI_ENABLE
#include "debug.h"

struct _idev_ble idev_ble;
#define __this  (&idev_ble)

extern void bt_ble_adv_enable(u8 enable);
extern void bt_ble_init(void);

int adpter_ble_send_data(u8 *data, u16 len)
{
    if (__this->ble_opt) {
        u32 vaild_len = __this->ble_opt->get_buffer_vaild(0);
        if (vaild_len >= len) {
            return __this->ble_opt->send_data(NULL, data, len);
        }
    }
    return 0;
}

static void adapter_ble_send_wakeup(void)
{
    //putchar('W');
}

static void adapter_ble_status_callback(void *priv, ble_state_e status)
{
    switch (status) {
    case BLE_ST_IDLE:
        break;
    case BLE_ST_ADV:
        break;
    case BLE_ST_CONNECT:
        break;
    case BLE_ST_DISCONN:
    case BLE_ST_SEND_DISCONN:
        adapter_process_event_notify(ADAPTER_EVENT_IDEV_MEDIA_CLOSE, 0);
        break;
    case BLE_ST_NOTIFY_IDICATE:
        break;
    case BLE_ST_CONNECTION_UPDATE_OK:
        adapter_process_event_notify(ADAPTER_EVENT_IDEV_MEDIA_OPEN, 0);
        break;
    default:
        break;
    }
}

static void adapter_ble_recieve_cbk(void *priv, u8 *buf, u16 len)
{
    //r_printf("adapter --- ble_api_rx(%d) \n", len);
    //printf_buf(buf, len);
    adapter_wireless_dec_frame_write(buf, len);
}

int adapter_idev_ble_open(void *priv)
{
    r_printf("adapter_idev_ble_open\n");
    if (__this->status == IDEV_BLE_OPEN) {
        g_printf("adapter_idev_ble_already_open\n");
        return 0;
    }

    memset(__this, 0, sizeof(struct _idev_ble));

#if TRANS_DATA_EN || BLE_WIRELESS_MIC_SERVER_EN
    ble_get_server_operation_table(&__this->ble_opt);
    __this->ble_opt->regist_recieve_cbk(0, adapter_ble_recieve_cbk);
    __this->ble_opt->regist_state_cbk(0, adapter_ble_status_callback);
    __this->ble_opt->regist_wakeup_send(NULL, adapter_ble_send_wakeup);
#endif

    __this->status = IDEV_BLE_OPEN;

    return 0;
}

int adapter_idev_ble_start(void *priv)
{
    r_printf("adapter_idev_ble_start\n");

    if (__this->status == IDEV_BLE_START) {
        r_printf("adapter_idev_ble_already_start\n");
        return 0;
    }
    __this->status = IDEV_BLE_START;

#if TRANS_DATA_EN || BLE_WIRELESS_MIC_SERVER_EN
    bt_ble_init();
#endif
    return 0;
}

int adapter_idev_ble_stop(void *priv)
{
    r_printf("adapter_idev_ble_stop\n");
    if (__this->status == IDEV_BLE_STOP) {
        r_printf("adapter_idev_ble_already_stop\n");
        return 0;
    }
    __this->status = IDEV_BLE_STOP;

#if BLE_CLIENT_EN || TRANS_MULTI_BLE_EN || BLE_WIRELESS_MIC_SERVER_EN
    ble_module_enable(0);
#endif
    return 0;
}

int adapter_idev_ble_close(void *priv)
{
    r_printf("adapter_idev_ble_close\n");

    if (__this->status == IDEV_BLE_CLOSE) {
        r_printf("adapter_idev_ble_already_close\n");
        return 0;
    }
    __this->status = IDEV_BLE_CLOSE;

#if BLE_CLIENT_EN || TRANS_MULTI_BLE_EN || BLE_WIRELESS_MIC_SERVER_EN
    ble_module_enable(0);
#endif

    return 0;
}

int adapter_idev_ble_config(int cmd, void *parm)
{
    switch (cmd) {
    case 0:
        break;
    default:
        break;
    }

    return 0;
}
#endif
