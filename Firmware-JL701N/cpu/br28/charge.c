#include "asm/clock.h"
#include "timer.h"
#include "asm/power/p33.h"
#include "asm/charge.h"
#include "asm/adc_api.h"
#include "uart.h"
#include "device/device.h"
#include "asm/power_interface.h"
#include "asm/power/power_wakeup.h"
#include "system/event.h"
#include "asm/efuse.h"
#include "gpio.h"
#include "app_config.h"

#define LOG_TAG_CONST   CHARGE
#define LOG_TAG         "[CHARGE]"
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#include "debug.h"

typedef struct _CHARGE_VAR {
    struct charge_platform_data *data;
    volatile u8 charge_online_flag;
    volatile u8 charge_event_flag;
    volatile u8 init_ok;
    volatile u8 detect_stop;
    volatile int ldo5v_timer;   //检测LDOIN状态变化的usr timer
    volatile int charge_timer;  //检测充电是否充满的usr timer
    volatile int cc_timer;      //涓流切恒流的sys timer
} CHARGE_VAR;

#define __this 	(&charge_var)
static CHARGE_VAR charge_var;
static u8 charge_flag;

#define BIT_LDO5V_IN		BIT(0)
#define BIT_LDO5V_OFF		BIT(1)
#define BIT_LDO5V_KEEP		BIT(2)

u8 get_charge_poweron_en(void)
{
    return __this->data->charge_poweron_en;
}

void set_charge_poweron_en(u32 onOff)
{
    __this->data->charge_poweron_en = onOff;
}

void charge_check_and_set_pinr(u8 level)
{
    u8 pinr_io, reg;
    reg = P33_CON_GET(P3_PINR_CON1);
    //开启LDO5V_DET长按复位
    if ((reg & BIT(0)) && ((reg & BIT(3)) == 0)) {
        if (level == 0) {
            P33_CON_SET(P3_PINR_CON1, 2, 1, 0);
        } else {
            P33_CON_SET(P3_PINR_CON1, 2, 1, 1);
        }
    }
}

extern void udelay(u32 usec);
static u8 check_charge_state(void)
{
    u16 i, det_max_time;

#if TCFG_UMIDIGI_BOX_ENABLE
    det_max_time = TIMER2CMESSAGE * 16;
#else
    det_max_time = 20;
#endif

    __this->charge_online_flag = 0;

    for (i = 0; i < det_max_time; i++) {
        if (LVCMP_DET_GET() || LDO5V_DET_GET()) {
            __this->charge_online_flag = 1;
            break;
        }
        udelay(1000);
    }
    return __this->charge_online_flag;
}

void set_charge_online_flag(u8 flag)
{
    __this->charge_online_flag = flag;
}

u8 get_charge_online_flag(void)
{
    return __this->charge_online_flag;
}

void set_charge_event_flag(u8 flag)
{
    __this->charge_event_flag = flag;
}

u8 get_ldo5v_online_hw(void)
{
    return LDO5V_DET_GET();
}

u8 get_lvcmp_det(void)
{
    return LVCMP_DET_GET();
}

u8 get_ldo5v_pulldown_en(void)
{
    if (!__this->data) {
        return 0;
    }
    if (get_ldo5v_online_hw()) {
        if (__this->data->ldo5v_pulldown_keep == 0) {
            return 0;
        }
    }
    return __this->data->ldo5v_pulldown_en;
}

u8 get_ldo5v_pulldown_res(void)
{
    if (__this->data) {
        return __this->data->ldo5v_pulldown_lvl;
    }
    return CHARGE_PULLDOWN_200K;
}

void charge_event_to_user(u8 event)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg  = (void *)DEVICE_EVENT_FROM_CHARGE;
    e.u.dev.event = event;
    e.u.dev.value = 0;
    sys_event_notify(&e);
}

static void charge_cc_check(void *priv)
{
    if ((adc_get_voltage(AD_CH_VBAT) * 4 / 10) > CHARGE_CCVOL_V) {
        set_charge_mA(__this->data->charge_mA);
        usr_timer_del(__this->cc_timer);
        __this->cc_timer = 0;
        power_awakeup_gpio_enable(IO_CHGFL_DET, 1);
        charge_wakeup_isr();
    }
}

void charge_start(void)
{
    u8 check_full = 0;
    log_info("%s\n", __func__);

#if !TCFG_EXTERN_CHARGE_ENABLE
    if (__this->charge_timer) {
        usr_timer_del(__this->charge_timer);
        __this->charge_timer = 0;
    }

    //进入恒流充电之后,才开启充满检测
    if ((adc_get_voltage(AD_CH_VBAT) * 4 / 10) > CHARGE_CCVOL_V) {
        set_charge_mA(__this->data->charge_mA);
        power_awakeup_gpio_enable(IO_CHGFL_DET, 1);
        check_full = 1;
    } else {
        //涓流阶段系统不进入低功耗,防止电池电量更新过慢导致涓流切恒流时间过长
        set_charge_mA(__this->data->charge_trickle_mA);
        if (!__this->cc_timer) {
            __this->cc_timer = usr_timer_add(NULL, charge_cc_check, 1000, 1);
        }
    }

    CHARGE_EN(1);
    CHGGO_EN(1);

    charge_event_to_user(CHARGE_EVENT_CHARGE_START);

    //开启充电时,充满标记为1时,主动检测一次是否充满
    if (check_full && CHARGE_FULL_FLAG_GET()) {
        charge_wakeup_isr();
    }
#else

    /* 插入检测时使用vpwr 。Full_DET没充满时高阻，充满时低。 */
    //检测到开始充电，仅发送消息给其他app
    charge_event_to_user(CHARGE_EVENT_CHARGE_START);

    /* 初始化充满检测io */
    //设置为上拉输入
    gpio_direction_input(TCFG_EXTERN_CHARGE_PORT);
    gpio_set_pull_down(TCFG_EXTERN_CHARGE_PORT, 0);
    gpio_set_pull_up(TCFG_EXTERN_CHARGE_PORT, 1);
    gpio_set_die(TCFG_EXTERN_CHARGE_PORT, 1);

    if (0 == gpio_read(TCFG_EXTERN_CHARGE_PORT)) {
        log_info("ex_charge has been full\n");
        //检测到低电平,已经充满了的情况下触发充电,直接走充满流程
        charge_event_to_user(CHARGE_EVENT_CHARGE_FULL);
    }
    power_wakeup_enable_with_port(TCFG_EXTERN_CHARGE_PORT);
    //此时还有一个低电检测定时器在跑，如果需要省充电功耗可以直接关机，等到下降沿来了就会会触发开机进行处理流程。
    /* power_set_soft_poweroff(); */
#endif

}

void charge_close(void)
{
    log_info("%s\n", __func__);

#if !TCFG_EXTERN_CHARGE_ENABLE
    CHARGE_EN(0);
    CHGGO_EN(0);

    power_awakeup_gpio_enable(IO_CHGFL_DET, 0);

    charge_event_to_user(CHARGE_EVENT_CHARGE_CLOSE);

    if (__this->charge_timer) {
        usr_timer_del(__this->charge_timer);
        __this->charge_timer = 0;
    }
    if (__this->cc_timer) {
        usr_timer_del(__this->cc_timer);
        __this->cc_timer = 0;
    }
#else
    power_wakeup_disable_with_port(TCFG_EXTERN_CHARGE_PORT);
    //检测到开始充电，仅发送消息给其他app
    charge_event_to_user(CHARGE_EVENT_CHARGE_CLOSE);
#endif
}

static void charge_full_detect(void *priv)
{
    static u16 charge_full_cnt = 0;

    if (CHARGE_FULL_FILTER_GET()) {
        /* putchar('F'); */
        if (CHARGE_FULL_FLAG_GET() && LVCMP_DET_GET()) {
            /* putchar('1'); */
            if (charge_full_cnt < 10) {
                charge_full_cnt++;
            } else {
                charge_full_cnt = 0;
                power_awakeup_gpio_enable(IO_CHGFL_DET, 0);
                usr_timer_del(__this->charge_timer);
                __this->charge_timer = 0;
                charge_event_to_user(CHARGE_EVENT_CHARGE_FULL);
            }
        } else {
            /* putchar('0'); */
            charge_full_cnt = 0;
        }
    } else {
        /* putchar('K'); */
        charge_full_cnt = 0;
        usr_timer_del(__this->charge_timer);
        __this->charge_timer = 0;
    }
}

static void ldo5v_detect(void *priv)
{
    /* log_info("%s\n",__func__); */

    static u16 ldo5v_on_cnt = 0;
    static u16 ldo5v_keep_cnt = 0;
    static u16 ldo5v_off_cnt = 0;

    if (__this->detect_stop) {
        return;
    }

    if (LVCMP_DET_GET()) {	//ldoin > vbat
        /* putchar('X'); */
        if (ldo5v_on_cnt < __this->data->ldo5v_on_filter) {
            ldo5v_on_cnt++;
        } else {
            /* printf("ldo5V_IN\n"); */
            set_charge_online_flag(1);
            ldo5v_off_cnt = 0;
            ldo5v_keep_cnt = 0;
            //消息线程未准备好接收消息,继续扫描
            if (__this->charge_event_flag == 0) {
                return;
            }
            ldo5v_on_cnt = 0;
            usr_timer_del(__this->ldo5v_timer);
            __this->ldo5v_timer = 0;
            if ((charge_flag & BIT_LDO5V_IN) == 0) {
                charge_flag = BIT_LDO5V_IN;
                charge_event_to_user(CHARGE_EVENT_LDO5V_IN);
            }
        }
    } else if (LDO5V_DET_GET() == 0) {	//ldoin<拔出电压（0.6）
        /* putchar('Q'); */
        if (ldo5v_off_cnt < (__this->data->ldo5v_off_filter + 20)) {
            ldo5v_off_cnt++;
        } else {
            /* printf("ldo5V_OFF\n"); */
            set_charge_online_flag(0);
            ldo5v_on_cnt = 0;
            ldo5v_keep_cnt = 0;
            //消息线程未准备好接收消息,继续扫描
            if (__this->charge_event_flag == 0) {
                return;
            }
            ldo5v_off_cnt = 0;
            usr_timer_del(__this->ldo5v_timer);
            __this->ldo5v_timer = 0;
            if ((charge_flag & BIT_LDO5V_OFF) == 0) {
                charge_flag = BIT_LDO5V_OFF;
                charge_event_to_user(CHARGE_EVENT_LDO5V_OFF);
            }
        }
    } else {	//拔出电压（0.6左右）< ldoin < vbat
        /* putchar('E'); */
        if (ldo5v_keep_cnt < __this->data->ldo5v_keep_filter) {
            ldo5v_keep_cnt++;
        } else {
            /* printf("ldo5V_ERR\n"); */
            set_charge_online_flag(1);
            ldo5v_off_cnt = 0;
            ldo5v_on_cnt = 0;
            //消息线程未准备好接收消息,继续扫描
            if (__this->charge_event_flag == 0) {
                return;
            }
            ldo5v_keep_cnt = 0;
            usr_timer_del(__this->ldo5v_timer);
            __this->ldo5v_timer = 0;
            if ((charge_flag & BIT_LDO5V_KEEP) == 0) {
                charge_flag = BIT_LDO5V_KEEP;
                if (__this->data->ldo5v_off_filter) {
                    charge_event_to_user(CHARGE_EVENT_LDO5V_KEEP);
                }
            }
        }
    }
}

void ldoin_wakeup_isr(void)
{
    if (!__this->init_ok) {
        return;
    }
    /* printf(" %s \n", __func__); */
    if (__this->ldo5v_timer == 0) {
        __this->ldo5v_timer = usr_timer_add(0, ldo5v_detect, 2, 1);
    }
}

void charge_wakeup_isr(void)
{
    if (!__this->init_ok) {
        return;
    }
    /* printf(" %s \n", __func__); */
    if (__this->charge_timer == 0) {
        __this->charge_timer = usr_timer_add(0, charge_full_detect, 2, 1);
    }
}

void charge_set_ldo5v_detect_stop(u8 stop)
{
    __this->detect_stop = stop;
}

u8 get_charge_mA_config(void)
{
    return __this->data->charge_mA;
}

void set_charge_mA(u8 charge_mA)
{
    static u8 charge_mA_old = 0xff;
    if (charge_mA_old != charge_mA) {
        charge_mA_old = charge_mA;
        if (charge_mA & BIT(4)) {
            CHG_TRICKLE_EN(1);
        } else {
            CHG_TRICKLE_EN(0);
        }
        CHARGE_mA_SEL(charge_mA & 0x0F);
    }
}

const u16 full_table[CHARGE_FULL_V_MAX] = {
    4041, 4061, 4081, 4101, 4119, 4139, 4159, 4179,
    4199, 4219, 4238, 4258, 4278, 4298, 4318, 4338,
    4237, 4257, 4275, 4295, 4315, 4335, 4354, 4374,
    4394, 4414, 4434, 4454, 4474, 4494, 4514, 4534,
};
u16 get_charge_full_value(void)
{
    ASSERT(__this->init_ok, "charge not init ok!\n");
    ASSERT(__this->data->charge_full_V < CHARGE_FULL_V_MAX);
    return full_table[__this->data->charge_full_V];
}

static void charge_config(void)
{
    u8 charge_trim_val;
    u8 offset = 0;
    u8 charge_full_v_val = 0;
    //判断是用高压档还是低压档
    if (__this->data->charge_full_V < CHARGE_FULL_V_4237) {
        CHG_HV_MODE(0);
        charge_trim_val = get_vbat_trim();//4.20V对应的trim出来的实际档位
        if (charge_trim_val == 0xf) {
            log_info("vbat low not trim, use default config!!!!!!");
            charge_trim_val = CHARGE_FULL_V_4199;
        }
        log_info("low charge_trim_val = %d\n", charge_trim_val);
        if (__this->data->charge_full_V >= CHARGE_FULL_V_4199) {
            offset = __this->data->charge_full_V - CHARGE_FULL_V_4199;
            charge_full_v_val = charge_trim_val + offset;
            if (charge_full_v_val > 0xf) {
                charge_full_v_val = 0xf;
            }
        } else {
            offset = CHARGE_FULL_V_4199 - __this->data->charge_full_V;
            if (charge_trim_val >= offset) {
                charge_full_v_val = charge_trim_val - offset;
            } else {
                charge_full_v_val = 0;
            }
        }
    } else {
        CHG_HV_MODE(1);
        charge_trim_val = get_vbat_trim_435();//4.35V对应的trim出来的实际档位
        if (charge_trim_val == 0xf) {
            log_info("vbat high not trim, use default config!!!!!!");
            charge_trim_val = CHARGE_FULL_V_4354 - 16;
        }
        log_info("high charge_trim_val = %d\n", charge_trim_val);
        if (__this->data->charge_full_V >= CHARGE_FULL_V_4354) {
            offset = __this->data->charge_full_V - CHARGE_FULL_V_4354;
            charge_full_v_val = charge_trim_val + offset;
            if (charge_full_v_val > 0xf) {
                charge_full_v_val = 0xf;
            }
        } else {
            offset = CHARGE_FULL_V_4354 - __this->data->charge_full_V;
            if (charge_trim_val >= offset) {
                charge_full_v_val = charge_trim_val - offset;
            } else {
                charge_full_v_val = 0;
            }
        }
    }

    log_info("charge_full_v_val = %d\n", charge_full_v_val);

    CHARGE_FULL_V_SEL(charge_full_v_val);
    CHARGE_FULL_mA_SEL(__this->data->charge_full_mA);
    set_charge_mA(__this->data->charge_trickle_mA);
}

int charge_init(const struct dev_node *node, void *arg)
{
    log_info("%s\n", __func__);

    __this->data = (struct charge_platform_data *)arg;

    ASSERT(__this->data);

    __this->init_ok = 0;
    __this->charge_online_flag = 0;

    /*先关闭充电使能，后面检测到充电插入再开启*/
    power_awakeup_gpio_enable(IO_CHGFL_DET, 0);
    CHARGE_EN(0);
    CHGGO_EN(0);

    /*LDO5V的100K下拉电阻使能*/
    L5V_RES_DET_S_SEL(__this->data->ldo5v_pulldown_lvl);
    L5V_LOAD_EN(__this->data->ldo5v_pulldown_en);

    charge_config();

    if (check_charge_state()) {
        if (__this->ldo5v_timer == 0) {
            __this->ldo5v_timer = usr_timer_add(0, ldo5v_detect, 2, 1);
        }
    } else {
        charge_flag = BIT_LDO5V_OFF;
    }

    __this->init_ok = 1;

    return 0;
}

void charge_module_stop(void)
{
    if (!__this->init_ok) {
        return;
    }
    charge_close();
    power_awakeup_index_enable(IO_LDOIN_DET, 0);
    power_awakeup_index_enable(IO_VBTCH_DET, 0);
    if (__this->ldo5v_timer) {
        usr_timer_del(__this->ldo5v_timer);
        __this->ldo5v_timer = 0;
    }
}

void charge_module_restart(void)
{
    if (!__this->init_ok) {
        return;
    }
    if (!__this->ldo5v_timer) {
        __this->ldo5v_timer = usr_timer_add(NULL, ldo5v_detect, 2, 1);
    }
    power_awakeup_index_enable(IO_LDOIN_DET, 1);
    power_awakeup_index_enable(IO_VBTCH_DET, 1);
}

const struct device_operations charge_dev_ops = {
    .init  = charge_init,
};
