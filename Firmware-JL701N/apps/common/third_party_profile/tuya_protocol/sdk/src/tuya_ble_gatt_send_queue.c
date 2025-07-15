/**
 * \file tuya_ble_gatt_send_queue.c
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
#include "tuya_ble_gatt_send_queue.h"
#include "tuya_ble_type.h"
#include "tuya_ble_config.h"
#include "tuya_ble_port.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_main.h"
#include "tuya_ble_log.h"

static tuya_ble_queue_t gatt_send_queue;
static tuya_ble_gatt_send_data_t send_buf[TUYA_BLE_GATT_SEND_DATA_QUEUE_SIZE];

static volatile uint8_t gatt_queue_flag = 0;

void tuya_ble_gatt_send_queue_init(void)
{
    gatt_queue_flag = 0;
    tuya_ble_queue_init(&gatt_send_queue, (void *) send_buf, TUYA_BLE_GATT_SEND_DATA_QUEUE_SIZE, sizeof(tuya_ble_gatt_send_data_t));
}

static void tuya_ble_gatt_send_queue_free(void)
{
    tuya_ble_gatt_send_data_t data   = {0};
    memset(&data, 0, sizeof(tuya_ble_gatt_send_data_t));
    while (tuya_ble_dequeue(&gatt_send_queue, &data) == TUYA_BLE_SUCCESS) {
        if (data.buf) {
            tuya_ble_free(data.buf);
        }
        memset(&data, 0, sizeof(tuya_ble_gatt_send_data_t));
    }
    TUYA_BLE_LOG_DEBUG("tuya_ble_gatt_send_queue_free execute.");
}

void tuya_ble_gatt_send_data_handle(void *evt)
{
    tuya_ble_gatt_send_data_t data   = {0};
    tuya_ble_evt_param_t event;
    tuya_ble_connect_status_t currnet_connect_status;

    while (tuya_ble_queue_get(&gatt_send_queue, &data) == TUYA_BLE_SUCCESS) {
        currnet_connect_status = tuya_ble_connect_status_get();
        if ((currnet_connect_status == BONDING_UNCONN) || (currnet_connect_status == UNBONDING_UNCONN)) {
            tuya_ble_gatt_send_queue_free();
            break;
        }

        if (tuya_ble_gatt_send_data(data.buf, data.size) == TUYA_BLE_SUCCESS) {
            tuya_ble_free(data.buf);
            tuya_ble_queue_decrease(&gatt_send_queue);
        } else {
            event.hdr.event = TUYA_BLE_EVT_GATT_SEND_DATA;
            event.hdr.event_handler = tuya_ble_gatt_send_data_handle;
            if (tuya_ble_event_send(&event) != 0) {
                tuya_ble_gatt_send_queue_free();
                TUYA_BLE_LOG_ERROR("TUYA_BLE_EVT_GATT_SEND_DATA  error.");
            }

            break;

        }
    }
    if (tuya_ble_get_queue_used(&gatt_send_queue) == 0) {
        tuya_ble_queue_flush(&gatt_send_queue);
        gatt_queue_flag = 0;
    }

}



tuya_ble_status_t tuya_ble_gatt_send_data_enqueue(uint8_t *p_data, uint8_t data_len)
{
    tuya_ble_gatt_send_data_t data   = {0};

    data.buf = tuya_ble_malloc(data_len);

    if (data.buf) {
        memcpy(data.buf, p_data, data_len);
        data.size = data_len;
        if (tuya_ble_enqueue(&gatt_send_queue, &data) == TUYA_BLE_SUCCESS) {
            if (gatt_queue_flag == 0) {
                gatt_queue_flag = 1;
                tuya_ble_gatt_send_data_handle(NULL);
            }
            return TUYA_BLE_SUCCESS;
        } else {
            tuya_ble_free(data.buf);
            return TUYA_BLE_ERR_NO_MEM;
        }
    } else {
        return TUYA_BLE_ERR_NO_MEM;
    }

}

uint32_t tuya_ble_get_gatt_send_queue_used(void)
{
    return tuya_ble_get_queue_used(&gatt_send_queue);
}


