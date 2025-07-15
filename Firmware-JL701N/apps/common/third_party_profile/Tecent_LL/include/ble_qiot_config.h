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

#ifndef QCLOUD_BLE_QIOT_CONFIG_H
#define QCLOUD_BLE_QIOT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <printf.h>
#include <stdint.h>

#define BLE_QIOT_SDK_VERSION "1.5.0"  // sdk version
#define BLE_QIOT_SDK_DEBUG   0        // sdk debug

// the device broadcast is controlled by the user, but we provide a mechanism to help the device save more power.
// if you want broadcast is triggered by something like press a button instead of all the time, and the broadcast
// stopped automatically in a few minutes if the device is not bind, define BLE_QIOT_BUTTON_BROADCAST is 1 and
// BLE_QIOT_BIND_TIMEOUT is the period that broadcast stopped.
// if the device in the bound state, broadcast dose not stop automatically.
#define BLE_QIOT_BUTTON_BROADCAST 0
#if BLE_QIOT_BUTTON_BROADCAST
#define BLE_QIOT_BIND_TIMEOUT (2 * 60 * 1000)  // unit: ms
#endif //BLE_QIOT_BUTTON_BROADCAST

// in some BLE stack the default ATT_MTU is 23, set BLE_QIOT_REMOTE_SET_MTU is 1 if you want to reset the mtu by the
// Tencent Lianlian. Tencent Lianlian will set the mtu get from function ble_get_user_data_mtu_size()
#define BLE_QIOT_REMOTE_SET_MTU (1)

// the following definition will affect the stack that LLSync used，the minimum value tested is 2048 bytes
// the max length of llsync event data, depends on the length of user data reported to Tencent Lianlian at a time
#define BLE_QIOT_EVENT_MAX_SIZE (128)
// the minimum between BLE_QIOT_EVENT_MAX_SIZE and mtu
#define BLE_QIOT_EVENT_BUF_SIZE (23)

// some data like integer need to be transmitted in a certain byte order, defined it according to your device
#define __ORDER_LITTLE_ENDIAN__ 1234
#define __ORDER_BIG_ENDIAN__    4321
#define __BYTE_ORDER__          __ORDER_LITTLE_ENDIAN__

// in some BLE stack ble_qiot_log_hex() maybe not work, user can use there own hexdump function
#define BLE_QIOT_USER_DEFINE_HEXDUMP 0
#if BLE_QIOT_USER_DEFINE_HEXDUMP
// add your code here like this
// #define ble_qiot_log_hex(level, hex_name, data, data_len) \
//    do { \
//        MY_RAW_LOG("\r\nble qiot dump: %s, length: %d\r\n", hex_name, data_len); \
//        MY_RAW_HEXDUMP_(data, data_len); \
//    } while(0)

// or use your own hexdump function with same definition
// void ble_qiot_log_hex(e_ble_qiot_log_level level, const char *hex_name, const char *data, uint32_t data_len);
#endif  // BLE_QIOT_USER_DEFINE_HEXDUMP

// Macro for logging a formatted string, the function must printf raw string without any color, prefix, newline or
// timestamp
#define BLE_QIOT_LOG_PRINT(...) printf(__VA_ARGS__)

#define BLE_QIOT_LLSYNC_STANDARD    1   // support llsync standard
#if BLE_QIOT_LLSYNC_STANDARD
// some users hope to confirm on the device before the binding, set BLE_QIOT_SECURE_BIND is 1 to enable the secure
// binding and enable secure bind in iot-explorer console. When the server is bound, the device callback ble_secure_bind_user_cb()
// will be triggered, the user agree or refuse connect by ble_secure_bind_user_confirm(). If the device does not respond
// and the connection timeout, or the user cancel the connection in Tencent Lianlian, a notify will received in function
// ble_secure_bind_user_notify().
#define BLE_QIOT_SECURE_BIND 0
#if BLE_QIOT_SECURE_BIND
#define BLE_QIOT_BIND_WAIT_TIME 60
#endif //BLE_QIOT_SECURE_BIND

// some sdk info needs to stored on the device and the address is up to you
#define BLE_QIOT_RECORD_FLASH_ADDR 5

// define user develop version, pick from "a-zA-Z0-9.-_" and length limits 1～32 bytes.
// must be consistent with the firmware version that user write in the iot-explorer console
// refer https://cloud.tencent.com/document/product/1081/40296
#define BLE_QIOT_USER_DEVELOPER_VERSION "0.0.1"

#define BLE_QIOT_SUPPORT_OTA 1  // 1 is support ota, others not
#if BLE_QIOT_SUPPORT_OTA
#define BLE_QIOT_SUPPORT_RESUMING 0  // 1 is support resuming, others not
#if BLE_QIOT_SUPPORT_RESUMING
// storage ota info in the flash if support resuming ota file
#define BLE_QIOT_OTA_INFO_FLASH_ADDR 10
#endif //BLE_QIOT_SUPPORT_RESUMING

#define BLE_QIOT_TOTAL_PACKAGES 0x12  // the total package numbers in a loop
#define BLE_QIOT_PACKAGE_LENGTH 0xb0  // the user data length in package, ble_get_user_data_mtu_size() - 3 is the max
#define BLE_QIOT_RETRY_TIMEOUT  0x20   // the max interval between two packages, unit: second
// the time spent for device reboot, the server waiting the device version reported after upgrade. unit: second
#define BLE_QIOT_REBOOT_TIME      20
#define BLE_QIOT_PACKAGE_INTERVAL 0xa  // the interval between two packages send by the server
// the package from the server will storage in the buffer, write the buffer to the flash at one time when the buffer
// overflow. reduce the flash write can speed up file download, we suggest the BLE_QIOT_OTA_BUF_SIZE is multiples
// of BLE_QIOT_PACKAGE_LENGTH and equal flash page size
#define BLE_QIOT_OTA_BUF_SIZE (512 * 4)
#endif //BLE_QIOT_SUPPORT_OTA
#endif //BLE_QIOT_LLSYNC_STANDARD

#define BLE_QIOT_LLSYNC_CONFIG_NET  (!BLE_QIOT_LLSYNC_STANDARD)   // support llsync configure network

#if (1 == BLE_QIOT_LLSYNC_STANDARD) && (1 == BLE_QIOT_LLSYNC_CONFIG_NET)
#error "llsync standard and llsync configure network is incompatible"
#endif

#ifdef __cplusplus
}
#endif

#endif  // QCLOUD_BLE_QIOT_CONFIG_H
