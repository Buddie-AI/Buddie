#include "typedef.h"
#include "asm/clock.h"
#include "timer.h"
#include "asm/efuse.h"
#include "irq.h"
#include "asm/power_interface.h"
#include "jiffies.h"
#include "app_config.h"
#include "syscfg_id.h"
#include "system/includes.h"


//*******************************************
// ****************dtemp pll trim************
//*******************************************


#define     APP_IO_DEBUG_0(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     APP_IO_DEBUG_1(i,x)       //{JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}


extern void get_bta_pll_midbank_temp(void);   //ok
extern const int config_bt_temperature_pll_trim;
static u16 trim_timer_handle = 0;
static u8 is_pll_trim_active = 0;
#define CHECK_TEMPERATURE_CYCLE_INIT                  (1000 * 8)   //检测周期
/* #define CHECK_TEMPERATURE_CYCLE_CONNECTED             (1000 * 5 )    //连接蓝牙检测周期 */

extern u8 btpll_status_det();

extern u8 le_hw_check_all_instant_will_be_valid();


static void btpll_trim_main()
{
    printf("midbank_trim>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
    local_irq_disable();
    is_pll_trim_active = 1;
    local_irq_enable();
    /* APP_IO_DEBUG_1(B,0); */

    get_bta_pll_midbank_temp();

    /* APP_IO_DEBUG_0(B,0); */
    local_irq_disable();
    is_pll_trim_active = 0;
    local_irq_enable();

}

void temperature_plltrim_handle()
{
    int msg[3] = {0};
    msg[0] = (int)btpll_trim_main;
    msg[1] = 1;
    msg[2] = 0;
    int ret = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);

}

void trim_process()
{
    if (!le_hw_check_all_instant_will_be_valid()) {
        if (btpll_status_det()) {
            temperature_plltrim_handle();
        }
    }
}


void trim_timer_add()
{
    if (config_bt_temperature_pll_trim) {
        if (!trim_timer_handle) {
            trim_timer_handle = sys_s_hi_timer_add(NULL, trim_process, CHECK_TEMPERATURE_CYCLE_INIT);
        }
    }
}


static u8 pll_trim_idle_query(void)
{
    return (!is_pll_trim_active);
}

REGISTER_LP_TARGET(plll_trim_lp_target) = {
    .name = "pll_trim",
    .is_idle = pll_trim_idle_query,
};


__initcall(trim_timer_add);
