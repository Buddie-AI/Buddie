#include "gSensor/EPMotion.h"
#include "gSensor/da230.h"
#include "gSensor/gSensor_manage.h"
#include "app_config.h"

#if TCFG_DA230_EN

#define LOG_TAG             "[DA230]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

static u8 da230_init_ok = 0;
static u8 tap_data = 0;
static u8 tap_start = 0;
static u8 event_ocur = 0;
static u8 da230_is_idle = 1;

void da230_set_idle_flag(u8 flag)
{
    da230_is_idle = flag;
}

u32 da230_register_read(u8 addr, u8 *data)
{
    _gravity_sensor_get_ndata(I2C_ADDR_DA230_R, addr, data, 1);
    return 0;
}

s8_m da230_register_write(u8_m addr, u8_m data)
{
    gravity_sensor_command(I2C_ADDR_DA230_W, addr, data);
    return 0;
}

s8_m  da230_read_nbyte_data(u8_m addr, u8_m *data, u8_m len)
{
    _gravity_sensor_get_ndata(I2C_ADDR_DA230_R, addr, data, len);
    return 0;
}

s8_m da230_register_mask_write(unsigned char addr, unsigned char mask, unsigned char data)
{
    int     res = 0;
    unsigned char      tmp_data;

    /* iic_readn(I2C_ADDR_DA230_R, addr, &tmp_data, 1); */
    da230_read_nbyte_data(addr, &tmp_data, 1);

    tmp_data &= ~mask;
    tmp_data |= data & mask;
    /* iic_write(I2C_ADDR_DA230_W, addr, &tmp_data, 1); */
    da230_register_write(addr, tmp_data);

    return 1;
}

s8_m handleEvent(EPMotion_EVENT event, u8 data)
{
    event_ocur = 0;
    tap_start = 0;
    da230_set_idle_flag(1);
    log_info("da230_set_idle_flag:1");
    switch (event) {
    case TEVENT_TAP_NOTIFY: {
        tap_data = data;
        printf("*******DATA:%d    \n", data);
    }
    break;
    default:
        break;
    }

    return 0;
}

u32 da230_register_write_bit(u8 addr, u8 start, u8 len, u8 data)
{
    u32 res = 0;
    u8 tmp_data;

    res = da230_register_read(addr, &tmp_data);
    if (res) {
        return res;
    }

    SFR(tmp_data, start, len, data);

    res = da230_register_write(addr, tmp_data);

    return res;
}

u32 da230_close_i2c_pullup(void)
{
    u32 res = 0;

    res |= da230_register_write(0x7f,  0x83);
    res |= da230_register_write(0x7f,  0x69);
    res |= da230_register_write(0x7f,  0xBD);

    /**********将0x8f bit1写0，去掉da230内部i2c弱上拉*********/
    res |= da230_register_write_bit(0x8f, 1, 1, 0);

    return res;
}
#if 0
u32 da230_read_block_data(u8 base_addr, u8 count, u8 *data)
{
    u32 i = 0;

    for (i = 0; i < count; i++) {
        if (da230_register_read(base_addr + i, (data + i))) {
            return -1;
        }
    }

    return count;
}


u32 da230_register_read_continuously(u8 addr, u8 count, u8 *data)
{
    u32 res = 0;

    res = (count == da230_read_block_data(addr, count, data)) ? 0 : 1;

    return res;
}

u32 da230_open_step_counter(void)
{
    u32 res = 0;

    res |=  da230_register_write(NSA_REG_STEP_CONGIF1, 0x01);
    res |=  da230_register_write(NSA_REG_STEP_CONGIF2, 0x62);
    res |=  da230_register_write(NSA_REG_STEP_CONGIF3, 0x46);
    res |=  da230_register_write(NSA_REG_STEP_CONGIF4, 0x32);
    res |=  da230_register_write(NSA_REG_STEP_FILTER, 0xa2);

    return res;
}

u16 da230_get_step(void)
{
    u8 tmp_data[4] = {0};
    u16 f_step = 0;

    if (da230_register_read_continuously(NSA_REG_STEPS_MSB, 4, tmp_data) == 0) {
        if (0x40 == tmp_data[2] && 0x07 == tmp_data[3]) {
            f_step = ((tmp_data[0] << 8 | tmp_data[1])) / 2;
        }
    }


    return (f_step);
}

u32 da230_close_step_counter(void)
{
    u32 res = 0;

    res =  da230_register_write(NSA_REG_STEP_FILTER, 0x22);

    return res;
}


u32 da230_register_write_bit(u8 addr, u8 start, u8 len, u8 data)
{
    u32 res = 0;
    u8 tmp_data;

    res = da230_register_read(addr, &tmp_data);
    if (res) {
        return res;
    }

    SFR(tmp_data, start, len, data);

    res = da230_register_write(addr, tmp_data);

    return res;
}

u32 da230_open_i2c_pullup(void)
{
    u32 res = 0;

    res |= da230_register_write(0x7f,  0x83);
    res |= da230_register_write(0x7f,  0x69);
    res |= da230_register_write(0x7f,  0xBD);

    /**********将0x8f bit1写1，开启da230内部i2c弱上拉*********/
    res |= da230_register_write_bit(0x8f, 1, 1, 1);

    return res;
}
#endif

struct EPMotion_op_s ops_handle = {
    {PIN_ONE, PIN_LEVEL_LOW},
    handleEvent,
    {da230_read_nbyte_data, da230_register_write},
    printf
};
u8  da230_init(void)
{
    u8 data = 0;

    JL_PORTB->DIR &= ~BIT(2);
    JL_PORTB->DIE |=  BIT(2);
    JL_PORTB->OUT |=  BIT(2);
    da230_register_read(NSA_REG_WHO_AM_I, &data);

    if (data != 0x13) {
        log_e("da230 init err1!!!!!!!\n");
        return -1;
    }

    if (EPMotion_Init(&ops_handle)) {
        log_e("da230 init err2!!!!!!!\n");
        return -1;
    }

    EPMotion_Set_Debug_level(DEBUG_ERR);
    EPMotion_Tap_Set_Parma(0x0d, LATCHED_100MS, 0x00); //0x05~0x1f
    //EPMotion_D_Tap_Set_Filter(1, 130);
    EPMotion_M_Tap_Set_Dur(70, 500);
//    EPMotion_M_Tap_Set_Dur(60,150);
    da230_register_mask_write(0x10, 0xe0, 0xc0);
    EPMotion_Control(M_TAP_T, ENABLE_T);
//    EPMotion_Tap_Set_Parma(0x08,LATCHED_25MS,0x00);   //0x05~0x1f


    //close i2c pullup
    da230_close_i2c_pullup();
    da230_register_mask_write(0x8f, 0x02, 0x00);

    da230_init_ok = 1;

    log_info("da230 init success!!!\n");

    return 0;
}

u32 da230_set_enable(u8 enable)
{
    u32 res = 0;

    if (da230_init_ok) {

        if (enable) {
            EPMotion_Chip_Power_On();
        } else {
            EPMotion_Chip_Power_Off();
        }
    }

    return res;
}


void da230_read_XYZ(short *x, short *y, short *z)
{
    if (da230_init_ok) {
        EPMotion_Chip_Read_XYZ(x, y, z);
    }
}

#if 0
u16 da230_open_interrupt(u16 num)
{
    u16   res = 0;
    res = da230_register_write(NSA_REG_INTERRUPT_SETTINGS1, 0x87);
    res = da230_register_write(NSA_REG_ACTIVE_THRESHOLD, 0x05);
    switch (num) {
    case 0:
        res = da230_register_write(NSA_REG_INTERRUPT_MAPPING1, 0x04);
        break;
    case 1:
        res = da230_register_write(NSA_REG_INTERRUPT_MAPPING3, 0x04);
        break;
    }
    return res;
}
#endif

void da230_check_tap_int(void)
{
    static int count = 0;
    if (tap_start) {
        if (event_ocur == 1) {
            count = 0;
        }
        if ((count % 5) == 0) {
            //printf("====process event %d\r\n",event_ocur);
//            gpio_direction_output(IO_PORT_DM, 1);
            EPMotion_Process_Data(event_ocur);
//			gpio_direction_output(IO_PORT_DM, 0);
        }
        count++;

        if (count >= 300000) {
            count = 0;
        }
    }
}

void da230_low_energy_mode(void)
{
    da230_register_write(0x11, 0x04);
}

void da230_int_io_detect(u8 int_io_status)
{
    static u8 int_io_status_old = 0;
    /* u8 int_io_status = 0; */
    if (da230_init_ok) {
        /* int_io_status = gpio_read(INT_IO); */
        if ((int_io_status) && (int_io_status_old == 0)) {
            log_e("da230_int_io_detect\n");
            if (tap_start == 0) {
                tap_start = 1;
                da230_set_idle_flag(0);
                log_info("da230_set_idle_flag:0");
            }
            event_ocur = 1;
            putchar('A');
        } else {
            event_ocur = 0;
        }
        int_io_status_old = int_io_status;

        da230_check_tap_int();

    }

}

static char da230_check_event()
{
    u8 tmp_tap_data = 0;
    tmp_tap_data = tap_data;
    if (tap_data) {
        tap_data = 0;
        printf("tmp_tap_data = %d\n", tmp_tap_data);
    }
    return tmp_tap_data;
    /* return EPMotion_Process_Data(); */
}

void da230_ctl(u8 cmd, void *arg)
{
    switch (cmd) {
    case GSENSOR_RESET_INT:
        EPMotion_Reset_Tap_INT();
        break;
    case GSENSOR_RESUME_INT:
        EPMotion_Resume_Tap_INT();
        break;
    case GSENSOR_INT_DET:
        da230_int_io_detect(*(u8 *)arg);
        break;
    default:

        break;
    }
}

static u8 da230_idle_query(void)
{
    return da230_is_idle;
}

REGISTER_GRAVITY_SENSOR(gSensor) = {
    .logo = "da230",
    .gravity_sensor_init  = da230_init,
    .gravity_sensor_check = da230_check_event,
    .gravity_sensor_ctl   = da230_ctl,
};

REGISTER_LP_TARGET(da230_lp_target) = {
    .name = "da230",
    .is_idle = da230_idle_query,
};

#endif


