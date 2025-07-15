/**
 * \file tuya_ble_port.h
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


#ifndef TUYA_BLE_PORT_H__
#define TUYA_BLE_PORT_H__

#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_config.h"
#include "tuya_ble_port_peripheral.h"

void tuya_ble_device_enter_critical(void);
void tuya_ble_device_exit_critical(void);

#if defined(TUYA_BLE_PORT_PLATFORM_HEADER_FILE)
/**
 * @file TUYA_BLE_PORT_PLATFORM_HEADER_FILE
 *
 * @brief Platform header file

 * @note
 *  Must declare or define 'tuya_ble_device_enter_critical' and 'tuya_ble_device_exit_critical' in the platform header file.

 *  An example on the nrf52832 platform is as follows

 * @example

 * @code
    #define TUYA_BLE_PORT_PLATFORM_HEADER_FILE  "tuya_ble_port_nrf52832.h" ///<in custom_tuya_ble_config.h file.
 * @endcode

 *  Defined in the tuya_ble_port_nrf52832.h file as follows
 *
 * @code
      #define tuya_ble_device_enter_critical() \
      {                                    \
          uint8_t __CR_NESTED = 0;         \
          app_util_critical_region_enter(&__CR_NESTED);

      #define tuya_ble_device_exit_critical()  \
          app_util_critical_region_exit(__CR_NESTED); \
      }

 * @endcode
*/
#include TUYA_BLE_PORT_PLATFORM_HEADER_FILE

#else

#define TUYA_BLE_PRINTF(...)
#define TUYA_BLE_HEXDUMP(...)

/**@brief Macro for entering and leaving a critical region.
 *
 * @note Due to implementation details, there must exist one and only one call to
 *       tuya_ble_device_exit_critical() for each call to tuya_ble_device_enter_critical(), and they must be located
 *       in the same scope.
 */
//#define tuya_ble_device_enter_critical()
//#define tuya_ble_device_exit_critical()

#endif


tuya_ble_status_t tuya_ble_gap_advertising_adv_data_update(uint8_t const *p_ad_data, uint8_t ad_len);

tuya_ble_status_t tuya_ble_gap_advertising_scan_rsp_data_update(uint8_t const *p_sr_data, uint8_t sr_len);

tuya_ble_status_t tuya_ble_gap_disconnect(void);

tuya_ble_status_t tuya_ble_gatt_send_data(const uint8_t *p_data, uint16_t len);

/**
 *@brief     Function for update the device information characteristic value.
 *@param[in] p_data     The pointer to the data to be updated.
 *@param[in] data_len   The length of the data to be updated.
 *
 *@note      The device information characteristic uuid : 00000003-0000-1001-8001-00805F9B07D0
 *
 * */
tuya_ble_status_t tuya_ble_device_info_characteristic_value_update(uint8_t const *p_data, uint8_t data_len);

/**
 * @brief   Create a timer.
 * @param   [out] p_timer_id: a pointer to timer id address which can uniquely identify the timer.
            [in] timeout_value_ms Number of milliseconds to time-out event
 *          [in] timeout_handler: a pointer to a function which can be
 * called when the timer expires.
 *          [in] mode: repeated or single shot.
 * @return  TUYA_BLE_SUCCESS             If the timer was successfully created.
 *          TUYA_BLE_ERR_INVALID_PARAM   Invalid timer id supplied.
 *          TUYA_BLE_ERR_INVALID_STATE   timer module has not been initialized or the
 * timer is running.
 *          TUYA_BLE_ERR_NO_MEM          timer pool is full.
 *
 * */
tuya_ble_status_t tuya_ble_timer_create(void **p_timer_id, uint32_t timeout_value_ms, tuya_ble_timer_mode mode, tuya_ble_timer_handler_t timeout_handler);

/**
 * @brief   Delete a timer.
 * @param   [in] timer_id: timer id
 * @return  TUYA_BLE_SUCCESS             If the timer was successfully deleted.
 *          TUYA_BLE_ERR_INVALID_PARAM   Invalid timer id supplied..
 * */
tuya_ble_status_t tuya_ble_timer_delete(void *timer_id);


/**
 * @brief   Start a timer.
 * @param   [in] timer_id: timer id
 *
 * @return  TUYA_BLE_SUCCESS             If the timer was successfully started.
 *          TUYA_BLE_ERR_COMMON          Invalid timer id supplied.
 * @note    If the timer has already started, it will start counting again.
 * */
tuya_ble_status_t tuya_ble_timer_start(void *timer_id);


/**
 * @brief   Restart a timer.
 * @param   [in] timer_id: timer id
 *          [in] timeout_value_ms: New number of milliseconds to time-out event
 * @return  TUYA_BLE_SUCCESS             If the timer was successfully started.
 *          TUYA_BLE_ERR_COMMON         Invalid timer id supplied.
 * @note    If the timer has already started, it will start counting again.
 * */
tuya_ble_status_t tuya_ble_timer_restart(void *timer_id, uint32_t timeout_value_ms);


/**
 * @brief   Stop a timer.
 * @param   [in] timer_id: timer id
 * @return  TUYA_BLE_SUCCESS             If the timer was successfully stopped.
 *          TUYA_BLE_ERR_INVALID_PARAM   Invalid timer id supplied.
 *
 * */
tuya_ble_status_t tuya_ble_timer_stop(void *timer_id);


/**
 * @brief Function for delaying execution for a number of milliseconds.
 *
 * @param ms_time Number of milliseconds to wait.
 */
void tuya_ble_device_delay_ms(uint32_t ms);


/**
 * @brief Function for delaying execution for a number of microseconds.
 *
 * @param us_time Number of microseconds to wait.
 */
void tuya_ble_device_delay_us(uint32_t us);



/**
 * @brief   Function for RESET device.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_device_reset(void);

/**
 * @brief   Function for get mac addr.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_gap_addr_get(tuya_ble_gap_addr_t *p_addr);

/**
 * @brief   Function for update mac addr.
 *
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_gap_addr_set(tuya_ble_gap_addr_t *p_addr);

/**
 * @brief   Get ture random bytes .
 * @param   [out] p_buf: pointer to data
 *          [in] len: Number of bytes to take from pool and place in
 * p_buff
 * @return  TUYA_BLE_SUCCESS          The requested bytes were written to
 * p_buff
 *          TUYA_BLE_ERR_NO_MEM       No bytes were written to the buffer, because
 * there were not enough random bytes available.
 * @note    SHOULD use TRUE random num generator
 * */
tuya_ble_status_t tuya_ble_rand_generator(uint8_t *p_buf, uint8_t len);


/**
 * @brief   Function for get the unix timestamp.
 *
 * @note    timezone: 100 times the actual time zone
 *.
 * */

tuya_ble_status_t tuya_ble_rtc_get_timestamp(uint32_t *timestamp, int32_t *timezone);

/**
 * @brief   Function for set the unix timestamp.
 *
 * @note    timezone: 100 times the actual time zone,Eastern eight zones:8x100
 *.
 * */

tuya_ble_status_t tuya_ble_rtc_set_timestamp(uint32_t timestamp, int32_t timezone);


/**
 * @brief Initialize the NV module.
 * @note
 * @note
 *
 * @param
 * @param
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_init(void);

/**
 * @brief Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_erase(uint32_t addr, uint32_t size);

/**
 * @brief Write data to flash.
 *
 * @note This operation must after erase. @see tuya_ble_nv_erase.
 *
 * @param addr flash address
 * @param p_data the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_write(uint32_t addr, const uint8_t *p_data, uint32_t size);



/**
 * @brief Read data from flash.
 * @note
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_read(uint32_t addr, uint8_t *p_data, uint32_t size);

/**
 * @brief Initialize uart peripheral.
 * @note   UART_PARITY_NO_PARTY,UART_STOP_BITS_1,UART_WROD_LENGTH_8BIT;
 *         9600 baud rate.
 * @param No parameter.
 *
 * @return tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_common_uart_init(void);


/**
 * @brief Send data to uart.
 * @note
 *
 * @param p_data the send data buffer
 * @param len to send bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_common_uart_send_data(const uint8_t *p_data, uint16_t len);


/**
 *
 *
 * \brief Create a new task and add it to the list of tasks that are ready to run.
 *
 * \param[out]  pp_handle  Used to pass back a handle by which the created task
 *                         can be referenced.
 *
 * \param[in]   p_name     A descriptive name for the task.
 *
 * \param[in]   p_routine  Pointer to task routine function that must be implemented
 *                         to never return.
 *
 * \param[in]   p_param    Pointer parameter passed to the task routine function.
 *
 * \param[in]   stack_size The size of the task stack that is specified as the number
 *                         of bytes.
 *
 * \param[in]   priority   The priority at which the task should run. Higher priority
 *                         task has higher priority value.
 *
 * \return           The status of the task creation.
 * \retval true      Task was created successfully and added to task ready list.
 * \retval false     Task was failed to create.
 */
bool tuya_ble_os_task_create(void **pp_handle, const char *p_name, void (*p_routine)(void *), void *p_param, uint16_t stack_size, uint16_t priority);


/**
 *
 *
 * \brief Remove a task from RTOS's task management. The task being deleted will be removed
 * from RUNNING, READY or WAITING state.
 *
 * \param[in] p_handle  The handle of the task to be deleted.
 *
 * \return           The status of the task deletion.
 * \retval true      Task was deleted successfully.
 * \retval false     Task was failed to delete.
 */
bool tuya_ble_os_task_delete(void *p_handle);


/**
 *
 *
 * \brief Suspend the task. The suspended task will not be scheduled and never get
 * any microcontroller processing time.
 *
 * \param[in] p_handle  The handle of the task to be suspended.
 *
 * \return           The status of the task suspension.
 * \retval true      Task was suspended successfully.
 * \retval false     Task was failed to suspend.
 */
bool tuya_ble_os_task_suspend(void *p_handle);

/**
 *
 *
 * \brief Resume the suspended task.
 *
 * \param[in] p_handle  The handle of the task to be resumed.
 *
 * \return           The status of the task resume.
 * \retval true      Task was resumed successfully.
 * \retval false     Task was failed to resume.
 */
bool tuya_ble_os_task_resume(void *p_handle);


/**
 *
 *
 * \brief   Creates a message queue instance. This allocates the storage required by the
 *          new queue and passes back a handle for the queue.
 *
 * \param[out]  pp_handle  Used to pass back a handle by which the message queue
 *                         can be referenced.
 *
 * \param[in]   msg_num    The maximum number of items that the queue can contain.
 *
 * \param[in]   msg_size   The number of bytes each item in the queue will require. Items
 *                         are queued by copy, not by reference, so this is the number of
 *                         bytes that will be copied for each posted item. Each item on the
 *                         queue must be the same size.
 *
 * \return           The status of the message queue creation.
 * \retval true      Message queue was created successfully.
 * \retval false     Message queue was failed to create.
 */
bool tuya_ble_os_msg_queue_create(void **pp_handle, uint32_t msg_num, uint32_t msg_size) ;

/**
 *
 *
 * \brief   Delete the specified message queue, and free all the memory allocated for
 *          storing of items placed on the queue.
 *
 * \param[in]   p_handle   The handle to the message queue being deleted.
 *
 * \return           The status of the message queue deletion.
 * \retval true      Message queue was deleted successfully.
 * \retval false     Message queue was failed to delete.
 */
bool tuya_ble_os_msg_queue_delete(void *p_handle);


/**
 *
 *
 * \brief    Peek the number of items sent and resided on the message queue.
 *
 * \param[in]   p_handle   The handle to the message queue being peeked.
 *
 * \param[out]  p_msg_num  Used to pass back the number of items residing on the message queue.
 *
 * \return           The status of the message queue peek.
 * \retval true      Message queue was peeked successfully.
 * \retval false     Message queue was failed to peek.
 */
bool tuya_ble_os_msg_queue_peek(void *p_handle, uint32_t *p_msg_num);

/**
 *
 *
 * \brief   Send an item to the back of the specified message queue. The item is
 *          queued by copy, not by reference.
 *
 * \param[in]   p_handle The handle to the message queue on which the item is to be sent.
 *
 * \param[in]   p_msg    Pointer to the item that is to be sent on the queue. The referenced
 *                       item rather than pointer itself will be copied on the queue.
 *
 * \param[in]   wait_ms  The maximum amount of time in milliseconds that the task should
 *                       block waiting for the item to sent on the queue.
 * \arg \c 0           No blocking and return immediately.
 * \arg \c 0xFFFFFFFF  Block infinitely until the item sent.
 * \arg \c others      The timeout value in milliseconds.
 *
 * \return           The status of the message item sent.
 * \retval true      Message item was sent successfully.
 * \retval false     Message item was failed to send.
 */
bool tuya_ble_os_msg_queue_send(void *p_handle, void *p_msg, uint32_t wait_ms);


/**
 *
 *
 * \brief   Receive an item from the specified message queue. The item is received by
 *          copy rather than by reference, so a buffer of adequate size must be provided.
 *
 * \param[in]   p_handle The handle to the message queue from which the item is to be received.
 *
 * \param[out]  p_msg    Pointer to the buffer into which the received item will be copied.
 *                       item rather than pointer itself will be copied on the queue.
 *
 * \param[in]   wait_ms  The maximum amount of time in milliseconds that the task should
 *                       block waiting for an item to be received from the queue.
 * \arg \c 0           No blocking and return immediately.
 * \arg \c 0xFFFFFFFF  Block infinitely until the item received.
 * \arg \c others      The timeout value in milliseconds.
 *
 * \return           The status of the message item received.
 * \retval true      Message item was received successfully.
 * \retval false     Message item was failed to receive.
 */
bool tuya_ble_os_msg_queue_recv(void *p_handle, void *p_msg, uint32_t wait_ms);


/*
 *
 *
 * \brief   If undefine TUYA_BLE_SELF_BUILT_TASK ,application should provide the task to sdk to process the event.
 *          SDK will use this port to send event to the task of provided by application.
 *
 * \param[in]   evt   the message data point to be send.
 *
 *
 * \param[in]   wait_ms  The maximum amount of time in milliseconds that the task should
 *                       block waiting for an item to be received from the queue.
 * \arg \c 0           No blocking and return immediately.
 * \arg \c 0xFFFFFFFF  Block infinitely until the item received.
 * \arg \c others      The timeout value in milliseconds.
 *
 * \return           The status of the message item received.
 * \retval true      Message item was received successfully.
 * \retval false     Message item was failed to receive.
 * */
bool tuya_ble_event_queue_send_port(tuya_ble_evt_param_t *evt, uint32_t wait_ms);


/**
    * @brief  128 bit AES ECB encryption on speicified plaintext and keys
    * @param  input    specifed plain text to be encypted
    * @param  input_len    byte length of the data to be descrypted, must be multiples of 16
    * @param  key          keys to encrypt the plaintext
    * @param  output    output buffer to store encrypted data
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   least significant octet of encrypted data corresponds to encypted[0]
    */
bool tuya_ble_aes128_ecb_encrypt(uint8_t *key, uint8_t *input, uint16_t input_len, uint8_t *output);

/**
    * @brief  128 bit AES ECB decryption on speicified encrypted data and keys
    * @param  input    specifed encypted data to be decypted
    * @param  input_len    byte length of the data to be descrypted, must be multiples of 16
    * @param  key          keys to decrypt the data
    * @param  output    output buffer to store plain data
    * @return decryption results
    * @retval true      successful
    * @retval false     fail
    * @note   least significant octet of encrypted data corresponds to encypted[0]
*/
bool tuya_ble_aes128_ecb_decrypt(uint8_t *key, uint8_t *input, uint16_t input_len, uint8_t *output);

/**
    * @brief  128 bit AES CBC encryption on speicified plaintext and keys
    * @param  input    specifed plain text to be encypted
    * @param  key          keys to encrypt the plaintext
    * @param  output    output buffer to store encrypted data
    * @param  iv         initialization vector (IV) for CBC mode
    * @param  input_len    byte length of the data to be descrypted, must be multiples of 16
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   least significant octet of encrypted data corresponds to encypted[0]
    */
bool tuya_ble_aes128_cbc_encrypt(uint8_t *key, uint8_t *iv, uint8_t *input, uint16_t input_len, uint8_t *output);

/**
    * @brief  128 bit AES CBC descryption on speicified plaintext and keys
    * @param  input    specifed encypted data to be decypted
    * @param  key          keys to decrypt the data
    * @param  output    output buffer to store plain data
    * @param  iv         initialization vector (IV) for CBC mode
    * @param  input_len    byte length of the data to be descrypted, must be multiples of 16
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note   least significant octet of encrypted data corresponds to encypted[0]
    */
bool tuya_ble_aes128_cbc_decrypt(uint8_t *key, uint8_t *iv, uint8_t *input, uint16_t input_len, uint8_t *output);

/**
    * @brief  MD5 checksum
    * @param  input    specifed plain text to be encypted
    * @param  output    output buffer to store md5 result data,output data len is always 16
    * @param  input_len    byte length of the data to be encypted
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note
    */
bool tuya_ble_md5_crypt(uint8_t *input, uint16_t input_len, uint8_t *output);

/**
    * @brief  SHA256 checksum
    * @param  input    specifed plain text to be encypted
    * @param  output    output buffer to store sha256 result data,output data len is always 32
    * @param  input_len    byte length of the data to be encypted
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note
    */
bool tuya_ble_sha256_crypt(const uint8_t *input, uint16_t input_len, uint8_t *output);

/*
 *@brief  storage private data, for example: token/dev cert/keys
 *@param  private_data_type     describe data type
 *@param  p_data                storage data point
 *@param  data_size             p_data size
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_storage_private_data(tuya_ble_private_data_type private_data_type, uint8_t *p_data, uint32_t data_size);


/*
 *@brief  Get device certificate length
 *@param
 *
 *@note
 *
 * */
uint32_t tuya_ble_get_dev_crt_len(void);


/*
 *@brief  Get device certificate data
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_get_dev_crt_der(uint8_t *p_der, uint32_t der_len);


/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_ecc_keypair_gen_secp256r1(uint8_t *public_key, uint8_t *private_key);

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_ecc_shared_secret_compute_secp256r1(uint8_t *public_key, uint8_t *private_key, uint8_t *secret_key);


/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_ecc_sign_secp256r1(const uint8_t *p_sk, const uint8_t *p_data, uint32_t data_size, uint8_t *p_sig);


/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_ecc_verify_secp256r1(const uint8_t *p_pk, const uint8_t *p_hash, uint32_t hash_size, const uint8_t *p_sig);

/**
    * @brief          This function calculates the full generic HMAC
    *                 on the input buffer with the provided key.
    * @param  key      The HMAC secret key.
    * @param  key_len  The length of the HMAC secret key in Bytes.
    * @param  input    specifed plain text to be encypted
    * @param  output    output buffer to store the result data
    * @param  input_len    byte length of the data to be encypted
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note
    */
bool tuya_ble_hmac_sha1_crypt(const uint8_t *key, uint32_t key_len, const uint8_t *input, uint32_t input_len, uint8_t *output);

/**
    * @brief          This function calculates the full generic HMAC
    *                 on the input buffer with the provided key.
    * @param  key      The HMAC secret key.
    * @param  key_len  The length of the HMAC secret key in Bytes.
    * @param  input    specifed plain text to be encypted
    * @param  output    output buffer to store the result data
    * @param  input_len    byte length of the data to be encypted
    * @return encryption results
    * @retval true      successful
    * @retval false     fail
    * @note
    */
bool tuya_ble_hmac_sha256_crypt(const uint8_t *key, uint32_t key_len, const uint8_t *input, uint32_t input_len, uint8_t *output);





/**
 * \brief    Allocate a memory block with required size.
 *
 *
 * \param[in]   size     Required memory size.
 *
 * \return     The address of the allocated memory block. If the address is NULL, the
 *             memory allocation failed.
 */
void *tuya_ble_port_malloc(uint32_t size);



/**
 *
 * \brief    Free a memory block that had been allocated.
 *
 * \param[in]   pv     The address of memory block being freed.
 *
 * \return     None.
 */
void tuya_ble_port_free(void *pv);


#endif

