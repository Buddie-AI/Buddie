/**
 * \file tuya_ble_feature_weather.h
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

#ifndef TUYA_BLE_FEATURE_WEATHER_H_
#define TUYA_BLE_FEATURE_WEATHER_H_

#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"


/**@brief  Total number of weather parameters currently supported. */
#define SUPPORT_WEATHER_KEY_TYPE_MAX_NUMS	( 25 )


/**@brief	Weather key type. */
typedef enum {
    WKT_TEMP 		= (1 << 0),		/**< temperature. */
    WKT_THIHG 		= (1 << 1),		/**< high temperature. */
    WKT_TLOW 		= (1 << 2),		/**< low temperature. */
    WKT_HUMIDITY 	= (1 << 3),		/**< humidity. */
    WKT_CONDITION 	= (1 << 4), 	/**< weather condition. */
    WKT_PRESSURE 	= (1 << 5), 	/**< pressure. */
    WKT_REALFEEL 	= (1 << 6),		/**< sendible temperature. */
    WKT_UVI 		= (1 << 7),		/**< uvi. */
    WKT_SUNRISE 	= (1 << 8),		/**< sunrise. */
    WKT_SUNSET 		= (1 << 9),		/**< sunset. */
    WKT_UNIX 		= (1 << 10),	/**< unix time, Use with sunrise and sunset. */
    WKT_LOCAL 		= (1 << 11),	/**< local time, Use with sunrise and sunset. */
    WKT_WINDSPEED 	= (1 << 12),	/**< wind speed. */
    WKT_WINDDIR 	= (1 << 13),	/**< wind direction. */
    WKT_WINDLEVEL 	= (1 << 14),	/**< wind speed scale/level. */
    WKT_AQI 		= (1 << 15),	/**< aqi. */
    WKT_TIPS 		= (1 << 16),	/**< tips. */
    WKT_RANK 		= (1 << 17),	/**< Detailed AQI status and national ranking. */
    WKT_PM10 		= (1 << 18),	/**< pm10. */
    WKT_PM25		= (1 << 19),	/**< pm2.5. */
    WKT_O3			= (1 << 20),	/**< o3. */
    WKT_NO2			= (1 << 21),	/**< no2. */
    WKT_CO			= (1 << 22),	/**< co. */
    WKT_SO2			= (1 << 23),	/**< so2. */
    WKT_CONDITIONNUM = (1 << 24), 	/**< weather condition mapping id. */

    WKT_COMBINE_BITMAP_MAXVAL = (1 << SUPPORT_WEATHER_KEY_TYPE_MAX_NUMS),
} tuya_ble_weather_key_type_t;


/**@brief	Weather value type. */
typedef enum {
    WVT_INTEGER = 0,
    WVT_STRING,
} tuya_ble_weather_value_type_t;


/**@brief	Weather location type. */
typedef enum {
    WLT_PAIR_NETWORK_LOCATION = 1, 	/**< Pair network location. */
    WLT_APP_CURRENT_LOCATION, 		/**< Mobile phone current location. */
    WLT_CUSTOM_LOCATION, 			/**< The current sdk version unsupport. */

    WLT_MAX,
} tuya_ble_weather_location_type_t;


/**@brief   Weather data object structure.
 *
 */
typedef struct {
    uint8_t 						n_day;		/**< which day. */
    tuya_ble_weather_key_type_t 	key_type;	/**< weather key type. */
    tuya_ble_weather_value_type_t 	val_type;	/**< weather value type. */
    uint8_t 						value_len;	/**< weather value length. */
    char 							vaule[];	/**< weather values. */
} tuya_ble_wd_object_t;


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   Function for request weather data. Default request current location of mobile phone
 *
 * @note   	Example1: request temperatur & humidity and two days, called: tuya_ble_feature_weather_data_request((WKT_TEMP | WKT_HUMIDITY), 2);
 *			Example2: request sunrise data&time and ont day, called: tuya_ble_feature_weather_data_request((WKT_SUNRISE | WKT_LOCAL), 1);
 *			Example3: request highest temperature and seven days, called: tuya_ble_feature_weather_data_request((WKT_THIHG), 7);
 * 			In addition, request sunrise/sunset must be matched with WKT_UNIX/WKT_LOCAL, otherwise the received data is unix time
 *
 * @param[in] combine_type 	Request combine types. For details, see the enum of tuya_ble_weather_key_type_t
 * @param[in] n_days		Request forecast days, rang from [1-7]; 1-means only today, 2-means today+tomorrow ...
 *
 * @retval  TUYA_BLE_ERR_INVALID_PARAM 	The provided Parameters are not valid.
 * @retval  TUYA_BLE_ERR_NO_MEM 		If no memory is available to accept the operation.
 * @retval  TUYA_BLE_ERR_INTERNAL		If weather request message send failed.
 * @retval  TUYA_BLE_SUCCESS 			Successful.
 *
 * */
extern tuya_ble_status_t tuya_ble_feature_weather_data_request(uint32_t combine_type, uint8_t n_days);


/**
 * @brief   Function for request weather data with appoint location.
 *
 * @param[in] location_type Request location types. For details, see the enum of tuya_ble_weather_location_type_t.
 * @param[in] combine_type 	Request combine types. For details, see the enum of tuya_ble_weather_key_type_t
 * @param[in] n_days		Request forecast days, rang from [1-7]; 1-means only today, 2-means today+tomorrow ...
 *
 * @retval  TUYA_BLE_ERR_INVALID_PARAM 	The provided Parameters are not valid.
 * @retval  TUYA_BLE_ERR_NO_MEM 		If no memory is available to accept the operation.
 * @retval  TUYA_BLE_ERR_INTERNAL		If weather request message send failed.
 * @retval  TUYA_BLE_SUCCESS 			Successful.
 *
 * */
extern tuya_ble_status_t tuya_ble_feature_weather_data_request_with_location(tuya_ble_weather_location_type_t location_type, uint32_t combine_type, uint8_t n_days);

/**
 * @brief   Function for application parse received weather datas, In tuya_cb_handler() function
 * add TUYA_BLE_CB_EVT_WEATHER_DATA_REQ_RESPONSE and TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED callback
 * event and process it. For example:
 *
	case TUYA_BLE_CB_EVT_WEATHER_DATA_REQ_RESPONSE:
		TUYA_APP_LOG_INFO("received weather data request response result code =%d",event->weather_req_response_data.status);
		break;

	case TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED:
		tuya_ble_wd_object_t *object;
		uint16_t object_len = 0;
		for (;;) {
			object = (tuya_ble_wd_object_t *)(event->weather_received_data.p_data + object_len);

			TUYA_APP_LOG_DEBUG("recvived weather data, n_days=[%d] key=[0x%08x] val_type=[%d] val_len=[%d]", \
							object->n_day, object->key_type, object->val_type, object->value_len);
			TUYA_APP_LOG_HEXDUMP_DEBUG("vaule :", (uint8_t *)object->vaule, object->value_len);

			// TODO .. YOUR JOBS

			object_len += (sizeof(tuya_ble_wd_object_t) + object->value_len);
			if (object_len >= event->weather_received_data.data_len)
				break;
		}
		break;
 * */


/**
 * @brief   Function for convert weather enum type to string
 *
 * @note   	For example input convert type=WKT_TEMP, output key "w.temp"
 *
 * @param[in]  type 	Convert type
 * @param[out] key 		The ponit of weather key string
 *
 * @retval  TUYA_BLE_ERR_INVALID_PARAM 	The provided Parameters are not valid.
 * @retval  TUYA_BLE_ERR_NOT_FOUND 		No corresponding type was found.
 * @retval  TUYA_BLE_SUCCESS 			Successful.
 *
 * */
extern tuya_ble_status_t tuya_ble_feature_weather_key_enum_type_to_string(tuya_ble_weather_key_type_t type, char *key);


/**
 * @brief   Function for handler weather data request event
 *
 * @details	Internal use of tuya ble sdk
 *
 * @param[in] evt	For details, see the struct of tuya_ble_evt_param_t
 *
 * */
extern void tuya_ble_handle_weather_data_request_evt(tuya_ble_evt_param_t *evt);


/**
 * @brief   Function for handler weather data request response/ack
 *
 * @details	Internal use of tuya ble sdk
 *
 * @param[in] recv_data 	The point of recvived response data
 * @param[in] recv_len	 	The numbers of data
 *
 * */
extern void tuya_ble_handle_weather_data_request_response(uint8_t *recv_data, uint16_t recv_len);


/**
 * @brief   Function for handler weather data received
 *
 * @details	Internal use of tuya ble sdk
 *
 * @param[in] recv_data 	The point of recvived weather data
 * @param[in] recv_len	 	The numbers of data
 *
 * */
extern void tuya_ble_handle_weather_data_received(uint8_t *recv_data, uint16_t recv_len);


#ifdef __cplusplus
}
#endif

#endif /* TUYA_BLE_FEATURE_WEATHER_H_ */
