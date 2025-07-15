#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "generic/lbuf.h"
#include "init.h"
#include "update_tws_new.h"
#include "dual_bank_updata_api.h"
#include "bt_tws.h"
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "update.h"
#include "app_main.h"
#include "timer.h"

#if (OTA_TWS_SAME_TIME_ENABLE && OTA_TWS_SAME_TIME_NEW) //(RCSP_ADV_EN || AI_APP_PROTOCOL))
//#define LOG_TAG_CONST       EARPHONE
#define LOG_TAG             "[UPDATE_TWS]"
#define log_errorOR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define THIS_TASK_NAME	            "tws_ota"

OS_SEM tws_ota_sem;
int tws_ota_timeout_hdl = 0;
static u8 sync_update_sn = 1;

static void (*user_chip_tws_update_handle)(void *data, u32 len) = NULL;
static void (*sync_update_crc_init_hdl)(void) = NULL;
static u32(*sync_update_crc_calc_hdl)(u32 init_crc, u8 *data, u32 len) = NULL;

int clk_set(const char *name, int clk);

__attribute__((weak))
void db_update_notify_fail_to_phone()
{

}

void tws_sync_update_crc_handler_register(void (*crc_init_hdl)(void), u32(*crc_calc_hdl)(u32 init_crc, u8 *data, u32 len))
{
    sync_update_crc_init_hdl = crc_init_hdl;
    sync_update_crc_calc_hdl = crc_calc_hdl;
    printf("sync_update_crc_init_hdl:%x\n  sync_update_crc_calc_hdl:%x\n", sync_update_crc_init_hdl, sync_update_crc_calc_hdl);
}

void sys_enter_soft_poweroff(void *priv);
int tws_ota_close(void);

int tws_ota_get_data_from_sibling(u8 opcode, u8 *data, u8 len)
{
    return 0;
}

void tws_ota_timeout(void *priv) 		//从机没收到命令超时就退出升级
{
    tws_ota_close();
    dual_bank_passive_update_exit(NULL);
}

void tws_ota_timeout_reset(void)
{
    if (tws_ota_timeout_hdl) {
        sys_timeout_del(tws_ota_timeout_hdl);
    }
    tws_ota_timeout_hdl = sys_timeout_add(NULL, tws_ota_timeout, 8000);
}

void tws_ota_timeout_del(void)
{
    if (tws_ota_timeout_hdl) {
        sys_timeout_del(tws_ota_timeout_hdl);
    }
}


int tws_ota_trans_to_sibling(u8 *data, u16 len)
{
    if (!(tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        return -1;
    }
    return tws_api_send_data_to_sibling(data, len, TWS_FUNC_ID_OTA_TRANS);
}

u8 dual_bank_update_burn_boot_info_callback(u8 ret)
{
    u8 rsp_data[3] = {0};
    rsp_data[0] = OTA_TWS_WRITE_BOOT_INFO_RSP;
    rsp_data[1] = ++ sync_update_sn;
    printf("sync_sn:%d\n", sync_update_sn);
    if (ret) {
        log_info("update_burn_boot_info err\n");
        rsp_data[2] = OTA_TWS_CMD_ERR;
        return -1;
    } else {
        log_info("slave update_burn_boot_info succ\n");
        rsp_data[2] = OTA_TWS_CMD_SUCC;
        //确保从机的回复命令送达到主机
        /* os_time_dly(50); */
        /* tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_SUCC); */
    }
    if (tws_ota_trans_to_sibling(rsp_data, 3)) {
        return -1;
    }

    return 0;
}

int bt_ota_event_handler(struct bt_event *bt)
{
    int ret = 0;
    switch (bt->event) {
    case OTA_START_UPDATE:
        log_info("OTA_START_UPDATE\n");
        break;
    case OTA_START_UPDATE_READY:
        break;
    case OTA_START_VERIFY:
        log_info("OTA_START_VERIFY\n");
        break;
    case OTA_UPDATE_OVER:
        log_info("OTA_UPDATE_OVER\n");
        update_result_set(UPDATA_SUCC);
        dual_bank_passive_update_exit(NULL);
        cpu_reset();
        //sys_enter_soft_poweroff((void *)1);
        break;
    case OTA_UPDATE_ERR:
        log_info("OTA_UPDATE_ERR\n");
        dual_bank_passive_update_exit(NULL);
        //cpu_reset();
        //sys_enter_soft_poweroff((void *)1);
        break;
    case OTA_UPDATE_SUCC:
        log_info("OTA_UPDATE_SUCC\n");
        update_result_set(UPDATA_SUCC);
        dual_bank_passive_update_exit(NULL);
        sys_enter_soft_poweroff((void *)1);
        break;
    default:
        break;
    }

    return 0;
}

void tws_ota_event_post(u32 type, u8 event)
{
    struct sys_event e;
    e.type = SYS_BT_EVENT;
    e.arg  = (void *)type;
    e.u.bt.event = event;
    sys_event_notify(&e);
}


int tws_ota_data_send_m_to_s(u8 *buf, u16 len)
{
    int ret = 0;
    u8 retry = 5;
    os_sem_set(&tws_ota_sem, 0);
    u8 *data = malloc(len + 2);
    data[0] = OTA_TWS_TRANS_UPDATE_DATA;
    data[1] = ++ sync_update_sn;
    memcpy(&data[2], buf, len);
    while (retry --) {
        ret = tws_ota_trans_to_sibling(data, len + 2);
        if (ret < 0 && ret != -1) {
            os_time_dly(5);
        } else {
            break;
        }
    }
    free(data);
    return 0;
}

int tws_ota_data_send_pend(void)
{
    if (os_sem_pend(&tws_ota_sem, 300) ==  OS_TIMEOUT) {       //等待收到从机的RSP
        return -1;
    }
    return 0;
}


int tws_ota_user_chip_update_send_m_to_s(u8 cmd, u8 *buf, u16 len)
{
    int ret = 0;
    u8 retry = 5;
    u8 *data = malloc(len + 3);
    data[0] = OTA_TWS_USER_CHIP_UPDATE;
    data[1] = ++ sync_update_sn;
    data[2] = cmd;
    memcpy(&data[3], buf, len);
    while (retry --) {
        ret = tws_ota_trans_to_sibling(data, len + 3);
        if (ret < 0 && ret != -1) {
            os_time_dly(5);
        } else {
            break;
        }
    }
    free(data);

    return 0;
}

u16 tws_ota_enter_verify(void *priv)
{
    u8 rsp_data[2];
    rsp_data[0] = OTA_TWS_VERIFY;
    rsp_data[1] = ++ sync_update_sn;
    printf("tws_ota_enter_verify111\n");
    os_sem_set(&tws_ota_sem, 0);
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        log_info("tws_disconn in verify\n");
        db_update_notify_fail_to_phone();
        return -1;
    }
    if (tws_ota_trans_to_sibling(&rsp_data, 2)) {
        return -1;
    }
    if (os_sem_pend(&tws_ota_sem, 800) ==  OS_TIMEOUT) {
        return -1;
    }
    printf("tws_ota_enter_verify222\n");
    return 0;

}

u16 tws_ota_exit_verify(u8 *res, u8 *up_flg)
{
    u8 rsp_data[2];
    rsp_data[0] = OTA_TWS_WRITE_BOOT_INFO;
    rsp_data[1] = ++ sync_update_sn;

    *up_flg = 1;
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        log_info("tws_disconn exit verify\n");
        db_update_notify_fail_to_phone();
        return 0;
    }
    if (tws_ota_trans_to_sibling(&rsp_data, 2)) {
        return 0;
    }
    if (os_sem_pend(&tws_ota_sem, 300) ==  OS_TIMEOUT) {
        return 0;
    }
    return 1;
}

int tws_ota_result(u8 result)
{
    if (result == 0) {
        tws_api_sync_call_by_uuid(0xA2E22223, SYNC_CMD_UPDATE_OVER, 400);
    } else {
        tws_api_sync_call_by_uuid(0xA2E22223, SYNC_CMD_UPDATE_ERR, 400);
    }
    return 0;
}

void tws_ota_stop(u8 reason)
{
    log_info("%s", __func__);

    tws_ota_close();
    dual_bank_passive_update_exit(NULL);
#if RCSP_UPDATE_EN
    extern void rcsp_db_update_fail_deal(); //双备份升级失败处理
    rcsp_db_update_fail_deal();
#endif
}

void tws_ota_app_event_deal(u8 evevt)
{
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

int tws_ota_init(void)
{
    u8 data;
    log_info("tws_ota_init\n");
    tws_api_auto_role_switch_disable();
    os_sem_create(&tws_ota_sem, 0);
    return 0;
}

int tws_ota_open(struct __tws_ota_para *para)
{
    int ret = 0;
    tws_api_auto_role_switch_disable();
    os_sem_create(&tws_ota_sem, 0);
    u8 *data = malloc(sizeof(struct __tws_ota_para) + 2);
    if (!data) {
        ret = -1;
        goto _ERR_RET;
    }
    log_info("tws_ota_open\n");

    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        log_info("tws_disconn, ota open fail");
        db_update_notify_fail_to_phone();
        ret = -1;
        goto _ERR_RET;
    }

    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        extern void bt_check_exit_sniff();
        bt_check_exit_sniff();
        //master 发命令给slave启动升级
    }

    log_info("tws_ota_open1\n");
    data[0] = OTA_TWS_START_UPDATE;
    data[1] = ++ sync_update_sn;
    memcpy(data + 2, para, sizeof(struct __tws_ota_para));
    log_info("tws_ota_open2\n");
    if (tws_ota_trans_to_sibling(data, sizeof(struct __tws_ota_para) + 2)) {
        ret = -1;
        goto _ERR_RET;
    }
    if (os_sem_pend(&tws_ota_sem, 500) == OS_TIMEOUT) {
        log_info("tws_ota_open pend timeout\n");
        ret = -1;
        goto _ERR_RET;
    }
    printf("tws_ota_open succ...\n");
_ERR_RET:
    if (data) {
        free(data);
    }
    return ret;
}

int tws_ota_close(void)
{
    int ret = 0;
    log_info("%s", __func__);
    task_kill(THIS_TASK_NAME);
    return ret;
}


int tws_ota_err_callback(u8 reason)
{
    tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_ERR);
    return 0;
}

u16 tws_ota_updata_boot_info_over(void *priv)
{
    return 0;
}

int tws_verify_result_hdl(int calc_crc)
{
    u8 rsp_data[3];
    printf("tws_verify_result_hdl:%d\n", calc_crc);
    rsp_data[0] = OTA_TWS_VERIFY_RSP;
    rsp_data[1] = ++ sync_update_sn;
    if (calc_crc == 1) {
        rsp_data[2] = OTA_TWS_CMD_SUCC;
    } else {
        rsp_data[2] = OTA_TWS_CMD_ERR;
    }
    if (tws_ota_trans_to_sibling(rsp_data, 3)) {
        return -1;
    }
    return 0;
}

int tws_trans_data_write_callback(void *priv)
{
    u8 rsp_data[3];
    rsp_data[0] = OTA_TWS_TRANS_UPDATE_DATA_RSP;
    rsp_data[1] = ++ sync_update_sn;
    rsp_data[2] = OTA_TWS_CMD_SUCC;
    tws_ota_trans_to_sibling(rsp_data, 3);
    return 0;
}


static void deal_sibling_tws_ota_trans(void *data, u16 len, bool rx)
{
    static u8 last_sync_update_sn = 0;
    int ret = 0;
    u8 rsp_data[3];
    u8 *recv_data = (u8 *)data;

    if (rx) {
        if (recv_data[1] == last_sync_update_sn) {        //处理同一包数据会出现两次回调的情况
            g_printf("recv_data == last_sync_update_sn:%d %d %d\n", recv_data[0], recv_data[1], last_sync_update_sn);
            return;
        }

        last_sync_update_sn = recv_data[1];
        /* log_info(">>>%s\n len:%d", __func__, len); */
        /* put_buf(data, len); */
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            tws_ota_timeout_reset();                //从机接收命令超时
        }
        switch (recv_data[0]) {
        case OTA_TWS_START_UPDATE:
            g_printf("MSG_OTA_TWS_START_UPDATE\n");
            struct __tws_ota_para para;
            memcpy(&para, recv_data + 2, sizeof(struct __tws_ota_para));
            printf("crc:%d fm_size:%d max_pkt_len:%d\n", para.fm_crc, para.fm_size, para.max_pkt_len);
            dual_bank_passive_update_init(para.fm_crc, para.fm_size, para.max_pkt_len, NULL);
            ret = dual_bank_update_allow_check(para.fm_size);
            if (ret) {
                rsp_data[0] = OTA_TWS_START_UPDATE_RSP;
                rsp_data[1] = ++ sync_update_sn;
                rsp_data[2] = OTA_TWS_CMD_ERR;
                tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_ERR);
            } else {
                rsp_data[0] = OTA_TWS_START_UPDATE_RSP;
                rsp_data[1] = ++ sync_update_sn;
                rsp_data[2] = OTA_TWS_CMD_SUCC;
            }
            tws_ota_trans_to_sibling(rsp_data, 3);
            break;
        case OTA_TWS_START_UPDATE_RSP:
            g_printf("MSG_OTA_TWS_START_UPDATE_RSP\n");
            if (recv_data[2] == OTA_TWS_CMD_SUCC) {
                os_sem_post(&tws_ota_sem);
            }
            break;
        case OTA_TWS_TRANS_UPDATE_DATA:
            printf("MSG_OTA_TWS_TRANS_UPDATE_DATA %d\n", recv_data[1]);
            dual_bank_update_write(recv_data + 2, len - 2, tws_trans_data_write_callback);
            break;
        case OTA_TWS_TRANS_UPDATE_DATA_RSP:
            printf("MSG_OTA_TWS_TRANS_UPDATE_DATA_RSP:%d\n", recv_data[1]);
            if (recv_data[2] == OTA_TWS_CMD_SUCC) {
                os_sem_post(&tws_ota_sem);
            }
            break;
        case OTA_TWS_VERIFY:
            g_printf("MSG_OTA_TWS_VERIFY\n");
            clk_set("sys", 120 * 1000000L);                                      //提升主频加快CRC校验速度
            ret =  dual_bank_update_verify(sync_update_crc_init_hdl, sync_update_crc_calc_hdl, tws_verify_result_hdl);
            if (ret) {
                rsp_data[0] = OTA_TWS_VERIFY_RSP;
                rsp_data[1] = ++ sync_update_sn;
                rsp_data[2] = OTA_TWS_CMD_ERR;
                tws_ota_trans_to_sibling(rsp_data, 2);
            }
            break;
        case OTA_TWS_VERIFY_RSP:
            g_printf("MSG_OTA_TWS_VERIFY_RSP %d\n", recv_data[1]);
            if (recv_data[2] == OTA_TWS_CMD_SUCC) {
                os_sem_post(&tws_ota_sem);
            }
            break;
        case OTA_TWS_WRITE_BOOT_INFO:
            g_printf("MSG_OTA_TWS_WRITE_BOOT_INFO\n");
            dual_bank_update_burn_boot_info(dual_bank_update_burn_boot_info_callback);
            break;
        case OTA_TWS_WRITE_BOOT_INFO_RSP:
            g_printf("MSG_OTA_TWS_WRITE_BOOT_INFO_RSP\n");
            if (recv_data[2] == OTA_TWS_CMD_SUCC) {
                os_sem_post(&tws_ota_sem);
            }
            break;
        case OTA_TWS_UPDATE_RESULT:
            g_printf("MSG_OTA_TWS_UPDATE_RESULT\n");
            if (recv_data[2] == OTA_TWS_CMD_SUCC) {
                tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_OVER);
            } else {
                tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_ERR);
            }
            break;

        case OTA_TWS_USER_CHIP_UPDATE:
            if (user_chip_tws_update_handle) {
                user_chip_tws_update_handle(recv_data + 2, len - 2);
            }
            break;
        }
    }
}


int tws_ota_sync_cmd(int reason)
{
    switch (reason) {
    case SYNC_CMD_UPDATE_OVER:
        g_printf("SYNC_CMD_UPDATE_OVER\n");
        cpu_reset();
        /* tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_OVER); */
        break;
    case SYNC_CMD_UPDATE_ERR:
        g_printf("SYNC_CMD_UPDATE_ERR\n");
        cpu_reset();
        /* tws_ota_event_post(SYS_BT_OTA_EVENT_TYPE_STATUS, OTA_UPDATE_ERR); */
        break;
    }
    return 0;
}

static void rcsp_adv_tws_ota_sync_handler(int reason, int err)
{
    tws_ota_sync_cmd(reason);
}

const update_op_tws_api_t update_tws_api = {
    .tws_ota_start = tws_ota_open,
    .tws_ota_data_send = tws_ota_data_send_m_to_s,
    .tws_ota_err = tws_ota_err_callback,
    .enter_verfiy_hdl = tws_ota_enter_verify,
    .exit_verify_hdl = tws_ota_exit_verify,
    .update_boot_info_hdl =  tws_ota_updata_boot_info_over,
    .tws_ota_result_hdl = tws_ota_result,
    .tws_ota_data_send_pend = tws_ota_data_send_pend,
    //for user_chip
    .tws_ota_user_chip_update_send = tws_ota_user_chip_update_send_m_to_s,
};

update_op_tws_api_t *get_tws_update_api(void)
{
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
        return &update_tws_api;
    } else {
        return NULL;
    }
}

REGISTER_TWS_FUNC_STUB(tws_ota_trans) = {
    .func_id = TWS_FUNC_ID_OTA_TRANS,
    .func    = deal_sibling_tws_ota_trans,
};

TWS_SYNC_CALL_REGISTER(rcsp_adv_tws_ota_sync) = {
    .uuid = 0xA2E22223,
    .task_name = "app_core",
    .func = rcsp_adv_tws_ota_sync_handler,
};

void tws_update_register_user_chip_update_handle(void (*update_handle)(void *data, u32 len))
{
    user_chip_tws_update_handle = update_handle;
}

#endif

