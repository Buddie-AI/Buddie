#include "gSensor/STK8321.h"
#include "gSensor/gSensor_manage.h"
#include "app_config.h"

#if TCFG_STK8321_EN

static u8 stk8321_timeout_start = 0;

u8 STK8321_Read(u8 addr)
{
    u8 data = 0;
    _gravity_sensor_get_ndata(I2C_ADDR_STK8321_R, addr, &data, 1);
    return data;
}

void STK8321_Write(u8 addr, u8 data)
{
    //g_sensor_write(G_SlaveAddr,addr,data);
    gravity_sensor_command(I2C_ADDR_STK8321_W, addr, data);
}

u8 STK8321_init(void)
{
    u8 k = 0, init_cnt = 10;
    u8 chip_id = 0;
    printf("\n###############>> [ init STK8321 start] <<################\n");

    do {

        chip_id = STK8321_Read(0x00);
        printf("\n STK8321 ID = [%x]\n", chip_id);
        init_cnt--;

    } while ((chip_id != 0x86) && (init_cnt));

    STK8321_Write(0x14, 0xB6); //software reset
    delay(1000);

    if (chip_id != 0x86) {
        return -1;
    }
    STK8321_Write(0x14, 0xB6); //software reset
    delay(100);
    // mode settings
    STK8321_Write(0x11, 0x00); //active mode
    STK8321_Write(0x0F, 0x08); //range = +-8g
    STK8321_Write(0x10, 0x0D); //BW = 250Hz or ODR = 500Hz
    STK8321_Write(0x34, 0x04); //Enable I2C WatchDog
    // INT mode
    STK8321_Write(0x20, 0x00); //INT1 and INT2 active high
    STK8321_Write(0x21, 0x8F); //latched & clear
    // slope settings
    STK8321_Write(0x16, 0x04); // enable slope on Z-axis
    STK8321_Write(0x19, 0x01); // map sig-motion to INT 1
    STK8321_Write(0x27, 0x01); // The actual number of samples SLP_DUR[1:0]+1
    STK8321_Write(0x28, 0x25); // SLP_THD[7:0]*15.64_mg
    STK8321_Write(0x2A, 0x06); //Enable significant and any-motion interrupt
    STK8321_Write(0x29, 0x05); //skip_time 1LSB=20ms
    STK8321_Write(0x2B, 0x0A); //proof_time 1LSB=20ms
    printf("\n###############>> [ init STK8321 end ] <<################\n");

    return 0;
}

static char STK8321_check_event()
{
    static u16 timeout_cnt = 125;   //需要等250ms
    u8 click_cnt = 0;
    if (stk8321_timeout_start) {
        if (timeout_cnt) {
            timeout_cnt --;
        } else {
            u8 RegReadValue = 0;
            g_printf("stk8321_timeout");
            RegReadValue = STK8321_Read(0x09);
            if ((RegReadValue & 0x04) == 0x00) {
                g_printf("Double Tap");
                click_cnt = 2;
            } else if ((RegReadValue & 0x04) == 0x04) {
                g_printf("Triple Tap");
                click_cnt = 3;
            } else {
                g_printf("NO Tap");
                click_cnt = 0;
            }
            STK8321_Write(0x2A, 0x02);
            STK8321_Write(0x21, 0x8F);
            stk8321_timeout_start = 0;
            timeout_cnt = 125;
        }
    }
    return click_cnt;
}

void STK8321_int_io_detect(u8 int_io_status)
{
    static u8 int_io_status_old;
    if (!stk8321_timeout_start) {
        if (!(int_io_status) && (int_io_status_old == 1)) {
            g_printf("STK8321_int_io_detect\n");
            stk8321_timeout_start = 1;
            STK8321_Write(0x2A, 0x04);
            STK8321_Write(0x21, 0x8F);
        }
        int_io_status_old = int_io_status;
    }
}

void STK8321_ctl(u8 cmd, void *arg)
{
    switch (cmd) {
    case GSENSOR_RESET_INT:
        break;
    case GSENSOR_RESUME_INT:
        break;
    case GSENSOR_INT_DET:
        STK8321_int_io_detect(*(u8 *)arg);
        break;
    default:

        break;
    }
}

static u8 STK8321_idle_query(void)
{
    return !stk8321_timeout_start;
}

REGISTER_GRAVITY_SENSOR(gSensor) = {
    .logo = "STK8321",
    .gravity_sensor_init  = STK8321_init,
    .gravity_sensor_check = STK8321_check_event,
    .gravity_sensor_ctl   = STK8321_ctl,
};

REGISTER_LP_TARGET(da230_lp_target) = {
    .name = "STK8321",
    .is_idle = STK8321_idle_query,
};
#endif
