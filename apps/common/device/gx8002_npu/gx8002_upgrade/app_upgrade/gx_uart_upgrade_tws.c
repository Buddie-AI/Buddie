#include "includes.h"
#include "../gx_uart_upgrade.h"
#include "../../gx8002_npu_api.h"
#include "../gx_uart_upgrade_porting.h"
//#include "../utils/gx_fifo.h"

#if (GX8002_UPGRADE_APP_TWS_TOGGLE && OTA_TWS_SAME_TIME_ENABLE)

struct gx8002_tws_update {
    update_op_tws_api_t *tws_handle;
    OS_SEM tws_sync_sem;
    u8 slave_result;
};

enum GX8002_TWS_UPDATE_CMD {
    GX8002_TWS_UPDATE_CMD_START = 'G',
    GX8002_TWS_UPDATE_CMD_UFW_DATA,
    GX8002_TWS_UPDATE_CMD_RESPOND,
    GX8002_TWS_UPDATE_CMD_RESULT,
};

static struct gx8002_tws_update *tws_update = NULL;

#define GX8002_OTA_UPGRADE_DEBUG_ENABLE 		0

#define ota_upgrade_info 		 printf

#if GX8002_OTA_UPGRADE_DEBUG_ENABLE
#define ota_upgrade_debug 	printf
#define ota_upgrade_put_buf  put_buf
#else
#define ota_upgrade_debug(...)
#define ota_upgrade_put_buf(...)
#endif /* #if GX8002_DEBUG_ENABLE */

#define GX8002_TWS_SYNC_TIMEOUT 	300


extern int gx8002_ota_data_passive_receive(u8 *data, u32 size);
extern int gx8002_app_ota_passive_start(int (*complete_callback)(void), u8 *ota_data, u16 len);
extern update_op_tws_api_t *get_tws_update_api(void);
extern void tws_update_register_user_chip_update_handle(void (*update_handle)(void *data, u32 len));

int gx8002_uart_app_ota_tws_update_sync_pend(void)
{
    ota_upgrade_debug("%s", __func__);
    if (tws_update) {
        if (os_sem_pend(&(tws_update->tws_sync_sem), GX8002_TWS_SYNC_TIMEOUT) == OS_TIMEOUT) {
            return -1;
        }
    }

    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_post(void)
{
    ota_upgrade_debug("%s", __func__);
    if (tws_update) {
        os_sem_post(&(tws_update->tws_sync_sem));
    }

    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_result(int result)
{
    ota_upgrade_info("%s: result: 0x%x", __func__, result);
    if (tws_update) {
        if (result) {
            tws_update->slave_result = 'N';
        } else {
            tws_update->slave_result = 'O';
        }
        if (tws_update->tws_handle) {
            tws_update->tws_handle->tws_ota_user_chip_update_send(GX8002_TWS_UPDATE_CMD_RESULT, &(tws_update->slave_result), 1);
        }
    }

    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_result_get(void)
{
    if (tws_update) {
        if (gx8002_uart_app_ota_tws_update_sync_pend()) {
            ota_upgrade_info("%s timeout", __func__);
            return -1;
        }
        if (tws_update->slave_result == 'O') {
            ota_upgrade_info("%s, get: O", __func__);
            return 0;
        }
    }

    ota_upgrade_info("%s state err", __func__);

    return -1;
}

int gx8002_uart_app_ota_tws_update_sync_respond(void)
{
    ota_upgrade_debug("%s", __func__);
    if (tws_update) {
        if (tws_update->tws_handle) {
            tws_update->tws_handle->tws_ota_user_chip_update_send(GX8002_TWS_UPDATE_CMD_RESPOND, NULL, 0);
        }
    }

    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_start(u8 *buf, u16 len)
{
    if (tws_update) {
        if (tws_update->tws_handle) {
            tws_update->tws_handle->tws_ota_user_chip_update_send(GX8002_TWS_UPDATE_CMD_START, buf, len);
        }
    }

    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_data(u8 *buf, u16 len)
{
    if (tws_update) {
        if (tws_update->tws_handle) {
            tws_update->tws_handle->tws_ota_user_chip_update_send(GX8002_TWS_UPDATE_CMD_UFW_DATA, buf, len);
        }
    }

    return 0;
}

int gx8002_uart_app_ota_tws_update_init(void *priv)
{
    ota_upgrade_info("%s", __func__);

    if (tws_update) {
        free(tws_update);
    }

    if (priv == NULL) {
        return -1;
    }

    tws_update = zalloc(sizeof(struct gx8002_tws_update));
    if (tws_update == NULL) {
        return -1;
    }
    tws_update->tws_handle = priv;

    os_sem_create(&(tws_update->tws_sync_sem), 0);

    return 0;
}


int gx8002_uart_app_ota_tws_update_close(void)
{
    if (tws_update) {
        free(tws_update);

        tws_update = NULL;
    }

    return 0;
}

static void gx8002_uart_app_ota_tws_update_handle(void *data, u32 len)
{
    u8 *rx_packet = (u8 *)data;
    int ret = 0;

    ota_upgrade_debug("receive cmd 0x%x, data len = 0x%x", rx_packet[0], len - 1);

    switch (rx_packet[0]) {
    case GX8002_TWS_UPDATE_CMD_START:
        gx8002_uart_app_ota_tws_update_init(get_tws_update_api());
        ret = gx8002_app_ota_passive_start(gx8002_uart_app_ota_tws_update_sync_respond, rx_packet + 1, len - 1);
        if (ret) {
            gx8002_uart_app_ota_tws_update_close();
        }
        break;

    case GX8002_TWS_UPDATE_CMD_UFW_DATA:
        gx8002_ota_data_passive_receive(rx_packet + 1, len - 1);
        break;

    case GX8002_TWS_UPDATE_CMD_RESPOND:
        gx8002_uart_app_ota_tws_update_sync_post();
        break;

    case GX8002_TWS_UPDATE_CMD_RESULT:
        ota_upgrade_info("result: 0x%x", rx_packet[1]);
        if (tws_update) {
            tws_update->slave_result = rx_packet[1];
        }
        gx8002_uart_app_ota_tws_update_sync_post();
        break;

    default:
        break;
    }
}

void gx8002_uart_app_ota_tws_update_register_handle(void)
{
    tws_update_register_user_chip_update_handle(gx8002_uart_app_ota_tws_update_handle);
}

#else /* #if (GX8002_UPGRADE_APP_TWS_TOGGLE && OTA_TWS_SAME_TIME_ENABLE) */

void gx8002_uart_app_ota_tws_update_register_handle(void)
{
    return;
}

int gx8002_uart_app_ota_tws_update_init(void *priv)
{
    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_pend(void)
{
    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_post(void)
{
    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_data(u8 *buf, u16 len)
{
    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_respond(void)
{
    return 0;
}

int gx8002_uart_app_ota_tws_update_close(void)
{
    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_start(u8 *buf, u16 len)
{
    return 0;
}

int gx8002_uart_app_ota_tws_update_sync_result(int result)
{
    return 0;
}


int gx8002_uart_app_ota_tws_update_sync_result_get(void)
{
    return 0;
}
#endif /* #if (GX8002_UPGRADE_APP_TWS_TOGGLE && OTA_TWS_SAME_TIME_ENABLE) */

