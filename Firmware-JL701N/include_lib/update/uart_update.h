#ifndef _UART_DEV_
#define _UART_DEV_

#include "typedef.h"

#define MSG_UART_UPDATE_READY       0x1
#define MSG_UART_UPDATE_START       0x2
#define MSG_UART_UPDATE_START_RSP   0X3
#define MSG_UART_UPDATE_READ_RSP    0x4

#define PROTOCAL_SIZE       528
#define SYNC_SIZE           6
#define SYNC_MARK0          0xAA
#define SYNC_MARK1          0x55

typedef union {
    u8 raw_data[PROTOCAL_SIZE + SYNC_SIZE];
    struct {
        u8 mark0;
        u8 mark1;
        u16 length;
        u8 data[PROTOCAL_SIZE + 2]; //最后CRC16
    } data;
} protocal_frame_t;

struct file_info {
    u8 cmd;
    u32 addr;
    u32 len;
} __attribute__((packed));

typedef struct __update_io {
    u16 rx;
    u16 tx;
    u8  input_channel;                  //input channel选择，根据方案选择未被使用的channel
    u8  output_channel;                 //同input channel
} uart_update_cfg;

void uart_update_init(uart_update_cfg *cfg);
void sava_uart_update_param(void);

#endif

