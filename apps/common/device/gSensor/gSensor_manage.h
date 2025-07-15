#ifndef _GSENSOR_MANAGE_H
#define _GSENSOR_MANAGE_H
#include "printf.h"
#include "cpu.h"
//#include "iic.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "timer.h"
#include "app_config.h"
#include "event.h"
#include "system/includes.h"

#define ACCEL_OF_GRAVITY        10
#define ACCEL_DATA_GAIN         10

enum {
    GSENSOR_DISABLE = 0,
    GSENSOR_RESET_INT,
    GSENSOR_RESUME_INT,
    GSENSOR_INT_DET,
    READ_GSENSOR_DATA,
    GET_ACCEL_DATA,
    SEARCH_SENSOR,
};

typedef struct {
    short x;
    short y;
    short z;
} axis_info_t;

typedef struct {
    u8   logo[20];
    u8(*gravity_sensor_init)(void);
    char (*gravity_sensor_check)(void);
    void (*gravity_sensor_ctl)(u8 cmd, void *arg);
} G_SENSOR_INTERFACE;


struct gsensor_platform_data {
    u8    iic;
    char  gSensor_name[20];
    u32   gSensor_int_io;
};

typedef struct {
    u8   iic_hdl;
    u8   iic_delay;                 //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    u8   ctl_flag;
    int  init_flag;
    int  check_cnt;
    int  check_timer_hdl;
} G_SENSOR_INFO;

int gravity_sensor_init(void *_data);
int gsensor_enable(void);
int gsensor_disable(void);
u8 gravity_sensor_command(u8 w_chip_id, u8 register_address, u8 function_command);
u8 _gravity_sensor_get_ndata(u8 r_chip_id, u8 register_address, u8 *buf, u8 data_len);
int get_gSensor_data(short *buf);
int gSensor_read_data(u8 *buf, u8 buflen);
int read_gsensor_buf(short *buf);
int read_gsensor_nbuf(short *buf, short datalen);
extern G_SENSOR_INTERFACE  gsensor_dev_begin[];
extern G_SENSOR_INTERFACE gsensor_dev_end[];

#define REGISTER_GRAVITY_SENSOR(gSensor) \
	static G_SENSOR_INTERFACE gSensor SEC_USED(.gsensor_dev)

#define list_for_each_gsensor(c) \
	for (c=gsensor_dev_begin; c<gsensor_dev_end; c++)

#define GSENSOR_PLATFORM_DATA_BEGIN(data) \
		static const struct gsensor_platform_data data = {

#define GSENSOR_PLATFORM_DATA_END() \
};

/*
enum {
    GSENSOR_EVENT_CLICK = 0,
    GSENSOR_EVENT_DOUBLE_CLICK,
    GSENSOR_EVENT_THREE_CLICK,
};

#define DEVICE_EVENT_GSENSOR	(('G' << 24) | ('S' << 16) | ('R' << 8) | '\0')
*/
#endif
