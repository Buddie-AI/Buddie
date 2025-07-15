#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "le_rcsp_adv_module.h"
#include "rcsp_adv_bluetooth.h"

#include "rcsp_adv_opt.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "adv_time_stamp_setting.h"
#include "adv_bt_name_setting.h"
#include "adv_key_setting.h"
#include "adv_led_setting.h"
#include "adv_mic_setting.h"
#include "adv_work_setting.h"
#include "adv_eq_setting.h"
#include "adv_high_low_vol.h"
#include "adv_anc_voice_key.h"
#include "bt_tws.h"
#include "btstack/avctp_user.h"

#include "adv_anc_voice.h"
#include "adv_hearing_aid_setting.h"

#if (RCSP_ADV_EN)
static u16 adv_setting_event_flag = (u16) - 1;
static void set_adv_setting_event_flag(u16 flag)
{
    adv_setting_event_flag = flag;
}

static u16 get_adv_setting_event_flag(void)
{
    return adv_setting_event_flag;
}

static u8 deal_adv_setting_string_item(u8 *des, u8 *src, u8 src_len, u8 type)
{
    des[0] = type;
    memcpy(des + 1, src, src_len);
    return src_len + sizeof(type);
}

void update_info_from_adv_vm_info(void)
{
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_TIME_STAMP)) {
        deal_time_stamp_setting(0, 1, 0);
    }

#if RCSP_ADV_NAME_SET_ENABLE
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_EDR_NAME)) {
        deal_bt_name_setting(NULL, 1, 0);
    }
#endif

#if RCSP_ADV_KEY_SET_ENABLE
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_KEY_SETTING)) {
        deal_key_setting(NULL, 1, 0);
    }

    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_ANC_VOICE_KEY)) {
        deal_anc_voice_key_setting(NULL, 1, 0);
    }
#endif

#if RCSP_ADV_LED_SET_ENABLE
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_LED_SETTING)) {
        deal_led_setting(NULL, 1, 0);
    }
#endif

#if RCSP_ADV_MIC_SET_ENABLE
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_MIC_SETTING)) {
        deal_mic_setting(0, 1, 0);
    }
#endif

#if RCSP_ADV_WORK_SET_ENABLE
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_WORK_MODE)) {
        deal_work_setting(0, 1, 0, 0);
    }
#endif

#if RCSP_ADV_EQ_SET_ENABLE
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_EQ_SETTING)) {
        deal_eq_setting(0, 1, 0);
    }
#endif

#if RCSP_ADV_HIGH_LOW_SET
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_HIGH_LOW_VOL)) {
        deal_high_low_vol(NULL, 1, 0);
    }
#endif

#if (RCSP_ADV_ANC_VOICE)
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_ANC_VOICE)) {
        deal_anc_voice(NULL, 1, 0);
    }
#endif

#if (RCSP_ADV_ASSISTED_HEARING)
    if (get_adv_setting_event_flag() & BIT(ATTR_TYPE_ASSISTED_HEARING)) {
        deal_hearing_aid_setting(-1, -1, NULL, 1, 0);
    }
#endif

    set_adv_setting_event_flag(0);
}

static u8 adv_read_data_from_vm(u8 syscfg_id, u8 *buf, u8 buf_len)
{
    int len = 0;
    u8 i = 0;


    len = syscfg_read(syscfg_id, buf, buf_len);

    if (len > 0) {
        for (i = 0; i < buf_len; i++) {
            if (buf[i] != 0xff) {
                return (buf_len == len);
            }
        }
    }

    return 0;
}

void adv_setting_init(void)
{
    u32 time_stamp = 0;
    if (adv_read_data_from_vm(CFG_RCSP_ADV_TIME_STAMP, (u8 *)&time_stamp, sizeof(time_stamp))) {
        set_adv_time_stamp(time_stamp);
        //deal_time_stamp_setting(0, 0, 0);
    }

#if RCSP_ADV_NAME_SET_ENABLE
    u8 bt_name_info[32] = {0};
    if (adv_read_data_from_vm(CFG_BT_NAME, bt_name_info, sizeof(bt_name_info))) {
        set_bt_name_setting(bt_name_info);
        //deal_bt_name_setting(NULL, 0, 0);
    }
#endif

#if RCSP_ADV_KEY_SET_ENABLE
    u8 key_setting_info[4] = {0};
    if (adv_read_data_from_vm(CFG_RCSP_ADV_KEY_SETTING, key_setting_info, sizeof(key_setting_info))) {
        set_key_setting(key_setting_info);
        deal_key_setting(NULL, 0, 0);
    }

    u8 key_anc_voice_mode[4] = {0};
    if (adv_read_data_from_vm(CFG_RCSP_ADV_ANC_VOICE_KEY, key_anc_voice_mode, sizeof(key_anc_voice_mode))) {
        set_anc_voice_key_mode(key_anc_voice_mode);
        deal_anc_voice_key_setting(NULL, 0, 0);
    }
#endif

#if RCSP_ADV_LED_SET_ENABLE
    u8 led_setting_info[3] = {0};
    if (adv_read_data_from_vm(CFG_RCSP_ADV_LED_SETTING, led_setting_info, sizeof(led_setting_info))) {
        set_led_setting(led_setting_info);
        deal_led_setting(NULL, 0, 0);
    }
#endif

#if RCSP_ADV_MIC_SET_ENABLE
    u8 mic_setting_info = 0;
    if (adv_read_data_from_vm(CFG_RCSP_ADV_MIC_SETTING, &mic_setting_info, sizeof(mic_setting_info))) {
        set_mic_setting(mic_setting_info);
        deal_mic_setting(0, 0, 0);
    }
#endif

#if RCSP_ADV_WORK_SET_ENABLE
    u8 work_setting_info = 0;
    if (adv_read_data_from_vm(CFG_RCSP_ADV_WORK_SETTING, &work_setting_info, sizeof(work_setting_info))) {
        set_work_setting(work_setting_info);
        deal_work_setting(0, 0, 0, 1);
    }
#endif

#if RCSP_ADV_EQ_SET_ENABLE
    u8 eq_setting_vm_info[11] = {0};
    u8 eq_setting_info[10] = {0};
    u8 eq_setting_mode = 0;
    u8 i;
    if (adv_read_data_from_vm(CFG_RCSP_ADV_EQ_DATA_SETTING, eq_setting_info, sizeof(eq_setting_info))) {
        if (adv_read_data_from_vm(CFG_RCSP_ADV_EQ_MODE_SETTING, &eq_setting_mode, sizeof(eq_setting_mode))) {
            eq_setting_vm_info[0] = eq_setting_mode;
            memcpy(eq_setting_vm_info + 1, eq_setting_info, sizeof(eq_setting_info));
            set_eq_setting(eq_setting_vm_info);
            deal_eq_setting(NULL, 0, 0);
        }
    }
#endif

#if RCSP_ADV_HIGH_LOW_SET
    u8 high_low_vol_setting[8] = {0};
    if (adv_read_data_from_vm(CFG_RCSP_ADV_HIGH_LOW_VOL, high_low_vol_setting, sizeof(high_low_vol_setting))) {
        set_high_low_vol_info(high_low_vol_setting);
        deal_high_low_vol(NULL, 0, 0);
    }
#endif

#if (RCSP_ADV_ANC_VOICE)
    u8 anc_voice_setting[25] = {0};
    if (anc_voice_setting_init()) {
        if (adv_read_data_from_vm(CFG_RCSP_ADV_ANC_VOICE_MODE, anc_voice_setting, 1)) {
            if (adv_read_data_from_vm(CFG_RCSP_ADV_ANC_VOICE, anc_voice_setting + 1, sizeof(anc_voice_setting) - 1)) {
                set_anc_voice_info(anc_voice_setting);
                deal_anc_voice(NULL, 0, 0);
            }
        }
    }
#endif
}

// type : 0 ~ 8
// mode : 0 - 从vm读出并更新全局变量数据 // 1 - 同步
void update_adv_setting(u16 type)
{
    u8 offset = 1;
    //                total_len  time      name       key       led       mic      work       eq         vol      anc      anc_voice
    u8 adv_setting_to_sync[1 + (4 + 1) + (32 + 1) + (4 + 1) + (3 + 1) + (1 + 1) + (1 + 1) + (11 + 1) + (8 + 1) + (25 + 1) + (4 + 1)] = {0};
    memset(adv_setting_to_sync, 0, sizeof(adv_setting_to_sync));

    if (type & BIT(ATTR_TYPE_TIME_STAMP)) {
        u32 time_stamp = get_adv_time_stamp();
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, (u8 *)&time_stamp, sizeof(time_stamp), ATTR_TYPE_TIME_STAMP);
    }

#if RCSP_ADV_NAME_SET_ENABLE
    if (type & BIT(ATTR_TYPE_EDR_NAME)) {
        u8 bt_name_info[32] = {0};
        get_bt_name_setting(bt_name_info);
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, bt_name_info, sizeof(bt_name_info), ATTR_TYPE_EDR_NAME);
    }
#endif

#if RCSP_ADV_KEY_SET_ENABLE
    if (type & BIT(ATTR_TYPE_KEY_SETTING)) {
        u8 key_setting_info[4] = {0};
        get_key_setting(key_setting_info);
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, key_setting_info, sizeof(key_setting_info), ATTR_TYPE_KEY_SETTING);
    }

    if (type & BIT(ATTR_TYPE_ANC_VOICE_KEY)) {
        u8 key_anc_voice_mode[4] = {0};
        get_anc_voice_key_mode(key_anc_voice_mode);
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, key_anc_voice_mode, sizeof(key_anc_voice_mode), ATTR_TYPE_ANC_VOICE_KEY);
    }
#endif

#if RCSP_ADV_LED_SET_ENABLE
    if (type & BIT(ATTR_TYPE_LED_SETTING)) {
        u8 led_setting_info[3] = {0};
        get_led_setting(led_setting_info);
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, led_setting_info, sizeof(led_setting_info), ATTR_TYPE_LED_SETTING);
    }
#endif

#if RCSP_ADV_MIC_SET_ENABLE
    if (type & BIT(ATTR_TYPE_MIC_SETTING)) {
        u8 mic_setting_info = get_mic_setting();
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, &mic_setting_info, sizeof(mic_setting_info), ATTR_TYPE_MIC_SETTING);
    }
#endif

#if RCSP_ADV_WORK_SET_ENABLE
    if (type & BIT(ATTR_TYPE_WORK_MODE)) {
        u8 work_setting_info = get_work_setting();
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, &work_setting_info, sizeof(work_setting_info), ATTR_TYPE_WORK_MODE);
    }
#endif

#if RCSP_ADV_EQ_SET_ENABLE
    if (type & BIT(ATTR_TYPE_EQ_SETTING)) {
        u8 eq_setting_info[11];
        get_eq_setting(eq_setting_info);
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, eq_setting_info, sizeof(eq_setting_info), ATTR_TYPE_EQ_SETTING);
    }
#endif

#if RCSP_ADV_HIGH_LOW_SET
    if (type & BIT(ATTR_TYPE_HIGH_LOW_VOL)) {
        u8 high_low_vol[8];
        get_high_low_vol_info(high_low_vol);
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, high_low_vol, sizeof(high_low_vol), ATTR_TYPE_HIGH_LOW_VOL);
    }
#endif

#if (RCSP_ADV_ANC_VOICE)
    if (type & BIT(ATTR_TYPE_ANC_VOICE)) {
        u8 anc_voice_info[25];
        get_anc_voice_info(anc_voice_info);
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, anc_voice_info, sizeof(anc_voice_info), ATTR_TYPE_ANC_VOICE);
    }
#endif

#if RCSP_ADV_ASSISTED_HEARING
    if (((u16) - 1) != type && type & BIT(ATTR_TYPE_ASSISTED_HEARING)) {
        u8 dha_fitting_data[3 + 2 * 4 * DHA_FITTING_CHANNEL_MAX];
        get_hearing_aid_setting(dha_fitting_data);
        offset += deal_adv_setting_string_item(adv_setting_to_sync + offset, dha_fitting_data, sizeof(dha_fitting_data), ATTR_TYPE_ASSISTED_HEARING);
    }
#endif

    if (offset > 1) {
        adv_setting_to_sync[0] = offset;
        if (type == ((u16) - 1)) {
            tws_api_send_data_to_sibling(adv_setting_to_sync, sizeof(adv_setting_to_sync), TWS_FUNC_ID_ADV_SETTING_SYNC);
#if RCSP_ADV_LED_SET_ENABLE
            tws_api_sync_call_by_uuid('T', SYNC_CMD_APP_RESET_LED_UI, 300);
#endif
        } else {
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                tws_api_send_data_to_sibling(adv_setting_to_sync, sizeof(adv_setting_to_sync), TWS_FUNC_ID_ADV_SETTING_SYNC);
#if RCSP_ADV_LED_SET_ENABLE
                tws_api_sync_call_by_uuid('T', SYNC_CMD_APP_RESET_LED_UI, 300);
#endif
            }
        }
    }
}

void deal_sibling_setting(u8 *buf)
{
    u8 type;
    u8 len = buf[0];
    u8 offset = 1;
    u8 *data;
    u32 time_stamp = 0;
    set_adv_setting_event_flag(0);
    while (offset < len) {
        type = buf[offset++];
        data = buf + offset;
        switch (type) {
#if RCSP_ADV_NAME_SET_ENABLE
        case ATTR_TYPE_EDR_NAME:
            set_bt_name_setting(data);
            offset += 32;
            break;
#endif
#if RCSP_ADV_KEY_SET_ENABLE
        case ATTR_TYPE_KEY_SETTING :
            set_key_setting(data);
            offset += 4;
            break;
        case ATTR_TYPE_ANC_VOICE_KEY:
            set_anc_voice_key_mode(data);
            offset += 4;
            break;
#endif
#if RCSP_ADV_LED_SET_ENABLE
        case ATTR_TYPE_LED_SETTING :
            set_led_setting(data);
            offset += 3;
            break;
#endif
#if RCSP_ADV_MIC_SET_ENABLE
        case ATTR_TYPE_MIC_SETTING :
            set_mic_setting(*data);
            offset += 1;
            break;
#endif
#if RCSP_ADV_WORK_SET_ENABLE
        case ATTR_TYPE_WORK_MODE :
            set_work_setting(*data);
            offset += 1;
            break;
#endif
#if RCSP_ADV_EQ_SET_ENABLE
        case ATTR_TYPE_EQ_SETTING :
            /* g_printf("tws eq setting\n"); */
            set_eq_setting(data);
            offset += 11;
            break;
#endif
#if RCSP_ADV_HIGH_LOW_SET
        case ATTR_TYPE_HIGH_LOW_VOL:
            set_high_low_vol_info(data);
            offset += 8;
            break;
#endif
#if (RCSP_ADV_ANC_VOICE)
        case ATTR_TYPE_ANC_VOICE:
            set_anc_voice_info(data);
            offset += 25;
            break;
#endif
#if (RCSP_ADV_ASSISTED_HEARING)
        case ATTR_TYPE_ASSISTED_HEARING:
            set_hearing_aid_setting(data);
            offset += 3 + 2 * 4 * DHA_FITTING_CHANNEL_MAX;
            break;
#endif
        case ATTR_TYPE_TIME_STAMP:
            time_stamp = (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));//*((u32 *)data);

            set_adv_time_stamp(time_stamp);
            offset += 4;
            break;
        default:
            return;
        }
        set_adv_setting_event_flag(get_adv_setting_event_flag() | BIT(type));
    }
    // 发送事件
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_ADV_SETTING_UPDATE, NULL, 0);
}

#if (0)
static void adv_reset_data_to_vm(u8 syscfg_id, u8 *buf, u8 buf_len)
{
    u8 res = 0;
    res = syscfg_write(syscfg_id, buf, buf_len);
}

u8 rcsp_setting_info_reset() //恢复默认的APP设置
{
    printf("%s", __func__);

    u32 time_stamp = 0xffffffff;
    adv_reset_data_to_vm(CFG_RCSP_ADV_TIME_STAMP, (u8 *)&time_stamp, sizeof(time_stamp));

    /* #if RCSP_ADV_NAME_SET_ENABLE */
    /*     u8 bt_name_info[32] = {0}; */
    /*     memset(bt_name_info,0xff,32); */
    /*     adv_reset_data_to_vm(CFG_BT_NAME, bt_name_info, sizeof(bt_name_info)); */
    /* #endif */

#if RCSP_ADV_KEY_SET_ENABLE
    u8 key_setting_info[4] = {0};
    memset(key_setting_info, 0xff, 4);
    adv_reset_data_to_vm(CFG_RCSP_ADV_KEY_SETTING, key_setting_info, sizeof(key_setting_info));
#endif

#if RCSP_ADV_LED_SET_ENABLE
    u8 led_setting_info[3] = {0};
    memset(led_setting_info, 0xff, 3);
    adv_reset_data_to_vm(CFG_RCSP_ADV_LED_SETTING, led_setting_info, sizeof(led_setting_info));
#endif

#if RCSP_ADV_MIC_SET_ENABLE
    u8 mic_setting_info = 0xff;
    adv_reset_data_to_vm(CFG_RCSP_ADV_MIC_SETTING, &mic_setting_info, sizeof(mic_setting_info));
#endif

#if RCSP_ADV_WORK_SET_ENABLE
    u8 work_setting_info = 0xff;
    adv_reset_data_to_vm(CFG_RCSP_ADV_WORK_SETTING, &work_setting_info, sizeof(work_setting_info));
#endif

#if RCSP_ADV_EQ_SET_ENABLE
    u8 eq_setting_info[10] = {0};
    u8 eq_setting_mode = 0xff;
    memset(eq_setting_info, 0xff, 10);
    adv_reset_data_to_vm(CFG_RCSP_ADV_EQ_DATA_SETTING, &eq_setting_info, sizeof(eq_setting_info));
    adv_reset_data_to_vm(CFG_RCSP_ADV_EQ_MODE_SETTING, &eq_setting_mode, sizeof(eq_setting_mode));
#endif
    return 0;
}

u8 remap_app_chargestore_data_deal(u8 *buf, u8 len)
{
    u8 send_buf[30];
    u8 ret = 0;
    send_buf[0] = buf[0];
//#define CMD_GET_NAME             	  0xc1
    switch (buf[0]) {
    case 0x08:
        //printf("\n\n\n\n\n\n---------------reset store sys\n");
        ret = 1 ;

        void bt_tws_remove_pairs();
        bt_tws_remove_pairs();
        extern u8 rcsp_setting_info_reset();
        rcsp_setting_info_reset();
        user_send_cmd_prepare(USER_CTRL_DEL_ALL_REMOTE_INFO, 0, NULL);
        os_time_dly(60);
        cpu_reset();

        break;

    default:
        break;
    }
    return ret;
}
#endif


#endif
