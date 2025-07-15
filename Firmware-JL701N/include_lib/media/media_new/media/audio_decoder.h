#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include "generic/typedef.h"
#include "generic/list.h"
#include "media/audio_base.h"
#include "os/os_api.h"



enum {
    AUDIO_DEC_EVENT_CURR_TIME = 0x20,
    AUDIO_DEC_EVENT_END,
    AUDIO_DEC_EVENT_ERR,
    AUDIO_DEC_EVENT_SUSPEND,
    AUDIO_DEC_EVENT_RESUME,
    AUDIO_DEC_EVENT_START,
};

enum {
    AUDIO_PLAY_EVENT_CURR_TIME = 0x20,
    AUDIO_PLAY_EVENT_END,
    AUDIO_PLAY_EVENT_ERR,
    AUDIO_PLAY_EVENT_SUSPEND,
    AUDIO_PLAY_EVENT_RESUME,
};

enum {
    AUDIO_RES_GET,
    AUDIO_RES_PUT,
};

enum {
    DEC_STA_TRY_START,
    DEC_STA_START,
    DEC_STA_WAIT_STOP,
    DEC_STA_WAIT_SUSPEND,
    DEC_STA_WAIT_RESUME,
    DEC_STA_WAIT_PAUSE,
};

enum {
    AUDIO_IOCTRL_CMD_REPEAT_PLAY = 0x90,
};

struct fixphase_repair_obj {
    short fifo_buf[18 + 12][32][2];
};

struct audio_repeat_mode_param {
    int flag;
    int headcut_frame;
    int tailcut_frame;
    int (*repeat_callback)(void *);
    void *callback_priv;
    struct fixphase_repair_obj *repair_buf;
};

struct audio_res_wait {
    struct list_head entry;
    u8 priority;
    u8 preemption : 1;
    u8 protect : 1;
    u8 only_del : 1; // 仅删除
    u8 snatch_same_prio : 1; // 优先级相同也抢断
    u32 format;
    int (*handler)(struct audio_res_wait *, int event);
};

struct audio_decoder_task {
    struct list_head head;
    struct list_head wait;
    const char *name;
    int wakeup_timer;
    int fmt_lock;
    OS_SEM sem;
};


struct audio_dec_breakpoint {
    int len;
    u32 fptr;
    int data_len;
    u8 data[0];
    // u8 data[128];
};

struct audio_decoder;


struct audio_dec_input {
    u32 coding_type;
    // 定义在p_more_coding_type数组中的解码器会依照顺序依次检测
    // 先检测coding_type再检测p_more_coding_type
    u32 *p_more_coding_type;
    u32 data_type : 8;
    union {
        struct {
            int (*fread)(struct audio_decoder *, void *buf, u32 len);
            int (*fseek)(struct audio_decoder *, u32 offset, int seek_mode);
            int (*ftell)(struct audio_decoder *);
            int (*flen)(struct audio_decoder *);
        } file;

        struct {
            int (*fget)(struct audio_decoder *, u8 **frame);
            void (*fput)(struct audio_decoder *, u8 *frame);
            int (*ffetch)(struct audio_decoder *, u8 **frame);
        } frame;
    } ops;
};

struct audio_dec_handler {
    int (*dec_probe)(struct audio_decoder *);
    int (*dec_output)(struct audio_decoder *, s16 *data, int len, void *priv);
    int (*dec_post)(struct audio_decoder *);
    int (*dec_stop)(struct audio_decoder *);
};
struct stream_codec_info {
    int  time;
    int  frame_num;
    u32  frame_len;
    int  frame_points;
    int  sequence_number;
    u32  sample_rate;
    u8   channel;
};

/*! \brief      音频解码器抽象接口 */
struct audio_decoder_ops {
    u32 coding_type;            /*!<  解码格式*/
    void *(*open)(void *priv);  /*!<  */
    int (*start)(void *);       /*!<  */
    int (*get_fmt)(void *, struct audio_fmt *fmt);
    int (*set_output_channel)(void *, enum audio_channel);
    int (*get_play_time)(void *);
    int (*fast_forward)(void *, int step_s);
    int (*fast_rewind)(void *, int step_s);
    int (*get_breakpoint)(void *, struct audio_dec_breakpoint *);
    int (*set_breakpoint)(void *, struct audio_dec_breakpoint *);
    int (*stream_info_scan)(void *, struct stream_codec_info *info, void *data, int len);
    int (*set_tws_mode)(void *, int);
    int (*run)(void *, u8 *);
    int (*stop)(void *);
    int (*close)(void *);
    int (*reset)(void *);
    int (*ioctrl)(void *, u32 cmd, void *parm);
};

#define REGISTER_AUDIO_DECODER(ops) \
        const struct audio_decoder_ops ops SEC(.audio_decoder)

extern const struct audio_decoder_ops audio_decoder_begin[];
extern const struct audio_decoder_ops audio_decoder_end[];

#define list_for_each_audio_decoder(p) \
    for (p = audio_decoder_begin; p < audio_decoder_end; p++)




struct audio_decoder {
    struct list_head entry;
    struct audio_decoder_task *task;
    struct audio_fmt fmt;
    const char *evt_owner;
    const struct audio_dec_input *input;
    const struct audio_decoder_ops *dec_ops;
    const struct audio_dec_handler *dec_handler;
    void (*evt_handler)(struct audio_decoder *dec, int, int *);
    void *dec_priv;
    void *bp;
    u16 id;
    u16 pick : 1;
    u16 tws : 1;
    u16 resume_flag : 1;
    u16 output_err : 1;
    u16 read_err : 1;
    u16 reserved : 11;
    u8 run_max;
    u8 output_channel;
    u8 state;
    u8 err;
    u8 remain;
    u32 magic;
};

#define AUDIO_DEC_ORIG_CH       AUDIO_CH_LR
#define AUDIO_DEC_L_CH          AUDIO_CH_L
#define AUDIO_DEC_R_CH          AUDIO_CH_R
#define AUDIO_DEC_MONO_LR_CH    AUDIO_CH_DIFF
#define AUDIO_DEC_DUAL_L_CH     AUDIO_CH_DUAL_L
#define AUDIO_DEC_DUAL_R_CH     AUDIO_CH_DUAL_R
#define AUDIO_DEC_DUAL_LR_CH    AUDIO_CH_DUAL_LR

#define AUDIO_DEC_IS_MONO(ch)	(((ch)==AUDIO_DEC_L_CH) || ((ch)==AUDIO_DEC_R_CH) || ((ch)==AUDIO_DEC_MONO_LR_CH))

int audio_decoder_task_create(struct audio_decoder_task *task, const char *name);

int audio_decoder_task_add_wait(struct audio_decoder_task *, struct audio_res_wait *);

void audio_decoder_task_del_wait(struct audio_decoder_task *, struct audio_res_wait *);
int audio_decoder_task_wait_state(struct audio_decoder_task *task);

int audio_decoder_resume_all(struct audio_decoder_task *task);
int audio_decoder_resume_off_limits(struct audio_decoder_task *task, u8 limit_num, int *limit_dec);

int audio_decoder_resume_all_by_sem(struct audio_decoder_task *task, int time_out);

int audio_decoder_fmt_lock(struct audio_decoder_task *task, int fmt);

int audio_decoder_fmt_unlock(struct audio_decoder_task *task, int fmt);

void *audio_decoder_get_output_buff(void *_dec, int *len);

int audio_decoder_put_output_buff(void *_dec, void *buff, int len, void *priv);

int audio_decoder_read_data(void *_dec, u8 *data, int len, u32 offset);

int audio_decoder_get_input_data_len(void *_dec);

int audio_decoder_get_frame(void *_dec, u8 **frame);

int audio_decoder_fetch_frame(void *_dec, u8 **frame);

void audio_decoder_put_frame(void *_dec, u8 *frame);

int audio_fmt_find_frame(void *_dec, u8 **frame);
int audio_decoder_open(struct audio_decoder *dec, const struct audio_dec_input *input,
                       struct audio_decoder_task *task);

int audio_decoder_data_type(void *_dec);

void audio_decoder_set_id(struct audio_decoder *dec, int);

int audio_decoder_get_fmt(struct audio_decoder *dec, struct audio_fmt **fmt);

int audio_decoder_set_fmt(struct audio_decoder *dec, struct audio_fmt *fmt);

int audio_decoder_get_fmt_info(struct audio_decoder *dec, struct audio_fmt *fmt);

void audio_decoder_set_handler(struct audio_decoder *dec, const struct audio_dec_handler *handler);


void audio_decoder_set_event_handler(struct audio_decoder *dec,
                                     void (*handler)(struct audio_decoder *, int, int *), u32 magic);

void audio_decoder_set_input_buff(struct audio_decoder *dec, u8 *buff, u16 buff_size);

void audio_decoder_set_output_buffs(struct audio_decoder *dec, s16 *buffs,
                                    u16 buff_size, u8 buff_num);

int audio_decoder_set_output_channel(struct audio_decoder *dec, enum audio_channel);

int audio_decoder_start(struct audio_decoder *dec);

int audio_decoder_stop(struct audio_decoder *dec);

int audio_decoder_pause(struct audio_decoder *dec);

int audio_decoder_suspend(struct audio_decoder *dec, int timeout_ms);

int audio_decoder_resume(struct audio_decoder *dec);

int audio_decoder_close(struct audio_decoder *dec);
int audio_decoder_reset(struct audio_decoder *dec);

int audio_decoder_set_breakpoint(struct audio_decoder *dec, struct audio_dec_breakpoint *bp);

int audio_decoder_get_breakpoint(struct audio_decoder *dec, struct audio_dec_breakpoint *bp);

int audio_decoder_forward(struct audio_decoder *dec, int step_s);

int audio_decoder_rewind(struct audio_decoder *dec, int step_s);

int audio_decoder_get_total_time(struct audio_decoder *dec);
int audio_decoder_get_play_time(struct audio_decoder *dec);

int audio_decoder_set_pick_stu(struct audio_decoder *dec, u8 pick);
int audio_decoder_get_pick_stu(struct audio_decoder *dec);

int audio_decoder_set_tws_stu(struct audio_decoder *dec, u8 tws);
int audio_decoder_get_tws_stu(struct audio_decoder *dec);

int audio_decoder_set_run_max(struct audio_decoder *dec, u8 run_max);

void audio_decoder_dual_switch(u8 ch_type, u8 half_lr, s16 *data, int len);

int audio_decoder_running_number(struct audio_decoder_task *task);

int audio_decoder_ioctrl(struct audio_decoder *dec, u32 cmd, void *parm);

void audio_decoder_set_channel(void *_dec, u8 ch_num);
int audio_decoder_get_channel(void *_dec);
int audio_decoder_get_frame_len(void *_dec);
int audio_decoder_get_sample_rate(void *_dec);
int audio_decoder_get_bit_rate(void *_dec);
#endif

