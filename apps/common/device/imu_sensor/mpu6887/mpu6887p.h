#ifndef __MPU6887P_H_
#define __MPU6887P_H_

#include <string.h>
/******************************************************************
 a 6-axis MotionTracking device:
    a 3-axis gyroscope(±250, ±500, ±1000, and ±2000 degrees/sec.)(16-bit ADCs),
    a 3-axis accelerometer(±2g, ±4g, ±8g, and ±16g)(16-bit ADCs)

    VDD/VDDIO: 1.71~3.45V
    FIFO:512-byte.  burst read sensor data and then go into a low-power mode.
    fifo_data:out_data_reg 59~72
    Host (slave) interface supports  I2C(400kHz), and 4-wire SPI(8MHz);
    no iic master,9 axes are not supported
 @The device will come up in sleep mode upon power-up
******************************************************************/

#if TCFG_MPU6887P_ENABLE
/******************************************************************
*    user config MPU6887P Macro Definition
******************************************************************/
#define  MPU6887P_USE_I2C 0 /*IIC.(<=400kHz) */
#define  MPU6887P_USE_SPI 1 /*SPI.(<=8MHz) */
#define MPU6887P_USER_INTERFACE          TCFG_MPU6887P_INTERFACE_TYPE//MPU6887P_USE_I2C
// #define MPU6887P_USER_INTERFACE           MPU6887P_USE_SPI
#define MPU6887P_SA0_IIC_ADDR            0 //1:iic模式SA0接VCC, 0:iic模式SA0接GND
#define MPU6887P_6_Axis_LOW_POWER_MODE   0 //1:6_Axis_LOW_POWER_MODE(1.33ma); 0:6_Axis Low-Noise Mode(2.79ma)
//int io config
// #define MPU6887P_INT_IO1         IO_PORTB_03//TCFG_IOKEY_POWER_ONE_PORT //
// #define MPU6887P_INT_IO1         IO_PORTA_06 //
// #define MPU6887P_INT_READ_IO1()   gpio_read(MPU6887P_INT_IO1)


#define MPU6887P_USE_FIFO_EN 1 //0:disable fifo  1:enable fifo(fifo size:512 bytes)
#define MPU6887P_USE_INT_EN 0 //0:disable //中断方式不成功,疑硬件问题
#define MPU6887P_USE_WOM_EN 0 //0:disable
#define MPU6887P_USE_FSYNC_EN 0 //0:disable
/******************************************************************
*   MPU6887P I2C address Macro Definition
*   (7bit):    (0x37)011 0111@SDO=1;    (0x36)011 0110@SDO=0;
******************************************************************/
#if MPU6887P_SA0_IIC_ADDR
#define MPU6887P_SLAVE_ADDRESS               (0x69<<1)//0XD0
#else
#define MPU6887P_SLAVE_ADDRESS               (0x68<<1)//0XD2
#endif

typedef u16(*IMU_read)(unsigned char devAddr,
                       unsigned char regAddr,
                       unsigned char *readBuf,
                       u16 readLen);
typedef u16(*IMU_write)(unsigned char devAddr,
                        unsigned char regAddr,
                        unsigned char *writeBuf,
                        u16 writeLen);

typedef struct {
    // u8 comms;     //0:IIC;  1:SPI //改为宏控制
#if (MPU6887P_USER_INTERFACE==MPU6887P_USE_I2C)
    u8 iic_hdl;
    u8 iic_delay;    //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#elif (MPU6887P_USER_INTERFACE==MPU6887P_USE_SPI)
    u8 spi_hdl;      //SPIx (role:master)
    u8 spi_cs_pin;   //
    //u8 spi_work_mode;//mpu6887p only suspport 4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;      //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;   //spi_clk: <=8MHz (spi结构体)

#endif
} mpu6887p_param;

/******************************************************************
*   Mpu6887p Registers Macro Definitions
******************************************************************/
enum Mpu6887pRegister {
    Mpu6887pRegister_XG_OFFS_TC_H = 4,      //4
    Mpu6887pRegister_XG_OFFS_TC_L,      //5
    Mpu6887pRegister_YG_OFFS_TC_H = 7,      //7
    Mpu6887pRegister_YG_OFFS_TC_L,      //8
    Mpu6887pRegister_ZG_OFFS_TC_H = 10,     //10
    Mpu6887pRegister_ZG_OFFS_TC_L,      //11
    Mpu6887pRegister_SELF_TEST_X_ACCEL = 13, //13
    Mpu6887pRegister_SELF_TEST_Y_ACCEL, //14
    Mpu6887pRegister_SELF_TEST_Z_ACCEL, //15
    Mpu6887pRegister_XG_OFFS_USRH = 19,     //19
    Mpu6887pRegister_XG_OFFS_USRL,      //20
    Mpu6887pRegister_YG_OFFS_USRH,      //21
    Mpu6887pRegister_YG_OFFS_USRL,      //22
    Mpu6887pRegister_ZG_OFFS_USRH,      //23
    Mpu6887pRegister_ZG_OFFS_USRL,      //24
    Mpu6887pRegister_SMPLRT_DIV,        //25
    Mpu6887pRegister_CONFIG,            //26     ////
    Mpu6887pRegister_GYRO_CONFIG,       //27
    Mpu6887pRegister_ACCEL_CONFIG,      //28
    Mpu6887pRegister_ACCEL_CONFIG_2,    //29
    Mpu6887pRegister_LP_MODE_CFG,       //30
    Mpu6887pRegister_ACCEL_WOM_X_THR = 32,  //32
    Mpu6887pRegister_ACCEL_WOM_Y_THR,   //33
    Mpu6887pRegister_ACCEL_WOM_Z_THR,   //34
    Mpu6887pRegister_FIFO_EN,           //35
    Mpu6887pRegister_FSYNC_INT = 54,        //54
    Mpu6887pRegister_INT_PIN_CFG,       //55
    Mpu6887pRegister_INT_ENABLE,        //56
    Mpu6887pRegister_FIFO_WM_INT_STATUS,//57
    Mpu6887pRegister_INT_STATUS,        //58
    Mpu6887pRegister_ACCEL_XOUT_H,      //59
    Mpu6887pRegister_ACCEL_XOUT_L,      //60
    Mpu6887pRegister_ACCEL_YOUT_H,      //61
    Mpu6887pRegister_ACCEL_YOUT_L,      //62
    Mpu6887pRegister_ACCEL_ZOUT_H,      //63
    Mpu6887pRegister_ACCEL_ZOUT_L,      //64
    Mpu6887pRegister_TEMP_OUT_H,        //65
    Mpu6887pRegister_TEMP_OUT_L,        //66
    Mpu6887pRegister_GYRO_XOUT_H,       //67
    Mpu6887pRegister_GYRO_XOUT_L,       //68
    Mpu6887pRegister_GYRO_YOUT_H,       //69
    Mpu6887pRegister_GYRO_YOUT_L,       //70
    Mpu6887pRegister_GYRO_ZOUT_H,       //71
    Mpu6887pRegister_GYRO_ZOUT_L,       //72
    Mpu6887pRegister_SELF_TEST_X_GYRO = 80,  //80
    Mpu6887pRegister_SELF_TEST_Y_GYRO,  //81
    Mpu6887pRegister_SELF_TEST_Z_GYRO,  //82
    Mpu6887pRegister_E_ID0,             //83
    Mpu6887pRegister_E_ID1,             //84
    Mpu6887pRegister_E_ID2,             //85
    Mpu6887pRegister_E_ID3,             //86
    Mpu6887pRegister_E_ID4,             //87
    Mpu6887pRegister_E_ID5,             //88
    Mpu6887pRegister_E_ID6,             //89
    Mpu6887pRegister_FIFO_WM_TH1 = 96,      //96
    Mpu6887pRegister_FIFO_WM_TH2,       //97
    Mpu6887pRegister_SIGNAL_PATH_RESET = 104, //104
    Mpu6887pRegister_ACCEL_INTEL_CTRL,  //105
    Mpu6887pRegister_USER_CTRL,         //106
    Mpu6887pRegister_PWR_MGMT_1,        //107    ////
    Mpu6887pRegister_PWR_MGMT_2,        //108
    Mpu6887pRegister_I2C_IF = 112,          //112
    Mpu6887pRegister_FIFO_COUNTH = 114,     //114
    Mpu6887pRegister_FIFO_COUNTL,       //115
    Mpu6887pRegister_FIFO_R_W,          //116
    Mpu6887pRegister_WHO_AM_I,          //117    ////
    Mpu6887pRegister_XA_OFFSET_H = 119,     //119
    Mpu6887pRegister_XA_OFFSET_L,       //120
    Mpu6887pRegister_YA_OFFSET_H = 122,     //122
    Mpu6887pRegister_YA_OFFSET_L,       //123
    Mpu6887pRegister_ZA_OFFSET_H = 125,     //125
    Mpu6887pRegister_ZA_OFFSET_L,       //126
};



// #ifndef M_PI
// #define M_PI    (3.14159265358979323846f)
// #endif
// #ifndef ONE_G
// #define ONE_G   (9.807f)
// #endif

typedef enum Mpu6887p_fifo_format {
    MPU6887P_FORMAT_EMPTY,
    MPU6887P_FORMAT_ACCEL_8_BYTES,
    MPU6887P_FORMAT_GYRO_8_BYTES,
    MPU6887P_FORMAT_ACCEL_GYRO_14_BYTES,

    MPU6887P_FORMAT_UNKNOWN = 0xff,
} Mpu6887p_fifo_format;



/***************************************************************************
        Exported Functions
****************************************************************************/

void mpu6887p_delay(int ms);
void mpu6887p_interface_mode_set(u8 spi_en);//1:spi(4wire) ,0:iic
void mpu6887p_accel_temp_rst(u8 accel_rst_en, u8 temp_rst_en);//1:rst ,0:dis
void mpu6887p_all_reg_rst(u8 all_reg_rst_en);//1:rst ,0:dis
void mpu6887p_device_reset();
bool mpu6887p_set_sleep_enabled(u8 enable);// 0:唤醒MPU, 1:disable
void mpu6887p_power_up_set();
// void mpu6887p_clock_select(enum mpu_clock_select clock);
void mpu6887p_disable_temp_Sensor(unsigned char temp_disable);//1:temperature disable, 0:temperature enable
void mpu6887p_disable_acc_Sensors(u8 acc_x_disable, u8 acc_y_disable, u8 acc_z_disable);//1:disable , 0:enable
void mpu6887p_disable_gyro_Sensors(u8 gyro_x_disable, u8 gyro_y_disable, u8 gyro_z_disable);//1:disable , 0:enable
u8 mpu6887p_config_acc_range(u8 fsr);//fsr:0,±2g;1,±4g;2,±8g;3,±16g
u8 mpu6887p_set_accel_dlpf(u16 lpf);//Low-Noise Mode
// u8 mpu6887p_set_accel_low_power(u8 averag);//Low Power Mode
u8 mpu6887p_config_gyro_range(u8 fsr);//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
u8 mpu_set_dlpf(u16 lpf);//Low-Noise Mode
// u8 mpu6887p_set_gyro_low_power(u8 gyro_cycle_en,u8 averag);//Low Power Mode
u8 mpu_set_rate(u16 rate);// 设置采样速率.
unsigned char mpu6887p_read_status(void);
unsigned char mpu6887p_read_fsync_status(void);
float mpu6887p_readTemp(void);
// void mpu6887p_read_raw_acc_xyz(short raw_acc_xyz[3]);
// void mpu6887p_read_raw_gyro_xyz(short raw_gyro_xyz[3]);
// void mpu6887p_read_raw_acc_gyro_xyz(short raw_acc_xyz[3], short raw_gyro_xyz[3]);
void mpu6887p_read_raw_acc_xyz(void *acc_data);
void mpu6887p_read_raw_gyro_xyz(void *gyro_data);
void mpu6887p_read_raw_acc_gyro_xyz(void *raw_data);


#if (MPU6887P_USE_FIFO_EN)
// void mpu6887p_fifo_rst(u8 fifo_rst_en);//1:rst ,0:dis
// void mpu6887p_fifo_operation_enable(u8 fifo_en);//1:enable fifo ,0:dis
// u8 mpu6887p_set_fifo_data_type(u8 acc_fifo_en, u8 gyro_fifo_en);//1:write data to fifo;0:disable.
// u8 mpu6887p_set_fifo_mode(u8 fifo_mode_en);//1:fifo mode ; 0:stream mode.
// unsigned char mpu6887p_set_fifo_wm_threshold(u16 watermark_level);//watermark_level:0~1023
void mpu6887p_config_fifo(u16 watermark_level, u8 fifo_mode, u8 acc_fifo_en, u8 gyro_fifo_en);
u16 mpu6887p_read_fifo_data(u8 *buf);
unsigned char mpu6887p_read_fifo_wm_int_status(void);//读FIFO_R_W register清除.如果FIFO没读完,又达到fifo_wm,还会产生中断.
#endif


#if (MPU6887P_USE_INT_EN)
u8 mpu6887p_interrupt_type_config(u8 fifo_oflow_int_en, u8 Gdrive_int_en, u8 data_RDY_int_en); //int_type_en:1-enable; 0-disable
u8 mpu6887p_interrupt_pin_config(u8 int_pin_level, u8 int_pin_open, u8 int_pin_latch_en, u8 int_pin_clear_mode);
u8 mpu6887p_interrupt_fsync_pin_config(u8 fsync_int_level, u8 fsync_int_mode_en);
#endif


#if (MPU6887P_USE_WOM_EN)
// u8 mpu6887p_interrupt_WOM_type_config(u8 WOM_x_int_en, u8 WOM_y_int_en,u8 WOM_z_int_en);//int_type_en:1-enable; 0-disable
// u8 mpu6887p_set_WOM_int_threshold(u8 acc_wom_x_thr, u8 acc_wom_y_thr, u8 acc_wom_z_thr);//
u8 mpu6887p_WOM_mode_config(u8 acc_wom_x_thr, u8 acc_wom_y_thr, u8 acc_wom_z_thr, u16 sample_rate);//
#endif

u8 mpu6887p_sensor_init(void *priv);

#endif
#endif

