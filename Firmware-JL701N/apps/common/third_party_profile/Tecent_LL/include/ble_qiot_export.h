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
#ifndef QCLOUD_BLE_QIOT_EXPORT_H
#define QCLOUD_BLE_QIOT_EXPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ble_qiot_config.h"

#define TENCENT_COMPANY_IDENTIFIER  0xFEE7  // Tencent Company ID, another is 0xFEBA

#define IOT_BLE_UUID_BASE                                                                              \
    {                                                                                                  \
        0xe2, 0xa4, 0x1b, 0x54, 0x93, 0xe4, 0x6a, 0xb5, 0x20, 0x4e, 0xd0, 0x65, 0x00, 0x00, 0x00, 0x00 \
    }

// llsync services uuid
#if BLE_QIOT_LLSYNC_STANDARD
#define IOT_BLE_UUID_SERVICE 0xFFE0
#endif //BLE_QIOT_LLSYNC_STANDARD
#if BLE_QIOT_LLSYNC_CONFIG_NET
#define IOT_BLE_UUID_SERVICE 0xFFF0
#endif //BLE_QIOT_LLSYNC_CONFIG_NET

// characteristics uuid
#define IOT_BLE_UUID_DEVICE_INFO 0xFFE1  // used to connection and identity authentication
#define IOT_BLE_UUID_EVENT       0xFFE3  // used to send data to the server from device

#if BLE_QIOT_LLSYNC_STANDARD
#define IOT_BLE_UUID_DATA        0xFFE2  // used to send data to the device from server
#define IOT_BLE_UUID_OTA         0xFFE4  // used to send ota data to the device from server
#endif //BLE_QIOT_LLSYNC_STANDARD

typedef enum {
    GATT_CHAR_BROADCAST      = (1 << 0),  // Broadcasting of the value permitted.
    GATT_CHAR_READ           = (1 << 1),  // Reading the value permitted.
    GATT_CHAR_WRITE_WO_RESP  = (1 << 2),  // Writing the value with Write Command permitted.
    GATT_CHAR_WRITE          = (1 << 3),  // Writing the value with Write Request permitted.
    GATT_CHAR_NOTIFY         = (1 << 4),  // Notification of the value permitted.
    GATT_CHAR_INDICATE       = (1 << 5),  // Indications of the value permitted.
    GATT_CHAR_AUTH_SIGNED_WR = (1 << 6),  // Writing the value with Signed Write Command permitted.
} char_props_s;

// the callback function prototype definition for the characteristics
typedef void (*ble_on_write_cb)(const uint8_t *buf, uint16_t len);

// the characteristics attributes
typedef struct {
    uint16_t        uuid16;
    uint8_t         gatt_char_props;
    ble_on_write_cb on_write;
} qiot_char_s;

// the service attributes
typedef struct {
    uint16_t service_uuid16;
    uint8_t  service_uuid128[16];
    uint16_t gatt_max_mtu;

    qiot_char_s device_info;
    qiot_char_s data;
    qiot_char_s event;
    qiot_char_s ota;
} qiot_service_init_s;

typedef enum {
    BLE_QIOT_RS_OK             = 0,   // success
    BLE_QIOT_RS_ERR            = -1,  // normal error
    BLE_QIOT_RS_ERR_FLASH      = -2,  // flash error
    BLE_QIOT_RS_ERR_PARA       = -3,  // parameters error
    BLE_QIOT_RS_VALID_SIGN_ERR = -4,
} ble_qiot_ret_status_t;

/**
 * @brief get llsync services context
 *
 * @return llsync services struct
 */
const qiot_service_init_s *ble_get_qiot_services(void);

/**
 * @brief llsync sdck initialize
 * @note  you should called it before any other sdk api
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_qiot_explorer_init(void);

/**
 * @brief  report mtu of the device to the server
 * @note   report mtu to the server to set the mtu
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_report_device_info(void);

/**
 * @brief  sync the device mtu to The Tencent Lianlian.
 * @note   if BLE_QIOT_REMOTE_SET_MTU is 1, The Tencent Lianlian will set the mtu get from function
 * ble_get_user_data_mtu_size(), but The Tencent Lianlian can not know the effective value even if the setting is
 * successful, so llsync need sync the mtu again. The user call this function in the BLE mtu callback.
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_sync_mtu(uint16_t llsync_mtu);

/**
 * @brief  start llsync advertising
 * @note   broadcast data according to the device bind state, reference to llsync protocol
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_qiot_advertising_start(void);

/**
 * @brief  stop advertising
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_qiot_advertising_stop(void);

/**
 * @brief device info write callbcak, call the function when characteristic IOT_BLE_UUID_DEVICE_INFO received data
 * @param buf a pointer point to the data
 * @param len data length
 * @return none
 */
void ble_device_info_write_cb(const uint8_t *buf, uint16_t len);

/**
 * @brief gap event connect call-back, when gap get ble connect event, use this function
 *       tell qiot ble sdk
 * @return none
 */
void ble_gap_connect_cb(void);

/**
 * @brief gap event disconnect call-back, when gap get ble disconnect event, use this function
 *       tell qiot ble sdk
 * @return none
 */
void ble_gap_disconnect_cb(void);

#ifdef BLE_QIOT_LLSYNC_STANDARD
/**
 * @brief  get property of the device from the server
 * @note   the property will be received from IOT_BLE_UUID_DATA if success
 * @return BLE_QIOT_RS_OK is success, other is error. if success, the data from server will come to
 */
ble_qiot_ret_status_t ble_event_get_status(void);

/**
 * @brief  report property of the device to the server
 * @note   the reply will be received from IOT_BLE_UUID_DATA if success
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_report_property(void);

/**
 * @brief  post event to the server
 * @param  event_id id of the event
 * @note   the reply will be received from IOT_BLE_UUID_DATA if success
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_post(uint8_t event_id);

/**
 * @brief data write callback, call the function when characteristic IOT_BLE_UUID_DATA received data
 * @param buf a pointer point to the data
 * @param len data length
 * @return none
 */
void ble_lldata_write_cb(const uint8_t *buf, uint16_t len);

/**
 * @brief ota data write callback, call the function when characteristic IOT_BLE_UUID_OTA received data
 * @param buf a pointer point to the data
 * @param len data length
 * @return none
 */
void ble_ota_write_cb(const uint8_t *buf, uint16_t len);

typedef enum {
    BLE_QIOT_SECURE_BIND_CONFIRM = 0,
    BLE_QIOT_SECURE_BIND_REJECT  = 1,
} ble_qiot_secure_bind_t;

/**
 * @brief  user choose whether to connect
 * @note   call the function when the user choose connect or not
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_secure_bind_user_confirm(ble_qiot_secure_bind_t choose);

// inform user the ota start
typedef void (*ble_ota_start_callback)(void);

enum {
    BLE_QIOT_OTA_SUCCESS     = 0,  // ota success
    BLE_QIOT_OTA_ERR_CRC     = 1,  // ota failed because crc error
    BLE_QIOT_OTA_ERR_TIMEOUT = 2,  // ota failed because download timeout
    BLE_QIOT_OTA_DISCONNECT  = 3,  // ota failed because ble disconnect
    BLE_QIOT_OTA_ERR_FILE    = 4,  // ota failed because the file mismatch the device
};
// inform user the ota stop and the result
typedef void (*ble_ota_stop_callback)(uint8_t result);

// llsync only valid the file crc, also allow the user valid the file by their way
typedef ble_qiot_ret_status_t (*ble_ota_valid_file_callback)(uint32_t file_size, char *file_version);
/**
 * @brief register ota callback
 * @param start_cb called before ota start, set null if not used
 * @param stop_cb called after ota stop, set null if not used
 * @param valid_file_cb called after the crc valid, set null if not used
 * @return none
 */
void ble_ota_callback_reg(ble_ota_start_callback start_cb, ble_ota_stop_callback stop_cb,
                          ble_ota_valid_file_callback valid_file_cb);
#endif //BLE_QIOT_LLSYNC_STANDARD

#if BLE_QIOT_LLSYNC_CONFIG_NET

typedef enum {
    BLE_WIFI_MODE_NULL = 0,  // invalid mode
    BLE_WIFI_MODE_STA  = 1,  // station
    BLE_WIFI_MODE_AP   = 2,  // ap
} BLE_WIFI_MODE;

typedef enum {
    BLE_WIFI_STATE_CONNECT = 0,  // wifi connect
    BLE_WIFI_STATE_OTHER   = 1,  // other state
} BLE_WIFI_STATE;

/**
 * @brief report wifi-mode setting result
 * @param result setting result, 0 is success, other failed
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_report_wifi_mode(uint8_t result);

/**
 * @brief report wifi-info setting result
 * @param result setting result, 0 is success, other failed
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_report_wifi_info(uint8_t result);

/**
 * @brief report wifi connection status
 * @param mode current wifi mode, reference BLE_WIFI_MODE
 * @param state current wifi state, reference BLE_WIFI_STATE
 * @param ssid_len length of ssid
 * @param ssid the ssid currently connected
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_report_wifi_connect(BLE_WIFI_MODE mode, BLE_WIFI_STATE state, uint8_t ssid_len,
        const char *ssid);

/**
 * @brief report wifi-token setting result
 * @param result setting result, 0 is success, other failed
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_report_wifi_token(uint8_t result);

/**
 * @brief report wifi-log
 * @param log log message
 * @param log_size length of log
 * @return BLE_QIOT_RS_OK is success, other is error
 */
ble_qiot_ret_status_t ble_event_report_wifi_log(const uint8_t *log, uint16_t log_size);

#endif //BLE_QIOT_LLSYNC_CONFIG_NET

#ifdef __cplusplus
}
#endif
#endif  // QCLOUD_BLE_QIOT_EXPORT_H
