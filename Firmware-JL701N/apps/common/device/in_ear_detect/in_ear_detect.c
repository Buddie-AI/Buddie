#include "in_ear_detect/in_ear_manage.h"
#include "in_ear_detect/in_ear_detect.h"

#include "system/includes.h"
#include "app_config.h"

#define LOG_TAG_CONST       EAR_DETECT
#define LOG_TAG             "[EAR_DETECT]"
#define LOG_INFO_ENABLE
#include "debug.h"

//#define log_info(format, ...)       y_printf("[EAR_DETECT] : " format "\r\n", ## __VA_ARGS__)

#if TCFG_EAR_DETECT_ENABLE

static struct ear_detect_t _ear_detect_t = {
    .is_idle = 1,
    .in_cnt = 0,
    .out_cnt = 0,
    .s_hi_timer = 0,
    .check_status = DETECT_IDLE,
};

u8 io_key_filter_flag = 0;
#define __this 	(&_ear_detect_t)

//*********************************************************************************//
//                               IR Detect                                        //
//*********************************************************************************//
static void __ear_detect_ir_init(void)
{
    log_info("%s\n", __func__);
    //IR VDD
    if (TCFG_EAR_DET_IR_POWER_IO != NO_CONFIG_PORT) {
        gpio_set_pull_up(TCFG_EAR_DET_IR_POWER_IO, 0);
        gpio_set_pull_down(TCFG_EAR_DET_IR_POWER_IO, 0);
        gpio_set_die(TCFG_EAR_DET_IR_POWER_IO, 1);
        gpio_set_direction(TCFG_EAR_DET_IR_POWER_IO, 0);
        gpio_set_hd0(TCFG_EAR_DET_IR_POWER_IO, 1);
        gpio_set_hd(TCFG_EAR_DET_IR_POWER_IO, 1);
        gpio_set_output_value(TCFG_EAR_DET_IR_POWER_IO, 1);
    }


    //ir i01 init
    gpio_set_pull_up(TCFG_EAR_DETECT_IRO1, 0);
    gpio_set_pull_down(TCFG_EAR_DETECT_IRO1, 0);
    gpio_set_direction(TCFG_EAR_DETECT_IRO1, 0);
    gpio_set_output_value(TCFG_EAR_DETECT_IRO1, !TCFG_EAR_DETECT_IRO1_LEVEL);

    //ir i02 init
#if TCFG_EAR_DETECT_IR_MODE
    log_info("ear_detect_ir_ad_mode\n");
    adc_add_sample_ch(TCFG_EAR_DETECT_AD_CH);
    gpio_set_pull_up(TCFG_EAR_DETECT_IRO2, !TCFG_EAR_DETECT_DET_LEVEL);
    gpio_set_pull_down(TCFG_EAR_DETECT_IRO2, TCFG_EAR_DETECT_DET_LEVEL);
    gpio_set_die(TCFG_EAR_DETECT_IRO2, 0);
    gpio_set_direction(TCFG_EAR_DETECT_IRO2, 1);
#else
    log_info("ear_detect_ir_io_mode\n");
    gpio_set_pull_up(TCFG_EAR_DETECT_IRO2, !TCFG_EAR_DETECT_DET_LEVEL);
    gpio_set_pull_down(TCFG_EAR_DETECT_IRO2, TCFG_EAR_DETECT_DET_LEVEL);
    gpio_set_die(TCFG_EAR_DETECT_IRO2, 1);
    gpio_set_direction(TCFG_EAR_DETECT_IRO2, 1);
#endif
}

extern u8 get_charge_online_flag(void);
static void __ear_detect_ir_run(void *priv)
{
    if (get_charge_online_flag()) {
        __this->check_status = DETECT_IDLE;
        gpio_set_output_value(TCFG_EAR_DETECT_IRO1, !TCFG_EAR_DETECT_IRO1_LEVEL);
        sys_hi_timer_modify(__this->s_hi_timer, __this->cfg->ear_det_ir_disable_time);
        //ear_detect_change_state_to_event(!TCFG_EAR_DETECT_DET_LEVEL);
        return;
    }

    if (__this->check_status == DETECT_IDLE) {
        //read det_level without enable power_port,maybe under sun
        if (__this->cfg->ear_det_ir_compensation_en == 1) {
            if (gpio_read(TCFG_EAR_DETECT_IRO2) == TCFG_EAR_DETECT_DET_LEVEL) {
                //putchar('L');
                if (is_ear_detect_state_in() == 1) {
                    ear_detect_change_state_to_event(!TCFG_EAR_DETECT_DET_LEVEL);
                    __this->in_cnt = 0;
                    __this->is_idle = 1;
                }
                return;
            }
        }

        __this->check_status = DETECT_CHECKING;
        gpio_set_output_value(TCFG_EAR_DETECT_IRO1, TCFG_EAR_DETECT_IRO1_LEVEL);

        sys_hi_timer_modify(__this->s_hi_timer, __this->cfg->ear_det_ir_enable_time);
        return;
    }
#if (!TCFG_EAR_DETECT_IR_MODE)
    if (gpio_read(TCFG_EAR_DETECT_IRO2) == TCFG_EAR_DETECT_DET_LEVEL) { //入耳
#else
    u32 ear_ad = adc_get_value(TCFG_EAR_DETECT_AD_CH);
    if (ear_ad >= TCFG_EAR_DETECT_AD_VALUE) {
#endif
        //putchar('i');
        __this->out_cnt = 0;
        if (__this->in_cnt < __this->cfg->ear_det_in_cnt) {
            __this->is_idle = 0; //过滤期间不进入sniff
            __this->in_cnt++;
            if (__this->in_cnt == __this->cfg->ear_det_in_cnt) {
                log_info("earphone ir in\n");
#if TCFG_KEY_IN_EAR_FILTER_ENABLE
                if (gpio_read(IO_PORTB_01) == 0) {
                    io_key_filter_flag = 1;
                } else {
                    io_key_filter_flag = 0;
                }
#endif
                __this->is_idle = 1;
                ear_detect_change_state_to_event(TCFG_EAR_DETECT_DET_LEVEL);
            }
        }
    } else {
        //putchar('o');
        __this->in_cnt = 0;
        if (__this->out_cnt < __this->cfg->ear_det_out_cnt) {
            __this->is_idle = 0; //过滤期间不进入sniff
            __this->out_cnt++;
            if (__this->out_cnt == __this->cfg->ear_det_out_cnt) {
                log_info("earphone ir out\n");
                __this->is_idle = 1;
                ear_detect_change_state_to_event(!TCFG_EAR_DETECT_DET_LEVEL);
            }
        }
    }
    __this->check_status = DETECT_IDLE;
    gpio_set_output_value(TCFG_EAR_DETECT_IRO1, !TCFG_EAR_DETECT_IRO1_LEVEL);
    sys_hi_timer_modify(__this->s_hi_timer, __this->cfg->ear_det_ir_disable_time);
}

//*********************************************************************************//
//                               TCH Detect                                        //
//*********************************************************************************//
void ear_detect_tch_wakeup_init(void)
{
    log_info("%s\n", __func__);

    gpio_set_direction(TCFG_EAR_DETECT_DET_IO, 1);
    gpio_set_die(TCFG_EAR_DETECT_DET_IO, 1);
    gpio_set_pull_up(TCFG_EAR_DETECT_DET_IO, 1);
    gpio_set_pull_down(TCFG_EAR_DETECT_DET_IO, 0);
}

void __ear_detect_tch_run(void *priv)
{
    if (__this->check_status == DETECT_IDLE) {
        __this->check_status = DETECT_CHECKING;
        gpio_set_pull_up(TCFG_EAR_DETECT_DET_IO, 1);
        gpio_set_pull_down(TCFG_EAR_DETECT_DET_IO, 0);
        gpio_set_die(TCFG_EAR_DETECT_DET_IO, 1);
        gpio_set_direction(TCFG_EAR_DETECT_DET_IO, 1);
        sys_hi_timer_modify(__this->s_hi_timer, 2);
        return;
    }
    if (gpio_read(TCFG_EAR_DETECT_DET_IO) == TCFG_EAR_DETECT_DET_LEVEL) { //入耳
        __this->out_cnt = 0;
        if (__this->in_cnt < __this->cfg->ear_det_in_cnt) {
            //putchar('i');
            __this->is_idle = 0; //过滤期间不进入idle
            __this->in_cnt++;
            if (__this->in_cnt == __this->cfg->ear_det_in_cnt) {
                log_info("earphone touch in\n");
#if TCFG_KEY_IN_EAR_FILTER_ENABLE
                if (gpio_read(IO_PORTB_01) == 0) {
                    io_key_filter_flag = 1;
                } else {
                    io_key_filter_flag = 0;
                }
#endif
                __this->is_idle = 1;
                ear_detect_change_state_to_event(TCFG_EAR_DETECT_DET_LEVEL);
            }
        }
    } else {
        __this->in_cnt = 0;
        if (__this->out_cnt < __this->cfg->ear_det_out_cnt) {
            //putchar('o');
            __this->is_idle = 0; //过滤期间不进入idle
            __this->out_cnt++;
            if (__this->out_cnt == __this->cfg->ear_det_out_cnt) {
                log_info("earphone touch out\n");
                __this->is_idle = 1;
                ear_detect_change_state_to_event(!TCFG_EAR_DETECT_DET_LEVEL);
            }
        }
    }
    __this->check_status = DETECT_IDLE;
    gpio_set_pull_up(TCFG_EAR_DETECT_DET_IO, 0);
    gpio_set_pull_down(TCFG_EAR_DETECT_DET_IO, 0);
    gpio_set_die(TCFG_EAR_DETECT_DET_IO, 0);
    gpio_set_direction(TCFG_EAR_DETECT_DET_IO, 1);
    sys_hi_timer_modify(__this->s_hi_timer, 10);
}


//*********************************************************************************//
//                               Detect                                        //
//*********************************************************************************//

static u8 ear_det_idle()
{
    return __this->is_idle;
}

REGISTER_LP_TARGET(ear_detect_lp_target) = {
    .name = "ear_det",
    .is_idle = ear_det_idle,
};

#if TCFG_KEY_IN_EAR_FILTER_ENABLE
u8 iokey_filter_hook(u8 io_state)
{
    u8 ret = 0;
    if (io_key_filter_flag) {
        if (io_state == 1) {
            io_key_filter_flag = 0;
        } else {
            ret = 1;
        }
    }
    return ret;
}
#endif

#if TCFG_EAR_DETECT_CTL_KEY
#if TCFG_IOKEY_ENABLE
int key_event_remap(struct sys_event *e)
#elif TCFG_LP_TOUCH_KEY_ENABLE
int lp_touch_key_event_remap(struct sys_event *e)
#endif
{
    if (ear_detect_get_key_delay_able() == 0) {
        log_info("key disable ");
        return 0;
    }
    if (!is_ear_detect_state_in()) { //耳机不在耳朵上，不发
        log_info("key remap");
        return 0;
    }
    return 1;
}
#endif

void ear_detect_ir_init(const struct ear_detect_platform_data *cfg)
{
    log_info("%s\n", __func__);
    ASSERT(cfg);
    __this->cfg = cfg;
    __ear_detect_ir_init();
    __this->s_hi_timer = sys_s_hi_timer_add(NULL, __ear_detect_ir_run, 10);
}

void ear_detect_tch_init(const struct ear_detect_platform_data *cfg)
{
    log_info("%s\n", __func__);
    ASSERT(cfg);
    __this->cfg = cfg;
#if TCFG_EAR_DETECT_TOUCH_MODE
    __this->s_hi_timer = sys_s_hi_timer_add(NULL, __ear_detect_tch_run, 10);
#else
    ear_touch_edge_wakeup_handle(0, TCFG_EAR_DETECT_DET_IO); //init first tch sta
#endif
}
#endif

