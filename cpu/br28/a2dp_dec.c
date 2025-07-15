/*************************************************************************************************/
/*!
*  \file      a2dp_dec.c
*
*  \brief
*
*  Copyright (c) 2011-2022 ZhuHai Jieli Technology Co.,Ltd.
*
*/
/*************************************************************************************************/
#include "sound_device.h"
#include "media/bt_audio_timestamp.h"
#include "media/a2dp_sample_detect.h"
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
#include "spatial_effect/spatial_effect.h"
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

#define A2DP_AUDIO_PLC_ENABLE       1

#if A2DP_AUDIO_PLC_ENABLE
#include "media/tech_lib/LFaudio_plc_api.h"
#endif

#if TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE
#include "icsd_adt_app.h"
#endif

#if AUDIO_VBASS_CONFIG
#include "application/audio_vbass.h"
#endif

#if TCFG_AUDIO_CVP_DUT_ENABLE
#include "audio_cvp_dut.h"
#endif /*TCFG_AUDIO_CVP_DUT_ENABLE*/

#include "audio_effect_develop.h"

#if (TCFG_AUDIO_SPATIAL_EFFECT_ENABLE)
#define CONFIG_AUDIO_EFFECT_TASK_ENABLE		TCFG_AUDIO_EFFECT_TASK_EBABLE
#else
#define CONFIG_AUDIO_EFFECT_TASK_ENABLE	    0
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/
#define A2DP_FLUENT_STREAM_MODE             1//流畅模式
#define A2DP_FLUENT_DETECT_INTERVAL         100000//ms 流畅播放延时检测时长
#if A2DP_FLUENT_STREAM_MODE
#define A2DP_MAX_PENDING_TIME               120
#else
#define A2DP_MAX_PENDING_TIME               40
#endif

#define A2DP_STREAM_NO_ERR                  0
#define A2DP_STREAM_UNDERRUN                1
#define A2DP_STREAM_OVERRUN                 2
#define A2DP_STREAM_MISSED                  3
#define A2DP_STREAM_DECODE_ERR              4
#define A2DP_STREAM_LOW_UNDERRUN            5

#ifdef TCFG_AUDIO_MUSIC_SAMPLE_RATE
#define A2DP_SOUND_SAMPLE_RATE      TCFG_AUDIO_MUSIC_SAMPLE_RATE
#else
#define A2DP_SOUND_SAMPLE_RATE      0
#endif

#if TCFG_USER_TWS_ENABLE && TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
#define CONFIG_TWS_SPATIAL_AUDIO_ENABLE     1
#else
#define CONFIG_TWS_SPATIAL_AUDIO_ENABLE     0
#endif

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
static u8 spatial_audio_enable = 1;
static u8 spatial_audio_head_tracked = 0;
void a2dp_spatial_audio_head_tracked_en(u8 en);
int aud_spatial_sensor_init();
int aud_spatial_sensor_exit();
int aud_spatial_sensor_run(void *priv, void *data, int len);
#endif
int aud_effect_push_data(void *priv, s16 *data, u16 len);

/**************************************************************************************************
 * A2DP音频数据流异步任务处理说明
 *
 * 1、解码后数据流的顺序
 *    通常A2DP解码后的数据流会按照以下顺序进行流数据处理：
 *    PLC -> 空间音频(Spatial audio) -> 数字音量(Digtal vol) -> 环绕音效(Surround) ->
 *    虚拟音效(Vbass) -> 低音增强（bass_boost）-> 动态增益控制(ANC) -> 音频同步(Syncts) -> EQ/DRC -> 混音器(mixer) -> DAC
 *
 *    以上流程以实际形态的功能开关为准。
 * 2、异步任务处理节点方法：
 *    a2dp_decoder_effect_task_enter();
 *
 *    要加入的节点
 *
 *    a2dp_decoder_effect_task_output();
 *
 *    由该方式包住的节点则都会被归纳到effect task中执行
 *
 **************************************************************************************************/
#define A2DP_NODE_BEGIN             0x1
#define A2DP_NODE_END               0x2
#define A2DP_NODE_BLOCK             0x4
#define A2DP_NODE_SUB_STREAM_BEGIN  0x8
#define A2DP_NODE_SUB_STREAM_END    0x10

enum a2dp_stream_id {
    A2DP_NODE_DECODER = 0x0,
#if A2DP_AUDIO_PLC_ENABLE
    A2DP_NODE_PLC,
#endif
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    A2DP_NODE_SPATIAL_AUDIO,
#if TCFG_SENSOR_DATA_READ_IN_DEC_TASK
    A2DP_NODE_SENSOR_DATA_STREAM,
#endif /*TCFG_SENSOR_DATA_READ_IN_DEC_TASK*/
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    A2DP_NODE_DIGITAL_VOLUME,
#endif
#if (AUDIO_SURROUND_CONFIG || AUDIO_VBASS_CONFIG)
    A2DP_NODE_SURROUND_VBASS,
#endif
#if defined(TCFG_AUDIO_BASS_BOOST)&&TCFG_AUDIO_BASS_BOOST
    A2DP_NODE_BASS_BOOST,
#endif

#if TCFG_AUDIO_ANC_ENABLE && ANC_MUSIC_DYNAMIC_GAIN_EN
    A2DP_NODE_DYNAMIC_GAIN,
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/

#if AUDIO_CODEC_SUPPORT_SYNC
    A2DP_NODE_BEFORE_SYNCTS,
    A2DP_NODE_SYNCTS,
#endif

#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
    A2DP_NODE_EFFECT_TASK_ENTER,
#endif

#if (TCFG_EQ_ENABLE && TCFG_BT_MUSIC_EQ_ENABLE)
    A2DP_NODE_EQ_DRC,
#endif

#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
    A2DP_NODE_EFFECT_TASK_OUT,
#endif

#if ((defined TCFG_EFFECT_DEVELOP_ENABLE) && TCFG_EFFECT_DEVELOP_ENABLE)
    A2DP_NODE_EFFECT_DEVELOP,
#endif
    A2DP_NODE_MIXER,
    A2DP_NODE_MAX,
};

struct a2dp_stream_node {
    enum a2dp_stream_id id;
    u8 attribute;
    u8 remain;
    void *priv;
    int (*data_handler)(void *priv, void *data, int len);
    void (*wakeup)(void *stream);
    int (*set_wakeup_handler)(void *priv, void *stream, void (*wakeup)(void *));
};

struct a2dp_dec_hdl {
    struct audio_decoder decoder;
    struct audio_res_wait wait;
    struct audio_mixer_ch mix_ch;
    enum audio_channel channel;
    u8 start;
    u8 ch;
    s16 header_len;
    u8 remain;
    u8 eq_remain;
    u8 fetch_lock;
    u8 preempt;
    u8 stream_error;
    u8 new_frame;
    u8 repair;
    u8 dut_enable;
    void *sample_detect;
    void *syncts;
    void *repair_pkt;
    s16 repair_pkt_len;
    u16 missed_num;
    u16 repair_frames;
    u16 overrun_seqn;
    u16 slience_frames;
#if AUDIO_CODEC_SUPPORT_SYNC
    u8 ts_start;
    u8 sync_step;
    void *ts_handle;
    u32 timestamp;
    u32 base_time;
#endif /*AUDIO_CODEC_SUPPORT_SYNC*/
#if A2DP_AUDIO_PLC_ENABLE
    LFaudio_PLC_API *plc_ops;
    void *plc_mem;
#endif /*A2DP_AUDIO_PLC_ENABLE*/
    u32 mix_ch_event_params[3];

    u32 pending_time;
    u16 seqn;
    u32 sample_rate;
    int timer;
    u32 coding_type;
    u16 delay_time;
    u16 detect_timer;
    u8  underrun_feedback;
    /*
    u8  underrun_count;
    u32 underrun_time;
    u32 underrun_cool_time;
    */

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    void *spatial_audio;
    s16 spatial_data_len;
    u8 spatial_node_id;
    u16 spatial_data_offset;
#endif
#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
    void *aud_effect;
#endif
#if TCFG_EQ_ENABLE&&TCFG_BT_MUSIC_EQ_ENABLE
    struct dec_eq_drc *eq_drc;
#endif//TCFG_BT_MUSIC_EQ_ENABLE

#if AUDIO_SURROUND_CONFIG
    struct dec_sur *sur;
#endif//AUDIO_SURROUND_CONFIG

#if AUDIO_VBASS_CONFIG
    vbass_hdl *vbass;               //虚拟低音句柄
#endif//AUDIO_VBASS_CONFIG

#if defined(TCFG_AUDIO_BASS_BOOST)&&TCFG_AUDIO_BASS_BOOST
    struct audio_drc *bass_boost;
#endif
#if ((defined TCFG_EFFECT_DEVELOP_ENABLE) && TCFG_EFFECT_DEVELOP_ENABLE)
    void *effect_develop;
#endif

    struct a2dp_stream_node stream_node[A2DP_NODE_MAX + 1];
    u8 node_num;
};

extern const int CONFIG_LOW_LATENCY_ENABLE;
extern const int CONFIG_A2DP_DELAY_TIME;
extern const int CONFIG_A2DP_DELAY_TIME_LO;
extern const int CONFIG_A2DP_SBC_DELAY_TIME_LO;

static u16 a2dp_delay_time;
static u8 a2dp_low_latency = 0;
static u16 drop_a2dp_timer;
static u16 a2dp_low_latency_seqn  = 0;


struct a2dp_dec_hdl *a2dp_dec = NULL;

static void a2dp_stream_node_init(struct a2dp_dec_hdl *dec);
static void a2dp_stream_node_add(struct a2dp_dec_hdl *dec,
                                 void *priv,
                                 int (*data_handler)(void *, void *, int),
                                 void (*wakeup)(void *),
                                 int (*set_wakeup_handler)(void *, void *, void (*wakeup)(void *)),
                                 u8 attribute);
static void a2dp_stream_node_wakeup(void *priv);

extern u32 bt_audio_sync_lat_time(void);
extern void bt_audio_sync_nettime_select(u8 base);
void audio_mix_ch_event_handler(void *priv, int event)
{
    switch (event) {
    case MIXER_EVENT_CH_OPEN:
        if (priv) {
            u32 *params = (u32 *)priv;
            struct audio_mixer_ch *ch = (struct audio_mixer_ch *)params[0];
            u32 base_time = params[2];
            sound_pcm_dev_add_syncts((void *)params[1]);
            u32 current_time = (bt_audio_sync_lat_time() * 625 * TIME_US_FACTOR);
            u32 time_diff = ((base_time - current_time) & 0xffffffff) / TIME_US_FACTOR;
            printf("-----base time : %u, current_time : %u------\n", base_time, current_time);
            if (time_diff < 500000) {
                int buf_frame = sound_pcm_dev_buffered_frames();
                int slience_frames = (u64)time_diff * audio_mixer_get_sample_rate(&mixer) / 1000000 - buf_frame;
                if (slience_frames < 0) {
                    slience_frames = 0;
                }
                printf("-------slience_frames : %d-------\n", slience_frames);
                sound_pcm_update_frame_num((void *)params[1], -slience_frames);
                audio_mixer_ch_add_slience_samples(ch, slience_frames * sound_pcm_dev_channel_mapping(0));
            }
        }
        break;
    case MIXER_EVENT_CH_CLOSE:
        if (priv) {
            u32 *params = (u32 *)priv;
            sound_pcm_dev_remove_syncts((void *)params[1]);
        }
        break;
    }
}

#define RB16(b)    (u16)(((u8 *)b)[0] << 8 | (((u8 *)b))[1])
#define RB32(b)    (u32)(((u8 *)b)[0] << 24 | (((u8 *)b))[1] << 16 | (((u8 *)b))[2] << 8 | (((u8 *)b))[3])

static int get_rtp_header_len(u8 new_frame, u8 *buf, int len)
{
    int ext, csrc;
    int byte_len;
    int header_len = 0;
    u8 *data = buf;

    csrc = buf[0] & 0x0f;
    ext  = buf[0] & 0x10;

    byte_len = 12 + 4 * csrc;
    buf += byte_len;

    if (ext) {
        ext = (RB16(buf + 2) + 1) << 2;
    }

    if (new_frame) {
        header_len = byte_len + ext + (a2dp_dec->coding_type == AUDIO_CODING_AAC ? 0 : 1);
    } else {
        header_len = byte_len + ext;
    }

    if (header_len > len - 1) {
        return len;
    }

    if (a2dp_dec->coding_type == AUDIO_CODING_SBC) {
        for (; header_len < len; header_len++) {
            if (data[header_len] == 0x9c) {
                return header_len;
            }
        }
        log_e("==== find sbc header error === \n");
        put_buf(data, len);
    }

    return header_len;
}

__attribute__((weak))
int audio_dac_get_channel(struct audio_dac_hdl *p)
{
    return 0;
}

void __a2dp_drop_frame(void *p)
{
    int len;
    u8 *frame;

#if 0
    int num = a2dp_media_get_packet_num();
    if (num > 1) {
        for (int i = 0; i < num; i++) {
            len = a2dp_media_get_packet(&frame);
            if (len <= 0) {
                break;
            }
            //printf("a2dp_drop_frame: %d\n", len);
            a2dp_media_free_packet(frame);
        }
    }
#else
    while (1) {
        len = a2dp_media_try_get_packet(&frame);
        if (len <= 0) {
            break;
        }
        a2dp_media_free_packet(frame);
    }

#endif

}

static void __a2dp_clean_frame_by_number(struct a2dp_dec_hdl *dec, u16 num)
{
    u16 end_seqn = dec->seqn + num;
    if (end_seqn == 0) {
        end_seqn++;
    }
    /*__a2dp_drop_frame(NULL);*/
    /*dec->drop_seqn = end_seqn;*/
    a2dp_media_clear_packet_before_seqn(end_seqn);
}

static void a2dp_tws_clean_frame(void *arg)
{
    u8 master = 0;
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        master = 1;
    }
#else
    master = 1;
#endif
    if (!master) {
        return;
    }

    int msecs = a2dp_media_get_remain_play_time(0);
    if (msecs <= 0) {
        return;
    }

    if (a2dp_dec && a2dp_dec->fetch_lock) {
        return;
    }
    int len = 0;
    u16 seqn = 0;
    u8 *packet = a2dp_media_fetch_packet(&len, NULL);
    if (!packet) {
        return;
    }
    seqn = RB16(packet + 2) + 10;
    if (seqn == 0) {
        seqn = 1;
    }
    a2dp_media_clear_packet_before_seqn(seqn);
}

static u8 a2dp_suspend = 0;
static u32 a2dp_resume_time = 0;



int a2dp_decoder_pause(void)
{
    if (a2dp_dec) {
        return audio_decoder_pause(&(a2dp_dec->decoder));
    }

    return 0;
}

int a2dp_decoder_start(void)
{
    if (a2dp_dec) {
        return audio_decoder_start(&(a2dp_dec->decoder));
    }

    return 0;
}
u8 get_a2dp_drop_frame_flag()
{
    if (a2dp_dec) {
        return a2dp_dec->timer;
    }
    return 0;
}

void a2dp_drop_frame_start()
{
    if (a2dp_dec && (a2dp_dec->timer == 0)) {
        a2dp_dec->timer = sys_timer_add(NULL, __a2dp_drop_frame, 50);
        a2dp_tws_audio_conn_offline();
    }
}

void a2dp_drop_frame_stop()
{
    if (a2dp_dec && a2dp_dec->timer) {
        sys_timer_del(a2dp_dec->timer);
        a2dp_tws_audio_conn_delete();
        a2dp_dec->timer = 0;
    }
}

static void a2dp_dec_set_output_channel(struct a2dp_dec_hdl *dec)
{
    int state = 0;
    enum audio_channel channel;
    u8 dac_connect_mode = 0;

    u8 ch_num = sound_pcm_dev_channel_mapping(1);
#if TCFG_USER_TWS_ENABLE
    state = tws_api_get_tws_state();
    if (state & TWS_STA_SIBLING_CONNECTED) {
        if (ch_num == 2) {
            channel = tws_api_get_local_channel() == 'L' ? AUDIO_CH_DUAL_L : AUDIO_CH_DUAL_R;
        } else {
            channel = tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R;
        }
    } else {
        if (ch_num == 2) {
            channel = AUDIO_CH_LR;
        } else {
            channel = AUDIO_CH_DIFF;
        }
    }
    dec->ch = ch_num;
#if CONFIG_TWS_SPATIAL_AUDIO_ENABLE
    if (spatial_audio_enable || dec->spatial_audio) {
        channel = AUDIO_CH_LR;
#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
        dec->ch = 2;
#endif/*CONFIG_AUDIO_EFFECT_TASK_ENABLE*/
        //printf("Spatial Effect,ch:%d,%d\n",dec->ch,channel);
    }
#endif/*CONFIG_TWS_SPATIAL_AUDIO_ENABLE*/
#else
    if (ch_num == 2) {
        channel = AUDIO_CH_LR;
    } else {
        channel = AUDIO_CH_DIFF;
    }
    dec->ch = ch_num;
#endif


#if TCFG_APP_FM_EMITTER_EN
    channel = AUDIO_CH_LR;
#endif
    if (channel != dec->channel) {
        printf("set_channel: %d\n", channel);
#if CONFIG_TWS_SPATIAL_AUDIO_ENABLE
        if (dec->spatial_audio) {
            if (state & TWS_STA_SIBLING_CONNECTED) {
                spatial_audio_set_mapping_channel(dec->spatial_audio, tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R);
            } else {
                spatial_audio_set_mapping_channel(dec->spatial_audio, AUDIO_CH_MIX_MONO);
            }
        }
#endif
        audio_decoder_set_output_channel(&dec->decoder, channel);
        dec->channel = channel;
#if TCFG_EQ_ENABLE&&TCFG_BT_MUSIC_EQ_ENABLE
        if (dec->eq_drc && dec->eq_drc->eq) {
            audio_eq_set_channel(dec->eq_drc->eq, dec->ch);
        }
#endif//TCFG_BT_MUSIC_EQ_ENABLE


        /* #if TCFG_USER_TWS_ENABLE */
#if AUDIO_SURROUND_CONFIG
        if (dec->sur) {
            audio_surround_set_ch(dec->sur, channel);
        }
#endif//AUDIO_SURROUND_CONFIG
        /* #endif//TCFG_USER_TWS_ENABLE */

    }
}


/*
 *
 */
static int a2dp_decoder_set_timestamp(struct a2dp_dec_hdl *dec, u16 seqn)
{
#if AUDIO_CODEC_SUPPORT_SYNC
    u32 timestamp;

    timestamp = a2dp_audio_update_timestamp(dec->ts_handle, seqn, audio_syncts_get_dts(dec->syncts));
    if (!dec->ts_start) {
        dec->ts_start = 1;
        dec->mix_ch_event_params[2] = timestamp;
    } else {
        audio_syncts_next_pts(dec->syncts, timestamp);
        audio_syncts_update_sample_rate(dec->syncts, a2dp_audio_sample_rate(dec->ts_handle));;
    }
    dec->timestamp = timestamp;
    /* printf("timestamp : %d, %d\n", seqn, timestamp / TIME_US_FACTOR / 625); */
#endif
    return 0;
}

static bool a2dp_audio_is_underrun(struct a2dp_dec_hdl *dec)
{
#if AUDIO_CODEC_SUPPORT_SYNC
    if (dec->ts_start != 2) {
        return false;
    }
#endif
    int underrun_time = a2dp_low_latency ? 1 : 20;
    if (sound_pcm_dev_buffered_time() < underrun_time) {
        return true;
    }
    return false;
}

static bool a2dp_bt_rx_overrun(void)
{
    return a2dp_media_get_remain_buffer_size() < 768 ? true : false;
}

static void a2dp_decoder_stream_free(struct a2dp_dec_hdl *dec, void *packet)
{
    if (packet) {
        a2dp_media_free_packet(packet);
    }

    if ((void *)packet == (void *)dec->repair_pkt) {
        dec->repair_pkt = NULL;
    }

    if (dec->repair_pkt) {
        a2dp_media_free_packet(dec->repair_pkt);
        dec->repair_pkt = NULL;
    }
    dec->repair_pkt_len = 0;
}

static void a2dp_stream_underrun_feedback(void *priv);
static int a2dp_audio_delay_time(struct a2dp_dec_hdl *dec);
#define a2dp_seqn_before(a, b)  ((a < b && (u16)(b - a) < 1000) || (a > b && (u16)(a - b) > 1000))
static int a2dp_buffered_stream_sample_rate(struct a2dp_dec_hdl *dec, u8 *from_packet, u16 *end_seqn)
{
    u8 *packet = from_packet;
    int len = 0;
    int sample_rate = 0;

    if (!dec->sample_detect) {
        return dec->sample_rate;
    }

    a2dp_frame_sample_detect_start(dec->sample_detect, a2dp_media_dump_rx_time(packet));
    while (1) {
        packet = a2dp_media_fetch_packet(&len, packet);
        if (!packet) {
            break;
        }
        sample_rate = a2dp_frame_sample_detect(dec->sample_detect, packet, len, a2dp_media_dump_rx_time(packet));
        *end_seqn = RB16(packet + 2);
    }

    /*printf("A2DP sample detect : %d - %d\n", sample_rate, dec->sample_rate);*/
    return sample_rate;
}

static int a2dp_stream_overrun_handler(struct a2dp_dec_hdl *dec, u8 **frame, int *len)
{
    u8 *packet = NULL;
    int rlen = 0;

    int msecs = 0;
    int overrun = 0;
    int sample_rate = 0;
    u16 from_seqn = RB16(dec->repair_pkt + 2);
    u16 seqn = from_seqn;
    u16 end_seqn = 0;
    overrun = 1;
    while (1) {
        msecs = a2dp_audio_delay_time(dec);
        if (msecs < (CONFIG_A2DP_DELAY_TIME + 50) && !a2dp_bt_rx_overrun()) {
            break;
        }
        overrun = 0;
        rlen = a2dp_media_try_get_packet(&packet);
        if (rlen <= 0) {
            break;
        }

        sample_rate = a2dp_buffered_stream_sample_rate(dec, dec->repair_pkt, &end_seqn);
        a2dp_decoder_stream_free(dec, NULL);
        dec->repair_pkt = packet;
        dec->repair_pkt_len = rlen;
        seqn = RB16(packet + 2);
        if (!a2dp_seqn_before(seqn, dec->overrun_seqn)) {
            *frame = packet;
            *len = rlen;
            /*printf("------> end frame : %d\n", dec->overrun_seqn);*/
            return 1;
        }

        if (/*sample_rate < (dec->sample_rate * 4 / 5) || */sample_rate > (dec->sample_rate * 4 / 3)) {
            if (a2dp_seqn_before(dec->overrun_seqn, end_seqn)) {
                dec->overrun_seqn = end_seqn;
            }
            continue;
        }
    }
    if (overrun) {
        /*putchar('+');*/
        /* dec->overrun_seqn++; */
    } else {
        /*putchar('-');*/
    }

    *frame = dec->repair_pkt;
    *len = dec->repair_pkt_len;
    return 0;
}

static int a2dp_stream_missed_handler(struct a2dp_dec_hdl *dec, u8 **frame, int *len)
{
    int msecs = a2dp_audio_delay_time(dec);
    *frame = dec->repair_pkt;
    *len = dec->repair_pkt_len;
    if ((msecs >= (dec->delay_time + 50) || a2dp_bt_rx_overrun()) || --dec->missed_num == 0) {
        /*putchar('M');*/
        return 1;
    }
    /*putchar('m');*/
    return 0;
}

static int a2dp_stream_underrun_handler(struct a2dp_dec_hdl *dec, u8 **packet)
{

    if (!a2dp_audio_is_underrun(dec)) {
        putchar('x');
        return 0;
    }
    putchar('X');
    if (dec->stream_error != A2DP_STREAM_UNDERRUN) {
        if (!dec->stream_error) {
            a2dp_stream_underrun_feedback(dec);
        }
        dec->stream_error = a2dp_low_latency ? A2DP_STREAM_LOW_UNDERRUN : A2DP_STREAM_UNDERRUN;
        dec->repair = a2dp_low_latency ? 0 : 1;
    }
    *packet = dec->repair_pkt;
    dec->repair_frames++;
    return dec->repair_pkt_len;
}

static int a2dp_stream_error_filter(struct a2dp_dec_hdl *dec, u8 new_packet, u8 *packet, int len)
{
    int err = 0;

    if (dec->coding_type == AUDIO_CODING_AAC) {
        dec->header_len = get_rtp_header_len(dec->new_frame, packet, len);
        dec->new_frame = 0;
    } else {
        dec->header_len = get_rtp_header_len(1, packet, len);
    }

    if (dec->header_len >= len) {
        printf("##A2DP header error : %d\n", dec->header_len);
        a2dp_decoder_stream_free(dec, packet);
        return -EFAULT;
    }

    u16 seqn = RB16(packet + 2);
    if (new_packet) {
        if (dec->stream_error == A2DP_STREAM_UNDERRUN) {
            int missed_frames = (u16)(seqn - dec->seqn) - 1;
            if (missed_frames > dec->repair_frames) {
                dec->stream_error = A2DP_STREAM_MISSED;
                dec->missed_num = missed_frames - dec->repair_frames + 1;
                /*printf("case 0 : %d, %d\n", missed_frames, dec->repair_frames);*/
                err = -EAGAIN;
            } else if (missed_frames < dec->repair_frames) {
                dec->stream_error = A2DP_STREAM_OVERRUN;
                dec->overrun_seqn = seqn + dec->repair_frames - missed_frames;
                /*printf("case 1 : %d, %d, seqn : %d, %d\n", missed_frames, dec->repair_frames, seqn, dec->overrun_seqn);*/
                err = -EAGAIN;
            }
        } else if (!dec->stream_error && (u16)(seqn - dec->seqn) > 1) {
            dec->stream_error = A2DP_STREAM_MISSED;
            if (a2dp_audio_delay_time(dec) < dec->delay_time) {
                dec->missed_num = (u16)(seqn - dec->seqn);
                err = -EAGAIN;
            }
            int pkt_len;
            void *head = a2dp_media_fetch_packet(&pkt_len, NULL);
            /*printf("case 2 : %d, %d, pkt : 0x%x, 0x%x\n", seqn, dec->seqn, (u32)head, (u32)packet);*/
            if (dec->missed_num > 30) {
                printf("##A serious mistake : A2DP stream missed too much, %d\n", dec->missed_num);
                dec->missed_num = 30;
            }
        }
        dec->repair_frames = 0;
    }
    if (!err && new_packet) {
        dec->seqn = seqn;
    }
    dec->repair_pkt = packet;
    dec->repair_pkt_len = len;
    return err;
}

static int a2dp_dec_get_frame(struct audio_decoder *decoder, u8 **frame)
{
    struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);
    u8 *packet = NULL;
    int len = 0;
    u8 new_packet = 0;

try_again:
    switch (dec->stream_error) {
    case A2DP_STREAM_OVERRUN:
        new_packet = a2dp_stream_overrun_handler(dec, &packet, &len);
        break;
    case A2DP_STREAM_MISSED:
        new_packet = a2dp_stream_missed_handler(dec, &packet, &len);
        break;
    default:
        len = a2dp_media_try_get_packet(&packet);
        if (len <= 0) {
            len = a2dp_stream_underrun_handler(dec, &packet);
        } else {
            a2dp_decoder_stream_free(dec, NULL);
            new_packet = 1;
        }
        break;
    }

    if (len <= 0) {
        return 0;
    }

    int err = a2dp_stream_error_filter(dec, new_packet, packet, len);
    if (err) {
        if (-err == EAGAIN) {
            dec->new_frame = 1;
            goto try_again;
        }
        return 0;
    }

    *frame = packet + dec->header_len;
    len -= dec->header_len;
    if (dec->stream_error && new_packet) {
#if AUDIO_CODEC_SUPPORT_SYNC && TCFG_USER_TWS_ENABLE
        if (dec->ts_handle) {
            tws_a2dp_share_timestamp(dec->ts_handle);
        }
#endif
        dec->stream_error = 0;
    }

    if (dec->slience_frames) {
        dec->slience_frames--;
    }
    a2dp_decoder_set_timestamp(dec, dec->seqn);

    return len;
}


static void a2dp_dec_put_frame(struct audio_decoder *decoder, u8 *frame)
{
    struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);

    if (frame) {
        if (!a2dp_media_channel_exist() || app_var.goto_poweroff_flag) {
            a2dp_decoder_stream_free(dec, (void *)(frame - dec->header_len));
        }
        /*a2dp_media_free_packet((void *)(frame - dec->header_len));*/
    }
}

static int a2dp_dec_fetch_frame(struct audio_decoder *decoder, u8 **frame)
{
    struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);
    u8 *packet = NULL;
    int len = 0;
    u32 wait_timeout = 0;

    if (!dec->start) {
        wait_timeout = jiffies + msecs_to_jiffies(500);
    }

    dec->fetch_lock = 1;
__retry_fetch:
    packet = a2dp_media_fetch_packet(&len, NULL);
    if (packet) {
        dec->header_len = get_rtp_header_len(1, packet, len);
        *frame = packet + dec->header_len;
        len -= dec->header_len;
    } else if (!dec->start) {
        if (time_before(jiffies, wait_timeout)) {
            os_time_dly(1);
            goto __retry_fetch;
        }
    }

    dec->fetch_lock = 0;
    return len;
}

static const struct audio_dec_input a2dp_input = {
    .coding_type = AUDIO_CODING_SBC,
    .data_type   = AUDIO_INPUT_FRAME,
    .ops = {
        .frame = {
            .fget = a2dp_dec_get_frame,
            .fput = a2dp_dec_put_frame,
            .ffetch = a2dp_dec_fetch_frame,
        }
    }
};

#define bt_time_before(t1, t2) \
         (((t1 < t2) && ((t2 - t1) & 0x7ffffff) < 0xffff) || \
          ((t1 > t2) && ((t1 - t2) & 0x7ffffff) > 0xffff))
#define bt_time_to_msecs(clk)   (((clk) * 625) / 1000)
#define msecs_to_bt_time(m)     (((m + 1)* 1000) / 625)

static int a2dp_audio_delay_time(struct a2dp_dec_hdl *dec)
{
    /*struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);*/
    int msecs = 0;

#if TCFG_USER_TWS_ENABLE
    msecs = a2dp_media_get_remain_play_time(1);
#else
    msecs = a2dp_media_get_remain_play_time(0);
#endif

    if (dec->syncts) {
        msecs += sound_buffered_between_syncts_and_device(dec->syncts, 0);
    }

    msecs += sound_pcm_dev_buffered_time();

    return msecs;
}


static int a2dp_dec_rx_delay_monitor(struct audio_decoder *decoder, struct rt_stream_info *info)
{
    struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);
    int msecs = 0;
    int err = 0;

#if AUDIO_CODEC_SUPPORT_SYNC
    msecs = a2dp_audio_delay_time(dec);
    if (dec->stream_error) {
        return 0;
    }

    if (dec->sync_step == 2) {
        int distance_time = msecs - a2dp_delay_time;
        if (a2dp_bt_rx_overrun() && distance_time < 50) {
            distance_time = 50;
        }

        if (dec->ts_handle) {
            a2dp_audio_delay_offset_update(dec->ts_handle, distance_time);
        }
    }

#endif
    /*printf("%d -> %dms, delay_time : %dms\n", msecs1, msecs, a2dp_delay_time);*/
    return 0;
}
static int a2dp_decoder_stream_delay_update(struct audio_decoder *decoder)
{
    struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);
    int msecs = 0;
    int err = 0;

#if AUDIO_CODEC_SUPPORT_SYNC
    msecs = a2dp_audio_delay_time(dec);
    if (dec->stream_error) {
        return 0;
    }

    if (dec->sync_step == 2) {
        int distance_time = msecs - a2dp_delay_time;
        if (a2dp_bt_rx_overrun() && distance_time < 50) {
            distance_time = 50;
        }

        if (dec->ts_handle) {
            a2dp_audio_delay_offset_update(dec->ts_handle, distance_time);
        }
    }

#endif
    /*printf("%d -> %dms, delay_time : %dms\n", msecs1, msecs, a2dp_delay_time);*/
    return 0;
}

/*
 * A2DP 音频同步控制处理函数
 * 1.包括音频延时浮动参数;
 * 2.处理因为超时等情况丢弃音频样点;
 * 3.调用与蓝牙主机音频延时做同步的功能;
 * 4.处理TWS从机加入与解码被打断的情况。
 *
 */
static int a2dp_decoder_audio_sync_handler(struct audio_decoder *decoder)
{
    struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);
    int err;

    if (!dec->syncts) {
        return 0;
    }


    err = a2dp_decoder_stream_delay_update(decoder);
    if (err) {
        audio_decoder_suspend(decoder, 0);
        return -EAGAIN;
    }

    return 0;
}

static u16 a2dp_max_interval = 0;
#define A2DP_EST_AUDIO_CAPACITY             550//ms

static void a2dp_stream_underrun_feedback(void *priv)
{
    struct a2dp_dec_hdl *dec = (struct a2dp_dec_hdl *)priv;

    dec->underrun_feedback = 1;

    if (a2dp_delay_time < a2dp_max_interval + 50) {
        a2dp_delay_time = a2dp_max_interval + 50;
    } else {
        a2dp_delay_time += 50;
    }
    if (a2dp_delay_time > A2DP_EST_AUDIO_CAPACITY) {
        a2dp_delay_time = A2DP_EST_AUDIO_CAPACITY;
    }
}

/*void a2dp_stream_interval_time_handler(int time)*/
void reset_a2dp_sbc_instant_time(u16 time)
{
    if (a2dp_max_interval < time) {
        a2dp_max_interval = time;
        if (a2dp_max_interval > 350) {
            a2dp_max_interval = 350;
        }
#if A2DP_FLUENT_STREAM_MODE
        if (a2dp_max_interval > a2dp_delay_time) {
            a2dp_delay_time = a2dp_max_interval + 5;
            if (a2dp_delay_time > A2DP_EST_AUDIO_CAPACITY) {
                a2dp_delay_time = A2DP_EST_AUDIO_CAPACITY;
            }
        }
#endif
        /*printf("Max : %dms\n", time);*/
    }
}

static void a2dp_stream_stability_detect(void *priv)
{
    struct a2dp_dec_hdl *dec = (struct a2dp_dec_hdl *)priv;

    if (dec->underrun_feedback) {
        dec->underrun_feedback = 0;
        return;
    }

    if (a2dp_delay_time > dec->delay_time) {
        if (a2dp_max_interval < a2dp_delay_time) {
            a2dp_delay_time -= 50;
            if (a2dp_delay_time < dec->delay_time) {
                a2dp_delay_time = dec->delay_time;
            }

            if (a2dp_delay_time < a2dp_max_interval) {
                a2dp_delay_time = a2dp_max_interval + 5;
            }
        }
        a2dp_max_interval = dec->delay_time;
    }
}

#if AUDIO_CODEC_SUPPORT_SYNC
static void a2dp_decoder_update_base_time(struct a2dp_dec_hdl *dec)
{
    int distance_time = a2dp_low_latency ? a2dp_delay_time : (a2dp_delay_time - a2dp_media_get_remain_play_time(1));
    if (!a2dp_low_latency) {
        distance_time = a2dp_delay_time;
    } else if (distance_time < 20) {
        distance_time = 20;
    }
    dec->base_time = bt_audio_sync_lat_time() + msecs_to_bt_time(distance_time);
}

#endif
static int a2dp_decoder_stream_is_available(struct a2dp_dec_hdl *dec)
{
    int err = 0;
    u8 *packet = NULL;
    int len = 0;
    int drop = 0;

#if AUDIO_CODEC_SUPPORT_SYNC
    if (dec->sync_step) {
        return 0;
    }

    packet = (u8 *)a2dp_media_fetch_packet(&len, NULL);
    if (!packet) {
        return -EINVAL;
    }

    dec->seqn = RB16(packet + 2);
    if (dec->ts_handle) {
#if TCFG_USER_TWS_ENABLE
        if (!tws_network_audio_was_started() && !a2dp_audio_timestamp_is_available(dec->ts_handle, dec->seqn, 0, &drop)) {
            if (drop) {
                local_irq_disable();
                u8 *check_packet = (u8 *)a2dp_media_fetch_packet(&len, NULL);
                if (check_packet && RB16(check_packet + 2) == dec->seqn) {
                    a2dp_media_free_packet(packet);
                }
                local_irq_enable();
                a2dp_decoder_update_base_time(dec);
                a2dp_audio_set_base_time(dec->ts_handle, dec->base_time);
            }
            return -EINVAL;
        }
#endif
    }
    dec->sync_step = 2;
#endif
    return 0;
}

static int a2dp_dec_probe_handler(struct audio_decoder *decoder)
{
    struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);
    int err = 0;

    err = a2dp_decoder_stream_is_available(dec);
    if (err) {
        audio_decoder_suspend(decoder, 0);
        return err;
    }

    err = a2dp_decoder_audio_sync_handler(decoder);
    if (err) {
        audio_decoder_suspend(decoder, 0);
        return err;
    }

    dec->new_frame = 1;

    a2dp_dec_set_output_channel(dec);

    return err;
}

/*
*********************************************************************
*                  蓝牙音乐解码输出
* Description: a2dp高级音频解码输出句柄
* Arguments  :
* Return	 : 返回后级消耗的解码数据长度
* Note(s)    : None.
*********************************************************************
*/
#if 0
static int a2dp_output_after_syncts_filter(void *priv, void *data, int len)
{
    int wlen = 0;
    int drop_len = 0;
    struct a2dp_dec_hdl *dec = (struct a2dp_dec_hdl *)priv;

#if TCFG_EQ_ENABLE&&TCFG_BT_MUSIC_EQ_ENABLE
    if (dec->eq_drc && dec->eq_drc->async) {//异步方式eq
        return eq_drc_run(dec->eq_drc, data, len);
    }
#endif//TCFG_BT_MUSIC_EQ_ENABLE

    return audio_mixer_ch_write(&dec->mix_ch, data, len);
}
#endif

static int a2dp_output_before_syncts_handler(struct a2dp_dec_hdl *dec, void *data, int len)
{
#if AUDIO_CODEC_SUPPORT_SYNC
    if (dec->ts_start == 1) {
        u32 timestamp = dec->timestamp;
        u32 current_time = ((bt_audio_sync_lat_time() + (30000 / 625)) & 0x7ffffff) * 625 * TIME_US_FACTOR;
#define time_after_timestamp(t1, t2) \
        ((t1 > t2 && ((t1 - t2) < (10000000L * TIME_US_FACTOR))) || (t1 < t2 && ((t2 - t1) > (10000000L * TIME_US_FACTOR))))
        if (!time_after_timestamp(timestamp, current_time)) {//时间戳与当前解码时间相差太近，直接丢掉
            audio_syncts_resample_suspend(dec->syncts);
        } else {
            audio_syncts_resample_resume(dec->syncts);
            dec->mix_ch_event_params[2] = timestamp;
            dec->ts_start = 2;
        }
    }
#endif
    return len;
}

static void audio_filter_resume_decoder(void *priv)
{
    struct audio_decoder *decoder = (struct audio_decoder *)priv;

    audio_decoder_resume(decoder);
}

static int a2dp_decoder_slience_plc_filter(struct a2dp_dec_hdl *dec, void *data, int len)
{
    if (len == 0) {
        a2dp_decoder_stream_free(dec, NULL);
        if (!dec->stream_error) {
            dec->stream_error = A2DP_STREAM_DECODE_ERR;
            dec->repair = 1;
        }
        return 0;
    }
    if (dec->stream_error) {
        memset(data, 0x0, len);
    }
#if TCFG_USER_TWS_ENABLE
    u8 tws_pairing = 0;
    if (dec->preempt || tws_network_audio_was_started()) {
        /*a2dp播放中副机加入，声音淡入进去*/
        tws_network_local_audio_start();
        dec->preempt = 0;
        tws_pairing = 1;
        memset(data, 0x0, len);
        dec->slience_frames = 30;
    }
#endif
#if A2DP_AUDIO_PLC_ENABLE
    if (dec->plc_ops) {
        if (dec->slience_frames) {
            dec->plc_ops->run(dec->plc_mem, data, data, len >> 1, 2);
        } else if (dec->stream_error) {
            dec->plc_ops->run(dec->plc_mem, data, data, len >> 1, dec->repair ? 1 : 2);
            dec->repair = 0;
        } else {
            dec->plc_ops->run(dec->plc_mem, data, data, len >> 1, 0);
        }
    }
#else
    if (dec->slience_frames) {
        memset(data, 0x0, len);
    }
#endif
    return len;
}

#if 0
static int a2dp_output_async_data_handler(struct a2dp_dec_hdl *dec, s16 *data, int len)
{
    int offset = 0;
    int wlen = 0;
    int data_len = len;

#if (CONFIG_AUDIO_EFFECT_TASK_ENABLE == 0)
#if CONFIG_TWS_SPATIAL_AUDIO_ENABLE
    if (dec->spatial_audio) {
        data_len = dec->spatial_data_len;
    }
#endif
#endif/*CONFIG_AUDIO_EFFECT_TASK_ENABLE*/

#if AUDIO_CODEC_SUPPORT_SYNC
    if (dec->syncts) {
        int wlen = audio_syncts_frame_filter(dec->syncts, data, data_len);
        if (wlen < data_len) {
            audio_syncts_trigger_resume(dec->syncts, (void *)&dec->decoder, audio_filter_resume_decoder);
        }
#if (CONFIG_AUDIO_EFFECT_TASK_ENABLE == 0)
#if CONFIG_TWS_SPATIAL_AUDIO_ENABLE
        if (dec->spatial_audio) {
            dec->spatial_data_len -= wlen;
        }
#endif /*CONFIG_TWS_SPATIAL_AUDIO_ENABLE*/
#endif/*CONFIG_AUDIO_EFFECT_TASK_ENABLE*/
        dec->remain = wlen < data_len ? 1 : 0;
        return dec->remain ? wlen : len;
    }
#endif
    wlen = a2dp_output_after_syncts_filter(dec, data, data_len);
#if (CONFIG_AUDIO_EFFECT_TASK_ENABLE == 0)
#if CONFIG_TWS_SPATIAL_AUDIO_ENABLE
    dec->spatial_data_len -= wlen;
#endif
#endif/*CONFIG_AUDIO_EFFECT_TASK_ENABLE*/
    dec->remain = wlen < data_len ? 1 : 0;
    return dec->remain ? wlen : len;

}
#endif

static void a2dp_stream_node_init(struct a2dp_dec_hdl *dec)
{
    struct a2dp_stream_node *node = &dec->stream_node[A2DP_NODE_DECODER];

    memset(dec->stream_node, 0x0, sizeof(dec->stream_node));

    node->priv = (void *)&dec->decoder;
    node->id = A2DP_NODE_DECODER;
    node->wakeup = (void (*)(void *))audio_decoder_resume;
    node->attribute = A2DP_NODE_BEGIN;

    node = &dec->stream_node[A2DP_NODE_MAX];
    node->attribute = A2DP_NODE_END;
    node->id = A2DP_NODE_MAX;
    dec->node_num = 1;
}

static void a2dp_stream_node_add(struct a2dp_dec_hdl *dec,
                                 void *priv,
                                 int (*data_handler)(void *, void *, int),
                                 void (*wakeup)(void *),
                                 int (*set_wakeup_handler)(void *, void *, void (*wakeup)(void *)),
                                 u8 attribute)
{
    struct a2dp_stream_node *node = &dec->stream_node[dec->node_num++];
    node->id = dec->node_num - 1;
    node->priv = priv;
    node->data_handler = data_handler;
    node->wakeup = wakeup;
    node->set_wakeup_handler = set_wakeup_handler;
    node->attribute |= attribute;
}

static void a2dp_stream_node_wakeup(void *priv)
{
    struct a2dp_stream_node *node = (struct a2dp_stream_node *)priv;
    struct a2dp_stream_node *prev_node = node - 1;
    while (1) {
        if (prev_node->wakeup) {
            prev_node->wakeup(prev_node->priv);
            break;
        }
        if (prev_node->attribute & A2DP_NODE_BEGIN) {
            break;
        }
        prev_node--;
    }
}

static void a2dp_stream_node_block_and_remain(struct a2dp_stream_node *node)
{
    struct a2dp_stream_node *prev_node = node - 1;

    if (node->set_wakeup_handler) {
        node->set_wakeup_handler(node->priv, node, a2dp_stream_node_wakeup);
    }

    while (1) {
        prev_node->remain = 1;
        if ((prev_node->attribute & A2DP_NODE_BEGIN) || (prev_node->attribute & A2DP_NODE_BLOCK)) {
            break;
        }
        prev_node--;
    }
}

static int a2dp_stream_node_data_handler(void *priv, void *data, int len)
{
    struct a2dp_stream_node *node = (struct a2dp_stream_node *)priv;

    if (node->attribute & A2DP_NODE_END) {
        return len;
    }

    while (1) {
        node++;
        if ((node->attribute & A2DP_NODE_END)) {
            break;
        }

        if (!node->data_handler) {
            continue;
        }

        if ((node->attribute & A2DP_NODE_BLOCK)) {
            int wlen = node->data_handler(node->priv, data, len);
            if (wlen < len) {
                a2dp_stream_node_block_and_remain(node);
            }

            return wlen;
        }

        if (!node->remain) {
            node->data_handler(node->priv, data, len);
        }
        node->remain = 0;
        if (node->attribute & A2DP_NODE_SUB_STREAM_END) {
            break;
        }
    }

    return len;
}

static int a2dp_dec_output_handler(struct audio_decoder *decoder, s16 *data, int len, void *priv)
{
    int wlen = 0;
    struct a2dp_dec_hdl *dec = container_of(decoder, struct a2dp_dec_hdl, decoder);

    return a2dp_stream_node_data_handler(&dec->stream_node[A2DP_NODE_DECODER], data, len);
#if 0
    if (!dec->remain) {
        wlen = a2dp_decoder_slience_plc_filter(dec, data, len);
        if (wlen < len) {
            return wlen;
        }
#if (CONFIG_AUDIO_EFFECT_TASK_ENABLE == 0)
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
        if (dec->spatial_audio) {
            dec->spatial_data_len = spatial_audio_filter(dec->spatial_audio, data, len);
        }
#endif
#endif/*CONFIG_AUDIO_EFFECT_TASK_ENABLE*/

#if (TCFG_AUDIO_SPATIAL_EFFECT_ENABLE && TCFG_SENSOR_DATA_READ_IN_DEC_TASK)
        aud_spatial_sensor_run();
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE && TCFG_SENSOR_DATA_READ_IN_DEC_TASK*/

//开空间音效的时候，软件数字音量在空间音效处理后面做
#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
        if (!spatial_audio_enable) {
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
            audio_digital_vol_run(MUSIC_DVOL, data, len);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/
        }
#endif /*CONFIG_AUDIO_EFFECT_TASK_ENABLE*/
        a2dp_surround_vbass_handler(dec, data, len);

#if TCFG_EQ_ENABLE&&TCFG_BT_MUSIC_EQ_ENABLE
        if (dec->eq_drc && !dec->eq_drc->async) {//同步方式eq
            eq_drc_run(dec->eq_drc, data, len);
        }
#endif//TCFG_BT_MUSIC_EQ_ENABLE

        a2dp_output_before_syncts_handler(dec, data, len);
    }

#if TCFG_AUDIO_ANC_ENABLE && ANC_MUSIC_DYNAMIC_GAIN_EN
    audio_anc_music_dynamic_gain_det(data, len);
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/

    return a2dp_output_async_data_handler(dec, data, len);
#endif
}

static int a2dp_dec_post_handler(struct audio_decoder *decoder)
{
    return 0;
}

static const struct audio_dec_handler a2dp_dec_handler = {
    .dec_probe  = a2dp_dec_probe_handler,
    .dec_output = a2dp_dec_output_handler,
    .dec_post   = a2dp_dec_post_handler,
};

void a2dp_dec_close();

static void a2dp_dec_release()
{
    audio_decoder_task_del_wait(&decode_task, &a2dp_dec->wait);
    a2dp_drop_frame_stop();

    local_irq_disable();
    free(a2dp_dec);
    a2dp_dec = NULL;
    local_irq_enable();
}

static void a2dp_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_DEC_EVENT_END:
        puts("AUDIO_DEC_EVENT_END\n");
        a2dp_dec_close();
        break;
    }
}


static void a2dp_decoder_delay_time_setup(struct a2dp_dec_hdl *dec)
{
#if TCFG_USER_TWS_ENABLE
    int a2dp_low_latency = tws_api_get_low_latency_state();
#endif
    if (a2dp_low_latency) {
        a2dp_delay_time = a2dp_dec->coding_type == AUDIO_CODING_AAC ? CONFIG_A2DP_DELAY_TIME_LO : CONFIG_A2DP_SBC_DELAY_TIME_LO;
        /*空间音频跑异步任务时，开低延时初始延时需要多10ms*/
#if (TCFG_AUDIO_SPATIAL_EFFECT_ENABLE && CONFIG_AUDIO_EFFECT_TASK_ENABLE)
        a2dp_delay_time += 10;
#endif

    } else {
        a2dp_delay_time = CONFIG_A2DP_DELAY_TIME;
    }
    a2dp_max_interval = 0;

    dec->delay_time = a2dp_delay_time;

    dec->detect_timer = sys_timer_add((void *)dec, a2dp_stream_stability_detect, A2DP_FLUENT_DETECT_INTERVAL);
}

static int a2dp_decoder_syncts_setup(struct a2dp_dec_hdl *dec)
{
    int err = 0;
#if AUDIO_CODEC_SUPPORT_SYNC
    a2dp_stream_node_add(dec, dec, a2dp_output_before_syncts_handler, NULL, NULL, 0);

    struct audio_syncts_params params = {0};
    params.nch = dec->ch;
    params.pcm_device = sound_pcm_sync_device_select();//PCM_INSIDE_DAC;
    if (audio_mixer_get_sample_rate(&mixer)) {
        params.rout_sample_rate = audio_mixer_get_sample_rate(&mixer);
    } else {
        params.rout_sample_rate = sound_pcm_match_sample_rate(dec->sample_rate);
    }
    params.network = AUDIO_NETWORK_BT2_1;
    params.rin_sample_rate = dec->sample_rate;
    params.priv = (void *)&dec->stream_node[dec->node_num];
    params.factor = TIME_US_FACTOR;
    params.output = a2dp_stream_node_data_handler;//a2dp_output_after_syncts_filter;

    bt_audio_sync_nettime_select(0);//0 - a2dp主机，1 - tws, 2 - BLE

    a2dp_decoder_update_base_time(dec);
    dec->ts_handle = a2dp_audio_timestamp_create(dec->sample_rate, dec->base_time, TIME_US_FACTOR);

    err = audio_syncts_open(&dec->syncts, &params);
    if (!err) {
        dec->mix_ch_event_params[0] = (u32)&dec->mix_ch;
        dec->mix_ch_event_params[1] = (u32)dec->syncts;
        dec->mix_ch_event_params[2] = dec->base_time * 625 * TIME_US_FACTOR;
        audio_mixer_ch_set_event_handler(&dec->mix_ch, (void *)dec->mix_ch_event_params, audio_mix_ch_event_handler);
    }

    dec->sync_step = 0;
    a2dp_stream_node_add(dec, dec->syncts, audio_syncts_frame_filter, NULL, audio_syncts_trigger_resume, A2DP_NODE_BLOCK);
#endif
    return err;
}

static void a2dp_decoder_syncts_free(struct a2dp_dec_hdl *dec)
{
#if AUDIO_CODEC_SUPPORT_SYNC
    if (dec->ts_handle) {
        a2dp_audio_timestamp_close(dec->ts_handle);
        dec->ts_handle = NULL;
    }
    if (dec->syncts) {
        audio_syncts_close(dec->syncts);
        dec->syncts = NULL;
    }
#endif
}

static int a2dp_decoder_plc_setup(struct a2dp_dec_hdl *dec)
{
#if A2DP_AUDIO_PLC_ENABLE
    int plc_mem_size;
    /*开空间音频没有使用独立任务处理时*/
#if (TCFG_AUDIO_SPATIAL_EFFECT_ENABLE && (!CONFIG_AUDIO_EFFECT_TASK_ENABLE))
    u8 ch = 2;
#else
    u8 ch = dec->ch;
#endif
    dec->plc_ops = get_lfaudioPLC_api();
    plc_mem_size = dec->plc_ops->need_buf(ch); // 3660bytes，请衡量是否使用该空间换取PLC处理
    dec->plc_mem = malloc(plc_mem_size);
    if (!dec->plc_mem) {
        dec->plc_ops = NULL;
        return -ENOMEM;
    }
    dec->plc_ops->open(dec->plc_mem, ch, a2dp_low_latency ? 4 : 0);
    a2dp_stream_node_add(dec, dec, a2dp_decoder_slience_plc_filter, NULL, NULL, 0);
#endif
    return 0;
}

static void a2dp_decoder_plc_free(struct a2dp_dec_hdl *dec)
{
#if A2DP_AUDIO_PLC_ENABLE
    if (dec->plc_mem) {
        free(dec->plc_mem);
        dec->plc_mem = NULL;
    }
#endif
}

void a2dp_decoder_sample_detect_setup(struct a2dp_dec_hdl *dec)
{
    dec->sample_detect = a2dp_sample_detect_open(dec->sample_rate, dec->coding_type);
}

void a2dp_decoder_sample_detect_free(struct a2dp_dec_hdl *dec)
{
    if (dec->sample_detect) {
        a2dp_sample_detect_close(dec->sample_detect);
        dec->sample_detect = NULL;
    }
}

static int a2dp_decoder_spatial_data_handler(void *priv, void *data, int len)
{
    struct a2dp_dec_hdl *dec = (struct a2dp_dec_hdl *)priv;
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE

    if (!dec->spatial_data_len) {
        dec->spatial_data_len = spatial_audio_filter(dec->spatial_audio, data, len);
        dec->spatial_data_offset = 0;
    }

    if (dec->spatial_data_len) {
        int wlen = a2dp_stream_node_data_handler(&dec->stream_node[dec->spatial_node_id], (u8 *)data + dec->spatial_data_offset, dec->spatial_data_len);
        dec->spatial_data_offset += wlen;
        dec->spatial_data_len -= wlen;

        return dec->spatial_data_len ? 0 : len;
    }

    return 0;
#endif
}

void a2dp_decoder_spatial_audio_setup(struct a2dp_dec_hdl *dec)
{
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    if (spatial_audio_enable) {
        dec->spatial_audio = spatial_audio_open();
        a2dp_spatial_audio_head_tracked_en(spatial_audio_head_tracked);
        /*clk_set("sys", SYS_128M);*/
#if CONFIG_TWS_SPATIAL_AUDIO_ENABLE
        if (tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED) {
            spatial_audio_set_mapping_channel(dec->spatial_audio, tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R);
        } else {
            spatial_audio_set_mapping_channel(dec->spatial_audio, AUDIO_CH_MIX_MONO);
        }
#endif
        a2dp_stream_node_add(dec, dec, a2dp_decoder_spatial_data_handler, NULL, NULL, A2DP_NODE_BLOCK);
        dec->spatial_node_id = dec->node_num - 1;
    }
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/
}

static void a2dp_stream_read_spatial_audio_setup(struct a2dp_dec_hdl *dec)
{
#if (TCFG_AUDIO_SPATIAL_EFFECT_ENABLE && TCFG_SENSOR_DATA_READ_IN_DEC_TASK)
    if (spatial_audio_head_tracked) {
        aud_spatial_sensor_init();
        a2dp_stream_node_add(dec, NULL, aud_spatial_sensor_run, NULL, NULL, 0);
    }
#endif /*TCFG_SENSOR_DATA_READ_IN_DEC_TASK*/
}

#if (TCFG_AUDIO_SPATIAL_EFFECT_ENABLE && TCFG_SENSOR_DATA_READ_IN_DEC_TASK)
typedef struct {
    s16 sensor_buf[512];
    cbuffer_t sensor_cbuf;
    u16 period;
    u32 next_period;
} aud_sensor_t;
aud_sensor_t *aud_sensor = NULL;

int aud_spatial_sensor_init()
{
    int err = 0;
    aud_sensor = zalloc(sizeof(aud_sensor_t));
    printf("aud_spatial_sensor_init,need bufsize:%d\n", sizeof(aud_sensor_t));
    if (aud_sensor) {
        cbuf_init(&aud_sensor->sensor_cbuf, aud_sensor->sensor_buf, sizeof(aud_sensor->sensor_buf));
        aud_sensor->period = TCFG_SENSOR_DATA_READ_INTERVAL;
        aud_sensor->next_period = jiffies;
    }
    return err;
}

int aud_spatial_sensor_exit()
{
    int err = 0;
    printf("aud_spatial_sensor_exit\n");
    if (aud_sensor) {
        free(aud_sensor);
        aud_sensor = NULL;
    }
    return err;
}

extern int spatial_audio_space_data_read(void *data);
/*定时读取传感器数据到cbuf*/
static s16 sensor_tmp_data[320];
int aud_spatial_sensor_run(void *priv, void *data, int len)
{
    if (aud_sensor) {
        if (time_after(jiffies, aud_sensor->next_period)) {
            aud_sensor->next_period = jiffies + msecs_to_jiffies(aud_sensor->period);
            int len = spatial_audio_space_data_read(sensor_tmp_data);
            if (len) {
                int wlen = cbuf_write(&aud_sensor->sensor_cbuf, sensor_tmp_data, len);
            }
        }
    }
    return len;
}

/*从cbuf读取传感器数据*/
int aud_spatial_sensor_data_read(s16 *data, int len)
{
    int rlen = 0;
    if (aud_sensor) {
        local_irq_disable();
        rlen = cbuf_read(&aud_sensor->sensor_cbuf, data, len);
        local_irq_enable();
    }
    return rlen;
}

/*当前传感器数据量*/
int aud_spatial_sensor_get_data_len()
{
    int len = 0;
    if (aud_sensor) {
        len = cbuf_get_data_len(&aud_sensor->sensor_cbuf);
    }
    return len;
}
#endif /*(TCFG_AUDIO_SPATIAL_EFFECT_ENABLE && TCFG_SENSOR_DATA_READ_IN_DEC_TASK)*/

#if CONFIG_AUDIO_EFFECT_TASK_ENABLE

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#define AUD_EFFECT_INBUF_POINTS		1024
#else
#define AUD_EFFECT_INBUF_POINTS		512
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
#define AUD_EFFECT_INBUF_SIZE		(AUD_EFFECT_INBUF_POINTS << 1)
#define A2DP_AUDIO_EFFECT_RUN               1
#define A2DP_AUDIO_EFFECT_FLUSH             2
#define A2DP_AUDIO_EFFECT_STOP              3
#define AUDIO_EFFECT_RUN_LEN        (AUD_EFFECT_INBUF_SIZE / 2)
typedef struct {
    volatile u8 run_flush;
    volatile u8 need_wakeup;
    u8 output;
    u8 trigger_resume;
    u8 *buff;
    u16 remain;
    u16 run_len;
    s16 *run_buff;
    s16 output_buf[AUD_EFFECT_INBUF_POINTS];
    s16 source_buf[AUD_EFFECT_INBUF_POINTS * 2];
    cbuffer_t source_cbuf;
    cbuffer_t output_cbuf;
    void *entry_stream;
    int (*entry_data_handler)(void *priv, void *data, int len);
    void *output_priv;
    int (*output_data_handler)(void *priv, void *data, int len);
    void *resume_priv;
    void (*resume_handler)(void *priv);
    void *parent;
    spinlock_t lock;

} aud_effect_t;
/*aud_effect_t *aud_effect = NULL;*/

int aud_effect_push_data(void *priv, s16 *data, u16 len)
{
    int wlen = 0;
    aud_effect_t *effect = (struct aud_effect *)priv;

    if (!effect) {
        return 0;
    }

    /*数据写入音效task的音源缓冲*/
    wlen = cbuf_write(&effect->source_cbuf, data, len);
    if (wlen < len) {
        spin_lock(&effect->lock);
        effect->trigger_resume = 1;
        spin_unlock(&effect->lock);
    } else {
        os_taskq_post_msg("aud_effect", 2, A2DP_AUDIO_EFFECT_RUN, (int)effect);
    }

    do {
        if (!effect->output) { /*还未确认运行数据长度*/
            break;
        }

        if (!effect->remain) {
            effect->run_len = AUDIO_EFFECT_RUN_LEN;
            /*上次输出缓冲已消耗完*/
            if (cbuf_get_data_len(&effect->output_cbuf) < effect->run_len) {
                /*输出缓冲不足一次音效run的长度,这里主要使用音效一次run的长度作为运行的基数*/
                break;
            }
            /*复用循环buffer作为输出的缓冲暂存*/
            effect->buff = cbuf_get_readptr(&effect->output_cbuf);
            effect->remain = effect->run_len;
        }

        int output_len = effect->output_data_handler(effect->output_priv, effect->buff, effect->remain);
        /*printf("%d - %d\n", effect->remain, output_len);*/
        effect->remain -= output_len;
        effect->buff += output_len;
        if (effect->remain) {
            break;
        }
        /*输出完成后更新读指针并刷新至音效task*/
        cbuf_read_updata(&effect->output_cbuf, effect->run_len);
        spin_lock(&effect->lock);
        if (effect->need_wakeup) {
            effect->need_wakeup = 0;
            os_taskq_post_msg("aud_effect", 2, A2DP_AUDIO_EFFECT_RUN, (int)effect);
        }
        spin_unlock(&effect->lock);
    } while (1);

    return wlen;
}

static void aud_effect_data_flush(aud_effect_t *effect)
{
    if (effect->need_wakeup) {
        effect->need_wakeup = 0;
    }
}

static int aud_effect_output_data_handler(void *priv, void *data, int len)
{
    aud_effect_t *effect = (aud_effect_t *)priv;
    if (!effect->output) {
        effect->output = 1;
    }

    spin_lock(&effect->lock);
    if (!cbuf_is_write_able(&effect->output_cbuf, len)) {
        effect->need_wakeup = 1;
        spin_unlock(&effect->lock);
        return 0;
    }
    spin_unlock(&effect->lock);

    return cbuf_write(&effect->output_cbuf, data, len);
}

static void aud_effect_data_handler(void *priv)
{
    aud_effect_t *effect = (aud_effect_t *)priv;
    struct a2dp_dec_hdl *dec = (struct a2dp_dec_hdl *)effect->parent;

    if (!effect) {
        return;
    }

    while (1) {
        if (cbuf_get_data_len(&effect->source_cbuf) < AUDIO_EFFECT_RUN_LEN) {
            return;
        }

        effect->run_buff = cbuf_get_readptr(&effect->source_cbuf);
        int run_len = AUDIO_EFFECT_RUN_LEN;

#if 0
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
        if (dec->spatial_audio) {
            run_len = spatial_audio_filter(dec->spatial_audio, effect->run_buff, run_len);
        }
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/
#else
        if (effect->entry_data_handler) {
            run_len = effect->entry_data_handler(effect->entry_stream, effect->run_buff, run_len);
        }
#endif

        /*run_len = aud_effect_output_data_handler(effect, effect->run_buff, run_len);*/
        /*printf("run : %d\n", run_len);*/
        cbuf_read_updata(&effect->source_cbuf, run_len);
        spin_lock(&effect->lock);
        if (effect->trigger_resume) {
            if (run_len) {
                effect->resume_handler(effect->resume_priv);
            }
            effect->trigger_resume = 0;
        }
        spin_unlock(&effect->lock);
        if (run_len < AUDIO_EFFECT_RUN_LEN) {
            break;
        }
    }
}

static void aud_effect_task(void *p)
{
    printf("===Audio Effect Task===\n");
    int wlen = 0;
    u8 pend = 1;
    int msg[16];
    int res;

    while (1) {
        if (pend) {
            res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        } else {
            res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        }

        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case A2DP_AUDIO_EFFECT_RUN:
                aud_effect_data_handler((void *)msg[2]);
                break;
            case A2DP_AUDIO_EFFECT_FLUSH:
                aud_effect_data_flush((aud_effect_t *)msg[2]);
                break;
            case A2DP_AUDIO_EFFECT_STOP:
                os_taskq_flush();
                os_sem_post((OS_SEM *)msg[2]);
                break;
            default:
                break;
            }
        }
    }
}

#endif /*CONFIG_AUDIO_EFFECT_TASK_ENABLE*/

int a2dp_decoder_effect_task_enter(struct a2dp_dec_hdl *dec)
{
    int err = 0;

#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
    aud_effect_t *aud_effect = zalloc(sizeof(aud_effect_t));
    printf("audio_dec_effect_process_start,need bufsize:%d\n", sizeof(aud_effect_t));
    if (aud_effect) {
        cbuf_init(&aud_effect->source_cbuf, aud_effect->source_buf, sizeof(aud_effect->source_buf));
        cbuf_init(&aud_effect->output_cbuf, aud_effect->output_buf, sizeof(aud_effect->output_buf));
        err = task_create(aud_effect_task, NULL, "aud_effect");

        aud_effect->parent = (void *)dec;
        aud_effect->resume_handler = (void (*)(void *))a2dp_stream_node_wakeup;
        aud_effect->resume_priv = (void *)&dec->stream_node[dec->node_num];
        aud_effect->entry_stream = (void *)&dec->stream_node[dec->node_num];
        aud_effect->entry_data_handler = a2dp_stream_node_data_handler;
        spin_lock_init(&aud_effect->lock);

        dec->aud_effect = aud_effect;
        a2dp_stream_node_add(dec, aud_effect, aud_effect_push_data, NULL, NULL, A2DP_NODE_SUB_STREAM_BEGIN | A2DP_NODE_BLOCK);
    }
#endif

    return err;
}

int a2dp_decoder_effect_task_output(struct a2dp_dec_hdl *dec)
{
#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
    if (dec->aud_effect) {
        aud_effect_t *aud_effect = (aud_effect_t *)dec->aud_effect;
        aud_effect->output_data_handler = a2dp_stream_node_data_handler;
        aud_effect->output_priv = (void *)&dec->stream_node[dec->node_num];
        a2dp_stream_node_add(dec, aud_effect, aud_effect_output_data_handler, NULL, NULL, A2DP_NODE_SUB_STREAM_END | A2DP_NODE_BLOCK);
    }
#endif
    return 0;
}

int audio_effect_process_stop(struct a2dp_dec_hdl *dec)
{
    int err = 0;
#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
    aud_effect_t *aud_effect = (aud_effect_t *)dec->aud_effect;
    printf("audio_effect_process_stop\n");
    if (aud_effect) {
        OS_SEM *sem = (OS_SEM *)malloc(sizeof(OS_SEM));
        os_sem_create(sem, 0);
        /*处理消息队列满了的情况*/
        while (os_taskq_post_msg("aud_effect", 2, A2DP_AUDIO_EFFECT_STOP, (int)sem)) {
            os_time_dly(2);
        }
        os_sem_pend(sem, 0);
        free(sem);
        task_kill("aud_effect");
        free(aud_effect);
        aud_effect = NULL;
    }
    dec->aud_effect = NULL;
#endif
    return err;
}

static int a2dp_decoder_volume_run(void *priv, void *data, int len)
{
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_run((u8)priv, data, len);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/
    return len;
}

static void a2dp_decoder_volume_setup(struct a2dp_dec_hdl *dec)
{
    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_open(MUSIC_DVOL, app_audio_get_volume(APP_AUDIO_STATE_MUSIC), MUSIC_DVOL_MAX, MUSIC_DVOL_FS, -1);
    a2dp_stream_node_add(dec, (void *)MUSIC_DVOL, a2dp_decoder_volume_run, NULL, NULL, 0);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

}


static int a2dp_anc_dynamic_gain_handler(void *priv, void *data, int len)
{
#if TCFG_AUDIO_ANC_ENABLE && ANC_MUSIC_DYNAMIC_GAIN_EN
    audio_anc_music_dynamic_gain_det(data, len);
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/
    return len;
}

static void a2dp_anc_dynamic_gain_setup(struct a2dp_dec_hdl *dec)
{
#if TCFG_AUDIO_ANC_ENABLE && ANC_MUSIC_DYNAMIC_GAIN_EN
    audio_anc_music_dynamic_gain_init(dec->sample_rate);
    a2dp_stream_node_add(dec, 0, a2dp_anc_dynamic_gain_handler, NULL, NULL, 0);
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/
}
static int a2dp_surround_vbass_handler(struct a2dp_dec_hdl *dec, void *data, int len);
static void a2dp_surround_vbass_setup(struct a2dp_dec_hdl *dec)
{
#if AUDIO_SURROUND_CONFIG
    dec->sur = audio_surround_setup(dec->channel, a2dp_surround_eff);
#endif//AUDIO_SURROUND_CONFIG

#if AUDIO_VBASS_CONFIG
    dec->vbass = audio_vbass_setup(dec->sample_rate, dec->ch);
#endif//AUDIO_VBASS_CONFIG

#if AUDIO_SURROUND_CONFIG || AUDIO_VBASS_CONFIG
    a2dp_stream_node_add(dec, dec, a2dp_surround_vbass_handler, NULL, NULL, 0);
#endif
}

static int a2dp_surround_vbass_handler(struct a2dp_dec_hdl *dec, void *data, int len)
{
#if AUDIO_SURROUND_CONFIG
    if (dec->sur && dec->sur->surround) {
        audio_surround_run(dec->sur->surround, data, len);
    }
#endif/*AUDIO_SURROUND_CONFIG*/

#if AUDIO_VBASS_CONFIG
    if (dec->vbass) {
        if (!dec->vbass->status) {
            struct virtual_bass_tool_set *parm = get_vbass_parm();
            GainProcess_16Bit(data, data, powf(10, parm->prev_gain / 20.0f), dec->ch, (dec->ch == 1 ? 1 : 2), (dec->ch == 1 ? 1 : 2), (len >> 1) / dec->ch);
        }

        audio_vbass_run(dec->vbass, data, len);
    }
#endif/*AUDIO_VBASS_CONFIG*/

    return len;
}

static void a2dp_eq_drc_setup(struct a2dp_dec_hdl *dec)
{
#if TCFG_EQ_ENABLE&&TCFG_BT_MUSIC_EQ_ENABLE
    u8 drc_en = 0;
#if TCFG_DRC_ENABLE&&TCFG_BT_MUSIC_DRC_ENABLE
    drc_en = 1;
#endif//TCFG_BT_MUSIC_DRC_ENABLE

    a2dp_decoder_sample_detect_setup(dec);
    int async = 1;
#if defined(AUDIO_GAME_EFFECT_CONFIG) && AUDIO_GAME_EFFECT_CONFIG
    if (a2dp_low_latency) {
        drc_en |= BIT(1);
        async = 0;
    }
#endif /*AUDIO_GAME_EFFECT_CONFIG*/

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
    async = 0;
#endif
    u32 sample_rate = dec->sample_rate;
    if (audio_mixer_get_sample_rate(&mixer)) {
        sample_rate = audio_mixer_get_sample_rate(&mixer);
    }
#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
    if (1) {//spatial_audio_enable) {
        /*audio_effect_process_start(dec, &dec->mix_ch, audio_mixer_ch_write);*/
        dec->eq_drc = dec_eq_drc_setup((void *)&dec->stream_node[dec->node_num], (eq_output_cb)a2dp_stream_node_data_handler, sample_rate, dec->ch, async, drc_en);
        a2dp_stream_node_add(dec, dec->eq_drc, eq_drc_run, NULL, NULL, async ? A2DP_NODE_BLOCK : 0);
    } else
#endif/*CONFIG_AUDIO_EFFECT_TASK_ENABLE*/
    {
        dec->eq_drc = dec_eq_drc_setup((void *)&dec->stream_node[dec->node_num], (eq_output_cb)a2dp_stream_node_data_handler, sample_rate, dec->ch, async, drc_en);
        a2dp_stream_node_add(dec, dec->eq_drc, eq_drc_run, NULL, NULL, async ? A2DP_NODE_BLOCK : 0);
    }

#endif//TCFG_BT_MUSIC_EQ_ENABLE
}

static void a2dp_bass_bost_setup(struct a2dp_dec_hdl *dec)
{
#if defined(TCFG_AUDIO_BASS_BOOST)&&TCFG_AUDIO_BASS_BOOST
    dec->bass_boost = audio_bass_boost_open(dec->sample_rate, dec->ch);
    a2dp_stream_node_add(dec, dec->bass_boost, audio_bass_boost_run, NULL, NULL, 0);
#endif
}

static int a2dp_effect_develop_setup(struct a2dp_dec_hdl *dec)
{
#if ((defined TCFG_EFFECT_DEVELOP_ENABLE) && TCFG_EFFECT_DEVELOP_ENABLE)
    dec->effect_develop = audio_effect_develop_open(dec->sample_rate, dec->ch, 16);
    a2dp_stream_node_add(dec, dec->effect_develop, audio_effect_develop_data_handler, NULL, NULL, 0);
#endif
    return 0;
}

static void a2dp_effect_develop_close(struct a2dp_dec_hdl *dec)
{
#if ((defined TCFG_EFFECT_DEVELOP_ENABLE) && TCFG_EFFECT_DEVELOP_ENABLE)
    if (dec->effect_develop) {
        audio_effect_develop_close(dec->effect_develop);
        dec->effect_develop = NULL;
    }
#endif
}

void audio_overlay_load_code(u32 type);
int a2dp_dec_start()
{
    int err;
    struct audio_fmt *fmt;
    enum audio_channel channel;
    struct a2dp_dec_hdl *dec = a2dp_dec;

    if (!a2dp_dec) {
        return -EINVAL;
    }

    a2dp_drop_frame_stop();
    puts("a2dp_dec_start: in\n");

    if (a2dp_dec->coding_type == AUDIO_CODING_AAC) {
        audio_overlay_load_code(a2dp_dec->coding_type);
    }

    err = audio_decoder_open(&dec->decoder, &a2dp_input, &decode_task);
    if (err) {
        goto __err1;
    }
    dec->channel = AUDIO_CH_MAX;

    audio_decoder_set_handler(&dec->decoder, &a2dp_dec_handler);
    audio_decoder_set_event_handler(&dec->decoder, a2dp_dec_event_handler, 0);

    if (a2dp_dec->coding_type != a2dp_input.coding_type) {
        struct audio_fmt f = {0};
        f.coding_type = a2dp_dec->coding_type;
        err = audio_decoder_set_fmt(&dec->decoder, &f);
        if (err) {
            goto __err2;
        }
    }

    err = audio_decoder_get_fmt(&dec->decoder, &fmt);
    if (err) {
        goto __err2;
    }

    if (fmt->sample_rate == 0) {
        log_w("A2DP stream maybe error\n");
        goto __err2;
    }
    //dac_hdl.dec_channel_num = fmt->channel;
    dec->sample_rate = fmt->sample_rate;

    a2dp_dec_set_output_channel(dec);

    a2dp_stream_node_init(dec);

    a2dp_decoder_delay_time_setup(dec);

    a2dp_decoder_plc_setup(dec);

#if !CONFIG_AUDIO_EFFECT_TASK_ENABLE /*当空间音效不在异步task中时放在解码后第二个节点效率最高*/
    a2dp_decoder_spatial_audio_setup(dec);
#endif
    a2dp_stream_read_spatial_audio_setup(dec);

    set_source_sample_rate(fmt->sample_rate);

    /*sound_pcm_set_underrun_params(3, dec, NULL);//a2dp_stream_underrun_feedback);*/

    audio_mixer_ch_open(&dec->mix_ch, &mixer);
    audio_mixer_ch_set_sample_rate(&dec->mix_ch, A2DP_SOUND_SAMPLE_RATE ? A2DP_SOUND_SAMPLE_RATE : sound_pcm_match_sample_rate(fmt->sample_rate));

#if !CONFIG_AUDIO_EFFECT_TASK_ENABLE
    a2dp_decoder_volume_setup(dec);
#endif

    a2dp_effect_develop_setup(dec);

    a2dp_surround_vbass_setup(dec);
    a2dp_bass_bost_setup(dec);

    a2dp_anc_dynamic_gain_setup(dec);

    a2dp_decoder_syncts_setup(dec);

    a2dp_eq_drc_setup(dec);

    a2dp_decoder_effect_task_enter(dec);    /*enter到output的节点全部在effect task中执行*/

#if CONFIG_AUDIO_EFFECT_TASK_ENABLE
    a2dp_decoder_spatial_audio_setup(dec);

    a2dp_decoder_volume_setup(dec); /*数字音量处理放在空间音频之后处理*/
#endif

    a2dp_decoder_effect_task_output(dec);

    a2dp_stream_node_add(dec, &dec->mix_ch, (int (*)(void *, void *, int))audio_mixer_ch_write, NULL, NULL, A2DP_NODE_BLOCK);
    audio_mixer_ch_set_resume_handler(&dec->mix_ch, &dec->stream_node[dec->node_num - 1], a2dp_stream_node_wakeup);

    a2dp_drop_frame_stop();
    dec->remain = 0;
    audio_codec_clock_set(AUDIO_A2DP_MODE, dec->coding_type, dec->wait.preemption);
    /* dec->state = A2DP_STREAM_START; */

    err = audio_decoder_start(&dec->decoder);
    if (err) {
        goto __err3;
    }

    if (get_sniff_out_status()) {
        clear_sniff_out_status();
    }
    sound_pcm_dev_try_power_on();
    dec->start = 1;

    return 0;

__err3:
    audio_mixer_ch_close(&a2dp_dec->mix_ch);
__err2:
    audio_decoder_close(&dec->decoder);
__err1:
    a2dp_dec_release();

    return err;
}

static int a2dp_dut_dec_start(void)
{
    int err;
    struct audio_fmt *fmt;
    enum audio_channel channel;
    struct a2dp_dec_hdl *dec = a2dp_dec;

    if (!a2dp_dec) {
        return -EINVAL;
    }

    a2dp_drop_frame_stop();
    puts("a2dp_dut_dec_start: in\n");

    if (a2dp_dec->coding_type == AUDIO_CODING_AAC) {
        audio_overlay_load_code(a2dp_dec->coding_type);
    }

    err = audio_decoder_open(&dec->decoder, &a2dp_input, &decode_task);
    if (err) {
        goto __err1;
    }
    dec->channel = AUDIO_CH_MAX;

    audio_decoder_set_handler(&dec->decoder, &a2dp_dec_handler);
    audio_decoder_set_event_handler(&dec->decoder, a2dp_dec_event_handler, 0);

    if (a2dp_dec->coding_type != a2dp_input.coding_type) {
        struct audio_fmt f = {0};
        f.coding_type = a2dp_dec->coding_type;
        err = audio_decoder_set_fmt(&dec->decoder, &f);
        if (err) {
            goto __err2;
        }
    }

    err = audio_decoder_get_fmt(&dec->decoder, &fmt);
    if (err) {
        goto __err2;
    }

    if (fmt->sample_rate == 0) {
        log_w("A2DP stream maybe error\n");
        goto __err2;
    }
    //dac_hdl.dec_channel_num = fmt->channel;
    dec->sample_rate = fmt->sample_rate;

    a2dp_dec_set_output_channel(dec);
    a2dp_stream_node_init(dec);

    a2dp_decoder_delay_time_setup(dec);

    a2dp_decoder_plc_setup(dec);

    set_source_sample_rate(fmt->sample_rate);

    audio_mixer_ch_open(&dec->mix_ch, &mixer);
    audio_mixer_ch_set_sample_rate(&dec->mix_ch, A2DP_SOUND_SAMPLE_RATE ? A2DP_SOUND_SAMPLE_RATE : sound_pcm_match_sample_rate(fmt->sample_rate));

    //dut 需添加数字音量处理
    a2dp_decoder_volume_setup(dec);

#if TCFG_AUDIO_CVP_DUT_ENABLE
    if (audio_dec_dut_en_get(0) == AUDIO_DEC_DUT_MODE_SPK_EQ) {
        a2dp_eq_drc_setup(dec);
    }
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/

    a2dp_decoder_syncts_setup(dec);

    a2dp_stream_node_add(dec, &dec->mix_ch, (int (*)(void *, void *, int))audio_mixer_ch_write, NULL, NULL, A2DP_NODE_BLOCK);
    audio_mixer_ch_set_resume_handler(&dec->mix_ch, &dec->stream_node[dec->node_num - 1], a2dp_stream_node_wakeup);
    dec->remain = 0;
    audio_codec_clock_set(AUDIO_A2DP_MODE, dec->coding_type, dec->wait.preemption);

    err = audio_decoder_start(&dec->decoder);
    if (err) {
        goto __err3;
    }

    sound_pcm_dev_try_power_on();
    dec->start = 1;

    return 0;

__err3:
    audio_mixer_ch_close(&a2dp_dec->mix_ch);
__err2:
    audio_decoder_close(&dec->decoder);
__err1:
    a2dp_dec_release();

    return err;
}

static int __a2dp_audio_res_close(void)
{
    a2dp_dec->start = 0;
    if (a2dp_dec->detect_timer) {
        sys_timer_del(a2dp_dec->detect_timer);
    }
    a2dp_decoder_sample_detect_free(a2dp_dec);
    audio_decoder_close(&a2dp_dec->decoder);
    audio_effect_process_stop(a2dp_dec);
    audio_mixer_ch_close(&a2dp_dec->mix_ch);
    a2dp_decoder_syncts_free(a2dp_dec);
    a2dp_decoder_plc_free(a2dp_dec);
    a2dp_effect_develop_close(a2dp_dec);
    a2dp_decoder_stream_free(a2dp_dec, NULL);
#if TCFG_EQ_ENABLE&&TCFG_BT_MUSIC_EQ_ENABLE
    if (a2dp_dec->eq_drc) {
        dec_eq_drc_free(a2dp_dec->eq_drc);
        a2dp_dec->eq_drc = NULL;
    }
#endif//TCFG_BT_MUSIC_EQ_ENABLE

#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    if (a2dp_dec->spatial_audio) {
        spatial_audio_close(a2dp_dec->spatial_audio);
        a2dp_dec->spatial_audio = NULL;
    }
#if TCFG_SENSOR_DATA_READ_IN_DEC_TASK
    aud_spatial_sensor_exit();
#endif /*TCFG_SENSOR_DATA_READ_IN_DEC_TASK*/
#endif /*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/

#if AUDIO_SURROUND_CONFIG
    if (a2dp_dec->sur) {
        audio_surround_free(a2dp_dec->sur);
        a2dp_dec->sur = NULL;
    }
#endif//AUDIO_SURROUND_CONFIG

#if AUDIO_VBASS_CONFIG
    if (a2dp_dec->vbass) {
        audio_vbass_free(a2dp_dec->vbass);
        a2dp_dec->vbass = NULL;
    }
#endif//AUDIO_VBASS_CONFIG

#if defined(TCFG_AUDIO_BASS_BOOST) && TCFG_AUDIO_BASS_BOOST
    if (a2dp_dec->bass_boost) {
        audio_bass_boost_close(a2dp_dec->bass_boost);
        a2dp_dec->bass_boost = NULL;
    }
#endif

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_close(MUSIC_DVOL);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/
#if 0
    clk_set_sys_lock(BT_NORMAL_HZ, 0);
#else
    audio_codec_clock_del(AUDIO_A2DP_MODE);
#endif

    a2dp_dec->preempt = 1;
    app_audio_state_exit(APP_AUDIO_STATE_MUSIC);
    return 0;
}

static int a2dp_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;

    if (event == AUDIO_RES_GET) {
        printf("a2dp_res:Get\n");
        if (a2dp_dec->dut_enable) {
            err = a2dp_dut_dec_start();
        } else {
            err = a2dp_dec_start();
        }
    } else if (event == AUDIO_RES_PUT) {
        printf("a2dp_res:Put\n");
        if (a2dp_dec->start) {
            __a2dp_audio_res_close();
            a2dp_drop_frame_start();
        }
    }
    return err;
}

#define A2DP_CODEC_SBC			0x00
#define A2DP_CODEC_MPEG12		0x01
#define A2DP_CODEC_MPEG24		0x02
#define A2DP_CODEC_LDAC			0x0B
extern bool a2dp_opened;
extern bool transcription_opened;
int __a2dp_dec_open(int media_type, u8 resume, u8 dut_enable)
{
#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_AND_MUSIC_MUTEX)
    /*辅听与播歌互斥时，关闭辅听*/
    if (get_hearing_aid_fitting_state() == 0) {
        audio_hearing_aid_suspend();
    }
#endif/*TCFG_AUDIO_HEARING_AID_ENABLE*/

    struct a2dp_dec_hdl *dec;

    if (strcmp(os_current_task(), "app_core") != 0) {
        log_e("a2dp dec open in task : %s\n", os_current_task());
    }

    if (a2dp_suspend) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            if (drop_a2dp_timer == 0) {
                drop_a2dp_timer = sys_timer_add(NULL,
                                                a2dp_media_clear_packet_before_seqn, 100);
            }
        }
        return 0;
    }

    if (a2dp_dec) {
        return 0;
    }

    media_type = a2dp_media_get_codec_type();
    printf("a2dp_dec_open: %d\n", media_type);

    dec = zalloc(sizeof(*dec));
    if (!dec) {
        return -ENOMEM;
    }

    switch (media_type) {
    case A2DP_CODEC_SBC:
        printf("a2dp_media_type:SBC");
        dec->coding_type = AUDIO_CODING_SBC;
        break;
    case A2DP_CODEC_MPEG24:
        printf("a2dp_media_type:AAC");
        dec->coding_type = AUDIO_CODING_AAC;
        break;
    case A2DP_CODEC_LDAC:
        printf("a2dp_media_type:LDAC");
        dec->coding_type = AUDIO_CODING_LDAC;
        break;
    default:
        printf("a2dp_media_type unsupoport:%d", media_type);
        free(dec);
        return -EINVAL;
    }

#if TCFG_USER_TWS_ENABLE
    if (CONFIG_LOW_LATENCY_ENABLE) {
        a2dp_low_latency = tws_api_get_low_latency_state();
    }
#endif
    a2dp_dec = dec;
    dec->preempt = resume ? 1 : 0;
    dec->wait.priority = 0;
    dec->wait.preemption = 1;
    dec->wait.handler = a2dp_wait_res_handler;
    dec->dut_enable = dut_enable;
    audio_decoder_task_add_wait(&decode_task, &dec->wait);


#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE)
    /*辅听验配过程不允许播放歌曲*/
    extern u8 get_hearing_aid_fitting_state(void);
    if (get_hearing_aid_fitting_state()) {
        printf("hearing aid fitting : %d\n !!!", get_hearing_aid_fitting_state());
        extern a2dp_tws_dec_suspend(void *p);
        a2dp_tws_dec_suspend(NULL);
        return 0;
    }
#endif /*TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_FITTING_ENABLE*/
    if (a2dp_dec && (a2dp_dec->start == 0)) {
        a2dp_drop_frame_start();
    }
    a2dp_opened = true;
    // if (transcription_opened) {
    //     clk_set_sys_lock(96 * (1000000L), 0);
    // }

    return 0;
}

int a2dp_dec_open(int media_type)
{
#if TCFG_AUDIO_SPEAK_TO_CHAT_ENABLE
    if (get_speak_to_chat_state() == AUDIO_ADT_CHAT) {
        audio_speak_to_char_sync_suspend();
    }
#endif
    return __a2dp_dec_open(media_type, 0, 0);
}

int a2dp_dut_dec_open(int media_type)
{
    return __a2dp_dec_open(media_type, 0, 1);
}

void a2dp_dec_close()
{
    if (!a2dp_dec) {
        if (CONFIG_LOW_LATENCY_ENABLE) {
            a2dp_low_latency_seqn = 0;
        }
        if (drop_a2dp_timer) {
            sys_timer_del(drop_a2dp_timer);
            drop_a2dp_timer = 0;
        }
        return;
    }

    if (a2dp_dec->start) {
        __a2dp_audio_res_close();
    }

    a2dp_dec_release();

#if TCFG_AUDIO_ANC_ENABLE && ANC_MUSIC_DYNAMIC_GAIN_EN
    audio_anc_music_dynamic_gain_reset(1);
#endif/*ANC_MUSIC_DYNAMIC_GAIN_EN*/
#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_AND_MUSIC_MUTEX)
    audio_hearing_aid_resume();
#endif/*TCFG_AUDIO_HEARING_AID_ENABLE*/

    a2dp_opened = false;
    // if (transcription_opened) {
    //     clk_set_sys_lock(96 * (1000000L), 0);
    // }
    puts("a2dp_dec_close: exit\n");
}

void a2dp_dec_dut_enable(u8 dut_enable)
{
    if (!a2dp_dec) {
        return;
    }
    if (a2dp_dec->start) {
        a2dp_dec_close();
        if (dut_enable) {
            a2dp_dut_dec_open(0);
        } else {
            a2dp_dec_open(0);
        }
    }
}

//*********************************************************************************//
//                           空间音效接口(Spatial Effect APIs)		               //
//*********************************************************************************//
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
int a2dp_spatial_audio_open(void)
{
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    spatial_audio_enable = 1;
    if (a2dp_dec && !a2dp_dec->spatial_audio) {
        a2dp_dec->spatial_audio = spatial_audio_open();
    }
#endif
    return 0;
}

void a2dp_spatial_audio_close(void)
{
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    local_irq_disable();
    if (a2dp_dec && a2dp_dec->spatial_audio) {
        spatial_audio_close(a2dp_dec->spatial_audio);
        a2dp_dec->spatial_audio = NULL;
    }
    spatial_audio_enable = 0;
    spatial_audio_head_tracked = 0;
    local_irq_enable();
#endif
}

u8 get_a2dp_spatial_audio_status(void)
{
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    if (a2dp_dec) {
        return (a2dp_dec->spatial_audio != NULL) ? 1 : 0;
    }
#endif
    return 0;
}

u8 get_spatial_audio_enable()
{
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    return spatial_audio_enable;
#endif
    return 0;
}

void a2dp_spatial_audio_head_tracked_en(u8 en)
{
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    if (a2dp_dec && a2dp_dec->spatial_audio) {
        spatial_audio_head_tracked = en;
        spatial_audio_head_tracked_en(a2dp_dec->spatial_audio, en);
    }
#endif
}

u8 get_a2dp_spatial_audio_head_tracked(void)
{
#if TCFG_AUDIO_SPATIAL_EFFECT_ENABLE
    return spatial_audio_head_tracked;
#endif
    return 0;
}

void a2dp_spatial_audio_setup(u8 en, u8 head_track)
{
    spatial_audio_enable = en;
    spatial_audio_head_tracked = head_track;
}
int a2dp_tws_dec_suspend(void *p);
void a2dp_tws_dec_resume(void);
/*
    空间音效模式切换
mode:
    0:关闭头部跟踪
    1:打开空间音效
    2:打开头部跟踪
tone_play:
    1 : 切换时播放提示音
    0 : 不播放提示音
*/
void audio_spatial_effects_mode_switch(u8 mode, u8 tone_play)
{
    u8 tone_index = 0;
    if (a2dp_dec) {
        /*先挂起再改变标志*/
        a2dp_tws_dec_suspend(a2dp_dec);
        if (mode == 0) {
            printf("SpatialAudio:close_fixed");
            a2dp_spatial_audio_setup(0, 0);
            tone_index = IDEX_TONE_NUM_0;
        } else if (mode == 1) {
            printf("SpatialAudio:open_fixed");
            a2dp_spatial_audio_setup(1, 0);
            tone_index = IDEX_TONE_NUM_1;
        } else if (mode == 2) {
            printf("SpatialAudio:open_tracked");
            a2dp_spatial_audio_setup(1, 1);
            tone_index = IDEX_TONE_NUM_2;
        }
        a2dp_tws_dec_resume();
        if (tone_play) {
            tone_play_index(tone_index, 1);
        }
    }
}
#endif/*TCFG_AUDIO_SPATIAL_EFFECT_ENABLE*/


static void a2dp_low_latency_clear_a2dp_packet(u8 *data, int len, int rx)
{
    if (rx) {
        a2dp_low_latency_seqn = (data[0] << 8) | data[1];
    }
}
REGISTER_TWS_FUNC_STUB(audio_dec_clear_a2dp_packet) = {
    .func_id = 0x132A6578,
    .func    = a2dp_low_latency_clear_a2dp_packet,
};


static void low_latency_drop_a2dp_frame(void *p)
{
    int len;

    /*y_printf("low_latency_drop_a2dp_frame\n");*/

    if (a2dp_low_latency_seqn == 0) {
        a2dp_media_clear_packet_before_seqn(0);
        return;
    }
    while (1) {
        u8 *packet = a2dp_media_fetch_packet(&len, NULL);
        if (!packet) {
            return;
        }
        u16 seqn = (packet[2] << 8) | packet[3];
        if (seqn_after(seqn, a2dp_low_latency_seqn)) {
            printf("clear_end: %d\n", seqn);
            break;
        }
        a2dp_media_free_packet(packet);
        /*printf("clear: %d\n", seqn);*/
    }

    if (drop_a2dp_timer) {
        sys_timer_del(drop_a2dp_timer);
        drop_a2dp_timer = 0;
    }
    int type = a2dp_media_get_codec_type();
    if (type >= 0) {
        /*a2dp_dec_open(type);*/
        __a2dp_dec_open(type, 1, 0);
    }

    if (a2dp_low_latency == 0) {
        tws_api_auto_role_switch_enable();
    }

    printf("a2dp_delay: %d\n", a2dp_media_get_remain_play_time(1));
}


int earphone_a2dp_codec_set_low_latency_mode(int enable, int msec)
{
    int ret = 0;
    int len, err;

    if (CONFIG_LOW_LATENCY_ENABLE == 0) {
        return -EINVAL;
    }

    if (esco_dec) {
        return -EINVAL;
    }
    if (drop_a2dp_timer) {
        return -EINVAL;
    }
    if (a2dp_suspend) {
        return -EINVAL;
    }

    a2dp_low_latency = enable;
    a2dp_low_latency_seqn = 0;

    r_printf("a2dp_low_latency: %d, %d, %d\n", a2dp_dec->seqn, a2dp_delay_time, enable);

    if (!a2dp_dec || a2dp_dec->start == 0) {
#if TCFG_USER_TWS_ENABLE
        tws_api_low_latency_enable(enable);
#endif
        return 0;
    }

    if (a2dp_dec->coding_type == AUDIO_CODING_SBC) {
        a2dp_low_latency_seqn = a2dp_dec->seqn + (msec + a2dp_delay_time) / 15;
    } else {
        a2dp_low_latency_seqn = a2dp_dec->seqn + (msec + a2dp_delay_time) / 20;
    }

#if TCFG_USER_TWS_ENABLE
    u8 data[2];
    data[0] = a2dp_low_latency_seqn >> 8;
    data[1] = a2dp_low_latency_seqn;
    err = tws_api_send_data_to_slave(data, 2, 0x132A6578);
    if (err == -ENOMEM) {
        return -EINVAL;
    }
#endif

    a2dp_dec_close();

    a2dp_media_clear_packet_before_seqn(0);

#if TCFG_USER_TWS_ENABLE
    if (enable) {
        tws_api_auto_role_switch_disable();
    }
    tws_api_low_latency_enable(enable);
#endif

    drop_a2dp_timer = sys_timer_add(NULL, low_latency_drop_a2dp_frame, 40);

    /*r_printf("clear_to_seqn: %d\n", a2dp_low_latency_seqn);*/

    return 0;
}

int earphone_a2dp_codec_get_low_latency_mode()
{
#if TCFG_USER_TWS_ENABLE
    return tws_api_get_low_latency_state();
#endif
    return a2dp_low_latency;
}


int a2dp_tws_dec_suspend(void *p)
{
    r_printf("a2dp_tws_dec_suspend\n");
    /*mem_stats();*/

    if (a2dp_suspend) {
        return -EINVAL;
    }
    a2dp_suspend = 1;

    if (a2dp_dec) {
        a2dp_dec_close();
        a2dp_media_clear_packet_before_seqn(0);
        if (tws_api_get_role() == 0) {
            drop_a2dp_timer = sys_timer_add(NULL, (void (*)(void *))a2dp_media_clear_packet_before_seqn, 100);
        }
    }

    int err = audio_decoder_fmt_lock(&decode_task, AUDIO_CODING_AAC);
    if (err) {
        log_e("AAC_dec_lock_faild\n");
    }

    return err;
}


void a2dp_tws_dec_resume(void)
{
    r_printf("a2dp_tws_dec_resume\n");

    if (a2dp_suspend) {
        a2dp_suspend = 0;

        if (drop_a2dp_timer) {
            sys_timer_del(drop_a2dp_timer);
            drop_a2dp_timer = 0;
        }

        audio_decoder_fmt_unlock(&decode_task, AUDIO_CODING_AAC);

        int type = a2dp_media_get_codec_type();
        printf("codec_type: %d\n", type);
        if (type >= 0) {
            if (tws_api_get_role() == 0) {
                a2dp_media_clear_packet_before_seqn(0);
            }
            a2dp_resume_time = jiffies + msecs_to_jiffies(80);
            /*a2dp_dec_open(type);*/
            __a2dp_dec_open(type, 1, 0);
        }
    }
}
