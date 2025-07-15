#include "asm/power_interface.h"
#include "app_config.h"
#include "includes.h"

static u8 gpiousb = 0x3;
static u32 usb_io_con = 0;

/* cpu公共流程：
 * 请勿添加板级相关的流程，例如宏定义
 * 可以重写改流程
 * 所有io保持原状，除usb io
 */
void sleep_gpio_protect(u32 gpio)
{
    if (gpio == IO_PORT_DP) {
        gpiousb &= ~BIT(0);
    } else if (gpio == IO_PORT_DM) {
        gpiousb &= ~BIT(1);
    }
}

void sleep_enter_callback_common(void *priv)
{
    usb_io_con = JL_USB_IO->CON0;

    if (gpiousb) {
        usb_iomode(1);

        if (gpiousb & BIT(0)) {
            gpio_set_pull_up(IO_PORT_DP, 0);
            gpio_set_pull_down(IO_PORT_DP, 0);
            gpio_set_direction(IO_PORT_DP, 1);
            gpio_set_die(IO_PORT_DP, 0);
            gpio_set_dieh(IO_PORT_DP, 0);
        }

        if (gpiousb & BIT(1)) {
            gpio_set_pull_up(IO_PORT_DM, 0);
            gpio_set_pull_down(IO_PORT_DM, 0);
            gpio_set_direction(IO_PORT_DM, 1);
            gpio_set_die(IO_PORT_DM, 0);
            gpio_set_dieh(IO_PORT_DM, 0);
        }
    }
}

void sleep_exit_callback_common(void *priv)
{
    JL_USB_IO->CON0 = usb_io_con;
    gpiousb = 0x3;
}

static struct gpio_value soff_gpio_config = {
    .gpioa = 0xffff,
    .gpiob = 0xffff,
    .gpioc = 0xffff,
    .gpiod = 0xffff,
    .gpioe = 0xffff,
    .gpiog = 0xffff,
    .gpiop = 0x1,//
    .gpiousb = 0x3,
};

void soff_gpio_protect(u32 gpio)
{
    if ((gpio >= 0) && (gpio < IO_MAX_NUM)) {
        port_protect((u16 *)&soff_gpio_config, gpio);
    } else if (gpio == IO_PORT_DP) {
        soff_gpio_config.gpiousb &= ~BIT(0);
    } else if (gpio == IO_PORT_DM) {
        soff_gpio_config.gpiousb &= ~BIT(1);
    }
}

/* cpu公共流程：
 * 请勿添加板级相关的流程，例如宏定义
 * 可以重写改流程
 * 释放除内置flash外的所有io
 */
//maskrom 使用到的io
static void mask_io_cfg()
{
    struct boot_soft_flag_t boot_soft_flag = {0};
    boot_soft_flag.flag0.boot_ctrl.wdt_dis = 0;
    boot_soft_flag.flag0.boot_ctrl.poweroff = 0;
    boot_soft_flag.flag0.boot_ctrl.is_port_b = JL_IOMAP->CON0 & BIT(16) ? 1 : 0;
    boot_soft_flag.flag0.boot_ctrl.lvd_en = (GET_VLVD_EN() ? 1 : 0);

    boot_soft_flag.flag1.misc.usbdm = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag1.misc.usbdp = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag1.misc.uart_key_port = 0;
    boot_soft_flag.flag1.misc.ldoin = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag2.pg2_pg3.pg2 = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag2.pg2_pg3.pg3 = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag3.pg4_res.pg4 = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag4.fast_boot_ctrl.fast_boot = 0;
    boot_soft_flag.flag4.fast_boot_ctrl.flash_stable_delay_sel = 0;

    mask_softflag_config(&boot_soft_flag);

}

void board_set_soft_poweroff_common(void *priv)
{
    //flash电源
    if (get_sfc_port() == 0) {
        soff_gpio_protect(SPI0_PWR_A);
        soff_gpio_protect(SPI0_CS_A);
        soff_gpio_protect(SPI0_CLK_A);
        soff_gpio_protect(SPI0_DO_D0_A);
        soff_gpio_protect(SPI0_DI_D1_A);
        if (get_sfc_bit_mode() == 4) {
            soff_gpio_protect(SPI0_WP_D2_A);
            soff_gpio_protect(SPI0_HOLD_D3_A);
        }
    } else {
        soff_gpio_protect(SPI0_PWR_B);
        soff_gpio_protect(SPI0_CS_B);
        soff_gpio_protect(SPI0_CLK_B);
        soff_gpio_protect(SPI0_DO_D0_B);
        soff_gpio_protect(SPI0_DI_D1_B);
        if (get_sfc_bit_mode() == 4) {
            soff_gpio_protect(SPI0_WP_D2_B);
            soff_gpio_protect(SPI0_HOLD_D3_B);
        }
    }

    mask_io_cfg();

    gpio_dir(GPIOA,    0, 16,  soff_gpio_config.gpioa, GPIO_OR);
    gpio_set_pu(GPIOA, 0, 16, ~soff_gpio_config.gpioa, GPIO_AND);
    gpio_set_pd(GPIOA, 0, 16, ~soff_gpio_config.gpioa, GPIO_AND);
    gpio_die(GPIOA,    0, 16, ~soff_gpio_config.gpioa, GPIO_AND);
    gpio_dieh(GPIOA,   0, 16, ~soff_gpio_config.gpioa, GPIO_AND);

    gpio_dir(GPIOB,    0, 16,  soff_gpio_config.gpiob, GPIO_OR);
    gpio_set_pu(GPIOB, 0, 16, ~soff_gpio_config.gpiob, GPIO_AND);
    gpio_set_pd(GPIOB, 0, 16, ~soff_gpio_config.gpiob, GPIO_AND);
    gpio_die(GPIOB,    0, 16, ~soff_gpio_config.gpiob, GPIO_AND);
    gpio_dieh(GPIOB,   0, 16, ~soff_gpio_config.gpiob, GPIO_AND);

    gpio_dir(GPIOC,    0, 16,  soff_gpio_config.gpioc, GPIO_OR);
    gpio_set_pu(GPIOC, 0, 16, ~soff_gpio_config.gpioc, GPIO_AND);
    gpio_set_pd(GPIOC, 0, 16, ~soff_gpio_config.gpioc, GPIO_AND);
    gpio_die(GPIOC,    0, 16, ~soff_gpio_config.gpioc, GPIO_AND);
    gpio_dieh(GPIOC,   0, 16, ~soff_gpio_config.gpioc, GPIO_AND);

    gpio_dir(GPIOD,    0, 16,  soff_gpio_config.gpiod, GPIO_OR);
    gpio_set_pu(GPIOD, 0, 16, ~soff_gpio_config.gpiod, GPIO_AND);
    gpio_set_pd(GPIOD, 0, 16, ~soff_gpio_config.gpiod, GPIO_AND);
    gpio_die(GPIOD,    0, 16, ~soff_gpio_config.gpiod, GPIO_AND);
    gpio_dieh(GPIOD,   0, 16, ~soff_gpio_config.gpiod, GPIO_AND);

    gpio_dir(GPIOE,    0, 16,  soff_gpio_config.gpioe, GPIO_OR);
    gpio_set_pu(GPIOE, 0, 16, ~soff_gpio_config.gpioe, GPIO_AND);
    gpio_set_pd(GPIOE, 0, 16, ~soff_gpio_config.gpioe, GPIO_AND);
    gpio_die(GPIOE,    0, 16, ~soff_gpio_config.gpioe, GPIO_AND);
    gpio_dieh(GPIOE,   0, 16, ~soff_gpio_config.gpioe, GPIO_AND);

    gpio_dir(GPIOG,    0, 16,  soff_gpio_config.gpiog, GPIO_OR);
    gpio_set_pu(GPIOG, 0, 16, ~soff_gpio_config.gpiog, GPIO_AND);
    gpio_set_pd(GPIOG, 0, 16, ~soff_gpio_config.gpiog, GPIO_AND);
    gpio_die(GPIOG,    0, 16, ~soff_gpio_config.gpiog, GPIO_AND);
    gpio_dieh(GPIOG,   0, 16, ~soff_gpio_config.gpiog, GPIO_AND);

    gpio_dir(GPIOP,    0, 1,   soff_gpio_config.gpiop, GPIO_OR);
    gpio_set_pu(GPIOP, 0, 1,  ~soff_gpio_config.gpiop, GPIO_AND);
    gpio_set_pd(GPIOP, 0, 1,  ~soff_gpio_config.gpiop, GPIO_AND);
    gpio_die(GPIOP,    0, 1,  ~soff_gpio_config.gpiop, GPIO_AND);
    gpio_dieh(GPIOP,   0, 1,  ~soff_gpio_config.gpiop, GPIO_AND);

    if (soff_gpio_config.gpiousb) {
        usb_iomode(1);

        if (soff_gpio_config.gpiousb & BIT(0)) {
            gpio_set_pull_up(IO_PORT_DP, 0);
            gpio_set_pull_down(IO_PORT_DP, 0);
            gpio_set_direction(IO_PORT_DP, 1);
            gpio_set_die(IO_PORT_DP, 0);
            gpio_set_dieh(IO_PORT_DP, 0);
        }

        if (soff_gpio_config.gpiousb & BIT(1)) {
            gpio_set_pull_up(IO_PORT_DM, 0);
            gpio_set_pull_down(IO_PORT_DM, 0);
            gpio_set_direction(IO_PORT_DM, 1);
            gpio_set_die(IO_PORT_DM, 0);
            gpio_set_dieh(IO_PORT_DM, 0);
        }
    }
}


