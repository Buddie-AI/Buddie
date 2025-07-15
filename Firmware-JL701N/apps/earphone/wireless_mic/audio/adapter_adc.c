#include "adapter_adc.h"
#include "media/includes.h"
#include "audio_config.h"
#include "asm/dac.h"
#include "audio_splicing.h"
#include "app_config.h"

#if	(TCFG_WIRELESS_MIC_ENABLE)
#if (TCFG_AUDIO_ADC_ENABLE)

#define ADAPTER_ADC_MIC_BUF_NUM        	3
#if (TCFG_WIRELESS_MIC_STEREO_EN)
#define ADAPTER_ADC_MIC_ADC_CH         	2
#else
#define ADAPTER_ADC_MIC_ADC_CH         	1
#endif

#define BUF_ALIN(var,al)     ((((var)+(al)-1)/(al))*(al))

static u16 adapter_adc_irq_points = 0;

extern struct audio_adc_hdl adc_hdl;
extern struct audio_dac_hdl dac_hdl;
extern struct adc_platform_data adc_data;

void adapter_adc_init(void)
{
    audio_adc_init(&adc_hdl, &adc_data);
#if TCFG_AUDIO_DAC_ENABLE
    //dac要开一下然后关一下, 否则mic不正常
    audio_dac_start(&dac_hdl);
    audio_dac_stop(&dac_hdl);
#endif//TCFG_AUDIO_DAC_ENABLE
}



struct __adapter_adc_mic {
    s16 *adc_buf;//[ADAPTER_ADC_MIC_BUF_SIZE];    //align 4Bytes
    u8  *pcm_buf;//[ADAPTER_PCM_BUF_SIZE];
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    cbuffer_t cbuf;
    u16 skip_cnt;                    //用来丢掉刚开mic时的前几包异常数据
    volatile u8 start;

    void *resume_priv;
    void (*resume)(void *resume_priv);

    void *data_callback_priv;
    void (*data_callback)(void *data_callback_priv, u8 *data, u16 len);
};


short test_sin_441k[441] = {
    0x0000, 0x122d, 0x23fb, 0x350f, 0x450f, 0x53aa, 0x6092, 0x6b85, 0x744b, 0x7ab5, 0x7ea2, 0x7fff, 0x7ec3, 0x7af6, 0x74ab, 0x6c03,
    0x612a, 0x545a, 0x45d4, 0x35e3, 0x24db, 0x1314, 0x00e9, 0xeeba, 0xdce5, 0xcbc6, 0xbbb6, 0xad08, 0xa008, 0x94fa, 0x8c18, 0x858f,
    0x8181, 0x8003, 0x811d, 0x84ca, 0x8af5, 0x9380, 0x9e3e, 0xaaf7, 0xb969, 0xc94a, 0xda46, 0xec06, 0xfe2d, 0x105e, 0x223a, 0x3365,
    0x4385, 0x5246, 0x5f5d, 0x6a85, 0x7384, 0x7a2d, 0x7e5b, 0x7ffa, 0x7f01, 0x7b75, 0x7568, 0x6cfb, 0x6258, 0x55b7, 0x4759, 0x3789,
    0x2699, 0x14e1, 0x02bc, 0xf089, 0xdea7, 0xcd71, 0xbd42, 0xae6d, 0xa13f, 0x95fd, 0x8ce1, 0x861a, 0x81cb, 0x800b, 0x80e3, 0x844e,
    0x8a3c, 0x928c, 0x9d13, 0xa99c, 0xb7e6, 0xc7a5, 0xd889, 0xea39, 0xfc5a, 0x0e8f, 0x2077, 0x31b8, 0x41f6, 0x50de, 0x5e23, 0x697f,
    0x72b8, 0x799e, 0x7e0d, 0x7fee, 0x7f37, 0x7bed, 0x761f, 0x6ded, 0x6380, 0x570f, 0x48db, 0x392c, 0x2855, 0x16ad, 0x048f, 0xf259,
    0xe06b, 0xcf20, 0xbed2, 0xafd7, 0xa27c, 0x9705, 0x8db0, 0x86ab, 0x821c, 0x801a, 0x80b0, 0x83da, 0x8988, 0x919c, 0x9bee, 0xa846,
    0xb666, 0xc603, 0xd6ce, 0xe86e, 0xfa88, 0x0cbf, 0x1eb3, 0x3008, 0x4064, 0x4f73, 0x5ce4, 0x6874, 0x71e6, 0x790a, 0x7db9, 0x7fdc,
    0x7f68, 0x7c5e, 0x76d0, 0x6ed9, 0x64a3, 0x5863, 0x4a59, 0x3acc, 0x2a0f, 0x1878, 0x0661, 0xf42a, 0xe230, 0xd0d0, 0xc066, 0xb145,
    0xa3bd, 0x9813, 0x8e85, 0x8743, 0x8274, 0x8030, 0x8083, 0x836b, 0x88da, 0x90b3, 0x9acd, 0xa6f5, 0xb4ea, 0xc465, 0xd515, 0xe6a3,
    0xf8b6, 0x0aee, 0x1ced, 0x2e56, 0x3ecf, 0x4e02, 0x5ba1, 0x6764, 0x710e, 0x786f, 0x7d5e, 0x7fc3, 0x7f91, 0x7cc9, 0x777a, 0x6fc0,
    0x65c1, 0x59b3, 0x4bd3, 0x3c6a, 0x2bc7, 0x1a41, 0x0833, 0xf5fb, 0xe3f6, 0xd283, 0xc1fc, 0xb2b7, 0xa503, 0x9926, 0x8f60, 0x87e1,
    0x82d2, 0x804c, 0x805d, 0x8303, 0x8833, 0x8fcf, 0x99b2, 0xa5a8, 0xb372, 0xc2c9, 0xd35e, 0xe4da, 0xf6e4, 0x091c, 0x1b26, 0x2ca2,
    0x3d37, 0x4c8e, 0x5a58, 0x664e, 0x7031, 0x77cd, 0x7cfd, 0x7fa3, 0x7fb4, 0x7d2e, 0x781f, 0x70a0, 0x66da, 0x5afd, 0x4d49, 0x3e04,
    0x2d7d, 0x1c0a, 0x0a05, 0xf7cd, 0xe5bf, 0xd439, 0xc396, 0xb42d, 0xa64d, 0x9a3f, 0x9040, 0x8886, 0x8337, 0x806f, 0x803d, 0x82a2,
    0x8791, 0x8ef2, 0x989c, 0xa45f, 0xb1fe, 0xc131, 0xd1aa, 0xe313, 0xf512, 0x074a, 0x195d, 0x2aeb, 0x3b9b, 0x4b16, 0x590b, 0x6533,
    0x6f4d, 0x7726, 0x7c95, 0x7f7d, 0x7fd0, 0x7d8c, 0x78bd, 0x717b, 0x67ed, 0x5c43, 0x4ebb, 0x3f9a, 0x2f30, 0x1dd0, 0x0bd6, 0xf99f,
    0xe788, 0xd5f1, 0xc534, 0xb5a7, 0xa79d, 0x9b5d, 0x9127, 0x8930, 0x83a2, 0x8098, 0x8024, 0x8247, 0x86f6, 0x8e1a, 0x978c, 0xa31c,
    0xb08d, 0xbf9c, 0xcff8, 0xe14d, 0xf341, 0x0578, 0x1792, 0x2932, 0x39fd, 0x499a, 0x57ba, 0x6412, 0x6e64, 0x7678, 0x7c26, 0x7f50,
    0x7fe6, 0x7de4, 0x7955, 0x7250, 0x68fb, 0x5d84, 0x5029, 0x412e, 0x30e0, 0x1f95, 0x0da7, 0xfb71, 0xe953, 0xd7ab, 0xc6d4, 0xb725,
    0xa8f1, 0x9c80, 0x9213, 0x89e1, 0x8413, 0x80c9, 0x8012, 0x81f3, 0x8662, 0x8d48, 0x9681, 0xa1dd, 0xaf22, 0xbe0a, 0xce48, 0xdf89,
    0xf171, 0x03a6, 0x15c7, 0x2777, 0x385b, 0x481a, 0x5664, 0x62ed, 0x6d74, 0x75c4, 0x7bb2, 0x7f1d, 0x7ff5, 0x7e35, 0x79e6, 0x731f,
    0x6a03, 0x5ec1, 0x5193, 0x42be, 0x328f, 0x2159, 0x0f77, 0xfd44, 0xeb1f, 0xd967, 0xc877, 0xb8a7, 0xaa49, 0x9da8, 0x9305, 0x8a98,
    0x848b, 0x80ff, 0x8006, 0x81a5, 0x85d3, 0x8c7c, 0x957b, 0xa0a3, 0xadba, 0xbc7b, 0xcc9b, 0xddc6, 0xefa2, 0x01d3, 0x13fa, 0x25ba,
    0x36b6, 0x4697, 0x5509, 0x61c2, 0x6c80, 0x750b, 0x7b36, 0x7ee3, 0x7ffd, 0x7e7f, 0x7a71, 0x73e8, 0x6b06, 0x5ff8, 0x52f8, 0x444a,
    0x343a, 0x231b, 0x1146, 0xff17, 0xecec, 0xdb25, 0xca1d, 0xba2c, 0xaba6, 0x9ed6, 0x93fd, 0x8b55, 0x850a, 0x813d, 0x8001, 0x815e,
    0x854b, 0x8bb5, 0x947b, 0x9f6e, 0xac56, 0xbaf1, 0xcaf1, 0xdc05, 0xedd3
};

static u16 tx_s_cnt = 0;
int get_sine_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 441) {
            *s_cnt = 0;
        }
        *data++ = test_sin_441k[*s_cnt];
        if (ch == 2) {
            *data++ = test_sin_441k[*s_cnt];
        }
        (*s_cnt)++;
    }
    return 0;
}


static u32 sine_send_count = 0;
static u8 sine_send_flag = 0;

static struct __adapter_adc_mic *adapter_adc_mic = NULL;

#define     MY_IO_DEBUG_0(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT &= ~BIT(x);}
#define     MY_IO_DEBUG_1(i,x)       {JL_PORT##i->DIR &= ~BIT(x), JL_PORT##i->OUT |= BIT(x);}

static void adapter_adc_mic_output(void *priv, s16 *data, int len)
{
    if (adapter_adc_mic == NULL || adapter_adc_mic->start == 0) {
        return ;
    }
    int olen;
#if (TCFG_WIRELESS_MIC_STEREO_EN)
    olen = len * 2;
#else
    olen = len;
#endif
    //printf("len = %d,olen = %d\n", len, olen);

    if (adapter_adc_mic->skip_cnt) {
        adapter_adc_mic->skip_cnt--;
        memset(data, 0, olen);
    }
    //memset(data, 0, olen);
    //putchar('A');
#if 0

    sine_send_count ++;
    if (sine_send_count >= 200) {
        sine_send_count = 0;
        sine_send_flag = !sine_send_flag;
    }

    if (sine_send_flag) {
        JL_PORTA->DIR &= ~BIT(4);
        JL_PORTA->OUT |= BIT(4);
        get_sine_data(&tx_s_cnt, data, olen / 2, 1);
    } else {
        JL_PORTA->DIR &= ~BIT(4);
        JL_PORTA->OUT &= ~BIT(4);
    }
#else
    //get_sine_data(&tx_s_cnt, data, olen / 2, 1);
#endif

    if (adapter_adc_mic->data_callback) {
        adapter_adc_mic->data_callback(adapter_adc_mic->data_callback_priv, (u8 *)data, olen);
    } else {
        int wlen = cbuf_write(&adapter_adc_mic->cbuf, data, olen);
        if (wlen != olen) {
            //putchar('M');
        }

        if (adapter_adc_mic->resume) {
            adapter_adc_mic->resume(adapter_adc_mic->resume_priv);
        }
    }
}

int adapter_adc_mic_data_read(void *priv, void *data, int len)
{
    if (adapter_adc_mic) {
        int rlen = adapter_adc_irq_points << 1;
        return cbuf_read(&adapter_adc_mic->cbuf, data, rlen);
    }
    return 0;
}

int adapter_adc_mic_open(u16 sample_rate, u8 gain, u16 irq_points)
{
#if	(TCFG_WIRELESS_MIC_STEREO_EN)
    gain = 0;//如果是linein gain只能为0 或 1， 并且前级增益为0db
#endif
    u32 buf_size = BUF_ALIN(sizeof(struct __adapter_adc_mic), 4)
                   + BUF_ALIN(irq_points * ADAPTER_ADC_MIC_BUF_NUM * ADAPTER_ADC_MIC_ADC_CH * 2, 4)
                   + BUF_ALIN(irq_points * 2 * 3, 4);
    u32 offset = 0;
    u8 *buf = zalloc(buf_size);
    struct __adapter_adc_mic *hdl = (struct __adapter_adc_mic *)(buf + offset);
    offset += BUF_ALIN(sizeof(struct __adapter_adc_mic), 4);

    hdl->adc_buf =	(s16 *)(buf + offset);
    offset += BUF_ALIN(irq_points * ADAPTER_ADC_MIC_BUF_NUM * ADAPTER_ADC_MIC_ADC_CH * 2, 4);

    hdl->pcm_buf =	(buf + offset);
    offset += BUF_ALIN(irq_points * 2 * 3, 4);

    if (hdl == NULL) {
        log_e("adapter_adc_mic zalloc fail\n");
        return -1;
    }
    // 打开mic驱动
    audio_adc_mic_open(&hdl->mic_ch, AUDIO_ADC_MIC_CH, &adc_hdl);
    audio_adc_mic_set_gain(&hdl->mic_ch, gain);
#if	(TCFG_WIRELESS_MIC_STEREO_EN)
    audio_mic_0dB_en(1);

    audio_adc_mic1_open(&hdl->mic_ch, AUDIO_ADC_MIC_CH, &adc_hdl);
    audio_adc_mic1_set_gain(&hdl->mic_ch, gain);
    audio_mic1_0dB_en(1);
#endif

    audio_adc_mic_set_sample_rate(&hdl->mic_ch, sample_rate);
    audio_adc_mic_set_buffs(
        &hdl->mic_ch,
        hdl->adc_buf,
        irq_points * 2,
        ADAPTER_ADC_MIC_BUF_NUM
    );
    hdl->adc_output.priv = NULL;
    hdl->adc_output.handler = adapter_adc_mic_output;
    audio_adc_add_output_handler(&adc_hdl, &hdl->adc_output);

    cbuf_init(&hdl->cbuf, hdl->pcm_buf, irq_points * 2 * 3);
    adapter_adc_irq_points = irq_points;
    hdl->skip_cnt = 30;
    hdl->start = 0;

    local_irq_disable();
    adapter_adc_mic = hdl;
    local_irq_enable();

    return 0;
}

void adapter_adc_mic_data_output_resume_register(void (*callback)(void *priv), void *priv)
{
    if (adapter_adc_mic)	{
        adapter_adc_mic->resume = callback;
        adapter_adc_mic->resume_priv = priv;
    }
}

void adapter_adc_mic_data_callback_register(void (*callback)(void *priv, u8 *data, u16 len), void *priv)
{
    if (adapter_adc_mic)	{
        adapter_adc_mic->data_callback = callback;
        adapter_adc_mic->data_callback_priv = priv;
    }
}

void adapter_adc_mic_close(void)
{
    if (adapter_adc_mic) {
        adapter_adc_mic->start = 0;
        audio_adc_mic_close(&adapter_adc_mic->mic_ch);
        audio_adc_del_output_handler(&adc_hdl, &adapter_adc_mic->adc_output);

        local_irq_disable();
        free(adapter_adc_mic);
        adapter_adc_mic = NULL;
        local_irq_enable();
    }
}

void adapter_adc_mic_start(void)
{
    if (adapter_adc_mic)	{
        adapter_adc_mic->start = 1;
        audio_adc_mic_start(&adapter_adc_mic->mic_ch);
    }
}

#endif//TCFG_AUDIO_ADC_ENABLE
#endif//TCFG_WIRELESS_MIC_ENABLE


