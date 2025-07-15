
/*************************************************************

             此文件函数主要是蓝牙发射器接口处理

1、关于AG对讲功能使用说明：
   发射器需要通过user_emitter_cmd_prepare接口来给接收器发送命令,例如：
   (1)发射器主动发起通话,发起通话前需要暂停高级音频
   user_emitter_cmd_prepare(USER_CTRL_HFP_CALL_LAST_NO, 0, NULL)

   (2)发射器主动关断通话
   user_emitter_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL)

   (3)接收器来电接听
   user_emitter_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL)

**************************************************************/

#include "system/app_core.h"
#include "system/includes.h"

#include "app_config.h"
#include "app_task.h"

#include "btstack/btstack_task.h"
#include "btcontroller_modules.h"
#include "btstack/avctp_user.h"
#include "classic/hci_lmp.h"

#include "earphone.h"
#include "bt_emitter.h"
#include "user_cfg.h"
#include "vm.h"
#include "app_main.h"
/* #include "common/app_common.h" */

#include "key_event_deal.h"
#include "rcsp_bluetooth.h"

#include "media/includes.h"
#include "audio_config.h"
#include "audio_digital_vol.h"
#include "audio_enc.h"
/* #include "audio_enc_transmitter.h" */
#include "media/file_decoder.h"

#include "music/music_player.h"


#if TCFG_USER_EMITTER_ENABLE

#define LOG_TAG_CONST        EARPHONE
#define LOG_TAG             "[EARPHONE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"




#define  SEARCH_BD_ADDR_LIMITED 0
#define  SEARCH_BD_NAME_LIMITED 1
#define  SEARCH_CUSTOM_LIMITED  2
#define  SEARCH_NULL_LIMITED    3

#define SEARCH_LIMITED_MODE  SEARCH_BD_NAME_LIMITED

#define SEARCH_NAME_DEBUG      0

#define HOWLING_ENABLE         0

struct remote_name {
    u16 crc;
    u8 addr[6];
    u8 name[32];
};

struct list_head inquiry_noname_list;

struct inquiry_noname_remote {
    struct list_head entry;
    u8 match;
    s8 rssi;
    u8 addr[6];
    u32 class;
};


extern int music_player_get_play_status(void);
extern int music_player_pp(void);
extern u16 get_emitter_curr_channel_state();
static u8 read_name_start = 0;
static u8 bt_search_busy = 0;
static u8 search_spp_device = 0;
static u8 emitter_dac_mute_en = 0;
static u8 bt_emitter_start = 0;

void bt_emitter_audio_set_mute(u8 mute)
{
    emitter_dac_mute_en = !!mute;
}

/*----------------------------------------------------------------------------*/
/**@brief    音频数据清0
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_data_set_zero(struct audio_stream_entry *entry,  struct audio_data_frame *data_buf)
{
    if (emitter_dac_mute_en) {
        memset(data_buf->data, 0, data_buf->data_len);
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射发起搜索设备
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void bt_search_device(void)
{
    if (!get_bt_init_status()) {
        log_info("bt on init >>>>>>>>>>>>>>>>>>>>>>>\n");
        return;
    }
    if (bt_search_busy) {
        log_info("bt_search_busy >>>>>>>>>>>>>>>>>>>>>>>\n");
        //return;
    }

    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);

    read_name_start = 0;
    bt_search_busy = 1;
    u8 inquiry_length = 20;   // inquiry_length * 1.28s
    user_send_cmd_prepare(USER_CTRL_SEARCH_DEVICE, 1, &inquiry_length);
    log_info("bt_search_start >>>>>>>>>>>>>>>>>>>>>>>\n");
}


/* ***************************************************************************/
/** @brief   ：蓝牙发射 提供搜索连接spp设备功能
    @param   ：无
    @return  ：无
    @note    ：
 */
/* ***************************************************************************/
void bt_search_spp_device()
{
    search_spp_device = 1;
    set_start_search_spp_device(1);
    bt_search_device();
}

/* ***************************************************************************/
/** @brief   ：蓝牙发射 搜索设备状态
    @param   ：无
    @return  ：无
    @note    ：
 */
/* ***************************************************************************/
u8 bt_search_status()
{
    return bt_search_busy;
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射 提供按键断开连接和恢复连接
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_connect_switch()
{
    if (get_emitter_curr_channel_state() != 0) {
        user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
        while (hci_standard_connect_check() != 0) {
            //wait disconnect;
            os_time_dly(10);
        }
    } else {
        /* log_info("start connect vm addr\n"); */
        user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY, 0, NULL);
    }
}

u8 bt_emitter_role_get()
{
    return bt_user_priv_var.emitter_or_receiver;
}
/* ***************************************************************************/
/** @brief   ：蓝牙发射 取消搜索
    @param   ：无
    @return  ：无
    @note    ：
 */
/* ***************************************************************************/
void bt_emitter_stop_search_device()
{
    user_send_cmd_prepare(USER_CTRL_INQUIRY_CANCEL, 0, NULL);
}

/* ***************************************************************************/
/** @brief   ：蓝牙发射 连接指定mac设备
    @param   ：mac
    @return  ：无
    @note    ：
 */
/* ***************************************************************************/
void emitter_bt_connect(u8 *mac)
{
    if (bt_user_priv_var.emitter_or_receiver != BT_EMITTER_EN) {
        return ;
    }

    while (hci_standard_connect_check() == 0x80) {
        //wait profile connect ok;
        if (get_emitter_curr_channel_state()) {
            break;
        }
        os_time_dly(10);
    }


    ////断开链接
    if (get_emitter_curr_channel_state() != 0) {
        user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
    } else {
        if (hci_standard_connect_check()) {
            user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_CONNECTION_CANCEL, 0, NULL);
        }
    }
    /* if there are some connected channel ,then disconnect*/
    while (hci_standard_connect_check() != 0) {
        //wait disconnect;
        os_time_dly(10);
    }

    user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, mac);
}

/* ***************************************************************************/
/** @brief   ：蓝牙发射 开始搜索设备
    @param   ：无
    @return  ：无
    @note    ：
 */
/* ***************************************************************************/
void bt_emitter_start_search_device()
{
    if (bt_user_priv_var.emitter_or_receiver != BT_EMITTER_EN) {
        return ;
    }
    while (hci_standard_connect_check() == 0x80) {
        //wait profile connect ok;
        if (get_emitter_curr_channel_state()) {
            break;
        }
        os_time_dly(10);
    }

    ////断开链接
    if (get_emitter_curr_channel_state() != 0) {
        user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
    } else {
        if (hci_standard_connect_check()) {
            user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_CONNECTION_CANCEL, 0, NULL);
        }
    }
    /* if there are some connected channel ,then disconnect*/
    while (hci_standard_connect_check() != 0) {
        //wait disconnect;
        os_time_dly(10);
    }

    ////关闭可发现可链接
    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
    ////切换样机状态
    bt_search_device();

}

////回链耳机音箱
u8 connect_last_sink_device_from_vm()
{
    bd_addr_t mac_addr;
    u8 flag = 0;
    flag = restore_remote_device_info_profile(&mac_addr, 1, get_remote_dev_info_index(), REMOTE_SINK);
    if (flag) {
        //connect last conn
        printf("last source device addr from vm:");
        put_buf(mac_addr, 6);
        user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, mac_addr);
    }
    //r_printf("connect last vm addr\n");
    return flag;
}

////回链手机
u8 connect_last_source_device_from_vm()
{
    bd_addr_t mac_addr;
    u8 flag = 0;
    flag = restore_remote_device_info_profile(&mac_addr, 1, get_remote_dev_info_index(), REMOTE_SOURCE);
    if (flag) {
        //connect last conn
        printf("last source device addr from vm:");
        put_buf(mac_addr, 6);
        user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, mac_addr);
    }
    //r_printf("connect last vm addr\n");
    return flag;
}
////自动调用,同步最大音量
void the_first_time_set_vol_change()
{
    user_emitter_cmd_prepare(USER_CTRL_AVCTP_OPID_SEND_VOL, 0, NULL);//同步音量
}
/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射 提供按键切换发射器或者是音箱功能
   @param    1:发射    0：接收
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_or_receiver_switch(u8 flag)
{
    printf("===emitter_or_receiver_switch %d %x\n", flag, hci_standard_connect_check());
    /*如果上一次操作记录跟传进来的参数一致，则不操作*/
    if (bt_user_priv_var.emitter_or_receiver == flag) {
        return ;
    }
    while (hci_standard_connect_check() == 0x80) {
        //wait profile connect ok;
        if (bt_user_priv_var.emitter_or_receiver == BT_EMITTER_EN) {   ///蓝牙发射器
            r_printf("cur_ch:0x%x", get_emitter_curr_channel_state());
            if (get_emitter_curr_channel_state()) {
                break;
            }
        } else {
            r_printf("cur_ch:0x%x", get_curr_channel_state());
            if (get_emitter_curr_channel_state()) {
                break;
            }
        }
        os_time_dly(10);
    }

    ////断开链接
    if ((get_curr_channel_state() != 0) || (get_emitter_curr_channel_state() != 0)) {
        user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
    } else {
        if (hci_standard_connect_check()) {
            user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_CONNECTION_CANCEL, 0, NULL);
        }
    }
    /* if there are some connected channel ,then disconnect*/
    while (hci_standard_connect_check() != 0) {
        //wait disconnect;
        os_time_dly(10);
    }

    g_printf("===wait to switch to mode %d\n", flag);
    bt_user_priv_var.emitter_or_receiver = flag;
    if (flag == BT_EMITTER_EN) {   ///蓝牙发射器
        /* bredr_bulk_change(0); */
        ////关闭可发现可链接
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
        ////切换样机状态
        __set_emitter_enable_flag(1);
        a2dp_source_init(NULL, 0, 1);
#if (USER_SUPPORT_PROFILE_HFP_AG==1)
        hfp_ag_buf_init(NULL, 0, 1);
#endif
        ////开启搜索设备
        if (connect_last_sink_device_from_vm()) {
            log_info("start connect device vm addr\n");
        } else {
            bt_search_device();
        }
    } else if (flag == BT_RECEIVER_EN) {  ///蓝牙接收
        /* bredr_bulk_change(1); */
        ////切换样机状态
        __set_emitter_enable_flag(0);
        emitter_media_source(NULL, 1, 0);
        hci_cancel_inquiry();
        a2dp_source_init(NULL, 0, 0);
#if (USER_SUPPORT_PROFILE_HFP_AG==1)
        hfp_ag_buf_init(NULL, 0, 0);
#endif
        ////开启可发现可链接
        if (connect_last_source_device_from_vm()) {
            log_info("start connect vm addr phone \n");
        } else {
            user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
        }
    }
}

/* ***************************************************************************/
/** @brief   ：蓝牙发射 自动选择发射器与接收器模式
    @param   ：无
    @return  ：无
    @note    ：
 */
/* ***************************************************************************/
void bt_emitter_receiver_sw()
{
    if (bt_user_priv_var.emitter_or_receiver != BT_EMITTER_EN) {
        emitter_or_receiver_switch(BT_EMITTER_EN);
    } else {
        emitter_or_receiver_switch(BT_RECEIVER_EN);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射变量初始化
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void bt_emitter_init()
{
    bt_emitter_start = 1;
    INIT_LIST_HEAD(&inquiry_noname_list);

    lmp_set_sniff_establish_by_remote(1);
    /* audio_sbc_enc_init(); */

#ifdef MEDIA_CODEC_TYPE_SWITCH
    __set_support_non_a2dp_flag(1);
#endif
}


/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射搜索设备没有名字的设备，放进需要获取名字链表
   @param    status : 获取成功     0：获取失败
  			 addr:设备地址
		     name：设备名字
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_search_noname(u8 status, u8 *addr, u8 *name)
{
    struct  inquiry_noname_remote *remote, *n;
    if (!bt_emitter_start) {
        return ;
    }

    u8 res = 0;
    local_irq_disable();
    if (status) {
        list_for_each_entry_safe(remote, n, &inquiry_noname_list, entry) {
            if (!memcmp(addr, remote->addr, 6)) {
                list_del(&remote->entry);
                free(remote);
            }
        }
        goto __find_next;
    }
    list_for_each_entry_safe(remote, n, &inquiry_noname_list, entry) {
        if (!memcmp(addr, remote->addr, 6)) {
            res = emitter_search_result(name, strlen(name), addr, remote->class, remote->rssi);
            if (res) {
                read_name_start = 0;
                remote->match = 1;
                user_send_cmd_prepare(USER_CTRL_INQUIRY_CANCEL, 0, NULL);
                local_irq_enable();
                return;
            }
            list_del(&remote->entry);
            free(remote);
        }
    }

__find_next:

    read_name_start = 0;
    remote = NULL;
    if (!list_empty(&inquiry_noname_list)) {
        remote =  list_first_entry(&inquiry_noname_list, struct inquiry_noname_remote, entry);
    }

    local_irq_enable();
    if (remote) {
        read_name_start = 1;
        user_send_cmd_prepare(USER_CTRL_READ_REMOTE_NAME, 6, remote->addr);
    }
}

void remote_name_speciali_deal(u8 status, u8 *addr, u8 *name)
{
    emitter_search_noname(status, addr, name);
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射停止搜索
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/

void emitter_search_stop(u8 result)
{
    struct  inquiry_noname_remote *remote, *n;
    bt_search_busy = 0;
    search_spp_device = 0;
    set_start_search_spp_device(0);
    u8 wait_connect_flag = 1;
    if (!list_empty(&inquiry_noname_list)) {
        user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
    }

    if (!result) {
        list_for_each_entry_safe(remote, n, &inquiry_noname_list, entry) {
            if (remote->match) {
                user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR, 6, remote->addr);
                wait_connect_flag = 0;
            }
            list_del(&remote->entry);
            free(remote);
        }
    }
    read_name_start = 0;
    if (wait_connect_flag) {
        /* log_info("wait conenct\n"); */
#if TCFG_SPI_LCD_ENABLE
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
#else
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
        if (!result) {
            user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
        }
#endif

    }
}


#if (SEARCH_LIMITED_MODE == SEARCH_BD_ADDR_LIMITED)
u8 bd_addr_filt[][6] = {
    {0x8E, 0xA7, 0xCA, 0x0A, 0x5E, 0xC8}, /*S10_H*/
    {0xA7, 0xDD, 0x05, 0xDD, 0x1F, 0x00}, /*ST-001*/
    {0xE9, 0x73, 0x13, 0xC0, 0x1F, 0x00}, /*HBS 730*/
    {0x38, 0x7C, 0x78, 0x1C, 0xFC, 0x02}, /*Bluetooth*/
};
/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射搜索通过地址过滤
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
u8 search_bd_addr_filt(u8 *addr)
{
    u8 i;
    log_info("bd_addr:");
    log_info_hexdump(addr, 6);
    for (i = 0; i < (sizeof(bd_addr_filt) / sizeof(bd_addr_filt[0])); i++) {
        if (memcmp(addr, bd_addr_filt[i], 6) == 0) {
            /* printf("bd_addr match:%d\n", i); */
            return TRUE;
        }
    }
    /*log_info("bd_addr not match\n"); */
    return FALSE;
}
#endif


#if (SEARCH_LIMITED_MODE == SEARCH_BD_NAME_LIMITED)
#if (0)
u8 bd_name_filt[][32] = {
    "BeMine",
    "EDIFIER CSR8635",/*CSR*/
    "JL-BT-SDK",/*Realtek*/
    "I7-TWS",/*ZKLX*/
    "TWS-i7",/*ZKLX*/
    "I9",/*ZKLX*/
    "小米小钢炮蓝牙音箱",/*XiaoMi*/
    "小米蓝牙音箱",/*XiaoMi*/
    "XMFHZ02",/*XiaoMi*/
    "JBL GO 2",
    "i7mini",/*JL tws AC690x*/
    "S08U",
    "AI8006B_TWS00",
    "S046",/*BK*/
    "AirPods",
    "CSD-TWS-01",
    "AC692X_wh",
    "JBL GO 2",
    "JBL Flip 4",
    "BT Speaker",
    "CSC608",
    "QCY-QY19",
    "Newmine",
    "HT1+",
    "S-35",
    "T12-JL",
    "Redmi AirDots_R",
    "Redmi AirDots_L",
    "AC69_Bluetooth",
    "FlyPods 3",
    "MNS",
    "Jam Heavy Metal",
    "Bluedio",
    "HR-686",
    "BT MUSIC",
    "BW-USB-DONGLE",
    "S530",
    "XPDQ7",
    "MICGEEK Q9S",
    "S10_H",
    "S10",/*JL AC690x*/
    "S11",/*JL AC460x*/
    "HBS-730",
    "SPORT-S9",
    "Q5",
    "IAEB25",
    "T5-JL",
    "MS-808",
    "LG HBS-730",
    "NG-BT07"
};
#else
u8 bd_name_filt[20][30] = {
    "TG-294",
    "Shanling UP4",
    "Jabra Elite 65t",
    "JBL GO",
    "G02",
};
#endif

u8 bd_spp_name_filt[20][30] = {
    "AC69_BT_SDK",
};

extern const char *bt_get_emitter_connect_name();
/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射搜索通过名字过滤
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
u8 search_bd_name_filt(char *data, u8 len, u32 dev_class, char rssi)
{
    char bd_name[64] = {0};
    u8 i;
    char *targe_name = NULL;
    char char_a = 0, char_b = 0;

    if ((len > (sizeof(bd_name))) || (len == 0)) {
        //printf("bd_name_len error:%d\n", len);
        return FALSE;
    }

    memset(bd_name, 0, sizeof(bd_name));
    memcpy(bd_name, data, len);
    log_info("33name:%s,len:%d,class %x ,rssi %d\n", bd_name, len, dev_class, rssi);
#if SEARCH_NAME_DEBUG
    extern char *get_edr_name(void);
    printf("tar name:%s,len:%d\n", get_edr_name(), strlen(get_edr_name()));
    targe_name = (char *)get_edr_name();
#if 1
//不区分大小写
    for (i = 0; i < len; i++) {
        char_a = bd_name[i];
        char_b = targe_name[i];
        if ('A' <= char_a && char_a <= 'Z') {
            char_a += 32;    //转换成小写
        }
        if ('A' <= char_b && char_b <= 'Z') {
            char_b += 32;    //转换成小写
        }
        printf("{%d-%d}", char_a, char_b);
        if (char_a != char_b) {
            return FALSE;
        }
    }
    log_info("\n*****find dev ok******\n");
    return TRUE;
#else
//区分大小写
    if (memcmp(data, bt_get_emitter_connect_name(), len) == 0) {
        log_info("\n*****find dev ok******\n");
        return TRUE;
    }
    return FALSE;
#endif

#else
    if (search_spp_device) {
        for (i = 0; i < (sizeof(bd_spp_name_filt) / sizeof(bd_spp_name_filt[0])); i++) {
            if (memcmp(data, bd_spp_name_filt[i], len) == 0) {
                puts("\n*****find dev ok******\n");
                return TRUE;
            }
        }
    } else {
        for (i = 0; i < (sizeof(bd_name_filt) / sizeof(bd_name_filt[0])); i++) {
            if (memcmp(data, bd_name_filt[i], len) == 0) {
                puts("\n*****find dev ok******\n");
                return TRUE;
            }
        }
    }
    return FALSE;
#endif

}


/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射搜索结果回调处理
   @param    name : 设备名字
			 name_len: 设备名字长度
			 addr:   设备地址
			 dev_class: 设备类型
			 rssi:   设备信号强度
   @return   无
   @note
 			蓝牙设备搜索结果，可以做名字/地址过滤，也可以保存搜到的所有设备
 			在选择一个进行连接，获取其他你想要的操作。
 			返回TRUE，表示搜到指定的想要的设备，搜索结束，直接连接当前设备
 			返回FALSE，则继续搜索，直到搜索完成或者超时
*/
/*----------------------------------------------------------------------------*/
u8 emitter_search_result(char *name, u8 name_len, u8 *addr, u32 dev_class, char rssi)
{
#if (SEARCH_LIMITED_MODE == SEARCH_BD_NAME_LIMITED)
    if (name == NULL) {
        struct inquiry_noname_remote *remote = malloc(sizeof(struct inquiry_noname_remote));
        remote->match  = 0;
        remote->class = dev_class;
        remote->rssi = rssi;
        memcpy(remote->addr, addr, 6);
        local_irq_disable();
        list_add_tail(&remote->entry, &inquiry_noname_list);
        local_irq_enable();
        if (read_name_start == 0) {
            read_name_start = 1;
            user_send_cmd_prepare(USER_CTRL_READ_REMOTE_NAME, 6, addr);
        }
    }
#endif

#if (SEARCH_LIMITED_MODE == SEARCH_BD_NAME_LIMITED)
    return search_bd_name_filt(name, name_len, dev_class, rssi);
#endif

#if (SEARCH_LIMITED_MODE == SEARCH_BD_ADDR_LIMITED)
    return search_bd_addr_filt(addr);
#endif

#if (SEARCH_LIMITED_MODE == SEARCH_CUSTOM_LIMITED)
    /*以下为搜索结果自定义处理*/
    char bt_name[63] = {0};
    u8 len;
    if (name_len == 0) {
        log_info("No_eir\n");
    } else {
        len = (name_len > 63) ? 63 : name_len;
        /* display bd_name */
        memcpy(bt_name, name, len);
        log_info("22name:%s,len:%d,class %x ,rssi %d\n", bt_name, name_len, dev_class, rssi);
    }

    /* display bd_addr */
    log_debug_hexdump(addr, 6);

    /* You can connect the specified bd_addr by below api      */
    //user_send_cmd_prepare(USER_CTRL_START_CONNEC_VIA_ADDR,6,addr);

    return FALSE;
#endif

#if (SEARCH_LIMITED_MODE == SEARCH_NULL_LIMITED)
    /*没有指定限制，则搜到什么就连接什么*/
    return TRUE;
#endif
}

typedef enum {
    AVCTP_OPID_VOLUME_UP   = 0x41,
    AVCTP_OPID_VOLUME_DOWN = 0x42,
    AVCTP_OPID_MUTE        = 0x43,
    AVCTP_OPID_PLAY        = 0x44,
    AVCTP_OPID_STOP        = 0x45,
    AVCTP_OPID_PAUSE       = 0x46,
    AVCTP_OPID_NEXT        = 0x4B,
    AVCTP_OPID_PREV        = 0x4C,
} AVCTP_CMD_TYPE;

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射接收到设备按键消息
   @param    cmd:按键命令
   @return   无
   @note
 			发射器收到接收器发过来的控制命令处理
 			根据实际需求可以在收到控制命令之后做相应的处理
 			蓝牙库里面定义的是weak函数，直接再定义一个同名可获取信息
*/
/*----------------------------------------------------------------------------*/
void emitter_rx_avctp_opid_deal(u8 cmd, u8 id)
{
    /* log_debug("avctp_rx_cmd:%x\n", cmd); */
    switch (cmd) {
    case AVCTP_OPID_NEXT:
        log_info("AVCTP_OPID_NEXT\n");
        app_task_put_key_msg(KEY_MUSIC_NEXT, 0);
        break;
    case AVCTP_OPID_PREV:
        log_info("AVCTP_OPID_PREV\n");
        app_task_put_key_msg(KEY_MUSIC_PREV, 0);
        break;
    case AVCTP_OPID_PAUSE:
    case AVCTP_OPID_STOP:
        log_info("AVCTP_OPID_PAUSE\n");
        /* bt_emitter_pp(0); */
        app_task_put_key_msg(KEY_MUSIC_PP, 0);
        break;
    case AVCTP_OPID_PLAY:
        /* bt_emitter_pp(1); */
        app_task_put_key_msg(KEY_MUSIC_PP, 0);
        log_info("AVCTP_OPID_PP\n");
        break;
    case AVCTP_OPID_VOLUME_UP:
        log_info("AVCTP_OPID_VOLUME_UP\n");
        app_task_put_key_msg(KEY_VOL_UP, 0);
        break;
    case AVCTP_OPID_VOLUME_DOWN:
        log_info("AVCTP_OPID_VOLUME_DOWN\n");
        app_task_put_key_msg(KEY_VOL_DOWN, 0);
        break;
    default:
        break;
    }
    return ;
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射接收设备同步音量
   @param    vol:接收到设备同步音量
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_rx_vol_change(u8 vol)
{
    /* log_info("vol_change:%d \n", vol); */
}

typedef struct _EMITTER_INFO {
    volatile u8 role;
    u8 media_source;
    u8 source_record;/*统计当前有多少设备可用*/
    u8 reserve;
} EMITTER_INFO_T;

EMITTER_INFO_T emitter_info = {
    .role = 0/*EMITTER_ROLE_SLAVE*/,
};

void emitter_open(u8 source)
{
    emitter_info.source_record |= source;
    if (emitter_info.media_source == source) {
        return;
    }
    emitter_info.media_source = source;
    __emitter_send_media_toggle(NULL, 1);
}

void emitter_close(u8 source)
{
    if (music_player_get_play_status() == FILE_DEC_STATUS_PLAY) {
        music_player_pp();
    }
    emitter_info.source_record &= ~source;
    if (emitter_info.media_source == source) {
        emitter_info.media_source = 0/*EMITTER_SOURCE_NULL*/;
        __emitter_send_media_toggle(NULL, 0);
        //emitter_media_source_next();
    }
    while (bt_emitter_stu_get()) {
        os_time_dly(2);
        putchar('K');
    }
}
/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射接收设备同步音量
   @param    source: 高级音频角色  1：source  0：sink
  			 en:关闭source
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_media_source(u8 *addr, u8 source, u8 en)
{
    if (en) {
        /*关闭当前的source通道*/
        //emitter_media_source_close(emitter_info.media_source);
        if (music_player_get_play_status() != FILE_DEC_STATUS_PLAY) {
            music_player_pp();
        }

        emitter_info.source_record |= source;
        if (emitter_info.media_source == source) {
            return;
        }
        emitter_info.media_source = source;
        __emitter_send_media_toggle(NULL, 1);
    } else {
        if (music_player_get_play_status() == FILE_DEC_STATUS_PLAY) {
            music_player_pp();
        }
        emitter_info.source_record &= ~source;
        if (emitter_info.media_source == source) {
            emitter_info.media_source = 0/*EMITTER_SOURCE_NULL*/;
            __emitter_send_media_toggle(NULL, 0);
            //emitter_media_source_next();
        }

    }
    log_info("current source: %x-%x\n", source, emitter_info.source_record);
}
#endif

/* ***************************************************************************/
/** @brief   ：蓝牙发射 获取音频发射状态
    @param   ：无
    @return  ：1:开启状态；2：关闭状态
    @note    ：
 */
/* ***************************************************************************/
u8 bt_emitter_stu_get(void)
{
    return 0;//audio_sbc_enc_is_work();//bt_emitter_on;
}
u8 bt_emitter_stu_set(u8 *addr, u8 on)
{
    if (bt_user_priv_var.emitter_or_receiver != BT_EMITTER_EN) {
        return 0;
    }
    if (!(get_emitter_curr_channel_state() & A2DP_SRC_CH)) {
        emitter_media_source(NULL, 1, 0);
        return 0;
    }
    log_debug("total con dev:%d ", get_total_connect_dev());
    if (on && (get_total_connect_dev() == 0)) {
        on = 0;
    }
    /* bt_emitter_on = on; */
    emitter_media_source(NULL, 1, on);
    return on;
}
/* ***************************************************************************/
/** @brief   ：蓝牙发射 高级音频发射开关
    @param   ：pp： 1：开启，0：关闭
    @return  ：无
    @note    ：
 */
/* ***************************************************************************/
u8 bt_emitter_pp(u8 pp)
{
    if (bt_user_priv_var.emitter_or_receiver != BT_EMITTER_EN) {
        return 0;
    }
    if (get_total_connect_dev() == 0) {
        //如果没有连接就启动一下搜索
        /* bt_search_device(); */
        return 0;
    }
    if (!(get_emitter_curr_channel_state() & A2DP_SRC_CH)) {
        return 0;
    }
    return bt_emitter_stu_set(NULL, pp);
}


u8 bt_emitter_stu_sw(void)
{
    return bt_emitter_pp(!bt_emitter_stu_get());
}


////////////////////////////////////////////////////////////////////////

//pin code 轮询功能
const char pin_code_list[10][4] = {
    {'0', '0', '0', '0'},
    {'1', '2', '3', '4'},
    {'8', '8', '8', '8'},
    {'1', '3', '1', '4'},
    {'4', '3', '2', '1'},
    {'1', '1', '1', '1'},
    {'2', '2', '2', '2'},
    {'3', '3', '3', '3'},
    {'5', '6', '7', '8'},
    {'5', '5', '5', '5'}
};

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射链接pincode 轮询
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
const char *bt_get_emitter_pin_code(u8 flag)
{
    static u8 index_flag = 0;
    int pincode_num = sizeof(pin_code_list) / sizeof(pin_code_list[0]);
    if (flag == 1) {
        //reset index
        index_flag = 0;
    } else if (flag == 2) {
        //查询是否要开始继续回连尝试pin code。
        if (index_flag >= pincode_num) {
            //之前已经遍历完了
            return NULL;
        } else {
            index_flag++; //准备使用下一个
        }
    } else {
        /* log_debug("get pin code index %d\n", index_flag); */
    }
    return &pin_code_list[index_flag][0];
}


/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射链接保存远端设备名字
   @param    addr:远端设备地址
			 name:远端设备名字
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_save_remote_name(u8 *addr, u8 *name)
{
    struct remote_name remote_n;
    u16 id = CFG_REMOTE_DN_00;

    while (1) {
        syscfg_read(id, &remote_n, sizeof(struct remote_name));
        if (remote_n.crc == CRC16((u8 *)&remote_n.addr, sizeof(struct remote_name) - 2)) {
            if (!memcmp(addr, remote_n.addr, 6)) {
                return;
            }
        } else {
            break;
        }
        id++;
        if (id >= 20) {
            break;
        }
    }
    memset(&remote_n, 0, sizeof(struct remote_name));
    memcpy(remote_n.addr, addr, 6);
    memcpy(remote_n.name, name, strlen(name));

    remote_n.crc = CRC16((u8 *)&remote_n.addr, sizeof(struct remote_name) - 2);

    syscfg_write(id, &remote_n, sizeof(struct remote_name));
}


/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射通过地址来获取设备名字
   @param    addr:远端设备地址
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
u8 *emitter_get_remote_name(u8 *addr)
{
    struct remote_name remote_n;
    u16 id = CFG_REMOTE_DN_00;
    while (1) {
        syscfg_read(id, &remote_n, sizeof(struct remote_name));
        if (remote_n.crc == CRC16((u8 *)&remote_n.addr, sizeof(struct remote_name) - 2)) {
            if (!memcmp(addr, remote_n.addr, 6)) {
                return &remote_n.name;
            }
        } else {
            break;
        }
        id++;
        if (id >= 20) {
            break;
        }
    }
    return NULL;
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射通过地址来删除vm的设备名字
   @param    addr:远端设备地址
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_delete_remote_name(u8 *addr)
{
    struct remote_name remote_n;
    u16 id = CFG_REMOTE_DN_00;
    while (1) {
        syscfg_read(id, &remote_n, sizeof(struct remote_name));
        if (remote_n.crc == CRC16((u8 *)&remote_n.addr, sizeof(struct remote_name) - 2)) {
            if (!memcmp(addr, remote_n.addr, 6)) {
                memset(&remote_n, 0xff, sizeof(struct remote_name));
                syscfg_write(id, &remote_n, sizeof(struct remote_name));
                return;
            }
        }
        id++;
    }
}


#define BD_ADDR_LEN 6
typedef uint8_t bd_addr_t[BD_ADDR_LEN];
static bd_addr_t remote_addr[20];
extern u8 restore_remote_device_info_opt(bd_addr_t *mac_addr, u8 conn_device_num, u8 id);

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射获取vm设备记忆
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_get_vm_device()
{
    memset(remote_addr, 0, sizeof(remote_addr));
    u8 flag = restore_remote_device_info_opt(&remote_addr, 20, get_remote_dev_info_index());
    for (u8 i = 0; i < 20; i++) {
        log_debug("----- \n");
        log_debug_hexdump(remote_addr[i], 6);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙  开关ag_a2dp
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void bredr_ag_a2dp_open_and_close()
{
    if (get_emitter_curr_channel_state() & A2DP_SRC_CH) {
        puts("start to disconnect a2dp ");
        user_emitter_cmd_prepare(USER_CTRL_DISCONN_A2DP, 0, NULL);
    } else {
        puts("start to connect a2dp ");
        user_emitter_cmd_prepare(USER_CTRL_CONN_A2DP, 0, NULL);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  蓝牙  开关ag_hfp
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void bredr_ag_hfp_open_and_close()
{
    if (get_emitter_curr_channel_state() & HFP_AG_CH) {
        puts("start to disconnect HFP ");
        user_emitter_cmd_prepare(USER_CTRL_HFP_DISCONNECT, 0, NULL);
    } else {
        puts("start to connect HFP ");
        user_emitter_cmd_prepare(USER_CTRL_HFP_CMD_BEGIN, 0, NULL);
    }
}
#else

void emitter_media_source(u8 *addr, u8 source, u8 en)
{

}
void emitter_or_receiver_switch(u8 flag)
{
}

u8 bt_search_status()
{
    return 0;
}


#endif

