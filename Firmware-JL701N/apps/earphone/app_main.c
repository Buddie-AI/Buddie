#include "system/includes.h"
/*#include "btcontroller_config.h"*/
#include "btstack/btstack_task.h"
#include "app_config.h"
#include "app_action.h"
#include "asm/pwm_led.h"
#include "tone_player.h"
#include "ui_manage.h"
#include "gpio.h"
#include "app_main.h"
#include "asm/charge.h"
#include "update.h"
#include "app_power_manage.h"
#include "audio_config.h"
#include "app_charge.h"
#include "bt_profile_cfg.h"
#include "dev_manager/dev_manager.h"
#include "update_loader_download.h"
#include "fft_and_pca.h"

#ifndef CONFIG_MEDIA_NEW_ENABLE
#ifndef CONFIG_MEDIA_DEVELOP_ENABLE
#include "audio_dec_server.h"
#endif
#endif

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif

#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif

#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
u8 user_at_cmd_send_support = 1;
#endif

/*任务列表 */
const struct task_info task_info_table[] = {
    {"app_core",            1,     0,   768,   256 },
    {"sys_event",           7,     0,   256,   0   },
    {"systimer",		    7,	   0,   128,   0   },
    {"btctrler",            4,     0,   512,   384 },
    {"btencry",             1,     0,   512,   128 },
    {"tws",                 5,     0,   512,   128 },
#if (BT_FOR_APP_EN)
    {"btstack",             3,     0,   1024,  256 },
#else
    {"btstack",             3,     0,   768,   256 },
#endif
    {"audio_dec",           5,     0,   800,   128 },
    {"aud_effect",          5,     1,   800,   128 },
    /*
     *为了防止dac buf太大，通话一开始一直解码，
     *导致编码输入数据需要很大的缓存，这里提高编码的优先级
     */
    {"audio_enc",           6,     0,   768,   128 },
    {"aec",					2,	   1,   768,   128 },
#if TCFG_AUDIO_HEARING_AID_ENABLE
    {"HearingAid",			6,	   0,   768,   128   },
#endif/*TCFG_AUDIO_HEARING_AID_ENABLE*/
#ifdef CONFIG_BOARD_AISPEECH_NR
    {"aispeech_enc",		2,	   1,   512,   128 },
#endif /*CONFIG_BOARD_AISPEECH_NR*/
#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
    {"asr",                 1,     0,   768,   128 },
    {"audio_asr_export_task",  1,     0,   512,   128 },
#endif/*CONFIG_BOARD_AISPEECH_VAD_ASR*/
#ifndef CONFIG_256K_FLASH
    {"aec_dbg",				3,	   0,   128,   128 },

#if AUDIO_ENC_MPT_SELF_ENABLE
    {"enc_mpt_self",		3,	   0,   512,   128 },
#endif/*AUDIO_ENC_MPT_SELF_ENABLE*/
    {"update",				1,	   0,   256,   0   },
    {"tws_ota",				2,	   0,   256,   0   },
    {"tws_ota_msg",			2,	   0,   256,   128 },
    {"dw_update",		 	2,	   0,   256,   128 },
    {"rcsp_task",		    2,	   0,   640,   128 },
    {"aud_capture",         4,     0,   512,   256 },
    {"data_export",         5,     0,   512,   256 },
    {"anc",                 3,     1,   512,   128 },
#endif

#if TCFG_GX8002_NPU_ENABLE
    {"gx8002",              2,     0,   256,   64  },
#endif /* #if TCFG_GX8002_NPU_ENABLE */
#if TCFG_GX8002_ENC_ENABLE
    {"gx8002_enc",          2,     0,   128,   64  },
#endif /* #if TCFG_GX8002_ENC_ENABLE */


#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
    {"kws",                 2,     0,   256,   64  },
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
    {"usb_msd",           	1,     0,   512,   128 },
#if !TCFG_USB_MIC_CVP_ENABLE
    {"usbmic_write",       	2,     0,   256,   128 },
#endif
#if AI_APP_PROTOCOL
    {"app_proto",           2,     0,   768,   64  },
#endif
#if (TCFG_SPI_LCD_ENABLE||TCFG_SIMPLE_LCD_ENABLE)
    {"ui",           	    2,     0,   768,   256 },
#else
    {"ui",                  3,     0,   384 - 64,  128  },
#endif
#if (TCFG_DEV_MANAGER_ENABLE)
    {"dev_mg",           	3,     0,   512,   512 },
#endif
    {"audio_vad",           1,     1,   512,   128 },
#if TCFG_KEY_TONE_EN
    {"key_tone",            5,     0,   256,   32  },
#endif
#if (TCFG_WIRELESS_MIC_ENABLE)
    {"wl_mic_enc",          2,     0,   768,   128 },
#endif
#if (TUYA_DEMO_EN)
    {"user_deal",           7,     0,   512,   512 },//定义线程 tuya任务调度
    {"dw_update",           2,     0,   256,   128 },
#endif
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    {"imu_trim",            1,     0,   256,   128 },
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    {"speak_to_chat",       2,     0,   256,   128 },
    {"icsd_adt",            2,     0,   512,   128 },
    {"icsd_src",            2,     1,   512,   128 },
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
    {"pmu_task",            6,      0,  256,   128  },
    {"WindDetect",          2,      0,  256,   128  },

    {"pca",                 1,      0,  256,   128  },
    {"vad_task",            1,      0,  256,   128  },
    {0, 0},
};


APP_VAR app_var;

/*
 * 2ms timer中断回调函数
 */
void timer_2ms_handler()
{

}

void app_var_init(void)
{
    memset((u8 *)&bt_user_priv_var, 0, sizeof(BT_USER_PRIV_VAR));
    app_var.play_poweron_tone = 1;

}



void app_earphone_play_voice_file(const char *name);

void clr_wdt(void);

void check_power_on_key(void)
{
    u32 delay_10ms_cnt = 0;

    while (1) {
        clr_wdt();
        os_time_dly(1);

        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            log_info("+");
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 70) {
                /* extern void set_key_poweron_flag(u8 flag); */
                /* set_key_poweron_flag(1); */
                return;
            }
        } else {
            log_info("-");
            delay_10ms_cnt = 0;
            log_info("enter softpoweroff\n");
            power_set_soft_poweroff();
        }
    }
}


extern int cpu_reset_by_soft();
extern int audio_dec_init();
extern int audio_enc_init();



__attribute__((weak))
u8 get_charge_online_flag(void)
{
    return 0;
}

/*充电拔出,CPU软件复位, 不检测按键，直接开机*/
static void app_poweron_check(int update)
{
#if (CONFIG_BT_MODE == BT_NORMAL)
    if (!update && cpu_reset_by_soft()) {
        app_var.play_poweron_tone = 0;
        return;
    }

#if TCFG_CHARGE_OFF_POWERON_NE
    if (is_ldo5v_wakeup()) {
        app_var.play_poweron_tone = 0;
        return;
    }
#endif
//#ifdef CONFIG_RELEASE_ENABLE
#if TCFG_POWER_ON_NEED_KEY
    check_power_on_key();
#endif
//#endif

#endif
}


bool bone_task_is_open = false;
bool pca_task_is_open = false;


extern u32 timer_get_ms(void);
extern int bt_modify_name(u8 *new_name);
void app_main()
{
    int update = 0;
    u32 addr = 0, size = 0;
    struct intent it;


    log_info("app_main\n");
    app_var.start_time = timer_get_ms();

#if (defined(CONFIG_MEDIA_NEW_ENABLE) || (defined(CONFIG_MEDIA_DEVELOP_ENABLE)))
    /*解码器*/
    audio_enc_init();
    audio_dec_init();
#endif


#ifdef BT_DUT_INTERFERE
    void audio_demo(void);
    audio_demo();
#endif/*BT_DUT_INTERFERE*/
#ifdef BT_DUT_ADC_INTERFERE
    void audio_adc_mic_dut_open(void);
    audio_adc_mic_dut_open();
#endif/*BT_DUT_ADC_INTERFERE*/

    if (!UPDATE_SUPPORT_DEV_IS_NULL()) {
        update = update_result_deal();
    }

    app_var_init();

#if TCFG_MC_BIAS_AUTO_ADJUST
    mc_trim_init(update);
#endif/*TCFG_MC_BIAS_AUTO_ADJUST*/

    if (get_charge_online_flag()) {

#if(TCFG_SYS_LVD_EN == 1)
        vbat_check_init();
#endif

        init_intent(&it);
        it.name = "idle";
        it.action = ACTION_IDLE_MAIN;
        start_app(&it);
    } else {
        check_power_on_voltage();

        app_poweron_check(update);

        ui_manage_init();
        ui_update_status(STATUS_POWERON);

#if TCFG_WIRELESS_MIC_ENABLE
        extern void wireless_mic_main_run(void);
        wireless_mic_main_run();
#endif

#if  TCFG_ENTER_PC_MODE
        init_intent(&it);
        it.name = "pc";
        it.action = ACTION_PC_MAIN;
        start_app(&it);
#else
        init_intent(&it);
        it.name = "earphone";
        it.action = ACTION_EARPHONE_MAIN;
        start_app(&it);
#endif
    }

#if TCFG_CHARGE_ENABLE
    set_charge_event_flag(1);
#endif

    bt_modify_name("Buddie");
    if (!pca_task_is_open) {
        pca_open();
        pca_task_is_open = true;
    }
    if (!bone_task_is_open)
    {
        bone_task_init();
        bone_task_is_open = true;
    }
}

int __attribute__((weak)) eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms)，或自己指定唤醒时间 */
    //1:Endless Sleep
    //0:100 ms wakeup
    //other: x ms wakeup
    if (get_charge_full_flag()) {
        /* log_i("Endless Sleep"); */
        power_set_soft_poweroff();
        return 1;
    } else {
        /* log_i("100 ms wakeup"); */
        return 0;
    }
}

__attribute__((used)) int *__errno()
{
    static int err;
    return &err;
}

enum {
    KEY_USER_DEAL_POST = 0,
    KEY_USER_DEAL_POST_MSG,
    KEY_USER_DEAL_POST_EVENT,
    KEY_USER_DEAL_POST_2,
};

#include "system/includes.h"
#include "system/event.h"

///自定义事件推送的线程

#define Q_USER_DEAL   0xAABBCC ///自定义队列类型
#define Q_USER_DATA_SIZE  10///理论Queue受任务声明struct task_info.qsize限制,但不宜过大,建议<=6

void user_deal_send_ver(void)
{
    //os_taskq_post("user_deal", 1,KEY_USER_DEAL_POST);
    os_taskq_post_msg("user_deal", 1, KEY_USER_DEAL_POST_MSG);
    //os_taskq_post_event("user_deal",1, KEY_USER_DEAL_POST_EVENT);
}

void user_deal_rand_set(u32 rand)
{
    os_taskq_post("user_deal", 2, KEY_USER_DEAL_POST_2, rand);
}

void user_deal_send_array(int *msg, int argc)
{
    if (argc > Q_USER_DATA_SIZE) {
        return;
    }
    os_taskq_post_type("user_deal", Q_USER_DEAL, argc, msg);
}
void user_deal_send_msg(void)
{
    os_taskq_post_event("user_deal", 1, KEY_USER_DEAL_POST_EVENT);
}

void user_deal_send_test(void)///模拟测试函数,可按键触发调用，自行看打印
{
    user_deal_send_ver();
    user_deal_rand_set(0x11223344);
    static u32 data[Q_USER_DATA_SIZE] = {0x11223344, 0x55667788, 0x11223344, 0x55667788, 0x11223344,
                                         0xff223344, 0x556677ee, 0x11223344, 0x556677dd, 0x112233ff,
                                        };
    user_deal_send_array(data, sizeof(data) / sizeof(int));
}

static void user_deal_task_handle(void *p)
{
    int msg[Q_USER_DATA_SIZE + 1] = {0, 0, 0, 0, 0, 0, 0, 0, 00, 0};
    int res = 0;
    while (1) {
        res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (res != OS_TASKQ) {
            continue;
        }
        r_printf("user_deal_task_handle:0x%x", msg[0]);
        put_buf(msg, (Q_USER_DATA_SIZE + 1) * 4);
        if (msg[0] == Q_MSG) {
            printf("use os_taskq_post_msg");
            switch (msg[1]) {
            case KEY_USER_DEAL_POST_MSG:
                printf("KEY_USER_DEAL_POST_MSG");
                break;
            default:
                break;
            }
        } else if (msg[0] == Q_EVENT) {
            printf("use os_taskq_post_event");
            switch (msg[1]) {
            case KEY_USER_DEAL_POST_EVENT:
                printf("KEY_USER_DEAL_POST_EVENT");
                break;
            default:
                break;
            }
        } else if (msg[0] == Q_CALLBACK) {
        } else if (msg[0] == Q_USER) {
            printf("use os_taskq_post");
            switch (msg[1]) {
            case KEY_USER_DEAL_POST:
                printf("KEY_USER_DEAL_POST");
                break;
            case KEY_USER_DEAL_POST_2:
                printf("KEY_USER_DEAL_POST_2:0x%x", msg[2]);
                break;
            default:
                break;
            }
        } else if (msg[0] == Q_USER_DEAL) {
            printf("use os_taskq_post_type");
            printf("0x%x 0x%x 0x%x 0x%x 0x%x", msg[1], msg[2], msg[3], msg[4], msg[5]);
            printf("0x%x 0x%x 0x%x 0x%x 0x%x", msg[6], msg[7], msg[8], msg[9], msg[10]);
        }
        puts("");
    }
}

void user_deal_init(void)
{
    task_create(user_deal_task_handle, NULL, "user_deal");
}

void user_deal_exit(void)
{
    task_kill("user_deal");
}

