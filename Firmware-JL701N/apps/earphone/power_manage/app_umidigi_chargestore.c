#include "app_umidigi_chargestore.h"
#include "init.h"
#include "app_config.h"
#include "system/includes.h"
#include "asm/charge.h"
#include "user_cfg.h"
#include "app_chargestore.h"
#include "device/vm.h"
#include "btstack/avctp_user.h"
#include "app_power_manage.h"
#include "app_action.h"
#include "app_main.h"
#include "app_charge.h"
#include "app_testbox.h"
#include "classic/tws_api.h"
#include "update.h"
#include "bt_ble.h"
#include "bt_tws.h"
#include "app_action.h"
#include "bt_common.h"
#include "le_rcsp_adv_module.h"
#include "tone_player.h"
#include "app_msg.h"
#include "key_event_deal.h"

#define LOG_TAG_CONST       APP_UMIDIGI_CHARGESTORE
#define LOG_TAG             "[APP_UMIDIGI_CHARGESTORE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

extern void bt_tws_remove_pairs();
extern void cpu_reset();


#if TCFG_UMIDIGI_BOX_ENABLE

struct umidigi_chargestore_info {
    int timer;					//合盖之后定时进入休眠的句柄
    u8 cover_status;			//充电舱盖子状态 	 0: 合盖       1:开盖
    u8 ear_number;				//合盖时耳机在线数
    u8 close_ing;				//等待合窗超时
    u8 bt_init_ok;				//蓝牙协议栈初始化成功
    u8 active_disconnect;		//主动断开连接
    u8 sibling_chg_lev;			//对耳同步的充电舱电量
    u8 pair_flag;				//配对标记
    u8 power_sync;				//第一次获取到充电舱电量时,同步到对耳
    u8 switch2bt;				//开盖切换到蓝牙的标记位
    u8 pre_power_lvl;

    /*以下为充电舱发送CMD时携带的信息*/
    u8 case_charger_state;		//充电舱自身充电状态 0: 不在充电   1: 充电中
    u8 case_battery_level;		//充电舱电量百分比
    u8 l_ear_whether_in_box;	//左耳是否在舱 		 0: 左耳不在仓 1：左耳在仓
    u8 r_ear_whether_in_box;	//右耳是否在舱 		 0: 右耳不在仓 1：右耳在仓
    u8 usb_state;				//usb插入/拔出标志(本地播放专用)  0: USB OUT    1: USB IN
    u16 open_case_timer;		//延迟处理开盖命令定时器
    u16 pair_timer;				//延迟处理配对命令定时器
};

static struct umidigi_chargestore_info info = {
    .case_charger_state = 1,
    .ear_number = 1,
    .case_battery_level = 0xff,
    .sibling_chg_lev = 0xff,
};

#define __this (&info)

/*获取充电舱电池电量*/
u8 umidigi_chargestore_get_power_level(void)
{
    if ((__this->case_battery_level == 0xff) ||
        ((!get_charge_online_flag()) &&
         (__this->sibling_chg_lev != 0xff))) {
        return __this->sibling_chg_lev;
    }
    return __this->case_battery_level;
}

/*获取充电舱充电状态*/
u8 umidigi_chargestore_get_power_status(void)
{
    return __this->case_charger_state;
}

/*获取充电舱合盖或开盖状态*/
u8 umidigi_chargestore_get_cover_status(void)
{
    return __this->cover_status;
}

/*获取合盖状态时耳机在仓数量*/
u8 umidigi_chargestore_get_earphone_online(void)
{
    return __this->ear_number;
}

/*获取蓝牙初始化成功标志位*/
void umidigi_chargestore_set_bt_init_ok(u8 flag)
{
    __this->bt_init_ok = flag;
}

#if TCFG_USER_TWS_ENABLE
static void umidigi_set_tws_sibling_charge_level(void *_data, u16 len, bool rx)
{
    if (rx) {
        u8 *data = (u8 *)_data;
        umidigi_chargestore_set_sibling_chg_lev(data[0]);
    }
}

REGISTER_TWS_FUNC_STUB(charge_level_stub) = {
    .func_id = TWS_FUNC_ID_CHARGE_SYNC,
    .func    = umidigi_set_tws_sibling_charge_level,
};

#endif //TCFG_USER_TWS_ENABLE

int umidigi_chargestore_sync_chg_level(void)
{
#if TCFG_USER_TWS_ENABLE
    int err = -1;
    struct application *app = get_current_app();
    if (app && (!strcmp(app->name, "earphone")) && (!app_var.goto_poweroff_flag)) {
        err = tws_api_send_data_to_sibling((u8 *)&__this->case_battery_level, 1,
                                           TWS_FUNC_ID_CHARGE_SYNC);
    }
    return err;
#else
    return 0;
#endif
}

/*设置对耳同步的充电舱电量*/
void umidigi_chargestore_set_sibling_chg_lev(u8 chg_lev)
{
    __this->sibling_chg_lev = chg_lev;
}

/*设置充电舱电池电量*/
void umidigi_chargestore_set_power_level(u8 power)
{
    __this->case_battery_level = power;
}

/*获取允许poweroff标志位*/
u8 umidigi_chargestore_check_going_to_poweroff(void)
{
    return __this->close_ing;
}

void umidigi_chargestore_set_phone_disconnect(void)
{
#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)
#if (!CONFIG_NO_DISPLAY_BUTTON_ICON)
    if (__this->pair_flag && get_tws_sibling_connect_state()) {
        //check ble inquiry
        //printf("get box log_key...con_dev=%d\n",get_tws_phone_connect_state());
        if ((bt_ble_icon_get_adv_state() == ADV_ST_RECONN)
            || (bt_ble_icon_get_adv_state() == ADV_ST_DISMISS)
            || (bt_ble_icon_get_adv_state() == ADV_ST_END)) {
            bt_ble_icon_reset();
            bt_ble_icon_open(ICON_TYPE_INQUIRY);
        }
    }
#endif
    __this->pair_flag = 0;
#elif (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV_RCSP)
#if (!CONFIG_NO_DISPLAY_BUTTON_ICON)
    if (__this->pair_flag && get_tws_sibling_connect_state()) {
        //check ble inquiry
        bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
    }
#endif
    __this->pair_flag = 0;
#endif
}

void umidigi_chargestore_set_phone_connect(void)
{
    __this->active_disconnect = 0;
}

extern void sys_enter_soft_poweroff(void *priv);
void umidigi_chargestore_timeout_deal(void *priv)
{
    __this->timer = 0;
    __this->close_ing = 0;

    //该类型的充电仓获取不到充电在线状态时,认为出仓了。
    //避免盒盖马上开盖迅速拔出,收不到开盖消息,导致连上又复位系统。
    if (get_charge_online_flag() == 0) {
        __this->cover_status = 1;
    }

    if ((!__this->cover_status) || __this->active_disconnect) {//当前为合盖或者主动断开连接
        struct application *app = get_current_app();
        if (app && strcmp(app->name, "idle")) {
            sys_enter_soft_poweroff((void *)1);
        }
    } else {
#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)

        /* if ((!get_total_connect_dev()) && (tws_api_get_role() == TWS_ROLE_MASTER) && (get_bt_tws_connect_status())) { */
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            printf("charge_icon_ctl...\n");
            bt_ble_icon_reset();
#if CONFIG_NO_DISPLAY_BUTTON_ICON
            if (get_total_connect_dev()) {
                //蓝牙未真正断开;重新广播
                bt_ble_icon_open(ICON_TYPE_RECONNECT);
            } else {
                //蓝牙未连接，重新开可见性
                bt_ble_icon_open(ICON_TYPE_INQUIRY);
            }
#else
            //不管蓝牙是否连接，默认打开
            bt_ble_icon_open(ICON_TYPE_RECONNECT);
#endif

        }
#elif (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV_RCSP)
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            printf("charge_icon_ctl...\n");
#if CONFIG_NO_DISPLAY_BUTTON_ICON
            if (get_total_connect_dev()) {
                //蓝牙未真正断开;重新广播
                bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_CONNECTED, 1);
            } else {
                //蓝牙未连接，重新开可见性
                bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_UNCONNECTED, 1);
            }
#else
            //不管蓝牙是否连接，默认打开
            bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_CONNECTED, 1);
#endif

        }
#endif

    }
    __this->ear_number = 1;
}

/*开盖命令处理函数*/
static void open_case_callback(void *priv)
{
    struct application *app = get_current_app();
    if (__this->cover_status) {
        //电压过低,不进响应开盖命令
#if TCFG_SYS_LVD_EN
        if (get_vbat_need_shutdown() == TRUE) {
            log_info(" lowpower deal!\n");
            __this->open_case_timer = 0;
            return;
        }
#endif
        if (__this->cover_status) {//当前为开盖
            if (__this->power_sync) {
                if (umidigi_chargestore_sync_chg_level() == 0) {
                    __this->power_sync = 0;
                }
            }
            if (app && strcmp(app->name, APP_NAME_BT) && (app_var.goto_poweroff_flag == 0)) {
                /* app_var.play_poweron_tone = 1; */
                app_var.play_poweron_tone = 0;
                power_set_mode(TCFG_LOWPOWER_POWER_SEL);
                r_printf("switch to bt\n");
                __this->switch2bt = 1;
#if TCFG_SYS_LVD_EN
                vbat_check_init();
#endif
                task_switch_to_bt();
                __this->switch2bt = 0;
            }
        }
        __this->open_case_timer = 0;
    } else {
        __this->open_case_timer = 0;
    }
}

/*等待蓝牙初始化完成再进行对耳*/
static void tws_pair_wait_bt_init_callback(void *priv)
{
#if TCFG_USER_TWS_ENABLE
    if (bt_tws_get_local_channel() == 'L') {
        r_printf("L channel\n");
//充电仓不支持 双击回播电话,也不支持 双击配对和取消配对
#if (!TCFG_CHARGESTORE_ENABLE)
#if TCFG_USER_TWS_ENABLE
        if (bt_tws_start_search_sibling()) {
            tone_play_index(IDEX_TONE_NORMAL, 1);
        }
#endif
#endif
        if (bt_user_priv_var.last_call_type ==  BT_STATUS_PHONE_INCOME) {
            user_send_cmd_prepare(USER_CTRL_DIAL_NUMBER, bt_user_priv_var.income_phone_len, bt_user_priv_var.income_phone_num);
        } else {
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_LAST_NO, 0, NULL);
        }
    } else if (bt_tws_get_local_channel() == 'R') {
        r_printf("R channel\n");
    } else if (bt_tws_get_local_channel() == 'U') {
        r_printf("U channel\n");
    }
#endif
    __this->pair_timer = 0;
}

/*配对命令处理函数*/
static void tws_pair_callback(void *priv)
{
    struct application *app = get_current_app();
#if CONFIG_TWS_PAIR_MODE == CONFIG_TWS_PAIR_BY_CLICK
    if (app && strcmp(app->name, APP_NAME_BT) && (app_var.goto_poweroff_flag == 0)) {
        /* app_var.play_poweron_tone = 1; */
        app_var.play_poweron_tone = 0;
        power_set_mode(TCFG_LOWPOWER_POWER_SEL);
        r_printf("switch to bt\n");
        task_switch_to_bt();
    }
    sys_timeout_add(NULL, tws_pair_wait_bt_init_callback, 1000);
#endif
}

/*充电舱命令处理函数，所有命令携带8bit有效数据*/
int app_umidigi_chargestore_event_handler(struct umidigi_chargestore_event *umidigi_chargestore_dev)
{
    int ret = false;
    struct application *app = get_current_app();
#if defined(TCFG_CHARGE_KEEP_UPDATA) && TCFG_CHARGE_KEEP_UPDATA
    //在升级过程中,不响应智能充电舱app层消息
    if (dual_bank_update_exist_flag_get() || classic_update_task_exist_flag_get()) {
        return ret;
    }
#endif

    switch (umidigi_chargestore_dev->event) {
    case CMD_RESERVED_0:
        log_info("event_CMD_RESERVED_0\n");
        break;

    case CMD_FACTORY_RESET:
        log_info("event_CMD_FACTORY_RESET\n");
        /*该命令携带固定数据0x55*/
        if (umidigi_chargestore_dev->value != 0x55) {
            break;
        }
#if TCFG_USER_TWS_ENABLE
        bt_tws_remove_pairs();
#endif
        user_send_cmd_prepare(USER_CTRL_DEL_ALL_REMOTE_INFO, 0, NULL);
        cpu_reset();
        break;

    case CMD_RESERVED_2:
        log_info("event_CMD_RESERVED_2\n");
        break;

    case CMD_OPEN_CASE:
        log_info("event_CMD_OPEN_CASE\n");
        /*在收到其中一个开盖命令后，延迟2s进行耳机端开盖的相关操作*/
        /*防止被其他开盖命令导致的通讯口电平变化所影响*/
        if (!__this->open_case_timer) {
            __this->open_case_timer = sys_timeout_add(NULL, open_case_callback, 2000);
        }
        if (__this->case_battery_level == 0xff) {
            __this->power_sync = 1;
        }
        /*该命令携带的8bit data，BIT[7]为充电舱状态 BIT[6:0]为充电舱电量*/
        __this->case_charger_state = umidigi_chargestore_dev->value >> 7;
        __this->case_battery_level = umidigi_chargestore_dev->value & 0x7f;
        if (__this->case_battery_level != __this->pre_power_lvl) {
            __this->power_sync = 1;
        }
        __this->pre_power_lvl = __this->case_battery_level;
        break;

    case CMD_CLOSE_CASE:
        log_info("event_CMD_CLOSE_CASE\n");
        /*该命令携带的8bit data，BIT[7:2]为固定数据0x15*/
        /*BIT[1]：1：右耳在仓 0：右耳不在仓, BIT[0]：1：左耳在仓 0：左耳不在仓*/
        if ((umidigi_chargestore_dev->value >> 2) != 0x15) {
            break;
        }
        __this->r_ear_whether_in_box = (umidigi_chargestore_dev->value & 0x02) >> 1;
        __this->l_ear_whether_in_box = umidigi_chargestore_dev->value & 0x01;
        __this->ear_number = __this->r_ear_whether_in_box + __this->l_ear_whether_in_box;
        /* r_printf("r=%d l=%d\n", __this->r_ear_whether_in_box, __this->l_ear_whether_in_box); */
        /* r_printf("ear_number = %d\n", __this->ear_number); */

        if (!__this->cover_status) {//当前为合盖
#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)
            if ((__this->bt_init_ok) && (tws_api_get_role() == TWS_ROLE_MASTER))  {
                bt_ble_icon_close(1);
            }
#elif (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV_RCSP)
            if ((__this->bt_init_ok) && (tws_api_get_role() == TWS_ROLE_MASTER))  {
                bt_ble_adv_ioctl(BT_ADV_SET_EDR_CON_FLAG, SECNE_DISMISS, 1);
            }
#endif
            if (!__this->timer) {
                __this->timer = sys_timeout_add(NULL, umidigi_chargestore_timeout_deal, 2000);
                if (!__this->timer) {
                    log_error("timer alloc err!\n");
                } else {
                    __this->close_ing = 1;
                }
            } else {
                sys_timer_modify(__this->timer, 2000);
                __this->close_ing = 1;
            }
        } else {
            __this->ear_number = 1;
            __this->close_ing = 0;
        }
        break;

    case CMD_ENTER_PAIRING_MODE:
        log_info("event_CMD_ENTER_PAIRING_MODE\n");
        /*该命令携带固定数据0x55*/
        if (umidigi_chargestore_dev->value != 0x55) {
            break;
        }
        if (!__this->pair_timer) {
            __this->pair_timer = sys_timeout_add(NULL, tws_pair_callback, 2000);
        }
        break;

    case CMD_DUT_MODE_CMD:
        log_info("event_CMD_DUT_MODE_CMD\n");
        break;

    case CMD_USB_STATE:
        log_info("event_CMD_USB_STATE\n");
        /*该命令携带固定数据0x05或0x70  0x05: USB IN, 0x70: USB OUT*/
        if (umidigi_chargestore_dev->value == 0x05) {
            __this->usb_state = 1;
        } else if (umidigi_chargestore_dev->value == 0x70) {
            __this->usb_state = 0;
        }
        break;

    case CMD_RESERVED_B:
        log_info("event_CMD_RESERVED_B\n");
        break;

    case CMD_SEND_CASE_BATTERY:
        log_info("event_CMD_SEND_CASE_BATTERY\n");
        /*该命令携带的8bit data，BIT[7]为充电舱状态 BIT[6:0]为充电舱电量*/
        __this->case_charger_state = umidigi_chargestore_dev->value >> 7;
        __this->case_battery_level = umidigi_chargestore_dev->value & 0x7f;
        break;

    default:
        break;
    }
}


void umidigi_chargestore_event_to_user(u32 type, u8 event, u8 data)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg  = (void *)type;
    e.u.umidigi_chargestore.event = event;
    e.u.umidigi_chargestore.value = data;
    sys_event_notify(&e);
}

/*umidigi充电舱消息处理*/
void app_umidigi_chargetore_message_deal(u16 message)
{
    u8 cmd, data;
    cmd  = (message & CMD_IN_MESSAGE) >> 10;
    data = (message & DATA_IN_MESSAGE) >> 2;

#if TCFG_TEST_BOX_ENABLE
    //测试盒在线时,不解码数据
    if (testbox_get_status()) {
        return;
    }
#endif

    switch (cmd) {
    case CMD_RESERVED_0:
        log_info("reserved0\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_RESERVED_0, data);
        break;
    case CMD_FACTORY_RESET:
        //充电仓不删除配对信息
        break;
        __this->cover_status = 1;
        __this->close_ing = 0;
        r_printf("factory reset\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_FACTORY_RESET, data);
        break;
    case CMD_RESERVED_2:
        log_info("reserved2\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_RESERVED_2, data);
        break;
    case CMD_OPEN_CASE:
        __this->cover_status = 1;
        __this->close_ing = 0;
        log_info("open case\n");
//切模式过程中不发送消息,防止堆满消息
        if (__this->switch2bt == 0) {
            umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_OPEN_CASE, data);
        }
        break;
    case CMD_CLOSE_CASE:
        __this->cover_status = 0;
        log_info("close case\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_CLOSE_CASE, data);
        break;
    case CMD_ENTER_PAIRING_MODE:
        __this->cover_status = 1;
        log_info("enter pairing mode\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_ENTER_PAIRING_MODE, data);
        break;
    case CMD_DUT_MODE_CMD:
        log_info("dut mode\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_DUT_MODE_CMD, data);
        break;
    case CMD_USB_STATE:
        log_info("usb state\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_USB_STATE, data);
        break;
    case CMD_RESERVED_B:
        log_info("reservedB\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_RESERVED_B, data);
        break;
    case CMD_SEND_CASE_BATTERY:
        log_info("received case battery\n");
        umidigi_chargestore_event_to_user(DEVICE_EVENT_UMIDIGI_CHARGE_STORE, CMD_SEND_CASE_BATTERY, data);
        break;
    default:
        log_info("received an invalid message!\n");
        break;
    }
}

#endif
