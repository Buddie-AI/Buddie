/*****************************************************************
>file name : spatial_tws.c
>create time : Fri 08 Apr 2022 11:48:16 AM CST
*****************************************************************/
#include "spatial_effect_tws.h"
#include "app_config.h"

extern int CONFIG_BTCTLER_TWS_ENABLE;

struct spatial_tws_connection {
    u16 period;
    u32 next_period;
    void *priv;
    void (*data_handler)(void *priv, void *data, int len);
};

static struct spatial_tws_connection *local_conn = NULL;

#define SPATIAL_TWS_AUDIO \
	((int)(('S' + 'P' + 'A' + 'T' + 'I' + 'A' + 'L') << (2 * 8)) | \
	 (int)(('T' + 'W' + 'S') << (1 * 8)) | \
	 (int)(('A' + 'U' + 'D' + 'I' + 'O') << (0 * 8)))

static void spatial_tws_audio_handler(void *buf, u16 len, bool rx)
{
    if (!CONFIG_BTCTLER_TWS_ENABLE) {
        return;
    }

    local_irq_disable();
    if (local_conn) {
        local_conn->data_handler(local_conn->priv, buf, len);
    }
    local_irq_enable();
}

REGISTER_TWS_FUNC_STUB(spatial_tws_audio) = {
    .func_id = SPATIAL_TWS_AUDIO,
    .func    = spatial_tws_audio_handler,
};

void *spatial_tws_create_connection(int period, void *priv, void (*handler)(void *, void *data, int len))
{
    if (!CONFIG_BTCTLER_TWS_ENABLE) {
        return NULL;
    }

    struct spatial_tws_connection *conn = (struct spatial_tws_connection *)zalloc(sizeof(struct spatial_tws_connection));

    if (!conn) {
        return NULL;
    }
    conn->period = period;
    conn->priv = priv;
    conn->data_handler = handler;
    conn->next_period = jiffies;
    local_conn = conn;
    return conn;
}

void spatial_tws_delete_connection(void *conn)
{
    if (!CONFIG_BTCTLER_TWS_ENABLE) {
        return;
    }
    local_irq_disable();
    local_conn = NULL;
    local_irq_enable();
    if (conn) {
        free(conn);
    }
}

int spatial_tws_audio_data_sync(void *_conn, void *buf, int len)
{
    if (!CONFIG_BTCTLER_TWS_ENABLE) {
        return 0;
    }

    struct spatial_tws_connection *conn = (struct spatial_tws_connection *)_conn;
    if (!conn) {
        return 0;
    }

    if (!(tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        conn->data_handler(conn->priv, buf, len);
        return len;
    }

    if (time_after(jiffies, conn->next_period)) {
        conn->next_period = jiffies + msecs_to_jiffies(conn->period);
#if ((defined TCFG_TWS_SPATIAL_AUDIO_AS_CHANNEL) && (TCFG_TWS_SPATIAL_AUDIO_AS_CHANNEL == 'L' || TCFG_TWS_SPATIAL_AUDIO_AS_CHANNEL == 'R'))
        if (tws_api_get_local_channel() == TCFG_TWS_SPATIAL_AUDIO_AS_CHANNEL) {
#else
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
#endif
            int err = tws_api_send_data_to_sibling(buf, len, SPATIAL_TWS_AUDIO);
            if (err) {
                return 0;
            }
        }
    }
    return len;
}
