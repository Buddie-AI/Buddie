/**
 * \file tuya_ble_log_internal.h
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

#ifndef TUYA_BLE_LOG_INTERNAL__H__
#define TUYA_BLE_LOG_INTERNAL__H__

#include "tuya_ble_port.h"
#include "tuya_ble_internal_config.h"


#if !defined(TUYA_BLE_PRINTF)
#error "Not defined printf function."
#elif !defined(TUYA_BLE_HEXDUMP)
#error "Not defined hexdump function."
#endif


#ifndef TUYA_BLE_LOG_COLORS_ENABLE
#define TUYA_BLE_LOG_COLORS_ENABLE      0
#endif

#ifndef TUYA_BLE_LOG_MODULE_NAME
#define TUYA_BLE_LOG_MODULE_NAME        "TUYA_BLE"
#endif



#if TUYA_BLE_LOG_COLORS_ENABLE
#define TUYA_BLE_LOG_COLOR_DEFAULT      "\x1B[0m"
#define TUYA_BLE_LOG_COLOR_BLACK        "\x1B[1;30m"
#define TUYA_BLE_LOG_COLOR_RED          "\x1B[1;31m"
#define TUYA_BLE_LOG_COLOR_GREEN        "\x1B[1;32m"
#define TUYA_BLE_LOG_COLOR_YELLOW       "\x1B[1;33m"
#define TUYA_BLE_LOG_COLOR_BLUE         "\x1B[1;34m"
#define TUYA_BLE_LOG_COLOR_MAGENTA      "\x1B[1;35m"
#define TUYA_BLE_LOG_COLOR_CYAN         "\x1B[1;36m"
#define TUYA_BLE_LOG_COLOR_WHITE        "\x1B[1;37m"
#else
#define TUYA_BLE_LOG_COLOR_DEFAULT
#define TUYA_BLE_LOG_COLOR_BLACK
#define TUYA_BLE_LOG_COLOR_RED
#define TUYA_BLE_LOG_COLOR_GREEN
#define TUYA_BLE_LOG_COLOR_YELLOW
#define TUYA_BLE_LOG_COLOR_BLUE
#define TUYA_BLE_LOG_COLOR_MAGENTA
#define TUYA_BLE_LOG_COLOR_CYAN
#define TUYA_BLE_LOG_COLOR_WHITE
#endif

#define TUYA_BLE_LOG_ERROR_COLOR   TUYA_BLE_LOG_COLOR_RED
#define TUYA_BLE_LOG_WARNING_COLOR TUYA_BLE_LOG_COLOR_YELLOW
#define TUYA_BLE_LOG_INFO_COLOR    TUYA_BLE_LOG_COLOR_DEFAULT
#define TUYA_BLE_LOG_DEBUG_COLOR   TUYA_BLE_LOG_COLOR_GREEN
#define TUYA_BLE_LOG_DEFAULT_COLOR TUYA_BLE_LOG_COLOR_DEFAULT

#define TUYA_BLE_LOG_BREAK      ": "

//#define TUYA_BLE_LOG_CRLF      "\r\n"
#define TUYA_BLE_LOG_CRLF ""

#define TUYA_BLE_ERROR_PREFIX   TUYA_BLE_LOG_ERROR_COLOR "[E] " TUYA_BLE_LOG_MODULE_NAME TUYA_BLE_LOG_BREAK
#define TUYA_BLE_WARNING_PREFIX TUYA_BLE_LOG_WARNING_COLOR "[W] " TUYA_BLE_LOG_MODULE_NAME TUYA_BLE_LOG_BREAK
#define TUYA_BLE_INFO_PREFIX    TUYA_BLE_LOG_INFO_COLOR "[I] " TUYA_BLE_LOG_MODULE_NAME TUYA_BLE_LOG_BREAK
#define TUYA_BLE_DEBUG_PREFIX   TUYA_BLE_LOG_DEBUG_COLOR "[D] " TUYA_BLE_LOG_MODULE_NAME TUYA_BLE_LOG_BREAK


#define TUYA_BLE_LOG_INTERNAL_ERROR(_fmt_, ...)                                       \
do {                                                                            \
    if (TUYA_BLE_LOG_LEVEL >= TUYA_BLE_LOG_LEVEL_ERROR)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_BLE_ERROR_PREFIX _fmt_ TUYA_BLE_LOG_DEFAULT_COLOR TUYA_BLE_LOG_CRLF, ##__VA_ARGS__);   \
    }                                                                           \
} while(0)

#define TUYA_BLE_LOG_INTERNAL_WARNING(_fmt_, ...)                                     \
do {                                                                            \
    if (TUYA_BLE_LOG_LEVEL >= TUYA_BLE_LOG_LEVEL_WARNING)                                   \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_BLE_WARNING_PREFIX _fmt_ TUYA_BLE_LOG_DEFAULT_COLOR TUYA_BLE_LOG_CRLF, ##__VA_ARGS__); \
    }                                                                           \
} while(0)

#define TUYA_BLE_LOG_INTERNAL_INFO(_fmt_, ...)                                        \
do {                                                                            \
    if (TUYA_BLE_LOG_LEVEL >= TUYA_BLE_LOG_LEVEL_INFO)                                      \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_BLE_INFO_PREFIX _fmt_ TUYA_BLE_LOG_DEFAULT_COLOR TUYA_BLE_LOG_CRLF, ##__VA_ARGS__);    \
    }                                                                           \
} while(0)

#define TUYA_BLE_LOG_INTERNAL_DEBUG(_fmt_, ...)                                       \
do {                                                                            \
    if (TUYA_BLE_LOG_LEVEL >= TUYA_BLE_LOG_LEVEL_DEBUG)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_BLE_DEBUG_PREFIX _fmt_ TUYA_BLE_LOG_DEFAULT_COLOR TUYA_BLE_LOG_CRLF, ##__VA_ARGS__);   \
    }                                                                           \
} while(0)

#define TUYA_BLE_LOG_INTERNAL_HEXDUMP_ERROR(_fmt_, p_data, len)                                    \
do {                                                                            \
    if (TUYA_BLE_LOG_LEVEL >= TUYA_BLE_LOG_LEVEL_ERROR)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_BLE_ERROR_PREFIX _fmt_ TUYA_BLE_LOG_DEFAULT_COLOR " [len=%d] :" TUYA_BLE_LOG_CRLF,len ); \
        TUYA_BLE_HEXDUMP(p_data, len);                                                \
        TUYA_BLE_PRINTF(TUYA_BLE_LOG_CRLF);                                      \
    }                                                                           \
} while(0)

#define TUYA_BLE_LOG_INTERNAL_HEXDUMP_WARNING(_fmt_, p_data, len)                                    \
do {                                                                            \
    if (TUYA_BLE_LOG_LEVEL >= TUYA_BLE_LOG_LEVEL_WARNING)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_BLE_WARNING_PREFIX _fmt_ TUYA_BLE_LOG_DEFAULT_COLOR " [len=%d] :" TUYA_BLE_LOG_CRLF,len); \
        TUYA_BLE_HEXDUMP(p_data, len);                                                \
        TUYA_BLE_PRINTF(TUYA_BLE_LOG_CRLF);                                     \
    }                                                                           \
} while(0)

#define TUYA_BLE_LOG_INTERNAL_HEXDUMP_INFO(_fmt_, p_data, len)                                    \
do {                                                                            \
    if (TUYA_BLE_LOG_LEVEL >= TUYA_BLE_LOG_LEVEL_INFO)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_BLE_INFO_PREFIX _fmt_ TUYA_BLE_LOG_DEFAULT_COLOR " [len=%d] :" TUYA_BLE_LOG_CRLF,len); \
        TUYA_BLE_HEXDUMP(p_data, len);                                                \
        TUYA_BLE_PRINTF(TUYA_BLE_LOG_CRLF);                                      \
    }                                                                           \
} while(0)

#define TUYA_BLE_LOG_INTERNAL_HEXDUMP_DEBUG(_fmt_, p_data, len)                                    \
do {                                                                            \
    if (TUYA_BLE_LOG_LEVEL >= TUYA_BLE_LOG_LEVEL_DEBUG)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_BLE_DEBUG_PREFIX _fmt_ TUYA_BLE_LOG_DEFAULT_COLOR " [len=%d] :" TUYA_BLE_LOG_CRLF,len); \
        TUYA_BLE_HEXDUMP(p_data, len);                                                \
        TUYA_BLE_PRINTF(TUYA_BLE_LOG_CRLF);                                      \
    }                                                                           \
} while(0)



/* TUYA APP LOG */

#ifndef TUYA_APP_LOG_COLORS_ENABLE
#define TUYA_APP_LOG_COLORS_ENABLE      0
#endif

#ifndef TUYA_APP_LOG_MODULE_NAME
#define TUYA_APP_LOG_MODULE_NAME        "TUYA_APP"
#endif



#if TUYA_APP_LOG_COLORS_ENABLE
#define TUYA_APP_LOG_COLOR_DEFAULT      "\x1B[0m"
#define TUYA_APP_LOG_COLOR_BLACK        "\x1B[1;30m"
#define TUYA_APP_LOG_COLOR_RED          "\x1B[1;31m"
#define TUYA_APP_LOG_COLOR_GREEN        "\x1B[1;32m"
#define TUYA_APP_LOG_COLOR_YELLOW       "\x1B[1;33m"
#define TUYA_APP_LOG_COLOR_BLUE         "\x1B[1;34m"
#define TUYA_APP_LOG_COLOR_MAGENTA      "\x1B[1;35m"
#define TUYA_APP_LOG_COLOR_CYAN         "\x1B[1;36m"
#define TUYA_APP_LOG_COLOR_WHITE        "\x1B[1;37m"
#else
#define TUYA_APP_LOG_COLOR_DEFAULT
#define TUYA_APP_LOG_COLOR_BLACK
#define TUYA_APP_LOG_COLOR_RED
#define TUYA_APP_LOG_COLOR_GREEN
#define TUYA_APP_LOG_COLOR_YELLOW
#define TUYA_APP_LOG_COLOR_BLUE
#define TUYA_APP_LOG_COLOR_MAGENTA
#define TUYA_APP_LOG_COLOR_CYAN
#define TUYA_APP_LOG_COLOR_WHITE
#endif

#define TUYA_APP_LOG_ERROR_COLOR   TUYA_APP_LOG_COLOR_RED
#define TUYA_APP_LOG_WARNING_COLOR TUYA_APP_LOG_COLOR_YELLOW
#define TUYA_APP_LOG_INFO_COLOR    TUYA_APP_LOG_COLOR_DEFAULT
#define TUYA_APP_LOG_DEBUG_COLOR   TUYA_APP_LOG_COLOR_GREEN
#define TUYA_APP_LOG_DEFAULT_COLOR TUYA_APP_LOG_COLOR_DEFAULT

#define TUYA_APP_LOG_BREAK      ": "

//#define TUYA_APP_LOG_CRLF      "\r\n"
#define TUYA_APP_LOG_CRLF ""

#define TUYA_APP_ERROR_PREFIX   TUYA_APP_LOG_ERROR_COLOR "[E] " TUYA_APP_LOG_MODULE_NAME TUYA_APP_LOG_BREAK
#define TUYA_APP_WARNING_PREFIX TUYA_APP_LOG_WARNING_COLOR "[W] " TUYA_APP_LOG_MODULE_NAME TUYA_APP_LOG_BREAK
#define TUYA_APP_INFO_PREFIX    TUYA_APP_LOG_INFO_COLOR "[I] " TUYA_APP_LOG_MODULE_NAME TUYA_APP_LOG_BREAK
#define TUYA_APP_DEBUG_PREFIX   TUYA_APP_LOG_DEBUG_COLOR "[D] " TUYA_APP_LOG_MODULE_NAME TUYA_APP_LOG_BREAK


#define TUYA_APP_LOG_INTERNAL_ERROR(_fmt_, ...)                                       \
do {                                                                            \
    if (TUYA_APP_LOG_LEVEL >= TUYA_APP_LOG_LEVEL_ERROR)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_APP_ERROR_PREFIX _fmt_ TUYA_APP_LOG_DEFAULT_COLOR TUYA_APP_LOG_CRLF, ##__VA_ARGS__);   \
    }                                                                           \
} while(0)

#define TUYA_APP_LOG_INTERNAL_WARNING(_fmt_, ...)                                     \
do {                                                                            \
    if (TUYA_APP_LOG_LEVEL >= TUYA_APP_LOG_LEVEL_WARNING)                                   \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_APP_WARNING_PREFIX _fmt_ TUYA_APP_LOG_DEFAULT_COLOR TUYA_APP_LOG_CRLF, ##__VA_ARGS__); \
    }                                                                           \
} while(0)

#define TUYA_APP_LOG_INTERNAL_INFO(_fmt_, ...)                                        \
do {                                                                            \
    if (TUYA_APP_LOG_LEVEL >= TUYA_APP_LOG_LEVEL_INFO)                                      \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_APP_INFO_PREFIX _fmt_ TUYA_APP_LOG_DEFAULT_COLOR TUYA_APP_LOG_CRLF, ##__VA_ARGS__);    \
    }                                                                           \
} while(0)

#define TUYA_APP_LOG_INTERNAL_DEBUG(_fmt_, ...)                                       \
do {                                                                            \
    if (TUYA_APP_LOG_LEVEL >= TUYA_APP_LOG_LEVEL_DEBUG)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_APP_DEBUG_PREFIX _fmt_ TUYA_APP_LOG_DEFAULT_COLOR TUYA_APP_LOG_CRLF, ##__VA_ARGS__);   \
    }                                                                           \
} while(0)

#define TUYA_APP_LOG_INTERNAL_HEXDUMP_ERROR(_fmt_, p_data, len)                                    \
do {                                                                            \
    if (TUYA_APP_LOG_LEVEL >= TUYA_APP_LOG_LEVEL_ERROR)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_APP_ERROR_PREFIX _fmt_ TUYA_APP_LOG_DEFAULT_COLOR " [len=%d] :" TUYA_APP_LOG_CRLF,len ); \
        TUYA_BLE_HEXDUMP(p_data, len);                                                \
        TUYA_BLE_PRINTF(TUYA_APP_LOG_CRLF);                                      \
    }                                                                           \
} while(0)

#define TUYA_APP_LOG_INTERNAL_HEXDUMP_WARNING(_fmt_, p_data, len)                                    \
do {                                                                            \
    if (TUYA_APP_LOG_LEVEL >= TUYA_APP_LOG_LEVEL_WARNING)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_APP_WARNING_PREFIX _fmt_ TUYA_APP_LOG_DEFAULT_COLOR " [len=%d] :" TUYA_APP_LOG_CRLF,len); \
        TUYA_BLE_HEXDUMP(p_data, len);                                                \
        TUYA_BLE_PRINTF(TUYA_APP_LOG_CRLF);                                     \
    }                                                                           \
} while(0)

#define TUYA_APP_LOG_INTERNAL_HEXDUMP_INFO(_fmt_, p_data, len)                                    \
do {                                                                            \
    if (TUYA_APP_LOG_LEVEL >= TUYA_APP_LOG_LEVEL_INFO)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_APP_INFO_PREFIX _fmt_ TUYA_APP_LOG_DEFAULT_COLOR " [len=%d] :" TUYA_APP_LOG_CRLF,len); \
        TUYA_BLE_HEXDUMP(p_data, len);                                                \
        TUYA_BLE_PRINTF(TUYA_APP_LOG_CRLF);                                      \
    }                                                                           \
} while(0)

#define TUYA_APP_LOG_INTERNAL_HEXDUMP_DEBUG(_fmt_, p_data, len)                                    \
do {                                                                            \
    if (TUYA_APP_LOG_LEVEL >= TUYA_APP_LOG_LEVEL_DEBUG)                                     \
    {                                                                           \
        TUYA_BLE_PRINTF(TUYA_APP_DEBUG_PREFIX _fmt_ TUYA_APP_LOG_DEFAULT_COLOR " [len=%d] :" TUYA_APP_LOG_CRLF,len); \
        TUYA_BLE_HEXDUMP(p_data, len);                                                \
        TUYA_BLE_PRINTF(TUYA_APP_LOG_CRLF);                                      \
    }                                                                           \
} while(0)


#endif // TUYA_BLE_LOG_INTERNAL__H__


