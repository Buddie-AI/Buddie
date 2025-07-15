/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2017 InvenSense Inc. All rights reserved.
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

/** @defgroup DriverApex IMU driver high level functions related to APEX and the DMP
 *  @brief High-level function to setup an IMU device
 *  @ingroup  Driver
 *  @{
 */

/** @file inv_imu_apex.h
 * High-level function to setup an IMU device
 */

#ifndef _INV_IMU_APEX_H_
#define _INV_IMU_APEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "inv_imu_defs.h"

#include "InvError.h"

#include <stdint.h>
#include <string.h>

/* Forward declarations */
struct inv_imu_device;

/** @brief IMU APEX inputs parameters definition
 */
typedef struct {
    APEX_CONFIG3_PEDO_AMP_TH_t pedo_amp_th;
    uint8_t pedo_step_cnt_th;
    uint8_t pedo_step_det_th;
    APEX_CONFIG4_PEDO_SB_TIMER_TH_t pedo_sb_timer_th;
    APEX_CONFIG4_PEDO_HI_ENRGY_TH_t pedo_hi_enrgy_th;
    APEX_CONFIG5_TILT_WAIT_TIME_t tilt_wait_time;
    APEX_CONFIG2_DMP_POWER_SAVE_TIME_t power_save_time;
    APEX_CONFIG0_DMP_POWER_SAVE_t power_save;
    APEX_CONFIG9_SENSITIVITY_MODE_t sensitivity_mode;
    APEX_CONFIG2_LOW_ENERGY_AMP_TH_t low_energy_amp_th;
    APEX_CONFIG9_SMD_SENSITIVITY_t smd_sensitivity;
    APEX_CONFIG9_FF_DEBOUNCE_DURATION_t ff_debounce_duration;
    APEX_CONFIG12_FF_MAX_DURATION_t ff_max_duration_cm;
    APEX_CONFIG12_FF_MIN_DURATION_t ff_min_duration_cm;
    APEX_CONFIG10_LOWG_PEAK_TH_t lowg_peak_th;
    APEX_CONFIG5_LOWG_PEAK_TH_HYST_t lowg_peak_hyst;
    APEX_CONFIG10_LOWG_TIME_TH_SAMPLES_t lowg_samples_th;
    APEX_CONFIG11_HIGHG_PEAK_TH_t highg_peak_th;
    APEX_CONFIG5_HIGHG_PEAK_TH_HYST_t highg_peak_hyst;
    APEX_CONFIG11_HIGHG_TIME_TH_SAMPLES_t highg_samples_th;
} inv_imu_apex_parameters_t;

/** @brief APEX pedometer outputs
 */
typedef struct inv_imu_apex_step_activity {
    uint16_t step_cnt;      /**< Number of steps taken */
    uint8_t step_cadence;   /**< Walk/run cadence in number of samples.
	                             Format is u6.2. E.g, At 50Hz and 2Hz walk frequency, if the cadency is 25 samples.
	                             The register will output 100. */
    uint8_t activity_class; /**< Detected activity unknown (0), walk (1) or run (2) */
} inv_imu_apex_step_activity_t;

/** @brief  Enable Free Fall.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_enable_ff(struct inv_imu_device *s);

/** @brief  Disable Free Fall.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_disable_ff(struct inv_imu_device *s);

/** @brief  Enable Significant Motion Detection.
 *  note : SMD requests to have the accelerometer enabled to work.
 *  To have good performance, it's recommended to set accelerometer ODR (Output Data Rate) to 20ms
 *  and the accelerometer in Low Power Mode.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_enable_smd(struct inv_imu_device *s);

/** @brief  Disable Significant Motion Detection.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_disable_smd(struct inv_imu_device *s);

/** @brief Fill the APEX parameters structure with all the default parameters for APEX algorithms (pedometer, tilt)
 *  @param[out] apex_inputs Default input parameters. See @sa inv_imu_apex_parameters_t
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_init_parameters_struct(struct inv_imu_device *s, inv_imu_apex_parameters_t *apex_inputs);

/** @brief Configures DMP parameters for APEX algorithms (pedometer, tilt, lowg, highg).
 *         This programmable parameters will be decoded and propagate to the SRAM to be executed at DMP start.
 *  @param[in] apex_inputs The requested input parameters. See @sa inv_imu_apex_parameters_t
 *  @warning APEX inputs can't change on the fly, this API should be called before enabling any APEX features.
 *  @warning APEX configuration can't be done too frequently, but only once every 10ms.
 *           Otherwise it can create unknown behavior.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_configure_parameters(struct inv_imu_device *s, const inv_imu_apex_parameters_t *apex_inputs);

/** @brief Returns current DMP parameters for APEX algorithms (pedometer, tilt).
 *  @param[out] apex_params The current parameter, fetched from registers. See @sa inv_imu_apex_parameters_t
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_get_parameters(struct inv_imu_device *s, inv_imu_apex_parameters_t *apex_params);

/** @brief Configure DMP Output Data Rate for APEX algorithms (pedometer, tilt)
 *  @param[in] frequency The requested frequency.
 *  @sa APEX_CONFIG1_DMP_ODR_t
 *  @warning DMP_ODR can change on the fly, and the DMP code will accommodate necessary modifications
 *  @warning The user needs to take care to set Accel frequency >= DMP frequency. This is a hard constraint
 since HW will not handle incorrect setting.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_set_frequency(struct inv_imu_device *s, const APEX_CONFIG1_DMP_ODR_t frequency);

/** @brief  Enable APEX algorithm Pedometer.
 *  note : Pedometer request to have the accelerometer enabled to works
 *         with accelerometer frequency less than dmp frequency.
 *  @return 0 on success, negative value on error.
 *  @warning Pedometer must be turned OFF to reconfigure it
 */
int inv_imu_apex_enable_pedometer(struct inv_imu_device *s);

/** @brief  Disable APEX algorithm Pedometer.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_disable_pedometer(struct inv_imu_device *s);

/** @brief  Enable APEX algorithm Tilt.
 *  note : Tilt request to have the accelerometer enabled to works
 *         with accelerometer frequency less than dmp frequency.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_enable_tilt(struct inv_imu_device *s);

/** @brief  Disable APEX algorithm Tilt.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_apex_disable_tilt(struct inv_imu_device *s);

/** @brief  Retrieve APEX pedometer outputs and format them
 *  @param[out] apex_activity Apex step and activity data value.
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_imu_apex_get_data_activity(struct inv_imu_device *s, inv_imu_apex_step_activity_t *apex_activity);

/** @brief  Retrieve APEX free fall outputs and format them
 *  @param[out] Free fall duration in number of sample.
 *  @return 0 in case of success, negative value on error. See enum inv_error
 */
int inv_imu_apex_get_data_free_fall(struct inv_imu_device *s, uint16_t *freefall_duration);


#ifdef __cplusplus
}
#endif

#endif /* _INV_IMU_APEX_H_ */

/** @} */

