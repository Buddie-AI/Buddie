#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_protocol_api.h"
#include "system/includes.h"
#include "vm.h"
#include "audio_config.h"
#include "app_power_manage.h"
#include "user_cfg.h"
#include "earphone.h"
#include "btstack/avctp_user.h"
#include "btstack/bluetooth.h"
#include "bt_tws.h"

#if AI_APP_PROTOCOL
#if APP_PROTOCOL_GFPS_CODE

extern void *gfps_app_ble_hdl;
/* extern u32 unactice_device_cmd_prepare(USER_CMD_TYPE cmd, u16 param_len, u8 *param); */
extern void tws_dual_conn_close();
extern int gfps_tws_data_deal(u8 *data, int len);
extern void audio_app_volume_set(u8 state, s16 volume, u8 fade);
extern bool get_gfps_pair_state(void);
extern void set_gfps_pair_state(u8 state);
extern void gfps_set_battery_ui_enable(uint8_t enable);
extern void gfps_set_pair_mode(void *priv);
extern void gfps_all_info_recover(u8 name_reset);
extern void gfps_sibling_sync_info();
extern int gfps_disconnect(void *addr);
extern void gfps_bt_ble_adv_enable(u8 enable);
extern void gfps_adv_interval_set(u16 value);
extern void gfps_sibling_data_send(u8 type, u8 *data, u16 len);
extern int gfps_ble_adv_enable(u8 enable);
extern void gfps_battery_update(void);
extern void bt_set_need_keep_scan(u8 en);
extern int audio_digital_vol_update_parm(u8 dvol_idx, s32 param);
extern void set_wait_poweroff_time(u16 time);

/**
 弹窗的逻辑：
 1、没有任何手机配对记录情况，广播初始配对广播。
 2、开机有配对记录广播后续配对广播，回连超时进入配对状态时，广播初始配对广播。
 3、回连手机发现linkkey_missing，且没有任何手机配对记录了，广播初始配对广播。
 4、连上任意一台手机后， 广播后续配对广播，断开进入配对状态也是广播后续配对广播。（客户要求断开后也广播初始配对广播）
 5、恢复出厂设置时，需要调用一下gfps_all_info_recover(1)清除一下GFP的账号信息和修改的蓝牙名字
 6、认证的时候，需要有一个UI来触发初始配对广播（调用gfps_set_pair_mode(NULL)），可以为长按左右耳5秒之类的
 7、耳机关机要关电量显示
 */

#define TONE_GOOGLE         "tone_en/google.*"
#define GFPS_SIBLING_SYNC_TYPE_ADV_MAC_STATE    0x04

#define ADV_INIT_PAIR_IN_PAIR_MODE      0 //进入配对状态广播初始配对广播

struct gfps_protocol_t {
    u8 poweron_reconn;
    u8 close_icon;
};
static struct gfps_protocol_t gfps_protocol = {0};

#define __this          (&gfps_protocol)

#define INIT_PAIR_ADV_INTERVAL      150
#define SUBS_PAIR_ADV_INTERVAL      352

void gfps_set_pair_mode_by_user(u8 disconn_ble)
{
    if (disconn_ble) {
        gfps_disconnect(NULL);
    }
    gfps_adv_interval_set(INIT_PAIR_ADV_INTERVAL);
    set_gfps_pair_state(0);
    gfps_bt_ble_adv_enable(0);
    gfps_bt_ble_adv_enable(1);
}

//重写弱函数，获取电量
extern u8 get_charge_online_flag(void);
extern u8 get_tws_sibling_bat_persent(void);
void generate_rfcomm_battery_data(u8 *data)
{
    u8 self_val = get_charge_online_flag() << 7 | get_vbat_percent();
#if TCFG_USER_TWS_ENABLE
    u8 sibling_val = get_tws_sibling_bat_persent();

    data[2] = 0xFF; //充电仓电量

#if TCFG_CHARGESTORE_ENABLE
    if (self_val >> 7 || sibling_val >> 7) { //有任意一直耳机充电时才显示充电仓电量
        data[2] = chargestore_get_power_level(); //充电仓电量
    }
#endif

    data[0] = (tws_api_get_local_channel() == 'L') ? self_val : sibling_val; //左耳电量
    data[1] = (tws_api_get_local_channel() == 'R') ? self_val : sibling_val; //右耳电量
#else
    data[0] = self_val;
    data[1] = 0xFF;
    data[2] = 0xFF;
#endif
    printf("%s, l:%x, r:%x, bat:%x", __func__, data[0], data[1], data[2]);
}

extern int lmp_hci_write_scan_enable(u8 enable);
static void gfps_ctl_bt_enter_pair_mode()
{
    printf("%s", __func__);
    tws_dual_conn_close();
    lmp_hci_write_scan_enable((1 << 1) | 1);
    bt_set_need_keep_scan(1);
}

//重写弱函数，GFP-BLE配对认证成功后，需要先断开一个设备或者开可连接，等手机连接经典蓝牙
extern u8 *get_cur_connect_phone_mac_addr(void);
void gfps_before_pair_new_device(u8 *seeker_addr)
{
#if TCFG_USER_GFPS_PROCESS_CTL
    app_user_gfps_before_pair_new_device(seeker_addr);
#else
    u8 remote_edr_addr[6];
    u8 *tmp_addr = NULL;

    for (int i = 0; i < 6; i ++) {
        remote_edr_addr[i] = seeker_addr[5 - i];
    }

    if (get_curr_channel_state()) {
        tmp_addr = get_cur_connect_phone_mac_addr();
        if (memcmp(tmp_addr, remote_edr_addr, 6) != 0) {
            printf("Subsequent pair\n");
            printf("USER_CTRL_DISCONNECTION_HCI send\n");
            user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
        } else {
            printf("Retroactive Pair\n");
        }
    }
    printf("func:%s, line:%d", __FUNCTION__, __LINE__);
#endif
}

struct google_tone_ctl_t {
    u8 l_en;
    u8 r_en;
    u8 l_mute;
    u8 r_mute;
};
static struct google_tone_ctl_t google_tone_ctl = {0};

static void google_tone_set_volume()
{
    printf("google_tone_set_volume");
    /* printf("func:%s:%d", __FUNCTION__, app_audio_volume_max_query(SysVol_TONE)); */
    /* u32 param = VOLUME_NODE_CMD_SET_VOL | app_audio_volume_max_query(SysVol_TONE); */
    /* jlstream_set_node_param(NODE_UUID_VOLUME_CTRLER, "Vol_BtcRing", &param, sizeof(param)); */
}

/* #if TCFG_USER_TWS_ENABLE */
/* static void tws_google_tone_callback(int priv, enum stream_event event) */
/* { */
/* printf("func:%s, event:%d", __FUNCTION__, event); */
/* if (event == STREAM_EVENT_START) { */
/* int msg[] = {(int)google_tone_set_volume, 0}; */
/* os_taskq_post_type("app_core", Q_CALLBACK, ARRAY_SIZE(msg), msg); */
/* } */
/* } */
/* REGISTER_TWS_TONE_CALLBACK(tws_google_tone_stub) = { */
/* .func_uuid  = 0x2EC850AE, */
/* .callback   = tws_google_tone_callback, */
/* }; */
/* #endif */

/* static int google_tone_play_callback(void *priv, enum stream_event event) */
/* { */
/* printf("func:%s, event:%d", __FUNCTION__, event); */
/* if (event == STREAM_EVENT_START) { */
/* int msg[] = {(int)google_tone_set_volume, 0}; */
/* os_taskq_post_type("app_core", Q_CALLBACK, ARRAY_SIZE(msg), msg); */
/* } */
/* return 0; */
/* } */

void google_tone_play_deal()
{
    if (strcmp(os_current_task(), "app_core")) {
        printf("%s, task:%s", __func__, os_current_task());
        int msg[] = {(int)google_tone_play_deal, 0};
        os_taskq_post_type("app_core", Q_CALLBACK, ARRAY_SIZE(msg), msg);
        return;
    }

    /* tone_player_stop(); */
    /* tone_player_play(); */
}

void google_tone_mute_ctl(u8 self, u8 charge_flag)
{
    u8 *mute = &google_tone_ctl.l_mute;

    if ((tws_api_get_local_channel() == 'L' && !self) ||
        (tws_api_get_local_channel() == 'R' && self)) {
        mute = &google_tone_ctl.r_mute;
        printf("%s, r_mute:%d, charge_flag:%d", __func__, *mute, charge_flag);
    } else {
        printf("%s, l_mute:%d, charge_flag:%d", __func__, *mute, charge_flag);
    }

    if (*mute != charge_flag) {
        *mute = charge_flag;
        google_tone_play_deal();
    }
}

int gfps_protocol_tws_send_to_sibling(u16 opcode, u8 *data, u16 len)
{
    return tws_api_send_data_to_sibling(data, len, 0x23080315);
}

static void __tws_rx_from_sibling(u8 *data, int len)
{
    gfps_tws_data_deal(data, len);

    switch (data[0]) {
    case GFPS_SIBLING_SYNC_TYPE_ADV_MAC_STATE:
        gfps_adv_interval_set(data[1] ? SUBS_PAIR_ADV_INTERVAL : INIT_PAIR_ADV_INTERVAL);
        set_gfps_pair_state(data[1]);
        le_controller_set_mac((void *)&data[2]);
        gfps_bt_ble_adv_enable(0);
        gfps_bt_ble_adv_enable(1);
        break;
    }
    free(data);
}

static void gfps_protocol_rx_from_sibling(void *_data, u16 len, bool rx)
{
    int err = 0;
    if (rx) {
        printf(">>>%s \n", __func__);
        printf("len :%d\n", len);
        put_buf(_data, len);

        u8 *rx_data = malloc(len);
        if (!rx_data) {
            return;
        }

        memcpy(rx_data, _data, len);

        int msg[4];
        msg[0] = (int)__tws_rx_from_sibling;
        msg[1] = 2;
        msg[2] = (int)rx_data;
        msg[3] = (int)len;
        err = os_taskq_post_type("app_core", Q_CALLBACK, ARRAY_SIZE(msg), msg);
        if (err) {
            printf("tws rx post fail\n");
        }
    }
}

//发送给对耳
REGISTER_TWS_FUNC_STUB(app_vol_sync_stub) = {
    .func_id = 0x23080315,
    .func    = gfps_protocol_rx_from_sibling,
};

int gfps_message_deal_handler(int id, int opcode, u8 *data, u32 len)
{
    switch (opcode) {
    case APP_PROTOCOL_LIB_TWS_DATA_SYNC:
        gfps_protocol_tws_send_to_sibling(opcode, data, len);
        break;
    case APP_PROTOCOL_GFPS_RING_STOP_ALL:
        printf("GFPS_RING_STOP_ALL");
        google_tone_ctl.l_en = 0;
        google_tone_ctl.r_en = 0;
        google_tone_play_deal();
        break;
    case APP_PROTOCOL_GFPS_RING_RIGHT:
        printf("GFPS_RING_RIGHT");
        google_tone_ctl.l_en = 0;
        google_tone_ctl.r_en = 1;
        google_tone_play_deal();
        break;
    case APP_PROTOCOL_GFPS_RING_LEFT:
        printf("GFPS_RING_LEFT");
        google_tone_ctl.l_en = 1;
        google_tone_ctl.r_en = 0;
        google_tone_play_deal();
        break;
    case APP_PROTOCOL_GFPS_RING_ALL:
        printf("GFPS_RING_ALL");
        google_tone_ctl.l_en = 1;
        google_tone_ctl.r_en = 1;
        google_tone_play_deal();
        break;
    case APP_PROTOCOL_GFPS_HEARABLE_CONTROLS:
        printf("APP_PROTOCOL_GFPS_HEARABLE_CONTROLS display:%x settable:%x current:%x\n", data[0], data[1], data[2]);
        break;
    }
    return 0;
}

//关机广播收仓
void gfps_need_adv_close_icon_set(u8 en)
{
    printf("func:%s, en:%d", __FUNCTION__, en);
    __this->close_icon = en;
}

void gfps_poweroff_adv_close_icon()
{
    printf("func:%s, line:%d", __FUNCTION__, __LINE__);
    if (__this->close_icon) {
        void set_wait_poweroff_time(u16 time);
        set_wait_poweroff_time(2000); //广播800ms
        gfps_adv_interval_set(48); //连接上手机间隔改为30ms
        gfps_set_battery_ui_enable(0); //关闭电量显示，会重新开关广播
    }
}

int gfps_hci_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        switch (bt->value) {
        case ERROR_CODE_PIN_OR_KEY_MISSING:
            printf("GFP linkkey missing");
            if (!btstack_get_num_of_remote_device_recorded()) { //没有配对记录的时候，广播初始配对广播
                gfps_all_info_recover(0);
                gfps_adv_interval_set(INIT_PAIR_ADV_INTERVAL); //连接上手机间隔改为93.75ms
                gfps_set_pair_mode_by_user(0);
            }
            break;
        }
        break;
    }
    return 0;
}

int gfps_bt_status_event_handler(struct bt_event *bt)
{
    u8 ret = 0;
    switch (bt->event) {
    // 需要切换蓝牙的命令
    case BT_STATUS_A2DP_MEDIA_START:
        break;
    case BT_STATUS_INIT_OK:
        if (btstack_get_num_of_remote_device_recorded()) {
            __this->poweron_reconn = 1;
        }
        break;
    case BT_STATUS_SECOND_CONNECTED:
    case BT_STATUS_FIRST_CONNECTED:
        __this->poweron_reconn = 0;
        gfps_adv_interval_set(SUBS_PAIR_ADV_INTERVAL); //连接上手机间隔改为220ms，防止fast pair validator测试fail
        set_gfps_pair_state(1);
        gfps_bt_ble_adv_enable(0);
        gfps_bt_ble_adv_enable(1);
        break;
    case BT_STATUS_PHONE_INCOME:
    case BT_STATUS_PHONE_OUT:
    case BT_STATUS_PHONE_ACTIVE:
        gfps_ble_adv_enable(0);
        break;
    case BT_STATUS_PHONE_HANGUP:
        gfps_ble_adv_enable(1);
        break;
    }
    return 0;
}


static int gfps_app_power_event_handler(struct device_event *dev)
{
    switch (dev->event) {
#if TCFG_USER_TWS_ENABLE
    case POWER_EVENT_SYNC_TWS_VBAT_LEVEL:
        printf("update gfps bat");
        if (get_tws_sibling_bat_persent() != 0xFF) {
            google_tone_mute_ctl(0, get_tws_sibling_bat_persent() >> 7);
        }
        gfps_battery_update();
        break;
#endif
    case POWER_EVENT_POWER_CHANGE:
        printf("update gfps bat");
        google_tone_mute_ctl(1, get_charge_online_flag());
        gfps_battery_update();
        break;
    }
    return 0;
}

int gfps_sys_event_handler_specific(struct sys_event *event)
{
    switch (event->type) {
    case SYS_DEVICE_EVENT:
        if ((u32)event->arg ==  DEVICE_EVENT_FROM_POWER) {
            gfps_app_power_event_handler(&event->u.dev);
        }
        break;
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            gfps_bt_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            gfps_hci_event_handler(&event->u.bt);
        }
        break;
    }

    return 0;
}

struct app_protocol_private_handle_t gfps_private_handle = {
    .sys_event_handler = gfps_sys_event_handler_specific,
};

#endif
#endif



