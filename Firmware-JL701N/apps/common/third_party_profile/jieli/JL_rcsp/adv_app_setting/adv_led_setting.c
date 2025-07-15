#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "asm/pwm_led.h"
#include "user_cfg.h"
#include "ui_manage.h"
#include "le_rcsp_adv_module.h"

#include "adv_led_setting.h"
#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"

#if (RCSP_ADV_EN)
#if RCSP_ADV_LED_SET_ENABLE

enum ADV_LED_TYPE {
    ADV_LED_TYPE_ALL_OFF = 0x0,
    ADV_LED_TYPE_RED_ON,
    ADV_LED_TYPE_BLUE_ON,
    ADV_LED_TYPE_RAD_BREATHE,
    ADV_LED_TYPE_BLUE_BREATHE,
    ADV_LED_TYPE_FAST_FLASH,
    ADV_LED_TYPE_SLOW_FLASH,
};

extern STATUS *get_led_config(void);
extern int get_bt_tws_connect_status();


__attribute__((weak))
u8 adv_get_led_status(void)
{
    return 1;
}

static u8 adv_led_value_conversion(u8 adv_led_opt)
{
    u8 conversion_value = adv_led_opt;
    switch (adv_led_opt) {
    case ADV_LED_TYPE_ALL_OFF:
        conversion_value = PWM_LED_ALL_OFF;
        break;
    case ADV_LED_TYPE_RED_ON:
        conversion_value = PWM_LED1_ON;
        break;
    case ADV_LED_TYPE_BLUE_ON:
        conversion_value = PWM_LED0_ON;
        break;
    case ADV_LED_TYPE_RAD_BREATHE:
        conversion_value = PWM_LED1_BREATHE;
        break;
    case ADV_LED_TYPE_BLUE_BREATHE:
        conversion_value = PWM_LED0_BREATHE;
        break;
    case ADV_LED_TYPE_FAST_FLASH:
        conversion_value = PWM_LED0_LED1_FAST_FLASH;
        break;
    case ADV_LED_TYPE_SLOW_FLASH:
        conversion_value = PWM_LED0_LED1_SLOW_FLASH;
        break;
    }
    return conversion_value;
}

void set_led_setting(u8 *led_setting_info)
{
    _s_info.led_status[2 * 0 + 1] = led_setting_info[0];
    _s_info.led_status[2 * 1 + 1] = led_setting_info[1];
    _s_info.led_status[2 * 2 + 1] = led_setting_info[2];
}

void get_led_setting(u8 *led_setting_info)
{
    led_setting_info[0] = _s_info.led_status[2 * 0 + 1];
    led_setting_info[1] = _s_info.led_status[2 * 1 + 1];
    led_setting_info[2] = _s_info.led_status[2 * 2 + 1];
}

// 1、写入VM
static void update_led_setting_vm_value(u8 *led_setting_info)
{
    syscfg_write(CFG_RCSP_ADV_LED_SETTING, led_setting_info, 3);
}
// 2、同步对端
static void led_setting_sync(u8 *led_setting_info)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_adv_setting(BIT(ATTR_TYPE_LED_SETTING));
    }
#endif
}
// 3、更新状态
void update_led_setting_state(void)
{
    u8 led_setting_info[3];
    get_led_setting(led_setting_info);

    // 直接修改led表
    STATUS *p_led = get_led_config();
    // 未配对
    p_led->bt_init_ok = adv_led_value_conversion(led_setting_info[0]);
    p_led->bt_disconnect = adv_led_value_conversion(led_setting_info[0]);
    // 未连接
    p_led->tws_connect_ok = adv_led_value_conversion(led_setting_info[1]);
    // 连接
    p_led->bt_connect_ok = adv_led_value_conversion(led_setting_info[2]);

    switch (adv_get_led_status()) {
    case STATUS_BT_INIT_OK:
    case STATUS_BT_DISCONN:
        ui_update_status(STATUS_BT_INIT_OK);
        break;
    case STATUS_BT_TWS_CONN:
        ui_update_status(STATUS_BT_TWS_CONN);
        break;
    case STATUS_PHONE_ACTIV:
    case STATUS_PHONE_OUT:
    case STATUS_PHONE_INCOME:
    case STATUS_BT_CONN:
        ui_update_status(STATUS_BT_CONN);
        break;
    }
}

void deal_led_setting(u8 *led_setting_info, u8 write_vm, u8 tws_sync)
{
    u8 led_setting[3];
    if (!led_setting_info) {
        get_led_setting(led_setting);
    } else {
        memcpy(led_setting, led_setting_info, 3);
    }
    if (write_vm) {
        update_led_setting_vm_value(led_setting);
    }
    if (tws_sync) {
        led_setting_sync(led_setting);
    }
    update_led_setting_state();
}

#endif
#endif
