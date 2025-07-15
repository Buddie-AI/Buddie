
#ifndef __AUDIO_ENC_MPT_FRE_RESPONE_H_
#define __AUDIO_ENC_MPT_FRE_RESPONE_H_

#include "asm/cpu.h"

/********************用户配置****************************/

//计算长度 输入单位s16/输出单位float
#define AUDIO_ENC_MPT_FRERES_POINT		1024

/********************非用户配置****************************/

//MIC频响测试通道ID u16
#define AUDIO_ENC_MPT_FF_MIC    	0X01	//测试TWS FFMIC or 头戴式LFF MIC
#define AUDIO_ENC_MPT_FB_MIC    	0X02	//测试TWS FBMIC or 头戴式LFB MIC
#define AUDIO_ENC_MPT_RFF_MIC   	0X04	//测试头戴式RFF MIC
#define AUDIO_ENC_MPT_RFB_MIC   	0X08	//测试头戴式RFB MIC
#define AUDIO_ENC_MPT_CVP_OUT    	0X10	//测试通话算法输出
#define AUDIO_ENC_MPT_TALK_MIC   	0X20	//测试通话TALK 主MIC
#define AUDIO_ENC_MPT_SLAVE_MIC  	0X40	//测试通话TALK 副MIC
#define AUDIO_ENC_MPT_TALK_FB_MIC  	0X80	//测试通话TALK FBMIC

//常见通道组合
//FF+TALK+算法输出频响测试，默认关闭DNS, 外部喇叭发声
#define AUDIO_ENC_MPT_CH_TWS_CVP_ENC  (AUDIO_ENC_MPT_SLAVE_MIC | AUDIO_ENC_MPT_TALK_MIC | AUDIO_ENC_MPT_CVP_OUT)

//FF回声/气密性+FB频响测试，耳机喇叭发声
#define AUDIO_ENC_MPT_CH_TWS_FF_FB    (AUDIO_ENC_MPT_FF_MIC | AUDIO_ENC_MPT_FB_MIC)


#define AUDIO_ENC_MPT_FRERES_ASYNC	1		//是否异步处理， 异步算不过来要加时钟提醒

enum {
    ENC_FRE_RES_STATE_START = 0,
    ENC_FRE_RES_STATE_RUN,
    ENC_FRE_RES_STATE_STOP,
};

enum {
    ENC_FRE_RESPONE_MSG_RUN = 0xA1,
};

//更新目标通道输入buf、len
void audio_enc_mpt_fre_response_inbuf(u16 id, s16 *buf, int len);

//音频测试频响计算运行
void audio_enc_mpt_fre_response_post_run(u16 id);

//音频测试频响计算启动, ch 对应目标的通道
void audio_enc_mpt_fre_response_start(u16 ch);

//音频测试频响计算停止
void audio_enc_mpt_fre_response_stop(void);

//工具获取数据文件
int audio_enc_mpt_fre_response_file_get(u8 **buf);

//数据获取结束，释放内存
void audio_enc_mpt_fre_response_release(void);

//音频测试MIC频响开启， ch 对应目标的通道
void audio_enc_mpt_fre_response_open(u16 ch);

//音频测试MIC频响关闭
void audio_enc_mpt_fre_response_close(void);

#endif /*__AUDIO_ENC_MPT_FRE_RESPONE_H_*/

