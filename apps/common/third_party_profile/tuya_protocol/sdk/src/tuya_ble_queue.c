/**
 * \file tuya_ble_queue.c
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

#include "tuya_ble_stdlib.h"
#include "tuya_ble_queue.h"
#include "tuya_ble_type.h"

tuya_ble_status_t tuya_ble_queue_init(tuya_ble_queue_t *q, void *buf, uint8_t queue_size, uint8_t elem_size)
{
    if (buf == NULL || q == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    q->buf = buf;
    q->size = queue_size;
    q->offset = elem_size;
    q->rd_ptr = 0;
    q->wr_ptr = 0;
    q->used = 0;

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_enqueue(tuya_ble_queue_t *q, void *in)
{
    if (q->used >= q->size) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy((uint8_t *)q->buf + q->wr_ptr * q->offset, in, q->offset);
    q->wr_ptr = (q->wr_ptr + 1) % q->size;
    q->used++;

    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_queue_get(tuya_ble_queue_t *q, void *out)
{
    if (q->used > 0) {
        memcpy(out, (uint8_t *)q->buf + q->rd_ptr * q->offset, q->offset);
        return TUYA_BLE_SUCCESS;
    } else {
        return TUYA_BLE_ERR_NOT_FOUND;
    }
}

tuya_ble_status_t tuya_ble_dequeue(tuya_ble_queue_t *q, void *out)
{
    if (q->used > 0) {
        memcpy(out, (uint8_t *)q->buf + q->rd_ptr * q->offset, q->offset);
        q->rd_ptr = (q->rd_ptr + 1) % q->size;
        q->used--;
        return TUYA_BLE_SUCCESS;
    } else {
        return TUYA_BLE_ERR_NOT_FOUND;
    }
}

void tuya_ble_queue_decrease(tuya_ble_queue_t *q)
{
    if (q->used > 0) {
        q->rd_ptr = (q->rd_ptr + 1) % q->size;
        q->used--;
    }
}


void tuya_ble_queue_flush(tuya_ble_queue_t *q)
{
    q->rd_ptr = 0;
    q->wr_ptr = 0;
    q->used = 0;
}

uint8_t tuya_ble_get_queue_used(tuya_ble_queue_t *q)
{
    return q->used;
}

