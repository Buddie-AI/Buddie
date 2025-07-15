#include "ui_manage.h"
#include "asm/pwm_led.h"
#include "system/includes.h"
#include "app_config.h"
#include "user_cfg.h"
#include "classic/tws_api.h"

#define LOG_TAG_CONST       UI
#define LOG_TAG             "[UI]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define LED_VDDIO_KEEP            1
#define UI_MANAGE_BUF             8

typedef struct _ui_var {
    u8 ui_init_flag;
    u8 other_status;
    u8 power_status;
    u8 current_status;
    volatile u8 ui_flash_cnt;
    cbuffer_t ui_cbuf;
    u8 ui_buf[UI_MANAGE_BUF];
    int sys_ui_timer;
} ui_var;

static ui_var sys_ui_var = {.power_status =  STATUS_NORMAL_POWER};

extern int get_bt_tws_connect_status();

u8   get_ui_status(u8 *status)
{
    return cbuf_read(&(sys_ui_var.ui_cbuf), status, 1);
}

u8   get_ui_status_len(void)
{
    return cbuf_get_data_size(&(sys_ui_var.ui_cbuf));
}

u8 get_ui_busy_status()
{
    return sys_ui_var.ui_flash_cnt;
}

#if (RCSP_ADV_EN)
u8 adv_get_led_status(void)
{
    return sys_ui_var.other_status;
}
#endif

void ui_manage_scan(void *priv)
{

    STATUS *p_led = get_led_config();

    sys_ui_var.sys_ui_timer = 0;

    log_info("ui_flash_cnt:%d cur_ui_status:%d", sys_ui_var.ui_flash_cnt, sys_ui_var.current_status);

    if (sys_ui_var.ui_flash_cnt == 0 || sys_ui_var.ui_flash_cnt == 7) {        //有特殊的闪烁状态等当前状态执行完再进入下一个状态
        if (get_ui_status(&sys_ui_var.current_status)) {
            if (sys_ui_var.current_status >= STATUS_CHARGE_START && sys_ui_var.current_status <= STATUS_NORMAL_POWER) {
                sys_ui_var.power_status = sys_ui_var.current_status;
            } else {
                sys_ui_var.other_status = sys_ui_var.current_status;
            }
        }
    }

    if (sys_ui_var.ui_flash_cnt) {
        sys_ui_var.ui_flash_cnt --;
        sys_ui_var.sys_ui_timer = usr_timeout_add(NULL, ui_manage_scan, 300, 1);
    } else if (get_ui_status_len()) {
        sys_ui_var.sys_ui_timer = usr_timeout_add(NULL, ui_manage_scan, 100, 1);
    }

#if ((RCSP_ADV_EN)&&(JL_EARPHONE_APP_EN))
    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        pwm_led_mode_set(PWM_LED_ALL_OFF);
        return;
    }
#endif

    if (sys_ui_var.other_status != STATUS_POWEROFF && sys_ui_var.power_status != STATUS_NORMAL_POWER) {  //关机的状态优先级要高于电源状态
        switch (sys_ui_var.power_status) {
        case STATUS_LOWPOWER:
            log_info("[STATUS_LOWPOWER]\n");
            pwm_led_mode_set(p_led->lowpower);
            return;

        case STATUS_CHARGE_START:
            log_info("[STATUS_CHARGE_START]\n");
            pwm_led_mode_set(p_led->charge_start);
            return;

        case STATUS_CHARGE_FULL:
            log_info("[STATUS_CHARGE_FULL]\n");
            pwm_led_mode_set(p_led->charge_full);
            return;

        case STATUS_CHARGE_ERR:
            log_info("[STATUS_CHARGE_ERR]\n");
            pwm_led_mode_set(PWM_LED0_ON);
            pwm_led_mode_set(PWM_LED1_ON);
            return;

        case STATUS_CHARGE_CLOSE:
            log_info("[STATUS_CHARGE_CLOSE]\n");
            pwm_led_mode_set(PWM_LED0_OFF);
            pwm_led_mode_set(PWM_LED1_OFF);
            return;

        case STATUS_CHARGE_LDO5V_OFF:
            log_info("[STATUS_CHARGE_LDO5V_OFF]\n");
        case STATUS_EXIT_LOWPOWER:
        case STATUS_NORMAL_POWER:
            //pwm_led_mode_set(PWM_LED0_OFF);
            //pwm_led_mode_set(PWM_LED1_OFF);
            sys_ui_var.power_status = STATUS_NORMAL_POWER;
            break;
        }
    }

    switch (sys_ui_var.other_status) {
    case STATUS_POWERON:
        log_info("[STATUS_POWERON]\n");
        if (p_led->power_on != PWM_LED0_FLASH_THREE) {
            pwm_led_mode_set(p_led->power_on);
        } else {
            if (sys_ui_var.ui_flash_cnt) {
                if (get_bt_tws_connect_status()) {
                    sys_ui_var.ui_flash_cnt = 0;
                    ui_manage_scan(NULL);
                    return;
                }
                if (sys_ui_var.ui_flash_cnt % 2) {
                    pwm_led_mode_set(PWM_LED0_OFF);
                } else {
                    pwm_led_mode_set(PWM_LED0_ON);
                }
            }
        }
        break;
    case STATUS_POWEROFF:
        log_info("[STATUS_POWEROFF]\n");
        if (p_led->power_off != PWM_LED1_FLASH_THREE) {
            pwm_led_mode_set(p_led->power_off);
        } else {
            if (sys_ui_var.ui_flash_cnt) {
                if (sys_ui_var.ui_flash_cnt % 2) {
                    pwm_led_mode_set(PWM_LED1_OFF);
                } else {
                    pwm_led_mode_set(PWM_LED1_ON);
                }
            }
        }
        break;

    case STATUS_BT_INIT_OK:
        log_info("[STATUS_BT_INIT_OK]\n");
        pwm_led_mode_set(p_led->bt_init_ok);
        break;

    case STATUS_BT_SLAVE_CONN_MASTER:
        pwm_led_mode_set(PWM_LED1_SLOW_FLASH);
        break;

    case STATUS_BT_CONN:
        log_info("[STATUS_BT_CONN]\n");
        pwm_led_mode_set(p_led->bt_connect_ok);
        break;

    case STATUS_BT_MASTER_CONN_ONE:
        log_info("[STATUS_BT_MASTER_CONN_ONE]\n");
        pwm_led_mode_set(PWM_LED0_LED1_SLOW_FLASH);
        break;

    case STATUS_BT_DISCONN:
        log_info("[STATUS_BT_DISCONN]\n");
        pwm_led_mode_set(p_led->bt_disconnect);
        break;

    case STATUS_PHONE_INCOME:
        log_info("[STATUS_PHONE_INCOME]\n");
        pwm_led_mode_set(p_led->phone_in);
        break;

    case STATUS_PHONE_OUT:
        log_info("[STATUS_PHONE_OUT]\n");
        pwm_led_mode_set(p_led->phone_out);
        break;

    case STATUS_PHONE_ACTIV:
        log_info("[STATUS_PHONE_ACTIV]\n");
        pwm_led_mode_set(p_led->phone_activ);
        break;

    case STATUS_POWERON_LOWPOWER:
        log_info("[STATUS_POWERON_LOWPOWER]\n");
        pwm_led_mode_set(p_led->lowpower);
        break;

    case STATUS_BT_TWS_CONN:
        log_info("[STATUS_BT_TWS_CONN]\n");
        pwm_led_mode_set(p_led->tws_connect_ok);
        break;
    case STATUS_BT_TWS_DISCONN:
        log_info("[STATUS_BT_TWS_DISCONN]\n");
        pwm_led_mode_set(p_led->tws_disconnect);
        break;
    }
}

int  ui_manage_init(void)
{
    if (!sys_ui_var.ui_init_flag) {
        cbuf_init(&(sys_ui_var.ui_cbuf), &(sys_ui_var.ui_buf), UI_MANAGE_BUF);
        sys_ui_var.ui_init_flag = 1;
    }
    return 0;
}

void ui_update_status(u8 status)
{
    STATUS *p_led = get_led_config();
    if (!sys_ui_var.ui_init_flag) {    //更新UI状态之前需先初始化ui_cbuf
        ui_manage_init();
    }
    log_info("update ui status :%d", status);
    if ((status == STATUS_POWERON && p_led->power_on == PWM_LED0_FLASH_THREE) || (status == STATUS_POWEROFF && p_led->power_off == PWM_LED1_FLASH_THREE)) {
        log_info(">>>set ui_flash_cnt");
        sys_ui_var.ui_flash_cnt = 7;
    }
    cbuf_write(&(sys_ui_var.ui_cbuf), &status, 1);
    /* if (!sys_ui_var.sys_ui_timer) { */
    /*     sys_ui_var.sys_ui_timer = usr_timeout_add(NULL, ui_manage_scan, 10); */
    /* } */
    if (sys_ui_var.ui_flash_cnt >= 1 && sys_ui_var.ui_flash_cnt <= 6) {
        return;
    }

    ui_manage_scan(NULL);
}

u8 pdown_keep_pw_gate()
{
#if(LED_VDDIO_KEEP == 1)        //led 进入powerdown时模块的电源是否降低，降低会导致灯抖动，不降的话功耗会高100ua
    return true;
#else
    if ((sys_ui_var.current_status == STATUS_BT_DISCONN) ||
        (sys_ui_var.current_status == STATUS_BT_INIT_OK) ||
        (sys_ui_var.current_status == STATUS_BT_TWS_DISCONN)) {
        return true;
    } else {
        return false;
    }
#endif
}

#if (TCFG_USER_TWS_ENABLE == 1)
__attribute__((weak))
bool is_led_module_on()
{
    return 0;
}

__attribute__((weak))
void led_module_on()
{

}

__attribute__((weak))
void led_module_off()
{

}

__attribute__((weak))
void led_timer_del()
{

}

__attribute__((weak))
void led_module_enter_sniff_mode()
{

}

__attribute__((weak))
void led_module_exit_sniff_mode()
{

}

__attribute__((weak))
void led_sniff_mode_double_display(u32 index)
{

}

extern int get_bt_tws_connect_status();
//对耳连接成功之后利用SNIFF的节奏来进行闪灯同步,只是通过开关LED模块来实现同步
void sniff_achor_point_hook(u32 slot)
{
    u8 led_mode = pwm_led_display_mode_get();
    if (led_mode != PWM_LED0_ONE_FLASH_5S && led_mode != PWM_LED1_ONE_FLASH_5S) {
        if (get_bt_tws_connect_status()) {
            //交替闪进sniff同步
            if (led_mode == PWM_LED0_LED1_FAST_FLASH) {
                if ((slot % ((CFG_DOUBLE_FAST_FLASH_FREQ * 1000) / 625)) < 800) {
                    led_sniff_mode_double_display((slot / ((CFG_DOUBLE_FAST_FLASH_FREQ * 1000) / 625)) & BIT(0));
                }
            } else if (led_mode == PWM_LED0_LED1_SLOW_FLASH) {
                if ((slot % ((CFG_DOUBLE_SLOW_FLASH_FREQ * 1000) / 625)) < 800) {
                    led_sniff_mode_double_display((slot / ((CFG_DOUBLE_SLOW_FLASH_FREQ * 1000) / 625)) & BIT(0));
                }
            }
        }
        return;
    }
    /* if (get_bt_tws_connect_status()) { */
#if defined CFG_LED_MODE && (CFG_LED_MODE == CFG_LED_SOFT_MODE)
    led_timer_del();
#endif
    if (is_led_module_on()) {
        led_module_off();
    }
    if ((slot % (1600 * 5)) < 800) {    //一个slot = 0.625ms 1600*0.625 = 1s
        led_module_on();
    }
    /* } else { */

    /* } */
}
#else

__attribute__((weak))
int get_bt_tws_connect_status()
{
    return 0;
}

#endif

