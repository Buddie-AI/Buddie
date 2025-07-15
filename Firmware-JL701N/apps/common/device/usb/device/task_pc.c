/**
 * @file task_pc.c
 * @brief 从机模式
 * @author chenrixin@zh-jieli.com
 * @version 1.0.0
 * @date 2020-02-29
 */

#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"
#include "app_config.h"
#include "app_action.h"
#include "os/os_api.h"
#include "device/sdmmc.h"

#include "app_charge.h"
#include "asm/charge.h"

#if TCFG_USB_SLAVE_ENABLE
#if  USB_PC_NO_APP_MODE == 0
#include "app_task.h"
#endif
#include "usb/usb_config.h"
#include "usb/device/usb_stack.h"

#if TCFG_USB_SLAVE_HID_ENABLE
#include "usb/device/hid.h"
#endif

#if TCFG_USB_SLAVE_MSD_ENABLE
#include "usb/device/msd.h"
#endif

#if TCFG_USB_SLAVE_CDC_ENABLE
#include "usb/device/cdc.h"
#endif

#if (TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0)
#include "dev_multiplex_api.h"
#endif

#if TCFG_USB_APPLE_DOCK_EN
#include "apple_dock/iAP.h"
#endif

#if (TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE)
#include "cfg_tool.h"
#endif

#if ((TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE) \
     && TCFG_CHARGESTORE_PORT == IO_PORT_DP)
#include "asm/chargestore.h"
#endif

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB_TASK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define     USB_TASK_NAME   "usb_msd"

#define USBSTACK_EVENT		    0x80
#define USBSTACK_MSD_RUN		0x81
#define USBSTACK_MSD_RELASE		0x82
#define USBSTACK_HID		    0x83
#define USBSTACK_MSD_RESET      0x84

extern int usb_audio_demo_init(void);
extern void usb_audio_demo_exit(void);

static usb_dev usbfd = 0;//SEC(.usb_g_bss);
static OS_MUTEX msd_mutex ;//SEC(.usb_g_bss);
static u8 msd_in_task;
static u8 msd_run_reset;


static void usb_task(void *p)
{
    int ret = 0;
    int msg[16];
    while (1) {
        ret = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
        if (msg[0] != Q_MSG) {
            continue;
        }
        switch (msg[1]) {
#if TCFG_USB_SLAVE_MSD_ENABLE
        case USBSTACK_MSD_RUN:
            os_mutex_pend(&msd_mutex, 0);
            msd_in_task = 1;
#if TCFG_USB_APPLE_DOCK_EN
            apple_mfi_link((void *)msg[2]);
#else
            USB_MassStorage((void *)msg[2]);
#endif
            if (msd_run_reset) {
                msd_reset((struct usb_device_t *)msg[2], 0);
                msd_run_reset = 0;
            }
            msd_in_task = 0;
            os_mutex_post(&msd_mutex);
            break;
        case USBSTACK_MSD_RELASE:
            os_sem_post((OS_SEM *)msg[2]);
            while (1) {
                os_time_dly(10000);
            }
            break;
//        case USBSTACK_MSD_RESET:
//            os_mutex_pend(&msd_mutex, 0);
//            msd_reset((struct usb_device_t *)msg[2], (u32)msg[3]);
//            os_mutex_post(&msd_mutex);
//            break;
#endif
        default:
            break;
        }
    }
}

static void usb_msd_wakeup(struct usb_device_t *usb_device)
{
    os_taskq_post_msg(USB_TASK_NAME, 2, USBSTACK_MSD_RUN, usb_device);
}
static void usb_msd_reset_wakeup(struct usb_device_t *usb_device, u32 itf_num)
{
    /* os_taskq_post_msg(USB_TASK_NAME, 3, USBSTACK_MSD_RESET, usb_device, itf_num); */
    if (msd_in_task) {
        msd_run_reset = 1;
    } else {
#if TCFG_USB_SLAVE_MSD_ENABLE
        msd_reset(usb_device, 0);
#endif
    }
}
static void usb_msd_init()
{
    r_printf("%s()", __func__);
    int err;
    os_mutex_create(&msd_mutex);
    err = task_create(usb_task, NULL, USB_TASK_NAME);
    if (err != OS_NO_ERR) {
        r_printf("usb_msd task creat fail %x\n", err);
    }
}
static void usb_msd_free()
{
    r_printf("%s()", __func__);

    os_mutex_del(&msd_mutex, 0);

    int err;
    OS_SEM sem;
    os_sem_create(&sem, 0);
    os_taskq_post_msg(USB_TASK_NAME, 2, USBSTACK_MSD_RELASE, (int)&sem);
    os_sem_pend(&sem, 0);


    err = task_kill(USB_TASK_NAME);
    if (!err) {
        r_printf("usb_msd_uninit succ!!\n");
    } else {
        r_printf("usb_msd_uninit fail!!\n");
    }
}

#if TCFG_USB_SLAVE_CDC_ENABLE
static void usb_cdc_wakeup(struct usb_device_t *usb_device)
{
    //回调函数在中断里，正式使用不要在这里加太多东西阻塞中断，
    //或者先post到任务，由任务调用cdc_read_data()读取再执行后续工作
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 buf[64] = {0};
    static u8 buf_rx[256] = {0};
    static u8 rx_len_total = 0;
    u32 rlen;

    log_debug("cdc rx hook");
    rlen = cdc_read_data(usb_id, buf, 64);

    /* put_buf(buf, rlen);//固件三部测试使用 */
    /* cdc_write_data(usb_id, buf, rlen);//固件三部测试使用 */

    if ((buf[0] == 0x5A) && (buf[1] == 0xAA) && (buf[2] == 0xA5)) {
        memset(buf_rx, 0, 256);
        memcpy(buf_rx, buf, rlen);
        /* log_info("need len = %d\n", buf_rx[5] + 6); */
        /* log_info("rx len = %d\n", rlen); */
        if ((buf_rx[5] + 6) == rlen) {
            rx_len_total = 0;
#if TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE
#if (TCFG_COMM_TYPE == TCFG_USB_COMM)
            /* put_buf(buf_rx, rlen); */
            online_cfg_tool_data_deal(buf_rx, rlen);
#else
            put_buf(buf, rlen);
            cdc_write_data(usb_id, buf, rlen);
#endif
#endif
        } else {
            rx_len_total += rlen;
        }
    } else {
        if ((rx_len_total + rlen) > 256) {
            memset(buf_rx, 0, 256);
            rx_len_total = 0;
            return;
        }
        memcpy(buf_rx + rx_len_total, buf, rlen);
        /* log_info("need len = %d\n", buf_rx[5] + 6); */
        /* log_info("rx len = %d\n", rx_len_total + rlen); */
        if ((buf_rx[5] + 6) == (rx_len_total + rlen)) {
#if TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE
#if (TCFG_COMM_TYPE == TCFG_USB_COMM)
            /* put_buf(buf_rx, rx_len_total + rlen); */
            online_cfg_tool_data_deal(buf_rx, rx_len_total + rlen);
#else
            put_buf(buf, rlen);
            cdc_write_data(usb_id, buf, rlen);
#endif
#endif
            rx_len_total = 0;
        } else {
            rx_len_total += rlen;
        }
    }
}
#endif

void usb_start()
{

#if TCFG_USB_SLAVE_AUDIO_ENABLE
    usb_audio_demo_init();
#endif

#ifdef USB_DEVICE_CLASS_CONFIG
    g_printf("USB_DEVICE_CLASS_CONFIG:%x", USB_DEVICE_CLASS_CONFIG);
#if TCFG_USB_CDC_BACKGROUND_RUN
    usb_device_mode(usbfd, USB_DEVICE_CLASS_CONFIG | CDC_CLASS);
#else
    usb_device_mode(usbfd, USB_DEVICE_CLASS_CONFIG);
#endif

#endif


#if TCFG_USB_SLAVE_MSD_ENABLE
    //没有复用时候判断 sd开关
    //复用时候判断是否参与复用
#if (!TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_SD0_ENABLE)\
     ||(TCFG_SD0_ENABLE && TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_DM_MULTIPLEX_WITH_SD_PORT != 0)
    msd_register_disk("sd0", NULL);
#endif

#if (!TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_SD1_ENABLE)\
     ||(TCFG_SD1_ENABLE && TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0 && TCFG_DM_MULTIPLEX_WITH_SD_PORT != 1)
    msd_register_disk("sd1", NULL);
#endif

#if TCFG_NOR_FAT
    msd_register_disk("fat_nor", NULL);
#endif

#if TCFG_VIR_UDISK_ENABLE
    msd_register_disk("vir_udisk0", NULL);
#endif

    msd_set_wakeup_handle(usb_msd_wakeup);
    msd_set_reset_wakeup_handle(usb_msd_reset_wakeup);
    usb_msd_init();
#endif

#if TCFG_USB_SLAVE_CDC_ENABLE
    cdc_set_wakeup_handler(usb_cdc_wakeup);
#endif
}
static void usb_remove_disk()
{
#if TCFG_USB_SLAVE_MSD_ENABLE
    os_mutex_pend(&msd_mutex, 0);
    msd_unregister_all();
    os_mutex_post(&msd_mutex);
#endif
}
void usb_pause()
{
    log_info("usb pause");

    usb_sie_disable(usbfd);

#if TCFG_USB_SLAVE_MSD_ENABLE
    if (msd_set_wakeup_handle(NULL)) {
        usb_remove_disk();
        usb_msd_free();
    }
#endif


#if TCFG_USB_SLAVE_AUDIO_ENABLE
    usb_audio_demo_exit();
#endif

    usb_device_mode(usbfd, 0);
}

void usb_stop()
{
    log_info("App Stop - usb");

    usb_pause();

    usb_sie_close(usbfd);
}

#if TCFG_USB_CDC_BACKGROUND_RUN
void usb_cdc_background_run()
{
    g_printf("CDC is running in the background");
    usb_device_mode(0, CDC_CLASS);
    cdc_set_wakeup_handler(usb_cdc_wakeup);
}
#endif

int pc_device_event_handler(struct sys_event *event)
{
    if ((int)event->arg != DEVICE_EVENT_FROM_OTG) {
        return false;
    }

    int switch_app_case = false;
    const char *usb_msg = (const char *)event->u.dev.value;
    log_debug("usb event : %d DEVICE_EVENT_FROM_OTG %s", event->u.dev.event, usb_msg);

    if (usb_msg[0] == 's') {
        if (event->u.dev.event == DEVICE_EVENT_IN) {
            log_info("usb %c online", usb_msg[2]);
            usbfd = usb_msg[2] - '0';
#if   USB_PC_NO_APP_MODE
            usb_start();
#elif TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0

#if TCFG_USB_CDC_BACKGROUND_RUN
            mult_sdio_suspend();
            usb_cdc_background_run();
            switch_app_case = 0;
#else
            usb_otg_suspend(0, OTG_KEEP_STATE);
            mult_sdio_suspend();
            usb_pause();
            mult_sdio_resume();
            switch_app_case = 0;
#endif//TCFG_USB_CDC_BACKGROUND_RUN

#else


#if (TWFG_APP_POWERON_IGNORE_DEV && TCFG_USB_CDC_BACKGROUND_RUN && TCFG_PC_ENABLE)
            if (jiffies_to_msecs(jiffies) < TWFG_APP_POWERON_IGNORE_DEV) {
                usb_cdc_background_run();
                switch_app_case = 0;
            } else {
                usb_pause();
                switch_app_case = 1;
            }
#elif (TCFG_USB_CDC_BACKGROUND_RUN)
            usb_cdc_background_run();
            switch_app_case = 0;

#elif (TWFG_APP_POWERON_IGNORE_DEV == 0)
            usb_pause();
            switch_app_case = 1;

#else
            usb_pause();
            switch_app_case = 1;
#endif

#endif
        } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
            log_info("usb %c offline", usb_msg[2]);
#if  USB_PC_NO_APP_MODE
            usb_stop();
#else

#ifdef CONFIG_SOUNDBOX
            if (true == app_check_curr_task(APP_PC_TASK)) {
#else
            if (app_get_curr_task() == APP_PC_TASK) {
#endif
                switch_app_case = 2;
            } else {
                /* 此处的模式限制会在USB已连接,蓝牙播歌切换到蓝牙模式后再拔出USB的情况下,
                   无法检测到再次插入USB */
#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                mult_sdio_suspend();
#endif
                usb_stop();
#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
                mult_sdio_resume();
#endif
            }

#endif
        }
    }

    return switch_app_case;
}

#if  USB_PC_NO_APP_MODE
void usbstack_init()
{
    register_sys_event_handler(SYS_DEVICE_EVENT, DEVICE_EVENT_FROM_OTG, 2,
                               (void (*)(struct sys_event *))pc_device_event_handler);
}

void usbstack_exit()
{
    unregister_sys_event_handler((void (*)(struct sys_event *))pc_device_event_handler);
}
#endif

#if (TCFG_OTG_MODE & OTG_DET_DM_ONLY)
static void otg_sof_check_before_hook()
{
    log_debug("sof_check_before");
#if ((TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE) \
     && TCFG_CHARGESTORE_PORT == IO_PORT_DP)
    power_wakeup_index_enable(2, 0);
    chargestore_api_stop();
#endif
}

static void otg_sof_check_after_hook()
{
    log_debug("sof_check_after");
#if ((TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE || TCFG_ANC_BOX_ENABLE) \
     && TCFG_CHARGESTORE_PORT == IO_PORT_DP)
    chargestore_api_restart();
    power_wakeup_index_enable(2, 1);
#endif
}

static int otg_sof_check_hook_register()
{
    //需要在otg定时器启动之前（即devices_init()之前）注册钩子函数
    usb_otg_sof_check_register_hooks(otg_sof_check_before_hook,
                                     otg_sof_check_after_hook);
    return 0;
}
early_initcall(otg_sof_check_hook_register);
#endif

#endif
