/*****************************************************************
>
>file name : spatial_imu_data.c
>create time : Mon 03 Jan 2022 12:21:19 PM CST
*****************************************************************/
#include "typedef.h"
#include "app_config.h"
#include "gSensor/mpu6050.h"
#include "gSensor/gSensor_manage.h"
#include "spatial_effect_imu.h"
#include "imu_sensor/imuSensor_manage.h"
#include "imu_sensor/mpu6887/mpu6887p.h"
#include "imu_sensor/qmi8658/qmi8658c.h"
#include "imu_sensor/icm_42670p/icm_42670p.h"

#define MOTION_SENSOR_FIFO_ENABLE       1

struct space_data_context {
    u8 first_data;
    u8 init_state;
    u16 timeout;
    u16 timer;
#if (TCFG_SENSOR_DATA_EXPORT_ENABLE == SENSOR_DATA_EXPORT_USE_UART)
    u8 cbuf[1024];
    cbuffer_t data_cbuf;
#endif /*TCFG_SENSOR_DATA_EXPORT_ENABLE*/
};

extern int aec_uart_open(u8 nch, u16 single_size);
extern int aec_uart_fill(u8 ch, void *buf, u16 size);
extern void aec_uart_write(void);
extern int aec_uart_close(void);
void imu_sensor_power_ctl(u32 gpio, u8 value);
static s16 mpu6050_read_data(addr)
{
    u8 high;
    u8 low;

    _gravity_sensor_get_ndata(I2C_ADDR_MPU6050_R, addr, &high, 1);
    _gravity_sensor_get_ndata(I2C_ADDR_MPU6050_R, addr + 1, &low, 1);

    return (high << 8) | low;
}

static int mpu6050_fifo_read_data(void *data, int len)
{
    int remain_len = len;
    int rlen = 0;
    int total_rlen = 0;
    do {
        rlen = _gravity_sensor_get_ndata(I2C_ADDR_MPU6050_R, MPU6050_RA_FIFO_R_W, data, remain_len > 254 ? 254 : remain_len);
        remain_len -= rlen;
        total_rlen += rlen;
    } while (remain_len > 0);

    return total_rlen;
}

static int swap_data_endian(s16 *data, int len)
{
#define SWAP_16(x) ((((x)&0xFF00) >> 8) | (((x)&0x00FF) << 8))
    len >>= 1;
    for (int i = 0; i < len; i++) {
        data[i] = SWAP_16(data[i]);
    }

    return len << 1;
}
/*
 +------------------------------------------------+
 |          陀螺仪满量程与LSB灵敏度对应表         |
 +--------+--------------------+------------------+
 | FS_SEL | Full Scale Range   | LSB Sensitivity  |
 +--------+--------------------+------------------+
 |   0    |      ±250°/s       |  131 LSB/°/s     |
 +--------+--------------------+------------------+
 |   1    |      ±500°/s       |  65.5 LSB/°/s    |
 +--------+--------------------+------------------+
 |   2    |      ±1000°/s      |  32.8 LSB/°/s    |
 +--------+--------------------+------------------+
 |   3    |      ±2000°/s      |  16.4 LSB/°/s    |
 +--------+--------------------+------------------+

 +------------------------------------------------+
 |          加速计满量程与LSB灵敏度对应表         |
 +--------+--------------------+------------------+
 | FS_SEL | Full Scale Range   | LSB Sensitivity  |
 +--------+--------------------+------------------+
 |   0    |         ±2g        |    16384 LSB/g   |
 +--------+--------------------+------------------+
 |   1    |         ±4g        |     8192 LSB/g   |
 +--------+--------------------+------------------+
 |   2    |         ±8g        |     4096 LSB/g   |
 +--------+--------------------+------------------+
 |   3    |         ±16g       |     2048 LSB/g   |
 +--------+--------------------+------------------+
 */

static void space_motion_detect_timer(void *arg)
{
#if MOTION_SENSOR_FIFO_ENABLE
    s16 data_len = mpu6050_read_data(MPU6050_RA_FIFO_COUNTH);
    s16 *data = NULL;
    int i = 0;

    if (data_len) {
        data = (s16 *)malloc(data_len);
        mpu6050_fifo_read_data(data, data_len);
        swap_data_endian(data, data_len);
        printf("-- data_len : %d --\n", data_len);
        data_len /= sizeof(*data);
        s16 *print_data = data;
        for (i = 0; i < data_len; i += 3) {
            printf("[%d, %d, %d]\n", data[i], data[i + 1], data[i + 2]);
        }
        free(data);
    }
#else
    s16 gyro_xout, gyro_yout, gyro_zout;
    s16 accel_xout, accel_yout, accel_zout;
    gyro_xout = mpu6050_read_data(MPU6050_RA_GYRO_XOUT_H);
    gyro_yout = mpu6050_read_data(MPU6050_RA_GYRO_YOUT_H);
    gyro_zout = mpu6050_read_data(MPU6050_RA_GYRO_ZOUT_H);

    accel_xout = mpu6050_read_data(MPU6050_RA_ACCEL_XOUT_H);
    accel_yout = mpu6050_read_data(MPU6050_RA_ACCEL_YOUT_H);
    accel_zout = mpu6050_read_data(MPU6050_RA_ACCEL_ZOUT_H);

    printf("x : %d, y : %d, z : %d\n accel, x : %d, y : %d, z : %d\n", gyro_xout, gyro_yout, gyro_zout, accel_xout, accel_yout, accel_zout);
#endif
}

int __mpu6050_space_motion_data_read(void *sensor, void *data, int len)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;

#if 1
    if (!ctx->init_state) {
        return 0;
    }
    s16 data_len = mpu6050_read_data(MPU6050_RA_FIFO_COUNTH);
    if (data_len < 12) {
        return 0;
    }
    if (data_len > 1000) {
        printf("gsensor fifo full : %d\n", data_len);
        gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_USER_CTRL, 0x44);
    }
    data_len = (data_len / 12) * 12;

    data_len = mpu6050_fifo_read_data(data, data_len);
    if (ctx->first_data) {
        ctx->first_data = 0;
        return 0;
    }
    swap_data_endian((s16 *)data, data_len);

    return data_len;
#else
    int *rand_data = (int *)data;
    rand_data[0] = 0x12345678;
    rand_data[1] = 0x23456789;
    rand_data[2] = 0x3456789A;
    rand_data[3] = 0x456789AB;
    rand_data[4] = 0x56789ABC;
    rand_data[5] = 0x6789ABCD;
    rand_data[6] = 0x789ABCDE;
    rand_data[7] = 0x89ABCDEF;
    return 32;
#endif
}


static void space_motion_sensor_init(void *arg)
{
    struct space_data_context *ctx = (struct space_data_context *)arg;

    sys_timeout_del(ctx->timeout);
    ctx->timeout = 0;

    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_PWR_MGMT_1, 0x4);         //关闭温度传感器
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_INT_ENABLE, 0x0);         //关闭中断
#if 1
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_SMPLRT_DIV, 7);          //设置采样率, 采样率=陀螺仪输出速率/(1+SMPLRT_DIV)
#if MOTION_SENSOR_FIFO_ENABLE
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_USER_CTRL, 0x44);
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_FIFO_EN, 0x78);       // 陀螺仪xyz fifo使能，加速计fifo使能
#endif
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_CONFIG, 0x6);         // 1kHz
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_GYRO_CONFIG, 0x18);   // ±2000°/s
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_ACCEL_CONFIG, 0x19);  // ±2g

#endif
    ctx->init_state = 1;
}

static void space_motion_sensor_reset(void)
{
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_PWR_MGMT_1, 0x80);         // 复位
}

void *__mpu6050_space_motion_detect_open(void)
{
    u8 id = 0;
    _gravity_sensor_get_ndata(I2C_ADDR_MPU6050_R, MPU6050_RA_WHO_AM_I, &id, 1);
    if (id != MPU_ADDR) {
        return NULL;
    }

    struct space_data_context *ctx = (struct space_data_context *)zalloc(sizeof(struct space_data_context));
    if (!ctx) {
        return NULL;
    }

    space_motion_sensor_reset();
    ctx->timeout = sys_timeout_add((void *)ctx, space_motion_sensor_init, 100);
    ctx->first_data = 1;
    /*ctx->timer = sys_timer_add(NULL, space_motion_detect_timer, 100); */
    return ctx;
}

void __mpu6050_space_motion_detect_close(void *sensor)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;

#if 1
    if (ctx->timeout) {
        sys_timeout_del(ctx->timeout);
    }
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_USER_CTRL, 0);
    gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_FIFO_EN, 0);       // 陀螺仪xyz fifo使能，加速计fifo使能
    /*gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_PWR_MGMT_1, BIT(6));*/
#endif
    if (ctx) {
        if (ctx->timer) {
            sys_timer_del(ctx->timer);
            ctx->timer = 0;
        }
        free(ctx);
    }
}

#if TCFG_ICM42670P_ENABLE
int __icm42670p_space_motion_data_read(void *sensor, void *data, int len)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;

#if 1
    if (!ctx->init_state) {
        return 0;
    }

    u8 sensor_name[20] = "icm42670p";
    int data_len = imu_sensor_io_ctl(sensor_name, IMU_GET_SENSOR_READ_FIFO, (u8 *)data);
    if (data_len < 12) {
        return 0;
    }

    //丢掉第一次读到的数据
    if (ctx->first_data) {
        ctx->first_data = 0;
        return 0;
    }

    return data_len;
#else
    int *rand_data = (int *)data;
    rand_data[0] = 0x12345678;
    rand_data[1] = 0x23456789;
    rand_data[2] = 0x3456789A;
    rand_data[3] = 0x456789AB;
    rand_data[4] = 0x56789ABC;
    rand_data[5] = 0x6789ABCD;
    rand_data[6] = 0x789ABCDE;
    rand_data[7] = 0x89ABCDEF;
    return 32;
#endif
}

void *__icm42670p_space_motion_detect_open(void)
{
    u8 sensor_name[20] = "icm42670p";
    u8 arg_data;
    //唤醒MPU
    imu_sensor_io_ctl(sensor_name, IMU_SENSOR_WAKEUP, &arg_data);

    //检查传感器id
    int ret = imu_sensor_io_ctl(sensor_name, IMU_SENSOR_SEARCH, &arg_data);
    printf("ret : %d\n", ret);
    if (ret != 0) {
        return NULL;
    }

    struct space_data_context *ctx = (struct space_data_context *)zalloc(sizeof(struct space_data_context));
    if (!ctx) {
        return NULL;
    }

    ctx->first_data = 1;
    ctx->init_state = 1;
    printf(" __space_motion_detect_open success");

    return ctx;
}

void __icm42670p_space_motion_detect_close(void *sensor)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;
    u8 sensor_name[20] = "icm42670p";
    u8 arg_data;
#if 1
    if (ctx->timeout) {
        sys_timeout_del(ctx->timeout);
    }
#endif
    if (ctx) {
        //MPU进入睡眠
        imu_sensor_io_ctl(sensor_name, IMU_SENSOR_SLEEP, NULL);
        if (ctx->timer) {
            sys_timer_del(ctx->timer);
            ctx->timer = 0;
        }
        free(ctx);
    }
}
#endif /*TCFG_ICM42670P_ENABLE*/

#if TCFG_MPU6887P_ENABLE
/*剔除mpu6887p fifo数据中的温度数据*/
/*数据格式:acc + temp + gyro*/
static int fifo_data_delete_temp(u8 *buf, int len)
{
    int data_len = len;
    len = len / 14; //数据帧数
    buf = buf + 6;//温度数据起始位置
    int tmp = 2;
    //把每帧温度数据后面的12字节的加速度和陀螺仪依次往前移，覆盖温度数据
    for (int i = 0; i < len; i++) {
        memcpy(buf, buf + tmp, 12);
        tmp = tmp + 2;
        buf = buf + 12;
    }

    return (data_len - (len << 1));
}

int __mpu6887p_space_motion_data_read(void *sensor, void *data, int len)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;

#if 1
    if (!ctx->init_state) {
        return 0;
    }

    int data_len = mpu6887p_read_fifo_data((u8 *)data);
    if (data_len < 14) {
        return 0;
    }
    /* if (data_len > 1000) { */
    /*     printf("gsensor fifo full : %d\n", data_len); */
    /*     gravity_sensor_command(I2C_ADDR_MPU6050_W, MPU6050_RA_USER_CTRL, 0x44); */
    /* } */

    //丢掉第一次读到的数据
    if (ctx->first_data) {
        ctx->first_data = 0;
        return 0;
    }
    //交换s16数据的高低位
    swap_data_endian((s16 *)data, data_len);
    //剔除温度数据
    data_len = fifo_data_delete_temp((u8 *)data, data_len);

    return data_len;
#else
    int *rand_data = (int *)data;
    rand_data[0] = 0x12345678;
    rand_data[1] = 0x23456789;
    rand_data[2] = 0x3456789A;
    rand_data[3] = 0x456789AB;
    rand_data[4] = 0x56789ABC;
    rand_data[5] = 0x6789ABCD;
    rand_data[6] = 0x789ABCDE;
    rand_data[7] = 0x89ABCDEF;
    return 32;
#endif
}

void *__mpu6887p_space_motion_detect_open(void)
{
    u8 name_test[20] = "mpu6887p";
    u8 arg_data;
    //唤醒MPU
    mpu6887p_set_sleep_enabled(0);// 0:唤醒MPU, 1:disable
    //判断mpu6887p是否已经初始化了
    u8 status_temp = mpu6887p_read_status();
    printf("status:0x%x", status_temp);
    if ((status_temp & 0x01) == 0) {
        //如果还没有初始化，重新初始化
        imu_sensor_io_ctl(name_test, IMU_SENSOR_ENABLE, &arg_data);
    }
    //检查传感器id
    int ret = imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);
    printf("ret : %d\n", ret);
    if (ret != 0) {
        return NULL;
    }

    struct space_data_context *ctx = (struct space_data_context *)zalloc(sizeof(struct space_data_context));
    if (!ctx) {
        return NULL;
    }

    //关闭温度传感器
    mpu6887p_disable_temp_Sensor(1);
    //设置fifo使能
    mpu6887p_config_fifo(300, 1, 1, 1);
    ctx->first_data = 1;
    ctx->init_state = 1;
    printf(" __space_motion_detect_open success");

    return ctx;
}

void __mpu6887p_space_motion_detect_close(void *sensor)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;

#if 1
    if (ctx->timeout) {
        sys_timeout_del(ctx->timeout);
    }
#endif
    if (ctx) {
        //MPU进入睡眠
        mpu6887p_set_sleep_enabled(1);// 0:唤醒MPU, 1:disable
        if (ctx->timer) {
            sys_timer_del(ctx->timer);
            ctx->timer = 0;
        }
        free(ctx);
    }
}
#endif /*TCFG_MPU6887P_ENABLE*/

#if TCFG_QMI8658_ENABLE
int __qmi8658_space_motion_data_read(void *sensor, void *data, int len)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;

#if 1
    if (!ctx->init_state) {
        return 0;
    }

    u8 name_test[20] = "qmi8658";
    int data_len = imu_sensor_io_ctl(name_test, IMU_GET_SENSOR_READ_FIFO, (u8 *)data);
    if (data_len < 12) {
        return 0;
    }

    //丢掉第一次读到的数据
    if (ctx->first_data) {
        ctx->first_data = 0;
        return 0;
    }

    return data_len;
#else
    int *rand_data = (int *)data;
    rand_data[0] = 0x12345678;
    rand_data[1] = 0x23456789;
    rand_data[2] = 0x3456789A;
    rand_data[3] = 0x456789AB;
    rand_data[4] = 0x56789ABC;
    rand_data[5] = 0x6789ABCD;
    rand_data[6] = 0x789ABCDE;
    rand_data[7] = 0x89ABCDEF;
    return 32;
#endif
}

void *__qmi8658_space_motion_detect_open(void)
{
    u8 name_test[20] = "qmi8658";
    u8 arg_data;
    //唤醒MPU
    imu_sensor_io_ctl(name_test, IMU_SENSOR_WAKEUP, &arg_data);

    //检查传感器id
    int ret = imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);
    printf("ret : %d\n", ret);
    if (ret != 0) {
        return NULL;
    }

    struct space_data_context *ctx = (struct space_data_context *)zalloc(sizeof(struct space_data_context));
    if (!ctx) {
        return NULL;
    }

    //设置fifo使能
    /* u8 mytmp[4]; */
    /* mytmp[0] = 128; */
    /* mytmp[1] = Qmi8658_Fifo_128; */
    /* mytmp[2] = Qmi8658_Fifo_Stream; */
    /* mytmp[3] = QMI8658_FORMAT_12_BYTES; */
    /* imu_sensor_io_ctl(name_test, IMU_SET_SENSOR_FIFO_CONFIG, &mytmp); */

    ctx->first_data = 1;
    ctx->init_state = 1;
    printf(" __space_motion_detect_open success");

    return ctx;
}

void __qmi8658_space_motion_detect_close(void *sensor)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;
    u8 name_test[20] = "qmi8658";
    u8 arg_data;
#if 1
    if (ctx->timeout) {
        sys_timeout_del(ctx->timeout);
    }
#endif
    if (ctx) {
        //MPU进入睡眠
        imu_sensor_io_ctl("qmi8658", IMU_SENSOR_SLEEP, NULL);
        if (ctx->timer) {
            sys_timer_del(ctx->timer);
            ctx->timer = 0;
        }
        free(ctx);
    }
}
#endif /*TCFG_QMI8658_ENABLE*/

#if TCFG_LSM6DSL_ENABLE
/*交换lsm6dsl陀螺仪和加速度数据的位置*/
/*交换前数据格式:gyro + acc*/
static int swap_gyro_acc_data(s16 *buf, int len)
{
    int frames = len / 12; //数据帧数
    s16 tmp[3];
    for (int i = 0; i < frames; i++) {
        memcpy(tmp, buf, 6);
        memcpy(buf, buf + 3, 6);
        memcpy(buf + 3, tmp, 6);
        buf = buf + 6;
    }

    return len;
}

int __lsm6dsl_space_motion_data_read(void *sensor, void *data, int len)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;

#if 1
    if (!ctx->init_state) {
        return 0;
    }

    u8 sensor_name[20] = "lsm6dsl";
    int data_len = imu_sensor_io_ctl(sensor_name, IMU_GET_SENSOR_READ_FIFO, (u8 *)data);
    if (data_len < 12) {
        return 0;
    }

    //丢掉第一次读到的数据
    if (ctx->first_data) {
        ctx->first_data = 0;
        return 0;
    }

    //交换陀螺仪和加速度的数据位置
    swap_gyro_acc_data(data, len);

    return data_len;
#else
    int *rand_data = (int *)data;
    rand_data[0] = 0x12345678;
    rand_data[1] = 0x23456789;
    rand_data[2] = 0x3456789A;
    rand_data[3] = 0x456789AB;
    rand_data[4] = 0x56789ABC;
    rand_data[5] = 0x6789ABCD;
    rand_data[6] = 0x789ABCDE;
    rand_data[7] = 0x89ABCDEF;
    return 32;
#endif
}

void *__lsm6dsl_space_motion_detect_open(void)
{
    u8 sensor_name[20] = "lsm6dsl";
    u8 arg_data;
    //唤醒MPU
    imu_sensor_io_ctl(sensor_name, IMU_SENSOR_WAKEUP, &arg_data);

    //检查传感器id
    int ret = imu_sensor_io_ctl(sensor_name, IMU_SENSOR_SEARCH, &arg_data);
    printf("ret : %d\n", ret);
    if (ret != 0) {
        return NULL;
    }

    struct space_data_context *ctx = (struct space_data_context *)zalloc(sizeof(struct space_data_context));
    if (!ctx) {
        return NULL;
    }

    //设置fifo使能
    /* u8 mytmp[4]; */
    /* mytmp[0] = 128; */
    /* mytmp[1] = Qmi8658_Fifo_128; */
    /* mytmp[2] = Qmi8658_Fifo_Stream; */
    /* mytmp[3] = QMI8658_FORMAT_12_BYTES; */
    /* imu_sensor_io_ctl(sensor_name, IMU_SET_SENSOR_FIFO_CONFIG, &mytmp); */

    ctx->first_data = 1;
    ctx->init_state = 1;
    printf(" __space_motion_detect_open success");

    return ctx;
}

void __lsm6dsl_space_motion_detect_close(void *sensor)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;
    u8 sensor_name[20] = "lsm6dsl";
    u8 arg_data;
#if 1
    if (ctx->timeout) {
        sys_timeout_del(ctx->timeout);
    }
#endif
    if (ctx) {
        //MPU进入睡眠
        imu_sensor_io_ctl(sensor_name, IMU_SENSOR_SLEEP, NULL);
        if (ctx->timer) {
            sys_timer_del(ctx->timer);
            ctx->timer = 0;
        }
        free(ctx);
    }
}
#endif /*TCFG_LSM6DSL_ENABLE*/

int space_motion_data_read(void *sensor, void *data, int len)
{
    struct space_data_context *ctx = (struct space_data_context *)sensor;
    if (!ctx) {
        return 0;
    }
    int data_len = 0;
#if TCFG_ICM42670P_ENABLE
    data_len =  __icm42670p_space_motion_data_read(sensor, data, len);
#elif TCFG_LSM6DSL_ENABLE
    data_len =  __lsm6dsl_space_motion_data_read(sensor, data, len);
#elif TCFG_MPU6887P_ENABLE
    data_len =  __mpu6887p_space_motion_data_read(sensor, data, len);
#elif TCFG_QMI8658_ENABLE
    data_len =  __qmi8658_space_motion_data_read(sensor, data, len);
#elif TCFG_MPU6050_EN
    data_len =  __mpu6050_space_motion_data_read(sensor, data, len);
#endif

#if (TCFG_SENSOR_DATA_EXPORT_ENABLE == SENSOR_DATA_EXPORT_USE_UART)
    int wlen = cbuf_write(&ctx->data_cbuf, data, data_len);
    if (cbuf_get_data_size(&ctx->data_cbuf) >= 360) {
        u8 tmp_buf[512];
        cbuf_read(&ctx->data_cbuf, tmp_buf, 360);
        aec_uart_fill(0, tmp_buf, 360);
        aec_uart_write();
        putchar('|');
    }

#elif (TCFG_SENSOR_DATA_EXPORT_ENABLE == SENSOR_DATA_EXPORT_USE_SPP)
    extern int audio_data_export_run(u8 ch, u8 * data, int len);
    audio_data_export_run(0, data, data_len);
#endif /*TCFG_SENSOR_DATA_EXPORT_ENABLE*/

    return data_len;
}

void *space_motion_detect_open(void)
{
    struct space_data_context *ctx = NULL;
    /*打开传感器电源*/
    imu_sensor_power_ctl(TCFG_IMU_SENSOR_PWR_PORT, 1);
    os_time_dly(1);
#if TCFG_ICM42670P_ENABLE
    ctx = __icm42670p_space_motion_detect_open();
#elif TCFG_LSM6DSL_ENABLE
    ctx = __lsm6dsl_space_motion_detect_open();
#elif TCFG_MPU6887P_ENABLE
    ctx = __mpu6887p_space_motion_detect_open();
#elif TCFG_QMI8658_ENABLE
    ctx = __qmi8658_space_motion_detect_open();
#elif TCFG_MPU6050_EN
    ctx = __mpu6050_space_motion_detect_open();
#endif

    if (ctx) {
#if (TCFG_SENSOR_DATA_EXPORT_ENABLE == SENSOR_DATA_EXPORT_USE_UART)
        cbuf_init(&ctx->data_cbuf, ctx->cbuf, sizeof(ctx->cbuf));
        aec_uart_open(1, 360);
#endif /*TCFG_SENSOR_DATA_EXPORT_ENABLE*/
    }
    return ctx;
}

void space_motion_detect_close(void *sensor)
{
    if (!sensor) {
        return;
    }
#if TCFG_ICM42670P_ENABLE
    __icm42670p_space_motion_detect_close(sensor);
#elif TCFG_LSM6DSL_ENABLE
    __lsm6dsl_space_motion_detect_close(sensor);
#elif TCFG_MPU6887P_ENABLE
    __mpu6887p_space_motion_detect_close(sensor);
#elif TCFG_QMI8658_ENABLE
    __qmi8658_space_motion_detect_close(sensor);
#elif TCFG_MPU6050_EN
    __mpu6050_space_motion_detect_close(sensor);
#endif
    /*关闭传感器电源*/
    /* imu_sensor_power_ctl(TCFG_IMU_SENSOR_PWR_PORT, 0); */
#if (TCFG_SENSOR_DATA_EXPORT_ENABLE == SENSOR_DATA_EXPORT_USE_UART)
    aec_uart_close();
#endif /*TCFG_SENSOR_DATA_EXPORT_ENABLE*/
}

void imu_sensor_ad0_selete(u32 gpio, u8 value)
{
#if (TCFG_MPU6887P_AD0_SELETE_IO != NO_CONFIG_PORT) || \
    (TCFG_QMI8658_AD0_SELETE_IO != NO_CONFIG_PORT)
    gpio_set_die(gpio, 1);
    gpio_set_pull_up(gpio, 0);
    gpio_set_pull_down(gpio, 0);
    gpio_direction_output(gpio, value);
#endif
}
void imu_sensor_power_ctl(u32 gpio, u8 value)
{
#if (TCFG_IMU_SENSOR_PWR_PORT != NO_CONFIG_PORT)
    gpio_set_die(gpio, 1);
    gpio_set_pull_up(gpio, 0);
    gpio_set_pull_down(gpio, 0);
    gpio_direction_output(gpio, value);
#endif
}

void imu_sensor_test(void)
{
#if 0
#include "imu_sensor/imuSensor_manage.h"
#include "imu_sensor/mpu6887/mpu6887p.h"
#include "imu_sensor/lsm6dsl/lsm6dsl.h"

    /* u8 name_test[20] = "mpu6887p";  */
    /* u8 name_test[20] = "lsm6dsl"; */
    u8 name_test[20] = "icm42670p";
    int arg_data = 0;
    int ret = 0;
    //获取传感器状态
    ret = imu_sensor_io_ctl(name_test, IMU_GET_SENSOR_STATUS, &arg_data);
    printf("sta : %d\n", arg_data);
    if ((arg_data & 0x01) == 0) {
        //如果还没有初始化，重新初始化
        imu_sensor_io_ctl(name_test, IMU_SENSOR_ENABLE, &arg_data);
    }
    //检查传感器id
    ret = imu_sensor_io_ctl(name_test, IMU_SENSOR_SEARCH, &arg_data);
    extern int swap_data_endian(s16 * data, int len);
    extern int fifo_data_delete_temp(u8 * buf, int len);

    //关闭温度传感器
    arg_data = 1;
    /* ret = imu_sensor_io_ctl(name_test, IMU_SET_SENSOR_TEMP_DISABLE, &arg_data); */

    //设置fifo使能
    u8 mytmp[5];
    mytmp[0] = (1000 & 0xff);
    mytmp[1] = (1000 >> 8) & 0xff;
    mytmp[2] = 1;
    mytmp[3] = 1;
    mytmp[4] = 1;
    /* ret = imu_sensor_io_ctl(name_test, IMU_SET_SENSOR_FIFO_CONFIG, &mytmp); */

    s16 s_data[1024];
    int data_len = 0;
    //读取六轴数据
    printf("lsm6dsl_read_raw_acc_gyro_xyz");
    /* data_len = imu_sensor_io_ctl(name_test, IMU_SENSOR_READ_DATA, s_data); */
    //d读取fifo数据
    u8 cnt = 10;
    while (cnt--) {
        printf("lsm6dsl_read_fifo");
        data_len = imu_sensor_io_ctl(name_test, IMU_GET_SENSOR_READ_FIFO, s_data);
        printf("data_len: %d", data_len);
        if (data_len >= 12) {

            /* swap_data_endian((s16 *)s_data, data_len); */
            /* data_len = fifo_data_delete_temp((u8 *)s_data, data_len); */
            for (int i = 0; i < data_len / 6; i = i + 6) {
                printf("%d, %d, %d, %d, %d, %d\n", s_data[i], s_data[i + 1], s_data[i + 2], s_data[i + 3], s_data[i + 4], s_data[i + 5]);
            }

        }
        os_time_dly(50);//500ms

    }
#endif

}
