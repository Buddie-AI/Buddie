#ifndef _IMUSENSOR_MANAGE_H
#define _IMUSENSOR_MANAGE_H

#include "app_config.h"
#include "system/includes.h"

#if TCFG_IMUSENSOR_ENABLE
enum OPERATION_SENSOR {
    IMU_SENSOR_SEARCH = 0,//检查传感器id
    IMU_GET_SENSOR_NAME,//
    IMU_SENSOR_ENABLE,
    IMU_SENSOR_DISABLE,
    IMU_SENSOR_RESET,
    IMU_SENSOR_SLEEP,
    IMU_SENSOR_WAKEUP,

    IMU_SENSOR_INT_DET,//传感器中断状态检查
    IMU_SENSOR_DATA_READY,//传感器数据准备就绪待读
    IMU_SENSOR_READ_DATA,//默认读传感器所有数据
    IMU_GET_ACCEL_DATA,//加速度数据
    IMU_GET_GYRO_DATA,//陀螺仪数据
    IMU_GET_MAG_DATA,//磁力计数据
    IMU_SENSOR_CHECK_DATA,//检查传感器缓存buf是否存满
    IMU_GET_SENSOR_STATUS,//获取传感器状态
    IMU_SET_SENSOR_FIFO_CONFIG,//配置传感器FIFO
    IMU_GET_SENSOR_READ_FIFO,//读取传感器FIFO数据
    IMU_SET_SENSOR_TEMP_DISABLE,//是否关闭温度传感器
};
typedef struct {
    char   logo[20];
    s8(*imu_sensor_init)(void *arg);
    char (*imu_sensor_check)(void);
    int (*imu_sensor_ctl)(u8 cmd, void *arg);
} IMU_SENSOR_INTERFACE;

typedef struct {
    u8   iic_hdl;
    u8   iic_delay;                 //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    int  init_flag;
} IMU_SENSOR_INFO;

struct imusensor_platform_data {
    u8 peripheral_hdl;        //iic_hdl     or  spi_hdl
    u8 peripheral_param0;     //iic_delay(iic byte间间隔)   or spi_cs_pin
    u8 peripheral_param1;     //spi_mode
    char  imu_sensor_name[20];
    int   imu_sensor_int_io;
};
typedef struct {
    short x;
    short y;
    short z;
} imu_axis_data_t;
typedef struct {
    imu_axis_data_t acc;
    imu_axis_data_t gyro;
    float temp_data;
} imu_sensor_data_t;

// u8 imusensor_write_nbyte(u8 w_chip_id, u8 register_address, u8 *buf, u8 data_len);
// u8 imusensor_read_nbyte(u8 r_chip_id, u8 register_address, u8 *buf, u8 data_len);
int imu_sensor_io_ctl(u8 *sensor_name, enum OPERATION_SENSOR cmd, void *arg);
int imu_sensor_init(void *_data, u16 _data_size);

extern IMU_SENSOR_INTERFACE imusensor_dev_begin[];
extern IMU_SENSOR_INTERFACE imusensor_dev_end[];

#define REGISTER_IMU_SENSOR(imuSensor) \
	static IMU_SENSOR_INTERFACE imuSensor SEC_USED(.imusensor_dev)

#define list_for_each_imusensor(c) \
	for (c=imusensor_dev_begin; c<imusensor_dev_end; c++)

#define IMU_SENSOR_PLATFORM_DATA_BEGIN(imu_sensor_data) \
		static const struct imusensor_platform_data imu_sensor_data[] = {

#define IMU_SENSOR_PLATFORM_DATA_END() \
};

#endif
#endif

