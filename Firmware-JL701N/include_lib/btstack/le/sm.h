/*********************************************************************************************
    *   Filename        : sm.h

    *   Description     :

    *   Author          : Minxian

    *   Email           : liminxian@zh-jieli.com

    *   Last modifiled  : 2020-07-01 16:36

    *   Copyright:(c)JIELI  2011-2020  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _BT_SM_H_
#define _BT_SM_H_

#include "typedef.h"

// IO Capability Values
typedef enum {
    IO_CAPABILITY_DISPLAY_ONLY = 0,
    IO_CAPABILITY_DISPLAY_YES_NO,
    IO_CAPABILITY_KEYBOARD_ONLY,
    IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
    IO_CAPABILITY_KEYBOARD_DISPLAY, // not used by secure simple pairing
} io_capability_t;

void ble_sm_setup_init(io_capability_t io_type, u8 auth_req, uint8_t min_key_size, u8 security_en);

void ble_cbk_handler_register(btstack_packet_handler_t packet_cbk, sm_stack_packet_handler_t sm_cbk);

void sm_just_works_confirm(hci_con_handle_t con_handle);

void sm_init(void);

/*接口同时设置master 和 slave的配置*/
void sm_set_io_capabilities(io_capability_t io_capability);

/*接口只设置master配置*/
void sm_set_master_io_capabilities(io_capability_t io_capability);

/*接口同时设置master 和 slave的配置*/
void sm_set_authentication_requirements(uint8_t auth_req);

/*接口只设置master配置*/
void sm_set_master_authentication_requirements(uint8_t auth_req);

void sm_set_encryption_key_size_range(uint8_t min_size, uint8_t max_size);

void sm_set_request_security(int enable);

void sm_event_callback_set(void(*cbk_sm_ph)(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size));

//配从机默认发请求加密命令
void sm_set_request_security(int enable);

//配主机默认发加密请求命令
void sm_set_master_request_pair(int enable);

//指定链接发加密请求命令
bool sm_api_request_pairing(hci_con_handle_t con_handle);

//设置回连出现key missing后,流程重新发起加密
void sm_set_master_pair_redo(int enable);

//设置回连时，延时发起加密流程的时间，可用于兼容一些设备连接
void sm_set_master_reconnect_enc_delay(u16 delay_ms);
void sm_passkey_input(hci_con_handle_t con_handle, uint32_t passkey);
#endif
