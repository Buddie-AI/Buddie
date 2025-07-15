#include "system/includes.h"
#include "music/music_player.h"
#include "music/breakpoint.h"
#include "app_action.h"
#include "key_event_deal.h"
#include "audio_config.h"
#include "bt_background.h"
#include "default_event_handler.h"
#include "usb/device/usb_stack.h"
#include "usb/device/hid.h"
#include "tone_player.h"
#include "user_cfg.h"
#include "app_task.h"
#include "earphone.h"
#include "app_power_manage.h"
#include "app_charge.h"

#if ((TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE) \
     && TCFG_CHARGESTORE_PORT == IO_PORT_DP)
#include "asm/chargestore.h"
#endif

#define LOG_TAG_CONST       PC
#define LOG_TAG             "[PC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_PC_ENABLE


struct app_pc_var {
    u8 inited;
    u8 idle;
};

static struct app_pc_var g_pc;

#if(TCFG_TYPE_C_ENABLE)
static u8 pc_key_table[][KEY_EVENT_MAX] = {
    // SHORT           LONG              HOLD
    // UP              DOUBLE            TRIPLE
    {
        KEY_MUSIC_PP,         KEY_CALL_ANSWER,       KEY_NULL,
        KEY_CALL_ANSWER_UP,   KEY_NULL,              KEY_NULL
    },

    {
        KEY_MUSIC_NEXT,    KEY_VOL_UP,         KEY_VOL_UP,
        KEY_NULL,          KEY_NULL,           KEY_NULL
    },

    {
        KEY_MUSIC_PREV,    KEY_VOL_DOWN,       KEY_VOL_DOWN,
        KEY_NULL,          KEY_NULL,           KEY_NULL
    },
};
#else
static u8 pc_key_table[][KEY_EVENT_MAX] = {
    // SHORT           LONG              HOLD
    // UP              DOUBLE            TRIPLE
    {
        KEY_MUSIC_PP,      KEY_POWEROFF,       KEY_POWEROFF_HOLD,
        KEY_NULL,          KEY_MODE_SWITCH,    KEY_NULL
    },

    {
        KEY_MUSIC_NEXT,    KEY_VOL_UP,         KEY_VOL_UP,
        KEY_NULL,          KEY_NULL,           KEY_NULL
    },

    {
        KEY_MUSIC_PREV,    KEY_VOL_DOWN,       KEY_VOL_DOWN,
        KEY_NULL,          KEY_NULL,           KEY_NULL
    },
};
#endif

static u8 pc_idle_query()
{
    return !g_pc.idle;
}
REGISTER_LP_TARGET(pc_lp_target) = {
    .name = "pc",
    .is_idle = pc_idle_query,
};


//*----------------------------------------------------------------------------*/
/**@brief    pc 在线检测  切换模式判断使用
   @param    无
   @return   1 linein设备在线 0 设备不在线
   @note
*/
/*----------------------------------------------------------------------------*/
int pc_app_check(void)
{
    log_info("pc_app_check");
#if TCFG_ONLY_PC_ENABLE
    return true; //不需要检测
#endif
    u32 r = usb_otg_online(0);
    if ((r == SLAVE_MODE) ||
        (r == SLAVE_MODE_WAIT_CONFIRMATION)) {
        return true;
    }
    log_info("pc check offline");
    return false;
}


static void app_pc_exit();
static void app_pc_init(enum app_state state)
{
    log_info("app_pc_init");
    g_pc.idle = 1;
#if TCFG_PC_ENABLE
    clk_set("sys", 96000000); //提高系统时钟,优化usb出盘符速度
    if (pc_app_check() == false) {
        if (state == APP_STA_RESUME) {
            log_info("APP_PC_RESUME err");
            /* APP_STA_RESUME时若设备不在线,不可在该位置切模式,
               因为上一次的app切换还没完成,还未真正进入PC模式 */
            app_pc_exit();
        } else {
            app_task_switch_next();
        }
    } else {
#if TCFG_USB_CDC_BACKGROUND_RUN
        usb_stop();
#endif
        usb_start();
    }
#endif
}

static void app_pc_exit()
{
    log_info("app_pc_exit");
    if (pc_app_check() == false) {
        usb_stop();
    } else {
        usb_pause();
#if TCFG_USB_CDC_BACKGROUND_RUN
        usb_cdc_background_run();
#endif
    }
    g_pc.idle = 0;
}

static int app_pc_key_event_handler(struct sys_event *event)
{
    struct key_event *key = &event->u.key;
    int key_event = pc_key_table[key->value][key->event];
    switch (key_event) {
#if TCFG_USB_SLAVE_HID_ENABLE
    case KEY_MUSIC_PP:
        log_info("KEY_MUSIC_PP");
        hid_key_handler(0, USB_AUDIO_PP);
        break;
    case KEY_MUSIC_NEXT:
        log_info("KEY_MUSIC_NEXT");
#if(TCFG_TYPE_C_ENABLE)
        hid_key_handler(0, USB_AUDIO_PP);
        os_time_dly(2);
        hid_key_handler(0, USB_AUDIO_PP);
#else
        hid_key_handler(0, USB_AUDIO_NEXTFILE);
#endif
        break;
    case KEY_MUSIC_PREV:
        log_info("KEY_MUSIC_PREV");
#if(TCFG_TYPE_C_ENABLE)
        hid_key_handler(0, USB_AUDIO_PP);
        os_time_dly(2);
        hid_key_handler(0, USB_AUDIO_PP);
        os_time_dly(2);
        hid_key_handler(0, USB_AUDIO_PP);
#else
        hid_key_handler(0, USB_AUDIO_PREFILE);
#endif
        break;
    case KEY_VOL_UP:
        log_info("KEY_MUSIC_UP");
        hid_key_handler(0, USB_AUDIO_VOLUP);
        break;
    case KEY_VOL_DOWN:
        log_info("KEY_MUSIC_DOWN");
        hid_key_handler(0, USB_AUDIO_VOLDOWN);
        break;
#if(TCFG_TYPE_C_ENABLE)
    case  KEY_CALL_ANSWER:
        puts("KEY_CALL_ANSWER\n");
        hid_key_handler_send_one_packet(0, USB_AUDIO_PP);
        break;
    case  KEY_CALL_ANSWER_UP:
        puts("KEY_CALL_ANSWER_UP\n");
        hid_key_handler_send_one_packet(0, 0);
        break;
#endif
#endif//end TCFG_USB_SLAVE_HID_ENABLE
    case KEY_MODE_SWITCH:
        log_info("KEY_MODE_SWITCH");
        app_task_switch_next();
        break;
    case KEY_POWEROFF:
    case KEY_POWEROFF_HOLD:
        app_earphone_key_event_handler(event);
        break;
    default:
        break;
    }

    return false;
}


/*
 * 系统事件处理函数
 */
static int event_handler(struct application *app, struct sys_event *event)
{
    /*if (SYS_EVENT_REMAP(event)) {
        g_printf("****SYS_EVENT_REMAP**** \n");
        return 0;
    }*/

    switch (event->type) {
    case SYS_KEY_EVENT:
        /*
         * 普通按键消息处理
         */
        return app_pc_key_event_handler(event);
    case SYS_BT_EVENT:
        /*
         * 蓝牙事件处理
         */
        break;
    case SYS_DEVICE_EVENT:
        /*
         * 系统设备事件处理
         */
        if ((u32)event->arg == DEVICE_EVENT_FROM_OTG) {
            if (event->u.dev.event == DEVICE_EVENT_OUT) {
                app_task_switch_next();
                return true;
            }
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            app_power_event_handler(&event->u.dev);
            return true;
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
#if TCFG_CHARGE_ENABLE
            app_charge_event_handler(&event->u.dev);
#endif
            return true;
        }

        break;
    default:
        return false;
    }

#if(!TCFG_ONLY_PC_ENABLE)
    default_event_handler(event);
#endif

    return false;
}

static int state_machine(struct application *app, enum app_state state,
                         struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        log_info("APP_STA_CREATE");
        STATUS *p_tone = get_tone_config();
        tone_play(TONE_PC_MODE, 1);
        break;
    case APP_STA_START:
        log_info("APP_STA_START");
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_PC_MAIN:
            log_info("ACTION_PC_MAIN");
            ///按键使能
            sys_key_event_enable();
            //关闭自动关机
            sys_auto_shut_down_disable();
#if ((TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE) \
     && TCFG_CHARGESTORE_PORT == IO_PORT_DP)
            power_wakeup_index_enable(2, 0);
            chargestore_api_stop();
#endif
            app_pc_init(state);
            break;
        }
        break;
    case APP_STA_PAUSE:
        log_info("APP_STA_PAUSE");
        app_pc_exit();
        break;
    case APP_STA_RESUME:
        log_info("APP_STA_RESUME");
        app_pc_init(state);
        break;
    case APP_STA_STOP:
        log_info("APP_STA_STOP");
        app_pc_exit();
#if ((TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE) \
     && TCFG_CHARGESTORE_PORT == IO_PORT_DP)
        chargestore_api_restart();
        power_wakeup_index_enable(2, 1);
#endif
        break;
    case APP_STA_DESTROY:
        log_info("APP_STA_DESTROY");
        break;
    }

    return 0;
}

static const struct application_operation app_pc_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

/*
 * 注册pc模式
 */
REGISTER_APPLICATION(app_pc) = {
    .name 	= "pc",
    .action	= ACTION_PC_MAIN,
    .ops 	= &app_pc_ops,
    .state  = APP_STA_DESTROY,
};


#endif

