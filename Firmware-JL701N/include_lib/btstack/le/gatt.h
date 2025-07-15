/*********************************************************************************************
    *   Filename        : btstack_event.h

    *   Description     : 

    *   Author          : Minxian

    *   Email           : liminxian@zh-jieli.com

    *   Last modifiled  : 2020-07-01 16:23

    *   Copyright:(c)JIELI  2011-2020  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef __BT_GATT_H
#define __BT_GATT_H

#include "typedef.h"

typedef struct {
    uint16_t start_group_handle;
    uint16_t end_group_handle;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} le_service_t, gatt_client_service_t;


typedef struct {
    uint16_t start_handle;
    uint16_t value_handle;
    uint16_t end_handle;
    uint16_t properties;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} le_characteristic_t, gatt_client_characteristic_t;

typedef struct {
    uint16_t handle;
    uint16_t uuid16;
    uint8_t  uuid128[16];
} gatt_client_characteristic_descriptor_t;

void gatt_client_deserialize_service(const uint8_t *packet, int offset, gatt_client_service_t *service);

void gatt_client_deserialize_characteristic(const uint8_t *packet, int offset, gatt_client_characteristic_t *characteristic);

void gatt_client_deserialize_characteristic_descriptor(const uint8_t *packet, int offset, gatt_client_characteristic_descriptor_t *descriptor);

uint8_t gatt_client_read_long_value_of_characteristic_using_value_handle_with_offset(btstack_packet_handler_t callback, hci_con_handle_t con_handle, uint16_t characteristic_value_handle, uint16_t offset);

uint8_t gatt_client_read_value_of_characteristic_using_value_handle(btstack_packet_handler_t callback, hci_con_handle_t con_handle, uint16_t value_handle);

uint8_t gatt_client_write_value_of_characteristic_without_response(hci_con_handle_t con_handle, uint16_t value_handle, uint16_t value_length, uint8_t *value);

uint8_t gatt_client_write_value_of_characteristic(btstack_packet_handler_t callback, hci_con_handle_t con_handle, uint16_t value_handle, uint16_t value_length, uint8_t *data);

void gatt_client_request_can_send_now_event(hci_con_handle_t con_handle);

#endif
