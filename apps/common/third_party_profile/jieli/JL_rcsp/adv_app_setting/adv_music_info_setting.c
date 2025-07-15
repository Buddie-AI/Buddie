#include "app_config.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "le_rcsp_adv_module.h"

#include "adv_setting_common.h"
#include "rcsp_adv_tws_sync.h"
#include "rcsp_adv_opt.h"
#include "adv_music_info_setting.h"
#include "btstack/avctp_user.h"
#include "rcsp_adv_bluetooth.h"
#include "bt_tws.h"

#if (RCSP_ADV_EN)
#if RCSP_ADV_MUSIC_INFO_ENABLE


struct music_info_t {
    u8 title_len;
    char title[64];
    u8 artist_len;
    char artist[64];
    u8 album_len;
    char album[64];
    u8 num_len;
    char number;
    u8 total_len;
    char total[2];
    u8 genre_len;
    char genre[16];
    char time[8];
    u8 player_state;
    u8 player_time_min;
    u8 player_time_sec;
    u32 curr_player_time;
    u8 player_time_en;
    u32 total_time;
    volatile int get_music_player_timer;
};

struct music_info_t  music_info;
void JL_rcsp_event_to_user(u32 type, u8 event, u8 *msg, u8 size);


u8 get_player_time_en(void)
{
    return music_info.player_time_en;
}

void set_player_time_en(u8 en)
{
    music_info.player_time_en = en;
}

void reset_player_time_en(void)
{
    music_player_time_timer_deal(music_info.player_time_en);
}

void music_player_time_deal(void *priv)
{
    if (BT_STATUS_PLAYING_MUSIC == get_bt_connect_status()) {
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_GET_PLAY_TIME, 0, NULL);
    }
}

void get_music_info(void)
{
    //printf("\nsend music info\n");
    user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_GET_MUSIC_INFO, 0, NULL);
}

void btstack_avrcp_ch_creat_ok(void)
{
    if (tws_api_get_role() == TWS_ROLE_SLAVE) {
        //printf("\n\n\n\nrcsp ge music info\n");
        tws_api_sync_call_by_uuid('T', SYNC_CMD_MUSIC_INFO, 300);
    } else {
        if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) == 0) {
            //printf("\n\n\nno tws rcsp ge music info\n\n");
            get_music_info();
        }
    }
}

char *get_music_title(void)
{
    return music_info.title;
}

u8 get_music_title_len(void)
{
    return music_info.title_len;
}

char *get_music_artist(void)
{
    return music_info.artist;
}

u8 get_music_artist_len(void)
{
    return music_info.artist_len;
}

char *get_music_album(void)
{
    return music_info.album;
}

u8 get_music_album_len(void)
{
    return music_info.album_len;
}

char *get_music_number(void)
{
    return &music_info.number;
}

u8 get_music_number_len(void)
{
    return music_info.num_len;
}

char *get_music_total(void)
{
    return music_info.total;
}

u8 get_music_total_len(void)
{
    return music_info.total_len;
}

char *get_music_genre(void)
{
    return music_info.genre;
}

u8 get_music_genre_len(void)
{
    return music_info.genre_len;
}

u16 get_music_total_time(void)
{
    return (music_info.total_time / 1000);
}

u32 get_music_curr_time(void)
{
    return music_info.curr_player_time;
    /* return (music_info.player_time_min * 60 + music_info.player_time_sec); */
}

u8 bt_music_total_time(u32 time)
{
    music_info.total_time = time;
    /* printf("get music time %d\n", music_info.total_time); */
    return 0;
}

u8 get_music_player_state(void)
{
    return music_info.player_state;
}

void rcsp_update_player_state(void)
{
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_PLAYER_STATE, NULL, 0);
}


void bt_status_change(u8 state)
{
    if (BT_STATUS_PLAYING_MUSIC == state) {
        music_info.player_state = 1;
    } else {
        music_info.player_state = 0;
    }

    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) == 0) {
        rcsp_update_player_state();
    } else {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            tws_api_sync_call_by_uuid('T', SYNC_CMD_MUSIC_PLAYER_STATE, 300);
        } else {
            tws_api_sync_call_by_uuid('T', SYNC_CMD_MUSIC_PLAYER_TIEM_EN, 300);
        }
    }
}

void stop_get_music_timer(u8 en)
{
    if (en) {
        music_info.player_time_en = 0;
    }

    if (music_info.get_music_player_timer) {
        sys_timeout_del(music_info.get_music_player_timer);
        music_info.get_music_player_timer = 0;
    }
}


void music_player_time_timer_deal(u8 en)
{
    if (en) {
        if (music_info.get_music_player_timer == 0) {
            music_info.get_music_player_timer  = sys_timer_add(NULL, music_player_time_deal, 800);
        }
    } else {
        if (music_info.get_music_player_timer) {
            sys_timer_del(music_info.get_music_player_timer);
            music_info.get_music_player_timer = 0;
        }
    }


}


void music_info_cmd_handle(u8 *p, u16 len)
{
    u8 cmd = *p;
    u8 *data = p + 1;

    switch (cmd) {
    case 0x01:
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL);
        break;

    case 0x02:
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PREV, 0, NULL);
        break;

    case 0x03:
        user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_NEXT, 0, NULL);
        break;

    case 0x04:

        music_info.player_time_en = *data;

        if (*data) {
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_MUSIC_PLAYER_TIME_TEMER, data, 1);
        } else {
            JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_MUSIC_PLAYER_TIME_TEMER, data, 1);
        }
        break;

    default:
        break;
    }
}

u32 char_to_hex(char num, u8 ten_num)
{
    u32 char_num;
    switch (num) {
    case '0':
        char_num = 0;
        break;
    case '1':
        char_num = 1;
        break;
    case '2':
        char_num = 2;
        break;
    case '3':
        char_num = 3;
        break;
    case '4':
        char_num = 4;
        break;
    case '5':
        char_num = 5;
        break;

    case '6':
        char_num = 6;
        break;

    case '7':
        char_num = 7;
        break;

    case '8':
        char_num = 8;
        break;
    case '9':
        char_num = 9;
        break;
    default:
        char_num = 0;

    }

    switch (ten_num) {
    case 0:
        char_num = char_num;
        break;
    case 1:
        char_num = char_num * 10;
        break;
    case 2:
        char_num = char_num * 100;
        break;
    case 3:
        char_num = char_num * 1000;
        break;
    case 4:
        char_num = char_num * 10000;
        break;
    case 5:
        char_num = char_num * 100000;
        break;

    case 6:
        char_num = char_num * 1000000;
        break;

    case 7:
        char_num = char_num * 10000000;
        break;

    default:
        char_num = char_num;
    }

    return char_num;
}

u32 num_char_to_hex(char *c, len)
{
    u32 total_num = 0;
    u8 i;
    for (i = 0; i < len; i++) {
        total_num += char_to_hex(*(c + i), len - i - 1);
    }
    return total_num;

}


void rcsp_adv_music_info_deal(u8 type, u32 time, u8 *info, u16 len)
{

    //printf("info len \n");
    //put_buf(info,len);
#if RCSP_ADV_FIND_DEVICE_ENABLE
    extern u8 find_device_key_flag_get(void);
    if (find_device_key_flag_get()) {
        return;
    }
#endif
    switch (type) {
    case 0:
        if (time != music_info.total_time) {
            /* if(music_info.player_time_en) */
            {
                music_info.curr_player_time = time;
                music_info.player_time_min = time / 1000 / 60;
                music_info.player_time_sec  =  time / 1000 - (music_info.player_time_min * 60);

                /* printf("total time %d %d->", music_info.total_time, time); */
                /* printf("muisc time %d %d->", music_info.player_time_min, music_info.player_time_sec); */
                JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_PLAYER_TIME, NULL, 0);
            }
        }
        return;
    case 1:
        if ((info) && (len)) {
            if (len > 64) {
                len = 64;
            }
            music_info.title_len = len;
            memcpy(music_info.title, info, len);
        }
        break;
    case 2:
        if ((info) && (len)) {
            if (len > 64) {
                len = 64;
            }
            music_info.artist_len = len;
            memcpy(music_info.artist, info, len);
        }
        break;
    case 3:
        if ((info) && (len)) {
            if (len > 64) {
                len = 64;
            }
            music_info.album_len = len;
            memcpy(music_info.album, info, len);
        }
        break;
    case 4:
        if ((info) && (len)) {
            len = 1;
            music_info.num_len = len;
            memcpy(&music_info.number, info, len);
        }
        break;
    case 5:
        if ((info) && (len)) {
            len = 2;
            music_info.total_len = len;
            memcpy(music_info.total, info, len);
        }
        break;
    case 6:
        if ((info) && (len)) {
            if (len > 16) {
                len = 16;
            }
            music_info.genre_len = len;
            memcpy(music_info.genre, info, len);
        }
        break;
    case 7:
        if ((info) && (len)) {
            if (len > 8) {
                len = 8;
            }
            memcpy(music_info.time, info, len);

            /* printf("get time %s\n", info); */
            /* put_buf(music_info.time, len); */
            music_info.total_time =  num_char_to_hex(music_info.time, len);
        }
        break;
    default:
        break;
    }

    if (music_info.player_time_en) {
        JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_MUSIC_INFO, &type, 1);
    }
    /* os_time_dly(2); */
}


#endif
#endif
