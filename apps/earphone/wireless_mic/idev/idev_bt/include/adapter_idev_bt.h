#ifndef __ADAPTER_IDEV_BT_H__
#define __ADAPTER_IDEV_BT_H__

#include "generic/typedef.h"

#include "adapter_idev_ble.h"

enum {
    IDEV_EDR = 0x0,
    IDEV_BLE,
};

struct _idev_bt_parm {
    u8 mode;
    struct _idev_ble_parm ble_parm;
};

struct _idev_bt {
    struct _idev_bt_parm *parm;
    struct _idev_ble *ble;
};

#endif//__ADAPTER_IDEV_BT_H__

