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
#include "audio_dec_pcm.h"

struct pcm_dec_hdl {
    struct audio_decoder decoder;
    struct audio_res_wait wait;
    struct audio_mixer_ch mix_ch;
    u8 start;
    u8 remain;
    u8 channel;
    u8 output_ch;
    int sample_rate;
    int (*pcm_read_cb)(void *, void *, int);
};
static struct pcm_dec_hdl *pcm_dec = NULL;
extern struct audio_decoder_task decode_task;
extern struct audio_dac_hdl dac_hdl;


void pcm_dec_set_read_cb(int (*pcm_read_func)(void *, void *, int))
{
    local_irq_disable();
    if (pcm_dec && pcm_dec->start == 1) {
        pcm_dec->pcm_read_cb = pcm_read_func;
    } else {
        y_printf(">>> %s, %d, pcm_dec set read cb failed!\n", __func__, __LINE__);
    }
    local_irq_enable();
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

static int pcm_stream_read(struct audio_decoder *decoder, void *buf, u32 len)
{
    int rlen = 0;
    struct pcm_dec_hdl *dec = container_of(decoder, struct pcm_dec_hdl, decoder);

    local_irq_disable();
    if (dec) {
        if (dec->pcm_read_cb) {
            rlen = dec->pcm_read_cb(NULL, (void *)buf, len);
        }
        if (dec->channel == 2 && dec->output_ch == 1) {
            /* pcm_LR_to_mono((s16 *)buf, (s16 *)buf, rlen >> 2); */
            /* rlen >>= 1; */
        }
    }
    local_irq_enable();
    if (!rlen) {
        return -1;
    }
    return rlen;
}

static const struct audio_dec_input pcm_input = {
    .coding_type = AUDIO_CODING_PCM,
    .data_type   = AUDIO_INPUT_FILE,
    .ops = {
        .file = {
            .fread = pcm_stream_read,
        }
    }
};

static int pcm_dec_probe_handler(struct audio_decoder *decoder)
{
    struct pcm_dec_hdl *dec = container_of(decoder, struct pcm_dec_hdl, decoder);
    return 0;
}

static int pcm_dec_output_handler(struct audio_decoder *decoder, s16 *data, int len, void *priv)
{
    struct pcm_dec_hdl *dec = container_of(decoder, struct pcm_dec_hdl, decoder);

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

static const struct audio_dec_handler pcm_dec_handler = {
    .dec_probe  = pcm_dec_probe_handler,
    .dec_output = pcm_dec_output_handler,
};

static void pcm_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_DEC_EVENT_END:
        /*pcm_audio_close();*/
        break;
    }
}

void pcm_dec_resume(void)
{
    local_irq_disable();
    if (pcm_dec && pcm_dec->start) {
        audio_decoder_resume(&pcm_dec->decoder);
    }
    local_irq_enable();
}

static int pcm_audio_play_start(void)
{
    int err;
    y_printf(">>>>>>>> Enter Func: %s \n", __func__);
    struct audio_fmt f = {0};
    struct pcm_dec_hdl *dec = pcm_dec;

    if (!pcm_dec) {
        return -EINVAL;
    }

    err = audio_decoder_open(&dec->decoder, &pcm_input, &decode_task);
    if (err) {
        goto __err;
    }

    audio_decoder_set_handler(&dec->decoder, &pcm_dec_handler);
    audio_decoder_set_event_handler(&dec->decoder, pcm_dec_event_handler, 0);

    u8 dac_conn = audio_dac_get_channel(&dac_hdl);
    if (dac_conn == DAC_OUTPUT_LR) {
        dec->output_ch = 2;
    } else {
        dec->output_ch = 1;
    }

    f.coding_type = AUDIO_CODING_PCM;
    f.sample_rate = dec->sample_rate;
    f.channel = dec->output_ch;

    err = audio_decoder_set_fmt(&dec->decoder, &f);

    printf(">>>>>>>> %s, dec->output_ch : %d\n", __func__, dec->output_ch);

#if TCFG_AUDIO_OUTPUT_IIS
    sound_pcm_dev_channel_mapping(dec->output_ch);
#endif

    if (err) {
        goto __err;
    }


#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_open(MUSIC_DVOL, app_audio_get_volume(APP_AUDIO_STATE_MUSIC), MUSIC_DVOL_MAX, MUSIC_DVOL_FS, -1);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

    // 设置叠加功能
    audio_mixer_ch_open(&dec->mix_ch, &mixer);
    audio_mixer_ch_set_sample_rate(&dec->mix_ch, audio_output_rate(f.sample_rate));
    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());

    int vol = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, vol, 0);

    err = audio_decoder_start(&dec->decoder);
    if (err) {
        goto __err;
    }
    audio_codec_clock_set(AUDIO_PC_MODE, AUDIO_CODING_PCM, dec->wait.preemption);
    dec->start = 1;
    return 0;

__err:

    return err;
}

static int pcm_audio_play_stop(void)
{
    if (!pcm_dec || !pcm_dec->start) {
        return 0;
    }
    pcm_dec->start = 0;
    audio_decoder_close(&pcm_dec->decoder);
    audio_mixer_ch_close(&pcm_dec->mix_ch);
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_close(MUSIC_DVOL);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/
    return 0;
}

static int pcm_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;

    if (event == AUDIO_RES_GET) {
        err = pcm_audio_play_start();
    } else if (event == AUDIO_RES_PUT) {
        /* pcm_audio_play_stop(); */
    }
    return err;
}


int pcm_audio_play_open(int sr, u8 ch)
{
    struct pcm_dec_hdl *dec;

    if (pcm_dec) {
        return 0;
    }

    dec = zalloc(sizeof(*dec));
    if (!dec) {
        return -ENOMEM;
    }

    pcm_dec = dec;

    dec->sample_rate = sr;
    dec->channel = ch;
    y_printf(">>> pcm_audio_play_open sr:%d ch:%d\n", dec->sample_rate, dec->channel); //通道是2个
    dec->wait.priority = 2;
    dec->wait.preemption = 1;
    dec->wait.handler = pcm_wait_res_handler;
    audio_decoder_task_add_wait(&decode_task, &dec->wait);

    clock_add(DEC_PCM_CLK);
    return 0;
}


void pcm_audio_play_close(void)
{
    if (!pcm_dec) {
        return;
    }
    pcm_audio_play_stop();
    audio_decoder_task_del_wait(&decode_task, &pcm_dec->wait);
    clock_remove(DEC_PCM_CLK);
    local_irq_disable();
    free(pcm_dec);
    pcm_dec = NULL;
    local_irq_enable();
}

