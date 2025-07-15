/**
 * \file tuya_ble_mutli_tsf_protocol.h
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
#ifndef _TUYA_BLE_MUTLI_TSF_PROTOCOL_H
#define _TUYA_BLE_MUTLI_TSF_PROTOCOL_H

#include "tuya_ble_stdlib.h"
#include "tuya_ble_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  __MUTLI_TSF_PROTOCOL_GLOBALS
#define __MUTLI_TSF_PROTOCOL_EXT
#else
#define __MUTLI_TSF_PROTOCOL_EXT extern
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
#define SNGL_PKG_TRSFR_LMT  TUYA_BLE_DATA_MTU_MAX // single package transfer limit

//#define FRM_TYPE_OFFSET (0x0f << 4)
#define FRM_VERSION_OFFSET (0x0f << 4)
#define FRM_SEQ_OFFSET  (0x0f << 0)

// frame total len
typedef uint32_t frame_total_t;

// frame subpackage num
typedef uint32_t frame_subpkg_num_t;


// frame sequence
typedef uint8_t frame_seq_t;

#define FRAME_SEQ_LMT 16

// frame subpackage len
typedef uint16_t frame_subpkg_len_t;

// frame package description
typedef uint8_t frm_pkg_desc_t;
#define FRM_PKG_INIT 0   // frame package init
#define FRM_PKG_FIRST 1  // frame package first
#define FRM_PKG_MIDDLE 2 // frame package middle
#define FRM_PKG_END 3    // frame package end

// mutil tsf ret code
typedef int32_t mtp_ret;
#define MTP_OK  0
#define MTP_INVALID_PARAM 1
#define MTP_COM_ERROR 2
#define MTP_TRSMITR_CONTINUE 3
#define MTP_TRSMITR_ERROR 4
#define MTP_MALLOC_ERR 5

// frame transmitter process
typedef struct {
    frame_total_t  total;         //4 bytes, total length of data, not including header
    uint8_t  version;             //1 byte, protocol major version number
    frame_seq_t    seq;           //1 byte
    frm_pkg_desc_t pkg_desc;      //1 byte, Current packet frame type (init/first/middle/end)
    frame_subpkg_num_t subpkg_num;//4 bytes, current sub-packet number
    uint32_t pkg_trsmitr_cnt;     //  package process count ,Number of bytes sent
    frame_subpkg_len_t subpkg_len;//1 byte, the data length in the current sub-packet
    uint8_t subpkg[SNGL_PKG_TRSFR_LMT];
} frm_trsmitr_proc_s;


// dp type description,please refer to the relevant help documents of Tuya iot platform for the usage of various types of dp points.
typedef uint8_t dp_type;
#define DT_RAW     0
#define DT_BOOL    1
#define DT_VALUE   2
#define DT_INT     DT_VALUE
#define DT_STRING  3
#define DT_ENUM    4
#define DT_BITMAP  5
#define DT_CHAR    7       //Not currently supported
#define DT_UCHAR   8       //Not currently supported
#define DT_SHORT   9       //Not currently supported
#define DT_USHORT  10      //Not currently supported
#define DT_LMT    DT_USHORT

#define DT_VALUE_LEN 4 // int
#define DT_BOOL_LEN 1
#define DT_ENUM_LEN 1
#define DT_BITMAP_MAX 4 // 1/2/4
#define DT_STR_MAX 255
#define DT_RAW_MAX 255
#define DT_INT_LEN DT_VALUE_LEN

typedef struct s_klv_node {
    struct s_klv_node *next;
    uint8_t id;
    dp_type type;
    uint16_t len;
    uint8_t *data;
} klv_node_s;


/***********************************************************
*************************variable define********************
***********************************************************/


/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************
*  Function: make_klv_list
*  description:
*  Input:
*  Output:
*  Return:
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
klv_node_s *make_klv_list(klv_node_s *list, uint8_t id, dp_type type, \
                          void *data, uint16_t len);

/***********************************************************
*  Function: free_klv_list
*  description:
*  Input: list
*  Output:
*  Return:
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
void free_klv_list(klv_node_s *list);

/***********************************************************
*  Function: klvlist_2_data
*  description:
*  Input: list
*  Output: data len
*  Return:
*  Note:data need free.
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret klvlist_2_data(klv_node_s *list, uint8_t **data, uint32_t *len, uint8_t type);

/***********************************************************
*  Function: data_2_klvlist
*  description:
*  Input: data len
*  Output: list
*  Return:
*  Note: list need to call free_klv_list to free.
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret data_2_klvlist(uint8_t *data, uint32_t len, klv_node_s **list, uint8_t type);

/***********************************************************
*  Function: create_trsmitr_init
*  description: create a transmitter and initialize
*  Input: none
*  Output:
*  Return: transmitter handle
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frm_trsmitr_proc_s *create_trsmitr_init(void);

/***********************************************************
*  Function: trsmitr_init
*  description: init a transmitter
*  Input: transmitter handle
*  Output:
*  Return:
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
void trsmitr_init(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: delete_trsmitr
*  description: delete transmitter
*  Input: transmitter handle
*  Output:
*  Return:
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
void delete_trsmitr(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_total_len
*  description: get a transmitter total data len
*  Input: transmitter handle
*  Output:
*  Return: frame_total_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frame_total_t get_trsmitr_frame_total_len(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_type
*  description:
*  Input: transmitter handle
*  Output:
*  Return: frame_type_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
uint8_t get_trsmitr_frame_version(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_frame_seq
*  description:
*  Input: transmitter handle
*  Output:
*  Return: frame_seq_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frame_seq_t get_trsmitr_frame_seq(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_subpkg_len
*  description:
*  Input: transmitter handle
*  Output:
*  Return: frame_subpkg_len_t
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
frame_subpkg_len_t get_trsmitr_subpkg_len(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: get_trsmitr_subpkg
*  description:
*  Input: transmitter handle
*  Output:
*  Return: subpackage buf
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
uint8_t *get_trsmitr_subpkg(frm_trsmitr_proc_s *frm_trsmitr);

/***********************************************************
*  Function: trsmitr_send_pkg_encode
*  description: frm_trsmitr->transmitter handle
*               type->frame type
*               buf->data buf
*               len->data len
*  Input:
*  Output:
*  Return: MTP_OK->buf send up
*          MTP_TRSMITR_CONTINUE->need call again to be continue
*          other->error
*  Note: could get from encode data len and encode data by calling method
         get_trsmitr_subpkg_len() and get_trsmitr_subpkg()
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret trsmitr_send_pkg_encode(frm_trsmitr_proc_s *frm_trsmitr, uint8_t version, uint8_t *buf, uint32_t len);

/***********************************************************
*  Function: trsmitr_send_pkg_encode_with_packet_length
*  description: Encoding function for specifying sub-packet length
*
*
*
*  Input:
*  Output:
*  Return: MTP_OK->buf send up
*          MTP_TRSMITR_CONTINUE->need call again to be continue
*          other->error
*  Note: could get from encode data len and encode data by calling method
         get_trsmitr_subpkg_len() and get_trsmitr_subpkg()
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret trsmitr_send_pkg_encode_with_packet_length(frm_trsmitr_proc_s *frm_trsmitr, uint32_t pkg_len_max, uint8_t version, uint8_t *buf, uint32_t len);

/***********************************************************
*  Function: trsmitr_recv_pkg_decode
*  description: frm_trsmitr->transmitter handle
*               raw_data->raw encode data
*               raw_data_len->raw encode data len
*  Input:
*  Output:
*  Return: MTP_OK->buf receive up
*          MTP_TRSMITR_CONTINUE->need call again to be continue
*          other->error
*  Note: could get from decode data len and decode data by calling method
         get_trsmitr_subpkg_len() and get_trsmitr_subpkg()
***********************************************************/
__MUTLI_TSF_PROTOCOL_EXT \
mtp_ret trsmitr_recv_pkg_decode(frm_trsmitr_proc_s *frm_trsmitr, uint8_t *raw_data, uint16_t raw_data_len);


#ifdef __cplusplus
}
#endif


#endif




