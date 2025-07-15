#ifndef _APP_CHARGESTORE_H_
#define _APP_CHARGESTORE_H_

#include "typedef.h"
#include "system/event.h"

//LDOIN升级口命令定义
#define CMD_TWS_CHANNEL_SET         0x01
#define CMD_TWS_REMOTE_ADDR         0x02
#define CMD_TWS_ADDR_DELETE         0x03
#define CMD_BOX_TWS_CHANNEL_SEL     0x04//测试盒获取地址
#define CMD_BOX_TWS_REMOTE_ADDR     0x05//测试盒交换地址
#define CMD_POWER_LEVEL_OPEN        0x06//开盖充电舱报告/获取电量
#define CMD_POWER_LEVEL_CLOSE       0x07//合盖充电舱报告/获取电量
#define CMD_RESTORE_SYS             0x08//恢复出厂设置
#define CMD_ENTER_DUT               0x09//进入测试模式
#define CMD_EX_FIRST_READ_INFO      0x0A//F95读取数据首包信息
#define CMD_EX_CONTINUE_READ_INFO   0x0B//F95读取数据后续包信息
#define CMD_EX_FIRST_WRITE_INFO     0x0C//F95写入数据首包信息
#define CMD_EX_CONTINUE_WRITE_INFO  0x0D//F95写入数据后续包信息
#define CMD_EX_INFO_COMPLETE        0x0E//F95完成信息交换
#define CMD_TWS_SET_CHANNEL         0x0F//F95设置左右声道信息
#define CMD_BOX_UPDATE              0x20//测试盒升级
#define CMD_BOX_MODULE              0x21//测试盒一级命令

#define CMD_SHUT_DOWN               0x80//充电舱关机,充满电关机,或者是低电关机
#define CMD_CLOSE_CID               0x81//充电舱盒盖
#define CMD_ANC_MODULE              0x90//ANC一级命令
#define CMD_FAIL                    0xfe//失败
#define CMD_UNDEFINE                0xff//未知命令回复

enum {
    TWS_CHANNEL_LEFT = 1, //左耳
    TWS_CHANNEL_RIGHT, //右耳
};

enum {
    TWS_DEL_TWS_ADDR = 1, //删除对箱地址
    TWS_DEL_PHONE_ADDR,//删除手机地址
    TWS_DEL_ALL_ADDR,//删除手机与对箱地址
};

struct _CHARGE_STORE_INFO {
    u8 tws_local_addr[6];
    u8 tws_remote_addr[6];
    u8 tws_mac_addr[6];
    u32 search_aa;
    u32 pair_aa;
    u8 local_channel;
    u16 device_ind;
    u16 reserved_data;
} _GNU_PACKED_;
typedef struct _CHARGE_STORE_INFO   CHARGE_STORE_INFO;

extern void chargestore_set_tws_channel_info(u8 channel);
extern bool chargestore_set_tws_remote_info(u8 *data, u8 len);
extern u16 chargestore_get_tws_remote_info(u8 *data);
extern u8 chargestore_get_power_level(void);
extern u8 chargestore_get_power_status(void);
extern u8 chargestore_get_cover_status(void);
extern u8 chargestore_get_sibling_power_level(void);
extern void chargestore_set_bt_init_ok(u8 flag);
extern int app_chargestore_event_handler(struct chargestore_event *chargestore_dev);
extern u8 chargestore_get_earphone_online(void);
extern u8 chargestore_get_earphone_pos(void);
extern int chargestore_sync_chg_level(void);
extern void chargestore_set_power_level(u8 power);
extern void chargestore_set_sibling_chg_lev(u8 chg_lev);
extern void chargestore_set_phone_disconnect(void);
extern void chargestore_set_phone_connect(void);
extern u8 chargestore_check_going_to_poweroff(void);
extern void chargestore_shutdown_reset(void);
extern void testbox_set_testbox_tws_paired(u8 flag);
extern u8 testbox_get_testbox_tws_paired(void);
extern u8 testbox_get_touch_trim_en(u8 *sec);
extern u8 testbox_get_softpwroff_after_paired(void);

#endif    //_APP_CHARGESTORE_H_
