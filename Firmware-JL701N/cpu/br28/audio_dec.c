#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "classic/tws_api.h"
#include "classic/hci_lmp.h"
#include "effectrs_sync.h"
#include "audio_syncts.h"
#include "application/eq_config.h"
#include "application/audio_energy_detect.h"
#include "application/audio_surround.h"
#include "app_config.h"
#include "audio_config.h"
#include "aec_user.h"
#include "audio_enc.h"
#include "app_main.h"
#include "btstack/avctp_user.h"
#include "btstack/a2dp_media_codec.h"
#include "tone_player.h"
/* #include "audio_demo/audio_demo.h" */
#include "media/audio_vbass.h"
#include "audio_plc.h"
#include "audio_dec_eff.h"
#include "audio_codec_clock.h"
#include "fft_and_pca.h"
#if TCFG_AUDIO_HEARING_AID_ENABLE
#include "audio_hearing_aid.h"
#endif/*TCFG_AUDIO_HEARING_AID_ENABLE*/
#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif/*TCFG_USER_TWS_ENABLE*/
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
#include "audio_dvol.h"
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/
#if TCFG_APP_FM_EMITTER_EN
#include "fm_emitter/fm_emitter_manage.h"
#endif/*TCFG_APP_FM_EMITTER_EN*/
#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif/*TCFG_AUDIO_ANC_ENABLE*/
#if (defined(TCFG_PHONE_MESSAGE_ENABLE) && (TCFG_PHONE_MESSAGE_ENABLE))
#include "phone_message/phone_message.h"
#endif/*TCFG_PHONE_MESSAGE_ENABLE*/

#if TCFG_SMART_VOICE_ENABLE
#include "smart_voice/smart_voice.h"
#include "media/jl_kws.h"
#if TCFG_CALL_KWS_SWITCH_ENABLE
#include "btstack/avctp_user.h"
#endif
#endif
#include "media/audio_gain_process.h"

#ifdef SUPPORT_MS_EXTENSIONS
/* #pragma bss_seg(".audio_decoder_bss") */
/* #pragma data_seg(".audio_decoder_data") */
#pragma const_seg(".audio_decoder_const")
#pragma code_seg(".audio_decoder_code")
#endif/*SUPPORT_MS_EXTENSIONS*/

#include "sound_device.h"

#if TCFG_ESCO_DL_NS_ENABLE
#include "audio_ns.h"
#define DL_NS_MODE				0
#define DL_NS_NOISELEVEL		100.0f
#define DL_NS_AGGRESSFACTOR		1.0f
#define DL_NS_MINSUPPRESS		0.09f
#endif/*TCFG_ESCO_DL_NS_ENABLE*/

#if TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE
#include "audio_NoiseGate.h"
static u8 *esco_dl_noisegate_buf = NULL;
#endif/*TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE*/

#if ((defined TCFG_DIAFX_ENABLE) && TCFG_DIAFX_ENABLE)
#include "diafx.h"
#endif

#if (TCFG_AUDIO_OUTPUT_IIS)
#include "audio_iis.h"
#endif

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#include "icsd_adt_app.h"
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

#define AUDIO_CODEC_SUPPORT_SYNC	1

#define ESCO_DRC_EN	0  //通话下行增加限幅器处理，默认关闭,need en TCFG_DRC_ENABLE

#if (!TCFG_DRC_ENABLE || !TCFG_PHONE_EQ_ENABLE)
#undef ESCO_DRC_EN 0
#define ESCO_DRC_EN	0
#endif

#if TCFG_AUDIO_ANC_ENABLE && TCFG_DRC_ENABLE
#define MIX_OUT_DRC_EN   0/*mixout后增加一级限幅处理,默认关闭,need en TCFG_DRC_ENABLE*/
#else
#define MIX_OUT_DRC_EN   0/*mixout后增加一级限幅处理,默认关闭,need en TCFG_DRC_ENABLE*/
#endif/*TCFG_AUDIO_ANC_ENABLE && TCFG_DRC_ENABLE*/



#if AUDIO_OUT_EFFECT_ENABLE
int audio_out_effect_open(void *priv, u16 sample_rate);
void audio_out_effect_close(void);
int audio_out_effect_stream_clear();
struct dec_eq_drc *audio_out_effect = NULL;
#endif /*AUDIO_OUT_EFFECT_ENABLE*/


static u8 audio_out_effect_dis = 0;
static  u8 mix_out_remain = 0;

struct tone_dec_hdl {
    struct audio_decoder decoder;
    void (*handler)(void *, int argc, int *argv);
    void *priv;
};


#if AUDIO_SURROUND_CONFIG
static u8 a2dp_surround_eff;  //音效模式记录
void a2dp_surround_set(u8 eff)
{
    a2dp_surround_eff = eff;
}
#endif


struct esco_dec_hdl {
    struct audio_decoder decoder;
    struct audio_res_wait wait;
    struct audio_mixer_ch mix_ch;
    u32 coding_type;
    u8 *frame;
    u8 frame_len;
    u8 offset;
    u8 data_len;
    u8 tws_mute_en;
    u8 start;
    u8 enc_start;
    u8 frame_time;
    u8 preempt;
    u8 channels;
    u8 ul_stream_open;
    u16 slience_frames;
    u16 sample_rate;
    void *syncts;
#if AUDIO_CODEC_SUPPORT_SYNC
    void *ts_handle;
    u8 ts_start;
    u8 sync_step;
#endif
    u32 mix_ch_event_params[3];
    u8 esco_len;
    u16 remain;
    int data[15];/*ALIGNED(4)*/

#if TCFG_EQ_ENABLE&&TCFG_PHONE_EQ_ENABLE
    struct dec_eq_drc *eq_drc;
#endif//TCFG_PHONE_EQ_ENABLE

#if TCFG_ESCO_DL_NS_ENABLE
    void *dl_ns;
    s16 dl_ns_out[NS_OUT_POINTS_MAX];
    u16 dl_ns_offset;
    u16 dl_ns_remain;
#endif/*TCFG_ESCO_DL_NS_ENABLE*/

#if TCFG_AUDIO_ESCO_LIMITER_ENABLE
    struct audio_drc *limiter;
#endif
    u32 hash;
    s16 fade_trigger;
    s16 fade_volume;
    s16 fade_step;
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    u8 icsd_adt_state;
#endif

};

#if AUDIO_OUTPUT_AUTOMUTE
void *mix_out_automute_hdl = NULL;
extern void mix_out_automute_open();
extern void mix_out_automute_close();
#endif  //#if AUDIO_OUTPUT_AUTOMUTE

#define AUDIO_DAC_DELAY_TIME    30

#define DIGITAL_OdB_VOLUME              16384

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)||(TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_LR_DIFF)
#define MIX_POINTS_NUM  256
#else
#define MIX_POINTS_NUM  256
#endif

s16 mix_buff[MIX_POINTS_NUM];

/* #define MAX_SRC_NUMBER      3 */
/* s16 audio_src_hw_filt[24 * 2 * MAX_SRC_NUMBER]; */

static u16 dac_idle_delay_max = 10000;
static u16 dac_idle_delay_cnt = 10000;
static struct tone_dec_hdl *tone_dec = NULL;
struct audio_decoder_task decode_task;
extern struct audio_dac_hdl dac_hdl;
struct audio_mixer mixer;
extern struct dac_platform_data dac_data;
extern struct audio_adc_hdl adc_hdl;

struct esco_dec_hdl *esco_dec = NULL;


int lmp_private_get_esco_remain_buffer_size();
int lmp_private_get_esco_data_len();
void *lmp_private_get_esco_packet(int *len, u32 *hash);
void lmp_private_free_esco_packet(void *packet);
extern int lmp_private_esco_suspend_resume(int flag);
extern void user_sat16(s32 *in, s16 *out, u32 npoint);

void *esco_play_sync_open(u8 channel, u16 sample_rate, u16 output_rate);
//void audio_adc_init(void);
void *audio_adc_open(void *param, const char *source);
int audio_adc_sample_start(void *adc);

void set_source_sample_rate(u32 sample_rate);
u8 bt_audio_is_running(void);
void audio_resume_all_decoder(void)
{
    audio_decoder_resume_all(&decode_task);
}

void audio_src_isr_deal(void)
{
    audio_resume_all_decoder();
}

__attribute__((weak))
u8 get_sniff_out_status()
{
    return 0;
}
__attribute__((weak))
void clear_sniff_out_status()
{

}
u8 is_dac_power_off()
{
#if 1
    return 1;
#else
//开机播完提示音再初始化蓝牙
    if (a2dp_dec || esco_dec || get_call_status() != BT_CALL_HANGUP || !tone_get_status()) {
        return 1;
    }
    return 0;
#endif
}

#include "a2dp_dec.c" //这里将A2DP解码代码分解到另外文件

#define AUDIO_DECODE_TASK_WAKEUP_TIME	4	//ms
#if AUDIO_DECODE_TASK_WAKEUP_TIME
#include "timer.h"
static void audio_decoder_wakeup_timer(void *priv)
{
    //putchar('k');
    audio_decoder_resume_all(&decode_task);
}
int audio_decoder_task_add_probe(struct audio_decoder_task *task)
{
    local_irq_disable();
    if (task->wakeup_timer == 0) {
        task->wakeup_timer = usr_timer_add(NULL, audio_decoder_wakeup_timer, AUDIO_DECODE_TASK_WAKEUP_TIME, 1);
        log_i("audio_decoder_task_add_probe:%d\n", task->wakeup_timer);
    }
    local_irq_enable();
    return 0;
}
int audio_decoder_task_del_probe(struct audio_decoder_task *task)
{
    log_i("audio_decoder_task_del_probe\n");
    if (audio_decoder_task_wait_state(task) > 0) {
        /*解码任务列表还有任务*/
        return 0;
    }

    local_irq_disable();
    if (task->wakeup_timer) {
        log_i("audio_decoder_task_del_probe:%d\n", task->wakeup_timer);
        usr_timer_del(task->wakeup_timer);
        task->wakeup_timer = 0;
    }
    local_irq_enable();
    return 0;
}

int audio_decoder_wakeup_modify(int msecs)
{
    if (decode_task.wakeup_timer) {
        usr_timer_modify(decode_task.wakeup_timer, msecs);
    }

    return 0;
}

/*
 * DA输出源启动后可使用DAC irq做唤醒，省去hi_timer
 */
int audio_decoder_wakeup_select(u8 source)
{
    if (source == 0) {
        /*唤醒源为hi_timer*/
        local_irq_disable();
        if (!decode_task.wakeup_timer) {
            decode_task.wakeup_timer = usr_timer_add(NULL, audio_decoder_wakeup_timer, AUDIO_DECODE_TASK_WAKEUP_TIME, 1);
        }
        local_irq_enable();
    } else if (source == 1) {
        /*唤醒源为输出目标中断*/
        if (!sound_pcm_dev_is_running()) {
            return audio_decoder_wakeup_select(0);
        }

        int err = sound_pcm_trigger_resume(a2dp_low_latency ? 2 : AUDIO_DECODE_TASK_WAKEUP_TIME,
                                           NULL, audio_decoder_wakeup_timer);

        if (err) {
            return audio_decoder_wakeup_select(0);
        }

        local_irq_disable();
        if (decode_task.wakeup_timer) {
            usr_timer_del(decode_task.wakeup_timer);
            decode_task.wakeup_timer = 0;
        }
        local_irq_enable();
    }
    return 0;
}


#endif/*AUDIO_DECODE_TASK_WAKEUP_TIME*/

static u8 bt_dec_idle_query()
{
    if (a2dp_dec || esco_dec) {
        return 0;
    }

    return 1;
}
REGISTER_LP_TARGET(bt_dec_lp_target) = {
    .name = "bt_dec",
    .is_idle = bt_dec_idle_query,
};


___interrupt
static void audio_irq_handler()
{
    if (JL_AUDIO->DAC_CON & BIT(7)) {
        JL_AUDIO->DAC_CON |= BIT(6);
        if (JL_AUDIO->DAC_CON & BIT(5)) {
            audio_decoder_resume_all(&decode_task);
            audio_dac_irq_handler(&dac_hdl);
        }
    }

    if (JL_AUDIO->ADC_CON & BIT(7)) {
        JL_AUDIO->ADC_CON |= BIT(6);
        if ((JL_AUDIO->ADC_CON & BIT(5)) && (JL_AUDIO->ADC_CON & BIT(4))) {
            audio_adc_irq_handler(&adc_hdl);
        }
    }
}

#if AUDIO_OUTPUT_AUTOMUTE

void audio_mix_out_automute_mute(u8 mute)
{
    printf(">>>>>>>>>>>>>>>>>>>> %s\n", mute ? ("MUTE") : ("UNMUTE"));
}

/* #define AUDIO_E_DET_UNMUTE      (0x00) */
/* #define AUDIO_E_DET_MUTE        (0x01) */
void mix_out_automute_handler(u8 event, u8 ch)
{
    printf(">>>> ch:%d %s\n", ch, event ? ("MUTE") : ("UNMUTE"));
#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_AND_MUSIC_MUTEX)
    if (ch == sound_pcm_dev_channel_mapping(ch)) {
        printf("[DHA]>>>>>>>>>>>>>>>>>>>> %s\n", event ? ("MUTE") : ("UNMUTE"));
        if (a2dp_dec) {
            if (event) {
                audio_hearing_aid_resume();
            } else {
                audio_hearing_aid_suspend();
            }
        }
    }
#else
    sound_pcm_dev_channel_mute(BIT(ch), event);
    if (ch == sound_pcm_dev_channel_mapping(ch)) {
        audio_mix_out_automute_mute(event);
    }
#endif/*TCFG_AUDIO_HEARING_AID_ENABLE*/
}

void mix_out_automute_skip(u8 skip)
{
    u8 mute = !skip;
    if (mix_out_automute_hdl) {
        audio_energy_detect_skip(mix_out_automute_hdl, 0xFFFF, skip);
        mix_out_automute_handler(mute, sound_pcm_dev_channel_mapping(0));
    }
}

void mix_out_automute_open()
{
    if (mix_out_automute_hdl) {
        printf("mix_out_automute is already open !\n");
        return;
    }
    audio_energy_detect_param e_det_param = {0};
    e_det_param.mute_energy = 5;
    e_det_param.unmute_energy = 10;
    e_det_param.mute_time_ms = 1000;
    e_det_param.unmute_time_ms = 50;
    e_det_param.count_cycle_ms = 10;
    e_det_param.sample_rate = 44100;
    e_det_param.event_handler = mix_out_automute_handler;
    e_det_param.ch_total = sound_pcm_dev_channel_mapping(0);
    e_det_param.dcc = 1;
    mix_out_automute_hdl = audio_energy_detect_open(&e_det_param);
}

void mix_out_automute_close()
{
    if (mix_out_automute_hdl) {
        audio_energy_detect_close(mix_out_automute_hdl);
    }
}
#endif  //#if AUDIO_OUTPUT_AUTOMUTE


static void mixer_event_handler(struct audio_mixer *mixer, int event)
{
    switch (event) {
    case MIXER_EVENT_CH_OPEN:
        if ((audio_mixer_get_ch_num(mixer) == 1) || (audio_mixer_get_start_ch_num(mixer) == 1)) {
#if AUDIO_OUTPUT_AUTOMUTE
            audio_energy_detect_sample_rate_update(mix_out_automute_hdl, audio_mixer_get_sample_rate(mixer));
#endif  //#if AUDIO_OUTPUT_AUTOMUTE
#if TCFG_APP_FM_EMITTER_EN

#else
#if TCFG_AUDIO_ANC_ENABLE
            audio_anc_mix_process(1);
#endif/*TCFG_AUDIO_ANC_ENABLE*/
            sound_pcm_dev_start(NULL, audio_mixer_get_sample_rate(mixer), app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
#endif
#if AUDIO_OUT_EFFECT_ENABLE
            if (!audio_out_effect_dis) {
                audio_out_effect_open(mixer, audio_mixer_get_sample_rate(mixer));
            }
#endif
            mix_out_remain = 0;
#if MIX_OUT_DRC_EN
            mix_out_drc_close();
            mix_out_drc_open(audio_mixer_get_sample_rate(mixer));
#endif//MIX_OUT_DRC_EN

        }
        break;
    case MIXER_EVENT_CH_CLOSE:
        if (audio_mixer_get_ch_num(mixer) == 0) {
#if AUDIO_OUT_EFFECT_ENABLE
            audio_out_effect_close();
#endif
#if MIX_OUT_DRC_EN
            mix_out_drc_close();
#endif//MIX_OUT_DRC_EN

#if TCFG_APP_FM_EMITTER_EN

#else
            sound_pcm_dev_stop(NULL);
#if TCFG_AUDIO_ANC_ENABLE
            audio_anc_mix_process(0);
#endif/*TCFG_AUDIO_ANC_ENABLE*/
#endif
        }
        break;
    }
}

static int mix_probe_handler(struct audio_mixer *mixer)
{
    return 0;
}

static int mix_output_handler(struct audio_mixer *mixer, s16 *data, u16 len)
{
    int rlen = len;
    int wlen = 0;
    if (!mix_out_remain) {
#if MIX_OUT_DRC_EN
        mix_out_drc_run(data, len);
#endif//MIX_OUT_DRC_EN

#if ((defined TCFG_DIAFX_ENABLE) && TCFG_DIAFX_ENABLE)
        jl_platform_diafx_run(data, len >> 1);
#endif

    }
    /* audio_aec_refbuf(data, len); */

#if AUDIO_OUTPUT_AUTOMUTE
    audio_energy_detect_run(mix_out_automute_hdl, data, len);
#endif  //#if AUDIO_OUTPUT_AUTOMUTE
#if TCFG_APP_FM_EMITTER_EN
    fm_emitter_cbuf_write((u8 *)data, len);
#else
    wlen = sound_pcm_dev_write(NULL, data, len);
    /*printf("0x%x, %d, %d\n", (u32)data, len, wlen);*/
    /*
    if (wlen == len) {
        audio_decoder_resume_all(&decode_task);
    }
    */
#endif/*TCFG_APP_FM_EMITTER_EN*/
    if (wlen >= len) {
        mix_out_remain = 0;
    } else {
        mix_out_remain = 1;
    }

    return wlen;
}

#if AUDIO_OUT_EFFECT_ENABLE
static int mix_output_effect_handler(struct audio_mixer *mixer, s16 *data, u16 len)
{
    if (!audio_out_effect) {
        return mix_output_handler(mixer, data, len);
    }
    if (audio_out_effect && audio_out_effect->async) {
        return eq_drc_run(audio_out_effect, data, len);
    }


    if (!audio_out_effect->remain) {
        if (audio_out_effect && !audio_out_effect->async) {
            eq_drc_run(audio_out_effect, data, len);
        }
    }
    int wlen = mix_output_handler(mixer, data, len);
    if (wlen == len) {
        audio_out_effect->remain = 0;
    } else {
        audio_out_effect->remain = 1;
    }
    return wlen;
}
#endif//AUDIO_OUT_EFFECT_ENABLE

static const struct audio_mix_handler mix_handler  = {
    .mix_probe  = mix_probe_handler,

#if AUDIO_OUT_EFFECT_ENABLE
    .mix_output = mix_output_effect_handler,
#else
    .mix_output = mix_output_handler,
#endif//AUDIO_OUT_EFFECT_ENABLE
};

static const u8 msbc_mute_data[] = {
    0xAD, 0x00, 0x00, 0xC5, 0x00, 0x00, 0x00, 0x00, 0x77, 0x6D, 0xB6, 0xDD, 0xDB, 0x6D, 0xB7, 0x76,
    0xDB, 0x6D, 0xDD, 0xB6, 0xDB, 0x77, 0x6D, 0xB6, 0xDD, 0xDB, 0x6D, 0xB7, 0x76, 0xDB, 0x6D, 0xDD,
    0xB6, 0xDB, 0x77, 0x6D, 0xB6, 0xDD, 0xDB, 0x6D, 0xB7, 0x76, 0xDB, 0x6D, 0xDD, 0xB6, 0xDB, 0x77,
    0x6D, 0xB6, 0xDD, 0xDB, 0x6D, 0xB7, 0x76, 0xDB, 0x6C, 0x00,
};

static int headcheck(u8 *buf)
{
    int sync_word = buf[0] | ((buf[1] & 0x0f) << 8);
    return (sync_word == 0x801) && (buf[2] == 0xAD);
}

static int esco_dump_rts_info(struct rt_stream_info *pkt)
{
    u32 hash = 0xffffffff;
    int read_len = 0;
    pkt->baddr = lmp_private_get_esco_packet(&read_len, &hash);
    pkt->seqn = hash;
    /* printf("hash0=%d,%d ",hash,pkt->baddr ); */
    if (pkt->baddr && read_len) {
        pkt->remain_len = lmp_private_get_esco_remain_buffer_size();
        pkt->data_len = lmp_private_get_esco_data_len();
        return 0;
    }
    if (read_len == -EINVAL) {
        //puts("----esco close\n");
        return -EINVAL;
    }
    if (read_len < 0) {

        return -ENOMEM;
    }
    return ENOMEM;
}

static int esco_dec_get_frame(struct audio_decoder *decoder, u8 **frame)
{
    int len = 0;
    u32 hash = 0;
    struct esco_dec_hdl *dec = container_of(decoder, struct esco_dec_hdl, decoder);

__again:
    if (dec->frame) {
        int len = dec->frame_len - dec->offset;
        if (len > dec->esco_len - dec->data_len) {
            len = dec->esco_len - dec->data_len;
        }
        /*memcpy((u8 *)dec->data + dec->data_len, msbc_mute_data, sizeof(msbc_mute_data));*/
        memcpy((u8 *)dec->data + dec->data_len, dec->frame + dec->offset, len);
        dec->offset   += len;
        dec->data_len += len;
        if (dec->offset == dec->frame_len) {
            lmp_private_free_esco_packet(dec->frame);
            dec->frame = NULL;
        }
    }
    if (dec->data_len < dec->esco_len) {
        dec->frame = lmp_private_get_esco_packet(&len, &hash);
        /* printf("hash1=%d,%d ",hash,dec->frame ); */
        if (len <= 0) {
            printf("rlen=%d ", len);
            return -EIO;
        }
#if AUDIO_CODEC_SUPPORT_SYNC
        u32 timestamp;
        if (dec->ts_handle) {
            timestamp = esco_audio_timestamp_update(dec->ts_handle, hash);
            if (dec->syncts && (((hash - dec->hash) & 0x7ffffff) == dec->frame_time)) {
                audio_syncts_next_pts(dec->syncts, timestamp);
            }
            dec->hash = hash;
            if (!dec->ts_start) {
                dec->ts_start = 1;
                dec->mix_ch_event_params[2] = timestamp;
            }
        }
#endif
#if (defined(TCFG_PHONE_MESSAGE_ENABLE) && (TCFG_PHONE_MESSAGE_ENABLE))
        phone_message_enc_write(dec->frame + 2, len - 2);
#endif
        dec->offset = 0;
        dec->frame_len = len;
        goto __again;
    }
    *frame = (u8 *)dec->data;
    return dec->esco_len;
}

static void esco_dec_put_frame(struct audio_decoder *decoder, u8 *frame)
{
    struct esco_dec_hdl *dec = container_of(decoder, struct esco_dec_hdl, decoder);

    dec->data_len = 0;
    /*lmp_private_free_esco_packet((void *)frame);*/
}

static const struct audio_dec_input esco_input = {
    .coding_type = AUDIO_CODING_MSBC,
    .data_type   = AUDIO_INPUT_FRAME,
    .ops = {
        .frame = {
            .fget = esco_dec_get_frame,
            .fput = esco_dec_put_frame,
        }
    }
};

u32 lmp_private_clear_sco_packet(u8 clear_num);
static void esco_dec_clear_all_packet(struct esco_dec_hdl *dec)
{
    lmp_private_clear_sco_packet(0xff);
}

static int esco_dec_probe_handler(struct audio_decoder *decoder)
{
    struct esco_dec_hdl *dec = container_of(decoder, struct esco_dec_hdl, decoder);
    int err = 0;
    int find_packet = 0;
    struct rt_stream_info rts_info = {0};
    err = esco_dump_rts_info(&rts_info);
    if (err == -EINVAL) {
        return err;
    }
    if (err || !dec->enc_start) {
        audio_decoder_suspend(decoder, 0);
        return -EAGAIN;
    }

#if TCFG_USER_TWS_ENABLE
    if (tws_network_audio_was_started()) {
        /*清除从机声音加入标志*/
        dec->slience_frames = 20;
        tws_network_local_audio_start();
    }
#endif
    if (dec->preempt) {
        dec->preempt = 0;
        dec->slience_frames = 20;
    }
    return err;

}



#define MONO_TO_DUAL_POINTS 30


//////////////////////////////////////////////////////////////////////////////
static inline void audio_pcm_mono_to_dual(s16 *dual_pcm, s16 *mono_pcm, int points)
{
    //printf("<%d,%x>",points,mono_pcm);
    s16 *mono = mono_pcm;
    int i = 0;
    u8 j = 0;

    for (i = 0; i < points; i++, mono++) {
        *dual_pcm++ = *mono;
        *dual_pcm++ = *mono;
    }
}
/*level:0~15*/
static const u16 esco_dvol_tab[] = {
    0,	//0
    111,//1
    161,//2
    234,//3
    338,//4
    490,//5
    708,//6
    1024,//7
    1481,//8
    2142,//9
    3098,//10
    4479,//11
    6477,//12
    9366,//13
    14955,//14
    16384 //15
};


/*
*********************************************************************
*                  ESCO Decode mix filter
* Description: ESCO语音解码输出接入mix滤镜
* Arguments  : data - 音频数据，len - 音频byte长度
* Return	 : 输出到后级的数据长度
* Note(s)    : 音频后级PCM设备为单声道直接写入mixer，PCM设备为双声道，
*              这里通过1 to 2ch的处理将数据写入mixer，并返回消耗的原始
*              数据长度。
*********************************************************************
*/
static s16 two_ch_data[MONO_TO_DUAL_POINTS * 2];
static s16 dual_offset_points = 0;
static s16 dual_total_points = 0;
static int esco_decoder_mix_filter(struct esco_dec_hdl *dec, s16 *data, int len)
{
    int wlen = 0;
    s16 point_num = 0;
    u8 mono_to_dual = 0;
    s16 *mono_data = (s16 *)data;
    u16 remain_points = (len >> 1);

    /*
     *如果dac输出是双声道，因为esco解码出来时单声道
     *所以这里要根据dac通道确定是否做单变双
     */

    mono_to_dual = sound_pcm_dev_channel_mapping(1) == 2 ? 1 : 0;

    if (mono_to_dual) {
        do {
            if (dual_offset_points < dual_total_points) {
                int tmp_len = audio_mixer_ch_write(&dec->mix_ch, &two_ch_data[dual_offset_points], (dual_total_points - dual_offset_points) << 1);
                dual_offset_points += tmp_len >> 1;
                if (tmp_len < ((dual_total_points - dual_offset_points) << 1)) {
                    break;
                }
                if (dual_offset_points < dual_total_points) {
                    continue;
                }
            }
            point_num = MONO_TO_DUAL_POINTS;
            if (point_num >= remain_points) {
                point_num = remain_points;
            }
            audio_pcm_mono_to_dual(two_ch_data, mono_data, point_num);
            dual_offset_points = 0;
            dual_total_points = point_num << 1;
            int tmp_len = audio_mixer_ch_write(&dec->mix_ch, &two_ch_data[dual_offset_points], (dual_total_points - dual_offset_points) << 1);
            dual_offset_points += tmp_len >> 1;
            wlen += dual_total_points << 1;
            remain_points -= (dual_total_points >> 1);
            mono_data += point_num;
            if (tmp_len < (dual_total_points - dual_offset_points) << 1) {
                break;
            }
        } while (remain_points);
    } else {

        wlen = audio_mixer_ch_write(&dec->mix_ch, data, len);
    }

    wlen = mono_to_dual ? (wlen >> 1) : wlen;

    return wlen;
}


/*
*********************************************************************
*                  ESCO Decode DL_NS filter
* Description: ESCO语音解码输出下行降噪滤镜
* Arguments  : data - 音频数据，len - 音频byte长度
* Return	 : 输出到后级的数据长度
* Note(s)    : 下行降噪通话接通前为直通，接通后进行NS RUN处理。
*********************************************************************
*/
static int esco_decoder_dl_ns_filter(struct esco_dec_hdl *dec, s16 *data, int len)
{
#if TCFG_ESCO_DL_NS_ENABLE
    int wlen = 0;
    int ret_len = len;

    if (dec->dl_ns_remain) {
        wlen = esco_decoder_mix_filter(dec, dec->dl_ns_out + (dec->dl_ns_offset / 2), dec->dl_ns_remain);
        dec->dl_ns_offset += wlen;
        dec->dl_ns_remain -= wlen;
        if (dec->dl_ns_remain) {
            return 0;
        }
    }

    if (get_call_status() == BT_CALL_ACTIVE) {
        /*接通的时候再开始做降噪*/
        int ns_out = audio_ns_run(dec->dl_ns, data, dec->dl_ns_out, len);
        //输入消耗完毕，没有输出
        if (ns_out == 0) {
            return len;
        }
        len = ns_out;
    } else {
        memcpy(dec->dl_ns_out, data, len);
    }
    wlen = esco_decoder_mix_filter(dec, dec->dl_ns_out, len);
    dec->dl_ns_offset = wlen;
    dec->dl_ns_remain = len - wlen;
    return ret_len;
#else
    return 0;
#endif/*TCFG_ESCO_DL_NS_ENABLE*/
}

/*
*********************************************************************
*                  ESCO output after syncts
* Description: esco同步滤镜后的数据流滤镜
* Arguments  : data - 音频数据，len - 音频byte长度
* Return	 : 输出到后级的数据长度
* Note(s)    : 音频同步后使用。
*********************************************************************
*/
static int esco_output_after_syncts_filter(void *priv, s16 *data, int len)
{
    struct esco_dec_hdl *dec = (struct esco_dec_hdl *)priv;
    int wlen = 0;
    /*
     *如果dac输出是双声道，因为esco解码出来时单声道
     *所以这里要根据dac通道确定是否做单变双
     */

#if TCFG_ESCO_DL_NS_ENABLE
    wlen = esco_decoder_dl_ns_filter(dec, data, len);
#else
    wlen = esco_decoder_mix_filter(dec, data, len);
#endif

    return wlen;
}
/*
 *slience_frames结束前增加淡入处理，防止声音突变，听到一下的不适感
 * */
static void esco_fade_in(struct esco_dec_hdl *dec, s16 *data, int len)
{
    if (dec->fade_trigger) {
        int tmp = 0;
        s16 *p = data;
        u16 frames = (len >> 1) / dec->channels;
        for (int i = 0; i < frames; i++) {
            for (int j = 0; j < dec->channels; j++) {
                tmp = *p;
                *p++ = dec->fade_volume * tmp / DIGITAL_OdB_VOLUME;
            }
            dec->fade_volume += dec->fade_step;
            if (dec->fade_volume > DIGITAL_OdB_VOLUME) {
                dec->fade_volume = DIGITAL_OdB_VOLUME;
                break;
            }
        }
        if (dec->fade_volume == DIGITAL_OdB_VOLUME) {
            dec->fade_trigger = 0;
        }
    }
}

/*
*********************************************************************
*                  ESCO Decode Output
* Description: 蓝牙通话解码输出
* Arguments  : *priv	数据包错误表示(1 == error)
* Return	 : 输出到后级的数据长度
* Note(s)    : dec->remain不等于0表示解码输出的数据后级没有消耗完
*			   也就是说，只有(dec->remain == 0)的情况，才是新解码
*			   出来的数据，才是需要做后处理的数据，否则直接输出到
*			   后级即可。
*********************************************************************
*/
extern void adc_dec_output_handler(s16 *data, int len);
static int esco_dec_output_handler(struct audio_decoder *decoder, s16 *buf, int size, void *priv)
{
    int wlen = 0;
    int ret_len = size;
    int len = size;
    short *data = buf;
    struct esco_dec_hdl *dec = container_of(decoder, struct esco_dec_hdl, decoder);

    /*非上次残留数据,进行后处理*/
    if (!dec->remain) {
        adc_dec_output_handler(buf, size);
        spk_insert_data(buf, size >> 1);
#if (defined(TCFG_PHONE_MESSAGE_ENABLE) && (TCFG_PHONE_MESSAGE_ENABLE))
        phone_message_call_api_esco_out_data(data, len);
#endif/*TCFG_PHONE_MESSAGE_ENABLE*/
        if (priv) {
            audio_plc_run(data, len, *(u8 *)priv);
        }
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
        audio_digital_vol_run(CALL_DVOL, data, len);
        u16 dvol_val = esco_dvol_tab[app_var.aec_dac_gain];
        for (u16 i = 0; i < len / 2; i++) {
            s32 tmp_data = data[i];
            if (tmp_data < 0) {
                tmp_data = -tmp_data;
                tmp_data = (tmp_data * dvol_val) >> 14;
                tmp_data = -tmp_data;
            } else {
                tmp_data = (tmp_data * dvol_val) >> 14;
            }
            data[i] = tmp_data;
        }
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

#if TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE
        /*来电去电铃声不做处理*/
        if ((get_call_status() == BT_CALL_ACTIVE) && esco_dl_noisegate_buf) {
            audio_noise_gate_run(esco_dl_noisegate_buf, data, data, len / 2);
        }
#endif/*TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE*/


#if TCFG_AUDIO_ESCO_LIMITER_ENABLE
        if (dec->limiter) {
            esco_limiter_run(dec->limiter, data, len);
        }
#endif

#if TCFG_EQ_ENABLE&&TCFG_PHONE_EQ_ENABLE
        eq_drc_run(dec->eq_drc, data, len);
#endif//TCFG_PHONE_EQ_ENABLE

        if (dec->slience_frames) {
            if (dec->slience_frames == 1) {
                int fade_time = 6;
                int sample_rate = dec->decoder.fmt.sample_rate;
                dec->fade_step = DIGITAL_OdB_VOLUME / ((fade_time * sample_rate) / 1000);
                dec->fade_trigger = 1;
                dec->fade_volume = 0;
            } else {
                dec->fade_trigger = 0;
                dec->fade_volume = 0;
                memset(data, 0x0, len);
            }
            dec->slience_frames--;
        }
        esco_fade_in(dec, data, len);

    }

#if AUDIO_CODEC_SUPPORT_SYNC
    if (dec->syncts) {
        wlen = audio_syncts_frame_filter(dec->syncts, data, len);
        if (wlen < len) {
            audio_syncts_trigger_resume(dec->syncts, (void *)decoder, audio_filter_resume_decoder);
        }
        goto ret_handle;
    }
#endif
    wlen = esco_output_after_syncts_filter(dec, data, len);

ret_handle:

    dec->remain = wlen == len ? 0 : 1;

    return wlen;
}

static int esco_dec_post_handler(struct audio_decoder *decoder)
{

    return 0;
}

static const struct audio_dec_handler esco_dec_handler = {
    .dec_probe  = esco_dec_probe_handler,
    .dec_output = esco_dec_output_handler,
    .dec_post   = esco_dec_post_handler,
};

void esco_dec_release()
{
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    if (get_icsd_adt_mode()) {
        audio_icsd_adt_open(0);
    }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

    audio_decoder_task_del_wait(&decode_task, &esco_dec->wait);
    free(esco_dec);
    esco_dec = NULL;
}

void esco_dec_close();

static void esco_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_DEC_EVENT_END:
        puts("AUDIO_DEC_EVENT_END\n");
        esco_dec_close();
        break;
    }
}

u32 source_sr;
void set_source_sample_rate(u32 sample_rate)
{
    source_sr = sample_rate;
}

u16 get_source_sample_rate()
{
    if (bt_audio_is_running()) {
        return source_sr;
    }
    return 0;
}

int esco_dec_dac_gain_set(u8 gain)
{
    app_var.aec_dac_gain = gain;
    if (esco_dec) {
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW)
        app_audio_set_max_volume(APP_AUDIO_STATE_CALL, gain);
        app_audio_set_volume(APP_AUDIO_STATE_CALL, app_audio_get_volume(APP_AUDIO_STATE_CALL), 1);
#else
        sound_pcm_dev_set_analog_gain(gain);
#endif/*SYS_VOL_TYPE*/
    }
    return 0;
}


#define ESCO_SIRI_WAKEUP()      (app_var.siri_stu == 1 || app_var.siri_stu == 2)

static int esco_decoder_syncts_setup(struct esco_dec_hdl *dec)
{
    int err = 0;
#if AUDIO_CODEC_SUPPORT_SYNC
#define ESCO_DELAY_TIME     60
#define ESCO_RECOGNTION_TIME 220
    int delay_time = ESCO_DELAY_TIME;
    if (get_sniff_out_status()) {
        clear_sniff_out_status();
        if (ESCO_SIRI_WAKEUP()) {
            /*fix : Siri出sniff蓝牙数据到音频通路延迟过长，容易引入同步的问题*/
            delay_time = ESCO_RECOGNTION_TIME;
        }
    }
    struct audio_syncts_params params = {0};
    int sample_rate = dec->decoder.fmt.sample_rate;

    params.nch = dec->decoder.fmt.channel;

    params.pcm_device = sound_pcm_sync_device_select();//PCM_INSIDE_DAC;
    params.rout_sample_rate = sound_pcm_match_sample_rate(sample_rate);
    params.network = AUDIO_NETWORK_BT2_1;
    params.rin_sample_rate = sample_rate;
    params.priv = dec;
    params.factor = TIME_US_FACTOR;
    params.output = (int (*)(void *, void *, int))esco_output_after_syncts_filter;

    bt_audio_sync_nettime_select(0);//0 - 主机，1 - tws, 2 - BLE
    u8 frame_clkn = dec->esco_len >= 60 ? 12 : 6;
    dec->ts_handle = esco_audio_timestamp_create(frame_clkn, delay_time, TIME_US_FACTOR);
    dec->frame_time = frame_clkn;
    audio_syncts_open(&dec->syncts, &params);
    if (!err) {
        dec->mix_ch_event_params[0] = (u32)&dec->mix_ch;
        dec->mix_ch_event_params[1] = (u32)dec->syncts;
        audio_mixer_ch_set_event_handler(&dec->mix_ch, (void *)dec->mix_ch_event_params, audio_mix_ch_event_handler);
    }
    dec->ts_start = 0;
#endif
    return err;
}

static void esco_decoder_syncts_free(struct esco_dec_hdl *dec)
{
#if AUDIO_CODEC_SUPPORT_SYNC
    if (dec->ts_handle) {
        esco_audio_timestamp_close(dec->ts_handle);
        dec->ts_handle = NULL;
    }
    if (dec->syncts) {
        audio_syncts_close(dec->syncts);
        dec->syncts = NULL;
    }
#endif
}

// extern bool esco_opened;
extern bool transcription_opened;
int esco_ul_stream_open(u32 sample_rate, u16 esco_len, u32 codec_type)
{
    int err = 0;
    printf("esco_ul_open,sr = %d,len = %d,type = %x\n", sample_rate, esco_len, codec_type);
    if (esco_dec->start == 0) {
        printf("esco_ul_stream close ing,return\n");
        return 0;
    }
    if (esco_dec->ul_stream_open) {
        printf("esco_ul_stream_open now,return\n");
        return 0;
    }
    esco_dec->ul_stream_open = 1;
    err = audio_aec_init(sample_rate);
    if (err) {
        printf("audio_aec_init failed:%d", err);
        //goto __err3;
    }

    // esco_opened = true;
    if (!transcription_opened) {
        err = esco_enc_open(codec_type, esco_len);
    }
    if (err) {
        printf("audio_enc_open failed:%d", err);
        //goto __err3;
    }
    return err;
}

int esco_ul_stream_close(void)
{
    printf("esco_ul_stream_close\n");
    if (esco_dec->ul_stream_open == 0) {
        printf("esco_ul_stream_closed,return\n");
        return 0;
    }
    audio_aec_close();
    // esco_opened = false;
    if (!transcription_opened) {
        esco_enc_close();
    }
    esco_dec->ul_stream_open = 0;
    return 0;
}

int esco_ul_stream_switch(u8 en)
{
    if (esco_dec == NULL) {
        printf("esco_ul_stream_close now,return\n");
        return 0;
    }
    if (en) {
        return esco_ul_stream_open(esco_dec->sample_rate, esco_dec->esco_len, esco_dec->coding_type);
    } else {
        return esco_ul_stream_close();
    }
}

int esco_enc_reset(void)
{
    if (esco_dec == NULL) {
        printf("esco_ul_stream_close now,return\n");
        return 0;
    }
    esco_enc_close();
    esco_enc_open(esco_dec->coding_type, esco_dec->esco_len);
    return 0;
}

int esco_dec_start()
{
    int err;
    struct audio_fmt f;
    enum audio_channel channel;
    struct esco_dec_hdl *dec = esco_dec;
    u16 mix_buf_len_fix = 240;

    if (!esco_dec) {
        return -EINVAL;
    }

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    /*通话前关闭adt*/
    esco_dec->icsd_adt_state = audio_icsd_adt_is_running();
    if (esco_dec->icsd_adt_state) {
        audio_icsd_adt_close(0);
    }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

    err = audio_decoder_open(&dec->decoder, &esco_input, &decode_task);
    if (err) {
        goto __err1;
    }

    audio_decoder_set_handler(&dec->decoder, &esco_dec_handler);
    audio_decoder_set_event_handler(&dec->decoder, esco_dec_event_handler, 0);

    if (dec->coding_type == AUDIO_CODING_MSBC) {
        f.coding_type = AUDIO_CODING_MSBC;
        f.sample_rate = 16000;
        f.channel = 1;
    } else if (dec->coding_type == AUDIO_CODING_CVSD) {
        f.coding_type = AUDIO_CODING_CVSD;
        f.sample_rate = 8000;
        f.channel = 1;
        mix_buf_len_fix = 120;
    }
    dec->sample_rate = f.sample_rate;
    set_source_sample_rate(f.sample_rate);

    err = audio_decoder_set_fmt(&dec->decoder, &f);
    if (err) {
        goto __err2;
    }
    dec->channels = f.channel;

    /*
     *虽然mix有直通的处理，但是如果混合第二种声音进来的时候，就会按照mix_buff
     *的大小来混合输出，该buff太大，回导致dac没有连续的数据播放
     */
    /*audio_mixer_set_output_buf(&mixer, mix_buff, sizeof(mix_buff) / 8);*/
    audio_mixer_set_output_buf(&mixer, mix_buff, mix_buf_len_fix);
    audio_mixer_ch_open(&dec->mix_ch, &mixer);
    audio_mixer_ch_set_sample_rate(&dec->mix_ch, sound_pcm_match_sample_rate(f.sample_rate));
    audio_mixer_ch_set_resume_handler(&dec->mix_ch, (void *)&dec->decoder, (void (*)(void *))audio_decoder_resume);



#if TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE
    esco_dl_noisegate_buf = audio_noise_gate_open(300, 5, ESCO_NOISEGATE_THR, ESCO_NOISEGATE_GAIN, 16000, 1);
#endif/*TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE*/

#if TCFG_AUDIO_ESCO_LIMITER_ENABLE
    dec->limiter = esco_limiter_open(f.sample_rate, f.channel);
#endif/*TCFG_AUDIO_ESCO_LIMITER_ENABLE*/

#if TCFG_EQ_ENABLE&&TCFG_PHONE_EQ_ENABLE
    u8 drc_en = 0;
#if TCFG_DRC_ENABLE&&ESCO_DRC_EN
    drc_en = 1;
#endif//ESCO_DRC_EN

    dec->eq_drc = esco_eq_drc_setup(&dec->mix_ch, (eq_output_cb)audio_mixer_ch_write, f.sample_rate, f.channel, 0, drc_en);
#endif//TCFG_PHONE_EQ_ENABLE

    app_audio_state_switch(APP_AUDIO_STATE_CALL, BT_CALL_VOL_LEAVE_MAX);

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_open(CALL_DVOL, app_audio_get_volume(APP_AUDIO_STATE_CALL), CALL_DVOL_MAX, CALL_DVOL_FS, -1);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/
    printf("max_vol:%d,call_vol:%d", app_var.aec_dac_gain, app_audio_get_volume(APP_AUDIO_STATE_CALL));
    app_audio_set_volume(APP_AUDIO_STATE_CALL, app_var.call_volume, 1);

#if AUDIO_CODEC_SUPPORT_SYNC
    esco_decoder_syncts_setup(dec);
#endif/*AUDIO_CODEC_SUPPORT_SYNC*/

    /*plc调用需要考虑到代码overlay或者压缩操作*/
    audio_plc_open(f.sample_rate);

    sound_pcm_dev_set_delay_time(30, 50);
    lmp_private_esco_suspend_resume(2);
    err = audio_decoder_start(&dec->decoder);
    if (err) {
        goto __err3;
    }
    sound_pcm_dev_try_power_on();
    audio_out_effect_dis = 1;//通话模式关闭高低音

    dec->start = 1;
    dec->remain = 0;

#if TCFG_ESCO_DL_NS_ENABLE
    dec->dl_ns = audio_ns_open(f.sample_rate, DL_NS_MODE, DL_NS_NOISELEVEL, DL_NS_AGGRESSFACTOR, DL_NS_MINSUPPRESS);
    dec->dl_ns_remain = 0;
#endif/*TCFG_ESCO_DL_NS_ENABLE*/

    err = esco_ul_stream_open(f.sample_rate, dec->esco_len, dec->coding_type);

#if (TCFG_AUDIO_OUTPUT_IIS)
    audio_aec_ref_src_open(TCFG_IIS_SR, f.sample_rate);
#endif

    clk_set_sys_lock(96 * (1000000L), 0);
    audio_codec_clock_set(AUDIO_ESCO_MODE, dec->coding_type, dec->wait.preemption);
    /* #if TCFG_DRC_ENABLE&&ESCO_DRC_EN */
    /* #endif//ESCO_DRC_EN */

    dec->enc_start = 1; //该函数所在任务优先级低可能未open编码就开始解码，加入enc开始的标志防止解码过快输出
    printf("esco_dec_start ok\n");


    return 0;

__err3:
    audio_mixer_ch_close(&dec->mix_ch);
__err2:
    audio_decoder_close(&dec->decoder);
__err1:
    esco_dec_release();
    return err;
}

static int __esco_dec_res_close(void)
{
    if (!esco_dec->start) {
        return 0;
    }

    esco_dec->start = 0;
    esco_dec->enc_start = 0;
    esco_dec->preempt = 1;

    esco_ul_stream_close();
    /* audio_aec_close(); */
    /* esco_enc_close(); */
#if TCFG_AUDIO_CVP_DUT_ENABLE
    cvp_dut_mode_set(CVP_DUT_MODE_ALGORITHM);	//通话结束 CVP_DUT恢复成算法模式
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/
    app_audio_state_exit(APP_AUDIO_STATE_CALL);
    audio_decoder_close(&esco_dec->decoder);
    audio_mixer_ch_close(&esco_dec->mix_ch);
    sound_pcm_dev_set_delay_time(20, AUDIO_DAC_DELAY_TIME);
#if AUDIO_CODEC_SUPPORT_SYNC
    esco_decoder_syncts_free(esco_dec);
    esco_dec->sync_step = 0;
#endif
#if TCFG_EQ_ENABLE&&TCFG_PHONE_EQ_ENABLE
    if (esco_dec->eq_drc) {
        esco_eq_drc_free(esco_dec->eq_drc);
        esco_dec->eq_drc = NULL;
    }
#endif//TCFG_PHONE_EQ_ENABLE

    audio_out_effect_dis = 0;

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_close(CALL_DVOL);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

#if TCFG_ESCO_DL_NS_ENABLE
    audio_ns_close(esco_dec->dl_ns);
#endif/*TCFG_ESCO_DL_NS_ENABLE*/

    audio_plc_close();
#if TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE
    audio_noise_gate_close(esco_dl_noisegate_buf);
#endif/*TCFG_AUDIO_ESCO_DL_NOISEGATE_ENABLE*/

#if TCFG_AUDIO_ESCO_LIMITER_ENABLE
    if (esco_dec->limiter) {
        esco_limiter_close(esco_dec->limiter);
        esco_dec->limiter = NULL;
    }
#endif
    audio_codec_clock_del(AUDIO_ESCO_MODE);
    dual_offset_points = 0;
    dual_total_points = 0;
    return 0;
}

static int esco_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;

    printf("esco_wait_res_handler:%d", event);
    if (event == AUDIO_RES_GET) {
        err = esco_dec_start();
    } else if (event == AUDIO_RES_PUT) {
        err = __esco_dec_res_close();
        lmp_private_esco_suspend_resume(1);
    }

    return err;
}

static void esco_smart_voice_detect_handler(void)
{
#if TCFG_SMART_VOICE_ENABLE
#if TCFG_CALL_KWS_SWITCH_ENABLE
    if (ESCO_SIRI_WAKEUP() || (get_call_status() != BT_CALL_INCOMING)) {
        audio_smart_voice_detect_close();
    }
#else
    audio_smart_voice_detect_close();
#endif
#endif
}

/*
*********************************************************************
*                  SCO/ESCO Decode Open
* Description: 打开蓝牙通话解码
* Arguments  : param 蓝牙协议栈传递上来的编解码信息
* Return	 : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************************
*/
int esco_dec_open(void *param, u8 mute)
{
    int err;
    struct esco_dec_hdl *dec;
    u32 esco_param = *(u32 *)param;
    int esco_len = esco_param >> 16;
    int codec_type = esco_param & 0x000000ff;

    printf("esco_dec_open, type=%d,len=%d\n", codec_type, esco_len);
#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_AND_CALL_MUTEX)
    audio_hearing_aid_suspend();
#endif/*TCFG_AUDIO_HEARING_AID_ENABLE*/

    esco_smart_voice_detect_handler();
    dec = zalloc(sizeof(*dec));
    if (!dec) {
        return -ENOMEM;
    }
    esco_dec = dec;
    dec->esco_len = esco_len;
    if (codec_type == 3) {
        dec->coding_type = AUDIO_CODING_MSBC;
    } else if (codec_type == 2) {
        dec->coding_type = AUDIO_CODING_CVSD;
    } else {
        printf("Unknow ESCO codec type:%d\n", codec_type);
    }

    dec->tws_mute_en = mute;

    dec->wait.priority = 2;
    dec->wait.preemption = 1;
    dec->wait.handler = esco_wait_res_handler;
    err = audio_decoder_task_add_wait(&decode_task, &dec->wait);
    if (esco_dec && esco_dec->start == 0) {
        dec->preempt = 1;
        lmp_private_esco_suspend_resume(1);
    }

#if AUDIO_OUTPUT_AUTOMUTE
    mix_out_automute_skip(1);
#endif

    return err;
}

/*
*********************************************************************
*                  SCO/ESCO Decode Close
* Description: 关闭蓝牙通话解码
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void esco_dec_close(void)
{
    if (!esco_dec) {
        return;
    }


    __esco_dec_res_close();
    esco_dec_release();

#if (TCFG_AUDIO_OUTPUT_IIS)
    audio_aec_ref_src_close();
#endif

#if AUDIO_OUTPUT_AUTOMUTE
    mix_out_automute_skip(0);
#endif

#if (defined(TCFG_PHONE_MESSAGE_ENABLE) && (TCFG_PHONE_MESSAGE_ENABLE))
    phone_message_call_api_stop();
#endif/*TCFG_PHONE_MESSAGE_ENABLE*/

    audio_mixer_set_output_buf(&mixer, mix_buff, sizeof(mix_buff));
#if TCFG_SMART_VOICE_ENABLE
    audio_smart_voice_detect_open(JL_KWS_COMMAND_KEYWORD);
#endif

#if (TCFG_AUDIO_HEARING_AID_ENABLE && TCFG_AUDIO_DHA_AND_CALL_MUTEX)
    audio_hearing_aid_resume();
#endif/*TCFG_AUDIO_HEARING_AID_ENABLE*/
    printf("esco_dec_close succ\n");
}


//////////////////////////////////////////////////////////////////////////////
u8 bt_audio_is_running(void)
{
    return (a2dp_dec || esco_dec);
}
u8 bt_media_is_running(void)
{
    return a2dp_dec != NULL;
}
u8 bt_phone_dec_is_running()
{
    return esco_dec != NULL;
}
__attribute__((weak))
void mic_trim_run(void)
{
    printf("==========weak fun for tmp\n");
}
extern u32 read_capless_DTB(void);
static u8 audio_dec_inited = 0;

/*音频配置实时跟踪，可以用来查看ADC/DAC增益，或者其他寄存器配置*/
static void audio_config_trace(void *priv)
{
    printf(">>Audio_Config_Trace:\n");
    audio_gain_dump();
    //audio_adda_dump();
}

//////////////////////////////////////////////////////////////////////////////
int audio_dec_init()
{
    int err;
    printf("audio_dec_init\n");

    tone_play_init();

#if TCFG_KEY_TONE_EN
    // 按键音初始化
    audio_key_tone_init();
#endif

    err = audio_decoder_task_create(&decode_task, "audio_dec");
#if TCFG_AUDIO_ANC_ENABLE
    /* audio_dac_anc_set(&dac_hdl, 1); */
    /* dac_data.max_ana_vol = anc_dac_gain_get(ANC_DAC_CH_L);		//获取ANC设置的模拟增益 */
#endif/*TCFG_AUDIO_ANC_ENABLE*/

    request_irq(IRQ_AUDIO_IDX, 2, audio_irq_handler, 0);

    /*硬件SRC模块滤波器buffer设置，可根据最大使用数量设置整体buffer*/
    /* audio_src_base_filt_init(audio_src_hw_filt, sizeof(audio_src_hw_filt)); */

    sound_pcm_driver_init();
    sound_pcm_dev_set_delay_time(20, AUDIO_DAC_DELAY_TIME);

    audio_mixer_open(&mixer);
    audio_mixer_set_handler(&mixer, &mix_handler);
    audio_mixer_set_event_handler(&mixer, mixer_event_handler);
#if defined(TCFG_AUDIO_DAC_24BIT_MODE) && TCFG_AUDIO_DAC_24BIT_MODE
    audio_mixer_set_mode(&mixer, 4, BIT24_MODE);
#endif
    audio_mixer_set_output_buf(&mixer, mix_buff, sizeof(mix_buff));

#if AUDIO_OUTPUT_AUTOMUTE
    mix_out_automute_open();
#endif  //#if AUDIO_OUTPUT_AUTOMUTE

#if TCFG_AUDIO_CONFIG_TRACE
    sys_timer_add(NULL, audio_config_trace, 3000);
#endif/*TCFG_AUDIO_CONFIG_TRACE*/

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
    spk_eq_read_from_vm();
#endif

#if ((defined TCFG_DIAFX_ENABLE) && TCFG_DIAFX_ENABLE)
    diafx_init_mode(0);
    diafx_enabled(1);
#endif
    audio_dec_inited = 1;

#if TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    /*需要再dac trim后调用*/
    /*同时开ANC和免摘功能时，配置ADC固定打开的adc数字通道数*/
    extern struct adc_platform_data adc_data;
    adc_data.anc_adt_en = 1;
    u8 esco_mic_num = audio_adc_file_get_esco_mic_num();
    u8 adt_mic_num = get_icsd_adt_mic_num();
    adc_data.anc_adt_mic_ch_num = esco_mic_num > adt_mic_num ? esco_mic_num : adt_mic_num;
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

    return err;
}

static u8 audio_dec_init_complete()
{
    if (!audio_dec_inited) {
        return 0;
    }

    return 1;
}
extern void audio_adc_mic_demo_open(u8 mic_idx, u8 gain, u16 sr, u8 mic_2_dac);
extern void dac_analog_power_control(u8 en);
void audio_fast_mode_test()
{
    sound_pcm_dev_start(NULL, 44100, app_audio_get_volume(APP_AUDIO_CURRENT_STATE));
    audio_adc_mic_demo_open(AUDIO_ADC_MIC_0, 10, 16000, 1);

}
REGISTER_LP_TARGET(audio_dec_init_lp_target) = {
    .name = "audio_dec_init",
    .is_idle = audio_dec_init_complete,
};

#if AUDIO_OUT_EFFECT_ENABLE
int audio_out_effect_stream_clear()
{
    if (!audio_out_effect) {
        return 0;
    }

    if (audio_out_effect->eq && audio_out_effect->async) {
        audio_eq_async_data_clear(audio_out_effect->eq);
    }
    return 0;
}

void audio_out_effect_close(void)
{
    if (!audio_out_effect) {
        return ;
    }
    audio_out_eq_drc_free(audio_out_effect);
    audio_out_effect = NULL;
}


int audio_out_effect_open(void *priv, u16 sample_rate)
{
    audio_out_effect_close();

    u8 ch_num;
#if TCFG_APP_FM_EMITTER_EN
    ch_num = 2;
#else
    ch_num = sound_pcm_dev_channel_mapping(0);
#endif//TCFG_APP_FM_EMITTER_EN

    u8 drc_en = 0;//
    u8 async = 0;
    if (drc_en) {
        async = 1;
    }

    audio_out_effect = audio_out_eq_drc_setup(priv, (eq_output_cb)mix_output_handler, sample_rate, ch_num, async, drc_en);
    return 0;
}

int audio_out_eq_spec_set_gain(u8 idx, int gain)
{
    if (!audio_out_effect) {
        return -1;
    }
    audio_out_eq_set_gain(audio_out_effect, idx, gain);
    return 0;
}
#endif//AUDIO_OUT_EFFECT_ENABLE

/*----------------------------------------------------------------------------*/
/**@brief    获取输出默认采样率
   @param
   @return   0: 采样率可变
   @return   非0: 固定采样率
   @note
*/
/*----------------------------------------------------------------------------*/
u32 audio_output_nor_rate(void)
{
#if TCFG_AUDIO_OUTPUT_IIS
    return TCFG_IIS_SR;
#endif /*TCFG_AUDIO_OUTPUT_IIS*/

#if AUDIO_OUTPUT_INCLUDE_DAC

#if (TCFG_MIC_EFFECT_ENABLE)
    return TCFG_REVERB_SAMPLERATE_DEFUAL;
#endif
    /* return  app_audio_output_samplerate_select(input_rate, 1); */
#elif (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_BT)

#elif (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_FM)
    return 41667;
#else
    return 44100;
#endif

    /* #if TCFG_VIR_UDISK_ENABLE */
    /*     return 44100; */
    /* #endif */

    return 0;
}

int audio_dac_sample_rate_select(struct audio_dac_hdl *dac, u32 sample_rate, u8 high)
{
    u32 sample_rate_tbl[] = {8000,  11025, 12000, 16000, 22050, 24000,
                             32000, 44100, 48000, 64000, 88200, 96000
                            };
    int rate_num = ARRAY_SIZE(sample_rate_tbl);
    int i = 0;

    for (i = 0; i < rate_num; i++) {
        if (sample_rate == sample_rate_tbl[i]) {
            return sample_rate;
        }

        if (sample_rate < sample_rate_tbl[i]) {
            if (high) {
                return sample_rate_tbl[i];
            } else {
                return sample_rate_tbl[i > 0 ? (i - 1) : 0];
            }
        }
    }

    return sample_rate_tbl[rate_num - 1];
}

/*******************************************************
* Function name	: app_audio_output_samplerate_select
* Description	: 将输入采样率与输出采样率进行匹配对比
* Parameter		:
*   @sample_rate    输入采样率
*   @high:          0 - 低一级采样率，1 - 高一级采样率
* Return        : 匹配后的采样率
********************* -HB ******************************/
int app_audio_output_samplerate_select(u32 sample_rate, u8 high)
{
    return audio_dac_sample_rate_select(&dac_hdl, sample_rate, high);
}

/*----------------------------------------------------------------------------*/
/**@brief    获取输出采样率
   @param    input_rate: 输入采样率
   @return   输出采样率
   @note
*/
/*----------------------------------------------------------------------------*/
u32 audio_output_rate(int input_rate)
{
    u32 out_rate = audio_output_nor_rate();
    if (out_rate) {
        return out_rate;
    }

#if (TCFG_REVERB_ENABLE || TCFG_MIC_EFFECT_ENABLE)
    if (input_rate > 48000) {
        return 48000;
    }
#endif
    return  app_audio_output_samplerate_select(input_rate, 1);
}

#if TCFG_USER_TWS_ENABLE
#if AUDIO_SURROUND_CONFIG
#define TWS_FUNC_ID_A2DP_EFF \
	((int)(('A' + '2' + 'D' + 'P') << (2 * 8)) | \
	 (int)(('E' + 'F' + 'F') << (1 * 8)) | \
	 (int)(('S' + 'Y' + 'N' + 'C') << (0 * 8)))
/*
 *发环绕左右耳效果同步
 * */
void audio_surround_voice_ctrl()
{
    int state = tws_api_get_tws_state();
    if (state & TWS_STA_SIBLING_CONNECTED) {
        if (a2dp_dec && a2dp_dec->sur && a2dp_dec->sur->surround) {
            if (!a2dp_dec->sur->surround_eff) {
                a2dp_dec->sur->surround_eff =  1;
            } else {
                a2dp_dec->sur->surround_eff =  0;
            }
            int a2dp_eff = a2dp_dec->sur->surround_eff;
            tws_api_send_data_to_sibling((u8 *)&a2dp_eff, sizeof(int), TWS_FUNC_ID_A2DP_EFF);
        }
    }
}

/*
 *左右耳环绕效果同步回调
 * */
static void tws_a2dp_eff_align(void *data, u16 len, bool rx)
{
    if (a2dp_dec && a2dp_dec->sur && a2dp_dec->sur->surround) {
        int a2dp_eff;
        memcpy(&a2dp_eff, data, sizeof(int));
        a2dp_dec->sur->surround_eff = a2dp_eff;
        a2dp_surround_set(a2dp_eff);
        audio_surround_voice(a2dp_dec->sur, a2dp_dec->sur->surround_eff);
    }
}

REGISTER_TWS_FUNC_STUB(a2dp_align_eff) = {
    .func_id = TWS_FUNC_ID_A2DP_EFF,
    .func    = tws_a2dp_eff_align,
};
#endif//AUDIO_SURROUND_CONFIG
#endif /* TCFG_USER_TWS_ENABLE */


