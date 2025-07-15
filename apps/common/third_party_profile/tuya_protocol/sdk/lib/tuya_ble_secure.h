/**
 * \file tuya_ble_secure.h
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

#ifndef TUYA_BLE_SECURE_H_
#define TUYA_BLE_SECURE_H_

#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif


enum {
    ENCRYPTION_MODE_NONE,
    ENCRYPTION_MODE_KEY_1,
    ENCRYPTION_MODE_KEY_2,
    ENCRYPTION_MODE_KEY_3,
    ENCRYPTION_MODE_KEY_4,
    ENCRYPTION_MODE_SESSION_KEY,
    ENCRYPTION_MODE_ECDH_KEY,
    ENCRYPTION_MODE_FTM_KEY,
    ENCRYPTION_MODE_MAX,
};

bool tuya_ble_register_key_generate(uint8_t *output, tuya_ble_parameters_settings_t *current_para);

uint8_t tuya_ble_encryption(uint16_t protocol_version, uint8_t encryption_mode, uint8_t *iv, uint8_t *in_buf, uint32_t in_len, uint32_t *out_len, uint8_t *out_buf, tuya_ble_parameters_settings_t *current_para_data, uint8_t *dev_rand);

uint8_t  tuya_ble_encrypt_old_with_key(uint8_t *key, uint8_t *in_buf, uint8_t in_len, uint8_t *out_buf);

bool tuya_ble_device_id_encrypt(uint8_t *key_in, uint16_t key_len, uint8_t *input, uint16_t input_len, uint8_t *output);

bool tuya_ble_device_id_decrypt(uint8_t *key_in, uint16_t key_len, uint8_t *input, uint16_t input_len, uint8_t *output);

bool tuya_ble_device_id_encrypt_v4(uint8_t *key_in, uint16_t key_len, uint8_t *input, uint16_t input_len, uint8_t *output);

uint8_t tuya_ble_decryption(uint16_t protocol_version, uint8_t const *in_buf, uint32_t in_len, uint32_t *out_len, uint8_t *out_buf, tuya_ble_parameters_settings_t *current_para_data, uint8_t *dev_rand);

bool tuya_ble_server_cert_data_verify(const uint8_t *p_data, uint16_t data_len, const uint8_t *p_sig);

bool tuya_ble_sig_data_verify(const uint8_t *p_pk, const uint8_t *p_data, uint16_t data_len, const uint8_t *p_sig);

void tuya_ble_event_process(tuya_ble_evt_param_t *tuya_ble_evt);


#ifdef __cplusplus
}
#endif

#endif // TUYA_BLE_SECURE_H_


