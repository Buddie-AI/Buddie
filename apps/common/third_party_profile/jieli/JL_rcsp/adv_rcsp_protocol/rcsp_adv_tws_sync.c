#include "app_config.h"
#include "rcsp_adv_tws_sync.h"
#include "adv_setting_common.h"
#include "rcsp_adv_opt.h"
#include "adv_time_stamp_setting.h"
#include "rcsp_adv_bluetooth.h"
#include "bt_tws.h"

#if (RCSP_ADV_EN && TCFG_USER_TWS_ENABLE)
static void adv_sync_tws_func_t(void *data, u16 len, bool rx)
{
    if (rx) {
        deal_sibling_setting((u8 *)data);
    }
}

REGISTER_TWS_FUNC_STUB(adv_tws_sync) = {
    .func_id = TWS_FUNC_ID_ADV_SETTING_SYNC,
    .func    = adv_sync_tws_func_t,
};

static void adv_sync_time_stamp_func_t(void *data, u16 len, bool rx)
{
    if (rx) {
        deal_sibling_time_stamp_setting_switch(data, len);
    }
}

REGISTER_TWS_FUNC_STUB(adv_time_stamp_sync) = {
    .func_id = TWS_FUNC_ID_TIME_STAMP_SYNC,
    .func    = adv_sync_time_stamp_func_t,
};

#define TWS_FUNC_ID_ADV_RESET_SYNC \
	(((u8)('R' + 'C' + 'S' + 'P') << (3 * 8)) | \
	 ((u8)('A' + 'D' + 'V') << (2 * 8)) | \
	 ((u8)('R' + 'E' + 'S' + 'E' + 'T') << (1 * 8)) | \
	 ((u8)('S' + 'Y' + 'N' + 'C') << (0 * 8)))

static void adv_sync_reset_sync_func_t(int args)
{
    extern void cpu_reset();
    cpu_reset();
}

TWS_SYNC_CALL_REGISTER(adv_reset_sync) = {
    .uuid = TWS_FUNC_ID_ADV_RESET_SYNC,
    .func    = adv_sync_reset_sync_func_t,
};

void modify_bt_name_and_reset(u32 msec)
{
    tws_api_sync_call_by_uuid(TWS_FUNC_ID_ADV_RESET_SYNC, 0, msec);
}

#define TWS_FUNC_ID_ADV_FIND_DEV_SYNC \
	(((u8)('A' + 'D' + 'V') << (3 * 8)) | \
	 ((u8)('F' + 'I' + 'N' + 'D') << (2 * 8)) | \
	 ((u8)('D' + 'E' + 'V') << (1 * 8)) | \
	 ((u8)('S' + 'Y' + 'N' + 'C') << (0 * 8)))

static void adv_find_dev_sync_func_t(int args)
{
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_RESUME, &args, sizeof(args));
}

TWS_SYNC_CALL_REGISTER(adv_find_dev_sync) = {
    .uuid = TWS_FUNC_ID_ADV_FIND_DEV_SYNC,
    .func    = adv_find_dev_sync_func_t,
};

void find_device_sync(u8 *param, u32 msec)
{
    int priv = 0;
    if (TWS_ROLE_MASTER == tws_api_get_role()) {
        memcpy(&priv, param, 3);
        tws_api_sync_call_by_uuid(TWS_FUNC_ID_ADV_FIND_DEV_SYNC, priv, msec);
    }
}

#define TWS_FUNC_ID_ADV_FIND_DEV_STOP_TIMER_SYNC \
	(((u8)('F' + 'I' + 'N' + 'D') << (3 * 8)) | \
	 ((u8)('D' + 'E' + 'V') << (2 * 8)) | \
	 ((u8)('S' + 'T' + 'O' + 'P') << (1 * 8)) | \
	 ((u8)('S' + 'Y' + 'N' + 'C') << (0 * 8)))

static void adv_find_dev_stop_sync_timer_func_t(int args)
{
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_STOP, &args, sizeof(args));
}

TWS_SYNC_CALL_REGISTER(adv_find_dev_stop_sync) = {
    .uuid = TWS_FUNC_ID_ADV_FIND_DEV_STOP_TIMER_SYNC,
    .func    = adv_find_dev_stop_sync_timer_func_t,
};

void find_devic_stop_timer(u8 *param, u32 msec)
{
    int priv = 0;
    if (TWS_ROLE_MASTER == tws_api_get_role()) {
        memcpy(&priv, param, 3);
        tws_api_sync_call_by_uuid(TWS_FUNC_ID_ADV_FIND_DEV_STOP_TIMER_SYNC, priv, msec);
    }
}
#endif
