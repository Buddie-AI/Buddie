/*************************************************************************************************/
/*!
*  \file      audio_effect_develop.h
*
*  \brief
*
*  Copyright (c) 2011-2023 ZhuHai Jieli Technology Co.,Ltd.
*
*/
/*************************************************************************************************/
#ifndef _AUDIO_EFFECT_DEVELOP_H_
#define _AUDIO_EFFECT_DEVELOP_H_

/*********************************************************************** 
 * 音效算法开发打开 
 * Input    :  sample_rate      - 采样率 
               nch              - 声道数 
               bit_width        - 位宽(通常默认为16bit)
 * Output   :  音效算法开发的主句柄 
 * Notes    : 
 * History  : 
 *=====================================================================*/  
void *audio_effect_develop_open(int sample_rate, u8 nch, u8 bit_width);

/*********************************************************************** 
 * 音效算法开发关闭 
 * Input    :  priv - 第三方音效算法的主要私有句柄 
 * Output   :   
 * Notes    :   
 * History  : 
 *=====================================================================*/  
void audio_effect_develop_close(void *priv);

/*********************************************************************** 
 * 音效算法开发处理函数 
 * Input    :  priv - 第三方音效算法的主要私有句柄 
               data - pcm数据 
               len  - pcm数据的byte长度 
 * Output   :  数据处理长度 
 * Notes    : 
 * History  : 
 *=====================================================================*/  
int audio_effect_develop_data_handler(void *priv, void *data, int len);

#endif
