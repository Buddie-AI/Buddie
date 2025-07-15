#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#include "typedef.h"


//LE
#include "le/ble_data_types.h"
#include "le/ble_api.h"
#include "le/le_user.h"
#include "le/att.h"
#include "le/gatt.h"
#include "le/sm.h"

#define ISO_SDU_DTAT_LENGTH 0x032c       //812byte
#define ISO_PDU_DTAT_LENGTH 251          //251byte
#define ISO_PDU_INTERVAL_M_S 0x4e20          //20ms
#define ISO_PDU_INTERVAL_S_M 0x4e20          //20ms
#define HCI_ISO_DATA_PACKET         0x05

//Classic




//Common
#include "btstack_event.h"


#define HCI_COMMAND_DATA_PACKET				0x01
#define HCI_ACL_DATA_PACKET	    			0x02
#define HCI_SCO_DATA_PACKET	    			0x03
#define HCI_EVENT_PACKET	    			0x04

// OGFs
#define OGF_LINK_CONTROL            0x01
#define OGF_LINK_POLICY             0x02
#define OGF_CONTROLLER_BASEBAND     0x03
#define OGF_INFORMATIONAL_PARAMETERS 0x04
#define OGF_STATUS_PARAMETERS       0x05
#define OGF_TESTING                 0x06
#define OGF_LE_CONTROLLER           0x08
#define OGF_VENDOR_LE_CONTROLLER	0x3e
#define OGF_VENDOR                  0x3f


// Events from host controller to host

/**
 * @format 1
 * @param status
 */
#define HCI_EVENT_INQUIRY_COMPLETE                         0x01

/**
 * @format 1B11132
 * @param num_responses
 * @param bd_addr
 * @param page_scan_repetition_mode
 * @param reserved1
 * @param reserved2
 * @param class_of_device
 * @param clock_offset
 */
#define HCI_EVENT_INQUIRY_RESULT                           0x02

/**
 * @format 12B11
 * @param status
 * @param connection_handle
 * @param bd_addr
 * @param link_type
 * @param encryption_enabled
 */
#define HCI_EVENT_CONNECTION_COMPLETE                      0x03
/**
 * @format B31
 * @param bd_addr
 * @param class_of_device
 * @param link_type
 */
#define HCI_EVENT_CONNECTION_REQUEST                       0x04
/**
 * @format 121
 * @param status
 * @param connection_handle
 * @param reason
 */
#define HCI_EVENT_DISCONNECTION_COMPLETE                   0x05
/**
 * @format 12
 * @param status
 * @param connection_handle
 */
#define HCI_EVENT_AUTHENTICATION_COMPLETE                   0x06
/**
 * @format 1BN
 * @param status
 * @param bd_addr
 * @param remote_name
 */
#define HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE             0x07
/**
 * @format 121
 * @param status
 * @param connection_handle
 * @param encryption_enabled
 */
#define HCI_EVENT_ENCRYPTION_CHANGE                        0x08
/**
 * @format 12
 * @param status
 * @param connection_handle
 */
#define HCI_EVENT_CHANGE_CONNECTION_LINK_KEY_COMPLETE      0x09
/**
 * @format 121
 * @param status
 * @param connection_handle
 * @param key_flag
 */
#define HCI_EVENT_MASTER_LINK_KEY_COMPLETE                 0x0A

#define HCI_EVENT_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE  0x0B

#define HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE 0x0C

#define HCI_EVENT_QOS_SETUP_COMPLETE                       0x0D

/**
 * @format 12R
 * @param num_hci_command_packets
 * @param command_opcode
 * @param return_parameters
 */
#define HCI_EVENT_COMMAND_COMPLETE                         0x0E
/**
 * @format 112
 * @param status
 * @param num_hci_command_packets
 * @param command_opcode
 */
#define HCI_EVENT_COMMAND_STATUS                           0x0F

/**
 * @format 1
 * @param hardware_code
 */
#define HCI_EVENT_HARDWARE_ERROR                           0x10

#define HCI_EVENT_FLUSH_OCCURRED                           0x11

/**
 * @format 1B1
 * @param status
 * @param bd_addr
 * @param role
 */
#define HCI_EVENT_ROLE_CHANGE                              0x12

// TODO: number_of_handles 1, connection_handle[H*i], hc_num_of_completed_packets[2*i]
#define HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS              0x13

/**
 * @format 1H12
 * @param status
 * @param handle
 * @param mode
 * @param interval
 */
#define HCI_EVENT_MODE_CHANGE_EVENT                        0x14

// TODO: num_keys, bd_addr[B*i], link_key[16 octets * i]
#define HCI_EVENT_RETURN_LINK_KEYS                         0x15

/**
 * @format B
 * @param bd_addr
 */
#define HCI_EVENT_PIN_CODE_REQUEST                         0x16

/**
 * @format B
 * @param bd_addr
 */
#define HCI_EVENT_LINK_KEY_REQUEST                         0x17

// TODO: bd_addr B, link_key 16octets, key_type 1
#define HCI_EVENT_LINK_KEY_NOTIFICATION                    0x18

/**
 * @format 1
 * @param link_type
 */
#define HCI_EVENT_DATA_BUFFER_OVERFLOW                     0x1A

/**
 * @format H1
 * @param handle
 * @param lmp_max_slots
 */
#define HCI_EVENT_MAX_SLOTS_CHANGED                        0x1B

/**
 * @format 1H2
 * @param status
 * @param handle
 * @param clock_offset
 */
#define HCI_EVENT_READ_CLOCK_OFFSET_COMPLETE               0x1C

/**
 * @format 1H2
 * @param status
 * @param handle
 * @param packet_types
 * @pnote packet_type is in plural to avoid clash with Java binding Packet.getPacketType()
 */
#define HCI_EVENT_CONNECTION_PACKET_TYPE_CHANGED           0x1D

#define HCI_EVENT_PAGE_SCAN_REPETITION_MODE_CHANGE         0x20
/**
 * @format 1B11321
 * @param num_responses
 * @param bd_addr
 * @param page_scan_repetition_mode
 * @param reserved
 * @param class_of_device
 * @param clock_offset
 * @param rssi
 */
#define HCI_EVENT_INQUIRY_RESULT_WITH_RSSI                 0x22

#define HCI_EVENT_READ_REMOTE_EXTERNED_FEATURES_COMPLETE   0x23
/**
 * @format 1HB111221
 * @param status
 * @param handle
 * @param bd_addr
 * @param link_type
 * @param transmission_interval
 * @param retransmission_interval
 * @param rx_packet_length
 * @param tx_packet_length
 * @param air_mode
 */
#define HCI_EVENT_SYNCHRONOUS_CONNECTION_COMPLETE          0x2C

// TODO: serialize extended_inquiry_response and provide parser
/**
 * @format 1B11321
 * @param num_responses
 * @param bd_addr
 * @param page_scan_repetition_mode
 * @param reserved
 * @param class_of_device
 * @param clock_offset
 * @param rssi
 */
#define HCI_EVENT_EXTENDED_INQUIRY_RESPONSE                0x2F
#define HCI_EVENT_EXTENDED_INQUIRY_RESULT                  0x2F

/**
 * @format 1H
 * @param status
 * @param handle
 */
#define HCI_EVENT_ENCRYPTION_KEY_REFRESH_COMPLETE          0x30

#define HCI_EVENT_IO_CAPABILITY_REQUEST                    0x31
#define HCI_EVENT_IO_CAPABILITY_RESPONSE                   0x32

/**
 * @format B4
 * @param bd_addr
 * @param numeric_value
 */
#define HCI_EVENT_USER_CONFIRMATION_REQUEST                0x33

/**
 * @format B
 * @param bd_addr
 */
#define HCI_EVENT_USER_PASSKEY_REQUEST                     0x34

/**
 * @format B
 * @param bd_addr
 */
#define HCI_EVENT_REMOTE_OOB_DATA_REQUEST                  0x35

/**
 * @format 1B
 * @param status
 * @param bd_addr
 */
#define HCI_EVENT_SIMPLE_PAIRING_COMPLETE                  0x36
#define HCI_EVENT_LINK_SUPPERVISION_TIMEOUT_CHANGE_EVENT   0x38

#define HCI_EVENT_USER_PRESSKEY_NOTIFICATION			   0x3B
#define HCI_EVENT_REMOTE_KEYPRESS_NOTIFICATION			   0x3C
#define HCI_EVENT_REMOTE_SUPPORTED_FEATURES_NOTIFICATION   0x3D
#define HCI_EVENT_LE_META                                  0x3E

// last used HCI_EVENT in 2.1 is 0x3d
// last used HCI_EVENT in 4.1 is 0x57

#define HCI_EVENT_VENDOR_CONNECTION_COMPLETE               0xEF

//event definition for new vendor sub event
#define HCI_EVENT_VENDOR_META							   0xF5
#define HCI_SUBEVENT_VENDOR_TEST_MODE_CFG				   0x01

#define HCI_EVENT_VENDOR_FRE_OFFSET_TRIM                   0xF6
#define HCI_EVENT_VENDOR_ENCRY_COMPLETE                    0xF7
#define HCI_EVENT_VENDOR_NO_RECONN_ADDR                    0xF8
#define HCI_EVENT_VENDOR_SETUP_COMPLETE                    0xF9
#define HCI_EVENT_VENDOR_DUT                               0xFA
#define HCI_EVENT_VENDOR_OSC_INTERNAL                      0xFB
#define HCI_EVENT_VENDOR_FAST_TEST                         0xFC
#define HCI_EVENT_VENDOR_REMOTE_UPDATE                     0xFD
#define HCI_EVENT_VENDOR_REMOTE_TEST                       0xFE
#define HCI_EVENT_VENDOR_SPECIFIC                          0xFF

#define BTSTACK_EVENT_HCI_CONNECTIONS_DELETE                  0x6D

/**
 * @format 11H11B2221
 * @param subevent_code
 * @param status
 * @param connection_handle
 * @param role
 * @param peer_address_type
 * @param peer_address
 * @param conn_interval
 * @param conn_latency
 * @param supervision_timeout
 * @param master_clock_accuracy
 */
#define HCI_SUBEVENT_LE_CONNECTION_COMPLETE                0x01

// array of advertisements, not handled by event accessor generator
#define HCI_SUBEVENT_LE_ADVERTISING_REPORT                 0x02

/**
 * @format 11H222
 * @param subevent_code
 * @param status
 * @param connection_handle
 * @param conn_interval
 * @param conn_latency
 * @param supervision_timeout
 */
#define HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE         0x03

/**
 * @format 1HD2
 * @param subevent_code
 * @param connection_handle
 * @param random_number
 * @param encryption_diversifier
 */
#define HCI_SUBEVENT_LE_READ_REMOTE_USED_FEATURES_COMPLETE 0x04

/**
 * @format 1HD2
 * @param subevent_code
 * @param connection_handle
 * @param random_number
 * @param encryption_diversifier
 */
#define HCI_SUBEVENT_LE_LONG_TERM_KEY_REQUEST              0x05

/**
 * @format 1H2222
 * @param subevent_code
 * @param connection_handle
 * @param interval_min
 * @param interval_max
 * @param latency
 * @param timeout
 */
#define HCI_SUBEVENT_LE_REMOTE_CONNECTION_PARAMETER_REQUEST 0x06

/**
 * @format 1H2222
 * @param subevent_code
 * @param connection_handle
 * @param max_tx_octets
 * @param max_tx_time
 * @param max_rx_octets
 * @param max_rx_time
 */
#define HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE 0x07

/**
 * @format 11QQ
 * @param subevent_code
 * @param status
 * @param dhkey_x x coordinate of P256 public key
 * @param dhkey_y y coordinate of P256 public key
 */
#define HCI_SUBEVENT_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE 0x08
/**
* @format 11Q
* @param subevent_code
* @param status
* @param dhkey Diffie-Hellman key
*/
#define HCI_SUBEVENT_LE_GENERATE_DHKEY_COMPLETE            0x09

/**
 * @format 11H11BBB2221
 * @param subevent_code
 * @param status
 * @param connection_handle
 * @param role
 * @param peer_address_type
 * @param perr_addresss
 * @param local_resolvable_private_addres
 * @param peer_resolvable_private_addres
 * @param conn_interval
 * @param conn_latency
 * @param supervision_timeout
 * @param master_clock_accuracy
 */
#define HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE                0x0A

// array of advertisements, not handled by event accessor generator
#define HCI_SUBEVENT_LE_DIRECT_ADVERTISING_REPORT                   0x0B

/**
 * @format 11211
 * @param subevent_code
 * @param status
 * @param connection_handle
 * @param TX_PHY
 * @param RX_PHY
 */
#define HCI_SUBEVENT_LE_PHY_UPDATE_COMPLETE                  0x0C

// array of advertisements, not handled by event accessor generator
#define HCI_SUBEVENT_LE_EXTENDED_ADVERTISING_REPORT                 0x0D

#define HCI_SUBEVENT_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHED       0x0E

/**
 * @format 1211111B
 * @param subevent_code
 * @param sync_handle
 * @param tx_power
 * @param rssi
 * @param unused
 * @param data_status
 * @param data_length
 * @param data
 */
#define HCI_SUBEVENT_LE_PERIODIC_ADVERTISING_REPORT                 0x0F

/**
 * @format 2
 * @param sync_handle
*/
#define HCI_SUBEVENT_LE_PERIODIC_ADVERTISING_SYNC_LOST              0x10

/**
 * @format
*/
#define HCI_SUBEVENT_LE_SCAN_TIMEOUT                                0x11

/**
 * @format 1121
 * @param subevent_code
 * @param status
 * @param advertising_handle
 * @param connection_handle
 * @param num_completed_extended_advertising_events
 */
#define HCI_SUBEVENT_LE_ADVERTISING_SET_TERMINATED                  0x12

/**
 * @format 1116
 * @param subevent_code
 * @param advertising_handle
 * @param scanner_address_type
 * @param scanner_address
 */
#define HCI_SUBEVENT_LE_SCAN_REQUEST_RECEIVED                       0x13

/**
 * @format 21
 * @param subevent_code
 * @param connection_handle
 * @param channel_selection_algorithm
 */
#define HCI_SUBEVENT_LE_CHANNEL_SELECTION_ALGORITHM                 0x14


#define HCI_SUBEVENT_LE_VENDOR_INTERVAL_COMPLETE                    0xF0

/**
 * @format
*/
#define HCI_EVENT_ANCS_META                                			0xEA


/**
 * compact HCI Command packet description
 */
typedef struct {
    uint16_t    opcode;
    const char *format;
} hci_cmd_t;

int hci_send_cmd(const hci_cmd_t *cmd, ...);


extern const hci_cmd_t hci_reset;
extern const hci_cmd_t hci_read_bd_addr;
extern const hci_cmd_t hci_read_local_supported_features;
extern const hci_cmd_t hci_read_buffer_size;
extern const hci_cmd_t hci_read_local_supported_commands;
extern const hci_cmd_t hci_read_local_version_information;
extern const hci_cmd_t hci_read_le_host_supported;
extern const hci_cmd_t hci_read_local_name;
extern const hci_cmd_t hci_write_class_of_device;
extern const hci_cmd_t hci_write_local_name;
extern const hci_cmd_t hci_write_scan_enable;
extern const hci_cmd_t hci_set_event_mask;
extern const hci_cmd_t hci_le_add_device_to_white_list;
extern const hci_cmd_t hci_le_clear_white_list;
extern const hci_cmd_t hci_le_connection_update;
extern const hci_cmd_t hci_le_create_connection;
extern const hci_cmd_t hci_le_create_connection_cancel;
extern const hci_cmd_t hci_le_encrypt;
extern const hci_cmd_t hci_le_generate_dhkey;
extern const hci_cmd_t hci_le_long_term_key_negative_reply;
extern const hci_cmd_t hci_le_long_term_key_request_reply;
extern const hci_cmd_t hci_le_rand;
extern const hci_cmd_t hci_le_read_advertising_channel_tx_power;
extern const hci_cmd_t hci_le_read_buffer_size;
extern const hci_cmd_t hci_le_read_channel_map;
extern const hci_cmd_t hci_le_read_local_p256_public_key;
extern const hci_cmd_t hci_le_read_maximum_data_length;
extern const hci_cmd_t hci_le_read_remote_used_features;
extern const hci_cmd_t hci_le_read_suggested_default_data_length;
extern const hci_cmd_t hci_le_read_supported_features;
extern const hci_cmd_t hci_le_read_supported_states;
extern const hci_cmd_t hci_le_read_white_list_size;
extern const hci_cmd_t hci_le_receiver_test;
extern const hci_cmd_t hci_le_remove_device_from_white_list;
extern const hci_cmd_t hci_le_set_advertise_enable;
extern const hci_cmd_t hci_le_set_advertising_data;
extern const hci_cmd_t hci_le_set_advertising_parameters;
extern const hci_cmd_t hci_le_set_data_length;
extern const hci_cmd_t hci_le_set_event_mask;
extern const hci_cmd_t hci_le_set_host_channel_classification;
extern const hci_cmd_t hci_le_set_random_address;
extern const hci_cmd_t hci_le_set_scan_enable;
extern const hci_cmd_t hci_le_set_scan_parameters;
extern const hci_cmd_t hci_le_set_scan_response_data;
extern const hci_cmd_t hci_le_start_encryption;
extern const hci_cmd_t hci_le_test_end;
extern const hci_cmd_t hci_le_transmitter_test;
extern const hci_cmd_t hci_le_write_suggested_default_data_length;
extern const hci_cmd_t hci_le_set_phy;

extern const hci_cmd_t hci_le_set_ext_advertising_parameters;
extern const hci_cmd_t hci_le_set_ext_advertising_data;
extern const hci_cmd_t hci_le_set_ext_advertise_enable;
extern const hci_cmd_t hci_le_set_ext_scan_parameters;
extern const hci_cmd_t hci_le_set_ext_scan_enable;

enum VENDOR_REMOTE_TEST_VALUE {
    VENDOR_TEST_DISCONNECTED = 0,
    VENDOR_TEST_LEGACY_CONNECTED_BY_BT_CLASSIC,
    VENDOR_TEST_LEGACY_CONNECTED_BY_BLE,
    VENDOR_TEST_CONNECTED_WITH_TWS,
};


#endif
