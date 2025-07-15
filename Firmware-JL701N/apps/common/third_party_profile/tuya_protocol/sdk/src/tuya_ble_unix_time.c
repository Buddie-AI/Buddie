/**
 * \file tuya_ble_unix_time.c
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
#include "tuya_ble_unix_time.h"



#define UTC_BASE_YEAR 1970
#define MONTH_PER_YEAR 12
#define DAY_PER_YEAR 365
#define SEC_PER_DAY 86400
#define SEC_PER_HOUR 3600
#define SEC_PER_MIN 60

/* Number of days per month */
static const uint8_t g_day_per_mon[MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};




/**
 * @brief   Function for Determine whether it is a leap year.
 *
 * @return  1：leap year
 * @note
 *.
 * */
static uint8_t applib_dt_is_leap_year(uint16_t year)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if ((year % 400) == 0) {
        return 1;
    } else if ((year % 100) == 0) {
        return 0;
    } else if ((year % 4) == 0) {
        return 1;
    } else {
        return 0;
    }
}


/**
 * @brief   Function for obtain the days of month.
 *
 * @return  days
 * @note
 *.
 * */
static uint8_t applib_dt_last_day_of_mon(uint8_t month, uint16_t year)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if ((month == 0) || (month > 12)) {
        return g_day_per_mon[1] + applib_dt_is_leap_year(year);
    }

    if (month != 2) {
        return g_day_per_mon[month - 1];
    } else {
        return g_day_per_mon[1] + applib_dt_is_leap_year(year);
    }
}


/**
 * @brief   Function for get the corresponding week based on the given date.
 *
 * @return
 * @note    0-Sunday  6-Saturday
 *.
 * */
static uint8_t applib_dt_dayindex(uint16_t year, uint8_t month, uint8_t day)
{
    int8_t century_code, year_code, month_code, day_code;
    int32_t week = 0;

    century_code = year_code = month_code = day_code = 0;

    if (month == 1 || month == 2) {
        century_code = (year - 1) / 100;
        year_code = (year - 1) % 100;
        month_code = month + 12;
        day_code = day;
    } else {
        century_code = year / 100;
        year_code = year % 100;
        month_code = month;
        day_code = day;
    }

    week = year_code + year_code / 4 + century_code / 4 - 2 * century_code + 26 * (month_code + 1) / 10 + day_code - 1;
    week = week > 0 ? (week % 7) : ((week % 7) + 7);

    return week;
}


/**
 * @brief   Function for Get the corresponding date according to the UTC timestamp.
 *
 * @param[in] daylightSaving:daylight saving time
 * @return
 * @note
 *.
 * */
void tuya_ble_utc_sec_2_mytime(uint32_t utc_sec, tuya_ble_time_struct_data_t *result, bool daylightSaving)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    int32_t sec, day;
    uint16_t y;
    uint8_t m;
    uint16_t d;
    // uint8_t dst;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    if (daylightSaving) {
        utc_sec += SEC_PER_HOUR;
    }

    /* hour, min, sec */
    /* hour */
    sec = utc_sec % SEC_PER_DAY;
    result->nHour = sec / SEC_PER_HOUR;

    /* min */
    sec %= SEC_PER_HOUR;
    result->nMin = sec / SEC_PER_MIN;

    /* sec */
    result->nSec = sec % SEC_PER_MIN;

    /* year, month, day */
    /* year */
    /* year */
    day = utc_sec / SEC_PER_DAY;
    for (y = UTC_BASE_YEAR; day > 0; y++) {
        d = (DAY_PER_YEAR + applib_dt_is_leap_year(y));
        if (day >= d) {
            day -= d;
        } else {
            break;
        }
    }

    result->nYear = y;

    for (m = 1; m < MONTH_PER_YEAR; m++) {
        d = applib_dt_last_day_of_mon(m, y);
        if (day >= d) {
            day -= d;
        } else {
            break;
        }
    }

    result->nMonth = m;
    result->nDay = (uint8_t)(day + 1);

    result->DayIndex = applib_dt_dayindex(result->nYear, result->nMonth, result->nDay);
}

/**
 * @brief   Function for Get the UTC timestamp according to  corresponding date .
 *
 * @param[in] daylightSaving:daylight saving time
 * @return
 * @note
 *.
 * */
uint32_t tuya_ble_mytime_2_utc_sec(tuya_ble_time_struct_data_t *currTime, bool daylightSaving)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    uint16_t i;
    uint32_t no_of_days = 0;
    uint32_t utc_time;
    uint8_t dst = 1;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (currTime->nYear < UTC_BASE_YEAR) {
        return 0;
    }

    /* year */
    for (i = UTC_BASE_YEAR; i < currTime->nYear; i++) {
        no_of_days += (DAY_PER_YEAR + applib_dt_is_leap_year(i));
    }

    /* month */
    for (i = 1; i < currTime->nMonth; i++) {
        no_of_days += applib_dt_last_day_of_mon((unsigned char) i, currTime->nYear);
    }

    /* day */
    no_of_days += (currTime->nDay - 1);

    /* sec */
    utc_time = (unsigned int) no_of_days * SEC_PER_DAY + (unsigned int)(currTime->nHour * SEC_PER_HOUR +
               currTime->nMin * SEC_PER_MIN + currTime->nSec);

    if (dst && daylightSaving) {
        utc_time -= SEC_PER_HOUR;
    }

    return utc_time;
}


void tuya_ble_utc_sec_2_mytime_string(uint32_t utc_sec, bool daylightSaving, char *s)
{
    tuya_ble_time_struct_data_t rtc_time = {0};
    uint32_t temp = 0;

    tuya_ble_utc_sec_2_mytime(utc_sec, &rtc_time, daylightSaving);

    temp = rtc_time.nYear;
    s[0] = temp / 1000 + 0x30;
    temp = rtc_time.nYear % 1000;
    s[1] = temp / 100 + 0x30;
    temp = temp % 100;
    s[2] = temp / 10 + 0x30;
    s[3] = temp % 10 + 0x30;
    s[4] = '-';
    s[5] = rtc_time.nMonth / 10 + 0x30;
    s[6] = rtc_time.nMonth % 10 + 0x30;
    s[7] = '-';
    s[8] = rtc_time.nDay / 10 + 0x30;
    s[9] = rtc_time.nDay % 10 + 0x30;
    s[10] = ' ';
    s[11] = rtc_time.nHour / 10 + 0x30;
    s[12] = rtc_time.nHour % 10 + 0x30;
    s[13] = ':';
    s[14] = rtc_time.nMin / 10 + 0x30;
    s[15] = rtc_time.nMin % 10 + 0x30;
    s[16] = ':';
    s[17] = rtc_time.nSec / 10 + 0x30;
    s[18] = rtc_time.nSec % 10 + 0x30;
    s[19] = '\0';
}



