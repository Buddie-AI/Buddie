#if(TCFG_JSA1221_ENABLE  == 1)
#include "asm/includes.h"
#include "irSensor/jsa1221.h"
#include "app_config.h"
#include "asm/iic_soft.h"
#include "asm/iic_hw.h"
#include "system/timer.h"

#include "system/includes.h"
#include "ir_manage.h"

static u32 jsa_high_th = 0;    //红外穿戴中断的触发阈值
static u32 jsa_low_th  = 0;

u8 jsa1221_pxs_int_flag(void)
{
    u8 int_flag = 0;
    irSensor_read(JSA_1221_READ_ADDR, JSA_INT_FLAG, &int_flag, 1);
    if (int_flag & BIT(1)) {
        return 1;
    } else {
        return 0;
    }
}

u16 jsa1221_read_pxs_data(void)
{
    u8 lower_data, upper_data = 0;
    irSensor_read(JSA_1221_READ_ADDR, JSA_PXS_DATA1, &lower_data, 1);
    irSensor_read(JSA_1221_READ_ADDR, JSA_PXS_DATA2, &upper_data, 1);
    return (upper_data * 256 + lower_data);
}

enum irSensor_event jsa1221_check(void *_para)
{
    if (jsa1221_pxs_int_flag()) {
        u16 pxs_data = jsa1221_read_pxs_data();
        g_printf("DATA:%d", pxs_data);
        if (pxs_data < jsa_low_th) {
            g_printf("FAR");
            return IR_SENSOR_EVENT_FAR;
        } else {
            g_printf("NEAR");
            return IR_SENSOR_EVENT_NEAR;
        }
    } else {
        return IR_SENSOR_EVENT_NULL;
    }
}

u8 jsa1221_init(u16 pxs_low_th, u16 pxs_high_th)
{
    u32 ret = 1;
    jsa_low_th  = pxs_low_th;
    jsa_high_th = pxs_high_th;
    ret &= irSensor_write(JSA_1221_WRITE_ADDR, JSA_CONFIGURE, PXS_ACTIVE_MODE);
    ret &= irSensor_write(JSA_1221_WRITE_ADDR, JSA_PXS_LOW_TH1, pxs_low_th & 0xff);          // low 8 bit
    ret &= irSensor_write(JSA_1221_WRITE_ADDR, JSA_PXS_LOW_TH2, pxs_low_th >> 8);            // upper 8 bit
    ret &= irSensor_write(JSA_1221_WRITE_ADDR, JSA_PXS_HIGH_TH1, pxs_high_th & 0xff);         // low 8 bit
    ret &= irSensor_write(JSA_1221_WRITE_ADDR, JSA_PXS_HIGH_TH2, pxs_high_th >> 8);           // upper 8 bit
    ret &= irSensor_write(JSA_1221_WRITE_ADDR, JSA_INT_CTRL, BIT(7));    //enable pxs int

    return ret;
}



REGISTER_IR_SENSOR(irSensor) = {
    .logo = "jsa1221",
    .ir_sensor_init  = jsa1221_init,
    .ir_sensor_check = jsa1221_check,
    .ir_sensor_ctl   = NULL,
};

#endif
