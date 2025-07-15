#ifndef _AEC_UART_DEBUG_H_
#define _AEC_UART_DEBUG_H_

#include "generic/typedef.h"

/*
*********************************************************
*                  aec_uart_open
* Description: 打开数据写卡接口
* Arguments_v1  : nch 总通道数， single_size 单个通道的数据大小
* Arguments_v2  : nch 允许发送的最大通道数， single_size 单次发送允许的最大包长
* Return     : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************
*/
int aec_uart_open(u8 nch, u16 single_size);

/*
*********************************************************
*                  aec_uart_init
* Description: 数据写卡初始化
* Arguments  : None.
* Return     : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************
*/
int aec_uart_init(void);

/*
*********************************************************
*                  aec_uart_fill
* Description: 填写对应通道的数据
* Arguments  : ch 通道号，buf 数据地址，size 数据大小
* Return     : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************
*/
int aec_uart_fill(u8 ch, void *buf, u16 size);

/*
*********************************************************
*                  aec_uart_write
* Description: 将写入通话的数据写入串口buffer
* Arguments  : None.
* Return     : None.
* Note(s)    : None.
*********************************************************
*/
void aec_uart_write(void);

/*
*********************************************************
*                  aec_uart_close
* Description: 关闭数据写卡
* Arguments  : None.
* Return     : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************
*/
int aec_uart_close(void);
#endif/*_AEC_UART_DEBUG_H_*/
