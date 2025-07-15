/**
 * \file tuya_ble_app_production_test.c
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
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_app_production_test.h"
#include "tuya_ble_log.h"
#include "tuya_ble_api.h"
#include "tuya_ble_utils.h"


#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)
#include "tuya_ecc.h"
#if (TUYA_BLE_INCLUDE_CJSON_COMPONENTS != 0)
#include "cJSON.h"
#endif
#endif

#if defined(CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE)
#include CUSTOMIZED_TUYA_BLE_APP_PRODUCT_TEST_HEADER_FILE
#endif


#if (TUYA_BLE_DEVICE_REGISTER_FROM_BLE&&TUYA_BLE_DEVICE_AUTH_DATA_STORE)

static uint8_t tuya_ble_production_test_flag = 0;

static uint8_t tuya_ble_production_test_with_ble_flag = 0;

#define tuya_ble_prod_monitor_timeout_ms  60000  //60s

tuya_ble_timer_t tuya_ble_xTimer_prod_monitor;


void tuya_ble_internal_production_test_with_ble_flag_clear(void)
{
    tuya_ble_production_test_with_ble_flag = 0;
}


uint8_t tuya_ble_internal_production_test_with_ble_flag_get(void)
{
    return tuya_ble_production_test_with_ble_flag;
}


static void tuya_ble_vtimer_prod_monitor_callback(tuya_ble_timer_t pxTimer)
{
    tuya_ble_device_delay_ms(1000);
    tuya_ble_device_reset();

}

static void tuya_ble_prod_monitor_timer_init(void)
{
    if (tuya_ble_timer_create(&tuya_ble_xTimer_prod_monitor, tuya_ble_prod_monitor_timeout_ms, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_vtimer_prod_monitor_callback) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xTimer_prod_monitor creat failed");
    }

}


static void tuya_ble_prod_monitor_timer_start(void)
{
    if (tuya_ble_timer_start(tuya_ble_xTimer_prod_monitor) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xTimer_prod_monitor start failed");
    }

}

static void tuya_ble_prod_monitor_timer_stop(void)
{

    if (tuya_ble_timer_stop(tuya_ble_xTimer_prod_monitor) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("tuya_ble_xTimer_prod_monitor stop failed");
    }

}


static uint32_t tuya_ble_uart_prod_send(uint8_t type, uint8_t *pdata, uint8_t len)
{
    uint8_t uart_send_len = 7 + len;
    uint8_t *uart_send_buffer = NULL;

    uart_send_buffer = (uint8_t *)tuya_ble_malloc(uart_send_len);
    if (uart_send_buffer != NULL) {
        uart_send_buffer[0] = 0x66;
        uart_send_buffer[1] = 0xAA;
        uart_send_buffer[2] = 0x00;
        uart_send_buffer[3] = type;
        uart_send_buffer[4] = 0;
        uart_send_buffer[5] = len;
        memcpy(uart_send_buffer + 6, pdata, len);
        uart_send_buffer[6 + len] = tuya_ble_check_sum(uart_send_buffer, 6 + len);
        tuya_ble_common_uart_send_data(uart_send_buffer, 7 + len);
        tuya_ble_free(uart_send_buffer);
    } else {
        TUYA_BLE_LOG_ERROR("uart prod send buffer malloc failed.");
        return 1;
    }

    return 0;
}


__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_updata_mac(uint8_t *mac)
{
    return TUYA_BLE_SUCCESS;
}

__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_beacon_scan_start(void)
{
    return TUYA_BLE_SUCCESS;
}

__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_beacon_scan_stop(void)
{
    return TUYA_BLE_SUCCESS;
}

__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_beacon_get_rssi_avg(int8_t *rssi)
{
    *rssi = -20;
    return TUYA_BLE_SUCCESS;
}


static void tuya_ble_auc_enter(uint8_t *para, uint16_t len)
{
    uint8_t buf[1];

    /* if dev is binding, can't entry ftm mode */
    if (tuya_ble_current_para.sys_settings.bound_flag == 1) {
        TUYA_BLE_LOG_DEBUG("AUC ENTER, BUT DEV IS BINDING");

        tuya_ble_device_delay_ms(200);
        tuya_ble_device_reset();
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC ENTER!");

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)
    buf[0] = (TUYA_BLE_AUC_FINGERPRINT_VER << TUYA_BLE_AUC_FW_FINGERPRINT_POS) | \
             (TUYA_BLE_AUC_WRITE_PID << TUYA_BLE_AUC_WRITE_PID_POS) |
             (TUYA_BLE_AUC_WRITE_DEV_CERT << TUYA_BLE_AUC_WRITE_DEV_CERT_POS);
#else
    buf[0] = (TUYA_BLE_AUC_FINGERPRINT_VER << TUYA_BLE_AUC_FW_FINGERPRINT_POS) | \
             (TUYA_BLE_AUC_WRITE_PID << TUYA_BLE_AUC_WRITE_PID_POS);
#endif

    if (tuya_ble_production_test_flag == 1) {
        //tuya_ble_stop_scan();
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_ENTER, buf, 1);
        return;
    }
    tuya_ble_prod_monitor_timer_init();

    tuya_ble_prod_monitor_timer_start();

    tuya_ble_prod_beacon_scan_start();

    tuya_ble_production_test_flag = 1;

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_ENTER, buf, 1);
}


static  void tuya_ble_auc_query_hid(uint8_t *para, uint16_t len)
{
    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC QUERY HID!");
    char buf[70] = "{\"ret\":true,\"hid\":\"\"}";


    if (tuya_ble_buffer_value_is_all_x(tuya_ble_current_para.auth_settings.h_id, H_ID_LEN, 0xFF)) {
        buf[19] = '\"';
        buf[20] = '}';
    } else if (tuya_ble_buffer_value_is_all_x(tuya_ble_current_para.auth_settings.h_id, H_ID_LEN, 0)) {
        buf[19] = '\"';
        buf[20] = '}';
    } else {
        memcpy(&buf[19], tuya_ble_current_para.auth_settings.h_id, H_ID_LEN);
        buf[38] = '\"';
        buf[39] = '}';
    }

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_QUERY_HID, (uint8_t *)buf, strlen(buf));

    TUYA_BLE_LOG_HEXDUMP_DEBUG("AUC QUERY HID response data : ", (uint8_t *)buf, strlen(buf));
}



__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_gpio_test(void)
{
    return TUYA_BLE_SUCCESS;
}


static void tuya_ble_auc_gpio_test(uint8_t *para, uint16_t len)
{
    char ture_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";
    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC GPIO TEST!");

    if (tuya_ble_prod_gpio_test() == TUYA_BLE_SUCCESS) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_GPIO_TEST, (uint8_t *)ture_buf, strlen(ture_buf));
        TUYA_BLE_LOG_DEBUG("AUC GPIO TEST successed!");
    } else {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_GPIO_TEST, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC GPIO TEST failed!");
    }

}


static void tuya_ble_prod_asciitohex(uint8_t *ascbuf, uint8_t len, uint8_t *hexbuf)
{
    uint8_t i = 0, j = 0;

    for (j = 0; j < (len / 2); j++) {
        if ((ascbuf[i] >= 0x30) && (ascbuf[i] <= 0x39)) {
            hexbuf[j] = ((ascbuf[i] - 0x30) << 4);
        } else if ((ascbuf[i] >= 65) && (ascbuf[i] <= 70)) {
            hexbuf[j] = ((ascbuf[i] - 55) << 4);
        } else if ((ascbuf[i] >= 97) && (ascbuf[i] <= 102)) {
            hexbuf[j] = ((ascbuf[i] - 87) << 4);
        }
        i++;
        if ((ascbuf[i] >= 0x30) && (ascbuf[i] <= 0x39)) {
            hexbuf[j] |= (ascbuf[i] - 0x30);
        } else if ((ascbuf[i] >= 65) && (ascbuf[i] <= 70)) {
            hexbuf[j] |= (ascbuf[i] - 55);
        } else if ((ascbuf[i] >= 97) && (ascbuf[i] <= 102)) {
            hexbuf[j] |= (ascbuf[i] - 87);
        }
        i++;

    }

}

static  void tuya_ble_auc_write_auth_info(uint8_t *para, uint16_t len)
{
    uint8_t mac_temp[6];
    uint8_t mac_char[13];
    uint8_t pid_len = 0, i = 0, pid_pos = 0;
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC WRITE AUTH INFO!");

    /*
      {
      "auzkey":"xxxx",    //"6":"32",         7   +  6+4
      "uuid":"xxxx",      //"4":"16",         7   +6+32+6   +    4+4
      "mac":"xxxxxx",     //"3":"12",
      "prod_test":"xxxx"    //"9":"4/5"
      "pid":"abcdefgh"    //if any
      }
      */

    if (len < 100) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO error ,since Invalid length!");
        return;
    }

#if ( TUYA_BLE_PROD_SUPPORT_OEM_TYPE == TUYA_BLE_PROD_OEM_TYPE_0_5 )
    if ((memcmp(&para[2], "auzkey", 6) != 0) || (memcmp(&para[46], "uuid", 4) != 0) || (memcmp(&para[72], "mac", 3) != 0) || ((memcmp(&para[110], "pid\":\"", 6) != 0) && (memcmp(&para[111], "pid\":\"", 6) != 0))) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO error ,since Invalid paras");
        return;
    }

    i = para[115] == '\"' ? 116 : 117;
    pid_pos = i;
    pid_len = 0;
    while ((para[i] != '\"') && (pid_len <= 20)) {
        i++;
        pid_len++;
    }
    if (pid_len > 20) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO error ,because pid len > 20 .");
        return;
    }

    memcpy(mac_char, &para[78], 12);
    tuya_ble_prod_asciitohex(mac_char, 12, mac_temp);

    if (tuya_ble_storage_write_auth_key_device_id_mac(&para[11], AUTH_KEY_LEN, &para[53], DEVICE_ID_LEN, mac_temp, MAC_LEN, mac_char, MAC_LEN * 2, &para[pid_pos], pid_len) == TUYA_BLE_SUCCESS) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO, (uint8_t *)true_buf, strlen(true_buf));
        TUYA_BLE_LOG_DEBUG("AUC WRITE AUTH INFO successed!");
    } else {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO failed!");
    }
#else
    if ((memcmp(&para[2], "auzkey", 6) != 0) || (memcmp(&para[46], "uuid", 4) != 0) || (memcmp(&para[72], "mac", 3) != 0)) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO error ,since Invalid paras");
        return;
    }

    memcpy(mac_char, &para[78], 12);
    tuya_ble_prod_asciitohex(mac_char, 12, mac_temp);

    if (tuya_ble_storage_write_auth_key_device_id_mac(&para[11], AUTH_KEY_LEN, &para[53], DEVICE_ID_LEN, mac_temp, MAC_LEN, mac_char, MAC_LEN * 2, NULL, 0) == TUYA_BLE_SUCCESS) {
        tuya_ble_prod_updata_mac(mac_temp);

        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO, (uint8_t *)true_buf, strlen(true_buf));
        TUYA_BLE_LOG_DEBUG("AUC WRITE AUTH INFO successed!");
    } else {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC_CMD_WRITE_AUTH_INFO failed!");
    }
#endif
}


#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)
volatile uint32_t comm_cfg_total_len = 0;
volatile uint32_t comm_cfg_cur_rcv_len = 0;
uint8_t *p_comm_cfg = NULL;


#if ( TUYA_BLE_INCLUDE_CJSON_COMPONENTS != 0 )
static void tuya_ble_auc_write_comm_cfg(uint8_t *para, uint16_t len)
{
    char reply_buf[64] = {0};
    uint16_t reply_buf_len = 0;
    bool reply_ret = false;

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC WRITE COMM CFG INFO!");

    cJSON *root = NULL, *item = NULL;
    int type;
    char *key = NULL;
    char *value = NULL;
    uint32_t offset, crc32, calc_crc32;

    unsigned char pstr[len + 1];
    memset(pstr, 0x00, len + 1);
    memcpy(pstr, para, len);

    root = cJSON_Parse((char *)pstr);
    if (!root) {
        TUYA_BLE_LOG_ERROR("cJSON error : [%s]\n", cJSON_GetErrorPtr());
        goto EXIT_ERR;
    }

    // type
    item = cJSON_GetObjectItem(root, "type");
    if (NULL == item) {
        goto EXIT_ERR;
    }
    type = item->valueint;

    // key
    item = cJSON_GetObjectItem(root, "key");
    if (NULL == item) {
        goto EXIT_ERR;
    }
    key = item->valuestring;

    if ((memcmp(key, "deviceCertificate", strlen("deviceCertificate")) != 0) && \
        (memcmp(key, "privateKey", strlen("privateKey")) != 0)) {
        TUYA_BLE_LOG_ERROR("WRITE COMM CFG, but key unknow.");
        goto EXIT_ERR;
    }

    /* process different types, 1->size 2->value 3->crc32 */
    if (type == 1) {
        // size
        item = cJSON_GetObjectItem(root, "size");
        if (NULL == item) {
            goto EXIT_ERR;
        }

        comm_cfg_total_len = item->valueint;
        comm_cfg_cur_rcv_len = 0;

        p_comm_cfg = (uint8_t *)tuya_ble_malloc(comm_cfg_total_len + 16);
        if (p_comm_cfg == NULL) {
            TUYA_BLE_LOG_ERROR("WRITE COMM CFG, malloc failed.");
        } else {
            reply_ret = true;
        }
    } else if (type == 2) {
        // value + offset
        item = cJSON_GetObjectItem(root, "value");
        if (NULL == item) {
            goto EXIT_ERR;
        }
        value = item->valuestring;

        item = cJSON_GetObjectItem(root, "offset");
        if (NULL == item) {
            goto EXIT_ERR;
        }
        offset = item->valueint;

        memcpy(&p_comm_cfg[offset], value, strlen(value));
        TUYA_BLE_LOG_DEBUG("WRITE COMM CFG offset=[%d] value_size=[%d]", offset, strlen(value));
        TUYA_BLE_LOG_HEXDUMP_DEBUG("value", (uint8_t *)value, strlen(value));

        comm_cfg_cur_rcv_len += strlen(value);
        reply_ret = true;
    } else if (type == 3) {
        // crc32
        if (comm_cfg_cur_rcv_len != comm_cfg_total_len) {
            TUYA_BLE_LOG_DEBUG("WRITE COMM CFG size err, recv=[%d] total=[%d]", comm_cfg_cur_rcv_len, comm_cfg_total_len);
            goto EXIT_ERR;
        }

        item = cJSON_GetObjectItem(root, "crc32");
        if (NULL == item) {
            goto EXIT_ERR;
        }
        crc32 = (uint32_t)item->valueint;

        calc_crc32 = tuya_ble_crc32_compute(p_comm_cfg, comm_cfg_total_len, NULL);
        if (crc32 != calc_crc32) {
            TUYA_BLE_LOG_DEBUG("WRITE COMM CFG crc32 err, calc=[%d] crc32=[%d]", calc_crc32, crc32);
        } else {
            TUYA_BLE_LOG_DEBUG("---------WRITE COMM CFG write context success---------");
            TUYA_BLE_LOG_DEBUG("context size  = [%d]", comm_cfg_total_len);
            TUYA_BLE_LOG_DEBUG("context crc32 = [%d]", crc32);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("context ", p_comm_cfg, comm_cfg_total_len);
            reply_ret = true;

            /* write to security chip */
            if (memcmp(key, "deviceCertificate", strlen("deviceCertificate")) == 0) {
                if (tuya_ble_storage_private_data(PRIVATE_DATA_DEV_CERT, p_comm_cfg, comm_cfg_total_len) != TUYA_BLE_SUCCESS) {
                    reply_ret = false;
                }
            } else if (memcmp(key, "privateKey", strlen("privateKey")) == 0) {
                uint8_t private_key[64] = {0};
                uint16_t private_key_len;

                if (tuya_ble_ecc_key_pem2hex((char *)p_comm_cfg, private_key, &private_key_len) == 0) {
                    TUYA_BLE_LOG_DEBUG("tuya ble ecc key pem2hex failed");
                    reply_ret = false;
                } else {
                    TUYA_BLE_LOG_HEXDUMP_DEBUG("private key raw", private_key, private_key_len);

                    if (tuya_ble_storage_private_data(PRIVATE_DATA_ECC_KEY, private_key, private_key_len) != TUYA_BLE_SUCCESS) {
                        reply_ret = false;
                    }
                }
            }

            if (p_comm_cfg != NULL) {
                tuya_ble_free(p_comm_cfg);
            }
        }
    } else {
        TUYA_BLE_LOG_ERROR("WRITE COMM CFG type err. [%d]", type);
    }

EXIT_ERR:
    if (reply_ret) {
        reply_buf_len = sprintf((char *)reply_buf, "{\"ret\":true,\"key\":\"%s\"}", key);
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_COMM_CFG, (uint8_t *)reply_buf, reply_buf_len);

        TUYA_BLE_LOG_DEBUG("AUC WRITE COMM CFG responsed successed.");
    } else {
        reply_buf_len = sprintf((char *)reply_buf, "{\"ret\":false,\"key\":\"%s\"}", key);
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_COMM_CFG, (uint8_t *)reply_buf, reply_buf_len);

        TUYA_BLE_LOG_DEBUG("AUC WRITE COMM CFG failed!");

        if (p_comm_cfg != NULL) {
            tuya_ble_free(p_comm_cfg);
        }
    }

    if (NULL != root) {
        cJSON_Delete(root);
    }
}
#else
static void tuya_ble_auc_write_comm_cfg(uint8_t *para, uint16_t len)
{
    char reply_buf[64] = {0};
    uint16_t reply_buf_len = 0;
    bool reply_ret = false;

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC WRITE COMM CFG INFO!");

    int i;
    int type, size, offset, crc32;
    char key[64] = {0};
    uint8_t symbol_colon_index[32] = {0};
    uint8_t symbol_comma_index[32] = {0};
    uint8_t symbol_colon_cnt, symbol_comma_cnt;
    uint16_t start_pos, end_pos, cut_len;
    uint32_t calc_crc32;

    symbol_colon_cnt = tuya_ble_search_symbol_index((char *)para, len, ':', symbol_colon_index);
    symbol_comma_cnt = tuya_ble_search_symbol_index((char *)para, len, ',', symbol_comma_index);
    if (symbol_colon_cnt == 0 || symbol_comma_cnt == 0) {
        goto EXIT_ERR;
    }

    TUYA_BLE_LOG_DEBUG("symbol->: cnt=[%d]", symbol_colon_cnt);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("symbol : list index", symbol_colon_index, symbol_colon_cnt);

    TUYA_BLE_LOG_DEBUG("symbol->, cnt=[%d]", symbol_comma_cnt);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("symbol , list index", symbol_comma_index, symbol_comma_cnt);

    // type
    start_pos = symbol_colon_index[0] + 1;
    type = para[start_pos] - '0';
    TUYA_BLE_LOG_DEBUG("parse type ->[%d]", type);

    // key
    start_pos = symbol_colon_index[1] + 2;
    end_pos = symbol_comma_index[1] - 2;
    cut_len = (end_pos - start_pos + 1);
    memcpy(key, &para[start_pos], cut_len);
    key[cut_len] = '\0';
    TUYA_BLE_LOG_DEBUG("parse key ->[%s]", key);

    if ((memcmp(key, "deviceCertificate", strlen("deviceCertificate")) != 0) && \
        (memcmp(key, "privateKey", strlen("privateKey")) != 0)) {
        TUYA_BLE_LOG_ERROR("WRITE COMM CFG, but key unknow.");
        goto EXIT_ERR;
    }

    /* process different types, 1->size 2->value 3->crc32 */
    if (type == 1) {
        /*
        {
            "type":1,
            "key":"xxxx",
            "size":xxx
        }
        */

        // size
        start_pos = symbol_colon_index[2] + 1;
        end_pos = len - 2;
        cut_len = (end_pos - start_pos + 1);
        size = tuya_ble_ascii_to_int((char *)&para[start_pos], cut_len);
        TUYA_BLE_LOG_DEBUG("parse size ->[%d]", size);

        comm_cfg_total_len = size;
        comm_cfg_cur_rcv_len = 0;

        p_comm_cfg = (uint8_t *)tuya_ble_malloc(comm_cfg_total_len + 16);
        if (p_comm_cfg == NULL) {
            TUYA_BLE_LOG_ERROR("WRITE COMM CFG, malloc failed.");
        } else {
            reply_ret = true;
        }
    } else if (type == 2) {
        /*
        {
        "type":2,
        "key":"xxxx",
        "value":"xxxx",
        "offset":xxxx
        }
        */

        // offset
        start_pos = symbol_colon_index[3] + 1;
        end_pos = len - 2;
        cut_len = (end_pos - start_pos + 1);
        offset = tuya_ble_ascii_to_int((char *)&para[start_pos], cut_len);
        TUYA_BLE_LOG_DEBUG("parse offset ->[%d]", offset);

        // value
        start_pos = symbol_colon_index[2] + 2;
        end_pos = symbol_comma_index[2] - 2;
        cut_len = (end_pos - start_pos + 1);

        // replace tab character "\r"->'\r'   "\n"->'\n'
        uint8_t *p_value = NULL;
        uint16_t buf_i = 0;
        p_value = (uint8_t *)tuya_ble_malloc(cut_len);

        for (i = start_pos; i <= end_pos; i++) {
            if ((para[i] == '\\') && (para[i + 1] == 'r')) {
                p_value[buf_i++] = '\r';

                i += 1;
                cut_len -= 1;
                TUYA_BLE_LOG_DEBUG("find r ");
            } else if ((para[i] == '\\') && (para[i + 1] == 'n')) {
                p_value[buf_i++] = '\n';

                i += 1;
                cut_len -= 1;
                TUYA_BLE_LOG_DEBUG("find n ");
            } else {
                p_value[buf_i++] = para[i];
            }
        }

        memcpy(&p_comm_cfg[offset], p_value, cut_len);
        tuya_ble_free(p_value);
        TUYA_BLE_LOG_DEBUG("WRITE COMM CFG offset=[%d] value_size=[%d]", offset, cut_len);

        comm_cfg_cur_rcv_len += cut_len;
        reply_ret = true;
    } else if (type == 3) {
        /*
        {
            "type":3,
            "key":"xxxx",
            "crc32":xxxx
        }
        */

        if (comm_cfg_cur_rcv_len != comm_cfg_total_len) {
            TUYA_BLE_LOG_DEBUG("WRITE COMM CFG size err, recv=[%d] total=[%d]", comm_cfg_cur_rcv_len, comm_cfg_total_len);
            goto EXIT_ERR;
        }

        // crc32
        start_pos = symbol_colon_index[2] + 1;
        end_pos = len - 2;
        cut_len = (end_pos - start_pos + 1);
        crc32 = tuya_ble_ascii_to_int((char *)&para[start_pos], cut_len);
        TUYA_BLE_LOG_DEBUG("parse crc32 ->[%d]", crc32);

        calc_crc32 = tuya_ble_crc32_compute(p_comm_cfg, comm_cfg_total_len, NULL);
        if (crc32 != calc_crc32) {
            TUYA_BLE_LOG_DEBUG("WRITE COMM CFG crc32 err, calc=[%d] crc32=[%d]", calc_crc32, crc32);
        } else {
            TUYA_BLE_LOG_DEBUG("---------WRITE COMM CFG write context success---------");
            TUYA_BLE_LOG_DEBUG("context size  = [%d]", comm_cfg_total_len);
            TUYA_BLE_LOG_DEBUG("context crc32 = [%d]", crc32);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("context ", p_comm_cfg, comm_cfg_total_len);
            reply_ret = true;

            /* write to security chip */
            if (memcmp(key, "deviceCertificate", strlen("deviceCertificate")) == 0) {
                if (tuya_ble_storage_private_data(PRIVATE_DATA_DEV_CERT, p_comm_cfg, comm_cfg_total_len) != TUYA_BLE_SUCCESS) {
                    reply_ret = false;
                }
            } else if (memcmp(key, "privateKey", strlen("privateKey")) == 0) {
                uint8_t private_key[64] = {0};
                uint16_t private_key_len;

                if (tuya_ble_ecc_key_pem2hex((char *)p_comm_cfg, private_key, &private_key_len) == 0) {
                    TUYA_BLE_LOG_DEBUG("tuya ble ecc key pem2hex failed");
                    reply_ret = false;
                } else {
                    TUYA_BLE_LOG_HEXDUMP_DEBUG("private key raw", private_key, private_key_len);

                    if (tuya_ble_storage_private_data(PRIVATE_DATA_ECC_KEY, private_key, private_key_len) != TUYA_BLE_SUCCESS) {
                        reply_ret = false;
                    }
                }
            }

            if (p_comm_cfg != NULL) {
                tuya_ble_free(p_comm_cfg);
            }
        }
    } else {
        TUYA_BLE_LOG_ERROR("WRITE COMM CFG type err. [%d]", type);
    }

EXIT_ERR:
    if (reply_ret) {
        reply_buf_len = sprintf((char *)reply_buf, "{\"ret\":true,\"key\":\"%s\"}", key);
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_COMM_CFG, (uint8_t *)reply_buf, reply_buf_len);

        TUYA_BLE_LOG_DEBUG("AUC WRITE COMM CFG responsed successed.");
    } else {
        reply_buf_len = sprintf((char *)reply_buf, "{\"ret\":false,\"key\":\"%s\"}", key);
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_COMM_CFG, (uint8_t *)reply_buf, reply_buf_len);

        TUYA_BLE_LOG_DEBUG("AUC WRITE COMM CFG failed!");

        if (p_comm_cfg != NULL) {
            tuya_ble_free(p_comm_cfg);
        }
    }
}
#endif // ( TUYA_BLE_INCLUDE_CJSON_COMPONENTS != 0 )
#endif // ( TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION )


static void tuya_ble_auc_query_info(uint8_t *para, uint16_t len)
{

    uint8_t i = 0;
    uint8_t mac_temp[13];
    uint8_t *alloc_buf = NULL;

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC QUERY INFO!");

    alloc_buf = (uint8_t *)tuya_ble_malloc(256);

    if (alloc_buf) {
        memset(alloc_buf, 0, 256);
    } else {
        TUYA_BLE_LOG_ERROR("AUC QUERY INFO alloc buf malloc failed.");
        return;
    }

    alloc_buf[i++] = '{';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], "ret", 3);
    i += 3;
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ':';
    memcpy(&alloc_buf[i], "true", 4);
    i += 4;

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], "auzKey", 6);
    i += 6;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);
    i += AUTH_KEY_LEN;

    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], "hid", 3);
    i += 3;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], tuya_ble_current_para.auth_settings.h_id, H_ID_LEN);
    i += 19;
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], "uuid", 4);
    i += 4;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN);
    i += DEVICE_ID_LEN;
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], "mac", 3);
    i += 3;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';

    memcpy(&alloc_buf[i], tuya_ble_current_para.auth_settings.mac_string, MAC_LEN * 2);
    i += MAC_LEN * 2;
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], "firmName", 8);
    i += 8;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], TUYA_BLE_APP_BUILD_FIRMNAME_STRING, strlen(TUYA_BLE_APP_BUILD_FIRMNAME_STRING));
    i += strlen(TUYA_BLE_APP_BUILD_FIRMNAME_STRING);
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], "firmVer", 7);
    i += 7;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], TUYA_BLE_APP_VERSION_STRING, strlen(TUYA_BLE_APP_VERSION_STRING));
    i += strlen(TUYA_BLE_APP_VERSION_STRING);
    alloc_buf[i++] = '\"';

    alloc_buf[i++] = ',';
    alloc_buf[i++] = '\"';
    memcpy(&alloc_buf[i], "prod_test", 9);
    i += 9;
    alloc_buf[i++] = '\"';
    alloc_buf[i++] = ':';

    memcpy(&alloc_buf[i], "false", 5);
    i += 5;

    alloc_buf[i++] = '}';

    alloc_buf[i++] = 0;

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_QUERY_INFO, (uint8_t *)alloc_buf, i - 1);

    TUYA_BLE_LOG_DEBUG("AUC_CMD_QUERY_INFO RESPONSE!");

    tuya_ble_free(alloc_buf);
}


static void tuya_ble_auc_reset(uint8_t *para, uint16_t len)
{
    uint8_t buf[1] = {0x00};

    if (tuya_ble_production_test_flag != 1) {
        return;
    }
    TUYA_BLE_LOG_DEBUG("auc RESET!");

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_RESET, buf, sizeof(buf));

    tuya_ble_device_delay_ms(1000);

    tuya_ble_device_reset();

}


static  void tuya_ble_auc_write_hid(uint8_t *para, uint16_t len)
{
    uint8_t hid[19];
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC WRITE AUTH HID!");

    if (len < 27) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("WRITE AUTH HID para length error!");
        return;
    }

    if (memcmp(&para[2], "hid", 3) != 0) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("WRITE AUTH HID para error!");
        return;
    }

    memcpy(hid, &para[8], H_ID_LEN);

    if (tuya_ble_storage_write_hid(hid, H_ID_LEN) == TUYA_BLE_SUCCESS) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID, (uint8_t *)true_buf, strlen(true_buf));
        TUYA_BLE_LOG_DEBUG("WRITE AUTH HID successed.");
    } else {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_HID, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("WRITE AUTH HID failed.");
    }



}


static void tuya_ble_auc_query_fingerprint(uint8_t *para, uint16_t len)
{
    int32_t length = 0;
    uint8_t *alloc_buf = NULL;

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC QUERY FINGERPRINT!");

    alloc_buf = (uint8_t *)tuya_ble_malloc(256);

    if (alloc_buf) {
        memset(alloc_buf, 0, 256);
    } else {
        TUYA_BLE_LOG_ERROR("AUC QUERY INFO alloc buf malloc failed.");
        return;
    }

    length = sprintf((char *)alloc_buf, "{\"ret\":true,\"firmName\":\"%s\",\"firmVer\":\"%s\"}", TUYA_BLE_APP_BUILD_FIRMNAME_STRING, TUYA_BLE_APP_VERSION_STRING);

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_QUERY_FINGERPRINT, alloc_buf, length);

    tuya_ble_free(alloc_buf);

    TUYA_BLE_LOG_DEBUG("AUC_CMD_QUERY_FINGERPRINT responsed.");

}




static void tuya_ble_auc_rssi_test(uint8_t *para, uint16_t len)
{
    uint8_t length = 0;
    int8_t rssi = 0;
    static const char false_buf[] = "{\"ret\":false}";
    char true_buf[30];

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC RSSI TEST!");

    memset(true_buf, 0, sizeof(true_buf));

    tuya_ble_prod_beacon_scan_stop();

    if (tuya_ble_prod_beacon_get_rssi_avg(&rssi) != TUYA_BLE_SUCCESS) {
        TUYA_BLE_LOG_ERROR("auc get rssi failed.");
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_RSSI_TEST, (uint8_t *)false_buf, strlen(false_buf));
    } else {
        length = sprintf((char *)true_buf, "{\"ret\":true,\"rssi\":\"%d\"}", rssi);
        TUYA_BLE_LOG_DEBUG("auc get rssi = %d", rssi);
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_RSSI_TEST, (uint8_t *)true_buf, length);
    }
}


static void tuya_ble_auc_read_mac(uint8_t *para, uint16_t len)
{
    (void)(para);
    (void)(len);

    int32_t length = 0;
    uint8_t buf[64];
    uint8_t mac_addr_str[12 + 1];

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC READ MAC ADDR!");

    // MAC string format: DC 23 xx xx xx xx
    memcpy(mac_addr_str, tuya_ble_current_para.auth_settings.mac_string, MAC_LEN * 2);
    mac_addr_str[12] = '\0';

    int swap_len = MAC_LEN * 2;
    char temp[2];

    for (int i = 0; i < swap_len / 2; i += 2) {
        temp[0] = mac_addr_str[i];
        temp[1] = mac_addr_str[i + 1];

        mac_addr_str[i] = mac_addr_str[swap_len - 2 - i];
        mac_addr_str[i + 1] = mac_addr_str[swap_len - 1 - i];

        mac_addr_str[swap_len - 2 - i] = temp[0];
        mac_addr_str[swap_len - 1 - i] = temp[1];
    }

    length = sprintf((void *)buf, "{\"ret\":true,\"mac\":\"%s\"}", mac_addr_str);

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_READ_MAC, buf, length);

    TUYA_BLE_LOG_DEBUG("AUC_CMD_READ_MAC responsed.");
}


static void tuya_ble_auc_exit(uint8_t *para, uint16_t len)
{
    (void)(para);
    (void)(len);

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC EXIT!");

    char buf[] = "{\"ret\":true}";
    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_EXIT, (uint8_t *)buf, strlen(buf));

    tuya_ble_device_delay_ms(1000);
    tuya_ble_device_reset();
}


#if ( TUYA_BLE_PROD_SUPPORT_OEM_TYPE == TUYA_BLE_PROD_OEM_TYPE_0_5 )

typedef struct {
    bool need_subpkg;		/**< whether need subpkg received. */

    uint8_t file_type;		/**< file type. 0x09-OEM file 0x10-homekit atoken. */
    uint32_t total_nums;	/**< subpkg total numbers. */
    uint32_t file_crc32;	/**< file crc32 value. */
    uint32_t rcv_file_len; 	/**< currently received file length. */
    uint8_t *p_file_data;	/**< the point of received file data. */
} write_subpkg_info_t;

static write_subpkg_info_t write_subpkg_info = {
    .need_subpkg = false,

    .file_type = 0,
    .total_nums = 0,
    .file_crc32 = 0,
    .rcv_file_len = 0,
    .p_file_data = NULL,
};


__TUYA_BLE_WEAK tuya_ble_status_t tuya_ble_prod_storage_oem_info(uint8_t *para, uint16_t len)
{
    return TUYA_BLE_SUCCESS;
}


static void tuya_ble_auc_write_oem_info(uint8_t *para, uint16_t len)
{
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";

    if (tuya_ble_production_test_flag != 1) {
        return;
    }

    TUYA_BLE_LOG_DEBUG("AUC WRITE OEM INFO!");

    if (len == 0) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_OEM_INFO, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("WRITE OEM INFO para length error!");
        return;
    }

    if ((write_subpkg_info.need_subpkg == false) && (write_subpkg_info.total_nums == 0)) { // need not subpkg
        if (para[0] != '{' || para[len - 1] != '}') {
            tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_OEM_INFO, (uint8_t *)false_buf, strlen(false_buf));
            TUYA_BLE_LOG_ERROR("WRITE OEM INFO failed.");
            return;
        }

        if (tuya_ble_prod_storage_oem_info(para, len) == TUYA_BLE_SUCCESS) {
            tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_OEM_INFO, (uint8_t *)true_buf, strlen(true_buf));
            TUYA_BLE_LOG_DEBUG("WRITE OEM INFO successed.");
        } else {
            tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_OEM_INFO, (uint8_t *)false_buf, strlen(false_buf));
            TUYA_BLE_LOG_ERROR("WRITE OEM INFO failed.");
        }
    } else {
        uint16_t cur_subpkg = (para[0] << 8) + para[1];
        if (cur_subpkg < write_subpkg_info.total_nums) {
            memcpy(write_subpkg_info.p_file_data + write_subpkg_info.rcv_file_len, para + 2, len - 2);
            write_subpkg_info.rcv_file_len += len - 2;

            tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_OEM_INFO, (uint8_t *)true_buf, strlen(true_buf));
            TUYA_BLE_LOG_DEBUG("WRITE OEM INFO subpkg successed.");
        } else {
            tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_OEM_INFO, (uint8_t *)false_buf, strlen(false_buf));
            TUYA_BLE_LOG_ERROR("WRITE OEM INFO subpkg failed.");
        }
    }
}


static void tuya_ble_auc_write_subpkg_start(uint8_t *para, uint16_t len)
{
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";

    uint8_t cmd;
    int packageNum, crc32;
    uint8_t symbol_colon_index[8] = {0};
    uint8_t symbol_comma_index[8] = {0};
    uint8_t symbol_colon_cnt, symbol_comma_cnt;
    uint16_t start_pos, end_pos, cut_len;

    symbol_colon_cnt = tuya_ble_search_symbol_index((char *)para, len, ':', symbol_colon_index);
    symbol_comma_cnt = tuya_ble_search_symbol_index((char *)para, len, ',', symbol_comma_index);

    /* check json filed */
    if ((symbol_colon_cnt == 0 || symbol_comma_cnt == 0) || \
        (memcmp(&para[2], "cmd", strlen("cmd")) != 0) || \
        (memcmp(&para[symbol_comma_index[0] + 2], "packageNum", strlen("packageNum")) != 0) || \
        (memcmp(&para[symbol_comma_index[1] + 2], "crc32", strlen("crc32")) != 0)) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_SUBPKG_START, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC WRITE SUBPKG START format err failed.");
        return;
    }

    /* cmd */
    start_pos = symbol_colon_index[0] + 2;
    end_pos = symbol_comma_index[0] - 2;
    cut_len = (end_pos - start_pos + 1);
    tuya_ble_hexstr2hex((uint8_t *)&para[start_pos], cut_len, &cmd);
    TUYA_BLE_LOG_DEBUG("parse cmd ->[0x%02x]", cmd);

    /* packageNum */
    start_pos = symbol_colon_index[1] + 1;
    end_pos = symbol_comma_index[1] - 1;
    cut_len = (end_pos - start_pos + 1);
    packageNum = tuya_ble_ascii_to_int((char *)&para[start_pos], cut_len);
    TUYA_BLE_LOG_DEBUG("parse packageNum ->[%d]", packageNum);

    /* crc32 */
    start_pos = symbol_colon_index[2] + 1;
    end_pos = len - 2;
    cut_len = (end_pos - start_pos + 1);
    crc32 = tuya_ble_ascii_to_int((char *)&para[start_pos], cut_len);
    TUYA_BLE_LOG_DEBUG("parse crc32 ->[%d]", crc32);

    write_subpkg_info.need_subpkg = true;
    write_subpkg_info.file_type   = cmd; //!< "09"-oem ocnfig data "10"-homekit atoken data. format: hexstring
    write_subpkg_info.total_nums  = packageNum;
    write_subpkg_info.file_crc32  = crc32;

    write_subpkg_info.p_file_data = (uint8_t *)tuya_ble_malloc(240 * write_subpkg_info.total_nums);
    if (write_subpkg_info.p_file_data == NULL) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_SUBPKG_START, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_ERROR("AUC WRITE SUBPKG START alloc buf malloc failed.");
        return;
    }

    tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_SUBPKG_START, (uint8_t *)true_buf, strlen(true_buf));
    TUYA_BLE_LOG_DEBUG("AUC WRITE SUBPKG START successed.");
}


static void tuya_ble_auc_write_subpkg_end(uint8_t *para, uint16_t len)
{
    char true_buf[] = "{\"ret\":true}";
    char false_buf[] = "{\"ret\":false}";
    uint32_t calc_crc32;

    /* check received file whether correct */
    calc_crc32 = tuya_ble_crc32_compute(write_subpkg_info.p_file_data, write_subpkg_info.rcv_file_len, NULL);
    if (calc_crc32 != write_subpkg_info.file_crc32) {
        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_SUBPKG_END, (uint8_t *)false_buf, strlen(false_buf));
        TUYA_BLE_LOG_DEBUG("AUC WRITE SUBPKG END crc32 check failed.");
    } else {
        if (write_subpkg_info.file_type == 0x09) {
            /* oem config data */
            tuya_ble_prod_storage_oem_info(write_subpkg_info.p_file_data, write_subpkg_info.rcv_file_len);
        } else if (write_subpkg_info.file_type == 0x10) {
            /* homekit atoken data */
            // TODO ..
        }

        tuya_ble_uart_prod_send(TUYA_BLE_AUC_CMD_WRITE_SUBPKG_END, (uint8_t *)true_buf, strlen(true_buf));
        TUYA_BLE_LOG_DEBUG("AUC WRITE SUBPKG END successed.");
    }

    /* Clear progress vars, and free p_file_data*/
    write_subpkg_info.need_subpkg = false;
    write_subpkg_info.file_crc32 = 0;
    write_subpkg_info.total_nums = 0;
    write_subpkg_info.file_type = 0;
    write_subpkg_info.rcv_file_len = 0;

    tuya_ble_free(write_subpkg_info.p_file_data);
}
#endif



__TUYA_BLE_WEAK void tuya_ble_custom_app_production_test_process(uint8_t channel, uint8_t *p_in_data, uint16_t in_len)
{
    uint16_t sub_cmd = 0;
    uint8_t *data_buffer = NULL;
    uint16_t data_len = ((p_in_data[4] << 8) + p_in_data[5]);

    if ((p_in_data[6] != 3) || (data_len < 3)) {
        return;
    }

    sub_cmd = (p_in_data[7] << 8) + p_in_data[8];
    data_len -= 3;
    if (data_len > 0) {
        data_buffer = p_in_data + 9;
    }

    switch (sub_cmd) {


    default:
        break;
    };


}


__TUYA_BLE_WEAK void tuya_ble_app_sdk_test_process(uint8_t channel, uint8_t *p_in_data, uint16_t in_len)
{
    uint16_t sub_cmd = 0;
    uint8_t *data_buffer = NULL;
    uint16_t data_len = ((p_in_data[4] << 8) + p_in_data[5]);

    if ((p_in_data[6] != 3) || (data_len < 3)) {
        return;
    }

    sub_cmd = (p_in_data[7] << 8) + p_in_data[8];
    data_len -= 3;
    if (data_len > 0) {
        data_buffer = p_in_data + 9;
    }

    switch (sub_cmd) {


    default:
        break;
    };


}



extern void tuya_ble_connect_monitor_timer_stop(void);
void tuya_ble_app_production_test_process(uint8_t channel, uint8_t *p_in_data, uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4] << 8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data + 6;
    /*
     if(tuya_ble_current_para.sys_settings.factory_test_flag==0) //
     {
         TUYA_BLE_LOG_WARNING("The production interface is closed!");
         return;
     }
     */
    if ((channel != 0) && (cmd != TUYA_BLE_AUC_CMD_EXTEND)) {
        TUYA_BLE_LOG_ERROR("The authorization instructions are not supported in non-serial channels!");
        return;
    }
    if ((channel == 1) && (cmd == TUYA_BLE_AUC_CMD_EXTEND)) {
        if (tuya_ble_production_test_with_ble_flag == 0) {
            tuya_ble_production_test_with_ble_flag = 1;
            if (tuya_ble_connect_status_get() != BONDING_CONN) {
                tuya_ble_connect_monitor_timer_stop();
            }
        }

    }
    switch (cmd) {
    case TUYA_BLE_AUC_CMD_EXTEND:
        tuya_ble_custom_app_production_test_process(channel, p_in_data, in_len);
        break;
    case TUYA_BLE_SDK_TEST_CMD_EXTEND:
        tuya_ble_app_sdk_test_process(channel, p_in_data, in_len);
        break;
    case TUYA_BLE_AUC_CMD_ENTER:
        tuya_ble_auc_enter(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_QUERY_HID:
        tuya_ble_auc_query_hid(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_GPIO_TEST:
        tuya_ble_auc_gpio_test(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_WRITE_AUTH_INFO:
        tuya_ble_auc_write_auth_info(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_QUERY_INFO:
        tuya_ble_auc_query_info(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_RESET:
        tuya_ble_auc_reset(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_QUERY_FINGERPRINT:
        tuya_ble_auc_query_fingerprint(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_WRITE_HID:
        tuya_ble_auc_write_hid(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_RSSI_TEST:
        tuya_ble_auc_rssi_test(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_READ_MAC:
        tuya_ble_auc_read_mac(data_buffer, data_len);
        break;

    case TUYA_BLE_AUC_CMD_EXIT:
        tuya_ble_auc_exit(data_buffer, data_len);
        break;

#if ( TUYA_BLE_PROD_SUPPORT_OEM_TYPE == TUYA_BLE_PROD_OEM_TYPE_0_5 )
    case TUYA_BLE_AUC_CMD_WRITE_OEM_INFO:
        tuya_ble_auc_write_oem_info(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_WRITE_SUBPKG_START:
        tuya_ble_auc_write_subpkg_start(data_buffer, data_len);
        break;
    case TUYA_BLE_AUC_CMD_WRITE_SUBPKG_END:
        tuya_ble_auc_write_subpkg_end(data_buffer, data_len);
        break;
#endif

#if (TUYA_BLE_SECURE_CONNECTION_TYPE == TUYA_BLE_SECURE_CONNECTION_WITH_AUTH_KEY_ADVANCED_ENCRYPTION)
    case TUYA_BLE_AUC_CMD_WRITE_COMM_CFG:
        tuya_ble_auc_write_comm_cfg(data_buffer, data_len);
        break;
#endif

    default:
        break;
    };


}

#else

void tuya_ble_app_production_test_process(uint8_t channel, uint8_t *p_in_data, uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4] << 8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data + 6;
    switch (cmd) {
    default:
        break;
    };


}

#endif



