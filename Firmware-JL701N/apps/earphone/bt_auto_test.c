#if 0

#include "system/includes.h"
#include "media/includes.h"
#include "tone_player.h"

#include "app_config.h"
#include "app_action.h"

#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "btstack/frame_queque.h"
#include "user_cfg.h"
#include "aec_user.h"
#include "classic/hci_lmp.h"
#include "bt_common.h"
#include "pbg_user.h"

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif

#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)
#include "include/bt_ble.h"
#endif

#if TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE
#include "app_chargestore.h"
#endif

#include "asm/charge.h"
#include "app_charge.h"
#include "ui_manage.h"

#include "app_chargestore.h"
#include "app_online_cfg.h"
#include "app_main.h"
#include "app_power_manage.h"
#include "gSensor/gSensor_manage.h"
#include "key_event_deal.h"
#include "classic/tws_api.h"
#include "asm/pwm_led.h"
#include "vol_sync.h"


#define CONFIG_SPP_PRINTF_DEBUG         0

#define CONFIG_A2DP_PLAY_PAUSE_TEST     0
#define CONFIG_A2DP_PLAY_NUM            0
#define CONFIG_POWEROFF_TEST            0
#define CONFIG_LOW_LATENCY_TEST         0
#define CONFIG_A2DP_AUTO_PLAY           1
#define CONFIG_IN_OUT_CHARGESTORE_TEST  0
#define CONFIG_TWS_SUPER_TIMEOUT_TEST   0

#define CONFIG_MODE_SWITCH_ENABLE       0
#define CONFIG_ROLE_SWITCH_TEST         0

#define CONFIG_GMA_TEST                 0



#if CONFIG_LOW_LATENCY_TEST

static void low_latency_switch_test(void *p)
{
    struct sys_event evt;

    r_printf("low_latency_test\n");

    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        evt.arg     = (void *)DEVICE_EVENT_FROM_KEY;
        evt.type    = SYS_KEY_EVENT;
        evt.u.key.value   = KEY_POWER_START;
        evt.u.key.event   = KEY_EVENT_TRIPLE_CLICK;

        sys_event_notify(&evt);
    }

}

#endif

#if CONFIG_GMA_TEST
static void gma_test(void *p)
{
    struct sys_event evt;

    r_printf("gma_test\n");

    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        evt.arg     = (void *)DEVICE_EVENT_FROM_KEY;
        evt.type    = SYS_KEY_EVENT;
        evt.u.key.value   = KEY_POWER_START;
        evt.u.key.event   = KEY_EVENT_LONG;

        sys_event_notify(&evt);

        int ms = 10000 + rand32() % 10 * 5000;
        sys_timeout_add(NULL, gma_test, ms);
    }
}
#endif



#if CONFIG_SPP_PRINTF_DEBUG
static u32 spp_buf[1024 / 4];
static cbuffer_t spp_buf_hdl;
static u32 send_len = 0;
static int spp_timer = 0;
static char out[256];

int log_print_time_to_buf(char *time);

void spp_printf(const char *format, ...)
{
    va_list args;
    char *p = out;

    if (spp_timer == 0) {
        return;
    }

    va_start(args, format);

    int len = log_print_time_to_buf(p);
    p += len;

    len += print(&p, 0, format, args);
    if (cbuf_is_write_able(&spp_buf_hdl, len)) {
        cbuf_write(&spp_buf_hdl, out, len);
    } else {
        putchar('N');
    }
}

void spp_send_data_timer(void *p)
{
    u8 *ptr;

__send:
    if (send_len == 0) {
        ptr = cbuf_read_alloc(&spp_buf_hdl, &send_len);
        if (send_len) {
            if (send_len > 250) {
                send_len = 250;
            }
            user_send_cmd_prepare(USER_CTRL_SPP_SEND_DATA, send_len, ptr);
        }
    } else {
        if (0 == user_send_cmd_prepare(USER_CTRL_SPP_SEND_DATA, 0, NULL)) {
            cbuf_read_updata(&spp_buf_hdl, send_len);
            send_len = 0;
            goto __send;
        }
    }
}

static void spp_data_handler(u8 packet_type, u16 ch, u8 *packet, u16 size)
{
    switch (packet_type) {
    case 1:
        cbuf_init(&spp_buf_hdl, spp_buf, sizeof(spp_buf));
        spp_timer = sys_timer_add(NULL, spp_send_data_timer, 50);
        spp_printf("spp connect\n");
        break;
    case 2:
        printf("spp disconnect\n");
        if (spp_timer) {
            sys_timer_del(spp_timer);
            spp_timer = 0;
        }
        break;
    case 7:
        //log_info("spp_rx:");
        //put_buf(packet,size);
        break;
    }
}

#endif



static int a2dp_open_cnt = 0;
u8 bt_audio_is_running(void);

static void a2dp_play_pause_test(void *p)
{
    struct sys_event evt;

    if (bt_audio_is_running()) {
        if (p) {
            return;
        }
    } else {
        if (!p) {
            return;
        }
    }
    r_printf("a2dp_play_pause_test\n");

    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        evt.arg     = (void *)DEVICE_EVENT_FROM_KEY;
        evt.type    = SYS_KEY_EVENT;
        evt.u.key.value   = KEY_POWER_START;
        evt.u.key.event   = KEY_EVENT_CLICK;
        /*evt.u.key.event   = KEY_EVENT_DOUBLE_CLICK;*/

        sys_event_notify(&evt);
    }
}

static void disconnect_test(void *p)
{
    r_printf("disconnect_test: %x\n", p);

    if (!p) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
        }
    } else {
        bt_user_priv_var.auto_connection_counter = 60 * 1000;
        bt_tws_phone_connect_timeout();
    }
}

#if CONFIG_TWS_SUPER_TIMEOUT_TEST
extern void tws_timeout_test();
static void tws_super_timeout_test(void *p)
{
    tws_timeout_test();
}
#endif

#if CONFIG_ROLE_SWITCH_TEST
static void tws_role_switch_test(void *p)
{
    tws_conn_switch_role();

    /*void bt_tws_role_switch();
    bt_tws_role_switch();*/
}
#endif


#if CONFIG_MODE_SWITCH_ENABLE
static void mode_switch_test()
{
    struct sys_event evt;

    if (tws_api_get_role() == 0) {
        evt.arg     = (void *)DEVICE_EVENT_FROM_KEY;
        evt.type    = SYS_KEY_EVENT;
        evt.u.key.value   = KEY_POWER_START;
        evt.u.key.event   = KEY_EVENT_TRIPLE_CLICK;
        sys_event_notify(&evt);
    }

}
#endif



static int bt_connction_status_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
    case BT_STATUS_FIRST_CONNECTED:
#if CONFIG_SPP_PRINTF_DEBUG
        spp_data_deal_handle_register(spp_data_handler);
#endif

#if CONFIG_A2DP_AUTO_PLAY
        if (tws_api_get_role() == 0) {
            sys_timeout_add((void *)1, a2dp_play_pause_test,  8000);
        }
#endif
#if CONFIG_LOW_LATENCY_TEST
        if (tws_api_get_role() == 0) {
            sys_timer_add(NULL, low_latency_switch_test, 15000);
        }
#endif
#if CONFIG_GMA_TEST
        if (tws_api_get_role() == 0) {
            sys_timeout_add(NULL, gma_test, 15000);
        }
#endif
#if CONFIG_POWEROFF_TEST
        {
            int msec = 10000 ;//+ rand32() % 5 * 5000;
            sys_timeout_add(NULL, disconnect_test, msec);
        }
#endif
#if CONFIG_MODE_SWITCH_ENABLE
        sys_timer_add(NULL, mode_switch_test,  20000);
#endif
        break;
    case BT_STATUS_FIRST_DISCONNECT:
#if CONFIG_POWEROFF_TEST
        a2dp_open_cnt = 0;
        sys_timeout_add((void *)1, disconnect_test, 2000);
#endif
        break;
    case BT_STATUS_A2DP_MEDIA_START:
#if CONFIG_A2DP_PLAY_PAUSE_TEST
        if (CONFIG_A2DP_PLAY_NUM && ++a2dp_open_cnt >= CONFIG_A2DP_PLAY_NUM) {
#if CONFIG_POWEROFF_TEST
            sys_timeout_add(NULL, disconnect_test, 5000);
#endif
        } else {
            sys_timeout_add(NULL, a2dp_play_pause_test,  10000);
        }
#endif
#if CONFIG_ROLE_SWITCH_TEST
        sys_timeout_add(NULL, tws_role_switch_test, 5000);
#endif
        break;
    case BT_STATUS_A2DP_MEDIA_STOP:
#if CONFIG_A2DP_PLAY_PAUSE_TEST
        if (CONFIG_A2DP_PLAY_NUM == 0 || a2dp_open_cnt < CONFIG_A2DP_PLAY_NUM) {
            sys_timeout_add((void *)1, a2dp_play_pause_test,  15000);
        }
#endif
        break;
    }
    return 0;
}

static int bt_tws_event_handler(struct bt_event *evt)
{
    int role = evt->args[0];
    int phone_link_connection = evt->args[1];
    int reason = evt->args[2];

    switch (evt->event) {
    case TWS_EVENT_CONNECTED:
#if CONFIG_IN_OUT_CHARGESTORE_TEST
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            void sys_enter_soft_poweroff(void *priv);
            sys_timeout_add((void *)1, sys_enter_soft_poweroff, 5000);
        }
#endif
#if CONFIG_TWS_SUPER_TIMEOUT_TEST
        sys_timeout_add(NULL, tws_super_timeout_test,  10000);
#endif
#if CONFIG_ROLE_SWITCH_TEST
        sys_timeout_add(NULL, tws_role_switch_test, 15000);
#endif
        break;
    case TWS_EVENT_ROLE_SWITCH:
#if CONFIG_ROLE_SWITCH_TEST
        sys_timeout_add(NULL, tws_role_switch_test, 5000);
#endif
        break;
    }
    return 0;
}


static void auto_test_event_handler(struct sys_event *event, void *priv)
{
    /*
     * 蓝牙事件处理
     */
    switch ((u32)event->arg) {
    case SYS_BT_EVENT_TYPE_CON_STATUS:
        bt_connction_status_event_handler(&event->u.bt);
        break;
    case SYS_BT_EVENT_TYPE_HCI_STATUS:
        break;
#if TCFG_USER_TWS_ENABLE
    case SYS_BT_EVENT_FROM_TWS:
        bt_tws_event_handler(&event->u.bt);
        break;
#endif
    }
}


static void bt_auto_test_task(void *p)
{
    int res;
    int msg[32];

    register_sys_event_handler(SYS_BT_EVENT, 0, 0, auto_test_event_handler);

    os_time_dly(2);

#if CONFIG_IN_OUT_CHARGESTORE_TEST
    void sys_enter_soft_poweroff(void *priv);
    int msec = 500 + rand32() % 10 * 200;
    sys_timeout_add((void *)1, sys_enter_soft_poweroff, msec);
#endif

    while (1) {
        res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res != OS_TASKQ) {
            continue;
        }
    }
}


int bt_auto_test_init()
{
    os_task_create(bt_auto_test_task, NULL, 1, 256, 256, "auto_test");
    return 0;
}
late_initcall(bt_auto_test_init);






#endif
