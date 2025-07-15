/**
 * \file tuya_ble_bulkdata.c
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

#include "tuya_ble_bulk_data.h"
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_unix_time.h"
#include "tuya_ble_log.h"
#include "tuya_ble_gatt_send_queue.h"


uint32_t tuya_ble_bulk_data_read_block_size_get(void)
{
    uint32_t mtu_length = 0;
    uint32_t read_block_size = 0;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        read_block_size = 0;
    } else {
        mtu_length = tuya_ble_send_packet_data_length_get();
        if (mtu_length < 100) {
            read_block_size = 256;
        } else if ((mtu_length >= 100) && (mtu_length < 150)) {
            read_block_size = 512;
        } else {
            read_block_size = TUYA_BLE_BULK_DATA_MAX_READ_BLOCK_SIZE;
        }
    }

    return read_block_size;
}


void tuya_ble_handle_bulk_data_req(uint16_t cmd, uint8_t *p_recv_data, uint32_t recv_data_len)
{
    uint16_t data_len;
    tuya_ble_cb_evt_param_t event;
    uint8_t err_code = 0;

    data_len = (p_recv_data[11] << 8) + p_recv_data[12];

    if (data_len == 0) {
        return;
    } else if (p_recv_data[13] != 0) { /*Currently only supports version 0*/
        return;
    } else {

    }

    event.evt = TUYA_BLE_CB_EVT_BULK_DATA;

    switch (cmd) {
    case FRM_BULK_DATA_READ_INFO_REQ:
        event.bulk_req_data.evt = TUYA_BLE_BULK_DATA_EVT_READ_INFO;
        event.bulk_req_data.bulk_type = p_recv_data[14];
        break;

    case FRM_BULK_DATA_READ_DATA_REQ:
        event.bulk_req_data.evt = TUYA_BLE_BULK_DATA_EVT_READ_BLOCK;
        event.bulk_req_data.bulk_type = p_recv_data[14];
        event.bulk_req_data.params.block_data_req_data.block_number = (p_recv_data[15] << 8) + p_recv_data[16];
        break;

    case FRM_BULK_DATA_ERASE_DATA_REQ:
        event.bulk_req_data.evt = TUYA_BLE_BULK_DATA_EVT_ERASE;
        event.bulk_req_data.bulk_type = p_recv_data[14];
        break;

    default:
        err_code = 1;
        break;
    }

    if (err_code == 0) {
        if (tuya_ble_cb_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya_ble_handle_bulk_data_req-tuya ble send cb event failed.");
        }
    }
}


tuya_ble_status_t tuya_ble_bulk_data_response(tuya_ble_bulk_data_response_t *p_data)
{
    tuya_ble_evt_param_t evt;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((p_data->evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) && (p_data->params.send_res_data.current_block_length > (TUYA_BLE_SEND_MAX_DATA_LEN - 8))) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    evt.hdr.event = TUYA_BLE_EVT_BULK_DATA_RESPONSE;

    memcpy(&evt.bulk_res_data, p_data, sizeof(tuya_ble_bulk_data_response_t));

    if (p_data->evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) {
        evt.bulk_res_data.params.send_res_data.p_current_block_data = NULL;
        evt.bulk_res_data.params.send_res_data.p_current_block_data = (uint8_t *)tuya_ble_malloc(p_data->params.send_res_data.current_block_length);
        if (evt.bulk_res_data.params.send_res_data.p_current_block_data == NULL) {
            return TUYA_BLE_ERR_NO_MEM;
        }
        memcpy(evt.bulk_res_data.params.send_res_data.p_current_block_data, p_data->params.send_res_data.p_current_block_data, p_data->params.send_res_data.current_block_length);
    }

    if (tuya_ble_event_send(&evt) != 0) {
        if (p_data->evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) {
            tuya_ble_free(evt.bulk_res_data.params.send_res_data.p_current_block_data);
        }

        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}


void tuya_ble_handle_bulk_data_evt(tuya_ble_evt_param_t *evt)
{
    uint16_t bulk_data_cmd = 0;
    uint8_t buffer[14];
    uint8_t *p_buf = NULL;
    uint16_t data_length = 0;
    uint8_t encry_mode = 0;
    tuya_ble_cb_evt_param_t event;
    uint8_t err_code = 0;


    if (tuya_ble_pair_rand_valid_get() == 1) {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    } else {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    buffer[0] = 0;
    buffer[1] = evt->bulk_res_data.bulk_type;

    switch (evt->bulk_res_data.evt) {
    case TUYA_BLE_BULK_DATA_EVT_READ_INFO:

        bulk_data_cmd = FRM_BULK_DATA_READ_INFO_RESP;
        buffer[2] = evt->bulk_res_data.params.bulk_info_res_data.status;
        buffer[3] = evt->bulk_res_data.params.bulk_info_res_data.flag;
        buffer[4] = evt->bulk_res_data.params.bulk_info_res_data.bulk_data_length >> 24;
        buffer[5] = evt->bulk_res_data.params.bulk_info_res_data.bulk_data_length >> 16;
        buffer[6] = evt->bulk_res_data.params.bulk_info_res_data.bulk_data_length >> 8;
        buffer[7] = evt->bulk_res_data.params.bulk_info_res_data.bulk_data_length;
        buffer[8] = evt->bulk_res_data.params.bulk_info_res_data.bulk_data_crc >> 24;
        buffer[9] = evt->bulk_res_data.params.bulk_info_res_data.bulk_data_crc >> 16;
        buffer[10] = evt->bulk_res_data.params.bulk_info_res_data.bulk_data_crc >> 8;
        buffer[11] = evt->bulk_res_data.params.bulk_info_res_data.bulk_data_crc;
        buffer[12] = evt->bulk_res_data.params.bulk_info_res_data.block_data_length >> 8;
        buffer[13] = evt->bulk_res_data.params.bulk_info_res_data.block_data_length;
        data_length = 14;
        break;

    case TUYA_BLE_BULK_DATA_EVT_READ_BLOCK:

        bulk_data_cmd = FRM_BULK_DATA_READ_DATA_RESP;
        buffer[2] = evt->bulk_res_data.params.block_res_data.status;
        buffer[3] = evt->bulk_res_data.params.block_res_data.block_number >> 8;
        buffer[4] = evt->bulk_res_data.params.block_res_data.block_number;
        buffer[5] = evt->bulk_res_data.params.block_res_data.block_data_length >> 8;
        buffer[6] = evt->bulk_res_data.params.block_res_data.block_data_length;
        buffer[7] = evt->bulk_res_data.params.block_res_data.max_packet_data_length >> 8;
        buffer[8] = evt->bulk_res_data.params.block_res_data.max_packet_data_length;
        buffer[9] = evt->bulk_res_data.params.block_res_data.block_data_crc16 >> 8;
        buffer[10] = evt->bulk_res_data.params.block_res_data.block_data_crc16;
        data_length = 11;
        break;

    case TUYA_BLE_BULK_DATA_EVT_SEND_DATA :

        bulk_data_cmd = FRM_BULK_DATA_SEND_DATA;
        p_buf = (uint8_t *)tuya_ble_malloc(evt->bulk_res_data.params.send_res_data.current_block_length + 8);
        if (p_buf) {
            p_buf[0] = 0;
            p_buf[1] = evt->bulk_res_data.bulk_type;
            p_buf[2] = evt->bulk_res_data.params.send_res_data.current_block_number >> 8;
            p_buf[3] = evt->bulk_res_data.params.send_res_data.current_block_number;
            p_buf[4] = 0;
            p_buf[5] = 0;
            p_buf[6] = evt->bulk_res_data.params.send_res_data.current_block_length >> 8;
            p_buf[7] = evt->bulk_res_data.params.send_res_data.current_block_length;
            memcpy(&p_buf[8], evt->bulk_res_data.params.send_res_data.p_current_block_data, evt->bulk_res_data.params.send_res_data.current_block_length);
            data_length = evt->bulk_res_data.params.send_res_data.current_block_length + 8;
        }
        tuya_ble_free(evt->bulk_res_data.params.send_res_data.p_current_block_data);
        break;

    case TUYA_BLE_BULK_DATA_EVT_ERASE :

        bulk_data_cmd = FRM_BULK_DATA_ERASE_DATA_RESP;
        buffer[2] = evt->bulk_res_data.params.erase_res_data.status;
        data_length = 3;

        break;

    default:
        break;
    }

    if (data_length > 0) {
        if (evt->bulk_res_data.evt == TUYA_BLE_BULK_DATA_EVT_READ_BLOCK) {
            err_code = tuya_ble_commData_send(bulk_data_cmd, 0, buffer, data_length, encry_mode);

            if (evt->bulk_res_data.params.block_res_data.status == 0) {
                event.evt = TUYA_BLE_CB_EVT_BULK_DATA;

                event.bulk_req_data.evt = TUYA_BLE_BULK_DATA_EVT_SEND_DATA;
                event.bulk_req_data.bulk_type = evt->bulk_res_data.bulk_type;

                event.bulk_req_data.params.send_data_req_data.block_number = evt->bulk_res_data.params.block_res_data.block_number;

                if (!err_code) {
                    if (tuya_ble_cb_event_send(&event) != 0) {
                        TUYA_BLE_LOG_ERROR("tuya_ble_handle_bulk_data_evt-tuya ble send cb event failed.");
                    }
                } else {
                    TUYA_BLE_LOG_ERROR("tuya_ble_handle_bulk_data_evt-response read bulk data error.");
                }
            }

        } else if (evt->bulk_res_data.evt == TUYA_BLE_BULK_DATA_EVT_SEND_DATA) {
            err_code = tuya_ble_commData_send(bulk_data_cmd, 0, p_buf, data_length, encry_mode);
            tuya_ble_free(p_buf);
        } else {
            tuya_ble_commData_send(bulk_data_cmd, 0, buffer, data_length, encry_mode);
        }
    }

}












