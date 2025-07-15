#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "string.h"
#include "attr.h"
#include "JL_rcsp_api.h"
#include "JL_rcsp_protocol.h"
#include "JL_rcsp_packet.h"
#include "spp_user.h"
#include "btstack/avctp_user.h"
#include "system/timer.h"
#include "app_core.h"
#include "user_cfg.h"
#include "asm/pwm_led.h"
#include "ui_manage.h"
#include "key_event_deal.h"
#include "syscfg_id.h"
#include "classic/tws_api.h"
#include "event.h"
#include "bt_tws.h"
#include "custom_cfg.h"
#include "app_config.h"

#include "rcsp_adv_user_update.h"
#include "le_rcsp_adv_module.h"
#include "rcsp_adv_bluetooth.h"
#include "adv_setting_common.h"
#include "adv_eq_setting.h"
#include "adv_music_info_setting.h"
#include "adv_high_low_vol.h"
#include "adv_work_setting.h"
#include "rcsp_adv_customer_user.h"
#include "vm.h"

#include "tone_player.h"
#include "audio_config.h"

#include "adv_anc_voice.h"
#include "adv_adaptive_noise_reduction.h"
#include "adv_anc_voice_key.h"
#include "audio_anc.h"
#include "adv_ai_no_pick.h"
#include "adv_scene_noise_reduction.h"
#include "adv_wind_noise_detection.h"
#include "adv_voice_enhancement_mode.h"

#include "adv_hearing_aid_setting.h"

#if (RCSP_ADV_EN)
#define JL_RCSP_LOW_VERSION  0x01
#define JL_RCSP_HIGH_VERSION 0x01

/* #define RCSP_DEBUG_EN */
#ifdef RCSP_DEBUG_EN
#define rcsp_putchar(x)                putchar(x)
#define rcsp_printf                    printf
#define rcsp_printf_buf(x,len)         put_buf(x,len)
#else
#define rcsp_putchar(...)
#define rcsp_printf(...)
#define rcsp_printf_buf(...)
#endif

struct JL_AI_VAR jl_ai_var = {
    .rcsp_run_flag = 0,
};

#define __this (&jl_ai_var)

#define RCSP_USE_BLE      0
#define RCSP_USE_SPP      1
#define RCSP_CHANNEL_SEL  RCSP_USE_SPP

#define COMMON_FUNCTION    		0xFF
#define BT_FUNCTION    		    0x0


#pragma pack(1)

struct _SYS_info {
    u8 bat_lev;
    u8 sys_vol;
    u8 max_vol;
    u8 reserve;
};

struct _EDR_info {
    u8 addr_buf[6];
    u8 profile;
    u8 state;
};

struct _DEV_info {
    u8 status;
    u32 usb_handle;
    u32 sd0_handle;
    u32 sd1_handle;
    u32 flash_handle;
};


struct _MUSIC_STATUS_info {
    u8 status;
    u32 cur_time;
    u32 total_time;
    u8 cur_dev;
};

struct _dev_version {
    u16 _sw_ver2: 4; //software l version
    u16 _sw_ver1: 4; //software m version
    u16 _sw_ver0: 4; //software h version
    u16 _hw_ver:  4; //hardware version
};


#pragma pack()

/* #pragma pack(1) */
/* #pragma pack() */

#define RES_MD5_FILE	SDFILE_RES_ROOT_PATH"md5.bin"

#if 1//RCSP_UPDATE_EN
static volatile u8 JL_bt_chl = 0;
u8 JL_get_cur_bt_channel_sel(void);
void JL_ble_disconnect(void);
void rcsp_reboot_dev(void);
u8 rcsp_get_update_flag(void);
#endif

u8 get_rcsp_support_new_reconn_flag(void)
{
    return __this->new_reconn_flag;
}

u8 get_rcsp_connect_status(void)
{
#if 1//RCSP_UPDATE_EN
    if (RCSP_BLE == JL_get_cur_bt_channel_sel()) {
        if (__this->JL_ble_status == BLE_ST_CONNECT || __this->JL_ble_status == BLE_ST_NOTIFY_IDICATE) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return __this->JL_spp_status;
    }
#else
    return 0;
#endif
}

extern void rcsp_update_data_api_unregister(void);
static u16 common_function_info(u32 mask, u8 *buf, u16 len);
static u16 bt_function_info(u32 mask, u8 *buf, u16 len);
#if RCSP_ADV_FIND_DEVICE_ENABLE
static void earphone_mute_handler(u8 *other_opt, u32 msec);
static void find_device_timeout_handle(u32 sec);
void find_decice_tws_connect_handle(u8 flag, u8 *param);
#endif

void JL_rcsp_event_to_user(u32 type, u8 event, u8 *msg, u8 size)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;

    if (size > sizeof(e.u.rcsp.args)) {
        rcsp_printf("rcsp event size overflow:%x %x\n", size, sizeof(e.u.rcsp.args));
    }

    e.arg  = (void *)type;
    e.u.rcsp.event = event;

    if (size) {
        memcpy(e.u.rcsp.args, msg, size);
    }

    e.u.rcsp.size = size;

    sys_event_notify(&e);
}

static u32 JL_auto_update_sys_info(u8 fun_type, u32 mask)
{
    u8 buf[512];
    u8 offset = 0;
    u32 ret = 0;

    rcsp_printf("JL_auto_update_sys_info\n");

    buf[0] = fun_type;

    u8 *p = buf + 1;
    u16 size = sizeof(buf) - 1;

    switch (fun_type) {
    case COMMON_FUNCTION:
        rcsp_printf("COMMON_FUNCTION\n");
        offset = common_function_info(mask, p, size);
        break;
    case BT_FUNCTION:
        rcsp_printf("BT_FUNCTION\n");
        offset = bt_function_info(mask, p, size);
        break;
    default:
        break;
    }

    rcsp_printf_buf(buf, offset + 1);

    ret = JL_CMD_send(JL_OPCODE_SYS_INFO_AUTO_UPDATE, buf, offset + 1, JL_NOT_NEED_RESPOND);
    return ret;
}

void JL_bt_phone_state_update(void)
{
    JL_auto_update_sys_info(COMMON_FUNCTION, BIT(COMMON_INFO_ATTR_PHONE_SCO_STATE_INFO));
}

int JL_rcsp_event_handler(struct rcsp_event *rcsp)
{
    int ret = 0;

    switch (rcsp->event) {
    case MSG_JL_ADV_SETTING_SYNC:
        update_adv_setting((u16) - 1);
        break;
    case MSG_JL_ADV_SETTING_UPDATE:
        update_info_from_adv_vm_info();
        break;
    case MSG_JL_UPDATE_EQ:
        JL_auto_update_sys_info(COMMON_FUNCTION, BIT(COMMON_INFO_ATTR_EQ));
        break;
    case MSG_JL_UPDATE_SEQ:
        printf("MSG_JL_UPDATE_SEQ\n");
        extern void adv_seq_vaule_sync(void);
        adv_seq_vaule_sync();
        break;
    case MSG_JL_SWITCH_DEVICE:
        if (RCSP_BLE == JL_get_cur_bt_channel_sel() && RCSP_SPP == rcsp->args[0]) {
            JL_ble_disconnect();
        }
        break;
#if RCSP_ADV_MUSIC_INFO_ENABLE
    case MSG_JL_UPDATE_PLAYER_TIME:
        if (JL_rcsp_get_auth_flag()) {
            JL_auto_update_sys_info(BT_FUNCTION, 0x100);
        }
        break;
    case MSG_JL_UPDATE_PLAYER_STATE:
        if (JL_rcsp_get_auth_flag()) {
            JL_auto_update_sys_info(BT_FUNCTION, 0x80);
        }
        break;
    case MSG_JL_UPDATE_MUSIC_INFO:
        if (JL_rcsp_get_auth_flag()) {
            /* printf("rcsp type %x\n",rcsp->args[0]); */
            JL_auto_update_sys_info(BT_FUNCTION, BIT(rcsp->args[0] - 1));
        }
        break;
    case  MSG_JL_UPDATE_MUSIC_PLAYER_TIME_TEMER:
        if (JL_rcsp_get_auth_flag()) {
            music_player_time_timer_deal(rcsp->args[0]);
        }
        break;
#endif
#if (RCSP_ADV_ANC_VOICE)
    case MSG_JL_UPDATE_ANC_VOICE:
        printf("MSG_JL_UPDATE_ANC_VOICE\n");
#if RCSP_ADV_ADAPTIVE_NOISE_REDUCTION
        JL_auto_update_sys_info(COMMON_FUNCTION, (BIT(COMMON_INFO_ATTR_ANC_VOICE) | BIT(COMMON_INFO_ATTR_ADAPTIVE_NOISE_REDUCTION)));
#else
        JL_auto_update_sys_info(COMMON_FUNCTION, BIT(COMMON_INFO_ATTR_ANC_VOICE));
#endif
        break;
    case MSG_JL_UPDATE_ANC_VOICE_MAX_SYNC:
        printf("MSG_JL_UPDATE_ANC_VOICE_MAX_SYNC\n");
#if TCFG_USER_TWS_ENABLE
        extern void anc_voice_max_val_swap_sync(void);
        anc_voice_max_val_swap_sync();
#endif
        break;
#endif
#if RCSP_ADV_ADAPTIVE_NOISE_REDUCTION
    case MSG_JL_UPDATE_ADAPTIVE_NOISE_REDUCTION:
        printf("MSG_JL_UPDATE_ADAPTIVE_NOISE_REDUCTION\n");
        JL_auto_update_sys_info(COMMON_FUNCTION, BIT(COMMON_INFO_ATTR_ADAPTIVE_NOISE_REDUCTION));
        break;
#endif
#if RCSP_ADV_AI_NO_PICK
    case MSG_JL_UPDATE_AI_NO_PICK:
        printf("MSG_JL_UPDATE_AI_NO_PICK\n");
        JL_auto_update_sys_info(COMMON_FUNCTION, BIT(COMMON_INFO_ATTR_AI_NO_PICK));
        break;
#endif
#if RCSP_ADV_SCENE_NOISE_REDUCTION
    case MSG_JL_UPDATE_SCENE_NOISE_REDUCTION:
        printf("MSG_JL_UPDATE_SCENE_NOISE_REDUCTION\n");
        JL_auto_update_sys_info(COMMON_FUNCTION, BIT(COMMON_INFO_ATTR_SCENE_NOISE_REDUCTION));
        break;
#endif
#if RCSP_ADV_WIND_NOISE_DETECTION
    case MSG_JL_UPDATE_WIND_NOISE_DETECTION:
        printf("MSG_JL_UPDATE_WIND_NOISE_DETECTION\n");
        JL_auto_update_sys_info(COMMON_FUNCTION, BIT(COMMON_INFO_ATTR_WIND_NOISE_DETECTION));
        break;
#endif
#if RCSP_ADV_VOICE_ENHANCEMENT_MODE
    case MSG_JL_UPDATE_VOICE_ENHANCEMENT_MODE:
        printf("MSG_JL_UPDATE_VOICE_ENHANCEMENT_MODE\n");
        JL_auto_update_sys_info(COMMON_FUNCTION, BIT(COMMON_INFO_ATTR_VOICE_ENHANCEMENT_MODE));
        break;
#endif
    case MSG_JL_UPDAET_ADV_STATE_INFO:
        if (get_rcsp_connect_status()) {
            JL_rcsp_set_auth_flag(rcsp->args[0]);
            bt_ble_adv_ioctl(BT_ADV_SET_NOTIFY_EN, rcsp->args[1], 1);
        }
        break;


    case MSG_JL_REBOOT_DEV:
        rcsp_reboot_dev();
        break;

#if RCSP_ADV_FIND_DEVICE_ENABLE
    case MSG_JL_FIND_DEVICE_RESUME:
        printf("MSG_JL_FIND_DEVICE_RESUME\n");
#if TCFG_USER_TWS_ENABLE
        find_decice_tws_connect_handle(2, rcsp->args);
#endif
        earphone_mute_handler(rcsp->args, 300);
        break;
    case MSG_JL_FIND_DEVICE_STOP:
        printf("MSG_JL_FIND_DEVICE_STOP\n");
        u16 sec = *((u16 *)rcsp->args);
#if TCFG_USER_TWS_ENABLE
        find_decice_tws_connect_handle(1, rcsp->args);
#endif
        find_device_timeout_handle(sec);
        break;
#endif
    case MSG_JL_TWS_NEED_UPDATE:
        printf("MSG_JL_TWS_NEED_UPDATE\n");
        syscfg_write(VM_UPDATE_FLAG, rcsp->args, 1);
        break;

    default:
#if RCSP_UPDATE_EN
        JL_rcsp_msg_deal(NULL, rcsp->event, rcsp->args);
#endif
        break;
    }

    return ret;
}

u8 adv_info_notify(u8 *buf, u16 len)
{
    return JL_CMD_send(JL_OPCODE_ADV_DEVICE_NOTIFY, buf, len, JL_NOT_NEED_RESPOND);
}

u8 adv_info_device_request(u8 *buf, u16 len)
{
    printf("JL_OPCODE_ADV_DEVICE_REQUEST\n");
    return JL_CMD_send(JL_OPCODE_ADV_DEVICE_REQUEST, buf, len, JL_NEED_RESPOND);
}

struct t_s_info _s_info = {
    .key_setting = {
        0x01, 0x01, 0x05, \
        0x02, 0x01, 0x05, \
        0x01, 0x02, 0x08, \
        0x02, 0x02, 0x08
    },
    .key_extra_setting = {
        0
    },
    .led_status = {
        0x01, 0x06, \
        0x02, 0x05, \
        0x03, 0x04
    },
    .mic_mode = 1,
    .work_mode = 1,
    .eq_mode_info = {0x00},
};

extern const char *bt_get_local_name();
extern void bt_adv_get_bat(u8 *buf);
static u32 JL_opcode_get_adv_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 buf[256];
    u8 offset = 0;

    u32 ret = 0;
    u32 mask = READ_BIG_U32(data);
    rcsp_printf("FEATURE MASK : %x\n", mask);
    /* #define ATTR_TYPE_BAT_VALUE  	(0) */
    /* #define ATTR_TYPE_EDR_NAME   	(1) */
    //get version
    if (mask & BIT(ATTR_TYPE_BAT_VALUE)) {
        rcsp_printf("ATTR_TYPE_BAT_VALUE\n");
        u8 bat[3];
        bt_adv_get_bat(bat);
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_BAT_VALUE, bat, 3);
    }
#if RCSP_ADV_NAME_SET_ENABLE
    if (mask & BIT(ATTR_TYPE_EDR_NAME)) {
        rcsp_printf("ATTR_TYPE_EDR_NAME\n");
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_EDR_NAME, (void *)_s_info.edr_name, strlen(_s_info.edr_name));
    }
#endif

#if RCSP_ADV_KEY_SET_ENABLE
    if (mask & BIT(ATTR_TYPE_KEY_SETTING)) {
        rcsp_printf("ATTR_TYPE_KEY_SETTING\n");
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_KEY_SETTING, (void *)_s_info.key_setting, sizeof(_s_info.key_setting));
    }

    if (mask & BIT(ATTR_TYPE_ANC_VOICE_KEY)) {
        rcsp_printf("ATTR_TYPE_ANC_VOICE_KEY\n");
        u8 anc_voice_key_mode[4] = {0};
        get_anc_voice_key_mode(anc_voice_key_mode);
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_ANC_VOICE_KEY, anc_voice_key_mode, sizeof(anc_voice_key_mode));
    }
#endif

#if RCSP_ADV_LED_SET_ENABLE
    if (mask & BIT(ATTR_TYPE_LED_SETTING)) {
        rcsp_printf("ATTR_TYPE_LED_SETTING\n");
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_LED_SETTING, (void *)_s_info.led_status, sizeof(_s_info.led_status));
    }
#endif


#if RCSP_ADV_MIC_SET_ENABLE
    if (mask & BIT(ATTR_TYPE_MIC_SETTING)) {
        rcsp_printf("ATTR_TYPE_MIC_SETTING\n");
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_MIC_SETTING, (void *)&_s_info.mic_mode, 1);
    }
#endif

#if RCSP_ADV_WORK_SET_ENABLE
    if (mask & BIT(ATTR_TYPE_WORK_MODE)) {
        rcsp_printf("ATTR_TYPE_WORK_MODE\n");
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_WORK_MODE, (void *)&_s_info.work_mode, 1);
    }
#endif

#if RCSP_ADV_PRODUCT_MSG_ENABLE
    if (mask & BIT(ATTR_TYPE_PRODUCT_MESSAGE)) {
        rcsp_printf("ATTR_TYPE_PRODUCT_MESSAGE\n");
        u16 vid = get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);
        u16 pid = get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG);
        u8 tversion[6];
        tversion[0] = 0x05;
        tversion[1] = 0xD6;
        tversion[3] = vid & 0xFF;
        tversion[2] = vid >> 8;
        tversion[5] = pid & 0xFF;
        tversion[4] = pid >> 8;
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_PRODUCT_MESSAGE, (void *)tversion, 6);

    }
#endif
    rcsp_printf_buf(buf, offset);

    ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, buf, offset);

    return ret;
}

// ----------------------------reboot----------------------//
/* extern void tws_sync_modify_bt_name_reset(void); */
/* void modify_bt_name_and_reset(u32 msec) */
/* { */
/* 	sys_timer_add(NULL, cpu_reset, msec); */
/* } */
// ----------------------------end----------------------//

static u8 adv_setting_result = 0;
static u8 adv_set_deal_one_attr(u8 *buf, u8 size, u8 offset)
{
    u8 rlen = buf[offset];
    if ((offset + rlen + 1) > (size - offset)) {
        rcsp_printf("\n\ndeal attr end!\n\n");
        return rlen;
    }
    u8 type = buf[offset + 1];
    u8 *pbuf = &buf[offset + 2];
    u8 dlen = rlen - 1;
    u8 bt_name[32];
    adv_setting_result = 0;

    switch (type) {
#if RCSP_ADV_NAME_SET_ENABLE
    case ATTR_TYPE_EDR_NAME:
        if (dlen > 20) {
            adv_setting_result = -1;
        } else {
            memcpy(bt_name, pbuf, dlen);
            bt_name[dlen] = '\0';
            memcpy(_s_info.edr_name, bt_name, 32);
            rcsp_printf("ATTR_TYPE_EDR_NAME %s\n", bt_name);
            rcsp_printf_buf(pbuf, dlen);
            deal_bt_name_setting(NULL, 1, 1);
        }
        break;
#endif
#if RCSP_ADV_KEY_SET_ENABLE
    case ATTR_TYPE_KEY_SETTING:
        rcsp_printf("ATTR_TYPE_KEY_SETTING\n");
        rcsp_printf_buf(pbuf, dlen);
        while (dlen >= 3) {
            if (pbuf[0] == 0x01) {
                if (pbuf[1] == 0x01) {
                    _s_info.key_setting[2] = pbuf[2];
                } else if (pbuf[1] == 0x02) {
                    _s_info.key_setting[8] = pbuf[2];
                }
            } else if (pbuf[0] == 0x02) {
                if (pbuf[1] == 0x01) {
                    _s_info.key_setting[5] = pbuf[2];
                } else if (pbuf[1] == 0x02) {
                    _s_info.key_setting[11] = pbuf[2];
                }
            }
            dlen -= 3;
            pbuf += 3;
        }
        deal_key_setting(NULL, 1, 1);
        break;
    case ATTR_TYPE_ANC_VOICE_KEY:
        rcsp_printf("ATTR_TYPE_ANC_VOICE_KEY\n");
        rcsp_printf_buf(pbuf, dlen);
        deal_anc_voice_key_setting(pbuf, 1, 1);
        break;
#endif
#if RCSP_ADV_LED_SET_ENABLE
    case ATTR_TYPE_LED_SETTING:
        rcsp_printf("ATTR_TYPE_LED_SETTING\n");
        rcsp_printf_buf(pbuf, dlen);
        while (dlen >= 2) {
            if (pbuf[0] == 0 || pbuf[0] > 3) {
                break;
            } else {
                _s_info.led_status[2 * (pbuf[0] - 1) + 1] = pbuf[1];
            }
            dlen -= 2;
            pbuf += 2;
        }
        deal_led_setting(NULL, 1, 1);
        break;
#endif
#if RCSP_ADV_MIC_SET_ENABLE
    case ATTR_TYPE_MIC_SETTING:
        rcsp_printf("ATTR_TYPE_MIC_SETTING\n");
        rcsp_printf_buf(pbuf, dlen);
        if (2 == _s_info.work_mode) {
            adv_setting_result = -1;
        } else {
            deal_mic_setting(pbuf[0], 1, 1);
        }
        break;
#endif
#if RCSP_ADV_WORK_SET_ENABLE
    case ATTR_TYPE_WORK_MODE:
        rcsp_printf("ATTR_TYPE_WORK_MODE\n");
        rcsp_printf_buf(pbuf, dlen);
        deal_work_setting(pbuf[0], 1, 1, 0);
        break;
#endif
    case ATTR_TYPE_TIME_STAMP:
        rcsp_printf("ATTR_TYPE_TIME_STAMP\n");
        rcsp_printf_buf(pbuf, dlen);
        //adv_time_stamp = (((pbuf[0] << 8) | pbuf[1]) << 8 | pbuf[2]) << 8 | pbuf[3];
        syscfg_read(CFG_BT_NAME, _s_info.edr_name, sizeof(_s_info.edr_name));
        //deal_time_stamp_setting();
        //set_adv_time_stamp((((pbuf[0] << 8) | pbuf[1]) << 8 | pbuf[2]) << 8 | pbuf[3]);
        deal_time_stamp_setting((((pbuf[0] << 8) | pbuf[1]) << 8 | pbuf[2]) << 8 | pbuf[3], 1, 1);
        break;
    default:
        rcsp_printf("ATTR UNKNOW\n");
        rcsp_printf_buf(pbuf, dlen);
        break;
    }

    return rlen + 1;
}

static u32 JL_opcode_set_adv_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    rcsp_printf("JL_opcode_set_adv_info:\n");
    rcsp_printf_buf(data, len);
    u8 offset = 0;
    while (offset < len) {
        offset += adv_set_deal_one_attr(data, len, offset);
    }
    u8 ret = 0;
    if (adv_setting_result) {
        /* JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, &ret, 1); */
        ret = 1;
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &ret, 1);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &ret, 1);
    }
    return 0;
}

extern const int support_dual_bank_update_en;
static u32 JL_opcode_get_target_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 buf[256];
    u8 offset = 0;

    __this->phone_platform = data[4];
    if (__this->phone_platform == ANDROID) {
        rcsp_printf("phone_platform == ANDROID\n");
    } else if (__this->phone_platform == APPLE_IOS) {
        rcsp_printf("phone_platform == APPLE_IOS\n");
    } else {
        rcsp_printf("phone_platform ERR\n");
    }

    u32 mask = READ_BIG_U32(data);
    rcsp_printf("FEATURE MASK : %x\n", mask);

    u32 ret = 0;

    //get version
    if (mask & BIT(ATTR_TYPE_PROTOCOL_VERSION)) {
        rcsp_printf("ATTR_TYPE_PROTOCOL_VERSION\n");
        u8 ver = get_rcsp_version();
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_PROTOCOL_VERSION, &ver, 1);
    }

    //get powerup sys info
    if (mask & BIT(ATTR_TYPE_SYS_INFO)) {
        rcsp_printf("ATTR_TYPE_SYS_INFO\n");

        struct _SYS_info sys_info;
        //extern u16 get_battery_level(void);
        sys_info.bat_lev = 0; //get_battery_level() / 10;
        sys_info.sys_vol = 0; //sound.vol.sys_vol_l;
        sys_info.max_vol = 0; //MAX_SYS_VOL_L;

        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_SYS_INFO, \
                               (u8 *)&sys_info, sizeof(sys_info));
    }

    //get EDR info
    if (mask & BIT(ATTR_TYPE_EDR_ADDR)) {
        rcsp_printf("ATTR_TYPE_EDR_ADDR\n");
        struct _EDR_info edr_info;
        /* extern void hook_get_mac_addr(u8 * btaddr); */
        /* hook_get_mac_addr(edr_info.addr_buf); */

        extern const u8 *bt_get_mac_addr();
        u8 taddr_buf[6];
        memcpy(taddr_buf, bt_get_mac_addr(), 6);
        edr_info.addr_buf[0] =  taddr_buf[5];
        edr_info.addr_buf[1] =  taddr_buf[4];
        edr_info.addr_buf[2] =  taddr_buf[3];
        edr_info.addr_buf[3] =  taddr_buf[2];
        edr_info.addr_buf[4] =  taddr_buf[1];
        edr_info.addr_buf[5] =  taddr_buf[0];
        /* extern u8 get_edr_suppor_profile(void); */
        /* edr_info.profile = get_edr_suppor_profile(); */
        edr_info.profile = 0x0E;
#if (RCSP_CHANNEL_SEL == RCSP_USE_BLE)
        edr_info.profile &= ~BIT(7);
#else
        edr_info.profile |= BIT(7);
#endif
        extern u8 get_bt_connect_status(void);
        if (get_bt_connect_status() ==  BT_STATUS_WAITINT_CONN) {
            edr_info.state = 0;
        } else {
            edr_info.state = 1;
        }
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_EDR_ADDR, (u8 *)&edr_info, sizeof(struct _EDR_info));
    }

    //get platform info
    if (mask & BIT(ATTR_TYPE_PLATFORM)) {
        rcsp_printf("ATTR_TYPE_PLATFORM\n");

        u8 lic_val = 0;
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_PLATFORM, &lic_val, 1);
    }

    //get function info
    if (mask & BIT(ATTR_TYPE_FUNCTION_INFO)) {
        rcsp_printf("ATTR_TYPE_FUNCTION_INFO\n");

        u8 tmp_buf[6] = {0};
        u8 function_info = 0x16;
        u32 func_mask = 1;
        func_mask = app_htonl(func_mask);
        memcpy(tmp_buf, &func_mask, 4);
        memcpy(tmp_buf + 4, &function_info, 1);

        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_FUNCTION_INFO, tmp_buf, sizeof(tmp_buf));
    }

    //get dev info
    if (mask & BIT(ATTR_TYPE_DEV_VERSION)) {
        rcsp_printf("ATTR_TYPE_DEV_VERSION\n");
        u16 ver = 0;
        ver =  get_vid_pid_ver_from_cfg_file(GET_VER_FROM_EX_CFG);
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_DEV_VERSION, &ver, 2);
    }

    if (mask & BIT(ATTR_TYPE_UBOOT_VERSION)) {
        rcsp_printf("ATTR_TYPE_UBOOT_VERSION\n");
        u8 *uboot_ver_flag = (u8 *)(0x1C000 - 0x8);
        u8 uboot_version[2] = {uboot_ver_flag[0], uboot_ver_flag[1]};
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_UBOOT_VERSION, uboot_version, sizeof(uboot_version));
    }

    if (mask & BIT(ATTR_TYPE_DOUBLE_PARITION)) {
        rcsp_printf("ATTR_TYPE_DOUBLE_PARITION:%x\n", support_dual_bank_update_en);
        u8 double_partition_value;
        u8 ota_loader_need_download_flag;
        u8 update_channel_sel = 0;
        if (support_dual_bank_update_en) {
            double_partition_value = 0x1;
            ota_loader_need_download_flag = 0x00;
        } else {
            double_partition_value = 0x0;
            ota_loader_need_download_flag = 0x01;
            //update_channel_sel |= (BIT(7) |  0x0);      //最低位为升级通道选择、0选择BLE升级、1选择SPP升级
        }
        update_channel_sel = 0x1;
        u8 update_param[3] = {
            double_partition_value,
            ota_loader_need_download_flag,
            update_channel_sel,
        };
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_DOUBLE_PARITION, (u8 *)update_param, sizeof(update_param));
    }

    if (mask & BIT(ATTR_TYPE_UPDATE_STATUS)) {
        rcsp_printf("ATTR_TYPE_UPDATE_STATUS\n");
        u8 update_status_value[2] = {0};
        /* syscfg_read(VM_UPDATE_FLAG, &update_status_value[1], 1);     //读出vm强制升级标志 */
        /* g_printf("get taget info:%d\n", update_status_value[1]); */
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_UPDATE_STATUS, (u8 *)&update_status_value, sizeof(update_status_value));
    }

    if (mask & BIT(ATTR_TYPE_DEV_VID_PID)) {
        rcsp_printf("ATTR_TYPE_DEV_VID_PID\n");
        u16 pvid[2] = {0};
        pvid[0] =  get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);
        pvid[1] =  get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG);
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_DEV_VID_PID, (u8 *)pvid, sizeof(pvid));
    }

    if (mask & BIT(ATTR_TYPE_SDK_TYPE)) {
        rcsp_printf("ATTR_TYPE_SDK_TYPE\n");
        u8 sdk_type = RCSP_SDK_TYPE;
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_SDK_TYPE, &sdk_type, 1);
    }

#if VER_INFO_EXT_COUNT
    // get AuthKey
    if (mask & BIT(ATTR_TYPE_DEV_AUTHKEY)) {
        rcsp_printf("ATTR_TYPE_DEV_AUTHKEY\n");
        u8 authkey_len = 0;
        u8 *local_authkey_data = NULL;
        get_authkey_procode_from_cfg_file(&local_authkey_data, &authkey_len, GET_AUTH_KEY_FROM_EX_CFG);
        if (local_authkey_data && authkey_len) {
            put_buf(local_authkey_data, authkey_len);
            offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_DEV_AUTHKEY, local_authkey_data, authkey_len);
        }
    }

    if (mask & BIT(ATTR_TYPE_DEV_PROCODE)) {
        rcsp_printf("ATTR_TYPE_DEV_PROCODE\n");
        u8 procode_len = 0;
        u8 *local_procode_data = NULL;
        get_authkey_procode_from_cfg_file(&local_procode_data, &procode_len, GET_PRO_CODE_FROM_EX_CFG);
        if (local_procode_data && procode_len) {
            put_buf(local_procode_data, procode_len);
            offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_DEV_PROCODE, local_procode_data, procode_len);
        }
    }
#endif

    if (mask & BIT(ATTR_TYPE_DEV_MAX_MTU)) {
        rcsp_printf(" ATTR_TYPE_DEV_MTU_SIZE\n");
        /* u16 JL_packet_get_rx_mtu(void); */
        /* u16 JL_packet_get_rx_max_mtu(void); */
        /* u16 JL_packet_get_tx_max_mtu(void); */
        u16 rx_max_mtu = JL_packet_get_rx_max_mtu();
        u16 tx_max_mtu = JL_packet_get_tx_max_mtu();
        u8 t_buf[4];
        t_buf[0] = (tx_max_mtu >> 8) & 0xFF;
        t_buf[1] = tx_max_mtu & 0xFF;
        t_buf[2] = (rx_max_mtu >> 8) & 0xFF;
        t_buf[3] = rx_max_mtu & 0xFF;
        offset += add_one_attr(buf, sizeof(buf), offset,  ATTR_TYPE_DEV_MAX_MTU, t_buf, 4);
    }

    if (mask & BIT(ATTR_TEYP_BLE_ADDR)) {
        rcsp_printf(" ATTR_TEYP_BLE_ADDR\n");
        extern void lib_make_ble_address(u8 * ble_address, u8 * edr_address);
        extern const u8 *bt_get_mac_addr();
        u8 taddr_buf[7];
        taddr_buf[0] = 0;
        lib_make_ble_address(taddr_buf + 1, (void *)bt_get_mac_addr());
        for (u8 i = 0; i < (6 / 2); i++) {
            taddr_buf[i + 1] ^= taddr_buf[7 - i - 1];
            taddr_buf[7 - i - 1] ^= taddr_buf[i + 1];
            taddr_buf[i + 1] ^= taddr_buf[7 - i - 1];
        }
        offset += add_one_attr(buf, sizeof(buf), offset,  ATTR_TEYP_BLE_ADDR, taddr_buf, sizeof(taddr_buf));
    }

    if (mask & BIT(ATTR_TYPE_MD5_GAME_SUPPORT)) {
        rcsp_printf("ATTR_TYPE_MD5_GAME_SUPPORT\n");
        u8 ext_function_flag[2] = {0};
        u8 md5_support = UPDATE_MD5_ENABLE;

        /* if(get_work_setting() == 2) */
        {
            md5_support |= BIT(1);
        }

#if (RCSP_ADV_FIND_DEVICE_ENABLE)
        md5_support |= BIT(2);
#endif
#if (RCSP_ADV_ANC_VOICE)
        md5_support |= BIT(6);
#endif
        ext_function_flag[0] = md5_support;
        u8 ext_function_flag_byte1 = 0;
#if (RCSP_ADV_ASSISTED_HEARING)
        ext_function_flag_byte1 |= BIT(0);
#endif
#if RCSP_ADV_ADAPTIVE_NOISE_REDUCTION
        ext_function_flag_byte1 |= BIT(1);
#endif
#if RCSP_ADV_AI_NO_PICK
        ext_function_flag_byte1 |= BIT(3);
#endif
#if RCSP_ADV_SCENE_NOISE_REDUCTION
        ext_function_flag_byte1 |= BIT(4);
#endif
#if RCSP_ADV_WIND_NOISE_DETECTION
        ext_function_flag_byte1 |= BIT(5);
#endif
#if RCSP_ADV_VOICE_ENHANCEMENT_MODE
        ext_function_flag_byte1 |= BIT(6);
#endif
        ext_function_flag[1] = ext_function_flag_byte1;
        offset += add_one_attr(buf, sizeof(buf), offset, ATTR_TYPE_MD5_GAME_SUPPORT, ext_function_flag, 2);
    }
    rcsp_printf_buf(buf, offset);

    ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, buf, offset);

    return ret;
}

static u16 common_function_info(u32 mask, u8 *buf, u16 len)
{
    u16 offset = 0;

#if RCSP_ADV_EQ_SET_ENABLE
    if (mask & BIT(COMMON_INFO_ATTR_EQ)) {
        u8 eq_info[11] = {0};
        u8 eq_info_size = app_get_eq_info(eq_info);
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_EQ, eq_info, eq_info_size);
    }
    if (mask & BIT(COMMON_INFO_ATTR_PRE_FETCH_ALL_EQ_INFO)) {
        u8 eq_pre_fetch_info[1  +  20  + (1 + 10) * 10] = {0}; // num + freq + all_gain_of_eq [max]
        u8 eq_per_fetch_size = app_get_eq_all_info(eq_pre_fetch_info);
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_PRE_FETCH_ALL_EQ_INFO, eq_pre_fetch_info, eq_per_fetch_size);
    }
#endif
    if (mask & BIT(COMMON_INFO_ATTR_PHONE_SCO_STATE_INFO)) {
        u8 phone_state = 0;
        if (BT_CALL_HANGUP != get_call_status()) {
            phone_state = 1;
        }
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_PHONE_SCO_STATE_INFO, &phone_state, sizeof(phone_state));
    }

#if RCSP_ADV_HIGH_LOW_SET
    if (mask & BIT(COMMON_INFO_ATTR_HIGH_LOW_SET)) {
        struct _HIGL_LOW_VOL gain_param = {0};
        get_high_low_vol_info(&gain_param);
        gain_param.low_vol = gain_param.low_vol < 0  ? 0  : gain_param.low_vol;
        gain_param.low_vol = gain_param.low_vol > 24 ? 24 : gain_param.low_vol;
        gain_param.high_vol = gain_param.high_vol < 0  ? 0  : gain_param.high_vol;
        gain_param.high_vol = gain_param.high_vol > 24 ? 24 : gain_param.high_vol;
        gain_param.low_vol  -= 12;
        gain_param.high_vol -= 12;
        gain_param.low_vol  = READ_BIG_U32((u8 *)&gain_param.low_vol);
        gain_param.high_vol = READ_BIG_U32((u8 *)&gain_param.high_vol);
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_HIGH_LOW_SET, (u8 *)(&gain_param), sizeof(gain_param));
    }
#endif

#if (RCSP_ADV_ANC_VOICE)
    if (mask & BIT(COMMON_INFO_ATTR_ANC_VOICE)) {
        u8 anc_info[9] = {0};
        anc_voice_info_get(anc_info, sizeof(anc_info));
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_ANC_VOICE, anc_info, sizeof(anc_info));
    }

    if (mask & BIT(COMMON_INFO_ATTR_FETCH_ALL_ANC_VOICE)) {
        u8 anc_fetch_all_info[28] = {0};
        anc_voice_info_fetch_all_get(anc_fetch_all_info, sizeof(anc_fetch_all_info));
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_FETCH_ALL_ANC_VOICE, anc_fetch_all_info, sizeof(anc_fetch_all_info));
    }
#endif

// 添加辅听的get函数
#if (RCSP_ADV_ASSISTED_HEARING)
    if (mask & BIT(COMMON_INFO_ATTR_ASSISTED_HEARING)) {
        u8 dha_fitting_data[3 + 2 + 2 * DHA_FITTING_CHANNEL_MAX] = {0};
        get_dha_fitting_info(dha_fitting_data);
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_ASSISTED_HEARING, dha_fitting_data, sizeof(dha_fitting_data));
    }
#endif

#if RCSP_ADV_ADAPTIVE_NOISE_REDUCTION
    if (mask & BIT(COMMON_INFO_ATTR_ADAPTIVE_NOISE_REDUCTION)) {
        u8 adaptive_noise_reduction_info[3] = {0};
        // 获取自适应降噪开关
        if (get_adaptive_noise_reduction_switch() != 0) {
            // 开启
            adaptive_noise_reduction_info[0] = 0x01;
        }
        // 获取设置自适应降噪状态
        if (get_adaptive_noise_reduction_reset_status() != 0) {
            // 设置中
            adaptive_noise_reduction_info[1] = 0x01;
        }
        // 获取设置自适应降噪结果
        if (get_adaptive_noise_reduction_reset_result() != 0) {
            // 设置中
            adaptive_noise_reduction_info[2] = 0x01;
        }
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_ADAPTIVE_NOISE_REDUCTION, adaptive_noise_reduction_info, sizeof(adaptive_noise_reduction_info));
    }
#endif

#if RCSP_ADV_AI_NO_PICK
    if (mask & BIT(COMMON_INFO_ATTR_AI_NO_PICK)) {
        u8 ai_no_pick_info[5] = {0};
        ai_no_pick_info[1] = 0xff;	// 获取智能免摘全部数据
        ai_no_pick_info[2] = get_ai_no_pick_switch() ? 0x01 : 0x00;
        ai_no_pick_info[3] = get_ai_no_pick_sensitivity();
        ai_no_pick_info[4] = get_ai_no_pick_auto_close_time();
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_AI_NO_PICK, ai_no_pick_info, sizeof(ai_no_pick_info));
    }
#endif

#if RCSP_ADV_SCENE_NOISE_REDUCTION
    if (mask & BIT(COMMON_INFO_ATTR_SCENE_NOISE_REDUCTION)) {
        u8 scene_noise_reduction_info[2] = {0};
        scene_noise_reduction_info[1] = get_scene_noise_reduction_type();
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_SCENE_NOISE_REDUCTION, scene_noise_reduction_info, sizeof(scene_noise_reduction_info));
    }
#endif

#if RCSP_ADV_WIND_NOISE_DETECTION
    if (mask & BIT(COMMON_INFO_ATTR_WIND_NOISE_DETECTION)) {
        u8 wind_noise_detection_info[2] = {0};
        wind_noise_detection_info[1] = get_wind_noise_detection_switch() ? 0x01 : 0x00;
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_WIND_NOISE_DETECTION, wind_noise_detection_info, sizeof(wind_noise_detection_info));
    }
#endif

#if RCSP_ADV_VOICE_ENHANCEMENT_MODE
    if (mask & BIT(COMMON_INFO_ATTR_VOICE_ENHANCEMENT_MODE)) {
        u8 voice_enhancement_mode_info[2] = {0};
        voice_enhancement_mode_info[1] = get_voice_enhancement_mode_switch() ? 0x01 : 0x00;
        offset += add_one_attr(buf, len, offset, COMMON_INFO_ATTR_VOICE_ENHANCEMENT_MODE, voice_enhancement_mode_info, sizeof(voice_enhancement_mode_info));
    }
#endif

    return offset;
}

static u16 bt_function_info(u32 mask, u8 *buf, u16 len)
{
    u16 offset = 0;
    u16 music_sec = 0;
    u32 curr_music_sec = 0;
    u8 music_state = 0;
    u8 player_time[4];



#if RCSP_ADV_MUSIC_INFO_ENABLE
    if (mask & BIT(BT_INFO_ATTR_MUSIC_TITLE)) {
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_TITLE, get_music_title(), get_music_title_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_ARTIST)) {
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_ARTIST, get_music_artist(), get_music_artist_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_ALBUM)) {
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_ALBUM, get_music_album(), get_music_album_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_NUMBER)) {
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_NUMBER, get_music_number(), get_music_number_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_TOTAL)) {
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_TOTAL, get_music_total(),  get_music_total_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_GENRE)) {
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_GENRE, get_music_genre(), get_music_genre_len());
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_TIME)) {
        music_sec = get_music_total_time();
        player_time[0] = music_sec >> 8;
        player_time[1] = music_sec;
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_TIME, player_time, 2);
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_STATE)) {
        //printf("\n music state\n");
        music_state = get_music_player_state();
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_STATE, &music_state, 1);
    }
    if (mask & BIT(BT_INFO_ATTR_MUSIC_CURR_TIME)) {
        //printf("\nmusic curr time\n");
        curr_music_sec = get_music_curr_time();
        player_time[0] = curr_music_sec >> 24;
        player_time[1] = curr_music_sec >> 16;
        player_time[2] = curr_music_sec >> 8;
        player_time[3] = curr_music_sec;
        offset += add_one_attr(buf, len, offset, BT_INFO_ATTR_MUSIC_CURR_TIME, player_time, 4);
    }
#endif
    return offset;
}

static u32 JL_opcode_get_sys_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 buf[512];
    u32 ret = 0;
    u8 offset = 0;
    u8 fun_type = data[0];
    u32 mask = READ_BIG_U32(data + 1);
    rcsp_printf("SYS MASK : %x\n", mask);

    /* printf_buf(data, len); */

    buf[0] = fun_type;

    u8 *p = buf + 1;
    u16 size = sizeof(buf) - 1;

    switch (fun_type) {
    case COMMON_FUNCTION:
        rcsp_printf("COMMON_FUNCTION\n");
        offset = common_function_info(mask, p, size);
        break;

    case BT_FUNCTION:
        rcsp_printf("BT_FUNCTION\n");
        offset = bt_function_info(mask, p, size);
        break;
    default:
        break;

    }

    rcsp_printf_buf(buf, offset + 1);

    ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, buf, offset + 1);

    return ret;
}

static void set_common_function_info(u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 offset = 0;
    //analysis sys info
    while (offset < len) {
        u8 len_tmp = data[offset];
        u8 type = data[offset + 1];
        rcsp_printf("common info:\n");
        rcsp_printf_buf(&data[offset], len_tmp + 1);

        switch (type) {
#if RCSP_ADV_EQ_SET_ENABLE
        case COMMON_INFO_ATTR_EQ :
            r_printf("COMMON_INFO_ATTR_EQ\n");

            deal_eq_setting(&data[offset + 2], 1, 1);
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_EQ, NULL, 0);

            break;
#endif

#if RCSP_ADV_HIGH_LOW_SET
        case COMMON_INFO_ATTR_HIGH_LOW_SET:
            r_printf("COMMON_INFO_ATTR_HIGH_LOW_SET");
            struct _HIGL_LOW_VOL gain_param = {0};
            gain_param.low_vol = READ_BIG_U32(data + 2) + 12;
            gain_param.high_vol = READ_BIG_U32(data + 2 + 4) + 12;
            gain_param.low_vol = gain_param.low_vol   > 24 ? 24 : gain_param.low_vol;
            gain_param.low_vol = gain_param.low_vol   < 0  ? 0 : gain_param.low_vol;
            gain_param.high_vol = gain_param.high_vol > 24 ? 24 : gain_param.high_vol;
            gain_param.high_vol = gain_param.high_vol < 0  ? 0 : gain_param.high_vol;
            deal_high_low_vol((u8 *)&gain_param, 1, 1);
            break;
#endif

#if RCSP_ADV_ANC_VOICE
        case COMMON_INFO_ATTR_ANC_VOICE:
            printf("SET COMMON_INFO_ATTR_ANC_VOICE\n");
            anc_voice_info_set(data + 2, len_tmp - 1);
            break;
#endif

// 添加辅听的set函数
#if (RCSP_ADV_ASSISTED_HEARING)
        case COMMON_INFO_ATTR_ASSISTED_HEARING:
            printf("SET COMMON_INFO_ATTR_ASSISTED_HEARING");
            deal_hearing_aid_setting(OpCode, OpCode_SN, data + 2, 0, 1);
            break;
#endif

#if RCSP_ADV_ADAPTIVE_NOISE_REDUCTION
        case COMMON_INFO_ATTR_ADAPTIVE_NOISE_REDUCTION:
            printf("SET COMMON_INFO_ATTR_ADAPTIVE_NOISE_REDUCTION\n");
            u8 *switch_adaptive_noise_reduction = data + 2;
            if (*switch_adaptive_noise_reduction) {
                u8 *is_adaptive_noise_reduction_check = data + 3;
                if (*is_adaptive_noise_reduction_check) {
                    // 重新检测
                    set_adaptive_noise_reduction_reset();
                } else {
                    // 开启自适应
                    set_adaptive_noise_reduction_on();
                }
            } else {
                // 关闭自适应
                set_adaptive_noise_reduction_off();
            }
            break;
#endif

#if RCSP_ADV_AI_NO_PICK
        case COMMON_INFO_ATTR_AI_NO_PICK:
            printf("SET COMMON_INFO_ATTR_AI_NO_PICK\n");
            u8 *ai_no_pick_func = data + 3;
            u8 *ai_no_pick_val = data + 4;
            if ((*ai_no_pick_func) == 0x00) {				// 开关智能免摘
                set_ai_no_pick_switch((*ai_no_pick_val) ? true : false);
            } else if ((*ai_no_pick_func) == 0x01) {		// 设置智能免摘敏感度
                set_ai_no_pick_sensitivity((*ai_no_pick_val));
            } else if ((*ai_no_pick_func) == 0x02) {		// 设置智能免摘启动后自动关闭时间
                set_ai_no_pick_auto_close_time((*ai_no_pick_val));
            }
            break;
#endif

#if RCSP_ADV_SCENE_NOISE_REDUCTION
        case COMMON_INFO_ATTR_SCENE_NOISE_REDUCTION:
            printf("SET COMMON_INFO_ATTR_SCENE_NOISE_REDUCTION\n");
            u8 *scene_noise_reduction_type = data + 3;
            set_scene_noise_reduction_type((RCSP_SCENE_NOISE_REDUCTION_TYPE)(*scene_noise_reduction_type));
            break;
#endif

#if RCSP_ADV_WIND_NOISE_DETECTION
        case COMMON_INFO_ATTR_WIND_NOISE_DETECTION:
            printf("SET COMMON_INFO_ATTR_WIND_NOISE_DETECTION\n");
            u8 *wind_noise_detection_switch = data + 3;
            set_wind_noise_detection_switch((*wind_noise_detection_switch) ? true : false);
            break;
#endif

#if RCSP_ADV_VOICE_ENHANCEMENT_MODE
        case COMMON_INFO_ATTR_VOICE_ENHANCEMENT_MODE:
            printf("SET COMMON_INFO_ATTR_VOICE_ENHANCEMENT_MODE\n");
            u8 *voice_enhancement_mode_switch = data + 3;
            set_voice_enhancement_mode_switch((*voice_enhancement_mode_switch) ? true : false);
            break;
#endif

        default:
            break;
        }

        offset += len_tmp + 1;
    }
}

#if (RCSP_ADV_ASSISTED_HEARING)
static u8 _hearing_aid_operating_flag = 0;
void set_hearing_aid_operating_flag()
{
    _hearing_aid_operating_flag = 1;
}
#endif

static u32 JL_opcode_set_sys_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u32 ret = 0;
    u8 fun_type = data[0];
    u8 *p = data + 1;

    len = len - 1;

    switch (fun_type) {
    case COMMON_FUNCTION:
        rcsp_printf("COMMON_FUNCTION\n");
        set_common_function_info(OpCode, OpCode_SN, p, len);
        break;
    default:
        break;
    }

#if (RCSP_ADV_ASSISTED_HEARING)
    if (_hearing_aid_operating_flag) {
        _hearing_aid_operating_flag = 0;
        return JL_ERR_NONE;
    }
#endif

    if (ret) {
        ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
    }

    return ret;
}


u32 bt_function_cmd_set(u8 *p, u16 len)
{
#if RCSP_ADV_MUSIC_INFO_ENABLE
    music_info_cmd_handle(p, len);
#endif
    return true;
}

void wait_reboot_dev(void *priv)
{
    cpu_reset();
}

void rcsp_reboot_dev(void)
{
    user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);

    extern void ble_module_enable(u8 en);
#if RCSP_ADV_NAME_SET_ENABLE
    extern void adv_edr_name_change_now(void);
    adv_edr_name_change_now();
#endif
    ble_module_enable(0);
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        //tws_sync_modify_bt_name_reset();
        modify_bt_name_and_reset(500);
    } else {
        if (sys_timer_add(NULL, wait_reboot_dev, 500) == 0) {
            cpu_reset();
        }
    }
#else
    if (sys_timer_add(NULL, wait_reboot_dev, 500) == 0) {
        cpu_reset();
    }
#endif

}

static void function_cmd_handle(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u32 ret = 0;
    u8 fun_type = data[0];
    u8 *p = data + 1;

    len = len - 1;

    switch (fun_type) {
    case BT_FUNCTION:
        rcsp_printf("BT_FUNCTION\n");
        ret = bt_function_cmd_set(p, len);
        break;
    default:
        break;
    }

    if (ret == true) {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
    } else {
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    }
}

#if RCSP_ADV_FIND_DEVICE_ENABLE

static u8 find_device_key_flag = 0;
static u16 find_device_timer = 0;

u8 find_device_key_flag_get(void)
{
    return find_device_key_flag;
}

void send_find_device_stop(void)
{
    if (0 == JL_rcsp_get_auth_flag()) {
        return;
    }
    //				 查找手机	关闭铃声	超时时间(不限制)
    u8 send_buf[4] = {0x00, 	0x00, 		0x00, 0x00};
    JL_CMD_send(JL_OPCODE_SYS_FIND_DEVICE, send_buf, sizeof(send_buf), JL_NOT_NEED_RESPOND);
}

void find_device_reset(void)
{
    find_device_key_flag = 0;
    if (find_device_timer) {
        sys_timeout_del(find_device_timer);
        find_device_timer = 0;
    }
}

static void find_device_stop(void *priv)
{
    find_device_reset();
    send_find_device_stop();

    // 0 - way, 1 - player, 2 - find_device_key_flag
    u8 other_opt[3] = {0};
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
        other_opt[2] = 2;
        extern void find_device_sync(u8 * param, u32 msec);
        find_device_sync(other_opt, 300);
        return;
    }
#endif
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_RESUME, other_opt, sizeof(other_opt));
}

static void find_device_timeout_handle(u32 sec)
{
    find_device_key_flag = 1;
    if (sec && (0 == find_device_timer)) {
        find_device_timer = sys_timeout_add(NULL, find_device_stop, sec * 1000);
    }
}

// channel : L - 左耳，R - 右耳
// state : 0 - 静音，1 - 有声音
static void earphone_mute(u8 channel, u8 state)
{
    if ('L' == channel) {
        channel = 1;
    } else if ('R' == channel) {
        channel = 2;
    } else {
        return;
    }
    extern void audio_dac_ch_mute(struct audio_dac_hdl * dac, u8 ch, u8 mute);
    audio_dac_ch_mute(NULL, channel, !state);
}

static void earphone_mute_timer_func(void *param)
{
    u8 way = ((u8 *)param)[0];
    u8 player = ((u8 *)param)[1];
    earphone_mute('L', 1);
    earphone_mute('R', 1);

#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {

        switch (way) {
        case 1:
            if ('R' == tws_api_get_local_channel()) {
                earphone_mute('R', 0);
            }
            break;
        case 2:
            if ('L' == tws_api_get_local_channel()) {
                earphone_mute('L', 0);
            }
            break;
        }
    }
#endif
    switch (player) {
    case 0:
        // app端播放/还原
        tone_play_stop();
        break;
    case 1:
        // 设备端播放
        tone_play(TONE_NORMAL, 1);
        earphone_mute_handler(param, 300);
        break;
    }
}

static void earphone_mute_handler(u8 *other_opt, u32 msec)
{
    static u16 earphone_mute_timer = 0;
    static u8 tmp_param[3] = {0};
    memcpy(tmp_param, other_opt, sizeof(other_opt));
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED && TWS_ROLE_SLAVE == tws_api_get_role() && 2 == tmp_param[2]) {
        find_device_reset();
        earphone_mute_timer_func(tmp_param);
    }
#endif
    if (earphone_mute_timer) {
        sys_timeout_del(earphone_mute_timer);
        earphone_mute_timer = 0;
    }

    if (find_device_key_flag) {
        earphone_mute_timer = sys_timeout_add(tmp_param, earphone_mute_timer_func, msec);
    }
}

#if TCFG_USER_TWS_ENABLE
void find_decice_tws_connect_handle(u8 flag, u8 *param)
{
    static u8 sync_param[4] = {0};
    u8 other_opt[3] = {0};
    switch (flag) {
    case 0:
        if (find_device_key_flag) {
            memcpy(other_opt, sync_param, 2);
            extern void find_devic_stop_timer(u8 * param, u32 msec);
            find_devic_stop_timer(other_opt, 300);
            memcpy(other_opt, sync_param + 2, 2);
            extern void find_device_sync(u8 * param, u32 msec);
            find_device_sync(other_opt, 300);
        }
        break;
    case 1:
        memcpy(sync_param, param, 2);
        break;
    case 2:
        memcpy(sync_param + 2, param, 2);
        break;
    }
}
#endif

static void find_device_handle(u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    if (BT_CALL_HANGUP != get_call_status()) {
        return;
    }
    u8 type = data[0];
    u8 opt = data[1];
    // 0 - way, 1 - player, 2 - find_device_key_flag
    u8 other_opt[3] = {0};

    if (opt) {
        u16 timeout = READ_BIG_U16(data + 2);
#if TCFG_USER_TWS_ENABLE
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            memcpy(other_opt, &timeout, sizeof(timeout));
            extern void find_devic_stop_timer(u8 * param, u32 msec);
            find_devic_stop_timer(other_opt, 300);
        } else
#endif
        {
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_STOP, &timeout, sizeof(timeout));
        }
    } else {
        find_device_stop(NULL);
    }

    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);

    // len为4时没有way和player两种设置
    if (4 == len) {
        return;
    }

    // 铃声播放时才给way和player赋值
    if (6 == len && opt) {
        other_opt[0] = data[4]; // way
        other_opt[1] = data[5]; // player
    }

    // 300ms超时，发送对端同步执行
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
        extern void find_device_sync(u8 * param, u32 msec);
        find_device_sync(other_opt, 300);
        return;
    }
#endif
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_FIND_DEVICE_RESUME, other_opt, sizeof(other_opt));
}

// 按键触发设备查找手机
void JL_rcsp_find_device(void)
{
    if (0 == get_rcsp_connect_status()) {
        return;
    }
    if (find_device_key_flag) {
        find_device_stop(NULL);
    } else {
        find_device_key_flag = 1;
        if ((__this->phone_platform == APPLE_IOS) && (0 != get_curr_channel_state())) {
            // IOS平台
            user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
        }
        //				 查找手机	播放铃声	超时时间(默认10s)
        u8 send_buf[4] = {0x00, 	0x01, 		0x00, 0x0A};
        JL_CMD_send(JL_OPCODE_SYS_FIND_DEVICE, send_buf, sizeof(send_buf), JL_NOT_NEED_RESPOND);
    }
}

#else
u8 find_device_key_flag_get(void)
{
    return 0;
}
#endif

//phone send cmd to firmware need respond
static void JL_rcsp_cmd_resp(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    FILE *fp = NULL;
    u32 r_len;
    u8 md5[32] = {0};
    u8 low_latency_param[6];
    rcsp_printf("JL_ble_cmd_resp\n");
    switch (OpCode) {
    case JL_OPCODE_GET_TARGET_FEATURE:
        rcsp_printf("JL_OPCODE_GET_TARGET_INFO\n");
        JL_opcode_get_target_info(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SWITCH_DEVICE:
        __this->device_type = data[0];
        if (len > 1) {          //新增一个Byte用于标识是否支持新的回连方式, 新版本APP会发多一个BYTE用于标识是否支持新的回连方式
            __this->new_reconn_flag = data[1];
            JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &__this->new_reconn_flag, 1);
        } else {
            JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
        }
#if RCSP_UPDATE_EN
        if (get_jl_update_flag()) {
            if (RCSP_BLE == JL_get_cur_bt_channel_sel()) {
                rcsp_printf("BLE_ CON START DISCON\n");
                JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_DEV_DISCONNECT, NULL, 0);
            } else {
                rcsp_printf("WAIT_FOR_SPP_DISCON\n");
            }
        }
#endif
        JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_SWITCH_DEVICE, &data[0], 1);

        break;

    case JL_OPCODE_SYS_INFO_GET:
        r_printf("JL_OPCODE_SYS_INFO_GET\n");
        JL_opcode_get_sys_info(priv, OpCode, OpCode_SN, data, len);
        break;

    case JL_OPCODE_SYS_INFO_SET:
        r_printf("JL_OPCODE_SYS_INFO_SET\n");
        JL_opcode_set_sys_info(priv, OpCode, OpCode_SN, data, len);
        break;

    case JL_OPCODE_SET_ADV:
        rcsp_printf(" JL_OPCODE_SET_ADV\n");
        JL_opcode_set_adv_info(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_GET_ADV:
        rcsp_printf(" JL_OPCODE_GET_ADV\n");
        JL_opcode_get_adv_info(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_ADV_NOTIFY_SETTING:
        rcsp_printf(" JL_OPCODE_ADV_NOTIFY_SETTING\n");
        bt_ble_adv_ioctl(BT_ADV_SET_NOTIFY_EN, *((u8 *)data), 1);
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
        break;
    case JL_OPCODE_ADV_DEVICE_REQUEST:
        rcsp_printf("JL_OPCODE_ADV_DEVICE_REQUEST\n");
        break;

#if RCSP_ADV_FIND_DEVICE_ENABLE
    case JL_OPCODE_SYS_FIND_DEVICE:
        rcsp_printf("JL_OPCODE_SYS_FIND_DEVICE\n");
        find_device_handle(OpCode, OpCode_SN, data, len);
        break;
#endif

    case JL_OPCODE_FUNCTION_CMD:
        function_cmd_handle(priv, OpCode, OpCode_SN, data, len);
        break;

    case JL_OPCODE_GET_MD5:
        rcsp_printf("JL_OPCODE_GET_MD5\n");
        fp = fopen(RES_MD5_FILE, "r");
        if (!fp) {
            JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
            break;
        }
        r_len = fread(fp, (void *)md5, 32);
        if (r_len != 32) {
            JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
            break;
        }

        printf("get [md5] succ:");
        put_buf(md5, 32);
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, md5, 32);

        fclose(fp);
        break;

    case JL_OPCODE_LOW_LATENCY_PARAM:
        low_latency_param[0] = 0;
        low_latency_param[1] = 20;
        low_latency_param[2] = 100 >> 24;
        low_latency_param[3] = 100 >> 16;
        low_latency_param[4] = 100 >> 8;
        low_latency_param[5] = 100;
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, low_latency_param, 6);
        break;

    case JL_OPCODE_SET_DEVICE_REBOOT:
        rcsp_printf("JL_OPCODE_SET_DEVICE_REBOOT\n");

        JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_REBOOT_DEV, NULL, 0);

        break;

    case JL_OPCODE_CUSTOMER_USER:
        rcsp_user_recv_cmd_resp(data, len);
        JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
        break;

    default:
#if RCSP_UPDATE_EN
        if ((OpCode >= JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET) && \
            (OpCode <= JL_OPCODE_SET_DEVICE_REBOOT)) {
            JL_rcsp_update_cmd_resp(priv, OpCode, OpCode_SN, data, len);
        } else
#endif
        {
            JL_CMD_response_send(OpCode, JL_ERR_NONE, OpCode_SN, data, len);
        }
        break;
    }

//  JL_ERR JL_CMD_response_send(u8 OpCode, u8 status, u8 sn, u8 *data, u16 len)
}

//phone send cmd to firmware not need respond
static void JL_rcsp_cmd_no_resp(void *priv, u8 OpCode, u8 *data, u16 len)
{
    rcsp_printf("JL_ble_cmd_no_resp\n");
    switch (OpCode) {

    default:
        break;
    }
}

//phone send data to firmware need respond
static void JL_rcsp_data_resp(void *priv, u8 OpCode_SN, u8 CMD_OpCode, u8 *data, u16 len)
{
    rcsp_printf("JL_ble_data_resp\n");
    switch (CMD_OpCode) {
    case 0:
        break;

    default:
        break;
    }

}
//phone send data to firmware not need respond
static void JL_rcsp_data_no_resp(void *priv, u8 CMD_OpCode, u8 *data, u16 len)
{
    rcsp_printf("JL_ble_data_no_resp\n");
    switch (CMD_OpCode) {
    case 0:
        break;

    default:
        break;
    }

}

//phone respone firmware cmd
static void JL_rcsp_cmd_recieve_resp(void *priv, u8 OpCode, u8 status, u8 *data, u16 len)
{
    rcsp_printf("rec resp:%x\n", OpCode);

    switch (OpCode) {

    case JL_OPCODE_CUSTOMER_USER:
        rcsp_user_recv_resp(data, len);
        break;

    default:
#if RCSP_UPDATE_EN
        if ((OpCode >= JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET) && \
            (OpCode <= JL_OPCODE_SET_DEVICE_REBOOT)) {
            JL_rcsp_update_cmd_receive_resp(priv, OpCode, status, data, len);
        }
#endif
        break;
    }
}
//phone respone firmware data
static void JL_rcsp_data_recieve_resp(void *priv, u8 status, u8 CMD_OpCode, u8 *data, u16 len)
{
    rcsp_printf("JL_ble_data_recieve_resp\n");
    switch (CMD_OpCode) {

    case 0:
        break;

    default:
        break;
    }

}
//wait resp timout
static u8 JL_rcsp_wait_resp_timeout(void *priv, u8 OpCode, u8 counter)
{
    rcsp_printf("JL_rcsp_wait_resp_timeout\n");

    return 0;
}


///**************************************************************************************///
///************     rcsp ble                                                   **********///
///**************************************************************************************///
static void JL_ble_status_callback(void *priv, ble_state_e status)
{
    rcsp_printf("JL_ble_status_callback==================== %d\n", status);
    __this->JL_ble_status = status;
    switch (status) {
    case BLE_ST_IDLE:
#if RCSP_UPDATE_EN
        if (get_jl_update_flag()) {
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_START, NULL, 0);
        }
#endif
        break;
    case BLE_ST_ADV:
        break;
    case BLE_ST_CONNECT:
#if (RCSP_ADV_FIND_DEVICE_ENABLE)
        printf("smartbox_find_device_reset\n");
        find_device_reset();
#endif
        break;
    case BLE_ST_SEND_DISCONN:
        break;
    case BLE_ST_DISCONN:
#if defined(TCFG_AUDIO_HEARING_AID_ENABLE) && (TCFG_AUDIO_HEARING_AID_ENABLE == 1)
        /*与APP断连时，需要关闭辅听验配单频声音*/
        extern void audio_dha_fitting_sync_close(void);
        audio_dha_fitting_sync_close();
#endif /*defined(TCFG_AUDIO_HEARING_AID_ENABLE) && (TCFG_AUDIO_HEARING_AID_ENABLE == 1)*/
#if RCSP_UPDATE_EN
        if (get_jl_update_flag()) {
            bt_ble_adv_enable(0);
        }
#endif
        break;
    case BLE_ST_NOTIFY_IDICATE:
        break;
    default:
        break;
    }
}

static bool JL_ble_fw_ready(void *priv)
{
    return ((__this->JL_ble_status == BLE_ST_NOTIFY_IDICATE) ? true : false);
}

static s32 JL_ble_send(void *priv, void *data, u16 len)
{
    if ((__this->rcsp_ble != NULL) && (__this->JL_ble_status == BLE_ST_NOTIFY_IDICATE)) {
        int err = __this->rcsp_ble->send_data(NULL, (u8 *)data, len);
        /* rcsp_printf("send :%d\n", len); */
        if (len < 128) {
            /* rcsp_printf_buf(data, len); */
        } else {
            /* rcsp_printf_buf(data, 128); */
        }

        if (err == 0) {
            return 0;
        } else if (err == APP_BLE_BUFF_FULL) {
            return 1;
        }
    } else {
        rcsp_printf("send err -1 !!\n");
    }

    return -1;
}

static const JL_PRO_CB JL_pro_BLE_callback = {
    .priv              = NULL,
    .fw_ready          = JL_ble_fw_ready,
    .fw_send           = JL_ble_send,
    .CMD_resp          = JL_rcsp_cmd_resp,
    .CMD_no_resp       = JL_rcsp_cmd_no_resp,
    .DATA_resp         = JL_rcsp_data_resp,
    .DATA_no_resp      = JL_rcsp_data_no_resp,
    .CMD_recieve_resp  = JL_rcsp_cmd_recieve_resp,
    .DATA_recieve_resp = JL_rcsp_data_recieve_resp,
    .wait_resp_timeout = JL_rcsp_wait_resp_timeout,
};

static void rcsp_ble_callback_set(void (*resume)(void), void (*recieve)(void *, void *, u16), void (*status)(void *, ble_state_e))
{
    __this->rcsp_ble->regist_wakeup_send(NULL, resume);
    __this->rcsp_ble->regist_recieve_cbk(NULL, recieve);
    __this->rcsp_ble->regist_state_cbk(NULL, status);
}
#if 1//RCSP_UPDATE_EN
u8 JL_get_cur_bt_channel_sel(void)
{
    return JL_bt_chl;
}

void JL_ble_disconnect(void)
{
    __this->rcsp_ble->disconnect(NULL);
}

u8 get_curr_device_type(void)
{
    return __this->device_type;
}

void set_curr_update_type(u8 type)
{
    __this->device_type = type;
}

u8 get_defalut_bt_channel_sel(void)
{
    return RCSP_CHANNEL_SEL;
}
#endif

///**************************************************************************************///
///************     rcsp spp                                                   **********///
///**************************************************************************************///
static void JL_spp_status_callback(u8 status)
{
    switch (status) {
    case 0:
        __this->JL_spp_status = 0;
        break;
    case 1:
        __this->JL_spp_status = 1;
#if (JL_EARPHONE_APP_EN && RCSP_ADV_FIND_DEVICE_ENABLE)
        printf("smartbox_find_device_reset\n");
        find_device_reset();
#endif
        break;
#if defined(TCFG_AUDIO_HEARING_AID_ENABLE) && (TCFG_AUDIO_HEARING_AID_ENABLE == 1)
    case 2:
        /*与APP断连时，需要关闭辅听验配单频声音*/
        extern int hearing_aid_fitting_start(u8 en);
        extern u8 set_hearing_aid_fitting_state(u8 state);
        hearing_aid_fitting_start(0);
        set_hearing_aid_fitting_state(0);
        break;
#endif /*defined(TCFG_AUDIO_HEARING_AID_ENABLE) && (TCFG_AUDIO_HEARING_AID_ENABLE == 1)*/
    default:
#if (JL_EARPHONE_APP_EN)
#if RCSP_ADV_MUSIC_INFO_ENABLE
        stop_get_music_timer(1);
#endif
#if (0 == BT_CONNECTION_VERIFY)
        JL_rcsp_auth_reset();
#endif
#if RCSP_UPDATE_EN
        if (get_jl_update_flag()) {
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_START, NULL, 0);
        }
#endif  //rcsp_update_en
#endif
        __this->JL_spp_status = 0;
        break;
    }
}

static bool JL_spp_fw_ready(void *priv)
{
    return (__this->JL_spp_status ? true : false);
}

static s32 JL_spp_send(void *priv, void *data, u16 len)
{
    if (len < 128) {
        rcsp_printf("send: \n");
        rcsp_printf_buf(data, (u32)len);
    }
    if ((__this->rcsp_spp != NULL) && (__this->JL_spp_status == 1)) {
        u32 err = __this->rcsp_spp->send_data(NULL, (u8 *)data, len);
        if (err == 0) {
            return 0;
        } else if (err == SPP_USER_ERR_SEND_BUFF_BUSY) {
            return 1;
        }
    } else {
        rcsp_printf("send err -1 !!\n");
    }

    return -1;
}

static const JL_PRO_CB JL_pro_SPP_callback = {
    .priv              = NULL,
    .fw_ready          = JL_spp_fw_ready,
    .fw_send           = JL_spp_send,
    .CMD_resp          = JL_rcsp_cmd_resp,
    .DATA_resp         = JL_rcsp_data_resp,
    .CMD_no_resp       = JL_rcsp_cmd_no_resp,
    .DATA_no_resp      = JL_rcsp_data_no_resp,
    .CMD_recieve_resp  = JL_rcsp_cmd_recieve_resp,
    .DATA_recieve_resp = JL_rcsp_data_recieve_resp,
    .wait_resp_timeout = JL_rcsp_wait_resp_timeout,
};

static void rcsp_spp_callback_set(void (*resume)(void), void (*recieve)(void *, void *, u16), void (*status)(u8))
{
    __this->rcsp_spp->regist_wakeup_send(NULL, resume);
    __this->rcsp_spp->regist_recieve_cbk(NULL, recieve);
    __this->rcsp_spp->regist_state_cbk(NULL, status);
}

static int rcsp_spp_data_send(void *priv, u8 *buf, u16 len)
{
    int err = 0;
    if (__this->rcsp_spp != NULL) {
        err = __this->rcsp_spp->send_data(NULL, (u8 *)buf, len);
    }

    return err;

}

static int rcsp_ble_data_send(void *priv, u8 *buf, u16 len)
{
    rcsp_printf("### rcsp_ble_data_send %d\n", len);
    int err = 0;
    if (__this->rcsp_ble != NULL) {
        err = __this->rcsp_ble->send_data(NULL, (u8 *)buf, len);
    }

    return err;

}

static void rcsp_spp_protocol_data_recieve(void *priv, void *buf, u16 len)
{
#if TCFG_USER_TWS_ENABLE
    if (get_jl_update_flag() && (tws_api_get_role() == TWS_ROLE_SLAVE)) {
        return;
    }
#endif
    JL_protocol_data_recieve(priv, buf, len);
}

static const u8 link_key_data[16] = {0x06, 0x77, 0x5f, 0x87, 0x91, 0x8d, 0xd4, 0x23, 0x00, 0x5d, 0xf1, 0xd8, 0xcf, 0x0c, 0x14, 0x2b};
void rcsp_dev_select(u8 type)
{
#if RCSP_UPDATE_EN
    set_jl_update_flag(0);
#endif
    if (type == RCSP_BLE) {
#if 1//RCSP_UPDATE_EN
        JL_bt_chl = RCSP_BLE;
#endif
        rcsp_printf("------RCSP_BLE-----\n");
        rcsp_spp_callback_set(NULL, NULL, NULL);
        rcsp_ble_callback_set(JL_protocol_resume, JL_protocol_data_recieve, JL_ble_status_callback);
        JL_protocol_dev_switch(&JL_pro_BLE_callback);
        JL_rcsp_auth_init(rcsp_ble_data_send, (u8 *)link_key_data, NULL);
    } else {
#if 1//RCSP_UPDATE_EN
        JL_bt_chl = RCSP_SPP;
#endif
        rcsp_printf("------RCSP_SPP-----\n");
        rcsp_spp_callback_set(JL_protocol_resume, rcsp_spp_protocol_data_recieve, JL_spp_status_callback);
        rcsp_ble_callback_set(NULL, NULL, NULL);
        JL_protocol_dev_switch(&JL_pro_SPP_callback);
        JL_rcsp_auth_init(rcsp_spp_data_send, (u8 *)link_key_data, NULL);
    }
}

void set_rcsp_dev_type_spp(void)
{
    rcsp_dev_select(RCSP_SPP);
}

////////////////////// RCSP process ///////////////////////////////

static OS_SEM rcsp_sem;

void JL_rcsp_resume_do(void)
{
    os_sem_post(&rcsp_sem);
}

static u32 rcsp_protocol_tick = 0;
static void rcsp_process_timer()
{
    JL_set_cur_tick(rcsp_protocol_tick++);
    os_sem_post(&rcsp_sem);
}

static void rcsp_process_task(void *p)
{
    while (1) {
        os_sem_pend(&rcsp_sem, 0);
        JL_protocol_process();
    }
}

/////////////////////////////////////////////////////

static u8 *rcsp_buffer = NULL;
void rcsp_init()
{
    memcpy(_s_info.edr_name, bt_get_local_name(), 32);

    if (__this->rcsp_run_flag) {
        return;
    }

    memset((u8 *)__this, 0, sizeof(struct JL_AI_VAR));

    /* __this->start_speech = start_speech; */
    /* __this->stop_speech = stop_speech; */
    //__this->rcsp_user = (struct __rcsp_user_var *)get_user_rcsp_opt();

    ble_get_server_operation_table(&__this->rcsp_ble);
    spp_get_operation_table(&__this->rcsp_spp);

    set_jl_mtu_send(256);

    u32 size = rcsp_protocol_need_buf_size();
    rcsp_printf("rcsp need buf size:%x\n", size);

    rcsp_buffer = zalloc(size);
    ASSERT(rcsp_buffer, "no, memory for rcsp_init\n");
    JL_protocol_init(rcsp_buffer, size);

    os_sem_create(&rcsp_sem, 0);
    sys_timer_add(NULL, rcsp_process_timer, 500);
    int err = task_create(rcsp_process_task, NULL, "rcsp_task");

    //default use ble , can switch spp anytime
    rcsp_dev_select(RCSP_BLE);

#if (!MUTIl_CHARGING_BOX_EN)
    adv_setting_init();
#endif

    __this->rcsp_run_flag = 1;
#if (0 != BT_CONNECTION_VERIFY)
    JL_rcsp_set_auth_flag(1);
#endif
}


void rcsp_exit(void)
{
#if ((AI_SOUNDBOX_EN==1) && (RCSP_ADV_EN==1))
    if (speech_status() == true) {
        rcsp_cancel_speech();
        speech_stop();
    }
    __set_a2dp_sound_detect_counter(50, 250); /*第一个参数是后台检测返回蓝牙的包数目，第二个参数是退出回到原来模式静音的包数目*/
    __this->wait_asr_end = 0;
#endif

    if (rcsp_buffer) {
        free(rcsp_buffer);
        rcsp_buffer = 0;
    }

    rcsp_update_data_api_unregister();
    rcsp_printf("####  rcsp_exit_cb\n");
    task_kill("rcsp_task");
    return;
}

int app_core_data_for_send(u8 *packet, u16 size)
{
    //printf("for app send size %d\n", size);

    if (JL_rcsp_get_auth_flag()) {
        *packet = 1;
    } else {
        *packet = 0;
    }

    if (RCSP_SPP == JL_get_cur_bt_channel_sel()) {
        if (get_ble_adv_notify()) {
            *(packet + 1) = 1;
        } else {
            *(packet + 1) = 0;
        }

#if RCSP_ADV_MUSIC_INFO_ENABLE
        if (get_player_time_en()) {
            *(packet + 2) = 1;
        } else {
            *(packet + 2) = 0;
        }
#else
        *(packet + 2) = 0;
#endif

        *(packet + 3) = get_connect_flag();
    }

    return 4;
}

void app_core_data_for_set(u8 *packet, u16 size)
{
    u8 rcsp_auth_flag =  *packet;
    u8 ble_adv_notify =  *(packet + 1);
    u8 player_time_en =  *(packet + 2);
    u8 connect_flag   =  *(packet + 3);

    if (RCSP_SPP == JL_get_cur_bt_channel_sel()) {
        JL_rcsp_set_auth_flag(rcsp_auth_flag);
        set_ble_adv_notify(ble_adv_notify);
#if RCSP_ADV_MUSIC_INFO_ENABLE
        set_player_time_en(player_time_en);
#endif
        set_connect_flag(connect_flag);
    }

    return;
}

u8 smart_auth_res_deal(void)
{

    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        if (RCSP_SPP == JL_get_cur_bt_channel_sel()) {
            tws_api_sync_call_by_uuid('T', SYNC_CMD_RCSP_AUTH_RES, 300);
            return 0;
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}


void rcsp_tws_auth_sync_deal(void)
{
    smart_auth_res_pass();
}



#endif
