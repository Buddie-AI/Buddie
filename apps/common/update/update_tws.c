#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "generic/lbuf.h"
#include "init.h"
#include "update_tws.h"
#include "dual_bank_updata_api.h"
#include "bt_tws.h"
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "update.h"
#include "app_main.h"

#if (OTA_TWS_SAME_TIME_ENABLE && !OTA_TWS_SAME_TIME_NEW)

//#define LOG_TAG_CONST       EARPHONE
#define LOG_TAG             "[UPDATE_TWS]"
#define log_errorOR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define THIS_TASK_NAME	            "tws_ota"
#define SLAVE_REV_BUF_LEN           1024*2

extern void sys_enter_soft_poweroff(void *priv);

void db_update_notify_fail_to_phone();

struct __tws_ota_var {
    OS_SEM master_sem;
    OS_SEM slave_sem;
    OS_SEM confirm_sem;
    struct __tws_ota_para para;
    u8 ota_type;
    u8 ota_status;
    u8 ota_remote_status;
    u8 ota_result;
    volatile u32 ota_data_len;
    u16 ota_timer_id;
    u8 ota_verify_cnt;
    volatile u32 ota_confirm;
    cbuffer_t cbuffer;
    u8 *slave_r_buf;
};
struct __tws_ota_var tws_ota_var;
#define __this (&tws_ota_var)

void tws_ota_event_post(u32 type, u8 event)
{
    struct sys_event e;
    e.type = SYS_BT_EVENT;
    e.arg  = (void *)type;
    e.u.bt.event = event;
    sys_event_notify(&e);
}

u8 tws_ota_control(int type, ...)
{
    int ret   = 0;
    int role  = 0;
    int value = 0;

    va_list argptr;
    va_start(argptr, type);

    switch (type) {
    case OTA_TYPE_SET:
        value = va_arg(argptr, int);
        __this->ota_type = value;
        break;
    case OTA_TYPE_GET:
        ret = __this->ota_type;
        break;
    case OTA_STATUS_SET:
        value = va_arg(argptr, int);
        __this->ota_status = value;
        break;
    case OTA_STATUS_GET:
        ret = __this->ota_status;
        break;
    case OTA_REMOTE_STATUS_SET:
        value = va_arg(argptr, int);
        __this->ota_remote_status = value;
        break;
    case OTA_REMOTE_STATUS_GET:
        ret = __this->ota_remote_status;
        break;
    case OTA_RESULT_SET:
        value = va_arg(argptr, int);
        role  = va_arg(argptr, int);
        if (value == 1) {
            __this->ota_result |= BIT(role);
        } else {
            __this->ota_result &= ~BIT(role);
        }
        break;
    case OTA_RESULT_GET:
        ret = __this->ota_result;
        break;
    }

    va_end(argptr);

    return ret;
}

int tws_ota_init(void)
{
    tws_api_auto_role_switch_disable();
    memset((u8 *)__this, 0, sizeof(struct __tws_ota_var));
    __this->ota_status = OTA_INIT;
    __this->ota_type = OTA_TWS;

    if (a2dp_get_status() == BT_MUSIC_STATUS_STARTING) {
        /* log_info("try pause a2dp music"); */
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
    }
    return 0;
}

#define TWS_FUNC_ID_OTA_SYNC    TWS_FUNC_ID('O', 'T', 'A', 'S')
static void tws_ota_data_read_s_from_m(void *_data, u16 len, bool rx)
{
    if (rx && __this->ota_status == OTA_START && __this->slave_r_buf) {
        /* r_log_info("Offset:%x r_len:%d\n",__this->ota_data_len,len); */
        /* put_buf(_data, len); */
        __this->ota_data_len += len;
        if (cbuf_is_write_able(&(__this->cbuffer), len)) {
            cbuf_write(&(__this->cbuffer), _data, len);
        } else {
            log_info("ota cbuf write err\n");
        }
        os_sem_post(&__this->slave_sem);
    }
}

REGISTER_TWS_FUNC_STUB(app_ota_sync_stub) = {
    .func_id = TWS_FUNC_ID_OTA_SYNC,
    .func    = tws_ota_data_read_s_from_m,
};

int tws_ota_data_send_m_to_s(u8 *buf, u16 len)
{
    if (!(tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        return 0;
    }

    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        return 0;
    }

    int ret = tws_api_send_data_to_slave(buf, len, TWS_FUNC_ID_OTA_SYNC);
    return ret;
}

int tws_ota_err_callback(u8 reason)
{
    tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_ERR);
    return 0;
}

static void tws_ota_update_loop(void *priv)
{
    u32 total_len = 0;
    u32 len = 0;
    u8 *tmp_buf = NULL;
    int ret = 0;

    u8 sniff_wait_exit_timeout = 30;//3s

    log_info("tws_ota_update_loop\n");

    while (1) {
        os_sem_pend(&__this->slave_sem, 0);

        if (sniff_wait_exit_timeout) {
            //wait slave exit sniff
            extern u8 btstcak_get_bt_mode(void);
            while (btstcak_get_bt_mode() && sniff_wait_exit_timeout--) {
                log_info("wait sniff exit \n");
                os_time_dly(10);
            }
            log_info(">>>>>>>sniff timeout:%d \n", sniff_wait_exit_timeout);

            if (!sniff_wait_exit_timeout) {
                log_info(">>>>>>wait sniff exit timeout !!!! \n");
            }
            sniff_wait_exit_timeout = 0;
            dual_bank_passive_update_init(__this->para.fm_crc16, __this->para.fm_size, __this->para.max_pkt_len, NULL);
            ret = dual_bank_update_allow_check(__this->para.fm_size);
            if (ret) {
                log_info("fm_size:%x can't update\n", __this->para.fm_size);
                tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_ERR);
                continue;
            }
            tws_api_sync_call_by_uuid(0xA2E22223, SYNC_CMD_START_UPDATE, 400);
            continue;
        }

        total_len = cbuf_get_data_size(&(__this->cbuffer));
        /* log_info("total_len : %d\n",total_len); */
        while (total_len) {
            if (total_len > get_dual_bank_passive_update_max_buf()) {
                len =  get_dual_bank_passive_update_max_buf();
            } else {
                len = total_len;
            }

            if (!len) {
                continue;
            }
            tmp_buf = malloc(len);
            if (tmp_buf) {
                if (cbuf_read(&(__this->cbuffer), tmp_buf, len) == len) {
                    putchar('D');
                    dual_bank_update_write(tmp_buf, len, NULL);
                } else {
                    log_error("read err\n");
                }
                free(tmp_buf);
            } else {
                log_error("malloc err\n");
            }
            total_len -= len;
        }
    }
}

static void ota_finish_confirm(void *priv)
{
    log_info("ota_finish_confirm:%d\n", __this->ota_confirm);
    if (!__this->ota_confirm) {
        //ota tws confirm err,earse boot info
        flash_update_clr_boot_info(CLEAR_APP_UPDATE_BANK);
        /* ASSERT(0, "ota tws confirm err\n"); */
    }
    tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_SUCC);

}

u16 tws_ota_enter_verify(void *priv)
{
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        log_info("tws_disconn in verify\n");
        db_update_notify_fail_to_phone();
        return -1;
    }
    tws_ota_send_data_to_sibling(TWS_UPDATE_VERIFY, NULL, 0);
    os_sem_pend(&__this->master_sem, 1000);
    //这里pend完，要做超时的准备
    if (__this->ota_status == OTA_VERIFY_ING) {
        return 0;
    } else {
        db_update_notify_fail_to_phone();
        return -1;
    }
}

u16 tws_ota_exit_verify(u8 *res, u8 *up_flg)
{
    //not updata boot info in lib
    *up_flg = 1;

    u8 tws_ota_result[2];
    u8 master_result = *res;
    u8 result = 0;
    tws_ota_control(OTA_STATUS_SET, OTA_VERIFY_END);
    tws_ota_control(OTA_RESULT_SET, !master_result, TWS_ROLE_MASTER);
__RESTART:
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        db_update_notify_fail_to_phone();
        return 0;
    }
    result = tws_ota_control(OTA_RESULT_GET);
    tws_ota_result[0] = result;
    tws_ota_result[1] = tws_ota_control(OTA_STATUS_GET);
    tws_ota_send_data_to_sibling(TWS_UPDATE_RESULT_EXCHANGE, (u8 *)&tws_ota_result, 2);

    if ((result & BIT(TWS_ROLE_SLAVE)) && (result & BIT(TWS_ROLE_SLAVE))) {
        log_info("tws already ota succ1\n");
        os_sem_set(&__this->master_sem, 0);
        tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_OVER);
        return 1;
    } else if (__this->ota_remote_status ==  OTA_VERIFY_END || __this->ota_remote_status ==  OTA_OVER) {
        tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_ERR);
        return 0;
    } else {
        os_sem_pend(&__this->master_sem, 200);
        goto __RESTART;
    }
    return 1;
}

u16 tws_ota_updata_boot_info_over(void *priv)
{
    log_info("master update_burn_boot_info succ\n");
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        log_info("tws_disconn, ota open fail");
        db_update_notify_fail_to_phone();
        return -1;
    }

    //等待从机更新完成
    log_info("1------pend in");
    os_sem_pend(&__this->confirm_sem, 300);
    log_info("1------pend out");

    log_info("-------mz01");
    if (__this->ota_confirm) {
        log_info("-------mz02");
        os_sem_set(&__this->confirm_sem, 0);
        tws_ota_send_data_to_sibling(TWS_UPDATE_OVER_CONFIRM_REQ, NULL, 0);
        log_info("2------pend in");
        if (OS_TIMEOUT == os_sem_pend(&__this->confirm_sem, 300)) {
            __this->ota_confirm = 0;
        }
        log_info("2------pend out");
    }

    if (!__this->ota_confirm) {
        log_info("-------mz03");
        //ota tws confirm err,earse boot info
        flash_update_clr_boot_info(CLEAR_APP_UPDATE_BANK);
        /* ASSERT(0, "ota tws confirm err\n"); */
        return -1;
    }
    /* tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS,OTA_UPDATE_SUCC); //主机等手机发重启命令*/
    return 0;

}

int tws_ota_open(struct __tws_ota_para *para)
{
    int ret = 0;
    log_info("tws_ota_open\n");

    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        log_info("tws_disconn, ota open fail");
        db_update_notify_fail_to_phone();
        return -1;
    }

    os_sem_create(&__this->master_sem, 0);
    os_sem_create(&__this->slave_sem, 0);
    os_sem_create(&__this->confirm_sem, 0);

    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        extern void bt_check_exit_sniff();
        bt_check_exit_sniff();
        //master 发命令给slave启动升级


        log_info("master updata info: crc16:%x size:%x max_pkt_len:%d\n", para->fm_crc16, para->fm_size, para->max_pkt_len);
        if (__this->ota_type == OTA_TWS) {
            tws_ota_send_data_to_sibling(TWS_UPDATE_START, (u8 *)para, sizeof(struct __tws_ota_para));
            log_info("sem pend in ...");
            os_sem_pend(&__this->master_sem, 600);
            log_info("sem pend out ...");
            //判断对耳状态
            if (__this->ota_status == OTA_START) {
                log_info("slave has ready");
                ret = 0;
            } else {
                log_info("slave answer timeout");
                db_update_notify_fail_to_phone();
                ret = -1;
            }
        }
    } else {
        log_info("slave updata info: crc16:%x size:%x max_pkt_len:%d\n", para->fm_crc16, para->fm_size, para->max_pkt_len);

        /* tws_api_sync_call_by_uuid('T', SYNC_CMD_START_UPDATE, 100); */
        tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_START_UPDATE_READY);
    }
    return ret;
}

int tws_ota_close(void)
{
    int ret = 0;
    log_info("%s", __func__);
    if (__this->slave_r_buf) {
        free(__this->slave_r_buf);
        __this->slave_r_buf = 0;
        task_kill(THIS_TASK_NAME);
    }
    return ret;
}

static void ota_verify_timeout(void *priv)
{
    log_info("ota_verify_timeout:%x %x\n", __this->para.fm_size, __this->ota_data_len);
    if (__this->ota_data_len == __this->para.fm_size) {
        /* tws_api_sync_call_by_uuid('T', SYNC_CMD_START_VERIFY, 100); */
        tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_START_VERIFY);
        return;
    }

    if (__this->ota_verify_cnt > 4) {
        log_info("code len err\n");
        tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_ERR);
        sys_timeout_del(__this->ota_timer_id);
        __this->ota_timer_id = 0;
        return;
    }

    __this->ota_verify_cnt ++;
    __this->ota_timer_id = 0;
    __this->ota_timer_id = sys_timeout_add(NULL, ota_verify_timeout, 500);
}

int tws_ota_get_data_from_sibling(u8 opcode, u8 *data, u8 len)
{
    u8 tws_ota_result[2];
    switch (opcode) {
    //master->slave
    case  TWS_UPDATE_START:
        log_info("TWS_AI_START_UPDATE:%d\n", opcode);
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            if (__this->slave_r_buf) {
                tws_ota_close();
            }
            tws_ota_init();
            memcpy((u8 *) & (__this->para), data, len);
            tws_ota_open(&(__this->para));
        }
        break;

    //master->slave
    case TWS_UPDATE_RESULT_EXCHANGE:
        log_info("TWS_UPDATE_RESULT_EXCHANGE:%d %d\n", data[0], data[1]);
        __this->ota_remote_status = data[1];
        tws_ota_control(OTA_RESULT_SET, (data[0] & BIT(TWS_ROLE_MASTER) ? 1 : 0), TWS_ROLE_MASTER);

        tws_ota_result[0] = __this->ota_result;
        tws_ota_result[1] = __this->ota_status;
        tws_ota_send_data_to_sibling(TWS_UPDATE_RESULT_EXCHANGE_RES, (u8 *)tws_ota_result, 2);
        log_info("master ota result:%x %d\n", tws_ota_control(OTA_RESULT_GET), __this->ota_status);
        break;

    //slave->master
    case TWS_UPDATE_RESULT_EXCHANGE_RES:
        log_info("TWS_UPDATE_RESULT_EXCHANGE_RES:%d %d\n", data[0], data[1]);
        __this->ota_remote_status = data[1];
        tws_ota_control(OTA_RESULT_SET, (data[0] & BIT(TWS_ROLE_SLAVE) ? 1 : 0), TWS_ROLE_SLAVE);
        log_info("slave ota result:%x %d\n", tws_ota_control(OTA_RESULT_GET), __this->ota_status);

        if (tws_ota_control(OTA_RESULT_GET) & BIT(TWS_ROLE_SLAVE)) {
            os_sem_post(&__this->master_sem);
        }
        break;

    //master->slave
    case TWS_UPDATE_VERIFY:
        log_info("TWS_UPDATE_VERIFY\n");
        tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_START_VERIFY);
        break;

    case TWS_UPDATE_OVER:
        log_info("TWS_AI_UPDATE_OVER:%d\n", opcode);
        __this->ota_status = OTA_OVER;
        break;

    //slave to master
    case TWS_UPDATE_OVER_CONFIRM:
        log_info("TWS_UPDATE_OVER_CONFIRM\n");
        __this->ota_confirm = 1;
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            log_info("1------post");
            os_sem_post(&__this->confirm_sem);
        }
        break;
    //master->slave
    case TWS_UPDATE_OVER_CONFIRM_REQ:
        log_info("TWS_UPDATE_OVER_CONFIRM_REQ\n");
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            __this->ota_confirm = 1;
            tws_ota_send_data_to_sibling(TWS_UPDATE_OVER_CONFIRM_RES, NULL, 0);
            log_info("2------post");
            os_sem_post(&__this->confirm_sem);
        }
        break;
    //slave->master
    case TWS_UPDATE_OVER_CONFIRM_RES:
        log_info("TWS_UPDATE_OVER_CONFIRM_RES\n");
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            log_info("3------post");
            os_sem_post(&__this->confirm_sem);
        }
        break;
    }
    return 0;
}

void tws_ota_send_data_to_sibling(u8 opcode, u8 *data, u8 len)
{
    extern void tws_data_to_sibling_send(u8 opcode, u8 * data, u8 len);
    tws_data_to_sibling_send(opcode, data, len);
}



u8 dual_bank_update_burn_boot_info_callback(u8 ret)
{
    if (ret) {
        log_info("update_burn_boot_info err\n");
    } else {
        log_info("slave update_burn_boot_info succ\n");
        tws_ota_send_data_to_sibling(TWS_UPDATE_OVER_CONFIRM, NULL, 0);

        //not recive master confirm
        log_info("3------pend in");
        os_sem_pend(&__this->confirm_sem, 300);
        log_info("3------pend out");

        if (!__this->ota_confirm) {
            //ota tws confirm err,earse boot info
            flash_update_clr_boot_info(CLEAR_APP_UPDATE_BANK);
            /* ASSERT(0, "ota tws confirm err\n"); */
            return 0;
        }

        //确保从机的回复命令送达到主机
        os_time_dly(50);

        tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_SUCC);
        return 0;
    }

    return 1;
}

//slave ota result
static sint32_t gma_ota_slave_result(int crc_res)
{
    u8 ret = crc_res;

    tws_ota_control(OTA_STATUS_SET, OTA_VERIFY_END);

    tws_ota_control(OTA_RESULT_SET, crc_res, TWS_ROLE_SLAVE);

    log_info("gma_ota_slave_result:%d %d\n", crc_res, tws_ota_control(OTA_STATUS_GET));
    return 0;
}

int tws_ota_sync_cmd(int reason)
{
    int ret = 1;

    switch (reason) {
    //slave request
    case SYNC_CMD_START_UPDATE:
        log_info("SYNC_CMD_START_UPDATE\n");

        if (__this->ota_status != OTA_INIT) {
            log_info("tws ota no init");
            break;
        }
        __this->ota_status = OTA_START;
        __this->ota_data_len = 0;
        __this->ota_remote_status = OTA_START;
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            os_sem_post(&__this->master_sem);
        } else {
        }
        break;

    //slave request
    case SYNC_CMD_START_VERIFY:
        log_info("SYNC_CMD_START_VERIFY\n");
        __this->ota_status = OTA_VERIFY_ING;
        __this->ota_remote_status = OTA_VERIFY_ING;

        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            os_sem_post(&__this->master_sem);
        } else {
            dual_bank_update_verify(NULL, NULL, gma_ota_slave_result);
        }
        break;

    //master request
    case SYNC_CMD_UPDATE_OVER:
        log_info("SYNC_CMD_UPDATE_OVER\n");

        __this->ota_status = OTA_OVER;
        __this->ota_remote_status = OTA_OVER;
        if ((__this->ota_result & BIT(TWS_ROLE_MASTER)) && (__this->ota_result & BIT(TWS_ROLE_SLAVE))) {
            log_info("OTA SUCCESS\n");
            //update boot info
            if (tws_api_get_role() == TWS_ROLE_SLAVE) {
                dual_bank_update_burn_boot_info(dual_bank_update_burn_boot_info_callback);
            }
        } else {
            log_info("OTA ERR\n");
            /* tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS,OTA_UPDATE_SUCC); */
        }
        break;

    //slave request
    case SYNC_CMD_UPDATE_ERR:
        log_info("SYNC_CMD_UPDATE_ERR\n");
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
        } else {
            tws_ota_stop(OTA_STOP_UPDATE_OVER_ERR);
        }
        break;

    default:
        ret = 0;
        break;
    }
    return ret;
}

void tws_ota_app_event_deal(u8 evevt)
{
    if (__this->ota_status == OTA_OVER) {
        return;
    }

    switch (evevt) {
    case TWS_EVENT_CONNECTION_DETACH:
    /* case TWS_EVENT_PHONE_LINK_DETACH: */
    case TWS_EVENT_REMOVE_PAIRS:
        log_info("stop ota : %d --1\n", evevt);
        tws_ota_stop(OTA_STOP_LINK_DISCONNECT);
        break;
    default:
        break;
    }
}

void tws_ota_stop(u8 reason)
{
    log_info("%s", __func__);

    if (__this->ota_status != OTA_OVER) {

        //reconnect hfp when start err
        if (reason == OTA_STOP_APP_DISCONNECT || reason == OTA_STOP_UPDATE_OVER_ERR) {
            //在更新信息的时候，手机app断开

            /* if(tws_api_get_role() == TWS_ROLE_MASTER) { */
            /*     extern void user_post_key_msg(u8 user_msg); */
            /*     user_post_key_msg(USER_TWS_OTA_RESUME); */
            /* } */
        }

        __this->ota_status = OTA_OVER;
        if (__this->ota_timer_id) {
            sys_timeout_del(__this->ota_timer_id);
            __this->ota_timer_id = 0;
        }

        tws_ota_close();
        dual_bank_passive_update_exit(NULL);
#if RCSP_UPDATE_EN
        extern void rcsp_db_update_fail_deal(); //双备份升级失败处理
        rcsp_db_update_fail_deal();
#endif
    }
}

int bt_ota_event_handler(struct bt_event *bt)
{
    int ret = 0;
    switch (bt->event) {
    case OTA_START_UPDATE:
        log_info("OTA_START_UPDATE\n");
        tws_api_sync_call_by_uuid(0xA2E22223, SYNC_CMD_START_UPDATE, 400);
        break;
    case OTA_START_UPDATE_READY:
        log_info("OTA_START_UPDATE_READY:%x %x %d\n", __this->para.fm_crc16, __this->para.fm_size, __this->para.max_pkt_len);
        extern void rcsp_before_enter_db_update_mode();
#if(RCSP_UPDATE_EN)
        rcsp_before_enter_db_update_mode();
#endif
        if (__this->slave_r_buf) {
            ASSERT(0, "tws_ota_update_loop already exit\n");
        }
        task_create(tws_ota_update_loop, NULL, THIS_TASK_NAME);
        __this->slave_r_buf = malloc(SLAVE_REV_BUF_LEN);
        ASSERT(__this->slave_r_buf, "slave_r_buf malloc err\n");
        cbuf_init(&(__this->cbuffer), __this->slave_r_buf, SLAVE_REV_BUF_LEN);

        os_sem_post(&__this->slave_sem);
        /* tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS,OTA_START_UPDATE); */
        break;
    case OTA_START_VERIFY:
        log_info("OTA_START_VERIFY\n");
        if (__this->ota_data_len == __this->para.fm_size) {
            tws_api_sync_call_by_uuid(0xA2E22223, SYNC_CMD_START_VERIFY, 1000);
        } else {
            if (__this->ota_timer_id) {
                sys_timeout_del(__this->ota_timer_id);
                __this->ota_timer_id = 0;
            }
            __this->ota_timer_id = sys_timeout_add(NULL, ota_verify_timeout, 500);
        }
        break;
    case OTA_UPDATE_OVER:
        log_info("OTA_UPDATE_OVER\n");
        tws_api_sync_call_by_uuid(0xA2E22223, SYNC_CMD_UPDATE_OVER, 400);
        break;
    case OTA_UPDATE_ERR:
        log_info("OTA_UPDATE_ERR\n");
        tws_api_sync_call_by_uuid(0xA2E22223, SYNC_CMD_UPDATE_ERR, 400);
        break;
    case OTA_UPDATE_SUCC:
        log_info("OTA_UPDATE_SUCC\n");
        update_result_set(UPDATA_SUCC);
        dual_bank_passive_update_exit(NULL);

        /* update_result_set(UPDATA_SUCC); */
        /* extern void cpu_reset(); */
        /* cpu_reset(); */
        //user_ctl.shutdown_need_adv = 0;
        sys_enter_soft_poweroff((void *)1);
        break;
    default:
        break;
    }

    return 0;
}

const update_op_tws_api_t update_tws_api = {
    .tws_ota_start = tws_ota_open,
    .tws_ota_data_send = tws_ota_data_send_m_to_s,
    .tws_ota_err = tws_ota_err_callback,
    .enter_verfiy_hdl = tws_ota_enter_verify,
    .exit_verify_hdl = tws_ota_exit_verify,
    .update_boot_info_hdl =  tws_ota_updata_boot_info_over,
    .tws_ota_result_hdl = NULL,
    .tws_ota_data_send_pend = NULL,
};

update_op_tws_api_t *get_tws_update_api(void)
{
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
        return &update_tws_api;
    } else {
        return NULL;
    }
}

static void rcsp_adv_tws_ota_sync_handler(int reason, int err)
{
    tws_ota_sync_cmd(reason);
}

TWS_SYNC_CALL_REGISTER(rcsp_adv_tws_ota_sync) = {
    .uuid = 0xA2E22223,
    .task_name = "app_core",
    .func = rcsp_adv_tws_ota_sync_handler,
};

#endif

