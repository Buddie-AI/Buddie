
#include "board_config.h"
#include "asm/dac.h"
#include "media/includes.h"
//#include "media/audio_dev.h"
#include "effectrs_sync.h"
#include "asm/gpio.h"

#if 0

extern struct audio_dac_hdl dac_hdl;
extern int bt_media_sync_master(u8 type);
extern u8 bt_media_device_online(u8 dev);
extern void *bt_media_sync_open(void);
extern void bt_media_sync_close(void *);
extern int a2dp_media_get_total_buffer_size();
/*extern u32 get_bt_slot_time(u8 type, u32 time, u32 *pre_time);*/
extern int bt_send_audio_sync_data(void *, void *buf, u32 len);
extern void bt_media_sync_set_handler(void *, void *priv,
                                      void (*event_handler)(void *, int *, int));
extern int bt_audio_sync_nettime_select(u8 basetime);


const static struct audio_tws_conn_ops tws_conn_ops = {
    .open = bt_media_sync_open,
    .set_handler = bt_media_sync_set_handler,
    .close = bt_media_sync_close,
    /*.time = get_bt_slot_time,*/
    .master = bt_media_sync_master,
    .online = bt_media_device_online,
    .send = bt_send_audio_sync_data,
};

#if (CONFIG_BD_NUM == 2)
#define SYNC_DATA_TOP_PERCENT       70
#define SYNC_DATA_BOTTOM_PERCENT    50
#define SYNC_DATA_BEGIN_PERCENT     60
#elif (TCFG_USER_TWS_ENABLE == 1)
#if TCFG_BT_SUPPORT_AAC
#define SYNC_DATA_TOP_PERCENT       70
#define SYNC_DATA_BOTTOM_PERCENT    50
#define SYNC_DATA_BEGIN_PERCENT     50
#else
#define SYNC_DATA_TOP_PERCENT       80
#define SYNC_DATA_BOTTOM_PERCENT    60
#define SYNC_DATA_BEGIN_PERCENT     60
#endif
#else
#define SYNC_DATA_TOP_PERCENT       80
#define SYNC_DATA_BOTTOM_PERCENT    60
#define SYNC_DATA_BEGIN_PERCENT     70
#endif

#define SYNC_START_TIME   100
void *a2dp_play_sync_open(u8 channel, u32 sample_rate, u32 output_rate, u32 coding_type)
{
    struct audio_wireless_sync_info sync_param;
    int a2dp_total_size = 0;

    if (coding_type == AUDIO_CODING_AAC) {
        a2dp_total_size = 15 * 1024;
    } else if (coding_type == AUDIO_CODING_SBC) {
        a2dp_total_size = a2dp_media_get_total_buffer_size();
    }

    sync_param.channel = channel;
    sync_param.tws_ops = &tws_conn_ops;
    sync_param.sample_rate = sample_rate;
    sync_param.output_rate = output_rate;

    sync_param.data_top = SYNC_DATA_BEGIN_PERCENT * a2dp_total_size / 100;
    sync_param.data_bottom = SYNC_DATA_BOTTOM_PERCENT * a2dp_total_size / 100;
    sync_param.begin_size = SYNC_DATA_BEGIN_PERCENT * a2dp_total_size / 100;
    sync_param.tws_together_time = SYNC_START_TIME;

    sync_param.protocol = WL_PROTOCOL_RTP;
    /*sync_param.dev_type = AUDIO_OUTPUT_DAC;*/
    sync_param.dev = &dac_hdl;

    bt_audio_sync_nettime_select(0);

    return audio_wireless_sync_open(&sync_param);
}

void *esco_play_sync_open(u8 channel, u16 sample_rate, u16 output_rate)

{
    struct audio_wireless_sync_info sync_param;

    int esco_buffer_size = 60 * 50;

    sync_param.channel = channel;
    sync_param.sample_rate = sample_rate;
    sync_param.tws_ops = &tws_conn_ops;
    sync_param.data_top = 40 * esco_buffer_size / 100;
    sync_param.data_bottom = 10 * esco_buffer_size / 100;
    sync_param.begin_size =  15 * esco_buffer_size / 100; /*60*50*6/100 = 180(3 packet)*/
    sync_param.tws_together_time = SYNC_START_TIME;
    /*sync_param.dev_type = AUDIO_OUTPUT_DAC;*/
    sync_param.dev = &dac_hdl;

    sync_param.protocol = WL_PROTOCOL_SCO;

    bt_audio_sync_nettime_select(0);

    return audio_wireless_sync_open(&sync_param);
}
#endif
