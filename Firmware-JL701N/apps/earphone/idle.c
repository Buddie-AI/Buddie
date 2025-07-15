#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "tone_player.h"
#include "asm/charge.h"
#include "app_charge.h"
#include "app_main.h"
#include "ui_manage.h"
#include "vm.h"
#include "app_chargestore.h"
#include "app_umidigi_chargestore.h"
#include "app_testbox.h"
#include "user_cfg.h"
#include "default_event_handler.h"

#if TCFG_ANC_BOX_ENABLE
#include "app_ancbox.h"
#endif

#define LOG_TAG_CONST       APP_IDLE
#define LOG_TAG             "[APP_IDLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if CHARGING_CLEAN_PHONE_INFO
#include "key_event_deal.h"
#include "btstack/btstack_typedef.h"
#endif

#ifdef CONFIG_MEDIA_DEVELOP_ENABLE
#define AUDIO_PLAY_EVENT_END 		AUDIO_DEC_EVENT_END
#endif /* #ifdef CONFIG_MEDIA_DEVELOP_ENABLE */

static void app_idle_enter_softoff(void)
{
    //ui_update_status(STATUS_POWEROFF);
    while (get_ui_busy_status()) {
        log_info("ui_status:%d\n", get_ui_busy_status());
    }
#if TCFG_CHARGE_ENABLE
    if (get_lvcmp_det() && (0 == get_charge_full_flag())) {
        log_info("charge inset, system reset!\n");
        cpu_reset();
    }
#endif

#if TCFG_SMART_VOICE_ENABLE
    extern void audio_smart_voice_detect_close(void);
    audio_smart_voice_detect_close();
#endif


#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
    extern void ais_platform_asr_close(void);
    ais_platform_asr_close();
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/

    extern void dac_power_off(void);
    dac_power_off();    // 关机前先关dac

    power_set_soft_poweroff();
}

static int app_idle_tone_event_handler(struct device_event *dev)
{
    int ret = false;

    switch (dev->event) {
    case AUDIO_PLAY_EVENT_END:
        if (app_var.goto_poweroff_flag) {
            log_info("audio_play_event_end,enter soft poweroff");
            app_idle_enter_softoff();
        }
        break;
    }

    return ret;
}

#if CHARGING_CLEAN_PHONE_INFO

#define IDLE_CLEAN_INFO_CNT     20

static u8 idle_key_table[KEY_NUM_MAX][KEY_EVENT_MAX] = {
    //SHORT      LONG                   HOLD                        UP                       DOUBLE    TRIPLE
    {KEY_NULL,   KEY_CLEAN_PHONE_INFO,  KEY_CLEAN_PHONE_INFO_HOLD,  KEY_CLEAN_PHONE_INFO_UP, KEY_NULL, KEY_NULL},   //KEY_0
};
int app_idle_key_event_handler(struct sys_event *event)
{
    int ret = false;
    struct key_event *key = &event->u.key;
    u8 key_event = idle_key_table[key->value][key->event];
    static u16 key_hold_cnt = 0;
    static u8 key_press_flag = 0;

    switch (key_event) {
    case KEY_CLEAN_PHONE_INFO:
        log_info("KEY_CLEAN_PHONE_INFO");
        key_hold_cnt = 0;
        key_press_flag = 1;
        P33_CON_SET(P3_PINR_CON, 0, 1, 0);
        break;
    case KEY_CLEAN_PHONE_INFO_HOLD:
        key_hold_cnt++;
        if (key_press_flag) {
            if (key_hold_cnt == IDLE_CLEAN_INFO_CNT) {
                log_info("KEY_CLEAN_PHONE_INFO_HOLD");
                extern void delete_link_key_for_app(bd_addr_t *bd_addr, u8 id);
                delete_link_key_for_app(NULL, 0);
                tone_play(TONE_SIN_NORMAL, 1);
                key_press_flag = 0;
            }
        }
        break;
    case KEY_CLEAN_PHONE_INFO_UP:
        log_info("KEY_CLEAN_PHONE_INFO_UP");
        key_hold_cnt = 0;
        key_press_flag = 0;
        P33_CON_SET(P3_PINR_CON, 0, 1, 1);
        break;
    }
    return ret;
}

#endif

static int idle_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
#if CHARGING_CLEAN_PHONE_INFO
        /* log_info("idle_event_handler:SYS_KEY_EVENT\n"); */
        app_idle_key_event_handler(event);
#endif
        return 0;
    case SYS_BT_EVENT:
        return 0;
    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
#if TCFG_CHARGE_ENABLE
            return app_charge_event_handler(&event->u.dev);
#endif
        }
        if ((u32)event->arg == DEVICE_EVENT_FROM_TONE) {
            return app_idle_tone_event_handler(&event->u.dev);
        }
#if TCFG_CHARGESTORE_ENABLE
        if ((u32)event->arg == DEVICE_EVENT_CHARGE_STORE) {
            app_chargestore_event_handler(&event->u.chargestore);
        }
#endif
#if TCFG_UMIDIGI_BOX_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_UMIDIGI_CHARGE_STORE) {
            app_umidigi_chargestore_event_handler(&event->u.umidigi_chargestore);
        }
#endif
#if TCFG_TEST_BOX_ENABLE
        if ((u32)event->arg == DEVICE_EVENT_TEST_BOX) {
            app_testbox_event_handler(&event->u.testbox);
        }
#endif
#if TCFG_ANC_BOX_ENABLE
        if ((u32)event->arg == DEVICE_EVENT_FROM_ANC) {
            return app_ancbox_event_handler(&event->u.ancbox);
        }
#endif
        break;
    default:
        return false;
    }

#if (CONFIG_BT_BACKGROUND_ENABLE || TCFG_PC_ENABLE)
#if(!TCFG_ONLY_PC_ENABLE)
    default_event_handler(event);
#endif
#endif

    return false;
}

static int idle_state_machine(struct application *app, enum app_state state,
                              struct intent *it)
{
    int ret;
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_IDLE_MAIN:
            log_info("ACTION_IDLE_MAIN\n");
            if (app_var.goto_poweroff_flag) {
                syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 1);
                /* tone_play(TONE_POWER_OFF); */
                os_taskq_flush();
                STATUS *p_tone = get_tone_config();
                ret = tone_play_index(p_tone->power_off, 1);
                printf("power_off tone play ret:%d", ret);
                if (ret) {
                    if (app_var.goto_poweroff_flag) {
                        log_info("power_off tone play err,enter soft poweroff");
                        app_idle_enter_softoff();
                    }
                }
            }
#if CHARGING_CLEAN_PHONE_INFO
            else {
                sys_key_event_enable();
            }
#endif
            break;
        case ACTION_IDLE_POWER_OFF:
            os_taskq_flush();
            syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 1);
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        break;
    }

    return 0;
}

static const struct application_operation app_idle_ops = {
    .state_machine  = idle_state_machine,
    .event_handler 	= idle_event_handler,
};

REGISTER_APPLICATION(app_app_idle) = {
    .name 	= "idle",
    .action	= ACTION_IDLE_MAIN,
    .ops 	= &app_idle_ops,
    .state  = APP_STA_DESTROY,
};


