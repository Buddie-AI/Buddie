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

#include "ble_qiot_config.h"

#if BLE_QIOT_LLSYNC_STANDARD
//#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ble_qiot_export.h"
#include "ble_qiot_import.h"
#include "ble_qiot_common.h"
#include "ble_qiot_llsync_data.h"
#include "ble_qiot_llsync_event.h"
#include "ble_qiot_utils_base64.h"
#include "ble_qiot_crc.h"
#include "ble_qiot_log.h"
#include "ble_qiot_param_check.h"
#include "ble_qiot_service.h"
#include "ble_qiot_template.h"
#include "ble_qiot_llsync_ota.h"
#include "dual_bank_updata_api.h"
#include "timer.h"
#include "os/os_api.h"
#include "update_loader_download.h"
#include "app_config.h"

#if BLE_QIOT_SUPPORT_OTA
static ble_ota_user_callback sg_ota_user_cb;  // user callback
// 1. monitor the data and request from the server if data lost; 2. call the user function if no data for a long time
static int sg_ota_timer                           = 0;
static uint8_t     sg_ota_timeout_cnt                     = 0;    // count the number of no data times
static uint8_t     sg_ota_data_buf[BLE_QIOT_OTA_BUF_SIZE] = {0};  // storage ota data and write to the flash at once
static uint16_t    sg_ota_data_buf_size                   = 0;    // the data size in the buffer
static uint32_t    sg_ota_download_file_size              = 0;    // the data size download from the server
static uint8_t     sg_ota_next_seq                        = 0;    // the next expect seq
static uint32_t    sg_ota_download_address                = 0;    // the address saved ota file
static uint8_t     sg_ota_download_percent                = 0;    // the percent of file had download
static uint8_t     sg_ota_flag                            = 0;    // ota control info
static ble_ota_info_record sg_ota_info;                           // the ota info storage in flash if support resuming
static ble_ota_reply_t     sg_ota_reply_info;                     // record the last reply info

static OS_SEM      sg_ota_sem;

static update_op_tws_api_t *sg_ota_tws_same_api = NULL;
static u8 pkt_flag = PKT_FLAG_FIRST;

#define BLE_QIOT_OTA_FLAG_SET(_BIT)    (sg_ota_flag |= (_BIT));
#define BLE_QIOT_OTA_FLAG_CLR(_BIT)    (sg_ota_flag &= ~(_BIT));
#define BLE_QIOT_OTA_FLAG_IS_SET(_BIT) ((sg_ota_flag & (_BIT)) == (_BIT))

uint32_t ble_ota_get_download_addr(void)
{
    return 0;
}

static inline bool ble_qiot_ota_info_valid(void)
{
    return sg_ota_info.valid_flag == BLE_QIOT_OTA_PAGE_VALID_VAL;
}
static inline uint8_t ble_ota_next_seq_get(void)
{
    return sg_ota_next_seq;
}
static inline void ble_ota_next_seq_inc(void)
{
    sg_ota_next_seq++;
}
static inline uint32_t ble_ota_download_address_get(void)
{
    return sg_ota_download_address;
}
static inline void ble_ota_download_address_set(void)
{
    sg_ota_download_address = ble_ota_get_download_addr();
    return;
}
static inline uint32_t ble_ota_download_size_get(void)
{
    return sg_ota_download_file_size;
}
static inline void ble_ota_download_size_inc(uint32_t size)
{
    sg_ota_download_file_size += size;
}
static inline uint8_t ble_ota_file_percent_get(void)
{
    return sg_ota_download_percent;
}
static inline void ble_ota_file_percent_set(uint8_t percent)
{
    sg_ota_download_percent = percent;
}

int ble_ota_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len)
{
    return sg_ota_data_buf_size;
}

void ble_ota_callback_reg(ble_ota_start_callback start_cb, ble_ota_stop_callback stop_cb,
                          ble_ota_valid_file_callback valid_file_cb)
{
    sg_ota_user_cb.start_cb      = start_cb;
    sg_ota_user_cb.stop_cb       = stop_cb;
    sg_ota_user_cb.valid_file_cb = valid_file_cb;
}
static inline void ble_ota_user_start_cb(void)
{
    if (NULL != sg_ota_user_cb.start_cb) {
        ble_qiot_log_i("start callback begin");
        sg_ota_user_cb.start_cb();
        ble_qiot_log_i("start callback end");
    }
}
static inline void ble_ota_user_stop_cb(uint8_t result)
{
    if (NULL != sg_ota_user_cb.stop_cb) {
        ble_qiot_log_i("stop callback begin");
        sg_ota_user_cb.stop_cb(result);
        ble_qiot_log_i("stop callback end");
    }
}
static inline ble_qiot_ret_status_t ble_ota_user_valid_cb(void)
{
    if (NULL != sg_ota_user_cb.valid_file_cb) {
        ble_qiot_log_i("valid callback begin");
        return sg_ota_user_cb.valid_file_cb(sg_ota_info.download_file_info.file_size,
                                            (char *)sg_ota_info.download_file_info.file_version);
    }
    return BLE_QIOT_RS_OK;
}
static inline ble_qiot_ret_status_t ble_ota_write_info(void)
{
#if BLE_QIOT_SUPPORT_RESUMING
    sg_ota_info.valid_flag     = BLE_QIOT_OTA_PAGE_VALID_VAL;
    sg_ota_info.last_file_size = ble_ota_download_size_get();
    sg_ota_info.last_address   = ble_ota_download_address_get();
    if (sizeof(ble_ota_info_record) !=
        ble_write_flash(BLE_QIOT_OTA_INFO_FLASH_ADDR, (const char *)&sg_ota_info, sizeof(ble_ota_info_record))) {
        ble_qiot_log_e("write ota info failed");
        return BLE_QIOT_RS_ERR;
    }
#endif //BLE_QIOT_SUPPORT_RESUMING
    return BLE_QIOT_RS_OK;
}
static inline void ble_ota_clear_info(void)
{
#if BLE_QIOT_SUPPORT_RESUMING
    sg_ota_info.valid_flag = ~BLE_QIOT_OTA_PAGE_VALID_VAL;
    if (sizeof(ble_ota_info_record) !=
        ble_write_flash(BLE_QIOT_OTA_INFO_FLASH_ADDR, (const char *)&sg_ota_info, sizeof(ble_ota_info_record))) {
        ble_qiot_log_e("clear ota info failed");
    }
#endif //BLE_QIOT_SUPPORT_RESUMING
    return;
}
static inline ble_qiot_ret_status_t ble_ota_reply_ota_data(void)
{
    uint8_t  req       = ble_ota_next_seq_get();
    uint32_t file_size = ble_ota_download_size_get();

    printf("used old file size %x, req %d", file_size, req);
    if (sg_ota_reply_info.file_size != file_size) {
        sg_ota_reply_info.file_size = file_size;
        sg_ota_reply_info.req       = req;
    } else {
        // in the case that the device reply info missed, the timer reply info again but the seq increased, so we need
        // saved the last reply info and used in the time
        req = sg_ota_reply_info.req;
    }

    file_size = HTONL(file_size);
    return ble_event_notify(BLE_QIOT_EVENT_UP_REPLY_OTA_DATA, &req, sizeof(uint8_t), (const char *)&file_size,
                            sizeof(uint32_t));
}
static inline ble_qiot_ret_status_t ble_ota_report_check_result(uint8_t firmware_valid, uint8_t error_code)
{
    uint8_t result = firmware_valid | error_code;
    return ble_event_notify(BLE_QIOT_EVENT_UP_REPORT_CHECK_RESULT, NULL, 0, (const char *)&result, sizeof(uint8_t));
}
static inline void ble_ota_timer_delete(void)
{
    if (NULL == sg_ota_timer) {
        ble_qiot_log_e("ble ota timer invalid, delete failed");
        return;
    }

    sys_timer_del(sg_ota_timer);
    /* if (BLE_QIOT_RS_OK != ble_timer_delete(sg_ota_timer)) { */
    /*     ble_qiot_log_e("ble ota timer delete failed"); */
    /* } */
    sg_ota_timer = 0;
    return;
}

static int ble_ota_write_end_callback(void *priv)
{
    os_sem_post(&sg_ota_sem);
    return 0;
}

static ble_qiot_ret_status_t ble_ota_write_data_to_flash(void)
{
    int ret        = 0;
    int write_addr = 0;

    // the download size include the data size, so the write address exclude the data size
    //os_time_dly(10);
    /* g_printf("%s\n", __func__); */
    dual_bank_update_write((const char *)sg_ota_data_buf, sg_ota_data_buf_size, ble_ota_write_end_callback);
    os_sem_pend(&sg_ota_sem, 0);

#if OTA_TWS_SAME_TIME_ENABLE
    if (pkt_flag != PKT_FLAG_FIRST) {
        if (sg_ota_tws_same_api && sg_ota_tws_same_api->tws_ota_data_send_pend) {
            if (sg_ota_tws_same_api->tws_ota_data_send_pend()) {
                printf("pend timeout\n");
                return BLE_QIOT_RS_ERR;
            }
        }
    }
    if (sg_ota_tws_same_api && sg_ota_tws_same_api->tws_ota_data_send) {
        sg_ota_tws_same_api->tws_ota_data_send(sg_ota_data_buf, sg_ota_data_buf_size);
    }

    pkt_flag  = PKT_FLAG_MIDDLE;
#endif

#if 0
    write_addr = ble_ota_download_address_get() + ble_ota_download_size_get() - sg_ota_data_buf_size;
    ret        = ble_ota_write_flash(write_addr, (const char *)sg_ota_data_buf, sg_ota_data_buf_size);
    if (ret != sg_ota_data_buf_size) {
        ble_qiot_log_e("ota data write flash failed");
        return BLE_QIOT_RS_ERR;
    }
#endif
    return BLE_QIOT_RS_OK;
}
static void ble_ota_timer_callback(void *param)
{
    if (BLE_QIOT_OTA_FLAG_IS_SET(BLE_QIOT_OTA_RECV_DATA_BIT)) {
        BLE_QIOT_OTA_FLAG_CLR(BLE_QIOT_OTA_RECV_DATA_BIT);
        return;
    }
    sg_ota_timeout_cnt++;

    ble_qiot_log_w("reply in the timer, count: %d", sg_ota_timeout_cnt);
    ble_ota_reply_ota_data();

    if (sg_ota_timeout_cnt >= BLE_QIOT_OTA_MAX_RETRY_COUNT) {
        sg_ota_flag = 0;
        ble_ota_timer_delete();
        // inform the user ota failed because timeout
#if BLE_QIOT_SUPPORT_OTA
        ble_ota_stop();
#endif //BLE_QIOT_SUPPORT_OTA
        ble_ota_user_stop_cb(BLE_QIOT_OTA_ERR_TIMEOUT);
    }
}
static inline void ble_ota_timer_start(void)
{
    if (NULL == sg_ota_timer) {
        sg_ota_timer = sys_timer_add(NULL, ble_ota_timer_callback, BLE_QIOT_RETRY_TIMEOUT * 1000);
        /* sg_ota_timer = ble_timer_create(BLE_TIMER_PERIOD_TYPE, ble_ota_timer_callback); */
        /* if (NULL == sg_ota_timer) { */
        /*     ble_qiot_log_e("ble ota timer create failed"); */
        /*     return; */
        /* } */
    }
    /* if (BLE_QIOT_RS_OK != ble_timer_start(sg_ota_timer, BLE_QIOT_RETRY_TIMEOUT * 1000)) { */
    /*     ble_qiot_log_e("ble ota timer start failed"); */
    /* } */
    return;
}
static ble_qiot_ret_status_t ble_ota_init(void)
{
    uint32_t size_align   = 0;
    uint8_t  file_percent = 0;

    // init the ota env
    memset(sg_ota_data_buf, 0, sizeof(sg_ota_data_buf));
    sg_ota_data_buf_size = 0;
    sg_ota_timeout_cnt   = 0;
    memset(&sg_ota_info, 0, sizeof(ble_ota_info_record));
    sg_ota_download_file_size = 0;
    sg_ota_next_seq           = 0;
    sg_ota_download_percent   = 0;
    sg_ota_flag               = 0;
    ble_ota_download_address_set();
    memset(&sg_ota_reply_info, 0, sizeof(sg_ota_reply_info));

#if BLE_QIOT_SUPPORT_RESUMING
    // start from 0 if read flash fail, but ota will continue so ignored the return code
    ble_read_flash(BLE_QIOT_OTA_INFO_FLASH_ADDR, (char *)&sg_ota_info, sizeof(sg_ota_info));
    ble_qiot_log_hex(BLE_QIOT_LOG_LEVEL_INFO, "ota info", &sg_ota_info, sizeof(sg_ota_info));
    // check if the valid flag legalled
    if (ble_qiot_ota_info_valid()) {
        // the ota info write to flash may be mismatch the file write to flash, we should download file from the byte
        // align the flash page
        if (ble_ota_download_address_get() == sg_ota_info.last_address) {
            size_align = sg_ota_info.last_file_size / BLE_QIOT_RECORD_FLASH_PAGESIZE * BLE_QIOT_RECORD_FLASH_PAGESIZE;
            ble_ota_download_size_inc(size_align);
            file_percent = size_align * 100 / sg_ota_info.download_file_info.file_size;
            ble_ota_file_percent_set(file_percent);
            ble_qiot_log_i("align file size: %x, the percent: %d", size_align, file_percent);
        } else {
            memset(&sg_ota_info, 0, sizeof(ble_ota_info_record));
        }
    } else {
        memset(&sg_ota_info, 0, sizeof(ble_ota_info_record));
    }
#endif //BLE_QIOT_SUPPORT_RESUMING
    return BLE_QIOT_RS_OK;
}
// stop ota if ble disconnect
void ble_ota_stop(void)
{
    if (!BLE_QIOT_OTA_FLAG_IS_SET(BLE_QIOT_OTA_REQUEST_BIT)) {
        ble_qiot_log_w("ota not start");
        return;
    }
    sg_ota_flag = 0;
    set_ota_status(0);
    ble_ota_timer_delete();
    // write data to flash if the ota stop
    ble_ota_write_data_to_flash();
    ble_ota_write_info();
    dual_bank_passive_update_exit(NULL);
#if OTA_TWS_SAME_TIME_ENABLE
    if (sg_ota_tws_same_api && sg_ota_tws_same_api->tws_ota_err) {
        sg_ota_tws_same_api->tws_ota_err(0);
    }
#endif
    // inform user ota failed because ble disconnect
    ble_ota_user_stop_cb(BLE_QIOT_OTA_DISCONNECT);
}

ble_qiot_ret_status_t ble_ota_request_handle(const char *in_buf, int buf_len)
{
    BUFF_LEN_SANITY_CHECK(buf_len, sizeof(ble_ota_file_info) - BLE_QIOT_OTA_MAX_VERSION_STR, BLE_QIOT_RS_ERR_PARA);

    uint8_t            ret         = 0;
    uint8_t            reply_flag  = 0;
    uint32_t           file_size   = 0;
    uint32_t           file_crc    = 0;
    uint8_t            version_len = 0;
    char              *p           = (char *)in_buf;
    char              *notify_data = NULL;
    uint16_t           notify_len  = 0;
    ble_ota_reply_info ota_reply_info;

    ble_ota_init();

    memcpy(&file_size, p, sizeof(file_size));
    p += sizeof(file_size);
    memcpy(&file_crc, p, sizeof(file_crc));
    p += sizeof(file_crc);
    version_len = *p++;
    file_size   = NTOHL(file_size);
    file_crc    = NTOHL(file_crc);
    ble_qiot_log_i("request ota, size: %x, crc: %x, version: %s", file_size, file_crc, p);

    // check if the ota is allowed
    ret = ble_ota_is_enable((const char *)p, file_size, file_crc);
    if (ret == BLE_OTA_ENABLE) {
#if OTA_TWS_SAME_TIME_ENABLE
        sg_ota_tws_same_api = get_tws_update_api();
        if (sg_ota_tws_same_api == NULL) {
            ret = 0;
        } else {
            if (sg_ota_tws_same_api && sg_ota_tws_same_api->tws_ota_start) {
                struct __tws_ota_para tws_update_para;
                tws_update_para.fm_size = file_size;
                tws_update_para.fm_crc  = file_crc;
                tws_update_para.max_pkt_len = BLE_QIOT_OTA_BUF_SIZE;
                if (sg_ota_tws_same_api->tws_ota_start(&tws_update_para) != 0) {    //tws same time err
                    ret = 0;
                }
            }
        }
#endif
    }

    if (BLE_OTA_ENABLE == ret) {
        reply_flag                      = BLE_QIOT_OTA_ENABLE;
        ota_reply_info.package_nums     = BLE_QIOT_TOTAL_PACKAGES;
        ota_reply_info.package_size     = BLE_QIOT_PACKAGE_LENGTH + BLE_QIOT_OTA_DATA_HEADER_LEN;
        ota_reply_info.retry_timeout    = BLE_QIOT_RETRY_TIMEOUT;
        ota_reply_info.reboot_timeout   = BLE_QIOT_REBOOT_TIME;
        ota_reply_info.last_file_size   = 0;
        ota_reply_info.package_interval = BLE_QIOT_PACKAGE_INTERVAL;
#if BLE_QIOT_SUPPORT_RESUMING
        reply_flag |= BLE_QIOT_OTA_RESUME_ENABLE;
        // check file crc to determine its the same file, download the new file if its different
        if (ble_qiot_ota_info_valid() && (file_crc == sg_ota_info.download_file_info.file_crc) &&
            (file_size == sg_ota_info.download_file_info.file_size)) {
            ota_reply_info.last_file_size = HTONL(ble_ota_download_size_get());
        }
#endif //BLE_QIOT_SUPPORT_RESUMING

        sg_ota_info.download_file_info.file_size = file_size;
        sg_ota_info.download_file_info.file_crc  = file_crc;
        memcpy(sg_ota_info.download_file_info.file_version, p, version_len);

        ble_ota_user_start_cb();
        ble_ota_timer_start();
        // handler the ota data after ota request
        BLE_QIOT_OTA_FLAG_SET(BLE_QIOT_OTA_REQUEST_BIT);
        notify_data = (char *)&ota_reply_info;
        notify_len  = sizeof(ble_ota_reply_info) - sizeof(ota_reply_info.rsv);

        os_sem_create(&sg_ota_sem, 0);
    } else {
        reply_flag &= ~BLE_QIOT_OTA_ENABLE;
        notify_data = (char *)&ret;
        notify_len  = sizeof(uint8_t);
    }
    return ble_event_notify(BLE_QIOT_EVENT_UP_REPLY_OTA_REPORT, &reply_flag, sizeof(uint8_t), (const char *)notify_data,
                            notify_len);
}

/* static int ble_ota_verify_result_hdl(int calc_crc){ */
/*     os_sem_post(&sg_ota_sem); */
/*     return 0; */
/* } */
// call the function after the server inform or the device receive the last package
static void ble_ota_reboot_timer(void *priv)
{
    cpu_reset();
}

static int ble_ota_write_boot_info_callback(int err)
{
#if OTA_TWS_SAME_TIME_ENABLE
    if (sg_ota_tws_same_api && sg_ota_tws_same_api->tws_ota_result_hdl) {
        sg_ota_tws_same_api->tws_ota_result_hdl(err);
    }
#else
    if (err == 0) {
        sys_timeout_add(NULL, ble_ota_reboot_timer, 500);
    }
#endif
    return 0;
}

ble_qiot_ret_status_t ble_ota_file_end_handle(void)
{
    int      crc_buf_len  = 0;
    uint32_t crc_file_len = 0;
    uint32_t crc          = 0;
    int slave_boot_info_state  = 0;
    // the function called only once in the same ota process
    sg_ota_flag = 0;
    ble_ota_timer_delete();

    ble_qiot_log_i("calc crc start");

    /* u32 dual_bank_update_read_data(u32 offset, u8 *read_buf, u32 read_len); */
    while (crc_file_len < sg_ota_info.download_file_info.file_size) {
        crc_buf_len = sizeof(sg_ota_data_buf) > (sg_ota_info.download_file_info.file_size - crc_file_len)
                      ? (sg_ota_info.download_file_info.file_size - crc_file_len)
                      : sizeof(sg_ota_data_buf);
        memset(sg_ota_data_buf, 0, sizeof(sg_ota_data_buf));
        dual_bank_update_read_data(ble_ota_download_address_get() + crc_file_len, (char *)sg_ota_data_buf, crc_buf_len);
        crc_file_len += crc_buf_len;
        crc = ble_qiot_crc32(crc, (const uint8_t *)sg_ota_data_buf, crc_buf_len);
        // maybe need task delay
    }

    ble_qiot_log_i("calc crc %x, file crc %x", crc, sg_ota_info.download_file_info.file_crc);

#if OTA_TWS_SAME_TIME_ENABLE
    if (sg_ota_tws_same_api && sg_ota_tws_same_api->enter_verfiy_hdl) {
        if (sg_ota_tws_same_api->enter_verfiy_hdl(NULL)) {      //slave verify err
            crc = 0;
        }
    }
#endif

    if (crc == sg_ota_info.download_file_info.file_crc) {
#if OTA_TWS_SAME_TIME_ENABLE
        if (sg_ota_tws_same_api && sg_ota_tws_same_api->exit_verify_hdl) {
            u8 update_boot_info_flag, verify_err;
            if (sg_ota_tws_same_api->exit_verify_hdl(&verify_err, &update_boot_info_flag) == 0) {      //slave write boot_info err
                slave_boot_info_state = 1;
            }
        }
#endif
        if (BLE_QIOT_RS_OK == ble_ota_user_valid_cb() && !slave_boot_info_state) {
            dual_bank_update_burn_boot_info(ble_ota_write_boot_info_callback);
            int ret = ble_ota_report_check_result(BLE_QIOT_OTA_VALID_SUCCESS, 0);
            ble_ota_user_stop_cb(BLE_QIOT_OTA_SUCCESS);
        } else {
            ble_ota_report_check_result(BLE_QIOT_OTA_VALID_FAIL, BLE_QIOT_OTA_FILE_ERROR);
            ble_ota_user_stop_cb(BLE_QIOT_OTA_ERR_FILE);
        }
    } else {
        ble_ota_report_check_result(BLE_QIOT_OTA_VALID_FAIL, BLE_QIOT_OTA_CRC_ERROR);
        ble_ota_user_stop_cb(BLE_QIOT_OTA_ERR_CRC);
    }
    ble_ota_clear_info();
    return BLE_QIOT_RS_OK;
}

static ble_qiot_ret_status_t ble_qiot_ota_data_saved(char *data, uint16_t data_len)
{
    int     ret     = 0;
    uint8_t percent = 0;

    // write data to flash if the buffer overflow
    if ((data_len + sg_ota_data_buf_size) > sizeof(sg_ota_data_buf)) {
        // ble_qiot_log_e("data buf overflow, write data");
        if (BLE_QIOT_RS_OK != ble_ota_write_data_to_flash()) {
            return BLE_QIOT_RS_ERR;
        }
        // update the ota info if support resuming
        percent = (ble_ota_download_size_get() + sg_ota_data_buf_size) * 100 / sg_ota_info.download_file_info.file_size;
        if (percent > ble_ota_file_percent_get()) {
            ble_ota_file_percent_set(percent);
            ret = ble_ota_write_info();
            if (ret != BLE_QIOT_RS_OK) {
                ble_qiot_log_e("update ota info failed");
                return BLE_QIOT_RS_ERR;
            }
        }
        memset(sg_ota_data_buf, 0, sizeof(sg_ota_data_buf));
        sg_ota_data_buf_size = 0;
    }

    memcpy(sg_ota_data_buf + sg_ota_data_buf_size, data, data_len);
    sg_ota_data_buf_size += data_len;
    ble_ota_download_size_inc(data_len);

    // if the last package, write to flash and reply the server
    if (ble_ota_download_size_get() == sg_ota_info.download_file_info.file_size) {
        ble_qiot_log_i("receive the last package");
        if (BLE_QIOT_RS_OK != ble_ota_write_data_to_flash()) {
            return BLE_QIOT_RS_ERR;
        }
        ble_ota_reply_ota_data();
        // set the file receive end bit
        BLE_QIOT_OTA_FLAG_SET(BLE_QIOT_OTA_RECV_END_BIT);
        memset(sg_ota_data_buf, 0, sizeof(sg_ota_data_buf));
        sg_ota_data_buf_size = 0;
    }

    return BLE_QIOT_RS_OK;
}

ble_qiot_ret_status_t ble_ota_data_handle(const char *in_buf, int buf_len)
{
    POINTER_SANITY_CHECK(in_buf, BLE_QIOT_RS_ERR_PARA);

    uint8_t  seq      = 0;
    char    *data     = NULL;
    uint16_t data_len = 0;

    if (!BLE_QIOT_OTA_FLAG_IS_SET(BLE_QIOT_OTA_REQUEST_BIT)) {
        ble_qiot_log_w("ota request is need first");
        return BLE_QIOT_RS_ERR;
    }

    seq      = in_buf[0];
    data     = (char *)in_buf + 1;
    data_len = (uint16_t)buf_len - 1;

    // ble_qiot_log_hex(BLE_QIOT_LOG_LEVEL_ERR, "data", in_buf, buf_len);
    if (seq == ble_ota_next_seq_get()) {
        BLE_QIOT_OTA_FLAG_SET(BLE_QIOT_OTA_RECV_DATA_BIT);
        BLE_QIOT_OTA_FLAG_SET(BLE_QIOT_OTA_FIRST_RETRY_BIT);
        sg_ota_timeout_cnt = 0;
        ble_ota_next_seq_inc();

        if (BLE_QIOT_RS_OK != ble_qiot_ota_data_saved(data, data_len)) {
            // stop ota and inform the server
            ble_qiot_log_e("stop ota because save data failed");
            return BLE_QIOT_RS_ERR;
        }
        if (BLE_QIOT_OTA_FLAG_IS_SET(BLE_QIOT_OTA_RECV_END_BIT)) {
            return BLE_QIOT_RS_OK;
        }
        // reply the server if received the last package in the loop
        if (BLE_QIOT_TOTAL_PACKAGES == ble_ota_next_seq_get()) {
            ble_qiot_log_e("reply loop");
            ble_ota_reply_ota_data();
            sg_ota_next_seq = 0;
        }
        BLE_QIOT_OTA_FLAG_SET(BLE_QIOT_OTA_RECV_DATA_BIT);
    } else {
        // request data only once in the loop, controlled by the flag
        ble_qiot_log_w("unexpect seq %d, expect seq %d", seq, ble_ota_next_seq_get());
        if (BLE_QIOT_OTA_FLAG_IS_SET(BLE_QIOT_OTA_FIRST_RETRY_BIT)) {
            BLE_QIOT_OTA_FLAG_CLR(BLE_QIOT_OTA_FIRST_RETRY_BIT);
            BLE_QIOT_OTA_FLAG_CLR(BLE_QIOT_OTA_RECV_DATA_BIT);
            ble_ota_reply_ota_data();
            // refresh the timer
            ble_ota_timer_start();
        }
    }
    return BLE_QIOT_RS_OK;
}
#endif //BLE_QIOT_SUPPORT_OTA

#endif //BLE_QIOT_LLSYNC_STANDARD

#ifdef __cplusplus
}
#endif
