#ifndef __ADAPTER_ODEV_BT_H__
#define __ADAPTER_ODEV_BT_H__

#include "generic/typedef.h"

#include "adapter_odev_edr.h"
#include "adapter_odev_ble.h"

enum {
    ODEV_EDR = 0x0,
    ODEV_BLE,
};

struct _odev_bt_parm {
    u8 mode;
    struct _odev_edr_parm edr_parm;
    struct _odev_ble_parm ble_parm;
};

struct _odev_bt {
    struct _odev_bt_parm *parm;
    struct _odev_edr *edr;
    struct _odev_ble *ble;
};
#endif//__ADAPTER_ODEV_BT_H__

