#include "audio_dec_iis.h"
#include "app_config.h"
#include "audio_decoder.h"
#include "media/includes.h"
#include "audio_config.h"
#include "system/includes.h"
#include "audio_enc.h"
#include "application/audio_eq.h"
#include "application/audio_drc.h"
#include "app_config.h"
#include "audio_config.h"
#include "audio_dec.h"
#include "app_main.h"
#include "clock_cfg.h"
#include "audio_dec_eff.h"
#include "audio_codec_clock.h"
#include "audio_dvol.h"
#include "audio_link.h"
#include "circular_buf.h"
#include "sound_device.h"


#if (TCFG_AUDIO_INPUT_IIS)

extern ALINK_PARM alink0_platform_data;
extern struct audio_dac_hdl dac_hdl;
extern struct audio_decoder_task decode_task;
extern struct audio_mixer mixer;
/* extern void *hw_alink; //alink初始化句柄 */
#define IIS_IN_STORE_PCM_SIZE   (4 * 1024)

//iis 驱动结构体
struct iis_in_sample_hdl {
    OS_SEM sem;
    s16 *store_pcm_buf;
    cbuffer_t cbuf;
    int sample_rate;
    void *hw_alink;
    void *alink_ch;
};

// iis 输入解码结构体
struct iis_in_dec_hdl {
    struct audio_decoder decoder;
    struct audio_res_wait wait;
    struct audio_mixer_ch mix_ch;
    u8 channel;
    u8 start;
    u8 need_resume;
    u8 remain;
    u8 output_ch;
    int sample_rate;
    void *iis_in;		//驱动句柄
};
static struct iis_in_dec_hdl *iis_in_dec = NULL;

// ********************** iis in 驱动 ************************* //
static void iis_in_dec_resume(void)
{
    if (iis_in_dec) {
        audio_decoder_resume(&iis_in_dec->decoder);
    }
}

static void pcm_LR_to_mono(s16 *pcm_lr, s16 *pcm_mono, int points_len)
{
    s16 pcm_L;
    s16 pcm_R;
    int i = 0;

    for (i = 0; i < points_len; i++, pcm_lr += 2) {
        pcm_L = *pcm_lr;
        pcm_R = *(pcm_lr + 1);
        *pcm_mono++ = (s16)(((int)pcm_L + pcm_R) >> 1);
    }
}

static void iis_in_data_handler(void *priv, void *_data, int len)
{
    struct iis_in_sample_hdl *iis_in = (struct iis_in_sample_hdl *)priv;
    s16 *data = (s16 *)_data;
    u16 temp_len = len;
    int wlen = 0;

    if (iis_in_dec->start == 0) {
        goto __exit;
    }
    //iis 默认输入的是双声道数据
    if (iis_in_dec->output_ch == 1) {
        s16 *mono_data = data;
        for (int i = 0; i < len / 4; i++) {
            mono_data[i] = mono_data[i * 2];
        }
        len = len / 2;
        wlen = cbuf_write(&iis_in->cbuf, mono_data, len);
        if (wlen != len) {
            putchar('w');
        }
    } else if (iis_in_dec->output_ch == 2) {
        wlen = cbuf_write(&iis_in->cbuf, data, len);
        /* os_sem_post(&iis_in->sem); */
        if (wlen != len) {
            putchar('W');
        }
    } else {
        putchar('e');
        goto __exit;
    }

    if (iis_in_dec && iis_in_dec->need_resume) {
        iis_in_dec->need_resume = 0;
        iis_in_dec_resume();
    }
__exit:
    alink_set_shn(&alink0_platform_data.ch_cfg[1], temp_len / 4);
}

static void *iis_in_open(int sr)
{
    struct iis_in_sample_hdl *iis_in = NULL;
    iis_in = zalloc(sizeof(struct iis_in_sample_hdl));
    if (!iis_in) {
        return NULL;
    }
    iis_in->store_pcm_buf = malloc(IIS_IN_STORE_PCM_SIZE);
    if (!iis_in->store_pcm_buf) {
        free(iis_in);
        return NULL;
    }
    cbuf_init(&iis_in->cbuf, iis_in->store_pcm_buf, IIS_IN_STORE_PCM_SIZE);
    iis_in->hw_alink = (ALINK_PARM *)get_iis_alink_param();
    if (iis_in->hw_alink == NULL) {
        iis_in->hw_alink = alink_init(&alink0_platform_data);
    }
    iis_in->alink_ch = alink_channel_init(iis_in->hw_alink, 1, ALINK_DIR_RX, (void *)iis_in, iis_in_data_handler);
    alink_start(iis_in->hw_alink);
    iis_in->sample_rate = sr;
    return iis_in;
}


// *********************** iis in dec 解码 ***************************//
static int iis_in_stream_read(struct audio_decoder *decoder, void *buf, u32 len)
{
    int rlen = 0;
    struct iis_in_sample_hdl *iis_in = (struct iis_in_sample_hdl *)iis_in_dec->iis_in;
    if (!iis_in || iis_in_dec->start == 0) {
        goto __exit;
    }
    if (!iis_in->store_pcm_buf) {
        goto __exit;
    }
    int read_len = cbuf_get_data_size(&iis_in->cbuf);
    if (read_len) {
        read_len = read_len > len ? len : read_len;
        rlen = cbuf_read(&iis_in->cbuf, buf, read_len);
        /* printf("--- rlen : %d\n", rlen); */
    }
__exit:
    if (!rlen) {
        iis_in_dec->need_resume = 1;
        return -1;
    }
    /* y_printf("--- rlen = %d\n",rlen); */
    return rlen;
}

static int iis_in_dec_probe_handler(struct audio_decoder *decoder)
{
    /* struct pcm_dec_hdl *dec = container_of(decoder, struct pcm_dec_hdl, decoder); */
    struct iis_in_dec_hdl *dec = container_of(decoder, struct iis_in_dec_hdl, decoder);
    return 0;
}

static int iis_in_dec_output_handler(struct audio_decoder *decoder, s16 *data, int len, void *priv)
{
    struct iis_in_dec_hdl *dec = container_of(decoder, struct iis_in_dec_hdl, decoder);

    if (!dec) {
        return 0;
    }
    int rlen = len;
    int wlen = 0;

    if (!dec->remain) {
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
        audio_digital_vol_run(MUSIC_DVOL, data, len);
#endif
    }

    wlen = audio_mixer_ch_write(&dec->mix_ch, data, rlen);
    if (!wlen) {
        putchar('w');
    }
    rlen -= wlen;

    if (rlen == 0) {
        dec->remain = 0;
    } else {
        dec->remain = 1;
    }
    return len - rlen;
}

static const struct audio_dec_handler iis_in_dec_handler = {
    .dec_probe  = iis_in_dec_probe_handler,
    .dec_output = iis_in_dec_output_handler,
};

static const struct audio_dec_input iis_input = {
    .coding_type = AUDIO_CODING_PCM,
    .data_type   = AUDIO_INPUT_FILE,
    .ops = {
        .file = {
            .fread = iis_in_stream_read,
        }
    }
};

static void iis_in_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_DEC_EVENT_END:
        /*pcm_audio_close();*/
        break;
    }
}

//开始 iis dec 解码
static int iis_in_dec_start(void)
{
    int err;
    y_printf(">>>>>>>> Enter Func: %s \n", __func__);
    struct audio_fmt f = {0};
    struct iis_in_dec_hdl *dec = iis_in_dec;

    if (!iis_in_dec && iis_in_dec->start == 1) {
        return -EINVAL;
    }

    err = audio_decoder_open(&dec->decoder, &iis_input, &decode_task);
    if (err) {
        goto __err;
    }
    if (dec->iis_in == NULL) {
        y_printf("--> %s, %d, Init iis_in!\n", __func__, __LINE__);
        dec->iis_in = iis_in_open(dec->sample_rate);
        if (!dec->iis_in) {
            goto __err;
        }
    }

    audio_decoder_set_handler(&dec->decoder, &iis_in_dec_handler);
    audio_decoder_set_event_handler(&dec->decoder, iis_in_dec_event_handler, 0);
    u8 dac_conn = audio_dac_get_channel(&dac_hdl);
    if (dac_conn == DAC_OUTPUT_LR) {
        dec->output_ch = 2;
    } else {
        dec->output_ch = 1;
    }

    f.coding_type = AUDIO_CODING_PCM;
    f.sample_rate = dec->sample_rate;
    f.channel = dec->output_ch;

    sound_pcm_dev_channel_mapping(dec->output_ch);

    err = audio_decoder_set_fmt(&dec->decoder, &f);
    if (err) {
        goto __err;
    }


#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_open(MUSIC_DVOL, app_audio_get_volume(APP_AUDIO_STATE_MUSIC), MUSIC_DVOL_MAX, MUSIC_DVOL_FS, -1);
#endif

    // 设置叠加功能
    audio_mixer_ch_open(&dec->mix_ch, &mixer);
    audio_mixer_ch_set_sample_rate(&dec->mix_ch, audio_output_rate(f.sample_rate));
    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());

    int vol = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, vol, 1);

    err = audio_decoder_start(&dec->decoder);
    if (err) {
        goto __err;
    }
    /* audio_codec_clock_set(AUDIO_PC_MODE, AUDIO_CODING_PCM, dec->wait.preemption); */
    dec->start = 1;
    return 0;

__err:
    return err;
}

// iis in 解码资源等待
void iis_in_dec_stop(void);

static int iis_in_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;
    if (event == AUDIO_RES_GET) {
        // 启动解码
        err = iis_in_dec_start();
    } else if (event == AUDIO_RES_PUT) {
        // 被打断
        iis_in_dec_stop();
    }
    return err;
}

int iis_in_dec_open(void)
{
    int err;
    struct iis_in_dec_hdl *dec = NULL;;
    dec = zalloc(sizeof(*dec));
    if (!dec) {
        return -ENOMEM;
    }
    iis_in_dec = dec;
    dec->sample_rate = TCFG_IIS_SR;
    dec->channel = 2;		//iis 是双声道数据
    dec->wait.priority = 2;
    dec->wait.preemption = 1;
    dec->wait.handler = iis_in_wait_res_handler;

    err = audio_decoder_task_add_wait(&decode_task, &dec->wait);
    clock_add(DEC_PCM_CLK);
    return err;
}


void iis_in_dec_stop(void)
{
    if (iis_in_dec && iis_in_dec->start == 1) {
        struct iis_in_sample_hdl *iis_in = (struct iis_in_sample_hdl *)iis_in_dec->iis_in;
        iis_in_dec->start = 0;
        audio_decoder_close(&iis_in_dec->decoder);
        audio_mixer_ch_close(&iis_in_dec->mix_ch);
        app_audio_state_exit(APP_AUDIO_STATE_MUSIC);

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
        audio_digital_vol_close(MUSIC_DVOL);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

        local_irq_disable();
#if 1
        if (iis_in) {
            void *alink_p = (ALINK_PARM *)get_iis_alink_param();
            if (iis_in->hw_alink && !alink_p) {
                alink_uninit(iis_in->hw_alink);
                iis_in->hw_alink = NULL;
            } else {
                /* alink_uninit(iis_in->hw_alink); */
                alink_channel_close(iis_in->alink_ch);
            }
            if (iis_in->store_pcm_buf) {
                free(iis_in->store_pcm_buf);
                iis_in->store_pcm_buf = NULL;
            }
            free(iis_in);
            iis_in = NULL;
            iis_in_dec->iis_in = NULL;
        }
#endif
        local_irq_enable();
    }
}

void iis_in_dec_close(void)
{
    if (!iis_in_dec) {
        return;
    }
    iis_in_dec_stop();
    audio_decoder_task_del_wait(&decode_task, &iis_in_dec->wait);
    clock_remove(DEC_PCM_CLK);
    local_irq_disable();
    free(iis_in_dec);
    iis_in_dec = NULL;
    local_irq_enable();
}



#endif



