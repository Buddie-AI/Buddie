#include "init.h"
#include "app_config.h"
#include "system/includes.h"
#include "asm/chargestore.h"
#include "user_cfg.h"
#include "app_chargestore.h"
#include "app_power_manage.h"
#include "app_testbox.h"
#include "device/vm.h"
#include "btstack/avctp_user.h"
#include "app_action.h"
#include "app_main.h"
#include "app_charge.h"
#include "classic/tws_api.h"
#include "update.h"
#include "bt_ble.h"
#include "bt_tws.h"
#include "app_action.h"
#include "bt_common.h"
#include "le_rcsp_adv_module.h"
#include "asm/lp_touch_key_api.h"

#define LOG_TAG_CONST       APP_TESTBOX
#define LOG_TAG             "[APP_TESTBOX]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_TEST_BOX_ENABLE

//testbox sub cmd
#define CMD_BOX_BT_NAME_INFO		0x01 //获取蓝牙名
#define CMD_BOX_SDK_VERSION			0x02 //获取sdk版本信息
#define CMD_BOX_BATTERY_VOL			0x03 //获取电量
#define CMD_BOX_ENTER_DUT			0x04 //进入dut模式
#define CMD_BOX_FAST_CONN			0x05 //进入快速链接
#define CMD_BOX_VERIFY_CODE			0x06 //获取校验码
#define CMD_BOX_ENTER_STORAGE_MODE  0x0a //进入仓储模式
#define CMD_BOX_GLOBLE_CFG			0x0b //测试盒配置命令(测试盒收到CMD_BOX_TWS_CHANNEL_SEL命令后发送,需使能测试盒某些配置)
#define CMD_BOX_CUSTOM_CODE			0xf0 //客户自定义命令处理

#define WRITE_LIT_U16(a,src)   {*((u8*)(a)+1) = (u8)(src>>8); *((u8*)(a)+0) = (u8)(src&0xff); }
#define WRITE_LIT_U32(a,src)   {*((u8*)(a)+3) = (u8)((src)>>24);  *((u8*)(a)+2) = (u8)(((src)>>16)&0xff);*((u8*)(a)+1) = (u8)(((src)>>8)&0xff);*((u8*)(a)+0) = (u8)((src)&0xff);}

#define READ_LIT_U32(a)   (*((u8*)(a))  + (*((u8*)(a)+1)<<8) + (*((u8*)(a)+2)<<16) + (*((u8*)(a)+3)<<24))

enum {
    UPDATE_MASK_FLAG_INQUIRY_VBAT_LEVEL = 0,
    UPDATE_MASK_FLAG_INQUIRY_VERIFY_CODE,
    UPDATE_MASK_FLAG_INQUIRY_SDK_VERSION,
};

static const u32 support_update_mask = BIT(UPDATE_MASK_FLAG_INQUIRY_VBAT_LEVEL) | \
                                       BIT(UPDATE_MASK_FLAG_INQUIRY_SDK_VERSION) | \
                                       BIT(UPDATE_MASK_FLAG_INQUIRY_VERIFY_CODE);

struct testbox_info {
    u8 bt_init_ok;//蓝牙协议栈初始化成功
    u8 testbox_status;//测试盒在线状态
    u8 connect_status;//通信成功
    u8 channel;//左右
    u8 event_hdl_flag;
    volatile u32 global_cfg;       //全局配置信息,控制耳机在/离仓行为
    volatile u8 keep_tws_conn_flag; //保持tws连接标志
    u8 tws_paired_flag;
};


enum {
    BOX_CFG_SOFTPWROFF_AFTER_PAIRED_BIT = 0,		//测试盒配对完耳机关机
    BOX_CFG_TOUCH_TRIM_OUT_OF_BOX_BIT = 1,		//离仓执行trim操作
    BOX_CFG_TRIM_START_TIME_BIT = 2,	//离仓多久trim,unit:2s (n+1)*2,即(2~8s)

};

#define BOX_CFG_BITS_GET_FROM_MASK(mask,index,len)  (((mask & BIT(index))>>index) & (0xffffffff>>(32-len)))
static struct testbox_info info = {
    .global_cfg = BIT(BOX_CFG_SOFTPWROFF_AFTER_PAIRED_BIT),
};
#define __this  (&info)

extern const char *bt_get_local_name();
extern u16 get_vbat_value(void);
extern void bt_fast_test_api(void);
extern const int config_btctler_eir_version_info_len;
extern const char *sdk_version_info_get(void);
extern u8 *sdfile_get_burn_code(u8 *len);
extern void bt_page_scan_for_test(u8 inquiry_en);
extern u8 get_jl_chip_id(void);
extern u8 get_jl_chip_id2(void);
extern void set_temp_link_key(u8 *linkkey);
static u8 ex_enter_dut_flag = 0;
static u8 ex_enter_storage_mode_flag = 0;//1 仓储模式, 0 普通模式
static u8 local_packet[36];

void testbox_set_bt_init_ok(u8 flag)
{
    __this->bt_init_ok = flag;
}

void testbox_set_testbox_tws_paired(u8 flag)
{
    __this->tws_paired_flag = flag;
}

u8 testbox_get_testbox_tws_paired(void)
{
    return __this->tws_paired_flag;
}

u8 testbox_get_touch_trim_en(u8 *sec)
{
    if (sec) {
        u8 sec_bits = BOX_CFG_BITS_GET_FROM_MASK(__this->global_cfg, BOX_CFG_TRIM_START_TIME_BIT, 2);
        *sec = (sec_bits + 1) * 2;
    }

    return BOX_CFG_BITS_GET_FROM_MASK(__this->global_cfg, BOX_CFG_TOUCH_TRIM_OUT_OF_BOX_BIT, 1);
}

u8 testbox_get_softpwroff_after_paired(void)
{
    return BOX_CFG_BITS_GET_FROM_MASK(__this->global_cfg, BOX_CFG_SOFTPWROFF_AFTER_PAIRED_BIT, 1);
}

u8 testbox_get_ex_enter_storage_mode_flag(void)
{
    return ex_enter_storage_mode_flag;
}

u8 testbox_get_ex_enter_dut_flag(void)
{
    return ex_enter_dut_flag;
}

u8 testbox_get_connect_status(void)
{
    return __this->connect_status;
}

void testbox_clear_connect_status(void)
{
    __this->connect_status = 0;
}

u8 testbox_get_status(void)
{
    return __this->testbox_status;
}

void testbox_clear_status(void)
{
    __this->testbox_status = 0;
    testbox_clear_connect_status();
}

u8 testbox_get_keep_tws_conn_flag(void)
{
    return __this->keep_tws_conn_flag;
}

void testbox_event_to_user(u8 *packet, u32 type, u8 event, u8 size)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    if (packet != NULL) {
        if (size > sizeof(local_packet)) {
            return;
        }
        e.u.chargestore.packet = local_packet;
        memcpy(e.u.chargestore.packet, packet, size);
    }
    e.arg  = (void *)type;
    e.u.chargestore.event = event;
    e.u.chargestore.size = size;
    sys_event_notify(&e);
}



extern u32 link_task_idle_disable(void);
static void app_testbox_sub_event_handle(u8 *data, u16 size)
{
    u8 mac = 0;
    switch (data[0]) {
    case CMD_BOX_FAST_CONN:
    case CMD_BOX_ENTER_DUT:
        __this->event_hdl_flag = 0;
        struct application *app = get_current_app();
        if (app) {
            if (strcmp(app->name, APP_NAME_BT)) {
                if (!app_var.goto_poweroff_flag) {
                    app_var.play_poweron_tone = 0;
                    task_switch_to_bt();
                }
            } else {
                if ((!__this->connect_status) && __this->bt_init_ok) {
                    log_info("\n\nbt_page_inquiry_scan_for_test\n\n");
                    __this->connect_status = 1;
                    extern void bredr_set_dut_enble(u8 en, u8 phone);
                    log_info("bredr_dut_enbale\n");
                    bredr_set_dut_enble(1, 1);
#if	TCFG_USER_TWS_ENABLE
                    bt_page_scan_for_test(1);
#endif
                    link_task_idle_disable();

                }
            }
        }
        break;
    case CMD_BOX_CUSTOM_CODE:
        __this->event_hdl_flag = 0;
        if (data[1] == 0x00) {
            bt_fast_test_api();
        }
        break;
    }
}

//事件执行函数,在任务调用
int app_testbox_event_handler(struct testbox_event *testbox_dev)
{
    struct application *app = get_current_app();
    switch (testbox_dev->event) {
    case CMD_BOX_MODULE:
        app_testbox_sub_event_handle(testbox_dev->packet, testbox_dev->size);
        break;
    case CMD_BOX_TWS_CHANNEL_SEL:
        __this->event_hdl_flag = 0;
#if	TCFG_USER_TWS_ENABLE
        chargestore_set_tws_channel_info(__this->channel);
#endif
        if (get_vbat_need_shutdown() == TRUE) {
            //电压过低,不进行测试
            break;
        }
        if (app) {
            if (strcmp(app->name, APP_NAME_BT)) {
                if (!app_var.goto_poweroff_flag) {
                    app_var.play_poweron_tone = 0;
                    task_switch_to_bt();
                }
            } else {
                if ((!__this->connect_status) && __this->bt_init_ok) {
                    __this->connect_status = 1;
#if	TCFG_USER_TWS_ENABLE
                    if (0 == __this->keep_tws_conn_flag) {
                        log_info("\n\nbt_page_scan_for_test\n\n");
                        bt_page_scan_for_test(0);
                    }
#endif
                }
            }
        }
        break;
#if TCFG_USER_TWS_ENABLE
    case CMD_BOX_TWS_REMOTE_ADDR:
        log_info("event_CMD_BOX_TWS_REMOTE_ADDR \n");
        chargestore_set_tws_remote_info(testbox_dev->packet, testbox_dev->size);
        break;
#endif
    //不是测试盒事件,返回0,未处理
    default:
        return 0;
    }
    return 1;
}

static const u8 own_private_linkkey[16] = {
    0x06, 0x77, 0x5f, 0x87, 0x91, 0x8d, 0xd4, 0x23,
    0x00, 0x5d, 0xf1, 0xd8, 0xcf, 0x0c, 0x14, 0x2b
};

static void app_testbox_sub_cmd_handle(u8 *send_buf, u8 buf_len, u8 *buf, u8 len)
{
    u8 temp_len;
    u8 send_len = 0;

    send_buf[0] = buf[0];
    send_buf[1] = buf[1];

    log_info("sub_cmd:%x\n", buf[1]);

    switch (buf[1]) {
    case CMD_BOX_BT_NAME_INFO:
        temp_len = strlen(bt_get_local_name());
        if (temp_len < (buf_len - 2)) {
            memcpy(&send_buf[2], bt_get_local_name(), temp_len);
            send_len = temp_len + 2;
            chargestore_api_write(send_buf, send_len);
        } else {
            log_error("bt name buf len err\n");
        }
        break;

    case CMD_BOX_BATTERY_VOL:
        send_buf[2] = get_vbat_value();
        send_buf[3] = get_vbat_value() >> 8;
        send_buf[4] = get_vbat_percent();
        send_len = sizeof(u16) + sizeof(u8) + 2; //vbat_value;u16,vabt_percent:u8,opcode:2 bytes
        log_info("bat_val:%d %d\n", get_vbat_value(), get_vbat_percent());
        chargestore_api_write(send_buf, send_len);
        break;

    case CMD_BOX_SDK_VERSION:
        if (config_btctler_eir_version_info_len) {
            temp_len = strlen(sdk_version_info_get());
            send_len = (temp_len > (buf_len - 2)) ? buf_len : temp_len + 2;
            log_info("version:%s ver_len:%x send_len:%x\n", sdk_version_info_get(), temp_len, send_len);
            memcpy(send_buf + 2, sdk_version_info_get(), temp_len);
            chargestore_api_write(send_buf, send_len);
        }
        break;

    case CMD_BOX_FAST_CONN:
        log_info("enter fast dut\n");
        set_temp_link_key((u8 *)own_private_linkkey);
        bt_get_vm_mac_addr(&send_buf[2]);
        if (0 == __this->event_hdl_flag) {
            testbox_event_to_user(&buf[1], DEVICE_EVENT_TEST_BOX, buf[0], 1);
            __this->event_hdl_flag = 1;
        }
        if (__this->bt_init_ok) {
            ex_enter_dut_flag = 1;
            chargestore_api_write(send_buf, 8);
        }
        break;

    case CMD_BOX_ENTER_DUT:
        log_info("enter dut\n");
        //__this->testbox_status = 1;
        ex_enter_dut_flag = 1;

        if (0 == __this->event_hdl_flag) {
            testbox_event_to_user(&buf[1], DEVICE_EVENT_TEST_BOX, buf[0], 1);
            __this->event_hdl_flag = 1;
        }
        if (__this->bt_init_ok) {
            chargestore_api_write(send_buf, 2);
        }
        break;

    case CMD_BOX_VERIFY_CODE:
        log_info("get_verify_code\n");
        u8 *p = sdfile_get_burn_code(&temp_len);
        send_len = (temp_len > (buf_len - 2)) ? buf_len : temp_len + 2;
        memcpy(send_buf + 2, p, temp_len);
        chargestore_api_write(send_buf, send_len);
        break;

    case CMD_BOX_CUSTOM_CODE:
        log_info("CMD_BOX_CUSTOM_CODE value=%x", buf[2]);
        if (buf[2] == 0) {//测试盒自定义命令，样机进入快速测试模式
            if (0 == __this->event_hdl_flag) {
                __this->event_hdl_flag = 1;
                testbox_event_to_user(&buf[1], DEVICE_EVENT_TEST_BOX, buf[0], 2);
            }
        }
        send_len = 0x3;
#if (defined(TCFG_AUDIO_SPATIAL_EFFECT_ENABLE) && TCFG_AUDIO_SPATIAL_EFFECT_ENABLE)
        if (buf[2] == 1) {
            __this->testbox_status = 1;
            extern int testbox_imu_trim_run(u8 * send_buf);
            send_len =  testbox_imu_trim_run(send_buf);
            put_buf(send_buf, 36);
            send_len += 2;
        }
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

#if TCFG_LP_TOUCH_KEY_ENABLE
        if (buf[2] == 0x6a) {//测试盒自定义命令， 0x6a 用于触摸动态阈值算法的结果显示，请大家不要冲突
            send_buf[2] = 0x6a;
            send_buf[3] = 'c';
            send_buf[4] = 's';
            send_buf[5] = 'm';
            send_buf[6] = 'r';
            send_buf[7] = lp_touch_key_alog_range_display((u8 *)&send_buf[8]);
            send_len = 8 + send_buf[7];
            log_info("send_len = %d\n", send_len);
        }
#endif

        chargestore_api_write(send_buf, send_len);
        break;

    case CMD_BOX_ENTER_STORAGE_MODE:
        log_info("CMD_BOX_ENTER_STORAGE_MODE");
        ex_enter_storage_mode_flag = 1;
        chargestore_api_write(send_buf, 2);
        break;

    case CMD_BOX_GLOBLE_CFG:
        log_info("CMD_BOX_GLOBLE_CFG:%d %x %x", len, READ_LIT_U32(buf + 2), READ_LIT_U32(buf + 6));
        __this->global_cfg = READ_LIT_U32(buf + 2);
#if 0 //for test
        u8 sec;
        u32 trim_en = testbox_get_touch_trim_en(&sec);
        log_info("box_cfg:%x %x %x\n",
                 testbox_get_softpwroff_after_paired(),
                 trim_en, sec);
#endif

        chargestore_api_write(send_buf, 2);
        break;

    default:
        send_buf[0] = CMD_UNDEFINE;
        send_len = 1;
        chargestore_api_write(send_buf, send_len);
        break;
    }

    log_info_hexdump(send_buf, send_len);
}

//数据执行函数,在串口中断调用
static int app_testbox_data_handler(u8 *buf, u8 len)
{
    u8 send_buf[36];
    send_buf[0] = buf[0];
    switch (buf[0]) {
    case CMD_BOX_MODULE:
        app_testbox_sub_cmd_handle(send_buf, sizeof(send_buf), buf, len);
        break;

    case CMD_BOX_UPDATE:
        __this->testbox_status = 1;
        if (buf[13] == get_jl_chip_id() || buf[13] == get_jl_chip_id2()) {
            chargestore_set_update_ram();
            cpu_reset();
        } else if (buf[13] == 0xff) {
            send_buf[1] = 0xff;
            WRITE_LIT_U32(&send_buf[2], support_update_mask);
            chargestore_api_write(send_buf, 2 + sizeof(support_update_mask));
            log_info("rsp update_mask\n");
        } else {
            send_buf[1] = 0x01;//chip id err
            chargestore_api_write(send_buf, 2);
        }
        break;
    case CMD_BOX_TWS_CHANNEL_SEL:
        __this->testbox_status = 1;
        if (len == 3) {
            __this->keep_tws_conn_flag = buf[2];
            putchar('K');
        }  else {
            __this->keep_tws_conn_flag = 0;
        }
        __this->channel = (buf[1] == TWS_CHANNEL_LEFT) ? 'L' : 'R';
        if (0 == __this->event_hdl_flag) {
            testbox_event_to_user(NULL, DEVICE_EVENT_TEST_BOX, CMD_BOX_TWS_CHANNEL_SEL, 0);
            __this->event_hdl_flag = 1;
        }
        if (__this->bt_init_ok) {
            len = chargestore_get_tws_remote_info(&send_buf[1]);
            chargestore_api_write(send_buf, len + 1);
        } else {
            send_buf[0] = CMD_UNDEFINE;
            chargestore_api_write(send_buf, 1);
        }
        break;
#if TCFG_USER_TWS_ENABLE
    case CMD_BOX_TWS_REMOTE_ADDR:
        __this->testbox_status = 1;
        testbox_event_to_user((u8 *)&buf[1], DEVICE_EVENT_TEST_BOX, buf[0], len - 1);
        chargestore_api_set_timeout(100);
        break;
#endif
    //不是测试盒命令,返回0,未处理
    default:
        return 0;
    }
    return 1;
}

CHARGESTORE_HANDLE_REG(testbox, app_testbox_data_handler);

#endif

