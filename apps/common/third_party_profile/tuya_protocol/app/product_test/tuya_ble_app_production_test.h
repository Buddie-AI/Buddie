/**
 * \file tuya_ble_app_production_test.h
 *
 * \brief
 */
/*
 *  Copyright (C) 2014-2019, Tuya Inc., All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of tuya ble sdk
 */

#ifndef _TUYA_BLE_APP_PRODUCTION_TEST_H_
#define _TUYA_BLE_APP_PRODUCTION_TEST_H_

#include <stdint.h>
#include "tuya_ble_internal_config.h"



#ifdef __cplusplus
extern "C" {
#endif

#define      TUYA_BLE_AUC_CMD_ENTER              0x00
#define      TUYA_BLE_AUC_CMD_QUERY_HID          0x01
#define      TUYA_BLE_AUC_CMD_GPIO_TEST          0x02
#define      TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO    0x03
#define      TUYA_BLE_AUC_CMD_QUERY_INFO         0x04
#define      TUYA_BLE_AUC_CMD_RESET              0x05
#define      TUYA_BLE_AUC_CMD_QUERY_FINGERPRINT  0x06
#define      TUYA_BLE_AUC_CMD_WRITE_HID          0x07
#define      TUYA_BLE_AUC_CMD_RSSI_TEST          0x08
#define      TUYA_BLE_AUC_CMD_WRITE_OEM_INFO     0x09
#define      TUYA_BLE_AUC_CMD_WRITE_SUBPKG_START 0x0C
#define      TUYA_BLE_AUC_CMD_WRITE_SUBPKG_END   0x0D
#define      TUYA_BLE_AUC_CMD_WRITE_COMM_CFG     0x12 // ADD
#define      TUYA_BLE_AUC_CMD_READ_MAC           0x13
#define      TUYA_BLE_AUC_CMD_EXIT               0x14

#define      TUYA_BLE_AUC_CMD_EXTEND             0xF0
#define      TUYA_BLE_SDK_TEST_CMD_EXTEND        0xF2


#define      TUYA_BLE_AUC_FINGERPRINT_VER   1

#if ( TUYA_BLE_PROD_SUPPORT_OEM_TYPE == TUYA_BLE_PROD_OEM_TYPE_0_5 )
#define      TUYA_BLE_AUC_WRITE_PID         1
#else
#define      TUYA_BLE_AUC_WRITE_PID         0
#endif

#define      TUYA_BLE_AUC_WRITE_DEV_CERT    1

enum {
    TUYA_BLE_AUC_FW_FINGERPRINT_POS = 0,
    TUYA_BLE_AUC_WRITE_PID_POS      = 1,
    TUYA_BLE_AUC_WRITE_DEV_CERT_POS = 5,
};


/*channel: 0-uart ,1 - ble.*/
void tuya_ble_app_production_test_process(uint8_t channel, uint8_t *p_in_data, uint16_t in_len);


#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE&&TUYA_BLE_DEVICE_AUTH_DATA_STORE)

tuya_ble_status_t tuya_ble_prod_beacon_scan_start(void);

tuya_ble_status_t tuya_ble_prod_beacon_scan_stop(void);

tuya_ble_status_t tuya_ble_prod_beacon_get_rssi_avg(int8_t *rssi);

tuya_ble_status_t tuya_ble_prod_gpio_test(void);

void tuya_ble_internal_production_test_with_ble_flag_clear(void);

uint8_t tuya_ble_internal_production_test_with_ble_flag_get(void);

#endif


#ifdef __cplusplus
}
#endif

#endif // _TUYA_BLE_APP_PRODUCTION_TEST_H_

