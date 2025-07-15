/**
 * \file tuya_ble_gatt_send_queue.h
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

#ifndef TUYA_BLE_GATT_SEND_QUEUE_H_
#define TUYA_BLE_GATT_SEND_QUEUE_H_

#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buf;
    uint8_t size;
} tuya_ble_gatt_send_data_t;

void tuya_ble_gatt_send_queue_init(void);
void tuya_ble_gatt_send_data_handle(void *evt);
tuya_ble_status_t tuya_ble_gatt_send_data_enqueue(uint8_t *p_data, uint8_t data_len);

#ifdef __cplusplus
}
#endif

#endif //TUYA_BLE_GATT_SEND_QUEUE_H_
