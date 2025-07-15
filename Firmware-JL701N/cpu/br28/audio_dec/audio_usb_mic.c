#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "device/uac_stream.h"
#include "audio_enc.h"
#include "app_main.h"
#include "user_cfg_id.h"
#include "app_config.h"
#include "audio_codec_clock.h"
#include "audio_config.h"
/* #include "application/audio_echo_reverb.h" */

/*USB MIC上行数据同步及变采样模块使能*/
#define USB_MIC_SRC_ENABLE	1

#if TCFG_USB_MIC_CVP_ENABLE
#include "aec_user.h"
#include "overlay_code.h"
#endif/*TCFG_USB_MIC_CVP_ENABLE*/

#if USB_MIC_SRC_ENABLE
#include "audio_track.h"
#include "Resample_api.h"
#endif/*USB_MIC_SRC_ENABLE*/

#define USB_AUDIO_MIC_WRITE_TASK_NAME   "usbmic_write"

#define PCM_ENC2USB_OUTBUF_LEN		(6 * 1024)

#define USB_MIC_BUF_NUM        3

#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
#define USB_MIC_CH_NUM		   3	/* 3mic通话 */
#elif TCFG_AUDIO_DUAL_MIC_ENABLE
#define USB_MIC_CH_NUM         2    /* 双mic通话 */
#else
#define USB_MIC_CH_NUM         1    /* 单mic通话 */
#endif

#define USB_MIC_IRQ_POINTS     256
#define USB_MIC_BUFS_SIZE      (USB_MIC_CH_NUM * USB_MIC_BUF_NUM * USB_MIC_IRQ_POINTS)

#define USB_MIC_STOP  0x00
#define USB_MIC_START 0x01

#ifndef VM_USB_MIC_GAIN
#define     VM_USB_MIC_GAIN             	 5
#endif

#define USB_PLNK_MIC_GAIN_ENABLE		1	  //PLNK_MIC数字增益放大使能
#define USB_PLNK_MIC_DIG_GAIN           60.f  //PLNK_MIC数字增益（Gain = 10^(dB_diff/20))

extern struct audio_adc_hdl adc_hdl;
extern u16 uac_get_mic_vol(const u8 usb_id);
extern int usb_output_sample_rate();
#if USB_MIC_SRC_ENABLE
typedef struct {
    u8 start;
    u8 busy;
    u16 in_sample_rate;
    u16 out_sample_rate;
    u32 *runbuf;
    s16 output[320 * 3 + 16];
    RS_STUCT_API *ops;
    void *audio_track;
} usb_mic_sw_src_t;
static usb_mic_sw_src_t *usb_mic_src = NULL;
#endif/*USB_MIC_SRC_ENABLE*/



struct _usb_mic_hdl {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch    mic_ch;
    s16 adc_buf[USB_MIC_BUFS_SIZE];
    enum enc_source source;
    OS_SEM sem;

    cbuffer_t output_cbuf;
    u8 *output_buf;//[PCM_ENC2USB_OUTBUF_LEN];
    u8 rec_tx_channels;
    u8 mic_data_ok;/*mic数据等到一定长度再开始发送*/
    u8 status;
    u8 drop_data; //用来丢掉刚开mic的前几帧数据
#if	TCFG_USB_MIC_ECHO_ENABLE
    ECHO_API_STRUCT *p_echo_hdl;
#endif
#if (USB_MIC_CH_NUM > 1)
    s16 tmp_buf[USB_MIC_IRQ_POINTS];
#endif/*USB_MIC_CH_NUM*/
#if (USB_MIC_CH_NUM > 2)
    s16 tmp_buf_1[USB_MIC_IRQ_POINTS];
#endif/*USB_MIC_CH_NUM*/
    s16 *usb_mic_data;
    u16 data_len;
    float dig_gain;
};

struct audio_sample_track_context {
    u32 in_sr;
    volatile u8 track_step;
    volatile int samples;
    volatile u32 sample_rate;
    volatile u32 adc_points;
    volatile u32 next_period;
    volatile int period;
};
static u32 adc_hwp = 0;
static u32 adc_points = 0;
static void *user_audio_local_sample_track_open(int in_sr, int sample_rate, int period)
{
    struct audio_sample_track_context *ctx;
    ctx = (struct audio_sample_track_context *)zalloc(sizeof(struct audio_sample_track_context));
    if (!ctx) {
        return NULL;
    }
    ctx->in_sr = in_sr;
    ctx->sample_rate = sample_rate;
    ctx->period = period * sample_rate / 1000;
    adc_points = 0;
    return ctx;
}
int get_audio_adc_points_time()//获取总的采样周期
{
    return adc_points + ((JL_AUDIO->ADC_HWP > adc_hwp) ? (JL_AUDIO->ADC_HWP - adc_hwp) : (JL_AUDIO->ADC_HWP + JL_AUDIO->ADC_LEN - adc_hwp));
}
static int user_audio_local_sample_track_in_period(void *c, int samples)
{
    struct audio_sample_track_context *ctx = (struct audio_sample_track_context *)c;
    if (ctx->track_step == 0) {
        ctx->track_step = 1;
        goto __exit;
    }
    if (ctx->track_step == 1) {
        ctx->track_step = 2;
        ctx->next_period = ctx->period;
        ctx->samples = 0;
        goto __exit;
    }
    ctx->samples += samples;
    if (time_after(ctx->samples, ctx->next_period)) {
        int adc_points = get_audio_adc_points_time();
        int points_diff = (adc_points - ctx->adc_points);
        if (points_diff > 0) {
            int sample_rate = ((u64)((u64)ctx->in_sr * (u64)ctx->samples)) / points_diff;
            /* printf("> track : %d %d %d %d< \n", sample_rate, ctx->samples, ctx->in_sr, points_diff); */
            /* printf("> track : %d < \n", sample_rate); */
            ctx->sample_rate = sample_rate;
        }
        ctx->adc_points = adc_points;
        ctx->next_period = ctx->period;
        ctx->samples = 0;
    }
__exit:
    return ctx->sample_rate;
}
static int user_audio_local_sample_track_rate(void *c)
{
    struct audio_sample_track_context *ctx = (struct audio_sample_track_context *)c;
    return ctx->sample_rate;
}
static void user_audio_local_sample_track_close(void *c)
{
    struct audio_sample_track_context *ctx = (struct audio_sample_track_context *)c;
    if (!ctx) {
        return;
    }
    ctx->track_step = 0;
    free(ctx);
}

static struct _usb_mic_hdl *usb_mic_hdl = NULL;
#if TCFG_USB_MIC_ECHO_ENABLE
ECHO_PARM_SET usbmic_echo_parm_default = {
    .delay = 200,				//回声的延时时间 0-300ms
    .decayval = 50,				// 0-70%
    .direct_sound_enable = 1,	//直达声使能  0/1
    .filt_enable = 1,			//发散滤波器使能
};
EF_REVERB_FIX_PARM usbmic_echo_fix_parm_default = {
    .wetgain = 2048,			////湿声增益：[0:4096]
    .drygain = 4096,				////干声增益: [0:4096]
    .sr = MIC_EFFECT_SAMPLERATE,		////采样率
    .max_ms = 200,				////所需要的最大延时，影响 need_buf 大小
};
#endif
static int usb_audio_mic_sync(u32 data_size)
{
#if 0
    int change_point = 0;

    if (data_size > __this->rec_top_size) {
        change_point = -1;
    } else if (data_size < __this->rec_bottom_size) {
        change_point = 1;
    }

    if (change_point) {
        struct audio_pcm_src src;
        src.resample = 0;
        src.ratio_i = (1024) * 2;
        src.ratio_o = (1024 + change_point) * 2;
        src.convert = 1;
        dev_ioctl(__this->rec_dev, AUDIOC_PCM_RATE_CTL, (u32)&src);
    }
#endif

    return 0;
}

static int usb_audio_mic_tx_handler(int event, void *data, int len)
{
    if (usb_mic_hdl == NULL) {
        return 0;
    }
    if (usb_mic_hdl->status == USB_MIC_STOP) {
        return 0;
    }

    int i = 0;
    int r_len = 0;
    u8 ch = 0;
    u8 double_read = 0;

    /* int rlen = 0; */

    if (usb_mic_hdl->mic_data_ok == 0) {
        if (usb_mic_hdl->output_cbuf.data_len > (PCM_ENC2USB_OUTBUF_LEN / 4)) {
            usb_mic_hdl->mic_data_ok = 1;
        }
        //y_printf("mic_tx NULL\n");
        memset(data, 0, len);
        return len;
    }

#if USB_MIC_SRC_ENABLE
    if (usb_mic_src && usb_mic_src->audio_track) {
        user_audio_local_sample_track_in_period(usb_mic_src->audio_track, (len >> 1) / usb_mic_hdl->rec_tx_channels);
    }
#endif/*USB_MIC_SRC_ENABLE*/

    /* usb_audio_mic_sync(size); */
    if (usb_mic_hdl->rec_tx_channels == 2) {
        len = cbuf_read(&usb_mic_hdl->output_cbuf, data, len / 2);
        s16 *tx_pcm = (s16 *)data;
        int cnt = len / 2;
        for (cnt = len / 2; cnt >= 2;) {
            tx_pcm[cnt - 1] = tx_pcm[cnt / 2 - 1];
            tx_pcm[cnt - 2] = tx_pcm[cnt / 2 - 1];
            cnt -= 2;
        }
        len *= 2;
    } else {
        len = cbuf_read(&usb_mic_hdl->output_cbuf, data, len);
    }
    if (!len) {
        putchar('M');
    }
    return len;
}


int usb_audio_mic_write(void *data, u16 len)
{
    int wlen = len;
    if (usb_mic_hdl) {
        if (usb_mic_hdl->status == USB_MIC_STOP) {
            return len;
        }

        int outlen = len;
        s16 *obuf = data;

#if USB_MIC_SRC_ENABLE
        if (usb_mic_src && usb_mic_src->start) {
            usb_mic_src->busy = 1;
            u32 sr = usb_output_sample_rate();
            usb_mic_src->ops->set_sr(usb_mic_src->runbuf, sr);
            outlen = usb_mic_src->ops->run(usb_mic_src->runbuf, data, len >> 1, usb_mic_src->output);
            usb_mic_src->busy = 0;
            ASSERT(outlen <= (sizeof(usb_mic_src->output) >> 1));
            //printf("16->48k:%d,%d,%d\n",len >> 1,outlen,sizeof(usb_mic_src->output));
            obuf = usb_mic_src->output;
            outlen = outlen << 1;
        }
#endif/*USB_MIC_SRC_ENABLE*/
        wlen = cbuf_write(&usb_mic_hdl->output_cbuf, obuf, outlen);
        if (wlen != outlen) {
            r_printf("usb_mic write full:%d-%d\n", wlen, outlen);
        }
    }
    return wlen;
}

static void adc_output_to_cbuf(void *priv, s16 *data, int len)
{
    if (usb_mic_hdl == NULL) {
        return ;
    }
    if (usb_mic_hdl->status == USB_MIC_STOP) {
        return ;
    }



    switch (usb_mic_hdl->source) {
    case ENCODE_SOURCE_MIC:
    case ENCODE_SOURCE_LINE0_LR:
    case ENCODE_SOURCE_LINE1_LR:
    case ENCODE_SOURCE_LINE2_LR: {

        adc_hwp = JL_AUDIO->ADC_HWP;
        adc_points += len / 2;


#if TCFG_USB_MIC_CVP_ENABLE
#if (USB_MIC_CH_NUM == 3)/*3 Mic*/
        s16 *mic0_data = data;
        s16 *mic1_data = usb_mic_hdl->tmp_buf;
        s16 *mic1_data_pos = usb_mic_hdl->tmp_buf_1;
        //printf("mic_data: %x, %x, %d\n",data, mic1_data_pos, len);
        for (u16 i = 0; i < (len >> 1); i++) {
            mic0_data[i] = data[i * 3];
            mic1_data[i] = data[i * 3 + 1];
            mic1_data_pos[i] = data[i * 3 + 2];
        }
        audio_aec_inbuf_ref(mic1_data, len);
        audio_aec_inbuf_ref_1(mic1_data_pos, len);
        audio_aec_inbuf(data, len);
        return;

#elif (USB_MIC_CH_NUM == 2)/*DualMic*/
        s16 *mic0_data = data;
        s16 *mic1_data = usb_mic_hdl->tmp_buf;
        s16 *mic1_data_pos = data + (len / 2);
        //printf("mic_data:%x,%x,%d\n",data,mic1_data_pos,len);
        for (u16 i = 0; i < (len >> 1); i++) {
            mic0_data[i] = data[i * 2];
            mic1_data[i] = data[i * 2 + 1];
        }
        memcpy(mic1_data_pos, mic1_data, len);

#if 0 /*debug*/
        static u16 mic_cnt = 0;
        if (mic_cnt++ > 300) {
            putchar('1');
            audio_aec_inbuf(mic1_data_pos, len);
            if (mic_cnt > 600) {
                mic_cnt = 0;
            }
        } else {
            putchar('0');
            audio_aec_inbuf(mic0_data, len);
        }
        return;
#endif/*debug end*/
#if (TCFG_AUDIO_DMS_MIC_MANAGE == DMS_MASTER_MIC0)
        audio_aec_inbuf_ref(mic1_data_pos, len);
        audio_aec_inbuf(data, len);
#else
        audio_aec_inbuf_ref(data, len);
        audio_aec_inbuf(mic1_data_pos, len);
#endif/*TCFG_AUDIO_DMS_MIC_MANAGE*/
#else/*SingleMic*/
        audio_aec_inbuf(data, len);
#endif/*USB_MIC_CH_NUM*/

#else /* TCFG_USB_MIC_CVP_ENABLE == 0 */
        if (usb_mic_hdl->drop_data != 0) {
            usb_mic_hdl-> drop_data--;
            memset(data, 0, len);
        }
#if	TCFG_USB_MIC_ECHO_ENABLE
        if (usb_mic_hdl->p_echo_hdl) {
            run_echo(usb_mic_hdl->p_echo_hdl, data, data, len);
        }
#endif

        /* 由于usb_audio_mic_write中SRC执行时间较长在adc中断中写数据会影响usb中断 */
        /* 改为在usb_audio_mic_write_task中向usb写数据 */
        /* adc输入一帧数据，唤醒一次运行任务处理数据 */
        usb_mic_hdl->usb_mic_data = data;
        usb_mic_hdl->data_len = len;
        os_sem_set(&usb_mic_hdl->sem, 0);
        os_sem_post(&usb_mic_hdl->sem);
#endif
    }
    break;
    default:
        break;
    }
}



#if USB_MIC_SRC_ENABLE
static int sw_src_init(u32 in_sr, u32 out_sr)
{
    usb_mic_src = zalloc(sizeof(usb_mic_sw_src_t));
    ASSERT(usb_mic_src);

    usb_mic_src->in_sample_rate = in_sr;
    usb_mic_src->out_sample_rate = out_sr;

    usb_mic_src->ops = get_rs16_context();
    /* usb_mic_src->ops = get_rsfast_context(); */
    g_printf("ops:0x%x\n", usb_mic_src->ops);
    ASSERT(usb_mic_src->ops);
    u32 need_buf = usb_mic_src->ops->need_buf();
    g_printf("runbuf:%d\n", need_buf);
    usb_mic_src->runbuf = malloc(need_buf);
    ASSERT(usb_mic_src->runbuf);
    RS_PARA_STRUCT rs_para_obj;
    rs_para_obj.nch = 1;

    rs_para_obj.new_insample = in_sr;
    rs_para_obj.new_outsample = out_sr;
    printf("sw src,in = %d,out = %d\n", rs_para_obj.new_insample, rs_para_obj.new_outsample);
    usb_mic_src->ops->open(usb_mic_src->runbuf, &rs_para_obj);

    usb_mic_src->audio_track = user_audio_local_sample_track_open(in_sr, out_sr, 2000);
    usb_mic_src->start = 1;
    return 0;
}

static int sw_src_exit(void)
{
    if (usb_mic_src) {
        usb_mic_src->start = 0;
        while (usb_mic_src->busy) {
            putchar('w');
            os_time_dly(2);
        }

        user_audio_local_sample_track_close(usb_mic_src->audio_track);
        usb_mic_src->audio_track = NULL;

        local_irq_disable();
        if (usb_mic_src->runbuf) {
            free(usb_mic_src->runbuf);
            usb_mic_src->runbuf = 0;
        }
        free(usb_mic_src);
        usb_mic_src = NULL;
        local_irq_enable();
        printf("sw_src_exit\n");
    }
    printf("sw_src_exit ok\n");
    return 0;
}
#endif/*USB_MIC_SRC_ENABLE*/



#if TCFG_USB_MIC_CVP_ENABLE
static int usb_mic_aec_output(s16 *data, u16 len)
{
    //putchar('k');
    if (usb_mic_hdl == NULL) {
        return len ;
    }
    if (usb_mic_hdl->status == USB_MIC_STOP) {
        return len;
    }

    int outlen = len;
    s16 *outdat = data;
    /* if (aec_hdl->dump_packet) { */
    /*     aec_hdl->dump_packet--; */
    /*     //memset(outdat, 0, outlen); */
    /* } */
    u16 wlen = usb_audio_mic_write(outdat, outlen);
    return len;

}
#endif

int uac_mic_vol_switch(int vol)
{
    return vol * 14 / 100;
}

float uac_mic_dig_gain_switch(int vol)
{
    float gain = vol / 10.f;
    return gain;
}

/*
*********************************************************************
*                  USB Audio MIC Write Task
* Description: USB MIC 写数据任务
* Arguments  : priv	私用参数
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
static void usb_audio_mic_write_task(void *priv)
{
    printf("==Audio Usb Mic Write Task==\n");

    while (1) {
        os_sem_pend(&usb_mic_hdl->sem, 0);

        putchar('.');
        if (usb_mic_hdl->status) {
            usb_audio_mic_write(usb_mic_hdl->usb_mic_data, usb_mic_hdl->data_len);
        }
    }
}

/*
*********************************************************************
*                  PDM MIC API
* Description: PDM mic操作接口
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
#if (TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC)
/* #define USB_PLNK_SR 		    16000#<{(|16000/44100/48000|)}># */
#define USB_PLNK_SCLK          2400000 /*1M-4M,SCLK/SR需为整数且在1-4096范围*/
#define USB_PLNK_CH_NUM 	     1  /*支持的最大通道(max = 2)*/
#define USB_PLNK_IRQ_POINTS    256 /*采样中断点数*/
#include "pdm_link.h"
#include "media/audio_gain_process.h"
static audio_plnk_t *audio_plnk_mic = NULL;
static void audio_plnk_mic_output(void *buf, u16 len)
{
    s16 *mic0 = (s16 *)buf;
    if (audio_plnk_mic) {
#if TCFG_USB_MIC_CVP_ENABLE
        audio_aec_inbuf(mic0, len << 1);
#else
#if USB_PLNK_MIC_GAIN_ENABLE
        GainProcess_16Bit(mic0, mic0, USB_PLNK_MIC_DIG_GAIN, 1, 1, 1, len);
#endif
        usb_mic_hdl->usb_mic_data = mic0;
        usb_mic_hdl->data_len = len << 1;
        os_sem_set(&usb_mic_hdl->sem, 0);
        os_sem_post(&usb_mic_hdl->sem);
#endif
    }
}

static int usb_pdm_mic_en(u8 en, u16 sr, u16 gain)
{
    printf("usb_pdm_mic_en:%d\n", en);
    if (en) {
        if (audio_plnk_mic) {
            printf("audio_plnk_mic re-malloc error\n");
            return -1;
        }
        audio_plnk_mic = zalloc(sizeof(audio_plnk_t));
        if (audio_plnk_mic == NULL) {
            printf("audio_plnk_mic zalloc failed\n");
            return -1;
        }
        audio_mic_pwr_ctl(MIC_PWR_ON);
        audio_plnk_mic->ch_num = USB_PLNK_CH_NUM;
        audio_plnk_mic->sr = sr;
        audio_plnk_mic->buf_len = USB_PLNK_IRQ_POINTS;
#if (PLNK_CH_EN == PLNK_CH0_EN)
        audio_plnk_mic->ch0_mode = CH0MD_CH0_SCLK_RISING_EDGE;
        audio_plnk_mic->ch1_mode = CH1MD_CH0_SCLK_FALLING_EDGE;
#elif (PLNK_CH_EN == PLNK_CH1_EN)
        audio_plnk_mic->ch0_mode = CH0MD_CH1_SCLK_FALLING_EDGE;
        audio_plnk_mic->ch1_mode = CH1MD_CH1_SCLK_RISING_EDGE;
#else
        audio_plnk_mic->ch0_mode = CH0MD_CH0_SCLK_RISING_EDGE;
        audio_plnk_mic->ch1_mode = CH1MD_CH1_SCLK_RISING_EDGE;
#endif
        audio_plnk_mic->buf = zalloc(audio_plnk_mic->buf_len * audio_plnk_mic->ch_num * 2 * 2);
        ASSERT(audio_plnk_mic->buf);
        audio_plnk_mic->sclk_io = TCFG_AUDIO_PLNK_SCLK_PIN;
        audio_plnk_mic->ch0_io = TCFG_AUDIO_PLNK_DAT0_PIN;
        audio_plnk_mic->ch1_io = TCFG_AUDIO_PLNK_DAT1_PIN;
        audio_plnk_mic->output = audio_plnk_mic_output;
        audio_plnk_open(audio_plnk_mic);
        audio_plnk_start(audio_plnk_mic);
        SFR(JL_ASS->CLK_CON, 0, 2, 3);
    } else {
        audio_plnk_close();
#if TCFG_AUDIO_ANC_ENABLE
        //printf("anc_status:%d\n",anc_status_get());
        if (anc_status_get() == 0) {
            audio_mic_pwr_ctl(MIC_PWR_OFF);
        }
#else
        audio_mic_pwr_ctl(MIC_PWR_OFF);
#endif/*TCFG_AUDIO_ANC_ENABLE*/
        if (audio_plnk_mic->buf) {
            free(audio_plnk_mic->buf);
        }
        if (audio_plnk_mic) {
            free(audio_plnk_mic);
            audio_plnk_mic = NULL;
        }
    }
    return 0;
}
#endif/* #if (TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC) */

int usb_audio_mic_open(void *_info)
{
    int err = 0;
    if (usb_mic_hdl) {
        return 0;
    }
    usb_mic_hdl = zalloc(sizeof(*usb_mic_hdl));
    if (!usb_mic_hdl) {
        return -EFAULT;
    }
    usb_mic_hdl->status = USB_MIC_STOP;

    usb_mic_hdl->output_buf = zalloc(PCM_ENC2USB_OUTBUF_LEN);
    if (!usb_mic_hdl->output_buf) {
        printf("usb_mic_hdl->output_buf NULL\n");

        if (usb_mic_hdl) {
            free(usb_mic_hdl);
            usb_mic_hdl = NULL;
        }
        return -EFAULT;
    }

    u32 sample_rate = (u32)_info & 0xFFFFFF;
    usb_mic_hdl->rec_tx_channels = (u32)_info >> 24;
    usb_mic_hdl->source = ENCODE_SOURCE_MIC;
    printf("usb mic sr:%d ch:%d\n", sample_rate, usb_mic_hdl->rec_tx_channels);


    usb_mic_hdl->drop_data = 10; //用来丢掉前几帧数据
    usb_mic_hdl->dig_gain = 1.f;  // 用来初始化数字增益
    u32 out_sample_rate = sample_rate;
#if TCFG_USB_MIC_CVP_ENABLE
    sample_rate = 16000;
    printf("usb mic sr[aec]:%d\n", sample_rate);
    /*加载清晰语音处理代码*/
    audio_cvp_code_load();
    audio_aec_open(sample_rate, TCFG_USB_MIC_CVP_MODE, usb_mic_aec_output);
#else
    os_sem_create(&usb_mic_hdl->sem, 0);
    err = task_create(usb_audio_mic_write_task, NULL, USB_AUDIO_MIC_WRITE_TASK_NAME);
#endif

#if USB_MIC_SRC_ENABLE
    sw_src_init(sample_rate, out_sample_rate);
#endif /*USB_MIC_SRC_ENABLE*/


    cbuf_init(&usb_mic_hdl->output_cbuf, usb_mic_hdl->output_buf, PCM_ENC2USB_OUTBUF_LEN);

#if TCFG_MIC_EFFECT_ENABLE
    app_var.usb_mic_gain = mic_effect_get_micgain();
#else
    app_var.usb_mic_gain = uac_mic_vol_switch(uac_get_mic_vol(0));
#endif//TCFG_MIC_EFFECT_ENABLE

#if (TCFG_USB_MIC_DATA_FROM_MICEFFECT)
    mic_effect_to_usbmic_onoff(1);
#else
#if (TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC)
    usb_pdm_mic_en(1, sample_rate, app_var.usb_mic_gain);
#else
    audio_mic_pwr_ctl(MIC_PWR_ON);
#if 1
#if (TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_0)
    printf("adc_mic0 open\n");
    audio_adc_mic_open(&usb_mic_hdl->mic_ch, AUDIO_ADC_MIC_0, &adc_hdl);
#ifndef TALK_MIC0_GAIN
    audio_adc_mic_set_gain(&usb_mic_hdl->mic_ch, app_var.usb_mic_gain);
#else
    audio_adc_mic_set_gain(&usb_mic_hdl->mic_ch, TALK_MIC0_GAIN);
#endif
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_0*/
#if (TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_1)
    printf("adc_mic1 open\n");
    audio_adc_mic1_open(&usb_mic_hdl->mic_ch, AUDIO_ADC_MIC_1, &adc_hdl);
#ifndef FF_MIC1_GAIN
    audio_adc_mic1_set_gain(&usb_mic_hdl->mic_ch, app_var.usb_mic_gain);
#else
    audio_adc_mic1_set_gain(&usb_mic_hdl->mic_ch, FF_MIC1_GAIN);
#endif
#endif/*(TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_1)*/
#if (TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_2)
    printf("adc_mic2 open\n");
    audio_adc_mic2_open(&usb_mic_hdl->mic_ch, AUDIO_ADC_MIC_2, &adc_hdl);
#ifndef FB_MIC2_GAIN
    audio_adc_mic2_set_gain(&usb_mic_hdl->mic_ch, app_var.usb_mic_gain);
#else
    audio_adc_mic2_set_gain(&usb_mic_hdl->mic_ch, FB_MIC2_GAIN);
#endif
#endif/*(TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_2)*/
#if (TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_3)
    printf("adc_mic3 open\n");
    audio_adc_mic3_open(&usb_mic_hdl->mic_ch, AUDIO_ADC_MIC_3, &adc_hdl);
#ifndef OTHER_MIC3_GAIN
    audio_adc_mic3_set_gain(&usb_mic_hdl->mic_ch, app_var.usb_mic_gain);
#else
    audio_adc_mic3_set_gain(&usb_mic_hdl->mic_ch, OTHER_MIC3_GAIN);
#endif
#endif/*(TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_3)*/
    audio_adc_mic_set_sample_rate(&usb_mic_hdl->mic_ch, sample_rate);
    audio_adc_mic_set_buffs(&usb_mic_hdl->mic_ch, usb_mic_hdl->adc_buf, USB_MIC_IRQ_POINTS * 2, USB_MIC_BUF_NUM);
    usb_mic_hdl->adc_output.handler = adc_output_to_cbuf;
    audio_adc_add_output_handler(&adc_hdl, &usb_mic_hdl->adc_output);
    audio_adc_mic_start(&usb_mic_hdl->mic_ch);
#else
    audio_mic_open(&usb_mic_hdl->mic_ch, sample_rate, app_var.usb_mic_gain);
    usb_mic_hdl->adc_output.handler = adc_output_to_cbuf;
    audio_mic_add_output(&usb_mic_hdl->adc_output);
    audio_mic_start(&usb_mic_hdl->mic_ch);
#endif
#endif/*TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC*/
#if TCFG_USB_MIC_ECHO_ENABLE
    usb_mic_hdl->p_echo_hdl = open_echo(&usbmic_echo_parm_default, &usbmic_echo_fix_parm_default);
    if (usb_mic_hdl->p_echo_hdl) {
        update_echo_gain(usb_mic_hdl->p_echo_hdl, usbmic_echo_fix_parm_default.wetgain, usbmic_echo_fix_parm_default.drygain);
    }
#endif
#endif//TCFG_USB_MIC_DATA_FROM_MICEFFECT
    set_uac_mic_tx_handler(NULL, usb_audio_mic_tx_handler);

    /* 需要用时钟管理，防止其他操作降低USB_MIC时钟 */
    audio_codec_clock_set(AUDIO_USB_MIC_MODE, AUDIO_CODING_PCM, 0);

    usb_mic_hdl->status = USB_MIC_START;
    /* __this->rec_begin = 0; */
    return 0;

    return -EFAULT;
}

u32 usb_mic_is_running()
{
    if (usb_mic_hdl) {
        return SPK_AUDIO_RATE;
    }
    return 0;
}

/*
 *************************************************************
 *
 *	usb mic gain save
 *
 *************************************************************
 */
static int usb_mic_gain_save_timer;
static u8  usb_mic_gain_save_cnt;
static void usb_audio_mic_gain_save_do(void *priv)
{
    //printf(" usb_audio_mic_gain_save_do %d\n", usb_mic_gain_save_cnt);
    local_irq_disable();
    if (++usb_mic_gain_save_cnt >= 5) {
        sys_timer_del(usb_mic_gain_save_timer);
        usb_mic_gain_save_timer = 0;
        usb_mic_gain_save_cnt = 0;
        local_irq_enable();
        printf("USB_GAIN_SAVE\n");
        syscfg_write(VM_USB_MIC_GAIN, &app_var.usb_mic_gain, 1);
        return;
    }
    local_irq_enable();
}

static void usb_audio_mic_gain_change(u8 gain)
{
    local_irq_disable();
    app_var.usb_mic_gain = gain;
    usb_mic_gain_save_cnt = 0;
    if (usb_mic_gain_save_timer == 0) {
        usb_mic_gain_save_timer = sys_timer_add(NULL, usb_audio_mic_gain_save_do, 1000);
    }
    local_irq_enable();
}

int usb_audio_mic_get_gain(void)
{
    return app_var.usb_mic_gain;
}

void usb_audio_mic_set_gain(int gain)
{
    if (usb_mic_hdl == NULL) {
        r_printf("usb_mic_hdl NULL gain");
        return;
    }
#ifndef CONFIG_BOARD_AISPEECH_NR_GAIN
    gain = uac_mic_vol_switch(gain);
    audio_adc_mic_set_gain(&usb_mic_hdl->mic_ch, gain);
    usb_audio_mic_gain_change(gain);
#else
    printf("AIS 3MIC set digital gain");
    usb_mic_hdl->dig_gain = uac_mic_dig_gain_switch(gain);
#endif /*CONFIG_BOARD_AISPEECH_NR_GAIN*/
}

float usb_audio_mic_get_dig_gain(void)
{
    if (usb_mic_hdl == NULL) {
        r_printf("usb_mic_hdl NULL gain");
        return 1;
    }

    return usb_mic_hdl->dig_gain;
}

int usb_audio_mic_close(void *arg)
{
    if (usb_mic_hdl == NULL) {
        r_printf("usb_mic_hdl NULL close");
        return 0;
    }
    printf("usb_mic_hdl->status %x\n", usb_mic_hdl->status);
    if (usb_mic_hdl && usb_mic_hdl->status == USB_MIC_START) {
        printf("usb_audio_mic_close in\n");
        usb_mic_hdl->status = USB_MIC_STOP;
#if TCFG_USB_MIC_CVP_ENABLE
        audio_aec_close();
#else
        task_kill(USB_AUDIO_MIC_WRITE_TASK_NAME);
#endif
#if (TCFG_USB_MIC_DATA_FROM_MICEFFECT)
        mic_effect_to_usbmic_onoff(0);
#else
#if (TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC)
        usb_pdm_mic_en(0, 0, 0);
#else
#if 1
        audio_adc_mic_close(&usb_mic_hdl->mic_ch);
        audio_adc_del_output_handler(&adc_hdl, &usb_mic_hdl->adc_output);
#else
        audio_mic_close(&usb_mic_hdl->mic_ch, &usb_mic_hdl->adc_output);
#endif
        audio_mic_pwr_ctl(MIC_PWR_OFF);
#endif/*TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC*/
#if TCFG_USB_MIC_ECHO_ENABLE
        if (usb_mic_hdl->p_echo_hdl) {
            close_echo(usb_mic_hdl->p_echo_hdl);
        }
#endif
#endif

#if USB_MIC_SRC_ENABLE
        sw_src_exit();
#endif /*USB_MIC_SRC_ENABLE*/
        cbuf_clear(&usb_mic_hdl->output_cbuf);
        if (usb_mic_hdl) {

            if (usb_mic_hdl->output_buf) {
                free(usb_mic_hdl->output_buf);
                usb_mic_hdl->output_buf = NULL;
            }
            free(usb_mic_hdl);
            usb_mic_hdl = NULL;
        }
#if TCFG_USB_MIC_CVP_ENABLE
        audio_codec_clock_del(AUDIO_USB_MIC_MODE);
#endif /*TCFG_USB_MIC_CVP_ENABLE*/
    }
    printf("usb_audio_mic_close out\n");

    return 0;
}

int usb_mic_stream_sample_rate(void)
{
#if USB_MIC_SRC_ENABLE
    if (usb_mic_src && usb_mic_src->audio_track) {
        int sr = user_audio_local_sample_track_rate(usb_mic_src->audio_track);
        if ((sr < (usb_mic_src->out_sample_rate + 50)) && (sr > (usb_mic_src->out_sample_rate - 50))) {
            return sr;
        }
        return usb_mic_src->out_sample_rate;
    }
#endif /*USB_MIC_SRC_ENABLE*/

    return 0;
}

u32 usb_mic_stream_size()
{
    if (!usb_mic_hdl) {
        return 0;
    }
    if (usb_mic_hdl->status == USB_MIC_START) {
        if (usb_mic_hdl) {
            return cbuf_get_data_size(&usb_mic_hdl->output_cbuf);
        }
    }

    return 0;
}

u32 usb_mic_stream_length()
{
    return PCM_ENC2USB_OUTBUF_LEN;
}

int usb_output_sample_rate()
{
    int sample_rate = usb_mic_stream_sample_rate();
    int buf_size = usb_mic_stream_size();

    sample_rate = (u32)(usb_mic_src->in_sample_rate * sample_rate) / usb_mic_src->out_sample_rate;/*统计输出采样率，重新转换成输入采样率调输入*/
    if (buf_size >= (usb_mic_stream_length() * 1 / 2)) {
        sample_rate += (sample_rate * 5 / 10000);
        //putchar('+');
    }
    if (buf_size <= (usb_mic_stream_length() / 3)) {
        sample_rate -= (sample_rate * 5 / 10000);
        //putchar('-');
    }

    return sample_rate;
}

