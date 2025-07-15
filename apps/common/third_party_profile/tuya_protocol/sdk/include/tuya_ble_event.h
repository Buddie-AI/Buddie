/**
 * \file tuya_ble_event.h
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

#ifndef TUYA_BLE_EVENT_H_
#define TUYA_BLE_EVENT_H_

#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"

#ifdef __cplusplus
extern "C" {
#endif


#if (!TUYA_BLE_USE_OS)


#define TUYA_BLE_EVT_MAX_NUM 		MAX_NUMBER_OF_TUYA_MESSAGE
#define TUYA_BLE_EVT_SIZE 		    52 //64

enum {
    TUYA_BLE_EVT_SEND_SUCCESS      = 0,
    TUYA_BLE_EVT_SEND_NO_MEMORY    = 1,
    TUYA_BLE_EVT_SEND_FAIL         = 2,
};

#define TUYA_BLE_ERROR_HANDLER(ERR_CODE)

#define TUYA_BLE_ERROR_CHECK(ERR_CODE)

#define TUYA_BLE_ERROR_CHECK_BOOL(BOOLEAN_VALUE)


#define CEIL_DIV(A, B)      \
    (((A) + (B) - 1) / (B))

/**@brief Compute number of bytes required to hold the scheduler buffer.
 *
 * @param[in] EVENT_SIZE   Maximum size of events to be passed through the scheduler.
 * @param[in] QUEUE_SIZE   Number of entries in scheduler queue (i.e. the maximum number of events
 *                         that can be scheduled for execution).
 *
 * @return    Required scheduler buffer size (in bytes).
 */
#define TUYA_BLE_SCHED_BUF_SIZE(EVENT_SIZE, QUEUE_SIZE)                                                 \
            ((EVENT_SIZE) * ((QUEUE_SIZE) + 1))


/**@brief Macro for initializing the event scheduler.
 *
 * @details It will also handle dimensioning and allocation of the memory buffer required by the
 *          scheduler, making sure the buffer is correctly aligned.
 *
 * @param[in] EVENT_SIZE   Maximum size of events to be passed through the scheduler.
 * @param[in] QUEUE_SIZE   Number of entries in scheduler queue (i.e. the maximum number of events
 *                         that can be scheduled for execution).
 *
 * @note Since this macro allocates a buffer, it must only be called once (it is OK to call it
 *       several times as long as it is from the same location, e.g. to do a reinitialization).
 */
#define TUYA_BLE_SCHED_INIT(EVENT_SIZE, QUEUE_SIZE)                                                     \
    do                                                                                             \
    {                                                                                              \
        static uint32_t TUYA_BLE_SCHED_BUF[CEIL_DIV(TUYA_BLE_SCHED_BUF_SIZE((EVENT_SIZE), (QUEUE_SIZE)),     \
                                               sizeof(uint32_t))];                                 \
        uint32_t ERR_CODE = tuya_ble_sched_init((EVENT_SIZE), (QUEUE_SIZE), TUYA_BLE_SCHED_BUF);            \
        TUYA_BLE_ERROR_CHECK(ERR_CODE);                                                                 \
    } while (0)

/**@brief Function for initializing the Scheduler.
 *
 * @details It must be called before entering the main loop.
 *
 * @param[in]   max_event_size   Maximum size of events to be passed through the scheduler.
 * @param[in]   queue_size       Number of entries in scheduler queue (i.e. the maximum number of
 *                               events that can be scheduled for execution).
 * @param[in]   p_evt_buffer   Pointer to memory buffer for holding the scheduler queue. It must
 *                               be dimensioned using the APP_SCHED_BUFFER_SIZE() macro. The buffer
 *                               must be aligned to a 4 byte boundary.
 *
 * @note Normally initialization should be done using the TUYA_SCHED_INIT() macro, as that will both
 *       allocate the scheduler buffer, and also align the buffer correctly.
 *
 * @retval      NRF_SUCCESS               Successful initialization.
 * @retval      NRF_ERROR_INVALID_PARAM   Invalid parameter (buffer not aligned to a 4 byte
 *                                        boundary).
 */
//uint32_t tuya_ble_sched_init(uint16_t max_event_size, uint16_t queue_size, void * p_evt_buffer);

/**@brief Function for executing all scheduled events.
 *
 * @details This function must be called from within the main loop. It will execute all events
 *          scheduled since the last time it was called.
 */
void tuya_sched_execute(void);

/**@brief Function for scheduling an event.
 *
 * @details Puts an event into the event queue.
 *
 * @param[in]   p_event_data   Pointer to event data to be scheduled.
 * @param[in]   event_size     Size of event data to be scheduled.
 * @param[in]   handler        Event handler to receive the event.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
//tuya_ble_status_t tuya_ble_sched_event_put(void const  * p_event_data, uint16_t  event_data_size);


/**@brief Function for getting the current amount of free space in the queue.
 *
 * @details The real amount of free space may be less if entries are being added from an interrupt.
 *          To get the sxact value, this function should be called from the critical section.
 *
 * @return Amount of free space in the queue.
 */
uint16_t tuya_ble_sched_queue_size_get(void);

uint16_t tuya_ble_sched_queue_space_get(void);

uint16_t tuya_ble_sched_queue_events_get(void);

void tuya_ble_event_queue_init(void);

tuya_ble_status_t tuya_ble_message_send(tuya_ble_evt_param_t *evt);


#endif

#ifdef __cplusplus
}
#endif

#endif //




