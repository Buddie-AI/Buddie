#ifndef __LSM6DSL_H_
#define __LSM6DSL_H_

#include <string.h>
/******************************************************************
a complete 6D MEMS.3-axis digital accelerometer and 3-axis digital gyroscope.
Host (slave) interface supports I2C, and
3-wire or 4-wire SPI
no supports an external magnetometer
******************************************************************/

#if TCFG_LSM6DSL_ENABLE
/******************************************************************
*    user config LSM6DSL Macro Definition
******************************************************************/
// spi模式切换到iic模式, 传感器可能需要重新上电
#define  LSM6DSL_USE_I2C 0 /*IIC.(<=400kHz) */
#define  LSM6DSL_USE_SPI 1 /*SPI.(<=10MHz) */
#define LSM6DSL_USER_INTERFACE          TCFG_LSM6DSL_INTERFACE_TYPE//LSM6DSL_USE_I2C
// #define LSM6DSL_USER_INTERFACE           LSM6DSL_USE_SPI

#define LSM6DSL_SD0_IIC_ADDR            1 //1:iic模式SD0接VCC, 0:iic模式SD0接GND
//int io config
// #define LSM6DSL_INT_IO1         IO_PORTA_06 //高有效 1.CTRL9_protocol 2.WoM
// #define LSM6DSL_INT_READ_IO1()   gpio_read(LSM6DSL_INT_IO1)
// #define LSM6DSL_INT_IO2         IO_PORTA_07 //高有效 1.data ready 2.fifo 3.test 4.AE_mode 5.WoM
// #define LSM6DSL_INT_READ_IO2()   gpio_read(LSM6DSL_INT_IO2)


// 注意: fifo模式切换到寄存器模式, 传感器需要重新上电,反之不需要
#define LSM6DSL_USE_FIFO_EN      1 //0:disable fifo  1:enable fifo(fifo size:1536 bytes)
#define LSM6DSL_FIFO_INT_OMAP_INT1  0//1:fifo中断映射到int1;  0:fifo中断映射到int2
#define LSM6DSL_USE_INT_EN       0 //0:disable
#define LSM6DSL_USE_ANYMOTION_EN 0
#define LSM6DSL_USE_TAP_EN       0
#define LSM6DSL_USE_STEP_EN      0

// #define LSM6DSL_SYNC_SAMPLE_MODE  0//寄存器中断模式0:disable sync sample, 1:enable

/******************************************************************
*   LSM6DSL I2C address Macro Definition
*   (7bit):    (0x37)011 0111@SDO=1;    (0x36)011 0110@SDO=0;
******************************************************************/
#if LSM6DSL_SD0_IIC_ADDR
#define LSM6DSL_IIC_SLAVE_ADDRESS               (0x6b<<1)//0xd6
#else
#define LSM6DSL_IIC_SLAVE_ADDRESS               (0x6a<<1)//0xd4
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
#if (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_I2C)
    u8 iic_hdl;
    u8 iic_delay;    //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#elif (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_SPI)
    u8 spi_hdl;      //SPIx (role:master)
    u8 spi_cs_pin;   //
    u8 spi_work_mode;//1:3wire(SPI_MODE_UNIDIR_1BIT) or 0:4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;      //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;   //spi_clk: <=15MHz (spi结构体)
#else
#endif
} lsm6dsl_param;






/******************************************************************
*   LSM6DSL Registers Macro Definitions
******************************************************************/
/*
 ******************************************************************************
 * @file    lsm6dsl_reg.h
 * @author  MEMS Software Solution Team
 * @date    30-August-2017
 * @brief   This file contains all the functions prototypes for the
 *          lsm6dsl_reg.c driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup lsm6dsl
 * @{
 */

#ifndef __MEMS_SHARED__TYPES
#define __MEMS_SHARED__TYPES

/** @defgroup ST_MEMS_common_types
  * @{
  */

typedef union {
    int16_t i16bit[3];
    uint8_t u8bit[6];
} axis3bit16_t;

typedef union {
    int32_t i32bit[3];
    uint8_t u8bit[12];
} axis3bit32_t;

typedef union {
    int16_t i16bit;
    uint8_t u8bit[2];
} axis1bit16_t;

typedef union {
    int32_t i32bit;
    uint8_t u8bit[4];
} axis1bit32_t;

typedef struct {
    uint8_t bit0       : 1;
    uint8_t bit1       : 1;
    uint8_t bit2       : 1;
    uint8_t bit3       : 1;
    uint8_t bit4       : 1;
    uint8_t bit5       : 1;
    uint8_t bit6       : 1;
    uint8_t bit7       : 1;
} byte_t;

#define PROPERTY_DISABLE                (0)
#define PROPERTY_ENABLE                 (1)

#endif /*__MEMS_SHARED__TYPES*/

/**
  * @}
  */

/** @defgroup lsm6dsl_interface
  * @{
  */

typedef int32_t (*lsm6dsl_write_ptr)(void *, uint8_t, uint8_t *, uint16_t);
typedef int32_t (*lsm6dsl_read_ptr)(void *, uint8_t, uint8_t *, uint16_t);

typedef struct {
    /** Component mandatory fields **/
    lsm6dsl_write_ptr  write_reg;
    lsm6dsl_read_ptr read_reg;
    /** Customizable optional pointer **/
    void *handle;
} lsm6dsl_ctx_t;

/**
  * @}
  */

/** @defgroup lsm6dsl_Infos
  * @{
  */

/** I2C Device Address 8 bit format: 0xD4 if SA0 = 0 else 0xD6 **/
// #define LSM6DSL_I2C_ADDRESS   0xD6

/** Device Identification (Who am I) **/
#define LSM6DSL_ID            0x6A

/**
  * @}
  */

/**
  * @defgroup lsm6dsl_Accelerometer_Sensitivity
  * @{
  */

#define FROM_FS_2g_TO_mg( lsb )      (int32_t)( ( lsb * 61 )  / 1000)
#define FROM_FS_4g_TO_mg( lsb )      (int32_t)( ( lsb * 122 ) / 1000)
#define FROM_FS_8g_TO_mg( lsb )      (int32_t)( ( lsb * 244 ) / 1000)
#define FROM_FS_16g_TO_mg( lsb )     (int32_t)( ( lsb * 488 ) / 1000)

/**
  * @}
  */

/**
  * @defgroup lsm6dsl_Gyro_Sensitivity
  * @{
  */

#define FROM_FS_125dps_TO_mdps( lsb )     (int32_t) ( ( lsb * 4375 ) / 1000 )
#define FROM_FS_250dps_TO_mdps( lsb )     (int32_t) ( ( lsb * 875  ) / 100  )
#define FROM_FS_500dps_TO_mdps( lsb )     (int32_t) ( ( lsb * 1750 ) / 100  )
#define FROM_FS_1000dps_TO_mdps( lsb )    (int32_t)   ( lsb * 35   )
#define FROM_FS_2000dps_TO_mdps( lsb )    (int32_t)   ( lsb * 70   )

/**
  * @}
  */

/**
  * @defgroup lsm6dsl_Temperature_Sensitivity
  * @{
  */

#define FROM_LSB_TO_degC(lsb)     (int16_t) ( ( lsb * 10 ) / 2560 ) + ( 25 )

/**
  * @}
  */

/** @defgroup lsm6dsl_device_registers
  * @{
  */

#define LSM6DSL_FUNC_CFG_ACCESS                  0x01
typedef struct {
    uint8_t not_used_01          : 5;
    uint8_t func_cfg_en          : 3; /** +  FUNC_CFG_EN + FUNC_CFG_EN_B  **/
} lsm6dsl_func_cfg_access_t;

#define LSM6DSL_SENSOR_SYNC_TIME_FRAME           0x04
typedef struct {
    uint8_t tph                   : 4;
    uint8_t not_used_01           : 4;
} lsm6dsl_sensor_sync_time_frame_t;

#define LSM6DSL_SENSOR_SYNC_RES_RATIO            0x05
typedef struct {
    uint8_t rr                    : 2;
    uint8_t not_used_01           : 6;
} lsm6dsl_sensor_sync_res_ratio_t;

/** + FTH[10:8] in FIFO_CTRL2 (07h)  **/
#define LSM6DSL_FIFO_CTRL1                       0x06

#define LSM6DSL_FIFO_CTRL2                       0x07
typedef struct {
    uint8_t fth                   : 3; /** +  + FTH[7:0] in FIFO_CTRL1 (06h)  **/
    uint8_t fifo_temp_en          : 1;
    uint8_t not_used_01           : 2;
    uint8_t  timer_pedo_fifo_drdy : 1;
    uint8_t timer_pedo_fifo_en    : 1;
} lsm6dsl_fifo_ctrl2_t;

#define LSM6DSL_FIFO_CTRL3                       0x08
typedef struct {
    uint8_t dec_fifo_xl           : 3;
    uint8_t dec_fifo_gyro         : 3;
    uint8_t not_used_01           : 2;
} lsm6dsl_fifo_ctrl3_t;

#define LSM6DSL_FIFO_CTRL4                       0x09
typedef struct {
    uint8_t dec_ds3_fifo          : 3;
    uint8_t dec_ds4_fifo          : 3;
    uint8_t only_high_data        : 1;
    uint8_t stop_on_fth           : 1;
} lsm6dsl_fifo_ctrl4_t;

#define LSM6DSL_FIFO_CTRL5                       0x0A
typedef struct {
    uint8_t fifo_mode             : 3;
    uint8_t odr_fifo              : 4;
    uint8_t not_used_01           : 1;
} lsm6dsl_fifo_ctrl5_t;

#define LSM6DSL_DRDY_PULSE_CFG_G                 0x0B
typedef struct {
    uint8_t int2_wrist_tilt      : 1;
    uint8_t not_used_01          : 6;
    uint8_t drdy_pulsed          : 1;
} lsm6dsl_drdy_pulse_cfg_g_t;

#define LSM6DSL_INT1_CTRL                        0x0D
typedef struct {
    uint8_t int1_drdy_xl         : 1;
    uint8_t int1_drdy_g          : 1;
    uint8_t int1_boot            : 1;
    uint8_t int1_fth             : 1;
    uint8_t int1_fifo_ovr        : 1;
    uint8_t int1_full_flag       : 1;
    uint8_t int1_sign_mot        : 1;
    uint8_t int1_step_detector   : 1;
} lsm6dsl_int1_ctrl_t;

#define LSM6DSL_INT2_CTRL                        0x0E
typedef struct {
    uint8_t int2_drdy_xl         : 1;
    uint8_t int2_drdy_g          : 1;
    uint8_t int2_drdy_temp       : 1;
    uint8_t int2_fth             : 1;
    uint8_t int2_fifo_ovr        : 1;
    uint8_t int2_full_flag       : 1;
    uint8_t int2_step_count_ov   : 1;
    uint8_t int2_step_delta      : 1;
} lsm6dsl_int2_ctrl_t;

#define LSM6DSL_WHO_AM_I                         0x0F

#define LSM6DSL_CTRL1_XL                         0x10
typedef struct {
    uint8_t bw0_xl               : 1;
    uint8_t lpf1_bw_sel          : 1;
    uint8_t fs_xl                : 2;
    uint8_t odr_xl               : 4;
} lsm6dsl_ctrl1_xl_t;

#define LSM6DSL_CTRL2_G                          0x11
typedef struct {
    uint8_t not_used_01          : 1;
    uint8_t fs_g                 : 3;
    uint8_t odr_g                : 4;
} lsm6dsl_ctrl2_g_t;

#define LSM6DSL_CTRL3_C                          0x12
typedef struct {
    uint8_t sw_reset             : 1;
    uint8_t ble                  : 1;
    uint8_t if_inc               : 1;
    uint8_t sim                  : 1;
    uint8_t pp_od                : 1;
    uint8_t h_lactive            : 1;
    uint8_t bdu                  : 1;
    uint8_t boot                 : 1;
} lsm6dsl_ctrl3_c_t;

#define LSM6DSL_CTRL4_C                          0x13
typedef struct {
    uint8_t not_used_01          : 1;
    uint8_t lpf1_sel_g           : 1;
    uint8_t i2c_disable          : 1;
    uint8_t drdy_mask            : 1;
    uint8_t den_drdy_int1        : 1;
    uint8_t int2_on_int1         : 1;
    uint8_t sleep                : 1;
    uint8_t den_xl_en            : 1;
} lsm6dsl_ctrl4_c_t;

#define LSM6DSL_CTRL5_C                          0x14
typedef struct {
    uint8_t st_xl                : 2;
    uint8_t st_g                 : 2;
    uint8_t den_lh               : 1;
    uint8_t rounding             : 3;
} lsm6dsl_ctrl5_c_t;

#define LSM6DSL_CTRL6_C                          0x15
typedef struct {
    uint8_t ftype                : 2;
    uint8_t not_used_01          : 1;
    uint8_t usr_off_w            : 1;
    uint8_t xl_hm_mode           : 1;
    uint8_t den_mode             : 3; /** +  TRIG_EN + LVL_EN + LVL2_EN **/
} lsm6dsl_ctrl6_c_t;

#define LSM6DSL_CTRL7_G                          0x16
typedef struct {
    uint8_t not_used_01          : 2;
    uint8_t rounding_status      : 1;
    uint8_t not_used_02          : 1;
    uint8_t hpm_g                : 2;
    uint8_t hp_en_g              : 1;
    uint8_t g_hm_mode            : 1;
} lsm6dsl_ctrl7_g_t;

#define LSM6DSL_CTRL8_XL                         0x17
typedef struct {
    uint8_t low_pass_on_6d       : 1;
    uint8_t not_used_01          : 1;
    uint8_t hp_slope_xl_en       : 1;
    uint8_t input_composite      : 1;
    uint8_t hp_ref_mode          : 1;
    uint8_t hpcf_xl              : 2;
    uint8_t lpf2_xl_en           : 1;
} lsm6dsl_ctrl8_xl_t;

#define LSM6DSL_CTRL9_XL                         0x18
typedef struct {
    uint8_t not_used_01          : 2;
    uint8_t soft_en              : 1;
    uint8_t not_used_02          : 1;
    uint8_t den_xl_g             : 1;
    uint8_t den_z                : 1;
    uint8_t den_y                : 1;
    uint8_t den_x                : 1;
} lsm6dsl_ctrl9_xl_t;

#define LSM6DSL_CTRL10_C                         0x19
typedef struct {
    uint8_t sign_motion_en       : 1;
    uint8_t pedo_rst_step        : 1;
    uint8_t func_en              : 1;
    uint8_t tilt_en              : 1;
    uint8_t pedo_en              : 1;
    uint8_t timer_en             : 1;
    uint8_t not_used_01          : 1;
    uint8_t wrist_tilt_en        : 1;
} lsm6dsl_ctrl10_c_t;

#define LSM6DSL_MASTER_CONFIG                    0x1A
typedef struct {
    uint8_t master_on            : 1;
    uint8_t iron_en              : 1;
    uint8_t pass_through_mode    : 1;
    uint8_t pull_up_en           : 1;
    uint8_t start_config         : 1;
    uint8_t not_used_01          : 1;
    uint8_t data_valid_sel_fifo  : 1;
    uint8_t drdy_on_int1         : 1;
} lsm6dsl_master_config_t;

#define LSM6DSL_WAKE_UP_SRC                      0x1B
typedef struct {
    uint8_t z_wu                 : 1;
    uint8_t y_wu                 : 1;
    uint8_t x_wu                 : 1;
    uint8_t wu_ia                : 1;
    uint8_t sleep_state_ia       : 1;
    uint8_t ff_ia                : 1;
    uint8_t not_used_01          : 2;
} lsm6dsl_wake_up_src_t;

#define LSM6DSL_TAP_SRC                          0x1C
typedef struct {
    uint8_t z_tap                : 1;
    uint8_t y_tap                : 1;
    uint8_t x_tap                : 1;
    uint8_t tap_sign             : 1;
    uint8_t double_tap           : 1;
    uint8_t single_tap           : 1;
    uint8_t tap_ia               : 1;
    uint8_t not_used_01          : 1;
} lsm6dsl_tap_src_t;

#define LSM6DSL_D6D_SRC                          0x1D
typedef struct {
    uint8_t xl                   : 1;
    uint8_t xh                   : 1;
    uint8_t yl                   : 1;
    uint8_t yh                   : 1;
    uint8_t zl                   : 1;
    uint8_t zh                   : 1;
    uint8_t d6d_ia               : 1;
    uint8_t den_drdy             : 1;
} lsm6dsl_d6d_src_t;

#define LSM6DSL_STATUS_REG                       0x1E
typedef struct {
    uint8_t xlda                 : 1;
    uint8_t gda                  : 1;
    uint8_t tda                  : 1;
    uint8_t not_used_01          : 5;
} lsm6dsl_status_reg_t;

#define LSM6DSL_OUT_TEMP_L                       0x20
#define LSM6DSL_OUT_TEMP_H                       0x21
#define LSM6DSL_OUTX_L_G                         0x22
#define LSM6DSL_OUTX_H_G                         0x23
#define LSM6DSL_OUTY_L_G                         0x24
#define LSM6DSL_OUTY_H_G                         0x25
#define LSM6DSL_OUTZ_L_G                         0x26
#define LSM6DSL_OUTZ_H_G                         0x27
#define LSM6DSL_OUTX_L_XL                        0x28
#define LSM6DSL_OUTX_H_XL                        0x29
#define LSM6DSL_OUTY_L_XL                        0x2A
#define LSM6DSL_OUTY_H_XL                        0x2B
#define LSM6DSL_OUTZ_L_XL                        0x2C
#define LSM6DSL_OUTZ_H_XL                        0x2D
#define LSM6DSL_SENSORHUB1_REG                   0x2E
#define LSM6DSL_SENSORHUB2_REG                   0x2F
#define LSM6DSL_SENSORHUB3_REG                   0x30
#define LSM6DSL_SENSORHUB4_REG                   0x31
#define LSM6DSL_SENSORHUB5_REG                   0x32
#define LSM6DSL_SENSORHUB6_REG                   0x33
#define LSM6DSL_SENSORHUB7_REG                   0x34
#define LSM6DSL_SENSORHUB8_REG                   0x35
#define LSM6DSL_SENSORHUB9_REG                   0x36
#define LSM6DSL_SENSORHUB10_REG                  0x37
#define LSM6DSL_SENSORHUB11_REG                  0x38
#define LSM6DSL_SENSORHUB12_REG                  0x39
#define LSM6DSL_FIFO_STATUS1                     0x3A
#define LSM6DSL_FIFO_STATUS2                     0x3B
typedef struct {
    uint8_t diff_fifo            : 3; /** + +FIFO_STATUS1 **/
    uint8_t not_used_01          : 1;
    uint8_t fifo_empty           : 1;
    uint8_t fifo_full_smart      : 1;
    uint8_t over_run             : 1;
    uint8_t waterm               : 1;
} lsm6dsl_fifo_status2_t;

#define LSM6DSL_FIFO_STATUS3                     0x3C
#define LSM6DSL_FIFO_STATUS4                     0x3D
#define LSM6DSL_FIFO_DATA_OUT_L                  0x3E
#define LSM6DSL_FIFO_DATA_OUT_H                  0x3F
#define LSM6DSL_TIMESTAMP0_REG                   0x40
#define LSM6DSL_TIMESTAMP1_REG                   0x41
#define LSM6DSL_TIMESTAMP2_REG                   0x42
#define LSM6DSL_STEP_TIMESTAMP_L                 0x49
#define LSM6DSL_STEP_TIMESTAMP_H                 0x4A
#define LSM6DSL_STEP_COUNTER_L                   0x4B
#define LSM6DSL_STEP_COUNTER_H                   0x4C
#define LSM6DSL_SENSORHUB13_REG                  0x4D
#define LSM6DSL_SENSORHUB14_REG                  0x4E
#define LSM6DSL_SENSORHUB15_REG                  0x4F
#define LSM6DSL_SENSORHUB16_REG                  0x50
#define LSM6DSL_SENSORHUB17_REG                  0x51
#define LSM6DSL_SENSORHUB18_REG                  0x52
#define LSM6DSL_FUNC_SRC1                        0x53
typedef struct {
    uint8_t sensorhub_end_op     : 1;
    uint8_t si_end_op            : 1;
    uint8_t hi_fail              : 1;
    uint8_t step_overflow        : 1;
    uint8_t step_detected        : 1;
    uint8_t tilt_ia              : 1;
    uint8_t sign_motion_ia       : 1;
    uint8_t  step_count_delta_ia : 1;
} lsm6dsl_func_src1_t;

#define LSM6DSL_FUNC_SRC2                        0x54
typedef struct {
    uint8_t wrist_tilt_ia        : 1;
    uint8_t not_used_01          : 2;
    uint8_t slave0_nack          : 1;
    uint8_t slave1_nack          : 1;
    uint8_t slave2_nack          : 1;
    uint8_t slave3_nack          : 1;
    uint8_t not_used_02          : 1;
} lsm6dsl_func_src2_t;

#define LSM6DSL_WRIST_TILT_IA                    0x55
typedef struct {
    uint8_t not_used_01          : 2;
    uint8_t wrist_tilt_ia_zneg   : 1;
    uint8_t wrist_tilt_ia_zpos   : 1;
    uint8_t wrist_tilt_ia_yneg   : 1;
    uint8_t wrist_tilt_ia_ypos   : 1;
    uint8_t wrist_tilt_ia_xneg   : 1;
    uint8_t wrist_tilt_ia_xpos   : 1;
} lsm6dsl_wrist_tilt_ia_t;

#define LSM6DSL_TAP_CFG                          0x58
typedef struct {
    uint8_t lir                  : 1;
    uint8_t tap_z_en             : 1;
    uint8_t tap_y_en             : 1;
    uint8_t tap_x_en             : 1;
    uint8_t slope_fds            : 1;
    uint8_t inact_en             : 2;
    uint8_t interrupts_enable    : 1;
} lsm6dsl_tap_cfg_t;

#define LSM6DSL_TAP_THS_6D                       0x59
typedef struct {
    uint8_t tap_ths              : 5;
    uint8_t sixd_ths             : 2;
    uint8_t d4d_en               : 1;
} lsm6dsl_tap_ths_6d_t;

#define LSM6DSL_INT_DUR2                         0x5A
typedef struct {
    uint8_t shock                : 2;
    uint8_t quiet                : 2;
    uint8_t dur                  : 4;
} lsm6dsl_int_dur2_t;

#define LSM6DSL_WAKE_UP_THS                      0x5B
typedef struct {
    uint8_t wk_ths               : 6;
    uint8_t not_used_01          : 1;
    uint8_t single_double_tap    : 1;
} lsm6dsl_wake_up_ths_t;

#define LSM6DSL_WAKE_UP_DUR                      0x5C
typedef struct {
    uint8_t sleep_dur            : 4;
    uint8_t timer_hr             : 1;
    uint8_t wake_dur             : 2;
    uint8_t ff_dur5              : 1; /** +  + FF_DUR[4:0] in FREE_FALL (5Dh)  **/
} lsm6dsl_wake_up_dur_t;

#define LSM6DSL_FREE_FALL                        0x5D
typedef struct {
    uint8_t ff_ths               : 3;
    uint8_t ff_dur               : 5; /** +  + FF_DUR[5] in WAKE_UP_DUR (5Ch)  **/
} lsm6dsl_free_fall_t;

#define LSM6DSL_MD1_CFG                          0x5E
typedef struct {
    uint8_t int1_timer           : 1;
    uint8_t int1_tilt            : 1;
    uint8_t int1_6d              : 1;
    uint8_t int1_double_tap      : 1;
    uint8_t int1_ff              : 1;
    uint8_t int1_wu              : 1;
    uint8_t int1_single_tap      : 1;
    uint8_t int1_inact_state     : 1;
} lsm6dsl_md1_cfg_t;

#define LSM6DSL_MD2_CFG                          0x5F
typedef struct {
    uint8_t int2_iron            : 1;
    uint8_t int2_tilt            : 1;
    uint8_t int2_6d              : 1;
    uint8_t int2_double_tap      : 1;
    uint8_t int2_ff              : 1;
    uint8_t int2_wu              : 1;
    uint8_t int2_single_tap      : 1;
    uint8_t int2_inact_state     : 1;
} lsm6dsl_md2_cfg_t;

#define LSM6DSL_MASTER_CMD_CODE                  0x60
#define LSM6DSL_SENS_SYNC_SPI_ERROR_CODE         0x61
#define LSM6DSL_OUT_MAG_RAW_X_L                  0x66
#define LSM6DSL_OUT_MAG_RAW_X_H                  0x67
#define LSM6DSL_OUT_MAG_RAW_Y_L                  0x68
#define LSM6DSL_OUT_MAG_RAW_Y_H                  0x69
#define LSM6DSL_OUT_MAG_RAW_Z_L                  0x6A
#define LSM6DSL_OUT_MAG_RAW_Z_H                  0x6B
#define LSM6DSL_X_OFS_USR                        0x73
#define LSM6DSL_Y_OFS_USR                        0x74
#define LSM6DSL_Z_OFS_USR                        0x75

/**
  * @}
  */

int32_t lsm6dsl_read_reg(lsm6dsl_ctx_t *ctx, uint8_t reg, uint8_t *data,
                         uint16_t len);
int32_t lsm6dsl_write_reg(lsm6dsl_ctx_t *ctx, uint8_t reg, uint8_t *data,
                          uint16_t len);

typedef enum {
    DRDY_LATCHED = 0,
    DRDY_PULSED  = 1,
} lsm6dsl_drdy_pulsed_t;
int32_t lsm6dsl_drdy_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_drdy_pulsed_t val);
int32_t lsm6dsl_drdy_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_drdy_pulsed_t *val);

typedef enum {
    XL_OFF       = 0,
    XL_12Hz5     = 1,
    XL_26Hz      = 2,
    XL_52Hz      = 3,
    XL_104Hz     = 4,
    XL_208Hz     = 5,
    XL_416Hz     = 6,
    XL_833Hz     = 7,
    XL_1kHz66Hz  = 8,
    XL_3kHz33Hz  = 9,
    XL_6kHz66Hz  = 10,
    XL_LP_1Hz6   = 11,
} lsm6dsl_odr_xl_t;
int32_t lsm6dsl_xl_data_rate_set(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_xl_t val);
int32_t lsm6dsl_xl_data_rate_get(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_xl_t *val);

typedef enum {
    FS_2g   = 0,
    FS_16g  = 1,
    FS_4g   = 2,
    FS_8g   = 3,
} lsm6dsl_fs_xl_t;
int32_t lsm6dsl_xl_full_scale_set(lsm6dsl_ctx_t *ctx, lsm6dsl_fs_xl_t val);
int32_t lsm6dsl_xl_full_scale_get(lsm6dsl_ctx_t *ctx, lsm6dsl_fs_xl_t *val);

typedef enum {
    GY_OFF       = 0,
    GY_12Hz5     = 1,
    GY_26Hz      = 2,
    GY_52Hz      = 3,
    GY_104Hz     = 4,
    GY_208Hz     = 5,
    GY_416Hz     = 6,
    GY_833Hz     = 7,
    GY_1kHz66Hz  = 8,
    GY_3kHz33Hz  = 9,
    GY_6kHz66Hz  = 10,
} lsm6dsl_odr_g_t;
int32_t lsm6dsl_gyro_data_rate_set(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_g_t val);
int32_t lsm6dsl_gyro_data_rate_get(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_g_t *val);

typedef enum {
    FS_250dps    = 0,
    FS_500dps    = 2,
    FS_1000dps   = 4,
    FS_2000dps   = 6,
    FS_125dps    = 1,
} lsm6dsl_fs_g_t;
int32_t lsm6dsl_gyro_full_scale_set(lsm6dsl_ctx_t *ctx, lsm6dsl_fs_g_t val);
int32_t lsm6dsl_gyro_full_scale_get(lsm6dsl_ctx_t *ctx, lsm6dsl_fs_g_t *val);

int32_t lsm6dsl_reset_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_reset_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_block_data_update_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_block_data_update_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_gyro_lpf1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_gyro_lpf1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_device_id(lsm6dsl_ctx_t *ctx, uint8_t *buff);
int32_t lsm6dsl_temperature_raw(lsm6dsl_ctx_t *ctx, axis1bit16_t *buff);
int32_t lsm6dsl_angular_rate_raw(lsm6dsl_ctx_t *ctx, axis3bit16_t *buff);
int32_t lsm6dsl_acceleration_raw(lsm6dsl_ctx_t *ctx, axis3bit16_t *buff);
int32_t lsm6dsl_sensorhub_set1_out_raw(lsm6dsl_ctx_t *ctx, uint8_t *buff);
int32_t lsm6dsl_sensorhub_set2_out_raw(lsm6dsl_ctx_t *ctx, uint8_t *buff);

typedef enum {
    USR_BANK   = 0,
    BANK_A     = 4,
    BANK_B     = 5,
} lsm6dsl_func_cfg_en_t;
int32_t lsm6dsl_change_bank_set(lsm6dsl_ctx_t *ctx, lsm6dsl_func_cfg_en_t val);
int32_t lsm6dsl_change_bank_get(lsm6dsl_ctx_t *ctx, lsm6dsl_func_cfg_en_t *val);

typedef struct {
    lsm6dsl_wake_up_src_t         wake_up_src;
    lsm6dsl_tap_src_t             tap_src;
    lsm6dsl_d6d_src_t             d6d_src;
    lsm6dsl_status_reg_t          status_reg;
    lsm6dsl_func_src1_t           func_src1;
    lsm6dsl_func_src2_t           func_src2;
    lsm6dsl_wrist_tilt_ia_t  wrist_tilt_ia;
} lsm6dsl_multi_status_t;
int32_t lsm6dsl_status_get(lsm6dsl_ctx_t *ctx, lsm6dsl_multi_status_t *val);

int32_t lsm6dsl_wrist_tilt_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_wrist_tilt_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_xl_drdy_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_xl_drdy_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_gyro_drdy_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_gyro_drdy_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_boot_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_boot_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_fifo_threshold_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_threshold_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_fifo_overrun_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_overrun_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_fifo_full_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_full_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_sign_motion_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_sign_motion_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_step_detect_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_step_detect_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_xl_drdy_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_xl_drdy_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_gyro_drdy_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_gyro_drdy_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_temperature_drdy_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_temperature_drdy_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_fifo_threshold_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_threshold_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_fifo_overrun_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_overrun_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_fifo_full_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_full_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_step_overflow_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_step_overflow_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_step_on_time_interval_on_int2_set(lsm6dsl_ctx_t *ctx,
        uint8_t val);
int32_t lsm6dsl_step_on_time_interval_on_int2_get(lsm6dsl_ctx_t *ctx,
        uint8_t *val);

int32_t lsm6dsl_fifo_temperature_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_temperature_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    ON_DRDY           = 0,
    ON_STEP_COUNTER   = 1,
} lsm6dsl_timer_pedo_fifo_drdy_t;
int32_t lsm6dsl_fifo_write_mode_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_timer_pedo_fifo_drdy_t val);
int32_t lsm6dsl_fifo_write_mode_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_timer_pedo_fifo_drdy_t *val);

int32_t lsm6dsl_fifo_step_timestamp_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_step_timestamp_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    XL_NO_FILL  = 0,
    XL_NO_DEC   = 1,
    XL_DEC_2    = 2,
    XL_DEC_3    = 3,
    XL_DEC_4    = 4,
    XL_DEC_8    = 5,
    XL_DEC_16   = 6,
    XL_DEC_32   = 7,
} lsm6dsl_dec_fifo_xl_t;
int32_t lsm6dsl_xl_decimation_set(lsm6dsl_ctx_t *ctx,
                                  lsm6dsl_dec_fifo_xl_t val);
int32_t lsm6dsl_xl_decimation_get(lsm6dsl_ctx_t *ctx,
                                  lsm6dsl_dec_fifo_xl_t *val);

typedef enum {
    GY_NO_FILL  = 0,
    GY_NO_DEC   = 1,
    GY_DEC_2    = 2,
    GY_DEC_3    = 3,
    GY_DEC_4    = 4,
    GY_DEC_8    = 5,
    GY_DEC_16   = 6,
    GY_DEC_32   = 7,
} lsm6dsl_dec_fifo_gyro_t;
int32_t lsm6dsl_gyro_decimation_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_dec_fifo_gyro_t val);
int32_t lsm6dsl_gyro_decimation_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_dec_fifo_gyro_t *val);

typedef enum {
    DS3_NO_FILL = 0,
    DS3_NO_DEC  = 1,
    DS3_DEC_2   = 2,
    DS3_DEC_3   = 3,
    DS3_DEC_4   = 4,
    DS3_DEC_8   = 5,
    DS3_DEC_16  = 6,
    DS3_DEC_32  = 7,
} lsm6dsl_dec_ds3_fifo_t;
int32_t lsm6dsl_dataset3_decimation_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_dec_ds3_fifo_t val);
int32_t lsm6dsl_dataset3_decimation_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_dec_ds3_fifo_t *val);

typedef enum {
    DS4_NO_FILL  = 0,
    DS4_NO_DEC   = 1,
    DS4_DEC_2    = 2,
    DS4_DEC_3    = 3,
    DS4_DEC_4    = 4,
    DS4_DEC_8    = 5,
    DS4_DEC_16   = 6,
    DS4_DEC_32   = 7,
} lsm6dsl_dec_ds4_fifo_t;
int32_t lsm6dsl_dataset4_decimation_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_dec_ds4_fifo_t val);
int32_t lsm6dsl_dataset4_decimation_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_dec_ds4_fifo_t *val);

int32_t lsm6dsl_fifo_xl_gy_8bit_data_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_xl_gy_8bit_data_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_fifo_stop_on_thr_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_fifo_stop_on_thr_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    BYPASS                = 0,
    STOP_WHEN_FULL        = 1,
    CONTINUOUS_TO_FIFO    = 3,
    BYPASS_TO_CONTINUOUS  = 4,
    CONTINUOS             = 6,
} lsm6dsl_fifo_mode_t;
int32_t lsm6dsl_fifo_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_fifo_mode_t val);
int32_t lsm6dsl_fifo_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_fifo_mode_t *val);

typedef enum {
    FIFO_OFF       = 0,
    FIFO_12Hz5     = 1,
    FIFO_26Hz      = 2,
    FIFO_52Hz      = 3,
    FIFO_104Hz     = 4,
    FIFO_208Hz     = 5,
    FIFO_416Hz     = 6,
    FIFO_833Hz     = 7,
    FIFO_1kHz66Hz  = 8,
    FIFO_3kHz33Hz  = 9,
    FIFO_6kHz66Hz  = 10,
} lsm6dsl_odr_fifo_t;
int32_t lsm6dsl_fifo_data_rate_set(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_fifo_t val);
int32_t lsm6dsl_fifo_data_rate_get(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_fifo_t *val);

typedef enum {
    XL_GY_DRDY_STEP_DETECT   = 0,
    SENS_HUB_DRDY             = 1,
} lsm6dsl_data_valid_sel_fifo_t;
int32_t lsm6dsl_fifo_data_valid_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_data_valid_sel_fifo_t val);
int32_t lsm6dsl_fifo_data_valid_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_data_valid_sel_fifo_t *val);

int32_t lsm6dsl_fifo_empty_get(lsm6dsl_ctx_t *ctx);

int32_t lsm6dsl_fifo_full_next_cycle_get(lsm6dsl_ctx_t *ctx);

int32_t lsm6dsl_fifo_over_run_get(lsm6dsl_ctx_t *ctx);

int32_t lsm6dsl_fifo_watermark_level_get(lsm6dsl_ctx_t *ctx);

typedef enum {
    FROM_SLOPE   = 0,
    FROM_HPF     = 1,
} lsm6dsl_slope_fds_t;
int32_t lsm6dsl_data_for_wake_up_set(lsm6dsl_ctx_t *ctx,
                                     lsm6dsl_slope_fds_t val);
int32_t lsm6dsl_data_for_wake_up_get(lsm6dsl_ctx_t *ctx,
                                     lsm6dsl_slope_fds_t *val);

int32_t lsm6dsl_fifo_threshold_set(lsm6dsl_ctx_t *ctx, uint16_t val);
int32_t lsm6dsl_fifo_threshold_get(lsm6dsl_ctx_t *ctx, uint16_t *val);

int32_t lsm6dsl_fifo_unread_words_set(lsm6dsl_ctx_t *ctx, uint16_t val);
int32_t lsm6dsl_fifo_unread_words_get(lsm6dsl_ctx_t *ctx, uint16_t *val);

int32_t lsm6dsl_fifo_recursive_pattern_set(lsm6dsl_ctx_t *ctx, uint16_t val);
int32_t lsm6dsl_fifo_recursive_pattern_get(lsm6dsl_ctx_t *ctx, uint16_t *val);

int32_t lsm6dsl_fifo_output_raw(lsm6dsl_ctx_t *ctx, axis1bit16_t *buff);

typedef enum {
    AN_BW_1kHz5   = 0,
    AN_BW_400Hz   = 1,
} lsm6dsl_bw0_xl_t;
int32_t lsm6dsl_xl_analog_bandwidth_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_bw0_xl_t val);
int32_t lsm6dsl_xl_analog_bandwidth_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_bw0_xl_t *val);

typedef enum {
    ODR_DIV_2 = 0,
    ODR_DIV_4 = 1,
} lsm6dsl_lpf1_bw_sel_t;
int32_t lsm6dsl_xl_lpf1_bandwidth_set(lsm6dsl_ctx_t *ctx,
                                      lsm6dsl_lpf1_bw_sel_t val);
int32_t lsm6dsl_xl_lpf1_bandwidth_get(lsm6dsl_ctx_t *ctx,
                                      lsm6dsl_lpf1_bw_sel_t *val);

typedef enum {
    LSB_IN_LOW_ADDRESS  = 0,
    MSB_IN_LOW_ADDRESS  = 1,
} lsm6dsl_ble_t;
int32_t lsm6dsl_data_format_set(lsm6dsl_ctx_t *ctx, lsm6dsl_ble_t val);
int32_t lsm6dsl_data_format_get(lsm6dsl_ctx_t *ctx, lsm6dsl_ble_t *val);

int32_t lsm6dsl_auto_increment_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_auto_increment_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    SPI_4_WIRE  = 0,
    SPI_3_WIRE  = 1,
} lsm6dsl_sim_t;
int32_t lsm6dsl_spi_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_sim_t val);
int32_t lsm6dsl_spi_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_sim_t *val);

typedef enum {
    PUSH_PULL   = 0,
    OPEN_DRAIN  = 1,
} lsm6dsl_pp_od_t;
int32_t lsm6dsl_pads_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_pp_od_t val);
int32_t lsm6dsl_pads_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_pp_od_t *val);

typedef enum {
    ACTIVE_HIGH  = 0,
    ACTIVE_LOW   = 1,
} lsm6dsl_h_lactive_t;
int32_t lsm6dsl_pads_polatity_set(lsm6dsl_ctx_t *ctx,
                                  lsm6dsl_h_lactive_t val);
int32_t lsm6dsl_pads_polatity_get(lsm6dsl_ctx_t *ctx,
                                  lsm6dsl_h_lactive_t *val);

int32_t lsm6dsl_i2c_interface_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_i2c_interface_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_mask_drdy_on_settling_data_set(lsm6dsl_ctx_t *ctx,
        uint8_t val);
int32_t lsm6dsl_mask_drdy_on_settling_data_get(lsm6dsl_ctx_t *ctx,
        uint8_t *val);

int32_t lsm6dsl_all_on_int1_pad_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_all_on_int1_pad_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_all_on_int1_pad_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_all_on_int1_pad_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_gyro_sleep_mode_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_gyro_sleep_mode_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_den_func_on_xl_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_den_func_on_xl_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    XL_ST_DISABLE   = 0,
    XL_ST_POSITIVE  = 1,
    XL_ST_NEGATIVE  = 2,
} lsm6dsl_st_xl_t;
int32_t lsm6dsl_xl_self_test_set(lsm6dsl_ctx_t *ctx, lsm6dsl_st_xl_t val);
int32_t lsm6dsl_xl_self_test_get(lsm6dsl_ctx_t *ctx, lsm6dsl_st_xl_t *val);

typedef enum {
    GY_ST_DISABLE   = 0,
    GY_ST_POSITIVE  = 1,
    GY_ST_NEGATIVE  = 3,
} lsm6dsl_st_g_t;
int32_t lsm6dsl_gyro_self_test_set(lsm6dsl_ctx_t *ctx, lsm6dsl_st_g_t val);
int32_t lsm6dsl_gyro_self_test_get(lsm6dsl_ctx_t *ctx, lsm6dsl_st_g_t *val);

typedef enum {
    DEN_LOW   = 0,
    DEN_HIGH  = 1,
} lsm6dsl_den_lh_t;
int32_t lsm6dsl_den_func_polarity_set(lsm6dsl_ctx_t *ctx,
                                      lsm6dsl_den_lh_t val);
int32_t lsm6dsl_den_func_polarity_get(lsm6dsl_ctx_t *ctx,
                                      lsm6dsl_den_lh_t *val);

typedef enum {
    ROUNDING_OFF           = 0,
    XL                     = 1,
    GYRO                   = 2,
    GYRO_XL                = 3,
    SENSORHUB1_6           = 4,
    XL_SENSORHUB1_6        = 5,
    GYRO_XL_SENSORHUB1_12  = 6,
    GYRO_XL_SENSORHUB1_6   = 7,
} lsm6dsl_rounding_t;
int32_t lsm6dsl_rounding_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_rounding_t val);
int32_t lsm6dsl_rounding_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_rounding_t *val);

typedef enum {
    GY_LPF1_MEDIUM            = 0,
    GY_LPF1_HIGH              = 1,
    GY_LPF1_VERY_AGGRESSIVE  = 2,
    GY_LPF1_AGGRESSIVE        = 3,
} lsm6dsl_ftype_t;
int32_t lsm6dsl_gyro_lpf1_bandwidth_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_ftype_t val);
int32_t lsm6dsl_gyro_lpf1_bandwidth_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_ftype_t *val);

typedef enum {
    LSb_1mg    = 0,
    LSb_16mg   = 1,
} lsm6dsl_usr_off_w_t;
int32_t lsm6dsl_xl_offset_weight_set(lsm6dsl_ctx_t *ctx,
                                     lsm6dsl_usr_off_w_t val);
int32_t lsm6dsl_xl_offset_weight_get(lsm6dsl_ctx_t *ctx,
                                     lsm6dsl_usr_off_w_t *val);

typedef enum {
    XL_HP_ENABLE    = 0,
    XL_HP_DISABLE   = 1,
} lsm6dsl_xl_hm_mode_t;
int32_t lsm6dsl_xl_high_performance_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_xl_hm_mode_t val);
int32_t lsm6dsl_xl_high_performance_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_xl_hm_mode_t *val);

typedef enum {
    LEVEL_FIFO     = 6,
    LEVEL_LETCHED  = 3,
    LEVEL_TRIGGER  = 2,
    EDGE_TRIGGER   = 4,
} lsm6dsl_den_mode_t;
int32_t lsm6dsl_den_func_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_den_mode_t val);
int32_t lsm6dsl_den_func_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_den_mode_t *val);

int32_t lsm6dsl_rounding_on_status_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_rounding_on_status_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    GY_HP_16mHz   = 0,
    GY_HP_65mHz   = 1,
    GY_HP_260mHz  = 2,
    GY_HP_1Hz04   = 3,
} lsm6dsl_hpm_g_t;
int32_t lsm6dsl_gyro_hp_cutoff_set(lsm6dsl_ctx_t *ctx, lsm6dsl_hpm_g_t val);
int32_t lsm6dsl_gyro_hp_cutoff_get(lsm6dsl_ctx_t *ctx, lsm6dsl_hpm_g_t *val);

int32_t lsm6dsl_gyro_hp_filter_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_gyro_hp_filter_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    GY_HP_ENABLE   = 0,
    GY_HP_DISABLE  = 1,
} lsm6dsl_g_hm_mode_t;
int32_t lsm6dsl_gyro_high_performance_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_g_hm_mode_t val);
int32_t lsm6dsl_gyro_high_performance_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_g_hm_mode_t *val);

typedef enum {
    RAW_DIV_2    = 0,
    FROM_LPF2    = 1,
} lsm6dsl_low_pass_on_6d_t;
int32_t lsm6dsl_data_on_6d_set(lsm6dsl_ctx_t *ctx,
                               lsm6dsl_low_pass_on_6d_t val);
int32_t lsm6dsl_data_on_6d_get(lsm6dsl_ctx_t *ctx,
                               lsm6dsl_low_pass_on_6d_t *val);

typedef enum {
    NORMAL_OR_LOW_PASS_XL  = 0,
    HIGH_PASS_OR_SLOPE     = 1,
} lsm6dsl_hp_slope_xl_en_t;
int32_t lsm6dsl_xl_out_data_set(lsm6dsl_ctx_t *ctx,
                                lsm6dsl_hp_slope_xl_en_t val);
int32_t lsm6dsl_xl_out_data_get(lsm6dsl_ctx_t *ctx,
                                lsm6dsl_hp_slope_xl_en_t *val);

typedef enum {
    XL_CMP_ODR_DIV_2  = 0,
    XL_CMP_ODR_DIV_4  = 1,
} lsm6dsl_input_composite_t;
int32_t lsm6dsl_xl_composite_filter_input_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_input_composite_t val);
int32_t lsm6dsl_xl_composite_filter_input_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_input_composite_t *val);

int32_t lsm6dsl_xl_hp_ref_mode_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_xl_hp_ref_mode_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    XL_ODR_DIV_50_OR_DIV4  = 0,
    XL_ODR_DIV_100  = 1,
    XL_ODR_DIV_9    = 2,
    XL_ODR_DIV_400  = 3,
} lsm6dsl_hpcf_xl_t;
int32_t lsm6dsl_xl_cutoff_set(lsm6dsl_ctx_t *ctx, lsm6dsl_hpcf_xl_t val);
int32_t lsm6dsl_xl_cutoff_get(lsm6dsl_ctx_t *ctx, lsm6dsl_hpcf_xl_t *val);

int32_t lsm6dsl_soft_iron_correction_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_soft_iron_correction_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    DEN_ON_GYRO  = 0,
    DEN_ON_XL    = 1,
} lsm6dsl_den_xl_g_t;
int32_t lsm6dsl_den_stamping_selection_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_den_xl_g_t val);
int32_t lsm6dsl_den_stamping_selection_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_den_xl_g_t *val);

int32_t lsm6dsl_den_store_on_x_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_den_store_on_x_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_den_store_on_y_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_den_store_on_y_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_den_store_on_z_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_den_store_on_z_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_significant_motion_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_significant_motion_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_rst_step_counter_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_rst_step_counter_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_embedded_functions_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_embedded_functions_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_tilt_calculation_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_tilt_calculation_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_pedometer_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_pedometer_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_timestamp_count_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_timestamp_count_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_wrist_tilt_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_wrist_tilt_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_hard_iron_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_hard_iron_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_letched_interrupts_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_letched_interrupts_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_tap_on_z_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_tap_on_z_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_tap_on_y_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_tap_on_y_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_tap_on_x_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_tap_on_x_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    XL_GYRO_NO_CHANGE           = 0,
    XL_LP_12Hz5_GYRO_NO_CHANGE  = 1,
    XL_LP_12Hz5_GYRO_LP_12Hz5   = 2,
    XL_LP_12Hz5_GYRO_PD         = 3,
} lsm6dsl_inact_en_t;
int32_t lsm6dsl_inactivity_mode_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_inact_en_t val);
int32_t lsm6dsl_inactivity_mode_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_inact_en_t *val);

int32_t lsm6dsl_enable_basic_interrupts_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_enable_basic_interrupts_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    DEG_80  = 0,
    DEG_70  = 1,
    DEG_60  = 2,
    DEG_50  = 3,
} lsm6dsl_sixd_ths_t;
int32_t lsm6dsl_threshold_6d_3d_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_sixd_ths_t val);
int32_t lsm6dsl_threshold_6d_3d_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_sixd_ths_t *val);

int32_t lsm6dsl_reduce_6d_to_4d_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_reduce_6d_to_4d_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    ONLY_SINGLE_TAP     = 0,
    BOTH_SINGLE_DOUBLE  = 1,
} lsm6dsl_single_double_tap_t;
int32_t lsm6dsl_tap_mode_set(lsm6dsl_ctx_t *ctx,
                             lsm6dsl_single_double_tap_t val);
int32_t lsm6dsl_tap_mode_get(lsm6dsl_ctx_t *ctx,
                             lsm6dsl_single_double_tap_t *val);
typedef enum {
    LSb_25us  = 0,
    LSb_6ms4  = 1,
} lsm6dsl_timer_hr_t;
int32_t lsm6dsl_timestamp_resolution_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_timer_hr_t val);
int32_t lsm6dsl_timestamp_resolution_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_timer_hr_t *val);

typedef enum {
    FF_TSH_156mg = 0,
    FF_TSH_219mg = 1,
    FF_TSH_250mg = 2,
    FF_TSH_312mg = 3,
    FF_TSH_344mg = 4,
    FF_TSH_406mg = 5,
    FF_TSH_469mg = 6,
    FF_TSH_500mg = 7,
} lsm6dsl_ff_ths_t;
int32_t lsm6dsl_free_fall_threshold_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_ff_ths_t val);
int32_t lsm6dsl_free_fall_threshold_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_ff_ths_t *val);

int32_t lsm6dsl_int1_routing_set(lsm6dsl_ctx_t *ctx,
                                 lsm6dsl_md1_cfg_t val);
int32_t lsm6dsl_int1_routing_get(lsm6dsl_ctx_t *ctx,
                                 lsm6dsl_md1_cfg_t *val);

int32_t lsm6dsl_int2_routing_set(lsm6dsl_ctx_t *ctx,
                                 lsm6dsl_md2_cfg_t val);
int32_t lsm6dsl_int2_routing_get(lsm6dsl_ctx_t *ctx,
                                 lsm6dsl_md2_cfg_t *val);

int32_t lsm6dsl_sincro_time_frame_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_sincro_time_frame_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_timestamp_raw(lsm6dsl_ctx_t *ctx, axis1bit32_t *buff);

int32_t lsm6dsl_step_timestamp_raw(lsm6dsl_ctx_t *ctx, axis1bit16_t *buff);

int32_t lsm6dsl_step_counter_raw(lsm6dsl_ctx_t *ctx, axis1bit16_t *buff);

int32_t lsm6dsl_tap_treshold_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_tap_treshold_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_gap_double_tap_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_gap_double_tap_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_quiet_after_tap_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_quiet_after_tap_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_duration_tap_overthreshold_set(lsm6dsl_ctx_t *ctx,
        uint8_t val);
int32_t lsm6dsl_duration_tap_overthreshold_get(lsm6dsl_ctx_t *ctx,
        uint8_t *val);

int32_t lsm6dsl_threshold_wakeup_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_threshold_wakeup_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_wakeup_dur_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_wakeup_dur_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_wait_before_sleep_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_wait_before_sleep_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_free_fall_duration_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_free_fall_duration_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_master_error_code_raw(lsm6dsl_ctx_t *ctx, uint8_t *buff);

int32_t lsm6dsl_synchro_error_code_raw(lsm6dsl_ctx_t *ctx, uint8_t *buff);

int32_t lsm6dsl_ext_magnetometer_raw(lsm6dsl_ctx_t *ctx, axis3bit16_t *buff);

int32_t lsm6dsl_xl_x_axis_offset_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_xl_x_axis_offset_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_xl_y_axis_offset_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_xl_y_axis_offset_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_xl_z_axis_offset_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_xl_z_axis_offset_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_sensor_hub_i2c_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_sensor_hub_i2c_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

int32_t lsm6dsl_sensor_hub_pass_through_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_sensor_hub_pass_through_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    USE_EXTERNAL      = 0,
    ENABLE_INTERNAL   = 1,
} lsm6dsl_pull_up_en_t;
int32_t lsm6dsl_sensor_hub_pull_up_set(lsm6dsl_ctx_t *ctx,
                                       lsm6dsl_pull_up_en_t val);
int32_t lsm6dsl_sensor_hub_pull_up_get(lsm6dsl_ctx_t *ctx,
                                       lsm6dsl_pull_up_en_t *val);

typedef enum {
    XL_GY_DRDY   = 0,
    INT2_SIGNAL  = 1,
} lsm6dsl_start_config_t;
int32_t lsm6dsl_sensor_hub_op_trigger_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_start_config_t val);
int32_t lsm6dsl_sensor_hub_op_trigger_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_start_config_t *val);

int32_t lsm6dsl_sensor_hub_drdy_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val);
int32_t lsm6dsl_sensor_hub_drdy_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val);

typedef enum {
    RES_RATIO_2_11   = 0,
    RES_RATIO_2_12   = 1,
    RES_RATIO_2_13   = 2,
    RES_RATIO_2_14   = 3,
} lsm6dsl_rr_t;
int32_t lsm6dsl_res_ratio_set(lsm6dsl_ctx_t *ctx, lsm6dsl_rr_t val);
int32_t lsm6dsl_res_ratio_get(lsm6dsl_ctx_t *ctx, lsm6dsl_rr_t *val);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif


#endif
#endif
