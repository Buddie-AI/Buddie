#ifndef _TONE_PLAYER_API_H
#define _TONE_PLAYER_API_H

#include "audio_config.h"
#include "sine_make.h"

#define TONE_STOP       0
#define TONE_START      1

#define CONFIG_USE_DEFAULT_SINE     1
#define DEVICE_EVENT_FROM_TONE		(('T' << 24) | ('N' << 16) | ('E' << 8) | '\0')

#define TONE_DEFAULT_VOL   SYS_MAX_VOL



#define TONE_REPEAT_BEGIN(a)  (char *)((0x1 << 30) | (a & 0xffff))
#define TONE_REPEAT_END()     (char *)(0x2 << 30)

#define IS_REPEAT_BEGIN(a)    ((((u32)a >> 30) & 0x3) == 0x1 ? 1 : 0)
#define IS_REPEAT_END(a)      ((((u32)a >> 30) & 0x3) == 0x2 ? 1 : 0)
#define TONE_REPEAT_COUNT(a)  (((u32)a) & 0xffff)

#define DEFAULT_SINE_TONE(a)     (char *)(((u32)0x3 << 30) | (a))
#define IS_DEFAULT_SINE(a)       ((((u32)a >> 30) & 0x3) == 0x3 ? 1 : 0)
#define DEFAULT_SINE_ID(a)       ((u32)a & 0xffff)

typedef const struct sin_param *(*get_sine_param_t)(u8 id, u8 *num);

void tone_play_set_sine_param_handler(get_sine_param_t handler);


int tone_play(const char *name, u8 preemption) ;

int tone_play_no_tws(const char *name, u8 preemption);

int tone_play_with_callback(const char *name, u8 preemption, void (*user_evt_handler)(void *priv), void *priv);

int tone_file_list_play(const char **list, u8 preemption);

int tone_play_stop(void);

int tone_sin_play(int time_ms, u8 wait);

int tone_get_status();

int tone_file_list_stop(u8 no_end);

/*
 *@brief:提示音比较，确认目标提示音和正在播放的提示音是否一致
 *@return: 0 	匹配
 *		   非0	不匹配或者当前没有提示音播放
 *@note:通过提示音名字比较
 */
int tone_name_compare(const char *name);


enum {
    TONE_PLAY_END_CB_CMD_NONE = -1,
    TONE_FLAG_KEY_START,
    TONE_FLAG_KEY_START_UP_CLK,
};



#endif
