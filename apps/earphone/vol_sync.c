#include "typedef.h"
#include "app_main.h"
#include "app_config.h"
#include "vol_sync.h"
#include "tone_player.h"
#include "app_core.h"

u8 vol_sys_tab[17] =  {0, 2, 3, 4, 6, 8, 10, 11, 12, 14, 16, 18, 19, 20, 22, 23, 25};
const u8 vol_sync_tab[17] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 127};

extern u8 get_max_sys_vol(void);
extern bool get_esco_busy_flag(void);

void vol_sys_tab_init(void)
{
#if BT_SUPPORT_MUSIC_VOL_SYNC

#if 1
    u8 i = 0;
    u8 max_vol = get_max_sys_vol();
    for (i = 0; i < 17; i++) {
        vol_sys_tab[i] = i * max_vol / 16;
    }
#else
    u8 i = 0;
    u8 max_vol = get_max_sys_vol();
    vol_sys_tab[0] = 0;

    //最大音量<16，补最大值
    if (max_vol <= 16) {
        for (i = 1; i <= max_vol; i++) {
            vol_sys_tab[i] = i;
        }
        for (; i < 17; i++) {
            vol_sys_tab[i] = max_vol;
        }
    } else {
        u8 j = max_vol - 16;
        u8 k = 1;
        for (i = 1; i <= max_vol; i++) {
            /* g_printf("i=%d j=%d  k=%d",i,j,k); */
            if (i % 2) {
            } else {
                //忽略多余的级数
                if (j > 0) {
                    j--;
                    continue;
                }
            }

            if (k < 17) {
                vol_sys_tab[k] = i;
            }
            k++;
        }

        vol_sys_tab[16] = max_vol;
    }

#endif

#if 1
    for (i = 0; i < 17; i++) {
        g_printf("[%d]:%d ", i, vol_sys_tab[i]);
    }
#endif

#endif //BT_SUPPORT_MUSIC_VOL_SYNC
}

//注册给库的回调函数，用户手机设置设备音量
void set_music_device_volume(int volume)
{
    r_printf("set_music_device_volume=%d\n", volume);
#if BT_SUPPORT_MUSIC_VOL_SYNC
    s16 music_volume;

#if CONFIG_BT_BACKGROUND_ENABLE
    struct application *app = get_current_app();
    if (app->name != "earphone") {
        app_var.bt_volume = ((volume + 1) * 16) / 127;
        r_printf("set_app_var.bt_volume=%d\n", app_var.bt_volume);
        return;
    }
#endif

    //音量同步最大是127，请计数比例
    if (volume > 127) {
        /*log_info("vol %d invalid\n",  volume);*/
#if TCFG_VOL_RESET_WHEN_NO_SUPPORT_VOL_SYNC
        /*不支持音量同步的设备，默认设置到最大值，可以根据实际需要进行修改。
         注意如果不是最大值，设备又没有按键可以调音量到最大，则输出也就达不到最大*/
        music_volume = get_max_sys_vol();
        log_i("unsupport vol_sync,reset vol:%d\n", music_volume);
        app_audio_set_volume(APP_AUDIO_STATE_MUSIC, music_volume, 1);
#endif
        return;
    }
    if (tone_get_status() || get_esco_busy_flag()) {
        log_i("It's not smart to sync a2dp vol now\n");
        //app_var.music_volume = vol_sys_tab[(volume + 1) / 8];
        app_var.music_volume = ((volume + 1) * 16) / 127;
        return;
    }

#if 1
    /*
     *0~16,总共17级
     *这里将手机的0~127的音量值换成实际的dac音量等级
     */
    music_volume = ((volume + 1) * 16) / 127;
#else
    music_volume = vol_sys_tab[(volume + 1) / 8];
#endif
    y_printf("phone_vol:%d,dac_vol:%d", volume, music_volume);
    app_var.opid_play_vol_sync = vol_sync_tab[(volume + 1) / 8];

    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, music_volume, 1);

#endif
}

//注册给库的回调函数，用于手机获取当前设备音量
int phone_get_device_vol(void)
{
    //音量同步最大是127，请计数比例
#if 0
    return (app_var.sys_vol_l * get_max_sys_vol() / 127) ;
#else
    return app_var.opid_play_vol_sync;
#endif
}


void opid_play_vol_sync_fun(u8 *vol, u8 mode)
{
#if BT_SUPPORT_MUSIC_VOL_SYNC
    u8 i = 0;
    vol_sys_tab[16] =  get_max_sys_vol();

    if (*vol == 0) {
        if (mode) {
            *vol = vol_sys_tab[1];
            app_var.opid_play_vol_sync = vol_sync_tab[1];
        } else {
            *vol = vol_sys_tab[0];
            app_var.opid_play_vol_sync = vol_sync_tab[0];
        }
    } else if (*vol >= get_max_sys_vol()) {
        if (mode) {
            *vol = vol_sys_tab[16];
            app_var.opid_play_vol_sync = vol_sync_tab[16];
        } else {
            *vol = vol_sys_tab[15];
            app_var.opid_play_vol_sync = vol_sync_tab[15];
        }
    } else {
        for (i = 0; i < sizeof(vol_sys_tab); i++) {
            if (*vol == vol_sys_tab[i]) {
                if (mode) {
                    app_var.opid_play_vol_sync = vol_sync_tab[i + 1];
                    *vol = vol_sys_tab[i + 1];
                } else {
                    app_var.opid_play_vol_sync = vol_sync_tab[i - 1];
                    *vol = vol_sys_tab[i - 1];
                }
                break;
            } else if (*vol < vol_sys_tab[i]) {
                if (mode) {
                    *vol = vol_sys_tab[i + 1];
                    app_var.opid_play_vol_sync = vol_sync_tab[i];
                } else {
                    *vol = vol_sys_tab[i - 1];
                    app_var.opid_play_vol_sync = vol_sync_tab[i - 1];
                }
                break;
            }
        }
    }
#endif
}

