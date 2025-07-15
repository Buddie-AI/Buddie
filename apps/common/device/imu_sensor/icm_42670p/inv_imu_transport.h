/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2015-2015 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively "Software") is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

/** @defgroup  Transport IMU transport
 *  @brief    Low-level IMU SCLK register access
 *  @ingroup  Driver
 *  @{
 */

/** @file  inv_imu_transport.h
 * Low-level IMU SCLK register access
 */

#ifndef _INV_IMU_TRANSPORT_H_
#define _INV_IMU_TRANSPORT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* forward declaration */
struct inv_imu_device;


/** @brief enumeration  of serial interfaces available on IMU */
typedef enum {
    UI_I2C,
    UI_SPI4,
    UI_SPI3
} SERIAL_IF_TYPE_t;

/** @brief basesensor serial interface
 */
struct inv_imu_serif {
    void *context;
    int (*read_reg)(struct inv_imu_serif *serif, uint8_t reg, uint8_t *buf, uint32_t len);
    int (*write_reg)(struct inv_imu_serif *serif, uint8_t reg, const uint8_t *buf, uint32_t len);
    int (*configure)(struct inv_imu_serif *serif);
    uint32_t max_read;
    uint32_t max_write;
    SERIAL_IF_TYPE_t serif_type;
};

/** @brief transport interface
 */
struct inv_imu_transport {
    struct inv_imu_serif serif; /**< Warning : this field MUST be the first one of struct inv_imu_transport */

    /** @brief Contains mirrored values of some IP registers */
    struct register_cache {
        uint8_t pwr_mgmt0_reg;     /**< PWR_MGMT0, Bank: 0 */
        uint8_t gyro_config0_reg;  /**< GYRO_CONFIG0, Bank: 0 */
        uint8_t accel_config0_reg; /**< ACCEL_CONFIG0, Bank: 0 */
        uint8_t tmst_config1_reg;  /**< TMST_CONFIG1, Bank: MREG_TOP1 */
    } register_cache; /**< Store mostly used register values on SRAM */

    uint8_t need_mclk_cnt; /**< internal counter to keep track of everyone that needs MCLK */

};

/** @brief Init cache variable.
 * @return            0 in case of success, -1 for any error
 */
int inv_imu_init_transport(struct inv_imu_device *s);

/** @brief Reads data from a register on IMU.
 * @param[in] reg    register address to be read
 * @param[in] len    number of byte to be read
 * @param[out] buf   output data from the register
 * @return            0 in case of success, -1 for any error
 */
int inv_imu_read_reg(struct inv_imu_device *s, uint32_t reg, uint32_t len, uint8_t *buf);

/** @brief Writes data to a register on IMU.
 * @param[in] reg    register address to be written
 * @param[in] len    number of byte to be written
 * @param[in] buf    input data to write
 * @return            0 in case of success, -1 for any error
 */
int inv_imu_write_reg(struct inv_imu_device *s, uint32_t reg, uint32_t len, const uint8_t *buf);

/** @brief Enable MCLK so that MREG are clocked and system beyond SOI can be safely accessed
 * @return            0 in case of success, -1 for any error
 */
int inv_imu_switch_on_mclk(struct inv_imu_device *s);

/** @brief Disable MCLK so that MREG are not clocked anymore, hence reducing power consumption
 * @return            0 in case of success, -1 for any error
 */
int inv_imu_switch_off_mclk(struct inv_imu_device *s);

#ifdef __cplusplus
}
#endif

#endif /* _INV_IMU_TRANSPORT_H_ */

/** @} */

