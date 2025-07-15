#ifndef __ADAPTER_ODEV_BLE_H__
#define __ADAPTER_ODEV_BLE_H__

#include "generic/typedef.h"
#include "le_client_demo.h"
#include "app_config.h"

#define DEVICE_RSSI_LEVEL        (-50)
#define POWER_ON_PAIR_TIME       (3500)//unit ms,切换搜索回连周期

struct _odev_ble_parm {
    client_conn_cfg_t *cfg_t;
};

enum {
    ODEV_BLE_CLOSE,
    ODEV_BLE_OPEN,
    ODEV_BLE_START,
    ODEV_BLE_STOP,
};

enum {
    MATCH_KEYBOARD,
    MATCH_MOUSE,
    MATCH_NULL,
};

struct _odev_ble {
    u16 ble_connected;
    u16 ble_timer_id;
    u8 status;
    u8 match_type;
    struct ble_client_operation_t *ble_client_api;
    struct _odev_ble_parm *parm;
};

int adapter_odev_ble_open(void *priv);
int adapter_odev_ble_start(void *priv);
int adapter_odev_ble_stop(void *priv);
int adapter_odev_ble_close(void *priv);
int adapter_odev_ble_config(int cmd, void *parm);
int adapter_odev_ble_send(void *priv, u8 *buf, u16 len);

void adapter_odev_ble_search_device(void);

//dongle
void dongle_ble_report_data_deal(att_data_report_t *report_data, target_uuid_t *search_uuid);
void dongle_adapter_event_callback(le_client_event_e event, u8 *packet, int size);
int dongle_ble_send(void *priv, u8 *buf, u16 len);

//wireless_mic
void wireless_mic_ble_report_data_deal(att_data_report_t *report_data, target_uuid_t *search_uuid);
void wireless_adapter_event_callback(le_client_event_e event, u8 *packet, int size);
int wireless_mic_ble_send(void *priv, u8 *buf, u16 len);

#define BLE_REPORT_DATA_DEAL(a,b)       wireless_mic_ble_report_data_deal(a,b)
#define ADAPTER_EVENT_CALLBACK(a,b,c)   wireless_adapter_event_callback(a,b,c)
#define ADAPTER_ODEV_BLE_SEND(a,b,c)    wireless_mic_ble_send(a,b,c)

extern struct _odev_ble odev_ble;
extern client_conn_cfg_t odev_ble_conn_config;

#endif//__ADAPTER_ODEV_BLE_H__

