/**
 * \file tuya_ble_log.h
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

#ifndef TUYA_BLE_LOG_H__
#define TUYA_BLE_LOG_H__

#include "tuya_ble_stdlib.h"
#include "tuya_ble_internal_config.h"
#include "tuya_ble_log_internal.h"

#if TUYA_BLE_LOG_ENABLE


#ifndef TUYA_BLE_LOG_LEVEL
#define TUYA_BLE_LOG_LEVEL              TUYA_BLE_LOG_LEVEL_DEBUG
#endif

#ifndef TUYA_BLE_LOG_COLORS_ENABLE
#define TUYA_BLE_LOG_COLORS_ENABLE      0
#endif


#define TUYA_BLE_LOG_ERROR(...)                       TUYA_BLE_LOG_INTERNAL_ERROR(__VA_ARGS__)
#define TUYA_BLE_LOG_WARNING(...)                     TUYA_BLE_LOG_INTERNAL_WARNING( __VA_ARGS__)
#define TUYA_BLE_LOG_INFO(...)                        TUYA_BLE_LOG_INTERNAL_INFO( __VA_ARGS__)
#define TUYA_BLE_LOG_DEBUG(...)                       TUYA_BLE_LOG_INTERNAL_DEBUG( __VA_ARGS__)

#define TUYA_BLE_LOG_HEXDUMP_ERROR(...)               TUYA_BLE_LOG_INTERNAL_HEXDUMP_ERROR(__VA_ARGS__)
#define TUYA_BLE_LOG_HEXDUMP_WARNING(...)             TUYA_BLE_LOG_INTERNAL_HEXDUMP_WARNING( __VA_ARGS__)
#define TUYA_BLE_LOG_HEXDUMP_INFO(...)                TUYA_BLE_LOG_INTERNAL_HEXDUMP_INFO( __VA_ARGS__)
#define TUYA_BLE_LOG_HEXDUMP_DEBUG(...)               TUYA_BLE_LOG_INTERNAL_HEXDUMP_DEBUG( __VA_ARGS__)

#else

#define TUYA_BLE_LOG_ERROR(...)
#define TUYA_BLE_LOG_WARNING(...)
#define TUYA_BLE_LOG_INFO(...)
#define TUYA_BLE_LOG_DEBUG(...)

#define TUYA_BLE_LOG_HEXDUMP_ERROR(...)
#define TUYA_BLE_LOG_HEXDUMP_WARNING(...)
#define TUYA_BLE_LOG_HEXDUMP_INFO(...)
#define TUYA_BLE_LOG_HEXDUMP_DEBUG(...)

#endif //


#if TUYA_APP_LOG_ENABLE

#ifndef TUYA_APP_LOG_LEVEL
#define TUYA_APP_LOG_LEVEL              TUYA_APP_LOG_LEVEL_DEBUG
#endif

#ifndef TUYA_APP_LOG_COLORS_ENABLE
#define TUYA_APP_LOG_COLORS_ENABLE      0
#endif


#define TUYA_APP_LOG_ERROR(...)                       TUYA_APP_LOG_INTERNAL_ERROR(__VA_ARGS__)
#define TUYA_APP_LOG_WARNING(...)                     TUYA_APP_LOG_INTERNAL_WARNING( __VA_ARGS__)
#define TUYA_APP_LOG_INFO(...)                        TUYA_APP_LOG_INTERNAL_INFO( __VA_ARGS__)
#define TUYA_APP_LOG_DEBUG(...)                       TUYA_APP_LOG_INTERNAL_DEBUG( __VA_ARGS__)

#define TUYA_APP_LOG_HEXDUMP_ERROR(...)               TUYA_APP_LOG_INTERNAL_HEXDUMP_ERROR(__VA_ARGS__)
#define TUYA_APP_LOG_HEXDUMP_WARNING(...)             TUYA_APP_LOG_INTERNAL_HEXDUMP_WARNING( __VA_ARGS__)
#define TUYA_APP_LOG_HEXDUMP_INFO(...)                TUYA_APP_LOG_INTERNAL_HEXDUMP_INFO( __VA_ARGS__)
#define TUYA_APP_LOG_HEXDUMP_DEBUG(...)               TUYA_APP_LOG_INTERNAL_HEXDUMP_DEBUG( __VA_ARGS__)

#else

#define TUYA_APP_LOG_ERROR(...)
#define TUYA_APP_LOG_WARNING(...)
#define TUYA_APP_LOG_INFO(...)
#define TUYA_APP_LOG_DEBUG(...)

#define TUYA_APP_LOG_HEXDUMP_ERROR(...)
#define TUYA_APP_LOG_HEXDUMP_WARNING(...)
#define TUYA_APP_LOG_HEXDUMP_INFO(...)
#define TUYA_APP_LOG_HEXDUMP_DEBUG(...)

#endif //


#ifdef TUYA_BLE_ASSERT

#define TUYA_BLE_ERR_HANDLER(ERR_CODE)                                       \
    do                                                                 \
    {                                                                  \
        TUYA_BLE_LOG_ERROR("Error code 0x%04X <%d>.  %s:%d\n", ERR_CODE, ERR_CODE, (uint32_t)__FILE__, __LINE__); \
    } while (0)

#define TUYA_BLE_ERR_CHECK(ERR_CODE)                              \
    do                                                      \
    {                                                       \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);         \
        if (LOCAL_ERR_CODE != 0)                            \
        {                                                   \
            TUYA_BLE_ERR_HANDLER(LOCAL_ERR_CODE);                 \
        }                                                   \
    } while (0)

#define TUYA_BLE_ERR_TEST(ERR_CODE, EXPECT)                       \
    do                                                      \
    {                                                       \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);         \
        if (LOCAL_ERR_CODE != (EXPECT))                     \
        {                                                   \
            TUYA_BLE_ERR_HANDLER(LOCAL_ERR_CODE);                 \
        }                                                   \
    } while (0)
#else //

#define TUYA_BLE_ERR_CHECK(ERR_CODE)

#define TUYA_BLE_ERR_TEST(ERR_CODE, EXPECT)

#endif //



#endif // TUYA_BLE_LOG_H__
