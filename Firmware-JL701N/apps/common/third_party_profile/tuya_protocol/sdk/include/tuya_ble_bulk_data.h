/**
 * \file tuya_ble_bulkdata.h
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

#ifndef TUYA_BLE_BULK_DATA_H_
#define TUYA_BLE_BULK_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDE
 */
#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"



/**@brief   Macro for defining bulk data transfer protocol. */

#define FRM_BULK_DATA_READ_INFO_REQ          0x0007  //APP->BLE
#define FRM_BULK_DATA_READ_INFO_RESP         0x0007  //BLE->APP
#define FRM_BULK_DATA_READ_DATA_REQ          0x0008  //APP->BLE
#define FRM_BULK_DATA_READ_DATA_RESP         0x0008  //BLE->APP
#define FRM_BULK_DATA_SEND_DATA              0x0009  //BLE->APP
#define FRM_BULK_DATA_ERASE_DATA_REQ         0x000A  //APP->BLE
#define FRM_BULK_DATA_ERASE_DATA_RESP        0x000A  //BLE->APP



/**@brief   Function for get the block size used for bulk data reading.
 *
 *
 * @param[out] block size used for bulk data reading.
 * @note    The block size depends on the mtu value negotiated after the BLE connection is established.
 *          If the return value is 0, stop the current bulk data transmission.
 */
uint32_t tuya_ble_bulk_data_read_block_size_get(void);

/**@brief   Function for handling the bulk data request.
 *
 *
 * @param[in] cmd           Request command.
 * @param[in] p_recv_data     Pointer to the buffer with received data.
 * @param[in] recv_data_len      Length of received data.
 */
void tuya_ble_handle_bulk_data_req(uint16_t cmd, uint8_t *p_recv_data, uint32_t recv_data_len);

/**@brief   Function for respond to app requests.
 *
 *
 * @param[in] p_data   The pointer to the response data.
 */
tuya_ble_status_t tuya_ble_bulk_data_response(tuya_ble_bulk_data_response_t *p_data);

/**@brief   Function for handling the bulk data events.
 *
 *
 * @param[in] p_evt    Event received from the application.
 */
void tuya_ble_handle_bulk_data_evt(tuya_ble_evt_param_t *p_evt);



#ifdef __cplusplus
}
#endif

#endif //TUYA_BLE_BULKDATA_H_


