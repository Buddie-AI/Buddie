/*********************************************************************************************
    *   Filename        : le_counter.h

    *   Description     :

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2017-01-17 15:17

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

#ifndef    __LE_COMMON_DEFINE_H_
#define    __LE_COMMON_DEFINE_H_

#include "typedef.h"
#include <stdint.h>
#include "btstack/bluetooth.h"

//--------------------------------------------

#define ADV_SET_1M_PHY                  1
#define ADV_SET_2M_PHY                  2
#define ADV_SET_CODED_PHY               3

#define SCAN_SET_1M_PHY                 BIT(0)
#define SCAN_SET_2M_PHY                 BIT(1)
#define SCAN_SET_CODED_PHY              BIT(2)

#define INIT_SET_1M_PHY                 BIT(0)
#define INIT_SET_2M_PHY                 BIT(1)
#define INIT_SET_CODED_PHY              BIT(2)

#define CONN_SET_1M_PHY                 BIT(0)
#define CONN_SET_2M_PHY                 BIT(1)
#define CONN_SET_CODED_PHY              BIT(2)

#define CONN_SET_PHY_OPTIONS_NONE        0
#define CONN_SET_PHY_OPTIONS_S2          1
#define CONN_SET_PHY_OPTIONS_S8          2

struct conn_param_t {
    u16 interval;
    u16 latency;
    u16 timeout;
};

// #define NOTIFY_TYPE           1
// #define INDICATION_TYPE       2
// Minimum/default MTU

#define ATT_CTRL_BLOCK_SIZE       (188)                    //note: fixed,libs use
#define ATT_PACKET_HEAD_SIZE      (6)                      //note: fixed,libs use

/*adv type*/
enum {
    ADV_IND = 0,         /*Connectable and scannable undirected advertising*/
    ADV_DIRECT_IND,      /*Connectable high duty cycle directed advertising */
    ADV_SCAN_IND,        /*Scannable undirected advertising*/
    ADV_NONCONN_IND,     /*Non connectable undirected advertising*/
    ADV_DIRECT_IND_LOW,  /*Connectable low duty cycle directed advertising*/
};


/*adv channel*/
#define  ADV_CHANNEL_37    BIT(0)
#define  ADV_CHANNEL_38    BIT(1)
#define  ADV_CHANNEL_39    BIT(2)
#define  ADV_CHANNEL_ALL  (ADV_CHANNEL_37 | ADV_CHANNEL_38 | ADV_CHANNEL_39)

/*scan type*/
enum {
    SCAN_PASSIVE = 0,
    SCAN_ACTIVE,
};

/*advertising report,event type*/
#define EVENT_ADV_IND                  ADV_IND
#define EVENT_ADV_DIRECT_IND           ADV_DIRECT_IND
#define EVENT_ADV_SCAN_IND             ADV_SCAN_IND
#define EVENT_ADV_NONCONN_IND          ADV_NONCONN_IND
#define EVENT_SCAN_RSP                 (4)

#define EVENT_DEFAULT_REPORT_BITMAP    (0x1f)


/*flags*/
#define FLAGS_LIMITED_DISCOVERABLE_MODE    BIT(0)
#define FLAGS_GENERAL_DISCOVERABLE_MODE    BIT(1)
#define FLAGS_EDR_NOT_SUPPORTED            BIT(2)
#define FLAGS_LE_AND_EDR_SAME_CONTROLLER   BIT(3)
#define FLAGS_LE_AND_EDR_SAME_HOST         BIT(4)

/*eir packet_type*/
typedef enum {
    HCI_EIR_DATATYPE_FLAGS =                                                 0x01,
    HCI_EIR_DATATYPE_MORE_16BIT_SERVICE_UUIDS =                              0x02,
    HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS =                          0x03,
    HCI_EIR_DATATYPE_MORE_32BIT_SERVICE_UUIDS =                              0x04,
    HCI_EIR_DATATYPE_COMPLETE_32BIT_SERVICE_UUIDS =                          0x05,
    HCI_EIR_DATATYPE_MORE_128BIT_SERVICE_UUIDS =                             0x06,
    HCI_EIR_DATATYPE_COMPLETE_128BIT_SERVICE_UUIDS =                         0x07,
    HCI_EIR_DATATYPE_SHORTENED_LOCAL_NAME =                                  0x08,
    HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME =                                   0x09,
    HCI_EIR_DATATYPE_TX_POWER_LEVEL =                                        0x0A,
    HCI_EIR_DATATYPE_CLASS_OF_DEVICE =                                       0x0D,
    HCI_EIR_DATATYPE_SIMPLE_PAIRING_HASH_C =                                 0x0E,
    HCI_EIR_DATATYPE_SIMPLE_PAIRING_RANDOMIZER_R =                           0x0F,
    HCI_EIR_DATATYPE_SECURITY_MANAGER_TK_VALUE =                             0x10,
    HCI_EIR_DATATYPE_SECURITY_MANAGER_OOB_FLAGS =                            0x11,
    HCI_EIR_DATATYPE_SLAVE_CONNECTION_INTERVAL_RANGE =                       0x12,
    HCI_EIR_DATATYPE_16BIT_SERVICE_SOLICITATION_UUIDS =                      0x14,
    HCI_EIR_DATATYPE_128BIT_SERVICE_SOLICITATION_UUIDS =                     0x15,
    HCI_EIR_DATATYPE_SERVICE_DATA =                                          0x16,
    HCI_EIR_DATATYPE_APPEARANCE_DATA =                                       0x19,
    HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA =                            0xFF
} HCI_EIR_datatype_t;


//按(长度 + 类型 + 内容)这样的格,组合填入广播包数据
static inline u8 make_eir_packet_data(u8 *buf, u16 offset, u8 data_type, u8 *data, u8 data_len)
{
    if (ADV_RSP_PACKET_MAX - offset < data_len + 2) {
        return offset + data_len + 2;
    }

    buf[0] = data_len + 1;
    buf[1] = data_type;
    memcpy(buf + 2, data, data_len);
    return data_len + 2;
}

//按(长度 + 类型 + 内容)这样的格,组合填入广播包数据
static inline u8 make_eir_packet_val(u8 *buf, u16 offset, u8 data_type, u32 val, u8 val_size)
{
    if (ADV_RSP_PACKET_MAX - offset < val_size + 2) {
        return offset + val_size + 2;
    }

    buf[0] = val_size + 1;
    buf[1] = data_type;
    memcpy(buf + 2, &val, val_size);
    return val_size + 2;
}

#define BLE_APPEARANCE_UNKNOWN                                0 /**< Unknown. */
#define BLE_APPEARANCE_GENERIC_PHONE                         64 /* Generic Phone. */
#define BLE_APPEARANCE_GENERIC_COMPUTER                     128 /* Generic Computer. */
#define BLE_APPEARANCE_GENERIC_WATCH                        192 /* Generic Watch. */
#define BLE_APPEARANCE_WATCH_SPORTS_WATCH                   193 /* Watch: Sports Watch. */
#define BLE_APPEARANCE_GENERIC_CLOCK                        256 /* Generic Clock. */
#define BLE_APPEARANCE_GENERIC_DISPLAY                      320 /* Generic Display. */
#define BLE_APPEARANCE_GENERIC_REMOTE_CONTROL               384 /* Generic Remote Control. */
#define BLE_APPEARANCE_GENERIC_EYE_GLASSES                  448 /* Generic Eye-glasses. */
#define BLE_APPEARANCE_GENERIC_TAG                          512 /* Generic Tag. */
#define BLE_APPEARANCE_GENERIC_KEYRING                      576 /* Generic Keyring. */
#define BLE_APPEARANCE_GENERIC_MEDIA_PLAYER                 640 /* Generic Media Player. */
#define BLE_APPEARANCE_GENERIC_BARCODE_SCANNER              704 /* Generic Barcode Scanner. */
#define BLE_APPEARANCE_GENERIC_THERMOMETER                  768 /* Generic Thermometer. */
#define BLE_APPEARANCE_THERMOMETER_EAR                      769 /* Thermometer: Ear. */
#define BLE_APPEARANCE_GENERIC_HEART_RATE_SENSOR            832 /* Generic Heart rate Sensor. */
#define BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT    833 /* Heart Rate Sensor: Heart Rate Belt. */
#define BLE_APPEARANCE_GENERIC_BLOOD_PRESSURE               896 /* Generic Blood Pressure. */
#define BLE_APPEARANCE_BLOOD_PRESSURE_ARM                   897 /* Blood Pressure: Arm. */
#define BLE_APPEARANCE_BLOOD_PRESSURE_WRIST                 898 /* Blood Pressure: Wrist. */
#define BLE_APPEARANCE_GENERIC_HID                          960 /* Human Interface Device (HID). */
#define BLE_APPEARANCE_HID_KEYBOARD                         961 /* Keyboard (HID Subtype). */
#define BLE_APPEARANCE_HID_MOUSE                            962 /* Mouse (HID Subtype). */
#define BLE_APPEARANCE_HID_JOYSTICK                         963 /* Joystick (HID Subtype). */
#define BLE_APPEARANCE_HID_GAMEPAD                          964 /* Gamepad (HID Subtype). */
#define BLE_APPEARANCE_HID_DIGITIZERSUBTYPE                 965 /* Digitizer Tablet (HID Subtype). */
#define BLE_APPEARANCE_HID_CARD_READER                      966 /* Card Reader (HID Subtype). */
#define BLE_APPEARANCE_HID_DIGITAL_PEN                      967 /* Digital Pen (HID Subtype). */
#define BLE_APPEARANCE_HID_BARCODE                          968 /* Barcode Scanner (HID Subtype). */
#define BLE_APPEARANCE_GENERIC_GLUCOSE_METER               1024 /* Generic Glucose Meter. */
#define BLE_APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR      1088 /* Generic Running Walking Sensor. */
#define BLE_APPEARANCE_RUNNING_WALKING_SENSOR_IN_SHOE      1089 /* Running Walking Sensor: In-Shoe. */
#define BLE_APPEARANCE_RUNNING_WALKING_SENSOR_ON_SHOE      1090 /* Running Walking Sensor: On-Shoe. */
#define BLE_APPEARANCE_RUNNING_WALKING_SENSOR_ON_HIP       1091 /* Running Walking Sensor: On-Hip. */
#define BLE_APPEARANCE_GENERIC_CYCLING                     1152 /* Generic Cycling. */
#define BLE_APPEARANCE_CYCLING_CYCLING_COMPUTER            1153 /* Cycling: Cycling Computer. */
#define BLE_APPEARANCE_CYCLING_SPEED_SENSOR                1154 /* Cycling: Speed Sensor. */
#define BLE_APPEARANCE_CYCLING_CADENCE_SENSOR              1155 /* Cycling: Cadence Sensor. */
#define BLE_APPEARANCE_CYCLING_POWER_SENSOR                1156 /* Cycling: Power Sensor. */
#define BLE_APPEARANCE_CYCLING_SPEED_CADENCE_SENSOR        1157 /* Cycling: Speed and Cadence Sensor. */
#define BLE_APPEARANCE_GENERIC_PULSE_OXIMETER              3136 /* Generic Pulse Oximeter. */
#define BLE_APPEARANCE_PULSE_OXIMETER_FINGERTIP            3137 /* Fingertip (Pulse Oximeter subtype). */
#define BLE_APPEARANCE_PULSE_OXIMETER_WRIST_WORN           3138 /* Wrist Worn(Pulse Oximeter subtype). */
#define BLE_APPEARANCE_GENERIC_WEIGHT_SCALE                3200 /* Generic Weight Scale. */
#define BLE_APPEARANCE_GENERIC_OUTDOOR_SPORTS_ACT          5184 /* Generic Outdoor Sports Activity. */
#define BLE_APPEARANCE_OUTDOOR_SPORTS_ACT_LOC_DISP         5185 /* Location Display Device (Outdoor Sports Activity subtype). */
#define BLE_APPEARANCE_OUTDOOR_SPORTS_ACT_LOC_AND_NAV_DISP 5186 /* Location and Navigation Display Device (Outdoor Sports Activity subtype). */
#define BLE_APPEARANCE_OUTDOOR_SPORTS_ACT_LOC_POD          5187 /* Location Pod (Outdoor Sports Activity subtype). */
#define BLE_APPEARANCE_OUTDOOR_SPORTS_ACT_LOC_AND_NAV_POD  5188 /* Location and Navigation Pod (Outdoor Sports Activity subtype). */

extern void le_l2cap_register_packet_handler(void (*handler)(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size));
#endif


