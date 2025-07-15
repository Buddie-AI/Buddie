#include "app_config.h"
#include "rcsp_adv_user_update.h"
#if RCSP_UPDATE_EN
#include "uart.h"
#include "system/timer.h"
#include "update.h"
#include "custom_cfg.h"
#include "btstack/avctp_user.h"
#include "JL_rcsp_packet.h"
#include "JL_rcsp_protocol.h"
#include "rcsp_adv_bluetooth.h"
#include "le_rcsp_adv_module.h"
#include "update_loader_download.h"
#include "classic/tws_api.h"
#include "syscfg_id.h"

#if (RCSP_ADV_EN)

#define RCSP_DEBUG_EN

#ifdef  RCSP_DEBUG_EN
#define rcsp_putchar(x)                putchar(x)
#define rcsp_printf                    printf
#define rcsp_printf_buf(x,len)         printf_buf(x,len)
#else
#define rcsp_putchar(...)
#define rcsp_printf(...)
#define rcsp_printf_buf(...)
#endif

#define DEV_UPDATE_FILE_INFO_OFFEST  0x00//0x40
#define DEV_UPDATE_FILE_INFO_LEN     0x00//(0x10 + VER_INFO_EXT_COUNT * (VER_INFO_EXT_MAX_LEN + 1))

typedef enum {
    UPDATA_START = 0x00,
    UPDATA_REV_DATA,
    UPDATA_STOP,
} UPDATA_BIT_FLAG;

#if 0
static update_file_id_t update_file_id_info = {
    .ver[0] = 0xff,
    .ver[1] = 0xff,
};
#else
static update_file_ext_id_t update_file_id_info = {
    .update_file_id_info.ver[0] = 0xff,
    .update_file_id_info.ver[1] = 0xff,
};
#endif

extern const int support_dual_bank_update_en;
extern void JL_ble_disconnect(void);
extern u8 check_le_pakcet_sent_finish_flag(void);

static u8 update_flag = 0;
static u8 tws_need_update = 0;             //标志耳机是否需要强制升级
static u8 tws_need_role_switch = 0;	 	   //用于tws连接之后版本号不匹配进行role_switch，低版本号的作为主机进行强制升级
static u8 disconnect_flag = 0;
u8 get_jl_update_flag(void)
{
    printf("get_update_flag:%x\n", update_flag);
    return update_flag;
}

void set_jl_update_flag(u8 flag)
{
    update_flag = flag;
    printf("update_flag:%x\n", update_flag);
}

typedef struct _update_mode_t {
    u8 opcode;
    u8 opcode_sn;

} update_mode_t;

static update_mode_t update_record_info;

void JL_controller_save_curr_cmd_para(u8 OpCode, u8 OpCode_SN)
{
    update_record_info.opcode = OpCode;
    update_record_info.opcode_sn = OpCode_SN;
}

void JL_controller_get_curr_cmd_para(u8 *OpCode, u8 *OpCode_SN)
{
    *OpCode = update_record_info.opcode;
    *OpCode_SN = update_record_info.opcode_sn;
}

static void (*fw_update_block_handle)(u8 state, u8 *buf, u16 len) = NULL;
void register_receive_fw_update_block_handle(void (*handle)(u8 state, u8 *buf, u16 len))
{
    fw_update_block_handle = handle;
}

static u16  ble_discon_timeout;
static void ble_discon_timeout_handle(void *priv)
{
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_START, NULL, 0);
}

void JL_rcsp_update_cmd_resp(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 msg[4];
    rcsp_printf("%s\n", __FUNCTION__);
    switch (OpCode) {
    case JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET:
        if (0 == len) {
            msg[0] = OpCode;
            msg[1] = OpCode_SN;
            rcsp_printf("JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET\n");
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP,
                                  MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET,
                                  msg,
                                  2);
        } else {
            rcsp_printf("JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET ERR\n");
        }
        break;
    case JL_OPCODE_INQUIRE_DEVICE_IF_CAN_UPDATE:
        rcsp_printf("JL_OPCODE_INQUIRE_DEVICE_IF_CAN_UPDATE:%x %x\n", len, data[0]);
        if (len) {
            extern void set_curr_update_type(u8 type);
            set_curr_update_type(data[0]);
            msg[0] = OpCode;
            msg[1] = OpCode_SN;
            msg[2] = 0x00;
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP,
                                  MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE,
                                  msg,
                                  3);
        }
        break;
    case JL_OPCODE_EXIT_UPDATE_MODE:
        rcsp_printf("JL_OPCODE_EXIT_UPDATE_MODE\n");
        break;
    case JL_OPCODE_ENTER_UPDATE_MODE:
        rcsp_printf("JL_OPCODE_ENTER_UPDATE_MODE\n");
        msg[0] = OpCode;
        msg[1] = OpCode_SN;
        JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP,
                              MSG_JL_ENTER_UPDATE_MODE,
                              msg,
                              2);
        break;
    case JL_OPCODE_SEND_FW_UPDATE_BLOCK:
        rcsp_printf("JL_OPCODE_SEND_FW_UPDATE_BLOCK\n");
        break;
    case JL_OPCODE_GET_DEVICE_REFRESH_FW_STATUS:
        rcsp_printf("JL_OPCODE_GET_DEVICE_REFRESH_FW_STATUS\n");
        JL_controller_save_curr_cmd_para(OpCode, OpCode_SN);
        if (fw_update_block_handle) {
            fw_update_block_handle(UPDATA_STOP, NULL, 0);
        }
        break;
    /* case JL_OPCODE_SET_DEVICE_REBOOT: */
    /*     rcsp_printf("JL_OPCODE_SET_DEVICE_REBOOT\n"); */
    /* 	if (support_dual_bank_update_en) { */
    /* 		cpu_reset();	 */
    /* 	} */
    /*     break; */
    default:
        break;
    }
}

static void JL_rcsp_resp_dev_update_file_info_offest(u8 OpCode, u8 OpCode_SN)
{
    u8 data[4 + 2];
    u16 update_file_info_offset = DEV_UPDATE_FILE_INFO_OFFEST;
    u16 update_file_info_len = DEV_UPDATE_FILE_INFO_LEN;
    WRITE_BIG_U32(data + 0, update_file_info_offset);
    WRITE_BIG_U16(data + 4, update_file_info_len);
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, sizeof(data));
}

static void JL_resp_inquire_device_if_can_update(u8 OpCode, u8 OpCode_SN, u8 update_sta)
{
    u8 data[1];
    data[0] = update_sta;
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, sizeof(data));
}

enum {
    UPDATE_FLAG_OK,
    UPDATE_FLAG_LOW_POWER,
    UPDATE_FLAG_FW_INFO_ERR,
    UPDATE_FLAG_FW_INFO_CONSISTENT,
    UPDATE_FLAG_TWS_DISCONNECT,
};

static u8 judge_remote_version_can_update(void)
{
    //extern u16 ex_cfg_get_local_version_info(void);
    u16 remote_file_ver = READ_BIG_U16(update_file_id_info.update_file_id_info.ver);
    //u16 local_ver = ex_cfg_get_local_version_info();
    u16 local_ver = get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);

#if (0 == VER_INFO_EXT_COUNT)
    //extern u16 ex_cfg_get_local_pid_info(void);
    //extern u16 ex_cfg_get_local_vid_info(void);

    //u16 local_pid = ex_cfg_get_local_pid_info();
    //u16 local_vid = ex_cfg_get_local_vid_info();
    u16 local_pid = get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG);
    u16 local_vid = get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);

    u16 remote_file_pid = READ_BIG_U16(update_file_id_info.update_file_id_info.pid);
    u16 remote_file_vid = READ_BIG_U16(update_file_id_info.update_file_id_info.vid);

    if (remote_file_ver > local_ver || remote_file_pid != local_pid || remote_file_vid != local_vid) {
        return UPDATE_FLAG_FW_INFO_ERR;
    }
#else
    //extern u32 ex_cfg_get_local_authkey_info(u8 * authkey_data[], u8 * authkey_len);
    //extern u32 ex_cfg_get_local_procode_info(u8 * procode_data[], u8 * procode_len);

    u8 authkey_len = 0;
    u8 *local_authkey_data = NULL;
    get_authkey_procode_from_cfg_file(&local_authkey_data, &authkey_len, GET_AUTH_KEY_FROM_EX_CFG);

    u8 procode_len = 0;
    u8 *local_procode_data = NULL;
    get_authkey_procode_from_cfg_file(&local_procode_data, &procode_len, GET_PRO_CODE_FROM_EX_CFG);

    //ex_cfg_get_local_authkey_info(&local_authkey_data, &authkey_len);
    //ex_cfg_get_local_procode_info(&local_procode_data, &procode_len);

    u8 *remote_authkey_data = update_file_id_info.ext;
    u8 *remote_procode_data = update_file_id_info.ext + authkey_len + 1;

    if (remote_file_ver < local_ver
        || 0 != memcmp(remote_authkey_data, local_authkey_data, authkey_len)
        || 0 != memcmp(remote_procode_data, local_procode_data, procode_len)) {
        return UPDATE_FLAG_FW_INFO_ERR;
    }
#endif

    if (remote_file_ver == local_ver) {
        rcsp_printf("remote_file_ver is %x, local_ver is %x, remote_file_ver is similar to local_ver\n", remote_file_ver, local_ver);
        return UPDATE_FLAG_FW_INFO_CONSISTENT;
    }

    return UPDATE_FLAG_OK;
}

static bool check_edr_is_disconnct(void)
{
    if (get_curr_channel_state()) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static bool check_ble_all_packet_sent(void)
{
    if (check_le_pakcet_sent_finish_flag()) {
        return TRUE;
    } else {
        return FALSE;
    }
}


static u32 rcsp_update_data_read(void *priv, u32 offset_addr, u16 len)
{
    u32 err;
    u8 data[4 + 2];
    WRITE_BIG_U32(data, offset_addr);
    WRITE_BIG_U16(data + 4, len);
    err = JL_CMD_send(JL_OPCODE_SEND_FW_UPDATE_BLOCK, data, sizeof(data), JL_NEED_RESPOND);

    return err;
}

static JL_ERR JL_controller_resp_get_dev_refresh_fw_status(u8 OpCode, u8 OpCode_SN, u8 result)
{
    JL_ERR send_err = JL_ERR_NONE;
    u8 data[1];

    data[0] = result;	//0:sucess 1:fail;

    send_err = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, sizeof(data));

    return send_err;
}

u32 rcsp_update_status_response(void *priv, u8 status)
{
    u8 OpCode;
    u8 OpCode_SN;

    JL_ERR send_err = JL_ERR_NONE;

    JL_controller_get_curr_cmd_para(&OpCode, &OpCode_SN);

    //log_info("get cmd para:%x %x\n", OpCode, OpCode_SN);

    if (JL_OPCODE_GET_DEVICE_REFRESH_FW_STATUS == OpCode) {
        send_err = JL_controller_resp_get_dev_refresh_fw_status(OpCode, OpCode_SN, status);
    }

    return send_err;
}



void JL_rcsp_update_cmd_receive_resp(void *priv, u8 OpCode, u8 status, u8 *data, u16 len)
{
    switch (OpCode) {
    case JL_OPCODE_SEND_FW_UPDATE_BLOCK:
        if (fw_update_block_handle) {
            fw_update_block_handle(UPDATA_REV_DATA, data, len);
        }
        break;
    default:
        break;
    }
}

static void rcsp_loader_download_result_handle(void *priv, u8 type, u8 cmd)
{
    if (UPDATE_LOADER_OK == cmd) {
        //JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP,MSG_JL_UPDATE_START,NULL,0);
        set_jl_update_flag(1);
        if (support_dual_bank_update_en) {
            rcsp_printf(">>>rcsp update succ\n");
            update_result_set(UPDATA_SUCC);
        }
    } else {
        rcsp_printf(">>>update loader err\n");
        /* #if OTA_TWS_SAME_TIME_ENABLE */
        /*         if((tws_ota_control(OTA_TYPE_GET) == OTA_TWS)) { */
        /*             tws_ota_stop(OTA_STOP_UPDATE_OVER_ERR); */
        /*         } */
        /* #endif */
        //rcsp_db_update_fail_deal();
    }
}

extern void register_receive_fw_update_block_handle(void (*handle)(u8 state, u8 *buf, u16 len));
extern void rcsp_update_data_api_register(u32(*data_send_hdl)(void *priv, u32 offset, u16 len), u32(*send_update_handl)(void *priv, u8 state));
extern void rcsp_update_handle(void *buf, int len);
extern void bt_set_low_latency_mode(int enable);
extern void bt_adv_seq_change(void);
extern int bt_tws_poweroff();
extern void bt_wait_phone_connect_control(u8 enable);
extern int tws_api_get_role(void);
extern void tws_cancle_all_noconn();
extern void bt_ble_rcsp_adv_disable(void);
extern void ble_module_enable(u8 en);
extern void bt_ble_rcsp_adv_enable(void);
extern u32 ex_cfg_fill_content_api(void);
extern void update_param_priv_fill(UPDATA_PARM *p, void *priv, u16 priv_len);

u8 rcsp_get_update_flag(void)
{
    return tws_need_update;
}

void rcsp_set_update_flag(u8 flag)
{
    tws_need_update = flag;
}

u8 rcsp_update_get_role_switch(void)
{
    return tws_need_role_switch;
}

void rcsp_update_set_role_switch(u8 sw)
{
    tws_need_role_switch = sw;
}

void update_slave_adv_reopen(void)
{
    r_printf("slave reopen adv...\n");
    ble_module_enable(1);
    bt_ble_rcsp_adv_enable();
}

static void rcsp_update_private_param_fill(UPDATA_PARM *p)
{
    u32 exif_addr = ex_cfg_fill_content_api();
    update_param_priv_fill(p, (void *)&exif_addr, sizeof(exif_addr));
}

static void rcsp_update_before_jump_handle(int type)
{
#if CONFIG_UPDATE_JUMP_TO_MASK
    y_printf(">>>[test]:latch reset update\n");
    latch_reset();
#if 0
    update_close_hw("null");
    ram_protect_close();
    /* save_spi_port(); */
    extern void __BT_UPDATA_JUMP();
    y_printf("update jump to __BT_UPDATA ...\n");
    /* clk_set("sys", 48 * 1000000L); */
    //跳转到uboot加载完,30ms左右(200410_yzb)
    __BT_UPDATA_JUMP();
#endif
#else
    cpu_reset();
#endif
}

void JL_rcsp_msg_deal(void *hdl, u8 event, u8 *msg)
{
    u16 remote_file_version;
    u8 can_update_flag = UPDATE_FLAG_FW_INFO_ERR;

    switch (event) {
    case MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET:
        rcsp_printf("MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET\n");

        /* extern void linein_mutex_stop(void *priv); */
        /* linein_mutex_stop(NULL); */
        /* extern void linein_mute(u8 mute); */
        /* linein_mute(1); */

        JL_rcsp_resp_dev_update_file_info_offest((u8)msg[0], (u8)msg[1]);
        break;

    case MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE:
        rcsp_printf("MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE\n");
#if 0
        remote_file_version = READ_BIG_U16(update_file_id_info.update_file_id_info.ver);
        rcsp_printf("remote_file_ver:V%d.%d.%d.%d\n",
                    (remote_file_version & 0xf000) >> 12,
                    (remote_file_version & 0x0f00) >> 8,
                    (remote_file_version & 0x00f0) >> 4,
                    (remote_file_version & 0x000f));

        if (0 == remote_file_version) {
            can_update_flag = UPDATE_FLAG_OK;
        } else {
            can_update_flag = judge_remote_version_can_update();
        }

        if (UPDATE_FLAG_OK == can_update_flag) {
            set_jl_update_flag(1);
        }
#else
#if OTA_TWS_SAME_TIME_ENABLE
        int get_bt_tws_connect_status();
        if (get_bt_tws_connect_status() || (!support_dual_bank_update_en)) {    //单备份返回成功
            can_update_flag = UPDATE_FLAG_OK;
        } else {
            can_update_flag = UPDATE_FLAG_TWS_DISCONNECT;
        }
#else
        can_update_flag = UPDATE_FLAG_OK;
#endif      //endif OTA_TWS_SAME_TIME_ENABLE
#endif
        //todo;judge voltage
        JL_resp_inquire_device_if_can_update((u8)msg[0], (u8)msg[1], can_update_flag);

        if (0 == support_dual_bank_update_en) {
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                g_printf("tws master start update...\n");
                JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_LOADER_DOWNLOAD_START, NULL, 0);
                //需要通知从机进入了升级
                u8 data = TWS_UPDATE_INFO;
                tws_api_send_data_to_sibling(&data, sizeof(data), TWS_FUNC_ID_SEQ_RAND_SYNC);
            } else {
                bt_ble_rcsp_adv_disable();
                ble_module_enable(0);               //关闭广播防止从机被手机误回连
                r_printf("slave close adv...\n");
                sys_timeout_add(NULL,  update_slave_adv_reopen, 1000 * 60);     //延迟一分钟再开广播
            }
            if (RCSP_SPP == JL_get_cur_bt_channel_sel()) {
#if TCFG_USER_TWS_ENABLE
                tws_api_detach(TWS_DETACH_BY_LOCAL); //单备份升级断开tws
                tws_cancle_all_noconn();
#endif
            }
        }

        break;

    case MSG_JL_DEV_DISCONNECT:
        if (check_ble_all_packet_sent()) {
            rcsp_printf("MSG_JL_DEV_DISCONNECT\n");
            JL_ble_disconnect();
            if (check_edr_is_disconnct()) {
                puts("-need discon edr\n");
                user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
            }
            ble_discon_timeout = sys_timeout_add(NULL, ble_discon_timeout_handle, 1000);
        } else {
            rcsp_printf("W");
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_DEV_DISCONNECT, NULL, 0);
        }
        break;

    case MSG_JL_LOADER_DOWNLOAD_START:
        rcsp_update_data_api_register(rcsp_update_data_read, rcsp_update_status_response);
        register_receive_fw_update_block_handle(rcsp_update_handle);
        if (RCSP_BLE == get_curr_device_type()) {
            rcsp_update_loader_download_init(BLE_APP_UPDATA, rcsp_loader_download_result_handle);
        } else if (RCSP_SPP == get_curr_device_type()) {
            rcsp_update_loader_download_init(SPP_APP_UPDATA, rcsp_loader_download_result_handle);
        }
        break;

    case MSG_JL_UPDATE_START:
        if (check_edr_is_disconnct()) {
            rcsp_printf("b");
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_START, NULL, 0);
            break;
        }

        if (RCSP_BLE == get_curr_device_type()) {
            rcsp_printf("BLE_APP_UPDATE\n");
            update_mode_api_v2(BLE_APP_UPDATA,
                               rcsp_update_private_param_fill,
                               rcsp_update_before_jump_handle);
        } else {
            rcsp_printf("SPP_APP_UPDATA\n");
            update_mode_api_v2(SPP_APP_UPDATA,
                               rcsp_update_private_param_fill,
                               rcsp_update_before_jump_handle);
        }
        break;

    case MSG_JL_ENTER_UPDATE_MODE:
        rcsp_printf("MSG_JL_ENTER_UPDATE_MODE:%x %x\n", msg[0], msg[1]);
        bt_set_low_latency_mode(0);
        if (support_dual_bank_update_en && !tws_api_get_role()) {
            u8 status = 0;
            JL_CMD_response_send(msg[0], JL_PRO_STATUS_SUCCESS, msg[1], &status, 1);
            rcsp_update_data_api_register(rcsp_update_data_read, rcsp_update_status_response);
            register_receive_fw_update_block_handle(rcsp_update_handle);
            rcsp_update_loader_download_init(DUAL_BANK_UPDATA, rcsp_loader_download_result_handle);
        }
        break;

    default:
        break;
    }
}
#endif

#else
//以下函数是为了编译通过
void update_slave_adv_reopen(void)
{
    printf("%s:NULL", __func__);
}
void rcsp_set_update_flag(u8 flag)
{
    printf("%s:NULL", __func__);
}
void rcsp_update_set_role_switch(u8 sw)
{
    printf("%s:NULL", __func__);
}
u8 get_jl_update_flag(void)
{
    printf("%s:NULL", __func__);
    return 0;
}
u8 rcsp_get_update_flag(void)
{
    printf("%s:NULL", __func__);
    return 0;
}
#endif

