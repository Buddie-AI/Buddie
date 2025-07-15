#include "jl_kws_common.h"

#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
#include "audio_enc.h"
#include "asm/audio_adc.h"
#include "audio_codec_clock.h"
//===================================================//
//                    ADC buf配置                    //
//===================================================//
#define ADC_DEMO_BUF_NUM        2
#define ADC_DEMO_IRQ_POINTS     256
#define ADC_DEMO_BUFS_SIZE      (ADC_DEMO_BUF_NUM * ADC_DEMO_IRQ_POINTS)
#define KWS_CBUF_SIZE 	(ADC_DEMO_BUFS_SIZE * sizeof(s16))


//==============================================//
//       KWS 数据来自外部输入(AEC)配置          //
//==============================================//
#ifdef CONFIG_EARPHONE_CASE_ENABLE
#define TCFG_JL_KWS_AUDIO_DATA_FROM_EXTERN 			1 //配置为1, 不需要本地开MIC, 由AEC外部输入
#else
#define TCFG_JL_KWS_AUDIO_DATA_FROM_EXTERN 			0 //配置为0, 需要本地开MIC
#endif /* #ifdef CONFIG_EARPHONE_CASE_ENABLE */


//==============================================//
//          KWS RAM使用OVERLAY配置              //
//==============================================//
#define KWS_USE_OVERLAY_RAM_ENABLE 			0
#define KWS_USE_OVERLAY_RAM_SIZE 			(35 * 1024)
#if KWS_USE_OVERLAY_RAM_ENABLE
static int kws_ram[KWS_USE_OVERLAY_RAM_SIZE / sizeof(int)] sec(.jl_kws_ram);
#endif /* #if KWS_USE_OVERLAY_RAM_ENABLE */

enum KWS_AUDIO_STATE {
    KWS_AUDIO_STATE_IDLE = 0,
    KWS_AUDIO_STATE_RUN,
    KWS_AUDIO_STATE_STOP,
    KWS_AUDIO_STATE_CLOSE,
};

struct kws_adc_mic {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    s16 adc_buf[ADC_DEMO_BUFS_SIZE];    //align 4Bytes
};

struct jl_kws_audio {
    u8 kws_audio_state;
    OS_SEM rx_sem;
    struct kws_adc_mic *kws_adc;
    cbuffer_t kws_cbuf;
    u32 cbuf[KWS_CBUF_SIZE / 4];
};

static struct jl_kws_audio *__kws_audio = NULL;

extern struct audio_adc_hdl adc_hdl;

static void kws_adc_mic_output(void *priv, s16 *data, int len)
{
    int wlen = 0;

    local_irq_disable();/*防止关闭kws时，kws任务和aec任务互相打断导致释放信号量错误死机的问题*/
    if (__kws_audio == NULL) {
        local_irq_enable();
        return;
    }
    if (__kws_audio->kws_audio_state != KWS_AUDIO_STATE_RUN) {
        local_irq_enable();
        return;
    }

    /* kws_putchar('w'); */
    wlen = cbuf_write(&(__kws_audio->kws_cbuf), data, len);
    if (wlen < len) {
        kws_info("kws cbuf full");
    } else {
        //kws_debug("wlen: %d", wlen);
    }

    os_sem_post(&(__kws_audio->rx_sem));
    local_irq_enable();
}


static int jl_kws_adc_mic_open(void)
{
    u16 mic_sr = 16000;
    u8 mic_gain = 10;

    if (__kws_audio->kws_adc != NULL) {
        return JL_KWS_ERR_AUDIO_MIC_STATE_ERR;
    }

    __kws_audio->kws_adc = zalloc(sizeof(struct kws_adc_mic));
    kws_debug("struct kws_adc_mic = 0x%x", (u32)sizeof(struct kws_adc_mic));
    if (__kws_audio->kws_adc == NULL) {
        return JL_KWS_ERR_AUDIO_MIC_NO_BUF;
    }

    kws_debug("kws local open mic");
    //audio_mic_pwr_ctl(MIC_PWR_ON);
    audio_adc_mic_open(&(__kws_audio->kws_adc->mic_ch), AUDIO_ADC_MIC_CH, &adc_hdl);
    audio_adc_mic_set_sample_rate(&(__kws_audio->kws_adc->mic_ch), mic_sr);
    audio_adc_mic_set_gain(&(__kws_audio->kws_adc->mic_ch), mic_gain);
    audio_adc_mic_set_buffs(&(__kws_audio->kws_adc->mic_ch), __kws_audio->kws_adc->adc_buf, ADC_DEMO_IRQ_POINTS * 2, ADC_DEMO_BUF_NUM);
    __kws_audio->kws_adc->adc_output.handler = kws_adc_mic_output;
    audio_adc_add_output_handler(&adc_hdl, &(__kws_audio->kws_adc->adc_output));

    return JL_KWS_ERR_NONE;
}

static int kws_speech_recognition_local_mic_open(void)
{
    jl_kws_adc_mic_open();

    jl_kws_audio_start();

    return 0;
}


//==========================================================//
// 				    JL_KWS AUDIO API                        //
//==========================================================//
int jl_kws_audio_init(void)
{
    int ret = JL_KWS_ERR_NONE;

    if (__kws_audio != NULL) {
        return JL_KWS_ERR_AUDIO_INIT_STATE_ERR;
    }

    __kws_audio = zalloc(sizeof(struct jl_kws_audio));
    kws_debug("struct jl_kws_audio = 0x%x", (u32)sizeof(struct jl_kws_audio));

    if (__kws_audio == NULL) {
        return JL_KWS_ERR_AUDIO_MIC_NO_BUF;
    }

    os_sem_create(&(__kws_audio->rx_sem), 0);

    cbuf_init(&(__kws_audio->kws_cbuf), __kws_audio->cbuf, KWS_CBUF_SIZE);

    audio_codec_clock_set(AUDIO_KWS_MODE, AUDIO_CODING_MSBC, 0);
    return ret;
}

u8 kws_aec_get_state(void)
{
#if TCFG_JL_KWS_AUDIO_DATA_FROM_EXTERN
    if (__kws_audio) {
        //aec初始化, 查询是否进入kws模式, 这时有可能需要kws本身打开了mic，需要close
        if (__kws_audio->kws_adc) {
            kws_debug("kws free adc buf");
            audio_adc_mic_close(&__kws_audio->kws_adc->mic_ch);
            audio_adc_del_output_handler(&adc_hdl, &(__kws_audio->kws_adc->adc_output));
            free(__kws_audio->kws_adc);
            __kws_audio->kws_adc = NULL;

            cbuf_clear(&(__kws_audio->kws_cbuf));
            os_sem_set(&(__kws_audio->rx_sem), 0);
        }
        return 1;
    } else {
        return 0;
    }

#else

    return 0;

#endif /* #if TCFG_JL_KWS_AUDIO_DATA_FROM_EXTERN */
}


u8 kws_get_state(void)
{
    return kws_aec_get_state();
}

u8 kws_is_running(void)
{
    return (__kws_audio != NULL);
}

__attribute__((weak))
int audio_aec_update(u8 EnableBit)
{
    return 0;
}

void kws_aec_data_output(void *priv, s16 *data, int len)
{
    if (__kws_audio == NULL) {
        return ;
    }

    if (__kws_audio->kws_adc) {
        audio_adc_del_output_handler(&adc_hdl, &__kws_audio->kws_adc->adc_output);
        kws_debug("kws free adc buf");
        free(__kws_audio->kws_adc);
        __kws_audio->kws_adc = NULL;
    }

    kws_adc_mic_output(priv, data, len);
}


int jl_kws_audio_get_data(void *buf, u32 len)
{
    int ret = 0;
    u32 data_len = 0;
    u32 rlen = 0;

    if (__kws_audio == NULL) {
        return 0;
    }

    data_len = cbuf_get_data_len(&(__kws_audio->kws_cbuf));

    if (data_len >= len) {
        cbuf_read(&(__kws_audio->kws_cbuf), buf, len);
        rlen = len;
    } else {
        os_sem_set(&(__kws_audio->rx_sem), 0);
#if TCFG_JL_KWS_AUDIO_DATA_FROM_EXTERN
        ret = os_sem_pend(&(__kws_audio->rx_sem), 100);
        if (ret == OS_TIMEOUT) {
            kws_speech_recognition_local_mic_open();
        }
#else
        os_sem_pend(&(__kws_audio->rx_sem), portMAX_DELAY);
#endif /* #if TCFG_JL_KWS_AUDIO_DATA_FROM_EXTERN */
    }

    return rlen;
}


int jl_kws_audio_start(void)
{
    int ret = JL_KWS_ERR_NONE;
    kws_debug("%s", __func__);
    __kws_audio->kws_audio_state = KWS_AUDIO_STATE_RUN;

#if (TCFG_JL_KWS_AUDIO_DATA_FROM_EXTERN == 0)
    ret = jl_kws_adc_mic_open();
    if (ret != JL_KWS_ERR_NONE) {
        return ret;
    }
#endif /* #if TCFG_JL_KWS_AUDIO_DATA_FROM_EXTERN */

    if (__kws_audio && (__kws_audio->kws_adc)) {
        audio_adc_mic_start(&(__kws_audio->kws_adc->mic_ch));
    }

    return ret;
}

void jl_kws_audio_stop(void)
{
    if (__kws_audio) {
        __kws_audio->kws_audio_state = KWS_AUDIO_STATE_STOP;
        if (__kws_audio->kws_adc) {
            audio_adc_mic_close(&__kws_audio->kws_adc->mic_ch);
            audio_adc_del_output_handler(&adc_hdl, &(__kws_audio->kws_adc->adc_output));
            free(__kws_audio->kws_adc);
            __kws_audio->kws_adc = NULL;
        }

        cbuf_clear(&(__kws_audio->kws_cbuf));
        os_sem_set(&(__kws_audio->rx_sem), 0);
    }
}


void jl_kws_audio_close(void)
{
    kws_debug("%s", __func__);
    if (__kws_audio) {
        __kws_audio->kws_audio_state = KWS_AUDIO_STATE_CLOSE;
        if (__kws_audio->kws_adc) {
            audio_adc_mic_close(&__kws_audio->kws_adc->mic_ch);
            audio_adc_del_output_handler(&adc_hdl, &(__kws_audio->kws_adc->adc_output));
            free(__kws_audio->kws_adc);
            __kws_audio->kws_adc = NULL;
        }
        audio_codec_clock_del(AUDIO_KWS_MODE);
        extern u8 audio_aec_status(void);
        if (audio_aec_status()) {
            audio_aec_update(0);
        }
        free(__kws_audio);
        __kws_audio = NULL;
    }
}


#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
