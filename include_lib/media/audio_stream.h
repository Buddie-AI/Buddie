#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "generic/includes.h"
#include "media/audio_base.h"


// 数据流IOCTRL命令
#define AUDIO_STREAM_IOCTRL_CMD_CHECK_ACTIVE		(1) // 检查数据流是否活动


struct audio_stream_entry;
struct audio_frame_copy;


/*
 * 数据流中的传输内容
 */
struct audio_data_frame {
    u8 channel;				// 通道数
    u16 stop : 1;			// 数据流停止标志
    u16 data_sync : 1;		// 数据流同步标志
    u16 no_subsequent : 1;	// 1-不再执行后续的数据流；0-正常执行
    u32 sample_rate;		// 采样率
    u16 offset;             // 数据偏移
    u16 data_len;			// 数据长度
    s16 *data;				// 数据地址
};


/*
 * 数据流
 * 多个数据流节点组成一个数据流
 */
struct audio_stream {
    struct audio_stream_entry *entry;	// 数据流节点
    void *priv;							// resume私有参数
    void (*resume)(void *priv);			// 激活回调接口
};


/*
 * 数据流群组
 * 用于串联多个数据流，在调用resume/clear等操作时依次处理各个数据流
 * 如数据流stream0最后一个节点是entry0_n，数据流1最后一个节点是entry1_n
 * 把这两个数据流加入到群组test_group中
 * audio_stream_group_add_entry(&test_group, &entry0_n);
 * audio_stream_group_add_entry(&test_group, &entry1_n);
 * 另一个数据流streamX的第一个节点是entryX_0，把群组关联到streamX数据流
 * entryX_0.group = &test_group;
 * 那么调用streamX中的resume就会依次调用stream0和stream1的resume了
 */
struct audio_stream_group {
    struct audio_stream_entry *entry;	// 数据流节点
};

/*
 * 数据流节点
 * 数据流节点依次串联，数据依次往后传递
 */
struct audio_stream_entry {
    u8  pass_by;    // 1-同步处理（即往后传的buf就是上层传入的buf）；
    // 0-异步处理（数据存到其他buf再往后传）
    u8  remain;		// 1-上次数据没输出完。0-上次数据输出完
    u16 offset;		// 同步处理时的数据偏移
    struct audio_stream *stream;		// 所属的数据流
    struct audio_stream_entry *input;	// 上一个节点
    struct audio_stream_entry *output;	// 下一个节点
    struct audio_stream_entry *sibling;	// 数据流群组中的数据流节点链表
    struct audio_stream_group *group;	// 数据流群组节点
    struct audio_frame_copy *frame_copy;	// 数据分支节点
    int (*prob_handler)(struct audio_stream_entry *,  struct audio_data_frame *in);	// 预处理
    int (*data_handler)(struct audio_stream_entry *,  struct audio_data_frame *in,
                        struct audio_data_frame *out);					// 数据处理
    void (*data_process_len)(struct audio_stream_entry *,  int len);	// 后级返回使用的数据长度
    void (*data_clear)(struct audio_stream_entry *);					// 清除节点数据
    int (*ioctrl)(struct audio_stream_entry *, int cmd, int *param);	// 节点IOCTRL
};

/*
 * 数据流分支
 * 支持一传多（分支）功能，内部自动生成struct audio_frame_copy来处理多个分支
 * 有多少个分支就会申请多少个空间，数据分别拷贝到对应分支空间然后往后传递
 */
struct audio_frame_copy {
    struct list_head head;				// 链表。用于连接各个分支
    struct audio_data_frame frame;		// 保存上层的传输内容
    struct audio_stream_entry entry;	// 连接上层的节点
};



/*
*********************************************************************
*                  Audio Stream Open
* Description: 创建一个数据流
* Arguments  : *priv		resume回调的私有参数
*              resume		模块唤醒数据流时的回调函数
* Return	 : 数据流句柄
* Note(s)    : None.
*********************************************************************
*/
struct audio_stream *audio_stream_open(void *priv, void (*resume)(void *priv));

/*
*********************************************************************
*                  Audio Stream Add First
* Description: 添加第一个数据流节点
* Arguments  : *stream		数据流句柄
*              *entry		数据流节点句柄
* Return	 : None.
* Note(s)    : 第一个节点一般为解码输出
*********************************************************************
*/
void audio_stream_add_first(struct audio_stream *stream,
                            struct audio_stream_entry *entry);

/*
*********************************************************************
*                  Audio Stream Add Head
* Description: 将节点插入到数据流的first节点后面
* Arguments  : *stream		数据流句柄
*              *entry		数据流节点句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_stream_add_head(struct audio_stream *stream,
                           struct audio_stream_entry *entry);

/*
*********************************************************************
*                  Audio Stream Add Tail
* Description: 将节点放到数据流的最后
* Arguments  : *stream		数据流句柄
*              *entry		数据流节点句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_stream_add_tail(struct audio_stream *stream,
                           struct audio_stream_entry *entry);

/*
*********************************************************************
*                  Audio Stream Add Entry
* Description: 将output节点添加到input节点之后
* Arguments  : *input		数据流节点句柄
*              *output		需要添加的数据流节点句柄
* Return	 : None.
* Note(s)    : 如果input节点后面已经连接有节点，将创建数据流分支处理
*********************************************************************
*/
void audio_stream_add_entry(struct audio_stream_entry *input,
                            struct audio_stream_entry *output);

/*
*********************************************************************
*                  Audio Stream Add List
* Description: 将entry数组中节点按顺序添加到数据流中
* Arguments  : *stream		数据流句柄
*              *entry[]		数据流节点数组
*              num			节点总个数
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_stream_add_list(struct audio_stream *stream,
                           struct audio_stream_entry *entry[], int num);

/*
*********************************************************************
*                  Audio Stream Delete Entry
* Description: 将节点从数据流中删除
* Arguments  : *entry		数据流节点句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_stream_del_entry(struct audio_stream_entry *entry);


/*
*********************************************************************
*                  Audio Stream Delete List Entry
* Description: 依次将数组中的节点删除
* Arguments  : *entry[]		数据流节点数组
*              num			节点总个数
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_stream_del_list(struct audio_stream_entry *entry[], int num);


/*
*********************************************************************
*                  Audio Stream Group Add Entry
* Description: 将节点加入到群组之中
* Arguments  : *group		数据流群组句柄
*              *entry		数据流节点句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_stream_group_add_entry(struct audio_stream_group *group,
                                  struct audio_stream_entry *entry);

/*
*********************************************************************
*                  Audio Stream Group Delete Entry
* Description: 将节点从群组之中删除
* Arguments  : *group		数据流群组句柄
*              *entry		数据流节点句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_stream_group_del_entry(struct audio_stream_group *group,
                                  struct audio_stream_entry *entry);

/*
*********************************************************************
*                  Audio Stream Group Get Entry Number
* Description: 获取群组中的节点个数
* Arguments  : *group		数据流群组句柄
* Return	 : 节点个数
* Note(s)    : None.
*********************************************************************
*/
int audio_stream_group_entry_num(struct audio_stream_group *group);

/*
*********************************************************************
*                  Audio Stream Run
* Description: 数据处理接口
* Arguments  : *entry		数据流节点句柄
*              *input		输入数据
* Return	 : 负数-出错
* Note(s)    : 数据流依次递归往后传输。直到最后一个节点，或者no_subsequent==1，
*              或者prob_handler/data_handler处理错误返回负数
*********************************************************************
*/
int audio_stream_run(struct audio_stream_entry *from, struct audio_data_frame *input);


/*
*********************************************************************
*                  Audio Stream Resume
* Description: 唤醒数据流
* Arguments  : *entry		数据流节点句柄
* Return	 : 0-成功
* Note(s)    : 最终会递归调用到struct audio_stream结构体中的.resume回调接口
*********************************************************************
*/
int audio_stream_resume(struct audio_stream_entry *entry);


/*
*********************************************************************
*                  Audio Stream Clear From Entry
* Description: 清除指定节点及后续所有节点的数据
* Arguments  : *entry		数据流节点句柄
* Return	 : None.
* Note(s)    : 会清除offset/pass_by等记录，并且调用data_clear回调接口清除用户数据
*********************************************************************
*/
void audio_stream_clear_from(struct audio_stream_entry *entry);

/*
*********************************************************************
*                  Audio Stream Clear
* Description: 清除数据流所有节点数据
* Arguments  : *stream		数据流句柄
* Return	 : None.
* Note(s)    : 会清除offset/pass_by等记录，并且调用data_clear回调接口清除用户数据
*********************************************************************
*/
void audio_stream_clear(struct audio_stream *stream);

/*
*********************************************************************
*                  Audio Stream Clear By Entry
* Description: 通过数据流某个节点清除数据流所有节点数据
* Arguments  : *entry		数据流节点句柄
* Return	 : None.
* Note(s)    : 先找到数据流起始位置，再调用audio_stream_clear()清除所有节点
*********************************************************************
*/
void audio_stream_clear_by_entry(struct audio_stream_entry *entry);


/*
*********************************************************************
*                  Audio Stream Check Active From Entry
* Description: 检测指定节点及后续节点是否有活动情况
* Arguments  : *entry		数据流节点句柄
* Return	 : true 		活动
* Note(s)    : 只要中间有某个节点活动就会返回数据流活动
*********************************************************************
*/
int audio_stream_check_active_from(struct audio_stream_entry *entry);


/*
*********************************************************************
*                  Audio Stream Close
* Description: 关闭数据流
* Arguments  : *stream		数据流句柄
* Return	 : None.
* Note(s)    : 需关闭所有数据流节点之后才能调用该函数
*********************************************************************
*/
void audio_stream_close(struct audio_stream *stream);

#endif





