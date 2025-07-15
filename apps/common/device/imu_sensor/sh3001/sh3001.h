#ifndef __SH3001_H
#define __SH3001_H

#include <string.h>


#if TCFG_SH3001_ENABLE
/******************************************************************
*	 user config SH3001 Macro Definition
******************************************************************/

#define SH3001_USER_INTERFACE		TCFG_SH3001_INTERFACE_TYPE //0:I2C(<=400kHz),     1:SPI(<=1MHz)
#define SH3001_SD0_IIC_ADDR		        1 //1:iic模式SD0接VCC, 0:iic模式SD0接GND
//int io config
// #define SH3001_INT_IO1	        IO_PORTB_03//TCFG_IOKEY_POWER_ONE_PORT //IO_PORTB_01
// #define SH3001_INT_READ_IO1()   gpio_read(SH3001_INT_IO1)
#define SH3001_INT_IO2	        IO_PORTA_06
#define SH3001_INT_READ_IO2()   gpio_read(SH3001_INT_IO2)

#define TCFG_SH3001_USER_INT_EN		1 //1:中断模式读, 0:定时模式读



/******************************************************************
*	SH3001 I2C address Macro Definition
*   (7bit):    (0x37)011 0111@SDO=1;    (0x36)011 0110@SDO=0;
******************************************************************/
#if SH3001_SD0_IIC_ADDR
#define SH3001_ADDRESS				(0x37<<1)
#else
#define SH3001_ADDRESS				(0x36<<1)
#endif
/******************************************************************
*	SH3001 Registers Macro Definitions
******************************************************************/
#define SH3001_ACC_XL				(0x00)
#define SH3001_ACC_XH				(0x01)
#define SH3001_ACC_YL				(0x02)
#define SH3001_ACC_YH				(0x03)
#define SH3001_ACC_ZL				(0x04)
#define SH3001_ACC_ZH				(0x05)
#define SH3001_GYRO_XL				(0x06)
#define SH3001_GYRO_XH				(0x07)
#define SH3001_GYRO_YL				(0x08)
#define SH3001_GYRO_YH				(0x09)
#define SH3001_GYRO_ZL				(0x0A)
#define SH3001_GYRO_ZH				(0x0B)
#define SH3001_TEMP_ZL				(0x0C)
#define SH3001_TEMP_ZH				(0x0D)
#define SH3001_CHIP_ID				(0x0F)
#define SH3001_INT_STA0				(0x10)
#define SH3001_INT_STA1				(0x11)
#define SH3001_INT_STA2				(0x12)
#define SH3001_INT_STA3				(0x14)
#define SH3001_INT_STA4				(0x15)
#define SH3001_FIFO_STA0			(0x16)
#define SH3001_FIFO_STA1			(0x17)
#define SH3001_FIFO_DATA			(0x18)
#define SH3001_TEMP_CONF0			(0x20)
#define SH3001_TEMP_CONF1			(0x21)

#define SH3001_ACC_CONF0			(0x22)		// accelerometer config 0x22-0x26
#define SH3001_ACC_CONF1			(0x23)
#define SH3001_ACC_CONF2			(0x25)
#define SH3001_ACC_CONF3			(0x26)

#define SH3001_GYRO_CONF0			(0x28)		// gyroscope config 0x28-0x2B
#define SH3001_GYRO_CONF1			(0x29)
#define SH3001_GYRO_CONF2			(0x2B)

#define SH3001_SPI_CONF				(0x32)
#define SH3001_FIFO_CONF0			(0x35)
#define SH3001_FIFO_CONF1			(0x36)
#define SH3001_FIFO_CONF2			(0x37)
#define SH3001_FIFO_CONF3			(0x38)
#define SH3001_FIFO_CONF4			(0x39)
#define SH3001_MI2C_CONF0			(0x3A)
#define SH3001_MI2C_CONF1			(0x3B)
#define SH3001_MI2C_CMD0			(0x3C)
#define SH3001_MI2C_CMD1			(0x3D)
#define SH3001_MI2C_WR				(0x3E)
#define SH3001_MI2C_RD				(0x3F)
#define SH3001_INT_ENABLE0			(0x40)
#define SH3001_INT_ENABLE1			(0x41)
#define SH3001_INT_CONF				(0x44)
#define SH3001_INT_LIMIT			(0x45)
#define SH3001_ORIEN_INTCONF0		(0x46)
#define SH3001_ORIEN_INTCONF1		(0x47)
#define SH3001_ORIEN_INT_LOW		(0x48)
#define SH3001_ORIEN_INT_HIGH		(0x49)
#define SH3001_ORIEN_INT_SLOPE_LOW	(0x4A)
#define SH3001_ORIEN_INT_SLOPE_HIGH	(0x4B)
#define SH3001_ORIEN_INT_HYST_LOW	(0x4C)
#define SH3001_ORIEN_INT_HYST_HIGH	(0x4D)
#define SH3001_FLAT_INT_CONF		(0x4E)
#define SH3001_ACT_INACT_INT_CONF	(0x4F)
#define SH3001_ACT_INACT_INT_LINK	(0x50)
#define SH3001_TAP_INT_THRESHOLD	(0x51)
#define SH3001_TAP_INT_DURATION		(0x52)
#define SH3001_TAP_INT_LATENCY		(0x53)
#define SH3001_DTAP_INT_WINDOW		(0x54)
#define SH3001_ACT_INT_THRESHOLD	(0x55)
#define SH3001_ACT_INT_TIME			(0x56)
#define SH3001_INACT_INT_THRESHOLDL	(0x57)
#define SH3001_INACT_INT_TIME		(0x58)
#define SH3001_HIGHLOW_G_INT_CONF	(0x59)
#define SH3001_HIGHG_INT_THRESHOLD	(0x5A)
#define SH3001_HIGHG_INT_TIME		(0x5B)
#define SH3001_LOWG_INT_THRESHOLD	(0x5C)
#define SH3001_LOWG_INT_TIME		(0x5D)
#define SH3001_FREEFALL_INT_THRES	(0x5E)
#define SH3001_FREEFALL_INT_TIME	(0x5F)
#define SH3001_INT_PIN_MAP0			(0x79)
#define SH3001_INT_PIN_MAP1			(0x7A)
#define SH3001_INACT_INT_THRESHOLDM	(0x7B)
#define SH3001_INACT_INT_THRESHOLDH	(0x7C)
#define SH3001_INACT_INT_1G_REFL	(0x7D)
#define SH3001_INACT_INT_1G_REFH	(0x7E)
#define SH3001_SPI_REG_ACCESS		(0x7F)
#define SH3001_GYRO_CONF3			(0x8F)
#define SH3001_GYRO_CONF4			(0x9F)
#define SH3001_GYRO_CONF5			(0xAF)
#define SH3001_CHIP_ID1				(0xDF)
#define SH3001_AUX_I2C_CONF			(0xFD)


/******************************************************************
*	ACC Config Macro Definitions
******************************************************************/
#define SH3001_ODR_1000HZ		(0x00)
#define SH3001_ODR_500HZ		(0x01)
#define SH3001_ODR_250HZ		(0x02)
#define SH3001_ODR_125HZ		(0x03)
#define SH3001_ODR_63HZ			(0x04)
#define SH3001_ODR_31HZ			(0x05)
#define SH3001_ODR_16HZ			(0x06)
#define SH3001_ODR_2000HZ		(0x08)
#define SH3001_ODR_4000HZ		(0x09)
#define SH3001_ODR_8000HZ		(0x0A)
#define SH3001_ODR_16000HZ		(0x0B)
#define SH3001_ODR_32000HZ		(0x0C)

#define SH3001_ACC_RANGE_16G	(0x02)
#define SH3001_ACC_RANGE_8G		(0x03)
#define SH3001_ACC_RANGE_4G		(0x04)
#define SH3001_ACC_RANGE_2G		(0x05)

#define SH3001_ACC_ODRX040		(0x00)
#define SH3001_ACC_ODRX025		(0x20)
#define SH3001_ACC_ODRX011		(0x40)
#define SH3001_ACC_ODRX004		(0x60)
#define SH3001_ACC_ODRX002		(0x80)

#define SH3001_ACC_FILTER_EN	(0x00)
#define SH3001_ACC_FILTER_DIS	(0x08)

/******************************************************************
*	GYRO Config Macro Definitions
******************************************************************/
#define SH3001_GYRO_RANGE_125	(0x02)
#define SH3001_GYRO_RANGE_250	(0x03)
#define SH3001_GYRO_RANGE_500	(0x04)
#define SH3001_GYRO_RANGE_1000	(0x05)
#define SH3001_GYRO_RANGE_2000	(0x06)

#define SH3001_GYRO_ODRX00		(0x00)
#define SH3001_GYRO_ODRX01		(0x04)
#define SH3001_GYRO_ODRX02		(0x08)
#define SH3001_GYRO_ODRX03		(0x0C)

#define SH3001_GYRO_FILTER_EN	(0x00)
#define SH3001_GYRO_FILTER_DIS	(0x10)


/******************************************************************
*	Temperature Config Macro Definitions
******************************************************************/
#define SH3001_TEMP_ODR_500		(0x00)
#define SH3001_TEMP_ODR_250		(0x10)
#define SH3001_TEMP_ODR_125		(0x20)
#define SH3001_TEMP_ODR_63		(0x30)

#define SH3001_TEMP_EN			(0x80)
#define SH3001_TEMP_DIS			(0x00)

/******************************************************************
*	INT Config Macro Definitions
******************************************************************/
#define SH3001_INT_LOWG				(0x8000)
#define SH3001_INT_HIGHG			(0x4000)
#define SH3001_INT_INACT			(0x2000)
#define SH3001_INT_ACT				(0x1000)
#define SH3001_INT_DOUBLE_TAP	  	(0x0800)
#define SH3001_INT_SINGLE_TAP	  	(0x0400)
#define SH3001_INT_FLAT				(0x0200)
#define SH3001_INT_ORIENTATION		(0x0100)
#define SH3001_INT_FIFO_GYRO		(0x0010)
#define SH3001_INT_GYRO_READY		(0x0008)
#define SH3001_INT_ACC_FIFO			(0x0004)
#define SH3001_INT_ACC_READY		(0x0002)
#define SH3001_INT_FREE_FALL		(0x0001)
#define SH3001_INT_UP_DOWN_Z    	(0x0040)


#define SH3001_INT_ENABLE			(0x01)
#define SH3001_INT_DISABLE			(0x00)

#define SH3001_INT_MAP_INT1			(0x01)
#define SH3001_INT_MAP_INT			(0x00)

#define SH3001_INT0_LEVEL_LOW		(0x80)
#define SH3001_INT0_LEVEL_HIGH		(0x7F)
#define SH3001_INT_NO_LATCH			(0x40)
#define SH3001_INT_LATCH			(0xBF)
#define SH3001_INT1_LEVEL_LOW		(0x20)
#define SH3001_INT1_LEVEL_HIGH		(0xDF)
#define SH3001_INT_CLEAR_ANY		(0x10)
#define SH3001_INT_CLEAR_STATUS	    (0xEF)
#define SH3001_INT1_NORMAL	    	(0x04)
#define SH3001_INT1_OD				(0xFB)
#define SH3001_INT0_NORMAL			(0x01)
#define SH3001_INT0_OD				(0xFE)

/******************************************************************
*	Orientation Blocking Config Macro Definitions
******************************************************************/
#define SH3001_ORIENT_BLOCK_MODE0	(0x00)
#define SH3001_ORIENT_BLOCK_MODE1	(0x04)
#define SH3001_ORIENT_BLOCK_MODE2	(0x08)
#define SH3001_ORIENT_BLOCK_MODE3	(0x0C)

#define SH3001_ORIENT_SYMM			(0x00)
#define SH3001_ORIENT_HIGH_ASYMM	(0x01)
#define SH3001_ORIENT_LOW_ASYMM		(0x02)


/******************************************************************
*	Flat Time Config Macro Definitions
******************************************************************/
#define SH3001_FLAT_TIME_500MS		(0x40)
#define SH3001_FLAT_TIME_1000MS		(0x80)
#define SH3001_FLAT_TIME_2000MS		(0xC0)


/******************************************************************
*	ACT and INACT Int Config Macro Definitions
******************************************************************/
#define SH3001_ACT_AC_MODE		(0x80)
#define SH3001_ACT_DC_MODE		(0x00)
#define SH3001_ACT_X_INT_EN		(0x40)
#define SH3001_ACT_X_INT_DIS	(0x00)
#define SH3001_ACT_Y_INT_EN		(0x20)
#define SH3001_ACT_Y_INT_DIS	(0x00)
#define SH3001_ACT_Z_INT_EN		(0x10)
#define SH3001_ACT_Z_INT_DIS	(0x00)


#define SH3001_INACT_AC_MODE	(0x08)
#define SH3001_INACT_DC_MODE	(0x00)
#define SH3001_INACT_X_INT_EN	(0x04)
#define SH3001_INACT_X_INT_DIS	(0x00)
#define SH3001_INACT_Y_INT_EN	(0x02)
#define SH3001_INACT_Y_INT_DIS	(0x00)
#define SH3001_INACT_Z_INT_EN	(0x01)
#define SH3001_INACT_Z_INT_DIS	(0x00)


#define SH3001_LINK_PRE_STA		(0x01)
#define SH3001_LINK_PRE_STA_NO	(0x00)


/******************************************************************
*	TAP Int Config Macro Definitions
******************************************************************/
#define SH3001_TAP_X_INT_EN		(0x08)
#define SH3001_TAP_X_INT_DIS	(0x00)
#define SH3001_TAP_Y_INT_EN		(0x04)
#define SH3001_TAP_Y_INT_DIS	(0x00)
#define SH3001_TAP_Z_INT_EN		(0x02)
#define SH3001_TAP_Z_INT_DIS	(0x00)


/******************************************************************
*	HIGHG Int Config Macro Definitions
******************************************************************/
#define SH3001_HIGHG_ALL_INT_EN		(0x80)
#define SH3001_HIGHG_ALL_INT_DIS	(0x00)
#define SH3001_HIGHG_X_INT_EN		(0x40)
#define SH3001_HIGHG_X_INT_DIS		(0x00)
#define SH3001_HIGHG_Y_INT_EN		(0x20)
#define SH3001_HIGHG_Y_INT_DIS		(0x00)
#define SH3001_HIGHG_Z_INT_EN		(0x10)
#define SH3001_HIGHG_Z_INT_DIS		(0x00)


/******************************************************************
*	LOWG Int Config Macro Definitions
******************************************************************/
#define SH3001_LOWG_ALL_INT_EN		(0x01)
#define SH3001_LOWG_ALL_INT_DIS		(0x00)


/******************************************************************
*	SPI Interface Config Macro Definitions
******************************************************************/
#define SH3001_SPI_3_WIRE		(0x01)
#define SH3001_SPI_4_WIRE		(0x00)



/******************************************************************
*	FIFO Config Macro Definitions
******************************************************************/
#define SH3001_FIFO_MODE_DIS		(0x00)
#define SH3001_FIFO_MODE_FIFO		(0x01)
#define SH3001_FIFO_MODE_STREAM		(0x02)
#define SH3001_FIFO_MODE_TRIGGER	(0x03)

#define SH3001_FIFO_ACC_DOWNS_EN	(0x80)
#define SH3001_FIFO_ACC_DOWNS_DIS	(0x00)
#define SH3001_FIFO_GYRO_DOWNS_EN	(0x08)
#define SH3001_FIFO_GYRO_DOWNS_DIS	(0x00)

#define SH3001_FIFO_FREQ_X1_2		(0x00)
#define SH3001_FIFO_FREQ_X1_4		(0x01)
#define SH3001_FIFO_FREQ_X1_8		(0x02)
#define SH3001_FIFO_FREQ_X1_16		(0x03)
#define SH3001_FIFO_FREQ_X1_32		(0x04)
#define SH3001_FIFO_FREQ_X1_64		(0x05)
#define SH3001_FIFO_FREQ_X1_128		(0x06)
#define SH3001_FIFO_FREQ_X1_256		(0x07)

#define SH3001_FIFO_EXT_Z_EN		(0x2000)
#define SH3001_FIFO_EXT_Y_EN		(0x1000)
#define SH3001_FIFO_EXT_X_EN		(0x0080)
#define SH3001_FIFO_TEMPERATURE_EN	(0x0040)
#define SH3001_FIFO_GYRO_Z_EN		(0x0020)
#define SH3001_FIFO_GYRO_Y_EN		(0x0010)
#define SH3001_FIFO_GYRO_X_EN		(0x0008)
#define SH3001_FIFO_ACC_Z_EN		(0x0004)
#define SH3001_FIFO_ACC_Y_EN		(0x0002)
#define SH3001_FIFO_ACC_X_EN		(0x0001)
#define SH3001_FIFO_ALL_DIS			(0x0000)

/******************************************************************
*	AUX I2C Config Macro Definitions
******************************************************************/
#define SH3001_MI2C_NORMAL_MODE			(0x00)
#define SH3001_MI2C_BYPASS_MODE			(0x01)

#define SH3001_MI2C_READ_ODR_200HZ	    (0x00)
#define SH3001_MI2C_READ_ODR_100HZ	    (0x10)
#define SH3001_MI2C_READ_ODR_50HZ		(0x20)
#define SH3001_MI2C_READ_ODR_25HZ		(0x30)

#define SH3001_MI2C_FAIL				(0x20)
#define SH3001_MI2C_SUCCESS				(0x10)

#define SH3001_MI2C_READ_MODE_AUTO		(0x40)
#define SH3001_MI2C_READ_MODE_MANUAL	(0x00)



/******************************************************************
*	Other Macro Definitions
******************************************************************/
#define SH3001_TRUE		(1)
#define SH3001_FALSE	(0)


#define SH3001_NORMAL_MODE		(0x00)
#define SH3001_SLEEP_MODE		(0x01)
#define SH3001_POWERDOWN_MODE	(0x02)
#define SH3001_ACC_NORMAL_MODE	(0x03)




/***************************************************************************
*       Type Definitions
****************************************************************************/
typedef unsigned char (*IMU_read)(unsigned char devAddr,
                                  unsigned char regAddr,
                                  unsigned char readLen,
                                  unsigned char *readBuf);

typedef unsigned char (*IMU_write)(unsigned char devAddr,
                                   unsigned char regAddr,
                                   unsigned char writeLen,
                                   unsigned char *writeBuf);

typedef struct {
    signed char cXY;
    signed char cXZ;
    signed char cYX;
    signed char cYZ;
    signed char cZX;
    signed char cZY;
    signed char jX;
    signed char jY;
    signed char jZ;
    unsigned char xMulti;
    unsigned char yMulti;
    unsigned char zMulti;
    unsigned char paramP0;
} compCoefType;


typedef struct {
    // u8 comms;     //0:IIC;  1:SPI //宏控制
#if (SH3001_USER_INTERFACE==0)
    u8 iic_hdl;
    u8 iic_delay;    //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#else
    u8 spi_hdl;      //SPIx (role:master)
    u8 spi_cs_pin;   //
    u8 spi_work_mode;//1:3wire(SPI_MODE_UNIDIR_1BIT) or 0:4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;      //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;   //spi_clk: <=1MHz (spi结构体)
#endif
} sh3001_param;


/***************************************************************************
        Exported Functions
****************************************************************************/

extern unsigned char SH3001_init(void);
extern void SH3001_GetImuData(short accData[3], short gyroData[3]);
extern void SH3001_GetImuCompData(short accData[3], short gyroData[3]);
extern float SH3001_GetTempData(void);
extern unsigned char SH3001_SwitchPowerMode(unsigned char powerMode);





// interrupt related fucntions
extern void SH3001_INT_Enable(unsigned short int intType,
                              unsigned char intEnable,
                              unsigned char intPinSel);

extern void SH3001_INT_Config(unsigned char int0Level,
                              unsigned char intLatch,
                              unsigned char int1Level,
                              unsigned char intClear,
                              unsigned char int1Mode,
                              unsigned char int0Mode,
                              unsigned char intTime);

extern void SH3001_INT_LowG_Config(unsigned char lowGEnDisIntAll,
                                   unsigned char lowGThres,
                                   unsigned char lowGTimsMs);

extern void SH3001_INT_HighG_Config(unsigned char highGEnDisIntX,
                                    unsigned char highGEnDisIntY,
                                    unsigned char highGEnDisIntZ,
                                    unsigned char highGEnDisIntAll,
                                    unsigned char highGThres,
                                    unsigned char highGTimsMs);

extern void SH3001_INT_Inact_Config(unsigned char inactEnDisIntX,
                                    unsigned char inactEnDisIntY,
                                    unsigned char inactEnDisIntZ,
                                    unsigned char inactIntMode,
                                    unsigned char inactLinkStatus,
                                    unsigned char inactTimeS,
                                    unsigned int inactIntThres,
                                    unsigned short int inactG1);

extern void SH3001_INT_Act_Config(unsigned char actEnDisIntX,
                                  unsigned char actEnDisIntY,
                                  unsigned char actEnDisIntZ,
                                  unsigned char actIntMode,
                                  unsigned char actTimeNum,
                                  unsigned char actIntThres,
                                  unsigned char actLinkStatus);

extern void SH3001_INT_Tap_Config(unsigned char tapEnDisIntX,
                                  unsigned char tapEnDisIntY,
                                  unsigned char tapEnDisIntZ,
                                  unsigned char tapIntThres,
                                  unsigned char tapTimeMs,
                                  unsigned char tapWaitTimeMs,
                                  unsigned char tapWaitTimeWindowMs);

extern void SH3001_INT_Flat_Config(unsigned char flatTimeTH,
                                   unsigned char flatTanHeta2);

extern void SH3001_INT_Orient_Config(unsigned char 	orientBlockMode,
                                     unsigned char 	orientMode,
                                     unsigned char	orientTheta,
                                     unsigned short	orientG1point5,
                                     unsigned short 	orientSlope,
                                     unsigned short 	orientHyst);

extern void SH3001_INT_FreeFall_Config(unsigned char freeFallThres,
                                       unsigned char freeFallTimsMs);

extern unsigned short SH3001_INT_Read_Status0(void);

extern unsigned char SH3001_INT_Read_Status2(void);

extern unsigned char SH3001_INT_Read_Status3(void);

extern unsigned char SH3001_INT_Read_Status4(void);

extern void SH3001_pre_INT_config(void);




// Fifo related fucntions
extern void SH3001_FIFO_Reset(unsigned char fifoMode);

extern void SH3001_FIFO_Freq_Config(unsigned char fifoAccDownSampleEnDis,
                                    unsigned char fifoAccFreq,
                                    unsigned char fifoGyroDownSampleEnDis,
                                    unsigned char fifoGyroFreq);

extern void SH3001_FIFO_Data_Config(unsigned short fifoMode,
                                    unsigned short fifoWaterMarkLevel);


extern unsigned char SH3001_FIFO_Read_Status(unsigned short int *fifoEntriesCount);

extern void SH3001_FIFO_Read_Data(unsigned char *fifoReadData,
                                  unsigned short int fifoDataLength);





// Master I2C related fucntions
extern void SH3001_MI2C_Reset(void);

extern void SH3001_MI2C_Bus_Config(unsigned char mi2cReadMode,
                                   unsigned char mi2cODR,
                                   unsigned char mi2cFreq);

extern void SH3001_MI2C_Cmd_Config(unsigned char mi2cSlaveAddr,
                                   unsigned char mi2cSlaveCmd,
                                   unsigned char mi2cReadMode);

extern unsigned char SH3001_MI2C_Write(unsigned char mi2cWriteData);

extern unsigned char SH3001_MI2C_Read(unsigned char *mi2cReadData);




// SPI interface related fucntions
extern void SH3001_SPI_Config(unsigned char spiInterfaceMode);

#endif
#endif

