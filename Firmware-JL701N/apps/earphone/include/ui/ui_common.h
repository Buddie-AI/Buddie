#ifndef _UI_COMMON_H_
#define _UI_COMMON_H_

#include "typedef.h"
/* =================== 十进制数值(1位)转ASCII值 =================== */
void itoa1(u8 i, u8 *buf);
/* =================== 十进制数值(2位)转ASCII值 =================== */
void itoa2(u8 i, u8 *buf);
/* =================== 十进制数值(3位)转ASCII值 =================== */
void itoa3(u16 i, u8 *buf);
/* =================== 十进制数值(4位)转ASCII值 =================== */
void itoa4(u16 i, u8 *buf);


#endif
