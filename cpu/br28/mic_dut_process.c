
#include "mic_dut_process.h"
#include "audio_enc.h"
#include "app_config.h"
#include "audio_adc.h"
#include "audio_config.h"
#include "asm/dac.h"
#include "audio_dvol.h"
#include "application/eq_config.h"
#include "audio_dec_eff.h"
/* #include "sound_device.h" */

#if ((defined TCFG_AUDIO_MIC_DUT_ENABLE) && TCFG_AUDIO_MIC_DUT_ENABLE)
#include "online_debug/aud_mic_dut.h"

/*移植代码时需要重新实现或者修改的函数或者变量
 * AUDIO_ADC_CH
 * int audio_mic_dut_info_get(mic_dut_info_t *info);
 * int audio_mic_dut_dac_start(u32 sr, u16 vol);
 * int audio_mic_dut_dac_stop(void);
 * int audio_mic_dut_dac_write(s16 *data, int len);
 * int audio_mic_en(u8 ,u16, u16, u16. u16, u16, void *);
 * int audio_mic_dut_gain_set(u16 gain);
 */


/*mic数据输出开头丢掉的数据包数*/
#define MIC_DUT_IN_DUMP_PACKET	10

/*MIC上行eq*/
#define MIC_DUT_EQ_ENABLE	1


#define AUDIO_ADC_BUF_NUM        3	//mic_adc采样buf个数
#define AUDIO_ADC_IRQ_POINTS     320 //mic_adc采样长度（单位：点数）
#define AUDIO_ADC_CH			 4	//双mic通话
#define AUDIO_ADC_BUFS_SIZE      (AUDIO_ADC_BUF_NUM * AUDIO_ADC_IRQ_POINTS * AUDIO_ADC_CH)
struct audio_mic_hdl {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    s16 adc_buf[AUDIO_ADC_BUFS_SIZE];    //align 2Bytes
    s16 output_buf[AUDIO_ADC_BUFS_SIZE];	//数据输出缓存
    cbuffer_t output_cbuf;
    u16 dump_packet;
    struct dec_eq_drc       *mic_dut_eq;
};
static struct audio_mic_hdl *audio_mic = NULL;

extern struct audio_adc_hdl adc_hdl;
extern struct audio_dac_hdl dac_hdl;

void *mic_dut_eq_open(u32 sample_rate, u8 ch_num)
{
#if TCFG_EQ_ENABLE

    struct dec_eq_drc *eff = zalloc(sizeof(struct dec_eq_drc));
    struct audio_eq_param eq_param = {0};

    eq_param.channels = ch_num;
    eq_param.online_en = 1;
    eq_param.mode_en = 0;
    eq_param.remain_en = 0;
    eq_param.no_wait = 0;
    eq_param.out_32bit = 0;
    eq_param.max_nsection = EQ_SECTION_MAX;
    eq_param.cb = eq_get_filter_info;
    eq_param.eq_name = song_eq_mode;
    eq_param.sr = sample_rate;
    eff->eq = audio_dec_eq_open(&eq_param);
    return eff;
#else
    return NULL;
#endif//TCFG_EQ_ENABLE
}

void mic_dut_eq_run(struct dec_eq_drc *eq_drc, void *data, u32 len)
{
    struct dec_eq_drc *eff_hdl = (struct dec_eq_drc *)eq_drc;
#if TCFG_EQ_ENABLE
    if (eff_hdl->eq) {
        audio_eq_run(eff_hdl->eq, data, len);
    }
#endif/*TCFG_EQ_ENABLE*/
}

void mic_dut_eq_close(struct dec_eq_drc *eq_drc)
{
#if TCFG_EQ_ENABLE
    struct dec_eq_drc *eff_hdl = (struct dec_eq_drc *)eq_drc;
    /* struct audio_eq_drc *eff_hdl = (struct audio_eq_drc *)eq_drc; */
    if (eff_hdl->eq) {
        audio_dec_eq_close(eff_hdl->eq);
        eff_hdl->eq = NULL;
    }
    free(eff_hdl);
#endif /*TCFG_EQ_ENABLE*/
}

/*获取麦克风测试硬件数据*/
int audio_mic_dut_info_get(mic_dut_info_t *info)
{
    mic_dut_info_t mic_info = {
        .version = 0x01,
#if (AUDIO_ADC_CH > 2)
        .amic_enable_bit = BIT(0) | BIT(1) | BIT(2) | BIT(3),
#elif (AUDIO_ADC_CH > 1)
        .amic_enable_bit = BIT(0) | BIT(1),
#else /*AUDIO_ADC_CH == 1*/
        .amic_enable_bit = BIT(0),
#endif /*AUDIO_ADC_CH*/
        .dmic_enable_bit = 0x00,
        .channel = 1,
        .bit_wide = 16,
        .sr_enable_bit = MIC_ADC_SR_8000 | MIC_ADC_SR_16000 | MIC_ADC_SR_32000,
        .sr_default = 16000,
        .mic_gain = 19,
        .mic_gain_default = 10,
        .dac_gain = 15,
        .dac_gain_default = 2,
    };

    memcpy(info, &mic_info, sizeof(mic_dut_info_t));
    return 0;
}

/* struct audio_dac_channel mic_dut_dac_ch; */
/*打开dac*/
int audio_mic_dut_dac_start(u32 sr, u16 vol)
{
    printf("dac sr: , vol:  %d\n", sr, vol);
    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());
    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, vol, 0);
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_open(MUSIC_DVOL, app_audio_get_volume(APP_AUDIO_STATE_MUSIC), MUSIC_DVOL_MAX, MUSIC_DVOL_FS, -1);
#endif /*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

#if 0
    // DAC 初始化
    audio_dac_new_channel(&dac_hdl, &mic_dut_dac_ch);
    struct audio_dac_channel_attr attr;
    attr.delay_time = 20;
    attr.protect_time = 8;
    attr.write_mode = WRITE_MODE_BLOCK;
    audio_dac_channel_set_attr(&mic_dut_dac_ch, &attr);
    sound_pcm_dev_start(&mic_dut_dac_ch, sr, app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
#else
    audio_dac_set_volume(&dac_hdl, vol);
    audio_dac_set_sample_rate(&dac_hdl, sr);
    audio_dac_start(&dac_hdl);
#endif
    return 0;
}

/*关闭dac*/
int audio_mic_dut_dac_stop(void)
{
#if 0
    sound_pcm_dev_stop(&mic_dut_dac_ch);
#else
    audio_dac_stop(&dac_hdl);
#endif

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_close(MUSIC_DVOL);
#endif /*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/
    return 0;
}

/*写数到dac*/
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
s16 mic_dut_mono_to_dual[AUDIO_ADC_IRQ_POINTS * 2 + 4];
#endif/*TCFG_AUDIO_DAC_CONNECT_MODE*/
int audio_mic_dut_dac_write(s16 *data, int len)
{
    s16 *dac_data = data;
    u16 data_len = len;
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_run(MUSIC_DVOL, dac_data, len);
#endif /*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
    for (u16 i = 0; i < (len >> 1); i++) {
        mic_dut_mono_to_dual[2 * i] 		= dac_data[i];
        mic_dut_mono_to_dual[2 * i + 1] 	= dac_data[i];
    }
    dac_data = mic_dut_mono_to_dual;
    data_len = len + data_len;
#endif/*TCFG_AUDIO_DAC_CONNECT_MODE*/

#if 0
    int wlen = 0;
    int offset = 0;
    int cnt = 0;
    do {
        wlen = sound_pcm_dev_write(&mic_dut_dac_ch, dac_data + (offset >> 1), data_len - offset);
        offset = offset + wlen;
        cnt ++;
    } while ((offset != data_len) && (cnt < 1000));

    return offset;
#else
    return audio_dac_write(&dac_hdl, dac_data, data_len);
#endif
}

/*
*********************************************************************
*                  AUDIO ADC MIC OPEN
* Description: 打开mic通道
* Arguments  : mic_ch 		mic通道 LADC_CH_MIC_R/LADC_CH_MIC_L/AUDIO_ADC_MIC_0/AUDIO_ADC_MIC_1/AUDIO_ADC_MIC_2/AUDIO_ADC_MIC_3
*			   sr			mic采样率 8000/16000/32000/44100/48000
*			   gain0		mic0增益
*			   gain1		mic0增益
*			   gain2		mic0增益
*			   gain3		mic0增益
* Return	 : None.
* Note(s)    : (1)打开一个mic通道示例：
*				audio_mic_en(LADC_CH_MIC_L,16000,10,0,0,0);
*				或者
*				audio_mic_en(LADC_CH_MIC_R,16000,10,0,0,0);
*			   (2)打开两个mic通道示例：
*				audio_mic_en(LADC_CH_MIC_R|LADC_CH_MIC_L,16000,10,10,0,0);
*********************************************************************
*/
int audio_mic_en(u8 mic_ch, u16 sr,
                 u16 gain0, u16 gain1, u16 gain2, u16 gain3,
                 void (*data_handler)(void *priv, s16 *data, int len))
{
    printf("mic ch : %d\n\n", mic_ch);
    if (mic_ch) {
        if (audio_mic) {
            printf("audio_mic re-malloc error\n");
            return -1;
        }
        audio_mic = zalloc(sizeof(struct audio_mic_hdl));
        if (audio_mic == NULL) {
            printf("audio mic zalloc failed\n");
            return -1;
        }

#if MIC_DUT_EQ_ENABLE
        audio_mic->mic_dut_eq = mic_dut_eq_open(sr, 1);
#endif /*MIC_DUT_EQ_ENABLE*/

        cbuf_init(&audio_mic->output_cbuf, audio_mic->output_buf, sizeof(audio_mic->output_buf));
        cbuf_clear(&audio_mic->output_cbuf);

        /*打开mic电压*/
        audio_mic_pwr_ctl(MIC_PWR_ON);
        /*打开mic0*/
        if (mic_ch & BIT(0)) {
            printf("adc_mic0 open, sr:%d, gain:%d\n", sr, gain0);
            audio_adc_mic_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic_set_gain(&audio_mic->mic_ch, gain0);
        }
        /*打开mic1*/
        if (mic_ch & BIT(1)) {
            printf("adc_mic1 open, sr:%d, gain:%d\n", sr, gain1);
            audio_adc_mic1_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic1_set_gain(&audio_mic->mic_ch, gain1);
        }
#if (AUDIO_ADC_CH > 2)
        /*打开mic2*/
        if (mic_ch & BIT(2)) {
            printf("adc_mic2 open, sr:%d, gain:%d\n", sr, gain2);
            audio_adc_mic2_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic2_set_gain(&audio_mic->mic_ch, gain2);
        }
        /*打开mic3*/
        if (mic_ch & BIT(3)) {
            printf("adc_mic3 open, sr:%d, gain:%d\n", sr, gain3);
            audio_adc_mic3_open(&audio_mic->mic_ch, mic_ch, &adc_hdl);
            audio_adc_mic3_set_gain(&audio_mic->mic_ch, gain3);
        }
#endif /*AUDIO_ADC_CH*/
        audio_adc_mic_set_sample_rate(&audio_mic->mic_ch, sr);
        audio_adc_mic_set_buffs(&audio_mic->mic_ch, audio_mic->adc_buf,
                                AUDIO_ADC_IRQ_POINTS * 2, AUDIO_ADC_BUF_NUM);
        audio_mic->adc_output.handler = data_handler;
        audio_adc_add_output_handler(&adc_hdl, &audio_mic->adc_output);
        audio_adc_mic_start(&audio_mic->mic_ch);
        /*清掉mic开头几包数据*/
        audio_mic->dump_packet = MIC_DUT_IN_DUMP_PACKET;
    } else {
        if (audio_mic) {
            audio_adc_mic_close(&audio_mic->mic_ch);
            audio_adc_del_output_handler(&adc_hdl, &audio_mic->adc_output);
            audio_mic_pwr_ctl(MIC_PWR_OFF);

#if MIC_DUT_EQ_ENABLE
            mic_dut_eq_close(audio_mic->mic_dut_eq);
#endif /*MIC_DUT_EQ_ENABLE*/

            free(audio_mic);
            audio_mic = NULL;
        }
    }
    return 0;
}

/*设置mic增益*/
int audio_mic_dut_gain_set(u16 gain)
{
    if (audio_mic) {
        audio_adc_mic_set_gain(&audio_mic->mic_ch, gain);
        audio_adc_mic1_set_gain(&audio_mic->mic_ch, gain);
#if (AUDIO_ADC_CH > 2)
        audio_adc_mic2_set_gain(&audio_mic->mic_ch, gain);
        audio_adc_mic3_set_gain(&audio_mic->mic_ch, gain);
#endif /*AUDIO_ADC_CH*/
    }
    return 0;
}

/*
*********************************************************************
*                  			Comfort Noise Generator
* Description: 舒适噪声生成
* Arguments  : state	伪随机数生成状态
*			   gain		舒适噪声的增益
*			   data		待添加舒适噪声的数据
*			   npoint	待添加舒适噪声的数据长度
* Return	 : None.
* Note(s)    : 增益计算公式：gain cal(python):
*				>>>import math
* 				>>>gain=math.pow(10,gain_db/20.)*2**20;
* 				>>>print(gain)
*********************************************************************
*/
//int wn_gain = 1865;	/*Normal*/
//int wn_gain = 1<<19;
//
static unsigned int next = 457;
void set_srand(unsigned int seek)
{
    next = seek;
}

static int my_rand()
{
    next = next * 1103515245 + 12345;
    return next >> 16;
}
#define COMFORT_NOISE_SEED 457
static unsigned int seed = COMFORT_NOISE_SEED;
void set_comfort_noise_seed(unsigned int sd)
{
    seed = sd;
    set_srand(sd);
}
static void comfort_noise_run(unsigned int *state, int gain, short *data, int npoint)
{
    for (int i = 0; i < npoint; i++) {
#if 0
        *state = (*state * 1103515245 + 12345) & 0x7fffffff;
        int tmp32 = (*state >> 15) - (1 << 15);
        tmp32 = (long long)tmp32 * gain >> 20;
        tmp32 += data[i];
        if (tmp32 > 32767) {
            tmp32 = 32767;
        }
        if (tmp32 < -32768) {
            tmp32 = -32768;
        }
        data[i] = tmp32;
#else
        short tmp16 = (short)my_rand();
        int tmp32 = ((long long)tmp16 * gain) >> 20;
        if (tmp32 > 32767) {
            tmp32 = 32767;
        }
        if (tmp32 < -32768) {
            tmp32 = -32768;
        }
        data[i] = (short)tmp32;
#endif
    }
}

/*
*********************************************************************
*                  AUDIO MIC OUTPUT
* Description: mic采样数据回调
* Arguments  : pric 私有参数
*			   data	mic数据
*			   len	数据长度
* Return	 : 0 成功 其他 失败
* Note(s)    : 单声道数据格式
*			   L0 L1 L2 L3 L4 ...
*				双声道数据格式()
*			   L0 R0 L1 R1 L2 R2...
*********************************************************************
*/
void audio_mic_dut_eq_run(s16 *data, int len)
{
    u8 scan_start = mic_dut_online_get_scan_state();
#if MIC_DUT_EQ_ENABLE
    if (scan_start && audio_mic) {
        mic_dut_eq_run(audio_mic->mic_dut_eq, data, len);
    }
#endif /*MIC_DUT_EQ_ENABLE*/
}

static void audio_mic_dut_output_handle(void *priv, s16 *data, int len)
{
    if (audio_mic) {
        s16 *mic_data = data;
        int wlen = 0;
        u8 mic_idx = mic_dut_online_get_mic_idx();
        u8 mic_start = mic_dut_online_get_mic_state();
        u8 scan_start = mic_dut_online_get_scan_state();
        /* printf("mic_data:%x,%d\n",data,len); */
        for (u16 i = 0; i < (len >> 1); i++) {
            mic_data[i] = data[i * AUDIO_ADC_CH + mic_idx];
        }
        /*清掉开头10包数据*/
        if (audio_mic->dump_packet) {
            audio_mic->dump_packet --;
        }
        if ((mic_start || scan_start) && !audio_mic->dump_packet) {
#if MIC_DUT_EQ_ENABLE
            /* if(scan_start) { */
            /*     mic_dut_eq_run(audio_mic->mic_dut_eq, mic_data, len); */
            /* } */
#endif /*MIC_DUT_EQ_ENABLE*/
            wlen = cbuf_write(&audio_mic->output_cbuf, mic_data, len);
            if (wlen != len) {
                printf("dut out buf full !!!\n");
            }
        }
        /*dac播放白噪*/
        if (scan_start && !audio_mic->dump_packet) {
            s16 tmpbuf[AUDIO_ADC_IRQ_POINTS];
            comfort_noise_run(&seed, 1048576, tmpbuf, len / 2);
            wlen = audio_mic_dut_dac_write(tmpbuf, len);
            if (wlen != len) {
                printf("dut scan dac write fail !!!\n");
            }
        }
        mic_dut_online_sem_post();
    }
}

int audio_mic_dut_sample_rate_set(u32 sr)
{

    return 0;
}

int audio_mic_dut_amic_select(u8 idx)
{

    return 0;
}

int audio_mic_dut_dmic_select(u8 idx)
{

    return 0;
}

/*打开mic*/
int audio_mic_dut_start(void)
{
    u8 amic_enable_bit = mic_dut_online_amic_enable_bit();
    u16 gain = mic_dut_online_get_mic_gain();
    u32 sr = mic_dut_online_get_sample_rate();
    /*打开所有MIC*/
    audio_mic_en(amic_enable_bit,
                 sr,
                 gain,
                 gain,
                 gain,
                 gain,
                 audio_mic_dut_output_handle);

    return 0;
}

/*关闭mic*/
int audio_mic_dut_stop(void)
{
    /*关闭所有MIC*/
    audio_mic_en(0, 0,
                 0, 0, 0, 0,
                 NULL);
    return 0;
}

/*获取mic数据*/
int audio_mic_dut_data_get(s16 *data, int len)
{
    /*获取对应的MIC数据，并返回获取到的数据大小*/
    int rlen = 0;
    if (audio_mic) {
        rlen = cbuf_read(&audio_mic->output_cbuf, data, len);
    }
    return rlen;
}

/*获取mic缓存数据长度*/
int audio_mic_dut_get_data_len(void)
{
    return cbuf_get_data_len(&audio_mic->output_cbuf);
}

/*设置dac输出音量*/
int audio_mic_dut_dac_gain_set(u16 gain)
{
    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, gain, 0);
    return 0;
}

/*开始mic频响扫描*/
int audio_mic_dut_scan_start(void)
{
    u32 sr = mic_dut_online_get_sample_rate();
    u16 vol = mic_dut_online_get_dac_gain();
    /*重置随机白噪声种子*/
    set_comfort_noise_seed(COMFORT_NOISE_SEED);
    // 打开DAC
    audio_mic_dut_dac_start(sr, vol);
    // 打开mic
    audio_mic_dut_start();

    return 0;
}

/*关闭mic频响扫描*/
int audio_mic_dut_scan_stop(void)
{
    //关闭mic
    audio_mic_dut_stop();
    // DAC 关闭
    audio_mic_dut_dac_stop();
    return 0;
}
#endif /*((defined TCFG_AUDIO_MIC_DUT_ENABLE) && TCFG_AUDIO_MIC_DUT_ENABLE)*/
