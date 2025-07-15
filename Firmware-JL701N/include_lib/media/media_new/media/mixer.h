#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include "generic/typedef.h"
#include "generic/list.h"

#define MIXER_EXT_MAX_NUM		4	// 扩展输出最大通道
#define PCM_0dB_VALUE           16384
enum {
    MIXER_EVENT_CH_OPEN,
    MIXER_EVENT_CH_CLOSE,
    MIXER_EVENT_CH_RESET,
};
#define BIT16_MODE  0
#define BIT24_MODE  BIT(0)
#define BIT32_MODE  BIT(1)

struct audio_mixer;

struct audio_mix_handler {
    int (*mix_probe)(struct audio_mixer *);
    int (*mix_output)(struct audio_mixer *, s16 *, u16);
#if MIXER_EXT_MAX_NUM
    /* 扩展输出回调接口
     * 最后一个参数代表当前输出的通道序号，0到ext_output_num-1;
     * 扩展输出不检测返回值，每次输出的长度与mix_output的返回值相同;
     */
    int (*mix_output_ext)(struct audio_mixer *, s16 *, u16, u8);
#endif
    int (*mix_post)(struct audio_mixer *);
};

struct audio_mixer {
    struct list_head head;
    s16 *output;
    u16 points;
    u16 remain_points;
    u32 sample_position : 20;
#if MIXER_EXT_MAX_NUM
    int ext_output_addr[MIXER_EXT_MAX_NUM];	// 扩展输出buf地址，每个buf的长度与point相同
    u8  ext_output_num;	// 扩展输出通道总数
#endif
    const struct audio_mix_handler *mix_handler;
    void (*evt_handler)(struct audio_mixer *, int);
    volatile u8 active;
    volatile u8 bit_mode_en;
    volatile u8 point_len;
};

struct audio_pcm_edit {
    u8  hide : 1;
    u8  highlight : 1;
    u8  state;
    u8  ch_num;
    u8  fade_chs;
    s16 fade_step;
    s16 fade_volume;
    s16 volume;
};

struct audio_mixer_ch {
    u8 start;
    u8 pause;
    u8 open;
    u32 no_wait : 1;	// 不等待有数
    u32 lose : 1;		// 丢数标记
    u32 need_resume : 1;
    u8 start_by_position;
    u16 offset;
    u32 sample_rate;
    u16 lose_time;		// 超过该时间还没有数据，则以为可以丢数。no_wait置1有效
    unsigned long lose_limit_time;	// 丢数超时中间运算变量
    struct list_head entry;
    struct audio_mixer *mixer;
#if MIXER_EXT_MAX_NUM
    u32 ext_out_mask;	// 标记该通道输出的扩展通道，如输出到第0和第2个，ext_flag |= BTI(0)|BIT(2)
    u8  main_out_dis;	// 不输出到主通道
#endif
    u32 slience_samples;
    u32 mix_timeout;
    u32 starting_position : 20;
    void *priv;
    void (*event_handler)(void *priv, int event);
    void *lose_priv;
    void (*lose_callback)(void *lose_priv, int lose_len);
    void *resume_data;
    void (*resume_callback)(void *);
    struct audio_pcm_edit *editor;
};


int audio_mixer_open(struct audio_mixer *mixer);

void audio_mixer_set_handler(struct audio_mixer *, const struct audio_mix_handler *);

void audio_mixer_set_event_handler(struct audio_mixer *mixer,
                                   void (*handler)(struct audio_mixer *, int));

void audio_mixer_set_output_buf(struct audio_mixer *mixer, s16 *buf, u16 len);

int audio_mixer_get_sample_rate(struct audio_mixer *mixer);

int audio_mixer_get_ch_num(struct audio_mixer *mixer);

int audio_mixer_ch_open(struct audio_mixer_ch *ch, struct audio_mixer *mixer);

void audio_mixer_ch_set_sample_rate(struct audio_mixer_ch *ch, u32 sample_rate);

int audio_mixer_reset(struct audio_mixer_ch *ch, struct audio_mixer *mixer);

int audio_mixer_ch_reset(struct audio_mixer_ch *ch);

int audio_mixer_ch_write(struct audio_mixer_ch *ch, s16 *data, int len);

int audio_mixer_ch_close(struct audio_mixer_ch *ch);

void audio_mixer_ch_pause(struct audio_mixer_ch *ch, u8 pause);

int audio_mixer_ch_data_len(struct audio_mixer_ch *ch);

int audio_mixer_ch_add_slience_samples(struct audio_mixer_ch *ch, int samples);

void audio_mixer_ch_set_resume_handler(struct audio_mixer_ch *ch, void *priv, void (*resume)(void *));

int audio_mixer_get_active_ch_num(struct audio_mixer *mixer);

void audio_mixer_ch_set_event_handler(struct audio_mixer_ch *ch, void *priv, void (*handler)(void *, int));

// 设置通道没数据时不等待（超时直接丢数）
void audio_mixer_ch_set_no_wait(struct audio_mixer_ch *ch, void *lose_priv, void (*lose_cb)(void *, int), u8 no_wait, u16 time_ms);

#if MIXER_EXT_MAX_NUM

void audio_mixer_set_ext_output_buf(struct audio_mixer *mixer, int *buf_addr_lst, u8 ext_num);

u32 audio_mixer_ch_get_ext_out_mask(struct audio_mixer_ch *ch);
void audio_mixer_ch_set_ext_out_mask(struct audio_mixer_ch *ch, u32 mask);
void audio_mixer_ch_main_out_disable(struct audio_mixer_ch *ch, u8 disable);

#endif /* MIXER_EXT_MAX_NUM */


int audio_mixer_get_output_buf_len(struct audio_mixer *mixer);

int audio_mixer_ch_set_starting_position(struct audio_mixer_ch *ch, u32 postion, int timeout);

void audio_mixer_position_correct(struct audio_mixer *ch, int diff);

u32 audio_mixer_get_input_position(struct audio_mixer *mixer);

int audio_mixer_get_start_ch_num(struct audio_mixer *mixer);

void audio_mixer_set_mode(struct audio_mixer *mixer, u8 point_len, u8 bit_mode_en);

int audio_mixer_ch_sound_highlight(struct audio_mixer_ch *ch, int hide_volume, int fade_frames, u8 data_channels);




#endif

