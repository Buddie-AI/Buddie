/*****************************************************************
>file name : audio_cfifo.h
>author : lichao
>create time : Tue 10 Nov 2020 02:22:37 PM CST
*****************************************************************/
#ifndef _AUDIO_CFIFO_H_
#define _AUDIO_CFIFO_H_
#include "generic/list.h"
#include "typedef.h"

#define WRITE_MODE_BLOCK            0
#define WRITE_MODE_FORCE            1

struct audio_cfifo {
    u8 sample_channel;          /*fifo采样声道数*/
    s16 *addr;                  /*fifo首地址*/
    u16 sample_size;            /*fifo采样长度*/
    u16 wp;                     /*fifo写偏移*/
    u16 rp;                     /*fifo读偏移*/
    u16 lock_rp;                /*fifo不可擦写的已读偏移*/
    u16 free_samples;           /*fifo可写入样点个数*/
    s16 unread_samples;         /*fifo未读样点个数*/
    int sample_rate;            /*fifo对应的音频采样率*/
    u32 sw_ptr;
    u32 hw_ptr;
    struct list_head head;      /*子通道数据链表头*/
};

struct audio_cfifo_channel {
    u8  write_mode;             /*写入模式*/
    u16 delay_time;             /*最大延时(ms)*/
    u16 rsp;                    /*读偏移*/
    u16 wsp;                    /*写偏移*/
    s16 unread_samples;         /*通道未读样点个数*/
    u16 max_samples;            /*通道缓冲最大样点个数*/
    u32 sw_ptr;
    u32 hw_ptr;
    struct audio_cfifo *fifo;   /*主fifo指针*/
    struct list_head entry;     /*通道接入entry*/
};

/*************************************************************************
 * Audio cfifo主fifo初始化
 * INPUT    :  fifo - 主fifo结构指针，
 *             buf  - fifo缓冲地址
 *             len  - fifo缓冲长度
 *             sample_rate - fifo音频采样率
 *             channel     - fifo音频声道数
 * OUTPUT   :  0 - 成功，非0 - 失败.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_init(struct audio_cfifo *fifo, void *buf, int len, int sample_rate, u8 channel);

/*************************************************************************
 * Audio cfifo主fifo复位
 * INPUT    :  fifo - 主fifo结构指针，
 *             buf  - fifo缓冲地址
 *             len  - fifo缓冲长度
 *             sample_rate - fifo音频采样率
 *             channel     - fifo音频声道数
 * OUTPUT   :  0 - 成功，非0 - 失败.
 * WARNINGS :  这里与audio_cfifo_init类似但意义不一样，不可以当作init使用.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_reset(struct audio_cfifo *fifo, void *buf, int len, int sample_rate, u8 channel);

/*************************************************************************
 * 添加fifo子通道
 * INPUT    :  fifo - 主fifo结构指针，
 *             ch   - fifo子通道
 *             delay_time  - 子通道延时
 *             write_mode  - 子通道写入模式
 * OUTPUT   :  0 - 成功，非0 - 失败.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_channel_add(struct audio_cfifo *fifo, struct audio_cfifo_channel *ch, int delay_time, u8 write_mode);

/*************************************************************************
 * 删除fifo子通道
 * INPUT    :  ch   - fifo子通道
 * OUTPUT   :  无.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
void audio_cfifo_channel_del(struct audio_cfifo_channel *ch);

/*************************************************************************
 * 主fifo读数更新
 * INPUT    :  fifo - 主fifo结构指针, samples - 读取的样点个数
 * OUTPUT   :  成功更新的样点个数.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_read_update(struct audio_cfifo *fifo, int samples);

/*************************************************************************
 * fifo子通道数据写入
 * INPUT    :  ch - fifo子通道, data - 数据指针, len - 数据长度
 * OUTPUT   :  写入fifo的长度.
 * WARNINGS :  强制写入模式无论是否可以写入都将返回预期写入长度.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_channel_write(struct audio_cfifo_channel *ch, void *data, int len);

/*************************************************************************
 * fifo子通道写入直流数据
 * INPUT    :  ch - fifo子通道, data - 直流值, len - 长度
 * OUTPUT   :  写入fifo的长度.
 * WARNINGS :  强制写入模式无论是否可以写入都将返回预期写入长度.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_channel_write_fixed_data(struct audio_cfifo_channel *ch, s16 data, int len);

/*************************************************************************
 * fifo子通道擦除
 * INPUT    :  ch - fifo子通道
 * OUTPUT   :  未知.
 * WARNINGS :  待开发.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_channel_clear(struct audio_cfifo_channel *ch);

/*************************************************************************
 * 主fifo获取写偏移
 * INPUT    :  fifo - 主fifo结构指针
 * OUTPUT   :  fifo写入偏移.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_get_write_offset(struct audio_cfifo *fifo);

/*************************************************************************
 * 主fifo获取未读样点查询
 * INPUT    :  fifo - 主fifo结构指针
 * OUTPUT   :  样点个数.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_get_unread_samples(struct audio_cfifo *fifo);

/*************************************************************************
 * 主fifo与子fifo未读样点个数差值
 * INPUT    :  ch - fifo子通道
 * OUTPUT   :  样点个数.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_channel_unread_diff_samples(struct audio_cfifo_channel *ch);

/*************************************************************************
 * 主fifo通道个数
 * INPUT    :  fifo - 主fifo结构指针
 * OUTPUT   :  通道个数.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_channel_num(struct audio_cfifo *fifo);

/*************************************************************************
 * 主fifo设置已读样点不可擦除区域
 * INPUT    :  fifo - 主fifo结构指针, samples - 不可擦除样点个数
 * OUTPUT   :  无.
 * WARNINGS :  特殊处理，无需理解，仅作为开发人员使用.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
void audio_cfifo_set_readlock_samples(struct audio_cfifo *fifo, int samples);

/*************************************************************************
 * 获取主fifo设置已读样点不可擦除区域大小
 * INPUT    :  fifo - 主fifo结构指针.
 * OUTPUT   :  不可擦除样点个数.
 * WARNINGS :  特殊处理，无需理解，仅作为开发人员使用.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_get_readlock_samples(struct audio_cfifo *fifo);

/*************************************************************************
 * fifo使用回调读取通道的有效混合数据
 * INPUT    :  fifo     - 主fifo结构指针
 *             offset   - 读偏移
 *             samples  - 读取样点个数
 *             priv     - 回调私有指针
 *             read_callback - 读回调
 * OUTPUT   :  读取长度.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_read_with_callback(struct audio_cfifo *fifo, int offset, int samples,
                                   void *priv, int (*read_callback)(void *priv, void *data, int len));

/*************************************************************************
 * fifo子通道未读样点查询
 * INPUT    :  ch - fifo子通道
 * OUTPUT   :  样点个数.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_channel_unread_samples(struct audio_cfifo_channel *ch);

/*************************************************************************
 * fifo子通道获取写偏移
 * INPUT    :  ch - fifo子通道
 * OUTPUT   :  偏移.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_cfifo_channel_write_offset(struct audio_cfifo_channel *ch);

/*************************************************************************
 * fifo获取缓冲最小的子通道
 * INPUT    :  fifo - 主fifo结构指针
 * OUTPUT   :  fifo子通道指针.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
struct audio_cfifo_channel *audio_cfifo_min_samples_channel(struct audio_cfifo *fifo);

int audio_cfifo_get_sw_ptr(struct audio_cfifo *fifo);

int audio_cfifo_get_hw_ptr(struct audio_cfifo *fifo);

int audio_cfifo_read_data(struct audio_cfifo *fifo, s16 *out_buf, int len);
#endif
