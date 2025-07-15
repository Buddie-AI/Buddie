#include "app_config.h"
#include "icsd_anc_user.h"
#include "audio_codec_clock.h"
#include "asm/clock.h"
#include "audio_enc.h"
#include "asm/audio_adc.h"
#include "tone_player.h"
#include "avctp_user.h"
#include "asm/audio_src_base.h"
#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif
#include "app_main.h"

#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"

struct audio_mic_hdl {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    s16 *adc_buf;    //align 2Bytes
    u16 dump_packet;
};
static struct audio_mic_hdl *audio_mic = NULL;
extern struct audio_dac_hdl dac_hdl;
extern struct audio_adc_hdl adc_hdl;
extern s16 esco_adc_buf[];    //align 2Bytes
extern void tws_tx_unsniff_req(void);
extern void tws_tx_sniff_req(void);
extern void set_tws_t_sniff(u16 slot);
extern void bt_sniff_set_enable(u8 en);

int audio_mic_en(u8 en, audio_mic_param_t *mic_param,
                 void (*data_handler)(void *priv, s16 *data, int len))
{
    printf("audio_mic_en : %d", en);
    if (en) {
        if (audio_mic) {
            printf("audio_mic re-malloc error\n");
            return -1;
        }
        audio_mic = zalloc(sizeof(struct audio_mic_hdl));
        if (audio_mic == NULL) {
            printf("audio mic zalloc failed\n");
            return -1;
        }

        u16 mic_ch = mic_param->mic_ch_sel;
        u16 sr = mic_param->sample_rate;
        u8 gain0 = mic_param->mic0_gain;
        u8 gain1 = mic_param->mic1_gain;
        u8 gain2 = mic_param->mic2_gain;
        u8 gain3 = mic_param->mic3_gain;

        u8 mic_num = 0;
        /*打开mic电压*/
        audio_mic_pwr_ctl(MIC_PWR_ON);
        /*打开mic0*/
        if (mic_ch & BIT(0)) {
            printf("adc_mic0 open, sr:%d, gain:%d\n", sr, gain0);
            audio_adc_mic_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic_set_gain(&audio_mic->mic_ch, gain0);
            mic_num ++;
        }
        /*打开mic1*/
        if (mic_ch & BIT(1)) {
            printf("adc_mic1 open, sr:%d, gain:%d\n", sr, gain1);
            audio_adc_mic1_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic1_set_gain(&audio_mic->mic_ch, gain1);
            mic_num ++;
        }
        /*打开mic2*/
        if (mic_ch & BIT(2)) {
            printf("adc_mic2 open, sr:%d, gain:%d\n", sr, gain2);
            audio_adc_mic2_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic2_set_gain(&audio_mic->mic_ch, gain2);
            mic_num ++;
        }
        /*打开mic3*/
        if (mic_ch & BIT(3)) {
            printf("adc_mic3 open, sr:%d, gain:%d\n", sr, gain3);
            audio_adc_mic3_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic3_set_gain(&audio_mic->mic_ch, gain3);
            mic_num ++;
        }

        int adc_buf_size = mic_param->adc_irq_points * 2 * mic_param->adc_buf_num * mic_num;
        printf("adc irq points %d, adc_buf_size : %d", mic_param->adc_irq_points, adc_buf_size);
        audio_mic->adc_buf = esco_adc_buf;
        /* audio_mic->adc_buf = zalloc(adc_buf_size); */
        if (audio_mic->adc_buf == NULL) {
            printf("audio->adc_buf mic zalloc failed\n");
            audio_mic_pwr_ctl(MIC_PWR_OFF);
            free(audio_mic);
            audio_mic = NULL;
            return -1;
        }

        audio_adc_mic_set_sample_rate(&audio_mic->mic_ch, sr);
        audio_adc_mic_set_buffs(&audio_mic->mic_ch, audio_mic->adc_buf,
                                mic_param->adc_irq_points * 2, mic_param->adc_buf_num);
        audio_mic->adc_output.handler = data_handler;
        audio_adc_add_output_handler(&adc_hdl, &audio_mic->adc_output);
        audio_adc_mic_start(&audio_mic->mic_ch);
    } else {
        if (audio_mic) {
            /*设置fb mic为复用mic*/
#if TCFG_AUDIO_ANC_ENABLE
            audio_anc_mic_mana_fb_mult_set(1);
#endif
            audio_adc_mic_close(&audio_mic->mic_ch);
            audio_adc_del_output_handler(&adc_hdl, &audio_mic->adc_output);
            if (audio_mic->adc_buf) {
                /* free(audio_mic->adc_buf);  */
            }
            /*清除fb mic为复用mic的标志*/
#if TCFG_AUDIO_ANC_ENABLE
            audio_anc_mic_mana_fb_mult_set(0);
            if (anc_status_get() == 0)
#endif
            {
                audio_mic_pwr_ctl(MIC_PWR_OFF);
            }
            free(audio_mic);
            audio_mic = NULL;
        }
    }
    return 0;
}

static char *tone_index_to_name(u8 index)
{
    char *file_name = NULL;
    switch (index) {
    case ICSD_ADT_TONE_NUM0 :
        file_name = TONE_NUM_0;
        break;
    case ICSD_ADT_TONE_NUM1 :
        file_name = TONE_NUM_1;
        break;
    case ICSD_ADT_TONE_NUM2 :
        file_name = TONE_NUM_2;
        break;
    case ICSD_ADT_TONE_NUM3 :
        file_name = TONE_NUM_3;
        break;
    case ICSD_ADT_TONE_NUM4 :
        file_name = TONE_NUM_4;
        break;
    case ICSD_ADT_TONE_NUM5 :
        file_name = TONE_NUM_5;
        break;
    case ICSD_ADT_TONE_NUM6 :
        file_name = TONE_NUM_6;
        break;
    case ICSD_ADT_TONE_NUM7 :
        file_name = TONE_NUM_7;
        break;
    case ICSD_ADT_TONE_NUM8 :
        file_name = TONE_NUM_8;
        break;
    case ICSD_ADT_TONE_NUM9 :
        file_name = TONE_NUM_9;
        break;
    case ICSD_ADT_TONE_NORMAL :
        /* file_name = (char *)get_tone_files()->normal; */
        break;
    case ICSD_ADT_TONE_SPKCHAT_ON :
        file_name = TONE_SPKCHAT_ON;
        break;
    case ICSD_ADT_TONE_SPKCHAT_OFF :
        file_name = TONE_SPKCHAT_OFF;
        break;
    case ICSD_ADT_TONE_WCLICK_ON :
        file_name = TONE_WCLICK_ON;
        break;
    case ICSD_ADT_TONE_WCLICK_OFF :
        file_name = TONE_WCLICK_OFF;
        break;
    case ICSD_ADT_TONE_WINDDET_ON :
        file_name = TONE_WINDDET_ON;
        break;
    case ICSD_ADT_TONE_WINDDET_OFF :
        file_name = TONE_WINDDET_OFF;
        break;
    }
    return file_name;
}

int icsd_adt_tone_play_callback(u8 index, void (*evt_handler)(void *priv), void *priv)
{
    int ret = 0;
    char *file_name = tone_index_to_name(index);
    if (index == ICSD_ADT_TONE_NORMAL) {
        ret = tone_play_index_with_callback(index, 0, evt_handler, priv);
    } else {
        ret = tone_play_with_callback(file_name, 0, evt_handler, priv);
    }
    return ret;
}

int icsd_adt_tone_play(u8 index)
{
    int ret = 0;
    char *file_name = tone_index_to_name(index);
    if (index == ICSD_ADT_TONE_NORMAL) {
        ret = tone_play_index(IDEX_TONE_NORMAL, 0);
    } else {
        ret = tone_play(file_name, 0);
    }
    return ret;
}


static void icsd_adt_clock_add()
{
    /* clock_refurbish(); */
    audio_codec_clock_set(SPEAK_TO_CHAT_MODE, AUDIO_CODING_PCM, 0);
}

static void icsd_adt_clock_del()
{
    audio_codec_clock_del(SPEAK_TO_CHAT_MODE);
}

void icsd_set_clk()
{
    clk_set_sys_lock(128 * (1000000L), 0);
    /* clock_alloc("icsd_adt", 128 * 1000000L); */
}

static void icsd_anc_fade_set(int gain)
{
#if TCFG_AUDIO_ANC_ENABLE
    audio_anc_fade_ctr_set(ANC_FADE_MODE_WIND_NOISE, AUDIO_ANC_FDAE_CH_ALL, gain);
#endif
}

u8 esco_player_runing()
{
    return bt_phone_dec_is_running();
}

u8 bt_get_call_status()
{
    return get_call_status();
}

u8 bt_a2dp_get_status()
{
    return a2dp_get_status();
}

void bt_cmd_prepare(u8 cmd, u16 param_len, u8 *param)
{
    user_send_cmd_prepare(cmd, param_len, param);
}

void *icsd_adt_src_init(int in_rate, int out_rate, int (*handler)(void *, void *, int))
{
    void *src_hdl = audio_resample_hw_open(1, in_rate, out_rate, 1);
    audio_resample_hw_set_output_handler(src_hdl, 0, handler);
    return src_hdl;

    /* void *src_hdl = zalloc(sizeof(struct audio_src_handle)); */
    /* audio_hw_src_open(src_hdl, 1, 1); */
    /* audio_hw_src_set_rate(src_hdl, in_rate, out_rate); */
    /* audio_src_set_output_handler(src_hdl, 0, handler); */
    /* return src_hdl; */
}

void icsd_adt_src_write(void *data, int len, void *resample)
{
    audio_resample_hw_write(resample, data, len);
    /* audio_src_resample_write(resample, data, len); */
}

void icsd_adt_src_push(void *resample)
{
    audio_resample_hw_push_data_out(resample);
    /* audio_src_push_data_out((struct audio_src_handle *)resample); */
}

void icsd_adt_src_close(void *resample)
{
    audio_resample_hw_close(resample);
    /* audio_hw_src_close(resample); */
    /* free(resample); */
    resample = NULL;
}

void icsd_adt_tx_unsniff_req()
{
#if TCFG_USER_TWS_ENABLE
    tws_tx_unsniff_req();
#endif
}

void icsd_bt_sniff_set_enable(u8 en)
{
    bt_sniff_set_enable(en);
}


void icsd_set_tws_t_sniff(u16 slot)
{
#if TCFG_USER_TWS_ENABLE
    set_tws_t_sniff(slot);
#endif
}

u8 icsd_get_talk_mic_ch(void)
{
    return app_var.talk_mic_ch;
}

u8 icsd_get_ref_mic_ch(void)
{
    return BIT(TCFG_AUDIO_ANCL_FF_MIC);
}

u8 icsd_get_fb_mic_ch(void)
{
    return BIT(TCFG_AUDIO_ANCL_FB_MIC);
}

u8 icsd_get_esco_mic_en_map(void)
{
    return TCFG_AUDIO_ADC_MIC_CHA;
}

u8 audio_adc_file_get_esco_mic_num(void)
{
    u8 mic_num = 0;
    for (int i = 0; i < 4; i++) {
        if (TCFG_AUDIO_ADC_MIC_CHA & BIT(i)) {
            mic_num++;
        }
    }
    return mic_num;
}

#endif /*TCFG_AUDIO_ANC_ENABLE*/
