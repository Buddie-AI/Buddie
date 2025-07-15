#include "app_config.h"
#include "user_cfg.h"
#include "fs.h"
#include "string.h"
#include "system/includes.h"
#include "vm.h"
#include "btcontroller_config.h"
#include "app_main.h"
#include "media/includes.h"
#include "audio_config.h"
#include "asm/pwm_led.h"
#include "aec_user.h"
#include "app_power_manage.h"

#define LOG_TAG_CONST       USER_CFG
#define LOG_TAG             "[USER_CFG]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

void lp_winsize_init(struct lp_ws_t *lp);
void bt_max_pwr_set(u8 pwr, u8 pg_pwr, u8 iq_pwr, u8 ble_pwr);
void app_set_sys_vol(s16 vol_l, s16  vol_r);

BT_CONFIG bt_cfg = {
    .edr_name        = {'Y', 'L', '-', 'B', 'R', '3', '0'},
    .mac_addr        = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .tws_local_addr  = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .rf_power        = 10,
    .dac_analog_gain = 25,
    .mic_analog_gain = 7,
    .tws_device_indicate = 0x6688,
};


AUDIO_CONFIG audio_cfg = {
    .max_sys_vol    = SYS_MAX_VOL,
    .default_vol    = SYS_DEFAULT_VOL,
    .tone_vol       = SYS_DEFAULT_TONE_VOL,
};

//======================================================================================//
//                                 		BTIF配置项表                               		//
//	参数1: 配置项名字                                			   						//
//	参数2: 配置项需要多少个byte存储														//
//	说明: 配置项ID注册到该表后该配置项将读写于BTIF区域, 其它没有注册到该表       		//
//		  的配置项则默认读写于VM区域.													//
//======================================================================================//
const struct btif_item btif_table[] = {
// 	 	item id 		   	   len   	//
    {CFG_BT_MAC_ADDR, 			6 },
    {CFG_BT_FRE_OFFSET,   		6 },   //测试盒矫正频偏值
    //{CFG_DAC_DTB,   			2 },
    //{CFG_MC_BIAS,   			1 },
    {0, 						0 },   //reserved cfg
};

//============================= VM 区域空间最大值 ======================================//
//以下宏在app_cfg中配置:
const int vm_max_page_align_size_config   = VM_MAX_PAGE_ALIGN_SIZE_CONFIG; 		//page对齐vm管理空间最大值配置
const int vm_max_sector_align_size_config = VM_MAX_SECTOR_ALIGN_SIZE_CONFIG; 	//sector对齐vm管理空间最大值配置
//======================================================================================//

struct lp_ws_t lp_winsize = {
    .lrc_ws_inc = 480,      //260
    .lrc_ws_init = 160,
    .bt_osc_ws_inc = 100,
    .bt_osc_ws_init = 140,
    .osc_change_mode = 0,
};

u16 bt_get_tws_device_indicate(u8 *tws_device_indicate)
{
    return bt_cfg.tws_device_indicate;
}

const u8 *bt_get_mac_addr()
{
    return bt_cfg.mac_addr;
}

void bt_update_mac_addr(u8 *addr)
{
    memcpy(bt_cfg.mac_addr, addr, 6);
}

static u8 bt_mac_addr_for_testbox[6] = {0};
void bt_get_vm_mac_addr(u8 *addr)
{
#if 0
    //中断不能调用syscfg_read;
    int ret = 0;

    ret = syscfg_read(CFG_BT_MAC_ADDR, addr, 6);
    if ((ret != 6)) {
        syscfg_write(CFG_BT_MAC_ADDR, addr, 6);
    }
#else

    memcpy(addr, bt_mac_addr_for_testbox, 6);
#endif
}

void bt_get_tws_local_addr(u8 *addr)
{
    memcpy(addr, bt_cfg.tws_local_addr, 6);
}

const char *sdk_version_info_get(void)
{
    extern u32 __VERSION_BEGIN;
    char *version_str = ((char *)&__VERSION_BEGIN) + 4;

    return version_str;
}

const char *bt_get_local_name()
{
    return (const char *)(bt_cfg.edr_name);
}

const char *bt_get_pin_code()
{
    return "0000";
}

extern STATUS_CONFIG status_config;
extern struct charge_platform_data charge_data;
/* extern struct dac_platform_data dac_data; */
extern struct adkey_platform_data adkey_data;
extern struct led_platform_data pwm_led_data;
extern struct adc_platform_data adc_data;

extern u8 key_table[KEY_EVENT_MAX][KEY_NUM_MAX];

u8 get_max_sys_vol(void)
{
    return (audio_cfg.max_sys_vol);
}

u8 get_tone_vol(void)
{
    if (!audio_cfg.tone_vol) {
        return (get_max_sys_vol());
    }
    if (audio_cfg.tone_vol > get_max_sys_vol()) {
        return (get_max_sys_vol());
    }

    return (audio_cfg.tone_vol);
}

#define USE_CONFIG_BIN_FILE                  0

#define USE_CONFIG_STATUS_SETTING            1                          //状态设置，包括灯状态和提示音
#define USE_CONFIG_AUDIO_SETTING             USE_CONFIG_BIN_FILE        //音频设置
#define USE_CONFIG_CHARGE_SETTING            USE_CONFIG_BIN_FILE        //充电设置
#define USE_CONFIG_KEY_SETTING               USE_CONFIG_BIN_FILE        //按键消息设置
#define USE_CONFIG_MIC_TYPE_SETTING          USE_CONFIG_BIN_FILE        //MIC类型设置
#define USE_CONFIG_LOWPOWER_V_SETTING        USE_CONFIG_BIN_FILE        //低电提示设置
#define USE_CONFIG_AUTO_OFF_SETTING          USE_CONFIG_BIN_FILE        //自动关机时间设置
#define USE_CONFIG_VOL_SETTING               1					        //音量表读配置

__attribute__((weak))
void get_config_mic_gain(u8 mic_gain0, u8 mic_gain1, u8 mic_gain2)
{

}

/*更加mic通道值获取使用的第几个mic*/
u8 get_mic_num(u8 bit)
{
    u8 i = 0;
    for (i = 0; i < 4; i++) {
        if (bit & 0x1) {
            return i;
        }
        bit >>= 1;
    }
    return 0;
}

__BANK_INIT_ENTRY
void cfg_file_parse(u8 idx)
{
    u8 tmp[128] = {0};
    int ret = 0;

    memset(tmp, 0x00, sizeof(tmp));

    /*************************************************************************/
    /*                      CFG READ IN cfg_tools.bin                        */
    /*************************************************************************/


    //-----------------------------CFG_BT_NAME--------------------------------------//
    ret = syscfg_read(CFG_BT_NAME, tmp, 32);
    if (ret < 0) {
        log_info("read bt name err");
    } else if (ret >= LOCAL_NAME_LEN) {
        memset(bt_cfg.edr_name, 0x00, LOCAL_NAME_LEN);
        memcpy(bt_cfg.edr_name, tmp, LOCAL_NAME_LEN);
        bt_cfg.edr_name[LOCAL_NAME_LEN - 1] = 0;
    } else {
        memset(bt_cfg.edr_name, 0x00, LOCAL_NAME_LEN);
        memcpy(bt_cfg.edr_name, tmp, ret);
    }
    /* g_printf("bt name config:%s", bt_cfg.edr_name); */
    log_info("bt name config:%s", bt_cfg.edr_name);

    //-----------------------------CFG_TWS_PAIR_CODE_ID----------------------------//
    ret = syscfg_read(CFG_TWS_PAIR_CODE_ID, &bt_cfg.tws_device_indicate, 2);
    if (ret < 0) {
        log_debug("read pair code err");
        bt_cfg.tws_device_indicate = 0x8888;
    }
    /* g_printf("tws pair code config:"); */
    log_info("tws pair code config:");
    log_info_hexdump(&bt_cfg.tws_device_indicate, 2);

    //-----------------------------CFG_BT_RF_POWER_ID----------------------------//
    ret = syscfg_read(CFG_BT_RF_POWER_ID, &app_var.rf_power, 1);
    if (ret < 0) {
        log_debug("read rf err");
        app_var.rf_power = 10;
    }
    bt_max_pwr_set(app_var.rf_power, 5, 8, 9);//last is ble tx_pwer(0~9)
    /* g_printf("rf config:%d\n", app_var.rf_power); */
    log_info("rf config:%d\n", app_var.rf_power);

    //-----------------------------CFG_AEC_ID------------------------------------//
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
    log_info("3mic config:");
    AEC_TMS_CONFIG aec;
    ret = syscfg_read(CFG_TMS_DNS_ID, &aec, sizeof(aec));
#elif TCFG_AUDIO_DUAL_MIC_ENABLE
#if ((defined TCFG_AUDIO_DMS_SEL) && (TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE))
#if(TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    log_info("dms_flexible config:");/*双mic话务耳机降噪*/
    DMS_FLEXIBLE_CONFIG aec;
    ret = syscfg_read(CFG_DMS_FLEXIBLE_ID, &aec, sizeof(aec));
#else /*DMS_DNS*/
    log_info("dms_dns_flexible config:");/*双mic话务耳机神经网络降噪*/
    DMS_FLEXIBLE_CONFIG aec;
    ret = syscfg_read(CFG_DMS_DNS_FLEXIBLE_ID, &aec, sizeof(aec));
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
#else /*TCFG_AUDIO_DMS_SEL == DMS_NORMAL*/
#if(TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    log_info("dms_ans config:");/*双mic传统降噪*/
    AEC_DMS_CONFIG aec;
    ret = syscfg_read(CFG_DMS_ID, &aec, sizeof(aec));
#else /*DMS_DNS*/
    log_info("dms_dns config:");/*双mic神经网络降噪*/
    AEC_DMS_CONFIG aec;
    ret = syscfg_read(CFG_DMS_DNS_ID, &aec, sizeof(aec));
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
#if TCFG_AUDIO_MIC_ARRAY_TRIM_ENABLE
    app_var.enc_degradation = aec.target_signal_degradation;
#endif /*TCFG_AUDIO_MIC_ARRAY_TRIM_ENABLE*/
#endif/*TCFG_AUDIO_DMS_SEL*/
#else /*single mic*/
    AEC_CONFIG aec;
#if(TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    log_info("sms_ans config:");
    ret = syscfg_read(CFG_AEC_ID, &aec, sizeof(aec));
#else /*SMS_DNS*/
    log_info("sms_dns config:");
    ret = syscfg_read(CFG_SMS_DNS_ID, &aec, sizeof(aec));
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
    //log_info("ret:%d,aec_cfg size:%d\n",ret,sizeof(aec));
    if (ret == sizeof(aec)) {
        log_info("aec cfg read succ\n");
        log_info_hexdump(&aec, sizeof(aec));

        app_var.aec_mic_gain = aec.mic_again;
        app_var.aec_mic1_gain = aec.mic_again;
        app_var.aec_mic2_gain = aec.mic_again;
        app_var.aec_mic3_gain = aec.mic_again;
#if TCFG_AUDIO_DUAL_MIC_ENABLE
#if ((defined TCFG_AUDIO_DMS_SEL) && (TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE))
        app_var.aec_mic1_gain = aec.mic1_again;
#endif/*TCFG_AUDIO_DMS_SEL*/
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
        get_config_mic_gain(app_var.aec_mic_gain, app_var.aec_mic1_gain, app_var.aec_mic1_gain);

        /*3麦增益设置*/
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
        app_var.talk_mic_ch = aec.talk_mic_ch;
        app_var.talk_ref_mic_ch = aec.talk_ref_mic_ch;
        app_var.talk_fb_mic_ch = aec.talk_fb_mic_ch;
        printf("3 mic ch sel , talk : %d, ref : %d, fb : %d", app_var.talk_mic_ch, app_var.talk_ref_mic_ch, app_var.talk_fb_mic_ch);

        u8 gain[4] = {0};
        gain[get_mic_num(aec.talk_mic_ch)] = aec.mic_again;
        gain[get_mic_num(aec.talk_ref_mic_ch)] = aec.mic_again;
        gain[get_mic_num(aec.talk_fb_mic_ch)] = aec.fb_mic_again;
        app_var.aec_mic_gain = gain[0];
        app_var.aec_mic1_gain = gain[1];
        app_var.aec_mic2_gain = gain[2];
        app_var.aec_mic3_gain = gain[3];
#endif

        app_var.aec_dac_gain = aec.dac_again;
#if AEC_READ_CONFIG
#ifdef CONFIG_MEDIA_ORIGIN_ENABLE
        aec_cfg_fill(&aec);
#endif/*CONFIG_MEDIA_ORIGIN_ENABLE*/
#endif/*AEC_READ_CONFIG*/
    } else {
        log_info("aec cfg read err ret: %d, aec : %d,use default value\n", ret, sizeof(aec));
        app_var.talk_mic_ch = 1;
        app_var.talk_ref_mic_ch = 2;
        app_var.talk_fb_mic_ch = 4;

        app_var.aec_mic_gain = 12;
        app_var.aec_mic1_gain = 12;
        app_var.aec_mic2_gain = 1;
        app_var.aec_mic3_gain = 1;
        app_var.aec_dac_gain = 12;/*初始化默认值要注意不能大于芯片支持最大模拟音量*/
    }
    log_info("CVP_cfg Mic0_gain:%d Mic1_gain:%d Mic2_gain:%d Mic3_gain:%d DAC_Gain:%d",
             app_var.aec_mic_gain, app_var.aec_mic1_gain, app_var.aec_mic2_gain, app_var.aec_mic3_gain, app_var.aec_dac_gain);

    //-----------------------------CFG_MIC_ARRAY_TRIM-------------------------------------//
#if TCFG_AUDIO_MIC_ARRAY_TRIM_ENABLE

    extern void put_float(double fv);
    ret = syscfg_read(CFG_MIC_ARRAY_TRIM_EN, (u8 *)(&app_var.audio_mic_array_trim_en), sizeof(app_var.audio_mic_array_trim_en));
    if (ret != sizeof(app_var.audio_mic_array_trim_en)) {
        log_info("audio_mic_array_trim_en vm read fail %d, %d!!!", ret, sizeof(app_var.audio_mic_array_trim_en));
    }
    ret = syscfg_read(CFG_MIC_ARRAY_DIFF_CMP_VALUE, (u8 *)(&app_var.audio_mic_array_diff_cmp), sizeof(app_var.audio_mic_array_diff_cmp));
    if (ret != sizeof(app_var.audio_mic_array_diff_cmp)) {
        log_info("audio_mic_array_diff_cmp vm read fail  %d, %d!!!", ret, sizeof(app_var.audio_mic_array_diff_cmp));
        app_var.audio_mic_array_diff_cmp = 1.0f;
    }
    ret = syscfg_read(CFG_MIC_TARGET_DIFF_CMP, (u8 *)(&app_var.audio_mic_cmp), sizeof(audio_mic_cmp_t));
    if (ret != sizeof(audio_mic_cmp_t)) {
        log_info("audio_mic_array_diff_cmp vm read fail  %d, %d!!!", ret, sizeof(audio_mic_cmp_t));
        app_var.audio_mic_cmp.talk = 1.0f;
        app_var.audio_mic_cmp.ff = 1.0f;
        app_var.audio_mic_cmp.fb = 1.0f;
    }

    log_info("mic array trim en: %d\n", app_var.audio_mic_array_trim_en);
    log_info("mic_array_diff_cmp:");
    put_float(app_var.audio_mic_array_diff_cmp);
    log_info("mic_cmp-talk:");
    put_float(app_var.audio_mic_cmp.talk);
    log_info("mic_cmp-ff:");
    put_float(app_var.audio_mic_cmp.ff);
    log_info("mic_cmp-fb:");
    put_float(app_var.audio_mic_cmp.fb);

#endif

    //-----------------------------CFG_VOL_LIST-------------------------------------//
    audio_volume_list_init(USE_CONFIG_VOL_SETTING);

    //-----------------------------CFG_MIC_TYPE_ID------------------------------------//
#if USE_CONFIG_MIC_TYPE_SETTING
    log_info("mic_type_config:");
    extern int read_mic_type_config(void);
    read_mic_type_config();

#endif

#if TCFG_MC_BIAS_AUTO_ADJUST
    ret = syscfg_read(CFG_MC_BIAS, &adc_data.mic_bias_res, 1);
    log_info("mic_bias_res:%d", adc_data.mic_bias_res);
    if (ret != 1) {
        log_info("mic_bias_adjust NULL");
    }
    u8 mic_ldo_idx;
    ret = syscfg_read(CFG_MIC_LDO_VSEL, &mic_ldo_idx, 1);
    if (ret == 1) {
        adc_data.mic_ldo_vsel = mic_ldo_idx & 0x3;
        log_info("mic_ldo_vsel:%d,%d\n", adc_data.mic_ldo_vsel, mic_ldo_idx);
    } else {
        log_info("mic_ldo_vsel_adjust NULL\n");
    }
#endif

#if USE_CONFIG_STATUS_SETTING
    /* g_printf("status_config:"); */
    log_info("status_config:");
    STATUS_CONFIG *status = (STATUS_CONFIG *)tmp;
    ret = syscfg_read(CFG_UI_TONE_STATUS_ID, status, sizeof(STATUS_CONFIG));
    if (ret > 0) {
        memcpy((u8 *)&status_config, (u8 *)status, sizeof(STATUS_CONFIG));
        log_info_hexdump(&status_config, sizeof(STATUS_CONFIG));
    }
#endif

#if USE_CONFIG_AUDIO_SETTING
    /* g_printf("app audio_config:"); */
    log_info("app audio_config:");
    ret = syscfg_read(CFG_AUDIO_ID, (u8 *)&audio_cfg, sizeof(AUDIO_CONFIG));
    if (ret > 0) {
        log_info_hexdump((u8 *)&audio_cfg, sizeof(AUDIO_CONFIG));
#else
    {
#endif
        /* dac_data.max_ana_vol = audio_cfg.max_sys_vol; */
        /* if (dac_data.max_ana_vol > 30) { */
        /* dac_data.max_ana_vol = 30; */
        /* } */

        u8 default_volume = audio_cfg.default_vol;
        s8 music_volume = -1;
#if SYS_DEFAULT_VOL
        default_volume = audio_cfg.default_vol;
#else
        syscfg_read(CFG_SYS_VOL, &default_volume, 1);
        ret = syscfg_read(CFG_MUSIC_VOL, &music_volume, 1);
        if (ret < 0) {
            music_volume = -1;
        }
#endif
        if (default_volume > audio_cfg.max_sys_vol) {
            default_volume = audio_cfg.max_sys_vol;
        }
        if (default_volume == 0) {
            default_volume = audio_cfg.max_sys_vol / 2;
        }

        app_var.music_volume = music_volume == -1 ? default_volume : music_volume;
        app_var.wtone_volume = audio_cfg.tone_vol;
#if TCFG_CALL_USE_DIGITAL_VOLUME
        app_var.call_volume = 15;
#else
        app_var.call_volume = app_var.aec_dac_gain;
#endif
        app_var.opid_play_vol_sync = default_volume * 127 / audio_cfg.max_sys_vol;

        log_info("max vol:%d default vol:%d tone vol:%d", audio_cfg.max_sys_vol, default_volume, audio_cfg.tone_vol);
    }

#if (USE_CONFIG_CHARGE_SETTING) && (TCFG_CHARGE_ENABLE)
    /* g_printf("app charge config:"); */
    log_info("app charge config:");
    CHARGE_CONFIG *charge = (CHARGE_CONFIG *)tmp;
    ret = syscfg_read(CFG_CHARGE_ID, charge, sizeof(CHARGE_CONFIG));
    if (ret > 0) {
        log_info_hexdump(charge, sizeof(CHARGE_CONFIG));
        log_info("sw:%d poweron_en:%d full_v:%d full_mA:%d charge_mA:%d",
                 charge->sw, charge->poweron_en, charge->full_v, charge->full_c, charge->charge_c);
        memcpy((u8 *)&charge_data, (u8 *)charge, sizeof(CHARGE_CONFIG));
    }
#endif

#if (USE_CONFIG_KEY_SETTING) && (TCFG_ADKEY_ENABLE || TCFG_IOKEY_ENABLE)
    /* g_printf("app key config:"); */
    log_info("app key config:");
    KEY_OP *key_msg = (KEY_OP *)tmp;
    ret = syscfg_read(CFG_KEY_MSG_ID, key_msg, sizeof(KEY_OP) * KEY_NUM_MAX);
    if (ret > 0) {
        log_info_hexdump(key_msg, sizeof(KEY_OP) * KEY_NUM);
        memcpy(key_table, key_msg, sizeof(KEY_OP) * KEY_NUM);
    }

    log_info("key_msg:");
    log_info_hexdump((u8 *)key_table, KEY_EVENT_MAX * KEY_NUM_MAX);
#endif


#if USE_CONFIG_LOWPOWER_V_SETTING
    /* g_printf("auto low power config:"); */
    log_info("auto low power config:");
    AUTO_LOWPOWER_V_CONFIG auto_lowpower;
    ret = syscfg_read(CFG_LOWPOWER_V_ID, &auto_lowpower, sizeof(AUTO_LOWPOWER_V_CONFIG));
    if (ret > 0) {
        app_var.warning_tone_v = auto_lowpower.warning_tone_v;
        app_var.poweroff_tone_v = auto_lowpower.poweroff_tone_v;
    }
    log_info("warning_tone_v:%d poweroff_tone_v:%d", app_var.warning_tone_v, app_var.poweroff_tone_v);
#else
    app_var.warning_tone_v = LOW_POWER_WARN_VAL;
    app_var.poweroff_tone_v = LOW_POWER_OFF_VAL;
    log_info("warning_tone_v:%d poweroff_tone_v:%d", app_var.warning_tone_v, app_var.poweroff_tone_v);
#endif

#if USE_CONFIG_AUTO_OFF_SETTING
    /* g_printf("auto off time config:"); */
    log_info("auto off time config:");
    AUTO_OFF_TIME_CONFIG auto_off_time;
    ret = syscfg_read(CFG_AUTO_OFF_TIME_ID, &auto_off_time, sizeof(AUTO_OFF_TIME_CONFIG));
    if (ret > 0) {
        app_var.auto_off_time = auto_off_time.auto_off_time * 60;
    }
    log_info("auto_off_time:%d", app_var.auto_off_time);
#else
    app_var.auto_off_time =  TCFG_AUTO_SHUT_DOWN_TIME;
    log_info("auto_off_time:%d", app_var.auto_off_time);
#endif

    /*************************************************************************/
    /*                      CFG READ IN VM                                   */
    /*************************************************************************/
    u8 mac_buf[6];
    u8 mac_buf_tmp[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 mac_buf_tmp2[6] = {0, 0, 0, 0, 0, 0};
#if TCFG_USER_TWS_ENABLE
    int len = syscfg_read(CFG_TWS_LOCAL_ADDR, bt_cfg.tws_local_addr, 6);
    if (len != 6) {
        get_random_number(bt_cfg.tws_local_addr, 6);
        syscfg_write(CFG_TWS_LOCAL_ADDR, bt_cfg.tws_local_addr, 6);
    }
    log_debug("tws_local_mac:");
    log_info_hexdump(bt_cfg.tws_local_addr, sizeof(bt_cfg.tws_local_addr));

    ret = syscfg_read(CFG_TWS_COMMON_ADDR, mac_buf, 6);
    if (ret != 6 || !memcmp(mac_buf, mac_buf_tmp, 6))
#endif
        do {
            ret = syscfg_read(CFG_BT_MAC_ADDR, mac_buf, 6);
            if ((ret != 6) || !memcmp(mac_buf, mac_buf_tmp, 6) || !memcmp(mac_buf, mac_buf_tmp2, 6)) {
                get_random_number(mac_buf, 6);
                syscfg_write(CFG_BT_MAC_ADDR, mac_buf, 6);
            }
        } while (0);


    syscfg_read(CFG_BT_MAC_ADDR, bt_mac_addr_for_testbox, 6);
    if (!memcmp(bt_mac_addr_for_testbox, mac_buf_tmp, 6)) {
        get_random_number(bt_mac_addr_for_testbox, 6);
        syscfg_write(CFG_BT_MAC_ADDR, bt_mac_addr_for_testbox, 6);
        log_info(">>>init mac addr!!!\n");
    }

    log_info("mac:");
    log_info_hexdump(mac_buf, sizeof(mac_buf));
    memcpy(bt_cfg.mac_addr, mac_buf, 6);

#if (CONFIG_BT_MODE != BT_NORMAL)
    const u8 dut_name[]  = "AC693x_DUT";
    const u8 dut_addr[6] = {0x12, 0x34, 0x56, 0x56, 0x34, 0x12};
    memcpy(bt_cfg.edr_name, dut_name, sizeof(dut_name));
    memcpy(bt_cfg.mac_addr, dut_addr, 6);
#endif

    /*************************************************************************/
    /*                      CFG READ IN isd_config.ini                       */
    /*************************************************************************/
    LRC_CONFIG lrc_cfg;
    ret = syscfg_read(CFG_LRC_ID, &lrc_cfg, sizeof(LRC_CONFIG));
    if (ret > 0) {
        log_info("lrc cfg:");
        log_info_hexdump(&lrc_cfg, sizeof(LRC_CONFIG));
        lp_winsize.lrc_ws_inc      = lrc_cfg.lrc_ws_inc;
        lp_winsize.lrc_ws_init     = lrc_cfg.lrc_ws_init;
        lp_winsize.bt_osc_ws_inc   = lrc_cfg.btosc_ws_inc;
        lp_winsize.bt_osc_ws_init  = lrc_cfg.btosc_ws_init;
        lp_winsize.osc_change_mode = lrc_cfg.lrc_change_mode;
    }
    /* printf("%d %d %d ",lp_winsize.lrc_ws_inc,lp_winsize.lrc_ws_init,lp_winsize.osc_change_mode); */
    lp_winsize_init(&lp_winsize);
}

extern void lmp_hci_write_local_name(const char *name);
int bt_modify_name(u8 *new_name)
{
    u8 new_len = strlen(new_name);

    if (new_len >= LOCAL_NAME_LEN) {
        new_name[LOCAL_NAME_LEN - 1] = 0;
    }

    if (strcmp(new_name, bt_cfg.edr_name)) {
        syscfg_write(CFG_BT_NAME, new_name, LOCAL_NAME_LEN);
        memcpy(bt_cfg.edr_name, new_name, LOCAL_NAME_LEN);
        lmp_hci_write_local_name(bt_get_local_name());
        log_info("mdy_name sucess\n");
        return 1;
    }
    return 0;
}


