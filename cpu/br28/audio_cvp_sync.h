#ifndef AUDIO_CVP_SYNC_H
#define AUDIO_CVP_SYNC_H

/*
*********************************************************************
*                  Audio CVP Sync Open
* Description:  通话上行同步打开
* Arguments  : sr 采样率
* Return	 : 0 成功 其他 失败
* Note(s)    : null
*********************************************************************
*/
int audio_cvp_sync_open(u16 sr);

/*
*********************************************************************
*                  Audio CVP Sync Run
* Description:  通话上行同步处理
* Arguments  : data 数据指针， len 数据长度(byte)
* Return	 : len 写入的数据长度，0 写入失败
* Note(s)    : null
*********************************************************************
*/
int audio_cvp_sync_run(s16 *data, int len);

/*
*********************************************************************
*                  Audio CVP Sync Close
* Description:  通话上行同步关闭
* Arguments  : null
* Return	 : 0 成功 其他 失败
* Note(s)    : null
*********************************************************************
*/
int audio_cvp_sync_close();

#endif/*AUDIO_CVP_SYNC_H*/
