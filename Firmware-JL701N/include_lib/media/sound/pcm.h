/*****************************************************************
>file name : pcm.h
>create time : Mon 18 Jan 2021 02:04:15 PM CST
*****************************************************************/
#ifndef _SOUND_PCM_H_
#define _SOUND_PCM_H_
#include "audio_cfifo.h"
#include "os/os_api.h"

struct sound_pcm_hardware {
    unsigned int info;		/* SOUND_PCM_INFO_* */
    unsigned int formats;			/* SOUND_PCM_FMTBIT_* */
    unsigned int rates;		/* SOUND_PCM_RATE_* */
    unsigned int rate_min;		/* min rate */
    unsigned int rate_max;		/* max rate */
    unsigned int channels_min;	/* min channels */
    unsigned int channels_max;	/* max channels */
    int buffer_bytes_max;	/* max buffer size */
    int period_bytes_min;	/* min period size */
    int period_bytes_max;	/* max period size */
    unsigned int periods_min;	/* min # of periods */
    unsigned int periods_max;	/* max # of periods */
    int fifo_size;		/* fifo size in bytes */
};

struct sound_pcm_substream;
struct sound_pcm_hw_params;

/*****************************************************************************/
/* SOUND PCM逻辑设备错误返回值*/
#define ESNDPCM_NOMEM               1       /* PCM设备申请不到内存 */
#define ESNDPCM_NODEV               2       /* 找不到PCM设备 */
#define ESNDPCM_INVAL               3       /* 无效的设备平台(未初始化) */
#define ESNDPCM_UNKNOWN_RATE        4       /* PCM设备不支持的采样率 */
#define ESNDPCM_CONFLICT_RATE       5       /* PCM设备与设置采样率冲突 */
#define ESNDPCM_NOFIFO              6       /* PCM设备中没有设置runtime的fifo */

/*****************************************************************************/

#define SOUND_PCM_TRIGGER_START     1       /* 触发DMA/CODEC开启 */
#define SOUND_PCM_TRIGGER_STOP      2       /* 触发DMA/CODEC停止 */
#define SOUND_PCM_TRIGGER_IRQ       3       /* 触发DMA下一个中断外传 */

#define SOUND_PCM_GET_HW_PARAMS             _IOR('P', 1, sizeof(struct sound_pcm_hw_params)) /* Get硬件参数 */
#define SOUND_PCM_SET_HW_PARAMS             _IOW('P', 2, sizeof(struct sound_pcm_hw_params)) /* Set硬件参数 */
#define SOUND_PCM_GET_HW_BUFFERED_LEN       _IOR('P', 3, sizeof(unsigned int)) /* Get硬件参数 */
#define SOUND_PCM_WAIT_SWN_MOVE             _IOR('P', 4, sizeof(unsigned int)) /* 等待dac swn 输出一个点后，再往后走硬件参数 */
#define SOUND_PCM_GET_HW_HRP                _IOR('P', 5, sizeof(unsigned int)) /* Get硬件读指针 */

enum pcm_state {
    SOUND_PCM_STATE_IDLE = 0,               /* PCM设备空闲 */
    SOUND_PCM_STATE_PREPARED,               /* PCM设备硬件准备就绪 */
    SOUND_PCM_STATE_RUNNING,                /* PCM设备DMA/CODEC运行中 */
    SOUND_PCM_STATE_SUSPENDED,              /* PCM设备挂起 */
};

/*
 * Sound PCM设备支持标准sample rate定义
 * */
#define SOUND_PCM_RATE_8000		    (1<<0)		/* 8000Hz */
#define SOUND_PCM_RATE_11025		(1<<1)		/* 11025Hz */
#define SOUND_PCM_RATE_12000		(1<<2)		/* 12000Hz */
#define SOUND_PCM_RATE_16000		(1<<3)		/* 16000Hz */
#define SOUND_PCM_RATE_22050		(1<<4)		/* 22050Hz */
#define SOUND_PCM_RATE_24000		(1<<5)		/* 24000Hz */
#define SOUND_PCM_RATE_32000		(1<<6)		/* 32000Hz */
#define SOUND_PCM_RATE_44100		(1<<7)		/* 44100Hz */
#define SOUND_PCM_RATE_48000		(1<<8)		/* 48000Hz */
#define SOUND_PCM_RATE_64000		(1<<9)		/* 64000Hz */
#define SOUND_PCM_RATE_88200		(1<<10)		/* 88200Hz */
#define SOUND_PCM_RATE_96000		(1<<11)		/* 96000Hz */
#define SOUND_PCM_RATE_128000		(1<<12)		/* 128000Hz */
#define SOUND_PCM_RATE_176400		(1<<13)		/* 176400Hz */
#define SOUND_PCM_RATE_192000		(1<<14)		/* 192000Hz */
#define SOUND_PCM_RATE_UNKNOWN      (1<<15)     /* Unknown sample rate */

#define SOUND_PCM_RATE_8000_44100	(SOUND_PCM_RATE_8000|SOUND_PCM_RATE_11025|SOUND_PCM_RATE_12000|\
					 SOUND_PCM_RATE_16000|SOUND_PCM_RATE_22050|SOUND_PCM_RATE_24000|\
					 SOUND_PCM_RATE_32000|SOUND_PCM_RATE_44100)
#define SOUND_PCM_RATE_8000_48000	(SOUND_PCM_RATE_8000_44100|SOUND_PCM_RATE_48000)
#define SOUND_PCM_RATE_8000_96000	(SOUND_PCM_RATE_8000_48000|SOUND_PCM_RATE_64000|\
					 SOUND_PCM_RATE_88200|SOUND_PCM_RATE_96000)
#define SOUND_PCM_RATE_8000_192000	(SOUND_PCM_RATE_8000_96000|SOUND_PCM_RATE_128000|SOUND_PCM_RATE_176400|\
					 SOUND_PCM_RATE_192000)

/*
 * PCM设备与硬件DMA同步flag
 */
#define DMA_SYNC_R          (1 << 0)                    /* 读同步 */
#define DMA_SYNC_W          (1 << 1)                    /* 写同步 */
#define DMA_SYNC_RW         (DMA_SYNC_R | DMA_SYNC_W)   /* 读写同步 */

/*
 * Sound运行时间单位
 */
#define SOUND_TIME_MS       0
#define SOUND_TIME_US       1


/*
 * PCM设备操作函数
 */
struct sound_pcm_ops {
    int (*open)(struct sound_pcm_substream *substream);
    int (*close)(struct sound_pcm_substream *substream);
    int (*ioctl)(struct sound_pcm_substream *substream,
                 unsigned int cmd, void *arg);
    int (*prepare)(struct sound_pcm_substream *substream);
    int (*trigger)(struct sound_pcm_substream *substream, int cmd);
    int (*pointer)(struct sound_pcm_substream *substream);
    int (*silence)(struct sound_pcm_substream *substream, int channel, u32 pos, u32 count);
};


struct sound_pcm_hw_params {
    u32 rates;
    int sample_rate;
    u8  sample_bits;
    u8  channels;
};

struct sound_dma_buffer {
    u8 mode;
    void *addr;
    s16  size;
    struct audio_cfifo fifo;
};

struct sound_pcm_map_status {
    u8 state;
    u8 ref_count;
};

struct sound_pcm_map_fifo {
    void *addr;                         /* FIFO地址 */
    u16 bytes;                          /* FIFO大小(bytes) */
    u16 frame_len;                      /* FIFO帧总长度(bytes / ch / 2) */
    struct audio_cfifo cfifo;           /* FIFO结构实体 */
    void (*sync)(struct sound_pcm_substream *substream, u8 flag);    /* FIFO同步 */
};

struct sound_pcm_runtime {
    /*fifo*/
    u32 hw_ptr;                         /* 当前硬件位置 */
    u32 sw_ptr;                         /* 当前软件位置 */
    u32 hw_ptr_jiffies;                 /* 当前硬件位置对应的时间 */
    u32 dma_irq_ptr;                    /* 设置的dma到达该ptr起中断 */
    u32(*sound_current_time)(void);     /* 当前采样时间获取 */
    struct sound_pcm_map_fifo *fifo;
    u8 time_type;                       /* 采样指针位置的时钟类型 */

    /* -- HW params -- */
    u8  channels;		/* 采样通道 */
    u8  sample_bits;
    u16 period_size;	/* 采样周期块大小(pingpong buffer模式) */
    u32 periods;		/* 采样周期个数 */
    int sample_rate;    /* sample_rate */

    /* -- SW params -- */
    u8 run_mode;            /*OVERRUN或非OVERRUN*/
    //u32 start_threshold;    [>启动DMA的阈值<]
    //u32 stop_threshold;     [>停止DMA的阈值<]
    //u32 silence_threshold;  [>静音数据阈值 <]
    //u32 silence_size;	    [>填充静音数据的大小 <]

    u32 silence_start;      /* 静音起始位置 */
    u32 silence_filled;     /* 已填充的静音数据 */
    int buffer_delay;

    struct sound_pcm_map_status *status; /*映射硬件中的状态*/

    /* -- DMA -- */
    struct sound_dma_buffer *dma;	/* DMA缓冲 */

    OS_MUTEX *mutex;
};


struct sound_pcm_stream;
struct sound_pcm_substream {
    struct sound_pcm_stream *stream;
    struct sound_pcm_runtime *runtime; /* 数据流运行结构 */
    struct audio_cfifo_channel fifo;   /* fifo缓冲 */
    void *private_data;
    struct list_head entry;
    void *irq_priv;
    void (*irq_handler)(void *priv);
    u8 direction;                      /* 数据流方向 */
    unsigned int hw_opened: 1;
};

struct sound_pcm_stream {
    struct sound_pcm_substream *substream;              /* 与设备共享的子数据流 */
    const struct sound_pcm_ops *ops;                          /* PCM操作函数 */
    void *platform_device;
    void *syncts;
};


/*************************************************************************
 * 采样率转换为bit表示
 * Input    :  sample_rate - 采样率
 * Output   :  采样率的bit表示
 * Notes    :
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_match_standard_rate(int sample_rate);

/*************************************************************************
 * 创建sound pcm数据流
 * Input    :  stream  - sound_pcm_stream二级指针，
 *             name    - 设备名，
 *             direction - 数据流方向.
 * Output   :  0 - 创建成功，非0 - 失败.
 * Notes    :
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_create(struct sound_pcm_stream **stream, const char *name, u8 direction);

/*************************************************************************
 * 设置pcm设备中断处理函数
 * Input    :  stream  - sound_pcm_stream指针，
 *             priv    - 回调私有数据，
 *             handler - 回调处理函数.
 * Output   :  无.
 * Notes    :  pingpong模式可以用来处理音频采样定时，
 *             也可以使用中断处理唤醒等功能。
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
void sound_pcm_set_irq_handler(struct sound_pcm_stream *stream, void *priv, void (*handler)(void *));

/*************************************************************************
 * pcm设备数据流准备
 * Input    :  stream  - sound_pcm_stream指针，
 *             sample_rate  - 设备采样率，
 *             delay - 设备缓冲延时，
 *             mode  - 数据流overrun/block模式选择.
 * Output   :  0 - 成功，非0 - 失败.
 * Notes    :
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_prepare(struct sound_pcm_stream *stream, int sample_rate, int delay, int mode);

/*************************************************************************
 * pcm设备数据流开启采样
 * Input    :  stream  - sound_pcm_stream指针.
 * Output   :  0 - 成功，非0 - 失败.
 * Notes    :
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_start(struct sound_pcm_stream *stream);

/*************************************************************************
 * pcm设备数据流停止采样
 * Input    :  stream  - sound_pcm_stream指针.
 * Output   :  0 - 成功，非0 - 失败.
 * Notes    :
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_stop(struct sound_pcm_stream *stream);

/*************************************************************************
 * pcm设备数据流触发一次中断
 * Input    :  stream  - sound_pcm_stream指针,
 *             time_unit - 设置中断起来时间的单位，
 *             time      - 设置多少时间后起来（临近）.
 * Output   :  0 - 成功，非0 - 失败.
 * Notes    :
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_trigger_interrupt(struct sound_pcm_stream *stream, u8 time_unit, int time);

/*************************************************************************
 * pcm设备数据子流中断处理函数
 * Input    :  substream  - sound_pcm_substream指针.
 * Output   :  0 - 成功，非0 - 失败.
 * Notes    :  一般由设备的DMA中断调用该函数进行中断分配和更新DMA信息.
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_substream_irq_handler(struct sound_pcm_substream *substream);

/*************************************************************************
 * pcm设备设置数据声道分布
 * Input    :  stream  - sound_pcm_stream指针,
 *             map     - 声道分布.
 * Output   :  0 - 成功，非0 - 失败.
 * Notes    :
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_set_data_map(struct sound_pcm_stream *stream, u16 map);

/*************************************************************************
 * pcm设备写入数据
 * Input    :  stream  - sound_pcm_stream指针,
 *             data    - 数字音频数据,
 *             len     - 数据长度.
 * Output   :  实际写入的长度.
 * Notes    :
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_write(struct sound_pcm_stream *stream, void *data, int len);

/*************************************************************************
 * pcm设备io控制函数
 * Input    :  stream  - sound_pcm_stream指针,
 *             cmd     - 命令,
 *             arg     - 参数.
 * Output   :  0 - 成功，非0 - 失败.
 * Notes    :  PCM数据流的ioctl函数(数字音频相关)
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_iotcl(struct sound_pcm_stream *stream, int cmd, void *arg);

/*************************************************************************
 * pcm设备控制器io控制函数
 * Input    :  stream  - sound_pcm_stream指针,
 *             cmd     - 命令,
 *             arg     - 参数.
 * Output   :  0 - 成功，非0 - 失败.
 * Notes    :  PCM设备的控制器ioctl函数(增益/电源等相关)
 * History  :  2021/03/11 by Lichao.
 *=======================================================================*/
int sound_pcm_ctl_ioctl(struct sound_pcm_stream *stream, int cmd, void *arg);
int sound_pcm_read(struct sound_pcm_stream *stream, void *data, int len);
void sound_pcm_free(struct sound_pcm_stream *stream);


void sound_pcm_set_syncts(struct sound_pcm_stream *stream, void *syncts);

#endif
