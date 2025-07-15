/*
 ****************************************************************************
 *							Audio Capture Module(AC)
 *
 *
 ****************************************************************************
 */

#include "audio_capture.h"
#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "tone_player.h"
#include "app_main.h"
#include "user_cfg.h"
#include "online_db_deal.h"
#include "audio_enc.h"
#if TCFG_SMART_VOICE_ENABLE
#include "smart_voice/smart_voice.h"
#endif

extern int spp_data_export(u8 ch, u8 *buf, u16 len);
extern int aec_data_export_init(u8 ch);

#if TCFG_AUDIO_DATA_EXPORT_ENABLE

#define AC_LOG_ENABLE
#ifdef AC_LOG_ENABLE
#define AC_LOG	y_printf
#define AC_ERR_LOG	r_printf
#else
#define AC_LOG(...)
#define AC_ERR_LOG(...)
#endif/*AC_LOG_ENABLE*/

extern struct audio_dac_hdl dac_hdl;
extern struct audio_adc_hdl adc_hdl;

#define AudioCapture_CLK			(128 * 1000000L)

#define AC_A_MIC			1	//2个模拟mic(Analog)
#define AC_D_MIC			2	//2个PDM mic(Digital)
#define AC_AD_MIC			3	//一个模拟mic、一个PDM mic
#define AC_VAD_MIC          4   //VAD模拟mic
#define AC_MIC_TYPE			AC_A_MIC
#define AC_SAMPLE_RATE		16000 //AudioCapture Sample Rate

/**********************LED INDICATE CONFIG*********************/
#define LED_INDICATE_EN		0
#define LED_PORT			JL_PORTC
#define LED_PORT_NUM		0
#define LED_DISPLAY_INIT()	LED_PORT->DIR &= ~BIT(LED_PORT_NUM)
#define LED_DISPLAY_ON()	LED_PORT->OUT |= BIT(LED_PORT_NUM)
#define LED_DISPLAY_OFF()	LED_PORT->OUT &= ~BIT(LED_PORT_NUM)
/***************************************************************/

#define DM_2_DAC_EN			0	//双mic的数据输出到DAC播放(debug)
#define PLNK_MIC_CH			2
#define NS_DEMO_EN			0	//降噪模型测试使能

#define DM_RUN_POINT		256						//DualMic运行frame长(points)
#define DM_RUN_SIZE			(DM_RUN_POINT * 2)		//DualMic运行frame长(bytes)

#define ADC_MIC0_EN			1
#define ADC_MIC1_EN			1
#define ADC_DM_BUF_NUM      2
#if (NS_DEMO_EN || (AC_MIC_TYPE & AC_VAD_MIC))
#define ADC_DM_CH_NUM       1
#else /*使用AC_AD_MIC*/
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
/*开3路ADC*/
#define ADC_DM_CH_NUM       3
#else /*双麦*/
/*开2路ADC*/
#define ADC_DM_CH_NUM       2
#endif /*TCFG_AUDIO_TRIPLE_MIC_ENABLE*/
#endif/*NS_DEMO_EN*/
#define ADC_DM_IRQ_POINTS   256
#define ADC_DM_BUFS_SIZE 	(ADC_DM_BUF_NUM * ADC_DM_IRQ_POINTS * ADC_DM_CH_NUM)

#if (AC_MIC_TYPE == AC_AD_MIC)
#undef PLNK_MIC_CH
#undef ADC_DM_CH_NUM
#undef ADC_DM_BUFS_SIZE
#define PLNK_MIC_CH			1
#define ADC_DM_CH_NUM       1
#define ADC_DM_BUFS_SIZE 	(ADC_DM_BUF_NUM * ADC_DM_IRQ_POINTS * ADC_DM_CH_NUM)
#endif


/******************SPP_DATA_EXPORT_CONFIG****************************/

#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP)
#define SPP_EXPORT_HEAD		4	/*4 bytes seqn*/
#define SPP_EXPORT_DATA_LEN	640 /*数据包长度*/
#define SPP_EXPORT_MTU		(SPP_EXPORT_DATA_LEN + SPP_EXPORT_HEAD)
#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
#define SPP_EXPORT_CH		3	/*导出数据通道数*/
#else /*两个通道*/
#define SPP_EXPORT_CH		2	/*导出数据通道数*/
#endif /*TCFG_AUDIO_TRIPLE_MIC_ENABLE*/
#define SPP_SEND_BY_TIMER	1	/*通过定时器定时发送*/
#if (SPP_EXPORT_CH == 3)
#define SPP_SEND_INTERVAL	4	//ms
#else
#define SPP_SEND_INTERVAL	6	//ms
#endif
#endif
/******************************************************************/

#define SD_START_BLOCK		50000					// SD卡起始扇区
#define SD_PCM_CHANNLE		1						// PCM数据通道
#define SD_PCM_BUF_SIZE		(512 * SD_PCM_CHANNLE)	// 每次写的数据长度
#define DE_HEADER_LEN	 	12						//数据头的长度
#define SD_PCM_BUF_SIZE_H	(SD_PCM_BUF_SIZE - DE_HEADER_LEN)	// 每次写的数据长度(with packet header)

/*数据导出添加数据头*/
#define DE_PACKET_HEADER_EN	0	//DE:DataExport

extern struct device *force_open_sd(char *sdx);
extern void force_write_sd(u8 *buf, u32 sector, u32 sector_num);

/*Audio Capture state*/
enum {
    AC_STATE_INIT,
    AC_STATE_START,
    AC_STATE_STOP,
};

static u16 mic_ch_sw = 0;

#if DE_PACKET_HEADER_EN
#define DE_HEADER_MAGIC		0x5A
typedef struct {
    u8 magic;		//标识符0x5A
    u8 ch;			//通道号:0 1 2 0 1 2
    u16 seqn;		//序列号:0 1 2 3 4 5
    u16 crc;		//校验码
    u16 len;		//数据长度
    u32 total_len;	//数据累加总长
} de_header;
#endif/*DATA_PACKET_HEADER_EN*/

typedef struct {
    u8 state;
    u16 sr;
    OS_SEM sem;
    struct device *sd_hdl;
    s16 mic0_buf[ADC_DM_IRQ_POINTS * 4];	// mic0的原始数据buf
    s16 mic1_buf[ADC_DM_IRQ_POINTS * 4];	// mic1的原始数据buf
    s16 mic2_buf[ADC_DM_IRQ_POINTS * 4];	// mic2的原始数据buf
    //s16 out_buf[ADC_DM_IRQ_POINTS * 4];		// dm noise reduce output
    cbuffer_t mic0_cb;
    cbuffer_t mic1_cb;
    cbuffer_t mic2_cb;
    //cbuffer_t out_cb;
    s16 mic0[DM_RUN_POINT];
    s16 mic1[DM_RUN_POINT];
    s16 mic2[DM_RUN_POINT];
    s16 output[DM_RUN_POINT];
#if (AC_MIC_TYPE & AC_VAD_MIC)
    void *vad_mic;
#endif

#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP)
    u8 export_buf[SPP_EXPORT_MTU];
#endif

#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SD)
    u32 sd_write_sector;
    OS_SEM sd_sem;
    u8 sd_wbuf[SD_PCM_BUF_SIZE];			// 数据导出buf
#if DE_PACKET_HEADER_EN
    s16 ch0_buf[ADC_DM_IRQ_POINTS * 3];
    s16 ch1_buf[ADC_DM_IRQ_POINTS * 3];
    s16 ch2_buf[ADC_DM_IRQ_POINTS * 3];
    cbuffer_t ch0_cb;
    cbuffer_t ch1_cb;
    cbuffer_t ch2_cb;
#endif/*DE_PACKET_HEADER_EN*/
#endif/*TCFG_AUDIO_DATA_EXPORT_ENABLE*/
    s16 out_buf[ADC_DM_IRQ_POINTS * 6];		// dm noise reduce output
    cbuffer_t out_cbuf;
} dual_mic_t;
dual_mic_t dm;

static void led_init()
{
#if LED_INDICATE_EN
    LED_DISPLAY_INIT();
    LED_DISPLAY_ON();
#endif/*LED_INDICATE_EN*/
}

static void led_run(u16 ms)
{
#if LED_INDICATE_EN
    static u16 led_flash_cnt = 2000;
    u16 flash_time = ms / 16;
    if (led_flash_cnt++ > flash_time) {
        LED_DISPLAY_OFF();
        if (led_flash_cnt > (flash_time + 30)) {
            led_flash_cnt = 0;
            LED_DISPLAY_ON();
        }
    }
#endif/*LED_INDICATE_EN*/
}

#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SD)
static int data_export_run(void *dat, u16 len, u8 ch)
{
#if DE_PACKET_HEADER_EN
    int wlen = 0;
    //printf("export_run:%d",ch);
    if (ch == 0) {
        wlen = cbuf_write(&dm.ch0_cb, dat, len);
        if (wlen != len) {
            AC_ERR_LOG("[dm]sd%d buf full\n", ch);
        }
    } else if (ch == 1) {
        wlen = cbuf_write(&dm.ch1_cb, dat, len);
        if (wlen != len) {
            AC_ERR_LOG("[dm]sd%d buf full\n", ch);
        }
    } else {
        wlen = cbuf_write(&dm.ch2_cb, dat, len);
        if (wlen != len) {
            AC_ERR_LOG("[dm]sd%d buf full\n", ch);
        }
    }
    os_sem_set(&dm.sd_sem, 0);
    os_sem_post(&dm.sd_sem);
#else
    int wlen = cbuf_write(&dm.out_cbuf, dat, len);
    if (wlen != len) {
        AC_ERR_LOG("[dm]sd buf full\n");
    }
    if (dm.out_cbuf.data_len >= SD_PCM_BUF_SIZE) {
        os_sem_set(&dm.sd_sem, 0);
        os_sem_post(&dm.sd_sem);
    }
#endif
    return 0;
}

#if DE_PACKET_HEADER_EN
static void DataExport_Task(void *p)
{
    AC_LOG(">>>DataExport_Task[Header]<<<\n");
    u8 pend;
    u8 wch = 0;
    de_header header;
    u16 seqn0 = 0;
    u16 seqn1 = 0;
    u16 seqn2 = 0;
    u32 total_len0 = 0;
    u32 total_len1 = 0;
    u32 total_len2 = 0;
    header.magic = DE_HEADER_MAGIC;
    while (1) {
        pend = 1;
        if (wch == 0) {
            if (dm.ch0_cb.data_len >= SD_PCM_BUF_SIZE_H) {		// 每次必须CBUF有足够的数据长度，才可以写。
                pend = 0;
                cbuf_read(&dm.ch0_cb, &dm.sd_wbuf[DE_HEADER_LEN], SD_PCM_BUF_SIZE_H);
                header.ch = wch;
                header.seqn = seqn0++;
                header.crc = CRC16(&dm.sd_wbuf[DE_HEADER_LEN], SD_PCM_BUF_SIZE_H);
                header.len = SD_PCM_BUF_SIZE_H;
                total_len0 += header.len;
                header.total_len = total_len0;
                memcpy(&dm.sd_wbuf, &header, sizeof(de_header));
                //y_printf("ch:%d,%d,%x,%d",header.ch,header.seqn,header.crc,header.len);
                if (dm.sd_hdl) {
                    putchar('.');
                    led_run(7000);
                    force_write_sd(dm.sd_wbuf, dm.sd_write_sector, SD_PCM_CHANNLE);
                    dm.sd_write_sector += SD_PCM_CHANNLE;
                    /* printf("sd:%d\n",dm.sd_write_sector);		//打印当前写的扇区位置。 */
                }
                wch++;
            }
        } else if (wch == 1) {
            if (dm.ch1_cb.data_len >= SD_PCM_BUF_SIZE_H) {		// 每次必须CBUF有足够的数据长度，才可以写。
                pend = 0;
                cbuf_read(&dm.ch1_cb, &dm.sd_wbuf[DE_HEADER_LEN], SD_PCM_BUF_SIZE_H);
                header.ch = wch;
                header.seqn = seqn1++;
                header.crc = CRC16(&dm.sd_wbuf[DE_HEADER_LEN], SD_PCM_BUF_SIZE_H);
                header.len = SD_PCM_BUF_SIZE_H;
                total_len1 += header.len;
                header.total_len = total_len1;
                memcpy(&dm.sd_wbuf, &header, sizeof(de_header));
                //g_printf("ch:%d,%d,%x,%d",header.ch,header.seqn,header.crc,header.len);
                if (dm.sd_hdl) {
                    putchar('.');
                    led_run(7000);
                    force_write_sd(dm.sd_wbuf, dm.sd_write_sector, SD_PCM_CHANNLE);
                    dm.sd_write_sector += SD_PCM_CHANNLE;
                    /* printf("sd:%d\n",dm.sd_write_sector);		//打印当前写的扇区位置。 */
                }
                wch++;
            }
        } else if (wch == 2) {
            if (dm.ch2_cb.data_len >= SD_PCM_BUF_SIZE_H) {		// 每次必须CBUF有足够的数据长度，才可以写。
                pend = 0;
                cbuf_read(&dm.ch2_cb, &dm.sd_wbuf[DE_HEADER_LEN], SD_PCM_BUF_SIZE_H);
                header.ch = wch;
                header.seqn = seqn2++;
                header.crc = CRC16(&dm.sd_wbuf[DE_HEADER_LEN], SD_PCM_BUF_SIZE_H);
                header.len = SD_PCM_BUF_SIZE_H;
                total_len2 += header.len;
                header.total_len = total_len2;
                memcpy(&dm.sd_wbuf, &header, sizeof(de_header));
                //g_printf("ch:%d,%d,%x,%d",header.ch,header.seqn,header.crc,header.len);
                if (dm.sd_hdl) {
                    putchar('.');
                    led_run(7000);
                    force_write_sd(dm.sd_wbuf, dm.sd_write_sector, SD_PCM_CHANNLE);
                    dm.sd_write_sector += SD_PCM_CHANNLE;
                    /* printf("sd:%d\n",dm.sd_write_sector);		//打印当前写的扇区位置。 */
                }
                wch = 0;
            }
        }
        //printf("wch:%d,pend:%d,len:%d,%d,%d",wch,pend,dm.ch0_cb.data_len,dm.ch1_cb.data_len,dm.ch2_cb.data_len);
        if (pend) {
            os_sem_pend(&dm.sd_sem, 0);
        }
    }
}

#else
static void DataExport_Task(void *p)
{
    AC_LOG(">>>DataExport_Task<<<\n");
    while (1) {
        if (dm.out_cbuf.data_len >= SD_PCM_BUF_SIZE) {		// 每次必须CBUF有足够的数据长度，才可以写。
            cbuf_read(&dm.out_cbuf, dm.sd_wbuf, SD_PCM_BUF_SIZE);
            if (dm.sd_hdl) {
                putchar('.');
                led_run(7000);

                force_write_sd(dm.sd_wbuf, dm.sd_write_sector, SD_PCM_CHANNLE);
                dm.sd_write_sector += SD_PCM_CHANNLE;
                /* printf("sd:%d\n",dm.sd_write_sector);		//打印当前写的扇区位置。 */
            }
        } else {
            os_sem_pend(&dm.sd_sem, 0);
        }
    }
}
#endif

#else
static int data_export_run(void *dat, u16 len, u8 ch)
{
    return 0;
}
#endif/*TCFG_AUDIO_DATA_EXPORT_ENABLE*/

/*
 *数据导出初始化
 */
typedef struct {
    u8 ch_idx;
    u8 retry;
    u16 send_timer;
    u32 seqn0;
    u32 seqn1;
    u32 seqn2;
} data_export_t;
data_export_t de;
int data_export_init()
{
    AC_LOG("[dm]data_export_init\n");
#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SD)
    /*使能板子RT9193，供电给sd卡*/
    JL_PORTB->DIR &= ~BIT(3);
    JL_PORTB->OUT |=  BIT(3);
    dm.sd_hdl = force_open_sd("sd0");
    AC_LOG("sd_hdl:%x\n", dm.sd_hdl);
    //os_time_dly(10);
#if DE_PACKET_HEADER_EN
    cbuf_init(&dm.ch0_cb, dm.ch0_buf, sizeof(dm.ch0_buf));
    cbuf_init(&dm.ch1_cb, dm.ch1_buf, sizeof(dm.ch1_buf));
    cbuf_init(&dm.ch2_cb, dm.ch2_buf, sizeof(dm.ch2_buf));
    AC_LOG("de packet header size:%d\n", sizeof(de_header));
#else
    cbuf_init(&dm.out_cbuf, dm.out_buf, sizeof(dm.out_buf));
#endif/*DE_PACKET_HEADER_EN*/
    dm.sd_write_sector = SD_START_BLOCK;
    os_sem_create(&dm.sd_sem, 0);
    task_create(DataExport_Task, NULL, "data_export");
#endif/*AUDIO_DATA_EXPORT_USE_SD*/

#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP)
    aec_data_export_init(SPP_EXPORT_CH);
#endif/*AUDIO_DATA_EXPORT_USE_SPP*/
    memset(&de, 0, sizeof(data_export_t));
    cbuf_init(&dm.out_cbuf, dm.out_buf, sizeof(dm.out_buf));

    return 0;
}


typedef struct {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    s16 adc_buf[ADC_DM_BUFS_SIZE];    //align 4Bytes
    s16 mic_tmp_data[ADC_DM_IRQ_POINTS];
    s16 mic1_tmp_data[ADC_DM_IRQ_POINTS];
} adc_dm_t;
adc_dm_t *adc_dm = NULL;

void dm_2_dac_open(u16 sr)
{
#if DM_2_DAC_EN
    y_printf("%s,sr = %d\n", sr);
    audio_dac_set_sample_rate(&dac_hdl, sr);
    audio_dac_start(&dac_hdl);
    printf("max_sys_vol:%d\n", get_max_sys_vol());
    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());
    printf("cur_vol:%d\n", app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
    audio_dac_set_volume(&dac_hdl, app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
#endif
}

static void adc_mic_output(void *priv, s16 *data, int len)
{
    /* printf("<%x,%x,%d>",adc_dm->adc_buf,data,len); */
    u16 wlen;
#if (ADC_DM_CH_NUM == 2)
    /*
     *这里先把mic0的数据取出来放在临时buf，mic1的数据就可以
     *放到mic本来的buf了，不用再开一个buf放mic1_data
     */
    s16 *mic0_data = adc_dm->mic_tmp_data;
    s16 *mic1_data = data;
    for (u16 i = 0; i < (len >> 1); i++) {
        mic0_data[i] = data[i * 2];
        mic1_data[i] = data[i * 2 + 1];
    }

    wlen = cbuf_write(&dm.mic0_cb, mic0_data, len);
    if (wlen != len) {
        AC_ERR_LOG("mic0_cbuf full:%d,%d\n", wlen, len);
    }

#if (TCFG_AUDIO_TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP)
#if (SPP_EXPORT_CH > 1)
    wlen = cbuf_write(&dm.mic1_cb, mic1_data, len);
    if (wlen != len) {
        AC_ERR_LOG("mic1_cbuf full\n");
    }
#endif/*SPP_EXPORT_CH == 2*/
#else
    wlen = cbuf_write(&dm.mic1_cb, mic1_data, len);
    if (wlen != len) {
        AC_ERR_LOG("mic1_cbuf full\n");
    }
#endif/*TCFG_AUDIO_TCFG_AUDIO_DATA_EXPORT_ENABLE*/

    os_sem_post(&dm.sem);

#if DM_2_DAC_EN
    if (tone_get_status()) {
        return;
    }
    if (mic_ch_sw++ < 300) {
        putchar('0');
        wlen = audio_dac_write(&dac_hdl, mic0_data, len);
    } else {
        putchar('1');
        wlen = audio_dac_write(&dac_hdl, mic1_data, len);
        if (mic_ch_sw >= 600) {
            mic_ch_sw = 0;
        }
    }
#endif/*DM_2_DAC_EN*/

#elif (ADC_DM_CH_NUM == 3)
    /*
     *这里先把mic0的数据取出来放在临时buf，mic1的数据就可以
     *放到mic本来的buf了，不用再开一个buf放mic1_data
     */
    s16 *mic0_data = data;
    s16 *mic1_data = adc_dm->mic_tmp_data;
    s16 *mic2_data = adc_dm->mic1_tmp_data;
    for (u16 i = 0; i < (len >> 1); i++) {
        mic0_data[i] = data[i * 3];
        mic1_data[i] = data[i * 3 + 1];
        mic2_data[i] = data[i * 3 + 2];
    }

    wlen = cbuf_write(&dm.mic0_cb, mic0_data, len);
    if (wlen != len) {
        AC_ERR_LOG("mic0_cbuf full:%d,%d\n", wlen, len);
    }

    wlen = cbuf_write(&dm.mic1_cb, mic1_data, len);
    if (wlen != len) {
        AC_ERR_LOG("mic1_cbuf full:%d,%d\n", wlen, len);
    }

    wlen = cbuf_write(&dm.mic2_cb, mic2_data, len);
    if (wlen != len) {
        AC_ERR_LOG("mic2_cbuf full:%d,%d\n", wlen, len);
    }
    os_sem_post(&dm.sem);

#if DM_2_DAC_EN
    if (tone_get_status()) {
        return;
    }
    if (mic_ch_sw++ < 300) {
        putchar('0');
        wlen = audio_dac_write(&dac_hdl, mic0_data, len);
    } else if (mic_ch_sw++ < 600) {
        putchar('1');
        wlen = audio_dac_write(&dac_hdl, mic1_data, len);
    } else {
        putchar('2');
        wlen = audio_dac_write(&dac_hdl, mic2_data, len);
        if (mic_ch_sw >= 900) {
            mic_ch_sw = 0;
        }
    }
#endif/*DM_2_DAC_EN*/

#else /*单模拟MIC(ADC_DM_CH_NUM == 1)*/
    //putchar('m');
    wlen = cbuf_write(&dm.mic1_cb, data, len);
    if (wlen != len) {
        AC_ERR_LOG("mic1_cbuf full\n");
    }
    os_sem_post(&dm.sem);
#if (DM_2_DAC_EN && (NS_DEMO_EN == 0))
    if (mic_ch_sw >= 300) {
        mic_ch_sw++;
        putchar('1');
        wlen = audio_dac_write(&dac_hdl, data, len);
        if (mic_ch_sw >= 600) {
            mic_ch_sw = 0;
        }
    }
#endif/*DM_2_DAC_EN*/
#endif/*ADC_DM_CH_NUM*/
}

static u8 dual_mic_idle_query()
{
    return 0;
}

REGISTER_LP_TARGET(dual_mic_lp_target) = {
    .name = "dual_mic",
    .is_idle = dual_mic_idle_query,
};

/*初始化 mic*/
int audio_mic_init(u16 sr)
{
    AC_LOG("[dm]audio_mic_init\n");
    adc_dm = zalloc(sizeof(*adc_dm));
    /*
     *range:0(00000:-8dB)~19(1001:30dB)
     *step:2dB
     */
    u8 mic0_gain = 10;
    u8 mic1_gain = 10;
    u8 mic2_gain = 10;
    u8 mic3_gain = 10;

    //使用通话配置的mic增益，更加直观反映mic的数据采样情况
    mic0_gain = app_var.aec_mic_gain;
    mic1_gain = app_var.aec_mic1_gain;
    mic2_gain = app_var.aec_mic2_gain;
    mic3_gain = app_var.aec_mic3_gain;
    AC_LOG("[AC]mic_gain:%d,%d,%d,%d\n", mic0_gain, mic1_gain, mic2_gain, mic3_gain);

    if (adc_dm) {
#if (TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA & AUDIO_ADC_MIC_0)
        audio_adc_mic_open(&adc_dm->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
        audio_adc_mic_set_gain(&adc_dm->mic_ch, app_var.aec_mic_gain);
#endif/*TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA & AUDIO_ADC_MIC_0*/
#if (TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA & AUDIO_ADC_MIC_1)
        audio_adc_mic1_open(&adc_dm->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
        audio_adc_mic1_set_gain(&adc_dm->mic_ch, app_var.aec_mic1_gain);
#endif/*(TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA & AUDIO_ADC_MIC_1)*/
#if (TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA & AUDIO_ADC_MIC_2)
        audio_adc_mic2_open(&adc_dm->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
        audio_adc_mic2_set_gain(&adc_dm->mic_ch, app_var.aec_mic2_gain);
#endif/*(TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA & AUDIO_ADC_MIC_2)*/
#if (TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA & AUDIO_ADC_MIC_3)
        audio_adc_mic3_open(&adc_dm->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
        audio_adc_mic3_set_gain(&adc_dm->mic_ch, app_var.aec_mic3_gain);
#endif/*(TCFG_SPP_DATA_EXPORT_ADC_MIC_CHA & AUDIO_ADC_MIC_3)*/

        audio_adc_mic_set_sample_rate(&adc_dm->mic_ch, sr);
        audio_adc_mic_set_buffs(&adc_dm->mic_ch, adc_dm->adc_buf, ADC_DM_IRQ_POINTS * 2, ADC_DM_BUF_NUM);
        adc_dm->adc_output.handler = adc_mic_output;
        audio_adc_add_output_handler(&adc_hdl, &adc_dm->adc_output);
        audio_adc_mic_start(&adc_dm->mic_ch);


    }
    return 0;
}

void audio_mic_close(void)
{
    if (adc_dm) {
        audio_adc_mic_close(&adc_dm->mic_ch);
        audio_adc_del_output_handler(&adc_hdl, &adc_dm->adc_output);
        free(adc_dm);
        adc_dm = NULL;
    }
}

#include "pdm_link.h"
audio_plnk_t *plnk_mic = NULL;

/*
 *数据结构：
 *单声道：ch00 ch01 ch02 ch0n
 *双声道：ch00 ch01 ch02 ch0n	ch10 ch11 ch12 ch1n
 */
static void plnk_mic_output(void *buf, u16 len)
{
    s16 *mic0;
    s16 *mic1;
    /* audio_plnk_t *plnk = priv; */
    /* s16 *buf = plnk->buf; */
    /* u16 len = plnk->buf_len; */

    //ch0
    mic0 = (s16 *)buf;
#if (PLNK_MIC_CH == 2)
    //ch1
    mic1 = (s16 *)buf + len * 2;
#endif/*PLNK_MIC_CH == 2*/

#if 0 //合并两个声道的声音：LLLRRR... -> LRLRLR...
    for (int cnt = 0; cnt < len; cnt++) {
        tmp_buf[cnt * 2] 		= mic0[cnt];
        tmp_buf[cnt * 2 + 1] 	= mic1[cnt];
    }
#endif

    u16 olen = len << 1;
    u16 wlen;

    putchar('o');
    //printf("olen:%d,%d",len,olen);

    wlen = cbuf_write(&dm.mic0_cb, mic0, olen);
    if (wlen != olen) {
        AC_ERR_LOG("mic0_cbuf full:%d,%d\n", wlen, olen);
    }
#if ((AC_MIC_TYPE == AC_D_MIC) && (PLNK_MIC_CH == 2))
    wlen = cbuf_write(&dm.mic1_cb, mic1, olen);
    if (wlen != olen) {
        AC_ERR_LOG("mic1_cbuf full:%d,%d\n", wlen, olen);
    }
#endif
    os_sem_post(&dm.sem);

#if DM_2_DAC_EN
#if (AC_MIC_TYPE == AC_D_MIC)
#if (PLNK_MIC_CH == 2)
    if (mic_ch_sw++ < 300) {
        putchar('0');
        wlen = audio_dac_write(&dac_hdl, mic0, olen);
    } else {
        putchar('1');
        wlen = audio_dac_write(&dac_hdl, mic1, olen);
        if (mic_ch_sw >= 600) {
            mic_ch_sw = 0;
        }
    }
#else/*单mic数据输出*/
    wlen = audio_dac_write(&dac_hdl, mic0, olen);
#endif/*PLNK_MIC_CH*/
#else
    if (mic_ch_sw < 300) {
        mic_ch_sw++;
        putchar('0');
        wlen = audio_dac_write(&dac_hdl, mic0, olen);
    }
#endif/*AC_MIC_TYPE*/
    if (wlen != olen) {
        putchar('F');
    }
#endif/*DM_2_DAC_EN*/
}

//#define PLNK_SCLK_PIN 		IO_PORTA_02
//#define PLNK_DAT0_PIN 		IO_PORTA_01
//#define PLNK_CH_EN			PLNK_CH0_EN //(PLNK_CH0_EN | PLNK_CH1_EN)
static int audio_pdm_mic_init(u16 sr)
{
#if 0/*br28目前app层还没有做pdm*/
    AC_LOG("[dm]audio_pdm_mic_init:%d\n", sr);

    //MIC_PWR0
    JL_PORTA->DIR &= ~BIT(2);
    JL_PORTA->OUT |= BIT(2);
    //6976B PA2 PA3双绑
    JL_PORTA->DIR |= BIT(3);
    JL_PORTA->PU &= ~BIT(3);
    JL_PORTA->PD &= ~BIT(3);

#if (AC_MIC_TYPE == AC_D_MIC)
    //MIC_PWR1
    JL_PORTB->DIR &= ~BIT(7);
    JL_PORTB->OUT |= BIT(7);
#endif

    plnk_mic = zalloc(sizeof(audio_plnk_t));
    if (plnk_mic) {
        plnk_mic->ch_num = PLNK_MIC_CH;
        plnk_mic->sr = sr;
        plnk_mic->buf_len = 256;
#if (PLNK_CH_EN == PLNK_CH0_EN)/*两个通道，复用ch0*/
        plnk_mic->ch0_mode = CH0MD_CH0_SCLK_RISING_EDGE;
        plnk_mic->ch1_mode = CH1MD_CH0_SCLK_FALLING_EDGE;
#elif (PLNK_CH_EN == PLNK_CH1_EN)/*两个通道，复用ch1*/
        plnk_mic->ch0_mode = CH0MD_CH1_SCLK_FALLING_EDGE;
        plnk_mic->ch1_mode = CH1MD_CH1_SCLK_RISING_EDGE;
#else/*两个通道，用两个data ch*/
        plnk_mic->ch0_mode = CH0MD_CH0_SCLK_RISING_EDGE;
        plnk_mic->ch1_mode = CH1MD_CH1_SCLK_RISING_EDGE;
#endif/*PLNK_CH_EN*/
        plnk_mic->buf = zalloc(plnk_mic->ch_num * plnk_mic->buf_len * 2 * 2);
        ASSERT(plnk_mic->buf);
        plnk_mic->output = plnk_mic_output;

        plnk_mic->sclk_io = TCFG_AUDIO_PLNK_SCLK_PIN;
        plnk_mic->ch0_io = TCFG_AUDIO_PLNK_DAT0_PIN;

        audio_plnk_open(plnk_mic);
        audio_plnk_start(plnk_mic);

        //dm_2_dac_open(plnk_mic->sr);
    }
#endif
    return 0;
}

static int DualMic_NoiseReduce_run(s16 *dat0, s16 *dat2, s16 *out, u16 points)
{
    memcpy(out, dat0, points << 1);
    return points;
}

#define SPP_EXPORT_RETRY_EN		0

#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP)
static void spp_data_export_run()
{
    int ret;
    int out_points;

    if (dm.state != AC_STATE_START) {
        return;
    }

#if AC_MIC_TYPE == AC_VAD_MIC
    de.ch_idx = 1;
#endif
    switch (de.ch_idx) {
    case 0:
        //putchar('0');
        if (de.retry == 0) {
            ret = cbuf_read(&dm.mic0_cb, &dm.export_buf[4], SPP_EXPORT_DATA_LEN);
            memcpy(dm.export_buf, &de.seqn0, SPP_EXPORT_HEAD);
        } else {
            ret = SPP_EXPORT_DATA_LEN;
        }
        if (ret == SPP_EXPORT_DATA_LEN) {
            ret = spp_data_export(0, dm.export_buf, SPP_EXPORT_MTU);
            if (ret == SPP_EXPORT_MTU) {
                de.seqn0++;
                de.ch_idx++;
                de.retry = 0;
                //putchar('A');
            } else if (ret == -1) {
                de.retry = SPP_EXPORT_RETRY_EN;
            } else {
                de.ch_idx++;
                de.retry = 0;
            }
        }
        break;
    case 1:
        //putchar('1');
#if NS_DEMO_EN
        ret = cbuf_read(&dm.out_cbuf, &dm.export_buf[4], SPP_EXPORT_DATA_LEN);
#else
#if AC_MIC_TYPE == AC_VAD_MIC
        if (cbuf_get_data_len(&dm.mic1_cb) < SPP_EXPORT_DATA_LEN) {
            break;
        }
#endif
        ret = cbuf_read(&dm.mic1_cb, &dm.export_buf[4], SPP_EXPORT_DATA_LEN);
#endif/*NS_DEMO_EN*/
        memcpy(dm.export_buf, &de.seqn1, SPP_EXPORT_HEAD);
        if (ret == SPP_EXPORT_DATA_LEN) {
            ret = spp_data_export(1, dm.export_buf, SPP_EXPORT_MTU);
            if (ret == SPP_EXPORT_MTU) {
                de.seqn1++;
                de.ch_idx++;
                de.retry = 0;
                //putchar('B');
            } else if (ret == -1) {
                de.retry = SPP_EXPORT_RETRY_EN;
            } else {
                de.ch_idx++;
                de.retry = 0;
            }
        }
        break;
    case 2:
        //putchar('2');
        ret = cbuf_read(&dm.mic2_cb, &dm.export_buf[4], SPP_EXPORT_DATA_LEN);
        memcpy(dm.export_buf, &de.seqn2, SPP_EXPORT_HEAD);
        if (ret == SPP_EXPORT_DATA_LEN) {
            //putchar('C');
            ret = spp_data_export(2, dm.export_buf, SPP_EXPORT_MTU);
            if (ret == SPP_EXPORT_MTU) {
                de.seqn2++;
                de.ch_idx++;
                de.retry = 0;
            } else if (ret == -1) {
                de.retry = SPP_EXPORT_RETRY_EN;
            } else {
                de.ch_idx++;
                de.retry = 0;
            }
        }
        break;
    }
    if (de.ch_idx > (SPP_EXPORT_CH - 1)) {
        de.ch_idx = 0;
    }

}
static void aec_export_timer(void *priv)
{
#if SPP_SEND_BY_TIMER
    spp_data_export_run();
#else
    os_sem_set(&dm.sem, 0);
    os_sem_post(&dm.sem);
#endif
}

#endif



static int audio_ns_open(u16 sr);
int audio_capture_start(void);
static void audio_capture_board_init()
{
    led_init();
    clk_set("sys", AudioCapture_CLK);
    data_export_init();
#if NS_DEMO_EN
    audio_ns_open(dm.sr);
#endif /*NS_DEMO_EN*/

    /*spp数据导出，由spp控制声音捕捉启动;SD数据导出，则默认自动启动声音捕捉*/
#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SD)
    audio_capture_start();
#endif/*AUDIO_DATA_EXPORT_USE_SD*/

#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_UART)
    audio_capture_start();
    aec_uart_init();
#endif/*AUDIO_DATA_EXPORT_USE_SD*/

    dm.state = AC_STATE_INIT;
}

int audio_capture_start(void)
{
    dm.sr = AC_SAMPLE_RATE;
    cbuf_init(&dm.mic0_cb, dm.mic0_buf, sizeof(dm.mic0_buf));
    cbuf_init(&dm.mic1_cb, dm.mic1_buf, sizeof(dm.mic1_buf));
    cbuf_init(&dm.mic2_cb, dm.mic2_buf, sizeof(dm.mic2_buf));
    //cbuf_init(&dm.out_cb, dm.out_buf, sizeof(dm.out_buf));
#if (AC_MIC_TYPE & AC_A_MIC)
    audio_mic_pwr_ctl(MIC_PWR_ON);
    audio_mic_init(dm.sr);
#endif/*AC_MIC_TYPE*/

#if (AC_MIC_TYPE & AC_D_MIC)
    audio_pdm_mic_init(dm.sr);
#endif/*AC_MIC_TYPE*/

#if (AC_MIC_TYPE & AC_VAD_MIC)
    /*VAD MIC数据先通过mic1通道导出*/
    dm.vad_mic = audio_vad_mic_capture(dm.sr, &dm, adc_mic_output);
#endif /*AC_VAD_MIC*/
    memset(&de, 0, sizeof(data_export_t));
#if SPP_SEND_BY_TIMER
    de.send_timer = usr_timer_add(NULL, aec_export_timer, SPP_SEND_INTERVAL, 1);
#endif/*SPP_SEND_BY_TIMER*/

    dm_2_dac_open(dm.sr);
    dm.state = AC_STATE_START;
    AC_LOG("DataExport Start\n");
    return 0;
}

void audio_capture_stop(void)
{
#if !(AC_MIC_TYPE & AC_VAD_MIC)
    audio_mic_pwr_ctl(MIC_PWR_OFF);
    audio_mic_close();
#else
    audio_vad_mic_stop_capture(dm.vad_mic);
    dm.vad_mic = NULL;
#endif
    if (de.send_timer) {
        usr_timeout_del(de.send_timer);
        de.send_timer = 0;
    }
    dm.state = AC_STATE_STOP;
    AC_LOG("DataExport Stop\n");
}

/*Noise Reduction/Suppression*/
#include "commproc_ns.h"
typedef struct {
    u16 bypass;
    s16 inbuf[DM_RUN_POINT];
    s16 outbuf[320];
    noise_suppress_param param;
} audio_ns_t;
audio_ns_t *audio_ns = NULL;

static int audio_ns_run(s16 *in, s16 *out, u16 points)
{
    int out_points = 0;
    if (audio_ns->bypass) {
        out_points = points;
        memcpy(out, in, (out_points << 1));
    } else {
        out_points = noise_suppress_run(in, out, points);
        //printf(" <%d> ",out_points);
    }
    return (out_points << 1);
}

static int audio_ns_open(u16 sr)
{
    printf("audio_ns_open:%d\n", sr);

    extern u32 aec_addr[], aec_begin[], aec_size[];
    memcpy(aec_addr, aec_begin, (u32)aec_size) ;

    audio_ns = zalloc(sizeof(audio_ns_t));
    //audio_ns->bypass = 1;
    audio_ns->param.wideband = 1;
    audio_ns->param.mode = 1;
    audio_ns->param.AggressFactor = 1.25f;
    audio_ns->param.MinSuppress = 0.04f;
    audio_ns->param.NoiseLevel = 2.2e4f;
    noise_suppress_open(&audio_ns->param);
    printf("audio_ns_open succ\n");
    return 0;
}

static void audio_ns_close(void)
{
    if (audio_ns) {
        noise_suppress_close();
        free(audio_ns);
        audio_ns = NULL;
    }
}

extern int aec_uart_init();
extern int aec_uart_fill(u8 ch, void *buf, u16 size);
extern void aec_uart_write(void);
extern int aec_uart_close(void);
#define UART_EXPORT_MTU		512
u8 uart_wbuf[UART_EXPORT_MTU];
static void AudioCapture_Task(void *p)
{
    int ret = 0;
    AC_LOG(">>>AudioCapture_Task<<<\n");

    while (tone_get_status()) {
        os_time_dly(2);
    }

    audio_capture_board_init();
#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SD)
    while (1) {
        if ((dm.mic0_cb.data_len >= DM_RUN_SIZE) && (dm.mic1_cb.data_len >= DM_RUN_SIZE)) {
            cbuf_read(&dm.mic0_cb, dm.mic0, DM_RUN_SIZE);
            data_export_run(dm.mic0, DM_RUN_SIZE, 0);
            cbuf_read(&dm.mic1_cb, dm.mic1, DM_RUN_SIZE);
            data_export_run(dm.mic1, DM_RUN_SIZE, 1);
            int out_points = DualMic_NoiseReduce_run(dm.mic0, dm.mic1, dm.output, DM_RUN_POINT);
            data_export_run(dm.output, (out_points << 1), 2);
        } else {
            os_sem_pend(&dm.sem, 0);
        }
    }
#elif (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_UART)
    AC_LOG(">>>AudioCapture_Task[UART]<<<\n");
    while (1) {
        if (dm.mic0_cb.data_len >= UART_EXPORT_MTU) {
            putchar('w');
            ret = cbuf_read(&dm.mic0_cb, uart_wbuf, UART_EXPORT_MTU);
            if (ret == UART_EXPORT_MTU) {
                aec_uart_fill(0, uart_wbuf, UART_EXPORT_MTU);		//主mic数据
                putchar('0');
            }
            ret = cbuf_read(&dm.mic1_cb, uart_wbuf, UART_EXPORT_MTU);
            if (ret == UART_EXPORT_MTU) {
                aec_uart_fill(1, uart_wbuf, UART_EXPORT_MTU);		//主mic数据
                putchar('1');
            }
            ret = cbuf_read(&dm.mic2_cb, uart_wbuf, UART_EXPORT_MTU);
            if (ret == UART_EXPORT_MTU) {
                aec_uart_fill(2, uart_wbuf, UART_EXPORT_MTU);		//主mic数据
                putchar('2');
            }
            aec_uart_write();
        } else {
            os_sem_pend(&dm.sem, 0);
        }
    }
#else
    while (1) {
        os_sem_pend(&dm.sem, 0);

        //do ans here:
        if (audio_ns) {
            if (cbuf_read(&dm.mic1_cb, audio_ns->inbuf, DM_RUN_SIZE) == DM_RUN_SIZE) {
                cbuf_write(&dm.mic0_cb, audio_ns->inbuf, DM_RUN_SIZE);
                int out_size = audio_ns_run(audio_ns->inbuf, audio_ns->outbuf, DM_RUN_POINT);
#if DM_2_DAC_EN
                ret = audio_dac_write(&dac_hdl, audio_ns->outbuf, out_size);
#endif /*DM_2_DAC_EN*/
                ret = cbuf_write(&dm.out_cbuf, audio_ns->outbuf, out_size);
                if (ret != out_size) {
                    AC_LOG("NS output full\n");
                }
            }
        }

#if (SPP_SEND_BY_TIMER == 0)
        spp_data_export_run();
#endif
    }
#endif
}

int audio_capture_init(void)
{
    AC_LOG("Audio Capture init\n");
    os_sem_create(&dm.sem, 0);
    task_create(AudioCapture_Task, NULL, "aud_capture");
    return 0;
}

int audio_capture_exit()
{
    return 0;
}

#endif
