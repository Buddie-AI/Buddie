#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "typedef.h"
#include "3th_profile_api.h"
#include "bt_tws.h"
#include "key_event_deal.h"
#include "system/timer.h"
#include "bt_common.h"
#include "btstack/avctp_user.h"
#include "os/os_api.h"

#if BT_FOR_APP_EN

#if (OTA_TWS_SAME_TIME_ENABLE && RCSP_ADV_EN && USER_APP_EN)
#include "rcsp_adv_tws_ota.h"
#endif

#if (OTA_TWS_SAME_TIME_ENABLE && SMART_BOX_EN)
#include "smartbox_update_tws.h"
#else
#include "update_tws.h"
#endif

static u8 mic_data_type = SOURCE_TYPE;
static u8 connect_type = TYPE_NULL;
static u8 tws_ble_type = TYPE_NULL;

void mic_set_data_source(u8 data_type)
{
    mic_data_type = data_type;
}

u8 mic_get_data_source(void)
{
    return mic_data_type;
}

u8 get_ble_connect_type(void)
{
    return tws_ble_type;
}

void set_ble_connect_type(u8 type)
{
    tws_ble_type = type;
}

u8 get_app_connect_type(void)
{
    return connect_type;
}

void set_app_connect_type(u8 type)
{
    connect_type = type;
}


#if TCFG_USER_TWS_ENABLE

int tws_data_to_sibling_send(u8 opcode, u8 *data, u8 len)
{
    u8 send_data[len + 2];
    printf(">>>>>>>>>>send data to sibling \n");
    send_data[0] = opcode;
    send_data[1] = len;
    memcpy(send_data + 2, data, len);

    return tws_api_send_data_to_sibling(send_data, sizeof(send_data), TWS_FUNC_ID_AI_SYNC);
}

static void __ai_tws_rx_from_sibling(u8 *data)
{
    u8 opcode = data[0];
    u8 len = data[1];
    u8 *rx_data = data + 2;

#if (OTA_TWS_SAME_TIME_ENABLE && (RCSP_BTMATE_EN || RCSP_ADV_EN || SMART_BOX_EN))
    tws_ota_get_data_from_sibling(opcode, rx_data, len);
#endif

    free(data);
}

static void ai_tws_rx_from_sibling(void *_data, u16 len, bool rx)
{
    int err = 0;
    if (rx) {
        printf(">>>%s \n", __func__);
        printf("len :%d\n", len);
        put_buf(_data, len);
        u8 *rx_data = malloc(len);
        memcpy(rx_data, _data, len);

        int msg[4];
        msg[0] = (int)__ai_tws_rx_from_sibling;
        msg[1] = 1;
        msg[2] = (int)rx_data;
        err = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
        if (err) {
            printf("tws rx post fail\n");
        }
    }
}

//发送给对耳
REGISTER_TWS_FUNC_STUB(app_vol_sync_stub) = {
    .func_id = TWS_FUNC_ID_AI_SYNC,
    .func    = ai_tws_rx_from_sibling,
};

#endif
#endif
