/*********************************************************************************************
    *   Filename        : hci_ll.h

    *   Description     : 提供Vendor Host 直接调用Controller API LL Part

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2018-12-04 11:58

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _HCI_LL_H_
#define _HCI_LL_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include "typedef.h"

// LE CONTROLLER COMMANDS
#define HCI_LE_SET_CIG_PARAMS                               0x0062
#define HCI_LE_SETUP_ISO_DATA_PATH                          0x006E
#define HCI_LE_CREATE_BIG                                   0x0068
#define HCI_LE_SET_EXTENDED_ADVERTISING_ENABLE              0x0039

// LE EVENTS
#define HCI_LE_CIS_ESTABLISHED_EVENT                        0x19
#define HCI_LE_CIS_REQUEST_EVENT                            0x1A
#define HCI_LE_CREATE_BIG_COMPLETE_EVENT                    0x1B
#define HCI_SUBEVENT_LE_BIG_INFO_ADV_REPORT_EVT             0x22

enum {
    LL_EVENT_SUPERVISION_TIMEOUT,
    LL_EVENT_RX,
    LL_EVENT_ACL_TX_POST,
};

typedef struct {
    u8 Own_Address_Type: 2;
    u8 Adv_Filter_Policy: 2;
    u8 Scan_Filter_Policy: 2;
    u8 initiator_filter_policy: 2;
} hci_ll_param_t;

/*! \brief      LE Set CIG Parameters */
typedef struct {
    uint8_t         GIG_ID;
    uint8_t         SDU_Interval_C_To_P[3];
    uint8_t         SDU_Interval_P_To_C[3];
    uint8_t         Worst_Case_SCA;
    uint8_t         Packing;
    uint8_t         Framing;
    uint16_t        Max_Transport_Latency_C_To_P;
    uint16_t        Max_Transport_Latency_P_To_C;
    uint8_t         CIS_Count;
    struct le_cis_param_t {
        uint8_t         CIS_ID;
        uint16_t        Max_SDU_C_To_P;
        uint16_t        Max_SDU_P_To_C;
        uint8_t         PHY_C_To_P;
        uint8_t         PHY_P_To_C;
        uint8_t         RTN_C_To_P;
        uint8_t         RTN_P_To_C;
    } _GNU_PACKED_ param[0];
} _GNU_PACKED_ le_set_cig_param_t;

/*! \brief      LE Create CIS */
typedef struct {
    uint8_t         CIS_Count;
    struct le_cis_hdl_t {
        uint16_t        CIS_Connection_Handle;
        uint16_t        ACL_Connection_Handle;
    } _GNU_PACKED_ param[0];
} _GNU_PACKED_ le_create_cis_t;

/*! \brief      LE Setup ISO Data Path */
typedef struct {
    uint16_t        Connection_Handle;
    uint8_t         Data_Path_Direction;
    uint8_t         Data_Path_ID;
    struct {
        uint8_t         Coding_Format;
        uint16_t        Company_Identifier;
        uint16_t        Vendor_ID;
    } _GNU_PACKED_ Codec_ID;
    uint8_t         Controller_Delay[3];
    uint8_t         Codec_Configuratin_Length;
    uint8_t         Codec_Configuratin[0];
} _GNU_PACKED_ le_setup_iso_data_path_t;

/*! \brief      LE Create BIG */
typedef struct {
    uint8_t         BIG_Handle;
    uint8_t         Advertising_Handle;
    uint8_t         Num_BIS;
    uint8_t         SDU_Interval[3];
    uint16_t        Max_SDU;
    uint16_t        Max_Transport_Latency;
    uint8_t         RTN;
    uint8_t         PHY;
    uint8_t         Packing;
    uint8_t         Framing;
    uint8_t         Encryption;
    uint8_t         Broadcast_Code[16];
} _GNU_PACKED_ le_create_big_t;

/*! \brief      LE BIG Create Sync */
typedef struct {
    uint8_t         BIG_Handle;
    uint16_t        Sync_Handle;
    uint8_t         Encryption;
    uint8_t         Broadcast_Code[16];
    uint8_t         MSE;
    uint16_t        BIG_Sync_Timeout;
    uint8_t         Num_BIS;
    uint8_t         BIS[0];
} _GNU_PACKED_ le_big_create_sync_t;

/*! \brief      HCI ISO Data packets */
typedef struct {
    uint32_t        Connection_Handle    : 12;
    uint32_t        PB_Flag              : 2;
    uint32_t        TS_Flag              : 1;
    uint32_t        RFU                  : 1;
    uint32_t        ISO_Data_Load_Length : 14;
    uint32_t        RFU2                 : 2;

    uint32_t        Time_Stamp;

    uint32_t        Packet_Sequence_Num : 16;
    uint32_t        ISO_SDU_Length      : 12;
    uint32_t        RFU3                : 2;
    uint32_t        Packet_Status_Flag  : 2;

    uint8_t         ISO_SDU_Fragment[0];
} _GNU_PACKED_ hci_iso_data_packets_t ;

typedef struct {
    u32 handle  : 12;
    //0b00 frist fragment of a fragmented SDU
    //0b01 a continuation fragment of a fragmented SDU
    //0b10 a complete SDU
    //0b11 the last fragment of an SDU
    u32 pb_flag : 2;
    u32 ts_flag : 1;
    u32 rfu     : 1;
    u32 iso_data_load_length : 14;
    u32 rfu2    : 2;

    u32 time_stamp;

    u32 packet_sequence_num : 16;
    u32 iso_sdu_length      : 12;
    u32 rfu3                : 2;
    //0b00 Valid data. The complete ISO_SDU was received correctly.
    //0b01 Possibly invalid data. The contents of the ISO_SDU may contain errors or
    //  part of the ISO_SDU may be missing. This is reported as "data with possible
    //  errors".
    //0b10 Part(s) of the ISO_SDU were not received correctly. This is reported as
    //  "lost data".
    u32 packet_status_flag  : 2;

    uint8_t *iso_sdu;
} hci_iso_hdr_t ;

/*! \brief      LE BIGInfo Advertising report event */
typedef struct {
    uint8_t         Subevent_Code;
    uint16_t        Sync_Handle;
    uint8_t         Num_BIS;
    uint8_t         NSE;
    uint16_t        ISO_Interval;
    uint8_t         BN;
    uint8_t         PTO;
    uint8_t         IRC;
    uint16_t        Max_PDU;
    uint8_t         SDU_Interval[3];
    uint16_t        Max_SDU;
    uint8_t         PHY;
    uint8_t         Framing;
    uint8_t         Encryption;
} _GNU_PACKED_ le_biginfo_adv_report_evt_t;

//Adjust Host part API
void ll_hci_init(void);

void ll_hci_reset(void);

void ll_hci_destory(void);

void ll_hci_set_event_mask(const u8 *mask);

void ll_hci_set_name(const char *name);

void ll_hci_adv_set_params(uint16_t adv_int_min, uint16_t adv_int_max, uint8_t adv_type,
                           uint8_t direct_address_type, uint8_t *direct_address,
                           uint8_t channel_map, uint8_t filter_policy);

void ll_hci_adv_set_data(uint8_t advertising_data_length, uint8_t *advertising_data);

void ll_hci_adv_scan_response_set_data(uint8_t scan_response_data_length, uint8_t *scan_response_data);

int ll_hci_adv_enable(bool enable);

void ll_hci_scan_set_params(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window);

int ll_hci_scan_enable(bool enable, u8 filter_duplicates);

int ll_hci_create_conn(u8 *conn_param, u8 *addr_param);
int ll_hci_create_conn_ext(void *param);

int ll_hci_create_conn_cancel(void);

int ll_hci_vendor_send_key_num(u16 con_handle, u8 num);

int ll_vendor_latency_hold_cnt(u16 conn_handle, u16 hold_cnt);

int ll_hci_encryption(u8 *key, u8 *plaintext_data);

int ll_hci_get_le_rand(void);


int ll_hci_start_encryption(u16 handle, u32 rand_low, u32 rand_high, u16 peer_ediv, u8 *ltk);

int ll_hci_long_term_key_request_reply(u16 handle, u8 *ltk);

int ll_hci_long_term_key_request_nagative_reply(u16 handle);

int ll_hci_connection_update(u16 handle, u16 conn_interval_min, u16 conn_interval_max,
                             u16 conn_latency, u16 supervision_timeout,
                             u16 minimum_ce_length, u16 maximum_ce_length);

u16 ll_hci_get_acl_data_len(void);

u16 ll_hci_get_acl_total_num(void);

void ll_hci_set_random_address(u8 *addr);

int ll_hci_disconnect(u16 handle, u8 reason);

int ll_hci_read_local_p256_pb_key(void);

int ll_hci_generate_dhkey(const u8 *data, u32 size);

//Adjust Controller part API
void ll_hci_cmd_handler(int *cmd);

void ll_event_handler(int *msg);

void ll_hci_private_free_dma_rx(u8 *rx_head);

void ll_hci_set_data_length(u16 conn_handle, u16 tx_octets, u16 tx_time);

hci_ll_param_t *ll_hci_param_config_get(void);
void hci_ll_get_device_address(uint8_t *addr_type, u8 *addr);
void ll_hci_set_host_channel_classification(u8 *channel_map);

// ble5
void ll_hci_set_ext_adv_params(u8 *data, u32 size);
void ll_hci_set_ext_adv_data(u8 *data, u32 size);
void ll_hci_set_ext_adv_enable(u8 *data, u32 size);
void ll_hci_set_phy(u16 conn_handle, u8 all_phys, u8 tx_phy, u8 rx_phy, u16 phy_options);
void ll_hci_set_ext_scan_params(u8 *data, u32 size);
void ll_hci_set_ext_scan_enable(u8 *data, u32 size);
void ll_hci_ext_create_conn(u8 *data, u32 size);
void ll_hci_set_periodic_adv_params(u8 *data, u32 size);
void ll_hci_set_periodic_adv_data(u8 *data, u32 size);
void ll_hci_set_periodic_adv_enable(u8 *data, u32 size);
void ll_hci_periodic_adv_creat_sync(u8 *data, u32 size);
void ll_hci_periodic_adv_terminate_sync(u8 *data, u32 size);
void ll_hci_periodic_adv_create_sync_cancel(void);

int le_controller_set_mac(void *addr);

#endif
