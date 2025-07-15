#ifndef APP_BT_TWS_H
#define APP_BT_TWS_H

#include "classic/tws_api.h"
#include "classic/tws_event.h"
#include "system/includes.h"

#define TWS_FUNC_ID_VOL_SYNC        TWS_FUNC_ID('V', 'O', 'L', 'S')
#define TWS_FUNC_ID_VBAT_SYNC       TWS_FUNC_ID('V', 'B', 'A', 'T')
#define TWS_FUNC_ID_CHARGE_SYNC     TWS_FUNC_ID('C', 'H', 'G', 'S')
#define TWS_FUNC_ID_BOX_SYNC        TWS_FUNC_ID('B', 'O', 'X', 'S')
#define TWS_FUNC_ID_AI_DMA_RAND     TWS_FUNC_ID('A', 'I', 'D', 'M')
#define TWS_FUNC_ID_AI_SPEECH_STOP  TWS_FUNC_ID('A', 'I', 'S', 'T')
#define TWS_FUNC_ID_APP_MODE  		TWS_FUNC_ID('M', 'O', 'D', 'E')
#define TWS_FUNC_ID_AI_SYNC			TWS_FUNC_ID('A', 'I', 'P', 'A')
#define TWS_FUNC_ID_EAR_DETECT_SYNC	TWS_FUNC_ID('E', 'D', 'E', 'T')
#define TWS_FUNC_ID_LL_SYNC_STATE	TWS_FUNC_ID('L', 'L', 'S', 'S')
#define TWS_FUNC_ID_TUYA_STATE      TWS_FUNC_ID('T', 'U', 'Y', 'A')

enum {
    DEBUG_LINK_PAGE_STATE = 0,
    DEBUG_LINK_INQUIRY_STATE,
    DEBUG_LINK_PAGE_SCAN_STATE,
    DEBUG_LINK_INQUIRY_SCAN_STATE,
    DEBUG_LINK_CONNECTION_STATE,
    DEBUG_LINK_PAGE_TWS_STATE,
    DEBUG_LINK_PAGE_SCAN_TWS_STATE,
};
enum {
    BT_TWS_STATUS_INIT_OK = 1,
    BT_TWS_STATUS_SEARCH_START,
    BT_TWS_STATUS_SEARCH_TIMEOUT,
    BT_TWS_STATUS_PHONE_CONN,
    BT_TWS_STATUS_PHONE_DISCONN,
};

enum {
    SYNC_TONE_TWS_CONNECTED = 1,
    SYNC_TONE_PHONE_CONNECTED,
    SYNC_TONE_PHONE_DISCONNECTED,
    SYNC_TONE_PHONE_NUM,
    SYNC_TONE_PHONE_RING,
    SYNC_TONE_PHONE_NUM_RING,
    SYNC_TONE_MAX_VOL,
    SYNC_TONE_POWER_OFF,
    SYNC_TONE_ANC_ON,
    SYNC_TONE_ANC_OFF,
    SYNC_TONE_ANC_TRANS,
    SYNC_TONE_EARDET_IN,
    SYNC_TONE_VOICE_RECOGNIZE,
    SYNC_TONE_HEARING_AID_OPEN,
    SYNC_TONE_DHA_FITTING_CLOSE,
    SYNC_TONE_ICSD_ADT_SUSPEND,
    SYNC_TONE_ICSD_ADT_VOICE_STATE,
    SYNC_TONE_ICSD_ADT_OPEN,
};

enum {
    SYNC_LED_STA_TWS_CONN,
    SYNC_LED_STA_PHONE_CONN,
    SYNC_LED_STA_PHONE_DISCONN,
};

enum {
    SYNC_CMD_SHUT_DOWN_TIME,
    SYNC_CMD_POWER_OFF_TOGETHER,
    SYNC_CMD_LOW_LATENCY_ENABLE,
    SYNC_CMD_LOW_LATENCY_DISABLE,
    SYNC_CMD_EARPHONE_CHAREG_START,
    SYNC_CMD_IRSENSOR_EVENT_NEAR,
    SYNC_CMD_IRSENSOR_EVENT_FAR,
#if(USE_DMA_TONE)
    SYNC_CMD_CUT_TWS_TONE,     //断开对耳提示音
    SYNC_CMD_START_SPEECH_TONE,//AI键提示音
    SYNC_CMD_DMA_CONNECTED_ALL_FINISH_TONE,   //AI连接成功
    SYNC_CMD_NEED_BT_TONE,     //需要连接蓝牙
    SYNC_CMD_PLEASE_OPEN_XIAODU_TONE,//请打开小度
#endif
#if(RCSP_ADV_EN)
    SYNC_CMD_SYNC_ADV_SETTING,
    SYNC_CMD_ADV_COMMON_SETTING_SYNC,
    SYNC_CMD_APP_RESET_LED_UI,
    SYNC_CMD_MUSIC_INFO,
    SYNC_CMD_RCSP_AUTH_RES,
    SYNC_CMD_MUSIC_PLAYER_STATE,
    SYNC_CMD_MUSIC_PLAYER_TIEM_EN,
#endif
    SYNC_CMD_OPUS_CLOSE,
};

enum {
    TWS_SYNC_VOL     = 0,
    TWS_SYNC_VBAT,
    TWS_SYNC_CHG,
    TWS_SYNC_CALL_VOL,
    TWS_SYNC_PBG_INFO,
    TWS_SYNC_ADSP_UART_CMD,
    TWS_APP_DATA_SEND,
    TWS_AI_DMA_RAND,
    TWS_DATA_SEND,

    TWS_UPDATE_START,
    TWS_UPDATE_RESULT_EXCHANGE,
    TWS_UPDATE_RESULT_EXCHANGE_RES,
    TWS_UPDATE_OVER,
    TWS_UPDATE_OVER_CONFIRM,
    TWS_UPDATE_OVER_CONFIRM_REQ,
    TWS_UPDATE_OVER_CONFIRM_RES,
    TWS_UPDATE_VERIFY,

    TWS_AI_A2DP_DROP_FRAME_CTL,
};
struct tws_sync_info_t {
    u8 type;
    union {
        s8 volume_lev;
        u16 vbat_lev;
        u8 chg_lev;
        u8 adsp_cmd[2];
        u8 conn_type;
        u8 data[9];
        u8 data_large[32];
    } u;
};



struct tws_sync_big_info_t {
    u8 type;
    u8 sub_type;
    union {
        u8 pbg_info[36];
    } u;
};


typedef struct time_stamp_bt_name {
    u8  bt_name[32];
    u32 time_stamp;
} tws_time_stamp_bt_name;


#define TWS_WAIT_CONN_TIMEOUT      (400)

#define TWS_SYNC_TIME_DO       400
// #define TWS_CON_SUPER_TIMEOUT   8000
#define TWS_CON_SEARCH_TIMEOUT  0x07//(n*1.28s)


char bt_tws_get_local_channel();

int bt_tws_connction_status_event_handler(struct bt_event *evt);

int bt_tws_poweron();

int bt_tws_poweroff();

int bt_tws_start_search_sibling();

void bt_tws_hci_event_connect();

bool get_tws_phone_connect_state(void);

int bt_tws_phone_connected();

void bt_tws_phone_page_timeout();

void bt_tws_phone_connect_timeout();

void bt_tws_phone_disconnected();

int bt_tws_sync_phone_num(void *priv);

int bt_tws_sync_led_status();

int get_bt_tws_connect_status();
u8 get_bt_tws_discon_dly_state();

void bt_tws_sync_volume();

u8 tws_network_audio_was_started(void);

void tws_network_local_audio_start(void);

u32 bt_tws_future_slot_time(u32 msecs);

void bt_tws_play_tone_at_same_time(int tone_name, int msec);
void bt_tws_led_state_sync(int led_state, int msec);
bool get_tws_sibling_connect_state(void);

void tws_cancle_all_noconn();
void tws_sniff_controle_check_enable(void);
void tws_sniff_controle_check_disable(void);
#endif
