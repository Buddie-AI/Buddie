/**
 * \file tuya_ble_data_handler.h
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

#ifndef TUYA_BLE_DATA_HANDLER_H_
#define TUYA_BLE_DATA_HANDLER_H_

#include "tuya_ble_stdlib.h"
#include "tuya_ble_internal_config.h"

#ifdef __cplusplus
extern "C" {
#endif


#define FRM_QRY_DEV_INFO_REQ  0x0000  //APP->BLE
#define FRM_QRY_DEV_INFO_RESP 0x0000  //BLE->APP
#define PAIR_REQ              0x0001  //APP->BLE
#define PAIR_RESP             0x0001  //BLE->APP
#define FRM_CMD_SEND          0x0002  //APP->BLE
#define FRM_CMD_RESP          0x0002  //BLE->APP
#define FRM_STATE_QUERY       0x0003  //APP->BLE
#define FRM_STATE_QUERY_RESP  0x0003  //BLE->APP
#define FRM_LOGIN_KEY_REQ     0x0004  //APP->BLE
#define FRM_LOGIN_KEY_RESP    0x0004  //BLE->APP
#define FRM_UNBONDING_REQ     0x0005  //APP->BLE
#define FRM_UNBONDING_RESP    0x0005  //BLE->APP
#define FRM_DEVICE_RESET      0x0006  //APP->BLE
#define FRM_DEVICE_RESET_RESP 0x0006  //BLE->APP

#define FRM_OTA_START_REQ         0x000C //APP->BLE
#define FRM_OTA_START_RESP        0x000C //BLE->APP
#define FRM_OTA_FILE_INFOR_REQ    0x000D //APP->BLE
#define FRM_OTA_FILE_INFOR_RESP   0x000D //BLE->APP
#define FRM_OTA_FILE_OFFSET_REQ   0x000E //APP->BLE
#define FRM_OTA_FILE_OFFSET_RESP  0x000E //BLE->APP
#define FRM_OTA_DATA_REQ          0x000F //APP->BLE
#define FRM_OTA_DATA_RESP         0x000F //BLE->APP
#define FRM_OTA_END_REQ           0x0010 //APP->BLE
#define FRM_OTA_END_RESP          0x0010 //BLE->APP

#define FRM_FACTORY_TEST_CMD           0x0012 //APP->BLE
#define FRM_FACTORY_TEST_RESP          0x0012 //BLE->APP

#define FRM_ANOMALY_UNBONDING_REQ      0x0014 //APP->BLE
#define FRM_ANOMALY_UNBONDING_RESP     0x0014 //BLE->APP

#define FRM_AUTHENTICATE_PHASE_1_REQ      0x0015 //APP->BLE
#define FRM_AUTHENTICATE_PHASE_1_RESP     0x0015 //BLE->APP

#define FRM_AUTHENTICATE_PHASE_2_REQ      0x0016 //APP->BLE
#define FRM_AUTHENTICATE_PHASE_2_RESP     0x0016 //BLE->APP

#define FRM_AUTHENTICATE_PHASE_3_REQ      0x0017 //APP->BLE
#define FRM_AUTHENTICATE_PHASE_3_RESP     0x0017 //BLE->APP

#define FRM_ANOMALY_UNBONDING_REQ      0x0014 //APP->BLE
#define FRM_ANOMALY_UNBONDING_RESP     0x0014 //BLE->APP


#define FRM_NET_CONFIG_INFO_REQ   0x0021 //APP->BLE
#define FRM_NET_CONFIG_INFO_RESP  0x0021 //BLE->APP

#define FRM_NET_CONFIG_RESPONSE_REPORT_REQ   0x0022 //APP->BLE
#define FRM_NET_CONFIG_RESPONSE_REPORT_RESP  0x0022 //BLE->APP


#define FRM_DATA_PASSTHROUGH_REQ   0x0023 //APP<->BLE

#define FRM_DP_DATA_WRITE_REQ       0x0027     //APP->BLE
#define FRM_DP_DATA_WRITE_RESP      0x0027     //BLE->APP

#define FRM_STAT_REPORT             0x8001  //BLE->APP
#define FRM_STAT_REPORT_RESP        0x8001  //APP->BLE

#define FRM_STAT_WITH_TIME_REPORT           0x8003  //BLE->APP
#define FRM_STAT_WITH_TIME_REPORT_RESP      0x8003  //APP->BLE

#define FRM_DATA_WITH_FLAG_REPORT           0x8004  //BLE->APP
#define FRM_DATA_WITH_FLAG_REPORT_RESP      0x8004  //APP->BLE

#define FRM_DATA_WITH_FLAG_AND_TIME_REPORT           0x8005  //BLE->APP
#define FRM_DATA_WITH_FLAG_AND_TIME_REPORT_RESP      0x8005  //APP->BLE

#define FRM_DP_DATA_SEND_REQ                            0x8006  //BLE->APP
#define FRM_DP_DATA_SEND_RESP                           0x8006  //APP->BLE

#define FRM_DP_DATA_WITH_TIME_SEND_REQ                  0x8007  //BLE->APP
#define FRM_DP_DATA_WITH_TIME_SEND_RESP                 0x8007  //APP->BLE

#define FRM_WEATHER_DATA_REQUEST           	0x8017  //BLE->APP
#define FRM_WEATHER_DATA_REQUEST_RESP 		0x8017  //APP->BLE

#define FRM_WEATHER_DATA_RECEIVED         	0x8018  //APP->BLE
#define FRM_WEATHER_DATA_RECEIVED_RESP     	0x8018  //BLE->APP

#define FRM_GET_UNIX_TIME_MS_REQ           0x8010  //BLE->APP
#define FRM_GET_UNIX_TIME_MS_RESP          0x8010  //APP->BLE
#define FRM_GET_UNIX_TIME_CHAR_MS_REQ      0x8011  //BLE->APP
#define FRM_GET_UNIX_TIME_CHAR_MS_RESP     0x8011  //APP->BLE
#define FRM_GET_UNIX_TIME_CHAR_DATE_REQ    0x8012  //BLE->APP
#define FRM_GET_UNIX_TIME_CHAR_DATE_RESP   0x8012  //APP->BLE

#define FRM_GET_APP_LOCAL_TIME_REQ     	   0x8013  //BLE->APP
#define FRM_GET_APP_LOCAL_TIME_RESP    	   0x8013  //APP->BLE
#define FRM_GET_UNIX_TIME_WITH_DST_REQ     0x8014  //BLE->APP [timestamp + timezone + daylight saving time]
#define FRM_GET_UNIX_TIME_WITH_DST_RESP    0x8014  //APP->BLE [timestamp + timezone + daylight saving time]

typedef enum {
    TUYA_BLE_OTA_STATUS_NONE,
    TUYA_BLE_OTA_STATUS_START,
    TUYA_BLE_OTA_STATUS_FILE_INFO,
    TUYA_BLE_OTA_STATUS_FILE_OFFSET,
    TUYA_BLE_OTA_STATUS_FILE_DATA,
    TUYA_BLE_OTA_STATUS_FILE_END,
    TUYA_BLE_OTA_STATUS_MAX,
} tuya_ble_ota_status_t;


typedef struct {
    uint32_t send_len;
    uint8_t  *send_data;
    uint32_t encrypt_data_buf_len;
    uint8_t  *encrypt_data_buf;
} tuya_ble_r_air_send_packet;


typedef struct {
    uint32_t recv_len;
    uint32_t recv_len_max;
    uint8_t  *recv_data;
    uint32_t decrypt_buf_len;
    uint8_t  *de_encrypt_buf;
} tuya_ble_r_air_recv_packet;

void tuya_ble_air_recv_packet_free(void);

void tuya_ble_set_device_version(uint32_t firmware_version, uint32_t hardware_version);

void tuya_ble_set_external_mcu_version(uint32_t firmware_version, uint32_t hardware_version);

uint8_t tuya_ble_commData_send(uint16_t cmd, uint32_t ack_sn, uint8_t *data, uint16_t len, uint8_t encryption_mode);

uint8_t tuya_ble_send(uint16_t cmd, uint32_t ack_sn, uint8_t *data, uint16_t len);

void tuya_ble_evt_process(uint16_t cmd, uint8_t *recv_data, uint32_t recv_len);

void tuya_ble_reset_ble_sn(void);

void tuya_ble_commonData_rx_proc(uint8_t *buf, uint16_t len);

void tuya_ble_pair_rand_clear(void);

uint8_t tuya_ble_pair_rand_valid_get(void);

uint32_t tuya_ble_send_packet_data_length_get(void);

void tuya_ble_device_unbond(void);

#if (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

void tuya_ble_auth_data_reset(void);

#endif

#ifdef __cplusplus
}
#endif

#endif // TUYA_BLE_COMMDATA_HANDLER_H_




