//*********************************************************************************//
//                                NTC det 									       //
//*********************************************************************************//
#include "ntc_det_api.h"
#include "asm/power/p33.h"
#include "system/includes.h"
#include "asm/charge.h"

#if NTC_DET_EN

#define NTC_DET_BAD_RES      0    //分压电阻损坏关闭检测

#ifndef NTC_DET_DUTY1
#define NTC_DET_DUTY1        5000 //检测周期
#endif

#ifndef NTC_DET_DUTY2
#define NTC_DET_DUTY2        10   //检测小周期
#endif

#ifndef NTC_DET_CNT
#define NTC_DET_CNT          3    //检测次数
#endif

#ifndef NTC_DET_UPPER
#define NTC_DET_UPPER        235  //正常范围AD值上限，0度时
#endif

#ifndef NTC_DET_LOWER
#define NTC_DET_LOWER        34  //正常范围AD值下限，45度时
#endif

#define NTC_IS_NORMAL(value, offset) (value >= NTC_DET_LOWER+(offset) && value <= NTC_DET_UPPER-(offset))
#define NTC_IS_BAD_RES(value) (value >= 1020 || value <= 5)

enum {
    NTC_STATE_NORMAL = 0,
    NTC_STATE_ABNORMAL,
};

struct ntc_det_t {
    u16 normal_cnt : 4; //温度正常的次数
    u16 cnt : 4; //温度检测的次数
    u16 res_cnt : 4; //分压电阻脱落或损坏
    u16 state : 1; //是否超出范围
    u16 timer;
};
static struct ntc_det_t ntc_det = {0};
extern u8 get_charge_full_flag(void);

u16 ntc_det_working()
{
    return ntc_det.timer;
}

static void ntc_det_timer_deal(void *priv)
{
    u32 value;

#if NTC_DET_CNT
    if (ntc_det.cnt == 0) {
        sys_timer_modify(ntc_det.timer, NTC_DET_DUTY2);
    }
#endif
    value = adc_get_value(NTC_DET_AD_CH);
    printf("%d", value);

    ntc_det.cnt++;
    if (NTC_IS_NORMAL(value, ntc_det.state * 8)) { //温度恢复一定范围后才算正常，防止临界状态
        ntc_det.normal_cnt++;
    } else if (NTC_IS_BAD_RES(value)) {
        ntc_det.res_cnt++;
    }
    if (ntc_det.cnt >= NTC_DET_CNT) {
        if (ntc_det.normal_cnt > NTC_DET_CNT / 2) {
            if (ntc_det.state == NTC_STATE_ABNORMAL) {
                printf("temperature recover, start charge");
                ntc_det.state = NTC_STATE_NORMAL;
                charge_start();
            }
        }
#if NTC_DET_BAD_RES
        else if (ntc_det.res_cnt > NTC_DET_CNT / 2) {
            printf("bad res, stop det");
            ntc_det_stop();
        }
#endif
        else {
            if (ntc_det.state == NTC_STATE_NORMAL) {
                printf("temperature is abnormall, stop charge");
                ntc_det.state = NTC_STATE_ABNORMAL;
                charge_close();
                CHARGE_EN(0);
            }
            /* power_set_soft_poweroff(); */
        }
        ntc_det.cnt = 0;
        ntc_det.res_cnt = 0;
        ntc_det.normal_cnt = 0;
        sys_timer_modify(ntc_det.timer, NTC_DET_DUTY1);
    }
}

void ntc_det_start(void)
{
    if (ntc_det.timer == 0) {
        printf("ntc det start");
        memset(&ntc_det, 0, sizeof(ntc_det));
        gpio_direction_output(NTC_POWER_IO, 1);

        gpio_set_pull_up(NTC_DETECT_IO, 0);
        gpio_set_pull_down(NTC_DETECT_IO, 0);
        gpio_set_die(NTC_DETECT_IO, 0);
        gpio_set_direction(NTC_DETECT_IO, 1);
        adc_add_sample_ch(NTC_DET_AD_CH);
        ntc_det.timer = sys_timer_add(NULL, ntc_det_timer_deal, NTC_DET_DUTY1);
    }
}

void ntc_det_stop(void)
{
    if (!get_charge_full_flag() && get_charge_online_flag() && ntc_det.state == NTC_STATE_ABNORMAL) {
        printf("charge protecting, wait recover");
        return;
    }
    if (ntc_det.timer) {
        printf("ntc det stop");
        sys_timer_del(ntc_det.timer);
        ntc_det.timer = 0;
        adc_remove_sample_ch(NTC_DET_AD_CH);
        gpio_set_pull_up(NTC_POWER_IO, 0);
        gpio_set_pull_down(NTC_POWER_IO, 0);
        gpio_set_die(NTC_POWER_IO, 0);
        gpio_set_direction(NTC_POWER_IO, 1);
    }
}
#endif

