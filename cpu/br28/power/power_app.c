#include "asm/power_interface.h"
#include "app_config.h"
#include "includes.h"


const char BTOSC_FASTBOOT_EN  = 0;


#if (TCFG_LOWPOWER_LOWPOWER_SEL == SLEEP_EN)


static enum LOW_POWER_LEVEL power_app_level(void)
{
    return LOW_POWER_MODE_SLEEP;
}

static u8 power_app_idle(void)
{
    return 1;
}

REGISTER_LP_TARGET(power_app_lp_target) = {
    .name = "power_app",
    .level = power_app_level,
    .is_idle = power_app_idle,
};

#endif
