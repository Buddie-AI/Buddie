#include "system/includes.h"
#include "music/music_player.h"
#include "music/breakpoint.h"
#include "app_action.h"
#include "app_main.h"
#include "earphone.h"
#include "key_event_deal.h"
#include "audio_config.h"
#include "bt_background.h"
#include "default_event_handler.h"
#include "app_online_cfg.h"
#include "user_cfg.h"
#include "tone_player.h"
#include "app_task.h"
#include "earphone.h"
#include "update.h"
#include "app_power_manage.h"
#include "app_charge.h"

#define LOG_TAG_CONST       MUSIC
#define LOG_TAG             "[MUSIC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_APP_MUSIC_EN
#define VALUE_NULL	0

//模式idle状态
static volatile u8 is_music_active = 0;

///模式参数结构体
struct __music_task_parm {
    u8 type;
    int val;
};

///music模式控制结构体
struct __music {
    struct __music_task_parm task_parm;
    u16 file_err_counter;//错误文件统计
    u8 file_play_direct;//0:下一曲， 1：上一曲
    u8 scandisk_break;//扫描设备打断标志
    char device_tone_dev[16];
#if SD_BAUD_RATE_CHANGE_WHEN_SCAN
    u32 old_speed;
#endif
};
struct __music music_hdl = {0};
#define __this (&music_hdl)
static struct __breakpoint *breakpoint = NULL;

static const u8 music_key_table[][KEY_EVENT_MAX] = {
    // SHORT           LONG              HOLD
    // UP              DOUBLE            TRIPLE
    {
        KEY_MUSIC_PP,   KEY_POWEROFF,      KEY_POWEROFF_HOLD,
        KEY_NULL,       KEY_MODE_SWITCH,   KEY_NULL
    },
    {
        KEY_MUSIC_NEXT, KEY_VOL_UP,        KEY_VOL_UP,
        KEY_NULL,       KEY_NULL,          KEY_NULL
    },
    {
        KEY_MUSIC_PREV, KEY_VOL_DOWN,      KEY_VOL_DOWN,
        KEY_NULL,       KEY_NULL,          KEY_NULL
    },
};


//*----------------------------------------------------------------------------*/
/**@brief    检测能否进入低功耗
   @param    无
   @return   无
   @note     无
   @note     放在按键事件处理后
*/
/*----------------------------------------------------------------------------*/
static void music_powerdown_enter_check(void)
{
    u8 music_play_status = 0;
    music_play_status = music_player_get_play_status();
    if ((is_music_active == 1) && (music_play_status == FILE_DEC_STATUS_PAUSE)) {
        is_music_active = 0;
    }
    /* log_info(">>>>>>>>>>>>>>>enter music_play_status = %d\n", music_play_status); */
}

//*----------------------------------------------------------------------------*/
/**@brief    检测能否退出低功耗
   @param    无
   @return   无
   @note     无
   @note     放在按键事件处理前
*/
/*----------------------------------------------------------------------------*/
static void music_powerdown_exit_check(void)
{
    u8 music_play_status = 0;
    music_play_status = music_player_get_play_status();
    //非暂停的自定义按键事件, 把进低功耗的定时器删除
    if (music_play_status == FILE_DEC_STATUS_PAUSE) {
        if (is_music_active == 0) {
            is_music_active = 1;
        }
    }
    /* log_info(">>>>>>>>>>>>>>>exit music_play_status = %d\n", music_play_status); */
}

//*----------------------------------------------------------------------------*/
/**@brief    music 解码成功回调
   @param    priv:私有参数， parm:暂时未用
   @return
   @note	 此处可以做一些用户操作， 如断点保存， 显示， 获取播放信息等
*/
/*----------------------------------------------------------------------------*/
static void music_player_play_success(void *priv, int parm)
{
    char *logo = music_player_get_dev_cur();
    log_info("\n\n----------------music_player_play_success----------------------\n");
    log_info("cur dev = %s\n", logo);
    log_info("total dev = %d\n", dev_manager_get_total(1));
    log_info("cur filenum = %d\n", music_player_get_file_cur());
    log_info("totol filenum = %d\n", music_player_get_file_total());
    log_info("totol time = %d\n", music_player_get_dec_total_time());
    log_info("sclust = %d\n", music_player_get_file_sclust());
    log_info("dir_cur = %d\n", music_player_get_dir_cur());
    log_info("dir_total = %d\n", music_player_get_dir_total());
    log_info("file indir = %d\n", music_player_get_fileindir_number());
    log_info("\n");
    ///save breakpoint, 只保存文件信息
    if (music_player_get_playing_breakpoint(breakpoint, 0) == true) {
        breakpoint_vm_write(breakpoint, logo);
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    music 发送自定义按键消息
   @param    msg:按键类型; value:发送的值
   @return   0:发送正常 -1:发送失败
   @note	 虚拟按键消息
*/
/*----------------------------------------------------------------------------*/
static int app_music_task_put_key_msg(int msg, int value)
{
    if (app_get_curr_task() != APP_MUSIC_TASK) {
        //不在音乐模式不发自定义按键消息
        log_w("custom msg invalid!");
        return -1;
    }
    struct sys_event e;
    e.type = SYS_KEY_EVENT;
    e.u.key.event = msg;
    e.u.key.value = value;
    e.arg  = (void *)DEVICE_EVENT_FROM_CUSTOM;
    sys_event_notify(&e);
    return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    music 解码结束回调处理
   @param
   @return
   @note	此处统一将错误通过消息的方式发出， 在key msg中统一响应
*/
/*----------------------------------------------------------------------------*/
static void music_player_play_end(void *priv, int parm)
{
    log_info("music_player_play_end\n");
    ///这里推出消息， 目的是在music主流程switch case统一入口
    app_music_task_put_key_msg(KEY_MUSIC_PLAYER_END, parm);
}
//*----------------------------------------------------------------------------*/
/**@brief    music 解码错误回调
   @param
   @return
   @note	此处统一将错误通过消息的方式发出， 在key msg中统一响应
*/
/*----------------------------------------------------------------------------*/
static void music_player_decode_err(void *priv, int parm)
{
    log_info("music_player_decode_err\n");
    ///这里推出消息， 目的是在music主流程switch case统一入口
    app_music_task_put_key_msg(KEY_MUSIC_PLAYER_DEC_ERR, parm);
}

static const struct __player_cb music_player_callback = {
    .start 		= music_player_play_success,
    .end   		= music_player_play_end,
    .err		= music_player_decode_err,
//    .fsn_break  = music_player_scandisk_break,
};

static const struct __scan_callback scan_cb = {
    .enter = NULL,
    .exit = NULL,
    .scan_break = NULL,
};

//*----------------------------------------------------------------------------*/
/**@brief    music 模式初始化处理
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void music_task_start()
{
    char *log = NULL;
    log_info("music task start!!!!!\n");
    ///时钟初始化
    clk_set("sys", CONFIG_MUSIC_DEC_CLOCK);
    ///按键使能
    sys_key_event_enable();
    //关闭自动关机
    sys_auto_shut_down_disable();

    ///播放器初始化
    struct __player_parm parm = {0};
    parm.cb = &music_player_callback;
    parm.scan_cb = &scan_cb;
    music_player_creat(NULL, &parm);
    music_player_set_repeat_mode(FCYCLE_ALL);
    ///获取断点句柄， 后面所有断点读/写都需要用到
    breakpoint = breakpoint_handle_creat();
    ///初始化一些参数
    __this->file_err_counter = 0;
    __this->file_play_direct = 0;
    __this->scandisk_break = 0;
    is_music_active = 1;
}

//*----------------------------------------------------------------------------*/
/**@brief    开始播歌
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void music_player_play_start(void)
{
    app_music_task_put_key_msg(KEY_MUSIC_PLAYER_START, VALUE_NULL);
}

static void music_save_breakpoint(int save_dec_bp)
{
    char *logo = music_player_get_dev_cur();

    ///save breakpoint, 只保存文件信息
    if (music_player_get_playing_breakpoint(breakpoint, save_dec_bp) == true) {
        breakpoint_vm_write(breakpoint, logo);
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    music 模式退出处理
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void music_task_close()
{
    music_save_breakpoint(1);
    music_player_stop(1);
    breakpoint_handle_destroy(&breakpoint);
    music_player_destroy();
    memset(__this, 0, sizeof(struct __music));
    is_music_active = 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    恢复音乐模式
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void music_task_resume(void)
{
    music_task_start();
    music_player_play_start();
}

//*----------------------------------------------------------------------------*/
/**@brief    清除自定义按键消息
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
static void custom_event_clr(void)
{
    struct sys_event clear_key_event = {
        .type =  SYS_KEY_EVENT,
        .arg = (void *)DEVICE_EVENT_FROM_CUSTOM,
    };
    sys_key_event_disable();
    sys_event_clear(&clear_key_event);
}

//*----------------------------------------------------------------------------*/
/**@brief    music 在线检测  切换模式判断使用
   @param    无
   @return   1 设备在线 0 设备不在线
   @note
*/
/*----------------------------------------------------------------------------*/
int music_app_check(void)
{
    log_info("music_app_check");
    if (dev_manager_get_total(1)) {
        return true;
    }
    return false;
}
//*----------------------------------------------------------------------------*/
/**@brief    music 模式解码错误处理
   @param    err:错误码，详细错误码描述请看music_player错误码表枚举
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void music_player_err_deal(int err)
{
    u16 msg = KEY_NULL;
    char *logo = NULL;
    if (err != MUSIC_PLAYER_ERR_NULL && err != MUSIC_PLAYER_SUCC) {
        log_e("music player err = %d\n", err);
    }

    switch (err) {
    case MUSIC_PLAYER_SUCC:
        __this->file_err_counter = 0;
        break;
    case MUSIC_PLAYER_ERR_NULL:
        break;
    case MUSIC_PLAYER_ERR_POINT:
    case MUSIC_PLAYER_ERR_NO_RAM:
        msg = KEY_MUSIC_PLAYER_QUIT;//退出音乐模式
        break;
    case MUSIC_PLAYER_ERR_DECODE_FAIL:
        if (__this->file_err_counter >= music_player_get_file_total()) {
            __this->file_err_counter = 0;
            dev_manager_set_valid_by_logo(music_player_get_dev_cur(), 0);///将设备设置为无效设备
            if (dev_manager_get_total(1) == 0) {//参数为1 ：获取所有有效设备  参数0：获取所有设备
                msg = KEY_MUSIC_PLAYER_QUIT;//没有设备了，退出音乐模式
            } else {
                msg = KEY_MUSIC_AUTO_NEXT_DEV;///所有文件都是错误的， 切换到下一个设备
            }
        } else {
            __this->file_err_counter ++;
            if (__this->file_play_direct == 0) {
                msg = KEY_MUSIC_NEXT;//播放下一曲
            } else {
                msg = KEY_MUSIC_PREV;//播放上一曲
            }
        }
        break;
    case MUSIC_PLAYER_ERR_DEV_NOFOUND:
        if (dev_manager_get_total(1) == 0) {//参数为1 ：获取所有有效设备  参数0：获取所有设备
            msg = KEY_MUSIC_PLAYER_QUIT;///没有设备在线， 退出音乐模式
        } else {
            msg = KEY_MUSIC_PLAYER_START;///没有找到指定设备， 播放之前的活动设备
        }
        break;

    case MUSIC_PLAYER_ERR_FSCAN:
        ///需要结合music_player_scandisk_break中处理的标志位处理
        if (__this->scandisk_break) {
            __this->scandisk_break = 0;
            ///此处不做任何处理， 打断的事件已经重发， 由重发事件执行后续处理
            break;
        }
    case MUSIC_PLAYER_ERR_DEV_READ:
    case MUSIC_PLAYER_ERR_DEV_OFFLINE:
        log_e("MUSIC_PLAYER_ERR_DEV_OFFLINE \n");
        logo = music_player_get_dev_cur();
        if (dev_manager_online_check_by_logo(logo, 1)) {
            ///如果错误失败在线， 并且是播放过程中产生的，先记录下断点
            if (music_player_get_playing_breakpoint(breakpoint, 1) == true) {
                music_player_stop(0);//先停止，防止下一步操作VM卡顿
                breakpoint_vm_write(breakpoint, logo);
            }
            if (err == MUSIC_PLAYER_ERR_FSCAN) {
                dev_manager_set_valid_by_logo(logo, 0);///将设备设置为无效设备
            } else {
                //针对读错误， 因为时间推到应用层有延时导致下一个模式判断不正常， 此处需要将设备卸载
                dev_manager_unmount(dev_manager_get_logo(dev_manager_find_active(0)));
            }
        }
        if (dev_manager_get_total(1) == 0) {
            msg = KEY_MUSIC_PLAYER_QUIT;///没有设备在线， 退出音乐模式
        } else {
            msg = KEY_MUSIC_AUTO_NEXT_DEV;///切换设备
        }
        break;
    case MUSIC_PLAYER_ERR_FILE_NOFOUND:
        ///查找文件有扫盘的可能，也需要结合music_player_scandisk_break中处理的标志位处理
        if (__this->scandisk_break) {
            __this->scandisk_break = 0;
            ///此处不做任何处理， 打断的事件已经重发， 由重发事件执行后续处理
            break;
        }
    case MUSIC_PLAYER_ERR_PARM:
        logo = music_player_get_dev_cur();
        if (dev_manager_online_check_by_logo(logo, 1)) {
            if (music_player_get_file_total()) {
                msg = KEY_MUSIC_PLAYER_PLAY_FIRST;///有文件,播放第一个文件
                break;
            }
        }

        if (dev_manager_get_total(1) == 0) {
            msg = KEY_MUSIC_PLAYER_QUIT;//没有设备了，退出音乐模式
        } else {
            msg = KEY_MUSIC_AUTO_NEXT_DEV;
        }
        break;
    case MUSIC_PLAYER_ERR_FILE_READ://文件读错误
        msg = KEY_MUSIC_NEXT;//播放下一曲
        break;
    }

    if (msg != KEY_NULL) {
        app_music_task_put_key_msg(msg, VALUE_NULL);
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    music 模式按键处理
   @param    按键消息
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static int app_music_key_event_handler(struct sys_event *event)
{
    int ret = true;
    int err = MUSIC_PLAYER_ERR_NULL;
    u8 vol, auto_next_dev;
    int mode ;
    char *logo = NULL;
    int key_event = 0;

    struct key_event *key = &event->u.key;
    if (event->arg == DEVICE_EVENT_FROM_KEY) {
        key_event = music_key_table[key->value][key->event];//物理按键事件
    } else {
        key_event = key->event;//软件按键事件
    }
    log_info("music task msg = %d\n", key_event);
    music_powerdown_exit_check();
    switch (key_event) {
    case KEY_MUSIC_PLAYER_START:
        log_info("KEY_MUSIC_PLAYER_START !!\n");
        ///断点播放活动设备
        logo = dev_manager_get_logo(dev_manager_find_active(1));
        if (music_player_get_play_status() == FILE_DEC_STATUS_PLAY) {
            if (music_player_get_dev_cur() && logo) {
                ///播放的设备跟当前活动的设备是同一个设备，不处理
                if (0 == strcmp(logo, music_player_get_dev_cur())) {
                    log_w("the same dev!!\n");
                    break;
                }
            }
        }
        if (true == breakpoint_vm_read(breakpoint, logo)) {
            err = music_player_play_by_breakpoint(logo, breakpoint);
        } else {
            err = music_player_play_first_file(logo);
        }
        break;
    ///播放器退出处理
    case KEY_MUSIC_PLAYER_QUIT:
        log_info("KEY_MUSIC_PLAYER_QUIT !!\n");
        app_task_switch_next();
        break;
    ///结束消息处理
    case KEY_MUSIC_PLAYER_END:
        log_info("KEY_MUSIC_PLAYER_END\n");
        err = music_player_end_deal(key->value);
        break;
    //播放器解码错误处理
    case KEY_MUSIC_PLAYER_DEC_ERR:
        err = music_player_decode_err_deal(key->value);
        break;
    ///播放执行类消息
    case  KEY_MUSIC_PP:
        log_info("KEY_MUSIC_PP\n");
        err = music_player_pp();
        log_info("=============================================%d\n", music_player_get_play_status());
        break;
    case KEY_MUSIC_PLAYER_AUTO_NEXT:
        log_info("KEY_MUSIC_PLAYER_AUTO_NEXT\n");
        err = music_player_play_auto_next();
        break;
    case KEY_MUSIC_PLAYER_PLAY_FIRST:
        log_info("KEY_MUSIC_PLAYER_PLAY_FIRST\n");
        err = music_player_play_first_file(NULL);
        break;
    case  KEY_MUSIC_PREV:
        log_info("KEY_MUSIC_PREV\n");
        __this->file_play_direct = 1;
        err = music_player_play_prev();
        break;
    case  KEY_MUSIC_NEXT:
        log_info("KEY_MUSIC_NEXT\n");
        __this->file_play_direct = 0;
        err = music_player_play_next();
        break;
    case KEY_MUSIC_PLAYE_PREV_FOLDER:
        log_info("KEY_MUSIC_PLAYE_PREV_FOLDER\n");
        err = music_player_play_folder_prev();
        break;
    case KEY_MUSIC_PLAYE_NEXT_FOLDER:
        log_info("KEY_MUSIC_PLAYE_NEXT_FOLDER\n");
        err = music_player_play_folder_next();
        break;

    case KEY_MUSIC_AUTO_NEXT_DEV:
    case  KEY_MUSIC_CHANGE_DEV:
        log_info("KEY_MUSIC_CHANGE_DEV\n");
        auto_next_dev = ((key_event == KEY_MUSIC_AUTO_NEXT_DEV) ? 1 : 0);
        logo = music_player_get_dev_next(auto_next_dev);
        log_info("next dev = %s\n", logo);
        if (logo == NULL) { ///找不到下一个设备，不响应设备切换
            break;
        }
        ///切换设备前先保存一下上一个设备的断点信息,包括文件和解码信息
        if (music_player_get_playing_breakpoint(breakpoint, 1) == true) {
            music_player_stop(0);//先停止，防止下一步操作VM卡顿
            breakpoint_vm_write(breakpoint, music_player_get_dev_cur());
        }
        if (true == breakpoint_vm_read(breakpoint, logo)) {
            err = music_player_play_by_breakpoint(logo, breakpoint);
        } else {
            err = music_player_play_first_file(logo);
        }
        break;
    case KEY_MUSIC_PLAYE_BY_DEV_FILENUM:
        log_info("KEY_MUSIC_PLAYE_BY_DEV_FILENUM, file_number = %d\n", key->value);
        logo = dev_manager_get_logo(dev_manager_find_active(1));
        err = music_player_play_by_number(logo, key->value);
        break;
    case KEY_MUSIC_PLAYE_BY_DEV_SCLUST:
        log_info("KEY_MUSIC_PLAYE_BY_DEV_SCLUST\n");
        logo = dev_manager_get_logo(dev_manager_find_active(1));
        err = music_player_play_by_sclust(logo, key->value);
        break;
    case KEY_MUSIC_PLAYE_BY_DEV_PATH:
        log_info("KEY_MUSIC_PLAYE_BY_DEV_PATH\n");
        err = music_player_play_by_path((char *)"udisk0", "/sin.wav");///this is a demo
        break;

    ///非播放执行类消息
    case KEY_MUSIC_FF:
        log_info("KEY_MUSIC_FF\n");
        music_player_ff(3);
        break;
    case KEY_MUSIC_FR:
        log_info("KEY_MUSIC_FR\n");
        music_player_fr(3);
        break;
    case KEY_MUSIC_CHANGE_REPEAT:
        log_info("KEY_MUSIC_CHANGE_REPEAT\n");
        mode = music_player_change_repeat_mode();
        break;
    case KEY_MUSIC_DELETE_FILE:
        log_info("KEY_MUSIC_DELETE_FILE\n");
        err = music_player_delete_playing_file();
        break;
    case KEY_MUSIC_PLAYER_AB_REPEAT_SWITCH:
        /* file_dec_ab_repeat_switch(); */
        break;

    case KEY_MODE_SWITCH:
        log_info("KEY_MUSIC_SWITCH\n");
        app_task_switch_next();
        break;
    case KEY_VOL_UP:
        log_info("KEY_VOL_UP\n");
        if (!tone_get_status()) {
            app_audio_volume_up(1);
            log_info("vol+: %d", app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
        }
        if (app_audio_get_volume(APP_AUDIO_CURRENT_STATE) == app_audio_get_max_volume()) {
            if (tone_get_status() == 0) {
#if TCFG_MAX_VOL_PROMPT
                STATUS *p_tone = get_tone_config();
                tone_play_index(p_tone->max_vol, 0);
#endif
            }
        }
        break;
    case KEY_VOL_DOWN:
        log_info("KEY_VOL_DOWN\n");
        app_audio_volume_down(1);
        log_info("vol-: %d", app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
        break;
    case KEY_POWEROFF:
    case KEY_POWEROFF_HOLD:
        app_earphone_key_event_handler(event);
        break;
    default:
        ret = false;
        break;
    }

    music_powerdown_enter_check();
    //退出模式后，不做错误处理
    if (is_music_active == 0) {
        return false;
    }
    ///错误处理
    log_info("music play err = %d\n", err);
    music_player_err_deal(err);
    return false;
}

/*
 * 系统事件处理函数
 */
static int event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        /*
         * 普通按键消息处理
         */
        return app_music_key_event_handler(event);
    case SYS_BT_EVENT:
        /*
         * 蓝牙事件处理
         */
        break;
    case SYS_DEVICE_EVENT:
        /*
         * 系统设备事件处理
         */
        if ((u32)event->arg == DRIVER_EVENT_FROM_SD0) {
            if (event->u.dev.event == DEVICE_EVENT_OUT) {
                update_clear_result();
                music_save_breakpoint(1);
                dev_manager_del((char *)event->u.dev.value);
                app_task_switch_next();
                return true;
            }
        }
        if ((u32)event->arg == DRIVER_EVENT_FROM_SD0) {
#if TCFG_ONLINE_ENABLE
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CI_UART) {
            ci_data_rx_handler(CI_UART);
#if TCFG_USER_TWS_ENABLE
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CI_TWS) {
            ci_data_rx_handler(CI_TWS);
#endif
#endif
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            app_power_event_handler(&event->u.dev);
            return true;
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
            app_charge_event_handler(&event->u.dev);
            return true;
        }
        break;
    default:
        break;
    }
    default_event_handler(event);
    return false;
}

static int state_machine(struct application *app, enum app_state state,
                         struct intent *it)
{
    int err = 0;
    int tone_play_err = 0;
    switch (state) {
    case APP_STA_CREATE: {
        log_info("APP_STA_CREATE\n");
        //播完提示音再开始播SD卡
        tone_play_err = tone_play_with_callback(TONE_MUSIC_MODE, 1, music_player_play_start, NULL);
        if (tone_play_err == -EINVAL) {
            log_info("tone play err! music_player_play_start!\n");
            music_player_play_start();
        }
    }
    break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_MUSIC_MAIN:
            log_info("ACTION_MUSIC_MAIN\n");
            music_task_start();
            break;
        case ACTION_MUSIC_TWS_RX:
            break;
        }
        break;
    case APP_STA_PAUSE:
        log_info("APP_STA_PAUSE\n");
        music_task_close();
        break;
    case APP_STA_RESUME:
        log_info("APP_STA_RESUME\n");
        music_task_resume();
        break;
    case APP_STA_STOP:
        log_info("APP_STA_STOP\n");
        music_task_close();
        break;
    case APP_STA_DESTROY:
        log_info("APP_STA_DESTROY\n");
        custom_event_clr();
        break;
    }
    return err;
}

static const struct application_operation app_music_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

/*
 * 注册music模式
 */
REGISTER_APPLICATION(app_music) = {
    .name 	= "music",
    .action	= ACTION_MUSIC_MAIN,
    .ops 	= &app_music_ops,
    .state  = APP_STA_DESTROY,
};

/*
 * 注册idle状态
 */
static u8 music_idle_query(void)
{
    return !is_music_active;
}
REGISTER_LP_TARGET(music_lp_target) = {
    .name = "music",
    .is_idle = music_idle_query,
};

#endif

