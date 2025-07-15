#include "adapter_odev.h"
#include "adapter_odev_bt.h"
#include "adapter_odev_edr.h"
//#include "wireless_mic/bt.h"
#include "btstack/avctp_user.h"
#include "classic/hci_lmp.h"
#include "list.h"
#include "os/os_api.h"
#include "circular_buf.h"
#include "app_config.h"
#include "event.h"
#include "bt_edr_fun.h"
#include "usb/device/hid.h"
#if TCFG_WIRELESS_MIC_ENABLE

#define  LOG_TAG_CONST       WIRELESSMIC
#define  LOG_TAG             "[ODEV_EDR]"
#define  LOG_ERROR_ENABLE
#define  LOG_DEBUG_ENABLE
#define  LOG_INFO_ENABLE
#define  LOG_DUMP_ENABLE
#define  LOG_CLI_ENABLE
#include "debug.h"


struct _odev_edr odev_edr;
#define __this  (&odev_edr)

struct inquiry_noname_remote {
    struct list_head entry;
    u8 match;
    s8 rssi;
    u8 addr[6];
    u32 class;
};

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射发起搜索设备
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void adapter_odev_edr_search_device(void)
{
    if (!get_bt_init_status()) {
        return;
    }

    printf("__this->edr_search_busy = %d", __this->edr_search_busy);
    if (__this->edr_search_busy) {
        return;
    }


    ////断开链接
    if (get_curr_channel_state() != 0) {
        user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
    } else {
        if (hci_standard_connect_check()) {
            user_send_cmd_prepare(USER_CTRL_PAGE_CANCEL, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_CONNECTION_CANCEL, 0, NULL);
        }
    }

    /* if there are some connected channel ,then disconnect*/

    ////关闭可发现可链接
    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);

    __this->edr_read_name_start = 0;
    __this->edr_search_busy = 1;

    u8 inquiry_length = __this->parm->inquiry_len;   // inquiry_length * 1.28s
    user_send_cmd_prepare(USER_CTRL_SEARCH_DEVICE, 1, &inquiry_length);

    printf("[adapter] %s, %d\n", __FUNCTION__, __LINE__);
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射发起搜索spp设备
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void adapter_odev_edr_search_spp_device()
{
    __this->edr_search_spp = 1;
    set_start_search_spp_device(1);
    adapter_odev_edr_search_device();
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射停止搜索
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void adapter_odev_edr_search_stop()
{
    log_info("adapter_odev_edr_search_stop\n");

    struct  inquiry_noname_remote *remote, *n;

    __this->edr_search_spp = 0;
    __this->edr_search_busy = 0;

    set_start_search_spp_device(0);

    list_for_each_entry_safe(remote, n, &__this->inquiry_noname_head, entry) {
        list_del(&remote->entry);
        free(remote);
    }

    __this->edr_read_name_start = 0;

}


//搜索完成回调
void bt_hci_event_inquiry(struct bt_event *bt)
{
    r_printf("bt_hci_event_inquiry result : %d\n", bt->value);

    if (!__this->parm->discon_always_search) {
        return;
    }
    adapter_odev_edr_search_stop();

    if (!bt->value) {
        adapter_odev_edr_search_device();
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射搜索通过名字过滤
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
#if (SEARCH_LIMITED_MODE == SEARCH_BD_NAME_LIMITED)

static u8 adapter_odev_edr_search_bd_name_filt(char *data, u8 len, u32 dev_class, char rssi)
{
    char bd_name[64] = {0};
    u8 i;

    if ((len > (sizeof(bd_name))) || (len == 0)) {
        r_printf("bd_name_len error:%d\n", len);
        return FALSE;
    }

    memset(bd_name, 0, sizeof(bd_name));
    memcpy(bd_name, data, len);

    g_printf("edr name:%s,len:%d,class %x ,rssi %d\n", bd_name, len, dev_class, rssi);

    if (__this->edr_search_spp) {
        for (i = 0; i < __this->parm->spp_name_filt_num; i++) {
            if (memcmp(data, __this->parm->spp_name_filt[i], len) == 0) {
                y_printf("*****find spp dev ok******\n");
                return TRUE;
            }
        }
    } else {
        for (i = 0; i < __this->parm->bt_name_filt_num; i++) {
            /* r_printf("%d : %s\n",i,__this->parm->bt_name_filt[i]); */
            if (memcmp(data, __this->parm->bt_name_filt[i], len) == 0) {
                y_printf("*****find dev ok******\n");
                return TRUE;
            }
        }
    }

    return FALSE;
}

#endif //(SEARCH_LIMITED_MODE == SEARCH_BD_NAME_LIMITED)


/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射搜索通过地址过滤
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
#if (SEARCH_LIMITED_MODE == SEARCH_BD_ADDR_LIMITED)

static u8 adapter_odev_edr_search_bd_addr_filt(u8 *addr)
{
    u8 i;
    log_info("search_bd_addr_filt:");
    log_info_hexdump(addr, 6);

    for (i = 0; i < __this->parm->bd_addr_filt_num; i++) {
        if (memcmp(addr, __this->parm->bd_addr_filt[i], 6) == 0) {
            y_printf("bd_addr match:%d\n", i);
            return TRUE;
        }
    }

    y_printf("bd_addr not match\n");

    return FALSE;
}

#endif //(SEARCH_LIMITED_MODE == SEARCH_BD_ADDR_LIMITED)

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
static u8 adapter_odev_edr_search_result(char *name, u8 name_len, u8 *addr, u32 dev_class, char rssi)
{
    if (__this->parm->disable_filt) {
        return TRUE;
    }

    if (name == NULL) {
        struct inquiry_noname_remote *remote = malloc(sizeof(struct inquiry_noname_remote));
        remote->match  = 0;
        remote->class = dev_class;
        remote->rssi = rssi;
        memcpy(remote->addr, addr, 6);

        local_irq_disable();
        list_add_tail(&remote->entry, &__this->inquiry_noname_head);
        local_irq_enable();

        if (__this->edr_read_name_start == 0) {
            __this->edr_read_name_start = 1;
            user_send_cmd_prepare(USER_CTRL_READ_REMOTE_NAME, 6, addr);
        }
    }

#if (SEARCH_LIMITED_MODE == SEARCH_BD_NAME_LIMITED)
    return adapter_odev_edr_search_bd_name_filt(name, name_len, dev_class, rssi);
#endif

#if (SEARCH_LIMITED_MODE == SEARCH_BD_ADDR_LIMITED)
    return adapter_odev_edr_search_bd_addr_filt(addr);
#endif

#if (SEARCH_LIMITED_MODE == SEARCH_CUSTOM_LIMITED)
    /*以下为搜索结果自定义处理*/
    log_info("name:%s,len:%d,class %x ,rssi %d\n", bt_name, name_len, dev_class, rssi);
    return FALSE;
#endif

#if (SEARCH_LIMITED_MODE == SEARCH_NULL_LIMITED)
    /*没有指定限制，则搜到什么就连接什么*/
    return TRUE;
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
static void adapter_odev_edr_search_noname(u8 status, u8 *addr, u8 *name)
{
    u8 res = 0;
    struct  inquiry_noname_remote *remote, *n;

    if (__this->status != ODEV_EDR_START) {
        return ;
    }

    local_irq_disable();

    if (status) {
        list_for_each_entry_safe(remote, n, &__this->inquiry_noname_head, entry) {
            if (!memcmp(addr, remote->addr, 6)) {
                list_del(&remote->entry);
                free(remote);
            }
        }
        goto __find_next;
    }

    list_for_each_entry_safe(remote, n, &__this->inquiry_noname_head, entry) {
        if (!memcmp(addr, remote->addr, 6)) {
            res = adapter_odev_edr_search_result(name, strlen(name), addr, remote->class, remote->rssi);
            if (res) {
                __this->edr_read_name_start = 0;
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

    __this->edr_read_name_start = 0;
    remote = NULL;

    if (!list_empty(&__this->inquiry_noname_head)) {
        remote =  list_first_entry(&__this->inquiry_noname_head, struct inquiry_noname_remote, entry);
    }

    local_irq_enable();

    if (remote) {
        __this->edr_read_name_start = 1;
        user_send_cmd_prepare(USER_CTRL_READ_REMOTE_NAME, 6, remote->addr);
    }
}

void remote_name_speciali_deal(u8 status, u8 *addr, u8 *name)
{
    adapter_odev_edr_search_noname(status, addr, name);
}

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
void emitter_rx_avctp_opid_deal(u8 cmd, u8 id)     //属于库的弱函数重写
{
    log_debug("avctp_rx_cmd:%x\n", cmd);
#if (USB_DEVICE_CLASS_CONFIG & HID_CLASS)
    switch (cmd) {
    case AVCTP_OPID_NEXT:
        log_info("AVCTP_OPID_NEXT\n");
        adapter_avctp_key_handler(USB_AUDIO_NEXTFILE);
        break;
    case AVCTP_OPID_PREV:
        log_info("AVCTP_OPID_PREV\n");
        adapter_avctp_key_handler(USB_AUDIO_PREFILE);
        break;
    case AVCTP_OPID_PAUSE:
    case AVCTP_OPID_PLAY:
        log_info("AVCTP_OPID_PP\n");
        adapter_avctp_key_handler(USB_AUDIO_PP);
        break;
    case AVCTP_OPID_VOLUME_UP:
        log_info("AVCTP_OPID_VOLUME_UP\n");
        adapter_avctp_key_handler(USB_AUDIO_VOLUP);
        break;
    case AVCTP_OPID_VOLUME_DOWN:
        log_info("AVCTP_OPID_VOLUME_DOWN\n");
        adapter_avctp_key_handler(USB_AUDIO_VOLDOWN);
        break;
    default:
        break;
    }
#endif//(USB_DEVICE_CLASS_CONFIG & HID_CLASS)
    return ;
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙发射接收设备同步音量
   @param    vol:接收到设备同步音量
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void emitter_rx_vol_change(u8 vol)           //属于库的弱函数重写
{
    log_info("vol_change:%d \n", vol);
}

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
const char *bt_get_emitter_pin_code(u8 flag)        //属于库的弱函数重写
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
        log_debug("get pin code index %d\n", index_flag);
    }
    return &pin_code_list[index_flag][0];
}

/*----------------------------------------------------------------------------*/
/**@brief    get source status
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
u8 bt_emitter_stu_get(void)                     //属于库的弱函数重写
{
    //extern int adapter_audio_sbc_enc_is_work(void);

    //return adapter_audio_sbc_enc_is_work();
    return 1;
}

/*----------------------------------------------------------------------------*/
/**@brief    set source status
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
static u8 adapter_odev_edr_stu_set(u8 on)
{
    if (!(get_emitter_curr_channel_state() & A2DP_SRC_CH)) {
        __emitter_send_media_toggle(NULL, 0);
        return 0;
    }
    log_debug("total con dev:%d ", get_total_connect_dev());
    if (on && (get_total_connect_dev() == 0)) {
        on = 0;
    }

    r_printf("adapter_odev_edr_stu_set:%d\n", on);
    __emitter_send_media_toggle(NULL, on);

    return on;
}

static void esco_call_timeout(void *priv)
{
    g_printf("esco_call_timeout\n");
    __this->call_timeout = 0;
    user_emitter_cmd_prepare(USER_CTRL_HFP_CALL_ANSWER, 0, NULL);
}

#define ESCO_CALL_TIMEOUT_MSEC      (50)
static int esco_pp(u8 pp)
{
    if (pp) {
        user_emitter_cmd_prepare(USER_CTRL_HFP_CALL_LAST_NO, 0, NULL);

        if (__this->call_timeout == 0) {
            __this->call_timeout = sys_timeout_add(NULL, esco_call_timeout, ESCO_CALL_TIMEOUT_MSEC);
        } else {
            sys_timer_modify(__this->call_timeout, ESCO_CALL_TIMEOUT_MSEC);
        }
    } else {
        user_emitter_cmd_prepare(USER_CTRL_HFP_CALL_HANGUP, 0, NULL);
    }
    return pp;
}

int a2dp_pp(u8 pp)
{
    if (get_total_connect_dev() == 0) {
        return 0;
    }

    if (!(get_emitter_curr_channel_state() & A2DP_SRC_CH)) {
        return 0;
    }

    return adapter_odev_edr_stu_set(pp);
}

int adapter_odev_edr_pp(u8 pp)
{
    int ret = 0;
    if (__this->mode) { //esco
        g_printf("esco mode\n");
        ret = esco_pp(pp);
    } else { //a2dp
        g_printf("a2dp mode\n");
        ret = a2dp_pp(pp);
    }
    return ret;
}

int adapter_odev_edr_open(void *priv)
{
    r_printf("adapter_odev_edr_open\n");

    if (__this->status == ODEV_EDR_OPEN) {
        r_printf("adapter_odev_edr_already_open\n");
        return 0;
    }
    __this->status = ODEV_EDR_OPEN;

    memset(__this, 0, sizeof(struct _odev_edr));

    __this->parm = (struct _odev_edr_parm *)priv;

    return 0;
}

int adapter_odev_edr_start(void *priv)
{
    r_printf("adapter_odev_edr_start\n");

    if (__this->status == ODEV_EDR_START) {
        r_printf("adapter_odev_edr_already_start\n");
        return 0;
    }

    __this->status = ODEV_EDR_START;

    INIT_LIST_HEAD(&__this->inquiry_noname_head);

    lmp_set_sniff_establish_by_remote(1);

    inquiry_result_handle_register(adapter_odev_edr_search_result);

    bredr_bulk_change(0);

    ////切换样机状态
    __set_emitter_enable_flag(1);

    a2dp_source_init(NULL, 0, 1);

#if (USER_SUPPORT_PROFILE_HFP_AG==1)
    hfp_ag_buf_init(NULL, 0, 1);
#endif

#if USER_SUPPORT_DUAL_A2DP_SOURCE
    dongle_1t2_init(__this);
#endif
    if (connect_last_device_from_vm()) {
        r_printf("start connect device vm addr\n");
    } else {
        if (__this->parm->poweron_start_search) {
            r_printf("start search device ...\n");
            adapter_odev_edr_search_device();
        }
    }

    return 0;
}

int adapter_odev_edr_stop(void *priv)
{
    if (__this->status == ODEV_EDR_STOP) {
        r_printf("adapter_odev_edr_already_stop\n");
        return 0;
    }
    __this->status = ODEV_EDR_STOP;

    if (__this->edr_search_busy) {
        adapter_odev_edr_search_stop();
    } else {
        adapter_odev_edr_pp(0);
    }
    return 0;
}

int adapter_odev_edr_close(void *priv)
{
    if (__this->status == ODEV_EDR_CLOSE) {
        r_printf("adapter_odev_edr_already_close\n");
        return 0;
    }
    __this->status = ODEV_EDR_CLOSE;

    adapter_odev_edr_search_stop();

    return 0;
}

int adapter_odev_edr_get_status(void *priv)
{
    g_printf("adapter_odev_edr_get_status : %x\n", get_emitter_curr_channel_state());
    if ((get_emitter_curr_channel_state() & A2DP_SRC_CH) && (get_emitter_curr_channel_state() & HFP_AG_CH)) {
        return 1;
    } else {
        return 0;
    }
}



int adapter_odev_edr_media_prepare(u8 mode, int (*fun)(void *, u8, u8, void *), void *priv)
{
    __this->mode = mode;
    __this->fun = fun;
    __this->priv = priv;

    u8 call_status = get_call_status();

    r_printf("mode : %d pp status:%d  call status:%d\n", mode, bt_emitter_stu_get(), call_status);

    if (mode) { //esco
        //stop a2dp
        y_printf("stop a2dp\n");
        a2dp_pp(0);

        if (call_status == BT_CALL_HANGUP) {
            //start esco
            y_printf("start esco\n");
            esco_pp(1);
        } else if (call_status == BT_CALL_ACTIVE) {
            extern struct app_bt_opr app_bt_hdl;
            if (__this->fun) {
                __this->fun(__this->priv, __this->mode, 1, &app_bt_hdl.sco_info);
            }
        }

    } else { //a2dp
        if (call_status != BT_CALL_HANGUP) {
            //stop esco
            y_printf("stop esco\n");
            esco_pp(0);
            __this->start_a2dp_flag = 1;
        } else {
            y_printf("start a2dp\n");
            a2dp_pp(1);
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙SBC编码初始化函数,属于库的弱函数重写
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int a2dp_sbc_encoder_init(void *sbc_struct)
{
    if (!__this->fun) {
        return 0;
    }

    if (sbc_struct) { //start a2dp
        __this->fun(__this->priv, __this->mode, 1, sbc_struct);
    } else {        //stop a2dp
        /* __this->fun(__this->priv, __this->mode, 0, sbc_struct); */
    }

    return 0;
}

int adapter_odev_edr_config(int cmd, void *parm)
{
    switch (cmd) {
    case 0:
        break;
    default:
        break;
    }

    return 0;
}
#endif
