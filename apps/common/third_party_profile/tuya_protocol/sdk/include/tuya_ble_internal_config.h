/**
 * \file tuya_ble_internal_config.h
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


#ifndef TUYA_BLE_INTERNAL_CONFIG_H__
#define TUYA_BLE_INTERNAL_CONFIG_H__


#include "tuya_ble_config.h"
#include "tuya_ble_port.h"


#define MAX_NUMBER_OF_TUYA_MESSAGE        16      //!<  tuya ble message queue size


//BLE Communication protocol version v4.2
#define TUYA_BLE_PROTOCOL_VERSION_HIGN   0x04
#define TUYA_BLE_PROTOCOL_VERSION_LOW    0x02

#define TUYA_BLE_DP_WRITE_CURRENT_VERSION   0

#if (TUYA_BLE_PROTOCOL_VERSION_HIGN>=4)

#define TUYA_BLE_AIR_FRAME_MAX                    1536

#define TUYA_UART_RECEIVE_MAX_DP_DATA_LEN         (512+4)

#define TUYA_UART_RECEIVE_MAX_DP_BUFFER_DATA_LEN  (512+4)

#define TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN          (512+4)

#define TUYA_BLE_SEND_MAX_DP_DATA_LEN              TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN

#define TUYA_BLE_SEND_MAX_DATA_LEN                 (((TUYA_BLE_AIR_FRAME_MAX-17)/16)*16-14)

#define TUYA_BLE_RECEIVE_MAX_DATA_LEN              TUYA_BLE_SEND_MAX_DATA_LEN

#else

#define TUYA_BLE_ADVANCED_ENCRYPTION_DEVICE 0

#define TUYA_BLE_AIR_FRAME_MAX                    1024

#define TUYA_UART_RECEIVE_MAX_DP_DATA_LEN         (255+4)
#define TUYA_UART_RECEIVE_MAX_DP_BUFFER_DATA_LEN  (255+4)

#define TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN          (255+3)

#define TUYA_BLE_SEND_MAX_DP_DATA_LEN              TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN

#define TUYA_BLE_REPORT_MAX_DP_DATA_LEN           TUYA_BLE_RECEIVE_MAX_DP_DATA_LEN

#define TUYA_BLE_TRANSMISSION_MAX_DATA_LEN       (TUYA_BLE_AIR_FRAME_MAX-29)

#define TUYA_BLE_SEND_MAX_DATA_LEN                TUYA_BLE_TRANSMISSION_MAX_DATA_LEN

#define TUYA_BLE_RECEIVE_MAX_DATA_LEN             TUYA_BLE_SEND_MAX_DATA_LEN

#endif


#define TUYA_BLE_GATT_SEND_DATA_QUEUE_SIZE  (MAX_NUMBER_OF_TUYA_MESSAGE*3)


#if (TUYA_BLE_USE_PLATFORM_MEMORY_HEAP==0)
/**
 * MACRO for memory management
 */
#define TUYA_BLE_TOTAL_HEAP_SIZE          5120

#if (TUYA_BLE_TOTAL_HEAP_SIZE<5120)
#define TUYA_BLE_BULK_DATA_MAX_READ_BLOCK_SIZE 512
#else
#define TUYA_BLE_BULK_DATA_MAX_READ_BLOCK_SIZE 1024
#endif

#else
#define TUYA_BLE_BULK_DATA_MAX_READ_BLOCK_SIZE 512
#endif


#ifndef TUYA_BLE_MAX_CALLBACKS
#define TUYA_BLE_MAX_CALLBACKS 1
#endif


#define	    TUYA_BLE_AUTH_FLASH_ADDR             (TUYA_NV_START_ADDR)
#define	    TUYA_BLE_AUTH_FLASH_BACKUP_ADDR      (TUYA_NV_START_ADDR+TUYA_NV_ERASE_MIN_SIZE)

#define	    TUYA_BLE_SYS_FLASH_ADDR              (TUYA_NV_START_ADDR+TUYA_NV_ERASE_MIN_SIZE*2)
#define	    TUYA_BLE_SYS_FLASH_BACKUP_ADDR       (TUYA_NV_START_ADDR+TUYA_NV_ERASE_MIN_SIZE*3)


/**
 * 1 - device register from ble  0 - from others
 * @note:
 */
#define TUYA_BLE_DEVICE_REGISTER_FROM_BLE  (TUYA_BLE_DEVICE_COMMUNICATION_ABILITY&TUYA_BLE_DEVICE_COMMUNICATION_ABILITY_REGISTER_FROM_BLE)


#define TUYA_BLE_DEVICE_AUTH_DATA_STORE                     TUYA_BLE_DEVICE_AUTH_SELF_MANAGEMENT

/**
 * if 1 ,tuya ble sdk authorization storage into extern medium, For example security chip
 */
#define TUYA_BLE_DEVICE_AUTH_DATA_STORE_EXT_MEDIUM          0


/**
 * if 1 ,used ble dongle for product test, the transmit datas will encryption
 */
#define TUYA_BLE_PROD_TEST_SUPPORT_ENCRYPTION				0


/**
 * if >=1, the product support oem, when execute prod test will write in pid and json config file
 *
 * Noticed!!! if enable the macro, need to realization 'tuya_ble_status_t tuya_ble_prod_storage_oem_info(uint8_t *para,uint16_t len)'
 * function yourself at the custom_app_product_test.c
 */
#define TUYA_BLE_PROD_SUPPORT_OEM_TYPE						TUYA_BLE_PROD_OEM_TYPE_NONE


#if (TUYA_BLE_SYS_FLASH_BACKUP_ADDR>=(TUYA_NV_AREA_SIZE+TUYA_NV_START_ADDR))
#error "Storage Memory overflow!"
#endif


#endif
