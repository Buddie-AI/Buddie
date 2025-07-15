#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_protocol_api.h"
#include "system/includes.h"
#include "vm.h"
#include "audio_config.h"
#include "app_power_manage.h"
#include "user_cfg.h"

#if APP_PROTOCOL_GMA_CODE

#if 1
#define APP_GMA_LOG       printf
#define APP_GMA_DUMP      put_buf
#else
#define APP_GMA_LOG(...)
#define APP_GMA_DUMP(...)
#endif

//*********************************************************************************//
//                                 GMA认证信息                                      //
//*********************************************************************************//

#define GMA_GMA_FROM_FLASH      0
#define GMA_GMA_FROM_FILE       1

#define GMA_GMA_SOURCH     GMA_GMA_FROM_FLASH


extern int le_controller_set_mac(void *addr);
extern void lmp_hci_write_local_address(const u8 *addr);
extern void lmp_hci_read_local_address(u8 *addr);
extern void bt_update_testbox_addr(u8 *addr);

typedef struct {
    uint8_t mac[6];
    uint32_t pid;
    uint8_t secret[16];
} ali_para_s;

static ali_para_s ali_para;
static ali_para_s *active_ali_para = NULL;

///mac reverse
void gma_mac_addr_reverse_get(uint8_t *buf)
{
    memcpy(buf, (const void *)active_ali_para->mac, 6);
}

#define LEGAL_PID_NUM(cha) (cha>='0' && cha<='9')
#define LEGAL_MAC_NUM(cha) ((cha>='0'&&cha<='9') || (cha>='a'&&cha<='f') || (cha>='A'&&cha<='F') || (cha==':'))
#define LEGAL_SECRET_NUM(cha) ((cha>='0'&&cha<='9') || (cha>='a'&&cha<='f') || (cha>='A'&&cha<='F'))

#define USELESS_CHAR ':'
#define GMA_ALI_PARA_END_CHAR		','
#define GMA_ALI_PARA_MAC_CHAR_NUMS		(sizeof(((ali_para_s *)0)->mac))
#define GMA_ALI_PARA_PID_CHAR_NUMS		4//(sizeof(((ali_para_s *)0)->pid))
#define GMA_ALI_PARA_SECRET_CHAR_NUMS	(sizeof(((ali_para_s *)0)->secret))

static u8 gma_ali_para_char2num(u8 cha)
{
    u8 num = 0;

    if (cha >= '0' && cha <= '9') {
        num = cha - '0';
    } else if (cha >= 'a' && cha <= 'f') {
        num = cha - 'a' + 10;
    } else if (cha >= 'A' && cha <= 'F') {
        num = cha - 'A' + 10;
    }

    return num;
}

static int gma_license_set(ali_para_s *ali_para)
{
    const u8 *gma_ptr = (u8 *)app_protocal_get_license_ptr();

    if (gma_ptr == NULL) {
        return -1;
    }

    ///license fill
    //pid
    memcpy((u8 *) & (ali_para->pid), gma_ptr, sizeof(ali_para->pid));
    gma_ptr += sizeof(ali_para->pid);
    APP_GMA_LOG("gma pid:%d \n", ali_para->pid);
    //secret
    memcpy((u8 *)(ali_para->secret), gma_ptr, sizeof(ali_para->secret));
    gma_ptr += sizeof(ali_para->secret);
    APP_GMA_LOG("gma secret:");
    APP_GMA_DUMP(ali_para->secret, sizeof(ali_para->secret));
    //mac
    memcpy((u8 *)(ali_para->mac), gma_ptr, sizeof(ali_para->mac));
    gma_ptr += sizeof(ali_para->mac);
    {
        //mac reverse
        u8 mac_backup_buf[6];
        memcpy(mac_backup_buf, ali_para->mac, 6);
        int i;
        for (i = 0; i < 6; i++) {
            ali_para->mac[i] = (mac_backup_buf)[5 - i];
        }
    }
    APP_GMA_LOG("gma mac:");
    APP_GMA_DUMP(ali_para->mac, sizeof(ali_para->mac));

    return 0;
}

#if GMA_GMA_SOURCH == GMA_GMA_FROM_FILE

#define ALI_PARA_ACTIVE_NUM               1

static const char *ali_para_string[] = {
    "jl_auif",
    //Product ID,Device Secret,Mac,Md5
    //此处可以用户自行填写从阿里申请的三元组信息
    " ",
};

static bool gma_ali_para_info_fill(const u8 *para, ali_para_s *ali_para)
{
    const u8 *para_string = para;
    u8 highbit_en = 0;
    uint32_t pid = 0;
    uint32_t char_nums = 0;
    uint8_t *secret = ali_para->secret;
    uint8_t *mac = ali_para->mac;

    while (*para_string && *para_string != GMA_ALI_PARA_END_CHAR) {
        if (!LEGAL_PID_NUM(*para_string)) {
            /* APP_GMA_LOG("pid illegal character !!! \n"); */
            para_string++;
            continue;
        }

        ///check pid character numbers legal scope
        char_nums++;
        if (char_nums > GMA_ALI_PARA_PID_CHAR_NUMS) {
            APP_GMA_LOG("pid character numbers overflow !!! \n");
            return false;
        }

        pid *= 10;
        pid += gma_ali_para_char2num(*para_string);
        para_string++;
    }

    ///check pid character numbers legal scope
    if (char_nums != GMA_ALI_PARA_PID_CHAR_NUMS) {
        APP_GMA_LOG("pid character numbers error:%d !!! \n", char_nums);
        return false;
    }

    APP_GMA_LOG(">>pid:%d \n", pid);
    ali_para->pid = pid;

    para_string++;
    char_nums = 0;
    memset(ali_para->secret, 0x00, sizeof(ali_para->secret));
    while (*para_string != GMA_ALI_PARA_END_CHAR) {
        if (!LEGAL_SECRET_NUM(*para_string)) {
            APP_GMA_LOG("secret illegal character !!! \n");
            para_string++;
            continue;
        }

        ///assignment secret
        if (!highbit_en) {
            *secret += gma_ali_para_char2num(*para_string);
            highbit_en ^= 1;
        } else {
            ///check secret character numbers legal scope
            char_nums++;
            if (char_nums > GMA_ALI_PARA_SECRET_CHAR_NUMS) {
                APP_GMA_LOG("secret character number overflow !!! \n");
                return false;
            }

            *secret <<= 4;
            *secret++ += gma_ali_para_char2num(*para_string);
            highbit_en ^= 1;
        }

        para_string++;
    }

    ///check secret character numbers legal scope
    if (char_nums != GMA_ALI_PARA_SECRET_CHAR_NUMS) {
        APP_GMA_LOG("secret character number error !!! :%d \n", char_nums);
        return false;
    }

    ///print secret
    APP_GMA_LOG(">>ali_para secret: ");
    for (int i = 0; i < sizeof(ali_para->secret) / sizeof(ali_para->secret[0]); i++) {
        APP_GMA_LOG("%x ", ali_para->secret[i]);
    }

    memset(ali_para->mac, 0x00, sizeof(ali_para->mac));
    para_string++;
    highbit_en = 0;
    char_nums = 0;
    while (*para_string != GMA_ALI_PARA_END_CHAR) {
        //if legal character
        if (!LEGAL_MAC_NUM(*para_string)) {
            APP_GMA_LOG("mac illegal character !!! \n");
            para_string++;
            continue;
//            return false;
        }

        if (*para_string == USELESS_CHAR) {
            para_string++;
            continue;
        }

        ///assignment mac
        if (!highbit_en) {
            *mac += gma_ali_para_char2num(*para_string);
            highbit_en ^= 1;
        } else {
            //check mac character numbers legal scope
            char_nums++;
            if (char_nums > GMA_ALI_PARA_MAC_CHAR_NUMS) {
                APP_GMA_LOG("mac character numbers overflow !!! \n");
                return false;
            }

            *mac <<= 4;
            *mac++ += gma_ali_para_char2num(*para_string);
            highbit_en ^= 1;
        }

        para_string++;
    }

    //check mac character numbers legal scope
    if (char_nums != GMA_ALI_PARA_MAC_CHAR_NUMS) {
        APP_GMA_LOG("mac character numbers error !!! \n");
        return false;
    }

    ///mac reverse
    for (int i = 0, tmp_mac = 0, array_size = (sizeof(ali_para->mac) / sizeof(ali_para->mac[0])); \
         i < (sizeof(ali_para->mac) / sizeof(ali_para->mac[0]) / 2); i++) {
        tmp_mac = ali_para->mac[i];
        ali_para->mac[i] = ali_para->mac[array_size - 1 - i];
        ali_para->mac[array_size - 1 - i] = tmp_mac;
    }

    ///print secret
    APP_GMA_LOG(">>ali_para mac: ");
    for (int i = 0; i < sizeof(ali_para->mac) / sizeof(ali_para->mac[0]); i++) {
        APP_GMA_LOG("%x ", ali_para->mac[i]);
    }
    return true;
}

static int gma_license2flash(const ali_para_s *ali_para)
{
    int i;
    u8 mac[6];
    u8 rdata[sizeof(ali_para_s)];

    for (i = 0; i < sizeof(mac); i++) {
        mac[i] = ali_para->mac[5 - i];
    }
    memcpy(rdata, &ali_para->pid, sizeof(ali_para->pid));
    memcpy(rdata + sizeof(ali_para->pid), ali_para->secret, sizeof(ali_para->secret));
    memcpy(rdata + sizeof(ali_para->pid) + sizeof(ali_para->secret), mac, sizeof(mac));

    return app_protocol_license2flash(rdata, sizeof(rdata));
}

#endif


#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
static ali_para_s ali_para_remote; //对耳的三元组

//VM_GMA_MAC    对耳的地址
//VM_GMA_ALI_PARA   对耳的三元组

static int gma_strcmp(const u8 *ptr1, const u8 *ptr2, u8 len)
{
    while (len--) {
        if (*ptr1 != *ptr2) {
            return (*ptr1 - *ptr2);
        }

        ptr1++;
        ptr2++;
    }

    return 0;
}

///mac reverse
u8 *gma_sibling_mac_get() //获取地址较小的那个
{
    ali_para_s *sibling_ali_para = NULL;//&ali_para;
    if (active_ali_para == &ali_para) {
        printf("ali_para active \n");
        sibling_ali_para = &ali_para_remote;
    }

    if (active_ali_para == &ali_para_remote) {
        printf("ali_para active \n");
        sibling_ali_para = &ali_para;
    }

    return (sibling_ali_para ? sibling_ali_para->mac : NULL);
}

int gma_select_ali_para_by_common_mac()
{
    u8 ret = 0;
    u8 mac[6];
    u8 mac_buf_tmp[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ret = syscfg_read(CFG_TWS_COMMON_ADDR, mac, 6);
    if (ret != 6 || !memcmp(mac, mac_buf_tmp, 6)) {
        printf("tws no conn\n");
        return -1;
    }
    APP_GMA_LOG("active mac:");
    APP_GMA_DUMP(mac, 6);
    ///select para
    ali_para_s ali_para_temp;
    syscfg_read(VM_GMA_ALI_PARA, (u8 *) &ali_para_temp, sizeof(ali_para_s));
    if (ali_para_temp.pid == ali_para.pid) {
        memcpy(&ali_para_remote, &ali_para_temp, sizeof(ali_para_s));
        APP_GMA_LOG("remote mac:");
        APP_GMA_DUMP(ali_para_remote.mac, 6);
        APP_GMA_LOG("remote pid:%d \n", ali_para_remote.pid);
        APP_GMA_LOG("remote secret:");
        APP_GMA_DUMP(ali_para_remote.secret, sizeof(ali_para_remote.secret));
        if (gma_strcmp(mac, ali_para_remote.mac, 6) == 0) {
            APP_GMA_LOG("used remote para \n");
            active_ali_para = &ali_para_remote;
            return 0;
        } else if (gma_strcmp(mac, ali_para.mac, 6) == 0) {
            APP_GMA_LOG("used local para \n");
            active_ali_para = &ali_para;
            return 0;
        } else {
            APP_GMA_LOG("MAC ERROR !!!!!!!!!!!!!!!!\n");
            return -1;
        }
    } else {
        APP_GMA_LOG("no remote mac \n");
    }
    return -1;
}

void gma_slave_sync_remote_addr(u8 *buf)
{
    u8 common_addr[6];
    ///remote ali_para fill
    memcpy(&ali_para_remote, buf, sizeof(ali_para_remote));
    syscfg_write(VM_GMA_MAC, ali_para_remote.mac, 6);

    ///write ali para to vm
    syscfg_write(VM_GMA_ALI_PARA, (u8 *)&ali_para_remote, sizeof(ali_para_remote));

    tws_api_get_local_addr(common_addr);
    if (memcmp(active_ali_para->mac, common_addr, 6)) { //common地址发生变化
        APP_GMA_LOG("gma addr change");
        ///used slave address
        active_ali_para = &ali_para_remote;
        app_protocol_set_tws_sibling_mac(ali_para.mac);
        app_protocol_set_info_group(GMA_HANDLER_ID, active_ali_para); //设置三元组
        //断开APP接口
        if (is_tws_master_role()) {
            APP_GMA_LOG("gma need reconn app");
            app_protocol_disconnect(NULL);
            app_protocol_ble_adv_switch(0);
            le_controller_set_mac(active_ali_para->mac);
            app_protocol_ble_adv_switch(1);
        }
    }
}

#ifdef CONFIG_NEW_BREDR_ENABLE
void tws_host_get_common_addr(u8 *remote_addr, u8 *common_addr, char channel)
#else
void tws_host_get_common_addr(u8 *local_addr, u8 *remote_addr, u8 *common_addr, char channel)
#endif
{
    APP_GMA_LOG(">>>>>>>>>tws_host_get_common_addr ch:%c \n", channel);

    u8 mac_buf[6];
    gma_mac_addr_reverse_get(mac_buf);
    APP_GMA_LOG(">>>local mac: ");
    APP_GMA_DUMP(mac_buf, 6);
    ///used the bigger one as host bt mac
    if (gma_strcmp(mac_buf, (const u8 *)remote_addr, 6) < 0) {
        ///used slave mac addr;remote_addr from slave addr reverse
        memcpy(common_addr, remote_addr, 6);
    } else {
        ///used master mac addr
        gma_mac_addr_reverse_get(common_addr);
    }
    APP_GMA_LOG("common_addr: ");
    APP_GMA_DUMP((const u8 *)common_addr, 6);
    APP_GMA_LOG("remote_addr: ");
    APP_GMA_DUMP((const u8 *)remote_addr, 6);
}

void gma_kick_license_to_sibling(void)
{
    APP_GMA_LOG("send license info to sibling\n");
    app_protocol_tws_send_to_sibling(GMA_TWS_CMD_SYNC_LIC, (u8 *) & (ali_para), sizeof(ali_para));;
}

void gma_tws_remove_paired(void)
{
    ali_para_s ali_para_temp;
    memset(&ali_para_temp, 0xff, sizeof(ali_para_s));
    syscfg_write(VM_GMA_ALI_PARA, (u8 *)&ali_para_temp, sizeof(ali_para_s));

    active_ali_para = &ali_para;
    app_protocol_set_tws_sibling_mac(ali_para_temp.mac);
    app_protocol_set_info_group(GMA_HANDLER_ID, active_ali_para); //设置三元组

    app_protocol_disconnect(NULL);
    app_protocol_ble_adv_switch(0);
    le_controller_set_mac(active_ali_para->mac);
    app_protocol_ble_adv_switch(1);
}

#endif

__attribute__((weak))
void bt_update_testbox_addr(u8 *addr)
{

}
int gma_prev_init(void)
{
    int err = 0;
    /* ali_para_s ali_para; */
#if GMA_GMA_SOURCH == GMA_GMA_FROM_FLASH
    err = gma_license_set(&ali_para);
#elif GMA_GMA_SOURCH == GMA_GMA_FROM_FILE
    u8 *ali_str = ali_para_string[ALI_PARA_ACTIVE_NUM];
    u8 ali_license[128] = {0};

    //check if exist flash file
    if (read_cfg_file(ali_license, sizeof(ali_license), SDFILE_RES_ROOT_PATH"license.txt") == TRUE) {
        ali_str = ali_license;
    }
    APP_GMA_LOG("ali_str:%s\n", ali_str);
    if (gma_ali_para_info_fill((const u8 *)ali_str, &ali_para) == false) {
        APP_GMA_LOG("array context not good \n");
///read license from flash
        err = gma_license_set(&ali_para);
    }
///write license to flash
    if (!err) {
        gma_license2flash(&ali_para);
    }
#endif
    if (!err) {
        active_ali_para = &ali_para;
#if TCFG_USER_TWS_ENABLE
        if (!gma_select_ali_para_by_common_mac()) {
            app_protocol_set_tws_sibling_mac(gma_sibling_mac_get());
        }
#endif
        app_protocol_set_info_group(GMA_HANDLER_ID, active_ali_para); //设置三元组
        bt_update_mac_addr(active_ali_para->mac);
        lmp_hci_write_local_address(active_ali_para->mac);
        app_protocol_ble_adv_switch(0);
        le_controller_set_mac(active_ali_para->mac); //修改BLE地址
        bt_update_testbox_addr(ali_para.mac);
    }
    return err;
}


//*********************************************************************************//
//                                 GMA 提示音                                     //
//*********************************************************************************//
const char *gma_tone_tab[APP_RROTOCOL_TONE_MAX] = {
    [APP_RROTOCOL_TONE_SPEECH_KEY_START]	    	= TONE_NORMAL,
};

//*********************************************************************************//
//                                 DMA私有消息处理                                 //
//*********************************************************************************//
#if TCFG_USER_TWS_ENABLE
void gma_rx_tws_data_deal(u16 opcode, u8 *data, u16 len)
{
    switch (opcode) {
    case GMA_TWS_CMD_SYNC_LIC:
        printf(">>> TWS_AI_GMA_START_SYNC_LIC \n");
        gma_slave_sync_remote_addr(data);
        break;
    }
}

static void gma_bt_tws_event_handler(struct bt_event *bt)
{
    int role = bt->args[0];
    int phone_link_connection = bt->args[1];
    int reason = bt->args[2];

    switch (bt->event) {
    case TWS_EVENT_CONNECTED:
        gma_kick_license_to_sibling();
        break;
    case TWS_EVENT_REMOVE_PAIRS:
        gma_tws_remove_paired();
        break;
    }
}
#endif

static int gma_bt_status_event_handler(struct bt_event *bt)
{
    return 0;
}

int gma_sys_event_deal(struct sys_event *event)
{
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            gma_bt_status_event_handler(&event->u.bt);
        }
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            gma_bt_tws_event_handler(&event->u.bt);
        }
#endif
        break;

    }

    return 0;

}

struct app_protocol_private_handle_t gma_private_handle = {
#if TCFG_USER_TWS_ENABLE
    .tws_rx_from_siblling = gma_rx_tws_data_deal,
#endif
    /* .tws_sync_func = ; */
    .sys_event_handler = gma_sys_event_deal,
};
#endif

