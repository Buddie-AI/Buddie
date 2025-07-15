#ifndef AUDIO_AUTOMUTE_H
#define AUDIO_AUTOMUTE_H

#include "generic/typedef.h"
#include "generic/list.h"

#define AUDIO_EVENT_AUTO_MUTE		0x10
#define AUDIO_EVENT_AUTO_UNMUTE		0x11

#define AUTOMUTE_CH    4

enum mute_value {
    AUDIO_MUTE_DEFAULT = 0,
    AUDIO_UNMUTE_DEFAULT,
    AUDIO_MUTE_L_CH,
    AUDIO_UNMUTE_L_CH,
    AUDIO_MUTE_R_CH,
    AUDIO_UNMUTE_R_CH,
};

struct pcm_energy {
    u32 points;        //计算多少个点
    u32 point_count;
    s32 total_value;
    /*void *priv;*/
    /*void (*output)(void *, u16, u8);*/
    s16 average_value;
};

struct automute_filter {
    u32 mute_number;
    u32 unmute_number;
    u32 mute_count;
    u32 unmute_count;
};

struct audio_automute {
    struct pcm_energy energy[AUTOMUTE_CH];
    void (*handler)(u8 event, u8 channel);
    u8  channels;
    u8  pcm_mute;
    u8  mute_channel;
    u8  mute;
    u32 mute_energy;
    u32 unmute_energy;
    u32 filt_points;
    u32 filt_mute_number;
    u32 filt_unmute_number;
    struct automute_filter filt[AUTOMUTE_CH];
    u8 on;
    u8 skip;
};

int audio_automute_open(struct audio_automute *automute);
void audio_automute_run(struct audio_automute *automute, void *data, int len);
void audio_automute_close(struct audio_automute *automute);
int audio_automute_onoff(struct audio_automute *automute, u8 onoff, u8 need_cb);
void audio_automute_skip(struct audio_automute *automute, u8 skip);

#endif
