/**
 * \file tuya_ble_main.h
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


#ifndef TUYA_BLE_MAIN_H__
#define TUYA_BLE_MAIN_H__

#include "tuya_ble_type.h"
#include "tuya_ble_internal_config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern tuya_ble_parameters_settings_t tuya_ble_current_para;

void tuya_ble_connect_status_set(tuya_ble_connect_status_t status);

tuya_ble_connect_status_t tuya_ble_connect_status_get(void);

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

void tuya_ble_link_status_set(tuya_ble_link_status_t status);

tuya_ble_link_status_t tuya_ble_link_status_get(void);

#endif

void tuya_ble_event_init(void);

uint8_t tuya_ble_event_send(tuya_ble_evt_param_t *evt);

uint8_t tuya_ble_cb_event_send(tuya_ble_cb_evt_param_t *evt);

uint8_t tuya_ble_get_adv_connect_request_bit_status(void);

void tuya_ble_adv_change(void);

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 3)
void tuya_ble_adv_change_with_connecting_request(void);
#endif

tuya_ble_status_t tuya_ble_inter_event_response(tuya_ble_cb_evt_param_t *param);

void tuya_ble_connect_monitor_timer_init(void);

void tuya_ble_connect_monitor_timer_start(void);

void tuya_ble_connect_monitor_timer_stop(void);

#ifdef __cplusplus
}
#endif


#endif

