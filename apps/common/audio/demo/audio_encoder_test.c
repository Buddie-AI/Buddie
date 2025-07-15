/*
 ****************************************************************
 *							AUDIO ENCODE TEST
 * File  : audio_encoder_test.c
 * By    :
 * Notes : 编码测试
 *         before和after之间就是编码对应功能的处理，
 *         可以在before和after中分别翻转IO来卡一下处理时间
 ****************************************************************
 */


#include "media/includes.h"


#define ENC_IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}
#define ENC_IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}



/*
*********************************************************************
*                  Audio encoder test output before
* Description: 编码输出
* Arguments  : *enc		编码句柄
*              *buff	输出buf
*              len		输出长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_encoder_test_out_before(struct audio_encoder *enc, void *buff, int len)
{
    ENC_IO_DEBUG_1(C, 3);
}
/*
*********************************************************************
*                  Audio encoder test output after
* Description: 编码输出
* Arguments  : *enc		编码句柄
*              wlen		实际输出长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_encoder_test_out_after(struct audio_encoder *enc, int wlen)
{
    ENC_IO_DEBUG_0(C, 3);
}

/*
*********************************************************************
*                  Audio encoder test get frame before
* Description: 编码获取帧数据
* Arguments  : *enc		编码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_encoder_test_get_frame_before(struct audio_encoder *enc, u16 frame_len)
{
    ENC_IO_DEBUG_1(C, 2);
}
/*
*********************************************************************
*                  Audio encoder test get frame after
* Description: 编码获取帧数据
* Arguments  : *enc		编码句柄
*              *frame 	获取到的帧数据
*              rlen		获取到的帧数据长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_encoder_test_get_frame_after(struct audio_encoder *enc, s16 *frame, int rlen)
{
    ENC_IO_DEBUG_0(C, 2);
}

/*
*********************************************************************
*                  Audio encoder test run before
* Description: 编码处理
* Arguments  : *enc		编码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_encoder_test_run_before(struct audio_encoder *enc)
{
    ENC_IO_DEBUG_1(C, 1);
}
/*
*********************************************************************
*                  Audio encoder test run after
* Description: 编码处理
* Arguments  : *enc		编码句柄
*              err 		编码处理返回值
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_encoder_test_run_after(struct audio_encoder *enc, int err)
{
    ENC_IO_DEBUG_0(C, 1);
}




