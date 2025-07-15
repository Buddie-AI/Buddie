#ifndef _AUDIO_DVOL_H_
#define _AUDIO_DVOL_H_

#include "generic/typedef.h"
#include "os/os_type.h"
#include "os/os_api.h"
#include "generic/list.h"

#define BG_DVOL_FADE_ENABLE		1	/*多路声音叠加，背景声音自动淡出小声*/

/*Digital Volume Channel*/
#define MUSIC_DVOL			0xA1
#define CALL_DVOL			0xA2
#define TONE_DVOL			0xA3
#define HEARING_DVOL		0xA4

/*Digital Volume Fade Step*/
#define MUSIC_DVOL_FS		2
#define CALL_DVOL_FS		4
#define TONE_DVOL_FS		30
#define HEARING_DVOL_FS		2

/*Digital Volume Max Level*/
#define MUSIC_DVOL_MAX		16
#define CALL_DVOL_MAX		15
#define TONE_DVOL_MAX		16
#define HEARING_DVOL_MAX	16

typedef struct {
    u8 toggle;					/*数字音量开关*/
    u8 fade;					/*淡入淡出标志*/
    u8 vol;						/*淡入淡出当前音量(level)*/
    u8 vol_max;					/*淡入淡出最大音量(level)*/
    char vol_limit;				/*最大数字音量限制*/
    u8 idx;						/*数字音量通道索引*/
    s16 vol_fade;				/*淡入淡出对应的起始音量*/
#if BG_DVOL_FADE_ENABLE
    s16 vol_bk;					/*后台自动淡出前音量值*/
#endif/*BG_DVOL_FADE_ENABLE*/
    struct list_head entry;
    volatile s16 vol_target;	/*淡入淡出对应的目标音量*/
    volatile u16 fade_step;		/*淡入淡出的步进*/
} dvol_handle;

int audio_digital_vol_init(u16 *vol_table, u16 vol_max);
void audio_digital_vol_bg_fade(u8 fade_out);
dvol_handle *audio_digital_vol_open(u8 dvol_idx, u8 vol, u8 vol_max, u16 fade_step, char vol_limit);
void audio_digital_vol_close(u8 dvol_idx);
void audio_digital_vol_set(u8 dvol_idx, u8 vol);
int audio_digital_vol_get(u8 dvol_idx);
int audio_digital_vol_run(u8 dvol_idx, void *data, u32 len);
void audio_digital_vol_reset_fade(u8 dvol_idx);
void audio_digital_vol_set_no_fade(u8 dvol_idx, u8 vol);

#endif/*_AUDIO_DVOL_H_*/
