#include "adapter_odev.h"
#include "adapter_odev_ble.h"
#include "app_config.h"
#include "btstack/le/att.h"
#include "btstack/le/le_user.h"
#include "ble_user.h"
#include "btcontroller_modules.h"
#if TCFG_WIRELESS_MIC_ENABLE

#define  LOG_TAG_CONST       WIRELESSMIC
#define  LOG_TAG             "[ODEV_BLE]"
#define  LOG_ERROR_ENABLE
#define  LOG_DEBUG_ENABLE
#define  LOG_INFO_ENABLE
#define  LOG_DUMP_ENABLE
#define  LOG_CLI_ENABLE
#include "debug.h"

struct _odev_ble odev_ble;
#define __this  (&odev_ble)

extern void ble_module_enable(u8 en);
extern void bt_ble_init(void);

static void ble_report_data_deal(att_data_report_t *report_data, target_uuid_t *search_uuid)
{
    /* dongle_ble_report_data_deal(report_data,search_uuid); */
    /* wireless_mic_ble_report_data_deal(report_data,search_uuid); */
    BLE_REPORT_DATA_DEAL(report_data, search_uuid);
}

static void adapter_event_callback(le_client_event_e event, u8 *packet, int size)
{
    /* dongle_adapter_event_callback(event,packet,size); */
    /* wireless_adapter_event_callback(event,packet,size); */
    ADAPTER_EVENT_CALLBACK(event, packet, size);
}

//client default 信息
client_conn_cfg_t odev_ble_conn_config = {
    .match_dev_cfg[0]       = NULL,      //匹配指定的名字
    .match_dev_cfg[1]       = NULL,
    .match_dev_cfg[2]       = NULL,
    .search_uuid_cnt        = 0,
    .security_en            = 0,       //加密配对
    .report_data_callback   = ble_report_data_deal,
    .event_callback         = adapter_event_callback,
};

int adapter_odev_ble_open(void *priv)
{
    r_printf("adapter_odev_ble_open\n");

    if (__this->status == ODEV_BLE_OPEN) {
        r_printf("adapter_odev_ble_already_open\n");
        return 0;
    }

    memset(__this, 0, sizeof(struct _odev_ble));

    __this->status = ODEV_BLE_OPEN;
    __this->match_type = MATCH_NULL;

    if (priv) {
        __this->parm = (struct _odev_ble_parm *)priv;

        if (__this->parm->cfg_t->match_dev_cfg[0]) {
            odev_ble_conn_config.match_dev_cfg[0] = __this->parm->cfg_t->match_dev_cfg[0];
        }

        if (__this->parm->cfg_t->match_dev_cfg[1]) {
            odev_ble_conn_config.match_dev_cfg[1] = __this->parm->cfg_t->match_dev_cfg[1];
        }

        if (__this->parm->cfg_t->match_dev_cfg[2]) {
            odev_ble_conn_config.match_dev_cfg[2] = __this->parm->cfg_t->match_dev_cfg[2];
        }

        if (__this->parm->cfg_t->search_uuid_table) {
            odev_ble_conn_config.search_uuid_table = __this->parm->cfg_t->search_uuid_table;
        }

        odev_ble_conn_config.search_uuid_cnt = __this->parm->cfg_t->search_uuid_cnt;
        odev_ble_conn_config.security_en = __this->parm->cfg_t->security_en;

        if (odev_ble_conn_config.match_dev_cfg[0]) {
            y_printf("match_dev_cfg[0] : %s\n", odev_ble_conn_config.match_dev_cfg[0]->compare_data);
        }
        if (odev_ble_conn_config.match_dev_cfg[1]) {
            y_printf("match_dev_cfg[1] : %s\n", odev_ble_conn_config.match_dev_cfg[1]->compare_data);
        }
        if (odev_ble_conn_config.match_dev_cfg[2]) {
            y_printf("match_dev_cfg[2] : %s\n", odev_ble_conn_config.match_dev_cfg[2]->compare_data);
        }
    }

#if BLE_CLIENT_EN || TRANS_MULTI_BLE_EN || BLE_WIRELESS_MIC_CLIENT_EN
    __this->ble_client_api = ble_get_client_operation_table();
    ASSERT(__this->ble_client_api, "%s : %s : %d\n", __FUNCTION__, "ble_client_api == NULL", __LINE__);

    __this->ble_client_api->init_config(0, &odev_ble_conn_config);

    extern void ble_vendor_set_hold_prio(u8 role, u8 enable);
    ble_vendor_set_hold_prio(0, 1);
#ifndef CONFIG_NEW_BREDR_ENABLE
    //优化多连接,卡音
    bredr_link_vendor_support_packet_enable(PKT_TYPE_2DH5_EU, 0);
#endif //CONFIG_NEW_BREDR_ENABLE

#endif //#if BLE_CLIENT_EN || TRANS_MULTI_BLE_EN || BLE_WIRELESS_MIC_CLIENT_EN

    return 0;
}

void adapter_odev_ble_search_device(void)
{
#if BLE_CLIENT_EN || TRANS_MULTI_BLE_EN || BLE_WIRELESS_MIC_CLIENT_EN
    ble_module_enable(1);
#endif
}

int adapter_odev_ble_start(void *priv)
{
    r_printf("adapter_odev_ble_start\n");

    if (__this->status == ODEV_BLE_START) {
        r_printf("adapter_odev_ble_already_start\n");
        return 0;
    }
    __this->status = ODEV_BLE_START;

#if BLE_CLIENT_EN || TRANS_MULTI_BLE_EN || BLE_WIRELESS_MIC_CLIENT_EN
    bt_ble_init();
#endif

    return 0;
}

int adapter_odev_ble_stop(void *priv)
{
    r_printf("adapter_odev_ble_stop\n");
    if (__this->status == ODEV_BLE_STOP) {
        r_printf("adapter_odev_ble_already_stop\n");
        return 0;
    }

    __this->status = ODEV_BLE_STOP;

#if BLE_CLIENT_EN || TRANS_MULTI_BLE_EN || BLE_WIRELESS_MIC_CLIENT_EN
    ble_module_enable(0);
#endif

    return 0;
}

int adapter_odev_ble_close(void *priv)
{
    r_printf("adapter_odev_ble_close\n");

    if (__this->status == ODEV_BLE_CLOSE) {
        r_printf("adapter_odev_ble_already_close\n");
        return 0;
    }
    __this->status = ODEV_BLE_CLOSE;

#if BLE_CLIENT_EN || TRANS_MULTI_BLE_EN || BLE_WIRELESS_MIC_CLIENT_EN
    ble_module_enable(0);
#endif

    __this->ble_client_api = NULL;
    return 0;
}


int adapter_odev_ble_send(void *priv, u8 *buf, u16 len)
{
    int ret = 0;
    if (!__this->ble_connected) {
        r_printf("adapter_odev_ble not connect\n");
        return -1;
    }
    ret = ADAPTER_ODEV_BLE_SEND(priv, buf, len);
    return ret;
}

int adapter_odev_ble_config(int cmd, void *parm)
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
