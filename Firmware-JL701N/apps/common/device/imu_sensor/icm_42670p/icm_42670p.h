#ifndef _ICM_42670P_H_
#define _ICM_42670P_H_
#include "typedef.h"



#define ICM42670P

#if TCFG_ICM42670P_ENABLE

/*打开这个宏后，fifo读到的数据为
(6(acc[s16])+6(gyro[s16]))
 */
#define ICM42670P_FIFO_DATA_FIT     1


/******************************************************************
*    user config ICM42670P Macro Definition
******************************************************************/
#define  ICM42670P_USE_I2C 0 /*IIC.(<=1MHz) */
#define  ICM42670P_USE_SPI 1 /*SPI.(<=24MHz) */
#define ICM42670P_USER_INTERFACE          TCFG_ICM42670P_INTERFACE_TYPE//ICM42670P_USE_I2C
// #define ICM42670P_USER_INTERFACE           ICM42670P_USE_SPI
#define ICM42670P_SA0_IIC_ADDR            1 //1:iic模式SA0接VCC, 0:iic模式SA0接GND
//int io config
// #define ICM42670P_INT_IO1         IO_PORTB_03//TCFG_IOKEY_POWER_ONE_PORT //
// #define ICM42670P_INT_IO1         IO_PORTA_06 //
// #define ICM42670P_INT_READ_IO1()   gpio_read(ICM42670P_INT_IO1)


// #define ICM42670P_USE_FIFO_EN 1 //0:disable fifo  1:enable fifo(fifo size:512 bytes)
#define ICM42670P_USE_INT_EN 0 //0:disable //中断方式不成功,疑硬件问题
// #define ICM42670P_USE_WOM_EN 0 //0:disable
// #define ICM42670P_USE_FSYNC_EN 0 //0:disable

#define SCALED_DATA_G_DPS 0 //1:output decode data;  0:Output raw data
//解码后数据: 8(timer[u64])+12(acc[float])+12(gyro[float])+4(temp[float])
//原始数据  : 8(timer[u64])+12(acc[int32])+12(gyro[int32])+2(temp[int16])
/******************************************************************
*   ICM42670P I2C address Macro Definition
*   (7bit):    (0x37)011 0111@SDO=1;    (0x36)011 0110@SDO=0;
******************************************************************/
#if ICM42670P_SA0_IIC_ADDR
#define ICM42670P_SLAVE_ADDRESS               (0x69<<1)//0XD0
#else
#define ICM42670P_SLAVE_ADDRESS               (0x68<<1)//0XD2
#endif

typedef u32(*IMU_read)(unsigned char devAddr,
                       unsigned char regAddr,
                       unsigned char *readBuf,
                       u32 readLen);
typedef u32(*IMU_write)(unsigned char devAddr,
                        unsigned char regAddr,
                        unsigned char *writeBuf,
                        u32 writeLen);

typedef struct {
    // u8 comms;     //0:IIC;  1:SPI //改为宏控制
#if (ICM42670P_USER_INTERFACE==ICM42670P_USE_I2C)
    u8 iic_hdl;
    u8 iic_delay;    //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#elif (ICM42670P_USER_INTERFACE==ICM42670P_USE_SPI)
    u8 spi_hdl;      //SPIx (role:master)
    u8 spi_cs_pin;   //
    //u8 spi_work_mode;//icm42670p only suspport 4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;      //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;   //spi_clk: <=8MHz (spi结构体)

#endif
} icm42670p_param;














/*
 * Select communication link between SmartMotion and IMU
 */
#if (ICM42670P_USER_INTERFACE==ICM42670P_USE_I2C)
#define SERIF_TYPE           UI_I2C
#elif (ICM42670P_USER_INTERFACE==ICM42670P_USE_SPI)
#define SERIF_TYPE           UI_SPI4
// #define SERIF_TYPE           UI_SPI3
#endif
/*
 * Set power mode flag
 * Set this flag to run example in low-noise mode.
 * Reset this flag to run example in low-power mode.
 * Note: low-noise mode is not available with sensor data frequencies less than 12.5Hz.
 */
#define USE_LOW_NOISE_MODE   1
/*
 * Select Fifo resolution Mode (default is low resolution mode)
 * Low resolution mode: 16 bits data format
 * High resolution mode: 20 bits data format
 * Warning: Enabling High Res mode will force FSR to 16g and 2000dps
 */
#define USE_HIGH_RES_MODE    0
/*
 * Select to use FIFO or to read data from registers
 */
#define USE_FIFO             1

enum inv_msg_level {
    INV_MSG_LEVEL_OFF     = 0,
    INV_MSG_LEVEL_ERROR,
    INV_MSG_LEVEL_WARNING,
    INV_MSG_LEVEL_INFO,
    INV_MSG_LEVEL_VERBOSE,
    INV_MSG_LEVEL_DEBUG,
    INV_MSG_LEVEL_MAX
};



#endif /*TCFG_ICM42670P_ENABLE*/
#endif /*_ICM_42670P_H_*/

