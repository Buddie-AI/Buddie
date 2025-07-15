#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "le_rcsp_adv_module.h"

#include "adv_eq_setting.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"
#include "application/audio_eq.h"

#if (RCSP_ADV_EN)
#if RCSP_ADV_EQ_SET_ENABLE
extern int get_bt_tws_connect_status();
extern int eq_mode_get_freq(u8 mode, u16 index);
extern s8 eq_mode_get_gain(u8 mode, u16 index);
extern int eq_mode_set_custom_param(u16 index, int gain);
extern int eq_mode_set(u8 mode);
extern int eq_mode_get_cur(void);

static void eq_setting_info_deal(u8 *eq_info_data)
{
    u8 data;
    u8 status;
    u8 mode;
    if (eq_info_data[0] >> 7) {
        status = eq_info_data[2];
    } else {
        status = *(((u8 *)eq_info_data) + 1);
    }
    mode = eq_info_data[0] & 0x7F;
    if (mode < EQ_MODE_CUSTOM) {
        eq_mode_set(mode);
    } else {
        // 自定义修改EQ参数
        if (EQ_MODE_CUSTOM == mode) {
            if (status != 0x7F)	{
                u8 i;
                for (i = 0; i < EQ_SECTION_MAX; i++) {
                    if (eq_info_data[0] >> 7) {
                        data = eq_info_data[i + 2];
                    } else {
                        data = eq_info_data[i + 1];
                    }
                    eq_mode_set_custom_param(i, (s8)data);
                }
            }
            eq_mode_set(mode);
        }
    }
}

void set_eq_setting(u8 *eq_setting)
{
    u8 i;
    u8 eq_setting_mode = eq_setting[0];
    if (eq_setting_mode >> 7) {
        // 多段eq
        for (i = 2; i < eq_setting[1] + 2; i++) {
            if ((eq_setting[i] > 12) && (eq_setting[i] < -12)) {
                eq_setting[i] = 0;
            }
        }
    } else {
        // 10段eq
        for (i = 1; i < 11; i++) {
            if ((eq_setting[i] > 12) && (eq_setting[i] < -12)) {
                eq_setting[i] = 0;
            }
        }
    }
    memcpy(_s_info.eq_mode_info, eq_setting, 11);
}

void get_eq_setting(u8 *eq_setting)
{
    memcpy(eq_setting, _s_info.eq_mode_info, 11);
}

// 1、写入VM
static void update_eq_vm_value(u8 *eq_setting)
{
    u8 status = *(((u8 *)eq_setting) + 1);

    syscfg_write(CFG_RCSP_ADV_EQ_MODE_SETTING, eq_setting, 1);

    /*自定义修改EQ参数*/
    if (EQ_MODE_CUSTOM == (eq_setting[0] & 0x7F)) {
        if (eq_setting[0] >> 7) {
            // 多段eq
            status = eq_setting[2];
        }
        if (status != 0x7F) {
            syscfg_write(CFG_RCSP_ADV_EQ_DATA_SETTING, &eq_setting[1], 10);
        }
    }

}
// 2、同步对端
static void eq_sync(u8 *eq_setting)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_EQ_SETTING));
    }
#endif
}

void deal_eq_setting(u8 *eq_setting, u8 write_vm, u8 tws_sync)
{
    u8 eq_info[11] = {0};
    if (!eq_setting) {
        get_eq_setting(eq_info);
    } else {
        u8 status = *(((u8 *)eq_setting) + 1);

        /*自定义修改EQ参数*/
        if (EQ_MODE_CUSTOM == (eq_setting[0] & 0x7F)) {
            if (eq_setting[0] >> 7) {
                status = eq_setting[2];
            }

            if (status != 0x7F) {
                memcpy(eq_info, eq_setting, 11);
                set_eq_setting(eq_info);
            } else {
                _s_info.eq_mode_info[0] = eq_setting[0];
                get_eq_setting(eq_info);
            }
        } else {
            memcpy(eq_info, eq_setting, 11);
            _s_info.eq_mode_info[0] = eq_setting[0];
        }

    }
    if (write_vm) {
        update_eq_vm_value(eq_info);
    }
    if (tws_sync) {
        eq_sync(eq_info);
    }
    eq_setting_info_deal(eq_info);
}


u8 app_get_eq_info(u8 *get_eq_info)
{
    u16 i;
    get_eq_info[0] = eq_mode_get_cur();
    u8 data_len = 1;

    if (10 == EQ_SECTION_MAX) {
        // 10段eq : mode + gain[10byte]
        for (i = 0; i < 10; i++) {
            get_eq_info[data_len] = eq_mode_get_gain(get_eq_info[0], i);
            data_len++;
        }
    } else {
        // 多段eq : mode + num + value(freq[2byte] + gain[1byte])
        get_eq_info[1] = EQ_SECTION_MAX;
        data_len++;
        for (i = 0; i < EQ_SECTION_MAX; i++) {
            get_eq_info[data_len] = eq_mode_get_gain(get_eq_info[0], i);
            data_len++;
        }
        get_eq_info[0] |= (1 << 7);
    }
    return data_len;
}

u8 app_get_eq_all_info(u8 *get_eq_info)
{
    u16 i;
    u8 mode = EQ_MODE_NORMAL;
    get_eq_info[0] = EQ_SECTION_MAX;
    u8 data_len = 1;
    // get freq
    for (i = 0; i < EQ_SECTION_MAX; i++) {
        u16 eq_freq = (u16)(eq_mode_get_freq(mode, i) & ((u16) - 1));
        eq_freq = ((((u8 *)&eq_freq)[0] << 8)  + ((u8 *)&eq_freq)[1]);
        memcpy(get_eq_info + data_len, &eq_freq, sizeof(eq_freq));
        data_len += 2;
    }
    // get gain
    for (; mode < EQ_MODE_CUSTOM; mode++) {
        get_eq_info[data_len] = mode;
        if (10 != EQ_SECTION_MAX) {
            get_eq_info[data_len] |= (1 << 7);
        }
        data_len++;
        for (i = 0; i < EQ_SECTION_MAX; i++) {
            u8 eq_gain = eq_mode_get_gain(mode, i);
            get_eq_info[data_len] = eq_gain;
            data_len++;
        }
    }
    // get custom
    get_eq_info[data_len] = EQ_MODE_CUSTOM;
    if (10 != EQ_SECTION_MAX) {
        get_eq_info[data_len] |= (1 << 7);
    }
    data_len++;
    for (i = 0; i < EQ_SECTION_MAX; i++) {
        if (10 == EQ_SECTION_MAX) {
            get_eq_info[data_len] = _s_info.eq_mode_info[i + 1];
        } else {
            get_eq_info[data_len] = _s_info.eq_mode_info[i + 2];
        }
        data_len++;
    }

    return data_len;
}


#endif

#endif
