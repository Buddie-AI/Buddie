/**
 * \file tuya_ble_api.c
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
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_type.h"
#include "tuya_ble_main.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_log.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_gatt_send_queue.h"
#include "tuya_ble_event.h"
#include "tuya_ble_app_production_test.h"

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)
#include "cJSON.h"
#endif

static uint8_t m_callback_numbers = 0;

#if TUYA_BLE_USE_OS
extern uint32_t m_cb_queue_table[TUYA_BLE_MAX_CALLBACKS];

/**
 * @brief   Function for registe queue to receive call back evt when use os
 *
 * @note
 *
 * */
tuya_ble_status_t tuya_ble_callback_queue_register(void *cb_queue)
{
    tuya_ble_status_t ret;

    tuya_ble_device_enter_critical();
    if (m_callback_numbers == TUYA_BLE_MAX_CALLBACKS) {
        ret = TUYA_BLE_ERR_RESOURCES;
    } else {
        m_cb_queue_table[m_callback_numbers] = (uint32_t)cb_queue;
        m_callback_numbers++;

        ret = TUYA_BLE_SUCCESS;
    }
    tuya_ble_device_exit_critical();

    return ret;
}


/**
 * @brief   Function for response the event.
 *
 * @note    if use os,must be sure to call this function after process one event in queue.
 *
 * */
tuya_ble_status_t tuya_ble_event_response(tuya_ble_cb_evt_param_t *param)
{
    return tuya_ble_inter_event_response(param);
}

#else


extern tuya_ble_callback_t m_cb_table[TUYA_BLE_MAX_CALLBACKS];
/**
 * @brief   Function for registe call back functions.
 *
 * @note    appliction should receive the message from the call back registed by this function.
 *
 * */
tuya_ble_status_t tuya_ble_callback_queue_register(tuya_ble_callback_t cb)
{
    tuya_ble_status_t ret;

    tuya_ble_device_enter_critical();
    if (m_callback_numbers == TUYA_BLE_MAX_CALLBACKS) {
        ret = TUYA_BLE_ERR_RESOURCES;
    } else {
        m_cb_table[m_callback_numbers] = cb;
        m_callback_numbers++;

        ret = TUYA_BLE_SUCCESS;
    }
    tuya_ble_device_exit_critical();

    return ret;
}

/**
 * @brief   Function for get scheduler queue size.
 *
 * @note    If it returns 0, it means that the queue has not been initialized.
 *
 * */
uint16_t tuya_ble_scheduler_queue_size_get(void)
{
    return tuya_ble_sched_queue_size_get();
}

/**
 * @brief   Function for get queue free space.
 *
 * @note
 *
 * */
uint16_t tuya_ble_scheduler_queue_space_get(void)
{
    return tuya_ble_sched_queue_space_get();
}

/**
 * @brief   Function for get the number of current events in the queue.
 *
 * @note
 *
 * */
uint16_t tuya_ble_scheduler_queue_events_get(void)
{
    return tuya_ble_sched_queue_events_get();
}

#endif

/**
 * @brief   Function for check if sleep is allowed.
 *
 * @note    If it returns true, it means that sleep is allowed.
 *
 * */
bool tuya_ble_sleep_allowed_check(void)
{
#if TUYA_BLE_USE_OS
    if (tuya_ble_internal_production_test_with_ble_flag_get() == 0)
#else
    if ((tuya_ble_scheduler_queue_events_get() == 0) && (tuya_ble_internal_production_test_with_ble_flag_get() == 0))
#endif
    {
        return true;
    } else {
        return false;
    }
}


/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_gatt_receive_data(uint8_t *p_data, uint16_t len)
{
    tuya_ble_evt_param_t event;

    memset(&event, 0, sizeof(tuya_ble_evt_param_t));

    event.hdr.event = TUYA_BLE_EVT_MTU_DATA_RECEIVE;

    if (len > TUYA_BLE_DATA_MTU_MAX) {
        event.mtu_data.len = TUYA_BLE_DATA_MTU_MAX;
    } else {
        event.mtu_data.len = len;
    }
    if (event.mtu_data.len <= 20) {
        memcpy(event.mtu_data.data, p_data, event.mtu_data.len);
    } else {
        event.mtu_data.p_data = (uint8_t *)tuya_ble_malloc(event.mtu_data.len);
        if (event.mtu_data.p_data == NULL) {
            printf("tuya_ble_gatt_receive_data error 1");
            return TUYA_BLE_ERR_NO_MEM;
        }
        memcpy(event.mtu_data.p_data, p_data, event.mtu_data.len);
    }

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send ble data error,data len = %d ", event.mtu_data.len);
        if (event.mtu_data.p_data) {
            printf("tuya_ble_gatt_receive_data error 2");
            tuya_ble_free(event.mtu_data.p_data);
        }
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}


/*
 *@brief Function for receive uart data.
 *@param
 *
 *@note
 *
 * */

typedef enum {
    TUYA_BLE_UART_REV_STATE_FOUND_NULL,
    TUYA_BLE_UART_REV_STATE_FOUND_HEAD,
    TUYA_BLE_UART_REV_STATE_FOUND_CMD,
    TUYA_BLE_UART_REV_STATE_FOUND_LEN_H,
    TUYA_BLE_UART_REV_STATE_FOUND_LEN_L,
    TUYA_BLE_UART_REV_STATE_FOUND_DATA,
    TUYA_BLE_UART_REV_STATE_UNKOWN,
} tuya_ble_uart_rev_state_type_t;

#define UART_RX_BUFFER_MAX   300
#define UART_RX_DATA_LEN_MAX 249
static volatile tuya_ble_uart_rev_state_type_t current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
static uint8_t UART_RX_Buffer[UART_RX_BUFFER_MAX];
static uint8_t UART_RX_Buffer_temp[3] = {0};
static uint16_t uart_data_len =  0;
static volatile uint16_t UART_RX_Count = 0;


static bool tuya_ble_common_uart_data_unpack(uint8_t data)
{
    bool ret = false;

    UART_RX_Buffer_temp[0] = UART_RX_Buffer_temp[1];
    UART_RX_Buffer_temp[1] = UART_RX_Buffer_temp[2];
    UART_RX_Buffer_temp[2] = data;

    if (((UART_RX_Buffer_temp[0] == 0x55) || (UART_RX_Buffer_temp[0] == 0x66)) && (UART_RX_Buffer_temp[1] == 0xAA) && ((UART_RX_Buffer_temp[2] == 0x00) || (UART_RX_Buffer_temp[2] == 0x01))) {
        memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
        memcpy(UART_RX_Buffer, UART_RX_Buffer_temp, 3);
        memset(UART_RX_Buffer_temp, 0, 3);
        UART_RX_Count = 3;
        current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_HEAD;
        uart_data_len = 0;
        return ret;
    }

    switch (current_uart_rev_state_type) {
    case TUYA_BLE_UART_REV_STATE_FOUND_NULL:
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_HEAD:
        UART_RX_Buffer[UART_RX_Count++] = data;
        current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_CMD;
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_CMD:
        UART_RX_Buffer[UART_RX_Count++] = data;
        current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_LEN_H;
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_LEN_H:
        UART_RX_Buffer[UART_RX_Count++] = data;
        uart_data_len = (UART_RX_Buffer[UART_RX_Count - 2] << 8) | UART_RX_Buffer[UART_RX_Count - 1];
        if (uart_data_len > UART_RX_DATA_LEN_MAX) {
            memset(UART_RX_Buffer_temp, 0, 3);
            memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
            UART_RX_Count = 0;
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
            uart_data_len = 0;
        } else if (uart_data_len > 0) {
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_LEN_L;
        } else {
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_DATA;
        }
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_LEN_L:
        UART_RX_Buffer[UART_RX_Count++] = data;   //DATA
        uart_data_len--;
        if (uart_data_len == 0) {
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_DATA;
        }
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_DATA:
        UART_RX_Buffer[UART_RX_Count++] = data;  //sum data
        ret = true;
        break;
    default:
        memset(UART_RX_Buffer_temp, 0, 3);
        memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
        UART_RX_Count = 0;
        current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
        uart_data_len = 0;
        break;
    };

    return ret;

}

tuya_ble_status_t tuya_ble_common_uart_receive_data(uint8_t *p_data, uint16_t len)
{
    tuya_ble_status_t ret = TUYA_BLE_ERR_NOT_FOUND;
    tuya_ble_evt_param_t event;
    uint8_t *uart_evt_buffer;
    uint16_t i;

    for (i = 0; i < len; i++) {
        if (tuya_ble_common_uart_data_unpack(p_data[i])) {

            uart_evt_buffer = (uint8_t *)tuya_ble_malloc(UART_RX_Count);

            if (uart_evt_buffer == NULL) {
                TUYA_BLE_LOG_ERROR("tuya_MemGet uart evt buffer fail.");
                ret = TUYA_BLE_ERR_NO_MEM;
            } else {
                event.hdr.event = TUYA_BLE_EVT_UART_CMD;

                event.uart_cmd_data.data_len = UART_RX_Count;

                event.uart_cmd_data.p_data = uart_evt_buffer;

                memcpy(event.uart_cmd_data.p_data, &UART_RX_Buffer[0], event.uart_cmd_data.data_len);

                if (tuya_ble_event_send(&event) != 0) {
                    TUYA_BLE_LOG_ERROR("tuya_event_send uart data error.");
                    tuya_ble_free(uart_evt_buffer);
                    ret = TUYA_BLE_ERR_BUSY;
                } else {
                    ret = TUYA_BLE_SUCCESS;
                }
            }
            memset(UART_RX_Buffer_temp, 0, 3);
            memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
            UART_RX_Count = 0;
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
            uart_data_len = 0;

        }
    }

    return ret;
}


/**
 * @brief   Function for send the full instruction received from uart to the sdk.
 *
 * @param
 *          [in]p_data  : pointer to full instruction data(Complete instruction,include0x55 or 0x66 0xaa and checksum.)
 *          [in]len     : Number of bytes of pdata.
 *
 * @note    If the application uses a custom uart parsing algorithm to obtain the full uart instruction,then call this function to send the full instruction.
 *
 * */
tuya_ble_status_t tuya_ble_common_uart_send_full_instruction_received(uint8_t *p_data, uint16_t len)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    tuya_ble_evt_param_t event;
    uint16_t rev_len, data_len;
    uint8_t *uart_evt_buffer;

    if (len < 7) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if ((p_data[0] != 0x55) && (p_data[0] != 0x66)) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    uart_evt_buffer = (uint8_t *)tuya_ble_malloc(len);

    if (uart_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("tuya_MemGet uart evt buffer fail.");
        ret = TUYA_BLE_ERR_NO_MEM;
    } else {
        event.hdr.event = TUYA_BLE_EVT_UART_CMD;

        event.uart_cmd_data.data_len = len;

        event.uart_cmd_data.p_data = uart_evt_buffer;

        memcpy(event.uart_cmd_data.p_data, p_data, len);

        if (tuya_ble_event_send(&event) != 0) {
            TUYA_BLE_LOG_ERROR("tuya_event_send uart full cmd error.");
            tuya_ble_free(uart_evt_buffer);
            ret = TUYA_BLE_ERR_BUSY;
        }
    }

    return ret;
}




/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_device_update_product_id(tuya_ble_product_id_type_t type, uint8_t len, uint8_t *p_buf)
{
    tuya_ble_evt_param_t event;

    if (len == 0) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if (len > TUYA_BLE_PRODUCT_ID_MAX_LEN) {
        len = TUYA_BLE_PRODUCT_ID_MAX_LEN;
    }

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    if (type == TUYA_BLE_PRODUCT_ID_TYPE_PID) {
        event.device_info_data.type = DEVICE_INFO_TYPE_PID;
    } else {
        event.device_info_data.type = DEVICE_INFO_TYPE_PRODUCT_KEY;
    }

    event.device_info_data.len = len;
    memcpy(event.device_info_data.data, p_buf, len);
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send product id update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}


/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_device_update_login_key(uint8_t *p_buf, uint8_t len)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    event.device_info_data.type = DEVICE_INFO_TYPE_LOGIN_KEY;

    if (len < LOGIN_KEY_LEN) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    event.device_info_data.len = LOGIN_KEY_LEN;
    memcpy(event.device_info_data.data, p_buf, LOGIN_KEY_LEN);
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send LOGIN KEY update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}
#if ((TUYA_BLE_PROTOCOL_VERSION_HIGN==4) && (TUYA_BLE_BEACON_KEY_ENABLE))
/**
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_device_update_beacon_key(uint8_t *p_buf, uint8_t len)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    event.device_info_data.type = DEVICE_INFO_TYPE_BEACON_KEY;

    if (len < BEACON_KEY_LEN) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    event.device_info_data.len = BEACON_KEY_LEN;
    memcpy(event.device_info_data.data, p_buf, BEACON_KEY_LEN);
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send BEACON KEY update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}
#endif
/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_device_update_mcu_version(uint32_t mcu_firmware_version, uint32_t mcu_hardware_version)
{
    tuya_ble_set_external_mcu_version(mcu_firmware_version, mcu_hardware_version);
    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_device_update_bound_state(uint8_t state)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    event.device_info_data.type = DEVICE_INFO_TYPE_BOUND;

    if ((state != 1) && (state != 0)) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    event.device_info_data.len = 1;
    event.device_info_data.data[0] = state;
    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send bound state update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}


#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==4)

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_send(uint32_t sn, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, uint8_t *p_dp_data, uint32_t dp_data_len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7)) || (dp_data_len == 0)) {
        TUYA_BLE_LOG_ERROR("send dp data len error,data len = %d , max data len = %d", dp_data_len, TUYA_BLE_SEND_MAX_DATA_LEN - 7);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(dp_data_len + 7);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;
        switch (type) {
        case DP_SEND_TYPE_ACTIVE:
            ble_evt_buffer[5] = 0;
            break;
        case DP_SEND_TYPE_PASSIVE:
            ble_evt_buffer[5] = 1;
            break;
        default:
            ble_evt_buffer[5] = 0;
            break;

        };
        if (ack == DP_SEND_WITHOUT_RESPONSE) {
            ble_evt_buffer[5] |= 0x80;
        }
        ble_evt_buffer[6] = mode;
        memcpy(ble_evt_buffer + 7, p_dp_data, dp_data_len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_SEND;
    evt.dp_send_data.sn = sn;
    evt.dp_send_data.type = type;
    evt.dp_send_data.mode = mode;
    evt.dp_send_data.ack = ack;
    evt.dp_send_data.p_data = ble_evt_buffer;
    evt.dp_send_data.data_len = dp_data_len + 7;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}


/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_with_time_send(uint32_t sn, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, uint8_t *p_time_data, uint8_t *p_dp_data, uint32_t dp_data_len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;
    uint16_t buffer_len = 0;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((dp_data_len > (TUYA_BLE_SEND_MAX_DATA_LEN - 7 - 14)) || (dp_data_len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ret = data_2_klvlist(p_dp_data, dp_data_len, &list, 1);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
        buffer_len = dp_data_len + 12;
    } else if (time_type == DP_TIME_TYPE_MS_STRING) {
        buffer_len = dp_data_len + 21;
    } else {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(buffer_len);

    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = TUYA_BLE_DP_WRITE_CURRENT_VERSION;
        ble_evt_buffer[1] = sn >> 24;
        ble_evt_buffer[2] = sn >> 16;
        ble_evt_buffer[3] = sn >> 8;
        ble_evt_buffer[4] = sn;
        ble_evt_buffer[5] = 0;    //must 0 - WITH RESPONSE

        ble_evt_buffer[6] = mode;
        ble_evt_buffer[7] = time_type;

        if (time_type == DP_TIME_TYPE_UNIX_TIMESTAMP) {
            memcpy(&ble_evt_buffer[8], p_time_data, 4);
            memcpy(ble_evt_buffer + 12, p_dp_data, dp_data_len);
        } else {
            memcpy(&ble_evt_buffer[8], p_time_data, 13);
            memcpy(ble_evt_buffer + 21, p_dp_data, dp_data_len);
        }


    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_SEND;
    evt.dp_with_time_send_data.sn = sn;
    evt.dp_with_time_send_data.type = DP_SEND_TYPE_ACTIVE;
    evt.dp_with_time_send_data.mode = mode;
    evt.dp_with_time_send_data.ack = DP_SEND_WITH_RESPONSE;
    evt.dp_with_time_send_data.p_data = ble_evt_buffer;
    evt.dp_with_time_send_data.data_len = buffer_len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#else
/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_report(uint8_t *p_data, uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > TUYA_BLE_REPORT_MAX_DP_DATA_LEN) || (len == 0)) {
        TUYA_BLE_LOG_ERROR("report dp data len error,data len = %d , max data len = %d", len, TUYA_BLE_REPORT_MAX_DP_DATA_LEN);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_REPORTED;
    evt.reported_data.p_data = ble_evt_buffer;
    evt.reported_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_with_flag_report(uint16_t sn, tuya_ble_dp_data_send_mode_t mode, uint8_t *p_data, uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > (TUYA_BLE_REPORT_MAX_DP_DATA_LEN - 3)) || (len == 0)) {
        TUYA_BLE_LOG_ERROR("report flag dp data len error,data len = %d , max data len = %d", len, (TUYA_BLE_REPORT_MAX_DP_DATA_LEN - 3));
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len + 3);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        ble_evt_buffer[0] = sn >> 8;
        ble_evt_buffer[1] = sn;
        ble_evt_buffer[2] = mode;
        memcpy(ble_evt_buffer + 3, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_REPORTED;
    evt.flag_reported_data.sn = sn;
    evt.flag_reported_data.mode = mode;
    evt.flag_reported_data.p_data = ble_evt_buffer;
    evt.flag_reported_data.data_len = len + 3;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_dp_data_with_time_report(uint32_t timestamp, uint8_t *p_data, uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > TUYA_BLE_REPORT_MAX_DP_DATA_LEN) || (len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_REPORTED;
    evt.reported_with_time_data.timestamp = timestamp;
    evt.reported_with_time_data.p_data = ble_evt_buffer;
    evt.reported_with_time_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_report(uint16_t sn, tuya_ble_dp_data_send_mode_t mode, uint32_t timestamp, uint8_t *p_data, uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > (TUYA_BLE_REPORT_MAX_DP_DATA_LEN - 8)) || (len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORTED;
    evt.flag_reported_with_time_data.sn = sn;
    evt.flag_reported_with_time_data.mode = mode;
    evt.flag_reported_with_time_data.timestamp = timestamp;
    evt.flag_reported_with_time_data.p_data = ble_evt_buffer;
    evt.flag_reported_with_time_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note  time_string: 13-byte millisecond string ,for example ,"0000000123456";
 *
 * */

tuya_ble_status_t tuya_ble_dp_data_with_time_ms_string_report(uint8_t *time_string, uint8_t *p_data, uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > TUYA_BLE_REPORT_MAX_DP_DATA_LEN) || (len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_STRING_REPORTED;
    memcpy(evt.reported_with_time_string_data.time_string, time_string, 13);
    evt.reported_with_time_string_data.p_data = ble_evt_buffer;
    evt.reported_with_time_string_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note  time_string: 13-byte millisecond string ,for example ,"0000000123456";
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_ms_string_report(uint16_t sn, tuya_ble_dp_data_send_mode_t mode, uint8_t *time_string, uint8_t *p_data, uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if ((len > (TUYA_BLE_REPORT_MAX_DP_DATA_LEN - 17)) || (len == 0)) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data, len, &list, 0);
    if (MTP_OK != ret) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    } else {
        memcpy(ble_evt_buffer, p_data, len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_STRING_REPORTED;
    evt.flag_reported_with_time_string_data.sn = sn;
    evt.flag_reported_with_time_string_data.mode  = mode;
    memcpy(evt.flag_reported_with_time_string_data.time_string, time_string, 13);
    evt.flag_reported_with_time_string_data.p_data = ble_evt_buffer;
    evt.flag_reported_with_time_string_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#endif

/**
 * @brief   Function for data passthrough.
 *
 * @note    The tuya sdk will forwards the data to the app.
 *.
 * */
tuya_ble_status_t tuya_ble_data_passthrough(uint8_t *p_data, uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer, (uint8_t *)p_data, len);
    evt.hdr.event = TUYA_BLE_EVT_DATA_PASSTHROUGH;
    evt.passthrough_data.p_data = ble_evt_buffer;
    evt.passthrough_data.data_len = len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

/**
 * @brief   Function for response the production test instruction asynchronous.
 *
 * @param   [in]channel: 0-uart ,1 - ble.
 *          [in]p_data  : pointer to production test cmd data(Complete instruction,include0x66 0xaa and checksum.)
 *          [in]len    : Number of bytes of pdata.
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_production_test_asynchronous_response(uint8_t channel, uint8_t *p_data, uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer;

    if (len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer, (uint8_t *)p_data, len);
    evt.hdr.event = TUYA_BLE_EVT_PRODUCTION_TEST_RESPONSE;
    evt.prod_test_res_data.p_data = ble_evt_buffer;
    evt.prod_test_res_data.data_len = len;
    evt.prod_test_res_data.channel = channel;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}


/**
 * @brief   Function for response for the net config req.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_net_config_response(int16_t result_code)
{
    tuya_ble_evt_param_t evt;
    tuya_ble_connect_status_t status = tuya_ble_connect_status_get();

    if ((status != BONDING_CONN) && (status != UNBONDING_CONN)) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_NET_CONFIG_RESPONSE;
    evt.net_config_response_data.result_code = result_code;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

#if (!TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
/**
 * @brief   Function for response for ubound req.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_ubound_response(uint8_t result_code)
{
    tuya_ble_evt_param_t evt;
    tuya_ble_connect_status_t status = tuya_ble_connect_status_get();

    if (status != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_UNBOUND_RESPONSE;
    evt.ubound_res_data.result_code = result_code;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}


/**
 * @brief   Function for response for anomaly ubound req.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_anomaly_ubound_response(uint8_t result_code)
{
    tuya_ble_evt_param_t evt;
    tuya_ble_connect_status_t status = tuya_ble_connect_status_get();

    if (status != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_ANOMALY_UNBOUND_RESPONSE;
    evt.anomaly_ubound_res_data.result_code = result_code;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

/**
 * @brief   Function for response for device reset req.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_device_reset_response(uint8_t result_code)
{
    tuya_ble_evt_param_t evt;
    tuya_ble_connect_status_t status = tuya_ble_connect_status_get();

    if (status != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_DEVICE_RESET_RESPONSE;
    evt.device_reset_res_data.result_code = result_code;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

#endif


/**
 *@brief
 *@param
 *
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_device_unbind(void)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_DEVICE_UNBIND;
    event.device_unbind_data.reserve = 0;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send device unbind error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_device_factory_reset(void)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_FACTORY_RESET;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send factory reset error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}


/*
 *@brief
 *@param time_type:
 *     	0 -13-bytes millisecond string[from cloud]
 *		1 - normal time format[from cloud]
 *		2 - normal time format[from app local]
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_time_req(uint8_t time_type)
{
    tuya_ble_evt_param_t event;

    if (time_type > 2) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    event.hdr.event = TUYA_BLE_EVT_TIME_REQ;
    event.time_req_data.time_type = time_type;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send time req error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_ota_response(tuya_ble_ota_response_t *p_data)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer;

    if (tuya_ble_connect_status_get() != BONDING_CONN) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if (p_data->data_len > TUYA_BLE_SEND_MAX_DATA_LEN) {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(p_data->data_len);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer, (uint8_t *)p_data->p_data, p_data->data_len);
    evt.hdr.event = TUYA_BLE_EVT_OTA_RESPONSE;
    evt.ota_response_data.type = p_data->type;
    evt.ota_response_data.p_data = ble_evt_buffer;
    evt.ota_response_data.data_len = p_data->data_len;

    if (tuya_ble_event_send(&evt) != 0) {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 3)
/*
 *@brief
 *@param [in]on_off: 0-off ,1 - on.
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_adv_data_connecting_request_set(uint8_t on_off)
{
    tuya_ble_connect_status_t currnet_connect_status;
    tuya_ble_evt_param_t evt;

    if (on_off > 1) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    currnet_connect_status = tuya_ble_connect_status_get();
    if ((currnet_connect_status != BONDING_UNCONN) && (currnet_connect_status != UNBONDING_UNCONN)) {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    evt.hdr.event = TUYA_BLE_EVT_CONNECTING_REQUEST;
    evt.connecting_request_data.cmd = on_off;

    if (tuya_ble_event_send(&evt) != 0) {
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

#endif

extern tuya_ble_parameters_settings_t tuya_ble_current_para;

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
void tuya_ble_connected_handler(void)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_CONNECT_STATUS_UPDATE;
    event.connect_change_evt = TUYA_BLE_CONNECTED;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send connect handler error");
    }
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
void tuya_ble_disconnected_handler(void)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_CONNECT_STATUS_UPDATE;
    event.connect_change_evt = TUYA_BLE_DISCONNECTED;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send disconnect handler error");
    }
}

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
void tuya_ble_link_encrypted_handler(void)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_LINK_STATUS_UPDATE;
    event.link_update_data.link_app_status = 1;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send link encrypted handler error");
    }
}

#endif

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_sdk_init(tuya_ble_device_param_t *param_data)
{
    tuya_ble_cb_evt_param_t event;
    uint8_t device_id_temp[16];
    uint8_t device_id_temp2[20];
    tuya_ble_gap_addr_t bt_addr;
    uint8_t mac_temp[6];

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION) && \
    (TUYA_BLE_INCLUDE_CJSON_COMPONENTS != 0)
    cJSON_Hooks hooks;
    hooks.malloc_fn = tuya_ble_malloc;
    hooks.free_fn = tuya_ble_free;
    cJSON_InitHooks(&hooks);
#endif

#if (!TUYA_BLE_DEVICE_AUTH_DATA_STORE)
    if ((param_data->use_ext_license_key != 1) || ((param_data->device_id_len != 16) && (param_data->device_id_len != 20))) {
        TUYA_BLE_LOG_ERROR("tuya_ble_sdk_init param_data->device_id_len error.");
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
#endif

    if (param_data->adv_local_name_len > TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN) {
        TUYA_BLE_LOG_ERROR("tuya_ble_sdk_init param_data->adv_local_name_len error.");
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    tuya_ble_storage_init();

    if (param_data->product_id_len > 0) {  //
        tuya_ble_current_para.pid_type = param_data->p_type;

        tuya_ble_current_para.pid_len = param_data->product_id_len;

        if (tuya_ble_current_para.pid_len > TUYA_BLE_PRODUCT_ID_MAX_LEN) {
            tuya_ble_current_para.pid_len = TUYA_BLE_PRODUCT_ID_MAX_LEN;
        }

        memcpy(tuya_ble_current_para.pid, param_data->product_id, tuya_ble_current_para.pid_len);
    } else {
#if ( TUYA_BLE_PROD_SUPPORT_OEM_TYPE == TUYA_BLE_PROD_OEM_TYPE_0_5 )
        tuya_ble_current_para.pid_type = tuya_ble_current_para.auth_settings.pid_type;
        tuya_ble_current_para.pid_len = tuya_ble_current_para.auth_settings.pid_len;
        memcpy(tuya_ble_current_para.pid, tuya_ble_current_para.auth_settings.factory_pid, tuya_ble_current_para.pid_len);
#endif
    }

    if (param_data->adv_local_name_len > 0) {  //
        memcpy(tuya_ble_current_para.adv_local_name, param_data->adv_local_name, param_data->adv_local_name_len);
        tuya_ble_current_para.adv_local_name_len = param_data->adv_local_name_len;
    } else {
        memcpy(tuya_ble_current_para.adv_local_name, "TY", 2);
        tuya_ble_current_para.adv_local_name_len = 2;
    }

#if (!TUYA_BLE_DEVICE_AUTH_DATA_STORE)
    if (param_data->device_id_len == 20) {
        TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", param_data->device_id, 20);
        tuya_ble_device_id_20_to_16(param_data->device_id, device_id_temp);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_16 ", device_id_temp, 16);
        tuya_ble_device_id_16_to_20(device_id_temp, device_id_temp2);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", device_id_temp2, 20);
        memcpy(tuya_ble_current_para.auth_settings.device_id, device_id_temp, DEVICE_ID_LEN);
    } else {
        memcpy(tuya_ble_current_para.auth_settings.device_id, param_data->device_id, DEVICE_ID_LEN);
    }

    memcpy(tuya_ble_current_para.auth_settings.auth_key, param_data->auth_key, AUTH_KEY_LEN);

    memcpy(tuya_ble_current_para.sys_settings.device_virtual_id, param_data->device_vid, DEVICE_VIRTUAL_ID_LEN);

    /* copy mac & mac_string into tuya_ble_current_para.auth_settings */
    tuya_ble_str_to_hex(param_data->mac_addr_string, MAC_STRING_LEN, mac_temp);
    TUYA_BLE_LOG_HEXDUMP_INFO("The MAC address passed in by the application", mac_temp, 6);
    tuya_ble_inverted_array(mac_temp, 6); //Convert to little endian format
    memcpy(param_data->mac_addr.addr, mac_temp, MAC_LEN);
    memcpy(tuya_ble_current_para.auth_settings.mac, mac_temp, MAC_LEN);
    tuya_ble_hextoascii(mac_temp, MAC_LEN, tuya_ble_current_para.auth_settings.mac_string);

    tuya_ble_current_para.sys_settings.bound_flag = param_data->bound_flag;

    if (tuya_ble_current_para.sys_settings.bound_flag) {
        memcpy(tuya_ble_current_para.sys_settings.login_key, param_data->login_key, LOGIN_KEY_LEN);

#if ((TUYA_BLE_PROTOCOL_VERSION_HIGN==4) && (TUYA_BLE_BEACON_KEY_ENABLE))

        memcpy(tuya_ble_current_para.sys_settings.beacon_key, param_data->beacon_key, BEACON_KEY_LEN);

#endif
    }

#else

    if ((param_data->use_ext_license_key == 1) && ((param_data->device_id_len == 16) || (param_data->device_id_len == 20))) {
        if (param_data->device_id_len == 20) {
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", param_data->device_id, 20);
            tuya_ble_device_id_20_to_16(param_data->device_id, device_id_temp);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_16 ", device_id_temp, 16);
            tuya_ble_device_id_16_to_20(device_id_temp, device_id_temp2);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", device_id_temp2, 20);
            memcpy(tuya_ble_current_para.auth_settings.device_id, device_id_temp, DEVICE_ID_LEN);
        } else {
            memcpy(tuya_ble_current_para.auth_settings.device_id, param_data->device_id, DEVICE_ID_LEN);
        }

        memcpy(tuya_ble_current_para.auth_settings.auth_key, param_data->auth_key, AUTH_KEY_LEN);

        /* copy mac & mac_string into tuya_ble_current_para.auth_settings */
        tuya_ble_str_to_hex(param_data->mac_addr_string, MAC_STRING_LEN, mac_temp);
        TUYA_BLE_LOG_HEXDUMP_INFO("The MAC address passed in by the application", mac_temp, 6);
        tuya_ble_inverted_array(mac_temp, 6); //Convert to little endian format
        memcpy(param_data->mac_addr.addr, mac_temp, MAC_LEN);
        memcpy(tuya_ble_current_para.auth_settings.mac, mac_temp, MAC_LEN);
        tuya_ble_hextoascii(mac_temp, MAC_LEN, tuya_ble_current_para.auth_settings.mac_string);
    }

#endif

    if (tuya_ble_gap_addr_get(&bt_addr) == TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_HEXDUMP_INFO("current device MAC address : ", bt_addr.addr, 6);

        if (TUYA_BLE_DEVICE_MAC_UPDATE) {
            if (param_data->device_id_len == 0) {
                bt_addr.addr_type = TUYA_BLE_ADDRESS_TYPE_RANDOM;
                memcpy(mac_temp, tuya_ble_current_para.auth_settings.mac, 6);
            } else {
                bt_addr.addr_type = param_data->mac_addr.addr_type;
                memcpy(mac_temp, param_data->mac_addr.addr, 6);
            }

            if ((!tuya_ble_buffer_value_is_all_x(mac_temp, 6, 0)) && (memcmp(mac_temp, bt_addr.addr, 6))) {
                memcpy(bt_addr.addr, mac_temp, 6);
                if (tuya_ble_gap_addr_set(&bt_addr) != TUYA_BLE_SUCCESS) {
                    TUYA_BLE_LOG_ERROR("GAP ADDR SET failed!");
                } else {
                    TUYA_BLE_LOG_INFO("GAP ADDR SET SUCCESSED!");
                    if (TUYA_BLE_DEVICE_MAC_UPDATE_RESET) {
                        tuya_ble_device_delay_ms(500);
                        tuya_ble_device_reset();
                    }
                }
            }
        }
    } else {
        TUYA_BLE_LOG_WARNING("GAP ADDR GET failed!");
    }

    tuya_ble_set_device_version(param_data->firmware_version, param_data->hardware_version);

    tuya_ble_set_external_mcu_version(0, 0); //Initialize to 0

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        tuya_ble_connect_status_set(BONDING_UNCONN);
    } else {
        tuya_ble_connect_status_set(UNBONDING_UNCONN);
    }

    tuya_ble_adv_change();

    tuya_ble_event_init();

    tuya_ble_gatt_send_queue_init();

    tuya_ble_common_uart_init();

    tuya_ble_connect_monitor_timer_init();

    TUYA_BLE_LOG_HEXDUMP_INFO("auth settings mac", tuya_ble_current_para.auth_settings.mac, MAC_LEN);
    TUYA_BLE_LOG_HEXDUMP_INFO("product_id", tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("device_uuid", tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("device_authkey", tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);
    TUYA_BLE_LOG_INFO("bond_flag = %d", tuya_ble_current_para.sys_settings.bound_flag);

    TUYA_BLE_LOG_INFO("tuya ble sdk version : "TUYA_BLE_SDK_VERSION_STR);

    return TUYA_BLE_SUCCESS;
}
