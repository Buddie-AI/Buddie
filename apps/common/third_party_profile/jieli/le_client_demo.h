// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)
#ifndef _LE_CLIENT_DEMO_H
#define _LE_CLIENT_DEMO_H

#include <stdint.h>
#include "btstack/le/att.h"

//搜索匹配连接方式
typedef enum {
    CLI_CREAT_BY_ADDRESS = 0,//指定地址创建连接
    CLI_CREAT_BY_NAME,//指定设备名称创建连接
    CLI_CREAT_BY_TAG,//匹配厂家标识创建连接
} cli_creat_mode_e;

//主机事件
typedef enum {
    CLI_EVENT_MATCH_DEV = 1,//搜索到匹配的设备
    CLI_EVENT_CONNECTED, //设备连接成功
    CLI_EVENT_DISCONNECT,//设备连接断开
    CLI_EVENT_MATCH_UUID,//搜索到匹配的UUID
    CLI_EVENT_SEARCH_PROFILE_COMPLETE, //搜索profile服务结束
    CLI_EVENT_CONNECTION_UPDATE,//设备连接参数更新成功
} le_client_event_e;


typedef struct {
    //服务16bit uuid,非0 代表uuid16格式，0--代表是uuid128格式,services_uuid128
    u16 services_uuid16;
    //服务16bit uuid,非0 代表uuid16格式，0--代表是uuid128格式,characteristic_uuid128
    u16 characteristic_uuid16;
    u8  services_uuid128[16];
    u8  characteristic_uuid128[16];
    u16 opt_type; //属性
    u8 read_long_enable: 1; //en
    u8 read_report_reference: 1; //en
    u8 res_bits: 6; //
} target_uuid_t;

//搜索操作记录的 handle
#define OPT_HANDLE_MAX   16
typedef struct {
    //匹配的UUID
    target_uuid_t *search_uuid;
    //可操作的handle
    u16 value_handle;
} opt_handle_t;

//最大匹配的设备个数
#define CLIENT_MATCH_CONN_MAX    3

typedef struct {
    u8 create_conn_mode;   //cli_creat_mode_e
    u8 bonding_flag;       //连接过后会绑定，默认快连，不搜索设备
    u8 compare_data_len;   //匹配信息长度
    const u8 *compare_data;//匹配信息，若是地址内容,由高到低位
    u8 filter_pdu_bitmap;     /*过滤指定的pdu包,不做匹配操作; bit map,event type*/
} client_match_cfg_t;


typedef struct {
    //搜索匹配信息
    const client_match_cfg_t *match_dev_cfg[CLIENT_MATCH_CONN_MAX];
    //加密保定配置 0 or 1
    u8 security_en;
    //搜索服务的个数
    u8 search_uuid_cnt; // <= OPT_HANDLE_MAX
    //搜索服务
    const target_uuid_t *search_uuid_table;
    //回调处理接收到的 notify or indicate 数据
    void (*report_data_callback)(att_data_report_t *data_report, target_uuid_t *search_uuid);
    //主机一些事件回调处理
    void (*event_callback)(le_client_event_e event, u8 *packet, int size);
} client_conn_cfg_t;

//清除绑定配对信息
void client_clear_bonding_info(void);
void client_send_conn_param_update(void);
void ble_module_enable(u8 en);
struct ble_client_operation_t *ble_get_client_operation_table(void);
void bt_ble_init(void);
void bt_ble_adv_enable(u8 enable);

#endif
