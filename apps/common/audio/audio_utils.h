#ifndef _AUDIO_UTILS_H_
#define _AUDIO_UTILS_H_

#include "generic/typedef.h"

/*
*********************************************************************
*                  Audio Digital Phase Inverter
* Description: 数字反相器，用来反转数字音频信号的相位
* Arguments  : dat  数据buf地址
*			   len	数据长度(unit:byte)
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void digital_phase_inverter_s16(s16 *dat, int len);

#endif/*_AUDIO_UTILS_H_*/
