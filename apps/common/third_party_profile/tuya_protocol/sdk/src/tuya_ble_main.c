/**
 * \file tuya_ble_main.c
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
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_event.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_app_uart_common_handler.h"
#include "tuya_ble_app_production_test.h"
#include "tuya_ble_log.h"
#include "tuya_ble_internal_config.h"

tuya_ble_parameters_settings_t tuya_ble_current_para;


static volatile tuya_ble_connect_status_t tuya_ble_connect_status = UNKNOW_STATUS;

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE
static volatile tuya_ble_link_status_t tuya_ble_link_status = LINK_UNKNOW_STATUS;
#endif

#if TUYA_BLE_USE_OS

uint32_t m_cb_queue_table[TUYA_BLE_MAX_CALLBACKS];

#else
tuya_ble_callback_t m_cb_table[TUYA_BLE_MAX_CALLBACKS];
#endif


void tuya_ble_connect_status_set(tuya_ble_connect_status_t status)
{
    tuya_ble_device_enter_critical();
    tuya_ble_connect_status = status;
    tuya_ble_device_exit_critical();

}

tuya_ble_connect_status_t tuya_ble_connect_status_get(void)
{
    return  tuya_ble_connect_status;
}

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

void tuya_ble_link_status_set(tuya_ble_link_status_t status)
{
    tuya_ble_device_enter_critical();
    tuya_ble_link_status = status;
    tuya_ble_device_exit_critical();
}

tuya_ble_link_status_t tuya_ble_link_status_get(void)
{
    return  tuya_ble_link_status;
}

#endif

tuya_ble_timer_t tuya_ble_xtimer_connect_monitor;

#define tuya_ble_connect_monitor_timeout_ms 30000

static void tuya_ble_vtimer_conncet_monitor_callback(tuya_ble_timer_t timer)
{
    tuya_ble_connect_status_t connect_state = tuya_ble_connect_status_get();

    if ((connect_state == UNBONDING_UNAUTH_CONN) || (connect_state == BONDING_UNAUTH_CONN)) {
        TUYA_BLE_LOG_DEBUG("ble disconncet because monitor timer timeout.");
        tuya_ble_gap_disconnect();
    }

}


void tuya_ble_connect_monitor_timer_init(void)
{
    if (tuya_ble_timer_create(&tuya_ble_xtimer_connect_monitor, tuya_ble_connect_monitor_timeout_ms, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_vtimer_conncet_monitor_callback) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_connect_monitor creat failed");
    }

}


void tuya_ble_connect_monitor_timer_start(void)
{
    if (tuya_ble_timer_start(tuya_ble_xtimer_connect_monitor) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_connect_monitor start failed");
    }

}


void tuya_ble_connect_monitor_timer_stop(void)
{

    if (tuya_ble_timer_stop(tuya_ble_xtimer_connect_monitor) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xtimer_connect_monitor start failed");
    }

}


#if TUYA_BLE_USE_OS

#if TUYA_BLE_SELF_BUILT_TASK
void *tuya_ble_task_handle;   //!< APP Task handle
void *tuya_ble_queue_handle;  //!< Event queue handle

static void tuya_ble_main_task(void *p_param)
{
    tuya_ble_evt_param_t tuya_ble_evt;
    uint32_t i = 0;

    while (true) {
        if (tuya_ble_os_msg_queue_recv(tuya_ble_queue_handle, &tuya_ble_evt, 0xFFFFFFFF) == true) {
            //printf("task event:%d", tuya_ble_evt.hdr.event);
            tuya_ble_event_process(&tuya_ble_evt);

        }

    }

}
#endif

#else

void tuya_ble_main_tasks_exec(void)
{
    tuya_sched_execute();
}
#endif




void tuya_ble_event_init(void)
{
#if TUYA_BLE_USE_OS

#if TUYA_BLE_SELF_BUILT_TASK
    tuya_ble_os_task_create(&tuya_ble_task_handle, "tuya", tuya_ble_main_task, 0, TUYA_BLE_TASK_STACK_SIZE, TUYA_BLE_TASK_PRIORITY);
    tuya_ble_os_msg_queue_create(&tuya_ble_queue_handle, MAX_NUMBER_OF_TUYA_MESSAGE, sizeof(tuya_ble_evt_param_t));
#endif

#else
    tuya_ble_event_queue_init();
#endif

#if TUYA_BLE_USE_OS
    for (uint8_t i = 0; i < TUYA_BLE_MAX_CALLBACKS; i++) {
        m_cb_queue_table[i] = 0;
    }

#else
    for (uint8_t i = 0; i < TUYA_BLE_MAX_CALLBACKS; i++) {
        m_cb_table[i] = NULL;
    }
#endif

}


uint8_t tuya_ble_event_send(tuya_ble_evt_param_t *evt)
{
#if TUYA_BLE_USE_OS

#if TUYA_BLE_SELF_BUILT_TASK
    if (tuya_ble_os_msg_queue_send(tuya_ble_queue_handle, evt, 0)) {
        return 0;
    } else {
        return 1;
    }
#else
    if (tuya_ble_event_queue_send_port(evt, 0)) {
        return 0;
    } else {
        return 1;
    }
#endif

#else
    if (tuya_ble_message_send(evt) == TUYA_BLE_SUCCESS) {
        return 0;
    } else {
        return 1;
    }
#endif
}


uint8_t tuya_ble_custom_event_send(tuya_ble_custom_evt_t evt)
{
    static tuya_ble_evt_param_t event;
    uint8_t ret = 0;

#if TUYA_BLE_USE_OS
    event.hdr.event = TUYA_BLE_EVT_CUSTOM;
    event.custom_evt = evt;

#if TUYA_BLE_SELT_BUILT_TASK
    if (tuya_ble_os_msg_queue_send(tuya_ble_queue_handle, &event, 0)) {
        return 0;
    } else {
        return 1;
    }
#else
    if (tuya_ble_event_queue_send_port(&event, 0)) {
        return 0;
    } else {
        return 1;
    }
#endif

#else

    event.hdr.event = TUYA_BLE_EVT_CUSTOM;
    event.custom_evt = evt;

    if (tuya_ble_message_send(&event) == TUYA_BLE_SUCCESS) {
        ret = 0;
    } else {
        ret = 1;
    }

    return ret;
#endif
}



tuya_ble_status_t tuya_ble_inter_event_response(tuya_ble_cb_evt_param_t *param)
{

    switch (param->evt) {
    case TUYA_BLE_CB_EVT_CONNECTE_STATUS:
        break;
    case TUYA_BLE_CB_EVT_DP_WRITE:
        if (param->dp_write_data.p_data) {
            tuya_ble_free(param->dp_write_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_DP_DATA_RECEIVED:
        if (param->dp_received_data.p_data) {
            tuya_ble_free(param->dp_received_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_DP_QUERY:
        if (param->dp_query_data.p_data) {
            tuya_ble_free(param->dp_query_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_OTA_DATA:
        if (param->ota_data.p_data) {
            tuya_ble_free(param->ota_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_NETWORK_INFO:
        if (param->network_data.p_data) {
            tuya_ble_free(param->network_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_WIFI_SSID:
        if (param->wifi_info_data.p_data) {
            tuya_ble_free(param->wifi_info_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_TIME_STAMP:
        break;
    case TUYA_BLE_CB_EVT_TIME_NORMAL:
        break;
    case TUYA_BLE_CB_EVT_APP_LOCAL_TIME_NORMAL:
        break;
    case TUYA_BLE_CB_EVT_TIME_STAMP_WITH_DST:
        if (param->timestamp_with_dst_data.p_data) {
            tuya_ble_free(param->timestamp_with_dst_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH:

        if (param->ble_passthrough_data.p_data) {
            tuya_ble_free(param->ble_passthrough_data.p_data);
        }
        break;
#if (TUYA_BLE_FEATURE_WEATHER_ENABLE != 0)
    case TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED:
        if (param->weather_received_data.p_data) {
            tuya_ble_free(param->weather_received_data.p_data);
        }
        break;
#endif
    default:
        break;
    }

    return TUYA_BLE_SUCCESS;
}



uint8_t tuya_ble_cb_event_send(tuya_ble_cb_evt_param_t *evt)
{
#if TUYA_BLE_USE_OS
    if (m_cb_queue_table[0]) {
        if (tuya_ble_os_msg_queue_send((void *)m_cb_queue_table[0], evt, 0)) {
            return 0;
        } else {
            return 1;
        }
    }
#else
    tuya_ble_callback_t fun;
    if (m_cb_table[0]) {
        fun = m_cb_table[0];
        fun(evt);
        tuya_ble_inter_event_response(evt);
    }
#endif
    return 0;

}


#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==4)

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */

#define TUYA_BLE_ADV_DATA_LEN_MAX  31

static const uint8_t adv_data_const[TUYA_BLE_ADV_DATA_LEN_MAX] = {
    0x02,
    0x01,
    0x06,
    0x03,
    0x02,
    0x50, 0xFD,
    0x17,
    0x16,
    0x50, 0xFD,
    0x41, 0x00,       //Frame Control
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


#define TUYA_BLE_SCAN_RSP_DATA_LEN_MAX  31
static const uint8_t scan_rsp_data_const[TUYA_BLE_SCAN_RSP_DATA_LEN_MAX] = {
    0x17,             // length
    0xFF,
    0xD0,
    0x07,
    0x00, //Encry Mode(8)
    0x00, 0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //FLAG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, //24
    0x09,
    0x54, 0x59,
};

static uint8_t adv_data[TUYA_BLE_ADV_DATA_LEN_MAX];
static uint8_t scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN_MAX];

uint8_t tuya_ble_get_adv_connect_request_bit_status(void)
{
    return (adv_data[11] & 0x02);
}

void tuya_ble_adv_get(u8 **output_adv_data, u8 *adv_data_len, u8 **output_rsp_data, u8 *rsp_data_len)
{
    uint8_t *aes_buf = NULL;
    uint8_t *p_buf = NULL;
    uint8_t adv_data_length = 0;
    uint8_t scan_rsp_data_length = 0;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN_MAX);
    memcpy(&scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN_MAX);

    adv_data[7] = 7 + tuya_ble_current_para.pid_len;

    adv_data[13] = tuya_ble_current_para.pid_type;
    adv_data[14] = tuya_ble_current_para.pid_len;

    if (TUYA_BLE_DEVICE_SHARED) {
        adv_data[11] |= 0x04 ;
    } else {
        adv_data[11] &= (~0x04);
    }

    adv_data[11] &= (~0x02); //clear connect request bit

    scan_rsp_data[4] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[5] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp_data[6] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY;

    if (tuya_ble_current_para.pid_len == 20) {
        scan_rsp_data[7] |= 0x01 ;
    } else {
        scan_rsp_data[7] &= (~0x01);
    }

#if (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

    if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT) {
        scan_rsp_data[7] |= 0x02 ;
    } else {
        scan_rsp_data[7] &= (~0x02);
    }

#endif
    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        adv_data[11] |= 0x08 ;
        //
        memcpy(aes_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
        memcpy(aes_key + LOGIN_KEY_LEN, tuya_ble_current_para.auth_settings.device_id, 16 - LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if (aes_buf == NULL) {
            TUYA_BLE_LOG_ERROR("Malloc failed for AES BUF in adv change.");
            return;
        } else {
            memset(aes_buf, 0, 200);
        }

        tuya_ble_encrypt_old_with_key(aes_key, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, aes_buf);

        memcpy(&adv_data[15], (uint8_t *)(aes_buf + 1), tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        tuya_ble_device_id_encrypt_v4(&adv_data[11], adv_data[7] - 3, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[8], encry_device_id, DEVICE_ID_LEN);

    } else {
        adv_data[11] &= (~0x08);

        memcpy(&adv_data[15], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt_v4(&adv_data[11], adv_data[7] - 3, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[8], encry_device_id, DEVICE_ID_LEN);
    }

    if (tuya_ble_current_para.adv_local_name_len > TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN) {
        tuya_ble_current_para.adv_local_name_len = TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN;
    }
    scan_rsp_data[24] = tuya_ble_current_para.adv_local_name_len + 1;
    scan_rsp_data[25] = 0x09;
    memcpy(&scan_rsp_data[26], tuya_ble_current_para.adv_local_name, tuya_ble_current_para.adv_local_name_len);

    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);

    adv_data_length = tuya_ble_current_para.pid_len + 15;
    scan_rsp_data_length = tuya_ble_current_para.adv_local_name_len + 26;

    *adv_data_len = adv_data_length;
    *rsp_data_len = scan_rsp_data_length;
    *output_adv_data = adv_data;
    *output_rsp_data = scan_rsp_data;

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

    p_buf = tuya_ble_malloc(adv_data_length + scan_rsp_data_length + 3);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("Malloc failed for read buf in adv change.");
        return;
    } else {
        memset(p_buf, 0, adv_data_length + scan_rsp_data_length + 3);
        p_buf[0] = 0;
        p_buf[1] = adv_data_length + scan_rsp_data_length;
        memcpy(&p_buf[2], adv_data, adv_data_length);
        memcpy(&p_buf[2 + adv_data_length], scan_rsp_data, scan_rsp_data_length);
    }

    tuya_ble_device_info_characteristic_value_update(p_buf, (adv_data_length + scan_rsp_data_length + 3));

    tuya_ble_free(p_buf);
#endif

}

void tuya_ble_adv_change(void)
{
    uint8_t *aes_buf = NULL;
    uint8_t *p_buf = NULL;
    uint8_t adv_data_length = 0;
    uint8_t scan_rsp_data_length = 0;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN_MAX);
    memcpy(&scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN_MAX);

    adv_data[7] = 7 + tuya_ble_current_para.pid_len;

    adv_data[13] = tuya_ble_current_para.pid_type;
    adv_data[14] = tuya_ble_current_para.pid_len;

    if (TUYA_BLE_DEVICE_SHARED) {
        adv_data[11] |= 0x04 ;
    } else {
        adv_data[11] &= (~0x04);
    }

    adv_data[11] &= (~0x02); //clear connect request bit


    scan_rsp_data[4] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[5] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp_data[6] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY;

    if (tuya_ble_current_para.pid_len == 20) {
        scan_rsp_data[7] |= 0x01 ;
    } else {
        scan_rsp_data[7] &= (~0x01);
    }

#if (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

    if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT) {
        scan_rsp_data[7] |= 0x02 ;
    } else {
        scan_rsp_data[7] &= (~0x02);
    }

#endif
    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        adv_data[11] |= 0x08 ;
        //
        memcpy(aes_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
        memcpy(aes_key + LOGIN_KEY_LEN, tuya_ble_current_para.auth_settings.device_id, 16 - LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if (aes_buf == NULL) {
            TUYA_BLE_LOG_ERROR("Malloc failed for AES BUF in adv change.");
            return;
        } else {
            memset(aes_buf, 0, 200);
        }

        tuya_ble_encrypt_old_with_key(aes_key, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, aes_buf);

        memcpy(&adv_data[15], (uint8_t *)(aes_buf + 1), tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        tuya_ble_device_id_encrypt_v4(&adv_data[11], adv_data[7] - 3, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[8], encry_device_id, DEVICE_ID_LEN);

    } else {
        adv_data[11] &= (~0x08);

        memcpy(&adv_data[15], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt_v4(&adv_data[11], adv_data[7] - 3, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[8], encry_device_id, DEVICE_ID_LEN);
    }

    if (tuya_ble_current_para.adv_local_name_len > TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN) {
        tuya_ble_current_para.adv_local_name_len = TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN;
    }
    scan_rsp_data[24] = tuya_ble_current_para.adv_local_name_len + 1;
    scan_rsp_data[25] = 0x09;
    memcpy(&scan_rsp_data[26], tuya_ble_current_para.adv_local_name, tuya_ble_current_para.adv_local_name_len);

    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);

    adv_data_length = tuya_ble_current_para.pid_len + 15;
    scan_rsp_data_length = tuya_ble_current_para.adv_local_name_len + 26;

    tuya_ble_gap_advertising_adv_data_update(adv_data, adv_data_length);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data, scan_rsp_data_length);

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

    p_buf = tuya_ble_malloc(adv_data_length + scan_rsp_data_length + 3);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("Malloc failed for read buf in adv change.");
        return;
    } else {
        memset(p_buf, 0, adv_data_length + scan_rsp_data_length + 3);
        p_buf[0] = 0;
        p_buf[1] = adv_data_length + scan_rsp_data_length;
        memcpy(&p_buf[2], adv_data, adv_data_length);
        memcpy(&p_buf[2 + adv_data_length], scan_rsp_data, scan_rsp_data_length);
    }

    tuya_ble_device_info_characteristic_value_update(p_buf, (adv_data_length + scan_rsp_data_length + 3));

    tuya_ble_free(p_buf);
#endif

}

void tuya_ble_adv_change_with_connecting_request(void)
{
    uint8_t *aes_buf = NULL;
    uint8_t *p_buf = NULL;
    uint8_t adv_data_length = 0;
    uint8_t scan_rsp_data_length = 0;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN_MAX);
    memcpy(&scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN_MAX);

    adv_data[7] = 7 + tuya_ble_current_para.pid_len;

    adv_data[13] = tuya_ble_current_para.pid_type;
    adv_data[14] = tuya_ble_current_para.pid_len;

    if (TUYA_BLE_DEVICE_SHARED) {
        adv_data[11] |= 0x04 ;
    } else {
        adv_data[11] &= (~0x04);
    }

    adv_data[11] |= 0x02;  //set connect request bit


    scan_rsp_data[4] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[5] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp_data[6] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY;

    if (tuya_ble_current_para.pid_len == 20) {
        scan_rsp_data[7] |= 0x01 ;
    } else {
        scan_rsp_data[7] &= (~0x01);
    }

#if (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

    if (TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT) {
        scan_rsp_data[7] |= 0x02 ;
    } else {
        scan_rsp_data[7] &= (~0x02);
    }

#endif
    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        adv_data[11] |= 0x08 ;
        //
        memcpy(aes_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
        memcpy(aes_key + LOGIN_KEY_LEN, tuya_ble_current_para.auth_settings.device_id, 16 - LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if (aes_buf == NULL) {
            TUYA_BLE_LOG_ERROR("Malloc failed for AES BUF in adv change with connecting request.");
            return;
        } else {
            memset(aes_buf, 0, 200);
        }

        tuya_ble_encrypt_old_with_key(aes_key, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, aes_buf);

        memcpy(&adv_data[15], (uint8_t *)(aes_buf + 1), tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        tuya_ble_device_id_encrypt_v4(&adv_data[11], adv_data[7] - 3, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[8], encry_device_id, DEVICE_ID_LEN);

    } else {
        adv_data[11] &= (~0x08);

        memcpy(&adv_data[15], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt_v4(&adv_data[11], adv_data[7] - 3, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[8], encry_device_id, DEVICE_ID_LEN);
    }

    if (tuya_ble_current_para.adv_local_name_len > TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN) {
        tuya_ble_current_para.adv_local_name_len = TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN;
    }
    scan_rsp_data[24] = tuya_ble_current_para.adv_local_name_len + 1;
    scan_rsp_data[25] = 0x09;
    memcpy(&scan_rsp_data[26], tuya_ble_current_para.adv_local_name, tuya_ble_current_para.adv_local_name_len);

    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);

    adv_data_length = tuya_ble_current_para.pid_len + 15;
    scan_rsp_data_length = tuya_ble_current_para.adv_local_name_len + 26;

    tuya_ble_gap_advertising_adv_data_update(adv_data, adv_data_length);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data, scan_rsp_data_length);

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

    p_buf = tuya_ble_malloc(adv_data_length + scan_rsp_data_length + 3);
    if (p_buf == NULL) {
        TUYA_BLE_LOG_ERROR("Malloc failed for read buf in adv change.");
        return;
    } else {
        memset(p_buf, 0, adv_data_length + scan_rsp_data_length + 3);
        p_buf[0] = 0;
        p_buf[1] = adv_data_length + scan_rsp_data_length;
        memcpy(&p_buf[2], adv_data, adv_data_length);
        memcpy(&p_buf[2 + adv_data_length], scan_rsp_data, scan_rsp_data_length);
    }

    tuya_ble_device_info_characteristic_value_update(p_buf, (adv_data_length + scan_rsp_data_length + 3));

    tuya_ble_free(p_buf);

#endif

}

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==3)

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */

#define TUYA_BLE_ADV_DATA_LEN  (12+TUYA_BLE_PRODUCT_ID_MAX_LEN)

static const uint8_t adv_data_const[TUYA_BLE_ADV_DATA_LEN] = {
    0x02,
    0x01,
    0x06,
    0x03,
    0x02,
    0x01, 0xA2,
    0x14,
    0x16,
    0x01, 0xA2,
    0x00,         //id type 00-pid 01-product key
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


#define TUYA_BLE_SCAN_RSP_DATA_LEN  30
static const uint8_t scan_rsp_data_const[TUYA_BLE_SCAN_RSP_DATA_LEN] = {
    0x03,
    0x09,
    0x54, 0x59,
    0x19,             // length
    0xFF,
    0xD0,
    0x07,
    0x00, //bond flag bit7
    0x03, //protocol version
    0x01, //Encry Mode
    0x00, 0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //data type
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t adv_data[TUYA_BLE_ADV_DATA_LEN];
static uint8_t scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN];

uint8_t tuya_ble_get_adv_connect_request_bit_status(void)
{
    return (scan_rsp_data[8] & 0x01);
}

void tuya_ble_adv_change(void)
{
    uint8_t *aes_buf = NULL;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN);
    memcpy(&scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN);

    adv_data[7] = 4 + tuya_ble_current_para.pid_len;
    adv_data[11] = tuya_ble_current_para.pid_type;

    if (TUYA_BLE_DEVICE_SHARED) {
        scan_rsp_data[8] |= 0x02 ;
    } else {
        scan_rsp_data[8] &= (~0x02);
    }

    scan_rsp_data[8] &= (~0x01);

    scan_rsp_data[9] = TUYA_BLE_PROTOCOL_VERSION_HIGN;

    scan_rsp_data[10] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[11] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp_data[12] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY;

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        scan_rsp_data[8] |= 0x80 ;
        //
        memcpy(aes_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
        memcpy(aes_key + LOGIN_KEY_LEN, tuya_ble_current_para.auth_settings.device_id, 16 - LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if (aes_buf == NULL) {
            return;
        } else {
            memset(aes_buf, 0, 200);
        }

        tuya_ble_encrypt_old_with_key(aes_key, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, aes_buf);

        memcpy(&adv_data[12], (uint8_t *)(aes_buf + 1), tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        memset(aes_key, 0, sizeof(aes_key));
        memcpy(aes_key, &adv_data[12], tuya_ble_current_para.pid_len);

        tuya_ble_device_id_encrypt(aes_key, tuya_ble_current_para.pid_len, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[14], encry_device_id, DEVICE_ID_LEN);

    } else {
        scan_rsp_data[8] &= (~0x80);

        memcpy(&adv_data[12], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt(tuya_ble_current_para.pid, tuya_ble_current_para.pid_len, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[14], encry_device_id, DEVICE_ID_LEN);
    }
    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
    tuya_ble_gap_advertising_adv_data_update(adv_data, tuya_ble_current_para.pid_len + 12);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data, sizeof(scan_rsp_data));

}

void tuya_ble_adv_change_with_connecting_request(void)
{
    uint8_t *aes_buf = NULL;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN);
    memcpy(&scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN);

    adv_data[7] = 4 + tuya_ble_current_para.pid_len;
    adv_data[11] = tuya_ble_current_para.pid_type;

    if (TUYA_BLE_DEVICE_SHARED) {
        scan_rsp_data[8] |= 0x02 ;
    } else {
        scan_rsp_data[8] &= (~0x02);
    }

    scan_rsp_data[8] |= 0x01 ;

    scan_rsp_data[9] = TUYA_BLE_PROTOCOL_VERSION_HIGN;

    scan_rsp_data[10] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[11] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY >> 8;
    scan_rsp_data[12] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY;

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        scan_rsp_data[8] |= 0x80 ;
        //
        memcpy(aes_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
        memcpy(aes_key + LOGIN_KEY_LEN, tuya_ble_current_para.auth_settings.device_id, 16 - LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if (aes_buf == NULL) {
            return;
        } else {
            memset(aes_buf, 0, 200);
        }

        tuya_ble_encrypt_old_with_key(aes_key, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, aes_buf);

        memcpy(&adv_data[12], (uint8_t *)(aes_buf + 1), tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        memset(aes_key, 0, sizeof(aes_key));
        memcpy(aes_key, &adv_data[12], tuya_ble_current_para.pid_len);

        tuya_ble_device_id_encrypt(aes_key, tuya_ble_current_para.pid_len, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[14], encry_device_id, DEVICE_ID_LEN);

    } else {
        scan_rsp_data[8] &= (~0x80);

        memcpy(&adv_data[12], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt(tuya_ble_current_para.pid, tuya_ble_current_para.pid_len, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);


        memcpy(&scan_rsp_data[14], encry_device_id, DEVICE_ID_LEN);
    }
    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
    tuya_ble_gap_advertising_adv_data_update(adv_data, tuya_ble_current_para.pid_len + 12);
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data, sizeof(scan_rsp_data));

}

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==2)

#define TUYA_BLE_ADV_DATA_LEN  11

#define TUYA_BLE_SCAN_RSP_DATA_LEN  30

static const uint8_t scan_rsp_data_const[30] = {
    /* Manufacturer Specific Data */
    0x1D,             /* length */
    0xFF,
    0x59,
    0x02,
    0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static const uint8_t adv_data_const[11] = {
    /* Flags */
    0x02,             /* length */
    0x01, /* type="Flags" */
    0x06,
    /* Local name */
    0x03,             /* length */
    0x09,
    'T', 'Y',
    0x03,
    0x02,
    0x01, 0xA2,
};

uint8_t adv_data[TUYA_BLE_ADV_DATA_LEN];
uint8_t scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN];

void tuya_ble_adv_change(void)
{
    uint8_t *aes_buf = NULL;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data, adv_data_const, TUYA_BLE_ADV_DATA_LEN);
    memcpy(scan_rsp_data, scan_rsp_data_const, TUYA_BLE_SCAN_RSP_DATA_LEN);

    scan_rsp_data[4] &= (~0x70);

    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        scan_rsp_data[4] |= 0x80 ;
        //
        memcpy(aes_key, tuya_ble_current_para.sys_settings.login_key, LOGIN_KEY_LEN);
        memcpy(aes_key + LOGIN_KEY_LEN, tuya_ble_current_para.auth_settings.device_id, 16 - LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if (aes_buf == NULL) {
            return;
        } else {
            memset(aes_buf, 0, 200);
        }

        tuya_ble_encrypt_old_with_key(aes_key, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, aes_buf);

        memcpy(&scan_rsp_data[6], (uint8_t *)(aes_buf + 1), tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        memset(aes_key, 0, sizeof(aes_key));
        memcpy(aes_key, &scan_rsp_data[6], tuya_ble_current_para.pid_len);

        tuya_ble_device_id_encrypt(aes_key, tuya_ble_current_para.pid_len, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[14], encry_device_id, DEVICE_ID_LEN);

    } else {
        scan_rsp_data[4] &= (~0x80);
        memcpy(&scan_rsp_data[6], tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt(tuya_ble_current_para.pid, 8, tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN, encry_device_id);

        memcpy(&scan_rsp_data[14], encry_device_id, DEVICE_ID_LEN);
    }

    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d", tuya_ble_current_para.sys_settings.bound_flag);

    tuya_ble_gap_advertising_adv_data_update(adv_data, sizeof(adv_data));
    tuya_ble_gap_advertising_scan_rsp_data_update(scan_rsp_data, sizeof(scan_rsp_data));

}

#endif



