/**@file  		lp_touch_key_tool.c
* @brief    	低功耗内置触摸调试工具
* @date     	2021-8-26
* @version  	V1.0
* @copyright    Copyright:(c)JIELI  2011-2020  @ , All Rights Reserved.
 */
#include "includes.h"
#include "asm/lp_touch_key_tool.h"
#include "asm/lp_touch_key_api.h"
#include "btstack/avctp_user.h"
#include "classic/tws_api.h"
#include "app_config.h"

#define LOG_TAG_CONST       LP_KEY
#define LOG_TAG             "[LP_KEY]"
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#define __this 		(&_ctmu_key)



/**@defgroup lp_touch_key_tool_tws
* @{
 */
#define TWS_FUNC_ID_VOL_LP_KEY      TWS_FUNC_ID('L', 'K', 'E', 'Y')


static int tws_api_send_data(void *data, int len, u32 func_id)
{
    int ret = -EINVAL;

    local_irq_disable();
    ret = tws_api_send_data_to_sibling(data, len, func_id);
    local_irq_enable();

    return ret;
}

void lpctmu_tws_send_event_data(int msg, int type)
{
    u32 event_data[4];
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
        event_data[0] = type;
        event_data[1] = jiffies_to_msecs(jiffies);
        event_data[2] = msg;
        tws_api_send_data(event_data, 3 * sizeof(int), TWS_FUNC_ID_VOL_LP_KEY);
    }
}

void lpctmu_tws_send_res_data(int type, int data1, int data2, int data3, int data4, int data5)
{
    u32 event_data[6];
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
        event_data[0] = type;
        event_data[1] = data1;
        event_data[2] = data2;
        event_data[3] = data3;
        event_data[4] = data4;
        event_data[5] = data5;
        tws_api_send_data(event_data, 6 * sizeof(int), TWS_FUNC_ID_VOL_LP_KEY);
    }
}

static void res_receive_handle(void *_data, u16 len, bool rx)
{
    u32 *data = _data;
    if (rx) {
        switch (data[0]) {
        case BT_KEY_CH_RES_MSG:
            log_debug("ch0:%08d,  ch1:%08d,  ch2:%08d,  ch3:%08d,  ch4:%08d\n",
                      data[1],
                      data[2],
                      data[3],
                      data[4],
                      data[5]);
            break;
        case BT_EARTCH_RES_MSG:
            log_debug("ear_ch:%08d,  ref_ch:%08d,  iir:%08d,  trim:%08d,  diff:%08d\n",
                      data[1],
                      data[2],
                      data[3],
                      data[4],
                      data[5]);
            break;
        case BT_EVENT_SW_MSG:
            log_debug("len = %d, %d ms", len, data[1]);
            if (data[2] == EPD_IN) {
                log_debug("SW: IN");
            } else if (data[2] == EPD_OUT) {
                log_debug("SW: OUT");
            }
            break;
        case BT_EVENT_HW_MSG:
            log_debug("len = %d, %d ms", len, data[1]);
            if (data[2] == EAR_IN) {
                log_debug("HW: IN");
            } else if (data[2] == EAR_OUT) {
                log_debug("HW: OUT");
            }
            break;
        case BT_EVENT_VDDIO:
            log_debug("BT_EVENT_VDDIO: %d", data[2]);
            break;
        }
    }
}

REGISTER_TWS_FUNC_STUB(lp_touch_res_sync_stub) = {
    .func_id = TWS_FUNC_ID_VOL_LP_KEY,
    .func    = res_receive_handle, //handle
};
/** @} lp_touch_key_tool_tws*/



/**@defgroup lp_touch_key_tool_spp
* @{
 */
#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
#include "online_db_deal.h"

//LP KEY在线调试工具版本号管理
const u8 lp_key_sdk_name[16] = "AC7016N";
const u8 lp_key_bt_ver[4] 	     = {0, 0, 1, 0};
struct lp_key_ver_info {
    char sdkname[16];
    u8 lp_key_ver[4];
};

//   通道号           按键事件个数
const char ch_content[] = {
    0x01,                                       //版本号
    'c', 'h', '0', '\0', 6, 0, 1, 2, 3, 4, 5,
    'c', 'h', '1', '\0', 6, 0, 1, 2, 3, 4, 5,
    'c', 'h', '2', '\0', 6, 0, 1, 2, 3, 4, 5,
    'c', 'h', '3', '\0', 6, 0, 1, 2, 3, 4, 5,
    'c', 'h', '4', '\0', 6, 0, 1, 2, 3, 4, 5
};

enum {
    TOUCH_RECORD_GET_VERSION = 0x05,
    TOUCH_RECORD_GET_CH_SIZE = 0x0B,
    TOUCH_RECORD_GET_CH_CONTENT = 0x0C,
    TOUCH_RECORD_CHANGE_MODE = 0x0E,
    TOUCH_RECORD_COUNT = 0x200,
    TOUCH_RECORD_START,
    TOUCH_RECORD_STOP,
    ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH,
    TOUCH_CH_CFG_UPDATE  = 0x3000,
    TOUCH_CH_VOL_CFG_UPDATE  = 0x3001,
    TOUCH_CH_CFG_CONFIRM = 0x3100,
};

enum {
    LP_KEY_ONLINE_ST_INIT = 0,
    LP_KEY_ONLINE_ST_READY,
    LP_KEY_ONLINE_ST_CH_RES_DEBUG_START,
    LP_KEY_ONLINE_ST_CH_RES_DEBUG_STOP,
    LP_KEY_ONLINE_ST_CH_KEY_DEBUG_START,
    LP_KEY_ONLINE_ST_CH_KEY_DEBUG_CONFIRM,
};

//小机接收命令包格式, 使用DB_PKT_TYPE_TOUCH通道接收
typedef struct {
    int cmd_id;
    int mode;
    int data[];
} touch_receive_cmd_t;

//小机发送按键消息格式, 使用DB_PKT_TYPE_TOUCH通道发送
typedef struct {
    u32 cmd_id;
    u32 mode;
    u32 key_event;
} lp_touch_online_key_event_t;

typedef struct {
    u8 state;
    u8 current_record_ch;
    u16 res_packet;
    struct ch_adjust_table debug_cfg;
    lp_touch_online_key_event_t online_key_event;
    struct ch_ana_cfg ch_ana;
} lp_touch_key_online;

static lp_touch_key_online lp_key_online = {0};

int lp_touch_key_online_debug_key_event_handle(u8 ch_index, struct sys_event *event)
{
    int err = 0;
    if ((lp_key_online.state == LP_KEY_ONLINE_ST_CH_KEY_DEBUG_START) && (lp_key_online.current_record_ch == ch_index)) {
        lp_key_online.online_key_event.cmd_id = 0x3100;
        lp_key_online.online_key_event.mode = 0;
        lp_key_online.online_key_event.key_event = event->u.key.event;
        log_debug("send %d event to PC", lp_key_online.online_key_event.key_event);
        err = app_online_db_send(DB_PKT_TYPE_TOUCH, (u8 *)(&(lp_key_online.online_key_event)), sizeof(lp_touch_online_key_event_t));
    }

    if ((lp_key_online.state == LP_KEY_ONLINE_ST_CH_KEY_DEBUG_CONFIRM) ||
        (lp_key_online.state <= LP_KEY_ONLINE_ST_READY)) {
        return 0;
    }

    return 1;
}

static int lp_touch_key_debug_reinit(u8 update_state)
{
    log_debug("%s, current_record_ch = %d", __func__, lp_key_online.current_record_ch);

    lp_touch_key_disable();

    u8 ch = lp_key_online.current_record_ch;
    switch (update_state) {
    case LP_KEY_ONLINE_ST_CH_RES_DEBUG_START:
        M2P_CTMU_CH_DEBUG &= ~0b11111;
        M2P_CTMU_CH_DEBUG |= BIT(ch);
        if (lp_key_online.ch_ana.isel) {
            LPCTMU_ANA0_CONFIG((lp_key_online.ch_ana.vhsel << 6) | (lp_key_online.ch_ana.vlsel << 4) | (lp_key_online.ch_ana.isel << 1));
        } else {
            LPCTMU_ANA0_CONFIG(get_lpctmu_ana_level());
        }
        break;
    case LP_KEY_ONLINE_ST_CH_RES_DEBUG_STOP:
        M2P_CTMU_CH_DEBUG &= ~0b11111;
        break;
    case LP_KEY_ONLINE_ST_CH_KEY_DEBUG_START:
        M2P_CTMU_CH_DEBUG &= ~0b11111;
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 8) = ((lp_key_online.debug_cfg.cfg0) & 0xFF);
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + ch * 8) = (((lp_key_online.debug_cfg.cfg0) >> 8) & 0xFF);
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1L + ch * 8) = ((lp_key_online.debug_cfg.cfg1) & 0xFF);
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1H + ch * 8) = (((lp_key_online.debug_cfg.cfg1) >> 8) & 0xFF);
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 8) = ((lp_key_online.debug_cfg.cfg2) & 0xFF);
        M2P_MESSAGE_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 8) = (((lp_key_online.debug_cfg.cfg2) >> 8) & 0xFF);
        break;
    case LP_KEY_ONLINE_ST_CH_KEY_DEBUG_CONFIRM:
        M2P_CTMU_CH_DEBUG &= ~0b11111;
        break;
    default:
        break;
    }

    if (__this->short_timer[ch] != 0xFFFF) {
        usr_timer_del(__this->short_timer[ch]);
        __this->short_timer[ch] = 0xFFFF;
    }
    __this->last_key[ch] = CTMU_KEY_NULL;
    P2M_CTMU_WKUP_MSG &= (~(P2M_MESSAGE_INIT_MODE_FLAG)); //普通模式初始化

    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;
    //CTMU 初始化命令
    lp_touch_key_send_cmd(CTMU_M2P_INIT);

    return 0;
}

static int lp_touch_key_online_debug_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    int res_data = 0;
    touch_receive_cmd_t *touch_cmd;
    int err = 0;
    u8 parse_seq = ext_data[1];
    struct ch_adjust_table *receive_cfg;
    struct ch_ana_cfg *ana_cfg;
    struct lp_key_ver_info ver_info = {0};

    static u32 _offset = 0;
    static u32 _size = 0;

    res_data = 4;
    log_debug("%s", __func__);
    put_buf(packet, size);
    put_buf(ext_data, ext_size);
    touch_cmd = (touch_receive_cmd_t *)packet;
    switch (touch_cmd->cmd_id) {
    case TOUCH_RECORD_START:
        log_debug("TOUCH_RECORD_START");
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 1); //该命令随便ack一个byte即可
        lp_key_online.state = LP_KEY_ONLINE_ST_CH_RES_DEBUG_START;
        lp_touch_key_debug_reinit(lp_key_online.state);
        break;
    case TOUCH_RECORD_STOP:
        log_debug("TOUCH_RECORD_STOP");
        app_online_db_ack(parse_seq, (u8 *)&res_data, 1); //该命令随便ack一个byte即可
        lp_key_online.state = LP_KEY_ONLINE_ST_CH_RES_DEBUG_STOP;
        lp_touch_key_debug_reinit(lp_key_online.state);
        break;
    case TOUCH_CH_CFG_UPDATE:
        log_debug("TOUCH_CH_CFG_UPDATE");
        app_online_db_ack(parse_seq, (u8 *)"OK", 2); //回"OK"字符串
        lp_key_online.state = LP_KEY_ONLINE_ST_CH_KEY_DEBUG_START;

        receive_cfg = (struct ch_adjust_table *)(touch_cmd->data);
        lp_key_online.debug_cfg.cfg0 = receive_cfg->cfg0;
        lp_key_online.debug_cfg.cfg1 = receive_cfg->cfg1;
        lp_key_online.debug_cfg.cfg2 = receive_cfg->cfg2;
        log_debug("update, cfg0 = %d, cfg1 = %d, cfg2 = %d", lp_key_online.debug_cfg.cfg0, lp_key_online.debug_cfg.cfg1, lp_key_online.debug_cfg.cfg2);
        lp_touch_key_debug_reinit(lp_key_online.state);
        break;
    case TOUCH_CH_VOL_CFG_UPDATE:
        log_debug("TOUCH_CH_VOL_CFG_UPDATE");
        app_online_db_ack(parse_seq, (u8 *)"OK", 2); //回"OK"字符串
        ana_cfg = (struct ch_ana_cfg *)(touch_cmd->data);
        lp_key_online.ch_ana.isel = ana_cfg->isel;
        lp_key_online.ch_ana.vhsel = ana_cfg->vhsel;
        lp_key_online.ch_ana.vlsel = ana_cfg->vlsel;
        log_debug("update, isel = %d, vhsel = %d, vlsel = %d", lp_key_online.ch_ana.isel, lp_key_online.ch_ana.vhsel, lp_key_online.ch_ana.vlsel);
        break;
    case TOUCH_CH_CFG_CONFIRM:
        log_debug("TOUCH_CH_CFG_CONFIRM");
        app_online_db_ack(parse_seq, (u8 *)"OK", 2); //回"OK"字符串
        lp_key_online.state = LP_KEY_ONLINE_ST_CH_KEY_DEBUG_CONFIRM;
        break;
    case TOUCH_RECORD_GET_VERSION:
        log_debug("TOUCH_RECORD_GET_VERSION");
        memcpy(ver_info.sdkname, lp_key_sdk_name, sizeof(lp_key_sdk_name));
        memcpy(ver_info.lp_key_ver, lp_key_bt_ver, sizeof(lp_key_bt_ver));
        app_online_db_ack(parse_seq, (u8 *)(&ver_info), sizeof(ver_info)); //回复版本号数据结构
        break;
    case TOUCH_RECORD_GET_CH_SIZE:
        log_debug("TOUCH_RECORD_GET_CH_SIZE");
        res_data = sizeof(ch_content);
        _size = res_data;
        _offset = 0;
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4); //回复对应的通道数据长度
        break;
    case TOUCH_RECORD_GET_CH_CONTENT:
        log_debug("TOUCH_RECORD_GET_CH_CONTENT");
        if (_size > 32) {
            app_online_db_ack(parse_seq, (u8 *)(&ch_content) +  _offset, 32);
            _size -= 32;
            _offset += 32;
        } else {
            app_online_db_ack(parse_seq, (u8 *)(&ch_content) +  _offset, _size);
            _size = 0;
            _offset = 0;
        }
        break;
    case TOUCH_RECORD_CHANGE_MODE:
        log_debug("TOUCH_RECORD_CHANGE_MODE, cmd_mode = %d", touch_cmd->mode);
        lp_key_online.current_record_ch = touch_cmd->mode;
        lp_key_online.ch_ana.isel = 0;
        app_online_db_ack(parse_seq, (u8 *)"OK", 2); //回"OK"字符串
        break;
    default:
        break;
    }

    return 0;
}

int lp_touch_key_online_debug_send(u8 ch, u16 val)
{
    int err = 0;

    putchar('s');
    if (lp_key_online.state == LP_KEY_ONLINE_ST_CH_RES_DEBUG_START) {
        lp_key_online.res_packet = val;
        err = app_online_db_send(DB_PKT_TYPE_DAT_CH0, (u8 *)(&(lp_key_online.res_packet)), 2);
    }

    return err;
}

int lp_touch_key_online_debug_init(void)
{
    log_debug("%s", __func__);
    app_online_db_register_handle(DB_PKT_TYPE_TOUCH, lp_touch_key_online_debug_parse);
    lp_key_online.state = LP_KEY_ONLINE_ST_READY;

    return 0;
}

int lp_touch_key_online_debug_exit(void)
{
    return 0;
}

#endif /* #if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE */
/** @} lp_touch_key_tool_spp*/



/**@defgroup lp_touch_key_tool_testbox
* @{
 */
extern u8 testbox_get_key_action_test_flag(void *priv);
extern void eartch_state_update(u8 state);

__attribute__((weak))
u32 user_send_cmd_prepare(USER_CMD_TYPE cmd, u16 param_len, u8 *param)
{
    return 0;
}

u8 lp_touch_key_testbox_remote_test(u8 ch, u8 event)
{
    u8 ret = true;
    u8 key_report = 0;
    if (event == (CTMU_P2M_CH0_FALLING_EVENT + ch * 8)) {
        key_report = 0xF1;
        log_info("Notify testbox CH%d Down", ch);
    } else if (event == (CTMU_P2M_CH0_RAISING_EVENT + ch * 8)) {
        key_report = 0xF2;
        log_info("Notify testbox CH%d Up", ch);
    } else if (event == (CTMU_P2M_CH0_SHORT_KEY_EVENT + ch * 8)) {
    } else if (event == (CTMU_P2M_CH0_LONG_KEY_EVENT + ch * 8)) {
    } else if (event == (CTMU_P2M_CH0_HOLD_KEY_EVENT + ch * 8)) {
    } else {
        ret = false;
    }
    if (key_report) {
        user_send_cmd_prepare(USER_CTRL_TEST_KEY, 1, &key_report); //音量加
    }
    return  ret;
}

void lp_touch_key_testbox_inear_trim(u8 flag)
{
#if (!TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE)
    if (flag == 1) {
        __this->eartch_trim_flag = 1;
        __this->eartch_inear_ok = 0;
        __this->eartch_trim_value = 0;
        M2P_CTMU_CH_DEBUG |= BIT(__this->config->eartch_ch);
        log_info("__this->eartch_trim_flag = %d", __this->eartch_trim_flag);
        set_lpkey_active(1);
    } else {
        __this->eartch_trim_flag = 0;
        M2P_CTMU_CH_DEBUG &= ~BIT(__this->config->eartch_ch);
        log_info("__this->eartch_trim_flag = %d", __this->eartch_trim_flag);
    }
#endif
}
/** @} lp_touch_key_tool_testbox*/



