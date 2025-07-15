#ifndef __PRIVATE_IIS_H
#define __PRIVATE_IIS_H

#include "cpu.h"

// #define PRIVATE_IIS_RX_SR	44100
#define PRIVATE_MESSAGE_SIZE	512			//接收消息
#define PRIVATE_IIS_CMD			0x12345678

#define PRIVATE_IIS_TX_LINEIN_CH_IDX  (AUDIO_ADC_LINE1 | AUDIO_ADC_LINE0)
#define PRIVATE_IIS_TX_LINEIN_GAIN     4

//定义命令的长度为多少字节, 变量个数* 变量类型字节数
#define  CMD_LEN      4 * 4


void private_iis_rx_open(void);
u8 get_private_iis_get_msg_flag(void);
void show_private_msg(void);
void private_iis_rx_close(void);

void private_iis_tx_open(void);
void private_iis_tx_close(void);


#endif

