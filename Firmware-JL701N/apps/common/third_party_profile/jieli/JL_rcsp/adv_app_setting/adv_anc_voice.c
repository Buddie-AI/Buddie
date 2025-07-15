#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "le_rcsp_adv_module.h"

#include "adv_anc_voice.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"

#include "rcsp_adv_bluetooth.h"

#if (RCSP_ADV_EN && RCSP_ADV_ANC_VOICE)

#include "audio_anc.h"

// anc信息：mode(1byte/0/1/2) + ( left_max(2byte) + right_max(2byte) + left_cur_val(2byte) + right_cur_val(2byte) ) * 3
static u8 g_anc_info[25] = {0};

extern int get_bt_tws_connect_status();
extern u8 JL_get_cur_bt_channel_sel(void);

static bool check_pos_neg(u8 mode)
{
    bool ret = false;
    u8 offset = 1 + mode * 8;
#if TCFG_USER_TWS_ENABLE
    if ('R' == tws_api_get_local_channel()) {
        offset += 2;
    }
#endif
    s16 val = g_anc_info[offset] << 8 | g_anc_info[offset + 1];
    if (val < 0) {
        ret = true;
    }
    return ret;
}

void get_anc_voice_info(u8 *anc_info)
{
    memcpy(anc_info, g_anc_info, sizeof(g_anc_info));
}

void set_anc_voice_info(u8 *anc_info)
{
    memcpy(g_anc_info, anc_info, sizeof(g_anc_info));
}

static void update_anc_voice_vm_value(u8 *anc_info)
{
    u8 anc_vm_info[25] = {-1};
    int ret = syscfg_read(CFG_RCSP_ADV_ANC_VOICE_MODE, anc_vm_info, 1);
    if (0 != memcmp(anc_vm_info, anc_info, 1)) {
        ret = syscfg_write(CFG_RCSP_ADV_ANC_VOICE_MODE, anc_info, 1);
    }

    ret = syscfg_read(CFG_RCSP_ADV_ANC_VOICE, anc_vm_info + 1, sizeof(anc_vm_info) - 1);
    if (0 != memcmp(anc_vm_info + 1, anc_info + 1, sizeof(anc_vm_info) - 1)) {
        ret = syscfg_write(CFG_RCSP_ADV_ANC_VOICE, anc_info + 1, sizeof(anc_vm_info) - 1);
    }
}

static void anc_voice_sync(u8 *anc_info)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_ANC_VOICE));
    }
#endif
}

static void anc_voice_effect_set(u8 mode, s16 value)
{
    static s16 value_old = -1;
#if TCFG_AUDIO_ANC_ENABLE
    extern void anc_gain_app_value_set(int app_value);
    if (value != value_old) {
        /* audio_anc_gain(value, value); */
        anc_gain_app_value_set(value);
    }
    anc_mode_switch(mode + 1, 1);
#endif
    value_old = value;
}

static void anc_voice_state_update(void)
{
    u8 offset = 0;
    u8 mode = g_anc_info[offset++];
    offset += mode * 8;
    s16 left_max = g_anc_info[offset++] << 8 | g_anc_info[offset++];
    s16 right_max = g_anc_info[offset++] << 8 | g_anc_info[offset++];
    s16 left_val = g_anc_info[offset++] << 8 | g_anc_info[offset++];
    s16 right_val = g_anc_info[offset++] << 8 | g_anc_info[offset++];

#if TCFG_USER_TWS_ENABLE
    if ('R' == tws_api_get_local_channel()) {
        if (check_pos_neg(mode) && right_val >= right_max) {
            anc_voice_effect_set(mode, right_val);
        } else if (right_max >= right_val) {
            anc_voice_effect_set(mode, right_val);
        }
        return;
    }
#endif

    if (check_pos_neg(mode) && left_val >= left_max) {
        anc_voice_effect_set(mode, left_val);
    } else if (left_max >= left_val) {
        anc_voice_effect_set(mode, left_val);
    }
}

void deal_anc_voice(u8 *anc_setting, u8 write_vm, u8 tws_sync)
{
    u8 anc_info[25] = {0};
    if (!anc_setting) {
        get_anc_voice_info(anc_info);
    } else {
        u8 mode = anc_setting[0];
        g_anc_info[0] = mode;
        memcpy(g_anc_info + 1 + mode * 8, anc_setting + 1, 8);
        memcpy(anc_info, g_anc_info, sizeof(anc_info));
    }
    if (write_vm) {
        update_anc_voice_vm_value(anc_info);
    }

    if (tws_sync) {
        anc_voice_sync(anc_info);
    }

    anc_voice_state_update();
}

#if TCFG_USER_TWS_ENABLE
#define TWS_FUNC_ID_ANC_VOICE_SYNC \
	(((u8)('R' + 'C' + 'S' + 'P') << (3 * 8)) | \
	 ((u8)('A' + 'N' + 'C' + 'S') << (2 * 8)) | \
	 ((u8)('V' + 'O' + 'I' + 'C' + 'E') << (1 * 8)) | \
	 ((u8)('S' + 'Y' + 'N' + 'C') << (0 * 8)))

void anc_voice_max_val_swap_sync(u8 *data, u16 len)
{
    update_anc_voice_vm_value(g_anc_info);
}

static void anc_voice_state_sync(u8 *data, u16 len)
{
    if (NULL == data || 0 == len) {
        JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_ANC_VOICE, NULL, 0);
        return;
    }
    u8 offset = 9;
    // 假如当前是右耳，应该填写左值
    // 假如当前是左耳，应该填写右值
    if ('L' == tws_api_get_local_channel()) {
        offset += 2;
    }
    // 根据左右耳填充对应的最大值信息
    // 降噪(2byte) + 通透(2byte)
    memcpy(g_anc_info + offset, data, 2);
    memcpy(g_anc_info + offset + 4, data + 4, 2);
    offset += 8;
    memcpy(g_anc_info + offset, data + 2, 2);
    memcpy(g_anc_info + offset + 4, data + 6, 2);
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_ANC_VOICE_MAX_SYNC, NULL, 0);
}

static void adv_anc_voice_tws_func_t(void *data, u16 len, bool rx)
{
    if (rx) {
        anc_voice_state_sync((u8 *)data, len);
    }
}

REGISTER_TWS_FUNC_STUB(adv_tws_sync) = {
    .func_id = TWS_FUNC_ID_ANC_VOICE_SYNC,
    .func    = adv_anc_voice_tws_func_t,
};

#endif

void rcsp_adv_voice_mode_update(u8 mode)
{
    u8 anc_info[25] = {0};
    get_anc_voice_info(anc_info);
    anc_info[0] = mode;
    if (mode) {
        memcpy(anc_info + 1, anc_info + 1 + mode * 8, 8);
    }
    deal_anc_voice(anc_info, 1, 0);
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status() && TWS_ROLE_SLAVE == tws_api_get_role()) {
        tws_api_send_data_to_sibling(NULL, 0, TWS_FUNC_ID_ANC_VOICE_SYNC);
        return;
    }
#endif
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_ANC_VOICE, NULL, 0);
}

int anc_voice_setting_sync()
{
    int ret = 0;
    u8 anc_info[8] = {0};
#if (TCFG_AUDIO_ANC_ENABLE)
    /* extern int anc_gain_tab[3]; */
    /* u16 noise_reduction = (u16)(16384 & (u16) - 1);		// 调用接口获取降噪最大值 */
    /* u16 transparent = (u16)(16384 & (u16) - 1);			// 调用接口获取通透最大值 */

    u16 noise_reduction = 16384;		// 调用接口获取降噪最大值
    u16 transparent = 16384;			// 调用接口获取通透最大值
#else
    u16 noise_reduction = 0;
    u16 transparent = 0;
#endif

    anc_info[0] = ((u8 *)&noise_reduction)[1];
    anc_info[1] = ((u8 *)&noise_reduction)[0];
    anc_info[2] = ((u8 *)&transparent)[1];
    anc_info[3] = ((u8 *)&transparent)[0];

    u8 offset = 9;
#if TCFG_USER_TWS_ENABLE
    if ('R' == tws_api_get_local_channel()) {
        offset += 2;
    }
#endif

    // 初始值最大和当前值都是同一个值
    // 默认是左值
    u8 tmp_value = -1;
    int result = 0;
    if ((result = syscfg_read(CFG_RCSP_ADV_ANC_VOICE_MODE, &tmp_value, sizeof(tmp_value))) != sizeof(tmp_value)) {
        tmp_value = offset;
        memcpy(g_anc_info + tmp_value, anc_info, 2);
        tmp_value += 4;
        memcpy(g_anc_info + tmp_value, anc_info, 2);
        tmp_value += 4;
        memcpy(g_anc_info + tmp_value, anc_info + 2, 2);
        tmp_value += 4;
        memcpy(g_anc_info + tmp_value, anc_info + 2, 2);
    }

#if TCFG_USER_TWS_ENABLE
    // 填写当前降噪/通透当前值
    memcpy(anc_info + 4, g_anc_info + offset + 4, 2);
    memcpy(anc_info + 6, g_anc_info + offset + 8 + 4, 2);
    // 把自己的最大值发送给对端
    if (get_bt_tws_connect_status()) {
        tws_api_send_data_to_sibling(anc_info, sizeof(anc_info), TWS_FUNC_ID_ANC_VOICE_SYNC);
    }
#endif
    return ret;
}

int anc_voice_setting_init(void)
{
    int ret = 1;
    u8 anc_mode = -1;
    if (syscfg_read(CFG_RCSP_ADV_ANC_VOICE_MODE, &anc_mode, sizeof(anc_mode)) != sizeof(anc_mode)) {
        anc_voice_setting_sync();
        ret = 0;
    }
    return ret;
}

// ====================================================
enum {
    anc_VOICE_GET_PREP_MSG,
    anc_VOICE_SET_CUR_MODE,
    anc_VOICE_NOTICE_MSG,
    anc_VOICE_GET_CUR_MODE,
};

static void s16_pos_neg_conversion(u8 *data)
{
    s16 tmp_val = data[0] << 8 | data[1];
    tmp_val = -tmp_val;
    data[1] = ((u8 *)&tmp_val)[0];
    data[0] = ((u8 *)&tmp_val)[1];
}

u8 anc_voice_info_get(u8 *data, u16 len)
{
    u8 offset = 0;
    u8 anc_info[25] = {0};
    get_anc_voice_info(anc_info);
    u8 mode = anc_info[0];
    data[offset++] = mode;
    memcpy(data + offset, anc_info + 1 + mode * 8, 8);
    if (check_pos_neg(mode)) {
        for (u8 i = 0; i < 8; i += 2) {
            s16_pos_neg_conversion(data + 1 + i);
        }
    }
    offset += 8;
    return offset;
}

u8 anc_voice_info_fetch_all_get(u8 *data, u16 len)
{
    u8 mode = 0;
    u8 offset = 1;
    u8 anc_info[25] = {0};
    get_anc_voice_info(anc_info);
    for (u8 resp_offset = 1; offset < sizeof(anc_info); offset += 8) {
        data[resp_offset++] = mode;
        memcpy(data + resp_offset, anc_info + offset, 8);
        if (check_pos_neg(mode)) {
            for (u8 i = 0; i < 8; i += 2) {
                s16_pos_neg_conversion(data + 2 + mode * 9 + i);
            }
        }
        resp_offset += 8;
        mode++;
    }
    data[0] = mode;
    return len;
}

int anc_voice_info_set(u8 *data, u16 len)
{
    int ret = 0;
    if (check_pos_neg(data[0])) {
        for (u8 i = 0; i < 8; i += 2) {
            s16_pos_neg_conversion(data + 1 + i);
        }
    }
    deal_anc_voice(data, 1, 1);
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_ANC_VOICE, NULL, 0);
    return ret;
}

#endif
