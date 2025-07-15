/**
 * \file tuya_ble_unix_time.h
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

#ifndef TUYA_BLE_UNIX_TIME_H_
#define TUYA_BLE_UNIX_TIME_H_

#include "tuya_ble_stdlib.h"

/* time struct */
typedef struct {
    uint16_t nYear;
    uint8_t nMonth;
    uint8_t nDay;
    uint8_t nHour;
    uint8_t nMin;
    uint8_t nSec;
    uint8_t DayIndex; /* 0 = Sunday */
} tuya_ble_time_struct_data_t;


void tuya_ble_utc_sec_2_mytime(uint32_t utc_sec, tuya_ble_time_struct_data_t *result, bool daylightSaving);

void tuya_ble_utc_sec_2_mytime_string(uint32_t utc_sec, bool daylightSaving, char *s);

uint32_t tuya_ble_mytime_2_utc_sec(tuya_ble_time_struct_data_t *currTime, bool daylightSaving);

#endif



