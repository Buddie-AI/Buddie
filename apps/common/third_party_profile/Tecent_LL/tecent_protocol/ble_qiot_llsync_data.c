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

//#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ble_qiot_config.h"

#if BLE_QIOT_LLSYNC_STANDARD

#include "ble_qiot_export.h"
#include "ble_qiot_import.h"
#include "ble_qiot_common.h"
#include "ble_qiot_llsync_event.h"
#include "ble_qiot_utils_base64.h"
#include "ble_qiot_log.h"
#include "ble_qiot_param_check.h"
#include "ble_qiot_service.h"
#include "ble_qiot_template.h"
#include "ble_qiot_llsync_data.h"

// parse tlv data and return the length parsed
int ble_lldata_parse_tlv(const char *buf, int buf_len, e_ble_tlv *tlv)
{
    int      ret_len  = 0;
    uint16_t type_len = 0;

    tlv->type = BLE_QIOT_PARSE_TLV_HEAD_TYPE(buf[0]);
    if (tlv->type >= BLE_QIOT_DATA_TYPE_BUTT) {
        ble_qiot_log_e("lldata parse invalid type: %d", tlv->type);
        return -1;
    }
    tlv->id = BLE_QIOT_PARSE_TLV_HEAD_ID(buf[0]);
    ret_len++;

    switch (tlv->type) {
    case BLE_QIOT_DATA_TYPE_BOOL:
        tlv->len = sizeof(uint8_t);
        tlv->val = (char *)buf + ret_len;
        ret_len += sizeof(uint8_t);
        break;
    case BLE_QIOT_DATA_TYPE_ENUM:
        tlv->len = sizeof(uint16_t);
        tlv->val = (char *)buf + ret_len;
        ret_len += sizeof(uint16_t);
        break;
    case BLE_QIOT_DATA_TYPE_INT:
    case BLE_QIOT_DATA_TYPE_FLOAT:
    case BLE_QIOT_DATA_TYPE_TIME:
        tlv->len = sizeof(uint32_t);
        tlv->val = (char *)buf + ret_len;
        ret_len += sizeof(uint32_t);
        break;
    case BLE_QIOT_DATA_TYPE_STRUCT:
    case BLE_QIOT_DATA_TYPE_STRING:
        if (buf_len < BLE_QIOT_MIN_STRING_TYPE_LEN) {
            ble_qiot_log_e("lldata len invalid, type: %d, len: %d", tlv->type, buf_len);
            return -1;
        }
        memcpy(&type_len, &buf[ret_len], sizeof(uint16_t));
        tlv->len = NTOHS(type_len);
        ret_len += sizeof(uint16_t);
        tlv->val = (char *)buf + ret_len;
        ret_len += tlv->len;
        break;
    default:
        break;
    }
    ble_qiot_log_d("tlv parsed, type: %d, id: %d, len: %d", tlv->type, tlv->id, tlv->len);

    return ret_len;
}

// handle property data, method is control or get status
static ble_qiot_ret_status_t ble_lldata_property_data_handle(bool is_request, const char *in_buf, int buf_len)
{
#ifdef BLE_QIOT_INCLUDE_PROPERTY
    uint16_t  parse_len = 0;
    int       ret_len   = 0;
    e_ble_tlv tlv;
    int8_t    ret = BLE_QIOT_REPLY_SUCCESS;

    ble_qiot_ret_status_t inform_ret = BLE_QIOT_RS_OK;

    ble_qiot_log_d("handle property data");
    while (parse_len < buf_len) {
        memset(&tlv, 0, sizeof(e_ble_tlv));
        ret_len = ble_lldata_parse_tlv(in_buf + parse_len, buf_len - parse_len, &tlv);
        if (ret_len < 0) {
            return BLE_QIOT_RS_ERR;
        }

        parse_len += ret_len;
        if (parse_len > buf_len) {
            ble_qiot_log_e("invalid peroperty data, %d(property len) > %d(buf len)", parse_len, buf_len);
            ret = BLE_QIOT_REPLY_DATA_ERR;
            break;
        }

        if (BLE_QIOT_RS_OK != ble_user_property_set_data(&tlv)) {
            ret = BLE_QIOT_REPLY_FAIL;
            break;
        }
    }
    if (is_request) {
        return ble_event_notify(BLE_QIOT_EVENT_UP_CONTROL_REPLY, NULL, 0, (const char *)&ret, sizeof(int8_t));
    } else {
        return (BLE_QIOT_REPLY_SUCCESS == ret) ? BLE_QIOT_RS_OK : BLE_QIOT_RS_ERR;
    }
#else
    ble_qiot_log_e("property" BLE_QIOT_NOT_SUPPORT_WARN);
    return BLE_QIOT_RS_OK;
#endif //BLE_QIOT_INCLUDE_PROPERTY
}

#ifdef BLE_QIOT_INCLUDE_PROPERTY
// post property
ble_qiot_ret_status_t ble_user_property_get_report_data(void)
{
    uint8_t  property_id   = 0;
    uint8_t  property_type = 0;
    int      property_len  = 0;
    uint16_t type_len      = 0;
    uint16_t data_len      = 0;
    uint16_t data_buf_off  = 0;

    uint8_t data_buf[BLE_QIOT_EVENT_MAX_SIZE] = {0};

    ble_qiot_log_d("report property");
    for (property_id = 0; property_id < BLE_QIOT_PROPERTY_ID_BUTT; property_id++) {
        property_type = ble_get_property_type_by_id(property_id);
        if (property_type >= BLE_QIOT_DATA_TYPE_BUTT) {
            ble_qiot_log_e("property(%d) type(%d) invalid", property_id, property_type);
            return BLE_QIOT_RS_ERR;
        }

        data_buf[data_len++] = BLE_QIOT_PACKAGE_TLV_HEAD(property_type, property_id);
        data_buf_off         = data_len;
        if ((BLE_QIOT_DATA_TYPE_STRING == property_type) || (BLE_QIOT_DATA_TYPE_STRUCT == property_type)) {
            // reserved 2 bytes for string/struct length
            data_buf_off += BLE_QIOT_STRING_TYPE_LEN;
        }
        property_len = ble_user_property_get_data_by_id(property_id, (char *)&data_buf[data_buf_off],
                       sizeof(data_buf) - data_buf_off);
        if (property_len < 0) {
            return BLE_QIOT_RS_ERR;
        } else if (property_len == 0) {
            // clean the property head cause no data to post
            data_len--;
            data_buf[data_len] = '0';
            ble_qiot_log_d("property: %d no data to post", property_id);
        } else {
            if ((BLE_QIOT_DATA_TYPE_STRING == property_type) || (BLE_QIOT_DATA_TYPE_STRUCT == property_type)) {
                // filling the payload length
                type_len = HTONS(property_len);
                memcpy(data_buf + data_len, &type_len, sizeof(uint16_t));
                data_len += sizeof(uint16_t);
            }
            data_len += property_len;
        }
    }
    // ble_qiot_log_hex(BLE_QIOT_LOG_LEVEL_INFO, "user data", data_buf, data_len);

    return ble_event_notify(BLE_QIOT_EVENT_UP_PROPERTY_REPORT, NULL, 0, (const char *)data_buf, data_len);
}
#endif //BLE_QIOT_INCLUDE_PROPERTY

// handle control
ble_qiot_ret_status_t ble_lldata_property_request_handle(const char *in_buf, int buf_len)
{
    return ble_lldata_property_data_handle(true, in_buf, buf_len);
}

// handle report_reply
static ble_qiot_ret_status_t ble_lldata_property_report_reply(const char *in_buf, int buf_len)
{
#ifdef BLE_QIOT_INCLUDE_PROPERTY
    (void)ble_user_property_report_reply_handle(in_buf[0]);
#else
    ble_qiot_log_e("property" BLE_QIOT_NOT_SUPPORT_WARN);
#endif //BLE_QIOT_INCLUDE_PROPERTY
    return BLE_QIOT_RS_OK;
}

// handle get_status_reply
static ble_qiot_ret_status_t ble_lldata_property_get_status_reply(const char *in_buf, int buf_len)
{
#ifdef BLE_QIOT_INCLUDE_PROPERTY
    uint8_t  result    = 0;
    uint16_t reply_len = 0;

    result = in_buf[0];
    ble_qiot_log_d("get status reply result: %d", result);
    if (BLE_QIOT_REPLY_SUCCESS == result) {
        memcpy(&reply_len, &in_buf[1], sizeof(uint16_t));
        return ble_lldata_property_data_handle(false, in_buf + sizeof(uint8_t) + sizeof(uint16_t), NTOHS(reply_len));
    } else {
        return BLE_QIOT_RS_ERR;
    }
#else
    ble_qiot_log_e("property" BLE_QIOT_NOT_SUPPORT_WARN);
    return BLE_QIOT_RS_OK;
#endif //BLE_QIOT_INCLUDE_PROPERTY
}

// handle reply from remote
ble_qiot_ret_status_t ble_lldata_property_reply_handle(uint8_t type, const char *in_buf, int buf_len)
{
    switch (type) {
    case BLE_QIOT_DATA_DOWN_REPORT_REPLY:
        return ble_lldata_property_report_reply(in_buf, buf_len);
    case BLE_QIOT_DATA_DOWN_GET_STATUS_REPLY:
        return ble_lldata_property_get_status_reply(in_buf, buf_len);
    default:
        ble_qiot_log_e("invalid property reply type");
        return BLE_QIOT_RS_ERR;
    }
}

// handle event_post_reply
ble_qiot_ret_status_t ble_lldata_event_handle(uint8_t id, const char *in_buf, int len)
{
#ifdef BLE_QIOT_INCLUDE_EVENT
    (void)ble_user_event_reply_handle(id, in_buf[0]);
#else
    ble_qiot_log_e("event" BLE_QIOT_NOT_SUPPORT_WARN);
#endif //BLE_QIOT_INCLUDE_PROPERTY
    return BLE_QIOT_RS_OK;
}

// handle action
ble_qiot_ret_status_t ble_lldata_action_handle(uint8_t action_id, const char *in_buf, int len)
{
#ifdef BLE_QIOT_INCLUDE_ACTION
    POINTER_SANITY_CHECK(in_buf, BLE_QIOT_RS_ERR_PARA);

    uint16_t parse_len        = 0;
    uint8_t  tlv_index        = 0;
    int      ret_len          = 0;
    int      handle_ret       = BLE_QIOT_REPLY_SUCCESS;
    uint8_t  output_id        = 0;
    uint8_t  output_type      = 0;
    uint16_t data_len         = 0;
    uint16_t data_buf_off     = 0;
    int      output_param_len = 0;
    uint16_t string_len       = 0;
    uint8_t  header_buf[2]    = {0};

    e_ble_tlv tlv[BLE_QIOT_ACTION_INPUT_ID_BUTT]                = {0};
    uint8_t   output_flag_array[BLE_QIOT_ACTION_OUTPUT_ID_BUTT] = {0};  // set true if the id have output
    uint8_t   data_buf[BLE_QIOT_EVENT_MAX_SIZE]                 = {0};

    header_buf[0] = BLE_QIOT_REPLY_SUCCESS;
    header_buf[1] = action_id;

    ble_qiot_log_d("input action: %d", action_id);
    while (parse_len < len) {
        if (tlv_index >= BLE_QIOT_ACTION_INPUT_ID_BUTT) {
            ble_qiot_log_e("invalid action(id: %d) data", action_id);
            handle_ret = BLE_QIOT_REPLY_DATA_ERR;
            goto end;
        }
        ret_len = ble_lldata_parse_tlv(in_buf + parse_len, len - parse_len, &tlv[tlv_index]);
        if (ret_len < 0) {
            handle_ret = BLE_QIOT_REPLY_DATA_ERR;
            goto end;
        }
        parse_len += ret_len;
        tlv_index++;
        if (parse_len > len) {
            ble_qiot_log_e("action data parse failed, parse len: %d > data len: %d", parse_len, len);
            handle_ret = BLE_QIOT_REPLY_DATA_ERR;
            goto end;
        }
    }
    if (0 != ble_action_user_handle_input_param(action_id, tlv, tlv_index, output_flag_array)) {
        handle_ret = BLE_QIOT_REPLY_FAIL;
        goto end;
    }

    for (output_id = 0; output_id < BLE_QIOT_ACTION_OUTPUT_ID_BUTT; output_id++) {
        if (output_flag_array[output_id]) {
            output_type = ble_action_get_output_type_by_id(action_id, output_id);
            if (output_type >= BLE_QIOT_DATA_TYPE_BUTT) {
                ble_qiot_log_e("action id(%d:%d) type invalid", action_id, output_id);
                handle_ret = BLE_QIOT_REPLY_FAIL;
                goto end;
            }
            data_buf[data_len++] = BLE_QIOT_PACKAGE_TLV_HEAD(output_type, output_id);
            data_buf_off         = data_len;
            if (BLE_QIOT_DATA_TYPE_STRING == output_type) {
                data_buf_off += BLE_QIOT_STRING_TYPE_LEN;
            }
            output_param_len = ble_action_user_handle_output_param(
                                   action_id, output_id, (char *)data_buf + data_buf_off, sizeof(data_buf) - data_buf_off);
            if (output_param_len < 0) {
                handle_ret = BLE_QIOT_REPLY_FAIL;
                goto end;
            } else if (output_param_len == 0) {
                // clean the head cause no data to post
                data_len--;
                data_buf[data_len] = '0';
                ble_qiot_log_d("action id(%d: %d) no data to post", action_id, output_id);
            } else {
                if (BLE_QIOT_DATA_TYPE_STRING == output_type) {
                    string_len = HTONS(output_param_len);
                    memcpy(data_buf + data_len, &string_len, sizeof(uint16_t));
                    data_len += sizeof(uint16_t);
                }
                data_len += output_param_len;
            }
        }
    }
    // ble_qiot_log_hex(BLE_QIOT_LOG_LEVEL_INFO, "user data", data_buf, data_len);
    return ble_event_notify(BLE_QIOT_EVENT_UP_ACTION_REPLY, header_buf, sizeof(header_buf), (const char *)data_buf,
                            data_len);

end:
    header_buf[0] = BLE_QIOT_REPLY_FAIL;
    ble_event_notify(BLE_QIOT_EVENT_UP_ACTION_REPLY, header_buf, sizeof(header_buf), (const char *)&handle_ret,
                     sizeof(uint8_t));
    return BLE_QIOT_RS_ERR;
#else
    ble_qiot_log_e("action" BLE_QIOT_NOT_SUPPORT_WARN);
    return BLE_QIOT_RS_OK;
#endif //BLE_QIOT_INCLUDE_PROPERTY
}

#endif //BLE_QIOT_LLSYNC_STANDARD

#ifdef __cplusplus
}
#endif
