#include "system/includes.h"
#include "media/includes.h"
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
#include "app_chargestore.h"
#include "app_charge.h"
#include "app_main.h"
#include "app_power_manage.h"
#include "user_cfg.h"
#include "asm/pwm_led.h"
#include "system/timer.h"
#include "asm/hwi.h"
#include "cpu.h"
#include "ui/ui_api.h"
#include "tone_player.h"
#include "bt_edr_fun.h"
#include "adapter_process.h"
//#include "adapter_decoder.h"
#include "aec_user.h"
#include "ui_manage.h"
#if TCFG_WIRELESS_MIC_ENABLE

#define LOG_TAG_CONST       WIRELESSMIC
#define LOG_TAG             "[BT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"


struct app_bt_opr app_bt_hdl = {
    .exit_flag = 1,
    .replay_tone_flag = 1,
    .esco_dump_packet = ESCO_DUMP_PACKET_CALL,
    .hid_mode = 0,
    .wait_exit = 0,
};

#define __this 	(&app_bt_hdl)

extern BT_USER_PRIV_VAR bt_user_priv_var;

u8 get_bt_init_status(void)
{
    return __this->init_ok;
}

u8 is_call_now(void)
{
    if (get_call_status() != BT_CALL_HANGUP) {
        return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙模式led状态更新
   @param
   @return   status : 蓝牙状态
   @note
*/
/*----------------------------------------------------------------------------*/
void bt_set_led_status(u8 status)
{
#if 1
    static u8 bt_status = STATUS_BT_INIT_OK;

    if (status) {
        bt_status = status;
    }

    pwm_led_mode_set(PWM_LED_ALL_OFF);
    ui_update_status(bt_status);
#endif
}


/*----------------------------------------------------------------------------*/
/**@brief    蓝牙spp 协议数据 回调
   @param    packet_type:数据类型
  			 ch         :
			 packet     :数据缓存
			size        ：数据长度
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void spp_data_handler(u8 packet_type, u16 ch, u8 *packet, u16 size)
{
    switch (packet_type) {
    case 1:
        log_debug("spp connect\n");
        break;
    case 2:
        log_debug("spp disconnect\n");
        break;
    case 7:
        //log_info("spp_rx:");
        //put_buf(packet,size);
#if AEC_DEBUG_ONLINE
        aec_debug_online(packet, size);
#endif
        break;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙获取在连接设备名字回调
   @param    status : 1：获取失败   0：获取成功
			 addr   : 配对设备地址
			 name   :配对设备名字
   @return
   @note     需要连接上设备后发起USER_CTRL_READ_REMOTE_NAME
   			 命令来
*/
/*----------------------------------------------------------------------------*/
__attribute__((weak))
void remote_name_speciali_deal(u8 status, u8 *addr, u8 *name)
{

}

static void bt_read_remote_name(u8 status, u8 *addr, u8 *name)
{
    if (status) {
        log_debug("remote_name fail \n");
    } else {
        log_debug("remote_name : %s \n", name);
    }

    log_debug_hexdump(addr, 6);

    remote_name_speciali_deal(status, addr, name);
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙歌词信息获取回调
   @param
   @return
   @note
   const u8 more_avctp_cmd_support = 1;置上1
   需要在void bredr_handle_register()注册回调函数
   要动态获取播放时间的，可以发送USER_CTRL_AVCTP_OPID_GET_PLAY_TIME命令就可以了
   要半秒或者1秒获取就做个定时发这个命令
*/
/*----------------------------------------------------------------------------*/
static void user_get_bt_music_info(u8 type, u32 time, u8 *info, u16 len)
{
    //profile define type: 1-title 2-artist name 3-album names 4-track number 5-total number of tracks 6-genre  7-playing time
    //JL define 0x10-total time , 0x11 current play position
    u8  min, sec;
    //printf("type %d\n", type );
    if ((info != NULL) && (len != 0)) {
        log_debug(" %s \n", info);
    }
    if (time != 0) {
        min = time / 1000 / 60;
        sec = time / 1000 - (min * 60);
        log_debug(" time %d %d\n ", min, sec);
    }
}



//音量同步
static void vol_sys_tab_init(void)
{
}

static void bt_set_music_device_volume(int volume)
{
    r_printf("bt_set_music_device_volume : %d\n", volume);
}

static int phone_get_device_vol(void)
{
    //音量同步最大是127，请计数比例
#if 0
    return (app_var.sys_vol_l * get_max_sys_vol() / 127) ;
#else
    return app_var.opid_play_vol_sync;
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙模式协议栈功能配置
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_function_select_init()
{
    set_idle_period_slot(1600);
    ////设置协议栈支持设备数
    __set_user_ctrl_conn_num(TCFG_BD_NUM);
    ////msbc功能使能
    __set_support_msbc_flag(1);
#if TCFG_BT_SUPPORT_AAC
    ////AAC功能使能
    __set_support_aac_flag(1);
#else
    __set_support_aac_flag(0);
#endif

#if BT_SUPPORT_DISPLAY_BAT
    ////设置更新电池电量的时间间隔
    __bt_set_update_battery_time(60);
#else
    __bt_set_update_battery_time(0);
#endif

    ////回连搜索时间长度设置,可使用该函数注册使用，ms单位,u16
    __set_page_timeout_value(8000);

    ////回连时超时参数设置。ms单位。做主机有效
    __set_super_timeout_value(8000);

#if (TCFG_BD_NUM == 2)
    ////设置开机回链的设备个数
    __set_auto_conn_device_num(2);
#endif

#if BT_SUPPORT_MUSIC_VOL_SYNC
    ////设置音乐音量同步的表
    vol_sys_tab_init();
#endif

    ////设置蓝牙是否跑后台
    __set_user_background_goback(BACKGROUND_GOBACK); // 后台链接是否跳回蓝牙 1:跳回

    ////设置蓝牙加密的level
    //io_capabilities ; /*0: Display only 1: Display YesNo 2: KeyboardOnly 3: NoInputNoOutput*/
    //authentication_requirements: 0:not protect  1 :protect
    __set_simple_pair_param(3, 0, 2);


#if (USER_SUPPORT_PROFILE_PBAP==1)
    ////设置蓝牙设备类型
    __change_hci_class_type(BD_CLASS_CAR_AUDIO);
#endif

#if (TCFG_BT_SNIFF_ENABLE == 0)
    void lmp_set_sniff_disable(void);
    lmp_set_sniff_disable();
#endif


    /*
                TX     RX
       AI800x   PA13   PA12
       AC692x   PA13   PA12
       AC693x   PA8    PA9
       AC695x   PA9    PA10
       AC696x   PA9    PA10
       AC694x   PB1    PB2
       AC697x   PC2    PC3
       AC631x   PA7    PA8

    */
    ////设置蓝牙接收状态io输出，可以外接pa
    /* bt_set_rxtx_status_enable(1); */

#if TCFG_USER_BLE_ENABLE
    {
        u8 tmp_ble_addr[6];
        lib_make_ble_address(tmp_ble_addr, (void *)bt_get_mac_addr());
        le_controller_set_mac((void *)tmp_ble_addr);

        printf("\n-----edr + ble 's address-----");
        printf_buf((void *)bt_get_mac_addr(), 6);
        printf_buf((void *)tmp_ble_addr, 6);
    }
#endif // TCFG_USER_BLE_ENABLE

#if (CONFIG_BT_MODE != BT_NORMAL)
    set_bt_enhanced_power_control(1);
#endif
}

extern u8  get_cur_battery_level(void);
static int bt_get_battery_value()
{
    //取消默认蓝牙定时发送电量给手机，需要更新电量给手机使用USER_CTRL_HFP_CMD_UPDATE_BATTARY命令
    /*电量协议的是0-9个等级，请比例换算*/
    return get_cur_battery_level();
}
/*----------------------------------------------------------------------------*/
/**@brief    蓝牙模式协议栈回调函数
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
static void bredr_handle_register()
{
#if (USER_SUPPORT_PROFILE_SPP==1)
#if APP_ONLINE_DEBUG
    extern void online_spp_init(void);
    spp_data_deal_handle_register(user_spp_data_handler);
    online_spp_init();
#else
    spp_data_deal_handle_register(spp_data_handler);
#endif
#endif

#if BT_SUPPORT_MUSIC_VOL_SYNC
    ///蓝牙音乐和通话音量同步
    music_vol_change_handle_register(bt_set_music_device_volume, phone_get_device_vol);
#endif

#if BT_SUPPORT_DISPLAY_BAT
    ///电量显示获取电量的接口
    get_battery_value_register(bt_get_battery_value);
#endif

    extern void bt_fast_test_api(void);
    ///被测试盒链接上进入快速测试回调
    bt_fast_test_handle_register(bt_fast_test_api);

    //extern void bt_dut_api(u8 value);
    ///样机进入dut被测试仪器链接上回调
    //bt_dut_test_handle_register(bt_dut_api);

    ///获取远端设备蓝牙名字回调
    read_remote_name_handle_register(bt_read_remote_name);

    ////获取歌曲信息回调
    /* bt_music_info_handle_register(user_get_bt_music_info); */
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙直接开关
   @param
   @return
   @note   如果想后台开机不需要进蓝牙，可以在poweron模式
   			直接调用这个函数初始化蓝牙
*/
/*----------------------------------------------------------------------------*/

extern void sys_auto_shut_down_enable(void);
extern void sys_auto_sniff_controle(u8 enable, u8 *addr);
void bt_init()
{
    if (__this->bt_direct_init) {
        return;
    }

    log_info(" bt_direct_init  \n");

    u32 sys_clk =  clk_get("sys");
    bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

    __this->ignore_discon_tone = 0;
    __this->bt_direct_init = 1;

    bt_function_select_init();
    bredr_handle_register();
    btstack_init();

    /* 按键消息使能 */
    sys_key_event_enable();
    sys_auto_shut_down_enable();
    sys_auto_sniff_controle(1, NULL);
}


/*----------------------------------------------------------------------------*/
/**@brief  蓝牙 退出蓝牙等待蓝牙状态可以退出
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void bt_close_check(void *priv)
{
    if (bt_user_priv_var.auto_connection_timer) {
        sys_timeout_del(bt_user_priv_var.auto_connection_timer);
        bt_user_priv_var.auto_connection_timer = 0;
    }

    if (__this->init_ok == 0) {
        /* putchar('#'); */
        return;
    }

    // if (bt_audio_is_running()) {
    //     /* putchar('$'); */
    //     return;
    // }

#if TCFG_USER_BLE_ENABLE
    bt_ble_exit();
#endif

    btstack_exit();
    log_info(" bt_direct_close_check ok\n");
    sys_timer_del(__this->timer);
    __this->init_ok = 0;
    __this->init_start = 0;
    __this->timer = 0;
    __this->bt_direct_init = 0;
    set_stack_exiting(0);
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙后台直接关闭
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void bt_close(void)
{
    if (__this->init_ok == 0) {
        /* putchar('#'); */
        return;
    }

    log_info(" bt_direct_close");

    __this->ignore_discon_tone = 1;
    sys_auto_shut_down_disable();

    set_stack_exiting(1);

    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_CONNECTION_CANCEL, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);

    if (__this->timer == 0) {
        __this->tmr_cnt = 0;
        __this->timer = sys_timer_add(NULL, bt_close_check, 10);
        printf("set exit timer\n");
    }
}
#endif


