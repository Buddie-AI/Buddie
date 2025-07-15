#include "app_protocol_api.h"
#include "app_protocol_common.h"
#include "system/includes.h"
#include "app_config.h"
#include "audio_config.h"
#include "app_chargestore.h"
#include "app_power_manage.h"
#include "asm/charge.h"
#include "bt_tws.h"
#include "btstack/avctp_user.h"
#include "3th_profile_api.h"
#include "bt_common.h"
#include "tone_player.h"
#include "app_main.h"
#include "key_event_deal.h"

#if AI_APP_PROTOCOL

#if 1
#define APP_PROTOCOL_LOG    printf
#define APP_PROTOCOL_DUMP   put_buf
#else
#define APP_PROTOCOL_LOG(...)
#define APP_PROTOCOL_DUMP(...)
#endif

__attribute__((weak))
void app_protocol_tone_play(int index, int tws_sync)
{

}
static const char **app_tone_table = NULL;
static struct app_protocol_private_handle_t *private_handle;

extern u8 get_remote_dev_company(void);
#define REMOTE_DEV_ANDROID			1
#define REMOTE_DEV_IOS				2
bool is_ios_system(void)
{
    APP_PROTOCOL_LOG("remote_dev_company :%d \n", get_remote_dev_company());
    return (get_remote_dev_company() == REMOTE_DEV_IOS);
}

void app_protocol_tone_register(const char **tone_table)
{
    app_tone_table = tone_table;
}

const char *app_protocol_get_tone(int index)
{
    if (app_tone_table) {
        return app_tone_table[index];
    }
    return NULL;
}

void app_protocol_handle_register(struct app_protocol_private_handle_t *hd)
{
    private_handle = hd;
}

struct app_protocol_private_handle_t *app_protocol_get_handle()
{
    return private_handle;
}

//协议私有同步消息处理函数
void app_protocol_tws_sync_private_deal(int cmd, int value)
{
    if (private_handle && private_handle->tws_sync_func) {
        private_handle->tws_sync_func(cmd, value);
    }
}

//协议私有收到对耳数据处理函数
void app_protocol_tws_rx_data_private_deal(u16 opcode, u8 *data, u16 len)
{
    if (private_handle && private_handle->tws_rx_from_siblling) {
        private_handle->tws_rx_from_siblling(opcode, data, len);
    }
}

int app_protocol_sys_event_private_deal(struct sys_event *event)
{
    if (private_handle && private_handle->sys_event_handler) {
        return private_handle->sys_event_handler(event);
    }
    return 0;
}

#if APP_PROTOCOL_SPEECH_EN
#define APP_PROTOCOL_MIC_TIMEOUT    8000 //mic使用超时时间，0不限时

static u16 mic_timer = 0;
static u16 tone_after_exit_sniff_timer = 0;
static u8 exit_sniff_cnt = 0;
void __app_protocol_speech_stop(void)
{
    if (ai_mic_is_busy()) {
        if (mic_timer) {
            sys_timeout_del(mic_timer);
            mic_timer = 0;
        }
        APP_PROTOCOL_LOG("app_protocol_speech_stop\n");

        ai_mic_rec_close();
        app_protocol_stop_speech_cmd();
    }
}

static void app_protocol_speech_timeout(void *priv)
{
    APP_PROTOCOL_LOG(" speech timeout !!! \n");
    __app_protocol_speech_stop();
}

static int app_protocol_speech_start_check()
{
    if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) {
        APP_PROTOCOL_LOG("phone ing...\n");
        return -1;
    }
    if (ai_mic_is_busy()) {
        APP_PROTOCOL_LOG("mic activing...\n");
        return -1;
    }
    return 0;
}

int __app_protocol_speech_start(void)
{
    if (!app_protocol_speech_start_check() && ai_mic_rec_start() == 0) {
#if APP_PROTOCOL_MIC_TIMEOUT
        mic_timer = sys_timeout_add(NULL, app_protocol_speech_timeout, APP_PROTOCOL_MIC_TIMEOUT);
#endif
        return 0;
    }
    APP_PROTOCOL_LOG("speech start fail\n");
    return -1;
}

int app_protocol_start_speech_by_app(u8 tone_en)
{
    if (app_protocol_speech_start_check()) {
        return -1;
    }
    if (app_var.siri_stu) {
        APP_PROTOCOL_LOG("siri activing...\n");
        return -1;
    }
    if (tone_en) {
        app_protocol_tone_play(APP_RROTOCOL_TONE_SPEECH_APP_START, 1);
    } else {
        app_protocol_post_bt_event(AI_EVENT_SPEECH_START, NULL);
    }
    return 0;
}

int app_protocol_stop_speech_by_app()
{
    app_protocol_post_bt_event(AI_EVENT_SPEECH_STOP, NULL);
    return 0;
}

static void speech_tone_play_after_exit_sniff()
{
    tone_after_exit_sniff_timer = 0;
    if (bt_is_sniff_close() && exit_sniff_cnt < 13) {
        exit_sniff_cnt++;
        tone_after_exit_sniff_timer = sys_timeout_add(NULL, speech_tone_play_after_exit_sniff, 100);
        return;
    }
    exit_sniff_cnt = 0;
    if (!app_protocol_speech_start_check()) {
        app_protocol_tone_play(APP_RROTOCOL_TONE_SPEECH_KEY_START, 1);
    }
}

int app_protocol_start_speech_by_key(struct sys_event *event)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status() && !is_tws_master_role()) {
        APP_PROTOCOL_LOG("tws slave, return\n");
        return 0;
    }
#endif

    user_send_cmd_prepare(USER_CTRL_HFP_GET_SIRI_STATUS, 0, NULL);
    if (app_protocol_speech_start_check()) {
        return -1;
    }

    if (app_protocol_check_connect_success() && (get_curr_channel_state()&A2DP_CH)) {
        printf("no connect\n");
    } else {
        if (get_curr_channel_state()&A2DP_CH) {
            //<协议未连接， 但是A2DP已连接， 点击唤醒键， 提示TTS【请打开小度APP】
            if (is_ios_system()) {
                APP_PROTOCOL_LOG("ble adv ibeaconn");
                app_protocol_ibeacon_switch(1);
            } else {
                user_send_cmd_prepare(USER_CTRL_HFP_GET_SIRI_OPEN, 0, NULL);
            }
            app_protocol_tone_play(APP_PROTOCOL_TONE_DISCONNECTED_ALL, 1);
        } else {
            //<蓝牙完全关闭状态， 用户按唤醒键， 提示TTS【蓝牙未连接， 请用手机蓝牙和我连接吧】
            app_protocol_tone_play(APP_RROTOCOL_TONE_OPEN_APP, 1);
        }
        APP_PROTOCOL_LOG("A2dp or protocol no connect err!!!\n");
        return -1;
    }


#if 1//等退出sniff后再播提示音
    if (bt_is_sniff_close()) {
        user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
        if (tone_after_exit_sniff_timer == 0) {
            tone_after_exit_sniff_timer = sys_timeout_add(NULL, speech_tone_play_after_exit_sniff, 100);
        }
        return 0;
    }
    exit_sniff_cnt = 0;
#endif

    app_protocol_tone_play(APP_RROTOCOL_TONE_SPEECH_KEY_START, 1);
    return 0;
}

int app_protocol_stop_speech_by_key(void)
{
    return 0;
}
#else
void __app_protocol_speech_stop(void)
{
}
#endif

void app_speech_tone_play_end_callback(void *priv, int flag)
{
    u32 index = (u32)priv;
    switch (index) {
#if APP_PROTOCOL_SPEECH_EN
    case APP_RROTOCOL_TONE_SPEECH_KEY_START:
        APP_PROTOCOL_LOG("APP_RROTOCOL_TONE_SPEECH_KEY_START\n");
        if (app_protocol_speech_start_check()) {
            break;
        }
        if (app_var.siri_stu) {
            APP_PROTOCOL_LOG("siri activing...\n");
            break;
        }
        ///按键启动需要report, app启动， 直接启动语音即可
        if (is_tws_master_role() && !app_protocol_start_speech_cmd()) {//从转发mic数据给主机或主机上报完成开启mic
            APP_PROTOCOL_LOG("app_protocol_mic_status_report success\n");
            app_protocol_post_bt_event(AI_EVENT_SPEECH_START, NULL);
        }
        break;
    case APP_RROTOCOL_TONE_SPEECH_APP_START:
        APP_PROTOCOL_LOG("APP_RROTOCOL_TONE_SPEECH_APP_START\n");
        if (app_protocol_speech_start_check()) {
            break;
        }
        if (app_var.siri_stu) {
            APP_PROTOCOL_LOG("siri activing...\n");
            break;
        }
        if (is_tws_master_role()) {
            app_protocol_post_bt_event(AI_EVENT_SPEECH_START, NULL);
        }
        break;
#endif
    default:
        break;
    }
}

int app_protocol_set_volume(u8 vol)
{
    u8 vol_l = (vol * get_max_sys_vol()) / 0x64;
    u8 vol_r = (vol * get_max_sys_vol()) / 0x64;

    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, APP_AUDIO_STATE_MUSIC, vol_r);
    return 0;
}
__attribute__((weak))
bool vbat_is_low_power(void)
{
    return 0;
}
u8 app_protocal_get_bat_by_type(u8 type)
{
    u8 value = 0;
    extern u8 get_tws_sibling_bat_persent(void);
    u8 sibling_val = get_tws_sibling_bat_persent();
#if TCFG_CHARGESTORE_ENABLE
    if (sibling_val == 0xff) {
        sibling_val = chargestore_get_sibling_power_level();
    }
#endif
    switch (type) {
    case APP_PROTOCOL_BAT_T_CHARGE_FLAG:
        value = get_charge_online_flag();
        break;
    case APP_PROTOCOL_BAT_T_MAIN:
        value = get_vbat_percent();
        break;
#if TCFG_CHARGESTORE_ENABLE
    case APP_PROTOCOL_BAT_T_BOX:
        value = chargestore_get_power_level();
        break;
#endif
    case APP_PROTOCOL_BAT_T_TWS_LEFT:
        value = (tws_api_get_local_channel() == 'L') ? get_vbat_percent() : sibling_val;
        break;
    case APP_PROTOCOL_BAT_T_TWS_RIGHT:
        value = (tws_api_get_local_channel() == 'R') ? get_vbat_percent() : sibling_val;
        break;
    case APP_PROTOCOL_BAT_T_TWS_SIBLING:
        value = sibling_val;
        break;
    case APP_PROTOCOL_BAT_T_LOW_POWER:
        value = vbat_is_low_power();
        break;
    default:
        break;
    }
    if (value == 0xff) { //获取不到电量返回0
        value = 0;
    }
    return value;
}

u32 read_cfg_file(void *buf, u16 len, char *path)
{
    FILE *fp =  fopen(path, "r");
    int rlen;

    if (!fp) {
        //printf("read_cfg_file:fopen err!\n");
        return FALSE;
    }

    rlen = fread(fp, buf, len);
    if (rlen <= 0) {
        //printf("read_cfg_file:fread err!\n");
        return FALSE;
    }

    fclose(fp);

    return TRUE;
}

#define LIC_PAGE_OFFSET 		80
#define FORCE_TO_ERASE		    1

typedef enum _FLASH_ERASER {
    CHIP_ERASER = 0,
    BLOCK_ERASER,
    SECTOR_ERASER,
    PAGE_ERASER,
} FLASH_ERASER;

typedef struct __flash_of_lic_para_head {
    s16 crc;
    u16 string_len;
    const u8 para_string[];
} __attribute__((packed)) _flash_of_lic_para_head;

static bool license_para_head_check(u8 *para)
{
    _flash_of_lic_para_head *head;

    //fill head
    head = (_flash_of_lic_para_head *)para;

    ///crc check
    u8 *crc_data = (u8 *)(para + sizeof(((_flash_of_lic_para_head *)0)->crc));
    u32 crc_len = sizeof(_flash_of_lic_para_head) - sizeof(((_flash_of_lic_para_head *)0)->crc)/*head crc*/ + (head->string_len)/*content crc,include end character '\0'*/;
    s16 crc_sum = 0;

    crc_sum = CRC16(crc_data, crc_len);

    if (crc_sum != head->crc) {
        printf("license crc error !!! %x %x \n", (u32)crc_sum, (u32)head->crc);
        return false;
    }

    return true;
}

//获取三元组信息的头指针
const u8 *app_protocal_get_license_ptr(void)
{
    u32 flash_capacity = sdfile_get_disk_capacity();
    u32 flash_addr = flash_capacity - 256 + LIC_PAGE_OFFSET;
    u8 *lic_ptr = NULL;
    _flash_of_lic_para_head *head;

    printf("flash capacity:%x \n", flash_capacity);
    lic_ptr = (u8 *)sdfile_flash_addr2cpu_addr(flash_addr);

    //head length check
    head = (_flash_of_lic_para_head *)lic_ptr;
    if (head->string_len >= 0xff) {
        printf("license length error !!! \n");
        return NULL;
    }

    ////crc check
    if (license_para_head_check(lic_ptr) == (false)) {
        printf("license head check fail\n");
        return NULL;
    }

    //put_buf(lic_ptr, 128);

    lic_ptr += sizeof(_flash_of_lic_para_head);
    return lic_ptr;
}

extern bool sfc_erase(int cmd, u32 addr);
extern u32 sfc_write(const u8 *buf, u32 addr, u32 len);
//将三元组信息保存到flash里面
int app_protocol_license2flash(const u8 *data, u16 len)
{
    _flash_of_lic_para_head header;
    u8 buffer[256];
    u32 flash_capacity = sdfile_get_disk_capacity();
    u32 sector = flash_capacity - 4 * 1024;
    u32 page_addr = flash_capacity - 256;
    u8 *auif_ptr = (u8 *)sdfile_flash_addr2cpu_addr(page_addr);

#if (!FORCE_TO_ERASE)
    APP_PROTOCOL_LOG("not supported flash erase !!! \n");
    return (-1);
#endif

    ///save last 256 byte
    /* memset(buffer, 0xff, sizeof(buffer)); */
    memcpy(buffer, auif_ptr, sizeof(buffer));
    auif_ptr += LIC_PAGE_OFFSET;

    header.string_len = len;
    ///length
    memcpy(&buffer[LIC_PAGE_OFFSET + sizeof(header.crc)], &(header.string_len), sizeof(header.string_len));
    ///context
    memcpy(&buffer[LIC_PAGE_OFFSET + sizeof(header)], data, len);
    ///crc
    header.crc = CRC16(&buffer[LIC_PAGE_OFFSET + sizeof(header.crc)], sizeof(header.string_len) + header.string_len);
    memcpy(&buffer[LIC_PAGE_OFFSET], &(header.crc), sizeof(header.crc));

    ///check if need update data to flash,erease license erea if there are some informations in license erea
    if (!memcmp(auif_ptr, buffer + LIC_PAGE_OFFSET, len + sizeof(_flash_of_lic_para_head))) {
        APP_PROTOCOL_LOG("flash information the same with license\n");
        return 0;
    }

    APP_PROTOCOL_LOG("erase license sector \n");
    sfc_erase(SECTOR_ERASER, sector);

    APP_PROTOCOL_LOG("write license to flash \n");
    sfc_write(buffer, page_addr, 256);

    return 0;
}

int app_protocol_tws_send_to_sibling(u16 opcode, u8 *data, u16 len)
{
    u8 send_data[len + 4];
    printf("app protocol send data to sibling \n");
    send_data[0] = opcode & 0xff;
    send_data[1] = opcode >> 8;
    send_data[2] = len & 0xff;
    send_data[3] = len >> 8;
    memcpy(send_data + 4, data, len);

    return tws_api_send_data_to_sibling(send_data, sizeof(send_data), APP_PROTOCOL_TWS_SEND_ID);
}

static void __tws_rx_from_sibling(u8 *data)
{
    u16 opcode = (data[1] << 8) | data[0];
    u16 len = (data[3] << 8) | data[2];
    u8 *rx_data = data + 4;

    switch (opcode) {
    case APP_PROTOCOL_TWS_FOR_LIB:
        APP_PROTOCOL_LOG("APP_PROTOCOL_TWS_FOR_LIB");
        app_protocol_get_tws_data_for_lib(rx_data, len);
        break;
    }
    app_protocol_tws_rx_data_private_deal(opcode, rx_data, len);

    free(data);
}

static void app_protocol_rx_from_sibling(void *_data, u16 len, bool rx)
{
    int err = 0;
    if (rx) {
        printf(">>>%s \n", __func__);
        printf("len :%d\n", len);
        put_buf(_data, len);
        u8 *rx_data = malloc(len);
        memcpy(rx_data, _data, len);

        int msg[4];
        msg[0] = (int)__tws_rx_from_sibling;
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
    .func_id = APP_PROTOCOL_TWS_SEND_ID,
    .func    = app_protocol_rx_from_sibling,
};

//对耳同步执行
int app_protocol_tws_sync_send(int cmd, int value)
{
    int data[2];
    data[0] = cmd;
    data[1] = value;
    return tws_api_send_data_to_slave(data, sizeof(data), APP_PROTOCOL_TWS_SYNC_ID);
}

void app_protocol_tws_sync_deal(int cmd, int value)
{
    switch (cmd) {
    case APP_PROTOCOL_SYNC_TONE:
        printf("APP_PROTOCOL_SYNC_TONE:%d\n", value);
        app_protocol_tone_play(value, 0);
        break;
    }
    app_protocol_tws_sync_private_deal(cmd, value);
}

static void app_protocol_tws_sync_rx(void *data, u16 len, bool rx)
{
    int err = 0;
    int *arg = (int *)data;
    int msg[8];

    msg[0] = (int)app_protocol_tws_sync_deal;
    msg[1] = len / sizeof(int);
    msg[2] = arg[0]; //cmd
    msg[3] = arg[1]; //value

    err = os_taskq_post_type("app_core", Q_CALLBACK, 2 + len / sizeof(int), msg);
    if (err) {
        printf("tws sync post fail\n");
    }
}

REGISTER_TWS_FUNC_STUB(app_protocol_sync) = {
    .func_id = APP_PROTOCOL_TWS_SYNC_ID,
    .func    = app_protocol_tws_sync_rx,
};

//在app_core任务里面进行处理
int app_protocol_post_app_core_callback(int callback, void *priv)
{
    int msg[8];
    msg[0] = callback;
    msg[1] = 1;
    msg[2] = (int)priv;
    return os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
}

void app_protocol_post_bt_event(u8 event, void *priv)
{
    struct sys_event e;

    memset(&e, 0, sizeof(e));
    e.type = SYS_BT_EVENT;
    e.arg  = (void *)SYS_BT_AI_EVENT_TYPE_STATUS;
    e.u.bt.event = event;
    e.u.bt.value = (int)priv;
    sys_event_notify(&e);
}


static int app_protocol_bt_status_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        app_protocol_all_init();
        break;
    case BT_STATUS_SECOND_CONNECTED:
    case BT_STATUS_FIRST_CONNECTED:
        if (app_protocol_check_connect_success()) {
            app_protocol_tone_play(APP_PROTOCOL_TONE_CONNECTED_ALL_FINISH, 1);
        } else {
            app_protocol_tone_play(APP_PROTOCOL_TONE_CONNECTED_NEED_OPEN_APP, 1);
        }
        break;
    case BT_STATUS_FIRST_DISCONNECT:
    case BT_STATUS_SECOND_DISCONNECT:
        app_protocol_tone_play(APP_PROTOCOL_TONE_DISCONNECTED, 1);
        app_protocal_update_tws_state_to_lib(USER_NOTIFY_STATE_MOBILE_DISCONNECTED);
        break;
    case BT_STATUS_PHONE_INCOME:
    case BT_STATUS_PHONE_OUT:
    case BT_STATUS_PHONE_ACTIVE:
        __app_protocol_speech_stop();
        app_protocol_ble_adv_switch(0);
        break;
    case BT_STATUS_PHONE_HANGUP:
        app_protocol_ble_adv_switch(1);
        break;

    case BT_STATUS_PHONE_MANUFACTURER:
        if (!app_protocol_check_connect_success()) {
            if (is_ios_system()) {
                app_protocol_ibeacon_switch(1);
            }

            if (get_call_status() != BT_CALL_HANGUP) {
                app_protocol_ble_adv_switch(0);
            }
        }
        break;
    case BT_STATUS_VOICE_RECOGNITION:
        if ((app_var.siri_stu == 1) || (app_var.siri_stu == 2)) {
            __app_protocol_speech_stop();
            app_protocol_ble_adv_switch(0);
        } else if (app_var.siri_stu == 0) {
            app_protocol_ble_adv_switch(1);
        }
        break;
    case BT_STATUS_CONN_A2DP_CH:
        app_protocal_update_tws_state_to_lib(USER_NOTIFY_STATE_MOBILE_CONNECTED);
        break;
    }
    return 0;
}

static void app_protocol_bt_tws_event_handler(struct bt_event *bt)
{
    int role = bt->args[0];
    int phone_link_connection = bt->args[1];
    int reason = bt->args[2];

    switch (bt->event) {
    case TWS_EVENT_CONNECTED:
        if (!is_tws_master_role()) {
            app_protocol_disconnect(NULL);
            app_protocol_ble_adv_switch(0);
        } else {
#if APP_PROTOCOL_SPEECH_EN
            if (ai_mic_is_busy()) { //mic在解码过程中，从机连接成功，挂起音频
                ai_mic_tws_sync_suspend();
            }
#endif
        }
        app_protocal_update_tws_state_to_lib(USER_NOTIFY_STATE_TWS_CONNECT);
        break;
    case TWS_EVENT_CONNECTION_DETACH:
        if (!app_protocol_check_connect_success()) { //APP未连接，开启广播
            if (0 == get_esco_coder_busy_flag()) {
                //esco在用的时候开广播会影响质量
                app_protocol_ble_adv_switch(1);
            }
        }
#if APP_PROTOCOL_SPEECH_EN
        if (!ai_mic_is_busy()) {
            __app_protocol_speech_stop(); //对耳断开时没启动mic，停止语音
        }
#endif
        app_protocal_update_tws_state_to_lib(USER_NOTIFY_STATE_BATTERY_LEVEL_UPDATE); //主动上报电量
        app_protocal_update_tws_state_to_lib(USER_NOTIFY_STATE_TWS_DISCONNECT);
        break;
    case TWS_EVENT_ROLE_SWITCH:
        app_protocal_update_tws_state_to_lib(USER_NOTIFY_STATE_ROLE_SWITCH_FINISH);
        app_protocol_disconnect(NULL);
        if (role == TWS_ROLE_MASTER && (0 == get_esco_coder_busy_flag())) {
            app_protocol_ble_adv_switch(1);
        } else {
            app_protocol_ble_adv_switch(0);
        }
        break;
    }
}

static int app_protocol_bt_ai_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
#if APP_PROTOCOL_SPEECH_EN
    case AI_EVENT_SPEECH_START:
        APP_PROTOCOL_LOG("AI_EVENT_SPEECH_START");
        __app_protocol_speech_start();
        break;
    case AI_EVENT_SPEECH_STOP:
        APP_PROTOCOL_LOG("AI_EVENT_SPEECH_STOP");
        __app_protocol_speech_stop();
        break;
#endif
    case AI_EVENT_APP_CONNECT:
        APP_PROTOCOL_LOG("AI_EVENT_APP_CONNECT");
        if (get_curr_channel_state()&A2DP_CH) {
            app_protocol_tone_play(APP_PROTOCOL_TONE_CONNECTED_ALL_FINISH, 1);
        } else {
            app_protocol_tone_play(APP_PROTOCOL_TONE_PROTOCOL_CONNECTED, 1);
        }
        break;
    case AI_EVENT_APP_DISCONNECT:
        APP_PROTOCOL_LOG("AI_EVENT_APP_DISCONNECT");
        __app_protocol_speech_stop();
        break;
    }
    return 0;
}

int app_protocol_sys_event_handler(struct sys_event *event)
{
#if TCFG_USER_GFPS_PROCESS_CTL
    if (app_user_protocol_sys_event_remap_hdl(event) == true) {
        return 0;
    }
#endif
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            app_protocol_bt_status_event_handler(&event->u.bt);
        }
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            app_protocol_bt_tws_event_handler(&event->u.bt);
        }
#endif
        else if (((u32)event->arg == SYS_BT_AI_EVENT_TYPE_STATUS)) {
            app_protocol_bt_ai_event_handler(&event->u.bt);
        }
        break;

    case SYS_KEY_EVENT:
        app_protocol_key_event_handler(event);
        break;
    }
    app_protocol_sys_event_private_deal(event);

    return 0;
}
void tell_lib_start_role_switch()
{
    app_protocol_ble_adv_switch(0);
    if (is_tws_master_role()) {
        printf("master dev will role switch\n");
        app_protocal_update_tws_state_to_lib(USER_NOTIFY_STATE_ROLE_SWITCH_START);
        /* os_time_dly(10); */
    }
}
#else
int app_protocol_sys_event_handler(struct sys_event *event)
{
    return 0;
}
void tell_lib_start_role_switch()
{

}

#endif


