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
#ifndef QCLOUD_BLE_QIOT_LLSYNC_OTA_H
#define QCLOUD_BLE_QIOT_LLSYNC_OTA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "ble_qiot_config.h"
#include "ble_qiot_export.h"

#define BLE_QIOT_GET_OTA_REQUEST_HEADER_LEN 3  // the ota request header len
#define BLE_QIOT_OTA_DATA_HEADER_LEN        3  // the ota data header len

// ota feature
#define BLE_QIOT_OTA_ENABLE        (1 << 0)
#define BLE_QIOT_OTA_RESUME_ENABLE (1 << 1)

// ota file valid result
#define BLE_QIOT_OTA_VALID_SUCCESS (1 << 7)
#define BLE_QIOT_OTA_VALID_FAIL    (0 << 7)

#define BLE_QIOT_OTA_MAX_VERSION_STR (32)  // max ota version length
#define BLE_QIOT_OTA_PAGE_VALID_VAL  0x5A  // ota info valid flag

#define BLE_QIOT_OTA_FIRST_TIMEOUT   (1)
#define BLE_QIOT_OTA_MAX_RETRY_COUNT 5  // disconnect if retry times more than BLE_QIOT_OTA_MAX_RETRY_COUNT

// ota control bits
#define BLE_QIOT_OTA_REQUEST_BIT     (1 << 0)
#define BLE_QIOT_OTA_RECV_END_BIT    (1 << 1)
#define BLE_QIOT_OTA_RECV_DATA_BIT   (1 << 2)
#define BLE_QIOT_OTA_FIRST_RETRY_BIT (1 << 3)
#define BLE_QIOT_OTA_DO_VALID_BIT    (1 << 4)
#define BLE_QIOT_OTA_HAVE_DATA_BIT   (1 << 5)

// the reason of ota file error
enum {
    BLE_QIOT_OTA_CRC_ERROR        = 0,
    BLE_QIOT_OTA_READ_FLASH_ERROR = 1,
    BLE_QIOT_OTA_FILE_ERROR       = 2,
};

// ota data type
enum {
    BLE_QIOT_OTA_MSG_REQUEST = 0,
    BLE_QIOT_OTA_MSG_DATA    = 1,
    BLE_QIOT_OTA_MSG_END     = 2,
    BLE_QIOT_OTA_MSG_BUTT,
};

// ota request reply
typedef struct ble_ota_reply_info_ {
    uint8_t  package_nums;      // package numbers in a loop
    uint8_t  package_size;      // each package size
    uint8_t  retry_timeout;     // data retry
    uint8_t  reboot_timeout;    // max time of device reboot
    uint32_t last_file_size;    // the file already received
    uint8_t  package_interval;  // package send interval on the server
    uint8_t  rsv[3];
} ble_ota_reply_info;

typedef struct ble_ota_file_info_ {
    uint32_t file_size;
    uint32_t file_crc;
    uint8_t  file_version[BLE_QIOT_OTA_MAX_VERSION_STR];
} ble_ota_file_info;

// ota info saved in flash if support resuming
typedef struct ble_ota_info_record_ {
    uint8_t           valid_flag;
    uint8_t           rsv[3];
    uint32_t          last_file_size;  // the file size already write in flash
    uint32_t          last_address;    // the address file saved
    ble_ota_file_info download_file_info;
} ble_ota_info_record;

// ota user callback
typedef struct ble_ota_user_callback_ {
    ble_ota_start_callback      start_cb;
    ble_ota_stop_callback       stop_cb;
    ble_ota_valid_file_callback valid_file_cb;
} ble_ota_user_callback;

// ota request package: 1 byte type + 2 bytes length + 4 bytes size + 4 bytes crc + 1 byte version length + 1 ~ 32 bytes
// version
typedef struct {
    bool     have_data;  // start received package
    uint8_t  type;       // event type
    uint16_t buf_len;    // the length of data
    char     buf[BLE_QIOT_GET_OTA_REQUEST_HEADER_LEN + 4 + 4 + 1 + BLE_QIOT_OTA_MAX_VERSION_STR];
} ble_ota_request_slice_t;

// ota reply info
typedef struct ble_ota_reply_t_ {
    uint32_t file_size;
    uint8_t  req;
} ble_ota_reply_t;

ble_qiot_ret_status_t ble_ota_request_handle(const char *in_buf, int buf_len);

ble_qiot_ret_status_t ble_ota_data_handle(const char *in_buf, int buf_len);

ble_qiot_ret_status_t ble_ota_file_end_handle(void);

void ble_ota_stop(void);

#ifdef __cplusplus
}
#endif

#endif  // QCLOUD_BLE_QIOT_LLSYNC_OTA_H
