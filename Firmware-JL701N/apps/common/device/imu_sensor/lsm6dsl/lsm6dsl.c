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
#include "imuSensor_manage.h"
#include "lsm6dsl.h"

// #include "spi1.h"
// #include "port_wkup.h"



#if TCFG_LSM6DSL_ENABLE

/*************Betterlife ic debug***********/
#undef LOG_TAG_CONST
#define LOG_TAG             "[LSM6DSL]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

void delay(volatile u32 t);
void udelay(u32 us);
#define   MDELAY(n)    mdelay(n)

static lsm6dsl_param *lsm6dsl_info;

#if (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_I2C)

#if TCFG_LSM6DSL_USER_IIC_TYPE
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
//起止信号间隔：standard mode(100khz):4.7us; fast mode(400khz):1.3us
//return: readLen:ok, other:fail
u16 lsm6dsl_I2C_Read_NBytes(unsigned char devAddr,
                            unsigned char regAddr,
                            unsigned char *readBuf,
                            u16 readLen)
{
    u16 i = 0;
    local_irq_disable();
    iic_start(lsm6dsl_info->iic_hdl);
    if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, devAddr)) {
        log_error("lsm6dsl iic read err1");
        goto __iic_exit_r;
    }
    delay(lsm6dsl_info->iic_delay);
    /* if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, regAddr |0x80)) {//|0x80地址自动递增 */
    if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, regAddr)) {//|0x80地址自动递增
        log_error("lsm6dsl iic read err2");
        goto __iic_exit_r;
    }
    delay(lsm6dsl_info->iic_delay);

    iic_start(lsm6dsl_info->iic_hdl);
    if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, devAddr + 1)) {
        log_error("lsm6dsl iic read err3");
        goto __iic_exit_r;
    }
    for (i = 0; i < readLen; i++) {
        delay(lsm6dsl_info->iic_delay);
        if (i == (readLen - 1)) {
            *readBuf++ = iic_rx_byte(lsm6dsl_info->iic_hdl, 0);
        } else {
            *readBuf++ = iic_rx_byte(lsm6dsl_info->iic_hdl, 1);
        }
        /* if(i%100==0)wdt_clear(); */
    }
__iic_exit_r:
    iic_stop(lsm6dsl_info->iic_hdl);
    local_irq_enable();

    return i;
}

//起止信号间隔：standard mode(100khz):4.7us; fast mode(400khz):1.3us
//return:writeLen:ok, other:fail
u16 lsm6dsl_I2C_Write_NBytes(unsigned char devAddr,
                             unsigned char regAddr,
                             unsigned char *writeBuf,
                             u16 writeLen)
{
    u16 i = 0;
    local_irq_disable();
    iic_start(lsm6dsl_info->iic_hdl);
    if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, devAddr)) {
        log_error("lsm6dsl iic write err1");
        goto __iic_exit_w;
    }
    delay(lsm6dsl_info->iic_delay);
    if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, regAddr)) {
        log_error("lsm6dsl iic write err2");
        goto __iic_exit_w;
    }

    for (i = 0; i < writeLen; i++) {
        delay(lsm6dsl_info->iic_delay);
        if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, writeBuf[i])) {
            log_error("lsm6dsl iic write err3:%d", i);
            goto __iic_exit_w;
        }
    }
__iic_exit_w:
    iic_stop(lsm6dsl_info->iic_hdl);
    local_irq_enable();
    return i;
}

//起止信号间隔：standard mode(100khz):4.7us; fast mode(400khz):1.3us
//return:1:ok, 0:fail
unsigned char lsm6dsl_I2C_Write_Byte(unsigned char devAddr,  //芯片支持1byte写
                                     unsigned char regAddr,
                                     unsigned char byte)
{
    u8 ret = 1;
    local_irq_disable();
    iic_start(lsm6dsl_info->iic_hdl);
    if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, devAddr)) {
        log_error("lsm6dsl iic write err1");
        ret = 0;
        goto __iic_exit_w;
    }
    delay(lsm6dsl_info->iic_delay);
    if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, regAddr)) {
        log_error("lsm6dsl iic write err2");
        ret = 0;
        goto __iic_exit_w;
    }
    delay(lsm6dsl_info->iic_delay);
    if (0 == iic_tx_byte(lsm6dsl_info->iic_hdl, byte)) {
        log_error("lsm6dsl iic write err3");
        ret = 0;
        goto __iic_exit_w;
    }
__iic_exit_w:
    iic_stop(lsm6dsl_info->iic_hdl);
    local_irq_enable();
    return ret;
}
IMU_read    lsm6dsl_read     = lsm6dsl_I2C_Read_NBytes;
IMU_write   lsm6dsl_write    = lsm6dsl_I2C_Write_Byte;

#elif (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_SPI)

#define spi_cs_init() \
    do { \
        gpio_write(lsm6dsl_info->spi_cs_pin, 1); \
        gpio_set_direction(lsm6dsl_info->spi_cs_pin, 0); \
        gpio_set_die(lsm6dsl_info->spi_cs_pin, 1); \
    } while (0)
#define spi_cs_uninit() \
    do { \
        gpio_set_die(lsm6dsl_info->spi_cs_pin, 0); \
        gpio_set_direction(lsm6dsl_info->spi_cs_pin, 1); \
        gpio_set_pull_up(lsm6dsl_info->spi_cs_pin, 0); \
        gpio_set_pull_down(lsm6dsl_info->spi_cs_pin, 0); \
    } while (0)
#define spi_cs_h()                  gpio_write(lsm6dsl_info->spi_cs_pin, 1)
#define spi_cs_l()                  gpio_write(lsm6dsl_info->spi_cs_pin, 0)

#define spi_read_byte()             spi_recv_byte(lsm6dsl_info->spi_hdl, NULL)
#define spi_write_byte(x)           spi_send_byte(lsm6dsl_info->spi_hdl, x)
#define spi_dma_read(x, y)          spi_dma_recv(lsm6dsl_info->spi_hdl, x, y)
#define spi_dma_write(x, y)         spi_dma_send(lsm6dsl_info->spi_hdl, x, y)
#define spi_set_width(x)            spi_set_bit_mode(lsm6dsl_info->spi_hdl, x)
#define spi_init()              spi_open(lsm6dsl_info->spi_hdl)
#define spi_closed()            spi_close(lsm6dsl_info->spi_hdl)
#define spi_suspend()           hw_spi_suspend(lsm6dsl_info->spi_hdl)
#define spi_resume()            hw_spi_resume(lsm6dsl_info->spi_hdl)

u16 lsm6dsl_SPI_readNBytes(unsigned char devAddr,
                           unsigned char regAddr,
                           unsigned char *readBuf,
                           u16 readLen)
{
    spi_cs_l();
    spi_write_byte(regAddr | 0x80);//| 0x80:read mode
    spi_dma_read(readBuf, readLen);
    spi_cs_h();
    return (readLen);
}
unsigned char lsm6dsl_SPI_writeByte(unsigned char devAddr,
                                    unsigned char regAddr,
                                    unsigned char writebyte)
{
    spi_cs_l();
    spi_write_byte(regAddr);
    spi_write_byte(writebyte);
    spi_cs_h();
    udelay(5);//delay5us
    return (1);
}
u16 lsm6dsl_SPI_writeNBytes(unsigned char devAddr,
                            unsigned char regAddr,
                            unsigned char *writeBuf,
                            u16 writeLen)
{
#if 1 //多字节dma写
    spi_cs_l();
    spi_write_byte(regAddr);
    spi_dma_write(writeBuf, writeLen);
    spi_cs_h();
#else
    u16 i = 0;
    for (; i < writeLen; i++) {
        lsm6dsl_SPI_writeByte(devAddr, regAddr + i, writeBuf[i]);
    }
#endif
    //SPIWrite((regAddr & 0x7F), writeBuf, writeLen);
    return (i);
}

IMU_read 	lsm6dsl_read	= lsm6dsl_SPI_readNBytes;
IMU_write	lsm6dsl_write	= lsm6dsl_SPI_writeByte;
#endif




static int32_t platform_write(void *handle, uint8_t Reg, uint8_t *Bufp,
                              uint16_t len)
{
    /* if(handle==iic){} */
#if (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_I2C)
    return lsm6dsl_I2C_Write_NBytes(LSM6DSL_IIC_SLAVE_ADDRESS, Reg, Bufp, len);
#elif (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_SPI)
    return lsm6dsl_SPI_writeNBytes(0, Reg, Bufp, len);
#endif
}

static int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp,
                             uint16_t len)
{
    /* if(handle==iic){} */
#if (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_I2C)
    return lsm6dsl_I2C_Read_NBytes(LSM6DSL_IIC_SLAVE_ADDRESS, Reg, Bufp, len);
#elif (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_SPI)
    return lsm6dsl_SPI_readNBytes(0, Reg, Bufp, len);
#endif
}
lsm6dsl_ctx_t lsm6dsl_ctx = {
    .write_reg = platform_write,
    .read_reg = platform_read,
    .handle = NULL//spi or iic
};




/*
 ******************************************************************************
 * @file    lsm6dsl_reg.c
 * @author  MEMS Software Solution Team
 * @date    30-August-2017
 * @brief    driver file
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


/**
  * @addtogroup  lsm6dsl
  * @brief  This file provides a set of functions needed to drive the
  *         lsm6dsl enanced inertial module.
  * @{
  */

/**
  * @addtogroup  interfaces_functions
  * @brief  This section provide a set of functions used to read and write
  *         a generic register of the device.
  * @{
  */

/**
  * @brief  Read generic device register
  *
  * @param  lsm6dsl_ctx_t* ctx: read / write interface definitions
  * @param  uint8_t reg: register to read
  * @param  uint8_t* data: pointer to buffer that store the data read
  * @param  uint16_t len: number of consecutive register to read
  *
  */

int32_t lsm6dsl_read_reg(lsm6dsl_ctx_t *ctx, uint8_t reg, uint8_t *data,
                         uint16_t len)
{
    return ctx->read_reg(ctx->handle, reg, data, len);
}

/**
  * @brief  Write generic device register
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t reg: register to write
  * @param  uint8_t* data: pointer to data to write in register reg
  * @param  uint16_t len: number of consecutive register to write
  *
*/
int32_t lsm6dsl_write_reg(lsm6dsl_ctx_t *ctx, uint8_t reg, uint8_t *data,
                          uint16_t len)
{
    return ctx->write_reg(ctx->handle, reg, data, len);
}

/**
  * @}
  */

/** @addtogroup common_functions
  * @brief     This section provide a set of most common functions used
  *            to drive the device.
  * @{
  */

/**
  * @brief  drdy_mode: [set]  Define DataReady mode, pulses are 75 μs long,
  *                    latched on outputs reads
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_drdy_pulsed_t: change the values of drdy_pulsed
  *                                in reg DRDY_PULSE_CFG_G
  *
  */
int32_t lsm6dsl_drdy_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_drdy_pulsed_t val)
{
    lsm6dsl_drdy_pulse_cfg_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_DRDY_PULSE_CFG_G,
                                (uint8_t *)&reg, 1);
    reg.drdy_pulsed = val;

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_DRDY_PULSE_CFG_G,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  drdy_mode: [get]  Define DataReady mode, pulses are 75 μs long,
  *                    latched on outputs reads
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_drdy_pulsed_t: Get the values of drdy_pulsed
  *                                in reg DRDY_PULSE_CFG_G
  *
  */
int32_t lsm6dsl_drdy_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_drdy_pulsed_t *val)
{
    lsm6dsl_drdy_pulse_cfg_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_DRDY_PULSE_CFG_G,
                                (uint8_t *)&reg, 1);
    *val = (lsm6dsl_drdy_pulsed_t) reg.drdy_pulsed;

    return mm_error;
}

/**
  * @brief  xl_data_rate: [set]  Accelerometer data rate selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_odr_xl_t: change the values of odr_xl in reg CTRL1_XL
  *
  */
int32_t lsm6dsl_xl_data_rate_set(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_xl_t val)
{
    lsm6dsl_ctrl1_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);
    reg.odr_xl = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_data_rate: [get]  Accelerometer data rate selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_odr_xl_t: Get the values of odr_xl in reg CTRL1_XL
  *
  */
int32_t lsm6dsl_xl_data_rate_get(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_xl_t *val)
{
    lsm6dsl_ctrl1_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_odr_xl_t) reg.odr_xl;

    return mm_error;
}

/**
  * @brief  xl_full_scale: [set]  Accelerometer full-scale selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_fs_xl_t: change the values of fs_xl in reg CTRL1_XL
  *
  */
int32_t lsm6dsl_xl_full_scale_set(lsm6dsl_ctx_t *ctx, lsm6dsl_fs_xl_t val)
{
    lsm6dsl_ctrl1_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);
    reg.fs_xl = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_full_scale: [get]  Accelerometer full-scale selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_fs_xl_t: Get the values of fs_xl in reg CTRL1_XL
  *
  */
int32_t lsm6dsl_xl_full_scale_get(lsm6dsl_ctx_t *ctx, lsm6dsl_fs_xl_t *val)
{
    lsm6dsl_ctrl1_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_fs_xl_t) reg.fs_xl;

    return mm_error;
}

/**
  * @brief  gyro_data_rate: [set]  Gyroscope data rate selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_odr_g_t: change the values of odr_g in reg CTRL2_G
  *
  */
int32_t lsm6dsl_gyro_data_rate_set(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_g_t val)
{
    lsm6dsl_ctrl2_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL2_G, (uint8_t *)&reg, 1);
    reg.odr_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL2_G, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_data_rate: [get]  Gyroscope data rate selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_odr_g_t: Get the values of odr_g in reg CTRL2_G
  *
  */
int32_t lsm6dsl_gyro_data_rate_get(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_g_t *val)
{
    lsm6dsl_ctrl2_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL2_G, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_odr_g_t) reg.odr_g;

    return mm_error;
}

/**
  * @brief  gyro_full_scale: [set]  Gyroscope full-scale selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_fs_g_t: change the values of fs_g in reg CTRL2_G
  *
  */
int32_t lsm6dsl_gyro_full_scale_set(lsm6dsl_ctx_t *ctx, lsm6dsl_fs_g_t val)
{
    lsm6dsl_ctrl2_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL2_G, (uint8_t *)&reg, 1);
    reg.fs_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL2_G, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_full_scale: [get]  Gyroscope full-scale selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_fs_g_t: Get the values of fs_g in reg CTRL2_G
  *
  */
int32_t lsm6dsl_gyro_full_scale_get(lsm6dsl_ctx_t *ctx, lsm6dsl_fs_g_t *val)
{
    lsm6dsl_ctrl2_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL2_G, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_fs_g_t) reg.fs_g;

    return mm_error;
}

/**
  * @brief  reset: [set]  Software reset. This bit is automatically cleared.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of sw_reset in reg CTRL3_C
  *
  */
int32_t lsm6dsl_reset_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    reg.sw_reset = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  reset: [get]  Software reset. This bit is automatically cleared.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of sw_reset in reg CTRL3_C
  *
  */
int32_t lsm6dsl_reset_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    *val = reg.sw_reset;

    return mm_error;
}

/**
  * @brief  block_data_update: [set]  If enabled registers not updated
  *                            until MSB and LSB have been read.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of bdu in reg CTRL3_C
  *
  */
int32_t lsm6dsl_block_data_update_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    reg.bdu = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  block_data_update: [get]  If enabled registers not updated
  * until MSB and LSB have been read.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of bdu in reg CTRL3_C
  *
  */
int32_t lsm6dsl_block_data_update_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    *val = reg.bdu;

    return mm_error;
}

/**
  * @brief  gyro_lpf1: [set]  Enable gyroscope digital LPF1. The bandwidth
  *                    can be selected through FTYPE
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of lpf1_sel_g in reg CTRL4_C
  *
  */
int32_t lsm6dsl_gyro_lpf1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    reg.lpf1_sel_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_lpf1: [get]  Enable gyroscope digital LPF1.
  *                    The bandwidth can be selected through FTYPE
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of lpf1_sel_g in reg CTRL4_C
  *
  */
int32_t lsm6dsl_gyro_lpf1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    *val = reg.lpf1_sel_g;

    return mm_error;
}

/**
  * @brief  device_id: [get] device_id value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t : union that group together the device_id data
  *
  */
int32_t lsm6dsl_device_id(lsm6dsl_ctx_t *ctx, uint8_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_WHO_AM_I, (uint8_t *) buff,
                            sizeof(uint8_t));
}

/**
  * @brief  temperature: [get] temperature value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  axis1byt16_t : union that group together the temperature data
  *
  */
int32_t lsm6dsl_temperature_raw(lsm6dsl_ctx_t *ctx, axis1bit16_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_OUT_TEMP_L, (uint8_t *) buff,
                            sizeof(axis1bit16_t));
}

float lsm6dsl_read_temperature(lsm6dsl_ctx_t *ctx)
{
    axis1bit16_t buff;
    lsm6dsl_read_reg(ctx, LSM6DSL_OUT_TEMP_L, (uint8_t *) &buff, sizeof(axis1bit16_t));
    return (float)(buff.i16bit) / 256.0f + 25;

}

/**
  * @brief  angular_rate: [get] angular_rate value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  axis3bit16_t : union that group together the angular_rate data
  *
  */
int32_t lsm6dsl_angular_rate_raw(lsm6dsl_ctx_t *ctx, axis3bit16_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_OUTX_L_G, (uint8_t *) buff,
                            sizeof(axis3bit16_t));
}

/**
  * @brief  acceleration: [get] acceleration value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  axis3bit16_t : union that group together the acceleration data
  *
  */
int32_t lsm6dsl_acceleration_raw(lsm6dsl_ctx_t *ctx, axis3bit16_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_OUTX_L_XL, (uint8_t *) buff,
                            sizeof(axis3bit16_t));
}

void lsm6dsl_read_raw_acc_xyz(lsm6dsl_ctx_t *ctx, void *acc_data)
{
    unsigned char	buf_reg[8];
    imu_axis_data_t *raw_acc_xyz = (imu_axis_data_t *)acc_data;
    lsm6dsl_read_reg(ctx, LSM6DSL_OUTX_L_XL, buf_reg, 6);
    raw_acc_xyz->x = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
    raw_acc_xyz->y = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_acc_xyz->z = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

    /* log_info("Qmi8658 acc: %d %d %d\n", raw_acc_xyz->x, raw_acc_xyz->y, raw_acc_xyz->z); */
}
void lsm6dsl_read_raw_gyro_xyz(lsm6dsl_ctx_t *ctx, void *gyro_data)
{
    unsigned char	buf_reg[6];
    imu_axis_data_t *raw_gyro_xyz = (imu_axis_data_t *)gyro_data;
    lsm6dsl_read_reg(ctx, LSM6DSL_OUTX_L_G, buf_reg, 6);
    raw_gyro_xyz->x = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
    raw_gyro_xyz->y = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_gyro_xyz->z = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

    /* log_info("Qmi8658 gyro: %d %d %d\n", raw_gyro_xyz->x, raw_gyro_xyz->y, raw_gyro_xyz->z); */
}
void lsm6dsl_read_raw_acc_gyro_xyz(lsm6dsl_ctx_t *ctx, void *raw_data)
{
    unsigned char	buf_reg[14];
    short temp = 0;
    imu_sensor_data_t *raw_sensor_data = (imu_sensor_data_t *)raw_data;
    /* lsm6dsl_read_reg(ctx, LSM6DSL_OUTX_L_G, (uint8_t*) buf_reg,12); */
    lsm6dsl_read_reg(ctx, LSM6DSL_OUT_TEMP_L, (uint8_t *) buf_reg, 14);
    temp = ((short)buf_reg[1] << 8) | buf_reg[0];
    raw_sensor_data->temp_data = (float)temp / 256.0f + 25;
    raw_sensor_data->gyro.x = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
    raw_sensor_data->gyro.y = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));
    raw_sensor_data->gyro.z = (short)((unsigned short)(buf_reg[7] << 8) | (buf_reg[6]));

    raw_sensor_data->acc.x = (short)((unsigned short)(buf_reg[9] << 8) | (buf_reg[8]));
    raw_sensor_data->acc.y = (short)((unsigned short)(buf_reg[11] << 8) | (buf_reg[10]));
    raw_sensor_data->acc.z = (short)((unsigned short)(buf_reg[13] << 8) | (buf_reg[12]));
    /* log_info("lsm6dsl raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_data->acc.x, raw_sensor_data->acc.y, raw_sensor_data->acc.z, raw_sensor_data->gyro.x, raw_sensor_data->gyro.y, raw_sensor_data->gyro.z); */
    /* log_info("lsm6dsl temp:%d.%d\n", (u16)(raw_sensor_data->temp_data), (u16)(((u16)((raw_sensor_data->temp_data)* 100)) % 100)); */
}

/**
  * @brief  lsm6dsl_status: [get] read all the device status and
  *                         source registers
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_multi_status_t: union of refisters
  *                                 from WAKE_UP_SRC to WRIST_TILT_IA
  *
  */
int32_t lsm6dsl_status_get(lsm6dsl_ctx_t *ctx, lsm6dsl_multi_status_t *val)
{
    int32_t mm_error;

    /* WAKE_UP_SRC + TAP_SRC + D6D_SRC + STATUS_REG */
    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_SRC, (uint8_t *)val, 4);
    val += 4;
    /* FUNC_SRC1 + FUNC_SRC2 + WRIST_TILT_IA */
    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FUNC_SRC1, (uint8_t *)val, 3);

    return mm_error;
}

/**
  * @}
  */

/**
 * @addtogroup Interrupt_functions
 * @brief     This section provide a set of functions used
 *            to manage the interrupts of the devices.
 *  @{
 */

/**
  * @brief  wrist_tilt_on_int2: [set]  Wrist tilt interrupt on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_wrist_tilt
  *                      in reg DRDY_PULSE_CFG_G
  *
  */
int32_t lsm6dsl_wrist_tilt_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_drdy_pulse_cfg_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_DRDY_PULSE_CFG_G,
                                (uint8_t *)&reg, 1);
    reg.int2_wrist_tilt = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_DRDY_PULSE_CFG_G,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  wrist_tilt_on_int2: [get]  Wrist tilt interrupt on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_wrist_tilt
  *                  in reg DRDY_PULSE_CFG_G
  *
  */
int32_t lsm6dsl_wrist_tilt_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_drdy_pulse_cfg_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_DRDY_PULSE_CFG_G,
                                (uint8_t *)&reg, 1);
    *val = reg.int2_wrist_tilt;

    return mm_error;
}

/**
  * @brief  xl_drdy_on_int1: [set]  Accelerometer Data Ready on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int1_drdy_xl in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_xl_drdy_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    reg.int1_drdy_xl = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_drdy_on_int1: [get]  Accelerometer Data Ready on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int1_drdy_xl in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_xl_drdy_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int1_drdy_xl;

    return mm_error;
}

/**
  * @brief  gyro_drdy_on_int1: [set]  Giroscope Data Ready on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int1_drdy_g in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_gyro_drdy_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    reg.int1_drdy_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_drdy_on_int1: [get]  Giroscope Data Ready on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int1_drdy_g in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_gyro_drdy_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int1_drdy_g;

    return mm_error;
}

/**
  * @brief  boot_on_int1: [set]  Boot signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int1_boot in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_boot_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    reg.int1_boot = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  boot_on_int1: [get]  Boot signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int1_boot in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_boot_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int1_boot;

    return mm_error;
}

/**
  * @brief  fifo_threshold_on_int1: [set]  FIFO threshold signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int1_fth in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_fifo_threshold_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    reg.int1_fth = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_threshold_on_int1: [get]  FIFO threshold signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int1_fth in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_fifo_threshold_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int1_fth;

    return mm_error;
}

/**
  * @brief  fifo_overrun_on_int1: [set]  FIFO overrun signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int1_fifo_ovr in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_fifo_overrun_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    reg.int1_fifo_ovr = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_overrun_on_int1: [get]  FIFO overrun signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int1_fifo_ovr in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_fifo_overrun_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int1_fifo_ovr;

    return mm_error;
}

/**
  * @brief  fifo_full_on_int1: [set]  FIFO full signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int1_full_flag in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_fifo_full_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    reg.int1_full_flag = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_full_on_int1: [get]  FIFO full signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int1_full_flag in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_fifo_full_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int1_full_flag;

    return mm_error;
}

/**
  * @brief   sign_motion_on_int1: [set]  Significant motion interrupt
  *                               enable on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int1_sign_mot in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_sign_motion_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    reg.int1_sign_mot = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   sign_motion_on_int1: [get]  Significant motion interrupt
  *                               enable on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int1_sign_mot in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_sign_motion_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int1_sign_mot;

    return mm_error;
}

/**
  * @brief   step_detect_on_int1: [set]  Pedometer step recognition
  *                               interrupt enable on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int1_step_detector
  *                      in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_step_detect_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    reg.int1_step_detector = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   step_detect_on_int1: [get]  Pedometer step recognition interrupt
  *                               enable on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int1_step_detector
  *                  in reg INT1_CTRL
  *
  */
int32_t lsm6dsl_step_detect_on_int1_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int1_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int1_step_detector;

    return mm_error;
}

/**
  * @brief  xl_drdy_on_int2: [set]  Accelerometer Data Ready on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_drdy_xl in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_xl_drdy_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    reg.int2_drdy_xl = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_drdy_on_int2: [get]  Accelerometer Data Ready on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_drdy_xl in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_xl_drdy_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int2_drdy_xl;

    return mm_error;
}

/**
  * @brief  gyro_drdy_on_int2: [set]  Giroscope Data Ready on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_drdy_g in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_gyro_drdy_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    reg.int2_drdy_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_drdy_on_int2: [get]  Giroscope Data Ready on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_drdy_g in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_gyro_drdy_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int2_drdy_g;

    return mm_error;
}

/**
  * @brief   temperature_drdy_on_int2: [set]  Temperature Data Ready
  *                                    on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_drdy_temp in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_temperature_drdy_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    reg.int2_drdy_temp = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   temperature_drdy_on_int2: [get]  Temperature Data Ready
  *                                    on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_drdy_temp in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_temperature_drdy_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int2_drdy_temp;

    return mm_error;
}

/**
  * @brief  fifo_threshold_on_int2: [set]  FIFO threshold signal on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_fth in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_fifo_threshold_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    reg.int2_fth = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_threshold_on_int2: [get]  FIFO threshold signal on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_fth in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_fifo_threshold_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int2_fth;

    return mm_error;
}

/**
  * @brief  fifo_overrun_on_int2: [set]  FIFO overrun signal on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_fifo_ovr in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_fifo_overrun_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    reg.int2_fifo_ovr = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_overrun_on_int2: [get]  FIFO overrun signal on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_fifo_ovr in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_fifo_overrun_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int2_fifo_ovr;

    return mm_error;
}

/**
  * @brief  fifo_full_on_int2: [set]  FIFO full signal on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_full_flag in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_fifo_full_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    reg.int2_full_flag = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_full_on_int2: [get]  FIFO full signal on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_full_flag in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_fifo_full_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int2_full_flag;

    return mm_error;
}

/**
  * @brief   step_overflow_on_int2: [set]  Step counter overflow
  *                                 interrupt enable on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_step_count_ov
  *                      in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_step_overflow_on_int2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    reg.int2_step_count_ov = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   step_overflow_on_int2: [get]  Step counter overflow
  *                                 interrupt enable on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_step_count_ov in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_step_overflow_on_int2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int2_step_count_ov;

    return mm_error;
}

/**
  * @brief   step_on_time_interval_on_int2: [set]  Pedometer step
  *                                         recognition interrupt on
  *                                         delta time enable on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_step_delta in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_step_on_time_interval_on_int2_set(lsm6dsl_ctx_t *ctx,
        uint8_t val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL,
                                (uint8_t *)&reg, 1);
    reg.int2_step_delta = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT2_CTRL,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   step_on_time_interval_on_int2: [get]  Pedometer step recognition
  *                                         interrupt on delta time enable
  *                                         on INT2 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_step_delta in reg INT2_CTRL
  *
  */
int32_t lsm6dsl_step_on_time_interval_on_int2_get(lsm6dsl_ctx_t *ctx,
        uint8_t *val)
{
    lsm6dsl_int2_ctrl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT2_CTRL, (uint8_t *)&reg, 1);
    *val = reg.int2_step_delta;

    return mm_error;
}

/**
  * @}
  */

/** @addtogroup FIFO_functions
  * @brief     This section provide a set of functions used
  *            to manage the FIFO of the devices.
  * @{
  */

/**
  * @brief  fifo_temperature: [set]  Enable the temperature data
  *                           storage in FIFO.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of fifo_temp_en in reg FIFO_CTRL2
  *
  */
int32_t lsm6dsl_fifo_temperature_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_fifo_ctrl2_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);
    reg.fifo_temp_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_temperature: [get]  Enable the temperature
  *                           data storage in FIFO.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of fifo_temp_en in reg FIFO_CTRL2
  *
  */
int32_t lsm6dsl_fifo_temperature_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_fifo_ctrl2_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);
    *val = reg.fifo_temp_en;

    return mm_error;
}

/**
  * @brief  fifo_write_mode: [set]  Enable write in FIFO on XL/Gyro
  *                          data-ready or at every step detected by
  *                          step counter
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_timer_pedo_fifo_drdy_t: change the values of
  *                                         timer_pedo_fifo_drdy in
  *                                         reg FIFO_CTRL2
  *
  */
int32_t lsm6dsl_fifo_write_mode_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_timer_pedo_fifo_drdy_t val)
{
    lsm6dsl_fifo_ctrl2_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);
    reg.timer_pedo_fifo_drdy = val;
    reg.not_used_01 = 0;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_write_mode: [get]  Enable write in FIFO on XL/Gyro
  *                          data-ready or at every step detected by step counter
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_timer_pedo_fifo_drdy_t: Get the values of
  *                                         timer_pedo_fifo_drdy in
  *                                         reg FIFO_CTRL2
  *
  */
int32_t lsm6dsl_fifo_write_mode_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_timer_pedo_fifo_drdy_t *val)
{
    lsm6dsl_fifo_ctrl2_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_timer_pedo_fifo_drdy_t) reg.timer_pedo_fifo_drdy;

    return mm_error;
}

/**
  * @brief   fifo_step_timestamp: [set]  Enable pedometer step
  *                               counter and timestamp as 4th FIFO data set.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of timer_pedo_fifo_en
  *                      in reg FIFO_CTRL2
  *
  */
int32_t lsm6dsl_fifo_step_timestamp_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_fifo_ctrl2_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);
    reg.timer_pedo_fifo_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   fifo_step_timestamp: [get]  Enable pedometer step counter
  *                               and timestamp as 4th FIFO data set.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of timer_pedo_fifo_en in reg FIFO_CTRL2
  *
  */
int32_t lsm6dsl_fifo_step_timestamp_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_fifo_ctrl2_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL2, (uint8_t *)&reg, 1);
    *val = reg.timer_pedo_fifo_en;

    return mm_error;
}

/**
  * @brief  xl_decimation: [set]  Accelerometer FIFO (second data set)
  *                        decimation setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_dec_fifo_xl_t: change the values of
  *                                dec_fifo_xl in reg FIFO_CTRL3
  *
  */
int32_t lsm6dsl_xl_decimation_set(lsm6dsl_ctx_t *ctx,
                                  lsm6dsl_dec_fifo_xl_t val)
{
    lsm6dsl_fifo_ctrl3_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL3, (uint8_t *)&reg, 1);
    reg.dec_fifo_xl = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL3, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_decimation: [get]  Accelerometer FIFO (second data set)
  *                        decimation setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_dec_fifo_xl_t: Get the values of dec_fifo_xl
  *                                in reg FIFO_CTRL3
  *
  */
int32_t lsm6dsl_xl_decimation_get(lsm6dsl_ctx_t *ctx,
                                  lsm6dsl_dec_fifo_xl_t *val)
{
    lsm6dsl_fifo_ctrl3_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL3, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_dec_fifo_xl_t) reg.dec_fifo_xl;

    return mm_error;
}

/**
  * @brief  gyro_decimation: [set]  Gyro FIFO (first data set)
  *                          decimation setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_dec_fifo_gyro_t: change the values of
  *                                  dec_fifo_gyro in reg FIFO_CTRL3
  *
  */
int32_t lsm6dsl_gyro_decimation_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_dec_fifo_gyro_t val)
{
    lsm6dsl_fifo_ctrl3_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL3, (uint8_t *)&reg, 1);
    reg.dec_fifo_gyro = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL3, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_decimation: [get]  Gyro FIFO (first data set)
  *                          decimation setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_dec_fifo_gyro_t: Get the values of dec_fifo_gyro
  *                                  in reg FIFO_CTRL3
  *
  */
int32_t lsm6dsl_gyro_decimation_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_dec_fifo_gyro_t *val)
{
    lsm6dsl_fifo_ctrl3_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL3, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_dec_fifo_gyro_t) reg.dec_fifo_gyro;

    return mm_error;
}

/**
  * @brief   dataset3_decimation: [set]  Third FIFO data set
  *                               decimation setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_dec_ds3_fifo_t: change the values of dec_ds3_fifo
  *                                 in reg FIFO_CTRL4
  *
  */
int32_t lsm6dsl_dataset3_decimation_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_dec_ds3_fifo_t val)
{
    lsm6dsl_fifo_ctrl4_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);
    reg.dec_ds3_fifo = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   dataset3_decimation: [get]  Third FIFO data set
  *                               decimation setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_dec_ds3_fifo_t: Get the values of dec_ds3_fifo
  *                                 in reg FIFO_CTRL4
  *
  */
int32_t lsm6dsl_dataset3_decimation_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_dec_ds3_fifo_t *val)
{
    lsm6dsl_fifo_ctrl4_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_dec_ds3_fifo_t) reg.dec_ds3_fifo;

    return mm_error;
}

/**
  * @brief   dataset4_decimation: [set]  Fourth FIFO data set
  *                               decimation setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_dec_ds4_fifo_t: change the values of dec_ds4_fifo
  *                                 in reg FIFO_CTRL4
  *
  */
int32_t lsm6dsl_dataset4_decimation_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_dec_ds4_fifo_t val)
{
    lsm6dsl_fifo_ctrl4_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);
    reg.dec_ds4_fifo = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   dataset4_decimation: [get]  Fourth FIFO data set decimation
  *                               setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_dec_ds4_fifo_t: Get the values of dec_ds4_fifo
  *                                 in reg FIFO_CTRL4
  *
  */
int32_t lsm6dsl_dataset4_decimation_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_dec_ds4_fifo_t *val)
{
    lsm6dsl_fifo_ctrl4_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_dec_ds4_fifo_t) reg.dec_ds4_fifo;

    return mm_error;
}

/**
  * @brief   fifo_xl_gy_8bit_data: [set]  MSByte only memorization in FIFO
  *                                for XL and Gyro in FIFO.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of only_high_data in reg FIFO_CTRL4
  *
  */
int32_t lsm6dsl_fifo_xl_gy_8bit_data_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_fifo_ctrl4_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);
    reg.only_high_data = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   fifo_xl_gy_8bit_data: [get]  MSByte only memorization
  *                                in FIFO for XL and Gyro in FIFO.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of only_high_data in reg FIFO_CTRL4
  *
  */
int32_t lsm6dsl_fifo_xl_gy_8bit_data_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_fifo_ctrl4_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);
    *val = reg.only_high_data;

    return mm_error;
}

/**
  * @brief  fifo_threshold: [set]  Enable FIFO threshold level use.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of stop_on_fth in reg FIFO_CTRL4
  *
  */
int32_t lsm6dsl_fifo_stop_on_thr_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_fifo_ctrl4_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);
    reg.stop_on_fth = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_threshold: [get]  Enable FIFO threshold level use.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of stop_on_fth in reg FIFO_CTRL4
  *
  */
int32_t lsm6dsl_fifo_stop_on_thr_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_fifo_ctrl4_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL4, (uint8_t *)&reg, 1);
    *val = reg.stop_on_fth;

    return mm_error;
}

/**
  * @brief  fifo_mode: [set]  Define FIFO working mode.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_fifo_mode_t: change the values of fifo_mode
  *                              in reg FIFO_CTRL5
  *
  */
int32_t lsm6dsl_fifo_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_fifo_mode_t val)
{
    lsm6dsl_fifo_ctrl5_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL5, (uint8_t *)&reg, 1);
    reg.fifo_mode = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL5, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_mode: [get]  Define FIFO working mode.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_fifo_mode_t: Get the values of fifo_mode in reg FIFO_CTRL5
  *
  */
int32_t lsm6dsl_fifo_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_fifo_mode_t *val)
{
    lsm6dsl_fifo_ctrl5_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL5, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_fifo_mode_t) reg.fifo_mode;

    return mm_error;
}

/**
  * @brief  fifo_data_rate: [set]  Define FIFO working data rate.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_odr_fifo_t: change the values of odr_fifo
  *                             in reg FIFO_CTRL5
  *
  */
int32_t lsm6dsl_fifo_data_rate_set(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_fifo_t val)
{
    lsm6dsl_fifo_ctrl5_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL5, (uint8_t *)&reg, 1);
    reg.odr_fifo = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL5, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_data_rate: [get]  Define FIFO working data rate.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_odr_fifo_t: Get the values of odr_fifo in reg FIFO_CTRL5
  *
  */
int32_t lsm6dsl_fifo_data_rate_get(lsm6dsl_ctx_t *ctx, lsm6dsl_odr_fifo_t *val)
{
    lsm6dsl_fifo_ctrl5_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL5, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_odr_fifo_t) reg.odr_fifo;

    return mm_error;
}

/**
  * @brief  fifo_data_valid: [set]  Selection of FIFO data-valid signal.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_data_valid_sel_fifo_t: change the values of
  *                                        data_valid_sel_fifo in
  *                                        reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_fifo_data_valid_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_data_valid_sel_fifo_t val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG, (uint8_t *)&reg, 1);
    reg.data_valid_sel_fifo = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MASTER_CONFIG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  fifo_data_valid: [get]  Selection of FIFO data-valid signal.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_data_valid_sel_fifo_t: Get the values of
  *                                        data_valid_sel_fifo in
  *                                        reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_fifo_data_valid_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_data_valid_sel_fifo_t *val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_data_valid_sel_fifo_t) reg.data_valid_sel_fifo;

    return mm_error;
}

/**
  * @brief  fifo_empty: [get] FIFOemptybit.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of fifo_empty in reg FIFO_STATUS2
  *
  */
int32_t lsm6dsl_fifo_empty_get(lsm6dsl_ctx_t *ctx)
{
    lsm6dsl_fifo_status2_t reg;

    lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_STATUS2, (uint8_t *)&reg, 1);

    return reg.fifo_empty;
}

/**
  * @brief   fifo_full_next_cycle: [get]  FSmart FIFO full status.
  *                                FIFO will be full at the next ODR.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of fifo_full_smart in reg FIFO_STATUS2
  *
  */
int32_t lsm6dsl_fifo_full_next_cycle_get(lsm6dsl_ctx_t *ctx)
{
    lsm6dsl_fifo_status2_t reg;

    lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_STATUS2, (uint8_t *)&reg, 1);

    return reg.fifo_full_smart;
}

/**
  * @brief  fifo_over_run: [get]  FIFO overrun status.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of over_run in reg FIFO_STATUS2
  *
  */
int32_t lsm6dsl_fifo_over_run_get(lsm6dsl_ctx_t *ctx)
{
    lsm6dsl_fifo_status2_t reg;

    lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_STATUS2, (uint8_t *)&reg, 1);

    return reg.over_run;
}

/**
  * @brief   fifo_watermark_level: [get]  FIFO watermark status. FIFO
  *                                filling is equal to or higher than
  *                                the watermark level.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of waterm in reg FIFO_STATUS2
  *
  */
int32_t lsm6dsl_fifo_watermark_level_get(lsm6dsl_ctx_t *ctx)
{
    lsm6dsl_fifo_status2_t reg;

    lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_STATUS2, (uint8_t *)&reg, 1);

    return reg.waterm;
}

/**
  * @brief  fifo_threshold: [set]  Watermark flag rises when the number
  *                         of bytes written to FIFO after the next write
  *                         is greater than or equal to the threshold level.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint16_t: change the values of fth
  *
  */
int32_t lsm6dsl_fifo_threshold_set(lsm6dsl_ctx_t *ctx, uint16_t val)
{
    int32_t mm_error;
    uint16_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL1, (uint8_t *) &reg,
                                sizeof(uint16_t));
    reg &= 0xF800;
    reg |= (val & 0x07FF);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FIFO_CTRL1, (uint8_t *) &reg,
                                 sizeof(uint16_t));

    return mm_error;
}

/**
  * @brief  fifo_threshold: [get]  Watermark flag rises when the number
  *                         of bytes written to FIFO after the next write
  *                         is greater than or equal to the threshold level.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint16_t: get the values of fth
  *
  */
int32_t lsm6dsl_fifo_threshold_get(lsm6dsl_ctx_t *ctx, uint16_t *val)
{
    int32_t mm_error;
    uint16_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_CTRL1, (uint8_t *) &reg,
                                sizeof(uint16_t));
    reg &= 0x07FF;

    return mm_error;
}

/**
  * @brief  fifo_unread_words: [get]  Number of unread words (16-bit axes)
  *                            stored in FIFO.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint16_t: get the values of diff_fifo
  *
  */
int32_t lsm6dsl_fifo_unread_words_get(lsm6dsl_ctx_t *ctx, uint16_t *val)
{
    int32_t mm_error;
    uint16_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_STATUS1, (uint8_t *) &reg,
                                sizeof(uint16_t));
    reg &= 0x07FF;

    return mm_error;
}

/**
  * @brief   fifo_recursive_pattern: [get]  Word of recursive pattern
  *                                  read at the next reading.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint16_t: Word of recursive pattern read at the next reading.
  *
  */
int32_t lsm6dsl_fifo_recursive_pattern_get(lsm6dsl_ctx_t *ctx, uint16_t *val)
{
    int32_t mm_error;
    uint16_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_STATUS3, (uint8_t *) &reg,
                                sizeof(uint16_t));
    reg &= 0x03FF;

    return mm_error;
}

/**
  * @brief  fifo_output: [get] fifo_output value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  axis1bit16_t : union that group together the fifo_output data
  *
  */
int32_t lsm6dsl_fifo_output_raw(lsm6dsl_ctx_t *ctx, axis1bit16_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_DATA_OUT_L, (uint8_t *) buff,
                            sizeof(axis1bit16_t));
}

lsm6dsl_fifo_mode_t fifo_mode_old = STOP_WHEN_FULL;
void lsm6dsl_fifo_mode_start()
{
    lsm6dsl_fifo_mode_set(&lsm6dsl_ctx, BYPASS);
    lsm6dsl_fifo_mode_set(&lsm6dsl_ctx, STOP_WHEN_FULL);
}
//data:gyro(6bytes)+acc(6bytes)+step(6bytes)+temperature(6bytes头尾补0)
int32_t lsm6dsl_read_fifo(lsm6dsl_ctx_t *ctx, u8 *buff)
{
    uint16_t reg = 0;
    uint16_t ret = 0;
    lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_STATUS1, (uint8_t *)&reg, 2);
    /* log_info("fifo status[15~12]:0x%x!",reg); */
    if (reg & 0x1000) {
        /* log_info("fifo empty!"); */
        putchar('.');
        /* } else if (reg & 0xe000) { */
    } else {
        /* } else if (reg & 0x8000) { */
        reg &= 0x07FF;
        /* log_info("fifo level:%d!",reg); */
        if (reg > 0) {
#if 0
            for (i = 1; i < reg; i++) {
            }
#else
            ret = lsm6dsl_read_reg(ctx, LSM6DSL_FIFO_DATA_OUT_L, (uint8_t *) buff, reg * 2);
            if (fifo_mode_old == STOP_WHEN_FULL) {
                lsm6dsl_fifo_mode_start();
            }
            return ret;
#endif
        }

    }
    return 0;
}

/**
  * @}
  */

/**
 * @addtogroup advanced_function_functions
 * @brief     This section provide a set of functions used
 *            to manage the advanced feature of the device.
 * @{
 */

/**
  * @brief   xl_analog_bandwidth: [set]  Accelerometer analog chain bandwidth
  *                                selection (only for accelerometer
  *                                ODR ≥ 1.67 kHz).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_bw0_xl_t: change the values of bw0_xl in reg CTRL1_XL
  *
  */
int32_t lsm6dsl_xl_analog_bandwidth_set(lsm6dsl_ctx_t *ctx, lsm6dsl_bw0_xl_t val)
{
    lsm6dsl_ctrl1_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);
    reg.bw0_xl = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   xl_analog_bandwidth: [get]  Accelerometer analog chain
  *                               bandwidth selection (only for accelerometer
  *                               ODR ≥ 1.67 kHz).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_bw0_xl_t: Get the values of bw0_xl in reg CTRL1_XL
  *
  */
int32_t lsm6dsl_xl_analog_bandwidth_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_bw0_xl_t *val)
{
    lsm6dsl_ctrl1_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_bw0_xl_t) reg.bw0_xl;

    return mm_error;
}

/**
  * @brief  xl_lpf1_bandwidth: [set]  Accelerometer digital LPF (LPF1)
  *                            bandwidth selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_lpf1_bw_sel_t: change the values of lpf1_bw_sel
  *                                in reg CTRL1_XL
  *
  */
int32_t lsm6dsl_xl_lpf1_bandwidth_set(lsm6dsl_ctx_t *ctx,
                                      lsm6dsl_lpf1_bw_sel_t val)
{
    lsm6dsl_ctrl1_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);
    reg.lpf1_bw_sel = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_lpf1_bandwidth: [get]  Accelerometer digital LPF (LPF1)
  *                            bandwidth selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_lpf1_bw_sel_t: Get the values of lpf1_bw_sel
  *                                in reg CTRL1_XL
  *
  */
int32_t lsm6dsl_xl_lpf1_bandwidth_get(lsm6dsl_ctx_t *ctx,
                                      lsm6dsl_lpf1_bw_sel_t *val)
{
    lsm6dsl_ctrl1_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_lpf1_bw_sel_t) reg.lpf1_bw_sel;

    return mm_error;
}

/**
  * @brief  data_format: [set]  Big/Little Endian Data selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_ble_t: change the values of ble in reg CTRL3_C
  *
  */
int32_t lsm6dsl_data_format_set(lsm6dsl_ctx_t *ctx, lsm6dsl_ble_t val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    reg.ble = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  data_format: [get]  Big/Little Endian Data selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_ble_t: Get the values of ble in reg CTRL3_C
  *
  */
int32_t lsm6dsl_data_format_get(lsm6dsl_ctx_t *ctx, lsm6dsl_ble_t *val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_ble_t) reg.ble;

    return mm_error;
}

/**  default: 1:auto_increment
  * @brief  auto_increment: [set]  Register address automatically
  *                         incremented during a multiple byte access with
  *                         a serial interface (I2C or SPI).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of if_inc in reg CTRL3_C
  *
  */
int32_t lsm6dsl_auto_increment_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    reg.if_inc = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  auto_increment: [get]  Register address automatically
  *                         incremented during a multiple byte access with
  *                         a serial interface (I2C or SPI).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of if_inc in reg CTRL3_C
  *
  */
int32_t lsm6dsl_auto_increment_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    *val = reg.if_inc;

    return mm_error;
}

/** default: 0:4wire
  * @brief  spi_mode: [set]  SPI Serial Interface Mode selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_sim_t: change the values of sim in reg CTRL3_C
  *
  */
int32_t lsm6dsl_spi_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_sim_t val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    reg.sim = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  spi_mode: [get]  SPI Serial Interface Mode selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_sim_t: Get the values of sim in reg CTRL3_C
  *
  */
int32_t lsm6dsl_spi_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_sim_t *val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_sim_t) reg.sim;

    return mm_error;
}

/**
  * @brief  pads_mode: [set]  Push-pull/open-drain selection on
  *                    INT1 and INT2 pads.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_pp_od_t: change the values of pp_od in reg CTRL3_C
  *
  */
int32_t lsm6dsl_pads_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_pp_od_t val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    reg.pp_od = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  pads_mode: [get]  Push-pull/open-drain selection
  *                    on INT1 and INT2 pads.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_pp_od_t: Get the values of pp_od in reg CTRL3_C
  *
  */
int32_t lsm6dsl_pads_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_pp_od_t *val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_pp_od_t) reg.pp_od;

    return mm_error;
}

/**
  * @brief  pads_polatity: [set]  Interrupt activation level.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_h_lactive_t: change the values of h_lactive
  *                              in reg CTRL3_C
  *
  */
int32_t lsm6dsl_pads_polatity_set(lsm6dsl_ctx_t *ctx, lsm6dsl_h_lactive_t val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    reg.h_lactive = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  pads_polatity: [get]  Interrupt activation level.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_h_lactive_t: Get the values of h_lactive in reg CTRL3_C
  *
  */
int32_t lsm6dsl_pads_polatity_get(lsm6dsl_ctx_t *ctx, lsm6dsl_h_lactive_t *val)
{
    lsm6dsl_ctrl3_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL3_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_h_lactive_t) reg.h_lactive;

    return mm_error;
}

/**
  * @brief  i2c_interface: [set]  Enable / Disable I2C interface.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of i2c_disable in reg CTRL4_C
  *
  */
int32_t lsm6dsl_i2c_interface_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    reg.i2c_disable = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  i2c_interface: [get]  Enable / Disable I2C interface.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of i2c_disable in reg CTRL4_C
  *
  */
int32_t lsm6dsl_i2c_interface_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    *val = reg.i2c_disable;

    return mm_error;
}

/**
  * @brief   mask_drdy_on_settling_data: [set]  Acts only on the
  *                                      accelerometer LPF1 and the gyroscope
  *                                      LPF2 digital filters settling time.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of drdy_mask in reg CTRL4_C
  *
  */
int32_t lsm6dsl_mask_drdy_on_settling_data_set(lsm6dsl_ctx_t *ctx,
        uint8_t val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    reg.drdy_mask = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   mask_drdy_on_settling_data: [get]  Acts only on the accelerometer
  *                                      LPF1 and the gyroscope LPF2 digital
  *                                      filters settling time.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of drdy_mask in reg CTRL4_C
  *
  */
int32_t lsm6dsl_mask_drdy_on_settling_data_get(lsm6dsl_ctx_t *ctx,
        uint8_t *val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    *val = reg.drdy_mask;

    return mm_error;
}

/**
  * @brief  all_on_int1_pad: [set]  All interrupt signals available
  *                          on INT1 pad enable.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of int2_on_int1 in reg CTRL4_C
  *
  */
int32_t lsm6dsl_all_on_int1_pad_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    reg.int2_on_int1 = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  all_on_int1_pad: [get]  All interrupt signals available
  *                          on INT1 pad enable.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of int2_on_int1 in reg CTRL4_C
  *
  */
int32_t lsm6dsl_all_on_int1_pad_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    *val = reg.int2_on_int1;

    return mm_error;
}

/**
  * @brief  gyro_sleep_mode: [set]  Gyroscope sleep mode enable.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of sleep in reg CTRL4_C
  *
  */
int32_t lsm6dsl_gyro_sleep_mode_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    reg.sleep = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_sleep_mode: [get]  Gyroscope sleep mode enable.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of sleep in reg CTRL4_C
  *
  */
int32_t lsm6dsl_gyro_sleep_mode_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    *val = reg.sleep;

    return mm_error;
}

/**
  * @brief  den_func_on_xl: [set]  Extend DEN functionality to
  *                         accelerometer sensor.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of den_xl_en in reg CTRL4_C
  *
  */
int32_t lsm6dsl_den_func_on_xl_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    reg.den_xl_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  den_func_on_xl: [get]  Extend DEN functionality
  *                         to accelerometer sensor.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of den_xl_en in reg CTRL4_C
  *
  */
int32_t lsm6dsl_den_func_on_xl_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl4_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL4_C, (uint8_t *)&reg, 1);
    *val = reg.den_xl_en;

    return mm_error;
}

/**
  * @brief  xl_self_test: [set]  Linear acceleration sensor self-test.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_st_xl_t: change the values of st_xl in reg CTRL5_C
  *
  */
int32_t lsm6dsl_xl_self_test_set(lsm6dsl_ctx_t *ctx, lsm6dsl_st_xl_t val)
{
    lsm6dsl_ctrl5_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);
    reg.st_xl = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_self_test: [get]  Linear acceleration sensor self-test.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_st_xl_t: Get the values of st_xl in reg CTRL5_C
  *
  */
int32_t lsm6dsl_xl_self_test_get(lsm6dsl_ctx_t *ctx, lsm6dsl_st_xl_t *val)
{
    lsm6dsl_ctrl5_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_st_xl_t) reg.st_xl;

    return mm_error;
}

/**
  * @brief  gyro_self_test: [set]  Linear gyro sensor self-test.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_st_g_t: change the values of st_g in reg CTRL5_C
  *
  */
int32_t lsm6dsl_gyro_self_test_set(lsm6dsl_ctx_t *ctx, lsm6dsl_st_g_t val)
{
    lsm6dsl_ctrl5_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);
    reg.st_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_self_test: [get]  Linear gyro sensor self-test.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_st_g_t: Get the values of st_g in reg CTRL5_C
  *
  */
int32_t lsm6dsl_gyro_self_test_get(lsm6dsl_ctx_t *ctx, lsm6dsl_st_g_t *val)
{
    lsm6dsl_ctrl5_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_st_g_t) reg.st_g;

    return mm_error;
}

/**
  * @brief  den_func_polarity: [set]  DEN active level configuration.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_den_lh_t: change the values of den_lh in reg CTRL5_C
  *
  */
int32_t lsm6dsl_den_func_polarity_set(lsm6dsl_ctx_t *ctx,
                                      lsm6dsl_den_lh_t val)
{
    lsm6dsl_ctrl5_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);
    reg.den_lh = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  den_func_polarity: [get]  DEN active level configuration.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_den_lh_t: Get the values of den_lh in reg CTRL5_C
  *
  */
int32_t lsm6dsl_den_func_polarity_get(lsm6dsl_ctx_t *ctx,
                                      lsm6dsl_den_lh_t *val)
{
    lsm6dsl_ctrl5_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_den_lh_t) reg.den_lh;

    return mm_error;
}

/**
  * @brief  rounding_mode: [set]  Circular burst-mode (rounding) read
  *                        from the output registers.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_rounding_t: change the values of rounding in reg CTRL5_C
  *
  */
int32_t lsm6dsl_rounding_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_rounding_t val)
{
    lsm6dsl_ctrl5_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);
    reg.rounding = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  rounding_mode: [get]  Circular burst-mode (rounding) read
  *                        from the output registers.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_rounding_t: Get the values of rounding in reg CTRL5_C
  *
  */
int32_t lsm6dsl_rounding_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_rounding_t *val)
{
    lsm6dsl_ctrl5_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL5_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_rounding_t) reg.rounding;

    return mm_error;
}

/**
  * @brief   gyro_lpf1_bandwidth: [set]  Gyroscope's low-pass filter
  *                               (LPF1) bandwidth selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_ftype_t: change the values of ftype in reg CTRL6_C
  *
  */
int32_t lsm6dsl_gyro_lpf1_bandwidth_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_ftype_t val)
{
    lsm6dsl_ctrl6_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);
    reg.ftype = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   gyro_lpf1_bandwidth: [get]  Gyroscope's low-pass filter
  *                               (LPF1) bandwidth selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_ftype_t: Get the values of ftype in reg CTRL6_C
  *
  */
int32_t lsm6dsl_gyro_lpf1_bandwidth_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_ftype_t *val)
{
    lsm6dsl_ctrl6_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_ftype_t) reg.ftype;

    return mm_error;
}

/**
  * @brief  xl_offset_weight: [set]  Weight of XL user offset bits of
  *                           registers X_OFS_USR (73h), Y_OFS_USR (74h),
  *                           Z_OFS_USR (75h)
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_usr_off_w_t: change the values of usr_off_w in reg CTRL6_C
  *
  */
int32_t lsm6dsl_xl_offset_weight_set(lsm6dsl_ctx_t *ctx,
                                     lsm6dsl_usr_off_w_t val)
{
    lsm6dsl_ctrl6_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);
    reg.usr_off_w = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_offset_weight: [get]  Weight of XL user offset bits of
  *                           registers X_OFS_USR (73h), Y_OFS_USR (74h),
  *                           Z_OFS_USR (75h)
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_usr_off_w_t: Get the values of usr_off_w in reg CTRL6_C
  *
  */
int32_t lsm6dsl_xl_offset_weight_get(lsm6dsl_ctx_t *ctx,
                                     lsm6dsl_usr_off_w_t *val)
{
    lsm6dsl_ctrl6_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_usr_off_w_t) reg.usr_off_w;

    return mm_error;
}

/**
  * @brief   xl_high_performance: [set]  High-performance operating
  *                               mode disable for accelerometer.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_xl_hm_mode_t: change the values of xl_hm_mode
  *                               in reg CTRL6_C
  *
  */
int32_t lsm6dsl_xl_high_performance_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_xl_hm_mode_t val)
{
    lsm6dsl_ctrl6_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);
    reg.xl_hm_mode = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   xl_high_performance: [get]  High-performance operating mode
  *                               disable for accelerometer.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_xl_hm_mode_t: Get the values of xl_hm_mode in reg CTRL6_C
  *
  */
int32_t lsm6dsl_xl_high_performance_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_xl_hm_mode_t *val)
{
    lsm6dsl_ctrl6_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_xl_hm_mode_t) reg.xl_hm_mode;

    return mm_error;
}

/**
  * @brief  den_func_mode: [set]  DEN functionality trigger mode selection.
  *                        (TRIG_EN & LVL_EN & LVL2_EN)
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_den_mode_t: change the values of den_mode in reg CTRL6_C
  *
  */
int32_t lsm6dsl_den_func_mode_set(lsm6dsl_ctx_t *ctx, lsm6dsl_den_mode_t val)
{
    lsm6dsl_ctrl6_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);
    reg.den_mode = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  den_func_mode: [get] DEN functionality trigger mode selection.
  *                        (TRIG_EN & LVL_EN & LVL2_EN)
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_den_mode_t: Get the values of den_mode in reg CTRL6_C
  *
  */
int32_t lsm6dsl_den_func_mode_get(lsm6dsl_ctx_t *ctx, lsm6dsl_den_mode_t *val)
{
    lsm6dsl_ctrl6_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL6_C, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_den_mode_t) reg.den_mode;

    return mm_error;
}

/**
  * @brief  rounding_on_status: [set]  Source register rounding function
  *                             on WAKE_UP_SRC (1Bh), TAP_SRC (1Ch),
  *                             D6D_SRC (1Dh), STATUS_REG (1Eh)
  *                             and FUNC_SRC (53h).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_den_mode_t: change the values of rounding_status
  *                             in reg CTRL7_G
  *
  */
int32_t lsm6dsl_rounding_on_status_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl7_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);
    reg.rounding_status = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  rounding_on_status: [get]  Source register rounding function
  *                             on WAKE_UP_SRC (1Bh), TAP_SRC (1Ch),
  *                             D6D_SRC (1Dh), STATUS_REG (1Eh)
  *                             and FUNC_SRC (53h).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of rounding_status in reg CTRL7_G
  *
  */
int32_t lsm6dsl_rounding_on_status_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl7_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);
    *val = reg.rounding_status;

    return mm_error;
}

/**
  * @brief  gyro_hp_cutoff: [set]  Gyroscope digital HP filter
  *                         cutoff selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_hpm_g_t: change the values of hpm_g in reg CTRL7_G
  *
  */
int32_t lsm6dsl_gyro_hp_cutoff_set(lsm6dsl_ctx_t *ctx, lsm6dsl_hpm_g_t val)
{
    lsm6dsl_ctrl7_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);
    reg.hpm_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_hp_cutoff: [get]  Gyroscope digital HP filter
  *                         cutoff selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_hpm_g_t: Get the values of hpm_g in reg CTRL7_G
  *
  */
int32_t lsm6dsl_gyro_hp_cutoff_get(lsm6dsl_ctx_t *ctx, lsm6dsl_hpm_g_t *val)
{
    lsm6dsl_ctrl7_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_hpm_g_t) reg.hpm_g;

    return mm_error;
}

/**
  * @brief  gyro_hp_filter: [set]  Gyroscope digital high-pass filter enable.
  *                         The filter is enabled only if the gyro
  *                         is in HP mode.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of hp_en_g in reg CTRL7_G
  *
  */
int32_t lsm6dsl_gyro_hp_filter_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl7_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);
    reg.hp_en_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  gyro_hp_filter: [get]  Gyroscope digital high-pass filter
  *                         enable. The filter is enabled only if the
  *                         gyro is in HP mode.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of hp_en_g in reg CTRL7_G
  *
  */
int32_t lsm6dsl_gyro_hp_filter_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl7_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);
    *val = reg.hp_en_g;

    return mm_error;
}

/**
  * @brief   gyro_high_performance: [set]  High-performance operating
  *                                 mode disable for gyroscope.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_g_hm_mode_t: change the values of g_hm_mode
  *                              in reg CTRL7_G
  *
  */
int32_t lsm6dsl_gyro_high_performance_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_g_hm_mode_t val)
{
    lsm6dsl_ctrl7_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);
    reg.g_hm_mode = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   gyro_high_performance: [get]  High-performance operating
  *                                 mode disable for gyroscope.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_g_hm_mode_t: Get the values of g_hm_mode in reg CTRL7_G
  *
  */
int32_t lsm6dsl_gyro_high_performance_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_g_hm_mode_t *val)
{
    lsm6dsl_ctrl7_g_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL7_G, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_g_hm_mode_t) reg.g_hm_mode;

    return mm_error;
}

/**
  * @brief  data_on_6d: [set]  Select data for 6d function.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_low_pass_on_6d_t: change the values of low_pass_on_6d
  *                                   in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_data_on_6d_set(lsm6dsl_ctx_t *ctx,
                               lsm6dsl_low_pass_on_6d_t val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    reg.low_pass_on_6d = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  data_on_6d: [get]  Select data for 6d function.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_low_pass_on_6d_t: Get the values of low_pass_on_6d
  *                                   in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_data_on_6d_get(lsm6dsl_ctx_t *ctx,
                               lsm6dsl_low_pass_on_6d_t *val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_low_pass_on_6d_t) reg.low_pass_on_6d;

    return mm_error;
}

/**
  * @brief  xl_out_data: [set]  Accelerometer slope filter / high-pass
  *                      filter selection on outputs.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_hp_slope_xl_en_t: change the values of
  *                                   hp_slope_xl_en in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_out_data_set(lsm6dsl_ctx_t *ctx,
                                lsm6dsl_hp_slope_xl_en_t val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    reg.hp_slope_xl_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_out_data: [get]  Accelerometer slope filter / high-pass
  *                      filter selection on outputs.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_hp_slope_xl_en_t: Get the values of hp_slope_xl_en
  *                                   in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_out_data_get(lsm6dsl_ctx_t *ctx,
                                lsm6dsl_hp_slope_xl_en_t *val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_hp_slope_xl_en_t) reg.hp_slope_xl_en;

    return mm_error;
}

/**
  * @brief   xl_composite_filter_input: [set]  Accelerometer composite
  *                                     filter input selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_input_composite_t: change the values of input_composite
  *                                    in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_composite_filter_input_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_input_composite_t val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    reg.input_composite = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   xl_composite_filter_input: [get]  Accelerometer composite
  *                                     filter input selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_input_composite_t: Get the values of input_composite
  *                                    in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_composite_filter_input_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_input_composite_t *val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_input_composite_t) reg.input_composite;

    return mm_error;
}

/**
  * @brief  xl_hp_ref_mode: [set]  Enable accelerometer HP filter
  *                         reference mode.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of hp_ref_mode in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_hp_ref_mode_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    reg.hp_ref_mode = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_hp_ref_mode: [get]  Enable accelerometer HP filter
  *                         reference mode.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of hp_ref_mode in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_hp_ref_mode_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    *val = reg.hp_ref_mode;

    return mm_error;
}

/**
  * @brief  xl_cutoff: [set]  Accelerometer LPF2 and high-pass filter
  *                    configuration and cutoff setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_hpcf_xl_t: change the values of hpcf_xl in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_cutoff_set(lsm6dsl_ctx_t *ctx, lsm6dsl_hpcf_xl_t val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    reg.hpcf_xl = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_cutoff: [get]  Accelerometer LPF2 and high-pass filter
  *                    configuration and cutoff setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_hpcf_xl_t: Get the values of hpcf_xl in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_cutoff_get(lsm6dsl_ctx_t *ctx, lsm6dsl_hpcf_xl_t *val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_hpcf_xl_t) reg.hpcf_xl;

    return mm_error;
}

/**
  * @brief  xl_lpf2: [set]  Accelerometer low-pass filter LPF2 selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of lpf2_xl_en in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_lpf2_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    reg.lpf2_xl_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  xl_lpf2: [get]  Accelerometer low-pass filter LPF2 selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of lpf2_xl_en in reg CTRL8_XL
  *
  */
int32_t lsm6dsl_xl_lpf2_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl8_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL8_XL, (uint8_t *)&reg, 1);
    *val = reg.lpf2_xl_en;

    return mm_error;
}

/**
  * @brief   soft_iron_correction: [set]  Accelerometer composite
  *                                filter input selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of soft_en in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_soft_iron_correction_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    reg.soft_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   soft_iron_correction: [get]  Accelerometer composite
  *                                filter input selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of soft_en in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_soft_iron_correction_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    *val = reg.soft_en;

    return mm_error;
}

/**
  * @brief   den_stamping_selection: [set]  DEN stamping sensor selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_den_xl_g_t: change the values of den_xl_g in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_den_stamping_selection_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_den_xl_g_t val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    reg.den_xl_g = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   den_stamping_selection: [get]  DEN stamping sensor selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_den_xl_g_t: Get the values of den_xl_g in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_den_stamping_selection_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_den_xl_g_t *val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_den_xl_g_t) reg.den_xl_g;

    return mm_error;
}

/**
  * @brief   den_store_on_x_axis: [set]  DEN value stored in LSB of X-axis.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of den_x in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_den_store_on_x_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    reg.den_x = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   den_store_on_x_axis: [get]  DEN value stored in LSB of X-axis.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of den_x in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_den_store_on_x_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    *val = reg.den_x;

    return mm_error;
}

/**
  * @brief   den_store_on_y_axis: [set]  DEN value stored in LSB of Y-axis.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of den_y in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_den_store_on_y_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    reg.den_y = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   den_store_on_y_axis: [get]  DEN value stored in LSB of Y-axis.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of den_y in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_den_store_on_y_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    *val = reg.den_y;

    return mm_error;
}

/**
  * @brief   den_store_on_z_axis: [set]  DEN value stored in LSB of Z-axis.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of den_z in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_den_store_on_z_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    reg.den_z = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   den_store_on_z_axis: [get]  DEN value stored in LSB of Z-axis.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of den_z in reg CTRL9_XL
  *
  */
int32_t lsm6dsl_den_store_on_z_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl9_xl_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL9_XL, (uint8_t *)&reg, 1);
    *val = reg.den_z;

    return mm_error;
}

/**
  * @brief  significant_motion: [set]  Enable significant motion detection
  *                             function.This is effective if the FUNC_EN
  *                             bit is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of sign_motion_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_significant_motion_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    reg.sign_motion_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  significant_motion: [get]  Enable significant motion detection
  *                             function.This is effective if the FUNC_EN bit
  *                             is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of sign_motion_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_significant_motion_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    *val = reg.sign_motion_en;

    return mm_error;
}

/**
  * @brief  rst_step_counter: [set]  Reset pedometer step counter.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of pedo_rst_step in
  *                      reg CTRL10_C
  *
  */
int32_t lsm6dsl_rst_step_counter_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    reg.pedo_rst_step = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  rst_step_counter: [get]  Reset pedometer step counter.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of pedo_rst_step in reg CTRL10_C
  *
  */
int32_t lsm6dsl_rst_step_counter_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    *val = reg.pedo_rst_step;

    return mm_error;
}

/**
  * @brief  embedded_functions: [set]  Enable embedded functionalities
  *                             (pedometer, tilt, significant motion
  *                             detection, sensor hub and ironing).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of func_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_embedded_functions_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    reg.func_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  embedded_functions: [get]  Enable embedded functionalities
  *                             (pedometer, tilt, significant motion
  *                             detection, sensor hub and ironing).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of func_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_embedded_functions_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    *val = reg.func_en;

    return mm_error;
}

/**
  * @brief  tilt_calculation: [set]  Enable tilt calculation.This is
  *                           effective if the FUNC_EN bit is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of tilt_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_tilt_calculation_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    reg.tilt_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  tilt_calculation: [get]  Enable tilt calculation.This is
  *                           effective if the FUNC_EN bit is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of tilt_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_tilt_calculation_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    *val = reg.tilt_en;

    return mm_error;
}

/**
  * @brief  pedometer: [set]  Enable pedometer algorithm.This is effective
  *                    if the FUNC_EN bit is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of pedo_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_pedometer_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    reg.pedo_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  pedometer: [get]  Enable pedometer algorithm.This is effective
  *                    if the FUNC_EN bit is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of pedo_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_pedometer_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    *val = reg.pedo_en;

    return mm_error;
}

/**
  * @brief  timestamp_count: [set]  Enable timestamp count. The count is
  *                          saved in TIMESTAMP0_REG (40h),
  *                          TIMESTAMP1_REG (41h) and TIMESTAMP2_REG (42h).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of timer_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_timestamp_count_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    reg.timer_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  timestamp_count: [get]  Enable timestamp count. The count is
  *                          saved in TIMESTAMP0_REG (40h),
  *                          TIMESTAMP1_REG (41h) and TIMESTAMP2_REG (42h).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of timer_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_timestamp_count_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    *val = reg.timer_en;

    return mm_error;
}

/**
  * @brief  wrist_tilt: [set]  Enable wrist tilt algorithm. This is
  *                     effective if the FUNC_EN bit is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of wrist_tilt_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_wrist_tilt_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    reg.wrist_tilt_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  wrist_tilt: [get]  Enable wrist tilt algorithm. This is effective
  *                     if the FUNC_EN bit is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of wrist_tilt_en in reg CTRL10_C
  *
  */
int32_t lsm6dsl_wrist_tilt_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_ctrl10_c_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_CTRL10_C, (uint8_t *)&reg, 1);
    *val = reg.wrist_tilt_en;

    return mm_error;
}

/**
  * @brief  hard_iron: [set]  Enable hard-iron correction algorithm
  *                    for magnetometer.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of iron_en in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_hard_iron_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG, (uint8_t *)&reg, 1);
    reg.iron_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MASTER_CONFIG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  hard_iron: [get]  Enable hard-iron correction algorithm
  *                    for magnetometer.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of iron_en in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_hard_iron_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG, (uint8_t *)&reg, 1);
    *val = reg.iron_en;

    return mm_error;
}

/**
  * @brief  letched_interrupts: [set]  Latched/pulsed interrupt.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of lir in reg TAP_CFG
  *
  */
int32_t lsm6dsl_letched_interrupts_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    reg.lir = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  letched_interrupts: [get]  Latched/pulsed interrupt.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of lir in reg TAP_CFG
  *
  */
int32_t lsm6dsl_letched_interrupts_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    *val = reg.lir;

    return mm_error;
}

/**
  * @brief  tap_on_z_axis: [set]  Enable Z direction in tap recognition.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of tap_z_en in reg TAP_CFG
  *
  */
int32_t lsm6dsl_tap_on_z_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    reg.tap_z_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  tap_on_z_axis: [get]  Enable Z direction in tap recognition.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of tap_z_en in reg TAP_CFG
  *
  */
int32_t lsm6dsl_tap_on_z_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    *val = reg.tap_z_en;

    return mm_error;
}

/**
  * @brief  tap_on_y_axis: [set]  Enable Y direction in tap recognition.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of tap_y_en in reg TAP_CFG
  *
  */
int32_t lsm6dsl_tap_on_y_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    reg.tap_y_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  tap_on_y_axis: [get]  Enable Y direction in tap recognition.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of tap_y_en in reg TAP_CFG
  *
  */
int32_t lsm6dsl_tap_on_y_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    *val = reg.tap_y_en;

    return mm_error;
}

/**
  * @brief  tap_on_x_axis: [set]  Enable X direction in tap recognition.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of tap_x_en in reg TAP_CFG
  *
  */
int32_t lsm6dsl_tap_on_x_axis_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    reg.tap_x_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  tap_on_x_axis: [get]  Enable X direction in tap recognition.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of tap_x_en in reg TAP_CFG
  *
  */
int32_t lsm6dsl_tap_on_x_axis_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    *val = reg.tap_x_en;

    return mm_error;
}

/**
  * @brief  inactivity_mode: [set]  Enable inactivity function.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_inact_en_t: change the values of inact_en in reg TAP_CFG
  *
  */
int32_t lsm6dsl_inactivity_mode_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_inact_en_t val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    reg.inact_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  inactivity_mode: [get]  Enable inactivity function.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_inact_en_t: Get the values of inact_en in reg TAP_CFG
  *
  */
int32_t lsm6dsl_inactivity_mode_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_inact_en_t *val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_inact_en_t) reg.inact_en;

    return mm_error;
}

/**
  * @brief   enable_basic_interrupts: [set]  Enable basic interrupts
  *                                   (6D/4D, free-fall, wake-up, tap,
  *                                   inactivity).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of interrupts_enable in reg TAP_CFG
  *
  */
int32_t lsm6dsl_enable_basic_interrupts_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    reg.interrupts_enable = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   enable_basic_interrupts: [get]  Enable basic interrupts (6D/4D,
  *                                   free-fall, wake-up, tap, inactivity).
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of interrupts_enable in reg TAP_CFG
  *
  */
int32_t lsm6dsl_enable_basic_interrupts_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    *val = reg.interrupts_enable;

    return mm_error;
}

/**
  * @brief  threshold_6d_3d: [set]  Threshold for 4D/6D function.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_sixd_ths_t: change the values of sixd_ths in
  *                             reg TAP_THS_6D
  *
  */
int32_t lsm6dsl_threshold_6d_3d_set(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_sixd_ths_t val)
{
    lsm6dsl_tap_ths_6d_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_THS_6D, (uint8_t *)&reg, 1);
    reg.sixd_ths = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_THS_6D, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  threshold_6d_3d: [get]  Threshold for 4D/6D function.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_sixd_ths_t: Get the values of sixd_ths in reg TAP_THS_6D
  *
  */
int32_t lsm6dsl_threshold_6d_3d_get(lsm6dsl_ctx_t *ctx,
                                    lsm6dsl_sixd_ths_t *val)
{
    lsm6dsl_tap_ths_6d_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_THS_6D, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_sixd_ths_t) reg.sixd_ths;

    return mm_error;
}

/**
  * @brief  reduce_6d_to_4d: [set]  4D orientation detection enable.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of d4d_en in reg TAP_THS_6D
  *
  */
int32_t lsm6dsl_reduce_6d_to_4d_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_tap_ths_6d_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_THS_6D, (uint8_t *)&reg, 1);
    reg.d4d_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_THS_6D, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  reduce_6d_to_4d: [get]  4D orientation detection enable.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of d4d_en in reg TAP_THS_6D
  *
  */
int32_t lsm6dsl_reduce_6d_to_4d_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_tap_ths_6d_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_THS_6D, (uint8_t *)&reg, 1);
    *val = reg.d4d_en;

    return mm_error;
}

/**
  * @brief  tap_mode: [set]  Single/double-tap event enable.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_single_double_tap_t: change the values of
  *                                      single_double_tap in
  *                                      reg WAKE_UP_THS
  *
  */
int32_t lsm6dsl_tap_mode_set(lsm6dsl_ctx_t *ctx,
                             lsm6dsl_single_double_tap_t val)
{
    lsm6dsl_wake_up_ths_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_THS, (uint8_t *)&reg, 1);
    reg.single_double_tap = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_WAKE_UP_THS, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  tap_mode: [get]  Single/double-tap event enable.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_single_double_tap_t: Get the values of single_double_tap
  *                                      in reg WAKE_UP_THS
  *
  */
int32_t lsm6dsl_tap_mode_get(lsm6dsl_ctx_t *ctx,
                             lsm6dsl_single_double_tap_t *val)
{
    lsm6dsl_wake_up_ths_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_THS, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_single_double_tap_t) reg.single_double_tap;

    return mm_error;
}

/**
  * @brief   timestamp_resolution: [set]  Timestamp register resolution
  *                                setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_timer_hr_t: change the values of timer_hr
  *                             in reg WAKE_UP_DUR
  *
  */
int32_t lsm6dsl_timestamp_resolution_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_timer_hr_t val)
{
    lsm6dsl_wake_up_dur_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg, 1);
    reg.timer_hr = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   timestamp_resolution: [get]  Timestamp register
  *                                resolution setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_timer_hr_t: Get the values of timer_hr in
  *                             reg WAKE_UP_DUR
  *
  */
int32_t lsm6dsl_timestamp_resolution_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_timer_hr_t *val)
{
    lsm6dsl_wake_up_dur_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_timer_hr_t) reg.timer_hr;

    return mm_error;
}

/**
  * @brief   free_fall_threshold: [set]  Free fall threshold setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_ff_ths_t: change the values of ff_ths in reg FREE_FALL
  *
  */
int32_t lsm6dsl_free_fall_threshold_set(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_ff_ths_t val)
{
    lsm6dsl_free_fall_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FREE_FALL, (uint8_t *)&reg, 1);
    reg.ff_ths = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FREE_FALL, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   free_fall_threshold: [get]  Free fall threshold setting.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_ff_ths_t: Get the values of ff_ths in reg FREE_FALL
  *
  */
int32_t lsm6dsl_free_fall_threshold_get(lsm6dsl_ctx_t *ctx,
                                        lsm6dsl_ff_ths_t *val)
{
    lsm6dsl_free_fall_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FREE_FALL, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_ff_ths_t) reg.ff_ths;

    return mm_error;
}

/**
  * @brief  int1_routing: [set]  Functions routing on INT1 pad
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_md1_cfg_t: change the values of reg MD1_CFG
  *
  */
int32_t lsm6dsl_int1_routing_set(lsm6dsl_ctx_t *ctx, lsm6dsl_md1_cfg_t val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MD1_CFG, (uint8_t *)&val, 1);

    return mm_error;
}

/**
  * @brief  int1_routing: [get]  Functions routing on INT1 pad
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_md1_cfg_t: change the values of reg MD1_CFG
  *
  */
int32_t lsm6dsl_int1_routing_get(lsm6dsl_ctx_t *ctx, lsm6dsl_md1_cfg_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx,  LSM6DSL_MD1_CFG, (uint8_t *)val, 1);

    return mm_error;
}

/**
  * @brief  int2_routing: [set]  Functions routing on INT2 pad
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_md2_cfg_t: change the values of reg MD2_CFG
  *
  */
int32_t lsm6dsl_int2_routing_set(lsm6dsl_ctx_t *ctx, lsm6dsl_md2_cfg_t val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MD2_CFG, (uint8_t *)&val, 1);

    return mm_error;
}

/**
  * @brief  int2_routing: [get]  Functions routing on INT2 pad
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_md2_cfg_t: change the values of reg MD2_CFG
  *
  */
int32_t lsm6dsl_int2_routing_get(lsm6dsl_ctx_t *ctx, lsm6dsl_md2_cfg_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx,  LSM6DSL_MD2_CFG, (uint8_t *)val, 1);

    return mm_error;
}

/**
  * @brief  sincro_time_frame: [set]  sensor_synchronization time frame
  *                            with the step of 500 ms and full range of 5 s.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of tph
  *
  */
int32_t lsm6dsl_sincro_time_frame_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;
    uint8_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_SENSOR_SYNC_TIME_FRAME,
                                (uint8_t *)&reg, sizeof(uint8_t));
    reg &= 0xF0;
    reg |= (val & 0x0F);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_SENSOR_SYNC_TIME_FRAME,
                                 (uint8_t *)&reg, sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  sincro_time_frame: [get]  sensor_synchronization time frame
  *                            with the step of 500 ms and full range of 5 s.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of tph
  *
  */
int32_t lsm6dsl_sincro_time_frame_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_SENSOR_SYNC_TIME_FRAME, val,
                                sizeof(uint8_t));
    *val &= 0x0F;

    return mm_error;
}

/**
  * @brief  timestamp: [get] timestamp value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  axis1bit32_t : union that group together the timestamp data
  *
  */
int32_t lsm6dsl_timestamp_raw(lsm6dsl_ctx_t *ctx, axis1bit32_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_TIMESTAMP0_REG, (uint8_t *) buff,
                            sizeof(axis1bit32_t));
}

/**
  * @brief  step_timestamp: [get] step_timestamp value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  axis1bit16_t : union that group together the step_timestamp data
  *
  */
int32_t lsm6dsl_step_timestamp_raw(lsm6dsl_ctx_t *ctx, axis1bit16_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_STEP_TIMESTAMP_L, (uint8_t *) buff,
                            sizeof(axis1bit16_t));
}

/**
  * @brief  step_counter: [get] step_counter value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  axis1bit16_t : union that group together the step_counter data
  *
  */
int32_t lsm6dsl_step_counter_raw(lsm6dsl_ctx_t *ctx, axis1bit16_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_STEP_COUNTER_L, (uint8_t *) buff,
                            sizeof(axis1bit16_t));
}

/**
  * @brief  tap_treshold: [set]  Threshold for tap recognition. 1 LSb
  *                       corresponds to FS_XL/32.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of tap_ths
  *
  */
int32_t lsm6dsl_tap_treshold_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;
    uint8_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_THS_6D, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg &= 0xE0;
    reg |= (val & 0x1F);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_THS_6D, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  tap_treshold: [get]  Threshold for tap recognition.
  *                       1 LSb corresponds to FS_XL/32.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of tap_ths
  *
  */
int32_t lsm6dsl_tap_treshold_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_THS_6D, val,
                                sizeof(uint8_t));
    *val &= 0x1F;

    return mm_error;
}

/**
  * @brief  gap_double_tap: [set]  Duration of maximum time gap for double
  *                         tap recognition. The default value of these bits
  *                         is 0000b which corresponds to 16*ODR_XL time.
  *                         If the DUR[3:0] bits are set to a different value,
  *                         1LSB corresponds to 32*ODR_XL time.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of dur
  *
  */
int32_t lsm6dsl_gap_double_tap_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;
    uint8_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT_DUR2, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg &= 0x0F;
    reg |= ((val << 4) & 0xF0);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT_DUR2, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  gap_double_tap: [get]  Duration of maximum time gap for double
  *                         tap recognition. The default value of these
  *                         bits is 0000b which corresponds to 16*ODR_XL time.
  *                         If the DUR[3:0] bits are set to a different value,
  *                         1LSB corresponds to 32*ODR_XL time.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of dur
  *
  */
int32_t lsm6dsl_gap_double_tap_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT_DUR2, val,
                                sizeof(uint8_t));
    *val &= 0xF0 >> 4;

    return mm_error;
}

/**
  * @brief  quiet_after_tap: [set]  Quiet time is the time after the first
  *                          detected tap in which there must not be any
  *                          overthreshold event. The default value of these
  *                          bits is 00b which corresponds to 2*ODR_XL time.
  *                          If the QUIET[1:0] bits are set to a different
  *                          value, 1LSB corresponds to 4*ODR_XL time.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of quiet
  *
  */
int32_t lsm6dsl_quiet_after_tap_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;
    uint8_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT_DUR2, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg &= ~0x0C;
    reg |= ((val << 2) & 0x0C);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT_DUR2, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  quiet_after_tap: [get]  Quiet time is the time after the first
  *                          detected tap in which there must not be any
  *                          overthreshold event. The default value of these
  *                          bits is 00b which corresponds to 2*ODR_XL time.
  *                          If the QUIET[1:0] bits are set to a different
  *                          value, 1LSB corresponds to 4*ODR_XL time.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of quiet
  *
  */
int32_t lsm6dsl_quiet_after_tap_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT_DUR2, val,
                                sizeof(uint8_t));
    *val &= 0x0C >> 2;

    return mm_error;
}

/**
  * @brief   duration_tap_overthreshold: [set]  Maximum duration is the
  *                                      maximum time of an overthreshold
  *                                      signal detection to be recognized as
  *                                      a tap event. The default value of
  *                                      these bits is 00b which corresponds
  *                                      to 4*ODR_XL time. If the SHOCK[1:0]
  *                                      bits are set to a different value,
  *                                      1LSB corresponds to 8*ODR_XL time.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of shock
  *
  */
int32_t lsm6dsl_duration_tap_overthreshold_set(lsm6dsl_ctx_t *ctx,
        uint8_t val)
{
    int32_t mm_error;
    uint8_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT_DUR2, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg &= ~0x03;
    reg |= (val  & 0x03);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_INT_DUR2, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief   duration_tap_overthreshold: [get]  Maximum duration is the
  *                                      maximum time of an overthreshold
  *                                      signal detection to be recognized as
  *                                      a tap event. The default value of
  *                                      these bits is 00b which correspondsto
  *                                      4*ODR_XL time. If the SHOCK[1:0] bits
  *                                      are set to a different value,
  *                                      1LSB corresponds to 8*ODR_XL time.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of shock
  *
  */
int32_t lsm6dsl_duration_tap_overthreshold_get(lsm6dsl_ctx_t *ctx,
        uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_INT_DUR2, val,
                                sizeof(uint8_t));
    *val &= 0x03 >> 2;

    return mm_error;
}

/**
  * @brief  threshold_wakeup: [set]  Threshold for wake-up. 1 LSb corresponds
  *                           to FS_XL/64
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of wk_ths
  *
  */
int32_t lsm6dsl_threshold_wakeup_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;
    uint8_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_THS, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg &= ~0x3F;
    reg |= (val  & 0x3F);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_WAKE_UP_THS, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  threshold_wakeup: [get]  Threshold for wake-up. 1 LSb corresponds
  *                           to FS_XL/64
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of wk_ths
  *
  */
int32_t lsm6dsl_threshold_wakeup_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_THS, val,
                                sizeof(uint8_t));
    *val &= 0x3F >> 2;

    return mm_error;
}

/**
  * @brief  wait_before_sleep: [set]  Duration to go in sleep mode. Default
  *                            value: 0000 (this corresponds to 16 ODR)
  *                            1 LSB = 512 ODR
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of sleep_dur
  *
  */
int32_t lsm6dsl_wait_before_sleep_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;
    lsm6dsl_wake_up_dur_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg.sleep_dur = val;

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  wait_before_sleep: [get]  Duration to go in sleep mode.
  *                            Default value: 0000 (this corresponds
  *                            to 16 ODR) 1 LSB = 512 ODR
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of sleep_dur
  *
  */
int32_t lsm6dsl_wait_before_sleep_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_DUR, val,
                                sizeof(uint8_t));
    *val &= 0x0F >> 2;

    return mm_error;
}

/**
  * @brief  lsm6dsl_wakeup_dur: [set]  Wake-up duration event.
  *                             1LSB = 1 ODR_time
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of wake_dur
  *
  */
int32_t lsm6dsl_wakeup_dur_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;
    lsm6dsl_wake_up_dur_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg.wake_dur = val;

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  lsm6dsl_wakeup_dur: [get]  Wake-up duration event.
  *                             1LSB = 1 ODR_time
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of wake_dur
  *
  */
int32_t lsm6dsl_wakeup_dur_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_DUR, val,
                                sizeof(uint8_t));
    *val &= 0x60 >> 5;

    return mm_error;
}

/**
  * @brief  free_fall_duration: [set]  For the complete configuration of
  *                             the free fall duration, refer to FF_DUR5 in
  *                             WAKE_UP_DUR (5Ch) configuration.
  *                             Value is expressed in number of samples
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of ff_dur
  *
  */
int32_t lsm6dsl_free_fall_duration_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;
    uint8_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FREE_FALL, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg &= ~0xF8;
    reg |= ((val << 3)  & 0xF8);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FREE_FALL, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg,
                                sizeof(uint8_t));
    reg &= ~0x80;
    reg |= ((val << 7)  & 0x80);

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_WAKE_UP_DUR, (uint8_t *)&reg,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  free_fall_duration: [get]  For the complete configuration of
  *                             the free fall duration, refer to FF_DUR5
  *                             in WAKE_UP_DUR (5Ch) configuration.
  *                             Value is expressed in number of samples
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of ff_dur
  *
  */
int32_t lsm6dsl_free_fall_duration_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;
    uint8_t reg;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FREE_FALL, &reg,
                                sizeof(uint8_t));
    *val = (reg & 0xF8) >> 3;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_WAKE_UP_DUR, &reg,
                                sizeof(uint8_t));
    *val |= (reg & 0x80) >> 2;

    return mm_error;
}

/**
  * @brief  master_error_code: [get] master_error_code value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t : union that group together the master_error_code data
  *
  */
int32_t lsm6dsl_master_error_code_raw(lsm6dsl_ctx_t *ctx, uint8_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CMD_CODE, (uint8_t *) buff,
                            sizeof(uint8_t));
}

/**
  * @brief  synchro_error_code: [get] synchro_error_code value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t : union that group together the synchro_error_code data
  *
  */
int32_t lsm6dsl_synchro_error_code_raw(lsm6dsl_ctx_t *ctx, uint8_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_SENS_SYNC_SPI_ERROR_CODE,
                            (uint8_t *) buff, sizeof(uint8_t));
}

/**
  * @brief  ext_magnetometer: [get] ext_magnetometer value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  axis3bit16_t : union that group together the ext_magnetometer data
  *
  */
int32_t lsm6dsl_ext_magnetometer_raw(lsm6dsl_ctx_t *ctx, axis3bit16_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_OUT_MAG_RAW_X_L,
                            (uint8_t *) buff, sizeof(axis3bit16_t));
}

/**
  * @brief  xl_x_axis_offset: [set]  Accelerometer X-axis user offset
  *                           correction expressed in two’s complement,
  *                           weight depends on CTRL6_C(4) bit.
  *                           The value must be in the range [-127 127].
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of x_ofs_usr
  *
  */
int32_t lsm6dsl_xl_x_axis_offset_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_X_OFS_USR, &val, sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  xl_x_axis_offset: [get]  Accelerometer X-axis user offset
  *                           correction expressed in two’s complement,
  *                           weight depends on CTRL6_C(4) bit.
  *                           The value must be in the range [-127 127].
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of x_ofs_usr
  *
  */
int32_t lsm6dsl_xl_x_axis_offset_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_X_OFS_USR, val, sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  xl_y_axis_offset: [set]  Accelerometer Y-axis user offset
  *                           correction expressed in two’s complement,
  *                           weight depends on CTRL6_C(4) bit.
  *                           The value must be in the range [-127 127].
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of y_ofs_usr
  *
  */
int32_t lsm6dsl_xl_y_axis_offset_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_Y_OFS_USR, &val, sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  xl_y_axis_offset: [get]  Accelerometer Y-axis user offset
  *                           correction expressed in two’s complement,
  *                           weight depends on CTRL6_C(4) bit.
  *                           The value must be in the range [-127 127].
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of y_ofs_usr
  *
  */
int32_t lsm6dsl_xl_y_axis_offset_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_Y_OFS_USR, val,
                                sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  xl_z_axis_offset: [set]  Accelerometer Z-axis user offset
  *                           correction expressed in two’s complement,
  *                           weight depends on CTRL6_C(4) bit.
  *                           The value must be in the range [-127 127].
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of y_ofs_usr
  *
  */
int32_t lsm6dsl_xl_z_axis_offset_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_Z_OFS_USR, &val,
                                 sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  xl_z_axis_offset: [get]  Accelerometer Z-axis user offset
  *                           correction expressed in two’s complement,
  *                           weight depends on CTRL6_C(4) bit.
  *                           The value must be in the range [-127 127].
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: get the values of y_ofs_usr
  *
  */
int32_t lsm6dsl_xl_z_axis_offset_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_Z_OFS_USR, val, sizeof(uint8_t));

    return mm_error;
}

/**
  * @brief  sensor_hub_i2c: [set]  Sensor hub I2C master enable.
  *                         This is effective if the FUNC_EN bit
  *                         is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of master_on in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_i2c_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                (uint8_t *)&reg, 1);
    reg.master_on = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  sensor_hub_i2c: [get]  Sensor hub I2C master enable. This
  *                         is effective if the FUNC_EN bit is set to '1'.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of master_on in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_i2c_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                (uint8_t *)&reg, 1);
    *val = reg.master_on;

    return mm_error;
}

/**
  * @brief   sensor_hub_pass_through: [set]  Sensor hub I2C interface act
  *                                   as pass-through.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of pass_through_mode
  *                      in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_pass_through_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                (uint8_t *)&reg, 1);
    reg.pass_through_mode = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   sensor_hub_pass_through: [get]  Sensor hub I2C interface act
  *                                   as pass-through.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of pass_through_mode
  *                  in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_pass_through_get(lsm6dsl_ctx_t *ctx, uint8_t *val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                (uint8_t *)&reg, 1);
    *val = reg.pass_through_mode;

    return mm_error;
}

/**
  * @brief  sensor_hub_pull_up: [set]  Auxiliary I2C pull-up.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_pull_up_en_t: change the values of pull_up_en
  *                               in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_pull_up_set(lsm6dsl_ctx_t *ctx,
                                       lsm6dsl_pull_up_en_t val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                (uint8_t *)&reg, 1);
    reg.pull_up_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  sensor_hub_pull_up: [get]  Auxiliary I2C pull-up.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_pull_up_en_t: Get the values of pull_up_en
  *                               in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_pull_up_get(lsm6dsl_ctx_t *ctx,
                                       lsm6dsl_pull_up_en_t *val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_pull_up_en_t) reg.pull_up_en;

    return mm_error;
}

/**
  * @brief   sensor_hub_op_trigger: [set]  Sensor Hub trigger signal selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_start_config_t: change the values of start_config
  *                                 in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_op_trigger_set(lsm6dsl_ctx_t *ctx,
        lsm6dsl_start_config_t val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                (uint8_t *)&reg, 1);
    reg.start_config = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   sensor_hub_op_trigger: [get]  Sensor Hub trigger signal selection.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_start_config_t: Get the values of start_config
  *                                 in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_op_trigger_get(lsm6dsl_ctx_t *ctx,
        lsm6dsl_start_config_t *val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_start_config_t) reg.start_config;

    return mm_error;
}

/**
  * @brief   sensor_hub_drdy_on_int1: [set]  Manage the Master DRDY
  *                                   signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t val: change the values of drdy_on_int1
  *                      in reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_drdy_on_int1_set(lsm6dsl_ctx_t *ctx, uint8_t val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                (uint8_t *)&reg, 1);
    reg.drdy_on_int1 = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief   sensor_hub_drdy_on_int1: [get]  Manage the Master DRDY
  *                                   signal on INT1 pad.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t: change the values of drdy_on_int1 in
  *                  reg MASTER_CONFIG
  *
  */
int32_t lsm6dsl_sensor_hub_drdy_on_int1_get(lsm6dsl_ctx_t *ctx,
        uint8_t *val)
{
    lsm6dsl_master_config_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_MASTER_CONFIG,
                                (uint8_t *)&reg, 1);
    *val = reg.drdy_on_int1;

    return mm_error;
}

/**
  * @brief  res_ratio: [set]  Resolution ratio of error code for
  *                    sensor_synchronization
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_rr_t: change the values of rr in
  *                       reg SENSOR_SYNC_RES_RATIO
  *
  */
int32_t lsm6dsl_res_ratio_set(lsm6dsl_ctx_t *ctx, lsm6dsl_rr_t val)
{
    lsm6dsl_sensor_sync_res_ratio_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_SENSOR_SYNC_RES_RATIO,
                                (uint8_t *)&reg, 1);
    reg.rr = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_SENSOR_SYNC_RES_RATIO,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  res_ratio: [get]  Resolution ratio of error code for
  *                    sensor_synchronization
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_rr_t: Get the values of rr in
  *                       reg SENSOR_SYNC_RES_RATIO
  *
  */
int32_t lsm6dsl_res_ratio_get(lsm6dsl_ctx_t *ctx, lsm6dsl_rr_t *val)
{
    lsm6dsl_sensor_sync_res_ratio_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_SENSOR_SYNC_RES_RATIO,
                                (uint8_t *)&reg, 1);
    *val = (lsm6dsl_rr_t) reg.rr;

    return mm_error;
}

/**
  * @brief  sensorhub_set1_out: [get] sensorhub_set1_out value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t : union that group together the sensorhub_set1_out data
  *
  */
int32_t lsm6dsl_sensorhub_set1_out_raw(lsm6dsl_ctx_t *ctx, uint8_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_SENSORHUB1_REG, (uint8_t *) buff,
                            sizeof(uint8_t));
}

/**
  * @brief  sensorhub_set2_out: [get] sensorhub_set2_out value in DIGIT
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  uint8_t : union that group together the sensorhub_set2_out data
  *
  */
int32_t lsm6dsl_sensorhub_set2_out_raw(lsm6dsl_ctx_t *ctx, uint8_t *buff)
{
    return lsm6dsl_read_reg(ctx, LSM6DSL_SENSORHUB13_REG, (uint8_t *) buff,
                            sizeof(uint8_t));
}

/**
  * @brief  change_bank: [set]  Enable access to the embedded_functions
  *                      configuration registers bank A and B
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_func_cfg_en_t: change the values of func_cfg_en
  *                                in reg FUNC_CFG_EN
  *
  */
int32_t lsm6dsl_change_bank_set(lsm6dsl_ctx_t *ctx, lsm6dsl_func_cfg_en_t val)
{
    lsm6dsl_func_cfg_access_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FUNC_CFG_ACCESS,
                                (uint8_t *)&reg, 1);
    reg.func_cfg_en = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_FUNC_CFG_ACCESS,
                                 (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  change_bank: [get]  Enable access to the embedded_functions
  *                      configuration registers bank A and B
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_func_cfg_en_t: Get the values of func_cfg_en
  *                                in reg FUNC_CFG_EN
  *
  */
int32_t lsm6dsl_change_bank_get(lsm6dsl_ctx_t *ctx, lsm6dsl_func_cfg_en_t *val)
{
    lsm6dsl_func_cfg_access_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_FUNC_CFG_ACCESS,
                                (uint8_t *)&reg, 1);
    *val = (lsm6dsl_func_cfg_en_t) reg.func_cfg_en;

    return mm_error;
}

/**
  * @brief  data_for_wake_up: [set]  HPF or SLOPE filter selection on
  *                           wake-up and Activity/Inactivity functions.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_slope_fds_t: change the values of slope_fds in reg TAP_CFG
  *
  */
int32_t lsm6dsl_data_for_wake_up_set(lsm6dsl_ctx_t *ctx,
                                     lsm6dsl_slope_fds_t val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    reg.slope_fds = val;
    mm_error = lsm6dsl_write_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);

    return mm_error;
}

/**
  * @brief  data_for_wake_up: [get]  HPF or SLOPE filter selection
  *                           on wake-up and Activity/Inactivity functions.
  *
  * @param  lsm6dsl_ctx_t *ctx: read / write interface definitions
  * @param  lsm6dsl_slope_fds_t: Get the values of slope_fds in reg TAP_CFG
  *
  */
int32_t lsm6dsl_data_for_wake_up_get(lsm6dsl_ctx_t *ctx,
                                     lsm6dsl_slope_fds_t *val)
{
    lsm6dsl_tap_cfg_t reg;
    int32_t mm_error;

    mm_error = lsm6dsl_read_reg(ctx, LSM6DSL_TAP_CFG, (uint8_t *)&reg, 1);
    *val = (lsm6dsl_slope_fds_t) reg.slope_fds;

    return mm_error;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

lsm6dsl_odr_xl_t xl_odr_temp = XL_104Hz;
u8 lsm6dsl_xl_power_down()
{
    int32_t mm_error;
    mm_error = lsm6dsl_xl_data_rate_get(&lsm6dsl_ctx, &xl_odr_temp);
    if (mm_error != 1) {
        xl_odr_temp = XL_OFF;
        log_error("lsm6dsl acc power down fail!");
        return 0;
    }
    log_info("xl_odr:%d", xl_odr_temp);

    mm_error = lsm6dsl_xl_data_rate_set(&lsm6dsl_ctx, XL_OFF);
    if (mm_error == 1) {
        log_info("lsm6dsl acc power down ok!");
        return 1;
    } else {
        log_error("lsm6dsl acc power down fail!");
        return 0;
    }
}
u8 lsm6dsl_xl_wake_up()
{
    int32_t mm_error;
    if (xl_odr_temp == XL_OFF) {
        log_error("lsm6dsl acc wake up fail!");
        return 0;
    }
    mm_error = lsm6dsl_xl_data_rate_set(&lsm6dsl_ctx, xl_odr_temp);
    if (mm_error == 1) {
        log_info("lsm6dsl acc wake up ok!");
        return 1;
    } else {
        log_error("lsm6dsl acc wake up fail!");
        return 0;
    }
}



lsm6dsl_odr_g_t g_odr_temp = GY_104Hz;
u8 lsm6dsl_g_power_down()
{
    int32_t mm_error;
    mm_error = lsm6dsl_gyro_data_rate_get(&lsm6dsl_ctx, &g_odr_temp);
    if (mm_error != 1) {
        g_odr_temp = GY_OFF;
        log_error("lsm6dsl gyro power down fail!");
        return 0;
    }
    log_info("g_odr:%d", g_odr_temp);

    mm_error = lsm6dsl_gyro_data_rate_set(&lsm6dsl_ctx, GY_OFF);
    if (mm_error == 1) {
        log_info("lsm6dsl gyro power down ok!");
        return 1;
    } else {
        log_error("lsm6dsl gyro power down fail!");
        return 0;
    }
}
u8 lsm6dsl_g_wake_up()
{
    int32_t mm_error;
    if (g_odr_temp == GY_OFF) {
        log_error("lsm6dsl gyro wake up fail!");
        return 0;
    }
    mm_error = lsm6dsl_gyro_data_rate_set(&lsm6dsl_ctx, g_odr_temp);
    if (mm_error == 1) {
        log_info("lsm6dsl gyro wake up ok!");
        return 1;
    } else {
        log_error("lsm6dsl gyro wake up fail!");
        return 0;
    }
}
unsigned char lsm6dsl_init()
{
    unsigned char lsm6dsl_chip_id = 0x00, rst;
    unsigned char lsm6dsl_revision_id = 0x00;
    unsigned char iCount = 0;

    unsigned char fw_version[3] = {0};
    unsigned char usid_version[6] = {0};
#if (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_SPI)
    if (lsm6dsl_info->spi_work_mode) {
        lsm6dsl_spi_mode_set(&lsm6dsl_ctx, 1);//使能spi_3_wire
    } else {
        lsm6dsl_spi_mode_set(&lsm6dsl_ctx, 0);//使能spi_4_wire
    }
#endif
    /* Check device ID */
    while ((lsm6dsl_chip_id != LSM6DSL_ID) && (iCount < 10)) {
        lsm6dsl_device_id(&lsm6dsl_ctx, &lsm6dsl_chip_id);
        if (lsm6dsl_chip_id == LSM6DSL_ID) {
            break;
        }
        iCount++;
        mdelay(1);
    }

    if (lsm6dsl_chip_id == LSM6DSL_ID) {
        /*  Restore default configuration */
        lsm6dsl_reset_set(&lsm6dsl_ctx, PROPERTY_ENABLE);
        do {
            lsm6dsl_reset_get(&lsm6dsl_ctx, &rst);
        } while (rst);

        log_info("lsm6dsl_init slave=0x%x  lsm6dslRegister_WhoAmI=0x%x\n", LSM6DSL_IIC_SLAVE_ADDRESS, lsm6dsl_chip_id);

#if (LSM6DSL_USE_TAP_EN)
        /* tap_enable(); */
#endif
#if (LSM6DSL_USE_ANYMOTION_EN)
        /* anymotion_lowpwr_config(); */
#endif
#if (LSM6DSL_USE_FIFO_EN)
        lsm6dsl_fifo_write_mode_set(&lsm6dsl_ctx, ON_DRDY);

        lsm6dsl_fifo_temperature_set(&lsm6dsl_ctx, 0);//0: temperature not included in FIFO; 1: temperature included in FIFO //1:DS4_NO_DEC
        lsm6dsl_fifo_step_timestamp_set(&lsm6dsl_ctx, 0);//0: disable step counter and timestamp data as 3rd FIFO data set;  1:enable  //1:DS3_NO_DEC
        lsm6dsl_xl_decimation_set(&lsm6dsl_ctx, XL_NO_DEC);//抽样XL_DEC_4
        lsm6dsl_gyro_decimation_set(&lsm6dsl_ctx, GY_NO_DEC);//抽样GY_DEC_4
        lsm6dsl_dataset3_decimation_set(&lsm6dsl_ctx, DS3_NO_FILL);//DS3_NO_DEC:step=1
        lsm6dsl_dataset4_decimation_set(&lsm6dsl_ctx, DS4_NO_FILL);//DS4_NO_DEC:tmeperature=1

        lsm6dsl_fifo_xl_gy_8bit_data_set(&lsm6dsl_ctx, 0);//

        lsm6dsl_fifo_stop_on_thr_set(&lsm6dsl_ctx, 1);//0: FIFO depth is not limited; 1: FIFO depth is limited to threshold level
        lsm6dsl_fifo_threshold_set(&lsm6dsl_ctx, 504);//0~2047(word).stop_on_thr=1


        lsm6dsl_fifo_data_rate_set(&lsm6dsl_ctx, FIFO_104Hz);//be effective only fifo write mode=ON_DRDY.  fifo_rate<=acc&gyro rate
        /* lsm6dsl_fifo_mode_set(&lsm6dsl_ctx, CONTINUOS);fifo_mode_old = CONTINUOS; */
        lsm6dsl_fifo_mode_set(&lsm6dsl_ctx, STOP_WHEN_FULL);
        fifo_mode_old = STOP_WHEN_FULL;

        //fifo int:
#if LSM6DSL_FIFO_INT_OMAP_INT1
        /* lsm6dsl_fifo_threshold_on_int1_set(&lsm6dsl_ctx, 1); */
        /* lsm6dsl_fifo_overrun_on_int1_set(&lsm6dsl_ctx, 1);// */
        lsm6dsl_fifo_full_on_int1_set(&lsm6dsl_ctx, 1);
#else
        /* lsm6dsl_fifo_threshold_on_int2_set(&lsm6dsl_ctx, 1); */
        /* lsm6dsl_fifo_overrun_on_int2_set(&lsm6dsl_ctx, 1);// */
        lsm6dsl_fifo_full_on_int2_set(&lsm6dsl_ctx, 1);
#endif

#endif
#if (LSM6DSL_USE_STEP_EN)
        /* qmi8658_config.inputSelection = QMI8658_CONFIG_ACC_ENABLE; */
        /* qmi8658_config.accRange = Qmi8658AccRange_8g; */
        /* qmi8658_config.accOdr = Qmi8658AccOdr_LowPower_21Hz;//Qmi8658AccOdr_62_5Hz;//Qmi8658AccOdr_LowPower_21Hz; */
        /* qmi8658_config.gyrRange = Qmi8658GyrRange_1024dps;		//Qmi8658GyrRange_2048dps   Qmi8658GyrRange_1024dps */
        /* qmi8658_config.gyrOdr = Qmi8658GyrOdr_500Hz; */
        /*  */
        /* Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl7, 0x00);		// */
        /* Qmi8658_write(QMI8658_SLAVE_ADDRESS, Qmi8658Register_Ctrl8, 0xD0); */
        /* stepConfig(21);  //ODR =  21hz */
#endif
        /* Set xl Full Scale */
        lsm6dsl_xl_full_scale_set(&lsm6dsl_ctx, FS_16g);
        /* Set gyro Full Scale */
        lsm6dsl_gyro_full_scale_set(&lsm6dsl_ctx, FS_2000dps);
        /* Enable Acc Block Data Update */
        lsm6dsl_block_data_update_set(&lsm6dsl_ctx, PROPERTY_ENABLE);//fifo mode:updata=1

        /* Set AXL bandwith: ODR/100  */
        lsm6dsl_xl_lpf2_set(&lsm6dsl_ctx, 1);
        lsm6dsl_xl_cutoff_set(&lsm6dsl_ctx, XL_ODR_DIV_100);
        //lsm6dsl_xl_cutoff_set(&lsm6dsl_ctx, XL_ODR_DIV_400);
        /* Set GYR bandwith:121HZ */
        lsm6dsl_gyro_lpf1_set(&lsm6dsl_ctx, 1);
        lsm6dsl_gyro_lpf1_bandwidth_set(&lsm6dsl_ctx, GY_LPF1_VERY_AGGRESSIVE);

        /* Set xl Output Data Rate */
        lsm6dsl_xl_data_rate_set(&lsm6dsl_ctx, XL_104Hz);
        /* Set gyro Output Data Rate */
        lsm6dsl_gyro_data_rate_set(&lsm6dsl_ctx, GY_104Hz);
        //int:
#if LSM6DSL_USE_INT_EN
        /* lsm6dsl_xl_drdy_on_int2_set(&lsm6dsl_ctx, 1); */
        lsm6dsl_gyro_drdy_on_int2_set(&lsm6dsl_ctx, 1);
        /* lsm6dsl_temperature_drdy_on_int2_set(&lsm6dsl_ctx, 1); */
#endif

        /* Read samples in polling mode (no int) */
#if 0
        unsigned char read_data[10];
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_CTRL1_XL, (uint8_t *)&read_data, 10);
        log_info("LSM6DSL_CTRL1_XL, CTRL2_G,CTRL3_C,CTRL4_C,CTRL5_C,CTRL6_C,CTRL7_G,CTRL8_XL,CTRL9_XL,CTRL10_C\n");
        log_info_hexdump(read_data, 10);
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_FIFO_CTRL1, (uint8_t *)&read_data, 5);
        log_info("LSM6DSL_fifo_CTRL1~5\n");
        log_info_hexdump(read_data, 5);
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_INT1_CTRL, (uint8_t *)&read_data, 2);
        log_info("LSM6DSL_int_CTRL1~2\n");
        log_info_hexdump(read_data, 2);

#endif
        return 1;
    } else {
        log_error("lsm6dsl_init fail(id:0x%x)\n", lsm6dsl_chip_id);
        lsm6dsl_chip_id = 0;
        return 0;
    }
}

static u8 lsm6dsl_int_pin1 = IO_PORTB_04;
static u8 lsm6dsl_int_pin2 = IO_PORTB_03;
//return:0:fail, 1:ok
u8 lsm6dsl_sensor_init(void *priv)
{
    if (priv == NULL) {
        log_error("lsm6dsl init fail(no param)\n");
        return 0;
    }
    lsm6dsl_info = (lsm6dsl_param *)priv;

#if (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_I2C) //iic interface
    iic_init(lsm6dsl_info->iic_hdl);
#elif (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_SPI)//spi interface
    spi_cs_init();
    spi_init();
#else
#endif
    gpio_set_die(lsm6dsl_int_pin1, 1);
    gpio_set_direction(lsm6dsl_int_pin1, 1);
    // gpio_set_pull_up(lsm6dsl_int_pin1,1);
    gpio_set_die(lsm6dsl_int_pin2, 1);
    gpio_set_direction(lsm6dsl_int_pin2, 1);
    // gpio_set_pull_up(lsm6dsl_int_pin2,1);
    return lsm6dsl_init();
}







static volatile u32 lsm6dsl_init_flag ;
static u8 imu_busy = 0;
static lsm6dsl_param lsm6dsl_info_data;
/* #define SENSORS_MPU_BUFF_LEN 14 */
/* u8 read_lsm6dsl_buf[SENSORS_MPU_BUFF_LEN]; */
#if (LSM6DSL_USE_FIFO_EN)
static u8 lsm6dsl_fifo_data[1024 * 4]; //size:n*6 * Number_of_enabled_sensors<4kbytes
#endif
void lsm6dsl_int_callback()
{
    if (lsm6dsl_init_flag == 0) {
        log_error("lsm6dsl init fail!");
        return ;
    }
    if (imu_busy) {
        log_error("lsm6dsl busy!");
        return ;
    }
    imu_busy = 1;

    imu_sensor_data_t raw_sensor_datas;
    float TempData = 0.0;
#if (LSM6DSL_USE_FIFO_EN)
    u16 fifo_level = 0;//fifo cnt(bytes)
    fifo_level = lsm6dsl_read_fifo(&lsm6dsl_ctx, lsm6dsl_fifo_data);
    TempData = lsm6dsl_read_temperature(&lsm6dsl_ctx);
    /* log_info("lsm6dsl raw:gyro_x:%d gyro_y:%d gyro_z:%d, acc_x:%d acc_y:%d acc_z:%d tim_count:%d", (s16)(lsm6dsl_fifo_data[1]<<8|lsm6dsl_fifo_data[0]), (s16)(lsm6dsl_fifo_data[3]<<8|lsm6dsl_fifo_data[2]), (s16)(lsm6dsl_fifo_data[5]<<8|lsm6dsl_fifo_data[4]), (s16)(lsm6dsl_fifo_data[7]<<8|lsm6dsl_fifo_data[6]), (s16)(lsm6dsl_fifo_data[9]<<8|lsm6dsl_fifo_data[8]), (s16)(lsm6dsl_fifo_data[11]<<8|lsm6dsl_fifo_data[10]),fifo_level);//共fifo_level组数据, 只打印第一组数据 */
#else
    /* lsm6dsl_status_reg_t status_reg; */
    u8 status_reg;
    lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_STATUS_REG, (uint8_t *)&status_reg, 1);
    /* log_info("status:0x%x", status_reg); */
    if (status_reg & 0x03) {
        lsm6dsl_read_raw_acc_gyro_xyz(&lsm6dsl_ctx, &raw_sensor_datas);//获取原始值
        /* TempData = lsm6dsl_read_temperature(&lsm6dsl_ctx); */
        TempData = raw_sensor_datas.temp_data;
        /* log_info("lsm6dsl raw:acc_x:%d acc_y:%d acc_z:%d gyro_x:%d gyro_y:%d gyro_z:%d", raw_sensor_datas.acc.x, raw_sensor_datas.acc.y, raw_sensor_datas.acc.z, raw_sensor_datas.gyro.x, raw_sensor_datas.gyro.y, raw_sensor_datas.gyro.z); */
    }
#endif
    /* log_info("lsm6dsl temp:%d.%d\n", (u16)TempData, (u16)(((u16)(TempData * 100)) % 100)); */
    imu_busy = 0;
}

s8 lsm6dsl_dev_init(void *arg)
{
    if (arg == NULL) {
        log_error("lsm6dsl init fail(no arg)\n");
        return -1;
    }
#if (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_I2C)
    lsm6dsl_info_data.iic_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl;
    lsm6dsl_info_data.iic_delay = ((struct imusensor_platform_data *)arg)->peripheral_param0;   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
    // u8 iic_clk;      //iic_clk: <=400kHz
#elif (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_SPI)
    lsm6dsl_info_data.spi_hdl = ((struct imusensor_platform_data *)arg)->peripheral_hdl,     //SPIx (role:master)
                      lsm6dsl_info_data.spi_cs_pin = ((struct imusensor_platform_data *)arg)->peripheral_param0; //IO_PORTA_05
    lsm6dsl_info_data.spi_work_mode = ((struct imusensor_platform_data *)arg)->peripheral_param1; //1:3wire(SPI_MODE_UNIDIR_1BIT) or 0:4wire(SPI_MODE_BIDIR_1BIT) (与spi结构体一样)
    // u8 port;         //SPIx group:A,B,C,D (spi结构体)
    // U8 spi_clk;      //spi_clk: <=15MHz (spi结构体)
#else
#endif
    lsm6dsl_int_pin2 = ((struct imusensor_platform_data *)arg)->imu_sensor_int_io;
    lsm6dsl_int_pin1 = lsm6dsl_int_pin2;
    if (imu_busy) {
        log_error("lsm6dsl busy!");
        return -1;
    }
    imu_busy = 1;
    if (lsm6dsl_sensor_init(&lsm6dsl_info_data)) {
#if (LSM6DSL_USE_INT_EN) //中断模式,暂不支持
        log_info("int mode en!");
        /* port_wkup_enable(lsm6dsl_int_pin2, 1, lsm6dsl_int_callback); //PA08-IO中断，1:下降沿触发，回调函数lsm6dsl_int_callback*/
#ifdef CONFIG_CPU_BR23
        io_ext_interrupt_init(lsm6dsl_int_pin2, 1, lsm6dsl_int_callback);
#elif defined(CONFIG_CPU_BR28)
        // br28外部中断回调函数，按照现在的外部中断注册方式
        // io配置在板级，定义在板级头文件，这里只是注册回调函数
        /* port_edge_wkup_set_callback_by_index(3, lsm6dsl_int_callback); // 序号需要和板级配置中的wk_param对应上 */
        port_edge_wkup_set_callback(lsm6dsl_int_callback);
#elif defined(CONFIG_CPU_BR27)
        port_edge_wkup_set_callback(lsm6dsl_int_callback);
#endif
#else //定时

#endif
        lsm6dsl_init_flag = 1;
        imu_busy = 0;
        log_info("lsm6dsl Device init success!%d\n", lsm6dsl_init_flag);
        return 0;
    } else {
        log_info("lsm6dsl Device init fail!\n");
        imu_busy = 0;
        return -1;
    }
}


int lsm6dsl_dev_ctl(u8 cmd, void *arg);
REGISTER_IMU_SENSOR(lsm6dsl_sensor) = {
    .logo = "lsm6dsl",
    .imu_sensor_init = lsm6dsl_dev_init,
    .imu_sensor_check = NULL,
    .imu_sensor_ctl = lsm6dsl_dev_ctl,
};

int lsm6dsl_dev_ctl(u8 cmd, void *arg)
{
    int ret = -1;
    u8 status_temp = 0;
    /* lsm6dsl_status_reg_t status_temp; */
    if (lsm6dsl_init_flag == 0) {
        log_error("lsm6dsl init fail!");
        return ret;//0:ok,,<0:err
    }
    if (imu_busy) {
        log_error("lsm6dsl busy!");
        return ret;//0:ok,,<0:err
    }
    imu_busy = 1;
    switch (cmd) {
    case IMU_GET_SENSOR_NAME:
        memcpy((u8 *)arg, &(lsm6dsl_sensor.logo), 20);
        ret = 0;
        break;
    case IMU_SENSOR_ENABLE: //enable demand cali
        /* cbuf_init(&hrsensor_cbuf, hrsensorcbuf, 24 * sizeof(int)); */
        lsm6dsl_init();
        ret = 0;
        break;
    case IMU_SENSOR_DISABLE:
        /* cbuf_clear(&hrsensor_cbuf); */
        //close power
        lsm6dsl_xl_power_down();
        lsm6dsl_g_power_down();
        ret = 0;
        break;
    case IMU_SENSOR_RESET://reset后必须enable
        /*  Restore default configuration */
        u8 rst;
        lsm6dsl_reset_set(&lsm6dsl_ctx, PROPERTY_ENABLE);
        do {
            lsm6dsl_reset_get(&lsm6dsl_ctx, &rst);
        } while (rst);
        ret = 0;
        break;
    case IMU_SENSOR_SLEEP:
        if ((lsm6dsl_xl_power_down()) && (lsm6dsl_g_power_down())) {
            log_info("lsm6dsl enter sleep ok!");
            ret = 0;
        } else {
            log_error("lsm6dsl enter sleep fail!");
        }
        break;
    case IMU_SENSOR_WAKEUP: //disable demand cali
        if ((lsm6dsl_xl_wake_up()) && (lsm6dsl_g_wake_up())) {
            log_info("lsm6dsl wakeup ok!");
            ret = 0;
        } else {
            log_error("lsm6dsl wakeup fail!");
        }
        break;
    case IMU_SENSOR_INT_DET://传感器中断状态检查
        break;
    case IMU_SENSOR_DATA_READY://传感器数据准备就绪待读
        break;
    case IMU_SENSOR_CHECK_DATA://检查传感器缓存buf是否存满
        break;
    case IMU_SENSOR_READ_DATA://默认读传感器所有数据
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_STATUS_REG, (uint8_t *)&status_temp, 1);
        /* log_info("status:0x%x", status_temp); */
        if (status_temp & 0x03) {
            lsm6dsl_read_raw_acc_gyro_xyz(&lsm6dsl_ctx, arg);//获取原始值
            ret = 0;
        }
        break;
    case IMU_GET_ACCEL_DATA://加速度数据
        /* float TempData = 0.0; */
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_STATUS_REG, (uint8_t *)&status_temp, 1);
        /* log_info("status:0x%x", status_temp); */
        if (status_temp & 0x01) {
            lsm6dsl_read_raw_acc_xyz(&lsm6dsl_ctx, arg);//获取原始值
            ret = 0;
            /* TempData = lsm6dsl_read_temperature(&lsm6dsl_ctx); */
        }
        break;
    case IMU_GET_GYRO_DATA://陀螺仪数据
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_STATUS_REG, (uint8_t *)&status_temp, 1);
        /* log_info("status:0x%x", status_temp);  */
        if (status_temp & 0x02) {
            lsm6dsl_read_raw_gyro_xyz(&lsm6dsl_ctx, arg);//获取原始值
            ret = 0;
        }
        break;
    case IMU_GET_MAG_DATA://磁力计数据
        log_error("lsm6dsl have no mag!\n");
        break;
    case IMU_SENSOR_SEARCH://检查传感器id
        lsm6dsl_device_id(&lsm6dsl_ctx, (u8 *)arg);
        if (*(u8 *)arg == LSM6DSL_ID) {
            ret = 0;
            log_info("lsm6dsl online!\n");
        } else {
            log_error("lsm6dsl offline!\n");
        }
        break;
    case IMU_GET_SENSOR_STATUS://获取传感器状态
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_STATUS_REG, (uint8_t *)&status_temp, 1);
        *(u8 *)arg = status_temp;
        ret = 0;
        break;
    case IMU_SET_SENSOR_FIFO_CONFIG://配置传感器FIFO
#if (LSM6DSL_USE_FIFO_EN)
        /* u8 *tmp = (u8 *)arg; */
        /* u8 wm_th = tmp[0]; */
        /* u8 fifo_size = tmp[1];//Qmi8658_Fifo_64 */
        /* u8 fifo_mode = tmp[2];//Qmi8658_Fifo_Stream */
        /* u8 fifo_format = tmp[3];//QMI8658_FORMAT_12_BYTES */
        /* Qmi8658_config_fifo(wm_th, fifo_size, fifo_mode, fifo_format); */
        ret = 0;
#endif
        break;
    case IMU_GET_SENSOR_READ_FIFO://读取传感器FIFO数据
#if (LSM6DSL_USE_FIFO_EN)
        ret = lsm6dsl_read_fifo(&lsm6dsl_ctx, (u8 *)arg);
#endif
        break;
    case IMU_SET_SENSOR_TEMP_DISABLE://温度传感器无法控制
        u8 temp = *(u8 *)arg;
        ret = 0;
        break;
    default:
        log_error("--cmd err!\n");
        break;
    }
    imu_busy = 0;
    return ret;//0:ok,,<0:err

}






/***************************lsm6dsl test*******************************/
#if 0 //测试
static lsm6dsl_param lsm6dsl_info_test = {
#if (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_I2C)
    .iic_hdl = 0,
    .iic_delay = 0,   //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
#elif (LSM6DSL_USER_INTERFACE==LSM6DSL_USE_SPI)
    .spi_hdl = 1,     //SPIx (role:master)
    .spi_cs_pin = IO_PORTA_05, //
    .spi_work_mode = 0; //4wire
#endif
};



/* Main Example --------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static axis3bit16_t data_raw;
static axis3bit32_t acceleration_mg;
static axis3bit32_t angular_rate_mdps;
static axis1bit16_t temperature_deg_c;
static uint8_t USBbuffer[5000];
int example_main(void)
{
    if (lsm6dsl_sensor_init(&lsm6dsl_info_test)) {
        //ok
    }
    /*
     * Read samples in polling mode (no int)
     */
    u16 fifo_cnt = 0;
    static axis3bit16_t data_raw_acc;
    local_irq_disable();
    while (1) {
#if (LSM6DSL_USE_FIFO_EN)
        fifo_cnt = lsm6dsl_read_fifo(&lsm6dsl_ctx, USBbuffer);
        if (fifo_cnt > 0) {
            for (u16 i = 0; i < 84; i++) {
                memcpy((u8 *)&data_raw, USBbuffer + i * 12, 6);
                memcpy((u8 *)&data_raw_acc, USBbuffer + i * 12 + 6, 6);
                printf("gy:%6d\t%6d\t%6d\t\txl:%6d\t%6d\t%6d,  %d", data_raw.i16bit[0], data_raw.i16bit[1], data_raw.i16bit[2], data_raw_acc.i16bit[0], data_raw_acc.i16bit[1], data_raw_acc.i16bit[2], i);
            }
            memset(USBbuffer, 0, 1024);
        }
#else
        /*
         * Read output only if new value is available
         */
        lsm6dsl_status_reg_t status_reg;
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_STATUS_REG, (uint8_t *)&status_reg, 1);
        log_info("status reg:0x%x", *((u8 *)&status_reg));
        lsm6dsl_read_reg(&lsm6dsl_ctx, LSM6DSL_STATUS_REG, (uint8_t *)&status_reg, 1);
        log_info("status reg:0x%x", *((u8 *)&status_reg));

        if (status_reg.gda) {
            lsm6dsl_angular_rate_raw(&lsm6dsl_ctx, &data_raw);
            angular_rate_mdps.i32bit[0] = FROM_FS_1000dps_TO_mdps(data_raw.i16bit[0]);
            angular_rate_mdps.i32bit[1] = FROM_FS_1000dps_TO_mdps(data_raw.i16bit[1]);
            angular_rate_mdps.i32bit[2] = FROM_FS_1000dps_TO_mdps(data_raw.i16bit[2]);

            printf("gy:%6d\t%6d\t%6d\t\t", data_raw.i16bit[0], data_raw.i16bit[1], data_raw.i16bit[2]);
            printf("gy:%6d\t%6d\t%6d\r\n", angular_rate_mdps.i32bit[0], angular_rate_mdps.i32bit[1], angular_rate_mdps.i32bit[2]);
        }

        if (status_reg.xlda) {
            lsm6dsl_acceleration_raw(&lsm6dsl_ctx, &data_raw);
            acceleration_mg.i32bit[0] = FROM_FS_4g_TO_mg(data_raw.i16bit[0]);
            acceleration_mg.i32bit[1] = FROM_FS_4g_TO_mg(data_raw.i16bit[1]);
            acceleration_mg.i32bit[2] = FROM_FS_4g_TO_mg(data_raw.i16bit[2]);

            printf("xl:%6d\t%6d\t%6d\t\t", data_raw.i16bit[0], data_raw.i16bit[1], data_raw.i16bit[2]);
            printf("xl:%6d\t%6d\t%6d\r\n", acceleration_mg.i32bit[0],
                   acceleration_mg.i32bit[1], acceleration_mg.i32bit[2]);
        }
        /*
         * If the gyroscope is not in Power-Down mode, the temperature data rate
         * is equal to 52Hz, regardless of the accelerometer and gyroscope
         * configuration. (Ref AN5040)
         */
        if (status_reg.tda) {
            lsm6dsl_temperature_raw(&lsm6dsl_ctx, &temperature_deg_c);
            printf("temp:%3d\t\t", temperature_deg_c.i16bit);
            temperature_deg_c.i16bit = FROM_LSB_TO_degC(temperature_deg_c.i16bit);

            printf("temp:%3d\r\n", temperature_deg_c.i16bit);
        }
#endif
        mdelay(100);
        wdt_clear();
    }
    return 0;
}

#endif



#endif
