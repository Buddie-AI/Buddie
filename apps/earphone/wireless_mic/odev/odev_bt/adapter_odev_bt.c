#include "adapter_odev.h"
#include "adapter_odev_bt.h"
#include "adapter_odev_edr.h"
#include "adapter_odev_ble.h"
#include "btstack/avctp_user.h"
#include "classic/hci_lmp.h"
#include "list.h"
#include "os/os_api.h"
#include "circular_buf.h"
#include "app_config.h"
#include "adapter_process.h"
#include "btstack/bluetooth.h"
#include "btstack/btstack_error.h"
#include "bt_edr_fun.h"

#if TCFG_WIRELESS_MIC_ENABLE
#define  LOG_TAG_CONST       WIRELESSMIC
#define  LOG_TAG             "[ODEV_BT]"
#define  LOG_ERROR_ENABLE
#define  LOG_DEBUG_ENABLE
#define  LOG_INFO_ENABLE
#define  LOG_DUMP_ENABLE
#define  LOG_CLI_ENABLE
#include "debug.h"


struct _odev_bt odev_bt;
#define __this  (&odev_bt)

static int adapter_odev_bt_open(void *parm)
{
    r_printf("adapter_odev_bt_open\n");

    memset(__this, 0, sizeof(struct _odev_bt));

    set_bt_dev_role(BT_AS_ODEV);

    __this->parm = (struct _odev_bt_parm *)parm;

#if 0   //parm test
    g_printf("bt filt:%d spp filt:%d addr filt:%d\n", __this->parm->edr_parm.bt_name_filt_num, __this->parm->edr_parm.spp_name_filt_num, __this->parm->edr_parm.bd_addr_filt_num);
    for (u8 i = 0; i < __this->parm->edr_parm.bt_name_filt_num; i++) {
        printf("%d:%s\n", i, __this->parm->edr_parm.bt_name_filt[i]);
    }
    for (u8 i = 0; i < __this->parm->edr_parm.spp_name_filt_num; i++) {
        printf("%d:%s\n", i, __this->parm->edr_parm.spp_name_filt[i]);
    }
    for (u8 i = 0; i < __this->parm->edr_parm.bd_addr_filt_num; i++) {
        put_buf(__this->parm->edr_parm.bd_addr_filt[i], 6);
    }
#endif

    if (__this->parm->mode & BIT(ODEV_BLE)) {
        adapter_odev_ble_open(&__this->parm->ble_parm);
    }

    bt_init();

    if (__this->parm->mode & BIT(ODEV_EDR)) {
#if 0	//for test,config edr_bt_name by config_tool
        extern const char *bt_get_local_name();
        memset(__this->parm->edr_parm.bt_name_filt[0], 0x00, LOCAL_NAME_LEN);
        memcpy(__this->parm->edr_parm.bt_name_filt[0], (u8 *)(bt_get_local_name()), LOCAL_NAME_LEN);
#endif
        adapter_odev_edr_open(&__this->parm->edr_parm);
    }

    __this->edr = &odev_edr;
    __this->ble = &odev_ble;
    return 0;
}

static int adapter_odev_bt_start(void *priv, struct adapter_media *media)
{
    r_printf("adapter_odev_bt_start\n");
    if (__this->parm->mode & BIT(ODEV_EDR)) {
        adapter_odev_edr_start(NULL);
    }

    if (__this->parm->mode & BIT(ODEV_BLE)) {
        adapter_odev_ble_start(NULL);
    }

    return 0;
}

static int adapter_odev_bt_stop(void *priv)
{
    r_printf("adapter_odev_bt_stop\n");

    if (__this->parm->mode & BIT(ODEV_EDR)) {
        adapter_odev_edr_stop(NULL);
    }

    if (__this->parm->mode & BIT(ODEV_BLE)) {
        adapter_odev_ble_stop(NULL);
    }

    return 0;
}

static int adapter_odev_bt_close(void)
{
    r_printf("adapter_odev_bt_close\n");

    if (__this->parm->mode & BIT(ODEV_EDR)) {
        adapter_odev_edr_close(NULL);
    }

    if (__this->parm->mode & BIT(ODEV_BLE)) {
        adapter_odev_ble_close(NULL);
    }

    return 0;
}

static int adapter_odev_bt_get_status(void *priv)
{
    if (__this->parm->mode & BIT(ODEV_EDR)) {
        return adapter_odev_edr_get_status(NULL);
    }
    return 0;
}

static int adapter_odev_bt_pp_ctl(u8 pp)
{
    if (__this->parm->mode & BIT(ODEV_EDR)) {
        return  adapter_odev_edr_pp(pp);
    }

    return 0;
}

static int adapter_odev_bt_media_prepare(u8 mode, int (*fun)(void *, u8, u8, void *), void *priv)
{
    if (__this->parm->mode & BIT(ODEV_EDR)) {
        return adapter_odev_edr_media_prepare(mode, fun, priv);
    }
    return -1;
}

static int adapter_odev_bt_office(struct sys_event *event)
{
    int ret = 0;
    if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
        bt_connction_status_event_handler(&event->u.bt, __this);
    } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
        //bt_hci_event_handler(&event->u.bt, __this);
    }
    return ret;
}

static int adapter_odev_bt_event_deal(struct sys_event *event)
{
    //r_printf("adapter_odev_bt_event_deal\n");

    int ret = 0;
    switch (event->type) {
    case SYS_KEY_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        break;
    case SYS_BT_EVENT:
        adapter_odev_bt_office(event);
        ret = 1;
        break;
    default:
        break;
    }

    return ret;
}

static int adapter_odev_bt_config(int cmd, void *parm)
{
    r_printf("adapter_odev_bt_config\n");

    if (__this->parm->mode & BIT(ODEV_EDR)) {
        adapter_odev_edr_config(cmd, parm);
    }

    if (__this->parm->mode & BIT(ODEV_BLE)) {
        adapter_odev_ble_config(cmd, parm);
    }

    return 0;
}

static int adapter_odev_bt_send(void *priv, u8 *buf, u16 len)
{
    int ret = -1;

    if (__this->parm->mode & BIT(ODEV_BLE)) {
        ret = adapter_odev_ble_send(priv, buf, len);
    }

    return ret;
}

REGISTER_ADAPTER_ODEV(adapter_odev_bt) = {
    .id     = ADAPTER_ODEV_BT,
    .open   = adapter_odev_bt_open,
    .start  = adapter_odev_bt_start,
    .stop   = adapter_odev_bt_stop,
    .close  = adapter_odev_bt_close,
    .media_pp = adapter_odev_bt_pp_ctl,
    .get_status = adapter_odev_bt_get_status,
    .media_prepare = adapter_odev_bt_media_prepare,
    .event_fun  = adapter_odev_bt_event_deal,
    .config = adapter_odev_bt_config,
    .output = adapter_odev_bt_send,
};
#endif
