#include "system/includes.h"
#include "media/includes.h"
#include "tone_player.h"
#include "earphone.h"

#include "app_config.h"
#include "app_action.h"
#include "app_task.h"

#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "btctrler/btctrler_task.h"
#include "btstack/frame_queque.h"
#include "user_cfg.h"
#include "aec_user.h"
#include "classic/hci_lmp.h"
#include "bt_common.h"
#include "bt_ble.h"
#include "bt_tws.h"
#include "pbg_user.h"
#include "btstack/bluetooth.h"

#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif/*TCFG_AUDIO_ANC_ENABLE*/

#if TCFG_ANC_BOX_ENABLE
#include "app_ancbox.h"
#endif

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
#include "spatial_imu_trim.h"
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)
#include "include/bt_ble.h"
#endif

#if TCFG_AUDIO_CVP_DUT_ENABLE
#include "audio_cvp_dut.h"
#endif /*TCFG_AUDIO_CVP_DUT_ENABLE*/

#if TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE
#include "app_chargestore.h"
#endif
#include "jl_kws/jl_kws_api.h"

#include "asm/charge.h"
#include "app_charge.h"
#include "ui_manage.h"

#include "app_chargestore.h"
#include "app_umidigi_chargestore.h"
#include "app_testbox.h"
#include "app_online_cfg.h"
#include "app_main.h"
#include "app_power_manage.h"
#include "gSensor/gSensor_manage.h"
#include "key_event_deal.h"
#include "classic/tws_api.h"
#include "asm/pwm_led.h"
#include "ir_sensor/ir_manage.h"
#include "in_ear_detect/in_ear_manage.h"
#include "vol_sync.h"
#include "bt_background.h"
#include "default_event_handler.h"

#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
extern int ais_platform_asr_open(void);
extern void ais_platform_asr_close(void);
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/

#define LOG_TAG_CONST       EARPHONE
#define LOG_TAG             "[EARPHONE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#define TIMEOUT_CONN_TIME         60 //超时断开之后回连的时间s
#define POWERON_AUTO_CONN_TIME    12  //开机去回连的时间
#define TWS_RETRY_CONN_TIMEOUT    ((rand32() & BIT(0)) ? 200 : 400)
#define PHONE_DLY_DISCONN_TIME    0//4000  //超时断开，快速连接上不播提示音

extern void dac_analog_power_control(u8 en);
extern u8 is_dac_power_off();
extern u8 get_esco_coder_busy_flag();
extern void dac_power_off(void);
extern void bredr_fcc_init(u8 mode, u8 fre);
extern void bredr_set_dut_enble(u8 en, u8 phone);
extern int lmp_private_esco_suspend_resume(int flag);
extern void tws_user_sync_call_vol();
extern void ble_enter_single_carrier_mode(u8 ch_index);
extern int music_player_get_play_status(void);

/* extern void set_adjust_conn_dac_check(u8 value); */

void sys_auto_shut_down_disable(void);
extern void bt_pll_para(u32 osc, u32 sys, u8 low_power, u8 xosc);

BT_USER_COMM_VAR bt_user_comm_var;
BT_USER_PRIV_VAR bt_user_priv_var;

int phone_call_begin(void *priv);
int phone_call_end(void *priv);
int earphone_a2dp_audio_codec_open(int media_type);
void earphone_a2dp_audio_codec_close();
u8 bt_audio_is_running(void);
void tws_try_connect_disable(void);
int lmp_private_esco_suspend_resume(int flag);

u8 hci_standard_connect_check(void);
void ble_adv_enable(u8 enable);
int a2dp_dec_open(int media_type);
int a2dp_dut_dec_open(int media_type);
void a2dp_dec_close();
int esco_dec_open(void *, u8);
void esco_dec_close();
void esco_enc_close();
void __set_sbc_cap_bitpool(u8 sbc_cap_bitpoola);


static u16 power_mode_timer = 0;

u8 init_ok = 0;
u8 get_bt_init_status(void)
{
    return init_ok;
}

static u8 sniff_out = 0;
u8 get_sniff_out_status()
{
    return sniff_out;
}
void clear_sniff_out_status()
{
    sniff_out = 0;
}

void earphone_change_pwr_mode(int mode, int msec)
{
#if TCFG_POWER_MODE_QUIET_ENABLE
    if (power_mode_timer) {
        sys_timeout_del(power_mode_timer);
        power_mode_timer = 0;
    }
    if (msec == 0) {
        power_set_mode(mode);
    } else {
        power_mode_timer = sys_timeout_add((void *)mode, power_set_mode, msec);
    }
#endif
}


/*开关可发现可连接的函数接口*/
void bt_wait_phone_connect_control(u8 enable)
{
#if TCFG_USER_TWS_ENABLE
    return;
#endif

    if (enable && app_var.goto_poweroff_flag) {
        return;
    }

    if (enable) {
        log_info("is_1t2_connection:%d \t total_conn_dev:%d\n", is_1t2_connection(), get_total_connect_dev());
        if (is_1t2_connection()) {
            /*达到最大连接数，可发现(0)可连接(0)*/
            user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
        } else {
            if (get_total_connect_dev() == 1) {
                /*支持连接2台，只连接一台的情况下，可发现(0)可连接(1)*/
                user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
                user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
            } else {
                /*可发现(1)可连接(1)*/
                EARPHONE_STATE_SET_PAGE_SCAN_ENABLE();
                user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
                user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
            }
        }
    } else {
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
    }
}

void bt_send_keypress(u8 key)
{
    user_send_cmd_prepare(USER_CTRL_KEYPRESS, 1, &key);
}

void bt_send_pair(u8 en)
{
    user_send_cmd_prepare(USER_CTRL_PAIR, 1, &en);
}

void bt_init_ok_search_index(void)
{
    if (!bt_user_priv_var.auto_connection_counter && get_current_poweron_memory_search_index(bt_user_priv_var.auto_connection_addr)) {
        log_info("bt_wait_connect_and_phone_connect_switch\n");
        clear_current_poweron_memory_search_index(1);
        bt_user_priv_var.auto_connection_counter = POWERON_AUTO_CONN_TIME * 1000; //8000ms
        EARPHONE_STATE_GET_CONNECT_MAC_ADDR();
    }
}

int bt_wait_connect_and_phone_connect_switch(void *p)
{
    int ret = 0;
    int timeout = 0;
    s32 wait_timeout;

    if (bt_user_priv_var.auto_connection_counter <= 0) {
        bt_user_priv_var.auto_connection_timer = 0;
        bt_user_priv_var.auto_connection_counter = 0;

        EARPHONE_STATE_CANCEL_PAGE_SCAN();

        log_info("auto_connection_counter = 0\n");
        user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);

        if (get_current_poweron_memory_search_index(NULL)) {
            bt_init_ok_search_index();
            return bt_wait_connect_and_phone_connect_switch(0);
        } else {
            bt_wait_phone_connect_control(1);
            return 0;
        }
    }
    /* log_info(">>>phone_connect_switch=%d\n",bt_user_priv_var.auto_connection_counter ); */
    if ((int)p == 0) {
        if (bt_user_priv_var.auto_connection_counter) {
            timeout = 5600;
            bt_wait_phone_connect_control(0);
            user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, bt_user_priv_var.auto_connection_addr);
            ret = 1;
        }
    } else {
        timeout = 400;
        user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
        bt_wait_phone_connect_control(1);
    }
    if (bt_user_priv_var.auto_connection_counter) {
        bt_user_priv_var.auto_connection_counter -= timeout ;
        log_info("do=%d\n", bt_user_priv_var.auto_connection_counter);
    }
    bt_user_priv_var.auto_connection_timer = sys_timeout_add((void *)(!(int)p),
            bt_wait_connect_and_phone_connect_switch, timeout);

    return ret;
}

void bt_close_page_scan(void *p)
{
    log_info(">>>%s\n", __func__);
    bt_wait_phone_connect_control(0);
    sys_timer_del(app_var.auto_stop_page_scan_timer);
}


extern bool get_esco_busy_flag();
extern u8  get_cur_battery_level(void);
extern void audio_adc_mic_demo(u8 mic_idx, u8 gain, u8 mic_2_dac);
extern void audio_fast_mode_test();
extern void audio_adc_mic_demo_close();
static int bt_get_battery_value()
{
    //取消默认蓝牙定时发送电量给手机，需要更新电量给手机使用USER_CTRL_HFP_CMD_UPDATE_BATTARY命令
    /*电量协议的是0-9个等级，请比例换算*/
    return get_cur_battery_level();
}

void bt_fast_test_api(void)
{
    log_info("------------bt_fast_test_api---------\n");
    //进入快速测试模式，用户根据此标志判断测试，如测试按键-开按键音  、测试mic-开扩音 、根据fast_test_mode根据改变led闪烁方式、关闭可发现可连接
    if (bt_user_priv_var.fast_test_mode == 0) {
        bt_user_priv_var.fast_test_mode = 1;
        audio_fast_mode_test();
        pwm_led_mode_set(PWM_LED_ALL_ON);
    }
}

static void bt_dut_api(void)
{
    log_info("bt in dut \n");
    sys_auto_shut_down_disable();
#if TCFG_USER_TWS_ENABLE
    extern 	void tws_cancle_all_noconn();
    tws_cancle_all_noconn() ;
#else
    sys_timer_del(app_var.auto_stop_page_scan_timer);
    extern void bredr_close_all_scan();
    bredr_close_all_scan();
#endif

    if (bt_user_priv_var.auto_connection_timer) {
        sys_timeout_del(bt_user_priv_var.auto_connection_timer);
        bt_user_priv_var.auto_connection_timer = 0;
    }

#if TCFG_USER_BLE_ENABLE
#if (CONFIG_BT_MODE == BT_NORMAL)
    bt_ble_adv_enable(0);
#endif
#endif
}

void bit_clr_ie(unsigned char index);
void bt_fix_fre_api(u8 fre)
{
    bt_dut_api();

    bit_clr_ie(IRQ_BREDR_IDX);
    bit_clr_ie(IRQ_BT_CLKN_IDX);

    bredr_fcc_init(BT_FRE, fre);
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙模式进入定频测试接收发射
   @param      mode ：0 测试发射    1：测试接收
   			  mac_addr:测试设置的地址
			  fre：定频的频点   0=2402  1=2403
			  packet_type：数据包类型

				#define DH1_1        0
				#define DH3_1        1
				#define DH5_1        2
				#define DH1_2        3
				#define DH3_2        4
				#define DH5_2        5

			  payload：数据包内容  0x0000  0x0055 0x00aa 0x00ff
						0xffff:prbs9
   @return
   @note     量产的时候通过串口，发送设置的参数，设置发送接收的参数

   关闭定频接收发射测试
	void link_fix_txrx_disable();

	更新接收结果
	void bt_updata_fix_rx_result()

struct link_fix_rx_result {
    u32 rx_err_b;  //接收到err bit
    u32 rx_sum_b;  //接收到正确bit
    u32 rx_perr_p;  //接收到crc 错误 包数
    u32 rx_herr_p;  //接收到crc 以外其他错误包数
    u32 rx_invail_p; //接收到crc错误bit太多的包数，丢弃不统计到err bit中
};

*/

/*----------------------------------------------------------------------------*/
void bt_fix_txrx_api(u8 mode, u8 *mac_addr, u8 fre, u8 packet_type, u16 payload)
{
    bt_dut_api();
    local_irq_disable();
    link_fix_txrx_disable();
    if (mode) {
        link_fix_rx_enable(mac_addr, fre, packet_type, 0xffff);
    } else {
        link_fix_tx_enable(mac_addr, fre, packet_type, 0xffff);
    }
    local_irq_enable();
}


void bt_updata_fix_rx_result()
{
    struct link_fix_rx_result fix_rx_result;
    link_fix_rx_update_result(&fix_rx_result);
    printf("err_b:%d sum_b:%d perr:%d herr_b:%d invaile:%d  \n",
           fix_rx_result.rx_err_b,
           fix_rx_result.rx_sum_b,
           fix_rx_result.rx_perr_p,
           fix_rx_result.rx_herr_p,
           fix_rx_result.rx_invail_p
          );
}
#if (defined TCFG_USER_RSSI_TEST_EN && TCFG_USER_RSSI_TEST_EN)
extern s8 get_rssi_api(s8 *phone_rssi, s8 *tws_rssi);
static void spp_send_rssi(int slave_rssi)
{
    s8 send_buf[8];
    send_buf[0] = 0xfe;
    send_buf[1] = 0xdc;
    send_buf[2] = 0x04;
    send_buf[3] = 0x02;
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_local_channel() == 'L') {
        get_rssi_api(&send_buf[4], &send_buf[6]);
        send_buf[5] = (s8)slave_rssi;
    } else
#endif
    {
        get_rssi_api(&send_buf[5], &send_buf[6]);
        send_buf[4] = (s8)slave_rssi;
    }
    send_buf[7] = 0xba;
    user_send_cmd_prepare(USER_CTRL_SPP_SEND_DATA, sizeof(send_buf), send_buf);
    /* put_buf(send_buf, sizeof(send_buf)); */
    /* printf("get rssi, l:%d, r:%d, tws:%d", send_buf[4], send_buf[5], send_buf[6]); */
}

static void app_core_send_rssi(int slave_rssi)
{
    int msg[3];

    msg[0] = spp_send_rssi;
    msg[1] = 1;
    msg[2] = slave_rssi;
    os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
}

#if TCFG_USER_TWS_ENABLE
#define TWS_FUNC_ID_RSSI_SYNC	TWS_FUNC_ID('R', 'S', 'S', 'I')

static void tws_get_rssi_handler(void *_data, u16 len, bool rx)
{
    u8 *data = (u8 *)_data;

    if (rx) {
        app_core_send_rssi(data[0]);
    }
}

REGISTER_TWS_FUNC_STUB(tws_get_rssi) = {
    .func_id = TWS_FUNC_ID_RSSI_SYNC,
    .func    = tws_get_rssi_handler,
};
#endif

static int spp_get_rssi_handler(u8 *packet, u16 size)
{
    const u8 get_rssi_cmd[] = {0xfe, 0xdc, 0x01, 0x01, 0xba};
    u8 send_cmd_buffer[2];
    if (size == strlen(get_rssi_cmd) && !memcmp(packet, get_rssi_cmd, size)) {
#if TCFG_USER_TWS_ENABLE
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            if (tws_api_get_role() == TWS_ROLE_SLAVE) {
                get_rssi_api(&send_cmd_buffer[0], &send_cmd_buffer[1]);
                tws_api_send_data_to_sibling(send_cmd_buffer, sizeof(send_cmd_buffer), TWS_FUNC_ID_RSSI_SYNC);
            }
        } else
#endif
        {
            app_core_send_rssi(-127);
        }
        return 1;
    }
    return 0;
}
#endif


void spp_data_handler(u8 packet_type, u16 ch, u8 *packet, u16 size)
{
    switch (packet_type) {
    case 1:
        log_info("spp connect\n");
        break;
    case 2:
        log_info("spp disconnect\n");
        break;
    case 7:
        //log_info("spp_rx:");
        //put_buf(packet,size);
#if AEC_DEBUG_ONLINE
        aec_debug_online(packet, size);
#endif

#if (defined TCFG_USER_RSSI_TEST_EN && TCFG_USER_RSSI_TEST_EN)
        spp_get_rssi_handler(packet, size);
#endif
        break;
    }
}

extern const char *sdk_version_info_get(void);
static u8 *bt_get_sdk_ver_info(u8 *len)
{
    char *p = sdk_version_info_get();
    if (len) {
        *len = strlen(p);
    }

    log_info("sdk_ver:%s %x\n", p, *len);
    return (u8 *)p;
}

extern void bt_set_tx_power(u8 txpower);
extern void __set_aac_bitrate(u32 bitrate);
extern void user_spp_data_handler(u8 packet_type, u16 ch, u8 *packet, u16 size);
extern void online_spp_init(void);
extern u16 get_vbat_value(void);
extern u8 get_vbat_percent(void);
extern u8 *sdfile_get_burn_code(u8 *len);

void bredr_handle_register()
{
#if (USER_SUPPORT_PROFILE_SPP==1)
#if APP_ONLINE_DEBUG
    spp_data_deal_handle_register(user_spp_data_handler);
    online_spp_init();
#else
    spp_data_deal_handle_register(spp_data_handler);
#endif
#endif
    bt_fast_test_handle_register(bt_fast_test_api);//测试盒快速测试接口
#if BT_SUPPORT_MUSIC_VOL_SYNC
    music_vol_change_handle_register(set_music_device_volume, phone_get_device_vol);
#endif
#if BT_SUPPORT_DISPLAY_BAT
    get_battery_value_register(bt_get_battery_value);   /*电量显示获取电量的接口*/
#endif

    bt_dut_test_handle_register(bt_dut_api);
}

extern void __set_support_ldac_flag(bool flag);
extern void lib_make_ble_address(u8 *ble_address, u8 *edr_address);
extern int le_controller_set_mac(void *addr);
void bt_function_select_init()
{

    /* #if TCFG_LOWEST_POWER_EN */
    void set_qos_value(u8 qos_on_value, u8 qos_off_value);
    void set_pure_monitor_eanble_qos(u8 en);
    /* set_pure_monitor_eanble_qos(1); */
    /* set_qos_value(5, 15); */
    /* #endif */
    /* set_bt_data_rate_acl_3mbs_mode(1); */
    if (CONFIG_WIFI_DETECT_ENABLE) {
        set_bt_afh_classs_enc(1);//report 手机用wifi_detect的频点,不设置用回手机
    }
    __set_user_ctrl_conn_num(TCFG_BD_NUM);
#ifdef CONFIG_CPU_BR21
    __set_support_msbc_flag(0);
#else
    __set_support_msbc_flag(1);
#endif

    __set_aac_bitrate(131 * 1000);
#if TCFG_BT_SUPPORT_AAC && (!CONFIG_A2DP_GAME_MODE_ENABLE)
    /* #if TCFG_BT_SUPPORT_AAC */
    __set_support_aac_flag(1);
#else
    __set_support_aac_flag(0);
#endif
#if TCFG_BT_SUPPORT_LDAC && (!CONFIG_A2DP_GAME_MODE_ENABLE)
    __set_support_ldac_flag(1);
    __set_a2dp_ldac_sampling_freq(LDAC_SAMPLING_FREQ_44100 | LDAC_SAMPLING_FREQ_48000 | LDAC_SAMPLING_FREQ_88200 | LDAC_SAMPLING_FREQ_96000); /*  设置ldac支持采样率，0x20//44.1KHZ,0x10//48KHZ,0x08//88.1KHZ,0x04//96KHZ */
#else
    __set_support_ldac_flag(0);
#endif
#if BT_SUPPORT_DISPLAY_BAT
    __bt_set_update_battery_time(60);
#else
    __bt_set_update_battery_time(0);
#endif
    __set_page_timeout_value(8000); /*回连搜索时间长度设置,可使用该函数注册使用，ms单位,u16*/
    __set_super_timeout_value(8000); /*回连时超时参数设置。ms单位。做主机有效*/
#if (TCFG_BD_NUM == 2)
    __set_auto_conn_device_num(2);
#endif
#if BT_SUPPORT_MUSIC_VOL_SYNC
    vol_sys_tab_init();
#endif
    //io_capabilities ; /*0: Display only 1: Display YesNo 2: KeyboardOnly 3: NoInputNoOutput*/
    //authentication_requirements: 0:not protect  1 :protect
    __set_simple_pair_param(3, 0, 2);
    bt_testbox_ex_info_get_handle_register(TESTBOX_INFO_VBAT_VALUE, get_vbat_value);
    bt_testbox_ex_info_get_handle_register(TESTBOX_INFO_VBAT_PERCENT, get_vbat_percent);
    bt_testbox_ex_info_get_handle_register(TESTBOX_INFO_BURN_CODE, sdfile_get_burn_code);
    bt_testbox_ex_info_get_handle_register(TESTBOX_INFO_SDK_VERSION, bt_get_sdk_ver_info);

#if TCFG_USER_BLE_ENABLE
    {
        u8 tmp_ble_addr[6];
#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)
        /* bt_set_tx_power(9);//ble txpwer level:0~9 */
        memcpy(tmp_ble_addr, (void *)bt_get_mac_addr(), 6);
#else
        lib_make_ble_address(tmp_ble_addr, (void *)bt_get_mac_addr());
#endif //
        le_controller_set_mac((void *)tmp_ble_addr);
        printf("\n-----edr + ble 's address-----");
        printf_buf((void *)bt_get_mac_addr(), 6);
        printf_buf((void *)tmp_ble_addr, 6);
    }
#endif // TCFG_USER_BLE_ENABLE

    /* #if (CONFIG_BT_MODE != BT_NORMAL) */
    set_bt_enhanced_power_control(1);
    /* #endif */
//#endif
#if (TCFG_USER_TWS_ENABLE==0)
    extern void  set_g_need_inc_power(u8 en);
    set_g_need_inc_power(1);
#endif
}

void phone_sync_vol(void)
{
    user_send_cmd_prepare(USER_CTRL_HFP_CALL_SET_VOLUME, 1, &bt_user_priv_var.phone_vol);
}

/*配置通话时前面丢掉的数据包包数*/
#define ESCO_DUMP_PACKET_ADJUST		1	/*配置使能*/
#define ESCO_DUMP_PACKET_DEFAULT	0
#ifdef CONFIG_NEW_BREDR_ENABLE
#define ESCO_DUMP_PACKET_CALL		60 /*0~0xFF*/
#else
#define ESCO_DUMP_PACKET_CALL		120 /*0~0xFF*/
#endif

static u8 esco_dump_packet = ESCO_DUMP_PACKET_CALL;
#if ESCO_DUMP_PACKET_ADJUST
u8 get_esco_packet_dump(void)
{
    //log_info("esco_dump_packet:%d\n", esco_dump_packet);
    return esco_dump_packet;
}
#endif

/*
 * app模式切换
 */
void task_switch(const char *name, int action)
{
    struct intent it;
    struct application *app;

    log_info("app_exit\n");

    init_intent(&it);
    app = get_current_app();
    if (app) {
        /*
         * 退出当前app, 会执行state_machine()函数中APP_STA_STOP 和 APP_STA_DESTORY
         */
        it.name = app->name;
        it.action = ACTION_BACK;
        start_app(&it);
    }

    /*
     * 切换到app (name)并执行action分支
     */
    it.name = name;
    it.action = action;
    start_app(&it);
}

#if(USE_DMA_UART_TEST) //使用dm串口测试时不能同时打开
#define MY_SNIFF_EN                   0
#else
#define MY_SNIFF_EN                   1 //默认打开
#endif

/*
 *以下情况，关闭sniff
 *(1)通过spp在线调试eq
 *(2)通过spp导出数据
 */
#if APP_ONLINE_DEBUG
#if (TCFG_EQ_ONLINE_ENABLE || (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP) || TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE)
#undef MY_SNIFF_EN
#define MY_SNIFF_EN  0
#endif
#endif/*APP_ONLINE_DEBUG*/

#define  SNIFF_CNT_TIME               10/////<空闲10S之后进入sniff模式

#define SNIFF_MAX_INTERVALSLOT        800
#define SNIFF_MIN_INTERVALSLOT        100
#define SNIFF_ATTEMPT_SLOT            4
#define SNIFF_TIMEOUT_SLOT            1

u8 sniff_ready_status = 0; //0:sniff_ready 1:sniff_not_ready
int exit_sniff_timer = 0;

bool bt_is_sniff_close(void)
{
    return (bt_user_priv_var.sniff_timer == 0);
}

void bt_check_exit_sniff()
{
    sys_timeout_del(exit_sniff_timer);
    exit_sniff_timer = 0;

#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        return;
    }
#endif
    user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
}

void bt_sniff_ready_clean(void)
{
    sniff_ready_status = 1;
}

static u8 bt_sniff_enable = 1;
void bt_check_enter_sniff()
{
    struct sniff_ctrl_config_t sniff_ctrl_config;
    u8 addr[12];
    u8 conn_cnt = 0;
    u8 i = 0;

    if (!bt_sniff_enable) {
        return;
    }

#if TCFG_AUDIO_ANC_ENABLE
    if (anc_train_open_query() || anc_online_busy_get()) { //如果ANC训练则不进入SNIFF
        return;
    }
#endif
#if (RCSP_ADV_EN)
    extern u8 get_ble_adv_modify(void);
    extern u8 get_ble_adv_notify(void);
    if (get_ble_adv_modify() || get_ble_adv_notify()) {
        return;
    }
#endif
    if (sniff_ready_status) {
        sniff_ready_status = 0;
        return;
    }
    /*putchar('H');*/
    conn_cnt = bt_api_enter_sniff_status_check(SNIFF_CNT_TIME, addr);

#if (TCFG_BD_NUM == 2)
    /* if (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status() || get_esco_coder_busy_flag()) { */
    if (get_esco_coder_busy_flag()) {
        return;
    }
#endif
    ASSERT(conn_cnt <= 2);

    for (i = 0; i < conn_cnt; i++) {
        log_info("-----USER SEND SNIFF IN %d %d\n", i, conn_cnt);
        sniff_ctrl_config.sniff_max_interval = SNIFF_MAX_INTERVALSLOT;
        sniff_ctrl_config.sniff_mix_interval = SNIFF_MIN_INTERVALSLOT;
        sniff_ctrl_config.sniff_attemp = SNIFF_ATTEMPT_SLOT;
        sniff_ctrl_config.sniff_timeout  = SNIFF_TIMEOUT_SLOT;
        memcpy(sniff_ctrl_config.sniff_addr, addr + i * 6, 6);
#if TCFG_USER_TWS_ENABLE
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            return;
        }
#endif
        user_send_cmd_prepare(USER_CTRL_SNIFF_IN, sizeof(struct sniff_ctrl_config_t), (u8 *)&sniff_ctrl_config);
    }

}
void sys_auto_sniff_controle(u8 enable, u8 *addr)
{
    if (app_var.goto_poweroff_flag) {
        return;
    }

    if (addr) {
        if (bt_api_conn_mode_check(enable, addr) == 0) {
            log_info("sniff ctr not change\n");
            return;
        }
    }

    if (enable) {
        if (addr) {
            log_info("sniff cmd timer init\n");
            user_send_cmd_prepare(USER_CTRL_HALF_SEC_LOOP_CREATE, 0, NULL);
            /* user_cmd_timer_init(); */
        }

        if (bt_user_priv_var.sniff_timer == 0) {
            log_info("check_sniff_enable\n");
            bt_user_priv_var.sniff_timer = sys_timer_add(NULL, bt_check_enter_sniff, 1000);
        }
        EARPHONE_STATE_SNIFF(0);
    } else {

        if (addr) {
            log_info("sniff cmd timer remove\n");
            user_send_cmd_prepare(USER_CTRL_HALF_SEC_LOOP_DEL, 0, NULL);
            /* remove_user_cmd_timer(); */
        }

        if (bt_user_priv_var.sniff_timer) {
            log_info("check_sniff_disable\n");
            sys_timeout_del(bt_user_priv_var.sniff_timer);
            bt_user_priv_var.sniff_timer = 0;

            if (exit_sniff_timer == 0) {
                /* exit_sniff_timer = sys_timer_add(NULL, bt_check_exit_sniff, 5000); */
            }
        }
        EARPHONE_STATE_SNIFF(1);
    }
}

u8 lmp_rx_sniff_req_check()
{
    return !bt_sniff_enable;
}

extern void tws_tx_unsniff_req(void);
void bt_sniff_set_enable(u8 en)
{
    bt_sniff_enable = en;

    if (en) {
        sys_auto_sniff_controle(1, NULL);
        tws_sniff_controle_check_enable();
    } else {
#if TCFG_USER_TWS_ENABLE
        if (get_tws_phone_connect_state() == TRUE) {
            user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
        } else {
            tws_tx_unsniff_req();
        }
        tws_sniff_controle_check_disable();
#else
        user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
#endif
        sys_auto_sniff_controle(0, NULL);
    }
}

void bt_sniff_feature_init()
{
#if TCFG_BT_SNIFF_DISABLE
    u8 feature = lmp_hci_read_local_supported_features(0);
    feature &= ~BIT(7); //BIT_SNIFF_MODE;
    lmp_hci_write_local_supported_features(feature, 0);
#endif
}

int btstack_exit();

void wait_exit_btstack_flag(void *priv)
{
    if (!bt_audio_is_running()) {
        //btstack_exit();
        lmp_hci_reset();
        os_time_dly(2);
        sys_timer_del(app_var.wait_exit_timer);
        if (priv == NULL) {
            log_info("task_switch to idle...\n");
            task_switch("idle", ACTION_IDLE_MAIN);
        } else if (priv == (void *)1) {
            log_info("cpu_reset!!!\n");
            cpu_reset();
        }

    } else {
        app_var.goto_poweroff_cnt++;
        if (app_var.goto_poweroff_cnt > 200) {
            log_info("audio:%d, status:%d, hci:%d\n",
                     bt_audio_is_running(),
                     get_curr_channel_state(),
                     hci_standard_connect_check());
            log_info("mabe ..... death!\n");
            log_info("cpu_reset!!!\n");
            cpu_reset();
        }
    }
}

#if TCFG_USER_TWS_ENABLE
static void power_off_at_same_time(void *p)
{

    if (!bt_audio_is_running()) {
        int state = tws_api_get_tws_state();
        log_info("wait_phone_link_detach: %x\n", state);
        if (state & TWS_STA_PHONE_DISCONNECTED) {
            if (state & TWS_STA_SIBLING_CONNECTED) {
                if (tws_api_get_role() == TWS_ROLE_MASTER) {
                    bt_tws_play_tone_at_same_time(SYNC_TONE_POWER_OFF, 400);
                } else {
                    /* 防止TWS异常断开导致从耳不关机 */
                    bt_tws_play_tone_at_same_time(SYNC_TONE_POWER_OFF, 2400);
                }
                sys_timer_del(app_var.wait_exit_timer);
                task_switch("idle", ACTION_IDLE_POWER_OFF);
            } else {
                sys_timer_del(app_var.wait_exit_timer);
                task_switch("idle", ACTION_IDLE_MAIN);
            }
        } else {
            lmp_hci_reset();
            os_time_dly(2);
        }
    } else {
        app_var.goto_poweroff_cnt++;
        if (app_var.goto_poweroff_cnt > 200) {
            log_info("audio:%d, status:%d, hci:%d\n",
                     bt_audio_is_running(),
                     get_curr_channel_state(),
                     hci_standard_connect_check());
            log_info("mabe ..... death!\n");
            log_info("cpu_reset!!!\n");
            cpu_reset();
        }
    }
}
#endif

extern int tws_sniff_state_check();
void poweroff_sniff_all_exit()
{
    u8 exit_cnt = 0;
    sys_auto_sniff_controle(0, NULL);
#if TCFG_USER_TWS_ENABLE
    log_info("%s %d %d %d %d\n", __func__, get_tws_phone_connect_state(), get_tws_sibling_connect_state(), bt_api_get_sniff_state(), tws_sniff_state_check());
    if (get_tws_phone_connect_state()) {
        while (bt_api_get_sniff_state() || tws_sniff_state_check()) {          //wait lmp and tws_sniff exit
            if (exit_cnt % 5 == 0) {
                user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);           //lmp_sniff exit then tws_sniff will exit
            }
            if (exit_cnt ++  >= 25) {
                exit_cnt = 0;
                break;
            }
            os_time_dly(10);        //500ms * 5
        }
    } else if (get_tws_sibling_connect_state()) {                               //if only tws connect
        while (tws_sniff_state_check()) {
            if (exit_cnt % 5 == 0) {
                tws_tx_unsniff_req();
            }
            if (exit_cnt ++ >= 25) {
                exit_cnt = 0;
                break;
            }
            os_time_dly(10);        //500ms * 5
        }
    }
    log_info("%s %d %d %d %d\n", __func__, get_tws_phone_connect_state(), get_tws_sibling_connect_state(), bt_api_get_sniff_state(), tws_sniff_state_check());
#else
    while (bt_api_get_sniff_state()) {          //wait lmp and tws_sniff exit
        if (exit_cnt % 5 == 0) {
            user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);           //lmp_sniff exit then tws_sniff will exit
        }
        if (exit_cnt ++  >= 25) {
            exit_cnt = 0;
            break;
        }
        os_time_dly(10);        //500ms * 5
    }
#endif  /*TCFG_USER_TWS_ENABLE*/
}


void sys_enter_soft_poweroff(void *priv)
{
    int detach_phone = 1;
    struct sys_event clear_key_event = {.type =  SYS_KEY_EVENT, .arg = "key"};

    log_info("%s, %d\n", __func__, (int)priv);

    if (app_var.goto_poweroff_flag) {
        return;
    }

    if ((int)priv != 4) {
#if CONFIG_BT_BACKGROUND_ENABLE
        bt_switch_to_foreground(ACTION_DO_NOTHING, 1);
#endif
        ui_update_status(STATUS_POWEROFF);
    }

    sys_auto_sniff_controle(0, NULL);
    app_var.goto_poweroff_flag = 1;
    app_var.goto_poweroff_cnt = 0;
    sys_key_event_disable();
    sys_event_clear(&clear_key_event);
    sys_auto_shut_down_disable();

    poweroff_sniff_all_exit();          //确保关机前sniff已经退出
    /* user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL); */

    EARPHONE_STATE_ENTER_SOFT_POWEROFF();

#if TCFG_AUDIO_ANC_ENABLE
    anc_poweroff();
#endif/*TCFG_AUDIO_ANC_ENABLE*/

#if TCFG_USER_TWS_ENABLE
    if ((int)priv == 3) {
        user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
        if (app_var.wait_exit_timer == 0) {
            app_var.wait_exit_timer = sys_timer_add(NULL, power_off_at_same_time, 50);
        }
        return;
    }

#if TCFG_CHARGESTORE_ENABLE
    if (chargestore_get_earphone_online() != 2) {
        bt_tws_poweroff();
    }
#else
    bt_tws_poweroff();
#endif

    int state = tws_api_get_tws_state();
    if (state & TWS_STA_PHONE_DISCONNECTED) {
        detach_phone = 0;
    }
    log_info("detach_phone: %x, %d\n", state, detach_phone);
#endif

    if (detach_phone) {
        user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
    }

    if (app_var.wait_exit_timer == 0) {
        app_var.wait_exit_timer = sys_timer_add(priv, wait_exit_btstack_flag, 50);
    }
}

static void sys_auto_shut_down_deal(void *priv)
{
    r_printf("%s\n", __func__);
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            tws_api_sync_call_by_uuid('T', SYNC_CMD_POWER_OFF_TOGETHER, 400);
        }
    } else
#endif
    {
        sys_enter_soft_poweroff(0);
    }
}

void sys_auto_shut_down_enable(void)
{
#if TCFG_AUTO_SHUT_DOWN_TIME
    log_info("sys_auto_shut_down_enable\n");
#if TCFG_AUDIO_ANC_ENABLE
    /*ANC打开，不支持自动关机*/
    if (anc_status_get()) {
        return;
    }
#endif/*TCFG_AUDIO_ANC_ENABLE*/
#if TCFG_APP_PC_EN
    /*PC模式下，不支持自动关机*/
    if (app_get_curr_task() == APP_PC_TASK) {
        log_info("APP_PC_TASK now sys_auto_shut_down_disable\n");
        return;
    }
#endif/*TCFG_APP_PC_EN*/
    if (app_var.auto_shut_down_timer == 0) {
        app_var.auto_shut_down_timer = sys_timeout_add(NULL, sys_auto_shut_down_deal, (app_var.auto_off_time * 1000));
    }
#endif
}

void sys_auto_shut_down_disable(void)
{
#if TCFG_AUTO_SHUT_DOWN_TIME
    log_info("sys_auto_shut_down_disable\n");
    if (app_var.auto_shut_down_timer) {
        sys_timeout_del(app_var.auto_shut_down_timer);
        app_var.auto_shut_down_timer = 0;
    }
#endif
}

/*static u32 len_lst[34];*/

static const u32 num0_9[] = {
    (u32)TONE_NUM_0,
    (u32)TONE_NUM_1,
    (u32)TONE_NUM_2,
    (u32)TONE_NUM_3,
    (u32)TONE_NUM_4,
    (u32)TONE_NUM_5,
    (u32)TONE_NUM_6,
    (u32)TONE_NUM_7,
    (u32)TONE_NUM_8,
    (u32)TONE_NUM_9,
} ;
static u8 check_phone_income_idle(void)
{
    if (bt_user_priv_var.phone_ring_flag) {
        return 0;
    }
    return 1;
}
REGISTER_LP_TARGET(phone_incom_lp_target) = {
    .name       = "phone_check",
    .is_idle    = check_phone_income_idle,
};

static void number_to_play_list(char *num, u32 *lst)
{
    u8 i = 0;

#if BT_PHONE_NUMBER
    if (num) {
        for (; i < strlen(num); i++) {
            lst[i] = num0_9[num[i] - '0'] ;
        }
    }
#endif

    lst[i++] = (u32)TONE_REPEAT_BEGIN(-1);
    lst[i++] = (u32)TONE_RING;
    lst[i++] = (u32)TONE_REPEAT_END();
    lst[i++] = (u32)NULL;
}

extern u8 is_siri_open();
void phone_num_play_timer(void *priv)
{
    char *len_lst[34];

    if (get_call_status() == BT_CALL_HANGUP || is_siri_open()) {
        log_info("hangup,--phone num play return\n");
        return;
    }
    log_info("%s\n", __FUNCTION__);

    if (bt_user_priv_var.phone_num_flag) {
        tone_play_stop();
        number_to_play_list((char *)(bt_user_priv_var.income_phone_num), (u32 *)len_lst);
        tone_file_list_play((const char *)len_lst, 1);
    } else {
        /*电话号码还没有获取到，定时查询*/
        bt_user_priv_var.phone_timer_id = sys_timeout_add(NULL, phone_num_play_timer, 200);
    }
}

static void phone_num_play_start(void)
{
    /* check if support inband ringtone */
    if (!bt_user_priv_var.inband_ringtone) {
        bt_user_priv_var.phone_num_flag = 0;
        bt_user_priv_var.phone_timer_id = sys_timeout_add(NULL, phone_num_play_timer, 500);
    }
}

void phone_check_inband_ring_play_timer(void *priv)
{
#if BT_INBAND_RINGTONE
    if (bt_user_priv_var.inband_ringtone && bt_user_priv_var.phone_ring_flag) {
        if (!get_esco_coder_busy_flag()) {
            r_printf("phone_check_inband_ring_play_timer play");
            bt_user_priv_var.inband_ringtone = 0;
#if TCFG_USER_TWS_ENABLE
            if (!bt_tws_sync_phone_num(NULL))
#endif
            {
                extern u8 phone_ring_play_start(void);
                phone_ring_play_start();
            }
        }
    }
#endif
}
u8 phone_ring_play_start(void)
{
    char *len_lst[34];

    if (get_call_status() == BT_CALL_HANGUP || is_siri_open()) {
        log_info("hangup,--phone ring play return\n");
        return 0;
    }
    log_info("%s,%d\n", __FUNCTION__, bt_user_priv_var.inband_ringtone);
    /* check if support inband ringtone */
    if (!bt_user_priv_var.inband_ringtone) {
        tone_play_stop();
        number_to_play_list(NULL, len_lst);
        tone_file_list_play((const char **)len_lst, 1);
        return 1;
    } else {
#if BT_INBAND_RINGTONE
        /* bt_user_priv_var.phone_timer_id = sys_timeout_add(NULL, phone_check_inband_ring_play_timer, 4000); //4s之后检测有没建立通话链路，没有建立播本地铃声 */
#endif
    }
    return 0;
}


static u16 play_poweron_ok_timer_id = 0;

static void play_poweron_ok_timer(void *priv)
{
    app_var.wait_timer_do = 0;

    log_d("\n-------play_poweron_ok_timer-------\n", priv);
    if (is_dac_power_off()) {
#if TCFG_USER_TWS_ENABLE
        bt_tws_poweron();
#else
        bt_wait_connect_and_phone_connect_switch(0);
#endif
        return;
    }

    app_var.wait_timer_do = sys_timeout_add(priv, play_poweron_ok_timer, 100);
}
static void play_bt_connect_dly(void *priv)
{
    app_var.wait_timer_do = 0;

    log_d("\n-------play_bt_connect_dly-------\n", priv);

    if (!app_var.goto_poweroff_flag) {
        STATUS *p_tone = get_tone_config();
        tone_play_index(p_tone->bt_connect_ok, 1);
    }
}



//=============================================================================//
//NOTE by MO: For fix Make, should be implemented later;
__attribute__((weak))
int earphone_a2dp_codec_get_low_latency_mode()
{
    return 0;
}

__attribute__((weak))
int earphone_a2dp_codec_set_low_latency_mode(int enable, int msec)
{
    return 0;
}
//=============================================================================//


int bt_get_low_latency_mode()
{
    return earphone_a2dp_codec_get_low_latency_mode();
}

void bt_set_low_latency_mode(int enable)
{
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            if (enable) {
                tws_api_sync_call_by_uuid('T', SYNC_CMD_LOW_LATENCY_ENABLE, 300);
            } else {
                tws_api_sync_call_by_uuid('T', SYNC_CMD_LOW_LATENCY_DISABLE, 300);
            }
        } else {
            if (earphone_a2dp_codec_set_low_latency_mode(enable, enable ? 800 : 600) == 0) {
                if (enable) {
                    tone_play(TONE_LOW_LATENCY_IN, 1);
                } else {
                    tone_play(TONE_LOW_LATENCY_OUT, 1);
                }
            }
        }
    }
#else
    earphone_a2dp_codec_set_low_latency_mode(enable, enable ? 800 : 600);
#endif
}

static void bt_discon_dly_handle(void *priv)
{
    app_var.phone_dly_discon_time = 0;

    STATUS *p_tone = get_tone_config();

#if(TCFG_BD_NUM == 2)               //对耳在bt_tws同步播放提示音
    /* tone_play(TONE_DISCONN); */
    tone_play_index(p_tone->bt_disconnect, 1);
#else

#if TCFG_USER_TWS_ENABLE
    if (!get_bt_tws_connect_status() && !get_bt_tws_discon_dly_state())
#endif
    {
        tone_play_index(p_tone->bt_disconnect, 1);
    }
#endif

#if TCFG_USER_TWS_ENABLE
    STATUS *p_led = get_led_config();
    if (get_bt_tws_connect_status()) {
#if TCFG_CHARGESTORE_ENABLE
        chargestore_set_phone_disconnect();
#endif
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            bt_tws_play_tone_at_same_time(SYNC_TONE_PHONE_DISCONNECTED, 400);
        }
    } else {
        //断开手机时，如果对耳未连接，要把LED时钟切到RC（因为单台会进SNIFF）
        pwm_led_clk_set((!TCFG_LOWPOWER_BTOSC_DISABLE) ? PWM_LED_CLK_RC32K : PWM_LED_CLK_BTOSC_24M);
        //pwm_led_clk_set(PWM_LED_CLK_RC32K);
        ui_update_status(STATUS_BT_DISCONN);
    }
#endif

}

#if TCFG_CALL_KWS_SWITCH_ENABLE
extern int audio_phone_call_kws_start(void);
extern int audio_phone_call_kws_close(void);
extern void audio_smart_voice_detect_close(void);
extern int esco_ul_stream_switch(u8 en);
extern u8 bt_phone_dec_is_running();
#endif

static void jl_call_kws_handler(int event)
{
    if (event == BT_STATUS_PHONE_INCOME) {
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
        extern void a2dp_media_suspend_resume(u8 flag);
        a2dp_media_suspend_resume(1);
        a2dp_dec_close();
        audio_aec_reboot(1);
        jl_kws_speech_recognition_open();
        jl_kws_speech_recognition_start();
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
#if TCFG_CALL_KWS_SWITCH_ENABLE
        extern void a2dp_media_suspend_resume(u8 flag);
        a2dp_media_suspend_resume(1);
        a2dp_dec_close();
        audio_aec_reboot(1);
        //esco_ul_stream_switch(0);
        audio_phone_call_kws_start();
#endif /* #if TCFG_CALL_KWS_SWITCH_ENABLE */
    } else if (event == BT_STATUS_PHONE_ACTIVE) {
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
        jl_kws_speech_recognition_close();
        audio_aec_reboot(0);
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
        printf("----aispeech_state phone active");
        ais_platform_asr_close();
        esco_mic_reset();
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/
#if TCFG_CALL_KWS_SWITCH_ENABLE
        audio_smart_voice_detect_close();
        audio_aec_reboot(0);
        //esco_ul_stream_switch(1);
#endif /* TCFG_CALL_KWS_SWITCH_ENABLE */
    } else if (event == BT_STATUS_PHONE_HANGUP) {
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
        jl_kws_speech_recognition_close();
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
#if TCFG_CALL_KWS_SWITCH_ENABLE
        if (!bt_phone_dec_is_running()) {
            audio_phone_call_kws_close();
        }
#endif /* TCFG_CALL_KWS_SWITCH_ENABLE */
    }
}

extern void tws_page_scan_deal_by_esco(u8 esco_flag);

/*
 * 对应原来的状态处理函数，连接，电话状态等
 */
void tws_conn_switch_role();
extern bool esco_opened;
extern bool transcription_opened;
extern bool earphone_connected;
static int bt_connction_status_event_handler(struct bt_event *bt)
{
    STATUS *p_tone = get_tone_config();
    u8 *phone_number = NULL;

#if CONFIG_BT_BACKGROUND_ENABLE
    int ret = bt_background_event_probe_handler(bt);
    if (ret == -EINVAL) {
        return 0;
    }
#endif
    log_info("pwt debug bt_conn_event: %d", bt->event);

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        /*
         * 蓝牙初始化完成
         */
        log_info("BT_STATUS_INIT_OK\n");
        init_ok = 1;

#if TCFG_AUDIO_DATA_EXPORT_ENABLE
#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP)
        lmp_hci_set_role_switch_supported(0);
#endif/*AUDIO_DATA_EXPORT_USE_SPP*/
        extern int audio_capture_init();
        audio_capture_init();
#endif/*TCFG_AUDIO_DATA_EXPORT_ENABLE*/

#if TCFG_AUDIO_ANC_ENABLE
        anc_poweron();
#endif/*TCFG_AUDIO_ANC_ENABLE*/
        __set_sbc_cap_bitpool(38);

#if (TCFG_USER_BLE_ENABLE)
        if (BT_MODE_IS(BT_BQB)) {
            ble_bqb_test_thread_init();
        } else {
#if !TCFG_WIRELESS_MIC_ENABLE
            bt_ble_init();
#endif

#if BLE_HID_EN && (!TCFG_USER_TWS_ENABLE)
            ble_module_enable(1);
#endif
        }

#endif

#if TCFG_USER_TWS_ENABLE
        extern void pbg_demo_init(void);
        pbg_demo_init();
#endif

#if TCFG_USER_EMITTER_ENABLE
////发射器设置回调等
        void bt_emitter_init() ;
        bt_emitter_init();
        extern u8 emitter_search_result(char *name, u8 name_len, u8 * addr, u32 dev_class, char rssi);
        inquiry_result_handle_register(emitter_search_result);
        extern void emitter_or_receiver_switch(u8 flag);
        emitter_or_receiver_switch(BT_EMITTER_EN);
        break;
#endif

        bt_init_ok_search_index();
        ui_update_status(STATUS_BT_INIT_OK);
#if TCFG_CHARGESTORE_ENABLE
        chargestore_set_bt_init_ok(1);
#endif
#if TCFG_TEST_BOX_ENABLE
        testbox_set_bt_init_ok(1);
#endif


#if ((CONFIG_BT_MODE == BT_BQB)||(CONFIG_BT_MODE == BT_PER))
        bt_wait_phone_connect_control(1);
#else
        if (is_dac_power_off()) {
#if TCFG_USER_TWS_ENABLE
            bt_tws_poweron();
#else
            bt_wait_connect_and_phone_connect_switch(0);
#endif
        } else {
            app_var.wait_timer_do = sys_timeout_add(NULL, play_poweron_ok_timer, 100);
        }
#endif

        /*if (app_var.play_poweron_tone) {
            tone_play_index(p_tone->power_on, 1);
        }*/
        break;

    case BT_STATUS_SECOND_CONNECTED:
        clear_current_poweron_memory_search_index(0);
    case BT_STATUS_FIRST_CONNECTED:
        log_info("BT_STATUS_CONNECTED\n");
        earphone_change_pwr_mode(PWR_DCDC15, 3000);
        sys_auto_shut_down_disable();
#if TCFG_ADSP_UART_ENABLE
        ADSP_UART_Init();
#endif

#if(JL_EARPHONE_APP_EN && RCSP_UPDATE_EN)
        extern u8 rcsp_update_get_role_switch(void);
        if (rcsp_update_get_role_switch()) {
            tws_conn_switch_role();
            tws_api_auto_role_switch_disable();
        }
#endif

#if(TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)
        {
            u8 connet_type;
            if (get_auto_connect_state(bt->args)) {
                connet_type = ICON_TYPE_RECONNECT;
            } else {
                connet_type = ICON_TYPE_CONNECTED;
            }

#if TCFG_USER_TWS_ENABLE
            if (tws_api_get_role() == TWS_ROLE_MASTER) {
                bt_ble_icon_open(connet_type);
            } else {
                //maybe slave already open
                bt_ble_icon_close(0);
            }

#else
            bt_ble_icon_open(connet_type);
#endif
        }
#endif


#if (TCFG_BD_NUM == 2)
        if (get_current_poweron_memory_search_index(NULL) == 0) {
            bt_wait_phone_connect_control(1);
        }
#endif

#if TCFG_USER_TWS_ENABLE
        if (!get_bt_tws_connect_status()) {   //如果对耳还未连接就连上手机，要切换闪灯状态
            ui_update_status(STATUS_BT_CONN);
        }

#if TCFG_CHARGESTORE_ENABLE
        chargestore_set_phone_connect();
#endif
        earphone_connected = true;
        bt_ble_adv_enable(1);
        printf("pwt debug BT_STATUS_FIRST_CONNECTED");

        if (bt_tws_phone_connected()) {
            break;
        }
#else
        ui_update_status(STATUS_BT_CONN);    //单台在此处设置连接状态,对耳的连接状态需要同步，在bt_tws.c中去设置
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
#endif
        /* tone_play(TONE_CONN); */


        /*os_time_dly(40); // for test*/
        log_info("tone status:%d\n", tone_get_status());
        if (get_call_status() == BT_CALL_HANGUP) {
            if (app_var.phone_dly_discon_time) {
                sys_timeout_del(app_var.phone_dly_discon_time);
                app_var.phone_dly_discon_time = 0;
            } else {
                app_var.wait_timer_do = sys_timeout_add(NULL, play_bt_connect_dly, 1600);
                /* tone_play_index(p_tone->bt_connect_ok, 1); */
            }
        }

        /*int timeout = 5000 + rand32() % 10000;
        sys_timeout_add(NULL, connect_phone_test,  timeout);*/
        break;
    case BT_STATUS_FIRST_DISCONNECT:
        earphone_connected = false;
        bt_ble_adv_enable(0);
        printf("pwt debug BT_STATUS_FIRST_DISCONNECT");
    case BT_STATUS_SECOND_DISCONNECT:
        log_info("BT_STATUS_DISCONNECT\n");
#if TCFG_ADSP_UART_ENABLE
        ADSP_UART_Deinit();
#endif
        if (app_var.goto_poweroff_flag) {
            /*关机不播断开提示音*/
            /*关机时不改UI*/
            break;
        }

#if PHONE_DLY_DISCONN_TIME
        if (app_var.phone_dly_discon_time == 0) {
            app_var.phone_dly_discon_time = sys_timeout_add(NULL, bt_discon_dly_handle, PHONE_DLY_DISCONN_TIME);
        }
#else
        bt_discon_dly_handle(NULL);
#endif
        break;

    //phone status deal
    case BT_STATUS_PHONE_INCOME:
        log_info("BT_STATUS_PHONE_INCOME\n");
        esco_dump_packet = ESCO_DUMP_PACKET_CALL;
        ui_update_status(STATUS_PHONE_INCOME);
        u8 tmp_bd_addr[6];
        memcpy(tmp_bd_addr, bt->args, 6);
        jl_call_kws_handler(BT_STATUS_PHONE_INCOME);
        /*
         *(1)1t2有一台通话的时候，另一台如果来电不要提示
         *(2)1t2两台同时来电，现来的题示，后来的不播
         */
        if ((check_esco_state_via_addr(tmp_bd_addr) != BD_ESCO_BUSY_OTHER) &&
            (bt_user_priv_var.phone_ring_flag == 0)) {

#if BT_INBAND_RINGTONE
            extern u8 get_device_inband_ringtone_flag(void);
            bt_user_priv_var.inband_ringtone = get_device_inband_ringtone_flag();
#else
            bt_user_priv_var.inband_ringtone = 0 ;
            lmp_private_esco_suspend_resume(3);
#endif

            g_printf("bt_user_priv_var.inband_ringtone=0x%x\n", bt_user_priv_var.inband_ringtone);
            bt_user_priv_var.phone_ring_flag = 1;
            bt_user_priv_var.phone_income_flag = 1;
#if TCFG_USER_TWS_ENABLE
            if (!bt_tws_sync_phone_num(NULL))
#endif
            {
#if BT_PHONE_NUMBER
                phone_num_play_start();
#else
                phone_ring_play_start();
#endif

            }
            user_send_cmd_prepare(USER_CTRL_HFP_CALL_CURRENT, 0, NULL); //发命令获取电话号码
        } else {
            log_info("SCO busy now:%d,%d\n", check_esco_state_via_addr(tmp_bd_addr),
                     bt_user_priv_var.phone_ring_flag);
        }
        break;
    case BT_STATUS_PHONE_OUT:
        log_info("BT_STATUS_PHONE_OUT\n");
        esco_dump_packet = ESCO_DUMP_PACKET_CALL;
        ui_update_status(STATUS_PHONE_OUT);
        bt_user_priv_var.phone_income_flag = 0;
        user_send_cmd_prepare(USER_CTRL_HFP_CALL_CURRENT, 0, NULL); //发命令获取电话号码
        break;
    case BT_STATUS_PHONE_ACTIVE:
        jl_call_kws_handler(BT_STATUS_PHONE_ACTIVE);
        log_info("BT_STATUS_PHONE_ACTIVE\n");
        ui_update_status(STATUS_PHONE_ACTIV);
        if (bt_user_priv_var.phone_call_dec_begin) {
            log_info("call_active,dump_packet clear\n");
            esco_dump_packet = ESCO_DUMP_PACKET_DEFAULT;
        }
        if (bt_user_priv_var.phone_ring_flag) {
            bt_user_priv_var.phone_ring_flag = 0;
            tone_play_stop();
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
        //phone_sync_vol();
#if TCFG_USER_TWS_ENABLE
        bt_tws_sync_volume();
#endif
        r_printf("phone_active:%d\n", app_var.call_volume);
        app_audio_set_volume(APP_AUDIO_STATE_CALL, app_var.call_volume, 1);
#if TCFG_EAR_DETECT_ENABLE
#if TCFG_EAR_DETECT_CALL_CTL_SCO
        ear_detect_phone_active_deal();
#endif
#if (TCFG_EAR_DETECT_AUTO_CHG_MASTER && TCFG_USER_TWS_ENABLE)
        ear_detect_call_chg_master_deal();
#endif //TCFG_EAR_DETECT_AUTO_CHG_MASTER
#endif // TCFG_EAR_DETECT_ENABLE

        break;
    case BT_STATUS_PHONE_HANGUP:
        esco_dump_packet = ESCO_DUMP_PACKET_CALL;
        log_info("phone_handup\n");
        jl_call_kws_handler(BT_STATUS_PHONE_HANGUP);
        if (bt_user_priv_var.phone_ring_flag) {
            bt_user_priv_var.phone_ring_flag = 0;
            tone_play_stop();
            if (bt_user_priv_var.phone_timer_id) {
                sys_timeout_del(bt_user_priv_var.phone_timer_id);
                bt_user_priv_var.phone_timer_id = 0;
            }
        }
        bt_user_priv_var.phone_income_flag = 0;
        bt_user_priv_var.phone_num_flag = 0;
        bt_user_priv_var.phone_con_sync_num_ring = 0;
        bt_user_priv_var.phone_con_sync_ring = 0;
        lmp_private_esco_suspend_resume(4);
#if (TCFG_EAR_DETECT_ENABLE && TCFG_EAR_DETECT_AUTO_CHG_MASTER && TCFG_USER_TWS_ENABLE)
        ear_detect_change_master_timer_del(); //假如通话结束了，但是还没开始切换主从，取消切换
#endif
        break;
    case BT_STATUS_PHONE_NUMBER:
        log_info("BT_STATUS_PHONE_NUMBER\n");
        phone_number = (u8 *)bt->value;
        //put_buf(phone_number, strlen((const char *)phone_number));
        if (bt_user_priv_var.phone_num_flag == 1) {
            break;
        }
        bt_user_priv_var.income_phone_len = 0;
        memset(bt_user_priv_var.income_phone_num, '\0', sizeof(bt_user_priv_var.income_phone_num));
        for (int i = 0; i < strlen((const char *)phone_number); i++) {
            if (phone_number[i] >= '0' && phone_number[i] <= '9') {
                //过滤，只有数字才能报号
                bt_user_priv_var.income_phone_num[bt_user_priv_var.income_phone_len++] = phone_number[i];
                if (bt_user_priv_var.income_phone_len >= sizeof(bt_user_priv_var.income_phone_num)) {
                    break;    /*buffer 空间不够，后面不要了*/
                }
            }
        }
        if (bt_user_priv_var.income_phone_len > 0) {
            bt_user_priv_var.phone_num_flag = 1;
        } else {
            log_info("PHONE_NUMBER len err\n");
        }
        break;
    case BT_STATUS_INBAND_RINGTONE:
        log_info("BT_STATUS_INBAND_RINGTONE\n");
#if BT_INBAND_RINGTONE
        bt_user_priv_var.inband_ringtone = bt->value;
#else
        bt_user_priv_var.inband_ringtone = 0;
#endif
        printf("bt_user_priv_var.inband_ringtone=0x%x\n", bt_user_priv_var.inband_ringtone);
        break;


    case BT_STATUS_BEGIN_AUTO_CON:
        log_info("BT_STATUS_BEGIN_AUTO_CON\n");
        break;
    case BT_STATUS_A2DP_MEDIA_START:
        log_info(" BT_STATUS_A2DP_MEDIA_START:0x%x", bt->value);
#if TCFG_AUDIO_CVP_DUT_ENABLE
        if (audio_dec_dut_en_get(0)) {
            a2dp_dut_dec_open(bt->value);
        } else
#endif /*TCFG_AUDIO_CVP_DUT_ENABLE*/
        {
            a2dp_dec_open(bt->value);
        }
        break;
    case BT_STATUS_A2DP_MEDIA_STOP:
        log_info(" BT_STATUS_A2DP_MEDIA_STop");
        a2dp_dec_close();

        extern void free_a2dp_using_decoder_conn();
        free_a2dp_using_decoder_conn();
        break;
    case BT_STATUS_SCO_STATUS_CHANGE:
        printf(" BT_STATUS_SCO_STATUS_CHANGE len:%d ,type:%d", (bt->value >> 16), (bt->value & 0x0000ffff));
        if (bt->value != 0xff) {
            esco_opened = true;
            puts("<<<<<<<<<<<esco_dec_stat\n");

            void aac_decoder_energy_det_close();
            aac_decoder_energy_det_close();
#if 0   //debug
            void mic_capless_feedback_toggle(u8 toggle);
            mic_capless_feedback_toggle(0);
#endif

#if PHONE_CALL_USE_LDO15
            if ((get_chip_version() & 0xff) <= 0x04) { //F版芯片以下的，通话需要使用LDO模式
                power_set_mode(PWR_LDO15);
            }
#endif
            /*phone_call_begin(&bt->value);*/
            esco_dec_open(&bt->value, 0);

            bt_user_priv_var.phone_call_dec_begin = 1;
            if (get_call_status() == BT_CALL_ACTIVE) {
                log_info("dec_begin,dump_packet clear\n");
                esco_dump_packet = ESCO_DUMP_PACKET_DEFAULT;
            }
#if TCFG_USER_TWS_ENABLE
            tws_page_scan_deal_by_esco(1);
            pbg_user_mic_fixed_deal(1);
#endif
        } else {
            esco_opened = false;
            puts("<<<<<<<<<<<esco_dec_stop\n");
            bt_user_priv_var.phone_call_dec_begin = 0;
            esco_dump_packet = ESCO_DUMP_PACKET_CALL;
            esco_dec_close();

#if TCFG_USER_TWS_ENABLE
            tws_page_scan_deal_by_esco(0);
            pbg_user_mic_fixed_deal(0);
#endif
#if PHONE_CALL_USE_LDO15
            if ((get_chip_version() & 0xff) <= 0x04) { //F版芯片以下的，通话需要使用LDO模式
                power_set_mode(TCFG_LOWPOWER_POWER_SEL);
            }
#endif
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
            extern u8 get_speak_to_chat_state();
            if (get_speak_to_chat_state()) {
                clk_set_sys_lock(48 * 1000000L, 2);
            } else
#endif
            {
                if (transcription_opened) {
                    clk_set_sys_lock(96 * (1000000L), 0);
                } else {
                    clk_set_sys_lock(BT_NORMAL_HZ, 2);
                }
            }
        }
        break;
    case BT_STATUS_CALL_VOL_CHANGE:
        printf("call_vol:%d", bt->value);
        //u8 volume = app_audio_get_max_volume() * bt->value / 15;
        u8 volume = bt->value;
        u8 call_status = get_call_status();

#if BT_INBAND_RINGTONE
        if ((call_status != BT_CALL_ACTIVE) && (bt->value == 0)) {
            printf("not set vol\n");
            break;
        }
#endif

        bt_user_priv_var.phone_vol = bt->value;
        if ((call_status == BT_CALL_ACTIVE) || (call_status == BT_CALL_OUTGOING) || app_var.siri_stu) {
            app_audio_set_volume(APP_AUDIO_STATE_CALL, volume, 1);
#if TCFG_USER_TWS_ENABLE
            if (call_status == BT_CALL_ACTIVE) {
                bt_tws_sync_volume();
            }
#endif
        } else { /*if (call_status != BT_CALL_HANGUP)*/
            /*
             *只保存，不设置到dac
             *微信语音通话的时候，会更新音量到app，但是此时的call status可能是hangup
             */
            app_var.call_volume = volume;
        }
        break;
    case BT_STATUS_SNIFF_STATE_UPDATE:
        log_info(" BT_STATUS_SNIFF_STATE_UPDATE %d\n", bt->value);    //0退出SNIFF
        if (bt->value == 0) {
            sniff_out = 1;
#if defined CFG_LED_MODE && (CFG_LED_MODE == CFG_LED_SOFT_MODE)
            pwm_led_mode_set(pwm_led_display_mode_get());
#endif
#if(TCFG_USER_TWS_ENABLE == 1)
            led_module_exit_sniff_mode();
            pwm_led_clk_set(PWM_LED_CLK_BTOSC_24M);
#endif
            sys_auto_sniff_controle(MY_SNIFF_EN, bt->args);
        } else {
#if(TCFG_USER_TWS_ENABLE == 1)
            led_module_enter_sniff_mode();
            pwm_led_clk_set((!TCFG_LOWPOWER_BTOSC_DISABLE) ? PWM_LED_CLK_RC32K : PWM_LED_CLK_BTOSC_24M);
#endif
            sys_auto_sniff_controle(0, bt->args);
        }
        break;

    case BT_STATUS_LAST_CALL_TYPE_CHANGE:
        log_info("BT_STATUS_LAST_CALL_TYPE_CHANGE:%d\n", bt->value);
        bt_user_priv_var.last_call_type = bt->value;
        break;

    case BT_STATUS_CONN_A2DP_CH:
    case BT_STATUS_CONN_HFP_CH:

        if ((!is_1t2_connection()) && (get_current_poweron_memory_search_index(NULL))) { //回连下一个device
            if (get_esco_coder_busy_flag()) {
                clear_current_poweron_memory_search_index(0);
            } else {
                user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
            }
        }
        break;
    case BT_STATUS_PHONE_MANUFACTURER:
        log_info("BT_STATUS_PHONE_MANUFACTURER:%d\n", bt->value);
        extern const u8 hid_conn_depend_on_dev_company;
        app_var.remote_dev_company = bt->value;
        if (hid_conn_depend_on_dev_company) {
            if (bt->value) {
                //user_send_cmd_prepare(USER_CTRL_HID_CONN, 0, NULL);
            } else {

                if (get_curr_channel_state() == HID_CH) {
                    //如果只连上hid 先不断开
                } else {
                    user_send_cmd_prepare(USER_CTRL_HID_DISCONNECT, 0, NULL);
                }
            }
        }
        break;
    case BT_STATUS_VOICE_RECOGNITION:
        log_info(" BT_STATUS_VOICE_RECOGNITION:%d \n", bt->value);
        /* put_buf(bt, sizeof(struct bt_event)); */
        app_var.siri_stu = bt->value;
        esco_dump_packet = ESCO_DUMP_PACKET_DEFAULT;
        break;
    case BT_STATUS_AVRCP_INCOME_OPID:
#define AVC_VOLUME_UP			0x41
#define AVC_VOLUME_DOWN			0x42
        log_info("BT_STATUS_AVRCP_INCOME_OPID:%d\n", bt->value);
        if (bt->value == AVC_VOLUME_UP) {

        }
        if (bt->value == AVC_VOLUME_DOWN) {

        }
        break;
    default:
        log_info(" BT STATUS DEFAULT\n");
        break;
    }
    return 0;
}




static void sys_time_auto_connection_deal(void *arg)
{
    if (bt_user_priv_var.auto_connection_counter) {
        log_info("------------------------auto_timeout conuter %d", bt_user_priv_var.auto_connection_counter);
        bt_user_priv_var.auto_connection_counter--;
        user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, bt_user_priv_var.auto_connection_addr);
    }
}

static void bt_hci_event_inquiry(struct bt_event *bt)
{
#if TCFG_USER_EMITTER_ENABLE
    if (bt_user_priv_var.emitter_or_receiver == BT_EMITTER_EN) {
        extern void emitter_search_stop();
        emitter_search_stop();
    }
#endif
}

static void bt_hci_event_connection(struct bt_event *bt)
{
    //clk_set_sys_lock(BT_CONNECT_HZ, 0);
    log_info("tws_conn_state=%d\n", bt_user_priv_var.tws_conn_state);

#if TCFG_USER_TWS_ENABLE
    bt_tws_hci_event_connect();
#ifndef CONFIG_NEW_BREDR_ENABLE
    tws_try_connect_disable();
#endif
#else
    if (bt_user_priv_var.auto_connection_timer) {
        sys_timeout_del(bt_user_priv_var.auto_connection_timer);
        bt_user_priv_var.auto_connection_timer = 0;
    }
    bt_user_priv_var.auto_connection_counter = 0;
    bt_wait_phone_connect_control(0);
    user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
#endif

}

extern void bt_get_vm_mac_addr(u8 *addr);
static void bt_discovery_and_connectable_using_loca_mac_addr(u8 inquiry_scan_en, u8 page_scan_en)
{

    u8 local_addr[6];
    log_info("<<<<<<<<<<<wait bt discon,inquiry_scan:%x pasge_scan:%x>>>>>>>>>>>>>\n", inquiry_scan_en, page_scan_en);

    bt_get_vm_mac_addr(local_addr);
    //log_info("local_adr\n");
    //put_buf(local_addr,6);

    lmp_hci_write_local_address(local_addr);
    if (page_scan_en) {
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
    }
    if (inquiry_scan_en) {
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
    }
}

static void bt_hci_event_disconnect(struct bt_event *bt)
{
    if (app_var.goto_poweroff_flag) {
        return;
    }
    earphone_change_pwr_mode(PWR_LDO15, 0);

    /* if (get_total_connect_dev() == 0) {    //已经没有设备连接 */
    /*     sys_auto_shut_down_enable(); */
    /* } */

    log_info("<<<<<<<<<<<<<<total_dev: %d>>>>>>>>>>>>>\n", get_total_connect_dev());
    if (!get_curr_channel_state()) {
        /* if (get_total_connect_dev() == 0) {    //已经没有设备连接 */
#if (TCFG_USER_TWS_ENABLE == 0)
        if (!app_var.goto_poweroff_flag) { /*关机时不改UI*/
            ui_update_status(STATUS_BT_DISCONN);
        }
#endif
        sys_auto_shut_down_enable();
    }
#if (TCFG_BD_NUM == 2)
    log_info("get_bt_connect_status = 0x%x,%x\n", get_bt_connect_status(), get_curr_channel_state());
    if (!get_curr_channel_state()) {
        sys_auto_shut_down_enable();
    }
#endif

#if TCFG_TEST_BOX_ENABLE
    if (testbox_get_ex_enter_dut_flag()) {
        bt_discovery_and_connectable_using_loca_mac_addr(1, 1);
        return;
    }

    if (testbox_get_status()) {
        bt_discovery_and_connectable_using_loca_mac_addr(0, 1);
        return;
    }
#endif

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

#if (TCFG_BD_NUM == 2)
    if ((bt->event == ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR)
        || (bt->event == ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED)
        || (bt->event == ERROR_CODE_ROLE_SWITCH_FAILED)
        || (bt->event == ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS)) {
        /*
         *连接接受超时
         *如果支持1t2，可以选择继续回连下一台，除非已经回连完毕
         */
        if (get_current_poweron_memory_search_index(NULL)) {
            user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
            return;
        }
    }
#endif

#if TCFG_USER_TWS_ENABLE
    if (bt->value == ERROR_CODE_CONNECTION_TIMEOUT) {
        bt_tws_phone_connect_timeout();
    } else {
        bt_tws_phone_disconnected();
    }
#else
    bt_wait_phone_connect_control(1);
#endif
}

static void bt_hci_event_linkkey_missing(struct bt_event *bt)
{
#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)
#if (CONFIG_NO_DISPLAY_BUTTON_ICON && TCFG_CHARGESTORE_ENABLE)
    //已取消配对了
    if (bt_ble_icon_get_adv_state() == ADV_ST_RECONN) {
        //切换广播
        log_info("switch_INQ\n");
        bt_ble_icon_open(ICON_TYPE_INQUIRY);
    }
#endif
#endif
}

static void bt_hci_event_page_timeout()
{

    log_info("------------------------HCI_EVENT_PAGE_TIMEOUT conuter %d", bt_user_priv_var.auto_connection_counter);
#if TCFG_USER_TWS_ENABLE
    bt_tws_phone_page_timeout();
#else
    /* if (!bt_user_priv_var.auto_connection_counter) { */
    if (bt_user_priv_var.auto_connection_timer) {
        sys_timer_del(bt_user_priv_var.auto_connection_timer);
        bt_user_priv_var.auto_connection_timer = 0;
    }

#if (TCFG_BD_NUM == 2)
    if (get_current_poweron_memory_search_index(NULL)) {
        user_send_cmd_prepare(USER_CTRL_START_CONNECTION, 0, NULL);
    }
#endif

    bt_wait_phone_connect_control(1);
#endif
}

static void bt_hci_event_connection_timeout(struct bt_event *bt)
{
    earphone_change_pwr_mode(PWR_LDO15, 0);

#if TCFG_TEST_BOX_ENABLE
    if (testbox_get_ex_enter_dut_flag()) {
        bt_discovery_and_connectable_using_loca_mac_addr(1, 1);
        return;
    }
#endif

    if (!get_remote_test_flag() && !get_esco_busy_flag()) {
        bt_user_priv_var.auto_connection_counter = (TIMEOUT_CONN_TIME * 1000);
        memcpy(bt_user_priv_var.auto_connection_addr, bt->args, 6);
#if TCFG_USER_TWS_ENABLE
        bt_tws_phone_connect_timeout();
#else

#if ((CONFIG_BT_MODE == BT_BQB)||(CONFIG_BT_MODE == BT_PER))
        bt_wait_phone_connect_control(1);
#else
        user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
#if (TCFG_BD_NUM == 2)
        if (bt_user_priv_var.auto_connection_timer) {
            log_info("1To2 had registered timer,del it\n");//1To2 存在两个手机都断连的情况，所以要判断,删掉第一个手机断连时候的定时器
            sys_timeout_del(bt_user_priv_var.auto_connection_timer);
            bt_user_priv_var.auto_connection_timer = 0;
        }
#endif
        bt_wait_connect_and_phone_connect_switch(0);
#endif
        //user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, bt->args);
#endif
    } else {
#if TCFG_USER_TWS_ENABLE
        bt_tws_phone_disconnected();
#else
        bt_wait_phone_connect_control(1);
#endif

    }
}

static void bt_hci_event_connection_exist(struct bt_event *bt)
{

    if (!get_remote_test_flag() && !get_esco_busy_flag()) {
        bt_user_priv_var.auto_connection_counter = (8 * 1000);
        memcpy(bt_user_priv_var.auto_connection_addr, bt->args, 6);
#if TCFG_USER_TWS_ENABLE
        bt_tws_phone_connect_timeout();
#else
        if (bt_user_priv_var.auto_connection_timer) {
            sys_timer_del(bt_user_priv_var.auto_connection_timer);
            bt_user_priv_var.auto_connection_timer = 0;
        }
        bt_wait_connect_and_phone_connect_switch(0);
        //user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, bt->args);
#endif
    } else {
#if TCFG_USER_TWS_ENABLE
        bt_tws_phone_disconnected();
#else
        bt_wait_phone_connect_control(1);
#endif

    }
}



void eartch_testbox_flag(u8 flag);

enum {
    TEST_STATE_INIT = 1,
    TEST_STATE_EXIT = 3,
};

enum {
    ITEM_KEY_STATE_DETECT = 0,
    ITEM_IN_EAR_DETECT,
};

static u8 in_ear_detect_test_flag = 0;
static void testbox_in_ear_detect_test_flag_set(u8 flag)
{
    in_ear_detect_test_flag = flag;
}

u8 testbox_in_ear_detect_test_flag_get(void)
{
    return in_ear_detect_test_flag;
}

static void bt_in_ear_detection_test_state_handle(u8 state, u8 *value)
{
    switch (state) {
    case TEST_STATE_INIT:
        testbox_in_ear_detect_test_flag_set(1);

#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
        eartch_testbox_flag(1);
#endif
        //start trim
        break;
    case TEST_STATE_EXIT:
        testbox_in_ear_detect_test_flag_set(0);
#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
        eartch_testbox_flag(0);
#endif
        break;
    }
}

static void bt_vendor_meta_event_handle(u8 sub_evt, u8 *arg, u8 len)
{
    log_info("vendor event:%x\n", sub_evt);
    log_info_hexdump(arg, 6);

    if (sub_evt != HCI_SUBEVENT_VENDOR_TEST_MODE_CFG) {
        log_info("unknow_sub_evt:%x\n", sub_evt);
        return;
    }

    u8 test_item = arg[0];
    u8 state = arg[1];

    if (ITEM_IN_EAR_DETECT == test_item) {
        bt_in_ear_detection_test_state_handle(state, NULL);
    }
}

extern void set_remote_test_flag(u8 own_remote_test);

static int bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("------------------------bt_hci_event_handler reason %x %x", bt->event, bt->value);

    if (bt->event == HCI_EVENT_VENDOR_REMOTE_TEST) {
        if (VENDOR_TEST_DISCONNECTED == bt->value) {
#if TCFG_TEST_BOX_ENABLE
            if (testbox_get_status()) {
                if (get_remote_test_flag()) {
                    testbox_clear_connect_status();
                }
            }
#endif
            set_remote_test_flag(0);
            log_info("clear_test_box_flag");
            cpu_reset();
            return 0;
        } else {

#if TCFG_USER_BLE_ENABLE
#if (CONFIG_BT_MODE == BT_NORMAL)
            //1:edr con;2:ble con;
            if (VENDOR_TEST_LEGACY_CONNECTED_BY_BT_CLASSIC == bt->value) {
                extern void bt_ble_adv_enable(u8 enable);
                bt_ble_adv_enable(0);
            }
#endif
#endif
#if TCFG_USER_TWS_ENABLE
            if (VENDOR_TEST_CONNECTED_WITH_TWS != bt->value) {
                bt_tws_poweroff();
            }
#endif
        }
    }

    /*     if (((bt->event != HCI_EVENT_CONNECTION_COMPLETE) && (bt->event != HCI_EVENT_VENDOR_REMOTE_TEST)) \ */
    /*         || ((bt->event == HCI_EVENT_CONNECTION_COMPLETE) && (bt->value != ERROR_CODE_SUCCESS))) { */
    /* #if TCFG_TEST_BOX_ENABLE */
    /*         if (testbox_get_status()) { */
    /*             if (get_remote_test_flag()) { */
    /*                 chargestore_clear_connect_status(); */
    /*             } */
    /*             //return 0; */
    /*         } */
    /* #endif */
    /*     } */

    switch (bt->event) {
    case HCI_EVENT_VENDOR_META:
        bt_vendor_meta_event_handle(bt->value, bt->args, 6);
        break;
    case HCI_EVENT_INQUIRY_COMPLETE:
        log_info(" HCI_EVENT_INQUIRY_COMPLETE \n");
        bt_hci_event_inquiry(bt);
        break;
    case HCI_EVENT_USER_CONFIRMATION_REQUEST:
        log_info(" HCI_EVENT_USER_CONFIRMATION_REQUEST \n");
        ///<可通过按键来确认是否配对 1：配对   0：取消
        bt_send_pair(1);
        break;
    case HCI_EVENT_USER_PASSKEY_REQUEST:
        log_info(" HCI_EVENT_USER_PASSKEY_REQUEST \n");
        ///<可以开始输入6位passkey
        break;
    case HCI_EVENT_USER_PRESSKEY_NOTIFICATION:
        log_info(" HCI_EVENT_USER_PRESSKEY_NOTIFICATION %x\n", bt->value);
        ///<可用于显示输入passkey位置 value 0:start  1:enrer  2:earse   3:clear  4:complete
        break;
    case HCI_EVENT_PIN_CODE_REQUEST :
        log_info("HCI_EVENT_PIN_CODE_REQUEST  \n");
        bt_send_pair(1);
        break;

    case HCI_EVENT_VENDOR_NO_RECONN_ADDR :
        log_info("HCI_EVENT_VENDOR_NO_RECONN_ADDR \n");
        bt_hci_event_disconnect(bt) ;
        break;

    case HCI_EVENT_DISCONNECTION_COMPLETE :
        log_info("HCI_EVENT_DISCONNECTION_COMPLETE \n");
        bt_hci_event_disconnect(bt) ;
        break;

    case BTSTACK_EVENT_HCI_CONNECTIONS_DELETE:
    case HCI_EVENT_CONNECTION_COMPLETE:
#if PHONE_DLY_DISCONN_TIME
        if (bt->value == ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION) {
            if (app_var.phone_dly_discon_time) {
                sys_timeout_del(app_var.phone_dly_discon_time);
                bt_discon_dly_handle(NULL);
            }
        }
#endif
        log_info(" HCI_EVENT_CONNECTION_COMPLETE \n");
        switch (bt->value) {
        case ERROR_CODE_SUCCESS :
            log_info("ERROR_CODE_SUCCESS  \n");
            testbox_in_ear_detect_test_flag_set(0);
            bt_hci_event_connection(bt);
            break;
        case ERROR_CODE_PIN_OR_KEY_MISSING:
            log_info(" ERROR_CODE_PIN_OR_KEY_MISSING \n");
            bt_hci_event_linkkey_missing(bt);

        case ERROR_CODE_SYNCHRONOUS_CONNECTION_LIMIT_TO_A_DEVICE_EXCEEDED :
        case ERROR_CODE_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES:
        case ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR:
        case ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED  :
        case ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION   :
        case ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST :
        case ERROR_CODE_AUTHENTICATION_FAILURE :
            //case CUSTOM_BB_AUTO_CANCEL_PAGE:
            bt_hci_event_disconnect(bt) ;
            break;

        case ERROR_CODE_PAGE_TIMEOUT:
            log_info(" ERROR_CODE_PAGE_TIMEOUT \n");
            bt_hci_event_page_timeout();
            break;

        case ERROR_CODE_CONNECTION_TIMEOUT:
            log_info(" ERROR_CODE_CONNECTION_TIMEOUT \n");
            bt_hci_event_connection_timeout(bt);
            break;

        case ERROR_CODE_ROLE_SWITCH_FAILED:
            log_info("ERROR_CODE_ROLE_SWITCH_FAILED   \n");
        case ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS  :
            log_info("ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS   \n");
            bt_hci_event_connection_exist(bt);
            break;
        default:
            break;

        }
        break;
    default:
        break;

    }
    return 0;
}


//恢复前台运行
static void earphone_run_at_foreground()
{
    sys_auto_shut_down_disable();
}




/*
 * 系统事件处理函数
 */
static int event_handler(struct application *app, struct sys_event *event)
{
    if (SYS_EVENT_REMAP(event)) {
        g_printf("****SYS_EVENT_REMAP**** \n");
        return 0;
    }

    switch (event->type) {
    case SYS_KEY_EVENT:
        /*
         * 普通按键消息处理
         */
        if (bt_user_priv_var.fast_test_mode) {
            audio_adc_mic_demo_close();
            tone_play_index(IDEX_TONE_NORMAL, 1);
        }
#if CONFIG_BT_BACKGROUND_ENABLE
        if (bt_in_background()) {
            break;
        }
#endif
        app_earphone_key_event_handler(event);
        break;

    case SYS_BT_EVENT:
        /*
         * 蓝牙事件处理
         */
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            bt_hci_event_handler(&event->u.bt);
        }
#if TCFG_ADSP_UART_ENABLE
        else if (!strcmp(event->arg, "UART")) {
            extern void bt_uart_command_handler(struct uart_event * event);
            bt_uart_command_handler(&event->u.uart);
        } else if (!strcmp(event->arg, "UART_CMD")) {
            extern void adsp_uart_command_event_handle(struct uart_cmd_event * uart_cmd);
            adsp_uart_command_event_handle(&event->u.uart_cmd);
        }
#endif
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            /*
             * tws事件处理函数
             */
            bt_tws_connction_status_event_handler(&event->u.bt);
        }
#endif
        break;
    case SYS_DEVICE_EVENT:
        /*
         * 系统设备事件处理
         */
        if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
#if TCFG_CHARGE_ENABLE
            return app_charge_event_handler(&event->u.dev);
#endif
#if (TCFG_ONLINE_ENABLE || TCFG_CFG_TOOL_ENABLE)
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CFG_TOOL) {
            extern int app_cfg_tool_event_handler(struct cfg_tool_event * cfg_tool_dev);
            app_cfg_tool_event_handler(&event->u.cfg_tool);
#endif
#if TCFG_ONLINE_ENABLE
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CI_UART) {
            ci_data_rx_handler(CI_UART);
#if TCFG_USER_TWS_ENABLE
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CI_TWS) {
            ci_data_rx_handler(CI_TWS);
#endif
#endif
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev);
        }
#if TCFG_CHARGESTORE_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_CHARGE_STORE) {
            app_chargestore_event_handler(&event->u.chargestore);
        }
#endif
#if TCFG_UMIDIGI_BOX_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_UMIDIGI_CHARGE_STORE) {
            app_umidigi_chargestore_event_handler(&event->u.umidigi_chargestore);
        }
#endif
#if TCFG_TEST_BOX_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_TEST_BOX) {
            app_testbox_event_handler(&event->u.testbox);
        }
#endif
#if TCFG_EAR_DETECT_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_FROM_EAR_DETECT) {
            ear_detect_event_handle(event->u.ear.value);
        }
#endif
#if TCFG_ANC_BOX_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_FROM_ANC) {
            return app_ancbox_event_handler(&event->u.ancbox);
        }
#endif
#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_FROM_EARTCH) {
            extern void eartch_event_handle(u8 state);
            eartch_event_handle(event->u.ear.value);
        }
#endif /* #if TCFG_EARTCH_EVENT_HANDLE_ENABLE */

#if APP_ONLINE_DEBUG
        else if ((u32)event->arg == DEVICE_EVENT_ONLINE_DATA) {
            extern void app_online_event_handle(int evt_value);
            app_online_event_handle(event->u.dev.value);
        }
#endif /* #if TCFG_EARTCH_EVENT_HANDLE_ENABLE */

        break;

#if TCFG_USER_TWS_ENABLE
    case SYS_IR_EVENT:
        g_printf("[SYS_IR_EVENT]");
#if TCFG_USER_TWS_ENABLE
        if (get_bt_tws_connect_status()) {
            if (event->u.ir.event == IR_SENSOR_EVENT_FAR) {  //FAR
                tws_api_sync_call_by_uuid('T', SYNC_CMD_IRSENSOR_EVENT_FAR, 500);
            } else if (event->u.ir.event == IR_SENSOR_EVENT_NEAR) { //NEAR
                tws_api_sync_call_by_uuid('T', SYNC_CMD_IRSENSOR_EVENT_NEAR, 500);
            }
        } else
#endif
        {
            if (event->u.ir.event == IR_SENSOR_EVENT_FAR) {  //FAR
                g_printf("TWS FAR STOP MUSIC PLAY");
                g_printf("a2dp_get_status = %d", a2dp_get_status());
                if (a2dp_get_status() == BT_MUSIC_STATUS_STARTING) {
                    r_printf("STOP...\n");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL);
                }
            } else if (event->u.ir.event == IR_SENSOR_EVENT_NEAR) { //NEAR
                g_printf("TWS NEAR START MUSIC PLAY");
                g_printf("a2dp_get_status = %d", a2dp_get_status());
                if (a2dp_get_status() != BT_MUSIC_STATUS_STARTING) {
                    r_printf("START...\n");
                    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
                }
            }
        }

        return 0;

    case SYS_PBG_EVENT:
        pbg_user_event_deal(&event->u.pbg);
        break;
#endif
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    /*空间音频校准*/
    case SYS_AUD_EVENT:
        if ((u32)event->arg == AUDIO_EVENT_TRIM_IMU_START) {
            extern int spatial_imu_trim_start();
            spatial_imu_trim_task_start();
        } else if ((u32)event->arg == AUDIO_EVENT_TRIM_IMU_STOP) {
            extern int spatial_imu_trim_task_stop();
            spatial_imu_trim_task_stop();
        }
        break;
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

    default:
        return false;
    }

    SYS_EVENT_HANDLER_SPECIFIC(event);
#ifdef CONFIG_BT_BACKGROUND_ENABLE
    if (app) {
        default_event_handler(event);
    }
#endif
    return false;
}

int bt_in_background_event_handler(struct sys_event *event)
{
    return event_handler(NULL, event);
}

u8 bt_app_exit_check(void)
{
    int esco_state;
    if (app_var.siri_stu && app_var.siri_stu != 3) {
        // siri不退出
        return 0;
    }
    esco_state = get_call_status();
    if (esco_state == BT_CALL_OUTGOING  ||
        esco_state == BT_CALL_ALERT     ||
        esco_state == BT_CALL_INCOMING  ||
        esco_state == BT_CALL_ACTIVE) {
        // 通话不退出
        return 0;
    }

    return 1;
}

int bt_app_exit(void *priv)
{
    int a2dp_state;
    init_ok = 0;
    a2dp_state = a2dp_get_status();
    extern u8 bt_media_is_running();
    extern void a2dp_dec_close();
    if (bt_media_is_running()) {
        a2dp_dec_close();
        a2dp_media_clear_packet_before_seqn(0);
    }

    tone_play_stop();

    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_CONNECTION_CANCEL, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);

#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_tws_state() & TWS_STA_SIBLING_DISCONNECTED) {
        return 1;
    }
#endif
    btstack_exit();
    sys_auto_shut_down_disable();
    return 1;
}

static void tone_play_end_callback(void *priv, int flag)
{
    int index = (int)priv;

    switch (index) {
    case IDEX_TONE_BT_MODE:
        printf("%s:IDEX_TONE_BT_MODE", __FUNCTION__);
        /* 按键消息使能 */
        sys_key_event_enable();
        break;
    }
}

/*
 * earphone 模式状态机, 通过start_app()控制状态切换
 */
/* extern int audio_mic_init(); */

static int state_machine(struct application *app, enum app_state state, struct intent *it)
{
    int error = 0;
    static u8 tone_player_err = 0;

    r_printf("bt_state_machine=%d\n", state);
    switch (state) {
    case APP_STA_CREATE:
        /* set_adjust_conn_dac_check(0); */
#if TCFG_SD0_ENABLE || TCFG_PC_ENABLE
        STATUS *p_tone = get_tone_config();
        tone_play_index(p_tone->bt_init_ok, 1);
#else
        if (app_var.play_poweron_tone) {
            STATUS *p_tone = get_tone_config();
            /* tone_play_index(p_tone->power_on, 1); */
            tone_player_err = tone_play_index_with_callback(p_tone->power_on, 1, tone_play_end_callback, (void *)IDEX_TONE_BT_MODE);
        } else {
#ifdef CONFIG_CURRENT_SOUND_DEAL_ENABLE
            dac_analog_power_control(0);
#endif
        }
#endif
        break;
    case APP_STA_START:
        //进入蓝牙模式,UI退出充电状态
        ui_update_status(STATUS_EXIT_LOWPOWER);
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_EARPHONE_MAIN:
            /*
             * earphone 模式初始化
             */

            clk_set("sys", BT_NORMAL_HZ);
            u32 sys_clk =  clk_get("sys");
            bt_pll_para(TCFG_CLOCK_OSC_HZ, sys_clk, 0, 0);

            /* bredr_set_dut_enble(1, 1); */
            bt_function_select_init();
            bredr_handle_register();
            EARPHONE_STATE_INIT();
            btstack_init();
#if TCFG_USER_TWS_ENABLE
            tws_profile_init();
            sys_key_event_filter_disable();
#endif

            /* 按键消息使能 */
            sys_key_event_enable();
#if TCFG_AUDIO_WIDE_AREA_TAP_ENABLE
            extern int audio_wat_click_open();
            audio_wat_click_open();
#endif /*TCFG_AUDIO_WIDE_AREA_TAP_ENABLE*/
            sys_auto_shut_down_enable();
            bt_sniff_feature_init();
            sys_auto_sniff_controle(MY_SNIFF_EN, NULL);
            app_var.dev_volume = -1;
            break;
        case ACTION_A2DP_START:
#if (BT_SUPPORT_MUSIC_VOL_SYNC && CONFIG_BT_BACKGROUND_ENABLE)
            app_audio_set_volume(APP_AUDIO_STATE_MUSIC, app_var.bt_volume, 1);
            r_printf("BT_BACKGROUND RESET ACTION_A2DP_START STATE_MUSIC bt_volume=%d\n", app_var.bt_volume);
#endif
            a2dp_dec_open(0);
            break;
        case ACTION_BY_KEY_MODE:
            user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
            break;
        case ACTION_TONE_PLAY:
            STATUS *p_tone = get_tone_config();
            tone_play_index(p_tone->bt_init_ok, 1);
            break;
        case ACTION_DO_NOTHING:
            break;
        }
        break;
    case APP_STA_PAUSE:
        //转入后台运行
#if CONFIG_BT_BACKGROUND_ENABLE
        error = bt_switch_to_background();
        if (error == 0) {
            sys_auto_shut_down_disable();
        }
#if (BT_SUPPORT_MUSIC_VOL_SYNC)
        app_var.bt_volume = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
        if (app_var.dev_volume != -1) { //切换到其它模式恢复上一次的音量
            r_printf("resume dev_volume=%d\n", app_var.dev_volume);
            app_audio_set_volume(APP_AUDIO_STATE_MUSIC, app_var.dev_volume, 1);
            app_var.dev_volume = -1;
        }
#endif
        log_info("APP_STA_PAUSE: %d\n", error);
#endif
        break;
    case APP_STA_RESUME:
#if (BT_SUPPORT_MUSIC_VOL_SYNC && CONFIG_BT_BACKGROUND_ENABLE)
        app_var.dev_volume = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
        r_printf("APP_STA_RESUME rec dev_volume=%d\n", app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
#endif
        //恢复前台运行
        sys_auto_shut_down_disable();
        sys_key_event_enable();
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        r_printf("APP_STA_DESTROY\n");
        if (!app_var.goto_poweroff_flag) {
            bt_app_exit(NULL);
        }
        break;
    }

    return error;
}

static const struct application_operation app_earphone_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

/*
 * 注册earphone模式
 */
REGISTER_APPLICATION(app_earphone) = {
    .name 	= "earphone",
    .action	= ACTION_EARPHONE_MAIN,
    .ops 	= &app_earphone_ops,
    .state  = APP_STA_DESTROY,
};
