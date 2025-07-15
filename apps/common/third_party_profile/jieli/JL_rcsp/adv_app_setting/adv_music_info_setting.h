#ifndef __ADV_MUSIC_INFO_SETTING_H__
#define __ADV_MUSIC_INFO_SETTING_H__


#include "le_rcsp_adv_module.h"

#if RCSP_ADV_MUSIC_INFO_ENABLE

char *get_music_title(void);
u8 get_music_title_len(void);
char *get_music_artist(void);
u8 get_music_artist_len(void);
char *get_music_album(void);
u8 get_music_album_len(void);
char *get_music_number(void);
u8 get_music_number_len(void);
char *get_music_total(void);
u8 get_music_total_len(void);
char *get_music_genre(void);
u8 get_music_genre_len(void);
u16 get_music_total_time(void);
u32 get_music_curr_time(void);
u8 get_music_player_state(void);
void music_info_cmd_handle(u8 *p, u16 len);
void stop_get_music_timer(u8 en);
extern void music_player_time_timer_deal(u8 en);
u8 get_player_time_en(void);
void set_player_time_en(u8 en);
void rcsp_adv_music_info_deal(u8 type, u32 time, u8 *info, u16 len);
#endif
#endif
