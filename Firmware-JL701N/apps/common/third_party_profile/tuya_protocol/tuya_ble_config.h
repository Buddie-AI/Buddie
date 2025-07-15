/**
 * \file tuya_ble_config.h
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


#ifndef TUYA_BLE_CONFIG_H__
#define TUYA_BLE_CONFIG_H__


#if defined(CUSTOMIZED_TUYA_BLE_CONFIG_FILE)
#include CUSTOMIZED_TUYA_BLE_CONFIG_FILE
#endif


/*
 * If the application has a customized production test project(file),please define CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_FILE in CUSTOMIZED_TUYA_BLE_CONFIG_FILE.
 */
#ifndef CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE
#undef  CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE
#endif

/*
 * If needed,please define CUSTOMIZED_TUYA_BLE_APP_UART_COMMON_FILE in CUSTOMIZED_TUYA_BLE_CONFIG_FILE.
 */
#ifndef CUSTOMIZED_TUYA_BLE_APP_UART_COMMON_HEADER_FILE
#undef  CUSTOMIZED_TUYA_BLE_APP_UART_COMMON_HEADER_FILE
#endif


/*
 * If needed,please define TUYA_BLE_PORT_PLATFORM_HEADER_FILE in CUSTOMIZED_TUYA_BLE_CONFIG_FILE.
 */
#ifndef TUYA_BLE_PORT_PLATFORM_HEADER_FILE
#undef  TUYA_BLE_PORT_PLATFORM_HEADER_FILE
#endif

/*
 * If using an OS, be sure to call the tuya api functions and SDK-related queues within the same task
 */
#ifndef TUYA_BLE_USE_OS
#define TUYA_BLE_USE_OS 1
#endif
/*
 * If using an OS, tuya ble sdk  will create a task autonomously to process ble event.
 */

#if TUYA_BLE_USE_OS

#ifndef TUYA_BLE_SELF_BUILT_TASK
#define TUYA_BLE_SELF_BUILT_TASK  1
#endif

#if TUYA_BLE_SELF_BUILT_TASK

#ifndef TUYA_BLE_TASK_PRIORITY
#define TUYA_BLE_TASK_PRIORITY  1
#endif

#ifndef TUYA_BLE_TASK_STACK_SIZE
#define TUYA_BLE_TASK_STACK_SIZE  256 * 10   //!<  Task stack size ，application do not change this default value
#endif

#endif

#endif
/*
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE                   ble normal
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE     device register from ble
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_MESH                  ble mesh
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G              wifi_2.4g
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_5G               wifi_5g
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_ZIGBEE                zigbee
 * TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_NB                    nb-iot
 * @note:
 * for example (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_MESH|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G)
 */
//#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_24G|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_WIFI_5G)
#ifndef TUYA_BLE_DEVICE_COMMUNICATION_ABILITY
#define TUYA_BLE_DEVICE_COMMUNICATION_ABILITY  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_BLE|TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE)
#endif

/*
 * Whether it is a shared device.
 */
#ifndef TUYA_BLE_DEVICE_SHARED
#define TUYA_BLE_DEVICE_SHARED  0
#endif

/*
 * If it is 1, then sdk need to perform the unbind operation,otherwise sdk do not need.
 */
#ifndef TUYA_BLE_DEVICE_UNBIND_MODE
#define TUYA_BLE_DEVICE_UNBIND_MODE  1
#endif

/*
 * If TUYA_BLE_WIFI_DEVICE_REGISTER_MODE is 1,Mobile app must first sends instructions to query network status after sending the pairing request .
 */
#ifndef TUYA_BLE_WIFI_DEVICE_REGISTER_MODE
#define TUYA_BLE_WIFI_DEVICE_REGISTER_MODE  1
#endif

/*
 * if 1 ,tuya ble sdk authorization self-management
 */
#ifndef TUYA_BLE_DEVICE_AUTH_SELF_MANAGEMENT
#define  TUYA_BLE_DEVICE_AUTH_SELF_MANAGEMENT   1
#endif

/*
 * TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY     encrypt with auth key
 * TUYA_BLE_SECURE_CONNECTION_WITH_ECC          encrypt with ECDH
 * TUYA_BLE_SECURE_CONNECTION_WTIH_PASSTHROUGH  no encrypt
 * TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION  advanced encrypt(security chip ) with auth key
 * @note : only choose one
*/
#ifndef  TUYA_BLE_SECURE_CONNECTION_TYPE
#define  TUYA_BLE_SECURE_CONNECTION_TYPE  TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY
#endif

/*
 * MACRO for advanced encryption,if 1 will perform two-way authentication on every connection,otherwise perform certification at the first registration.
 */
#if (TUYA_BLE_SECURE_CONNECTION_TYPE==TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)

#ifndef TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT
#define TUYA_BLE_ADVANCED_ENCRYPTION_AUTH_ON_CONNECT 0
#endif

#ifndef TUYA_BLE_INCLUDE_CJSON_COMPONENTS
#define TUYA_BLE_INCLUDE_CJSON_COMPONENTS 1
#endif

#endif

/*
 * if 1 ,ble sdk will update mac address with with the address of the authorization information.
 */
#ifndef  TUYA_BLE_DEVICE_MAC_UPDATE
#define  TUYA_BLE_DEVICE_MAC_UPDATE         0
#endif

/*
 * if 1 ,after update mac will reset.
 */
#ifndef  TUYA_BLE_DEVICE_MAC_UPDATE_RESET
#define  TUYA_BLE_DEVICE_MAC_UPDATE_RESET   0
#endif

/*
 * if defined ,ble sdk use platform memory heap.
 */
#ifndef  TUYA_BLE_USE_PLATFORM_MEMORY_HEAP
#define  TUYA_BLE_USE_PLATFORM_MEMORY_HEAP   0
#endif


/*
 * if defined ,include UART module
 */
//#define TUYA_BLE_UART


/*
 * if defined ,include product test module
 */
//#define TUYA_BLE_PRODUCTION_TEST


/*
 * gatt mtu max sizes
 */

#ifndef TUYA_BLE_DATA_MTU_MAX
#define TUYA_BLE_DATA_MTU_MAX  243
#endif

/*
 * if defined ,enable sdk log output
 */

#ifndef TUYA_BLE_LOG_ENABLE
#define TUYA_BLE_LOG_ENABLE 1
#endif

#ifndef TUYA_BLE_LOG_COLORS_ENABLE
#define TUYA_BLE_LOG_COLORS_ENABLE  0
#endif

#ifndef TUYA_BLE_LOG_LEVEL
#define TUYA_BLE_LOG_LEVEL  TUYA_BLE_LOG_LEVEL_DEBUG
#endif


/*
 * if defined ,enable app log output
 */

#ifndef TUYA_APP_LOG_ENABLE
#define TUYA_APP_LOG_ENABLE 1
#endif

#ifndef TUYA_APP_LOG_COLORS_ENABLE
#define TUYA_APP_LOG_COLORS_ENABLE  0
#endif

#ifndef TUYA_APP_LOG_LEVEL
#define TUYA_APP_LOG_LEVEL  TUYA_APP_LOG_LEVEL_DEBUG
#endif

/*
 * If it is 1, then sdk need to request beacon key from app when binding .
 */
#ifndef TUYA_BLE_BEACON_KEY_ENABLE
#define TUYA_BLE_BEACON_KEY_ENABLE  0
#endif

/*
 * Enable the weather module.
 */
#ifndef TUYA_BLE_FEATURE_WEATHER_ENABLE
#define TUYA_BLE_FEATURE_WEATHER_ENABLE  0
#endif

/*
 * Enable support the link layer encryption.
 */
#ifndef TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE
#define TUYA_BLE_LINK_LAYER_ENCRYPTION_SUPPORT_ENABLE  0
#endif

/*
 * Whether to automatically perform a time request after the connection is established.
 * - 0  Not request.
 * - 1  Request cloud time.
 * - 2  Request phone local time.
 */
#ifndef TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE
#define TUYA_BLE_AUTO_REQUEST_TIME_CONFIGURE  1
#endif


//nv
/* The minimum size of flash erasure. May be a flash sector size. */
#ifndef TUYA_NV_ERASE_MIN_SIZE
#define TUYA_NV_ERASE_MIN_SIZE         (0x100)
#endif
/* the flash write granularity, unit: byte*/
#ifndef TUYA_NV_WRITE_GRAN
#define TUYA_NV_WRITE_GRAN             (4)
#endif
/* start address */
#ifndef TUYA_NV_START_ADDR
#define TUYA_NV_START_ADDR              0
#endif

/* area size. */
#ifndef TUYA_NV_AREA_SIZE
#define TUYA_NV_AREA_SIZE              (4*TUYA_NV_ERASE_MIN_SIZE)
#endif

/*MACRO for production test module*/
#ifndef TUYA_BLE_APP_VERSION_STRING
#define TUYA_BLE_APP_VERSION_STRING  "1.0"
#endif

#ifndef TUYA_BLE_APP_BUILD_FIRMNAME_STRING
#define TUYA_BLE_APP_BUILD_FIRMNAME_STRING  "tuya_ble_sdk_app_demo_xxx"
#endif

#ifndef TUYA_BLE_APP_FIRMWARE_KEY
#define TUYA_BLE_APP_FIRMWARE_KEY  ""
#endif

#endif



