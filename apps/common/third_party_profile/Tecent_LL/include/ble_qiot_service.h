/*
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef QCLOUD_BLE_IOT_SERVICE_H_
#define QCLOUD_BLE_IOT_SERVICE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "ble_qiot_export.h"
#include "ble_qiot_llsync_data.h"
#include "ble_qiot_template.h"

// message type, reference data template
enum {
    BLE_QIOT_MSG_TYPE_PROPERTY = 0,
    BLE_QIOT_MSG_TYPE_EVENT,
    BLE_QIOT_MSG_TYPE_ACTION,
    BLE_QIOT_MSG_TYPE_BUTT,
};

// define reply result
enum {
    BLE_QIOT_REPLY_SUCCESS = 0,
    BLE_QIOT_REPLY_FAIL,
    BLE_QIOT_REPLY_DATA_ERR,
    BLE_QIOT_REPLY_BUTT,
};

// define message type that from server to device
enum {
    BLE_QIOT_DATA_DOWN_REPORT_REPLY = 0,
    BLE_QIOT_DATA_DOWN_CONTROL,
    BLE_QIOT_DATA_DOWN_GET_STATUS_REPLY,
    BLE_QIOT_DATA_DOWN_ACTION,
    BLE_QIOT_DATA_DOWN_EVENT_REPLY,
};

// define message type that from device to server
enum {
    BLE_QIOT_EVENT_UP_PROPERTY_REPORT = 0,
    BLE_QIOT_EVENT_UP_CONTROL_REPLY,
    BLE_QIOT_EVENT_UP_GET_STATUS,
    BLE_QIOT_EVENT_UP_EVENT_POST,
    BLE_QIOT_EVENT_UP_ACTION_REPLY,
    BLE_QIOT_EVENT_UP_BIND_SIGN_RET,
    BLE_QIOT_EVENT_UP_CONN_SIGN_RET,
    BLE_QIOT_EVENT_UP_UNBIND_SIGN_RET,
    BLE_QIOT_EVENT_UP_REPORT_MTU,
    BLE_QIOT_EVENT_UP_REPLY_OTA_REPORT,
    BLE_QIOT_EVENT_UP_REPLY_OTA_DATA,
    BLE_QIOT_EVENT_UP_REPORT_CHECK_RESULT,
    BLE_QIOT_EVENT_UP_SYNC_MTU,
    BLE_QIOT_EVENT_UP_SYNC_WAIT_TIME,
    BLE_QIOT_EVENT_UP_WIFI_MODE = 0xE0,
    BLE_QIOT_EVENT_UP_WIFI_INFO,
    BLE_QIOT_EVENT_UP_WIFI_CONNECT,
    BLE_QIOT_EVENT_UP_WIFI_TOKEN,
    BLE_QIOT_EVENT_UP_WIFI_LOG,
    BLE_QIOT_EVENT_UP_BUTT,
};

// msg header define, bit 7-6 is msg type, bit 5 means request or reply, bit 4 - 0 is id
#define	BLE_QIOT_PARSE_MSG_HEAD_TYPE(_C)        	(((_C) & 0XFF) >> 6)
#define	BLE_QIOT_PARSE_MSG_HEAD_EFFECT(_C)      	((((_C) & 0XFF) & 0X20) ? BLE_QIOT_EFFECT_REPLY : BLE_QIOT_EFFECT_REQUEST)
#define	BLE_QIOT_PARSE_MSG_HEAD_ID(_C)          	((_C) & 0X1F)

// tlv header define, bit 7 - 5 is type, bit 4 - 0 depends on type of data template
#define	BLE_QIOT_PARSE_TLV_HEAD_TYPE(_C)        	(((_C) & 0XFF) >> 5)
#define	BLE_QIOT_PARSE_TLV_HEAD_ID(_C)          	((_C) & 0X1F)

// handle llsync device info
// return 0 is success, other is error
int ble_device_info_msg_handle(const char *in_buf, int in_len);

// lldata message from remote
// return 0 is success, other is error
int ble_lldata_msg_handle(const char *in_buf, int in_len);

// ota message from remote
// return 0 is success, other is error
int ble_ota_msg_handle(const char *buf, uint16_t len);

#ifdef __cplusplus
}
#endif
#endif  // QCLOUD_BLE_IOT_SERVICE_H_
