#include "app_config.h"
#include "ir_sensor/ir_manage.h"

#if(TCFG_IRSENSOR_ENABLE == 1)

#define LOG_TAG             "[IRSENSOR]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_SENOR_USER_IIC_TYPE
#define iic_init(iic)                       hw_iic_init(iic)
#define iic_uninit(iic)                     hw_iic_uninit(iic)
#define iic_start(iic)                      hw_iic_start(iic)
#define iic_stop(iic)                       hw_iic_stop(iic)
#define iic_tx_byte(iic, byte)              hw_iic_tx_byte(iic, byte)
#define iic_rx_byte(iic, ack)               hw_iic_rx_byte(iic, ack)
#define iic_read_buf(iic, buf, len)         hw_iic_read_buf(iic, buf, len)
#define iic_write_buf(iic, buf, len)        hw_iic_write_buf(iic, buf, len)
#define iic_suspend(iic)                    hw_iic_suspend(iic)
#define iic_resume(iic)                     hw_iic_resume(iic)
#else
#define iic_init(iic)                       soft_iic_init(iic)
#define iic_uninit(iic)                     soft_iic_uninit(iic)
#define iic_start(iic)                      soft_iic_start(iic)
#define iic_stop(iic)                       soft_iic_stop(iic)
#define iic_tx_byte(iic, byte)              soft_iic_tx_byte(iic, byte)
#define iic_rx_byte(iic, ack)               soft_iic_rx_byte(iic, ack)
#define iic_read_buf(iic, buf, len)         soft_iic_read_buf(iic, buf, len)
#define iic_write_buf(iic, buf, len)        soft_iic_write_buf(iic, buf, len)
#define iic_suspend(iic)                    soft_iic_suspend(iic)
#define iic_resume(iic)                     soft_iic_resume(iic)
#endif

static const struct irSensor_platform_data *platform_data;
IR_SENSOR_INTERFACE *irSensor_hdl = NULL;
IR_SENSOR_INFO  __irSensor_info = {.iic_delay = 30, .ir_event = IR_SENSOR_EVENT_NULL};
#define irSensor_info (&__irSensor_info)

#define JSA_IIC_DELAY       50



u8 irSensor_write(u8 w_chip_id, u8 register_address, u8 function_command)
{
    //spin_lock(&iic_lock);
    u8 ret = 1;
    iic_start(irSensor_info->iic_hdl);
    if (0 == iic_tx_byte(irSensor_info->iic_hdl, w_chip_id)) {
        ret = 0;
        log_e("\n JSA iic wr err 0\n");
        goto __gcend;
    }

    delay(JSA_IIC_DELAY);

    if (0 == iic_tx_byte(irSensor_info->iic_hdl, register_address)) {
        ret = 0;
        log_e("\n JSA iic wr err 1\n");
        goto __gcend;
    }

    delay(JSA_IIC_DELAY);

    if (0 == iic_tx_byte(irSensor_info->iic_hdl, function_command)) {
        ret = 0;
        log_e("\n JSA iic wr err 2\n");
        goto __gcend;
    }

    g_printf("JSA iic wr succ\n");
__gcend:
    iic_stop(irSensor_info->iic_hdl);

    //spin_unlock(&iic_lock);
    return ret;
}


u8 irSensor_read(u8 r_chip_id, u8 register_address, u8 *buf, u8 data_len)
{
    u8 read_len = 0;

    //spin_lock(&iic_lock);

    iic_start(irSensor_info->iic_hdl);
    if (0 == iic_tx_byte(irSensor_info->iic_hdl, r_chip_id - 1)) {
        log_e("\n JSA iic rd err 0\n");
        read_len = 0;
        goto __gdend;
    }


    delay(JSA_IIC_DELAY);
    if (0 == iic_tx_byte(irSensor_info->iic_hdl, register_address)) {
        log_e("\n JSA iic rd err 1\n");
        read_len = 0;
        goto __gdend;
    }

    iic_start(irSensor_info->iic_hdl);
    if (0 == iic_tx_byte(irSensor_info->iic_hdl, r_chip_id)) {
        log_e("\n JSA iic rd err 2\n");
        read_len = 0;
        goto __gdend;
    }

    delay(JSA_IIC_DELAY);

    for (; data_len > 1; data_len--) {
        *buf++ = iic_rx_byte(irSensor_info->iic_hdl, 1);
        read_len ++;
    }

    *buf = iic_rx_byte(irSensor_info->iic_hdl, 0);
    read_len ++;

__gdend:

    iic_stop(irSensor_info->iic_hdl);
    delay(JSA_IIC_DELAY);

    //spin_unlock(&iic_lock);
    return read_len;
}

void irSensor_int_io_detect(void *priv)
{
    struct sys_event e;
    if (gpio_read(irSensor_info->int_io) == 0) {
        irSensor_info->ir_event = irSensor_hdl->ir_sensor_check();
        if (irSensor_info->ir_event != IR_SENSOR_EVENT_NULL) {
            log_info("irSensor event trigger:%d", irSensor_info->ir_event);
            e.type = SYS_IR_EVENT;
            e.u.ir.event = irSensor_info->ir_event;
            sys_event_notify(&e);
            //Interface: initial state after reboot is near
        }
    }
}

enum irSensor_event get_irSensor_event(void)
{
    return irSensor_info->ir_event;
}

int ir_sensor_init(void *_data)
{
    int retval = 0;
    platform_data = (const struct irSensor_platform_data *)_data;
    irSensor_info->iic_hdl = platform_data->iic;
    irSensor_info->int_io = platform_data->irSensor_int_io;

    retval = iic_init(irSensor_info->iic_hdl);
    if (retval < 0) {
        log_e("\n  open iic for gsensor err\n");
        return retval;
    } else {
        log_info("\n iic open succ\n");
    }

    retval = -EINVAL;
    list_for_each_irSensor(irSensor_hdl) {
        if (!memcmp(irSensor_hdl->logo, platform_data->irSensor_name, strlen(platform_data->irSensor_name))) {
            retval = 0;
            break;
        }
    }

    if (retval < 0) {
        log_e(">>>irSensor_hdl logo err\n");
        return retval;
    } else {

    }

    g_printf("irSensor_init_io:%d", platform_data->irSensor_int_io);
    gpio_set_pull_up(platform_data->irSensor_int_io, 0);
    gpio_set_pull_down(platform_data->irSensor_int_io, 0);
    gpio_set_direction(platform_data->irSensor_int_io, 1);
    gpio_set_die(platform_data->irSensor_int_io, 1);


    if (irSensor_hdl->ir_sensor_init(platform_data->pxs_low_th, platform_data->pxs_high_th)) {
        log_info(">>>>irSensor_Int SUCC\n");
        sys_s_hi_timer_add(NULL, irSensor_int_io_detect, 10);
    } else {
        log_e(">>>>irSensor_Int ERROR\n");
    }
    return 0;

}

#endif
