#ifndef __VOL_SYNC_H__
#define __VOL_SYNC_H__

void vol_sys_tab_init(void);
void set_music_device_volume(int volume);
int  phone_get_device_vol(void);
void opid_play_vol_sync_fun(u8 *vol, u8 mode);

#endif
