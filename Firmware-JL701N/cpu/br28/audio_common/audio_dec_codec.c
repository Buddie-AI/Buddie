#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "classic/hci_lmp.h"
#include "aec_user.h"
#include "asm/audio_src.h"
#include "audio_enc.h"
#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif
#include "app_main.h"
#include "btstack/avctp_user.h"

#if 1//(TCFG_ENC_OPUS_ENABLE) || (TCFG_ENC_SPEEX_ENABLE)

extern struct audio_encoder_task *encode_task;
//static struct audio_encoder_task *encode_task = NULL;

#define ENC_ADC_BUF_NUM        (2)
#if TCFG_AUDIO_ANC_ENABLE
//JL701N打开ANC，ADC需要使用相同buffer和中断点数
#define ENC_ADC_IRQ_POINTS     (240)
#else
#define ENC_ADC_IRQ_POINTS     (320)
#endif/*TCFG_AUDIO_ANC_ENABLE*/
#define ENC_ADC_BUFS_SIZE      (ENC_ADC_BUF_NUM * ENC_ADC_IRQ_POINTS)

#define MIC_USE_MIC_CHANNEL    (1)
#define MIC_ENC_IN_SIZE		(ENC_ADC_IRQ_POINTS * 2)
#define MIC_ENC_OUT_SIZE       (ENC_ADC_IRQ_POINTS)

struct dec_enc_hdl {
    struct audio_encoder encoder;
    OS_SEM pcm_frame_sem;
    u8 output_frame[MIC_ENC_OUT_SIZE];
    u8  pcm_frame[MIC_ENC_IN_SIZE];
    u8 frame_size;
    u8 in_cbuf_buf[MIC_ENC_IN_SIZE * 4];
    cbuffer_t pcm_in_cbuf;
    int (*dec_output)(void *priv, void *buf, int len);
#if mic_ENC_PACK_ENABLE
    u16 cp_type;
    u16 packet_head_sn;
#endif
};

static struct dec_enc_hdl *dec_enc = NULL;

static void dec_enc_output_func_register(int (*output_func)(void *priv, void *buf, int len))
{
    if (dec_enc) {
        dec_enc->dec_output = output_func;
    }
}

void dec_enc_resume(void)
{
    if (dec_enc) {
        os_sem_post(&dec_enc->pcm_frame_sem);
    }
}

static int dec_enc_pcm_get(struct audio_encoder *encoder, s16 **frame, u16 frame_len)
{
    int pcm_len = 0;
    if (dec_enc == NULL) {
        r_printf("dec_enc NULL\n");
        return 0;
    }


    /* putchar('!'); */
    if ((&dec_enc->pcm_in_cbuf)->data_len < frame_len) {
        /* putchar('#'); */
        os_sem_set(&dec_enc->pcm_frame_sem, 0);
        os_sem_pend(&dec_enc->pcm_frame_sem, 5);
        if (dec_enc == NULL) {
            printf("dec_enc is NULL\n");
            return 0;
        }
    }

    pcm_len = cbuf_read(&dec_enc->pcm_in_cbuf, dec_enc->pcm_frame, frame_len);
    if (pcm_len != frame_len) {
        putchar('L');
    }
    /* putchar('D'); */

    *frame = dec_enc->pcm_frame;
    return pcm_len;
}

static void dec_enc_pcm_put(struct audio_encoder *encoder, s16 *frame)
{
}

static const struct audio_enc_input dec_enc_input = {
    .fget = dec_enc_pcm_get,
    .fput = dec_enc_pcm_put,
};

static int dec_enc_probe_handler(struct audio_encoder *encoder)
{
    return 0;
}




static int dec_enc_output_handler(struct audio_encoder *encoder, u8 *frame, int len)
{
    if (encoder == NULL) {
        r_printf("encoder NULL");
    }
    wdt_clear();
    /* printf("mic frame len:%d \n",len); */
    if (dec_enc && dec_enc->dec_output) {
        dec_enc->dec_output(NULL, frame, len);
    }
    /* put_buf(frame, 16); */
    return len;
}

const static struct audio_enc_handler dec_enc_handler = {
    .enc_probe = dec_enc_probe_handler,
    .enc_output = dec_enc_output_handler,
};

static void dec_enc_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    printf("dec_enc_event_handler:0x%x,%d\n", argv[0], argv[0]);
    switch (argv[0]) {
    case AUDIO_ENC_EVENT_END:
        puts("AUDIO_ENC_EVENT_END\n");
        break;
    }
}



void adc_dec_output_handler(s16 *data, int len)
{
    if (dec_enc) {
        u16 wlen = cbuf_write(&dec_enc->pcm_in_cbuf, data, len);
        if (wlen != len) {
            putchar('@');
        }
        audio_encoder_resume(&dec_enc->encoder);
        dec_enc_resume();
    }
}



#define HIGH_COMPLEX		0
#define LOW_COMPLEX			(BIT(4))


extern struct audio_adc_hdl adc_hdl;
int audio_dec_enc_open(int (*dec_output)(void *priv, void *buf, int len), u32 code_type, u8 ai_type)
{
    int err;
    struct audio_fmt fmt;

    switch (code_type) {
    case AUDIO_CODING_OPUS:
        //1. quality:bitrate     0:16kbps    1:32kbps    2:64kbps
        //   quality: MSB_2:(bit7_bit6)     format_mode    //0:百度_无头.                   1:酷狗_eng+range.
        //   quality:LMSB_2:(bit5_bit4)     low_complexity //0:高复杂度,高质量.兼容之前库.  1:低复杂度,低质量.
        //2. sample_rate         sample_rate=16k         ignore
        fmt.quality = 0 | ai_type/*| LOW_COMPLEX*/;
        fmt.sample_rate = 16000;
        fmt.coding_type = AUDIO_CODING_OPUS;
        break;
    case AUDIO_CODING_SPEEX:
        fmt.quality = 5;
        fmt.complexity = 2;
        fmt.sample_rate = 16000;
        fmt.coding_type = AUDIO_CODING_SPEEX;
        break;
    default:
        printf("do not support this type !!!\n");
        return -1;
        break;
    }

    if (!encode_task) {
        encode_task = zalloc(sizeof(*encode_task));
        if (!encode_task) {
            printf("encode_task NULL !!!\n");
        }
        audio_encoder_task_create(encode_task, "audio_enc");
    }
    if (!dec_enc) {
        dec_enc = zalloc(sizeof(*dec_enc));
        if (!dec_enc) {
            printf("dec_enc NULL !!!\n");
        }
        memset(dec_enc, 0x00, sizeof(*dec_enc));
    }

    dec_enc_output_func_register(dec_output);
    cbuf_init(&dec_enc->pcm_in_cbuf, dec_enc->in_cbuf_buf, MIC_ENC_IN_SIZE * 4);
    os_sem_create(&dec_enc->pcm_frame_sem, 0);
    audio_encoder_open(&dec_enc->encoder, &dec_enc_input, encode_task);
    audio_encoder_set_handler(&dec_enc->encoder, &dec_enc_handler);
    audio_encoder_set_fmt(&dec_enc->encoder, &fmt);
    audio_encoder_set_event_handler(&dec_enc->encoder, dec_enc_event_handler, 0);
    audio_encoder_set_output_buffs(&dec_enc->encoder, dec_enc->output_frame,
                                   sizeof(dec_enc->output_frame), 1);
    if (!dec_enc->encoder.enc_priv) {
        log_e("encoder err, maybe coding(0x%x) disable \n", fmt.coding_type);
        err = -EINVAL;
        goto __err;
    }

    int start_err = audio_encoder_start(&dec_enc->encoder);

// #if MIC_USE_MIC_CHANNEL
//     fmt.sample_rate = 16000;
//     audio_mic_pwr_ctl(MIC_PWR_ON);
//     audio_adc_mic_open(&dec_enc->mic_ch, BIT(0), &adc_hdl);
//     audio_adc_mic_set_sample_rate(&mic_enc->mic_ch, fmt.sample_rate);
// #if TCFG_AUDIO_ANC_ENABLE
//     app_var.aec_mic_gain = audio_anc_ffmic_gain_get();
// #endif
//     printf(">>>>>>>>>mic gain:%d \n", app_var.aec_mic_gain);
//     audio_adc_mic_set_gain(&mic_enc->mic_ch, app_var.aec_mic_gain);

// #if TCFG_AUDIO_ANC_ENABLE
//     JL701N打开ANC，ADC需要使用相同buffer和中断点数
//     extern s16 esco_adc_buf[];
//     audio_adc_mic_set_buffs(&mic_enc->mic_ch, esco_adc_buf,
//                             ENC_ADC_IRQ_POINTS * 2, ENC_ADC_BUF_NUM);
// #else
//     audio_adc_mic_set_buffs(&mic_enc->mic_ch, mic_enc->adc_buf,
//                             ENC_ADC_IRQ_POINTS * 2, ENC_ADC_BUF_NUM);
// #endif/*TCFG_AUDIO_ANC_ENABLE*/
//     dec_enc->adc_output.handler = adc_dec_output_handler;
//     audio_adc_add_output_handler(&adc_hdl, &mic_enc->adc_output);


//     app_audio_output_samplerate_set(44100);
//     app_audio_output_start();
//     audio_adc_mic_start(&mic_enc->mic_ch);
// #endif

    printf("dec_enc_open ok %d\n", start_err);


    return 0;
__err:
    audio_encoder_close(&dec_enc->encoder);

    local_irq_disable();
    free(dec_enc);
    dec_enc = NULL;
    local_irq_enable();

    return err;
}




int audio_dec_enc_close()
{
    if (!dec_enc) {
        return -1;
    }
    printf("audio_dec_enc_close\n");
// #if MIC_USE_MIC_CHANNEL
//     audio_adc_mic_close(&dec_enc->mic_ch);
// #if TCFG_AUDIO_ANC_ENABLE
//     if (anc_status_get() == 0) {
//         audio_mic_pwr_ctl(MIC_PWR_OFF);
//     }
// #else
//     audio_mic_pwr_ctl(MIC_PWR_OFF);
// #endif
//     audio_adc_del_output_handler(&adc_hdl, &mic_enc->adc_output);
// #endif
    dec_enc_resume();
    audio_encoder_close(&dec_enc->encoder);

    /* if (encode_task) { */
    /* audio_encoder_task_del(encode_task); */
    /* free(encode_task); */
    /* encode_task = NULL; */
    /* } */

    free(dec_enc);
    dec_enc = NULL;

    printf("audio_dec_enc_close end\n");
    return 0;
}

#endif
