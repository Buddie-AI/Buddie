#ifndef _BTSTACK_TYPEDEF_H_
#define _BTSTACK_TYPEDEF_H_

#include "typedef.h"

#define BD_ADDR_LEN 6
typedef uint8_t bd_addr_t[BD_ADDR_LEN];

typedef uint16_t hci_con_handle_t;

typedef int (*sm_stack_packet_handler_t)(uint8_t packet_type, uint8_t *packet, uint16_t size);

typedef void (*btstack_packet_handler_t)(u8 packet_type, u16 channel, u8 *packet, u16 size);

typedef void (*ble_cbk_handler_t)(void);

void reverse_bd_addr(const bd_addr_t src, bd_addr_t dest);

void reverse_bytes(const u8 *src, u8 *dst, u32 len);

uint16_t little_endian_read_16(const uint8_t *buffer, int pos);
uint32_t little_endian_read_24(const uint8_t *buffer, int pos);
uint32_t little_endian_read_32(const uint8_t *buffer, int pos);


#endif
