/**
 * \file tuya_ble_app_uart_common_handler.h
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

#ifndef _TUYA_BLE_APP_UART_COMMON_HANDLER_H_
#define _TUYA_BLE_APP_UART_COMMON_HANDLER_H_

#include <stdint.h>
#include "tuya_ble_internal_config.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   Function for transmit ble data from peer devices to tuya sdk.
 *
 * @note    This function must be called from where the ble data is received.
 *.
 * */

void tuya_ble_uart_common_process(uint8_t *p_in_data, uint16_t in_len);

void tuya_ble_uart_common_mcu_ota_data_from_ble_handler(uint16_t cmd, uint8_t *recv_data, uint32_t recv_len);


#ifdef __cplusplus
}
#endif

#endif // _TUYA_BLE_APP_UART_COMMON_HANDLER_H_

