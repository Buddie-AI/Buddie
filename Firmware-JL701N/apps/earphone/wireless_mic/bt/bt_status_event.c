#include "app_config.h"
#include "app_task.h"
#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "btstack/btstack_error.h"
#include "btctrler/btctrler_task.h"
#include "classic/hci_lmp.h"
#include "bt_ble.h"
#include "bt_common.h"
#include "spp_user.h"
#include "app_main.h"
#include "app_power_manage.h"
#include "ui/ui_api.h"
#include "bt_edr_fun.h"
#include "adapter_process.h"
//#include "adapter_decoder.h"
#include "adapter_odev_bt.h"
//#include "dongle_1t2.h"
#include "ui_manage.h"

#if TCFG_WIRELESS_MIC_ENABLE

#define LOG_TAG_CONST        WIRELESSMIC
#define LOG_TAG             "[BT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

u8 set_call_vol_flag = 0;

#define __this 	(&app_bt_hdl)

static u8 bt_dev_role = 0;

void set_bt_dev_role(u8 role)
{
    bt_dev_role |= BIT(role);
}

/*************************************************************
             蓝牙模式协议栈状态事件处理
**************************************************************/
static void bt_status_init_ok(struct bt_event *bt)
{
    __this->init_ok = 1;
    if (__this->init_ok_time == 0) {
        __this->init_ok_time = timer_get_ms();
        __this->auto_exit_limit_time = POWERON_AUTO_CONN_TIME * 1000;
    }

#if TCFG_USER_BLE_ENABLE
    if (BT_MODE_IS(BT_BQB)) {
        ble_bqb_test_thread_init();
    } else {
    }
#endif

    bt_set_led_status(STATUS_BT_INIT_OK);
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 设备连接上状态
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_connect(struct bt_event *bt)
{

    sys_auto_sniff_controle(1, NULL);
    sys_auto_shut_down_disable();


#if (TCFG_BD_NUM == 2)
    if (get_current_poweron_memory_search_index(NULL) == 0) {
#if !USER_SUPPORT_DUAL_A2DP_SOURCE
        bt_wait_phone_connect_control(1);
#endif
    }
#endif

    bt_set_led_status(STATUS_BT_CONN);    //单台在此处设置连接状态,对耳的连接状态需要同步，在bt_tws.c中去设置

#if (TCFG_AUTO_STOP_PAGE_SCAN_TIME && TCFG_BD_NUM == 2)
    if (get_total_connect_dev() == 1) {   //当前有一台连接上了
        if (app_var.auto_stop_page_scan_timer == 0) {
            app_var.auto_stop_page_scan_timer = sys_timeout_add(NULL, bt_close_page_scan, (TCFG_AUTO_STOP_PAGE_SCAN_TIME * 1000)); //2
        }
    } else {
        if (app_var.auto_stop_page_scan_timer) {
            sys_timeout_del(app_var.auto_stop_page_scan_timer);
            app_var.auto_stop_page_scan_timer = 0;
        }
    }
#endif   //endif AUTO_STOP_PAGE_SCAN_TIME

}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 设备断开连接
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_disconnect(struct bt_event *bt)
{
    log_info("BT_STATUS_DISCONNECT\n");

    sys_auto_sniff_controle(0, NULL);

    __this->call_flag = 0;


    if (get_total_connect_dev() == 0) {    //已经没有设备连接
        if (!app_var.goto_poweroff_flag) { /*关机时不改UI*/
            bt_set_led_status(STATUS_BT_DISCONN);
        }
        sys_auto_shut_down_enable();
    }
#if USER_SUPPORT_DUAL_A2DP_SOURCE
    dongle_1t2_bt_disconn(bt);
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机来电
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_phone_income(struct bt_event *bt)
{
    __this->esco_dump_packet = ESCO_DUMP_PACKET_CALL;
    ui_update_status(STATUS_PHONE_INCOME);
    u8 tmp_bd_addr[6];
    memcpy(tmp_bd_addr, bt->args, 6);
    /*
     *(1)1t2有一台通话的时候，另一台如果来电不要提示
     *(2)1t2两台同时来电，现来的题示，后来的不播
     */
    if ((check_esco_state_via_addr(tmp_bd_addr) != BD_ESCO_BUSY_OTHER) && (bt_user_priv_var.phone_ring_flag == 0)) {
#if BT_INBAND_RINGTONE
        extern u8 get_device_inband_ringtone_flag(void);
        bt_user_priv_var.inband_ringtone = get_device_inband_ringtone_flag();
#else
        bt_user_priv_var.inband_ringtone = 0 ;
        lmp_private_esco_suspend_resume(3);
#endif

        bt_user_priv_var.phone_ring_flag = 1;
        bt_user_priv_var.phone_income_flag = 1;

#if 0
#if BT_PHONE_NUMBER
        phone_num_play_start();
#else
        phone_ring_play_start();
#endif
#endif

        user_send_cmd_prepare(USER_CTRL_HFP_CALL_CURRENT, 0, NULL); //发命令获取电话号码
    } else {
        /* log_debug("SCO busy now:%d,%d\n", check_esco_state_via_addr(tmp_bd_addr), bt_user_priv_var.phone_ring_flag); */
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机打出电话
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_phone_out(struct bt_event *bt)
{
    lmp_private_esco_suspend_resume(4);
    __this->esco_dump_packet = ESCO_DUMP_PACKET_CALL;
    ui_update_status(STATUS_PHONE_OUT);
    bt_user_priv_var.phone_income_flag = 0;
    user_send_cmd_prepare(USER_CTRL_HFP_CALL_CURRENT, 0, NULL); //发命令获取电话号码
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙通话音量同步,设置音量
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void phone_sync_vol(void *priv)
{
    log_debug("phone_sync_vol\n");
    bt_user_priv_var.phone_vol = 14;
    user_send_cmd_prepare(USER_CTRL_HFP_CALL_SET_VOLUME, 1, &bt_user_priv_var.phone_vol);
    user_send_cmd_prepare(USER_CTRL_HFP_CALL_VOLUME_UP, 0, NULL);
    bt_user_priv_var.phone_vol = 15;
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机接通电话
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_phone_active(struct bt_event *bt)
{
    ui_update_status(STATUS_PHONE_ACTIV);
    if (bt_user_priv_var.phone_call_dec_begin) {
        /* log_debug("call_active,dump_packet clear\n"); */
        __this->esco_dump_packet = ESCO_DUMP_PACKET_DEFAULT;
    }
    if (bt_user_priv_var.phone_ring_flag) {
        bt_user_priv_var.phone_ring_flag = 0;
        //tone_play_stop();
        if (bt_user_priv_var.phone_timer_id) {
            sys_timeout_del(bt_user_priv_var.phone_timer_id);
            bt_user_priv_var.phone_timer_id = 0;
        }
    }
    lmp_private_esco_suspend_resume(4);
    bt_user_priv_var.phone_income_flag = 0;
    bt_user_priv_var.phone_num_flag = 0;
    bt_user_priv_var.phone_con_sync_num_ring = 0;
    bt_user_priv_var.phone_con_sync_ring = 0;
    bt_user_priv_var.phone_vol = 15;
    /* log_debug("phone_active:%d\n", app_var.call_volume); */
#ifdef PHONE_CALL_DEFAULT_MAX_VOL
    set_call_vol_flag |= BIT(1);
    if ((BIT(1) | BIT(2)) == set_call_vol_flag) {
        //app_audio_set_volume(APP_AUDIO_STATE_CALL, 15, 1);
        sys_timeout_add(NULL, phone_sync_vol, 1200);
    }
#else
    //app_audio_set_volume(APP_AUDIO_STATE_CALL, app_var.call_volume, 1);
#endif
}
/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机挂断电话
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_phone_hangup(struct bt_event *bt)
{
    __this->esco_dump_packet = ESCO_DUMP_PACKET_CALL;
    /* log_info("phone_handup\n"); */
    if (bt_user_priv_var.phone_ring_flag) {
        bt_user_priv_var.phone_ring_flag = 0;
        //tone_play_stop();
        if (bt_user_priv_var.phone_timer_id) {
            sys_timeout_del(bt_user_priv_var.phone_timer_id);
            bt_user_priv_var.phone_timer_id = 0;
        }
    }
    lmp_private_esco_suspend_resume(4);
    bt_user_priv_var.phone_num_flag = 0;
    bt_user_priv_var.phone_con_sync_num_ring = 0;
    bt_user_priv_var.phone_con_sync_ring = 0;
    __this->call_back_flag &= ~BIT(0);

    if (get_call_status() == BT_CALL_HANGUP) {
        //call handup
        set_call_vol_flag  = 0;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机来电号码
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_phone_number(struct bt_event *bt)
{
    u8 *phone_number = NULL;
    phone_number = (u8 *)bt->value;
    //put_buf(phone_number, strlen((const char *)phone_number));
    if (bt_user_priv_var.phone_num_flag == 1) {
        return ;
    }
    bt_user_priv_var.income_phone_len = 0;
    memset(bt_user_priv_var.income_phone_num, '\0', sizeof(bt_user_priv_var.income_phone_num));
    for (int i = 0; i < strlen((const char *)phone_number); i++) {
        if (phone_number[i] >= '0' && phone_number[i] <= '9') {
            //过滤，只有数字才能报号
            bt_user_priv_var.income_phone_num[bt_user_priv_var.income_phone_len++] = phone_number[i];
            if (bt_user_priv_var.income_phone_len >= sizeof(bt_user_priv_var.income_phone_num)) {
                return;    /*buffer 空间不够，后面不要了*/
            }
        }
    }
    if (bt_user_priv_var.income_phone_len > 0) {
        bt_user_priv_var.phone_num_flag = 1;
    } else {
        log_debug("PHONE_NUMBER len err\n");
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机来电铃声
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_inband_ringtone(struct bt_event *bt)
{

#if BT_INBAND_RINGTONE
    bt_user_priv_var.inband_ringtone = bt->value;
#else
    bt_user_priv_var.inband_ringtone = 0;
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机高级音频开始播放
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_a2dp_media_start(struct bt_event *bt)
{
    __this->call_flag = 0;
    __this->a2dp_start_flag = 1;
    //a2dp_dec_open(bt->value);
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机高级音频停止播放
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_a2dp_media_stop(struct bt_event *bt)
{
    __this->a2dp_start_flag = 0;
    //a2dp_dec_close();
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机通话链路切换
   @param
   @return
   @note   手机音频切换、微信等都会有这个状态
*/
/*----------------------------------------------------------------------------*/
static void bt_status_sco_change(struct bt_event *bt, struct _odev_bt *odev_bt)
{
    extern void mem_stats(void);
    mem_stats();

    struct _odev_edr *edr = odev_bt->edr;

    /* log_info(" BT_STATUS_SCO_STATUS_CHANGE len:%d ,type:%d", (bt->value >> 16), (bt->value & 0x0000ffff)); */
    if (bt->value != 0xff) {

#ifdef PHONE_CALL_DEFAULT_MAX_VOL
        set_call_vol_flag |= BIT(2);
        if ((BIT(1) | BIT(2)) == set_call_vol_flag) {
            sys_timeout_add(NULL, phone_sync_vol, 1200);
        }
#endif
        log_info("<<<<<<<<<<<esco_dec_stat : %x\n", bt->value);
        __this->call_back_flag |= BIT(1);

        //app_reset_vddiom_lev(VDDIOM_VOL_32V);

        __this->sco_info = bt->value;

        if (edr->fun) {
            edr->fun(edr->priv, edr->mode, 1, &bt->value);
        }
        /* esco_dec_open(&bt->value, 0); */

        bt_user_priv_var.phone_call_dec_begin = 1;

        if (get_call_status() == BT_CALL_ACTIVE) {
            log_debug("dec_begin,dump_packet clear\n");
            __this->esco_dump_packet = ESCO_DUMP_PACKET_DEFAULT;
        }

    } else {

        log_info("<<<<<<<<<<<esco_dec_stop\n");
        __this->call_back_flag &= ~BIT(1);

        //app_reset_vddiom_lev(VDDIOM_VOL_34V);


        set_call_vol_flag &= ~BIT(2);
        bt_user_priv_var.phone_call_dec_begin = 0;
        __this->esco_dump_packet = ESCO_DUMP_PACKET_CALL;

#if 0
        if (edr->fun) {
            edr->fun(edr->priv, edr->mode, 0, &bt->value);
        }
#endif

        if (odev_edr.start_a2dp_flag) {
            //start a2dp
            y_printf("start a2dp\n");

            extern int a2dp_pp(u8 pp);
            a2dp_pp(1);
            odev_edr.start_a2dp_flag = 0;
        }
        /* esco_dec_close(); */
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机通话过程中设置音量产生状态
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_call_vol_change(struct bt_event *bt)
{
    u8 volume = bt->value;  //app_audio_get_max_volume() * bt->value / 15;
    u8 call_status = get_call_status();
    bt_user_priv_var.phone_vol = bt->value;
    if ((call_status == BT_CALL_ACTIVE) || (call_status == BT_CALL_OUTGOING) || app_var.siri_stu) {
        //app_audio_set_volume(APP_AUDIO_STATE_CALL, volume, 1);
    } else if (call_status != BT_CALL_HANGUP) {
        /*只保存，不设置到dac*/
        app_var.call_volume = volume;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status  样机和链接设备进入sniff模式后返回来的状态
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_sniff_state_update(struct bt_event *bt)
{
    if (bt->value == 0) {
        sys_auto_sniff_controle(1, bt->args);
    } else {
        sys_auto_sniff_controle(0, bt->args);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 最后拨打电话类型
   @param
   @return
   @note    只区分打入和打出
*/
/*----------------------------------------------------------------------------*/
static void bt_status_last_call_type_change(struct bt_event *bt)
{
    __this->call_back_flag |= BIT(0);
    bt_user_priv_var.last_call_type = bt->value;
}


/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 高级音频链接上
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_conn_a2dp_ch(struct bt_event *bt)
{
    r_printf("bt_status_conn_a2dp_ch\n");
    extern void adapter_process_event_notify(u8 event, int value);
#if USER_SUPPORT_DUAL_A2DP_SOURCE
    if (get_total_connect_dev() == 2) {
        r_printf("%s,%d", __func__, __LINE__);
        __emitter_send_media_toggle(bt->args, 1);
    } else {
        r_printf("%s,%d", __func__, __LINE__);
        adapter_process_event_notify(ADAPTER_EVENT_ODEV_MEDIA_OPEN, 0);
    }
    dongle_1t2_connect(bt);
#else
    adapter_process_event_notify(ADAPTER_EVENT_ODEV_MEDIA_OPEN, 0);
#endif




}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status 手机音频链接上
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_conn_hfp_ch(struct bt_event *bt)
{
#if !TCFG_USER_EMITTER_ENABLE
    if ((!is_1t2_connection()) && (get_current_poweron_memory_search_index(NULL))) { //回连下一个device
        if (get_esco_coder_busy_flag()) {
            clear_current_poweron_memory_search_index(0);
        } else {
            user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
        }
    }
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status   获取手机厂商
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_phone_menufactuer(struct bt_event *bt)
{
    extern const u8 hid_conn_depend_on_dev_company;
    app_var.remote_dev_company = bt->value;

    if (hid_conn_depend_on_dev_company) {
        if (bt->value) {
            //user_send_cmd_prepare(USER_CTRL_HID_CONN, 0, NULL);
        } else {
            user_send_cmd_prepare(USER_CTRL_HID_DISCONNECT, 0, NULL);
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status   语音识别
   @param
   @return
   @note    bt->value 0-siri关闭状态 ，
   				      1-siri开启状态 ，
					  2-手动打开siri成功状态.
                      3-安卓手机上选择语音识别软件状态
*/
/*----------------------------------------------------------------------------*/
static void bt_status_voice_recognition(struct bt_event *bt)
{
    __this->esco_dump_packet = ESCO_DUMP_PACKET_DEFAULT;

    app_var.siri_stu = bt->value;
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙status   接收远端设备发过来的avrcp命令
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_status_avrcp_income_opid(struct bt_event *bt)
{

#define AVC_VOLUME_UP			0x41
#define AVC_VOLUME_DOWN			0x42
    log_debug("BT_STATUS_AVRCP_INCOME_OPID:%d\n", bt->value);
    if (bt->value == AVC_VOLUME_UP) {

    }
    if (bt->value == AVC_VOLUME_DOWN) {

    }
}


/*----------------------------------------------------------------------------*/
/**@brief    蓝牙模式协议栈对应状态处理函数
   @param    bt:事件
   @return
   @note     蓝牙初始化完成、链接、通话播歌等状态
*/
/*----------------------------------------------------------------------------*/
int bt_connction_status_event_handler(struct bt_event *bt, struct _odev_bt *odev_bt)
{

    g_printf("-----------------------bt_connction_status_event_handler %d\n", bt->event);

    switch (bt->event) {
    case BT_STATUS_EXIT_OK:
        log_info("BT_STATUS_EXIT_OK\n");
        break;
    case BT_STATUS_INIT_OK:
        log_info("BT_STATUS_INIT_OK\n");
        bt_status_init_ok(bt);
        extern void adapter_process_event_notify(u8 event, int value);

        if (bt_dev_role & BIT(BT_AS_IDEV)) {
            adapter_process_event_notify(ADAPTER_EVENT_IDEV_INIT_OK, 0);
        }
        if (bt_dev_role & BIT(BT_AS_ODEV)) {
            adapter_process_event_notify(ADAPTER_EVENT_ODEV_INIT_OK, 0);
        }

        log_info("BT_STATUS_INIT_OK &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
        break;
    case BT_STATUS_START_CONNECTED:
        log_info(" BT_STATUS_START_CONNECTED\n");
        break;
    case BT_STATUS_ENCRY_COMPLETE:
        log_info(" BT_STATUS_ENCRY_COMPLETE\n");
        break;
    case BT_STATUS_SECOND_CONNECTED:
        log_info(" BT_STATUS_SECOND_CONNECTED\n");
        clear_current_poweron_memory_search_index(0);
    case BT_STATUS_FIRST_CONNECTED:
        log_info("BT_STATUS_CONNECTED\n");
        bt_status_connect(bt);
        break;
    case BT_STATUS_FIRST_DISCONNECT:
    case BT_STATUS_SECOND_DISCONNECT:
        log_info(" BT_STATUS_SECOND_DISCONNECT\n");
        bt_status_disconnect(bt);
        break;
    case BT_STATUS_PHONE_INCOME:
        log_info("BT_STATUS_PHONE_INCOME\n");
        bt_status_phone_income(bt);
        break;
    case BT_STATUS_PHONE_OUT:
        log_info("BT_STATUS_PHONE_OUT\n");
        bt_status_phone_out(bt);
        break;
    case BT_STATUS_PHONE_ACTIVE:
        log_info("BT_STATUS_PHONE_ACTIVE\n");
        bt_status_phone_active(bt);
        break;
    case BT_STATUS_PHONE_HANGUP:
        log_info(" BT_STATUS_PHONE_HANGUP\n");
        bt_status_phone_hangup(bt);
        break;
    case BT_STATUS_PHONE_NUMBER:
        log_info("BT_STATUS_PHONE_NUMBER\n");
        bt_status_phone_number(bt);
        break;
    case BT_STATUS_INBAND_RINGTONE:
        log_info("BT_STATUS_INBAND_RINGTONE\n");
        bt_status_inband_ringtone(bt);
        break;
    case BT_STATUS_BEGIN_AUTO_CON:
        log_info("BT_STATUS_BEGIN_AUTO_CON\n");
        break;
    case BT_STATUS_A2DP_MEDIA_START:
        log_info(" BT_STATUS_A2DP_MEDIA_START\n");
        bt_status_a2dp_media_start(bt);
        break;
    case BT_STATUS_A2DP_MEDIA_STOP:
        log_info(" BT_STATUS_A2DP_MEDIA_STOP");
        bt_status_a2dp_media_stop(bt);
        break;
    case BT_STATUS_SCO_STATUS_CHANGE:
        log_info(" BT_STATUS_SCO_STATUS_CHANGE");
        bt_status_sco_change(bt, odev_bt);
        break;
    case BT_STATUS_CALL_VOL_CHANGE:
        log_info(" BT_STATUS_CALL_VOL_CHANGE ");
        bt_status_call_vol_change(bt);
        break;
    case BT_STATUS_SNIFF_STATE_UPDATE:
        log_info(" BT_STATUS_SNIFF_STATE_UPDATE \n");    //0退出SNIFF
        bt_status_sniff_state_update(bt);
        break;
    case BT_STATUS_LAST_CALL_TYPE_CHANGE:
        log_info("BT_STATUS_LAST_CALL_TYPE_CHANGE\n");
        bt_status_last_call_type_change(bt);
        break;
    case BT_STATUS_CONN_A2DP_CH:
        bt_status_conn_a2dp_ch(bt);
    /* break; */
    case BT_STATUS_CONN_HFP_CH:
        bt_status_conn_hfp_ch(bt);
        break;
    case BT_STATUS_PHONE_MANUFACTURER:
        log_info("BT_STATUS_PHONE_MANUFACTURER\n");
        bt_status_phone_menufactuer(bt);
        break;
    case BT_STATUS_VOICE_RECOGNITION:
        log_info(" BT_STATUS_VOICE_RECOGNITION \n");
        bt_status_voice_recognition(bt);
        break;
    case BT_STATUS_AVRCP_INCOME_OPID:
        log_info("  BT_STATUS_AVRCP_INCOME_OPID \n");
        bt_status_avrcp_income_opid(bt);
        break;
    case  BT_STATUS_RECONN_OR_CONN:
        log_info("  BT_STATUS_RECONN_OR_CONN \n");
        break;
    default:
        log_info(" BT STATUS DEFAULT\n");
        break;
    }
    return 0;
}
#endif
