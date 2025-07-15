/*
* Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
* Licensed under the MIT License (the "License"); you may not use this file except in
* compliance with the License. You may obtain a copy of the License at
* http://opensource.org/licenses/MIT
* Unless required by applicable law or agreed to in writing, software distributed under the License is
* distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
* either express or implied. See the License for the specific language governing permissions and
* limitations under the License.
*
*/
#ifdef __cplusplus
extern "C" {
#endif

#include "ble_qiot_template.h"

#include <printf.h>
//#include <stdbool.h>
#include <string.h>

#include "ble_qiot_export.h"
#include "ble_qiot_common.h"
#include "ble_qiot_param_check.h"
#include "ble_qiot_llsync_event.h"
#include "ble_qiot_service.h"
#include "ble_qiot_llsync_device.h"
#include "gpio.h"

static uint8_t LL_led = 0;
#define LLSYNC_SWITCH_PIN IO_PORTA_01
static int ble_property_power_switch_set(const char *data, uint16_t len)
{
    printf("ble_property_power_switch_set led to %d\n", data[0]);
    LL_led = data[0];
    gpio_write(LLSYNC_SWITCH_PIN, LL_led);
    return 0;
}

static int ble_property_power_switch_get(char *data, uint16_t buf_len)
{
    printf("ble_property_power_switch_get, led state is %d\n", LL_led);
    data[0] = LL_led;
    return sizeof(uint8_t);
}

static ble_property_t sg_ble_property_array[BLE_QIOT_PROPERTY_ID_BUTT] = {
    {ble_property_power_switch_set, ble_property_power_switch_get, BLE_QIOT_PROPERTY_AUTH_RW, BLE_QIOT_DATA_TYPE_BOOL},
};

ble_qiot_ret_status_t ble_event_report_device_switch_state(u8 led_state)
{
    char device_info[2] = {0};

    device_info[0] = BLE_QIOT_PROPERTY_ID_POWER_SWITCH;
    device_info[1] = led_state;

    return ble_event_notify(BLE_QIOT_EVENT_UP_PROPERTY_REPORT, NULL, 0, device_info, sizeof(device_info));
}

void ll_sync_led_switch(void)
{
    LL_led = !LL_led;

    printf("ll_sync set led to %d\n", LL_led);
    gpio_write(LLSYNC_SWITCH_PIN, LL_led);
    ble_event_report_device_switch_state(LL_led);
}

void ll_sync_unbind(void)
{
    printf("ll_sync factory data reset\n");
    ble_unbind_write_result();
}

void llsync_device_state_sync(void)
{
    printf("llsync_device_state_sync, led = %d\n", LL_led);
    ble_event_report_device_switch_state(LL_led);
}

static bool ble_check_space_enough_by_type(uint8_t type, uint16_t left_size)
{
    switch (type) {
    case BLE_QIOT_DATA_TYPE_BOOL:
        return left_size >= sizeof(uint8_t);
    case BLE_QIOT_DATA_TYPE_INT:
    case BLE_QIOT_DATA_TYPE_FLOAT:
    case BLE_QIOT_DATA_TYPE_TIME:
        return left_size >= sizeof(uint32_t);
    case BLE_QIOT_DATA_TYPE_ENUM:
        return left_size >= sizeof(uint16_t);
    default:
        // string length is unknow, default true
        return true;
    }
}

static uint16_t ble_check_ret_value_by_type(uint8_t type, uint16_t buf_len, uint16_t ret_val)
{
    switch (type) {
    case BLE_QIOT_DATA_TYPE_BOOL:
        return ret_val <= sizeof(uint8_t);
    case BLE_QIOT_DATA_TYPE_INT:
    case BLE_QIOT_DATA_TYPE_FLOAT:
    case BLE_QIOT_DATA_TYPE_TIME:
        return ret_val <= sizeof(uint32_t);
    case BLE_QIOT_DATA_TYPE_ENUM:
        return ret_val <= sizeof(uint16_t);
    default:
        // string length is unknow, default true
        return ret_val <= buf_len;
    }
}

uint8_t ble_get_property_type_by_id(uint8_t id)
{
    if (id >= BLE_QIOT_PROPERTY_ID_BUTT) {
        ble_qiot_log_e("invalid property id %d", id);
        return BLE_QIOT_DATA_TYPE_BUTT;
    }
    return sg_ble_property_array[id].type;
}

int ble_user_property_set_data(const e_ble_tlv *tlv)
{
    POINTER_SANITY_CHECK(tlv, BLE_QIOT_RS_ERR_PARA);
    if (tlv->id >= BLE_QIOT_PROPERTY_ID_BUTT) {
        ble_qiot_log_e("invalid property id %d", tlv->id);
        return BLE_QIOT_RS_ERR;
    }

    if (NULL != sg_ble_property_array[tlv->id].set_cb) {
        if (0 != sg_ble_property_array[tlv->id].set_cb(tlv->val, tlv->len)) {
            ble_qiot_log_e("set property id %d failed", tlv->id);
            return BLE_QIOT_RS_ERR;
        } else {
            return BLE_QIOT_RS_OK;
        }
    }
    ble_qiot_log_e("invalid set callback, id %d", tlv->id);

    return BLE_QIOT_RS_ERR;
}

int ble_user_property_get_data_by_id(uint8_t id, char *buf, uint16_t buf_len)
{
    int ret_len = 0;

    POINTER_SANITY_CHECK(buf, BLE_QIOT_RS_ERR_PARA);
    if (id >= BLE_QIOT_PROPERTY_ID_BUTT) {
        ble_qiot_log_e("invalid property id %d", id);
        return -1;
    }

    if (NULL != sg_ble_property_array[id].get_cb) {
        if (!ble_check_space_enough_by_type(sg_ble_property_array[id].type, buf_len)) {
            ble_qiot_log_e("not enough space get property id %d data", id);
            return -1;
        }
        ret_len = sg_ble_property_array[id].get_cb(buf, buf_len);
        if (ret_len < 0) {
            ble_qiot_log_e("get property id %d data failed", id);
            return -1;
        } else {
            if (ble_check_ret_value_by_type(sg_ble_property_array[id].type, buf_len, ret_len)) {
                return ret_len;
            } else {
                ble_qiot_log_e("property id %d length invalid", id);
                return -1;
            }
        }
    }
    ble_qiot_log_e("invalid callback, property id %d", id);

    return 0;
}

int ble_user_property_report_reply_handle(uint8_t result)
{
    ble_qiot_log_d("report reply result %d", result);

    return BLE_QIOT_RS_OK;
}

int ble_user_property_struct_handle(const char *in_buf, uint16_t buf_len, ble_property_t struct_arr[], uint8_t arr_size)
{
    uint16_t              parse_len = 0;
    uint16_t              ret_len   = 0;
    e_ble_tlv             tlv;

    while (parse_len < buf_len) {
        memset(&tlv, 0, sizeof(e_ble_tlv));
        ret_len = ble_lldata_parse_tlv(in_buf + parse_len, buf_len - parse_len, &tlv);
        parse_len += ret_len;
        if (parse_len > buf_len) {
            ble_qiot_log_e("parse struct failed");
            return parse_len;
        }

        if (tlv.id >= arr_size) {
            ble_qiot_log_e("invalid array index %d", tlv.id);
            return parse_len;
        }
        if (NULL == struct_arr[tlv.id].set_cb) {
            ble_qiot_log_e("invalid member id %d", tlv.id);
            return parse_len;
        }
        if (BLE_QIOT_RS_OK != struct_arr[tlv.id].set_cb(tlv.val, tlv.len)) {
            ble_qiot_log_e("user handle property error, member id %d, type %d, len %d", tlv.id, tlv.type, tlv.len);
            return parse_len;
        }
    }

    return 0;
}

int ble_user_property_struct_get_data(char *in_buf, uint16_t buf_len, ble_property_t struct_arr[], uint8_t arr_size)
{
    uint8_t  property_id                       = 0;
    uint8_t  property_type                     = 0;
    int      property_len                      = 0;
    char     *data_buf                         = in_buf;
    uint16_t data_len                          = 0;
    uint16_t string_len                        = 0;

    for (property_id = 0; property_id < arr_size; property_id++) {
        property_type = struct_arr[property_id].type;
        if (property_type >= BLE_QIOT_DATA_TYPE_BUTT) {
            ble_qiot_log_e("member id %d type %d invalid", property_id, property_type);
            return BLE_QIOT_RS_ERR;
        }
        data_buf[data_len++] = BLE_QIOT_PACKAGE_TLV_HEAD(property_type, property_id);
        if (BLE_QIOT_DATA_TYPE_STRING == property_type) {
            // reserved 2 bytes for string length
            property_len = struct_arr[property_id].get_cb((char *)data_buf + data_len + 2, buf_len - data_len - 2);
        } else {
            property_len = struct_arr[property_id].get_cb((char *)data_buf + data_len, buf_len - data_len);
        }
        if (property_len < 0) {
            ble_qiot_log_e("too long data, member id %d, data length %d", property_id, data_len);
            return BLE_QIOT_RS_ERR;
        } else if (property_len == 0) {
            // no data to post
            data_len--;
            data_buf[data_len] = '0';
            ble_qiot_log_d("member id %d no data to post", property_id);
        } else {
            if (BLE_QIOT_DATA_TYPE_STRING == property_type) {
                string_len = HTONS(property_len);
                memcpy(data_buf + data_len, &string_len, sizeof(uint16_t));
                data_len += sizeof(uint16_t);
            }
            data_len += property_len;
        }
    }

    return data_len;
}


#ifdef __cplusplus
}
#endif
