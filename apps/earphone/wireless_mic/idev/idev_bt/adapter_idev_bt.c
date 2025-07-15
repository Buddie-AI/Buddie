#include "adapter_idev.h"
#include "adapter_idev_bt.h"
#include "adapter_idev_ble.h"
#include "list.h"
#include "os/os_api.h"
#include "circular_buf.h"
#include "app_config.h"
#include "adapter_process.h"
#include "bt_edr_fun.h"
#if TCFG_WIRELESS_MIC_ENABLE

#define  LOG_TAG_CONST       WIRELESSMIC
#define  LOG_TAG             "[IDEV_BT]"
#define  LOG_ERROR_ENABLE
#define  LOG_DEBUG_ENABLE
#define  LOG_INFO_ENABLE
#define  LOG_DUMP_ENABLE
#define  LOG_CLI_ENABLE
#include "debug.h"

struct _idev_bt idev_bt;
#define __this  (&idev_bt)

static int adapter_idev_bt_open(void *parm)
{
    r_printf("adapter_idev_bt_open\n");

    memset(__this, 0, sizeof(struct _idev_bt));

    set_bt_dev_role(BT_AS_IDEV);

    __this->parm = (struct _idev_bt_parm *)parm;

    if (__this->parm->mode & BIT(IDEV_BLE)) {
        adapter_idev_ble_open(&__this->parm->ble_parm);
    }

    bt_init();

    __this->ble = &idev_ble;

    return 0;
}

static int adapter_idev_bt_start(struct adapter_media *media)
{
    printf("adapter_idev_bt_start\n");

    if (__this->parm->mode & BIT(IDEV_BLE)) {
        adapter_idev_ble_start(NULL);
    }

    return 0;
}

static int adapter_idev_bt_office(struct sys_event *event)
{
    int ret = 0;
    if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
        bt_connction_status_event_handler(&event->u.bt, __this);
    } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
        //bt_hci_event_handler(&event->u.bt, __this);
    }
    return ret;
}

static int adapter_idev_bt_event_deal(struct sys_event *event)
{
    //r_printf("adapter_odev_bt_event_deal\n");

    int ret = 0;
    switch (event->type) {
    case SYS_KEY_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        break;
    case SYS_BT_EVENT:
        adapter_idev_bt_office(event);
        ret = 1;
        break;
    default:
        break;
    }

    return ret;
}

static int adapter_idev_bt_close(void)
{
    r_printf("adapter_idev_bt_close\n");

    if (__this->parm->mode & BIT(IDEV_BLE)) {
        adapter_idev_ble_close(NULL);
    }

    return 0;
}

static int adapter_idev_bt_stop(void *priv)
{
    r_printf("adapter_idev_bt_stop\n");

    if (__this->parm->mode & BIT(IDEV_BLE)) {
        adapter_idev_ble_stop(NULL);
    }

    return 0;
}

REGISTER_ADAPTER_IDEV(adapter_idev_bt) = {
    .id         = ADAPTER_IDEV_BT,
    .open       = adapter_idev_bt_open,
    .close      = adapter_idev_bt_close,
    .start      = adapter_idev_bt_start,
    .stop       = adapter_idev_bt_stop,
    .event_fun  = adapter_idev_bt_event_deal,
};
#endif
