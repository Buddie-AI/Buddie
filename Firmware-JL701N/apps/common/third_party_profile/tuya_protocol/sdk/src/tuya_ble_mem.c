/**
 * \file tuya_ble_mem.c
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

#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_heap.h"
#include "tuya_ble_mem.h"
#include "tuya_ble_internal_config.h"


#if (TUYA_BLE_USE_PLATFORM_MEMORY_HEAP==0)

/**
 *@brief      Allocate and clear a memory block with required size.
 *@param[in]  size     Required memory size.
 *
 *@note
 *
 * */
void *tuya_ble_malloc(uint16_t size)
{
    uint8_t *ptr = pvTuyaPortMalloc(size);
    if (ptr) {
        memset(ptr, 0x0, size); //allocate buffer need init
    }
    return ptr;
}


/**
 *@brief    Free a memory block that had been allocated.
 *@param[in] ptr    The address of memory block being freed.
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_free(uint8_t *ptr)
{
    if (ptr == NULL) {
        return TUYA_BLE_SUCCESS;
    }

    vTuyaPortFree(ptr);
    return TUYA_BLE_SUCCESS;
}


/**
 *@brief
 *@param
 *
 *@note
 *
 * */
void *tuya_ble_calloc_n(uint32_t n, uint32_t size)
{
    void *ptr = NULL;
    ptr = pvTuyaPortMalloc(n * size);
    if (ptr != NULL) {
        memset(ptr, 0, n * size);
    }
    return ptr;
}


/**
 *@brief
 *@param
 *
 *@note
 *
 * */
void tuya_ble_free_n(void *ptr)
{
    vTuyaPortFree(ptr);
}

#else


/**
 *@brief      Allocate and clear a memory block with required size.
 *@param[in]  size     Required memory size.
 *
 *@note
 *
 * */
void *tuya_ble_malloc(uint16_t size)
{
    uint8_t *ptr = tuya_ble_port_malloc(size);
    if (ptr) {
        memset(ptr, 0x0, size); //allocate buffer need init
    }
    return ptr;
}


/**
 *@brief    Free a memory block that had been allocated.
 *@param[in] ptr    The address of memory block being freed.
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_free(uint8_t *ptr)
{
    if (ptr == NULL) {
        return TUYA_BLE_SUCCESS;
    }

    tuya_ble_port_free(ptr);
    return TUYA_BLE_SUCCESS;
}


#endif



