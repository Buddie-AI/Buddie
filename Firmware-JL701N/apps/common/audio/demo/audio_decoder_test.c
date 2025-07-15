/*
 ****************************************************************
 *							AUDIO DECODER TEST
 * File  : audio_decoder_test.c
 * By    :
 * Notes : 解码测试
 *         before和after之间就是解码对应功能的处理，
 *         可以在before和after中分别翻转IO来卡一下处理时间
 ****************************************************************
 */


#include "media/includes.h"


#define DEC_IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}
#define DEC_IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}


/*
*********************************************************************
*                  Audio Decoder test output before
* Description: 解码输出
* Arguments  : *dec		解码句柄
*              *buff	输出buf
*              len		输出长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_out_before(struct audio_decoder *dec, void *buff, int len)
{
    DEC_IO_DEBUG_1(C, 3);
}
/*
*********************************************************************
*                  Audio Decoder test output after
* Description: 解码输出
* Arguments  : *dec		解码句柄
*              wlen		实际输出长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_out_after(struct audio_decoder *dec, int wlen)
{
    DEC_IO_DEBUG_0(C, 3);
}

/*
*********************************************************************
*                  Audio Decoder test read file before
* Description: 解码获取文件数据
* Arguments  : *dec		解码句柄
*              len		获取数据长度
*              offset 	获取数据偏移地址
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_read_before(struct audio_decoder *dec, int len, u32 offset)
{
    DEC_IO_DEBUG_1(C, 2);
}
/*
*********************************************************************
*                  Audio Decoder test read file after
* Description: 解码获取文件数据
* Arguments  : *dec		解码句柄
*              *data 	获取到的数据
*              rlen		获取到的数据长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_read_after(struct audio_decoder *dec, u8 *data, int rlen)
{
    DEC_IO_DEBUG_0(C, 2);
}

/*
*********************************************************************
*                  Audio Decoder test get frame before
* Description: 解码获取帧数据
* Arguments  : *dec		解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_get_frame_before(struct audio_decoder *dec)
{
    DEC_IO_DEBUG_1(C, 2);
}
/*
*********************************************************************
*                  Audio Decoder test get frame after
* Description: 解码获取帧数据
* Arguments  : *dec		解码句柄
*              *frame 	获取到的帧数据
*              rlen		获取到的帧数据长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_get_frame_after(struct audio_decoder *dec, u8 *frame, int rlen)
{
    DEC_IO_DEBUG_0(C, 2);
}

/*
*********************************************************************
*                  Audio Decoder test fetch frame before
* Description: 解码检查帧数据
* Arguments  : *dec		解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_fetch_before(struct audio_decoder *dec)
{
    DEC_IO_DEBUG_1(C, 2);
}
/*
*********************************************************************
*                  Audio Decoder test fetch frame after
* Description: 解码检查帧数据
* Arguments  : *dec		解码句柄
*              *frame 	获取到的帧数据
*              rlen		获取到的帧数据长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_fetch_after(struct audio_decoder *dec, u8 *frame, int rlen)
{
    DEC_IO_DEBUG_0(C, 2);
}

/*
*********************************************************************
*                  Audio Decoder test run before
* Description: 解码处理
* Arguments  : *dec		解码句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_run_before(struct audio_decoder *dec)
{
    DEC_IO_DEBUG_1(C, 1);
}
/*
*********************************************************************
*                  Audio Decoder test run after
* Description: 解码处理
* Arguments  : *dec		解码句柄
*              err 		解码处理返回值
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_decoder_test_run_after(struct audio_decoder *dec, int err)
{
    DEC_IO_DEBUG_0(C, 1);
}


/*
 * 数据流节点统计时间示例
 * 这里的时间是数据流节点运行的时间加上输出的时间
 * 数据流节点的运行时间约等于当前统计的时间减去下一个节点的统计时间
 * */
#if 0

// 保存原来的数据流数据处理
static void *demo_data_handler_save;
// 数据流data_handler处理
static int demo_new_data_handler(struct audio_stream_entry *entry,
                                 struct audio_data_frame *in,
                                 struct audio_data_frame *out)
{
    DEC_IO_DEBUG_1(C, 1);

    // 调用原来的接口输出
    int wlen = ((int (*)(struct audio_stream_entry *, struct audio_data_frame *, struct audio_data_frame *))demo_data_handler_save)(entry, in, out);

    DEC_IO_DEBUG_0(C, 1);
    return wlen;
}


void test_start(void)
{
#if 1
    // 用变量保存原来的数据处理接口，然后重新赋值新的数据处理
    // 这里以获取mix节点的数据为例
    demo_data_handler_save = (void *)bt_a2dp_dec->mix_ch.entry.data_handler;
    bt_a2dp_dec->mix_ch.entry.data_handler = demo_new_data_handler;
#endif

    // 数据流串联
    struct audio_stream_entry *entries[8] = {NULL};
    u8 entry_cnt = 0;
    entries[entry_cnt++] = &dec->dec.decoder.entry;
    entries[entry_cnt++] = &dec->mix_ch.entry;
    // ...
}


#endif


