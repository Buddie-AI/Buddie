/**@file  		power_check.c
* @brief        低功耗检查点
* @details		HW API
* @author		JL
* @date     	2021-11-30
* @version  	V1.0
* @copyright    Copyright:(c)JIELI  2011-2020  @ , All Rights Reserved.
 */

#include "asm/power_interface.h"
#include "app_config.h"
#include "includes.h"
#include "asm/clock_hw.h"

#pragma bss_seg(".volatile_ram_code")
#pragma data_seg(".volatile_ram_code")
#pragma code_seg(".volatile_ram_code")
#pragma const_seg(".volatile_ram_code")
#pragma str_literal_override(".volatile_ram_code")

void pmut_put_u8hex(u8 dat);
void pmu_put_u16hex(u16 dat);
void pmu_put_u32hex(u32 dat);
void pmu_put_u64hex(u64 dat);
void pmu_put_buf(const u8 *buf, int len);

AT_VOLATILE_RAM_CODE_POWER
u8 sleep_safety_check(void)
{
#if 0
    u32 get_rch_freq();
    JL_PORTA->DIR &= ~BIT(5);
    JL_OMAP->PA5_OUT = FO_UART0_TX;
    MAIN_CLOCK_SEL(MAIN_CLOCK_IN_RC);
    HSB_CLK_DIV(0);
    LSB_CLK_DIV(0);
    UART_CLOCK_IN(UART_CLOCK_IN_LSB);

    JL_UART0->BAUD = (get_rch_freq() * 1000 / 115200) / 4 - 1;

    void check_gpio_safety();
    check_gpio_safety();

    void wdt_close();
    wdt_close();
    while (1);
#endif
    return 0;
}

AT_VOLATILE_RAM_CODE_POWER
u8 soff_safety_check(void)
{
#if 0
    u32 get_rch_freq();
    JL_PORTA->DIR &= ~BIT(5);
    JL_OMAP->PA5_OUT = FO_UART0_TX;
    MAIN_CLOCK_SEL(MAIN_CLOCK_IN_RC);
    HSB_CLK_DIV(0);
    LSB_CLK_DIV(0);
    UART_CLOCK_IN(UART_CLOCK_IN_LSB);

    JL_UART0->BAUD = (get_rch_freq() * 1000 / 115200) / 4 - 1;

    void check_gpio_safety();
    check_gpio_safety();
#endif
    return 0;
}
