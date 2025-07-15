/**
 * \file tuya_ble_event_handler.h
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


#ifndef TUYA_BLE_EVENT_HANDLER_H__
#define TUYA_BLE_EVENT_HANDLER_H__

#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif



void tuya_ble_handle_device_info_update_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_dp_data_reported_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_dp_data_with_time_reported_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_dp_data_with_time_string_reported_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_dp_data_with_flag_reported_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_dp_data_with_flag_and_time_reported_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_dp_data_with_flag_and_time_string_reported_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_dp_data_send_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_dp_data_with_time_send_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_device_unbind_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_factory_reset_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_ota_response_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_bulk_data_evt(tuya_ble_evt_param_t *p_evt);

void tuya_ble_handle_data_passthrough_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_data_prod_test_response_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_uart_cmd_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_ble_cmd_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_net_config_response_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_time_request_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_extend_time_request_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_unbound_response_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_anomaly_unbound_response_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_device_reset_response_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_connect_change_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_ble_data_evt(uint8_t *buf, uint16_t len);

void tuya_ble_handle_connecting_request_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_link_update_evt(tuya_ble_evt_param_t *evt);

void tuya_ble_handle_weather_data_request_evt(tuya_ble_evt_param_t *evt);

#ifdef __cplusplus
}
#endif


#endif

