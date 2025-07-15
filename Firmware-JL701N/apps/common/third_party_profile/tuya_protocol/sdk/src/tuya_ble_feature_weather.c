/**
 * \file tuya_ble_feature_weather.c
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
#include "tuya_ble_internal_config.h"
#include "tuya_ble_feature_weather.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_unix_time.h"
#include "tuya_ble_log.h"
#include "tuya_ble_gatt_send_queue.h"

#if ( (TUYA_BLE_PROTOCOL_VERSION_HIGN==4) && (TUYA_BLE_FEATURE_WEATHER_ENABLE != 0) )

/**@brief   Weather key string. */
const char *weather_key[] = {
    "w.temp",
    "w.thigh",
    "w.tlow",
    "w.humidity",
    "w.condition",
    "w.pressure",
    "w.realFeel",
    "w.uvi",
    "w.sunRise",
    "w.sunSet",
    "t.unix",
    "t.local",
    "w.windSpeed",
    "w.windDir",
    "w.windLevel",
    "w.aqi",
    "w.tips",
    "w.rank",
    "w.pm10",
    "w.pm25",
    "w.o3",
    "w.no2",
    "w.co",
    "w.so2",
    "w.conditionNum",

    "w.data.", //**< w.data.n, n rang from [1-7]
};

static tuya_ble_weather_location_type_t cur_req_weather_location_type;

/**
 * @brief   Function for calculate weather key max string length
 *
 * @return  The max string len
 *
 * */
static uint16_t calc_weather_key_max_string_len(void)
{
    int i, string_array_element = sizeof(weather_key) / sizeof(weather_key[0]);
    uint16_t string_len = 0, tmp;

    for (i = 0; i < string_array_element; i++) {
        tmp = strlen(weather_key[i]);
        if (tmp > string_len) {
            string_len = tmp;
        }
    }

    return string_len;
}


/**
 * @brief   Function for convert weather key to enum type
 *
 * @note   	For example input key "w.temp.0", convert type=WKT_TEMP day=1
 *
 * @param[in] 	key		The ponit of input weather key string
 * @param[out] 	type	Convert type
 * @param[out] 	day		The day of weather data, rang from [1-7], 1-means today
 *
 * @retval 	TUYA_BLE_ERR_INVALID_PARAM 	The provided Parameters are not valid.
 * @retval 	TUYA_BLE_ERR_NOT_FOUND 		No corresponding key was found.
 * @retval 	TUYA_BLE_SUCCESS 			Successful.
 *
 * */
static tuya_ble_status_t weather_key_string_to_enum_type(char *key, uint8_t key_str_len, tuya_ble_weather_key_type_t *type, uint8_t *day)
{
    int i, string_array_element;

    if (key == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    string_array_element = sizeof(weather_key) / sizeof(weather_key[0]);
    for (i = 0; i < string_array_element; i++) {
        if ((strncmp(key, weather_key[i], strlen(weather_key[i])) == 0) && \
            (key_str_len <= (strlen(weather_key[i]) + 2))) {
            if (type != NULL) {
                *type = (tuya_ble_weather_key_type_t)(1 << i);
            }

            if (day != NULL) {
                if (key_str_len > strlen(weather_key[i])) {
                    /* key.n */
                    *day = key[strlen(weather_key[i]) + 1] - '0' + 1;
                } else {
                    /* only today */
                    *day = 1;
                }
            }

            return TUYA_BLE_SUCCESS;
        }
    }

    return TUYA_BLE_ERR_NOT_FOUND;
}


tuya_ble_status_t tuya_ble_feature_weather_key_enum_type_to_string(tuya_ble_weather_key_type_t type, char *key)
{
    int bit;

    if (type >= WKT_COMBINE_BITMAP_MAXVAL || key == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    /* Convert to actual weather key string based on type */
    for (bit = 0; bit < SUPPORT_WEATHER_KEY_TYPE_MAX_NUMS; bit++) {
        if (((type >> bit) & 0x01) == 0x01) {
            memcpy(key, weather_key[bit], strlen(weather_key[bit]));
            return TUYA_BLE_SUCCESS;
        }
    }

    return TUYA_BLE_ERR_NOT_FOUND;
}


/**
 * @brief   Function for parse weather lktlv format data
 *
 * @param[in] recv_data 	The point of input data
 * @param[in] recv_len 		The length of input data
 *
 * @retval  TUYA_BLE_ERR_INVALID_PARAM 	The provided Parameters are not valid.
 * @retval  TUYA_BLE_SUCCESS			Successful.
 *
 * */
static tuya_ble_status_t weather_data_lktlv_parse(uint8_t *in, uint16_t in_len, uint8_t *out, uint16_t *out_len, uint16_t *object_nums)
{
    uint8_t key_len, value_len, n_day;
    uint16_t object_len = 0;

    tuya_ble_weather_key_type_t weather_type;
    tuya_ble_weather_value_type_t value_type;
    tuya_ble_wd_object_t *object;
    uint16_t tmp_out_len = 0, tmp_object_nums = 0;

    if (in == NULL || in_len == 0 || out == NULL || out_len == NULL || object_nums == NULL) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    /*-------------------------------------------------------------------------
    |	  1byte	   |   key_len	 |	  1byte  	|	   1byte	|	value_len |
    |	 key_len   |	 key	 |	value_type	|	 value_len	|	 value	  |
    ---------------------------------------------------------------------------*/
    for (;;) {
        key_len 	= in[0 + object_len];
        value_type	= in[1 + key_len + object_len];
        value_len 	= in[2 + key_len + object_len];

        if (weather_key_string_to_enum_type((char *)&in[1 + object_len], key_len, &weather_type, &n_day) != TUYA_BLE_SUCCESS) {
            return TUYA_BLE_ERR_INVALID_PARAM;
        }

        TUYA_BLE_LOG_DEBUG("parse weather data, n_day=[%d] type=[0x%08x] val_type=[%d] ,", n_day, weather_type, value_type);
        TUYA_BLE_LOG_HEXDUMP_DEBUG("value :", &in[3 + key_len + object_len], value_len);

        /* Convert lktlv format to tuya_ble_wd_object */
        object = (tuya_ble_wd_object_t *)tuya_ble_malloc(sizeof(tuya_ble_wd_object_t) + value_len);
        if (object == NULL) {
            return TUYA_BLE_ERR_NO_MEM;
        }

        object->n_day 		= n_day;
        object->key_type 	= weather_type;
        object->val_type 	= value_type;
        object->value_len 	= value_len;
        memcpy(&(object->vaule), &in[3 + key_len + object_len], value_len);

        memcpy(&out[tmp_out_len], object, (sizeof(tuya_ble_wd_object_t) + value_len));
        tmp_out_len += (sizeof(tuya_ble_wd_object_t) + value_len);
        tuya_ble_free((uint8_t *)object);

        tmp_object_nums += 1;
        object_len += (key_len + value_len + 3);
        if (object_len >= in_len) {
            *out_len = tmp_out_len;
            *object_nums = tmp_object_nums;
            break;
        }
    }

    TUYA_BLE_LOG_DEBUG("parse weather data finish, total count=[%d]", tmp_object_nums);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_feature_weather_data_request_with_location(tuya_ble_weather_location_type_t location_type, uint32_t combine_type, uint8_t n_days)
{
    tuya_ble_evt_param_t event;
    uint8_t *ble_evt_buffer = NULL;
    uint16_t ble_evt_buffer_len = 0;
    uint16_t per_weather_key_malloc_size, weather_key_len;
    int bit, count;

    if ((location_type >= WLT_MAX) || (location_type == WLT_CUSTOM_LOCATION)) {
        TUYA_BLE_LOG_ERROR("request weather location type this sdk version unsupport");
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if (combine_type >= WKT_COMBINE_BITMAP_MAXVAL || n_days < 1 || n_days > 7) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    /* Calc total bit1 counters */
    count = tuya_ble_count_bits((uint32_t)(combine_type));
    if (0 == count) {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    /* Malloc data buffer */
    count += 1; // add w.data.n
    per_weather_key_malloc_size = (calc_weather_key_max_string_len() + 4);
    ble_evt_buffer = (uint8_t *)tuya_ble_malloc(2 + per_weather_key_malloc_size * count);
    if (ble_evt_buffer == NULL) {
        return TUYA_BLE_ERR_NO_MEM;
    }

    /* Fill version & location info */
    cur_req_weather_location_type = location_type;

    ble_evt_buffer[0] = 0;
    ble_evt_buffer[1] = (uint8_t)(location_type);
    ble_evt_buffer_len += 2;

    /* Convert to actual weather key based on type */
    for (bit = 0; bit < SUPPORT_WEATHER_KEY_TYPE_MAX_NUMS; bit++) {
        if (((combine_type >> bit) & 0x01) == 0x01) {
            /* Weather data format: [len + key] */
            weather_key_len = strlen(weather_key[bit]);
            ble_evt_buffer[ble_evt_buffer_len] = weather_key_len;
            memcpy(&ble_evt_buffer[ble_evt_buffer_len + 1], weather_key[bit], weather_key_len);

            ble_evt_buffer_len += (1 + weather_key_len);
        }
    }

    /* Fill request n_days */
    weather_key_len = sprintf((char *)&ble_evt_buffer[ble_evt_buffer_len + 1], "w.date.%d", n_days);
    ble_evt_buffer[ble_evt_buffer_len] = weather_key_len;
    ble_evt_buffer_len += (1 + weather_key_len);

    TUYA_BLE_LOG_DEBUG("request weather location=[%d] type=[0x%x], n_days=[%d]", location_type, combine_type, n_days);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("request weather data :", ble_evt_buffer, ble_evt_buffer_len);

    event.hdr.event = TUYA_BLE_EVT_WEATHER_DATA_REQ;
    event.weather_req_data.p_data = ble_evt_buffer;
    event.weather_req_data.data_len = ble_evt_buffer_len;

    if (tuya_ble_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_event_send weather req data error");
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_feature_weather_data_request(uint32_t combine_type, uint8_t n_days)
{
    return tuya_ble_feature_weather_data_request_with_location(WLT_APP_CURRENT_LOCATION, combine_type, n_days);
}


void tuya_ble_handle_weather_data_request_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t encry_mode = 0;

    encry_mode = (tuya_ble_pair_rand_valid_get()) ? (ENCRYPTION_MODE_SESSION_KEY) : (ENCRYPTION_MODE_KEY_4);

    tuya_ble_commData_send(FRM_WEATHER_DATA_REQUEST, 0, evt->weather_req_data.p_data, evt->weather_req_data.data_len, encry_mode);

    if (evt->weather_req_data.p_data) {
        tuya_ble_free(evt->weather_req_data.p_data);
    }
}


void tuya_ble_handle_weather_data_request_response(uint8_t *recv_data, uint16_t recv_len)
{
    tuya_ble_cb_evt_param_t event;
    uint16_t data_len = recv_data[11] << 8 | recv_data[12];

    if (data_len != 1) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_weather_data_request_response- invalid data len received.");
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_WEATHER_DATA_REQ_RESPONSE;
    event.weather_req_response_data.status = recv_data[13];

    if (tuya_ble_cb_event_send(&event) != 0) {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_weather_data_request_response-tuya ble send cb event failed.");
    } else {

    }
}


void tuya_ble_handle_weather_data_received(uint8_t *recv_data, uint16_t recv_len)
{
    uint8_t p_buf[1];
    uint16_t data_len = 0;
    uint32_t ack_sn = 0;
    tuya_ble_cb_evt_param_t event;
    uint16_t weather_data_start_pos, weather_data_len;
    uint16_t buffer_len, object_count;

    ack_sn  = recv_data[1] << 24;
    ack_sn += recv_data[2] << 16;
    ack_sn += recv_data[3] << 8;
    ack_sn += recv_data[4];

    data_len = (recv_data[11] << 8) | recv_data[12];

    if ((data_len < 5) || (data_len > TUYA_BLE_RECEIVE_MAX_DATA_LEN)) {
        TUYA_BLE_LOG_ERROR("cmd weather data write error,receive data len == %d", data_len);
        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_WEATHER_DATA_RECEIVED_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else if (recv_data[13] != 0x00) {
        TUYA_BLE_LOG_ERROR("cmd weather data write error, version not support");
        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_WEATHER_DATA_RECEIVED_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else if ((recv_data[14] >= WLT_MAX) || (recv_data[14] == WLT_CUSTOM_LOCATION)) {
        TUYA_BLE_LOG_ERROR("cmd weather data write error, location type this sdk version unsupport");
        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_WEATHER_DATA_RECEIVED_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    } else if (recv_data[14] != cur_req_weather_location_type) {
        TUYA_BLE_LOG_ERROR("cmd weather data write error, location type no correspondence");
        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_WEATHER_DATA_RECEIVED_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    }

    TUYA_BLE_LOG_HEXDUMP_DEBUG("cmd weather data write : ", recv_data + 13, data_len);

    /* parse location info */
    weather_data_start_pos = 15;
    weather_data_len = (data_len - 2);

    uint8_t *ble_cb_evt_buffer = (uint8_t *)tuya_ble_malloc(weather_data_len);
    if (ble_cb_evt_buffer == NULL) {
        TUYA_BLE_LOG_ERROR("ble_cb_evt_buffer malloc failed.");
        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_WEATHER_DATA_RECEIVED_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    }

    if (weather_data_lktlv_parse(&recv_data[weather_data_start_pos], weather_data_len, ble_cb_evt_buffer, &buffer_len, &object_count) != TUYA_BLE_SUCCESS) {
        tuya_ble_free(ble_cb_evt_buffer);

        TUYA_BLE_LOG_ERROR("cmd weather data parse failed");
        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_WEATHER_DATA_RECEIVED_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
        return;
    }

    event.evt = TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED;
    event.weather_received_data.object_count = object_count;
    event.weather_received_data.location = recv_data[14];
    event.weather_received_data.p_data = ble_cb_evt_buffer;
    event.weather_received_data.data_len = buffer_len;

    if (tuya_ble_cb_event_send(&event) != 0) {
        tuya_ble_free(ble_cb_evt_buffer);
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_weather_data_received-tuya ble send cb event failed.");

        p_buf[0] = 0x01;
        tuya_ble_commData_send(FRM_WEATHER_DATA_RECEIVED_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
    } else {
        p_buf[0] = 0x00;
        tuya_ble_commData_send(FRM_WEATHER_DATA_RECEIVED_RESP, ack_sn, p_buf, 1, ENCRYPTION_MODE_SESSION_KEY);
    }
}

#else

void tuya_ble_handle_weather_data_request_evt(tuya_ble_evt_param_t *evt)
{

}

void tuya_ble_handle_weather_data_request_response(uint8_t *recv_data, uint16_t recv_len)
{

}

void tuya_ble_handle_weather_data_received(uint8_t *recv_data, uint16_t recv_len)
{

}

#endif
