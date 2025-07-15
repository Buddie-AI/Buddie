/**
 * \file tuya_ble_app_uart_common_handler.c
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
#include "tuya_ble_app_uart_common_handler.h"
#include "tuya_ble_log.h"


#define TUYA_BLE_OTA_MCU_TEST  0

#define TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX  200

#define TUYA_BLE_OTA_MCU_TYPE 1


#define TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST			    0xEA
#define TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO			    0xEB
#define TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET	        0xEC
#define TUYA_BLE_UART_COMMON_MCU_OTA_DATA 			        0xED
#define TUYA_BLE_UART_COMMON_MCU_OTA_END			        0xEE


#if (!TUYA_BLE_OTA_MCU_TEST)


void tuya_ble_uart_common_mcu_ota_data_from_ble_handler(uint16_t cmd, uint8_t *recv_data, uint32_t recv_len)
{
    static uint8_t uart_data_temp[42];
    uint8_t *uart_data_buffer = NULL;
    uint8_t uart_data_len = 0;

    if (cmd == FRM_OTA_DATA_REQ) {
        uart_data_buffer = (uint8_t *)tuya_ble_malloc(recv_len + 7);
        if (uart_data_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("uart_data_buffer malloc failed.");
            return;
        }

    } else {
        uart_data_buffer = uart_data_temp;
    }

    uart_data_buffer[0] = 0x55;
    uart_data_buffer[1] = 0xAA;
    uart_data_buffer[2] = 0x00;
    switch (cmd) {
    case FRM_OTA_START_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 2;
        uart_data_buffer[6] = TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX >> 8;
        uart_data_buffer[7] = (uint8_t)TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX;
        uart_data_len = 8;
        break;
    case FRM_OTA_FILE_INFOR_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 35;
        memcpy(uart_data_buffer + 6, recv_data, 8);
        uart_data_buffer[14] = recv_data[9];
        uart_data_buffer[15] = recv_data[10];
        uart_data_buffer[16] = recv_data[11];
        memcpy(&uart_data_buffer[17], recv_data + 12, 24);
        uart_data_len = 41;
        break;
    case FRM_OTA_FILE_OFFSET_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 4;
        memcpy(uart_data_buffer + 6, recv_data, 4);
        uart_data_len = 10;
        break;
    case FRM_OTA_DATA_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_DATA;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = recv_len;
        memcpy(uart_data_buffer + 6, recv_data, recv_len);
        uart_data_len = 6 + recv_len;
        break;
    case FRM_OTA_END_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_END;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 0;
        uart_data_len = 6;
        break;
    default:
        break;
    }

    uart_data_buffer[uart_data_len] = tuya_ble_check_sum(uart_data_buffer, uart_data_len);

    tuya_ble_common_uart_send_data(uart_data_buffer, uart_data_len + 1);

    TUYA_BLE_LOG_HEXDUMP_DEBUG("mcu ota uart send data : ", uart_data_buffer, uart_data_len + 1);

    if (cmd == FRM_OTA_DATA_REQ) {
        tuya_ble_free(uart_data_buffer);
    }

}



static void tuya_ble_uart_common_mcu_ota_data_from_uart_handler(uint8_t cmd, uint8_t *data_buffer, uint16_t data_len)
{
    static uint8_t ble_data_buffer[30];
    static uint8_t ble_data_len = 0;
    uint16_t ble_cmd = 0;
    tuya_ble_connect_status_t currnet_connect_status;

    memset(ble_data_buffer, 0, sizeof(ble_data_buffer));
    ble_data_len = 0;
    switch (cmd) {
    case TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST:
        ble_data_buffer[0] = data_buffer[0];
        ble_data_buffer[1] = 3;
        ble_data_buffer[2] = TUYA_BLE_OTA_MCU_TYPE;
        ble_data_buffer[3] = 0;
        memcpy(&ble_data_buffer[4], data_buffer + 1, 5);
        ble_data_len = 9;
        ble_cmd = FRM_OTA_START_RESP;
        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO:
        ble_data_buffer[0] = TUYA_BLE_OTA_MCU_TYPE;
        memcpy(&ble_data_buffer[1], data_buffer, 25);
        ble_data_len = 26;
        ble_cmd = FRM_OTA_FILE_INFOR_RESP;
        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET:
        ble_data_buffer[0] = TUYA_BLE_OTA_MCU_TYPE;
        memcpy(&ble_data_buffer[1], data_buffer, 4);
        ble_data_len = 5;
        ble_cmd = FRM_OTA_FILE_OFFSET_RESP;
        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_DATA:
        ble_data_buffer[0] = TUYA_BLE_OTA_MCU_TYPE;
        ble_data_buffer[1] = data_buffer[0];
        ble_data_len = 2;
        ble_cmd = FRM_OTA_DATA_RESP;
        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_END:
        ble_data_buffer[0] = TUYA_BLE_OTA_MCU_TYPE;
        ble_data_buffer[1] = data_buffer[0];
        ble_data_len = 2;
        ble_cmd = FRM_OTA_END_RESP;
        break;
    default:
        break;
    };

    currnet_connect_status = tuya_ble_connect_status_get();

    if (currnet_connect_status != BONDING_CONN) {
        TUYA_BLE_LOG_ERROR("tuya_ble_uart_common_mcu_ota_process FAILED.");
        return;
    }

    if (ble_data_len > 0) {
        tuya_ble_commData_send(ble_cmd, 0, ble_data_buffer, ble_data_len, ENCRYPTION_MODE_SESSION_KEY);
    }

}


__TUYA_BLE_WEAK void tuya_ble_custom_app_uart_common_process(uint8_t *p_in_data, uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4] << 8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data + 6;

    switch (cmd) {

    default:
        break;
    };

}



void tuya_ble_uart_common_process(uint8_t *p_in_data, uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4] << 8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data + 6;

    switch (cmd) {
    case TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET:
    case TUYA_BLE_UART_COMMON_MCU_OTA_DATA:
    case TUYA_BLE_UART_COMMON_MCU_OTA_END:
        tuya_ble_uart_common_mcu_ota_data_from_uart_handler(cmd, data_buffer, data_len);
        break;
    default:
        tuya_ble_custom_app_uart_common_process(p_in_data, in_len);
        break;
    };

}


#else


tuya_ble_timer_t tuya_ble_xtimer_heartbeat ;

static uint32_t tuya_ble_heartbeat_timeout_ms = 1000;

static uint8_t  heartbeat_uart_buffer[10];

static uint8_t   mcu_info_get = 0;


static void tuya_ble_common_uart_protocol_send(uint8_t cmd, uint8_t *pdata, uint8_t len)
{
    static uint8_t alloc_buf[256];

    uint8_t i = 0;
    alloc_buf[0 + 1] = 0x55;
    alloc_buf[1 + 1] = 0xaa;
    alloc_buf[2 + 1] = 0x00;
    alloc_buf[3 + 1] = cmd;
    alloc_buf[4 + 1] = 0;
    alloc_buf[5 + 1] = len;

    memcpy(alloc_buf + 6 + 1, pdata, len);
    alloc_buf[7 + len] = tuya_ble_check_sum(alloc_buf + 1, 6 + len);

    tuya_ble_common_uart_send_data(alloc_buf + 1, 7 + len);
}


static void tuya_ble_vtimer_heartbeat_callback(tuya_ble_timer_t timer)
{
    heartbeat_uart_buffer[0] = 0x55;
    heartbeat_uart_buffer[1] = 0xAA;
    heartbeat_uart_buffer[2] = 0x00;
    heartbeat_uart_buffer[3] = 0x00;
    heartbeat_uart_buffer[4] = 0x00;
    heartbeat_uart_buffer[5] = 0x00;
    heartbeat_uart_buffer[6] = 0xFF;

    tuya_ble_common_uart_send_data(heartbeat_uart_buffer, 7);

}


static void tuya_ble_heartbeat_timer_create(void)
{
    if (tuya_ble_timer_create(&tuya_ble_xtimer_heartbeat, tuya_ble_heartbeat_timeout_ms, TUYA_BLE_TIMER_REPEATED, tuya_ble_vtimer_heartbeat_callback) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_heartbeat creat failed");
    }

}


static void tuya_ble_heartbeat_timer_start(void)
{
    if (tuya_ble_timer_start(tuya_ble_xtimer_heartbeat) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_heartbeat start failed");
    }

}

static void tuya_ble_heartbeat_timer_restart(uint32_t ms)
{
    if (tuya_ble_timer_restart(tuya_ble_xtimer_heartbeat, ms) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_heartbeat restart failed");
    }

}


static void tuya_ble_heartbeat_timer_stop(void)
{

    if (tuya_ble_timer_stop(tuya_ble_xtimer_heartbeat) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_heartbeat stop failed");
    }

}


static void tuya_ble_heartbeat_timer_delete(void)
{

    if (tuya_ble_timer_delete(tuya_ble_xtimer_heartbeat) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_heartbeat delete failed");
    }

}




static void  tuya_ble_ota_data_process_test(uint8_t *p_data, uint16_t p_data_len);

void tuya_ble_uart_common_mcu_ota_data_from_ble_handler(uint16_t cmd, uint8_t *recv_data, uint32_t recv_len)
{
    static uint8_t uart_data_temp[42];
    uint8_t uart_cmd = 0xFF;
    uint8_t *uart_data_buffer = NULL;
    uint8_t uart_data_len = 0;

    if (cmd == FRM_OTA_DATA_REQ) {
        uart_data_buffer = (uint8_t *)tuya_ble_malloc(recv_len + 7);
        if (uart_data_buffer == NULL) {
            TUYA_BLE_LOG_ERROR("uart_data_buffer malloc failed.\n");
            return;
        }

    } else {
        uart_data_buffer = uart_data_temp;
    }

    uart_data_buffer[0] = 0x55;
    uart_data_buffer[1] = 0xAA;
    uart_data_buffer[2] = 0x00;
    switch (cmd) {
    case FRM_OTA_START_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 2;
        uart_data_buffer[6] = TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX >> 8;
        uart_data_buffer[7] = (uint8_t)TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX;
        uart_data_len = 8;
        break;
    case FRM_OTA_FILE_INFOR_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 35;
        memcpy(uart_data_buffer + 6, recv_data, 8);
        uart_data_buffer[14] = recv_data[9];
        uart_data_buffer[15] = recv_data[10];
        uart_data_buffer[16] = recv_data[11];
        memcpy(&uart_data_buffer[17], recv_data + 12, 24);
        uart_data_len = 41;
        break;
    case FRM_OTA_FILE_OFFSET_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 4;
        memcpy(uart_data_buffer + 6, recv_data, 4);
        uart_data_len = 10;
        break;
    case FRM_OTA_DATA_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_DATA;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = recv_len;
        memcpy(uart_data_buffer + 6, recv_data, recv_len);
        uart_data_len = 6 + recv_len;
        break;
    case FRM_OTA_END_REQ:
        uart_data_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_END;
        uart_data_buffer[4] = 0;
        uart_data_buffer[5] = 0;
        uart_data_len = 6;
        break;
    default:
        break;
    }

    uart_data_buffer[uart_data_len] = tuya_ble_check_sum(uart_data_buffer, uart_data_len);

    tuya_ble_common_uart_send_data(uart_data_buffer, uart_data_len + 1);

    TUYA_BLE_LOG_HEXDUMP_DEBUG("mcu ota uart send data : ", uart_data_buffer, uart_data_len + 1);

    if (cmd == FRM_OTA_DATA_REQ) {
        tuya_ble_free(uart_data_buffer);
    }

}



static void tuya_ble_uart_common_mcu_ota_data_from_uart_handler(uint8_t cmd, uint8_t *data_buffer, uint16_t data_len)
{
    static uint8_t ble_data_buffer[30];
    static uint8_t ble_data_len = 0;
    uint16_t ble_cmd = 0;
    tuya_ble_connect_status_t currnet_connect_status;

    memset(ble_data_buffer, 0, sizeof(ble_data_buffer));
    ble_data_len = 0;
    switch (cmd) {
    case TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST:
        ble_data_buffer[0] = data_buffer[0];
        ble_data_buffer[1] = 3;
        ble_data_buffer[2] = TUYA_BLE_OTA_MCU_TYPE;
        ble_data_buffer[3] = 0;
        memcpy(&ble_data_buffer[4], data_buffer + 1, 5);
        ble_data_len = 9;
        ble_cmd = FRM_OTA_START_RESP;
        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO:
        ble_data_buffer[0] = TUYA_BLE_OTA_MCU_TYPE;
        memcpy(&ble_data_buffer[1], data_buffer, 25);
        ble_data_len = 26;
        ble_cmd = FRM_OTA_FILE_INFOR_RESP;
        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET:
        ble_data_buffer[0] = TUYA_BLE_OTA_MCU_TYPE;
        memcpy(&ble_data_buffer[1], data_buffer, 4);
        ble_data_len = 5;
        ble_cmd = FRM_OTA_FILE_OFFSET_RESP;
        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_DATA:
        ble_data_buffer[0] = TUYA_BLE_OTA_MCU_TYPE;
        ble_data_buffer[1] = data_buffer[0];
        ble_data_len = 2;
        ble_cmd = FRM_OTA_DATA_RESP;
        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_END:
        ble_data_buffer[0] = TUYA_BLE_OTA_MCU_TYPE;
        ble_data_buffer[1] = data_buffer[0];
        ble_data_len = 2;
        ble_cmd = FRM_OTA_END_RESP;
        break;
    default:
        break;
    };

    currnet_connect_status = tuya_ble_connect_status_get();

    if (currnet_connect_status != BONDING_CONN) {
        TUYA_BLE_LOG_ERROR("tuya_ble_uart_common_mcu_ota_process FAILED.");
        return;
    }

    if (ble_data_len > 0) {
        tuya_ble_commData_send(ble_cmd, 0, ble_data_buffer, ble_data_len, ENCRYPTION_MODE_SESSION_KEY);
    }

}


void tuya_ble_uart_common_send_current_ble_state(void)
{
    uint8_t ble_work_state = 0;
    tuya_ble_connect_status_t ble_state;

    ble_state = tuya_ble_connect_status_get();

    if ((ble_state == UNBONDING_UNCONN) || (ble_state == UNBONDING_UNCONN) || (ble_state == UNBONDING_UNCONN)) {
        ble_work_state = 1;
    } else if ((ble_state == BONDING_UNCONN) || (ble_state == BONDING_UNAUTH_CONN)) {
        ble_work_state = 2;
    } else if (ble_state == BONDING_CONN) {
        ble_work_state = 3;
    } else {
        ble_work_state = 0;
    }

    tuya_ble_common_uart_protocol_send(TUYA_BLE_UART_COMMON_REPORT_WORK_STATE_TYPE, (uint8_t *)&ble_work_state, 1);

}


void tuya_ble_uart_common_process(uint8_t *p_in_data, uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4] << 8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data + 6;
    uint32_t mcu_firmware_version = 0, mcu_hardware_version = 0;
    switch (cmd) {
    case TUYA_BLE_UART_COMMON_HEART_MSG_TYPE:
        if ((mcu_info_get == 0) || (data_buffer[0] == 0)) {
            mcu_info_get = 1;
            tuya_ble_common_uart_protocol_send(TUYA_BLE_UART_COMMON_SEARCH_PID_TYPE, NULL, 0);
        }

        break;

    case TUYA_BLE_UART_COMMON_SEARCH_PID_TYPE:
        if (memcmp(tuya_ble_current_para.pid, data_buffer, 8) != 0) {
            TUYA_BLE_LOG_HEXDUMP_DEBUG("PID change to : ", data_buffer, 16);
            tuya_ble_device_update_product_id(TUYA_BLE_PRODUCT_ID_TYPE_PID, 8, data_buffer);

        }

        tuya_ble_heartbeat_timer_restart(10000);
        tuya_ble_common_uart_protocol_send(TUYA_BLE_UART_COMMON_CK_MCU_TYPE, NULL, 0);
        tuya_ble_common_uart_protocol_send(TUYA_BLE_UART_COMMON_QUERY_MCU_VERSION, NULL, 0);

        break;

    case TUYA_BLE_UART_COMMON_CK_MCU_TYPE:

        tuya_ble_uart_common_send_current_ble_state();

        break;

    case TUYA_BLE_UART_COMMON_QUERY_MCU_VERSION:
        mcu_firmware_version = (data_buffer[0] << 16) | (data_buffer[1] << 8) | (data_buffer[2]);
        mcu_hardware_version = (data_buffer[3] << 16) | (data_buffer[4] << 8) | (data_buffer[5]);
        TUYA_BLE_LOG_DEBUG("reveived mcu_firmware_version : 0x%04x mcu_hardware_version : 0x%04x", mcu_firmware_version, mcu_hardware_version);
        tuya_ble_device_update_mcu_version(mcu_firmware_version, mcu_hardware_version);

        break;

    case TUYA_BLE_UART_COMMON_MCU_SEND_VERSION:
        mcu_firmware_version = (data_buffer[0] << 16) | (data_buffer[1] << 8) | (data_buffer[2]);
        mcu_hardware_version = (data_buffer[3] << 16) | (data_buffer[4] << 8) | (data_buffer[5]);
        TUYA_BLE_LOG_DEBUG("reveived mcu_firmware_version : 0x%04x mcu_hardware_version : 0x%04x", mcu_firmware_version, mcu_hardware_version);
        tuya_ble_device_update_mcu_version(mcu_firmware_version, mcu_hardware_version);
        break;

    case TUYA_BLE_UART_COMMON_REPORT_WORK_STATE_TYPE:

        break;
    case TUYA_BLE_UART_COMMON_RESET_TYPE:

        break;
    case TUYA_BLE_UART_COMMON_SEND_CMD_TYPE:

        break;
    case TUYA_BLE_UART_COMMON_SEND_STATUS_TYPE:

        break;
    case TUYA_BLE_UART_COMMON_QUERY_STATUS:

        break;
    case TUYA_BLE_UART_COMMON_SEND_STORAGE_TYPE:

        break;
    case TUYA_BLE_UART_COMMON_SEND_TIME_SYNC_TYPE:

        break;
    case TUYA_BLE_UART_COMMON_MODIFY_ADV_INTERVAL:

        break;
    case TUYA_BLE_UART_COMMON_TURNOFF_SYSTEM_TIME:

        break;
    case TUYA_BLE_UART_COMMON_ENANBLE_LOWER_POWER:

        break;
    case TUYA_BLE_UART_COMMON_SEND_ONE_TIME_PASSWORD_TOKEN:

        break;
    case TUYA_BLE_UART_COMMON_ACTIVE_DISCONNECT:

        break;
    case TUYA_BLE_UART_COMMON_BLE_OTA_STATUS:

        break;

    case TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET:
    case TUYA_BLE_UART_COMMON_MCU_OTA_DATA:
    case TUYA_BLE_UART_COMMON_MCU_OTA_END:
        tuya_ble_uart_common_mcu_ota_data_from_uart_handler(cmd, data_buffer, data_len);
        break;
    default:
        break;
    };

}

static uint8_t mcu_ota_flag = 0;

#define MCU_OTA_MAX_DATA_SIZE 200

#define CURRENT_MCU_FIRMWARE_VERSION 0x010000

#define SUPPORT_MCU_OTA_FILE_MAX_LENGTH  102400

typedef struct {
    uint32_t file_version;
    uint8_t  file_md5[16];
    uint32_t file_length;
    uint32_t file_crc32;
} mcu_ota_file_info_data_t;

static mcu_ota_file_info_data_t mcu_ota_file_data = {0};

static uint16_t current_package = 0;
static uint16_t last_package = 0;

static uint32_t current_image_crc_last;
static uint32_t current_image_write_offset;

static void mcu_ota_current_para_init(void)
{
    current_package = 0;
    last_package = 0;
    mcu_ota_flag = 0;
    current_image_crc_last = 0;
    current_image_write_offset = 0;
}

void mcu_ota_test_init(void)
{
    mcu_ota_current_para_init();
    tuya_ble_heartbeat_timer_create();
    tuya_ble_heartbeat_timer_start();
}

void mcu_ota_test_para_init_connect(void)
{
    mcu_ota_current_para_init();
}


static void  tuya_ble_ota_data_process_test(uint8_t *p_data, uint16_t p_data_len)
{
    uint16_t len = 0;
    uint16_t crc16_rev, crc16_temp;
    static uint8_t ota_data_response_buffer[40];
    uint8_t ota_data_response_len;
    uint8_t cmd = p_data[3];
    uint16_t ota_data_len = (p_data[4] << 8) + p_data[5];
    uint8_t *ota_data_buffer = p_data + 6;
    memset(ota_data_response_buffer, 0, sizeof(ota_data_response_buffer));
    ota_data_response_buffer[0] = 0x55;
    ota_data_response_buffer[1] = 0xaa;
    ota_data_response_buffer[2] = 0x00;
    switch (cmd) {
    case TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST:
        ota_data_response_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST;
        ota_data_response_buffer[4] = 0;
        ota_data_response_buffer[5] = 6;
        if (mcu_ota_flag == 0) {
            mcu_ota_current_para_init();
            ota_data_response_buffer[6] = 0;
            ota_data_response_buffer[7] = ((uint32_t)CURRENT_MCU_FIRMWARE_VERSION >> 16);
            ota_data_response_buffer[8] = ((uint32_t)CURRENT_MCU_FIRMWARE_VERSION >> 8);
            ota_data_response_buffer[9] = ((uint32_t)CURRENT_MCU_FIRMWARE_VERSION);
            ota_data_response_buffer[10] = ((uint16_t)MCU_OTA_MAX_DATA_SIZE >> 8);
            ota_data_response_buffer[11] = ((uint16_t)MCU_OTA_MAX_DATA_SIZE);
            mcu_ota_flag = 1;
            memset(&mcu_ota_file_data, 0, sizeof(mcu_ota_file_data));
        } else {
            ota_data_response_buffer[6] = 1;
            mcu_ota_flag = 0;
        }
        ota_data_response_len = 12;

        break;

    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO:
        ota_data_response_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO;
        ota_data_response_buffer[4] = 0;
        ota_data_response_buffer[5] = 25;
        if (mcu_ota_flag != 1) {
            ota_data_response_buffer[6] = 4;
            mcu_ota_flag = 0;
        } else {
            if (memcmp(ota_data_buffer, tuya_ble_current_para.pid, 8)) {
                ota_data_response_buffer[6] = 1;
                mcu_ota_flag = 0;
            } else {
                mcu_ota_file_data.file_version = (ota_data_buffer[8] << 16) + (ota_data_buffer[9] << 8) + ota_data_buffer[10];
                memcpy(mcu_ota_file_data.file_md5, ota_data_buffer + 11, 16);
                mcu_ota_file_data.file_length = (ota_data_buffer[27] << 24) + (ota_data_buffer[28] << 16) + (ota_data_buffer[29] << 8) + ota_data_buffer[30];
                mcu_ota_file_data.file_crc32 = (ota_data_buffer[31] << 24) + (ota_data_buffer[32] << 16) + (ota_data_buffer[33] << 8) + ota_data_buffer[34];

                if (mcu_ota_file_data.file_version <= CURRENT_MCU_FIRMWARE_VERSION) {
                    ota_data_response_buffer[6] = 2;
                    mcu_ota_flag = 0;
                } else if (mcu_ota_file_data.file_length > SUPPORT_MCU_OTA_FILE_MAX_LENGTH) {
                    ota_data_response_buffer[6] = 3;
                    mcu_ota_flag = 0;
                } else {
                    ota_data_response_buffer[6] = 0;
                    mcu_ota_flag = 2;

                }


            }
        }
        ota_data_response_len = 31;

        break;
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET:
        ota_data_response_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET;
        ota_data_response_buffer[4] = 0;
        ota_data_response_buffer[5] = 4;
        if (mcu_ota_flag == 2) {
            memset(&ota_data_response_buffer[6], 0, 4);
            mcu_ota_flag = 3;
            ota_data_response_len = 10;
        } else {
            mcu_ota_flag = 0;
            ota_data_response_len = 0;
        }

        break;

    case TUYA_BLE_UART_COMMON_MCU_OTA_DATA:
        ota_data_response_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_DATA;
        ota_data_response_buffer[4] = 0;
        ota_data_response_buffer[5] = 1;

        if ((mcu_ota_flag == 3) || (mcu_ota_flag == 4)) {
            current_package = (ota_data_buffer[0] << 8) | ota_data_buffer[1];
            len = (ota_data_buffer[2] << 8) | ota_data_buffer[3];

            if ((current_package != (last_package + 1)) && (current_package != 0)) {
                TUYA_BLE_LOG_ERROR("mcu ota received package number error.received package number : %d", current_package);
                ota_data_response_buffer[6] = 1;
            } else if ((len > MCU_OTA_MAX_DATA_SIZE) || ((len + 6) != (p_data_len - 7))) {
                TUYA_BLE_LOG_ERROR("mcu ota received package data length error : len = %d,p_data_len = %d", len, p_data_len);
                ota_data_response_buffer[6] = 2;
            } else {
                crc16_temp = (ota_data_buffer[4] << 8) | ota_data_buffer[5];
                crc16_rev = tuya_ble_crc16_compute(&ota_data_buffer[6], len, NULL);
                if (crc16_temp != crc16_rev) {
                    TUYA_BLE_LOG_ERROR("mcu ota received package data crc16 error,crc_16_cal = 0x%04x ; crc16_rev = 0x%04x", crc16_temp, crc16_rev);
                    ota_data_response_buffer[6] = 3;
                }

            }


            if (ota_data_response_buffer[6] == 0) {
                current_image_crc_last = tuya_ble_crc32_compute(&ota_data_buffer[6], len, &current_image_crc_last);
                current_image_write_offset    += len;
                mcu_ota_flag = 4;
                last_package = current_package;
            } else {
                mcu_ota_flag = 0;
                mcu_ota_current_para_init();
            }
            ota_data_response_len = 7;

        } else {
            mcu_ota_flag = 0;
            ota_data_response_len = 0;
        }

        break;

    case TUYA_BLE_UART_COMMON_MCU_OTA_END:
        ota_data_response_buffer[3] = TUYA_BLE_UART_COMMON_MCU_OTA_END;
        ota_data_response_buffer[4] = 0;
        ota_data_response_buffer[5] = 1;
        if (mcu_ota_flag != 0) {
            if (mcu_ota_file_data.file_length != current_image_write_offset) {
                ota_data_response_buffer[6] = 1;
            } else if (mcu_ota_file_data.file_crc32 != current_image_crc_last) {
                ota_data_response_buffer[6] = 2;
            } else {
                ota_data_response_buffer[6] = 0;
                mcu_ota_flag = 0;
                TUYA_BLE_LOG_DEBUG("mcu ota test succeed.");
            }
            ota_data_response_len = 7;
        } else {
            mcu_ota_flag = 0;
            ota_data_response_len = 0;
        }


        break;

    default:

        break;
    };

    if (ota_data_response_len > 0) {
        ota_data_response_buffer[ota_data_response_len] = tuya_ble_check_sum(ota_data_response_buffer, ota_data_response_len);
        ota_data_response_len++;
        tuya_ble_common_uart_send_full_instruction_received(ota_data_response_buffer, ota_data_response_len);
    }

}

#endif





