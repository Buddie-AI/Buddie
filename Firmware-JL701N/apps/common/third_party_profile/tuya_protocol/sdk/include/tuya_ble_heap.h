/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


#ifndef TUYA_HEAP_H__
#define TUYA_HEAP_H__

#include "tuya_ble_stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (TUYA_BLE_USE_PLATFORM_MEMORY_HEAP==0)

#define portBYTE_ALIGNMENT          4



#if portBYTE_ALIGNMENT == 32
#define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if portBYTE_ALIGNMENT == 16
#define portBYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if portBYTE_ALIGNMENT == 8
#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
#define portBYTE_ALIGNMENT_MASK	( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
#define portBYTE_ALIGNMENT_MASK	( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
#define portBYTE_ALIGNMENT_MASK	( 0x0000 )
#endif

#ifndef portBYTE_ALIGNMENT_MASK
#error "Invalid portBYTE_ALIGNMENT definition"
#endif


#ifndef tuyaASSERT
#define tuyaASSERT( x )
#define tuyaASSERT_DEFINED 0
#else
#define tuyaASSERT_DEFINED 1
#endif


#ifndef tuyaCOVERAGE_TEST_MARKER
#define tuyaCOVERAGE_TEST_MARKER()
#endif


#ifndef tuya_traceMALLOC
#define tuya_traceMALLOC( pvAddress, uiSize )
#endif

#ifndef tuya_traceFREE
#define tuya_traceFREE( pvAddress, uiSize )
#endif



void *pvTuyaPortMalloc(uint32_t xWantedSize);

void vTuyaPortFree(void *pv);

uint32_t xTuyaPortGetFreeHeapSize(void);

uint32_t xTuyaPortGetMinimumEverFreeHeapSize(void);


#endif

#ifdef __cplusplus
}
#endif

#endif //TUYA_HEAP_H__

/** @} */
