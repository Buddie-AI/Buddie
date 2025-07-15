#include "adapter_idev.h"
#include "adapter_odev.h"
#include "adapter_idev_bt.h"
//#include "adapter_odev_dac.h"
#include "adapter_process.h"
#include "adapter_media.h"
//#include "application/audio_dig_vol.h"
#include "le_client_demo.h"
#include "system/malloc.h"
#include "key_event_deal.h"

#if TCFG_WIRELESS_MIC_ENABLE

#define WIRELESS_MIC_SLAVE_OUTPUT_DAC 			0
#define WIRELESS_MIC_SLAVE_OUTPUT_USB_MIC 		1
#define WIRELESS_MIC_SLAVE_OUTPUT_SEL 			WIRELESS_MIC_SLAVE_OUTPUT_DAC//WIRELESS_MIC_SLAVE_OUTPUT_DAC//WIRELESS_MIC_SLAVE_OUTPUT_USB_MIC//


#if (APP_MAIN ==  APP_WIRELESS_MIC)


extern u8 get_charge_online_flag(void);
extern u8 app_common_device_event_deal(struct sys_event *event);

//bt idev config
struct _idev_bt_parm idev_bt_parm_list = {
    .mode = BIT(IDEV_BLE),
};

//bt odev config
static const u8 dongle_remoter_name1[] = "S2_MIC_0";

static const client_match_cfg_t match_dev01 = {
    .create_conn_mode = BIT(CLI_CREAT_BY_NAME),
    .compare_data_len = sizeof(dongle_remoter_name1) - 1, //去结束符
    .compare_data = dongle_remoter_name1,
    .bonding_flag = 0,
};

static client_conn_cfg_t ble_conn_config = {
    .match_dev_cfg[0] = &match_dev01,      //匹配指定的名字
    .match_dev_cfg[1] = NULL,
    .match_dev_cfg[2] = NULL,
    .search_uuid_cnt = 0,
    .security_en = 0,       //加密配对
};

struct _odev_bt_parm odev_bt_parm_list = {
    .mode = BIT(ODEV_BLE), //only support BLE

    //for BLE config
    .ble_parm.cfg_t = &ble_conn_config,
};


u8 wl_mic_mode = 0;

void wireless_mic_change_mode(u8 mode)
{
#ifdef APP_WIRELESS_MIC_MASTER
    return;
#endif
    wl_mic_mode = mode;
    printf("%s = %d", __func__, wl_mic_mode);
    syscfg_write(CFG_USER_WIRELESSMIC_MODE, &wl_mic_mode, sizeof(wl_mic_mode));
    os_time_dly(1);
    cpu_reset();
}
extern u8 key_table[KEY_NUM_MAX][KEY_EVENT_MAX];
static int adapter_key_event_handler(struct sys_event *event)
{
    int ret = 0;
    struct key_event *key = &event->u.key;

    u8 key_event = event->u.key.event;

    key_event = key_table[key->value][key->event];
    static u8 cnt = 0;
    printf("key_event:%d %d %d\n", key_event, key->value, key->event);
    switch (key_event) {
    case  KEY_POWEROFF:
        printf("KEY_EVENT_LONG\n");
        wireless_mic_change_mode(1);
        ret = 1;
        break;
    case  KEY_NULL:
        break;
    }
    return ret;
}

extern int app_charge_event_handler(struct device_event *dev);
static int event_handle_callback(struct sys_event *event)
{
    //处理用户关注的事件
    int ret = 0;
    switch (event->type) {
    case SYS_KEY_EVENT:
        ret = adapter_key_event_handler(event);
        break;
    case SYS_DEVICE_EVENT:
        //app_common_device_event_deal(event);
        if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
#if TCFG_CHARGE_ENABLE
            return app_charge_event_handler(&event->u.dev);
#endif
        }
        break;
    default:
        break;

    }
    return ret;
}


void wireless_mic_main_run(void)
{
    int ret = 0;
    printf("%s\n", __func__);
#ifdef APP_WIRELESS_MIC_SLAVE
    ret = syscfg_read(CFG_USER_WIRELESSMIC_MODE, &wl_mic_mode, sizeof(wl_mic_mode));
    printf("sizeof(wl_mic_mode) = %d,ret = %d", sizeof(wl_mic_mode), ret);
    if (sizeof(wl_mic_mode) == ret) {
        printf("wl_mic_mode = %d", wl_mic_mode);
    } else {
        wl_mic_mode = 0;
        printf("wl_mic_mode = %d,read_micless_mode_err!!!!", wl_mic_mode);
    }

    if (wl_mic_mode == 1) {
        printf("return EARPHONE MODE");
        return;
    }
#endif

    while (1) {
        //初始化
#ifdef APP_WIRELESS_MIC_MASTER
        struct idev *idev = adapter_idev_open(ADAPTER_IDEV_MIC, NULL);
        struct odev *odev = adapter_odev_open(ADAPTER_ODEV_BT, (void *)&odev_bt_parm_list);
        struct adapter_media *media = adapter_media_open(NULL);
        printf("app_main_wireless_master ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
#endif//APP_WIRELESS_MIC_MASTER

#ifdef APP_WIRELESS_MIC_SLAVE
        struct idev *idev = adapter_idev_open(ADAPTER_IDEV_BT, (void *)&idev_bt_parm_list);
#if (WIRELESS_MIC_SLAVE_OUTPUT_SEL == WIRELESS_MIC_SLAVE_OUTPUT_USB_MIC)
        struct odev *odev = adapter_odev_open(ADAPTER_ODEV_USB, NULL);
#else
        struct odev *odev = adapter_odev_open(ADAPTER_ODEV_DAC, NULL);
#endif
        struct adapter_media *media = adapter_media_open(NULL);
        printf("app_main_wireless_slave ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
#endif//APP_WIRELESS_MIC_SLAVE
        ASSERT(idev);
        ASSERT(odev);
        ASSERT(media);
        struct adapter_pro *pro = adapter_process_open(idev, odev, media, event_handle_callback);//event_handle_callback 用户想拦截处理的事件

        ASSERT(pro, "adapter_process_open fail!!\n");

        //执行(包括事件解析、事件执行、媒体启动/停止, HID等事件转发)
        adapter_process_run(pro);

        //退出/关闭
        adapter_process_close(&pro);
        adapter_media_close(media);
        adapter_idev_close(idev);
        adapter_odev_close(odev);

        ///run idle off poweroff
        printf("enter poweroff !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        if (get_charge_online_flag()) {
            printf("charge_online,cpu reset");
            cpu_reset();
        } else {
            power_set_soft_poweroff();
        }
    }
}


#endif //(APP_MAIN ==  APP_WIRELESS_MIC)
#else
#include "system/includes.h"
void app_main_run(void)
{
    printf("app_main_wireless_mic");
}
#endif
