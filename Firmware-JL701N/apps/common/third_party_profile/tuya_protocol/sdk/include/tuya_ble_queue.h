/**
 * \file tuya_ble_queue.h
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

#ifndef TUYA_BLE_QUEUE_H_
#define TUYA_BLE_QUEUE_H_

#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *buf;
    volatile uint8_t size;
    volatile uint8_t offset;
    volatile uint8_t rd_ptr;
    volatile uint8_t wr_ptr;
    volatile uint8_t used;
} tuya_ble_queue_t;

tuya_ble_status_t tuya_ble_queue_init(tuya_ble_queue_t *q, void *buf, uint8_t size, uint8_t elem_size);
tuya_ble_status_t tuya_ble_enqueue(tuya_ble_queue_t *q, void *in);
tuya_ble_status_t tuya_ble_queue_get(tuya_ble_queue_t *q, void *out);
tuya_ble_status_t tuya_ble_dequeue(tuya_ble_queue_t *q, void *out);
void tuya_ble_queue_decrease(tuya_ble_queue_t *q);
void tuya_ble_queue_flush(tuya_ble_queue_t *q);
uint8_t tuya_ble_get_queue_used(tuya_ble_queue_t *q);

#ifdef __cplusplus
}
#endif

#endif //TUYA_BLE_QUEUE_H_
