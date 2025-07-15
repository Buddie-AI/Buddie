#ifndef _AUD_HEARING_AID_LOW_POWER_H_
#define _AUD_HEARING_AID_LOW_POWER_H_


/*
*********************************************************************
*                  Energy Detect Open
* Description: 打开低功耗能量检测功能
* Arguments  : ch_num	  要处理的数据的通道数
*			   hw_open	  注册进去在出低功耗正常工作时会执行的回调
a              hw_close   注册进去在进低功耗时会执行的回调
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/

void audio_hearing_aid_lp_open(u8 ch_num, void (*hw_open)(void), void (*hw_close)(void));

/*
*********************************************************************
*                  Energy Detect Run
* Description: 对传进去的数据进行能量检测，并根据结果决定进出低功耗
* Arguments  : priv	  私有参数
*			   data	  要做处理的数据地址
a              len    要做处理的数据单个声道的数据长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/

void audio_hearing_aid_lp_detect(void *priv, s16 *data, int len);

/*
*********************************************************************
*                  Energy Detect Close
* Description: 关闭能量检测
* Arguments  : NULL
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/

void audio_hearing_aid_lp_close(void);

/*
*********************************************************************
*                  Get Low Power Flag
* Description: 获取是否能进低功耗的标志位
* Arguments  : NULL
* Return	 : 1:代表可以进入低功耗
               0:代表不能进入低功耗
* Note(s)    : None.
*********************************************************************
*/

u8 audio_hearing_aid_lp_flag(void);

#endif/*_AUD_HEARING_AID_LOW_POWER_H_*/
