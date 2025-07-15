// *****************************************************************************
/* EXAMPLE_START(le_counter): LE Peripheral - Heartbeat Counter over GATT
 *
 * @text All newer operating systems provide GATT Client functionality.
 * The LE Counter examples demonstrates how to specify a minimal GATT Database
 * with a custom GATT Service and a custom Characteristic that sends periodic
 * notifications.
 */
// *****************************************************************************

#include "system/app_core.h"
#include "system/includes.h"

#include "app_config.h"
#include "app_action.h"

#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "user_cfg.h"
#include "vm.h"
#include "btcontroller_modules.h"
#include "bt_common.h"

#include "le_client_demo.h"
#include "le_common.h"
#include "ble_user.h"

#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_WIRELESS_MIC_CLIENT)

#define SUPPORT_TEST_BOX_BLE_MASTER_TEST_EN	    1
#define SHOW_RX_DATA_RATE                       1
#define SHOW_TX_DATA_RATE                       1

#if LE_DEBUG_PRINT_EN
/* #define log_info            printf */
#define log_info(x, ...)    printf("[LE_CLIENT]" x " ", ## __VA_ARGS__)
#define log_info_hexdump    put_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif


//------
#define ATT_LOCAL_MTU_SIZE        (517)                    //note: need >= 20
#define ATT_SEND_CBUF_SIZE        (200)//(256)//(512*10)                   //note: need >= 20,缓存大小，可修改
#define ATT_RAM_BUFSIZE           (ATT_CTRL_BLOCK_SIZE + ATT_LOCAL_MTU_SIZE + ATT_SEND_CBUF_SIZE)                   //note:
static u8 att_ram_buffer[ATT_RAM_BUFSIZE] __attribute__((aligned(4)));

#define SEARCH_PROFILE_BUFSIZE    (512)                   //note:
static u8 search_ram_buffer[SEARCH_PROFILE_BUFSIZE] __attribute__((aligned(4)));
#define scan_buffer search_ram_buffer
//---------------
//搜索类型
#define SET_SCAN_TYPE       SCAN_ACTIVE
//搜索 周期大小, 参数一：320 参数二：320 参数三：168
#define SET_SCAN_INTERVAL   320//48//256//48 //(unit:0.625ms)， 96没有干扰的情况下， 连接速度为60ms, 如需要降低功耗， 可以在+16的倍数
//搜索 窗口大小 参数一：102 参数二：70 参数三：30
#define SET_SCAN_WINDOW     70//48//24//48 //(unit:0.625ms)

//连接周期
#define SET_CONN_INTERVAL   3 //(unit:1.25ms)
//连接latency
#define SET_CONN_LATENCY    0  //(unit:conn_interval)
//连接超时
#define SET_CONN_TIMEOUT    400 //(unit:10ms)
#if WIRELESS_TOOL_BLE_NAME_EN
#define TEST_BOX_BLE_NAME		"W_MIC_01"
#define TEST_BOX_BLE_NAME_LEN	(sizeof(TEST_BOX_BLE_NAME)-1)
static u8 *match_name;
#endif
//----------------------------------------------------------------------------
static u8 scan_ctrl_en;
static u8 ble_work_state = 0;
static void (*app_recieve_callback)(void *priv, void *buf, u16 len) = NULL;
static void (*app_ble_state_callback)(void *priv, ble_state_e state) = NULL;
static void (*ble_resume_send_wakeup)(void) = NULL;
static u32 channel_priv;

static hci_con_handle_t con_handle;

#define BLE_VM_HEAD_TAG           (0xB95C)
#define BLE_VM_TAIL_TAG           (0x5CB9)
struct pair_info_t {
    u16 head_tag;
    u8  pair_flag;
    u8  peer_address_info[7];
    u16 tail_tag;
};
static struct pair_info_t  conn_pair_info;
static u8 pair_bond_enable = 0;

//-------------------------------------------------------------------------------
typedef struct {
    uint16_t read_handle;
    uint16_t read_long_handle;
    uint16_t write_handle;
    uint16_t write_no_respond;
    uint16_t notify_handle;
    uint16_t indicate_handle;
} target_hdl_t;

//记录handle 使用
static target_hdl_t target_handle;
static opt_handle_t opt_handle_table[OPT_HANDLE_MAX];
static u8 opt_handle_used_cnt;
static u8 force_seach_onoff = 0;
static s8 force_seach_rssi = -127;
static u8 cur_peer_address_info[7];
static u8 flag_specific_adv_name = 0;
static u8 flag_need_judge_rssi = 0;
static u8 flag_specific_sacn = 0;

static void bt_ble_create_connection(u8 *conn_addr, u8 addr_type);
static int bt_ble_scan_enable(void *priv, u32 en);
static int client_write_send(void *priv, u8 *data, u16 len);
static int client_operation_send(u16 handle, u8 *data, u16 len, u8 att_op_type);
#if WIRELESS_TOOL_BLE_NAME_EN
static ble_state_e get_ble_work_state(void);
#else
static void set_ble_work_state(ble_state_e state);
#endif
static void client_create_connection_cannel(void);
//---------------------------------------------------------------------------
#if 1//default
//指定搜索uuid
//指定搜索uuid
static const target_uuid_t  default_search_uuid_table[] = {

    // for uuid16
    // PRIMARY_SERVICE, ae30
    // CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC,
    // CHARACTERISTIC,  ae02, NOTIFY,

    {
        .services_uuid16 = 0xae30,
        .characteristic_uuid16 = 0xae01,
        .opt_type = ATT_PROPERTY_WRITE_WITHOUT_RESPONSE,
    },

    {
        .services_uuid16 = 0xae30,
        .characteristic_uuid16 = 0xae02,
        .opt_type = ATT_PROPERTY_NOTIFY,
    },

    //for uuid128,sample
    //	PRIMARY_SERVICE, 0000F530-1212-EFDE-1523-785FEABCD123
    //	CHARACTERISTIC,  0000F531-1212-EFDE-1523-785FEABCD123, NOTIFY,
    //	CHARACTERISTIC,  0000F532-1212-EFDE-1523-785FEABCD123, WRITE_WITHOUT_RESPONSE | DYNAMIC,
    /*
    	{
    		.services_uuid16 = 0,
    		.services_uuid128 =       {0x00,0x00,0xF5,0x30 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23} ,
    		.characteristic_uuid16 = 0,
    		.characteristic_uuid128 = {0x00,0x00,0xF5,0x31 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23},
    		.opt_type = ATT_PROPERTY_NOTIFY,
    	},

    	{
    		.services_uuid16 = 0,
    		.services_uuid128 =       {0x00,0x00,0xF5,0x30 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23} ,
    		.characteristic_uuid16 = 0,
    		.characteristic_uuid128 = {0x00,0x00,0xF5,0x32 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23},
    		.opt_type = ATT_PROPERTY_WRITE_WITHOUT_RESPONSE,
    	},
    */

};


static void default_report_data_deal(att_data_report_t *report_data, target_uuid_t *search_uuid)
{
    log_info("report_data:%02x,%02x,%d,len(%d)", report_data->packet_type,
             report_data->value_handle, report_data->value_offset, report_data->blob_length);

    log_info_hexdump(report_data->blob, report_data->blob_length);

    switch (report_data->packet_type) {
    case GATT_EVENT_NOTIFICATION:  //notify
        break;
    case GATT_EVENT_INDICATION://indicate
    case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT://read
        break;
    case GATT_EVENT_LONG_CHARACTERISTIC_VALUE_QUERY_RESULT://read long
        break;

    default:
        break;
    }
}

static const u8 test_remoter_name1[] = "AC637N_MX(BLE)";//
/* static const u8 test_remoter_name2[] = "AC630N_HID567(BLE)";// */
static u16 default_client_write_handle;
static u16 test_client_timer = 0;

static const client_match_cfg_t match_dev01 = {
    .create_conn_mode = BIT(CLI_CREAT_BY_NAME),
    .compare_data_len = sizeof(test_remoter_name1) - 1, //去结束符
    .compare_data = test_remoter_name1,
    .bonding_flag = 0,
};

/* static const client_match_cfg_t match_dev02 = { */
/* .create_conn_mode = BIT(CLI_CREAT_BY_NAME), */
/* .compare_data_len = sizeof(test_remoter_name2) - 1, //去结束符 */
/* .compare_data = test_remoter_name2, */
/* .bonding_flag = 1, */
/* }; */

static void default_test_write(void)
{
    static u32 count = 0;
    count++;
    int ret = client_operation_send(default_client_write_handle, &count, 16, ATT_OP_WRITE_WITHOUT_RESPOND);
    log_info("test_write:%x", ret);
}

static void default_event_callback(le_client_event_e event, u8 *packet, int size)
{
    switch (event) {
    case CLI_EVENT_MATCH_DEV: {
        client_match_cfg_t *match_dev = packet;
        log_info("match_name:%s\n", match_dev->compare_data);
    }
    break;

    case CLI_EVENT_MATCH_UUID: {
        opt_handle_t *opt_hdl = packet;
        if (opt_hdl->search_uuid == &default_search_uuid_table[0]) {
            default_client_write_handle = opt_hdl->value_handle;
            log_info("match_uuid22\n");
        }
    }
    break;

    case CLI_EVENT_SEARCH_PROFILE_COMPLETE:
        log_info("CLI_EVENT_SEARCH_PROFILE_COMPLETE\n");
        if ((!test_client_timer) && default_client_write_handle) {
            log_info("test timer_add\n");
            test_client_timer = sys_hi_timer_add(0, default_test_write, 500);
        }
        break;

    case CLI_EVENT_CONNECTED:
        break;

    case CLI_EVENT_DISCONNECT:
        if (test_client_timer) {
            sys_timeout_del(test_client_timer);
            test_client_timer = 0;
        }
        default_client_write_handle = 0;
        break;

    default:
        break;
    }
}


static const client_conn_cfg_t client_conn_config_default = {
    .match_dev_cfg[0] = &match_dev01,
    .match_dev_cfg[1] = NULL,
    .match_dev_cfg[2] = NULL,
    /* .match_dev_cfg[1] = &match_dev02, */
    .report_data_callback = default_report_data_deal,
    .search_uuid_cnt = (sizeof(default_search_uuid_table) / sizeof(target_uuid_t)),
    .search_uuid_table = default_search_uuid_table,
    .security_en = 0,
    .event_callback = default_event_callback,
};

#endif

//---------------------------------------------------------------------------
static  client_conn_cfg_t *client_config = NULL; //(void *) &client_conn_config_default ;
//----------------------------------------------------------------------------


static const struct conn_update_param_t connection_param_table[] = {
    {16, 24, 0, 600},//11
    {12, 28, 0, 600},//3.7
    {8,  20, 0, 600},
    {50, 60, 0, 600},
};

static u8 send_param_index = 3;

static int client_create_connect_api(u8 *addr, u8 addr_type, u8 mode);
static int client_create_cannel_api(void);
//----------------------------------------------------------------------------

static void client_event_report(le_client_event_e event, u8 *packet, int size)
{
    if (client_config->event_callback) {
        client_config->event_callback(event, packet, size);
    }
}

static bool check_device_is_match(u8 info_type, u8 *data, int size)
{
    int i;
    u8  conn_mode = BIT(info_type);
    client_match_cfg_t *cfg;

    /* log_info_hexdump(data,size); */

    for (i = 0; i < CLIENT_MATCH_CONN_MAX; i++) {
        cfg = client_config->match_dev_cfg[i];
        if (cfg == NULL) {
            continue;
        }
        /* log_info("cfg = %08x\n",cfg);	 */
        /* log_info_hexdump(cfg,sizeof(client_match_cfg_t)); */
        if (cfg->create_conn_mode == conn_mode && size == cfg->compare_data_len) {
            log_info("match check\n");
            /* log_info_hexdump(data, size); */
            /* log_info_hexdump(cfg->compare_data, size); */
            if (0 == memcmp(data, cfg->compare_data, cfg->compare_data_len)) {
                log_info("match ok\n");
                pair_bond_enable = cfg->bonding_flag;
                client_event_report(CLI_EVENT_MATCH_DEV, cfg, sizeof(client_match_cfg_t));
                return true;
            }
        }
    }
    return false;
}

static void conn_pair_vm_do(struct pair_info_t *info, u8 rw_flag)
{
    /* return; */

    int ret;
    int vm_len = sizeof(struct pair_info_t);

    log_info("-conn_pair_info vm_do:%d\n", rw_flag);
    if (rw_flag == 0) {
        ret = syscfg_read(CFG_BLE_MODE_INFO, (u8 *)info, vm_len);
        if (!ret) {
            log_info("-null--\n");
        }
        if ((BLE_VM_HEAD_TAG == info->head_tag) && (BLE_VM_TAIL_TAG == info->tail_tag)) {
            log_info("-exist--\n");
            log_info_hexdump((u8 *)info, vm_len);
        } else {
            memset(info, 0, vm_len);
            info->head_tag = BLE_VM_HEAD_TAG;
            info->tail_tag = BLE_VM_TAIL_TAG;
        }
    } else {
        syscfg_write(CFG_BLE_MODE_INFO, (u8 *)info, vm_len);
    }
}
//------------------------------------------------------------
static ble_state_e get_ble_work_state(void)
{
    return ble_work_state;
}

void clear_bonding_info(void)
{
    log_info("client_clear_bonding_info\n");
    conn_pair_vm_do(&conn_pair_info, 0);
    if (conn_pair_info.pair_flag) {
        //del pair bond
        memset(&conn_pair_info, 0, sizeof(struct pair_info_t));
        conn_pair_vm_do(&conn_pair_info, 1);

#if WIRELESS_PAIR_BONDING
        if (get_ble_work_state() == BLE_ST_CREATE_CONN) {
            client_create_connection_cannel();
            bt_ble_scan_enable(0, 1);
        } else if (con_handle) {
            ble_disconnect();
        }
#endif
    }
}

//------------------------------------------------------------
static void set_ble_work_state(ble_state_e state)
{
    if (state != ble_work_state) {
        log_info("ble_client_work_st:%x->%x\n", ble_work_state, state);
        ble_work_state = state;
        if (app_ble_state_callback) {
            app_ble_state_callback((void *)channel_priv, state);
        }
    }
}


//-------------------------------------------------------------------------------
static void check_target_uuid_match(search_result_t *result_info)
{
    u32 i;
    target_uuid_t *t_uuid;

    for (i = 0; i < client_config->search_uuid_cnt; i++) {
        t_uuid = &client_config->search_uuid_table[i];
        if (result_info->services.uuid16) {
            if (result_info->services.uuid16 != t_uuid->services_uuid16) {
                /* log_info("b1"); */
                continue;
            }
        } else {
            if (memcmp(result_info->services.uuid128, t_uuid->services_uuid128, 16)) {
                /* log_info("b2"); */
                continue;
            }
        }

        if (result_info->characteristic.uuid16) {
            if (result_info->characteristic.uuid16 != t_uuid->characteristic_uuid16) {
                /* log_info("b3"); */
                /* log_info("%d: %04x--%04x",result_info->characteristic.uuid16,t_uuid->characteristic_uuid16); */
                continue;
            }
        } else {
            if (memcmp(result_info->characteristic.uuid128, t_uuid->characteristic_uuid128, 16)) {
                /* log_info("b4"); */
                continue;
            }
        }

        break;//match one
    }

    if (i >= client_config->search_uuid_cnt) {
        return;
    }

    if (opt_handle_used_cnt >= OPT_HANDLE_MAX) {
        log_info("opt_handle is full!!!\n");
        return;
    }

    if ((t_uuid->opt_type & result_info->characteristic.properties) != t_uuid->opt_type) {
        log_info("properties not match!!!\n");
        return;
    }

    log_info("match one uuid\n");

    opt_handle_t *opt_get = &opt_handle_table[opt_handle_used_cnt++];
    opt_get->value_handle = result_info->characteristic.value_handle;
    opt_get->search_uuid = t_uuid;

    switch (t_uuid->opt_type) {
    case ATT_PROPERTY_READ:
        target_handle.read_handle = result_info->characteristic.value_handle;
        break;

    case ATT_PROPERTY_WRITE_WITHOUT_RESPONSE:
        target_handle.write_no_respond = result_info->characteristic.value_handle;
        break;

    case ATT_PROPERTY_WRITE:
        target_handle.write_handle = result_info->characteristic.value_handle;
        break;

    case ATT_PROPERTY_NOTIFY:
        target_handle.notify_handle  = result_info->characteristic.value_handle;
        break;

    case ATT_PROPERTY_INDICATE:
        target_handle.indicate_handle = result_info->characteristic.value_handle;
        break;

    default:
        break;
    }

    client_event_report(CLI_EVENT_MATCH_UUID, opt_get, sizeof(opt_handle_t));

}

//操作handle，完成 write ccc
static void do_operate_search_handle(void)
{
    u16 tmp_16;
    u16 i, cur_opt_type;
    opt_handle_t *opt_hdl_pt;

    log_info("opt_handle_used_cnt= %d\n", opt_handle_used_cnt);

    log_info("find target_handle:");
    log_info_hexdump(&target_handle, sizeof(target_hdl_t));

    if (0 == opt_handle_used_cnt) {
        goto opt_end;
    }

    /* test_send_conn_update();//for test */

    for (i = 0; i < opt_handle_used_cnt; i++) {
        opt_hdl_pt = &opt_handle_table[i];
        cur_opt_type = opt_hdl_pt->search_uuid->opt_type;
        switch ((u8)cur_opt_type) {
        case ATT_PROPERTY_READ:
            if (1) {
                tmp_16  = 0x55A2;//fixed
                log_info("read_long:%04x\n", opt_hdl_pt->value_handle);
                client_operation_send(opt_hdl_pt->value_handle, (u8 *)&tmp_16, 2, ATT_OP_READ_LONG);
            } else {
                tmp_16  = 0x55A1;//fixed
                log_info("read:%04x\n", opt_hdl_pt->value_handle);
                client_operation_send(opt_hdl_pt->value_handle, (u8 *)&tmp_16, 2, ATT_OP_READ);
            }
            break;

        case ATT_PROPERTY_NOTIFY:
            tmp_16  = 0x01;//fixed
            log_info("write_ntf_ccc:%04x\n", opt_hdl_pt->value_handle);
            client_operation_send(opt_hdl_pt->value_handle + 1, &tmp_16, 2, ATT_OP_WRITE);
            break;

        case ATT_PROPERTY_INDICATE:
            tmp_16  = 0x02;//fixed
            log_info("write_ind_ccc:%04x\n", opt_hdl_pt->value_handle);
            client_operation_send(opt_hdl_pt->value_handle + 1, &tmp_16, 2, ATT_OP_WRITE);
            break;

        default:
            break;
        }
    }

opt_end:
    set_ble_work_state(BLE_ST_SEARCH_COMPLETE);

}

//协议栈内部调用
//return: 0--accept,1--reject
int l2cap_connection_update_request_just(u8 *packet, hci_con_handle_t handle)
{
    log_info("slave request conn_update:\n-interval_min= %d,\n-interval_max= %d,\n-latency= %d,\n-timeout= %d\n",
             little_endian_read_16(packet, 0), little_endian_read_16(packet, 2),
             little_endian_read_16(packet, 4), little_endian_read_16(packet, 6));
    return 0;
    /* return 1; */
}

//协议栈内部调用
void user_client_report_search_result(search_result_t *result_info)
{
    if (result_info == (void *) - 1) {
        log_info("client_report_search_result finish!!!\n");
        do_operate_search_handle();
        client_event_report(CLI_EVENT_SEARCH_PROFILE_COMPLETE, 0, 0);
        return;
    }

    log_info("\n*** services, uuid16:%04x,index=%d ***\n", result_info->services.uuid16, result_info->service_index);
    log_info("{charactc, uuid16:%04x,index=%d,handle:%04x~%04x,value_handle=%04x}\n",
             result_info->characteristic.uuid16, result_info->characteristic_index,
             result_info->characteristic.start_handle, result_info->characteristic.end_handle,
             result_info->characteristic.value_handle
            );

    if (!result_info->services.uuid16) {
        log_info("######services_uuid128:");
        log_info_hexdump(result_info->services.uuid128, 16);
    }

    if (!result_info->characteristic.uuid16) {
        log_info("######charact_uuid128:");
        log_info_hexdump(result_info->characteristic.uuid128, 16);
    }

    check_target_uuid_match(result_info);
}

#if (SHOW_RX_DATA_RATE | SHOW_TX_DATA_RATE)

static u32 client_timer_handle = 0;
static u32 test_rx_data_count;
static u32 test_tx_data_count;

static void client_timer_handler(void)
{
    if (!con_handle) {
        test_rx_data_count = 0;

        test_tx_data_count = 0;

        return;
    }

    if (test_rx_data_count) {
        log_info("peer_rssi = %d\n", ble_vendor_get_peer_rssi(con_handle));
        /* log_info("\n-ble_data_rate: %d bps-\n", test_rx_data_count * 8); */
        log_info("%d bytes receive: %d.%02d KB/s \n", test_rx_data_count, test_rx_data_count / 1000, test_rx_data_count % 1000);

        test_rx_data_count = 0;
    }

    if (test_tx_data_count) {
        log_info("peer_rssi = %d\n", ble_vendor_get_peer_rssi(con_handle));
        log_info("%d bytes send: %d.%02d KB/s \n", test_tx_data_count, test_tx_data_count / 1000, test_tx_data_count % 1000);

        test_tx_data_count = 0;
    }
}

static void client_timer_start(void)
{
    client_timer_handle  = sys_timer_add(NULL, client_timer_handler, 1000);
}

#endif /* SHOW_RX_DATA_RATE | SHOW_TX_DATA_RATE */

static target_uuid_t *get_match_handle_target(u16 handle)
{
    for (int i = 0; i < opt_handle_used_cnt; i++) {
        if (opt_handle_table[i].value_handle == handle) {
            return opt_handle_table[i].search_uuid;
        }
    }
    return NULL;
}

//协议栈内部调用
void user_client_report_data_callback(att_data_report_t *report_data)
{
    /* log_info("\n-report_data:type %02x,handle %04x,offset %d,len %d:",report_data->packet_type, */
    /* 		report_data->value_handle,report_data->value_offset,report_data->blob_length); */
    /* log_info_hexdump(report_data->blob,report_data->blob_length); */

#if SHOW_RX_DATA_RATE
    test_rx_data_count += report_data->blob_length;
#endif /* SHOW_RX_DATA_RATE */

    target_uuid_t *search_uuid = get_match_handle_target(report_data->value_handle);

    if (client_config->report_data_callback) {
        client_config->report_data_callback(report_data, search_uuid);
        return;
    }

    switch (report_data->packet_type) {
    case GATT_EVENT_NOTIFICATION://notify
//        log_info("\n-notify_rx(%d):",report_data->blob_length);
    case GATT_EVENT_INDICATION://indicate
    case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT://read
    case GATT_EVENT_LONG_CHARACTERISTIC_VALUE_QUERY_RESULT://read long
        break;
    default:
        break;
    }
}

//	PRIMARY_SERVICE,AE30
/* static const u16 test_services_uuid16 = 0xae30; */

//	PRIMARY_SERVICE, 0000F530-1212-EFDE-1523-785FEABCD123
/* static const u8  test_services_uuid128[16] = {0x00,0x00,0xF5,0x30 ,0x12,0x12 ,0xEF, 0xDE ,0x15,0x23 ,0x78,0x5F,0xEA ,0xBC,0xD1,0x23};  */

static void client_search_profile_start(void)
{
    opt_handle_used_cnt = 0;
    memset(&target_handle, 0, sizeof(target_hdl_t));
    user_client_init(con_handle, search_ram_buffer, SEARCH_PROFILE_BUFSIZE);
    if (client_config->search_uuid_cnt) {
        printf("[debug] %s, %d\n", __FUNCTION__, __LINE__);
        ble_op_search_profile_all();
    } else {
        printf("[debug] %s, %d\n", __FUNCTION__, __LINE__);
        user_client_set_search_complete();
    }
}

//------------------------------------------------------------
static bool resolve_adv_report(u8 *adv_address, u8 data_length, u8 *data, s8 rssi)
{
    u8 i, lenght, ad_type;
    u8 *adv_data_pt;
    u8 find_remoter = 0;
    /* u8 tmp_addr[6]; */
    u32 tmp32;

    if (check_device_is_match(CLI_CREAT_BY_ADDRESS, adv_address, 6)) {
        find_remoter = 1;
    }

    adv_data_pt = data;
    for (i = 0; i < data_length;) {
        if (*adv_data_pt == 0) {
            /* log_info("analyze end\n"); */
            break;
        }

        lenght = *adv_data_pt++;

        if (lenght >= data_length || (lenght + i) >= data_length) {
            /*过滤非标准包格式*/
            printf("!!!error_adv_packet:");
            put_buf(data, data_length);
            break;
        }

        ad_type = *adv_data_pt++;
        i += (lenght + 1);

        switch (ad_type) {
        case HCI_EIR_DATATYPE_FLAGS:
            /* log_info("flags:%02x\n",adv_data_pt[0]); */
            break;

        case HCI_EIR_DATATYPE_MORE_16BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_MORE_32BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_32BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_MORE_128BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_128BIT_SERVICE_UUIDS:
            /* log_info("service uuid:"); */
            /* log_info_hexdump(adv_data_pt, lenght - 1); */
            break;

        case HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME:
        case HCI_EIR_DATATYPE_SHORTENED_LOCAL_NAME:
            tmp32 = adv_data_pt[lenght - 1];
            adv_data_pt[lenght - 1] = 0;;
            log_info("remoter_name: %s,rssi:%d\n", adv_data_pt, rssi);
            log_info_hexdump(adv_address, 6);
            adv_data_pt[lenght - 1] = tmp32;
            //-------
#if 1	//无线麦产线通过识别特殊字符串进行近距离连接测试
            if (0 == memcmp(adv_data_pt, SPECIFIC_STRING, strlen(SPECIFIC_STRING))) {
                flag_specific_adv_name = 1;
                flag_need_judge_rssi = 1;
            }
            if (flag_specific_sacn) {
                flag_need_judge_rssi = 1;
            }
            if (flag_need_judge_rssi) {
                flag_need_judge_rssi = 0;
                if (rssi + TCFG_WIRELESS_RSSI < 0) {
                    printf("rssi no satisfy,break");
                    break;
                } else {
                    printf("rssi satisfy,RSSI = %d", rssi);
                    if (flag_specific_adv_name) {
                        find_remoter = 1;
                        break;
                    }
                }
            }

            flag_specific_adv_name = 0;
#endif

#if SUPPORT_TEST_BOX_BLE_MASTER_TEST_EN
#if !WIRELESS_TOOL_BLE_NAME_EN
#define TEST_BOX_BLE_NAME		"W_MIC_01"
#define TEST_BOX_BLE_NAME_LEN	sizeof(TEST_BOX_BLE_NAME)
            printf("TEST_BOX_BLE_NAME_LEN=%d", TEST_BOX_BLE_NAME_LEN);
            if (0 == memcmp(adv_data_pt, TEST_BOX_BLE_NAME, TEST_BOX_BLE_NAME_LEN)) {
                find_remoter = 1;
                break;
            }
#else
            put_buf(match_name, strlen(match_name));
            if (lenght == strlen(match_name) + 1) {
                printf("TEST_BOX_BLE_NAME_LEN=%d", TEST_BOX_BLE_NAME_LEN);
                if (0 == memcmp(adv_data_pt, match_name, lenght - 1)) {
                    find_remoter = 1;
                    break;
                }
            }
#endif
#endif
            /* log_info("target name:%s", client_config->compare_data); */
            if (check_device_is_match(CLI_CREAT_BY_NAME, adv_data_pt, lenght - 1)) {
                find_remoter = 1;
                log_info("catch name ok\n");
            }
            break;

        case HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA:
            if (check_device_is_match(CLI_CREAT_BY_TAG, adv_data_pt, lenght - 1)) {
                log_info("get_tag_string!\n");
                find_remoter = 1;
            }
            break;

        case HCI_EIR_DATATYPE_APPEARANCE_DATA:
            /* log_info("get_class_type:%04x\n",little_endian_read_16(adv_data_pt,0)); */
            break;

        default:
            /* log_info("unknow ad_type:"); */
            break;
        }

        if (find_remoter) {
            log_info_hexdump(adv_data_pt, lenght - 1);
        }
        adv_data_pt += (lenght - 1);
    }

    return find_remoter;
}

static void client_report_adv_data(adv_report_t *report_pt, u16 len)
{
    bool find_remoter;

    /* log_info("event_type,addr_type: %x,%x; ",report_pt->event_type,report_pt->address_type); */
    /* log_info_hexdump(report_pt->address,6); */

    /* log_info("adv_data_display:"); */
    /* log_info_hexdump(report_pt->data,report_pt->length); */

//    log_info("rssi:%d\n",report_pt->rssi);
#if WIRELESS_PAIR_BONDING

    if (conn_pair_info.pair_flag) {
        if (report_pt->event_type != 1) {
            return;
        }
        printf("report_add_type == %x", report_pt->address_type);
        printf("conn_add_type == %x", conn_pair_info.peer_address_info[0]);
        put_buf(report_pt->address, 6);
        put_buf(&conn_pair_info.peer_address_info[1], 6);
        if (report_pt->address_type == conn_pair_info.peer_address_info[0]
            && 0 == memcmp(&conn_pair_info.peer_address_info[1], report_pt->address, 6)) {
            log_info("match bond_dev\n");
            find_remoter = 1;
            goto just_creat;
        } else {
            return;
        }
    }
    find_remoter = resolve_adv_report(report_pt->address, report_pt->length, report_pt->data, report_pt->rssi);

just_creat:

#else
    find_remoter = resolve_adv_report(report_pt->address, report_pt->length, report_pt->data, report_pt->rssi);
#endif

    if (find_remoter) {
        if (force_seach_onoff && force_seach_rssi > report_pt->rssi) {
            log_info("match but rssi fail!!!:%d,%d\n", force_seach_rssi, report_pt->rssi);
            return;
        }

        log_info("rssi:%d\n", report_pt->rssi);
        log_info("\n*********create_connection***********\n");
        log_info("***remote type %d,addr:", report_pt->address_type);
        log_info_hexdump(report_pt->address, 6);
        bt_ble_scan_enable(0, 0);
        client_create_connect_api(report_pt->address, report_pt->address_type, 0);
        log_info("*create_finish\n");
    }
}


static void client_create_connection(u8 *conn_addr, u8 addr_type)
{
    struct create_conn_param_t *create_conn_par = scan_buffer;
    if (get_ble_work_state() == BLE_ST_CREATE_CONN) {
        log_info("already create conn!!!\n");
        return;
    }
    create_conn_par->conn_interval = SET_CONN_INTERVAL;
    create_conn_par->conn_latency = SET_CONN_LATENCY;
    create_conn_par->supervision_timeout = SET_CONN_TIMEOUT;
    memcpy(create_conn_par->peer_address, conn_addr, 6);
    create_conn_par->peer_address_type = addr_type;

    set_ble_work_state(BLE_ST_CREATE_CONN);
    log_info_hexdump(create_conn_par, sizeof(struct create_conn_param_t));
    ble_op_create_connection(create_conn_par);
}

static void bt_ble_create_connection(u8 *conn_addr, u8 addr_type)
{
    client_create_connection(conn_addr, addr_type);
}

static void client_create_connection_cannel(void)
{
    if (get_ble_work_state() == BLE_ST_CREATE_CONN) {
        set_ble_work_state(BLE_ST_SEND_CREATE_CONN_CANNEL);
        ble_op_create_connection_cancel();
    }
}

static int ble_disconnect(void *priv)
{
    if (con_handle) {
        if (BLE_ST_SEND_DISCONN != get_ble_work_state()) {
            log_info(">>>ble send disconnect\n");
            set_ble_work_state(BLE_ST_SEND_DISCONN);
            ble_op_disconnect(con_handle);
        } else {
            log_info(">>>ble wait disconnect...\n");
        }
        return APP_BLE_NO_ERROR;
    } else {
        return APP_BLE_OPERATION_ERROR;
    }
}

static void connection_update_complete_success(u8 *packet)
{
    int con_handle, conn_interval, conn_latency, conn_timeout;

    conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
    conn_latency = hci_subevent_le_connection_update_complete_get_conn_latency(packet);
    conn_timeout = hci_subevent_le_connection_update_complete_get_supervision_timeout(packet);

    log_info("conn_interval = %d\n", conn_interval);
    log_info("conn_latency = %d\n", conn_latency);
    log_info("conn_timeout = %d\n", conn_timeout);
}


static void cbk_sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    sm_just_event_t *event = (void *)packet;
    u32 tmp32;
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            log_info("Just Works Confirmed.\n");
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            log_info_hexdump(packet, size);
            memcpy(&tmp32, event->data, 4);
            log_info("Passkey display: %06u.\n", tmp32);
            break;
        }
        break;
    }
}

static void can_send_now_wakeup(void)
{
    /* putchar('E'); */
    if (ble_resume_send_wakeup) {
        ble_resume_send_wakeup();
    }
}

const char *const phy_result[] = {
    "None",
    "1M",
    "2M",
    "Coded",
};

static void set_connection_data_length(u16 tx_octets, u16 tx_time)
{
    if (con_handle) {
        ble_op_set_data_length(con_handle, tx_octets, tx_time);
    }
}

static void set_connection_data_phy(u8 tx_phy, u8 rx_phy)
{
    if (0 == con_handle) {
        return;
    }

    u8 all_phys = 0;
    u16 phy_options = 0;

    ble_op_set_ext_phy(con_handle, all_phys, tx_phy, rx_phy, phy_options);
}

static void client_profile_start(u16 con_handle)
{
    ble_op_att_send_init(con_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_MTU_SIZE);
    set_ble_work_state(BLE_ST_CONNECT);
    printf("[debug] %s, %d\n", __FUNCTION__, __LINE__);
    if (0 == client_config->security_en) {
        printf("[debug] %s, %d\n", __FUNCTION__, __LINE__);
        client_search_profile_start();
    }
}

/* LISTING_START(packetHandler): Packet Handler */
static void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    int mtu;
    u32 tmp;
    u8 status;

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {

        /* case DAEMON_EVENT_HCI_PACKET_SENT: */
        /* break; */
        case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
            log_info("ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE\n");
        case ATT_EVENT_CAN_SEND_NOW:
            can_send_now_wakeup();
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {

            case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
                status = hci_subevent_le_enhanced_connection_complete_get_status(packet);
                if (status) {
                    log_info("LE_MASTER CREATE CONNECTION FAIL!!! %0x\n", status);
                    set_ble_work_state(BLE_ST_DISCONN);
                    break;
                }
                con_handle = hci_subevent_le_enhanced_connection_complete_get_connection_handle(packet);
                log_info("HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE : 0x%0x\n", con_handle);
                log_info("conn_interval = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_interval(packet));
                log_info("conn_latency = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_latency(packet));
                log_info("conn_timeout = %d\n", hci_subevent_le_enhanced_connection_complete_get_supervision_timeout(packet));
                client_profile_start(con_handle);
                break;

            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                if (packet[3]) {
                    log_info("LE_MASTER CREATE CONNECTION FAIL!!! %0x\n", packet[3]);
                    set_ble_work_state(BLE_ST_DISCONN);
                    break;
                }
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                log_info("HCI_SUBEVENT_LE_CONNECTION_COMPLETE : %0x\n", con_handle);
                connection_update_complete_success(packet + 8);
                client_profile_start(con_handle);
                client_event_report(CLI_EVENT_CONNECTED, packet, size);
#if WIRELESS_PAIR_BONDING
                memcpy(cur_peer_address_info, &packet[7], 7);
                conn_pair_info.pair_flag = 1;
                printf("pair_flag == %d", conn_pair_info.pair_flag);
                put_buf(cur_peer_address_info, 7);
                memcpy(&conn_pair_info.peer_address_info, cur_peer_address_info, 7);
                conn_pair_info.head_tag = BLE_VM_HEAD_TAG;
                conn_pair_info.tail_tag = BLE_VM_TAIL_TAG;
                conn_pair_vm_do(&conn_pair_info, 1);
#else
                if (pair_bond_enable) {
                    conn_pair_info.pair_flag = 1;
                    memcpy(&conn_pair_info.peer_address_info, &packet[7], 7);
                    conn_pair_vm_do(&conn_pair_info, 1);
                    pair_bond_enable = 0;
                }
#endif
                break;

            case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                log_info("APP HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE\n");
                connection_update_complete_success(packet);

                client_event_report(CLI_EVENT_CONNECTION_UPDATE, packet, size);

                break;

            case HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE:
                log_info("APP HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE");
                break;

            case HCI_SUBEVENT_LE_PHY_UPDATE_COMPLETE:
                log_info("APP HCI_SUBEVENT_LE_PHY_UPDATE %s\n", hci_event_le_meta_get_phy_update_complete_status(packet) ? "Fail" : "Succ");
                log_info("Tx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_tx_phy(packet)]);
                log_info("Rx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_rx_phy(packet)]);
                break;

            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            log_info("HCI_EVENT_DISCONNECTION_COMPLETE: %0x\n", packet[5]);
            con_handle = 0;
            ble_op_att_send_init(con_handle, 0, 0, 0);
            set_ble_work_state(BLE_ST_DISCONN);
            client_event_report(CLI_EVENT_DISCONNECT, packet, size);

            //auto to do
#if !WIRELESS_PAIR_BONDING
            if (conn_pair_info.pair_flag) {
                client_create_connect_api(0, 0, 1);
            } else
#endif
            {
                bt_ble_scan_enable(0, 1);
            }
            break;

        case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
            mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
            log_info("ATT MTU = %u\n", mtu);
            ble_op_att_set_send_mtu(mtu);
            break;

        case HCI_EVENT_VENDOR_REMOTE_TEST:
            log_info("--- HCI_EVENT_VENDOR_REMOTE_TEST\n");
            break;

        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            tmp = little_endian_read_16(packet, 4);
            log_info("-update_rsp: %02x\n", tmp);
            break;

        case GAP_EVENT_ADVERTISING_REPORT:
            /* putchar('@'); */
            client_report_adv_data((void *)&packet[2], packet[1]);
            break;

        case HCI_EVENT_ENCRYPTION_CHANGE:
            log_info("HCI_EVENT_ENCRYPTION_CHANGE= %d\n", packet[2]);
            if (client_config->security_en) {
                client_search_profile_start();
            }
            break;
        }
        break;
    }
}


static int get_buffer_vaild_len(void *priv)
{
    u32 vaild_len = 0;
    ble_op_att_get_remain(&vaild_len);
    return vaild_len;
}

static int client_operation_send(u16 handle, u8 *data, u16 len, u8 att_op_type)
{
    int ret = APP_BLE_NO_ERROR;
    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (!handle) {
        log_info("handle is null\n");
        return APP_BLE_OPERATION_ERROR;
    }


    if (get_buffer_vaild_len(0) < len) {
        /* log_info("opt_buff_full!!!\n"); */
        return APP_BLE_BUFF_FULL;
    }

    ret = ble_op_att_send_data(handle, data, len, att_op_type);
    if (ret == BLE_BUFFER_FULL) {
        ret = APP_BLE_BUFF_FULL;
    } else {
#if SHOW_TX_DATA_RATE
        test_tx_data_count += len;
#endif /* SHOW_TX_DATA_RATE */
    }

    return ret;
}


//-----------------------------------------------
static int client_write_send(void *priv, u8 *data, u16 len)
{
    return client_operation_send(target_handle.write_handle, data, len, ATT_OP_WRITE);
}

static int client_write_without_respond_send(void *priv, u8 *data, u16 len)
{
    return client_operation_send(target_handle.write_no_respond, data, len, ATT_OP_WRITE_WITHOUT_RESPOND);
}

static int client_read_value_send(void *priv)
{
    u16 tmp_flag = 0x55A1;
    return client_operation_send(target_handle.read_handle, (u8 *)&tmp_flag, 2, ATT_OP_READ);
}

static int client_read_long_value_send(void *priv)
{
    u16 tmp_flag = 0x55A2;
    return client_operation_send(target_handle.read_handle, (u8 *)&tmp_flag, 2, ATT_OP_READ_LONG);
}


//扫描数设置
static void scanning_setup_init(void)
{
    ble_op_set_scan_param(SET_SCAN_TYPE, SET_SCAN_INTERVAL, SET_SCAN_WINDOW);
}

static int bt_ble_scan_enable(void *priv, u32 en)
{
    ble_state_e next_state, cur_state;

    if (!scan_ctrl_en && en) {
        return 	APP_BLE_OPERATION_ERROR;
    }

    if (en) {
        next_state = BLE_ST_SCAN;
    } else {
        next_state = BLE_ST_IDLE;
    }

    cur_state =  get_ble_work_state();
    switch (cur_state) {
    case BLE_ST_SCAN:
    case BLE_ST_IDLE:
    case BLE_ST_INIT_OK:
    case BLE_ST_NULL:
    case BLE_ST_DISCONN:
    case BLE_ST_CONNECT_FAIL:
    case BLE_ST_SEND_CREATE_CONN_CANNEL:
        break;
    default:
        return APP_BLE_OPERATION_ERROR;
        break;
    }

    if (cur_state == next_state) {
        return APP_BLE_NO_ERROR;
    }


    log_info("scan_en:%d\n", en);
    set_ble_work_state(next_state);

    if (en) {
        scanning_setup_init();
    }
    ble_op_scan_enable2(en, 0);

    return APP_BLE_NO_ERROR;
}

static int client_regiest_wakeup_send(void *priv, void *cbk)
{
    /* att_regist_wakeup_send(cbk); */
    return APP_BLE_NO_ERROR;
}

static int client_regiest_recieve_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_recieve_callback = cbk;
    return APP_BLE_NO_ERROR;
}

static int client_regiest_state_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_ble_state_callback = cbk;
    return APP_BLE_NO_ERROR;
}

//该接口重新配置搜索的配置项
static int client_init_config(void *priv, const client_conn_cfg_t *cfg)
{
    log_info("client_init_config\n");
    client_config = cfg;//reset config
    return APP_BLE_NO_ERROR;
}

//可配置进入强制搜索方式连接，更加信号强度过滤设备
static int client_force_search(u8 onoff, s8 rssi)
{
    force_seach_rssi = rssi;
    if (force_seach_onoff != onoff) {

        force_seach_onoff = onoff;
        //强制搜索前后，关创建监听
        if (get_ble_work_state() == BLE_ST_CREATE_CONN) {
            client_create_connection_cannel();
        }
    }
    return 0;
}

static int client_create_connect_api(u8 *addr, u8 addr_type, u8 mode)
{
    u8 cur_state =  get_ble_work_state();

    switch (cur_state) {
    case BLE_ST_SCAN:
    case BLE_ST_IDLE:
    case BLE_ST_INIT_OK:
    case BLE_ST_NULL:
    case BLE_ST_DISCONN:
    case BLE_ST_CONNECT_FAIL:
    case BLE_ST_SEND_CREATE_CONN_CANNEL:
        break;
    default:
        return APP_BLE_OPERATION_ERROR;
        break;
    }

    if (cur_state == BLE_ST_SCAN) {
        log_info("stop scan\n");
        bt_ble_scan_enable(0, 0);
    }

    //pair mode
    if (mode == 1) {
        if (conn_pair_info.pair_flag) {
            if (conn_pair_info.pair_flag) {
                //有配对,跳过搜索,直接创建init_creat
                log_info("pair to creat!\n");
                log_info_hexdump(conn_pair_info.peer_address_info, 7);
                bt_ble_create_connection(&conn_pair_info.peer_address_info[1], conn_pair_info.peer_address_info[0]);
                return 0;
            }

        } else {
            log_info("no pair to creat!\n");
            return APP_BLE_OPERATION_ERROR;
        }
    } else {
        log_info("addr to creat!\n");
        log_info_hexdump(addr, 7);
        bt_ble_create_connection(addr, addr_type);
    }
    return 0;
}

static int client_create_cannel_api(void)
{
    if (get_ble_work_state() == BLE_ST_CREATE_CONN) {
        client_create_connection_cannel();
        return 0;
    }
    return 1;
}

static const struct ble_client_operation_t client_operation = {
    .scan_enable = bt_ble_scan_enable,
    .disconnect = ble_disconnect,
    .get_buffer_vaild = get_buffer_vaild_len,
    .write_data = (void *)client_write_without_respond_send,
    .read_do = (void *)client_read_value_send,
    .regist_wakeup_send = client_regiest_wakeup_send,
    .regist_recieve_cbk = client_regiest_recieve_cbk,
    .regist_state_cbk = client_regiest_state_cbk,
    .init_config = client_init_config,
    .opt_comm_send = client_operation_send,
    .set_force_search = client_force_search,
    .create_connect = client_create_connect_api,
    .create_connect_cannel = client_create_cannel_api,
    .get_work_state = get_ble_work_state,
};

struct ble_client_operation_t *ble_get_client_operation_table(void)
{
    return &client_operation;
}

#define PASSKEY_ENTER_ENABLE      0 //输入passkey使能，可修改passkey
//重设passkey回调函数，在这里可以重新设置passkey
//passkey为6个数字组成，十万位、万位。。。。个位 各表示一个数字 高位不够为0
static void reset_passkey_cb(u32 *key)
{
#if 1
    u32 newkey = rand32();//获取随机数

    newkey &= 0xfffff;
    if (newkey > 999999) {
        newkey = newkey - 999999; //不能大于999999
    }
    *key = newkey; //小于或等于六位数
    log_info("set new_key= %06u\n", *key);
#else
    *key = 123456; //for debug
#endif
}

#if 0
//协议栈内部调用
extern void sm_set_master_request_pair(int enable);
static void ble_sm_setup_init(io_capability_t io_type, u8 auth_req, uint8_t min_key_size, u8 security_en)
{
    //setup SM: Display only
    sm_init();
    sm_set_io_capabilities(io_type);
    sm_set_authentication_requirements(auth_req);
    sm_set_encryption_key_size_range(min_key_size, 16);
    sm_set_master_request_pair(security_en);
    sm_event_callback_set(&cbk_sm_packet_handler);

    if (io_type == IO_CAPABILITY_DISPLAY_ONLY) {
        reset_PK_cb_register(reset_passkey_cb);
    }
}
#endif


//协议栈内部调用
void ble_profile_init(void)
{
    log_info("ble profile init\n");
    le_device_db_init();
    ble_stack_gatt_role(1);

//#if PASSKEY_ENTER_ENABLE
//    ble_sm_setup_init(IO_CAPABILITY_DISPLAY_ONLY, SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING, 7, client_config->security_en);
//#else
//    ble_sm_setup_init(IO_CAPABILITY_NO_INPUT_NO_OUTPUT, SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING, 7, client_config->security_en);
//#endif

    /* setup ATT client */
    gatt_client_init();
    gatt_client_register_packet_handler(cbk_packet_handler);

    // register for HCI events
    hci_event_callback_set(&cbk_packet_handler);
    le_l2cap_register_packet_handler(&cbk_packet_handler);

    ble_vendor_set_default_att_mtu(ATT_LOCAL_MTU_SIZE);
}


static void device_bonding_init(void)
{
    int i;
    int cfg_bonding = 0;
    client_match_cfg_t *cfg;
    for (i = 0; i < CLIENT_MATCH_CONN_MAX; i++) {
        cfg = client_config->match_dev_cfg[i];
        if (cfg == NULL) {
            continue;
        }
        if (cfg->bonding_flag) {
            cfg_bonding = 1;
        }
    }
    if (!cfg_bonding) {
        clear_bonding_info();
    }
}

static u8 client_idle_query(void)
{
    return 0;
}

REGISTER_LP_TARGET(client_user_target) = {
    .name = "client_user_demo",
    .is_idle = client_idle_query,
};




void bt_ble_init(void)
{
    log_info("***** ble_init******\n");
#if WIRELESS_TOOL_BLE_NAME_EN
    u8 *config_name;
    u8 config_name_len;
#endif

#if (WIRELESS_24G_ENABLE)
    rf_set_24g_hackable_coded(WIRELESS_24G_CODE_ID);
#endif
#if WIRELESS_TOOL_BLE_NAME_EN
#if 1	//使用配置工具的蓝牙名
    extern const char *bt_get_local_name();
    config_name = (u8 *)(bt_get_local_name());
    config_name_len = strlen(config_name) + 1;
    put_buf(config_name, config_name_len);
    printf("len = %d", config_name_len);
#else
    config_name = TEST_BOX_BLE_NAME;
    config_name_len = sizeof(TEST_BOX_BLE_NAME);
#endif
    match_name = (u8 *)zalloc(config_name_len);
    memcpy(match_name, config_name, config_name_len);
    printf("match_name");
    put_buf(match_name, config_name_len);
    printf("%s", match_name);
#endif
    if (0 == memcmp(match_name, SPECIFIC_STRING, strlen(SPECIFIC_STRING))) {
        match_name = &match_name[strlen(SPECIFIC_STRING)];
        printf("specific scan%s", match_name);
        flag_specific_sacn = 1;

    }

    set_ble_work_state(BLE_ST_INIT_OK);
    conn_pair_vm_do(&conn_pair_info, 0);

#if !WIRELESS_PAIR_BONDING
    device_bonding_init();
#endif
    ble_module_enable(1);
    extern void wifi_detect_set_master_first(u8 first);
#if TCFG_WIFI_DETECT_ENABLE
#if TCFG_WIFI_DETCET_PRIOR
    wifi_detect_set_master_first(0);
#else
    wifi_detect_set_master_first(1);
#endif
#endif

#if (SHOW_RX_DATA_RATE | SHOW_TX_DATA_RATE)
    client_timer_start();
#endif /* SHOW_RX_DATA_RATE | SHOW_TX_DATA_RATE */
}

void bt_ble_exit(void)
{
    log_info("***** ble_exit******\n");
}

void ble_app_disconnect(void)
{
    ble_disconnect(NULL);
}
//统一接口，关闭模块
void bt_ble_adv_enable(u8 enable)
{
    ble_module_enable(enable);
}

//模块开关
void ble_module_enable(u8 en)
{
    log_info("mode_en:%d\n", en);
    if (en) {
        scan_ctrl_en = 1;
        bt_ble_scan_enable(0, 1);
    } else {
        if (con_handle) {
            scan_ctrl_en = 0;
            ble_disconnect(NULL);
        } else {
            bt_ble_scan_enable(0, 0);
            scan_ctrl_en = 0;
        }
    }

}

void client_send_conn_param_update(void)
{
    struct conn_update_param_t *param = (void *)&connection_param_table[send_param_index];//for test
    log_info("client update param:-%d-%d-%d-%d-\n", param->interval_min, param->interval_max, param->latency, param->timeout);
    if (con_handle) {
        ble_op_conn_param_update(con_handle, param);
    }
}



//----------------------------------------------------------------------------------



#endif



