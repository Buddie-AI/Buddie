#include "gSensor/mpu6050.h"
#include "gSensor/gSensor_manage.h"
#include "app_config.h"
#include "imuSensor_manage.h"

#if TCFG_MPU6050_EN

int gsensorlen = 32;
OS_MUTEX SENSOR_IIC_MUTEX;
spinlock_t sensor_iic;
u8 sensor_iic_init_status = 0;

#define ACCEL_ONLY_LOW_POWER    0

u8 mpu6050_register_read(u8 addr, u8 *data)
{
    _gravity_sensor_get_ndata(I2C_ADDR_MPU6050_R, addr, data, 1);
    return 0;
}

u8 mpu6050_register_write(u8 addr, u8 data)
{
    gravity_sensor_command(I2C_ADDR_MPU6050_W, addr, data);
    return 0;
}

u8 mpu6050_read_nbyte_data(u8 addr, u8 *data, u8 len)
{
    return _gravity_sensor_get_ndata(I2C_ADDR_MPU6050_R, addr, data, len);
}

u8 mpu6050_get_xyz_data(u8 addr, void *xyz_data)
{
    u8 buf[6], read_len;
    axis_info_t *data = (axis_info_t *)xyz_data;
    read_len = mpu6050_read_nbyte_data(addr, buf, 6);
    if (read_len == 6) {
        data->x = ((u16)buf[0] << 8) | buf[1];
        data->y = ((u16)buf[2] << 8) | buf[3];
        data->z = ((u16)buf[4] << 8) | buf[5];

        data->x = 2 * ACCEL_OF_GRAVITY * ACCEL_DATA_GAIN * data->x / 32768;
        data->y = 2 * ACCEL_OF_GRAVITY * ACCEL_DATA_GAIN * data->y / 32768;
        data->z = 2 * ACCEL_OF_GRAVITY * ACCEL_DATA_GAIN * data->z / 32768;
    }
    return read_len;
}

extern void step_cal_init();
extern int step_cal();
u8 mpu6050_init()
{
    u8 res = 0;
    u8 data;
    res = mpu6050_register_read(MPU6050_RA_WHO_AM_I, &data);               //读ID
    if (data == MPU_ADDR) {
        g_printf("read MPU6050 ID suss");
    } else {
        g_printf("read MPU6050 ID err");
        return -1;
    }

    data = 0x80;
    mpu6050_register_write(MPU6050_RA_PWR_MGMT_1, data);                   //复位
    os_time_dly(10);

    data = 0x00;
    data |= BIT(3);                                                        //关闭温度传感器
#if ACCEL_ONLY_LOW_POWER
    data |= BIT(5);                                                        //设置accel-only低功耗模式
#endif
    mpu6050_register_write(MPU6050_RA_PWR_MGMT_1, data);                   //退出休眠

    data = 0x00;
    data |= (BIT(0) | BIT(1) | BIT(2));                                    //关闭陀螺仪
#if ACCEL_ONLY_LOW_POWER
    data |= (3 << 6);                                                      //accel-only低功耗模式下，唤醒频率为40Hz
#endif
    /*mpu6050_register_write(MPU6050_RA_PWR_MGMT_2, data);*/

    data = ACCEL_RANGE_2G << 3;
    mpu6050_register_write(MPU6050_RA_ACCEL_CONFIG, data);                 //加速度计量程

#if (!ACCEL_ONLY_LOW_POWER)
    data = 0x06;
    mpu6050_register_write(MPU6050_RA_CONFIG, data);                       //设置陀螺仪输出速率

    data = MPU6050_GYRO_OUT_RATE / MPU6050_SAMPLE_RATE - 1;
    mpu6050_register_write(MPU6050_RA_SMPLRT_DIV, data);                   //设置采样率, 采样率=陀螺仪输出速率/(1+SMPLRT_DIV)

    data = 0x00;
    mpu6050_register_write(MPU6050_RA_INT_ENABLE, data);                   //关闭中断

    data = 0x00;
    mpu6050_register_write(MPU6050_RA_USER_CTRL, data);                    //关闭主机IIC
#endif

    /*step_cal_init();*/
    /*sys_timer_add(NULL, step_cal, 50);*/
    return 0;
}

void mpu6050_ctl(u8 cmd, void *arg)
{
    switch (cmd) {
    case GET_ACCEL_DATA:
    case IMU_GET_ACCEL_DATA:
        mpu6050_get_xyz_data(MPU6050_RA_ACCEL_XOUT_H, arg);
        break;
    case IMU_GET_GYRO_DATA:
        mpu6050_get_xyz_data(MPU6050_RA_GYRO_XOUT_H, arg);
        break;
    case READ_GSENSOR_DATA:

        break;
    }
}

REGISTER_GRAVITY_SENSOR(gSensor) = {
    .logo = "mpu6050",
    .gravity_sensor_init  = mpu6050_init,
    .gravity_sensor_check = NULL,
    .gravity_sensor_ctl   = mpu6050_ctl,
};

//未合到imu: init:gravity_sensor_init(&motion_sensor_data);
REGISTER_IMU_SENSOR(mpu6050_sensor) = {
    .logo = "mpu6050",
    .imu_sensor_init = mpu6050_init,
    .imu_sensor_check = NULL,
    .imu_sensor_ctl = mpu6050_ctl,
};

#endif //TCFG_MPU6050_EN
