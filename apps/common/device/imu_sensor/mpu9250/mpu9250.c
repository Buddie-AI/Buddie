#include "app_config.h"
#include "asm/clock.h"
#include "asm/cpu.h"
#include "generic/typedef.h"
#include "generic/gpio.h"
#include "mpu9250.h"
#include "typedef.h"
#include "system/includes.h"
#include "media/includes.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "asm/timer.h"
#include "imuSensor_manage.h"

#if TCFG_TP_MPU9250_ENABLE

#undef LOG_TAG_CONST
#define LOG_TAG     "[mpu9250]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

#if TCFG_MPU9250_USER_IIC_TYPE
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

static mpu9250_param *mpu9250_iic_info;
static mpu9250_data mpu9250_raw_data = {0};


#define MPU9250_INT_IO		(-1)//TCFG_TP_INT_IO //pg5
/* #define MPU9250_INT_IO		IO_PORTA_04 */
/* #define MPU9250_NCS_IO		IO_PORTA_05 */
/* #define MPU9250_FSYNC_IO	IO_PORTA_05 */
#define MPU9250_INT_R()   gpio_read(MPU9250_INT_IO)
static u8 mpu9250_int_pin = MPU9250_INT_IO;
static u8 mpu_i2c_buf_write(u8 slave_addr, u8 reg_addr,  u8 *buf, u8 len)
{
    u8 i;
    iic_start(mpu9250_iic_info->iic_hdl);
    if (0 == iic_tx_byte(mpu9250_iic_info->iic_hdl, slave_addr)) {
        iic_stop(mpu9250_iic_info->iic_hdl);
        return 0;
    }
    udelay(mpu9250_iic_info->iic_delay);
    if (0 == iic_tx_byte(mpu9250_iic_info->iic_hdl, reg_addr)) {
        iic_stop(mpu9250_iic_info->iic_hdl);
        return 0;
    }

    for (i = 0; i < len; i++) {
        udelay(mpu9250_iic_info->iic_delay);
        if (0 == iic_tx_byte(mpu9250_iic_info->iic_hdl, buf[i])) {
            iic_stop(mpu9250_iic_info->iic_hdl);
            return 0;
        }
    }
    iic_stop(mpu9250_iic_info->iic_hdl);
    return i;
}


static u8 mpu_i2c_buf_read(u8 slave_addr, u8 reg_addr, u8 *buf, u8 len)
{
    u8 i;
    iic_start(mpu9250_iic_info->iic_hdl);
    if (0 == iic_tx_byte(mpu9250_iic_info->iic_hdl, slave_addr)) {
        iic_stop(mpu9250_iic_info->iic_hdl);
        return 0;
    }
    udelay(mpu9250_iic_info->iic_delay);
    if (0 == iic_tx_byte(mpu9250_iic_info->iic_hdl, reg_addr)) {
        iic_stop(mpu9250_iic_info->iic_hdl);
        return 0;
    }
    udelay(mpu9250_iic_info->iic_delay);

    iic_start(mpu9250_iic_info->iic_hdl);
    if (0 == iic_tx_byte(mpu9250_iic_info->iic_hdl, slave_addr + 1)) {
        iic_stop(mpu9250_iic_info->iic_hdl);
        return 0;
    }
    for (i = 0; i < len; i++) {
        udelay(mpu9250_iic_info->iic_delay);
        if (i == (len - 1)) {
            *buf++ = iic_rx_byte(mpu9250_iic_info->iic_hdl, 0);
        } else {
            *buf++ = iic_rx_byte(mpu9250_iic_info->iic_hdl, 1);
        }
    }
    iic_stop(mpu9250_iic_info->iic_hdl);

    return i;
}

//设置MPU9250陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:1,设置成功
//    0,设置失败
u8 mpu6500_set_full_scale_gyro_range(u8 fsr)//0x1b(bit34)
{
    u8 res = 0;
    u8 temp_data = 0;
    fsr &= 0x3;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_GYRO_CFG_REG, &temp_data, 1);
    if (res == 1) {
        if (((temp_data & 0x18) >> 3) != fsr) {
            SFR(temp_data, 3, 2, fsr);
            res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_GYRO_CFG_REG, &temp_data, 1);
        }
    }
    return res;
}
//设置MPU9250加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:1,设置成功
//    0,设置失败
u8 mpu6500_set_full_scale_accel_range(u8 fsr)//0x1c(bit34)
{
    u8 res = 0;
    u8 temp_data = 0;
    fsr &= 0x3;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_ACCEL_CFG_REG, &temp_data, 1);
    if (res == 1) {
        if (((temp_data & 0x18) >> 3) != fsr) {
            SFR(temp_data, 3, 2, fsr);
            res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_ACCEL_CFG_REG, &temp_data, 1);
        }
    }
    return res;
}

/**
 *For the DLPF to be used, fchoice[1:0] must be set to 2’b11, fchoice_b[1:0] is 2’b00.
 See table 3 below.
The DLPF is configured by DLPF_CFG, when FCHOICE_B [1:0] = 2b’00. The gyroscope and
temperature sensor are filtered according to the value of DLPF_CFG and FCHOICE_B as shown in the table below. Note that FCHOICE mentioned in the table below is the inverted value of FCHOICE_B (e.g. FCHOICE=2b’00 is same as FCHOICE_B=2b’11).

//设置MPU9250gyroscope and temperature sensor的数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:1,设置成功
//    0,设置失败
 * */
u8 mpu_set_dlpf(u16 lpf)//0x1a(bit012)
{
    u8 data = 0;
    if (lpf >= 188) {
        data = 1;
    } else if (lpf >= 98) {
        data = 2;
    } else if (lpf >= 42) {
        data = 3;
    } else if (lpf >= 20) {
        data = 4;
    } else if (lpf >= 10) {
        data = 5;
    } else {
        data = 6;
    }

    u8 res = 0;
    u8 temp_data = 0;
    data &= 0x07;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_CFG_REG, &temp_data, 1);
    if (res == 1) {
        if ((temp_data & 0x07) != data) {
            temp_data &= ~0x07;
            temp_data |= data;
            res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_CFG_REG, &temp_data, 1); //设置数字低通滤波器
        }
    }
    return res;
}

/*
 *Divides the internal sample rate (see register CONFIG) to generate the
 sample rate that controls sensor data output rate, FIFO sample rate.
NOTE: This register is only effective when Fchoice = 2’b11 (fchoice_b
register bits are 2’b00), and (0 < dlpf_cfg < 7), such that the average filter’s
output is selected (see chart below).
This is the update rate of sensor register.
	SAMPLE_RATE= Internal_Sample_Rate / (1 + SMPLRT_DIV)
Data should be sampled at or above sample rate; SMPLRT_DIV is only used for1kHz internal sampling.
//rate:4~1000(Hz)
//设置mpu9250gyroscope的采样率(假定 Fs=1KHz)
//返回值:1,设置成功
//    0,设置失败
 * */  //0x19
u8 mpu_set_rate(u16 rate)// 设置采样速率: 1000 / (1 + rate)
{
    u8 data;
    if (rate > 1000) {
        rate = 1000;
    }
    if (rate < 4) {
        rate = 4;
    }
    data = 1000 / rate - 1;
    mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_SAMPLE_RATE_REG, &data, 1);	//设置数字低通滤波器
    /* return mpu_set_dlpf(rate/2);	//自动设置LPF为采样率的一半 */
    return mpu_set_dlpf(98);
}

bool mpu6500_set_sleep_enabled(u8 enable);// 唤醒MPU6500
//返回值:1,成功
//    0,错误代码
u8 mpu9250_init1(void *param)//bypass模式读取ak8963数据
{
    u8 res = 0, temp_data = 0;
    if (param == NULL) {
        log_info("mpu9250 init fail(no param)\n");
        return false;
    }
    mpu9250_iic_info = (mpu9250_param *)param;

    temp_data = 0X80;
    mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1); //复位MPU9250
    mdelay(100);  //延时100ms
    temp_data = 0X00;
    mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1); //唤醒MPU9250
    mdelay(10);  //延时
    mpu6500_set_full_scale_gyro_range(MPU6500_GYRO_FS_2000);//陀螺仪传感器,±2000dps
    mpu6500_set_full_scale_accel_range(0);					       	 	//加速度传感器,±2g
    mpu_set_rate(50);						       	 	//设置采样率50Hz
    mpu6500_set_sleep_enabled(false);	// 唤醒MPU6500
    temp_data = 0X00;
    mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_INT_EN_REG, &temp_data, 1); //关闭所有中断
    temp_data = 0X00;
    mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_USER_CTRL_REG, &temp_data, 1); //I2C主模式关闭
    temp_data = 0X00;
    mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_FIFO_EN_REG, &temp_data, 1);	//关闭FIFO
    temp_data = 0X82;
    mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_INTBP_CFG_REG, &temp_data, 1); //INT引脚低电平有效，开启bypass模式，可以直接读取磁力计
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_DEVICE_ID_REG, &temp_data, 1); //读取MPU6500的ID
    if (temp_data == 0x71) { //器件ID正确
        log_info("read mpu id:0x%x", temp_data);
        temp_data = 0X01;
        mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);  	//设置CLKSEL,PLL X轴为参考
        temp_data = 0X00;
        mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_PWR_MGMT2_REG, &temp_data, 1);  	//加速度与陀螺仪都工作
        mpu_set_rate(50);						       	//设置采样率为50Hz
    } else {
        log_error("read mpu id(0x%x) fail!", temp_data);
        return false;
    }

    res = mpu_i2c_buf_read(MAG_IIC_ADDRESS_W, MAG_WHO_AM_I, &temp_data, 1);    			//读取AK8963 ID
    if (temp_data == 0x48) {
        log_info("read mag id:0x%x", temp_data);
        temp_data = 0X11;
        mpu_i2c_buf_write(MAG_IIC_ADDRESS_W, MAG_CNTL_1, &temp_data, 1);		//设置AK8963为单次测量模式
    } else {
        log_error("read mag id(0x%x) fail!", temp_data);
        /* return false; */
    }
    return true;
}

//得到温度值
//temp温度值
//返回值:2,成功
//    其他,错误代码
u8 mpu_get_temperature(float *temp)
{
    u8 buf[2], res;
    short raw;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_TEMP_OUTH_REG, buf, 2);
    if (res == 2) {
        raw = ((s16)buf[0] << 8) | buf[1];
        *temp = 21 + (raw) / 333.87;
    }
    return res;
}
//得到陀螺仪值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:6,成功
//    其他,错误代码
u8 mpu_get_gyroscope(short *gx, short *gy, short *gz)
{
    u8 buf[6], res;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_GYRO_XOUTH_REG, buf, 6);
    if (res == 6) {
        *gx = ((u16)buf[0] << 8) | buf[1];
        *gy = ((u16)buf[2] << 8) | buf[3];
        *gz = ((u16)buf[4] << 8) | buf[5];
    }
    return res;
}
//得到加速度值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:6,成功
//    其他,错误代码
u8 mpu_get_accelerometer(short *ax, short *ay, short *az)
{
    u8 buf[6], res;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_ACCEL_XOUTH_REG, buf, 6);
    if (res == 6) {
        *ax = ((u16)buf[0] << 8) | buf[1];
        *ay = ((u16)buf[2] << 8) | buf[3];
        *az = ((u16)buf[4] << 8) | buf[5];
    }
    return res;
}

//得到磁力计值(原始值)
//mx,my,mz:磁力计x,y,z轴的原始读数(带符号)
//返回值:6,成功
//    其他,错误代码
u8 mpu_get_magnetometer(short *mx, short *my, short *mz)
{
    u8 buf[6], res;
    res = mpu_i2c_buf_read(MAG_IIC_ADDRESS_W, MAG_XOUT_L, buf, 6);
    if (res == 6) {
        *mx = ((u16)buf[1] << 8) | buf[0];
        *my = ((u16)buf[3] << 8) | buf[2];
        *mz = ((u16)buf[5] << 8) | buf[4];
    }
    buf[0] = 0X11;
    mpu_i2c_buf_write(MAG_IIC_ADDRESS_W, MAG_CNTL_1, buf, 1); //AK8963每次读完以后都需要重新设置为单次测量模式
    return res;
}

//iic scl:<=400KHz


#if 0

static mpu9250_param mpu9250_iic_info_test1 = {
    .iic_hdl = 0,
    .iic_delay = 0, //字节间延时，单位us
};

void mpu9250_test1()
{
    iic_init(mpu9250_iic_info_test1.iic_hdl);
    gpio_set_direction(mpu9250_int_pin, 1);
    gpio_set_die(mpu9250_int_pin, 1);
    /* gpio_set_pull_up(mpu9250_int_pin, 1); */
    /* gpio_set_pull_down(mpu9250_int_pin, 0); */

    if (mpu9250_init1(&mpu9250_iic_info_test1)) {
        log_info("mpu9250 Device init pass!\n");

        while (1) {
            mpu_get_temperature(&mpu9250_raw_data.temp_data);
            mpu_get_gyroscope(&mpu9250_raw_data.gyro_data.x_data, &mpu9250_raw_data.gyro_data.y_data, &mpu9250_raw_data.gyro_data.z_data);
            mpu_get_accelerometer(&mpu9250_raw_data.accel_data.x_data, &mpu9250_raw_data.accel_data.y_data, &mpu9250_raw_data.accel_data.z_data);
            mpu_get_magnetometer(&mpu9250_raw_data.mag_data.x_data, &mpu9250_raw_data.mag_data.y_data, &mpu9250_raw_data.mag_data.z_data);
            log_info("mpu9250_raw_data.accel_data:X:%d,Y:%d,Z:%d", mpu9250_raw_data.accel_data.x_data, mpu9250_raw_data.accel_data.y_data, mpu9250_raw_data.accel_data.z_data);
            log_info("mpu9250_raw_data.gyro_data:X:%d,Y:%d,Z:%d", mpu9250_raw_data.gyro_data.x_data, mpu9250_raw_data.gyro_data.y_data, mpu9250_raw_data.gyro_data.z_data);
            log_info("mpu9250_raw_data.mag_data:X:%d,Y:%d,Z:%d", mpu9250_raw_data.mag_data.x_data, mpu9250_raw_data.mag_data.y_data, mpu9250_raw_data.mag_data.z_data);
            log_info("mpu9250_raw_data.temp_data:%d", (s16)mpu9250_raw_data.temp_data);
            wdt_clear();
            mdelay(10);
        }
    } else {
        log_info("mpu9250 Device init fail!\n");
    }
}

#endif




/******************************mpu6500 config****************************/
//return:1:ok, 0:fail
bool mpu6500_reset()// 复位MPU6500
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);
    if (res == 1) {
        temp_data |= BIT(7);
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);
    }
    return res;
}
//return:1:ok, 0:fail
bool mpu6500_set_sleep_enabled(u8 enable)// 唤醒MPU6500
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(6);
        } else {
            temp_data &= ~ BIT(6);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);
    }
    return res;
}

/** Set clock source setting.
 * An internal 8MHz oscillator, gyroscope based clock, or external sources c&= ~ * be selected as the MPU-60X0 clock source. When the internal 8 MHz oscillat|= * or an external source is chosen as the clock source, the MPU-60X0 can operate in low power modes with the gyroscopes disabled.
 *
 * Upon power up, the MPU-60X0 clock source defaults to the internal oscillator.
 * However, it is highly recommended that the device be configured to use one of the gyroscopes (or an external clock source) as the clock reference f|= * improved stability. The clock source can be selected according to the following table:
 * CLK_SEL | Clock Source
 * --------+--------------------------------------
 * 0       | Internal oscillat|= * 1       | PLL with X Gyro reference
 * 2       | PLL with Y Gyro reference
 * 3       | PLL with Z Gyro reference
 * 4       | PLL with external 32.768kHz reference
 * 5       | PLL with external 19.2MHz reference
 * 6       | Reserved
 * 7       | Stops the clock and keeps the timing generator in reset
 * */
//return:1:ok, 0:fail
bool mpu6500_set_clocksource(u8 clocksource)// 设置X轴陀螺作为时钟
{
    u8 res = 0;
    u8 temp_data = 0;
    clocksource &= 0x07;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);
    if (res == 1) {
        if ((temp_data & 0x07) != clocksource) {
            temp_data &= ~0x07;
            temp_data |= clocksource;
            res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);
        }
    }
    return res;
}

//return:1:ok, 0:fail
bool mpu6500_set_temp_enabled(u8 enable)// 使能温度传感器
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data &= ~ BIT(3);
        } else {
            temp_data |= BIT(3);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_PWR_MGMT1_REG, &temp_data, 1);
    }
    return res;
}

//return:1:ok, 0:fail
bool mpu6500_set_int_enabled(u8 int_data)// 关闭中断
{
    u8 res = 0;
    u8 temp_data = int_data;
    res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_INT_EN_REG, &temp_data, 1);
    return res;
}
//return:1:ok, 0:fail
//数据就绪中断使能
// This event occurs each time a write operation to all of the sensor registers has been completed. Will be set 0 for disabled, 1 for enabled.
bool mpu6500SetIntDataReadyEnabled(u8 enable)//0x38(bit0)
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_INT_EN_REG, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(0);
        } else {
            temp_data &= ~ BIT(0);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_INT_EN_REG, &temp_data, 1);
    }
    return res;
}

//return:1:ok, 0:fail
bool mpu6500_set_iic_MST_enabled(u8 enable)//I2C主模式开关
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_USER_CTRL_REG, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(5);
        } else {
            temp_data &= ~ BIT(5);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_USER_CTRL_REG, &temp_data, 1);
    }
    return res;
}

//return:1:ok, 0:fail
// I2C_MST_EN (Register 106 bit[5]) has to be equal to 0
// 1:旁路模式，磁力计和其它连接到主IIC ; 0:主机模式
bool mpu6500_set_iic_bypass_enabled(u8 enable)//0x37(bit1)
{
    u8 res = 0;
    u8 temp_data = 0;
    /* mpu6500_set_iic_MST_enabled(false)//I2C主模式关 */
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_INTBP_CFG_REG, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(1);
        } else {
            temp_data &= ~ BIT(1);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_INTBP_CFG_REG, &temp_data, 1);
    }
    return res;
}

//默认0x00:rate4k
//return:1:ok, 0:fail
bool mpu6500_set_accel_DLPF(u8 dlpf_data)// 设置加速计数字低通滤波
{
    u8 res = 0;
    u8 temp_data = 0;
    dlpf_data &= 0x07;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_ACCEL_CFG_REG_2, &temp_data, 1);
    if (res == 1) {
        if ((temp_data & 0x07) != dlpf_data) {
            temp_data &= ~0x07;
            temp_data |= dlpf_data;
            temp_data |= BIT(3);//bit3=1,dlpf才才有效
            res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_ACCEL_CFG_REG_2, &temp_data, 1);
        }
    }
    return res;
}


//return:1:ok, 0:fail
// 从机读取速率: 100Hz = (1000Hz / (1 + slave_delay))
bool mpu6500SetSlave4MasterDelay(u8 slave_delay)//0x34(bit0~4)
{
    u8 res = 0;
    u8 temp_data = 0;
    slave_delay &= 0x1f;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_I2CSLV4_CTRL_REG, &temp_data, 1);
    if (res == 1) {
        if ((temp_data & 0x1f) != slave_delay) {
            temp_data &= ~0x1f;
            temp_data |= slave_delay;
            res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CSLV4_CTRL_REG, &temp_data, 1);
        }
    }
    return res;
}

//return:1:ok, 0:fail
//Set wait-for-external-sensor-data enabled value.
bool mpu6500SetWaitForExternalSensorEnabled(u8 enable)//0x24(bit6)
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_I2CMST_CTRL_REG, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(6);
        } else {
            temp_data &= ~ BIT(6);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CMST_CTRL_REG, &temp_data, 1);
    }
    return res;
}

//return:1:ok, 0:fail
//This bit controls the I2C Master’s transition from one slave read to the next
//slave read. If 0, there is a restart between reads. If 1, there is a stop between
//reads.
bool mpu6500SetSlaveReadWriteTransitionEnabled(u8 enable)
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_I2CMST_CTRL_REG, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(4);
        } else {
            temp_data &= ~ BIT(4);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CMST_CTRL_REG, &temp_data, 1);
    }
    return res;
}

//return:1:ok, 0:fail
//Set I2C master clock speed.
bool mpu6500SetMasterClockSpeed(u8 iic_master_rate)
{
    u8 res = 0;
    u8 temp_data = 0;
    iic_master_rate &= 0x0f;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_I2CMST_CTRL_REG, &temp_data, 1);
    if (res == 1) {
        temp_data &= ~ 0x0f;
        temp_data |= iic_master_rate;
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CMST_CTRL_REG, &temp_data, 1);
    }
    return res;
}

//return:1:ok, 0:fail
//bit7: Set interrupt logic level mode.(0=active-high, 1=active-low)
//bit6: Set interrupt drive mode.(0=push-pull, 1=open-drain)
//bit5: Set interrupt latch mode.(0=50us-pulse, 1=latch-until-int-cleared)
//bit4: Set interrupt latch clear mode.(0=status-read-only, 1=any-register-read)
bool mpu6500SetInterruptMode(u8 int_cfg)
{
    u8 res = 0;
    u8 temp_data = 0;
    int_cfg &= 0xf0;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_INTBP_CFG_REG, &temp_data, 1);
    if (res == 1) {
        temp_data &= ~ 0xf0;
        temp_data |= int_cfg;
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_INTBP_CFG_REG, &temp_data, 1);
    }
    return res;
}

//return:1:ok, 0:fail
bool mpu6500SetSlaveAddress(u8 num, u8 addr)
{
    u8 temp_data = 0;
    if (num > 3) {
        return false;
    }
    temp_data = addr;
    return mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CSLV0_ADDR_REG + num * 3, &temp_data, 1);
}
//return:1:ok, 0:fail
bool mpu6500SetSlaveRegister(u8 num, u8 reg)
{
    u8 temp_data = 0;
    if (num > 3) {
        return false;
    }
    temp_data = reg;
    return mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CSLV0_REG + num * 3, &temp_data, 1);
}
//return:1:ok, 0:fail
bool mpu6500SetSlaveDataLength(u8 num, u8 length)
{
    u8 res = 0;
    u8 temp_data = 0;
    if (num > 3) {
        return false;
    }
    length &= 0x0f;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_I2CSLV0_CTRL_REG + num * 3, &temp_data, 1);
    if (res == 1) {
        temp_data &= ~ 0x0f;
        temp_data |= length;
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CSLV0_CTRL_REG + num * 3, &temp_data, 1);
    }
    return res;
}
//return:1:ok, 0:fail
bool mpu6500SetSlaveEnabled(u8 num, u8 enable)
{
    u8 res = 0;
    u8 temp_data = 0;
    if (num > 3) {
        return false;
    }
    enable &= 0x0f;
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_I2CSLV0_CTRL_REG + num * 3, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(7);
        } else {
            temp_data &= ~ BIT(7);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CSLV0_CTRL_REG + num * 3, &temp_data, 1);
    }
    return res;
}

//return:1:ok, 0:fail
bool mpu6500SetSlaveDelayEnabled(u8 num, u8 enable)
{
    u8 res = 0;
    u8 temp_data = 0;
    if (num > 4) {
        return false;
    }
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_I2CMST_DELAY_REG, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(num);
        } else {
            temp_data &= ~ BIT(num);
        }
        res = mpu_i2c_buf_write(MPU9250_ADDRESS_W, MPU_I2CMST_DELAY_REG, &temp_data, 1);
    }
    return res;
}

/*设置传感器从模式读取*/
static void sensors_setup_slave_read(void)
{
    mpu6500SetSlave4MasterDelay(9);// 从机读取速率: 100Hz = (1000Hz / (1 + 9))//zxb

    mpu6500_set_iic_bypass_enabled(false);	//主机模式
    mpu6500SetWaitForExternalSensorEnabled(true);
    mpu6500SetInterruptMode(0x90);//中断低电平有效 推挽输出 中断锁存模式(0=50us-pulse) 中断清除模式(1=any-register-read)
    mpu6500SetSlaveReadWriteTransitionEnabled(false); // 关闭从机间读写结束位的传输
    mpu6500SetMasterClockSpeed(13); 	// 设置i2c速度400kHz

#if MAG_AK8963_ENABLE
    // 设置MPU6500主机要读取的寄存器
    mpu6500SetSlaveAddress(0, 0x80 | MAG_IIC_ADDRESS); 	// 设置磁力计为0号从机
    mpu6500SetSlaveRegister(0, MAG_STATE_1); 			// 从机0需要读取的寄存器
    mpu6500SetSlaveDataLength(0, SENSORS_MAG_BUFF_LEN); // 读取8个字节(ST1, x, y, z heading, ST2 (overflow check))
    mpu6500SetSlaveDelayEnabled(0, true);
    mpu6500SetSlaveEnabled(0, true);
#endif
    mpu6500_set_iic_MST_enabled(true);	//使能mpu6500主机模式
    mpu6500SetIntDataReadyEnabled(true);	//数据就绪中断使能
}



/******************************ak8963 config****************************/
//return:1:ok, 0:fail
bool ak8963_check_connection()
{
    u8 res = 0;
    u8 temp_data = 0;
    mpu_i2c_buf_read(MAG_IIC_ADDRESS_W, MAG_WHO_AM_I, &temp_data, 1); //读取AK8963 ID
    if (temp_data == 0x48) {
        log_info("read mag id:0x%x ok!", temp_data);
    } else {
        log_error("read mag id(0x%x) fail!", temp_data);
        return false;
    }
    return true;

}
/* bool ak8963_check_connection() */
/* { */
/* 	u8 res=0; */
/* 	u8 temp_data=0; */
/* 	while(1){ */
/* 		if(res==0xfe)return false; */
/* 		mpu_i2c_buf_read(res,MAG_WHO_AM_I,&temp_data,1); //读取AK8963 ID */
/* 		if(temp_data==0x48) */
/* 		{ */
/* 			log_info("read mag id:0x%x ok!",temp_data); */
/* 		}else { */
/* 			log_error("read mag id(0x%x) fail!",temp_data); */
/* 			res +=2; */
/* 			temp_data=0; */
/* 			#<{(| return false; |)}># */
/* 		} */
/* 		mdelay(10); */
/* 	} */
/* 	return true; */
/*  */
/* } */

//return:1:ok, 0:fail
bool ak8963_set_mode(u8 data)
{
    u8 temp_data = data;
    return mpu_i2c_buf_write(MAG_IIC_ADDRESS_W, MAG_CNTL_1, &temp_data, 1);
}

//return:1:ok, 0:fail
bool ak8963Reset()
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu_i2c_buf_read(MAG_IIC_ADDRESS_W, MAG_CNTL_1, &temp_data, 1);
    if (res == 1) {
        temp_data &= ~0x0f;
        res = mpu_i2c_buf_write(MAG_IIC_ADDRESS_W, MAG_CNTL_1, &temp_data, 1);
    }
    return res;
}


bool mpu9250_init2(void *param)//主模式获取ak8963
{
    u8 res = 0, temp_data = 0;
    if (param == NULL) {
        log_info("mpu9250 init fail(no param)\n");
        return false;
    }
    mpu9250_iic_info = (mpu9250_param *)param;

    mdelay(10);
    mpu6500_reset();	// 复位MPU6500
    mdelay(20);
    res = mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_DEVICE_ID_REG, &temp_data, 1); //读取MPU6500的ID
    if (temp_data == 0x71) { //器件ID正确
        log_info("read mpu id:0x%x ok!", temp_data);
    } else {
        log_error("read mpu id(0x%x) fail!", temp_data);
        return false;
    }
    mpu6500_set_sleep_enabled(false);	// 唤醒MPU6500
    mdelay(10);
    mpu6500_set_clocksource(MPU6500_CLOCK_PLL_XGYRO);// 设置X轴陀螺作为时钟
    mdelay(10);// 延时等待时钟稳定
    mpu6500_set_temp_enabled(true);// 使能温度传感器
    mpu6500_set_int_enabled(0x00);// 关闭中断

    mpu6500_set_iic_MST_enabled(false);//I2C主模式关//zxb
    mpu6500_set_iic_bypass_enabled(true);// 旁路模式，磁力计和气压连接到主IICnot sure//zxb

    mpu6500_set_full_scale_gyro_range(MPU6500_GYRO_FS_2000);	// 设置陀螺量程
    mpu6500_set_full_scale_accel_range(MPU6500_ACCEL_FS_16);// 设置加速计量程
    /* mpu6500_set_accel_DLPF(MPU6500_ACCEL_DLPF_BW_41);		// 设置加速计数字低通滤波 */

    mpu_set_rate(MPU6500_DATA_SAMPLE_RATE);//设置采样速率: 1000Hz(max)//zxb

#if MAG_AK8963_ENABLE
    if (ak8963_check_connection() == true) {
        ak8963_set_mode(AK8963_MODE_16BIT | AK8963_MODE_CONT2); // 16bit 100Hz
        log_info("AK8963 I2C connection [OK].\n");
    }	else {
        log_error("AK8963 I2C connection [FAIL].\n");
        return false;
    }
#endif

//外部中断
    mdelay(150);
    sensors_setup_slave_read();
    return true;
}


static u8 init_flag = 0;
static u8 imu_busy = 0;
static mpu9250_param mpu9250_info_data;
volatile u8 mpu9250_int_flag = 0;
#if MAG_AK8963_ENABLE
#define SENSORS_MPU_BUFF_LEN (SENSORS_MPU6500_BUFF_LEN + SENSORS_MAG_BUFF_LEN)
#else
#define SENSORS_MPU_BUFF_LEN SENSORS_MPU6500_BUFF_LEN
#endif
u8 read_mpu_buf[SENSORS_MPU_BUFF_LEN];

void mpu9250_int_callback()
{
    if (init_flag == 0) {
        log_error("mpu9250 init fail!");
        return;
    }
    if (imu_busy) {
        log_error("mpu9250 busy!");
        return;
    }
    imu_busy = 1;

    mpu9250_int_flag = 1;
    mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_ACCEL_XOUTH_REG, read_mpu_buf, SENSORS_MPU_BUFF_LEN);
#if 1
    s16 x = 0, y = 0, z = 0;
    if (mpu9250_int_flag) {/*{{{*/
        x = (((s16) read_mpu_buf[0]) << 8) | read_mpu_buf[1];
        y = ((((s16) read_mpu_buf[2]) << 8) | read_mpu_buf[3]);
        z = (((s16) read_mpu_buf[4]) << 8) | read_mpu_buf[5];
        log_info("mpu9250_raw_data.accel_data:X:%d,Y:%d,Z:%d", x, y, z);
        x = (((s16) read_mpu_buf[8]) << 8) | read_mpu_buf[9];
        y = (((s16) read_mpu_buf[10]) << 8) | read_mpu_buf[11];
        z = (((s16) read_mpu_buf[12]) << 8) | read_mpu_buf[13];
        log_info("mpu9250_raw_data.gyro_data:X:%d,Y:%d,Z:%d", x, y, z);
#if MAG_AK8963_ENABLE
        if (read_mpu_buf[0 + SENSORS_MPU6500_BUFF_LEN]&BIT(0)) {
            x = (((s16) read_mpu_buf[2 + SENSORS_MPU6500_BUFF_LEN ]) << 8) | read_mpu_buf[1 + SENSORS_MPU6500_BUFF_LEN ];
            y = (((s16) read_mpu_buf[4 + SENSORS_MPU6500_BUFF_LEN ]) << 8) | read_mpu_buf[3 + SENSORS_MPU6500_BUFF_LEN ];
            z = (((s16) read_mpu_buf[6 + SENSORS_MPU6500_BUFF_LEN ]) << 8) | read_mpu_buf[5 + SENSORS_MPU6500_BUFF_LEN ];
            log_info("mpu9250_raw_data.mag_data:X:%d,Y:%d,Z:%d", x, y, z);
        }
#endif
        x = (((s16) read_mpu_buf[6]) << 8) | read_mpu_buf[7];
        x = 21 + (s16)((float)(x) / 333.87);
        log_info("mpu9250_raw_data.temp_data:%d", x);

        mpu9250_int_flag = 0;
    }/*}}}*/
#endif
    imu_busy = 0;
}

s8 mpu9250_dev_init(void *arg)
{
    if (arg == NULL) {
        log_error("mpu9250 init fail(no arg)\n");
        return -1;
    }
#if 1//MPU9250_USER_INTERFACE_I2C
    mpu9250_info_data.iic_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl;
    mpu9250_info_data.iic_delay = ((struct imusensor_platform_data *)arg)->peripheral_param0;   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#else
    mpu9250_info_data.spi_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl,     //SPIx (role:master)
                      mpu9250_info_data.spi_cs_pin = ((struct imusensor_platform_data *)arg)->peripheral_param0; //IO_PORTA_05
    mpu9250_info_data.spi_work_mode = ((struct imusensor_platform_data *)arg)->peripheral_param1; //1:3wire(SPI_MODE_UNIDIR_1BIT) or 0:4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;         //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;      //spi_clk: <=1MHz (spi结构体)
#endif
    iic_init(mpu9250_info_data.iic_hdl);
//int module io init
    mpu9250_int_pin = ((struct imusensor_platform_data *)arg)->imu_sensor_int_io;
    gpio_set_direction(mpu9250_int_pin, 1);
    gpio_set_die(mpu9250_int_pin, 1);
    /* gpio_set_pull_up(mpu9250_int_pin, 1); */
    /* gpio_set_pull_down(mpu9250_int_pin, 0); */

    if (imu_busy) {
        log_error("mpu9250 busy!");
        return -1;
    }
    imu_busy = 1;

    if (mpu9250_init2(&mpu9250_info_data)) {
        log_info("mpu9250 Device init success!\n");
        log_info("int mode en!");
        /* port_wkup_enable(mpu9250_int_pin, 1, mpu9250_int_callback); //PA08-IO中断，1:下降沿触发，回调函数mpu9250_int_callback*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(mpu9250_int_pin, 1, mpu9250_int_callback);
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, mpu9250_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(mpu9250_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(mpu9250_int_callback);
#endif
        init_flag = 1;
        imu_busy = 0;
        return 0;
    } else {
        log_info("mpu9250 Device init fail!\n");
        imu_busy = 0;
        return -1;
    }
}


int mpu9250_dev_ctl(u8 cmd, void *arg);
REGISTER_IMU_SENSOR(mpu9250_sensor) = {
    .logo = "mpu9250",
    .imu_sensor_init = mpu9250_dev_init,
    .imu_sensor_check = NULL,
    .imu_sensor_ctl = mpu9250_dev_ctl,
};

int mpu9250_dev_ctl(u8 cmd, void *arg)
{
    int ret = -1;
    if (init_flag == 0) {
        log_error("mpu9250 init fail!");
        return ret;//0:ok,,<0:err
    }
    if (imu_busy) {
        log_error("mpu9250 busy!");
        return ret;//0:ok,,<0:err
    }
    imu_busy = 1;

    switch (cmd) {
    case IMU_GET_SENSOR_NAME:
        memcpy((u8 *)arg, &(mpu9250_sensor.logo), 20);
        break;
    case IMU_SENSOR_ENABLE:
        /* cbuf_init(&hrsensor_cbuf, hrsensorcbuf, 24 * sizeof(int)); */
        mpu9250_init2(&mpu9250_info_data);
        break;
    case IMU_SENSOR_DISABLE:
        /* cbuf_clear(&hrsensor_cbuf); */
        break;
    case IMU_SENSOR_RESET:
        break;
    case IMU_SENSOR_SLEEP:
        break;
    case IMU_SENSOR_WAKEUP:
        break;
    case IMU_SENSOR_INT_DET://传感器中断状态检查
        break;
    case IMU_SENSOR_DATA_READY://传感器数据准备就绪待读
        mpu9250_int_callback();
        break;
    case IMU_SENSOR_CHECK_DATA://检查传感器缓存buf是否存满
        break;
    case IMU_SENSOR_READ_DATA://默认读传感器所有数据
        break;
    case IMU_GET_ACCEL_DATA://加速度数据
        break;
    case IMU_GET_GYRO_DATA://陀螺仪数据
        break;
    case IMU_GET_MAG_DATA://磁力计数据
        break;
    case IMU_SENSOR_SEARCH://检查传感器id
        mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_DEVICE_ID_REG, (u8 *)arg, 1); //读取MPU9250的ID
        if (*(u8 *)arg == 0x71) {
            ret = 0;
            log_info("mpu9250 online!\n");
        } else {
            log_error("mpu9250 offline!\n");
        }
        break;
    default:
        log_error("--cmd err!\n");
        break;
    }
    imu_busy = 0;
    return ret;
}


/***************************mpu9250 test*******************************/
#if 0

static mpu9250_param mpu9250_iic_info_test2 = {
    .iic_hdl = 1,
    .iic_delay = 0, //iic字节间间隔，单位us
};

void port_wkup_irq_cbfun_test(u8 index, u8 gpio)
{
    mpu9250_int_flag = 1;
    mpu_i2c_buf_read(MPU9250_ADDRESS_W, MPU_ACCEL_XOUTH_REG, read_mpu_buf, SENSORS_MPU_BUFF_LEN);
}
void read_mpu_all_reg();
void mpu9250_test2()
{
    s16 x = 0, y = 0, z = 0;
    iic_init(mpu9250_iic_info_test2.iic_hdl);
    if (mpu9250_init2(&mpu9250_iic_info_test2)) {
        log_info("mpu9250 Device init pass!\n");
        log_info("-------------------port wkup isr---------------------------");
        /* port_wkup_enable(mpu9250_int_pin, 1, port_wkup_irq_cbfun_test); //PA08-IO中断，1:下降沿触发，回调函数port_wkup_irq_cbfun_test */
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(mpu9250_int_pin, 1, port_wkup_irq_cbfun_test);
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, port_wkup_irq_cbfun_test); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(port_wkup_irq_cbfun_test);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(port_wkup_irq_cbfun_test);
#endif

        /* read_mpu_all_reg(); */
        while (1) {
            if (mpu9250_int_flag) {
                x = (((s16) read_mpu_buf[0]) << 8) | read_mpu_buf[1];
                y = ((((s16) read_mpu_buf[2]) << 8) | read_mpu_buf[3]);
                z = (((s16) read_mpu_buf[4]) << 8) | read_mpu_buf[5];
                log_info("mpu9250_raw_data.accel_data:X:%d,Y:%d,Z:%d", x, y, z);
                x = (((s16) read_mpu_buf[8]) << 8) | read_mpu_buf[9];
                y = (((s16) read_mpu_buf[10]) << 8) | read_mpu_buf[11];
                z = (((s16) read_mpu_buf[12]) << 8) | read_mpu_buf[13];
                log_info("mpu9250_raw_data.gyro_data:X:%d,Y:%d,Z:%d", x, y, z);
#if MAG_AK8963_ENABLE
                if (read_mpu_buf[0 + SENSORS_MPU6500_BUFF_LEN]&BIT(0)) {
                    x = (((s16) read_mpu_buf[2 + SENSORS_MPU6500_BUFF_LEN ]) << 8) | read_mpu_buf[1 + SENSORS_MPU6500_BUFF_LEN ];
                    y = (((s16) read_mpu_buf[4 + SENSORS_MPU6500_BUFF_LEN ]) << 8) | read_mpu_buf[3 + SENSORS_MPU6500_BUFF_LEN ];
                    z = (((s16) read_mpu_buf[6 + SENSORS_MPU6500_BUFF_LEN ]) << 8) | read_mpu_buf[5 + SENSORS_MPU6500_BUFF_LEN ];
                    log_info("mpu9250_raw_data.mag_data:X:%d,Y:%d,Z:%d", x, y, z);
                }
#endif
                x = (((s16) read_mpu_buf[6]) << 8) | read_mpu_buf[7];
                x = 21 + (s16)((float)(x) / 333.87);
                log_info("mpu9250_raw_data.temp_data:%d", x);

                mpu9250_int_flag = 0;
            }
            wdt_clear();
            mdelay(10);
        }
    } else {
        log_info("mpu9250 Device init fail!\n");
    }
}

void read_mpu_all_reg()
{
    u8 addr_tab[62] = {0, 1, 2, 13, 14, 15, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 58, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 114, 115, 116, 117, 119, 120, 122, 123, 125, 126};
    u8 ii = 0;
    u8 temp1_data[62];
    for (ii = 0; ii < 62; ii++) {
        mpu_i2c_buf_read(MPU9250_ADDRESS_W, addr_tab[ii], &temp1_data[ii], 1);
    }
    log_info("addr:%d", addr_tab[61]);
    log_info_hexdump(temp1_data, 62);
    log_info("id:0x%x", temp1_data[55]);
}

#endif


#endif




