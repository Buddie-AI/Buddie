#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "classic/hci_lmp.h"
#include "aec_user.h"
#include "audio_config.h"
#include "app_main.h"
#include "audio_enc.h"
#include "fft_and_pca.h"
#if TCFG_AUDIO_CVP_DUT_ENABLE
#include "audio_cvp_dut.h"
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/
#if TCFG_AUDIO_ANC_ENABLE
#include "audio_anc.h"
#endif
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#include "icsd_adt_app.h"
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
#include "btstack/avctp_user.h"
extern int ais_platform_asr_open(void);
extern void ais_platform_asr_close(void);
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/

extern struct adc_platform_data adc_data;

extern int msbc_encoder_init();
extern int cvsd_encoder_init();

struct audio_adc_hdl adc_hdl;
struct esco_enc_hdl *esco_enc = NULL;
struct audio_encoder_task *encode_task = NULL;

struct esco_enc_hdl {
    struct audio_encoder encoder;
    OS_SEM pcm_frame_sem;
    s16 output_frame[30];               //align 4Bytes
    int pcm_frame[60];
    u8 state;               //align 4Bytes
};

/*
 *mic电源管理
 *设置mic vdd对应port的状态
 */
void audio_mic_pwr_io(u32 gpio, u8 out_en)
{
    gpio_set_die(gpio, 1);
    gpio_set_pull_up(gpio, 0);
    gpio_set_pull_down(gpio, 0);
    gpio_direction_output(gpio, out_en);
}

void audio_mic_pwr_ctl(u8 state)
{
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_GPIO)
    switch (state) {
    case MIC_PWR_INIT:
    case MIC_PWR_OFF:
        /*mic供电IO配置：输出0*/
#if ((TCFG_AUDIO_MIC_PWR_PORT != NO_CONFIG_PORT))
        audio_mic_pwr_io(TCFG_AUDIO_MIC_PWR_PORT, 0);
#endif/*TCFG_AUDIO_MIC_PWR_PORT*/
#if ((TCFG_AUDIO_MIC1_PWR_PORT != NO_CONFIG_PORT))
        audio_mic_pwr_io(TCFG_AUDIO_MIC1_PWR_PORT, 0);
#endif/*TCFG_AUDIO_MIC1_PWR_PORT*/
#if ((TCFG_AUDIO_MIC2_PWR_PORT != NO_CONFIG_PORT))
        audio_mic_pwr_io(TCFG_AUDIO_MIC2_PWR_PORT, 0);
#endif/*TCFG_AUDIO_MIC2_PWR_PORT*/

        /*mic信号输入端口配置：
         *mic0:PA1
         *mic1:PB8*/
#if (TCFG_AUDIO_ADC_MIC_CHA & LADC_CH_MIC_L)
        audio_mic_pwr_io(IO_PORTA_01, 0);//MIC0
#endif/*TCFG_AUDIO_ADC_MIC_CHA*/

#if (TCFG_AUDIO_ADC_MIC_CHA & LADC_CH_MIC_R)
        audio_mic_pwr_io(IO_PORTB_08, 0);//MIC1
#endif/*TCFG_AUDIO_ADC_MIC_CHA*/
        break;

    case MIC_PWR_ON:
        /*mic供电IO配置：输出1*/
#if (TCFG_AUDIO_MIC_PWR_PORT != NO_CONFIG_PORT)
        audio_mic_pwr_io(TCFG_AUDIO_MIC_PWR_PORT, 1);
#endif/*TCFG_AUDIO_MIC_PWR_CTL*/
#if (TCFG_AUDIO_MIC1_PWR_PORT != NO_CONFIG_PORT)
        audio_mic_pwr_io(TCFG_AUDIO_MIC1_PWR_PORT, 1);
#endif/*TCFG_AUDIO_MIC1_PWR_PORT*/
#if (TCFG_AUDIO_MIC2_PWR_PORT != NO_CONFIG_PORT)
        audio_mic_pwr_io(TCFG_AUDIO_MIC2_PWR_PORT, 1);
#endif/*TCFG_AUDIO_MIC2_PWR_PORT*/
        break;

    case MIC_PWR_DOWN:
        break;
    }
#endif/*TCFG_AUDIO_MIC_PWR_CTL*/
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
/* #define ESCO_PLNK_SR 		    16000#<{(|16000/44100/48000|)}># */
#define ESCO_PLNK_SCLK          2400000 /*1M-4M,SCLK/SR需为整数且在1-4096范围*/
#define ESCO_PLNK_CH_NUM 	     1  /*支持的最大通道(max = 2)*/
#define ESCO_PLNK_IRQ_POINTS    256 /*采样中断点数*/
#include "pdm_link.h"
static audio_plnk_t *audio_plnk_mic = NULL;
static void audio_plnk_mic_output(void *buf, u16 len)
{
    s16 *mic0 = (s16 *)buf;
    if (esco_enc) {
#if (defined(TCFG_PHONE_MESSAGE_ENABLE) && (TCFG_PHONE_MESSAGE_ENABLE))
        int ret = phone_message_mic_write(mic0, len << 1);
        if (ret >= 0) {
            esco_enc_resume();
            return ;
        }
#endif/*TCFG_PHONE_MESSAGE_ENABLE*/
        audio_aec_inbuf(mic0, len << 1);
    }
}

static int esco_pdm_mic_en(u8 en, u16 sr, u16 gain)
{
    printf("esco_pdm_mic_en:%d\n", en);
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
        audio_plnk_mic->ch_num = ESCO_PLNK_CH_NUM;
        audio_plnk_mic->sr = sr;
        audio_plnk_mic->buf_len = ESCO_PLNK_IRQ_POINTS;
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

/*
*********************************************************************
*                  IIS MIC API
* Description: iis mic操作接口
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
static void audio_iis_mic_output(s16 *data, u16 len)
{

}
static int esco_iis_mic_en(u8 en, u16 sr, u16 gain)
{
    if (en) {

    } else {

    }
    return 0;
}

/*
*********************************************************************
*                  ADC MIC API
* Description: adc mic操作接口
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
#define ESCO_ADC_BUF_NUM        3
#define ESCO_ADC_IRQ_POINTS     256
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
#define ESCO_ADC_CH			    3	//3mic通话
#elif TCFG_AUDIO_DUAL_MIC_ENABLE
#define ESCO_ADC_CH			    2	//双mic通话
#else
#define ESCO_ADC_CH			    1	//单mic通话
#endif
#if defined(TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#define ESCO_ADC_BUFS_SIZE      (ESCO_ADC_BUF_NUM * ESCO_ADC_IRQ_POINTS * ESCO_ADC_CH * 3)
#else
#define ESCO_ADC_BUFS_SIZE      (ESCO_ADC_BUF_NUM * ESCO_ADC_IRQ_POINTS * ESCO_ADC_CH)
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/
struct esco_mic_hdl {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    /* s16 adc_buf[ESCO_ADC_BUFS_SIZE];    //align 2Bytes */
#if (ESCO_ADC_CH > 1)
    s16 tmp_buf[ESCO_ADC_IRQ_POINTS];
#endif/*ESCO_ADC_CH*/
#if (ESCO_ADC_CH > 2)
    s16 tmp_buf_1[ESCO_ADC_IRQ_POINTS];
#endif/*ESCO_ADC_CH*/
};
s16 esco_adc_buf[ESCO_ADC_BUFS_SIZE];    //align 2Bytes
static struct esco_mic_hdl *esco_mic = NULL;
static void adc_mic_output_handler(void *priv, s16 *data, int len)
{
    //printf("buf:%x,data:%x,len:%d",esco_enc->adc_buf,data,len);
    if (esco_mic) {

#if defined(TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        if (adc_data.anc_adt_en) {
            u8 esco_mic_num = audio_adc_file_get_esco_mic_num();
            u8 adt_mic_num = get_icsd_adt_mic_num();
            /* 1mic通话，2mic/3mic免摘
             * 2mic通话，3mic免摘*/
            if (esco_mic_num < adt_mic_num) {
                for (int i = 0; i < len / 2; i++) {
                    memcpy(&data[esco_mic_num * i], &data[adt_mic_num * i], esco_mic_num * sizeof(short));
                }
            }
        }
#endif /*TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

#if (defined(TCFG_PHONE_MESSAGE_ENABLE) && (TCFG_PHONE_MESSAGE_ENABLE))
        int ret = phone_message_mic_write(data, len);
        if (ret >= 0) {
            esco_enc_resume();
            return ;
        }
#endif/*TCFG_PHONE_MESSAGE_ENABLE*/

#if TCFG_AUDIO_CVP_DUT_ENABLE
        if (cvp_dut_mode_get() == CVP_DUT_MODE_BYPASS) {
            audio_aec_inbuf(data, len);
            return;
        }
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/

#if (ESCO_ADC_CH == 3)/*3 Mic*/
        s16 *mic_data[3];
        mic_data[0] = data;
        mic_data[1] = data + (len / 2);
        mic_data[2] = data + (len / 2) * 2;

        /*拆分mic数据*/
        s16 *mic1_data_pos = esco_mic->tmp_buf;
        s16 *mic2_data_pos = esco_mic->tmp_buf_1;
        //printf("mic_data:%x,%x,%d\n",data,mic1_data_pos,len);
        for (u16 i = 0; i < (len >> 1); i++) {
            mic_data[0][i] = data[i * 3];
            mic1_data_pos[i] = data[i * 3 + 1];
            mic2_data_pos[i] = data[i * 3 + 2];
        }
        memcpy(mic_data[1], mic1_data_pos, len);
        memcpy(mic_data[2], mic2_data_pos, len);

        u8 mic_ch = TCFG_AUDIO_ADC_MIC_CHA;
        u8 cnt = 0;
        u8 talk_data_num = 0;//记录通话MIC数据位置
        for (int i = 0; i < 4; i++) {
            if ((mic_ch & BIT(i)) == app_var.talk_fb_mic_ch) {
                audio_aec_inbuf_ref_1(mic_data[cnt++], len);
                continue;
            }
            if ((mic_ch & BIT(i)) == app_var.talk_mic_ch) {
                talk_data_num = cnt++;
                continue;
            }
            if ((mic_ch & BIT(i)) == app_var.talk_ref_mic_ch) {
                audio_aec_inbuf_ref(mic_data[cnt++], len);
                continue;
            }
        }
        /*通话MIC数据需要最后传进算法*/
        audio_aec_inbuf(mic_data[talk_data_num], len);
        return;

#elif (ESCO_ADC_CH == 2)/*DualMic*/
        s16 *mic0_data = data;
        s16 *mic1_data = esco_mic->tmp_buf;
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
#else
#if (TCFG_AUDIO_DMS_MIC_MANAGE == DMS_MASTER_MIC0)
        audio_aec_inbuf_ref(mic1_data_pos, len);
        audio_aec_inbuf(data, len);
#else
        audio_aec_inbuf_ref(data, len);
        audio_aec_inbuf(mic1_data_pos, len);
#endif/*TCFG_AUDIO_DMS_MIC_MANAGE*/
#endif/*debug end*/
#else/*SingleMic*/
        mic_insert_data(data, len >> 1);

        audio_aec_inbuf(data, len);
#endif/*ESCO_ADC_CH*/
    }
}

static int esco_mic_en(u8 en, u16 sr, u16 mic0_gain, u16 mic1_gain, u16 mic2_gain, u16 mic3_gain)
{
    u8 mic_ch;
    printf("esco_mic_en:%d\n", en);
    if (en) {
        if (esco_mic) {
            printf("esco_mic re-malloc error\n");
            return -1;
        }
        esco_mic = zalloc(sizeof(struct esco_mic_hdl));
        if (esco_mic == NULL) {
            printf("esco mic zalloc failed\n");
            return -1;
        }
        audio_mic_pwr_ctl(MIC_PWR_ON);
#if TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_DYNAMIC_ADC_GAIN
        anc_dynamic_micgain_start(get_master_mic_gain(1));
#endif/*TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_DYNAMIC_ADC_GAIN*/
#if (TCFG_AUDIO_CVP_DUT_ENABLE && TCFG_AUDIO_ANC_ENABLE)
        if (cvp_dut_mode_get() == CVP_DUT_MODE_BYPASS) {
            //使用ANC MIC增益 测试ANC MIC频响
            mic0_gain = audio_anc_mic_gain_get(0);
            mic1_gain = audio_anc_mic_gain_get(1);
            mic2_gain = audio_anc_mic_gain_get(2);
            mic3_gain = audio_anc_mic_gain_get(3);
            mic_ch = cvp_dut_mic_ch_get();
        } else {
            mic_ch = TCFG_AUDIO_ADC_MIC_CHA;
        }
#else
        mic_ch = TCFG_AUDIO_ADC_MIC_CHA;
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/

        if (mic_ch & AUDIO_ADC_MIC_0) {
            audio_adc_mic_open(&esco_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic_set_gain(&esco_mic->mic_ch, mic0_gain);
        }
        if (mic_ch & AUDIO_ADC_MIC_1) {
            audio_adc_mic1_open(&esco_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic1_set_gain(&esco_mic->mic_ch, mic1_gain);
        }
        if (mic_ch & AUDIO_ADC_MIC_2) {
            audio_adc_mic2_open(&esco_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic2_set_gain(&esco_mic->mic_ch, mic2_gain);
        }
        if (mic_ch & AUDIO_ADC_MIC_3) {
            audio_adc_mic3_open(&esco_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic3_set_gain(&esco_mic->mic_ch, mic3_gain);
        }

        audio_adc_mic_set_sample_rate(&esco_mic->mic_ch, sr);
        audio_adc_mic_set_buffs(&esco_mic->mic_ch, esco_adc_buf,
                                ESCO_ADC_IRQ_POINTS * 2, ESCO_ADC_BUF_NUM);
        esco_mic->adc_output.handler = adc_mic_output_handler;
        audio_adc_add_output_handler(&adc_hdl, &esco_mic->adc_output);

        audio_adc_mic_start(&esco_mic->mic_ch);
    } else {
        if (esco_mic) {
#if TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_DYNAMIC_ADC_GAIN
            anc_dynamic_micgain_stop();
#endif/*TCFG_AUDIO_ANC_ENABLE && TCFG_AUDIO_DYNAMIC_ADC_GAIN*/
            audio_adc_mic_close(&esco_mic->mic_ch);
            audio_adc_del_output_handler(&adc_hdl, &esco_mic->adc_output);
#if TCFG_AUDIO_ANC_ENABLE
            //printf("anc_status:%d\n",anc_status_get());
            if (anc_status_get() == 0) {
                audio_mic_pwr_ctl(MIC_PWR_OFF);
            }
#else
            audio_mic_pwr_ctl(MIC_PWR_OFF);
#endif/*TCFG_AUDIO_ANC_ENABLE*/
            free(esco_mic);
            esco_mic = NULL;
        }
    }
    return 0;
}

/*按顺序给用到的1/2/3个麦赋值*/
void get_config_mic_gain(u8 mic_gain0, u8 mic_gain1, u8 mic_gain2)
{
    u8 mic_ch = TCFG_AUDIO_ADC_MIC_CHA;
    u8 temp = 0, i = 0;
    u8 gain[4];
    for (i = 0; i < 4; i++) {
        temp = mic_ch & 0x01;
        if (temp && mic_gain0) {
            gain[i] = mic_gain0;
            mic_gain0 = 0;
        } else if (temp && mic_gain1) {
            gain[i] = mic_gain1;
            mic_gain1 = 0;
        } else if (temp && mic_gain2) {
            gain[i] = mic_gain2;
            mic_gain2 = 0;
        } else {
            gain[i] = 0;
        }
        mic_ch >>= 1;
    }
    app_var.aec_mic_gain = gain[0];
    app_var.aec_mic1_gain = gain[1];
    app_var.aec_mic2_gain = gain[2];
    app_var.aec_mic3_gain = gain[3];
}

/*
*********************************************************************
*                  get_master_mic_gain
* Description: 获取双麦时主麦或副麦增益
* Arguments  : master, 1:获取主麦增益，0:获取副麦增益
* Return	 : 获取到的mic增益
* Note(s)    : None.
*********************************************************************
*/
u8 get_master_mic_gain(u8 master)
{
    u8 gain[4], i = 0, cnt = 0;
    gain[0] = app_var.aec_mic_gain;
    gain[1] = app_var.aec_mic1_gain;
    gain[2] = app_var.aec_mic2_gain;
    gain[3] = app_var.aec_mic3_gain;
    for (i = 0; i < 4; i++) {
        if (gain[i]) {
            cnt++;
#if (TCFG_AUDIO_DMS_MIC_MANAGE == DMS_MASTER_MIC0)/*第一个麦是主麦时*/
            if (master && cnt == 1) {
                /*返回第一个使用的麦的增益*/
                return gain[i];
            } else if (!master && cnt == 2) {
                return gain[i];
            }
#else /*第二个麦是主麦时*/
            if (master && cnt == 2) {
                /*返回第二个使用的麦的增益*/
                return gain[i];
            } else if (!master && cnt == 1) {
                return gain[i];
            }
#endif
        }
    }
    return 0;
}

/*用于ANC打开ADC数字时，初始化ADC的buffer的地址和长度*/
void esco_adc_mic_set_buffs(void)
{
    audio_adc_mic_set_buffs(NULL, esco_adc_buf,
                            ESCO_ADC_IRQ_POINTS * 2, ESCO_ADC_BUF_NUM);
}

__attribute__((weak)) int audio_aec_output_read(s16 *buf, u16 len)
{
    return 0;
}

void esco_enc_resume(void)
{
    if (esco_enc) {
        audio_encoder_resume(&esco_enc->encoder);
    }
}

static int esco_enc_pcm_get(struct audio_encoder *encoder, s16 **frame, u16 frame_len)
{
    int rlen = 0;
    if (encoder == NULL) {
        printf("encoder NULL");
    }
    struct esco_enc_hdl *enc = container_of(encoder, struct esco_enc_hdl, encoder);

    if (enc == NULL) {
        printf("enc NULL");
    }

    while (1) {
#if (defined(TCFG_PHONE_MESSAGE_ENABLE) && (TCFG_PHONE_MESSAGE_ENABLE))
        rlen = phone_message_output_read(enc->pcm_frame, frame_len);
        if (rlen == frame_len) {
            break;
        } else if (rlen == 0) {
            return 0;
        }
#endif/*TCFG_PHONE_MESSAGE_ENABLE*/
        rlen = audio_aec_output_read(enc->pcm_frame, frame_len);
        if (rlen == frame_len) {
            /*esco编码读取数据正常*/
            break;
        } else if (rlen == 0) {
            /*esco编码读不到数，返回0*/
            return 0;
        } else {
            /*通话结束，aec已经释放*/
            printf("audio_enc end:%d\n", rlen);
            rlen = 0;
            break;
        }
    }

    *frame = enc->pcm_frame;
    return rlen;
}
static void esco_enc_pcm_put(struct audio_encoder *encoder, s16 *frame)
{
}

static const struct audio_enc_input esco_enc_input = {
    .fget = esco_enc_pcm_get,
    .fput = esco_enc_pcm_put,
};

static int esco_enc_probe_handler(struct audio_encoder *encoder)
{
    return 0;
}

static int esco_enc_output_handler(struct audio_encoder *encoder, u8 *frame, int len)
{
    lmp_private_send_esco_packet(NULL, frame, len);
    //printf("frame:%x,out:%d\n",frame, len);

    return len;
}

const static struct audio_enc_handler esco_enc_handler = {
    .enc_probe = esco_enc_probe_handler,
    .enc_output = esco_enc_output_handler,
};

static void esco_enc_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    printf("esco_enc_event_handler:0x%x,%d\n", argv[0], argv[0]);
    switch (argv[0]) {
    case AUDIO_ENC_EVENT_END:
        puts("AUDIO_ENC_EVENT_END\n");
        break;
    }
}

#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
static u16 aispeech_esco_sample_rate = 16000;
static u16 aispeech_esco_mic0_gain = TALK_MIC0_GAIN;
static u16 aispeech_esco_mic1_gain = FF_MIC1_GAIN;
static u16 aispeech_esco_mic2_gain = FB_MIC2_GAIN;
static u16 aispeech_esco_mic3_gain = OTHER_MIC3_GAIN;

void esco_mic_cfg_set(u16 sr, u16 mic0_gain, u16 mic1_gain, u16 mic2_gain, u16 mic3_gain)
{
    aispeech_esco_sample_rate = sr;
    aispeech_esco_mic0_gain = mic0_gain;
    aispeech_esco_mic1_gain = mic1_gain;
    aispeech_esco_mic2_gain = mic2_gain;
    aispeech_esco_mic3_gain = mic3_gain;
}

void esco_mic_reset(void)
{
    esco_mic_en(1, aispeech_esco_sample_rate,
                aispeech_esco_mic0_gain,
                aispeech_esco_mic1_gain,
                aispeech_esco_mic2_gain,
                aispeech_esco_mic3_gain);
}
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/

int esco_enc_open(u32 coding_type, u8 frame_len)
{
    int err;
    struct audio_fmt fmt;

    printf("esco_enc_open: %d,frame_len:%d\n", coding_type, frame_len);

    fmt.channel = 1;
    fmt.frame_len = frame_len;
    if (coding_type == AUDIO_CODING_MSBC) {
        fmt.sample_rate = 16000;
        fmt.coding_type = AUDIO_CODING_MSBC;
    } else if (coding_type == AUDIO_CODING_CVSD) {
        fmt.sample_rate = 8000;
        fmt.coding_type = AUDIO_CODING_CVSD;
    } else {
        /*Unsupoport eSCO Air Mode*/
    }


    if (!encode_task) {
        encode_task = zalloc(sizeof(*encode_task));
        audio_encoder_task_create(encode_task, "audio_enc");
    }
    if (!esco_enc) {
        esco_enc = zalloc(sizeof(*esco_enc));
    }

    os_sem_create(&esco_enc->pcm_frame_sem, 0);

    audio_encoder_open(&esco_enc->encoder, &esco_enc_input, encode_task);
    audio_encoder_set_handler(&esco_enc->encoder, &esco_enc_handler);
    audio_encoder_set_fmt(&esco_enc->encoder, &fmt);
    audio_encoder_set_event_handler(&esco_enc->encoder, esco_enc_event_handler, 0);
    audio_encoder_set_output_buffs(&esco_enc->encoder, esco_enc->output_frame,
                                   sizeof(esco_enc->output_frame), 1);

    if (!esco_enc->encoder.enc_priv) {
        log_e("encoder err, maybe coding(0x%x) disable \n", fmt.coding_type);
        err = -EINVAL;
        goto __err;
    }

    audio_encoder_start(&esco_enc->encoder);

#ifdef CONFIG_BOARD_AISPEECH_NR_GAIN
    app_var.aec_mic_gain = TALK_MIC0_GAIN;
#endif/*CONFIG_BOARD_AISPEECH_NR_GAIN*/

#if TCFG_AUDIO_ANC_ENABLE && (!TCFG_AUDIO_DYNAMIC_ADC_GAIN)
    app_var.aec_mic_gain = audio_anc_ffmic_gain_get();
#endif/*TCFG_AUDIO_ANC_ENABLE && (!TCFG_AUDIO_DYNAMIC_ADC_GAIN)*/

    printf("esco sample_rate: %d,mic_gain:%d %d %d %d\n", fmt.sample_rate, app_var.aec_mic_gain, \
           app_var.aec_mic1_gain, app_var.aec_mic2_gain, app_var.aec_mic3_gain);

    /*pdm mic*/
#if (TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC)
#if TCFG_AUDIO_CVP_DUT_ENABLE
    if (cvp_dut_mode_get() == CVP_DUT_MODE_BYPASS) {
        esco_mic_en(1, fmt.sample_rate, app_var.aec_mic_gain, app_var.aec_mic1_gain, app_var.aec_mic2_gain, app_var.aec_mic3_gain);
    } else
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/
    {
        esco_pdm_mic_en(1, fmt.sample_rate, app_var.aec_mic_gain);
    }
    return 0;
#endif/*TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC*/

    /*iis mic*/
#if (TCFG_AUDIO_ADC_MIC_CHA == ALNK_MIC)
    esco_iis_mic_en(1, fmt.sample_rate, app_var.aec_mic_gain);
    return 0;
#endif/*TCFG_AUDIO_ADC_MIC_CHA == ALNK_MIC*/

#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
    if (get_call_status() != BT_CALL_INCOMING) {
        ais_platform_asr_close();
        /*adc mic*/
        /* esco_mic_en(1, fmt.sample_rate, app_var.aec_mic_gain, app_var.aec_mic_gain, 1, app_var.aec_mic_gain); */
        esco_mic_en(1, fmt.sample_rate, TALK_MIC0_GAIN, FF_MIC1_GAIN, FB_MIC2_GAIN, OTHER_MIC3_GAIN);
    }
    esco_mic_cfg_set(fmt.sample_rate, TALK_MIC0_GAIN, FF_MIC1_GAIN, FB_MIC2_GAIN, OTHER_MIC3_GAIN);
    /* esco_mic_cfg_set(fmt.sample_rate, app_var.aec_mic_gain, app_var.aec_mic_gain, 1, app_var.aec_mic_gain); */
#elif (defined CONFIG_BOARD_AISPEECH_NR)
    esco_mic_en(1, fmt.sample_rate, TALK_MIC0_GAIN, FF_MIC1_GAIN, FB_MIC2_GAIN, OTHER_MIC3_GAIN);
#else /*JL自研算法*/
    /*adc mic*/
    esco_mic_en(1, fmt.sample_rate, app_var.aec_mic_gain, app_var.aec_mic1_gain, app_var.aec_mic2_gain, app_var.aec_mic3_gain);
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/

    esco_enc->state = 1;
    return 0;
__err:
    audio_encoder_close(&esco_enc->encoder);

#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
    ais_platform_asr_open();
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/

    local_irq_disable();
    free(esco_enc);
    esco_enc = NULL;
    local_irq_enable();

    return err;
}

int esco_adc_mic_en()
{
    if (esco_enc && esco_enc->state) {
        /* audio_adc_mic_start(&esco_enc->mic_ch); */
        /* audio_mic_start(&esco_enc->mic_ch); */
        return 0;
    }
    return -1;
}

void esco_enc_close()
{
    printf("esco_enc_close\n");
    if (!esco_enc) {
        printf("esco_enc NULL\n");
        return;
    }
    esco_enc->state = 0;
    os_sem_post(&esco_enc->pcm_frame_sem);
#if (TCFG_AUDIO_ADC_MIC_CHA == PLNK_MIC)
#if TCFG_AUDIO_CVP_DUT_ENABLE
    if (cvp_dut_mode_get() == CVP_DUT_MODE_BYPASS) {
        esco_mic_en(0, 0, 0, 0, 0, 0);
    } else
#endif /*TCFG_AUDIO_CVP_DUT_ENABLE*/
    {
        esco_pdm_mic_en(0, 0, 0);
    }
#elif (TCFG_AUDIO_ADC_MIC_CHA == ALNK_MIC)
    esco_iis_mic_en(0, 0, 0);
#else
    esco_mic_en(0, 0, 0, 0, 0, 0);
#endif/*TCFG_AUDIO_ADC_MIC_CH*/
    audio_encoder_close(&esco_enc->encoder);


    free(esco_enc);
    esco_enc = NULL;
    if (encode_task) {
        audio_encoder_task_del(encode_task);
        free(encode_task);
        encode_task = NULL;
    }
#ifdef CONFIG_BOARD_AISPEECH_VAD_ASR
    ais_platform_asr_open();
#endif /*CONFIG_BOARD_AISPEECH_VAD_ASR*/
}

int esco_enc_mic_gain_set(u8 gain)
{
    app_var.aec_mic_gain = gain;
    if (esco_mic) {
        printf("esco mic 0 set gain %d\n", app_var.aec_mic_gain);
        audio_adc_mic_set_gain(&esco_mic->mic_ch, app_var.aec_mic_gain);
    }
    return 0;
}
int esco_enc_mic1_gain_set(u8 gain)
{
    app_var.aec_mic_gain = gain;
    if (esco_mic) {
        printf("esco mic 1 set gain %d\n", app_var.aec_mic_gain);
        audio_adc_mic1_set_gain(&esco_mic->mic_ch, app_var.aec_mic_gain);
    }
    return 0;
}
int esco_enc_mic2_gain_set(u8 gain)
{
    app_var.aec_mic_gain = gain;
    if (esco_mic) {
        printf("esco mic 2 set gain %d\n", app_var.aec_mic_gain);
        audio_adc_mic2_set_gain(&esco_mic->mic_ch, app_var.aec_mic_gain);
    }
    return 0;
}
int esco_enc_mic3_gain_set(u8 gain)
{
    app_var.aec_mic_gain = gain;
    if (esco_mic) {
        printf("esco mic 3 set gain %d\n", app_var.aec_mic_gain);
        audio_adc_mic3_set_gain(&esco_mic->mic_ch, app_var.aec_mic_gain);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
#if (defined(TCFG_PCM_ENC2TWS_ENABLE) && (TCFG_PCM_ENC2TWS_ENABLE))

#define PCM_ENC2TWS_OUTBUF_LEN				(4 * 1024)
struct pcm2tws_enc_hdl {
    struct audio_encoder encoder;
    OS_SEM pcm_frame_sem;
    s16 output_frame[30];               //align 4Bytes
    int pcm_frame[60];                 //align 4Bytes
    u8 output_buf[PCM_ENC2TWS_OUTBUF_LEN];
    cbuffer_t output_cbuf;
    void (*resume)(void);
    u32 status : 3;
    u32 reserved: 29;
};
struct pcm2tws_enc_hdl *pcm2tws_enc = NULL;

extern void local_tws_start(u32 coding_type, u32 rate);
extern void local_tws_stop(void);
extern int local_tws_resolve(u32 coding_type, u32 rate);
extern int local_tws_output(s16 *data, int len);

void pcm2tws_enc_close();
void pcm2tws_enc_resume(void);

int pcm2tws_enc_output(void *priv, s16 *data, int len)
{
    if (!pcm2tws_enc) {
        return 0;
    }
    u16 wlen = cbuf_write(&pcm2tws_enc->output_cbuf, data, len);
    os_sem_post(&pcm2tws_enc->pcm_frame_sem);
    if (!wlen) {
        /* putchar(','); */
    }
    /* printf("wl:%d ", wlen); */
    pcm2tws_enc_resume();
    return wlen;
}

void pcm2tws_enc_set_resume_handler(void (*resume)(void))
{
    if (pcm2tws_enc) {
        pcm2tws_enc->resume = resume;
    }
}

static void pcm2tws_enc_need_data(void)
{
    if (pcm2tws_enc && pcm2tws_enc->resume) {
        pcm2tws_enc->resume();
    }
}

static int pcm2tws_enc_pcm_get(struct audio_encoder *encoder, s16 **frame, u16 frame_len)
{
    int rlen = 0;
    if (encoder == NULL) {
        r_printf("encoder NULL");
    }
    struct pcm2tws_enc_hdl *enc = container_of(encoder, struct pcm2tws_enc_hdl, encoder);

    if (enc == NULL) {
        r_printf("enc NULL");
    }
    os_sem_set(&pcm2tws_enc->pcm_frame_sem, 0);
    /* printf("l:%d", frame_len); */

    do {
        rlen = cbuf_read(&pcm2tws_enc->output_cbuf, enc->pcm_frame, frame_len);
        if (rlen == frame_len) {
            break;
        }
        if (rlen == -EINVAL) {
            return 0;
        }
        if (!pcm2tws_enc->status) {
            return 0;
        }
        pcm2tws_enc_need_data();
        os_sem_pend(&pcm2tws_enc->pcm_frame_sem, 2);
    } while (1);

    *frame = enc->pcm_frame;
    return rlen;
}
static void pcm2tws_enc_pcm_put(struct audio_encoder *encoder, s16 *frame)
{
}

static const struct audio_enc_input pcm2tws_enc_input = {
    .fget = pcm2tws_enc_pcm_get,
    .fput = pcm2tws_enc_pcm_put,
};

static int pcm2tws_enc_probe_handler(struct audio_encoder *encoder)
{
    return 0;
}

static int pcm2tws_enc_output_handler(struct audio_encoder *encoder, u8 *frame, int len)
{
    struct pcm2tws_enc_hdl *enc = container_of(encoder, struct pcm2tws_enc_hdl, encoder);
    local_tws_resolve(encoder->fmt.coding_type, encoder->fmt.sample_rate | (encoder->fmt.channel << 16));
    int ret = local_tws_output(frame, len);
    if (!ret) {
        /* putchar('L'); */
    } else {
        /* printf("w:%d ", len);	 */
    }
    return ret;
}

const static struct audio_enc_handler pcm2tws_enc_handler = {
    .enc_probe = pcm2tws_enc_probe_handler,
    .enc_output = pcm2tws_enc_output_handler,
};



int pcm2tws_enc_open(u32 codec_type, u32 info)
{
    int err;
    struct audio_fmt fmt;
    u16 rate = info & 0x0000ffff;
    u16 channel = (info >> 16) & 0x0f;

    printf("pcm2tws_enc_open: %d\n", codec_type);

    fmt.channel = channel;
    fmt.sample_rate = rate;
    fmt.coding_type = codec_type;

    if (!encode_task) {
        encode_task = zalloc(sizeof(*encode_task));
        audio_encoder_task_create(encode_task, "audio_enc");
    }
    if (pcm2tws_enc) {
        pcm2tws_enc_close();
    }
    pcm2tws_enc = zalloc(sizeof(*pcm2tws_enc));

    os_sem_create(&pcm2tws_enc->pcm_frame_sem, 0);

    cbuf_init(&pcm2tws_enc->output_cbuf, pcm2tws_enc->output_buf, PCM_ENC2TWS_OUTBUF_LEN);

    audio_encoder_open(&pcm2tws_enc->encoder, &pcm2tws_enc_input, encode_task);
    audio_encoder_set_handler(&pcm2tws_enc->encoder, &pcm2tws_enc_handler);
    audio_encoder_set_fmt(&pcm2tws_enc->encoder, &fmt);
    audio_encoder_set_output_buffs(&pcm2tws_enc->encoder, pcm2tws_enc->output_frame,
                                   sizeof(pcm2tws_enc->output_frame), 1);

    if (!pcm2tws_enc->encoder.enc_priv) {
        log_e("encoder err, maybe coding(0x%x) disable \n", fmt.coding_type);
        err = -EINVAL;
        goto __err;
    }

    local_tws_start(pcm2tws_enc->encoder.fmt.coding_type, pcm2tws_enc->encoder.fmt.sample_rate | (pcm2tws_enc->encoder.fmt.channel << 16));

    pcm2tws_enc->status = 1;

    audio_encoder_start(&pcm2tws_enc->encoder);

    printf("sample_rate: %d\n", fmt.sample_rate);

    return 0;
__err:
    audio_encoder_close(&pcm2tws_enc->encoder);

    local_irq_disable();
    free(pcm2tws_enc);
    pcm2tws_enc = NULL;
    local_irq_enable();

    return err;
}

void pcm2tws_enc_close()
{
    if (!pcm2tws_enc) {
        return;
    }
    pcm2tws_enc->status = 0;
    printf("pcm2tws_enc_close");
    local_tws_stop();
    audio_encoder_close(&pcm2tws_enc->encoder);
    free(pcm2tws_enc);
    pcm2tws_enc = NULL;
    if (encode_task) {
        audio_encoder_task_del(encode_task);
        free(encode_task);
        encode_task = NULL;
    }
}

void pcm2tws_enc_resume(void)
{
    if (pcm2tws_enc && pcm2tws_enc->status) {
        audio_encoder_resume(&pcm2tws_enc->encoder);
    }
}

#endif

//////////////////////////////////////////////////////////////////////////////
int audio_enc_init()
{
    printf("audio_enc_init\n");

    audio_adc_init(&adc_hdl, &adc_data);
#if TCFG_AUDIO_CVP_DUT_ENABLE
    cvp_dut_init();
#endif /*TCFG_AUDIO_CVP_DUT_ENABLE*/
    return 0;
}

