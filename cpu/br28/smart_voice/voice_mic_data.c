/*****************************************************************
>file name : voice_mic_data.c
>author : lichao
>create time : Mon 01 Nov 2021 11:33:32 AM CST
*****************************************************************/
#include "app_config.h"
#include "voice_mic_data.h"
#include "smart_voice.h"
#include "vad_mic.h"
#include "app_main.h"
#include "audio_anc.h"

#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif/*TCFG_AUDIO_ANC_ENABLE*/

extern struct audio_adc_hdl adc_hdl;

#define CONFIG_VOICE_MIC_DATA_DUMP          0

#define CONFIG_VOICE_MIC_DATA_EXPORT          0
#if CONFIG_VOICE_MIC_DATA_EXPORT
#include "aec_uart_debug.h"
#endif

#define MAIN_ADC_GAIN               app_var.aec_mic_gain
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
#define MAIN_ADC_CH_NUM             3
#elif TCFG_AUDIO_DUAL_MIC_ENABLE
#define MAIN_ADC_CH_NUM             2
#else
#define MAIN_ADC_CH_NUM             1
#endif

extern s16 esco_adc_buf[];
struct main_adc_context {
    struct audio_adc_output_hdl dma_output;
    struct adc_mic_ch mic_ch;
    /*s16 dma_buf[VOICE_MIC_DATA_PERIOD_FRAMES * 2 * MAIN_ADC_CH_NUM];*/
    s16 *dma_buf;
#if (MAIN_ADC_CH_NUM > 1)
    s16 mic0_sample_data[VOICE_MIC_DATA_PERIOD_FRAMES];
#endif /*MAIN_ADC_CH_NUM*/
};
/*
 * Mic 数据接收buffer(循环buffer，动态大小)
 */
struct voice_mic_data {
    u8 open;
    u8 source;
    struct main_adc_context *main_adc;
    void *vad_mic;
    struct list_head head;
    cbuffer_t cbuf;
    u8 buf[0];
};

struct voice_mic_capture_channel {
    void *priv;
    void (*output)(void *priv, s16 *data, int len);
    struct list_head entry;
};


static struct voice_mic_data *voice_handle = NULL;

#define __this      (voice_handle)

#if CONFIG_VOICE_MIC_DATA_DUMP
static u8 mic_data_dump = 0;
#endif

static int voice_mic_data_output(void *priv, s16 *data, int len)
{
    struct voice_mic_data *voice = (struct voice_mic_data *)priv;
    struct voice_mic_capture_channel *ch;
    list_for_each_entry(ch, &voice->head, entry) {
        if (ch->output) {
            ch->output(ch->priv, data, len);
        }
    }
    int wlen = cbuf_write(&voice->cbuf, data, len);
    if (wlen < len) {
        putchar('D');
    }

    return wlen;
}

static void audio_main_adc_dma_data_handler(void *priv, s16 *data, int len)
{
    struct voice_mic_data *voice = (struct voice_mic_data *)priv;
    if (!voice || voice->source != VOICE_MCU_MIC) {
        return;
    }

    s16 *pcm_data = data;
#if (MAIN_ADC_CH_NUM > 1)
    pcm_data = voice->main_adc->mic0_sample_data;
    int frames = len >> 1;
    int i = 0;
    for (i = 0; i < frames; i++) {
        pcm_data[i] = data[i * MAIN_ADC_CH_NUM];
    }
#endif
    voice_mic_data_output(voice, pcm_data, len);
    smart_voice_core_post_msg(1, SMART_VOICE_MSG_DMA);
}

#if TCFG_CALL_KWS_SWITCH_ENABLE
static void audio_main_adc_mic_close(struct voice_mic_data *voice, u8 all_channel);
static void audio_main_adc_suspend_handler(int all_channel, int arg)
{
    OS_SEM *sem = (OS_SEM *)arg;
    if (__this) {
        audio_main_adc_mic_close(__this, all_channel);
    }
    os_sem_post(sem);
}

static void audio_main_adc_suspend_in_core_task(u8 all_channel)
{
    if (!__this || !__this->main_adc) {
        return;
    }
    int argv[5];
    OS_SEM *sem = malloc(sizeof(OS_SEM));
    os_sem_create(sem, 0);

    argv[0] = (int)audio_main_adc_suspend_handler;
    argv[1] = 2;
    argv[2] = all_channel;
    argv[3] = (int)sem;

    do {
        int err = os_taskq_post_type(ASR_CORE, Q_CALLBACK, 4, argv);
        if (err == OS_ERR_NONE) {
            break;
        }

        if (err != OS_Q_FULL) {
            audio_main_adc_suspend_handler(all_channel, (int)sem);
            goto exit;
        }
        os_time_dly(2);
    } while (1);

    os_sem_pend(sem, 100);
exit:
    free(sem);
}

void kws_aec_data_output(void *priv, s16 *data, int len)
{
    if (!__this || __this->source != VOICE_MCU_MIC) {
        return;
    }

    audio_main_adc_suspend_in_core_task(0);

    voice_mic_data_output(__this, data, len);
    smart_voice_core_post_msg(1, SMART_VOICE_MSG_DMA);
}

u8 kws_get_state(void)
{
    if (!__this || __this->source != VOICE_MCU_MIC) {
        return 0;
    }

    //aec初始化, 查询是否进入kws模式, 这时有可能需要kws本身打开了mic，需要close
    audio_main_adc_suspend_in_core_task(1);
    return 1;
}
#endif


#define audio_main_adc_mic_ch_setup(ch, mic_ch, ch_map, adc_handle) \
    if (ch_map & BIT(ch)) { \
        audio_adc_mic##ch##_open(mic_ch, ch_map, adc_handle); \
    }

static int audio_main_adc_mic_open(struct voice_mic_data *voice)
{
    if (!voice->main_adc) {
        voice->main_adc = zalloc(sizeof(struct main_adc_context));
    }

    if (!voice->main_adc) {
        return -ENOMEM;
    }

#if TCFG_AUDIO_ANC_ENABLE && (!TCFG_AUDIO_DYNAMIC_ADC_GAIN)
    MAIN_ADC_GAIN = audio_anc_ffmic_gain_get();
#elif TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_DYNAMIC_ADC_GAIN
    anc_dynamic_micgain_start(MAIN_ADC_GAIN);
#endif/*TCFG_AUDIO_ANC_ENABLE && (!TCFG_AUDIO_DYNAMIC_ADC_GAIN)*/
    voice->main_adc->dma_buf = esco_adc_buf;
    audio_adc_mic_open(&voice->main_adc->mic_ch, AUDIO_ADC_MIC_0, &adc_hdl);
    audio_adc_mic_set_gain(&voice->main_adc->mic_ch, MAIN_ADC_GAIN);
#ifdef TCFG_AUDIO_ADC_MIC_CHA
    audio_main_adc_mic_ch_setup(1, &voice->main_adc->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
    audio_main_adc_mic_ch_setup(2, &voice->main_adc->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
    audio_main_adc_mic_ch_setup(3, &voice->main_adc->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
#endif
    audio_adc_mic_set_sample_rate(&voice->main_adc->mic_ch, 16000);
    audio_adc_mic_set_buffs(&voice->main_adc->mic_ch, voice->main_adc->dma_buf,
                            VOICE_MIC_DATA_PERIOD_FRAMES * 2, 2);
    voice->main_adc->dma_output.priv = voice;
    voice->main_adc->dma_output.handler = audio_main_adc_dma_data_handler;
    audio_adc_add_output_handler(&adc_hdl, &voice->main_adc->dma_output);
    audio_adc_mic_start(&voice->main_adc->mic_ch);

    return 0;
}

static void audio_main_adc_mic_close(struct voice_mic_data *voice, u8 all_channel)
{
    if (voice->main_adc) {
#if TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_DYNAMIC_ADC_GAIN
        anc_dynamic_micgain_stop();
#endif/*TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_DYNAMIC_ADC_GAIN*/
        if (all_channel) {
            audio_adc_mic_close(&voice->main_adc->mic_ch);
        }
        audio_adc_del_output_handler(&adc_hdl, &voice->main_adc->dma_output);
        free(voice->main_adc);
        voice->main_adc = NULL;
    }
}


void *voice_mic_data_open(u8 source, int buffer_size, int sample_rate)
{
    if (!__this) {
        __this = zalloc(sizeof(struct voice_mic_data) + buffer_size);
    }

    if (!__this) {
        return NULL;
    }

    if (__this->open) {
        return __this;
    }
    cbuf_init(&__this->cbuf, __this->buf, buffer_size);
    __this->source = source;
    INIT_LIST_HEAD(&__this->head);

#if CONFIG_VOICE_MIC_DATA_EXPORT
    aec_uart_open(1, VOICE_MIC_DATA_PERIOD_FRAMES * 2);
#endif

    if (source == VOICE_VAD_MIC) {
        __this->vad_mic = lp_vad_mic_open((void *)__this, voice_mic_data_output);
    } else if (source == VOICE_MCU_MIC) {
        audio_main_adc_mic_open(__this);
        smart_voice_core_post_msg(1, SMART_VOICE_MSG_WAKE);
    }
    __this->open = 1;
    return __this;
}

void voice_mic_data_close(void *mic)
{
    struct voice_mic_data *voice = (struct voice_mic_data *)mic;
    if (voice->source == VOICE_VAD_MIC) {
        lp_vad_mic_close(voice->vad_mic);
        voice->vad_mic = NULL;
    } else if (voice->source == VOICE_MCU_MIC) {
        audio_main_adc_mic_close(voice, 1);
        smart_voice_core_post_msg(1, SMART_VOICE_MSG_STANDBY);
    }

#if CONFIG_VOICE_MIC_DATA_EXPORT
    aec_uart_close();
#endif

    if (voice) {
        free(voice);
    }
    __this = NULL;
}

void voice_mic_data_switch_source(void *mic, u8 source, int buffer_size, int sample_rate)
{
    struct voice_mic_data *voice = (struct voice_mic_data *)mic;

    if (voice->source == source) {
        return;
    }
    voice->source = source;

    if (voice->source == VOICE_VAD_MIC) {
        audio_main_adc_mic_close(voice, 1);
        voice->vad_mic = lp_vad_mic_open(voice, voice_mic_data_output);
    } else if (voice->source == VOICE_MCU_MIC) {
        lp_vad_mic_close(voice->vad_mic);
        voice->vad_mic = NULL;
        audio_main_adc_mic_open(voice);
        smart_voice_core_post_msg(1, SMART_VOICE_MSG_WAKE);
    }
}

void *voice_mic_data_capture(int sample_rate, void *priv, void (*output)(void *priv, s16 *data, int len))
{
    struct voice_mic_capture_channel *ch = (struct voice_mic_capture_channel *)zalloc(sizeof(struct voice_mic_capture_channel));

    if (!ch) {
        return NULL;
    }
    voice_mic_data_open(VOICE_VAD_MIC, 2048, sample_rate);
    if (!__this) {
        free(ch);
        return NULL;
    }
    ch->priv = priv;
    ch->output = output;
    list_add(&ch->entry, &__this->head);

    lp_vad_mic_test();
    return ch;
}

void voice_mic_data_stop_capture(void *mic)
{
    struct voice_mic_capture_channel *ch = (struct voice_mic_capture_channel *)mic;

    if (ch) {
        list_del(&ch->entry);
        free(ch);
    }

    if (list_empty(&__this->head)) {
        lp_vad_mic_test();
    }
}

int voice_mic_data_read(void *mic, void *data, int len)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;
    int wlen = 0;
    if (cbuf_get_data_len(&fb->cbuf) < len) {
        return 0;
    } else {
        wlen = cbuf_read(&fb->cbuf, data, len);
#if CONFIG_VOICE_MIC_DATA_EXPORT
        aec_uart_fill(0, data, len);
        aec_uart_write();
#endif
        return wlen;
    }
}

int voice_mic_data_buffered_samples(void *mic)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;

    return cbuf_get_data_len(&fb->cbuf) >> 1;
}

void voice_mic_data_clear(void *mic)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;

    cbuf_clear(&fb->cbuf);
}

void voice_mic_data_dump(void *mic)
{
    struct voice_mic_data *fb = (struct voice_mic_data *)mic;

#if CONFIG_VOICE_MIC_DATA_DUMP
    mic_data_dump = 1;
    int len = 0;
    int i = 0;
    s16 *data = (s16 *)cbuf_read_alloc(&fb->cbuf, &len);

    len >>= 1;
    if (data) {
#if 0
        for (i = 0; i < len; i++) {
            if ((i % 3000) == 0) {
                wdt_clear();
            }
            printf("%d\n", data[i]);
        }
#else
        put_buf(data, len << 1);
#endif
    }
    cbuf_read_updata(&fb->cbuf, len << 1);
    mic_data_dump = 0;
#endif
}
