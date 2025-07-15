#include "app_config.h"

#if AI_APP_PROTOCOL
#include "app_protocol_ota.h"
#include "app_protocol_api.h"
#include "fs.h"
#include "os/os_error.h"
#include "update_loader_download.h"
#include "dual_bank_updata_api.h"
#include "update.h"
#include "timer.h"
#include "debug.h"

#define LOG_TAG_CONST	APP_OTA
#define LOG_TAG  		"[APP_OTA]"


int clk_set(const char *name, int clk);
update_op_tws_api_t *get_tws_update_api(void);
void tws_sync_update_crc_handler_register(void (*crc_init_hdl)(void), u32(*crc_calc_hdl)(u32 init_crc, u8 *data, u32 len));

#define APP_UPDATE_FAILED               0
#define APP_UPDATE_SUCC                 1

#define APP_CHECK_CRC_SUCC              0
#define APP_CHECK_CRC_FAILED            1

typedef struct _app_protocol_ota_param_t {
    u32 state;
    u32 read_len;
    u32 need_rx_len;
    u8 *read_buf;
    void (*resume_hdl)(void *priv);
    int (*sleep_hdl)(void *priv);
    app_protocol_ota_api ota_api;
    u32 file_offset;
    u8 seek_type;
    u8 update_result;
    u8 sync_update_first_packet;
    u8 ota_cancle_flag;
    u8 wait_app_reboot;
    OS_SEM ota_sem;
} app_protocol_ota_param_t;

static  app_protocol_ota_param_t  app_protocol_ota_param;
static  update_op_tws_api_t *app_protocol_update_tws_api = NULL;

#define __this (&app_protocol_ota_param)

enum {
    BT_SEEK_SET = 0x01,
    BT_SEEK_CUR = 0x02,
    BT_SEEK_TYPE_UPDATE_LEN = 0x10,
};

typedef enum {
    UPDATA_START = 0x00,
    UPDATA_REV_DATA,
    UPDATA_STOP,
} UPDATA_BIT_FLAG;

#define RETRY_TIMES		3

const app_protocol_ota_type_map ota_type_map[] = {
    {.protocol_id = GMA_HANDLER_ID,  .ota_type = PASSIVE_OTA},
    {.protocol_id = MMA_HANDLER_ID,  .ota_type = INITIATIVE_OTA},
    {.protocol_id = DMA_HANDLER_ID,  .ota_type = NO_SUPPORT_OTA},
    {.protocol_id = TME_HANDLER_ID,  .ota_type = INITIATIVE_OTA},
    {.protocol_id = AMA_HANDLER_ID,  .ota_type = NO_SUPPORT_OTA},
};

//根据protocol_id获取协议升级类型
u8 check_ota_type_by_protocol_id(int id)
{
    u8 i = 0;
    for (; i < sizeof(ota_type_map) / sizeof(app_protocol_ota_type_map); i++) {
        if (id == ota_type_map[i].protocol_id) {
            return ota_type_map[i].ota_type;
        }
    }
    return NO_SUPPORT_OTA;
}

static void initiative_ota_resume_hdl_register(void (*resume_hdl)(void *priv), int (*sleep_hdl)(void *priv))
{
    __this->resume_hdl = resume_hdl;
    __this->sleep_hdl = sleep_hdl;
}

void app_protocol_ota_init(app_protocol_ota_api  *api, u8 need_wait_app_reboot)
{
    memcpy(&(__this->ota_api), api, sizeof(app_protocol_ota_api));
    __this->wait_app_reboot = need_wait_app_reboot;
#if OTA_TWS_SAME_TIME_ENABLE
    tws_sync_update_crc_handler_register(__this->ota_api.ota_crc_init_hdl, __this->ota_api.ota_crc_calc_hdl);
    g_printf("crc_init_hdl:0x%x crc_calc_hdl:0x%x\n", __this->ota_api.ota_crc_init_hdl, __this->ota_api.ota_crc_calc_hdl);
#endif
}


void initiative_ota_init(void (*resume_hdl)(void *priv), int (*sleep_hdl)(void *priv))
{
    os_sem_create(&(__this->ota_sem), 0);
    initiative_ota_resume_hdl_register(resume_hdl, sleep_hdl);
}

u16 initiative_ota_f_open(void)
{
    log_info(">>>initiative_ota_f_open\n");
    __this->file_offset = 0;
    __this->seek_type = BT_SEEK_SET;
    return 1;
}

void initiative_ota_handle(u8 state, void *buf, int len)
{
    /* log_info("R"); */
    if (state != __this->state) {
        log_info(">>>initiative state err\n");
        return;
    }

    switch (state) {
    case UPDATA_REV_DATA:
        if (__this->read_buf) {
            memcpy(__this->read_buf, buf, len);
            __this->read_len = len;
            __this->state = 0;
        }
        break;

    case UPDATA_STOP:
        __this->state = 0;
        break;
    }

    if (__this->resume_hdl) {
        __this->resume_hdl(NULL);
    }
}

u16 initiative_ota_f_read(void *fp, u8 *buff, u16 len)
{
    u8 retry_cnt = 0;

    __this->need_rx_len = len;
    __this->state = UPDATA_REV_DATA;
    __this->read_len = 0;
    __this->read_buf = buff;

__RETRY:
    if (app_protocol_get_cur_handle_id() == 0) { //如果已经断开连接直接返回-1 */
        return -1;
    }

    if (__this->ota_cancle_flag) {
        return -1;
    }

    if (__this->ota_api.ota_request_data == NULL) {
        return -1;
    }

    printf("initiative_ota_read\n");
    __this->ota_api.ota_request_data(fp, __this->file_offset, len);

    while (!((0 == __this->state) && (__this->read_len == len))) {
        if (__this->sleep_hdl && app_protocol_get_cur_handle_id()) {
            __this->sleep_hdl(NULL);
        } else {
            len = -1;
            break;
        }

        if (!((0 == __this->state) && (__this->read_len == len))) {
            if (retry_cnt++ > RETRY_TIMES) {
                len = (u16) - 1;
                break;
            } else {
                goto __RETRY;
            }
        }
    }

    if ((u16) - 1 != len) {
        __this->file_offset += len;
    }

    return len;
}

int initiative_ota_f_seek(void *fp, u8 type, u32 offset)
{
    if (type == SEEK_SET) {
        __this->file_offset = offset;
        __this->seek_type = BT_SEEK_SET;
    } else if (type == SEEK_CUR) {
        __this->file_offset += offset;
        __this->seek_type = BT_SEEK_CUR;
    }

    return 0;//FR_OK;
}

static u16 initiative_ota_f_stop(u8 err)
{
    //err = update_result_handle(err);
    __this->state = UPDATA_STOP;
    log_info(">>>initiative_ota_stop:%x err:0x%x\n", __this->state, err);

    if (__this->ota_cancle_flag) {
        return -1;
    }

    if (__this->ota_api.ota_request_data) {
        __this->ota_api.ota_request_data(NULL, 0, 0);
    }

    while (!(0 == __this->state)) {
        if (__this->sleep_hdl) {          // && get_rcsp_connect_status()) {
            if (__this->sleep_hdl(NULL) == OS_TIMEOUT) {
                break;
            }
        } else {
            break;
        }
    }

    if (__this->ota_api.ota_report_result) {
        __this->ota_api.ota_report_result(err);
    }

    return 1;
}

static int initiative_ota_notify_update_content_size(void *priv, u32 size)
{
    int err;
    u8 data[4];
    /* WRITE_BIG_U32(data, size); */
    /* user_change_ble_conn_param(0); */
    log_info("send content_size:%x\n", size);
    if (__this->ota_api.ota_notify_file_size) {
        __this->ota_api.ota_notify_file_size(size);
    }
    /* err = JL_CMD_send(JL_OPCODE_NOTIFY_UPDATE_CONENT_SIZE, data, sizeof(data), JL_NEED_RESPOND); */

    return err;
}

const update_op_api_t initiative_update_op = {
    .ch_init = initiative_ota_init,
    .f_open = initiative_ota_f_open,
    .f_read = initiative_ota_f_read,
    .f_seek = initiative_ota_f_seek,
    .f_stop = initiative_ota_f_stop,
    .notify_update_content_size = initiative_ota_notify_update_content_size,
};

void app_protocol_ota_update_success_reset(void *priv)
{
    cpu_reset();
}

static void initiative_ota_state_cbk(int type, u32 state, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)priv;
    if (ret_code) {
        log_info("state:%x err:%x\n", ret_code->stu, ret_code->err_code);
    }
    switch (state) {
    case UPDATE_CH_EXIT:
        if (UPDATE_DUAL_BANK_IS_SUPPORT()) {
            if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
                /* set_jl_update_flag(1); */
                log_info(">>>initiative update succ\n");
                update_result_set(UPDATA_SUCC);
                __this->update_result = APP_UPDATE_SUCC;
#if !OTA_TWS_SAME_TIME_ENABLE
                sys_timeout_add(NULL,  app_protocol_ota_update_success_reset, 2000);          //延时一段时间再reset保证命令已经发送
#endif
            } else {
                log_info(">>>initiative update failed\n");
                update_result_set(UPDATA_DEV_ERR);
                __this->update_result = APP_UPDATE_FAILED;
            }
        } else {
            if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
                __this->update_result = APP_UPDATE_SUCC;
                /* set_jl_update_flag(1); */
            }
        }
        os_sem_post(&(__this->ota_sem));
        break;
    }
}

//被动升级请求下一帧数据
int passive_ota_write_callback(void *priv)
{
    if (__this->ota_api.ota_request_data) {
        __this->ota_api.ota_request_data(NULL, 0, 0);
    }
    return 0;
}


int passive_ota_set_boot_info_callback(int ret)
{
    if (__this->ota_api.ota_report_result) {
        __this->ota_api.ota_report_result(ret);
    }

    if (ret == 0) {             //0表示boot info写成功
#if OTA_TWS_SAME_TIME_ENABLE
        if (app_protocol_update_tws_api && app_protocol_update_tws_api->tws_ota_result_hdl && !__this->wait_app_reboot) {
            app_protocol_update_tws_api->tws_ota_result_hdl(0);
        }
#else
        sys_timeout_add(NULL,  app_protocol_ota_update_success_reset, 2000);          //延时一段时间再reset保证命令已经发送
#endif
        __this->update_result = APP_UPDATE_SUCC;
    } else {
        dual_bank_passive_update_exit(NULL);
#if OTA_TWS_SAME_TIME_ENABLE
        if (app_protocol_update_tws_api && app_protocol_update_tws_api->tws_ota_err) {
            app_protocol_update_tws_api->tws_ota_err(0);
        }
        __this->update_result = APP_UPDATE_FAILED;
#endif
    }
    return 0;
}

int passive_ota_crc_result_call_back(int crc_res)
{
    if (crc_res) {      //校验成功
        log_info("crc check succ, write boot_info\n");
        u8 update_boot_info_flag, verify_err;
#if OTA_TWS_SAME_TIME_ENABLE
        if (app_protocol_update_tws_api && app_protocol_update_tws_api->exit_verify_hdl) {
            if (app_protocol_update_tws_api->exit_verify_hdl(&verify_err, &update_boot_info_flag)) {
                dual_bank_update_burn_boot_info((int (*)(int))passive_ota_set_boot_info_callback);
            } else {     //从机写boot_info失败
                if (__this->ota_api.ota_report_result) {
                    __this->ota_api.ota_report_result(APP_CHECK_CRC_FAILED);
                }
                if (app_protocol_update_tws_api && app_protocol_update_tws_api->tws_ota_err) {      //通知从机升级失败
                    app_protocol_update_tws_api->tws_ota_err(0);
                }
                __this->update_result = APP_UPDATE_FAILED;
            }
        } else
#endif
        {
            dual_bank_update_burn_boot_info((int (*)(int))passive_ota_set_boot_info_callback);
        }
    } else {              //校验失败
        if (__this->ota_api.ota_report_result) {
            __this->ota_api.ota_report_result(APP_CHECK_CRC_FAILED);
        }
#if OTA_TWS_SAME_TIME_ENABLE
        if (app_protocol_update_tws_api && app_protocol_update_tws_api->tws_ota_err) {      //通知从机升级失败
            app_protocol_update_tws_api->tws_ota_err(0);
        }
#endif
        __this->update_result = APP_UPDATE_FAILED;
        //app_protocol_reply_frame_check_result(crc_res);
        dual_bank_passive_update_exit(NULL);
    }
    return 0;
}

int app_protocol_ota_interrupt_by_disconnect(int id)
{
    if (check_ota_type_by_protocol_id(id) == INITIATIVE_OTA) {

    } else {
#if OTA_TWS_SAME_TIME_ENABLE
        if (app_protocol_update_tws_api  && app_protocol_update_tws_api->tws_ota_err) {
            app_protocol_update_tws_api->tws_ota_err(0);
        }
#endif
        dual_bank_passive_update_exit(NULL);
    }
    return 0;
}

int app_protocol_update_success_flag(void)
{
    return __this->update_result;
}

int app_protocol_ota_message_handler(int id, int opcode, u8 *data, u32 len)
{
    int err_code = UPDATE_RESULT_ERR_NONE;
    int ret = 0;
    switch (opcode) {
    case APP_PROTOCOL_OTA_CHECK:
        log_info("APP_PROTOCOL_OTA_CHECK\n");
        ota_frame_info *frame_info = data;
        dual_bank_passive_update_init(frame_info->frame_crc, frame_info->frame_size, frame_info->max_pkt_len, NULL);
        ret = dual_bank_update_allow_check(frame_info->frame_size);
        if (ret) {
            err_code = UPDATE_RESULT_RESOURCE_LIMIT;
            dual_bank_passive_update_exit(NULL);
        } else { //for tws sync update init
#if OTA_TWS_SAME_TIME_ENABLE
            app_protocol_update_tws_api = get_tws_update_api();
            struct __tws_ota_para tws_update_para;
            tws_update_para.fm_size = frame_info->frame_size;
            tws_update_para.fm_crc16 = frame_info->frame_crc;
            tws_update_para.max_pkt_len = frame_info->max_pkt_len;
            if (app_protocol_update_tws_api && app_protocol_update_tws_api->tws_ota_start) {
                if (app_protocol_update_tws_api->tws_ota_start(&tws_update_para)) {
                    err_code = UPDATE_RESULT_OTA_TWS_START_ERR;
                    dual_bank_passive_update_exit(NULL);
                }
            }
            __this->sync_update_first_packet = 1;
#endif
        }
        break;

    case APP_PROTOCOL_OTA_BEGIN:
        update_mode_info_t info = {
            .type = BLE_APP_UPDATA,
            .state_cbk = initiative_ota_state_cbk,
            .p_op_api = &initiative_update_op,
            .task_en = 1,
        };
        __this->ota_cancle_flag = 0;
        app_active_update_task_init(&info);
        break;

    case APP_PROTOCOL_OTA_TRANS_DATA:
        log_info("APP_PROTOCOL_OTA_TRANS_DATA\n");
        if (check_ota_type_by_protocol_id(id) == INITIATIVE_OTA) {
            initiative_ota_handle(UPDATA_REV_DATA, data, len);
        } else {
#if OTA_TWS_SAME_TIME_ENABLE
            if (!__this->sync_update_first_packet) {   //第一次传输不需要pend
                if (app_protocol_update_tws_api && app_protocol_update_tws_api->tws_ota_data_send_pend) {
                    if (app_protocol_update_tws_api->tws_ota_data_send_pend()) {
                        log_info(" UPDATE_ERR_WAIT_TWS_RESPONSE_TIMEOUT\n");
                        err_code = UPDATE_RESULT_OTA_TWS_NO_RSP;
                        break;
                    }
                }
            }
            __this->sync_update_first_packet = 0;
#endif
            dual_bank_update_write(data, len, passive_ota_write_callback);
#if OTA_TWS_SAME_TIME_ENABLE
            if (app_protocol_update_tws_api && app_protocol_update_tws_api->tws_ota_data_send) {
                app_protocol_update_tws_api->tws_ota_data_send(data, len);
            }
#endif
        }
        break;

    case APP_PROTOCOL_OTA_GET_APP_VERSION:
        u32 fm_version = app_protocol_get_version_by_id(id);
        break;

    case APP_PROTOCOL_OTA_CHECK_CRC:
        if (check_ota_type_by_protocol_id(id) == INITIATIVE_OTA) {
            os_sem_pend(&(__this->ota_sem), 0);
            err_code = __this->update_result;
        } else {
            clk_set("sys", 120 * 1000000L);                                      //提升主频加快CRC校验速度
#if OTA_TWS_SAME_TIME_ENABLE
            if (app_protocol_update_tws_api && app_protocol_update_tws_api->tws_ota_data_send_pend) {           //pend 最后一包数据
                if (app_protocol_update_tws_api->tws_ota_data_send_pend()) {
                    log_info(" UPDATE_ERR_WAIT_TWS_RESPONSE_TIMEOUT\n");
                    err_code = UPDATE_RESULT_OTA_TWS_NO_RSP;
                    break;
                }
            }

            if (app_protocol_update_tws_api && app_protocol_update_tws_api->enter_verfiy_hdl) {
                if (app_protocol_update_tws_api->enter_verfiy_hdl(NULL)) {      //从机校验CRC
                    log_info("update enter verify err\n");
                    if (__this->ota_api.ota_report_result) {
                        __this->ota_api.ota_report_result(APP_CHECK_CRC_FAILED);
                    }
                } else {
                    log_info("update enter verify succ\n");
                    dual_bank_update_verify(__this->ota_api.ota_crc_init_hdl, __this->ota_api.ota_crc_calc_hdl, passive_ota_crc_result_call_back);
                }
            } else
#endif
            {
                dual_bank_update_verify(__this->ota_api.ota_crc_init_hdl, __this->ota_api.ota_crc_calc_hdl, passive_ota_crc_result_call_back);
            }
        }
        break;

    case APP_PROTOCOL_OTA_CANCLE:
        __this->ota_cancle_flag = 1;
        break;

    case APP_PROTOCOL_OTA_REBOOT:
        __this->state = 0;
        /* #if OTA_TWS_SAME_TIME_ENABLE            //同步升级在update lib里进行重启 */
        /*         g_printf("APP_PROTOCOL_OTA_REBOOT!!!\n"); */
        /* #else */
        /*         cpu_reset(); */
        /* #endif */
        break;

    case APP_PROTOCOL_OTA_PERCENT:

        break;
    case APP_PROTOCOL_OTA_END:

        break;
    case APP_PROTOCOL_OTA_SUCCESS:

        break;
    case APP_PROTOCOL_OTA_FAIL:

        break;
    }
    return err_code;
}
#endif
