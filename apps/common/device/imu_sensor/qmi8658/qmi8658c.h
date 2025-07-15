
#ifndef __QMI8658C_H_
#define __QMI8658C_H_

#include <string.h>
/******************************************************************
a complete 6D MEMS.
The QMI8658C provides an I2C master interface (I2CM)to connect with an external magnetometer.
Currently the QMI8658C can support the following magnetometers:
    AK09915C, AK09918CZ, and QMC6308.
Host (slave) interface supports MIPI™ I3C, I2C, and
3-wire or 4-wire SPI; auxiliary master I2C interface
supports an external magnetometer
******************************************************************/


#if TCFG_QMI8658_ENABLE
/******************************************************************
*    user config QMI8658 Macro Definition
******************************************************************/
// spi模式切换到iic模式, 传感器可能需要重新上电
#define  QMI8658_USE_I2C 0 /*IIC.(<=400kHz) */
#define  QMI8658_USE_SPI 1 /*SPI.(<=15MHz) */
#define  QMI8658_USE_I3C 2 /*I3C. */
#define QMI8658_USER_INTERFACE          TCFG_QMI8658_INTERFACE_TYPE//QMI8658_USE_I2C
// #define QMI8658_USER_INTERFACE           QMI8658_USE_SPI

#define QMI8658_SD0_IIC_ADDR            1 //1:iic模式SD0接VCC, 0:iic模式SD0接GND
//int io config
// #define QMI8658_INT_IO1         IO_PORTA_06 //高有效 1.CTRL9_protocol 2.WoM
// #define QMI8658_INT_READ_IO1()   gpio_read(QMI8658_INT_IO1)
// #define QMI8658_INT_IO2         IO_PORTA_07 //高有效 1.data ready 2.fifo 3.test 4.AE_mode 5.WoM
// #define QMI8658_INT_READ_IO2()   gpio_read(QMI8658_INT_IO2)


// 注意: fifo模式切换到寄存器模式, 传感器需要重新上电,反之不需要
#define QMI8658_USE_FIFO_EN      1 //0:disable fifo  1:enable fifo(fifo size:1536 bytes)
#define QMI8658_USE_INT_EN       0 //0:disable
#define QMI8658_USE_ANYMOTION_EN 0
#define QMI8658_USE_TAP_EN       0
#define QMI8658_USE_STEP_EN      0

#define QMI8658_UINT_MG_DPS      0
#define QMI8658_SYNC_SAMPLE_MODE  0//寄存器中断模式0:disable sync sample, 1:enable

#define QMI8658_HANDSHAKE_NEW
#define QMI8658_HANDSHAKE_TO_STATUS
#define QMI8658_NEW_HANDSHAKE  1//另一种qmi8658
#if (QMI8658_NEW_HANDSHAKE == 1)
#define QMI8658_FIFO_INT_OMAP_INT1  1//1:fifo中断映射到int1;  0:fifo中断映射到int2
#endif
/******************************************************************
*   QMI8658 I2C address Macro Definition
*   (7bit):    (0x37)011 0111@SDO=1;    (0x36)011 0110@SDO=0;
******************************************************************/
#if QMI8658_SD0_IIC_ADDR
#define QMI8658_SLAVE_ADDRESS               (0x6a<<1)//0xd4
#else
#define QMI8658_SLAVE_ADDRESS               (0x6b<<1)//0xd6
#endif

typedef u16(*IMU_read)(unsigned char devAddr,
                       unsigned char regAddr,
                       unsigned char *readBuf,
                       u16 readLen);
typedef unsigned char (*IMU_write)(unsigned char devAddr,
                                   unsigned char regAddr,
                                   unsigned char writebyte);
// typedef unsigned char (*IMU_write)( unsigned char devAddr,
//                                     unsigned char regAddr,
//                                     unsigned char *writeBuf,
//                                     unsigned char writeLen);
typedef struct {
    // u8 comms;     //0:IIC;  1:SPI //宏控制
#if (QMI8658_USER_INTERFACE==QMI8658_USE_I2C)
    u8 iic_hdl;
    u8 iic_delay;    //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#elif (QMI8658_USER_INTERFACE==QMI8658_USE_SPI)
    u8 spi_hdl;      //SPIx (role:master)
    u8 spi_cs_pin;   //
    u8 spi_work_mode;//1:3wire(SPI_MODE_UNIDIR_1BIT) or 0:4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;      //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;   //spi_clk: <=15MHz (spi结构体)
#else
    //I3C
#endif
} qmi8658_param;

/******************************************************************
*   QMI8658 Registers Macro Definitions
******************************************************************/

#ifndef M_PI
#define M_PI    (3.14159265358979323846f)
#endif
#ifndef ONE_G
#define ONE_G   (9.807f)
#endif

#define QMI8658_CTRL7_DISABLE_ALL           (0x0)
#define QMI8658_CTRL7_ACC_ENABLE            (0x1)
#define QMI8658_CTRL7_GYR_ENABLE            (0x2)
#define QMI8658_CTRL7_MAG_ENABLE            (0x4)
#define QMI8658_CTRL7_AE_ENABLE             (0x8)
#define QMI8658_CTRL7_GYR_SNOOZE_ENABLE     (0x10)
#define QMI8658_CTRL7_ENABLE_MASK           (0xF)

#define QMI8658_CONFIG_ACC_ENABLE           QMI8658_CTRL7_ACC_ENABLE
#define QMI8658_CONFIG_GYR_ENABLE           QMI8658_CTRL7_GYR_ENABLE
#define QMI8658_CONFIG_MAG_ENABLE           QMI8658_CTRL7_MAG_ENABLE
#define QMI8658_CONFIG_AE_ENABLE            QMI8658_CTRL7_AE_ENABLE
#define QMI8658_CONFIG_ACCGYR_ENABLE        (QMI8658_CONFIG_ACC_ENABLE | QMI8658_CONFIG_GYR_ENABLE)
#define QMI8658_CONFIG_ACCGYRMAG_ENABLE     (QMI8658_CONFIG_ACC_ENABLE | QMI8658_CONFIG_GYR_ENABLE | QMI8658_CONFIG_MAG_ENABLE)
#define QMI8658_CONFIG_AEMAG_ENABLE         (QMI8658_CONFIG_AE_ENABLE | QMI8658_CONFIG_MAG_ENABLE)

#define QMI8658_STATUS1_CMD_DONE            (0x01)
#define QMI8658_STATUS1_WAKEUP_EVENT        (0x04)

#if (QMI8658_NEW_HANDSHAKE)
#define QMI8658_CTRL8_INT_SEL			0x40		// bit6: 1 int1, 0 int2
#define QMI8658_CTRL8_PEDOMETER_EN		0x10
#define QMI8658_CTRL8_SIGMOTION_EN		0x08
#define QMI8658_CTRL8_NOMOTION_EN		0x04
#define QMI8658_CTRL8_ANYMOTION_EN		0x02
#define QMI8658_CTRL8_TAP_EN			0x01

#define QMI8658_INT1_ENABLE				0x08
#define QMI8658_INT2_ENABLE				0x10

#define QMI8658_DRDY_DISABLE			0x20	// ctrl7

#define QMI8658_FIFO_MAP_INT1			0x04	// ctrl1
#define QMI8658_FIFO_MAP_INT2			~0x04
#endif
enum Qmi8658Register {
    Qmi8658Register_WhoAmI = 0, // 0
    Qmi8658Register_Revision, // 1
    Qmi8658Register_Ctrl1, // 2
    Qmi8658Register_Ctrl2, // 3
    Qmi8658Register_Ctrl3, // 4
    Qmi8658Register_Ctrl4, // 5
    Qmi8658Register_Ctrl5, // 6
    Qmi8658Register_Ctrl6, // 7
    Qmi8658Register_Ctrl7, // 8
    Qmi8658Register_Ctrl8, // 9
    Qmi8658Register_Ctrl9,  // 10
    Qmi8658Register_Cal1_L = 11,
    Qmi8658Register_Cal1_H,
    Qmi8658Register_Cal2_L,
    Qmi8658Register_Cal2_H,
    Qmi8658Register_Cal3_L,
    Qmi8658Register_Cal3_H,
    Qmi8658Register_Cal4_L,
    Qmi8658Register_Cal4_H,
    Qmi8658Register_FifoWmkTh = 19,
    Qmi8658Register_FifoCtrl = 20,
    Qmi8658Register_FifoCount = 21,
    Qmi8658Register_FifoStatus = 22,
    Qmi8658Register_FifoData = 23,
    Qmi8658Register_StatusI2CM = 44,
    Qmi8658Register_StatusInt = 45,
    Qmi8658Register_Status0,
    Qmi8658Register_Status1,
    Qmi8658Register_Timestamp_L = 48,
    Qmi8658Register_Timestamp_M,
    Qmi8658Register_Timestamp_H,
    Qmi8658Register_Tempearture_L = 51,
    Qmi8658Register_Tempearture_H,
    Qmi8658Register_Ax_L = 53,
    Qmi8658Register_Ax_H,
    Qmi8658Register_Ay_L,
    Qmi8658Register_Ay_H,
    Qmi8658Register_Az_L,
    Qmi8658Register_Az_H,
    Qmi8658Register_Gx_L = 59,
    Qmi8658Register_Gx_H,
    Qmi8658Register_Gy_L,
    Qmi8658Register_Gy_H,
    Qmi8658Register_Gz_L,
    Qmi8658Register_Gz_H,
    Qmi8658Register_Mx_L = 65,
    Qmi8658Register_Mx_H,
    Qmi8658Register_My_L,
    Qmi8658Register_My_H,
    Qmi8658Register_Mz_L,
    Qmi8658Register_Mz_H,
    Qmi8658Register_Q1_L = 73,
    Qmi8658Register_Q1_H,
    Qmi8658Register_Q2_L,
    Qmi8658Register_Q2_H,
    Qmi8658Register_Q3_L,
    Qmi8658Register_Q3_H,
    Qmi8658Register_Q4_L,
    Qmi8658Register_Q4_H,
    Qmi8658Register_Dvx_L = 81,
    Qmi8658Register_Dvx_H,
    Qmi8658Register_Dvy_L,
    Qmi8658Register_Dvy_H,
    Qmi8658Register_Dvz_L,
    Qmi8658Register_Dvz_H,
    Qmi8658Register_AeReg1 = 87,
    Qmi8658Register_AeOverflow,
    Qmi8658Register_AccEl_X = 90,
    Qmi8658Register_AccEl_Y,
    Qmi8658Register_AccEl_Z,

    Qmi8658Register_Reset = 96, //err?????

    Qmi8658Register_I2CM_STATUS = 110, //err???
};

enum Qmi8658_Ois_Register {
    /*-----------------------------*/
    /* Setup and Control Registers */
    /*-----------------------------*/
    /*! \brief SPI Endian Selection, and SPI 3/4 Wire */
    Qmi8658_OIS_Reg_Ctrl1 = 0x02,       // 2  [0x02] -- Dflt: 0x20
    /*! \brief Accelerometer control: ODR, Full Scale, Self Test  */
    Qmi8658_OIS_Reg_Ctrl2,              //  3  [0x03]
    /*! \brief Gyroscope control: ODR, Full Scale, Self Test */
    Qmi8658_OIS_Reg_Ctrl3,              //  4  [0x04]
    /*! \brief Sensor Data Processing Settings */
    Qmi8658_OIS_Reg_Ctrl5   = 0x06,             //  6  [0x06]
    /*! \brief Sensor enabled status: Enable Sensors  */
    Qmi8658_OIS_Reg_Ctrl7   = 0x08,             //  8  [0x08]
    /*-------------------*/
    /* Status Registers  */
    /*-------------------*/
    /*! \brief Sensor Data Availability and Lock Register */
    Qmi8658_OIS_Reg_StatusInt = 0x2D,           // 45  [0x2D]
    /*! \brief Output data overrun and availability */
    Qmi8658_OIS_Reg_Status0   = 0x2E,           // 46  [0x2E]

    /*-----------------------------------------------------*/
    /* OIS Sensor Data Output Registers. 16-bit 2's complement */
    /*-----------------------------------------------------*/
    /*! \brief Accelerometer X axis least significant byte */
    Qmi8658_OIS_Reg_Ax_L  = 0x33,           // 53  [0x35]
    /*! \brief Accelerometer X axis most significant byte */
    Qmi8658_OIS_Reg_Ax_H,                   // 54  [0x36]
    /*! \brief Accelerometer Y axis least significant byte */
    Qmi8658_OIS_Reg_Ay_L,                   // 55  [0x37]
    /*! \brief Accelerometer Y axis most significant byte */
    Qmi8658_OIS_Reg_Ay_H,                   // 56  [0x38]
    /*! \brief Accelerometer Z axis least significant byte */
    Qmi8658_OIS_Reg_Az_L,                   // 57  [0x39]
    /*! \brief Accelerometer Z axis most significant byte */
    Qmi8658_OIS_Reg_Az_H,                   // 58  [0x3A]

    /*! \brief Gyroscope X axis least significant byte */
    Qmi8658_OIS_Reg_Gx_L  = 0x3B,           // 59  [0x3B]
    /*! \brief Gyroscope X axis most significant byte */
    Qmi8658_OIS_Reg_Gx_H,                   // 60  [0x3C]
    /*! \brief Gyroscope Y axis least significant byte */
    Qmi8658_OIS_Reg_Gy_L,                   // 61  [0x3D]
    /*! \brief Gyroscope Y axis most significant byte */
    Qmi8658_OIS_Reg_Gy_H,                   // 62  [0x3E]
    /*! \brief Gyroscope Z axis least significant byte */
    Qmi8658_OIS_Reg_Gz_L,                   // 63  [0x3F]
    /*! \brief Gyroscope Z axis most significant byte */
    Qmi8658_OIS_Reg_Gz_H,                   // 64  [0x40]
};


enum Qmi8658_Ctrl9Command {
    Qmi8658_Ctrl9_Cmd_NOP                   = 0X00,
    Qmi8658_Ctrl9_Cmd_GyroBias              = 0X01,
    Qmi8658_Ctrl9_Cmd_Rqst_Sdi_Mod          = 0X03,
    Qmi8658_Ctrl9_Cmd_Rst_Fifo              = 0X04,
    Qmi8658_Ctrl9_Cmd_Req_Fifo              = 0X05,
    Qmi8658_Ctrl9_Cmd_I2CM_Write            = 0X06,
    Qmi8658_Ctrl9_Cmd_WoM_Setting           = 0x08,
    Qmi8658_Ctrl9_Cmd_AccelHostDeltaOffset  = 0x09,
    Qmi8658_Ctrl9_Cmd_GyroHostDeltaOffset   = 0x0A,
    Qmi8658_Ctrl9_Cmd_EnableExtReset        = 0x0B,
    Qmi8658_Ctrl9_Cmd_EnableTap         = 0x0C,
    Qmi8658_Ctrl9_Cmd_EnablePedometer   = 0x0D,
    Qmi8658_Ctrl9_Cmd_Reset_Pedometer       = 0x0F,

    Qmi8658_Ctrl9_Cmd_Motion                = 0x0E,
    Qmi8658_Ctrl9_Cmd_CopyUsid              = 0x10,
    Qmi8658_Ctrl9_Cmd_SetRpu                = 0x11,
    Qmi8658_Ctrl9_Cmd_AHB_Clock_Gating		= 0x12,
    Qmi8658_Ctrl9_Cmd_On_Demand_Cali		= 0xA2,
    Qmi8658_Ctrl9_Cmd_Dbg_WoM_Data_Enable   = 0xF8,

};




enum Qmi8658_LpfConfig {
    Qmi8658Lpf_Disable, /*!< \brief Disable low pass filter. */
    Qmi8658Lpf_Enable   /*!< \brief Enable low pass filter. */
};

enum Qmi8658_HpfConfig {
    Qmi8658Hpf_Disable, /*!< \brief Disable high pass filter. */
    Qmi8658Hpf_Enable   /*!< \brief Enable high pass filter. */
};

enum Qmi8658_StConfig {
    Qmi8658St_Disable, /*!< \brief Disable high pass filter. */
    Qmi8658St_Enable   /*!< \brief Enable high pass filter. */
};

enum Qmi8658_LpfMode {
    A_LSP_MODE_0 = 0x00 << 1,
    A_LSP_MODE_1 = 0x01 << 1,
    A_LSP_MODE_2 = 0x02 << 1,
    A_LSP_MODE_3 = 0x03 << 1,

    G_LSP_MODE_0 = 0x00 << 5,
    G_LSP_MODE_1 = 0x01 << 5,
    G_LSP_MODE_2 = 0x02 << 5,
    G_LSP_MODE_3 = 0x03 << 5
};

enum Qmi8658_AccRange {
    Qmi8658AccRange_2g = 0x00 << 4, /*!< \brief +/- 2g range */
    Qmi8658AccRange_4g = 0x01 << 4, /*!< \brief +/- 4g range */
    Qmi8658AccRange_8g = 0x02 << 4, /*!< \brief +/- 8g range */
    Qmi8658AccRange_16g = 0x03 << 4 /*!< \brief +/- 16g range */
};


enum Qmi8658_AccOdr {
    Qmi8658AccOdr_8000Hz = 0x00,  /*!< \brief High resolution 8000Hz output rate. */
    Qmi8658AccOdr_4000Hz = 0x01,  /*!< \brief High resolution 4000Hz output rate. */
    Qmi8658AccOdr_2000Hz = 0x02,  /*!< \brief High resolution 2000Hz output rate. */
    Qmi8658AccOdr_1000Hz = 0x03,  /*!< \brief High resolution 1000Hz output rate. */
    Qmi8658AccOdr_500Hz = 0x04,  /*!< \brief High resolution 500Hz output rate. */
    Qmi8658AccOdr_250Hz = 0x05, /*!< \brief High resolution 250Hz output rate. */
    Qmi8658AccOdr_125Hz = 0x06, /*!< \brief High resolution 125Hz output rate. */
    Qmi8658AccOdr_62_5Hz = 0x07, /*!< \brief High resolution 62.5Hz output rate. */
    Qmi8658AccOdr_31_25Hz = 0x08,  /*!< \brief High resolution 31.25Hz output rate. */
    Qmi8658AccOdr_LowPower_128Hz = 0x0c, /*!< \brief Low power 128Hz output rate. */
    Qmi8658AccOdr_LowPower_21Hz = 0x0d,  /*!< \brief Low power 21Hz output rate. */
    Qmi8658AccOdr_LowPower_11Hz = 0x0e,  /*!< \brief Low power 11Hz output rate. */
    Qmi8658AccOdr_LowPower_3Hz = 0x0f    /*!< \brief Low power 3Hz output rate. */
};

enum Qmi8658_GyrRange {

    Qmi8658GyrRange_16dps = 0 << 4,   /*!< \brief +-32 degrees per second. */
    Qmi8658GyrRange_32dps = 1 << 4,   /*!< \brief +-32 degrees per second. */
    Qmi8658GyrRange_64dps = 2 << 4,   /*!< \brief +-64 degrees per second. */
    Qmi8658GyrRange_128dps = 3 << 4,  /*!< \brief +-128 degrees per second. */
    Qmi8658GyrRange_256dps = 4 << 4,  /*!< \brief +-256 degrees per second. */
    Qmi8658GyrRange_512dps = 5 << 4,  /*!< \brief +-512 degrees per second. */
    Qmi8658GyrRange_1024dps = 6 << 4, /*!< \brief +-1024 degrees per second. */
    Qmi8658GyrRange_2048dps = 7 << 4, /*!< \brief +-2048 degrees per second. */

};

/*!
 * \brief Gyroscope output rate configuration.
 */
enum Qmi8658_GyrOdr {
    Qmi8658GyrOdr_8000Hz = 0x00,  /*!< \brief High resolution 8000Hz output rate. */
    Qmi8658GyrOdr_4000Hz = 0x01,  /*!< \brief High resolution 4000Hz output rate. */
    Qmi8658GyrOdr_2000Hz = 0x02,  /*!< \brief High resolution 2000Hz output rate. */
    Qmi8658GyrOdr_1000Hz = 0x03,    /*!< \brief High resolution 1000Hz output rate. */
    Qmi8658GyrOdr_500Hz = 0x04, /*!< \brief High resolution 500Hz output rate. */
    Qmi8658GyrOdr_250Hz = 0x05, /*!< \brief High resolution 250Hz output rate. */
    Qmi8658GyrOdr_125Hz = 0x06, /*!< \brief High resolution 125Hz output rate. */
    Qmi8658GyrOdr_62_5Hz    = 0x07, /*!< \brief High resolution 62.5Hz output rate. */
    Qmi8658GyrOdr_31_25Hz   = 0x08  /*!< \brief High resolution 31.25Hz output rate. */
};

enum Qmi8658_AeOdr {
    Qmi8658AeOdr_1Hz = 0x00,  /*!< \brief 1Hz output rate. */
    Qmi8658AeOdr_2Hz = 0x01,  /*!< \brief 2Hz output rate. */
    Qmi8658AeOdr_4Hz = 0x02,  /*!< \brief 4Hz output rate. */
    Qmi8658AeOdr_8Hz = 0x03,  /*!< \brief 8Hz output rate. */
    Qmi8658AeOdr_16Hz = 0x04, /*!< \brief 16Hz output rate. */
    Qmi8658AeOdr_32Hz = 0x05, /*!< \brief 32Hz output rate. */
    Qmi8658AeOdr_64Hz = 0x06,  /*!< \brief 64Hz output rate. */
    Qmi8658AeOdr_128Hz = 0x07,  /*!< \brief 128Hz output rate. */
    /*!
     * \brief Motion on demand mode.
     *
     * In motion on demand mode the application can trigger AttitudeEngine
     * output samples as necessary. This allows the AttitudeEngine to be
     * synchronized with external data sources.
     *
     * When in Motion on Demand mode the application should request new data
     * by calling the Qmi8658_requestAttitudeEngineData() function. The
     * AttitudeEngine will respond with a data ready event (INT2) when the
     * data is available to be read.
     */
    Qmi8658AeOdr_motionOnDemand = 128
};

enum Qmi8658_MagOdr {
    Qmi8658MagOdr_1000Hz = 0x00,   /*!< \brief 1000Hz output rate. */
    Qmi8658MagOdr_500Hz = 0x01, /*!< \brief 500Hz output rate. */
    Qmi8658MagOdr_250Hz = 0x02, /*!< \brief 250Hz output rate. */
    Qmi8658MagOdr_125Hz = 0x03, /*!< \brief 125Hz output rate. */
    Qmi8658MagOdr_62_5Hz = 0x04,    /*!< \brief 62.5Hz output rate. */
    Qmi8658MagOdr_31_25Hz = 0x05    /*!< \brief 31.25Hz output rate. */
};

enum Qmi8658_MagDev {
    MagDev_AKM09918 = (0 << 3), /*!< \brief AKM09918. */
};

enum Qmi8658_AccUnit {
    Qmi8658AccUnit_g,  /*!< \brief Accelerometer output in terms of g (9.81m/s^2). */
    Qmi8658AccUnit_ms2 /*!< \brief Accelerometer output in terms of m/s^2. */
};

enum Qmi8658_GyrUnit {
    Qmi8658GyrUnit_dps, /*!< \brief Gyroscope output in degrees/s. */
    Qmi8658GyrUnit_rads /*!< \brief Gyroscope output in rad/s. */
};

enum Qmi8658_fifo_format {
    QMI8658_FORMAT_EMPTY,
    QMI8658_FORMAT_ACCEL_6_BYTES,
    QMI8658_FORMAT_GYRO_6_BYTES,
    QMI8658_FORMAT_MAG_6_BYTES,
    QMI8658_FORMAT_12_BYTES,  //默认:acc + gyro
    QMI8658_FORMAT_18_BYTES,  //acc + gyro + mag

    QMI8658_FORMAT_UNKNOWN = 0xff,
};

enum Qmi8658_FifoMode {
    Qmi8658_Fifo_Bypass = 0,
    Qmi8658_Fifo_Fifo = 1,
    Qmi8658_Fifo_Stream = 2,
    Qmi8658_Fifo_StreamToFifo = 3
};

//fifo size:1536 bytes
enum Qmi8658_FifoSize {
    Qmi8658_Fifo_16 = (0 << 2), //support 3 sensor
    Qmi8658_Fifo_32 = (1 << 2), //support 3 sensor
    Qmi8658_Fifo_64 = (2 << 2), //support 3 sensor
    Qmi8658_Fifo_128 = (3 << 2) //support 2 sensor
};


// enum Qmi8658_FifoWmkLevel
// {
//     Qmi8658_Fifo_WmkEmpty =         (0 << 4),
//     Qmi8658_Fifo_WmkOneQuarter =    (1 << 4),
//     Qmi8658_Fifo_WmkHalf =          (2 << 4),
//     Qmi8658_Fifo_WmkThreeQuarters = (3 << 4)
// };


enum Qmi8658_anymotion_G_TH {
    Qmi8658_1G = 1 << 5,
    Qmi8658_2G = 2 << 5,
    Qmi8658_3G = 3 << 5,
    Qmi8658_4G = 4 << 5,
    Qmi8658_5G = 5 << 5,
    Qmi8658_6G = 6 << 5,
    Qmi8658_7G = 7 << 5,
    Qmi8658_8G = 8 << 5,
};




struct Qmi8658_offsetCalibration {
    enum Qmi8658_AccUnit accUnit;
    float accOffset[3];
    enum Qmi8658_GyrUnit gyrUnit;
    float gyrOffset[3];
};

struct Qmi8658_sensitivityCalibration {
    float accSensitivity[3];
    float gyrSensitivity[3];
};

enum Qmi8658_Interrupt {
    Qmi8658_Int1_low = (0 << 6),
    Qmi8658_Int2_low = (1 << 6),
    Qmi8658_Int1_high = (2 << 6),
    Qmi8658_Int2_high = (3 << 6)
};

enum Qmi8658_InterruptState {
    Qmi8658State_high = (1 << 7), /*!< Interrupt high. */
    Qmi8658State_low  = (0 << 7)  /*!< Interrupt low. */
};

enum Qmi8658_WakeOnMotionThreshold {
    Qmi8658WomThreshold_off  = 0,
    Qmi8658WomThreshold_low  = 32,
    Qmi8658WomThreshold_mid = 128,
    Qmi8658WomThreshold_high = 255
};

struct Qmi8658Config {
    unsigned char inputSelection;
    enum Qmi8658_AccRange accRange;
    enum Qmi8658_AccOdr accOdr;
    enum Qmi8658_GyrRange gyrRange;
    enum Qmi8658_GyrOdr gyrOdr;
    enum Qmi8658_AeOdr aeOdr;
    enum Qmi8658_MagOdr magOdr;
    enum Qmi8658_MagDev magDev;
#if (QMI8658_USE_FIFO_EN)
    unsigned char           fifo_ctrl;
    unsigned char           fifo_fss;
    enum Qmi8658_fifo_format     fifo_format;
    //unsigned char fifo_status;
#endif
};



/***************************************************************************
        Exported Functions
****************************************************************************/
extern unsigned char qmi8658_init(u8 demand_cali_en);
extern void Qmi8658_Config_apply(struct Qmi8658Config const *config);
extern void Qmi8658_enableSensors(unsigned char enableFlags);
extern void Qmi8658_read_acc_xyz(float acc_xyz[3]);
extern void Qmi8658_read_gyro_xyz(float gyro_xyz[3]);
extern void Qmi8658_read_xyz(float acc[3], float gyro[3], unsigned int *tim_count);
extern void Qmi8658_read_xyz_raw(short raw_acc_xyz[3], short raw_gyro_xyz[3], unsigned int *tim_count);
extern void Qmi8658_read_ae(float quat[4], float velocity[3]);
extern float Qmi8658_readTemp(void);
// extern unsigned char Qmi8658_readStatusInt(void);//无
extern unsigned char Qmi8658_readStatus0(void);
extern unsigned char Qmi8658_readStatus1(void);

extern void Qmi8658_enableWakeOnMotion(enum Qmi8658_Interrupt int_set, enum Qmi8658_WakeOnMotionThreshold threshold, unsigned char blankingTime);
extern void Qmi8658_disableWakeOnMotion(void);
#if (QMI8658_USE_FIFO_EN)
extern void Qmi8658_config_fifo(unsigned char watermark, enum Qmi8658_FifoSize size, enum Qmi8658_FifoMode mode, enum Qmi8658_fifo_format format);
// extern void Qmi8658_enable_fifo(void);//无
unsigned short Qmi8658_read_fifo(unsigned char *data);
void Qmi8658_get_fifo_format(enum Qmi8658_fifo_format *format);
#endif
#if (QMI8658_USE_STEP_EN)
unsigned char stepConfig(unsigned short odr);
uint32_t  Qmi8658_step_read_stepcounter(uint32_t *step);
void step_disable(void);
#endif
#if (QMI8658_USE_ANYMOTION_EN)
// unsigned char anymotion_Config(enum Qmi8658_anymotion_G_TH motion_g_th, unsigned char  motion_mg_th );
void anymotion_lowpwr_config(void);
void anymotion_high_odr_enable(void);
void anymotion_disable(void);
#endif
#if (QMI8658_USE_TAP_EN)
// unsigned char tap_Config(void);
void tap_enable(void);
#endif

#endif
#endif

