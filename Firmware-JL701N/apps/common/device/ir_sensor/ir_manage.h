#ifndef _IR_SENSOR_MANAGE_H
#define _IR_SENSOR_MANAGE_H

#include "printf.h"
#include "cpu.h"
//#include "iic.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "timer.h"
#include "app_config.h"
#include "event.h"
#include "system/includes.h"

enum {
    IR_SENSOR_RESET_INT = 0,
    IR_SENSOR_RESUME_INT,
    IR_SENSOR_INT_DET,
};

typedef struct {
    u8   logo[20];
    u8(*ir_sensor_init)(u16 low_th, u16 high_th);
    char (*ir_sensor_check)(void);
    void (*ir_sensor_ctl)(u8 cmd, void *arg);
} IR_SENSOR_INTERFACE;


struct irSensor_platform_data {
    u8    iic;
    char  irSensor_name[20];
    int   irSensor_int_io;
    u16   pxs_low_th;
    u16   pxs_high_th;
};

enum irSensor_event {
    IR_SENSOR_EVENT_NULL = 0,
    IR_SENSOR_EVENT_FAR,
    IR_SENSOR_EVENT_NEAR,
};

typedef struct {
    u8   iic_hdl;
    u8   iic_delay;                 //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    int  int_io;
    int  check_timer_hdl;
    enum irSensor_event ir_event;
} IR_SENSOR_INFO;

int ir_sensor_init(void *_data);
u8 ir_sensor_command(u8 w_chip_id, u8 register_address, u8 function_command);

extern IR_SENSOR_INTERFACE  irSensor_dev_begin[];
extern IR_SENSOR_INTERFACE  irSensor_dev_end[];

#define REGISTER_IR_SENSOR(gSensor) \
	static IR_SENSOR_INTERFACE irSensor SEC_USED(.irSensor_dev)

#define list_for_each_irSensor(c) \
	for (c=irSensor_dev_begin; c<irSensor_dev_end; c++)

#define IR_SENSOR_PLATFORM_DATA_BEGIN(data) \
		static const struct irSensor_platform_data data = {

#define IR_GSENSOR_PLATFORM_DATA_END() \
};


int ir_sensor_init(void *_data);
u8 irSensor_write(u8 w_chip_id, u8 register_address, u8 function_command);
u8 irSensor_read(u8 r_chip_id, u8 register_address, u8 *buf, u8 data_len);
enum irSensor_event get_irSensor_event(void);

#endif
