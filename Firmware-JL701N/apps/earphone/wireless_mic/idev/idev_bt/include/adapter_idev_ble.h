#ifndef __ADAPTER_IDEV_BLE_H__
#define __ADAPTER_IDEV_BLE_H__

#include "generic/typedef.h"

enum {
    IDEV_BLE_CLOSE,
    IDEV_BLE_OPEN,
    IDEV_BLE_START,
    IDEV_BLE_STOP,
};

struct _idev_ble_parm {

};

struct _idev_ble {
    u8 status;
    struct ble_server_operation_t *ble_opt;
};

int adapter_idev_ble_open(void *priv);
int adapter_idev_ble_start(void *priv);
int adapter_idev_ble_stop(void *priv);
int adapter_idev_ble_close(void *priv);
int adapter_idev_ble_config(int cmd, void *parm);

extern struct _idev_ble idev_ble;

#endif//__ADAPTER_IDEV_BLE_H__

