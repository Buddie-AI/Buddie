#ifndef APP_MAIN_H
#define APP_MAIN_H

typedef struct {
    float talk;
    float ff;
    float fb;
} audio_mic_cmp_t;

typedef struct _APP_VAR {
    s8 bt_volume;
    s8 dev_volume;
    s8 music_volume;
    s8 call_volume;
    s8 wtone_volume;
    u8 opid_play_vol_sync;
    u8 aec_dac_gain;
    u8 aec_mic_gain;
    u8 aec_mic1_gain;
    u8 aec_mic2_gain;
    u8 aec_mic3_gain;
    u8 rf_power;
    u8 goto_poweroff_flag;
    u8 goto_poweroff_cnt;
    u8 play_poweron_tone;
    u8 remote_dev_company;
    u8 siri_stu;
    u8 cycle_mode;
    u8 have_mass_storage;
    int auto_stop_page_scan_timer;     //用于1拖2时，有一台连接上后，超过三分钟自动关闭Page Scan
    volatile int auto_shut_down_timer;
    volatile int wait_exit_timer;
    u16 auto_off_time;
    u16 warning_tone_v;
    u16 poweroff_tone_v;
    u16 phone_dly_discon_time;
    u8 usb_mic_gain;
    int wait_timer_do;
    u32 start_time;
    float audio_mic_array_diff_cmp;//麦克风阵列补偿值
    u8 audio_mic_array_trim_en; //麦克风阵列校准
    audio_mic_cmp_t audio_mic_cmp;
    float enc_degradation;//default:1,range[0:1]
    /*3麦通话配置使用:选择第几个mic*/
    u8 talk_mic_ch;		//主MIC通道选择
    u8 talk_ref_mic_ch;	//副MIC通道选择
    u8 talk_fb_mic_ch;	//FB通道选择
} APP_VAR;

typedef struct _BT_USER_PRIV_VAR {
    //phone
    u8 phone_ring_flag: 1;
    u8 phone_num_flag: 1;
    u8 phone_income_flag: 1;
    u8 phone_call_dec_begin: 1;
    u8 phone_con_sync_num_ring: 1;
    u8 phone_con_sync_ring: 1;
    // u8 reserved: 2;
    u8 emitter_or_receiver: 2;
    u8 get_phone_num_timecnt;

    u8 inband_ringtone;
    u8 phone_vol;
    u16 phone_timer_id;
    u8 last_call_type;
    u8 income_phone_num[30];
    u8 income_phone_len;
    s32 auto_connection_counter;
    int auto_connection_timer;
    u8 auto_connection_addr[6];
    int tws_con_timer;
    u8 tws_start_con_cnt;
    u8 tws_conn_state;
    bool search_tws_ing;
    int sniff_timer;
    bool fast_test_mode;
} BT_USER_PRIV_VAR;

#define    BT_EMITTER_EN     1
#define    BT_RECEIVER_EN    2

typedef struct _BT_USER_COMM_VAR {
} BT_USER_COMM_VAR;

extern APP_VAR app_var;
extern BT_USER_PRIV_VAR bt_user_priv_var;

#define earphone (&bt_user_priv_var)

#endif
