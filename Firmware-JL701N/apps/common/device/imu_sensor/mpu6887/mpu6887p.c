#include "app_config.h"
#include "asm/clock.h"
#include "asm/cpu.h"
#include "generic/typedef.h"
#include "generic/gpio.h"
#include "typedef.h"
#include "system/includes.h"
#include "media/includes.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "asm/timer.h"
#include "mpu6887p.h"
#include "imuSensor_manage.h"

/* #include "spi1.h" */
/* #include "port_wkup.h" */

#if TCFG_MPU6887P_ENABLE


/*************Betterlife ic debug***********/
#undef LOG_TAG_CONST
#define LOG_TAG             "[MPU6887P]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

void delay(volatile u32 t);
void udelay(u32 us);
#define   MDELAY(n)    mdelay(n)


static mpu6887p_param *mpu6887p_info;

// static mpu6887p_data mpu6887p_raw_data={0};

/******************************************************************
* Description:	I2C or SPI bus interface functions	and delay time function
*
* Parameters:
*   devAddr: I2C device address
*            If SPI interface, please ingnore the parameter.
*   regAddr: register address
*   readLen: data length to read
*   *readBuf: data buffer to read
*   writeLen: data length to write
*   *writeBuf: data buffer to write
*
******************************************************************/

#if (MPU6887P_USER_INTERFACE==MPU6887P_USE_I2C)

#if TCFG_MPU6887P_USER_IIC_TYPE
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
//起止信号间隔：:>1.3us
//return: readLen:ok, other:fail
u16 mpu6887p_I2C_Read_NBytes(unsigned char devAddr,
                             unsigned char regAddr,
                             unsigned char *readBuf,
                             u16 readLen)
{
    u16 i = 0;
    local_irq_disable();
    iic_start(mpu6887p_info->iic_hdl);
    if (0 == iic_tx_byte(mpu6887p_info->iic_hdl, devAddr)) {
        log_error("mpu6887p iic read err1");
        goto __iic_exit_r;
    }
    delay(mpu6887p_info->iic_delay);
    /* if (0 == iic_tx_byte(mpu6887p_info->iic_hdl, regAddr |0x80)) {//|0x80地址自动递增 */
    if (0 == iic_tx_byte(mpu6887p_info->iic_hdl, regAddr)) {//|0x80地址自动递增
        log_error("mpu6887p iic read err2");
        goto __iic_exit_r;
    }
    delay(mpu6887p_info->iic_delay);

    iic_start(mpu6887p_info->iic_hdl);
    if (0 == iic_tx_byte(mpu6887p_info->iic_hdl, devAddr + 1)) {
        log_error("mpu6887p iic read err3");
        goto __iic_exit_r;
    }
    for (i = 0; i < readLen; i++) {
        delay(mpu6887p_info->iic_delay);
        if (i == (readLen - 1)) {
            *readBuf++ = iic_rx_byte(mpu6887p_info->iic_hdl, 0);
        } else {
            *readBuf++ = iic_rx_byte(mpu6887p_info->iic_hdl, 1);
        }
        /* if(i%100==0)wdt_clear(); */
    }
__iic_exit_r:
    iic_stop(mpu6887p_info->iic_hdl);
    local_irq_enable();

    return i;
}

//起止信号间隔：:>1.3us
//return:writeLen:ok, other:fail
u16 mpu6887p_I2C_Write_NBytes(unsigned char devAddr,
                              unsigned char regAddr,
                              unsigned char *writeBuf,
                              u16 writeLen)
{
    u16 i = 0;
    local_irq_disable();
    iic_start(mpu6887p_info->iic_hdl);
    if (0 == iic_tx_byte(mpu6887p_info->iic_hdl, devAddr)) {
        log_error("mpu6887p iic write err1");
        goto __iic_exit_w;
    }
    delay(mpu6887p_info->iic_delay);
    if (0 == iic_tx_byte(mpu6887p_info->iic_hdl, regAddr)) {
        log_error("mpu6887p iic write err2");
        goto __iic_exit_w;
    }

    for (i = 0; i < writeLen; i++) {
        delay(mpu6887p_info->iic_delay);
        if (0 == iic_tx_byte(mpu6887p_info->iic_hdl, writeBuf[i])) {
            log_error("mpu6887p iic write err3:%d", i);
            goto __iic_exit_w;
        }
    }
__iic_exit_w:
    iic_stop(mpu6887p_info->iic_hdl);
    local_irq_enable();
    return i;
}
IMU_read    mpu6887p_read     = mpu6887p_I2C_Read_NBytes;
IMU_write   mpu6887p_write    = mpu6887p_I2C_Write_NBytes;

#elif (MPU6887P_USER_INTERFACE==MPU6887P_USE_SPI)
// only support 4-wire mode
#define spi_cs_init() \
    do { \
        gpio_write(mpu6887p_info->spi_cs_pin, 1); \
        gpio_set_direction(mpu6887p_info->spi_cs_pin, 0); \
        gpio_set_die(mpu6887p_info->spi_cs_pin, 1); \
    } while (0)
#define spi_cs_uninit() \
    do { \
        gpio_set_die(mpu6887p_info->spi_cs_pin, 0); \
        gpio_set_direction(mpu6887p_info->spi_cs_pin, 1); \
        gpio_set_pull_up(mpu6887p_info->spi_cs_pin, 0); \
        gpio_set_pull_down(mpu6887p_info->spi_cs_pin, 0); \
    } while (0)
#define spi_cs_h()                  gpio_write(mpu6887p_info->spi_cs_pin, 1)
#define spi_cs_l()                  gpio_write(mpu6887p_info->spi_cs_pin, 0)

#define spi_read_byte()             spi_recv_byte(mpu6887p_info->spi_hdl, NULL)
#define spi_write_byte(x)           spi_send_byte(mpu6887p_info->spi_hdl, x)
#define spi_dma_read(x, y)          spi_dma_recv(mpu6887p_info->spi_hdl, x, y)
#define spi_dma_write(x, y)         spi_dma_send(mpu6887p_info->spi_hdl, x, y)
#define spi_set_width(x)            spi_set_bit_mode(mpu6887p_info->spi_hdl, x)
#define spi_init()              spi_open(mpu6887p_info->spi_hdl)
#define spi_closed()            spi_close(mpu6887p_info->spi_hdl)
#define spi_suspend()           hw_spi_suspend(mpu6887p_info->spi_hdl)
#define spi_resume()            hw_spi_resume(mpu6887p_info->spi_hdl)

u16 mpu6887p_SPI_readNBytes(unsigned char devAddr,
                            unsigned char regAddr,
                            unsigned char *readBuf,
                            u16 readLen)
{
    spi_cs_l();
    spi_write_byte(regAddr | 0x80);//| 0x80:read mode
    spi_dma_read(readBuf, readLen);
    spi_cs_h();
    //SPIRead((regAddr | 0x80), readBuf, readLen);
    return (readLen);
}
unsigned char mpu6887p_SPI_writeByte(unsigned char devAddr,
                                     unsigned char regAddr,
                                     unsigned char writebyte)

{
    spi_cs_l();
    spi_write_byte((regAddr) & 0x7F);
    spi_write_byte(writebyte);
    spi_cs_h();
    udelay(5);//delay5us
    return (1);
}
u16 mpu6887p_SPI_writeNBytes(unsigned char devAddr,
                             unsigned char regAddr,
                             unsigned char *writeBuf,
                             u16 writeLen)
{
#if 1 //多字节dma写
    spi_cs_l();
    spi_write_byte(regAddr & 0x7F);
    spi_dma_write(writeBuf, writeLen);
    spi_cs_h();
#else
    u16 i = 0;
    spi_cs_l();
    spi_write_byte((regAddr) & 0x7F);
    for (; i < writeLen; i++) {
        spi_write_byte(writeBuf[i]);
    }
    spi_cs_h();
    // for(;i<writeLen;i++){
    // 	mpu6887p_SPI_writeByte(devAddr, regAddr+i,writeBuf[i]);
    // }
#endif
    //SPIWrite((regAddr & 0x7F), writeBuf, writeLen);
    return (writeLen);
}

IMU_read 	mpu6887p_read	= mpu6887p_SPI_readNBytes;
IMU_write	mpu6887p_write	= mpu6887p_SPI_writeNBytes;
#endif

void mpu6887p_delay(int ms)
{
    //your delay code(mSecond: millisecond):
    MDELAY(ms);
}



static unsigned short acc_lsb_div = 0;
static unsigned short gyro_lsb_div = 0;


//默认iic模式,Chip reset is also IIC mode.perform after wait for start-up time
void mpu6887p_interface_mode_set(u8 spi_en)//1:spi(4wire) ,0:iic
{
    unsigned char data = 0;
    if (spi_en) {
        data = 0x40;
    }
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_I2C_IF, &data, 1);
}

void mpu6887p_accel_temp_rst(u8 accel_rst_en, u8 temp_rst_en)//1:rst ,0:dis
{
    unsigned char data = 0;
    if (accel_rst_en) {
        data |= 0x02;
    }
    if (temp_rst_en) {
        data |= 0x01;
    }
    if (data) {
        mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_SIGNAL_PATH_RESET, &data, 1);
    }
}

void mpu6887p_fifo_enable(u8 fifo_en)//1:enable fifo ,0:dis
{
    unsigned char data = 0;
    if (fifo_en) {
        data = 0x40;
    }
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_USER_CTRL, &data, 1);
}
void mpu6887p_all_reg_rst(u8 all_reg_rst_en)//1:rst ,0:dis
{
    unsigned char data = 0x01;
    if (all_reg_rst_en) {
        mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_USER_CTRL, &data, 1);
    }
}

void mpu6887p_device_reset()
{
    unsigned char data = 0;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &data, 1);
    data |= 0x80;
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &data, 1);//

    // mpu6887p_power_up_set();
}
//return:1:ok, 0:fail
bool mpu6887p_set_sleep_enabled(u8 enable)// 0:唤醒MPU, 1:disable
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &temp_data, 1);
    if (res == 1) {
        if (enable) {
            temp_data |= BIT(6);
        } else {
            temp_data &= ~ BIT(6);
        }
        res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &temp_data, 1);
    }
    return res;
}

//上电必须执行.
void mpu6887p_power_up_set()
{
    unsigned char data = 0;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_INTEL_CTRL, &data, 1);
    data |= 0x02;
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_INTEL_CTRL, &data, 1);//OUTPUT_LIMIT

    //The default value of CLKSEL[2:0] is 001. CLKSEL[2:0] must be set to 001 to achieve full gyroscope performance.
    // data = 0;
    // mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &data, 1);
    // data &= 0xf8;
    // data |= 0x01;
    // mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &data, 1);//CLKSEL
}

enum mpu_clock_select {
    internal_20_MHz_oscillator = 0,
    Auto_selects = 1, //Auto selects the best available clock source – PLL if ready, else use the Internal oscillator
    close_clock = 7 //Stops the clock and keeps timing generator in reset
};
// The default value of CLKSEL[2:0] is 001. CLKSEL[2:0] must be set to 001 to achieve full gyroscope performance.
void mpu6887p_clock_select(enum mpu_clock_select clock)
{
    unsigned char data = 0;

    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &data, 1);
    data &= 0xf8;
    data |= clock;
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &data, 1);//clock set
}

//温度传感器默认打开
void mpu6887p_disable_temp_Sensor(unsigned char temp_disable)//1:temperature disable, 0:temperature enable
{
    u8 data = 0;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &data, 1);
    if (temp_disable) {
        data |= 0x08;
    } else {
        data &= ~ 0x08;
    }
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &data, 1);//
}

//acc / gyro 复位状态都使能
void mpu6887p_disable_acc_Sensors(u8 acc_x_disable, u8 acc_y_disable, u8 acc_z_disable)//1:disable , 0:enable
{
    u8 read_data = 0, temp_data = 0;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_2, &read_data, 1);
    temp_data = read_data;
    if ((acc_x_disable << 5) != (read_data & 0x20)) {
        read_data ^= 0x20;
    }
    if ((acc_y_disable << 4) != (read_data & 0x10)) {
        read_data ^= 0x10;
    }
    if ((acc_z_disable << 3) != (read_data & 0x08)) {
        read_data ^= 0x08;
    }
    log_info("scc en_status read data:0x%x,change:0x%x", temp_data, read_data);
    if (temp_data != read_data) {
        mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_2, &read_data, 1);//
    }
}
//acc / gyro 复位状态都使能
void mpu6887p_disable_gyro_Sensors(u8 gyro_x_disable, u8 gyro_y_disable, u8 gyro_z_disable)//1:disable , 0:enable
{
    u8 read_data = 0, temp_data = 0;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_2, &read_data, 1);
    temp_data = read_data;
    if ((gyro_x_disable << 2) != (read_data & 0x04)) {
        read_data ^= 0x04;
    }
    if ((gyro_y_disable << 1) != (read_data & 0x02)) {
        read_data ^= 0x02;
    }
    if ((gyro_z_disable) != (read_data & 0x01)) {
        read_data ^= 0x01;
    }
    log_info("gyro en_status read data:0x%x,change:0x%x", temp_data, read_data);
    if (temp_data != read_data) {
        mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_2, &read_data, 1); //
    }
}
//设置MPU加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:1,设置成功
//    0,设置失败
u8 mpu6887p_config_acc_range(u8 fsr)//fsr:0,±2g;1,±4g;2,±8g;3,±16g
{
    u8 res = 0;
    u8 temp_data = 0;
    fsr &= 0x3;
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_CONFIG, &temp_data, 1);
    if (res == 1) {
        if (((temp_data & 0x18) >> 3) != fsr) {
            SFR(temp_data, 3, 2, fsr);
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_CONFIG, &temp_data, 1);
        }
    }
    return res;
}

//设置MPU Accelerometer sensor的数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:1,设置成功
//    0,设置失败
u8 mpu6887p_set_accel_dlpf(u16 lpf)//Low-Noise Mode
{
    u8 data = 0;
    if (lpf >= 420) {
        data = 7;
    } else if (lpf >= 218) {
        data = 1;
    } else if (lpf >= 99) {
        data = 2;
    } else if (lpf >= 44) {
        data = 3;
    } else if (lpf >= 21) {
        data = 4;
    } else if (lpf >= 10) {
        data = 5;
    } else {
        data = 6;
    }

    u8 res = 0;
    u8 temp_data = 0;
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_CONFIG_2, &temp_data, 1);
    if (res == 1) {
        if ((temp_data & 0x07) != data) {
            temp_data &= ~0x07;
            temp_data |= data;
            temp_data &= ~0x08;//bit3:ACCEL_FCHOICE_B=0,才有效.
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_CONFIG_2, &temp_data, 1); //设置数字低通滤波器
        }
    }
    return res;
}
//设置MPU Accelerometer sensor的低功耗模式
//averag:Averaging filter settings for Low Power Accelerometer mode:
//		0 = Average 4 samples.
//		1 = Average 8 samples.
//		2 = Average 16 samples.
//		3 = Average 32 samples.
//返回值:1,设置成功
//    0,设置失败
//从低功耗恢复时需要重新配置ODR(rate)和dplf
u8 mpu6887p_set_accel_low_power(u8 averag)//Low Power Mode
{
    u8 data;
    u8 temp_data = 0;
    data = averag & 0x03;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_SMPLRT_DIV, &temp_data, 1);
    if (temp_data < 9) {
        if (temp_data >= 4) {
            if (data > 2) {
                data = 2;
            }
        } else if (temp_data >= 3) {
            if (data > 1) {
                data = 1;
            }
        } else {
            if (data > 0) {
                data = 0;
            }
        }
    }
    data = data << 4;
    data |= 0x07;//ACCEL_FCHOICE_B=0, A_DLPF_CFG=7

    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_CONFIG_2, &data, 1); //
    return 1;
}

//设置MPU陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:1,设置成功
//    0,设置失败
u8 mpu6887p_config_gyro_range(u8 fsr)//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
{
    u8 res = 0;
    u8 temp_data = 0;
    fsr &= 0x3;
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_GYRO_CONFIG, &temp_data, 1);
    if (res == 1) {
        if (((temp_data & 0x18) >> 3) != fsr) {
            SFR(temp_data, 3, 2, fsr);
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_GYRO_CONFIG, &temp_data, 1);
        }
    }
    return res;
}
//设置MPU gyroscope and temperature sensor的数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:1,设置成功
//    0,设置失败
u8 mpu_set_gyro_dlpf(u16 lpf)//Low-Noise Mode
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
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_CONFIG, &temp_data, 1);
    if (res == 1) {
        if ((temp_data & 0x07) != data) {
            temp_data &= ~0x07;
            temp_data |= data;
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_CONFIG, &temp_data, 1); //设置数字低通滤波器
        }
    }
    return res;
}
//设置MPU gyroscope sensor的低功耗模式
//gyro_cycle_en:1:enable low power; 0:disable low power.
//averag:Averaging filter configuration for low-power gyroscope mode. Default setting is ‘000’.
//		0 = Average 1 samples.
//		1 = Average 2 samples.
//		2 = Average 4 samples.
//		3 = Average 8 samples.
//		4:16, 5:32, 6:64, 7:128,
//返回值:1,设置成功
//    0,设置失败
u8 mpu6887p_set_gyro_low_power(u8 gyro_cycle_en, u8 averag) //Low Power Mode
{
    u8 res, data;
    u8 temp_data = 0;
    if (gyro_cycle_en == 0) {
        res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_LP_MODE_CFG, &temp_data, 1); //关闭低功耗
        return res;
    }
    data = averag & 0x07;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_SMPLRT_DIV, &temp_data, 1);
    if (temp_data < 65) {
        if (temp_data >= 33) {
            if (data > 6) {
                data = 6;
            }
        } else if (temp_data >= 17) {
            if (data > 5) {
                data = 5;
            }
        } else if (temp_data >= 9) {
            if (data > 4) {
                data = 4;
            }
        } else if (temp_data >= 6) {
            if (data > 3) {
                data = 3;
            }
        } else if (temp_data >= 3) {
            if (data > 2) {
                data = 2;
            }
        } else {
            if (data > 1) {
                data = 1;
            }
        }
    }
    data = data << 4;
    data |= 0x80;//GYRO_CYCLE=1:enable low power.
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_LP_MODE_CFG, &data, 1); //
    return 1;
}
/*
 *Divides the internal sample rate (see register CONFIG) to generate the
 sample rate that controls sensor data output rate, FIFO sample rate.
NOTE: This register is only effective when fchoice_b
register bits are 2’b00), and (0 < dlpf_cfg < 7).
This is the update rate of sensor register.
	SAMPLE_RATE= Internal_Sample_Rate / (1 + SMPLRT_DIV)
Data should be sampled at or above sample rate; SMPLRT_DIV is only used for1kHz internal sampling.
//rate:4~1000(Hz)
//设置mpu densor的采样率
//复位后fchoice_b默认00,即默认使用dlpf
//返回值:1,设置成功
//    0,设置失败
 * */
//3.91, 7.81, 15.63, 31.25, 62.50, 125, 250, 500, 1K
u8 mpu_set_sample_rate(u16 rate)// 设置采样速率.
{
    u8 res = 0;
    u8 data;
    if (rate > 1000) {
        rate = 1000;
    }
    if (rate < 4) {
        rate = 4;
    }
    data = 1000 / rate - 1;
    res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_SMPLRT_DIV, &data, 1);	//设置数字低通滤波器
    /* mpu6887p_set_accel_dlpf(rate/2);//acc dlpf */
    /* return mpu_set_gyro_dlpf(rate/2);	//自动设置gyro/temp LPF为采样率的一半  */
    // return mpu_set_dlpf(98);
    return res;
}

/* status:58(0x3A)READ to CLEAR.
 * BIT:  7     6      5      4            3    2        1     0
 *      WOM_X  WOM_Y  WOM_Z  FIFO_OFLOW   RES  Gdriver  RES   DATA_RDY
 */
unsigned char mpu6887p_read_status(void)
{
    unsigned char status;

    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_STATUS, &status, 1);
    /* log_info("status[0x%x]",status); */

    return status;
}
/*!FSYNC INTERRUPT STATUS:54(0x36)FSYNC_INT READ to CLEAR.
 * bit7:  FSYNC_INT.
 * bit6~bit0: reserved
 * \returns Status byte .
 */
unsigned char mpu6887p_read_fsync_status(void)
{
    unsigned char status;

    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_FSYNC_INT, &status, 1);
    log_info("fsync status[0x%x]\n", status);

    return status;
}

float mpu6887p_readTemp(void)
{
    unsigned char buf[2];
    short temp = 0;
    float temp_f = 0;

    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_TEMP_OUT_H, buf, 2);
    temp = ((short)buf[0] << 8) | buf[1];
    temp_f = (float)temp / 326.8f + 25;

    return temp_f;
}

void mpu6887p_read_raw_acc_xyz(void *acc_data)
{
    unsigned char	buf_reg[8];
    imu_axis_data_t *raw_acc_xyz = (imu_axis_data_t *)acc_data;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_XOUT_H, buf_reg, 8); 	// 59
    raw_acc_xyz->x = (short)((unsigned short)(buf_reg[0] << 8) | (buf_reg[1]));
    raw_acc_xyz->y = (short)((unsigned short)(buf_reg[2] << 8) | (buf_reg[3]));
    raw_acc_xyz->z = (short)((unsigned short)(buf_reg[4] << 8) | (buf_reg[5]));

    /* log_info("mpu6887p acc: %d %d %d\n", raw_acc_xyz->x, raw_acc_xyz->y, raw_acc_xyz->z); */
}

void mpu6887p_read_raw_gyro_xyz(void *gyro_data)
{
    unsigned char	buf_reg[6];
    imu_axis_data_t *raw_gyro_xyz = (imu_axis_data_t *)gyro_data;

    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_GYRO_XOUT_H, buf_reg, 6);  	// 0x3b, 59
    raw_gyro_xyz->x = (short)((unsigned short)(buf_reg[0] << 8) | (buf_reg[1]));
    raw_gyro_xyz->y = (short)((unsigned short)(buf_reg[2] << 8) | (buf_reg[3]));
    raw_gyro_xyz->z = (short)((unsigned short)(buf_reg[4] << 8) | (buf_reg[5]));

    /* log_info("mpu6887p gyro: %d %d %d\n", raw_gyro_xyz->x, raw_gyro_xyz->y, raw_gyro_xyz->z); */
}

void mpu6887p_read_raw_acc_gyro_xyz(void *raw_data)
{
    unsigned char	buf_reg[14];
    short temp = 0;
    imu_sensor_data_t *raw_sensor_data = (imu_sensor_data_t *)raw_data;
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_XOUT_H, buf_reg, 14); 	// 59
    raw_sensor_data->acc.x = (short)((unsigned short)(buf_reg[0] << 8) | (buf_reg[1]));
    raw_sensor_data->acc.y = (short)((unsigned short)(buf_reg[2] << 8) | (buf_reg[3]));
    raw_sensor_data->acc.z = (short)((unsigned short)(buf_reg[4] << 8) | (buf_reg[5]));

    raw_sensor_data->gyro.x = (short)((unsigned short)(buf_reg[8] << 8) | (buf_reg[9]));
    raw_sensor_data->gyro.y = (short)((unsigned short)(buf_reg[10] << 8) | (buf_reg[11]));
    raw_sensor_data->gyro.z = (short)((unsigned short)(buf_reg[12] << 8) | (buf_reg[13]));
    temp = ((short)buf_reg[6] << 8) | buf_reg[7];
    raw_sensor_data->temp_data = (float)temp / 326.8f + 25;
    /* log_info("mpu6887p raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_data->acc.x, raw_sensor_data->acc.y, raw_sensor_data->acc.z, raw_sensor_data->gyro.x, raw_sensor_data->gyro.y, raw_sensor_data->gyro.z); */
    /* log_info("mpu6887p temp:%d.%d\n", (u16)(raw_sensor_data->temp_data), (u16)(((u16)((raw_sensor_data->temp_data)* 100)) % 100)); */
}





//Reset FIFO module.
void mpu6887p_fifo_rst(u8 fifo_rst_en)//1:rst ,0:dis
{
    unsigned char data = 0;
    if (fifo_rst_en) {
        mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_USER_CTRL, &data, 1);
        data |= 0x04;
        mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_USER_CTRL, &data, 1);
    }
}

#if (MPU6887P_USE_FIFO_EN)
void mpu6887p_fifo_operation_enable(u8 fifo_en)//1:enable fifo ,0:dis
{
    unsigned char data = 0;
    if (fifo_en) {
        data = 0x40;
    }
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_USER_CTRL, &data, 1);
}
/*
 *设置MPU6887p fifo gyro or acc enable.
 *acc_fifo_en:
 * 		1 – Write ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H, ACCEL_YOUT_L, ACCEL_ZOUT_H,
 * 		ACCEL_ZOUT_L, TEMP_OUT_H, and TEMP_OUT_L to the FIFO at the sample rate;
 *   	0 – Function is disabled.
 *gyro_fifo_en:
 *		1 – Write TEMP_OUT_H, TEMP_OUT_L, GYRO_XOUT_H, GYRO_XOUT_L, GYRO_YOUT_H,
 *		GYRO_YOUT_L, GYRO_ZOUT_H, and GYRO_ZOUT_L to the FIFO at the sample rate;
 *		If enabled,buffering of data occurs even if data path is in standby.
 *		0 – Function is disabled.
 *
 *	 If both GYRO_FIFO_EN And ACCEL_FIFO_EN are 1, write ACCEL_XOUT_H, ACCEL_XOUT_L,
 *	 ACCEL_YOUT_H, ACCEL_YOUT_L, ACCEL_ZOUT_H, ACCEL_ZOUT_L,TEMP_OUT_H, TEMP_OUT_L,
 *	 GYRO_XOUT_H, GYRO_XOUT_L, GYRO_YOUT_H, GYRO_YOUT_L, GYRO_ZOUT_H, and GYRO_ZOUT_L
 *	  to the FIFO at the sample rate.
 * 芯片复位后为disable
 * 返回值:1,设置成功
 *  0,设置失败
 */
u8 mpu6887p_set_fifo_data_type(u8 acc_fifo_en, u8 gyro_fifo_en)//1:write data to fifo;0:disable.
{
    u8 res = 0;
    u8 temp_data = 0, read_data = 0;
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_FIFO_EN, &read_data, 1);
    temp_data = read_data;
    if (res == 1) {
        if ((read_data & 0x10) != (acc_fifo_en << 4)) {
            read_data ^= 0x10;
        }
        if ((read_data & 0x08) != (gyro_fifo_en << 3)) {
            read_data ^= 0x08;
        }
        if (temp_data != read_data) {
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_FIFO_EN, &read_data, 1);
        }
    }
    return res;
}
//设置MPU6887p fifo work mode:stream mode or fifo mode.
//fifo_mode_en:1:fifo mode ; 0:stream mode.
//芯片复位后为stream mode.
//返回值:1,设置成功
//    0,设置失败
u8 mpu6887p_set_fifo_mode(u8 fifo_mode_en)//1:fifo mode ; 0:stream mode.
{
    u8 res = 0;
    u8 temp_data = 0;
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_CONFIG, &temp_data, 1);
    if (res == 1) {
        if ((temp_data & 0x40) != (fifo_mode_en << 6)) {
            temp_data ^= 0x40;
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_CONFIG, &temp_data, 1); //
        }
    }
    return res;
}

/*
 * FIFO watermark threshold set.
 * fifo watermask interrupt enable:FIFO_WM_TH != 0.(When FIFO_WM_TH = 0, the FIFO watermark interrupt is disabled.)
 *  an interrupt is triggered: FIFO_COUNT[15:0] ≥ FIFO_WM_TH[9:0]
 * 读FIFO_R_W register清除.如果FIFO没读完,又达到fifo_wm,还会产生中断.
 *
 * watermark_level:0~1023
 * return:2:ok,other:fail.
 */
unsigned char mpu6887p_set_fifo_wm_threshold(u16 watermark_level)//watermark_level:0~1023
{
    u8 res = 0;
    u8 temp_data = 0;
    u8 write_data[2];
    if (watermark_level == 0) {
        return 0;
    }
    if (watermark_level > 1023) {
        watermark_level = 1023;
    }
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_CONFIG, &temp_data, 1);
    if (res == 1) {
        if ((temp_data & 0x80) != 0) {
            temp_data &= ~0x80;
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_CONFIG, &temp_data, 1);
        }
    }

    write_data[0] = (watermark_level >> 8) & 0x03;
    write_data[1] = (u8)(watermark_level & 0x00ff);
    res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_FIFO_WM_TH1, write_data, 2);

    return res;
}

/* When FIFO_WM_TH[9:0] = 0, the FIFO watermark interrupt is disabled.
 * watermark_level != 0 ,自动打开 fifo_wm 中断.watermark_level:0~1023
 * fifo_mode:1:fifo mode ; 0:stream mode.
 * acc_fifo_en:0:disable;1:ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H, ACCEL_YOUT_L,
 * ACCEL_ZOUT_H, ACCEL_ZOUT_L, TEMP_OUT_H, and TEMP_OUT_L to the FIFO at the sample rate;
 * gyro_fifo_en:0:disable;1:TEMP_OUT_H, TEMP_OUT_L, GYRO_XOUT_H, GYRO_XOUT_L, GYRO_YOUT_H,
 * GYRO_YOUT_L, GYRO_ZOUT_H, and GYRO_ZOUT_L to the FIFO at the sample rate;
 * If enabled,buffering of data occurs even if data path is in standby.
 *
 * acc_fifo_en=gyro_fifo_en=1:
 * 		write ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H, ACCEL_YOUT_L, ACCEL_ZOUT_H,
 * 		 ACCEL_ZOUT_L, TEMP_OUT_H, TEMP_OUT_L, GYRO_XOUT_H, GYRO_XOUT_L, GYRO_YOUT_H,
 * 		 GYRO_YOUT_L, GYRO_ZOUT_H, and GYRO_ZOUT_L to the FIFO at the sample rate.
 * */
Mpu6887p_fifo_format fifo_format = MPU6887P_FORMAT_EMPTY;
void mpu6887p_config_fifo(u16 watermark_level, u8 fifo_mode, u8 acc_fifo_en, u8 gyro_fifo_en)
{
    mpu6887p_fifo_operation_enable(1);//1:enable fifo ,0:dis
    mpu6887p_set_fifo_data_type(acc_fifo_en, gyro_fifo_en);//1:write data to fifo;0:disable.
    mpu6887p_set_fifo_mode(fifo_mode);//1:fifo mode ; 0:stream mode.

    if (watermark_level) { //watermark_level != 0 ,自动打开 fifo_wm 中断.
        mpu6887p_set_fifo_wm_threshold(watermark_level);//watermark_level:0~1023
    }

    if ((acc_fifo_en == 1) && (gyro_fifo_en == 1)) {
        fifo_format = MPU6887P_FORMAT_ACCEL_GYRO_14_BYTES; //acc + temp + gyro
    } else if (acc_fifo_en) {
        fifo_format = MPU6887P_FORMAT_ACCEL_8_BYTES; //acc + temp
    } else if (gyro_fifo_en) {
        fifo_format = MPU6887P_FORMAT_GYRO_8_BYTES; //temp + gyro
    }
}
/*
 * count indicates the number of written bytes in the FIFO.
 *
 * note: 1.Reading this byte latches the data for both FIFO_COUNTH, and FIFO_COUNTL.
 * 		2.Must read FIFO_COUNTL to latch new data for both FIFO_COUNTH and FIFO_COUNTL.
 * 		3.If the FIFO buffer is empty, reading register FIFO_DATA will return a unique value of 0xFF until new data is available
 *
 * return:0:fail(read error or fifo empty); other:ok(=fifo_count).
 */
u16 mpu6887p_read_fifo_data(u8 *buf)
{
    u16 res = 0;
    u16 fifo_level;
    u16 fifo_count;
    u8 read_data[2];

    if (fifo_format == MPU6887P_FORMAT_EMPTY) {
        log_info(" the FIFO is disabled!");
        return 0;
    }

    local_irq_disable();
    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_FIFO_COUNTH, read_data, 2); //读取过程一定不能被打断
    local_irq_enable();

    fifo_count = (u16)(read_data[0] << 8) | read_data[1];
    /* log_info("fifo count reg:%d", fifo_count); */
    if (fifo_count >= 8) {
        if (fifo_format == MPU6887P_FORMAT_ACCEL_8_BYTES || fifo_format == MPU6887P_FORMAT_GYRO_8_BYTES) {
            fifo_level = fifo_count / 8;
            fifo_count = fifo_level * 8;
            /* log_info("fifo read bytes:%d, fifo level:%d", fifo_count, fifo_level); */
        } else if (fifo_format == MPU6887P_FORMAT_ACCEL_GYRO_14_BYTES) {
            fifo_level = fifo_count / 14;
            fifo_count = fifo_level * 14;
            /* log_info("fifo read bytes:%d, fifo level:%d", fifo_count, fifo_level); */
        }
        res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_FIFO_R_W, buf, fifo_count);
    } else {
        /* log_info(" the FIFO buffer is empty!"); */
        return 0;
    }
    if (res != fifo_count) {
        log_error("read fifo fail!");
        res = 0;
    }
    return res;
}

/*
 *fifo中断:2个:fifo_wm, fifo_oflow
 *1.FIFO WATERMARK INTERRUPT STATUS:57(0x39)FIFO_WM_INT READ to CLEAR.
 * bit7:  reserved.
 * bit6: FIFO_WM_INT
 * bit6~bit0: reserved
 *
 * 2.INT_STATUS:58(0x3a):FIFO_OFLOW_INT
 * 		bit4:FIFO_OFLOW_INT
 *
 * returns fifo watermask interrupt Status byte .have no fifo overflow.
 *
 * note: Rather, whenever FIFO_R_W register is read, FIFO_WM_INT status bit is cleared automatically.
 */
unsigned char mpu6887p_read_fifo_wm_int_status(void)//读FIFO_R_W register清除.如果FIFO没读完,又达到fifo_wm,还会产生中断.
{
    unsigned char status;

    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_FIFO_WM_INT_STATUS, &status, 1);
    /* log_info("fifo watermark status(bit6)[0x%x]",status); */

    return status;
}
#endif





/* #if (MPU6887P_USE_INT_EN) */
//enum Mpu6887p_Interrupt_type{
//Mpu6887p_WOM_X_INT_EN = 0x80, /*1:Enable WoM interrupt on X-axis accelerometer. Default setting is 0. */
//Mpu6887p_WOM_Y_INT_EN = 0x40, /*1:Enable WoM interrupt on Y-axis accelerometer. Default setting is 0. */
//Mpu6887p_WOM_Z_INT_EN = 0x20, /*1:Enable WoM interrupt on Z-axis accelerometer. Default setting is 0. */
//Mpu6887p_FIFO_OFLOW_INT_EN = 0x10, /*1 – Enables a FIFO buffer overflow to generate an interrupt */
//Mpu6887p_GDRIVE_INT_EN = 0x04, /*Gyroscope Drive System Ready interrupt enable */
//Mpu6887p_DATA_RDY_INT_EN = 0x01, /*Data ready interrupt enable. */
//};

/*Mpu6887pRegister_INT_ENABLE(56)
 * int_type_en:1-enable; 0-disable
 * fifo_oflow_int_en:
 * 		1:enable; 0:disable.
 * Gdrive_int_en:
 * 		1:enable; 0:disable.
 * data_RDY_int_en:
 * 		1:enable; 0:disable.
 *
 * return:1:ok; 0:fail.
 */
u8 mpu6887p_interrupt_type_config(u8 fifo_oflow_int_en, u8 Gdrive_int_en, u8 data_RDY_int_en) //int_type_en:1-enable; 0-disable
{
    u8 res = 0;
    u8 temp_data;
    u8 write_data;

    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_ENABLE, &temp_data, 1);
    write_data = temp_data;
    if (res == 1) {
        if ((temp_data & 0x10) != (fifo_oflow_int_en << 4)) {
            write_data ^= 0x10;
        }
        if ((temp_data & 0x04) != (Gdrive_int_en << 2)) {
            write_data ^= 0x04;
        }
        if ((temp_data & 0x01) != (data_RDY_int_en << 0)) {
            write_data ^= 0x01;
        }
        if (write_data != temp_data) {
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_ENABLE, &write_data, 1);
        }
    }
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_ENABLE, &temp_data, 1);
    log_info("int_en_cfg:0x%x", temp_data);

    return res;
}
/*
 *interrupt_pin_config
 *
 * int_pin_level:
 * 		1 – The logic level for INT/DRDY pin is active low.
 * 		0 – The logic level for INT/DRDY pin is active high.
 *int_pin_open:
 *		1 – INT/DRDY pin is configured as open drain.
 *		0 – INT/DRDY pin is configured as push-pull
 *int_pin_latch_en:
 *		1 – INT/DRDY pin level held until interrupt status is cleared.
 *		0 – INT/DRDY pin indicates interrupt pulse’s width is 50 µs.
 *int_pin_clear_mode:
 * 		1 – Interrupt status is cleared if any read operation is performed.
 * 		0 – Interrupt status is cleared only by reading INT_STATUS register.
 * return:1:ok; 0:fail.
 */
u8 mpu6887p_interrupt_pin_config(u8 int_pin_level, u8 int_pin_open, u8 int_pin_latch_en, u8 int_pin_clear_mode)//
{
    u8 res = 0;
    u8 temp_data = 0;
    u8 write_data;

    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_PIN_CFG, &temp_data, 1);
    write_data = temp_data;
    if (res == 1) {
        if ((temp_data & 0x80) != (int_pin_level << 7)) {
            write_data ^= 0x80;
        }
        if ((temp_data & 0x40) != (int_pin_open << 6)) {
            write_data ^= 0x40;
        }
        if ((temp_data & 0x20) != (int_pin_latch_en << 5)) {
            write_data ^= 0x20;
        }
        if ((temp_data & 0x10) != (int_pin_clear_mode << 4)) {
            write_data ^= 0x10;
        }
        if (write_data != temp_data) {
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_PIN_CFG, &write_data, 1);
        }
    }
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_PIN_CFG, &temp_data, 1);
    log_info("int_pin_cfg:0x%x", temp_data);

    return res;
}

/*
 *interrupt_fsync_pin_config
 *
 * fsync_int_level:
 * 			1 – The logic level for the FSYNC pin as an interrupt is active low.
 * 			0 – The logic level for the FSYNC pin as an interrupt is active high.
 * fsync_int_mode_en:
 * 			When this bit is equal to 1, the FSYNC pin will trigger an interrupt when it transitions
 * 			 to the level specified by FSYNC_INT_LEVEL. When this bit is equal to 0, the FSYNC pin
 * 			 is disabled from causing an interrupt
 * return:1:ok; 0:fail.
 */
u8 mpu6887p_interrupt_fsync_pin_config(u8 fsync_int_level, u8 fsync_int_mode_en)//
{
    u8 res = 0;
    u8 temp_data = 0;
    u8 write_data;

    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_PIN_CFG, &temp_data, 1);
    write_data = temp_data;
    if (res == 1) {
        if ((temp_data & 0x08) != (fsync_int_level << 3)) {
            write_data ^= 0x08;
        }
        if ((temp_data & 0x04) != (fsync_int_mode_en << 2)) {
            write_data ^= 0x04;
        }
        if (write_data != temp_data) {
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_PIN_CFG, &write_data, 1);
        }
    }

    return res;
}
/* #endif */





#if (MPU6887P_USE_WOM_EN)

/*Mpu6887pRegister_INT_ENABLE(56)
 * int_type_en:1-enable; 0-disable
 * WOM_x_int_en:
 * 		1:enable; 0:disable.
 * WOM_y_int_en:
 * 		1:enable; 0:disable.
 * WOM_z_int_en:
 * 		1:enable; 0:disable.
 *
 * return:1:ok; 0:fail.
 */
u8 mpu6887p_interrupt_WOM_type_config(u8 WOM_x_int_en, u8 WOM_y_int_en, u8 WOM_z_int_en) //int_type_en:1-enable; 0-disable
{
    u8 res = 0;
    u8 temp_data;
    u8 write_data;

    res = mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_ENABLE, &temp_data, 1);
    write_data = temp_data;
    if (res == 1) {
        if ((temp_data & 0x80) != (WOM_x_int_en << 7)) {
            write_data ^= 0x80;
        }
        if ((temp_data & 0x40) != (WOM_y_int_en << 6)) {
            write_data ^= 0x40;
        }
        if ((temp_data & 0x20) != (WOM_z_int_en << 5)) {
            write_data ^= 0x20;
        }
        if (write_data != temp_data) {
            res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_INT_ENABLE, &write_data, 1);
        }
    }

    return res;
}

// u8 mpu6887p_accel_intel_ctrl_config()//105
// {
// }

//This register holds the threshold value for the Wake on Motion Interrupt for X/Y/Z-axis accelerometer.
//return:1:ok; 0:fail.
u8 mpu6887p_set_WOM_int_threshold(u8 acc_wom_x_thr, u8 acc_wom_y_thr, u8 acc_wom_z_thr)//
{
    u8 res = 0;

    res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_WOM_X_THR, &acc_wom_x_thr, 1);
    res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_WOM_Y_THR, &acc_wom_y_thr, 1);
    res = mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_WOM_Z_THR, &acc_wom_z_thr, 1);
    return res;
}

//only enable Accelerometer Low-Power Mode, WOM interrupt.
//the chip will cycle between sleep and taking a single accelerometer sample at a rate determined by SMPLRT_DIV.
//sample_rate:4Hz – 500Hz
u8 mpu6887p_WOM_mode_config(u8 acc_wom_x_thr, u8 acc_wom_y_thr, u8 acc_wom_z_thr, u16 sample_rate)//
{
    u8 res = 0;
    u8 temp_data = 0;

//Ensure that Accelerometer is running
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &temp_data, 1);
    temp_data &= ~0x70; //CYCLE = 0, SLEEP = 0(唤醒), and GYRO_STANDBY = 0.
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &temp_data, 1);

    mpu6887p_power_up_set();
    mpu6887p_fifo_rst(1);//1:rst ,0:dis

    mpu6887p_disable_acc_Sensors(0, 0, 0);//1:disable , 0:enable (enable acc xyz)
    mpu6887p_disable_gyro_Sensors(1, 1, 1);//1:disable , 0:enable (disable gyro xyz)
//Set Accelerometer LPF bandwidth to 218.1Hz
    mpu6887p_set_accel_dlpf(218);//
//Enable Motion Interrupt
    mpu6887p_interrupt_pin_config(1, 0, 0, 1);//active low;push-pull out;no latch;read any clear
    mpu6887p_interrupt_type_config(0, 0, 0); //fifo_oflow_int_dis, Gdrive_int_dis,data_RDY_int_dis
    mpu6887p_interrupt_WOM_type_config(1, 1, 1);//int_type_en:1-enable; 0-disable
// Set Motion Threshold
    mpu6887p_set_WOM_int_threshold(acc_wom_x_thr, acc_wom_y_thr, acc_wom_z_thr);
// Enable Accelerometer Hardware Intelligence
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &temp_data, 1);
    temp_data |= 0xc0; //enables the Wake-on-Motion detection logic, Compare the current sample with the previous sample.
    temp_data &= ~0x01; //WOM_TH_MODE 0 – Set WoM interrupt on the OR of all enabled accelerometer thresholds.
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_ACCEL_INTEL_CTRL, &temp_data, 1);
//Set Frequency of Wake-Up  3.9Hz – 500Hz
    mpu_set_sample_rate(sample_rate);// 设置采样速率.
//Enable Cycle Mode (Accelerometer Low-Power Mode)
    mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &temp_data, 1);
    temp_data |= 0x20; //CYCLE = 1
    mpu6887p_write(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_PWR_MGMT_1, &temp_data, 1);

    return res;
}
/* status:58(0x3A)READ to CLEAR.
 * BIT:  7     6      5      4            3    2        1     0
 *      WOM_X  WOM_Y  WOM_Z  FIFO_OFLOW   RES  Gdriver  RES   DATA_RDY
 *
 */
// unsigned char mpu6887p_read_status(void)
// read acc data.
#endif

//未完成:(2)
//sync
/* 26: sync */
//power
/* 107:POWER:CYCLE/GYRO_STANDBY */




unsigned char mpu6887p_init(void)
{
    unsigned char mpu6887p_chip_id = 0x00;
    unsigned char iCount = 0;

    udelay(1000);
#if (MPU6887P_USER_INTERFACE==MPU6887P_USE_SPI)
    mpu6887p_interface_mode_set(1);//使能spi_4_wire
    // mpu6887p_interface_mode_set(0);//使能IIC
#endif
    mpu6887p_device_reset();
    MDELAY(20);
    while ((mpu6887p_chip_id == 0x00) && (iCount < 2)) {
        mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_WHO_AM_I, &mpu6887p_chip_id, 1);
        if (mpu6887p_chip_id == 0x0F) {
            break;
        }
        iCount++;
    }


    if (mpu6887p_chip_id == 0x0F) {
        log_info("mpu6887p_init slave=0x%x  mpu6887pRegister_WhoAmI=0x%x\n", MPU6887P_SLAVE_ADDRESS, mpu6887p_chip_id);

        mpu6887p_set_sleep_enabled(0);// 0:唤醒MPU
        MDELAY(10);
        mpu6887p_power_up_set();

        //The default value of CLKSEL[2:0] is 001. CLKSEL[2:0] must be set to 001 to achieve full gyroscope performance.
        // mpu6887p_clock_select(Auto_selects);

        mpu6887p_config_acc_range(3);//fsr:0,±2g;1,±4g;2,±8g;3,±16g
        mpu6887p_config_gyro_range(3);//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
        mpu_set_sample_rate(100);// 设置采样速率.100hz
        mpu6887p_set_accel_dlpf(100 / 2); //acc dlpf
#if MPU6887P_6_Axis_LOW_POWER_MODE //6_Axis low power mode:
        mpu6887p_set_gyro_low_power(1, 1);
#else //low noise mode:
        mpu_set_gyro_dlpf(100 / 2);	//设置gyro/temp LPF为采样率的一半
#endif
        mpu6887p_disable_temp_Sensor(1);//默认关闭温度传感器

#if (MPU6887P_USE_FIFO_EN)
        mpu6887p_config_fifo(300, 0, 1, 1); //watermark_level:300,stream mode,acc fifo en,gyro fifo en
#endif
#if (MPU6887P_USE_INT_EN)
        mpu6887p_interrupt_type_config(1, 1, 1); //fifo_oflow_int_en, Gdrive_int_dis,data_RDY_int_en
        mpu6887p_interrupt_pin_config(0, 0, 0, 1);//active low;push-pull out;no latch;read any clear
#endif
#if (MPU6887P_USE_WOM_EN)
        mpu6887p_WOM_mode_config(200, 200, 200, 200);
#endif

#if 0
        unsigned char E_ID0[7] = {0};
        mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_E_ID0, E_ID0, 3);
        log_info("mpu6887p_E_ID0:");
        log_info_hexdump(E_ID0, 7);
#endif

        return 1;
    } else {
        log_error("mpu6887p_init fail\n");
        mpu6887p_chip_id = 0;
        return 0;
    }
}
static u8 mpu6887_int_pin = IO_PORTB_03;
//return:0:fail, 1:ok
u8 mpu6887p_sensor_init(void *priv)
{
    if (priv == NULL) {
        log_error("mpu6887p init fail(no param)\n");
        return 0;
    }
    mpu6887p_info = (mpu6887p_param *)priv;

#if (MPU6887P_USER_INTERFACE==MPU6887P_USE_I2C) //iic interface
    iic_init(mpu6887p_info->iic_hdl);
#elif (MPU6887P_USER_INTERFACE==MPU6887P_USE_SPI)//spi interface
    spi_cs_init();
    spi_init();
#endif
//int module io init
    gpio_set_die(mpu6887_int_pin, 1);
    gpio_set_direction(mpu6887_int_pin, 1);
    /* gpio_set_pull_up(mpu6887_int_pin, 1); */
    /* gpio_set_pull_down(mpu6887_int_pin, 0); */
    return mpu6887p_init();
}



static u8 init_flag = 0;
static u8 imu_busy = 0;
static mpu6887p_param mpu6887p_info_data;
volatile u8 mpu6887p_int_flag = 0;
/* #define SENSORS_MPU_BUFF_LEN 14 */
/* u8 read_mpu6887p_buf[SENSORS_MPU_BUFF_LEN]; */

void mpu6887p_int_callback()
{
    if (init_flag == 0) {
        log_error("mpu6887p init fail!");
        return ;
    }
    if (imu_busy) {
        log_error("mpu6887p busy!");
        return ;
    }
    imu_busy = 1;

    mpu6887p_int_flag = 1;
    imu_sensor_data_t raw_sensor_datas;
    float TempData = 0.0;
    u8 status_temp = 0;

    status_temp = mpu6887p_read_status();
    /* log_info("status:0x%x", status_temp); */
    if (status_temp & 0x01) {
        mpu6887p_read_raw_acc_gyro_xyz(&raw_sensor_datas);//获取原始值
        /* TempData = mpu6887p_readTemp(); */
        TempData = raw_sensor_datas.temp_data;
        log_info("mpu6887p raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_datas.acc.x, raw_sensor_datas.acc.y, raw_sensor_datas.acc.z, raw_sensor_datas.gyro.x, raw_sensor_datas.gyro.y, raw_sensor_datas.gyro.z);
        log_info("mpu6887p temp:%d.%d\n", (u16)TempData, (u16)(((u16)(TempData * 100)) % 100));
    }
    imu_busy = 0;
}

s8 mpu6887p_dev_init(void *arg)
{
    if (arg == NULL) {
        log_error("mpu6887p init fail(no arg)\n");
        return -1;
    }
#if (MPU6887P_USER_INTERFACE==MPU6887P_USE_I2C)
    mpu6887p_info_data.iic_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl;
    mpu6887p_info_data.iic_delay = ((struct imusensor_platform_data *)arg)->peripheral_param0;   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#elif (MPU6887P_USER_INTERFACE==MPU6887P_USE_SPI)
    mpu6887p_info_data.spi_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl,     //SPIx (role:master)
                       mpu6887p_info_data.spi_cs_pin = ((struct imusensor_platform_data *)arg)->peripheral_param0; //IO_PORTA_05
    // u8 port;         //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;      //spi_clk: <=1MHz (spi结构体)
#else
    //I3C
#endif
    mpu6887_int_pin = ((struct imusensor_platform_data *)arg)->imu_sensor_int_io;
    if (imu_busy) {
        log_error("mpu6887p busy!");
        return -1;
    }
    imu_busy = 1;
    if (mpu6887p_sensor_init(&mpu6887p_info_data)) {
        log_info("mpu6887p Device init success!\n");
#if (MPU6887P_USE_INT_EN) //中断模式,暂不支持
        log_info("int mode en!");
        /* port_wkup_enable(mpu6887_int_pin, 1, mpu6887p_int_callback); //PA08-IO中断，1:下降沿触发，回调函数mpu6887p_int_callback*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(mpu6887_int_pin, 1, mpu6887p_int_callback);
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, mpu6887p_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(mpu6887p_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(mpu6887p_int_callback);
#endif
#else //定时

#endif
        init_flag = 1;
        imu_busy = 0;
        return 0;
    } else {
        log_info("mpu6887p Device init fail!\n");
        imu_busy = 0;
        return -1;
    }
}


int mpu6887p_dev_ctl(u8 cmd, void *arg);
REGISTER_IMU_SENSOR(mpu6887p_sensor) = {
    .logo = "mpu6887p",
    .imu_sensor_init = mpu6887p_dev_init,
    .imu_sensor_check = NULL,
    .imu_sensor_ctl = mpu6887p_dev_ctl,
};

int mpu6887p_dev_ctl(u8 cmd, void *arg)
{
    int ret = -1;
    u8 status_temp = 0;
    if (init_flag == 0) {
        log_error("mpu6887p init fail!");
        return ret;//0:ok,,<0:err
    }
    if (imu_busy) {
        log_error("mpu6887p busy!");
        return ret;//0:ok,,<0:err
    }
    imu_busy = 1;
    switch (cmd) {
    case IMU_GET_SENSOR_NAME:
        memcpy((u8 *)arg, &(mpu6887p_sensor.logo), 20);
        ret = 0;
        break;
    case IMU_SENSOR_ENABLE:
        /* cbuf_init(&hrsensor_cbuf, hrsensorcbuf, 24 * sizeof(int)); */
        mpu6887p_init();
        ret = 0;
        break;
    case IMU_SENSOR_DISABLE:
        /* cbuf_clear(&hrsensor_cbuf); */
        /* mpu6887p_device_reset(); */
        ret = 0;
        break;
    case IMU_SENSOR_RESET:
        mpu6887p_device_reset();
        ret = 0;
        break;
    case IMU_SENSOR_SLEEP:
        if (mpu6887p_set_sleep_enabled(1)) {
            log_info("mpu6887p enter sleep ok!");
            ret = 0;
        } else {
            log_error("mpu6887p enter sleep fail!");
        }
        break;
    case IMU_SENSOR_WAKEUP:
        if (mpu6887p_set_sleep_enabled(0)) {
            log_info("mpu6887p wakeup ok!");
            ret = 0;
        } else {
            log_error("mpu6887p wakeup fail!");
        }
        break;
    case IMU_SENSOR_INT_DET://传感器中断状态检查
        break;
    case IMU_SENSOR_DATA_READY://传感器数据准备就绪待读
        /* mpu6887p_int_callback(); */
        break;
    case IMU_SENSOR_CHECK_DATA://检查传感器缓存buf是否存满
        break;
    case IMU_SENSOR_READ_DATA://默认读传感器所有数据
        status_temp = mpu6887p_read_status();
        /* log_info("status:0x%x", status_temp); */
        if (status_temp & 0x01) {
            mpu6887p_read_raw_acc_gyro_xyz(arg);//获取原始值
            ret = 0;
        }
        break;
    case IMU_GET_ACCEL_DATA://加速度数据
        /* float TempData = 0.0; */
        status_temp = mpu6887p_read_status();
        /* log_info("status:0x%x", status_temp); */
        if (status_temp & 0x01) {
            mpu6887p_read_raw_acc_xyz(arg);//获取原始值
            ret = 0;
            /* TempData = mpu6887p_readTemp(); */
        }
        break;
    case IMU_GET_GYRO_DATA://陀螺仪数据
        status_temp = mpu6887p_read_status();
        /* log_info("status:0x%x", status_temp); */
        if (status_temp & 0x01) {
            mpu6887p_read_raw_gyro_xyz(arg);//获取原始值
            ret = 0;
        }
        break;
    case IMU_GET_MAG_DATA://磁力计数据
        log_error("mpu6887p have no mag!\n");
        break;
    case IMU_SENSOR_SEARCH://检查传感器id
        mpu6887p_read(MPU6887P_SLAVE_ADDRESS, Mpu6887pRegister_WHO_AM_I, (u8 *)arg, 1); //读取MPU6887的ID
        if (*(u8 *)arg == 0x0f) {
            ret = 0;
            log_info("mpu6887p online!\n");
        } else {
            log_error("mpu6887p offline!\n");
        }
        break;
    case IMU_GET_SENSOR_STATUS://获取传感器状态
        status_temp = mpu6887p_read_status();
        *(u8 *)arg = status_temp;
        ret = 0;
        break;
    case IMU_SET_SENSOR_FIFO_CONFIG://配置传感器FIFO
        u8 *tmp = (u8 *)arg;
        u16 wm_th = tmp[0] | (tmp[1] << 8);
        u8 fifo_mode = tmp[2];
        u8 acc_en = tmp[3];
        u8 gyro_en = tmp[4];
        mpu6887p_config_fifo(wm_th, fifo_mode, acc_en, gyro_en);
        ret = 0;
        break;
    case IMU_GET_SENSOR_READ_FIFO://读取传感器FIFO数据
        status_temp = mpu6887p_read_status();
        if (status_temp & 0x01) {
            ret = mpu6887p_read_fifo_data((u8 *)arg);
        }
        break;
    case IMU_SET_SENSOR_TEMP_DISABLE://关闭温度传感器
        u8 temp = *(u8 *)arg;
        mpu6887p_disable_temp_Sensor(temp);
        ret = 0;
        break;
    default:
        log_error("--cmd err!\n");
        break;
    }
    imu_busy = 0;
    return ret;//0:ok,,<0:err

}


/***************************MPU6887P test*******************************/
#if 0 //测试
static mpu6887p_param mpu6887p_info_test = {
#if (MPU6887P_USER_INTERFACE==MPU6887P_USE_I2C)
    .iic_hdl = 0,
    .iic_delay = 0,   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
#elif (MPU6887P_USER_INTERFACE==MPU6887P_USE_SPI)
    .spi_hdl = 1,     //SPIx (role:master)
    .spi_cs_pin = IO_PORTA_05, //
#else
    //I3C
#endif
};
/********************int test*******************/
void mpu6887p_test()
{
    imu_sensor_data_t raw_sensor_datas;
    u8 status_temp = 0;
    float TempData;
    if (mpu6887p_sensor_init(&mpu6887p_info_test)) { //no fifo no int
        log_info("mpu6887p init success!\n");
        /* MDELAY(10); */
#if (MPU6887P_USE_INT_EN==0) //定时
        while (1) {
            MDELAY(500);
            status_temp = mpu6887p_read_status();
            log_info("status0:0x%x", status_temp);
            if (status_temp & 0x01) {
                mpu6887p_read_raw_acc_gyro_xyz(&raw_sensor_datas);//获取原始值
                /* TempData = mpu6887p_readTemp(); //温度 */
                TempData = raw_sensor_datas.temp_data;
                log_info("mpu6887p raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_datas.acc.x, raw_sensor_datas.acc.y, raw_sensor_datas.acc.z, raw_sensor_datas.gyro.x, raw_sensor_datas.gyro.y, raw_sensor_datas.gyro.z);
                log_info("mpu6887p temp:%d.%d\n", (u16)TempData, (u16)(((u16)(TempData * 100)) % 100));
            }
            wdt_clear();
        }
#else //中断
        //开中断
        log_info("-------------------port wkup isr---------------------------");
        /* port_wkup_enable(mpu6887_int_pin, 1, mpu6887p_int_callback);*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(mpu6887_int_pin, 1, mpu6887p_int_callback);
        /* #elif defined(CONFIG_CPU_BR28)||defined(CONFIG_CPU_BR27) */
        /*         // br28外部中断回调函数，按照现在的外部中断注册方式 */
        /*         // io配置在板级，定义在板级头文件，这里只是注册回调函数 */
        /*         port_edge_wkup_set_callback(mpu6887p_int_callback); */
        /* #endif */
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, mpu6887p_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(mpu6887p_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(mpu6887p_int_callback);
#endif
        while (1) {
            MDELAY(500);
            status_temp = mpu6887p_read_status();
            log_info("int status0:0x%x", status_temp);
            wdt_clear();
        }
#endif
    } else {
        log_error("mpu6887p init fail!\n");
    }
}
#endif

#endif

