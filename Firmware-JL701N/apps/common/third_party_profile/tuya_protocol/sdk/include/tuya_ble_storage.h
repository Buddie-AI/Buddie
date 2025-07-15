/**
 * \file tuya_ble_storage.h
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

#ifndef TUYA_BLE_STORAGE_H_
#define TUYA_BLE_STORAGE_H_

#include "tuya_ble_type.h"
#include "tuya_ble_internal_config.h"

uint32_t tuya_ble_storage_save_sys_settings(void);

uint32_t tuya_ble_storage_save_auth_settings(void);

uint32_t tuya_ble_storage_init(void);


#if (TUYA_BLE_DEVICE_AUTH_DATA_STORE)

tuya_ble_status_t tuya_ble_storage_write_pid(tuya_ble_product_id_type_t pid_type, uint8_t pid_len, uint8_t *pid);

tuya_ble_status_t tuya_ble_storage_write_hid(uint8_t *hid, uint8_t len);

tuya_ble_status_t tuya_ble_storage_read_id_info(tuya_ble_factory_id_data_t *id);

tuya_ble_status_t tuya_ble_storage_write_auth_key_device_id_mac(uint8_t *auth_key, uint8_t auth_key_len, uint8_t *device_id, uint8_t device_id_len,
        uint8_t *mac, uint8_t mac_len, uint8_t *mac_string, uint8_t mac_string_len, uint8_t *pid, uint8_t pid_len);

#endif

#endif


