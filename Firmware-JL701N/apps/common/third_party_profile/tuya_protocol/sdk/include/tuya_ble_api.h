/**
 * \file tuya_ble_api.h
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


#ifndef TUYA_BLE_API_H__
#define TUYA_BLE_API_H__

#include "tuya_ble_type.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_port.h"

#if (TUYA_BLE_USE_OS==0)
/**
 * @brief   Function for executing all enqueued tasks.
 *
 * @note    This function must be called from within the main loop. It will
 * execute all events scheduled since the last time it was called.
 * */
void tuya_ble_main_tasks_exec(void);
#endif

/**
 * @brief   Function for transmit ble data from peer devices to tuya sdk.
 *
 * @note    This function must be called from where the ble data is received.
 *.
 * */
tuya_ble_status_t tuya_ble_gatt_receive_data(uint8_t *p_data, uint16_t len);

/**
 * @brief   Function for transmit uart data to tuya sdk.
 *
 * @note    This function must be called from where the uart data is received.
 *.
 * */
tuya_ble_status_t tuya_ble_common_uart_receive_data(uint8_t *p_data, uint16_t len);

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
tuya_ble_status_t tuya_ble_common_uart_send_full_instruction_received(uint8_t *p_data, uint16_t len);


/**
 * @brief   Function for update the device id to tuya sdk.
 *
 * @note    the following id of the device must be update immediately when changed.
 *.
 * */

tuya_ble_status_t tuya_ble_device_update_product_id(tuya_ble_product_id_type_t type, uint8_t len, uint8_t *p_buf);

tuya_ble_status_t tuya_ble_device_update_login_key(uint8_t *p_buf, uint8_t len);

#if ((TUYA_BLE_PROTOCOL_VERSION_HIGN==4) && (TUYA_BLE_BEACON_KEY_ENABLE))
tuya_ble_status_t tuya_ble_device_update_beacon_key(uint8_t *p_buf, uint8_t len);
#endif

tuya_ble_status_t tuya_ble_device_update_bound_state(uint8_t state);

tuya_ble_status_t tuya_ble_device_update_mcu_version(uint32_t mcu_firmware_version, uint32_t mcu_hardware_version);

/**
 * @brief   Function for initialize the tuya sdk.
 *
 * @note    appliction should call this after all platform init complete.
 *.
 * */

tuya_ble_status_t tuya_ble_sdk_init(tuya_ble_device_param_t *param_data);


#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==4)

/**
 * @brief   Function for send the dp point data.
 *
 * @param   [in]sn: The sending sequence number of the application definition management.
 *          [in]type  : DP_SEND_TYPE_ACTIVE- The device actively sends dp data;DP_SEND_TYPE_PASSIVE- The device passively sends dp data. For example, in order to answer the dp query command of the mobile app. Currently only applicable to WIFI+BLE combo devices.
 *          [in]mode  : See the description in the 'tuya_ble_type.h' file for details.
 *          [in]ack   : See the description in the 'tuya_ble_type.h' file for details.
 *          [in]p_dp_data  : The pointer of dp data .
 *          [in]dp_data_len  : The length of dp data .
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *.
 * */
tuya_ble_status_t tuya_ble_dp_data_send(uint32_t sn, tuya_ble_dp_data_send_type_t type, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_ack_t ack, uint8_t *p_dp_data, uint32_t dp_data_len);

/**
 * @brief   Function for send the dp data with time.
 *
 * @param   [in]sn: The sending sequence number of the application definition management.
 *          [in]mode  : See the description in the 'tuya_ble_type.h' file for details.
 *          [in]time_type   : DP_TIME_TYPE_MS_STRING - Indicates that the following 'p_time_data' is a string of milliseconds that must be 13 bytes in length.
 *                            E.g, 'p_time_data' points to the string "1600777955000";
 *                            DP_TIME_TYPE_UNIX_TIMESTAMP - Indicates that the following 'p_time_data' points to the four-byte unix timestamp data.
 *                            E.g, unix timestamp is 1600777955 = 0x5F69EEE3, then 'p_time_data' is {0x5F,0x69,0xEE,0xE3} ;
 *          [in]p_time_data   : time data pointer.
 *          [in]p_dp_data   : The pointer of dp data .
 *          [in]dp_data_len  : The length of dp data .
 * @note    new api, The length of each dp data sent through this function must be 2 bytes. DTLD->'L' must be 2 bytes.
 *.         The default type of this function is 'DP_SEND_TYPE_ACTIVE', and the default ack is 'DP_SEND_WITH_RESPONSE', which cannot be changed.
 * */
tuya_ble_status_t tuya_ble_dp_data_with_time_send(uint32_t sn, tuya_ble_dp_data_send_mode_t mode, tuya_ble_dp_data_send_time_type_t time_type, uint8_t *p_time_data, uint8_t *p_dp_data, uint32_t dp_data_len);

#else

/**
 * @brief   Function for report the dp point data.
 *
 * @note
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_report(uint8_t *p_data, uint32_t len);

/**
 * @brief   Function for report the dp point data with time.
 *
 * @note
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_with_time_report(uint32_t timestamp, uint8_t *p_data, uint32_t len);

/**
 * @brief   Function for report the dp point data with time.
 *
 * @note    time_string: 13-byte millisecond string ,for example ,"0000000123456";
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_with_time_ms_string_report(uint8_t *time_string, uint8_t *p_data, uint32_t len);

/**
 * @brief   Function for report the dp point data with flag.
 *
 * @note
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_with_flag_report(uint16_t sn, tuya_ble_dp_data_send_mode_t mode, uint8_t *p_data, uint32_t len);

/**
 * @brief   Function for report the dp point data with flag and time.
 *
 * @note
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_report(uint16_t sn, tuya_ble_dp_data_send_mode_t mode, uint32_t timestamp, uint8_t *p_data, uint32_t len);

/**
 * @brief   Function for report the dp point data with flag and time.
 *
 * @note    time_string: 13-byte millisecond string ,for example ,"0000000123456";
 *.
 * */
tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_ms_string_report(uint16_t sn, tuya_ble_dp_data_send_mode_t mode, uint8_t *time_string, uint8_t *p_data, uint32_t len);

#endif

/**
 * @brief   Function for process the internal state of tuya sdk, application should  call this in connect handler.
 *
 * @note
 *.
 * */
void tuya_ble_connected_handler(void);


/**
 * @brief   Function for process the internal state of tuya sdk, application should  call this in disconnect handler.
 *
 * @note
 *.
 * */
void tuya_ble_disconnected_handler(void);

#if TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE

/**
 *@brief Function for update the link encrypted status to sdk, application should  call this function when the link layer encryption is successful.
 *@note
 *
 * */
void tuya_ble_link_encrypted_handler(void);

#endif

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN >= 3)
/**
 * @brief   Function for process the internal state of tuya sdk, application should  call this in disconnect handler.
 * @param   [in]on_off: 0-off ,1 - on.
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_adv_data_connecting_request_set(uint8_t on_off);
#endif

/**
 * @brief   Function for data passthrough.
 *
 * @note    The tuya sdk will forwards the data to the app.
 *.
 * */
tuya_ble_status_t tuya_ble_data_passthrough(uint8_t *p_data, uint32_t len);


/**
 * @brief   Function for response the production test instruction asynchronous.
 *
 * @param   [in]channel: 0-uart ,1 - ble.
 *          [in]pdata  : pointer to production test cmd data(Complete instruction,include0x66 0xaa and checksum.)
 *          [in]len    : Number of bytes of pdata.
 * @note    The tuya sdk will forwards the data to the app.
 *.
 * */
tuya_ble_status_t tuya_ble_production_test_asynchronous_response(uint8_t channel, uint8_t *p_data, uint32_t len);

/**
 * @brief   Function for response for the net config req.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_net_config_response(int16_t result_code);


#if (!TUYA_BLE_DEVICE_REGISTER_FROM_BLE)
/**
 * @brief   Function for response for ubound req.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_ubound_response(uint8_t result_code);


/**
 * @brief   Function for response for anomaly ubound req.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_anomaly_ubound_response(uint8_t result_code);

/**
 * @brief   Function for response for device reset req.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_device_reset_response(uint8_t result_code);

#endif

/**
 * @brief   Function for get the ble connet status.
 *
 * @note
 *.
 * */

tuya_ble_connect_status_t tuya_ble_connect_status_get(void);

/**
 * @brief   Function for notify the sdk the device has unbind.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_device_unbind(void);

/**
 * @brief   Function for notify the sdk the device has resumes factory Settings.
 *
 * @note    When the device resumes factory Settings,shoule notify the sdk.
 *.
 * */

tuya_ble_status_t tuya_ble_device_factory_reset(void);


/**
 * @brief   Function for Request update time.
 *
 * @param time_type: 0-13-byte millisecond string [from cloud],1 - normal time format [from cloud] , 2 - normal time format[from app local]
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_time_req(uint8_t time_type);

/**
 * @brief   Function for response the ota req.
 *
 * @note    response the ota data req from call back event.
 *.
 * */

tuya_ble_status_t tuya_ble_ota_response(tuya_ble_ota_response_t *p_data);



/**
 * @brief   Function for send custom event to main process of ble sdk.
 *
 * @note
 *.
 * */
uint8_t tuya_ble_custom_event_send(tuya_ble_custom_evt_t evt);



#if TUYA_BLE_USE_OS

/**
 * @brief   Function for registe queue to receive call back evt when use os
 *
 * @note
 *
 * */
tuya_ble_status_t tuya_ble_callback_queue_register(void *cb_queue);

/**
 * @brief   Function for response the event.
 *
 * @note    if use os,must be sure to call this function after process one event in queue.
 *
 * */
tuya_ble_status_t tuya_ble_event_response(tuya_ble_cb_evt_param_t *param);

#else

/**
 * @brief   Function for registe call back functions.
 *
 * @note    appliction should receive the message from the call back registed by this function.
 *
 * */
tuya_ble_status_t tuya_ble_callback_queue_register(tuya_ble_callback_t cb);

/**
 * @brief   Function for get scheduler queue size.
 *
 * @note    If it returns 0, it means that the queue has not been initialized.
 *
 * */
uint16_t tuya_ble_scheduler_queue_size_get(void);
/**
 * @brief   Function for get queue free space.
 *
 * @note
 *
 * */
uint16_t tuya_ble_scheduler_queue_space_get(void);
/**
 * @brief   Function for get the number of current events in the queue.
 *
 * @note
 *
 * */
uint16_t tuya_ble_scheduler_queue_events_get(void);

#endif

/**
 * @brief   Function for check if sleep is allowed.
 *
 * @note    If it returns true, it means that sleep is allowed.
 *
 * */
bool tuya_ble_sleep_allowed_check(void);


#endif

