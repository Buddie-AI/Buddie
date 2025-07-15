#ifndef AUDIO_ENCODER_H
#define AUDIO_ENCODER_H

#include "generic/typedef.h"
#include "generic/list.h"
#include "generic/circular_buf.h"
#include "media/audio_base.h"



enum {
    AUDIO_ENC_EVENT_CURR_TIME = 0x20,
    AUDIO_ENC_EVENT_END,
    AUDIO_ENC_EVENT_ERR,
    AUDIO_ENC_EVENT_SUSPEND,
    AUDIO_ENC_EVENT_RESUME,
};


struct audio_encoder_task {
    struct list_head head;
    const char *name;
};


struct audio_encoder;


struct audio_enc_input {
    int (*fget)(struct audio_encoder *, s16 **frame, u16 frame_len);
    void (*fput)(struct audio_encoder *, s16 *frame);
};

struct audio_enc_handler {
    int (*enc_probe)(struct audio_encoder *);
    int (*enc_output)(struct audio_encoder *, u8 *data, int len);
    int (*enc_post)(struct audio_encoder *);
    int (*enc_close)(struct audio_encoder *);//add by wuxu 20200221
};

struct audio_encoder_ops {
    u32 coding_type;
    void *(*open)(void *priv);
    int (*start)(void *);
    int (*set_fmt)(void *, struct audio_fmt *fmt);
    int (*run)(void *);
    int (*stop)(void *);
    int (*close)(void *);
    int (*ioctrl)(void *, int argc, int argv[]);
};

#define REGISTER_AUDIO_ENCODER(ops) \
        const struct audio_encoder_ops ops SEC(.audio_encoder)

extern const struct audio_encoder_ops audio_encoder_begin[];
extern const struct audio_encoder_ops audio_encoder_end[];

#define list_for_each_audio_encoder(p) \
    for (p = audio_encoder_begin; p < audio_encoder_end; p++)


#define AUDIO_ENC_OS_MUTEX_EN     1

#if (AUDIO_ENC_OS_MUTEX_EN)
#include "os/os_api.h"
#endif//AUDIO_ENC_OS_MUTEX_EN

struct audio_encoder {
    struct list_head entry;
    struct audio_encoder_task *task;
    struct audio_fmt fmt;
    const char *evt_owner;
    const struct audio_enc_input *input;
    const struct audio_encoder_ops *enc_ops;
    const struct audio_enc_handler *enc_handler;
    void (*evt_handler)(struct audio_encoder *enc, int, int *);
    void *enc_priv;
    u32 pend_timeout;
    s16 *pcm_frame;
    u16 pcm_remain;
    u16 pcm_len;
    s16 *output_buffs;
    u16 output_buff_size;
    u8 output_buff_num;
    u8 curr_output_buff;
    u8 output_channel;
    u8 state;
    u8 err;
    volatile u8 resume_flag;
    u32 magic;
#if (AUDIO_ENC_OS_MUTEX_EN)
    OS_MUTEX mutex;
#endif//AUDIO_ENC_OS_MUTEX_EN
};

enum {
    AUDIO_ENCODER_IOCTRL_CMD_GET_HEAD_INFO = 0x0,
    AUDIO_ENCODER_IOCTRL_CMD_GET_TIME,
    AUDIO_ENCODER_IOCTRL_CMD_GET_TMARK,//书签
};

int audio_encoder_task_create(struct audio_encoder_task *task, const char *name);

int audio_encoder_task_del(struct audio_encoder_task *task);

int audio_encoder_resume_all(struct audio_encoder_task *task);

int audio_encoder_get_output_buff(void *_enc, s16 **buf);

int audio_encoder_put_output_buff(void *_enc, void *buff, int len);

int audio_encoder_get_frame(void *_enc, s16 **frame, u16 len);

void audio_encoder_put_frame(void *_enc, s16 *frame);

int audio_encoder_open(struct audio_encoder *enc, const struct audio_enc_input *input,
                       struct audio_encoder_task *task);

int audio_encoder_get_fmt(struct audio_encoder *enc, struct audio_fmt **fmt);

int audio_encoder_set_fmt(struct audio_encoder *enc, struct audio_fmt *fmt);

void audio_encoder_set_handler(struct audio_encoder *enc, const struct audio_enc_handler *handler);


void audio_encoder_set_event_handler(struct audio_encoder *enc,
                                     void (*handler)(struct audio_encoder *, int, int *), u32 maigc);

void audio_encoder_set_input_buff(struct audio_encoder *enc, u8 *buff, u16 buff_size);

void audio_encoder_set_output_buffs(struct audio_encoder *enc, s16 *buffs,
                                    u16 buff_size, u8 buff_num);

int audio_encoder_set_output_channel(struct audio_encoder *enc, enum audio_channel);

int audio_encoder_start(struct audio_encoder *enc);

int audio_encoder_stop(struct audio_encoder *enc);

int audio_encoder_suspend(struct audio_encoder *enc, int timeout_ms);

int audio_encoder_resume(struct audio_encoder *enc);

int audio_encoder_close(struct audio_encoder *enc);

int audio_encoder_ioctrl(struct audio_encoder *enc, int argc, ...);

/*
*********************************************************************
*                  Audio Encoder Get channel
* Description: 获取编码器声道配置参数
* Arguments  : *enc 	编码句柄
* Return	 : 声道数
* Note(s)    : None
*********************************************************************
*/
int audio_encoder_get_channel(struct audio_encoder *enc);

#endif

