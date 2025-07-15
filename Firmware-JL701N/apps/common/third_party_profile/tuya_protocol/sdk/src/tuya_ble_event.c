/**
 * \file tuya_ble_event.c
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
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_sdk_version.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_event.h"
#include "tuya_ble_log.h"

#if (!TUYA_BLE_USE_OS)

static  uint8_t        *m_queue_event_data;     /**< Array for holding the queue event data. */
static volatile uint8_t m_queue_start_index;    /**< Index of queue entry at the start of the queue. */
static volatile uint8_t m_queue_end_index;      /**< Index of queue entry at the end of the queue. */
static uint16_t         m_queue_event_size;     /**< Maximum event size in queue. */
static uint16_t         m_queue_size;           /**< Number of queue entries. */


/**@brief Function for incrementing a queue index, and handle wrap-around.
 *
 * @param[in]   index   Old index.
 *
 * @return      New (incremented) index.
 */
static __TUYA_BLE_INLINE uint8_t next_index(uint8_t index)
{
    return (index < m_queue_size) ? (index + 1) : 0;
}


static __TUYA_BLE_INLINE uint8_t tuya_sched_queue_full(void)
{
    uint8_t tmp = m_queue_start_index;
    return next_index(m_queue_end_index) == tmp;
}

/**@brief Macro for checking if a queue is full. */
#define TUYA_BLE_SCHED_QUEUE_FULL() tuya_sched_queue_full()


static __TUYA_BLE_INLINE uint8_t tuya_sched_queue_empty(void)
{
    uint8_t tmp = m_queue_start_index;
    return m_queue_end_index == tmp;
}

/**@brief Macro for checking if a queue is empty. */
#define TUYA_BLE_SCHED_QUEUE_EMPTY() tuya_sched_queue_empty()


uint32_t tuya_ble_sched_init(uint16_t event_size, uint16_t queue_size, void *p_event_buffer)
{

    // Check that buffer is correctly aligned
    if (!tuya_ble_is_word_aligned_tuya(p_event_buffer)) {
        TUYA_BLE_LOG_ERROR("tuya_ble_sched_init error");
        return 1;
    }

    // Initialize event scheduler
    m_queue_event_data    = &((uint8_t *)p_event_buffer)[0];
    m_queue_end_index     = 0;
    m_queue_start_index   = 0;
    m_queue_event_size    = event_size;
    m_queue_size          = queue_size;

    return 0;
}

uint16_t tuya_ble_sched_queue_size_get(void)
{
    return m_queue_size;
}

uint16_t tuya_ble_sched_queue_space_get(void)
{
    uint16_t start = m_queue_start_index;
    uint16_t end   = m_queue_end_index;
    uint16_t free_space = m_queue_size - ((end >= start) ?
                                          (end - start) : (m_queue_size + 1 - start + end));
    return free_space;
}


uint16_t tuya_ble_sched_queue_events_get(void)
{
    uint16_t start = m_queue_start_index;
    uint16_t end   = m_queue_end_index;
    uint16_t number_of_events;
    if (m_queue_size == 0) {
        number_of_events = 0;
    } else {
        number_of_events = ((end >= start) ? (end - start) : (m_queue_size + 1 - start + end));
    }
    return number_of_events;
}


static tuya_ble_status_t tuya_ble_sched_event_put(void const   *p_event_data, uint16_t  event_data_size)
{
    tuya_ble_status_t err_code;

    if (event_data_size <= m_queue_event_size) {
        uint16_t event_index = 0xFFFF;

        tuya_ble_device_enter_critical();

        if (!TUYA_BLE_SCHED_QUEUE_FULL()) {
            event_index       = m_queue_end_index;
            m_queue_end_index = next_index(m_queue_end_index);

        }

        tuya_ble_device_exit_critical();

        if (event_index != 0xFFFF) {
            // NOTE: This can be done outside the critical region since the event consumer will
            //       always be called from the main loop, and will thus never interrupt this code.

            if ((p_event_data != NULL) && (event_data_size > 0)) {
                memcpy(&m_queue_event_data[event_index * m_queue_event_size],
                       p_event_data,
                       event_data_size);

            } else {

            }

            err_code = TUYA_BLE_SUCCESS;
        } else {
            err_code = TUYA_BLE_ERR_NO_MEM;
        }
    } else {
        err_code = TUYA_BLE_ERR_INVALID_LENGTH;
    }

    return err_code;
}


void tuya_sched_execute(void)
{
    static tuya_ble_evt_param_t tuya_ble_evt;
    tuya_ble_evt_param_t *evt;

    evt = &tuya_ble_evt;

    uint8_t end_ix = m_queue_end_index;

    while (m_queue_start_index != end_ix) { //(!TUYA_BLE_SCHED_QUEUE_EMPTY())
        // Since this function is only called from the main loop, there is no
        // need for a critical region here, however a special care must be taken
        // regarding update of the queue start index (see the end of the loop).
        uint16_t event_index = m_queue_start_index;

        void *p_event_data;

        p_event_data = &(m_queue_event_data[event_index * m_queue_event_size]);

        memcpy(evt, p_event_data, sizeof(tuya_ble_evt_param_t));

        //TUYA_BLE_LOG_DEBUG("TUYA_RECEIVE_EVT-0x%04x,start index-0x%04x,end index-0x%04x\n",evt->hdr.event,m_queue_start_index,m_queue_end_index);

        tuya_ble_event_process(evt);

        // Event processed, now it is safe to move the queue start index,
        // so the queue entry occupied by this event can be used to store
        // a next one.
        m_queue_start_index = next_index(m_queue_start_index);
    }

}



void tuya_ble_event_queue_init(void)
{
    if ((sizeof(tuya_ble_evt_param_t)) > TUYA_BLE_EVT_SIZE) {
        TUYA_BLE_LOG_ERROR("ERROR!!TUYA_BLE_EVT_SIZE is not enough!");
        return;
    }

    TUYA_BLE_SCHED_INIT(TUYA_BLE_EVT_SIZE, TUYA_BLE_EVT_MAX_NUM);
}


tuya_ble_status_t tuya_ble_message_send(tuya_ble_evt_param_t *evt)
{
    return tuya_ble_sched_event_put(evt, m_queue_event_size);
}


#endif


