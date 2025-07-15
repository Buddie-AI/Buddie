// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#ifndef _BT_COMMON_H
#define _BT_COMMON_H
#include <stdint.h>
#include "app_config.h"

#define UART_FIFIO_BUF_LEN 1024

#ifdef APP_PRIVATE_PROFILE_CFG
#include "lib_profile_cfg.h"
#else
#include "bt_profile_cfg.h"
#endif

#define SYNC_TIMEOUT_MS(x)              {(x / 10) & 0xff, (x / 10) >> 8}

#define ALIGN_2BYTE(size)                   (((size)+1)&0xfffffffe)
#define BYTE_LEN(x...)                      sizeof((u8 []) {x})
#define MS_TO_SLOT(x)                       (x * 8 / 5)
#define UINT24_TO_BYTE(x) \
{ \
    (x),      \
    (x) >> 8, \
    (x) >> 16 \
}

enum {
    ST_BIT_INQUIRY = 0,
    ST_BIT_PAGE_SCAN,
    ST_BIT_BLE_ADV,
    ST_BIT_SPP_CONN,
    ST_BIT_BLE_CONN,
    ST_BIT_WEIXIN_CONN,
};

enum {
    BLE_PRIV_MSG_PAIR_CONFIRM = 0xF0,
    BLE_PRIV_PAIR_ENCRYPTION_CHANGE,
};//ble_state_e


// #define SYS_BT_EVENT_FORM_COMMON       (('C' << 24) | ('M' << 16) | ('M' << 8) | '\0')
enum {
    COMMON_EVENT_EDR_REMOTE_TYPE = 1,
    COMMON_EVENT_BLE_REMOTE_TYPE,
    COMMON_EVENT_SHUTDOWN_ENABLE,
    COMMON_EVENT_SHUTDOWN_DISABLE,
    COMMON_EVENT_MODE_DETECT,
};

// LE Controller Commands
struct __ext_adv_report_event {
    u8  Subevent_Code;
    u8  Num_Reports;
    u16 Event_Type;
    u8  Address_Type;
    u8  Address[6];
    u8  Primary_PHY;
    u8  Secondary_PHY;
    u8  Advertising_SID;
    u8  Tx_Power;
    u8  RSSI;
    u16 Periodic_Advertising_Interval;
    u8  Direct_Address_Type;
    u8  Direct_Address[6];
    u8  Data_Length;
    u8  Data[0];
} _GNU_PACKED_;

struct __periodic_adv_report_event {
    u8  Subevent_Code;
    u16 Sync_Handle;
    u8  Tx_Power;
    u8  RSSI;
    u8  Unused;
    u8  Data_Status;
    u8  Data_Length;
    u8  Data[0];
} _GNU_PACKED_ ;

struct __periodic_creat_sync {
    u8 Filter_Policy;
    u8 Advertising_SID;
    u8 Advertising_Address_Type;
    u8 Advertiser_Address[6];
    u8 Skip[2];
    u8 Sync_Timeout[2];
    u8 Unused;
} _GNU_PACKED_;

struct __ext_scan_param {
    u8 Own_Address_Type;
    u8 Scanning_Filter_Policy;
    u8 Scanning_PHYs;
    u8  Scan_Type;
    u16 Scan_Interval;
    u16 Scan_Window;
} _GNU_PACKED_;

struct __ext_scan_enable {
    u8  Enable;
    u8  Filter_Duplicates;
    u16 Duration;
    u16 Period;
} _GNU_PACKED_;

struct ext_advertising_param {
    u8 Advertising_Handle;
    u16 Advertising_Event_Properties;
    u8 Primary_Advertising_Interval_Min[3];
    u8 Primary_Advertising_Interval_Max[3];
    u8 Primary_Advertising_Channel_Map;
    u8 Own_Address_Type;
    u8 Peer_Address_Type;
    u8 Peer_Address[6];
    u8 Advertising_Filter_Policy;
    u8 Advertising_Tx_Power;
    u8 Primary_Advertising_PHY;
    u8 Secondary_Advertising_Max_Skip;
    u8 Secondary_Advertising_PHY;
    u8 Advertising_SID;
    u8 Scan_Request_Notification_Enable;
} _GNU_PACKED_;

struct ext_advertising_data  {
    u8 Advertising_Handle;
    u8 Operation;
    u8 Fragment_Preference;
    u8 Advertising_Data_Length;
    u8 Advertising_Data[31];
} _GNU_PACKED_;

struct ext_advertising_enable {
    u8  Enable;
    u8  Number_of_Sets;
    u8  Advertising_Handle;
    u16 Duration;
    u8  Max_Extended_Advertising_Events;
} _GNU_PACKED_;

struct periodic_advertising_param {
    u8 Advertising_Handle;
    u16 Periodic_Advertising_Interval_Min;
    u16 Periodic_Advertising_Interval_Max;
    u16 Periodic_Advertising_Properties;
} _GNU_PACKED_;

struct periodic_advertising_data  {
    u8 Advertising_Handle;
    u8 Operation;
    u8 Advertising_Data_Length;
    u8 Advertising_Data[31];
} _GNU_PACKED_;

struct periodic_advertising_enable {
    u8  Enable;
    u8  Advertising_Handle;
} _GNU_PACKED_;

typedef struct {
    uint16_t Sync_Handle;
} _GNU_PACKED_ periodic_adv_terminate_sync;

extern const int config_le_hci_connection_num;//支持同时连接个数
extern const int config_le_sm_support_enable; //是否支持加密配对
extern const int config_le_gatt_server_num;   //支持server角色个数
extern const int config_le_gatt_client_num;   //支持client角色个数

extern uint16_t little_endian_read_16(const uint8_t *buffer, int pos);
extern uint32_t little_endian_read_24(const uint8_t *buffer, int pos);
extern uint32_t little_endian_read_32(const uint8_t *buffer, int pos);
extern void swapX(const uint8_t *src, uint8_t *dst, int len);

extern void little_endian_store_16(uint8_t *buffer, uint16_t pos, uint16_t value);
extern void little_endian_store_32(uint8_t *buffer, uint16_t pos, uint32_t value);
extern void big_endian_store_16(uint8_t *buffer, uint16_t pos, uint16_t value);
extern void big_endian_store_32(uint8_t *buffer, uint16_t pos, uint32_t value);

//common api
extern void bt_ble_init(void);
extern void bt_ble_exit(void);
extern void bt_ble_adv_enable(u8 enable);
extern void ble_app_disconnect(void);
extern void ble_module_enable(u8 en);
extern const u8 *bt_get_mac_addr();
extern void lib_make_ble_address(u8 *ble_address, u8 *edr_address);
extern void user_spp_data_handler(u8 packet_type, u16 ch, u8 *packet, u16 size);
extern void bt_get_vm_mac_addr(u8 *addr);
extern void set_remote_test_flag(u8 own_remote_test);
extern void bredr_power_get(void);
extern void bredr_power_put(void);
extern void radio_set_eninv(int v);
extern void transport_spp_init(void);
extern const char *bt_get_local_name();
extern void wdt_clear(void);
extern u8 ble_update_get_ready_jump_flag(void);
extern void reset_PK_cb_register(void (*reset_pk)(u32 *));
extern void reset_PK_cb_register_ext(void (*reset_pk)(u32 *, u16));
extern void att_server_flow_enable(u8 enable);
extern void le_device_db_init(void);
extern bool get_remote_test_flag();
extern void transport_spp_disconnect(void);
extern int transport_spp_flow_enable(u8 en);
extern void transport_spp_flow_cfg(void);

void att_set_conn_handle(u16 conn_handle, u8 handle_type);
void at_send_rx_cid_data(u8 cid, u8 *packet, u16 size);
void at_send_conn_result(u8 cid, u8 is_sucess);
void at_send_disconnect(u8 cid);
void at_send_connected(u8 cid);
void at_send_string(u8 *str);
void at_respond_send_err(u32 err_id);

int le_att_server_send_data(u8 cid, u8 *packet, u16 size);
int le_att_server_send_ctrl(u8 cid, u8 *packet, u16 size);
int le_att_client_send_data(u8 cid, u8 *packet, u16 size);
int le_at_client_creat_connection(u8 *conn_addr, u8 addr_type);
void black_list_check(u8 sta, u8 *peer_addr);


//errid
enum {
    ERR_AT_CMD = 1,
    //add here
};

//初始化配置蓝牙发射功率最大值范围,解析详见 btcontroller_modules.h
void bt_max_pwr_set(u8 pwr, u8 pg_pwr, u8 iq_pwr, u8 ble_pwr);

#endif
