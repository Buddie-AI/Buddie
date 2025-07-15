/*
 ****************************************************************************
 *							Audio ADC Demo
 *
 *Description	: Audio ADC使用范例
 *Notes			: (1)本demo为开发测试范例，请不要修改该demo， 如有需求，请自行
 *				  复制再修改
 *				  (2)mic工作模式说明
 *				A.单端隔直(cap_mode)
 *				  可以选择mic供电方式：外部供电还是内部供电(mic_bias_inside = 1)
 *				B.单端省电容(capless_mode)
 *				C.差分模式(cap_diff_mode)
 ****************************************************************************
 */
#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "app_main.h"
#include "audio_config.h"
#include "audio_demo.h"
#include "asm/audio_adc.h"

#if 1
#define ADC_DEMO_LOG	printf
#else
#define ADC_DEMO_LOG(...)
#endif

extern struct audio_dac_hdl dac_hdl;
extern struct audio_adc_hdl adc_hdl;

#define ADC_DEMO_CH_NUM        	4	/*支持的最大采样通道(max = 2)*/
#define ADC_DEMO_BUF_NUM        2	/*采样buf数*/
#define ADC_DEMO_IRQ_POINTS     256	/*采样中断点数*/
#define ADC_DEMO_BUFS_SIZE      (ADC_DEMO_CH_NUM * ADC_DEMO_BUF_NUM * ADC_DEMO_IRQ_POINTS)

struct adc_demo {
    u8 adc_2_dac;
    u8 mic_idx;
    u8 linein_idx;
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    struct adc_linein_ch linein_ch;
    s16 adc_buf[ADC_DEMO_BUFS_SIZE];
    u8 linein_ch_num;
    u8 linein_ch_sel;
};
static struct adc_demo *mic_demo = NULL;
static struct adc_demo *linein_demo = NULL;

/* 选择输出哪路 linein采样的数据,可通过按键调用此函数实现每按一下按键切一下输出的linein通路 */
void linein_output_channel_sel()
{
    if (linein_demo) {
        linein_demo->linein_ch_sel += 1;
        if (linein_demo->linein_ch_sel > linein_demo->linein_ch_num - 1) {
            linein_demo->linein_ch_sel = 0;
        }
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

#define ADC_DEBUG_BUF       (0)
#define ADC_DEBUF_SIZE      (32000)

#if ADC_DEBUG_BUF
u32 d_skip_cnt = 100;
s16 d_buf[ADC_DEBUF_SIZE / 2] = {0};
u8 *d_ptr = d_buf;
u32 d_buf_cnt = 0;
#endif

static void adc_mic_demo_output(void *priv, s16 *data, int len)
{
    //printf("> %d %d %d %d\n", data[0], data[1], data[2], data[3]);

    int wlen = 0;
    if (mic_demo && mic_demo->adc_2_dac) {
        if (mic_demo->mic_idx == (AUDIO_ADC_MIC_0 | AUDIO_ADC_MIC_1)) {
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
            /*输出两个mic的数据到DAC两个通道*/
            putchar('a');
            wlen = audio_dac_write(&dac_hdl, data, len * 2);
#else
            /*输出其中一个mic的数据到DAC一个通道*/
            putchar('b');
            s16 *mono_data = data;
            for (int i = 0; i < (len / 2); i++) {
                mono_data[i] = data[i * 2];			/*MIC_L*/
                //mono_data[i] = data[i * 2 + 1];		/*MIC_R*/
            }
            wlen = audio_dac_write(&dac_hdl, mono_data, len);
#endif/*TCFG_AUDIO_DAC_CONNECT_MODE*/
        } else {
            /*输出一个mic的数据到DAC一个通道*/
            putchar('c');
            wlen = audio_dac_write(&dac_hdl, data, len);
        }
    }

#if ADC_DEBUG_BUF

    wdt_close();
    if (d_skip_cnt) {
        d_skip_cnt--;
        return;
    }

    d_buf_cnt += len * ADC_DEMO_CH_NUM;
    if (d_buf_cnt >= ADC_DEBUF_SIZE) {
        wdt_close();
        local_irq_disable();
        printf(">>> hex\n\n");
        put_buf(d_buf, ADC_DEBUF_SIZE);
        printf(">>> hex end\n\n");
        u32 i;
        for (i = 0; i < ADC_DEBUF_SIZE / 2; i++) {
            /* printf("%d\n", d_buf[i]); */
        }
        printf(">>> printf end\n\n");
        while (1);
    }
    memcpy(d_ptr, data, len * ADC_DEMO_CH_NUM);
    d_ptr += len * ADC_DEMO_CH_NUM;
#endif
}

/*
*********************************************************************
*                  AUDIO Linein OUTPUT
* Description: linein采样数据回调
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

#define LINEIN_DEBUG_BUF       (0)
#define LINEIN_DEBUF_SIZE      (32000)

#if LINEIN_DEBUG_BUF
u32 l_skip_cnt = 100;
s16 l_buf[LINEIN_DEBUF_SIZE / 2] = {0};
u8 *l_ptr = l_buf;
u32 l_buf_cnt = 0;
#endif


static void adc_linein_demo_output(void *priv, s16 *data, int len)
{
    /* printf("> %d %d %d %d\n", data[0], data[1], data[2], data[3]); */
    int wlen = 0;
    if (linein_demo && linein_demo->adc_2_dac) {
        if (linein_demo->linein_ch_num >= 2) {
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
            /*输出两个linein的数据到DAC两个通道,默认输出第一个和第二个采样通道的数据*/
            putchar('a');
            s16 temp[len * 2];
            for (int i = 0; i < len / 2; i++) {
                temp[2 * i] = data[linein_demo->linein_ch_num * i];
                temp[2 * i + 1] = data[linein_demo->linein_ch_num * i + 1];
            }
            wlen = audio_dac_write(&dac_hdl, temp, len * 2);
#else
            /*输出其中一个linein的数据到DAC一个通道*/
            putchar('b');
            s16 *mono_data = data;
            for (int i = 0; i < (len / 2); i++) {
                switch (linein_demo->linein_ch_sel) {
                case 0:
                    mono_data[i] = data[i * linein_demo->linein_ch_num];			/*第一路 linein采样的数据*/
                    break;
                case 1:
                    mono_data[i] = data[i * linein_demo->linein_ch_num + 1];	     /*第一路 linein采样的数据*/
                    break;
                case 2:
                    mono_data[i] = data[i * linein_demo->linein_ch_num + 2];		 /*第三路 linein采样的数据*/
                    break;
                case 3:
                    mono_data[i] = data[i * linein_demo->linein_ch_num + 3];	     /*第四路 linein采样的数据*/
                    break;
                default:
                    break;
                }
            }
            wlen = audio_dac_write(&dac_hdl, mono_data, len);
#endif/*TCFG_AUDIO_DAC_CONNECT_MODE*/
        } else {
            /*输出一个linein的数据到DAC一个通道*/
            putchar('c');
            wlen = audio_dac_write(&dac_hdl, data, len);
        }
    }

#if LINEIN_DEBUG_BUF
    wdt_close();
    if (l_skip_cnt) {
        l_skip_cnt--;
        return;
    }

    l_buf_cnt += len * ADC_DEMO_CH_NUM;
    if (l_buf_cnt >= LINEIN_DEBUF_SIZE) {
        wdt_close();
        local_irq_disable();
        printf(">>> hex\n\n");
        put_buf(l_buf, LINEIN_DEBUF_SIZE);
        printf(">>> hex end\n\n");
        u32 i;
        for (i = 0; i < LINEIN_DEBUF_SIZE / 2; i++) {
            /* printf("%d\n", l_buf[i]); */
        }
        printf(">>> printf end\n\n");
        while (1);
    }
    memcpy(l_ptr, data, len * ADC_DEMO_CH_NUM);
    l_ptr += len * ADC_DEMO_CH_NUM;
#endif
}

/*
*********************************************************************
*                  AUDIO ADC MIC OPEN
* Description: 打开mic通道
* Arguments  : mic_idx 		mic通道
*			   gain			mic增益
*			   sr			mic采样率
*			   mic_2_dac 	mic数据（通过DAC）监听
* Return	 : None.
* Note(s)    : (1)打开一个mic通道示例：
*				audio_adc_mic_demo_open(AUDIO_ADC_MIC_0,10,16000,1);
*				或者
*				audio_adc_mic_demo_open(AUDIO_ADC_MIC_1,10,16000,1);
*			   (2)打开两个mic通道示例：
*				audio_adc_mic_demo_open(AUDIO_ADC_MIC_1|AUDIO_ADC_MIC_0,10,16000,1);
*********************************************************************
*/
void audio_adc_mic_demo_open(u8 mic_idx, u8 gain, u16 sr, u8 mic_2_dac)
{
    ADC_DEMO_LOG("audio_adc_mic_demo open:%d,gain:%d,sr:%d,mic_2_dac:%d\n", mic_idx, gain, sr, mic_2_dac);
    mic_demo = zalloc(sizeof(struct adc_demo));
    if (mic_demo) {
        //step0:打开mic通道，并设置增益
        audio_adc_mic_open(&mic_demo->mic_ch, AUDIO_ADC_MIC_0, &adc_hdl);
        audio_adc_mic_set_gain(&mic_demo->mic_ch, gain);
        audio_adc_mic_0dB_en(1);

        //step1:设置mic通道采样率
        audio_adc_mic_set_sample_rate(&mic_demo->mic_ch, sr);
        //step2:设置mic采样buf
        audio_adc_mic_set_buffs(&mic_demo->mic_ch, mic_demo->adc_buf, ADC_DEMO_IRQ_POINTS * 2, ADC_DEMO_BUF_NUM);
        //step3:设置mic采样输出回调函数
        mic_demo->adc_output.handler = adc_mic_demo_output;
        audio_adc_add_output_handler(&adc_hdl, &mic_demo->adc_output);
        //step4:启动mic通道采样
        audio_adc_mic_start(&mic_demo->mic_ch);

        /*监听配置（可选）*/
        mic_demo->adc_2_dac = mic_2_dac;
        mic_demo->mic_idx = mic_idx;
        if (mic_2_dac) {
            ADC_DEMO_LOG("max_sys_vol:%d\n", get_max_sys_vol());
            app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());
            ADC_DEMO_LOG("cur_vol:%d\n", app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
            audio_dac_set_volume(&dac_hdl, app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
            audio_dac_set_sample_rate(&dac_hdl, sr);
            audio_dac_start(&dac_hdl);
        }
    }
    ADC_DEMO_LOG("audio_adc_mic_demo start succ\n");

    int res = 0;
    int msg[16];
    //	循环一直往dac写数据
#ifdef BT_DUT_ADC_INTERFERE
    while (0) {
#else
    while (1) {
#endif/*BT_DUT_ADC_INTERFERE*/
        // 这句是为了防止线程太久没有响应系统而产生异常，实际使用不需要
        res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        os_time_dly(500);
    }
}

/*
*********************************************************************
*                  AUDIO ADC MIC CLOSE
* Description: 关闭mic采样模块
* Arguments  : None.
* Return     : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_adc_mic_demo_close(void)
{
    ADC_DEMO_LOG("audio_adc_mic_demo_close\n");
    if (mic_demo) {
        audio_adc_del_output_handler(&adc_hdl, &mic_demo->adc_output);
        audio_adc_mic_close(&mic_demo->mic_ch);
        free(mic_demo);
        mic_demo = NULL;
    }
}

/*
*********************************************************************
*                  AUDIO ADC linein OPEN
* Description: 打开linein通道
* Arguments  : linein_idx 		linein通道
*			   gain			linein增益
*			   sr			linein采样率
*			   linein_2_dac 	linein数据（通过DAC）监听
* Return	 : None.
* Note(s)    : (1)打开一个linein通道示例：
*				audio_adc_linein_demo_open(AUDIO_ADC_LINE0,10,16000,1);
*				或者
*				audio_adc_linein_demo_open(AUDIO_ADC_LINE1,10,16000,1);
*			   (2)打开两个linein通道示例：
*				audio_adc_linein_demo_open(AUDIO_ADC_LINE1|AUDIO_ADC_LINE0,10,16000,1);
*			   (3)打开四个linein通道示例：
*				audio_adc_linein_demo_open(AUDIO_ADC_LINE3|AUDIO_ADC_LINE2|AUDIO_ADC_LINE1|AUDIO_ADC_LINE0,10,16000,1);
*********************************************************************
*/
void audio_adc_linein_demo_open(u8 linein_idx, u8 gain, u16 sr, u8 linein_2_dac)
{
    ADC_DEMO_LOG("audio_adc_linein_demo open:%d,gain:%d,sr:%d,linein_2_dac:%d\n", linein_idx, gain, sr, linein_2_dac);
    linein_demo = zalloc(sizeof(struct adc_demo));
    if (linein_demo) {
        //step0:打开linein通道，并设置增益
        if (linein_idx & AUDIO_ADC_LINE0) {
            audio_adc_linein_open(&linein_demo->linein_ch, AUDIO_ADC_LINE0, &adc_hdl);
            audio_adc_linein_set_gain(&linein_demo->linein_ch, gain);
            audio_adc_linein_0dB_en(1);
            linein_demo->linein_ch_num += 1;
        }
        if (linein_idx & AUDIO_ADC_LINE1) {
            audio_adc_linein1_open(&linein_demo->linein_ch, AUDIO_ADC_LINE1, &adc_hdl);
            audio_adc_linein1_set_gain(&linein_demo->linein_ch, gain);
            audio_adc_linein1_0dB_en(1);
            linein_demo->linein_ch_num += 1;
        }
        if (linein_idx & AUDIO_ADC_LINE2) {
            audio_adc_linein2_open(&linein_demo->linein_ch, AUDIO_ADC_LINE2, &adc_hdl);
            audio_adc_linein2_set_gain(&linein_demo->linein_ch, gain);
            audio_adc_linein2_0dB_en(1);
            linein_demo->linein_ch_num += 1;

        }
        if (linein_idx & AUDIO_ADC_LINE3) {
            audio_adc_linein3_open(&linein_demo->linein_ch, AUDIO_ADC_LINE3, &adc_hdl);
            audio_adc_linein3_set_gain(&linein_demo->linein_ch, gain);
            audio_adc_linein3_0dB_en(1);
            linein_demo->linein_ch_num += 1;
        }
        //step1:设置linein通道采样率
        audio_adc_linein_set_sample_rate(&linein_demo->linein_ch, sr);
        //step2:设置linein采样buf
        audio_adc_linein_set_buffs(&linein_demo->linein_ch, linein_demo->adc_buf, ADC_DEMO_IRQ_POINTS * 2, ADC_DEMO_BUF_NUM);
        //step3:设置linein采样输出回调函数
        linein_demo->adc_output.handler = adc_linein_demo_output;
        audio_adc_add_output_handler(&adc_hdl, &linein_demo->adc_output);
        //step4:启动linein通道采样
        audio_adc_linein_start(&linein_demo->linein_ch);

        /*监听配置（可选）*/
        linein_demo->adc_2_dac = linein_2_dac;
        linein_demo->linein_idx = linein_idx;
        if (linein_2_dac) {
            ADC_DEMO_LOG("max_sys_vol:%d\n", get_max_sys_vol());
            app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());
            ADC_DEMO_LOG("cur_vol:%d\n", app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
            audio_dac_set_volume(&dac_hdl, app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
            audio_dac_set_sample_rate(&dac_hdl, sr);
            audio_dac_start(&dac_hdl);
        }
    }
    ADC_DEMO_LOG("audio_adc_linein_demo start succ\n");

    int res = 0;
    int msg[16];
    //	循环一直往dac写数据
    while (1) {
        // 这句是为了防止线程太久没有响应系统而产生异常，实际使用不需要
        res = os_taskq_accept(ARRAY_SIZE(msg), msg);
        os_time_dly(500);
    }
}

/*
*********************************************************************
*                  AUDIO ADC linein CLOSE
* Description: 关闭linein采样模块
* Arguments  : None.
* Return     : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_adc_linein_demo_close(void)
{
    ADC_DEMO_LOG("audio_adc_linein_demo_close\n");
    if (linein_demo) {
        audio_adc_del_output_handler(&adc_hdl, &linein_demo->adc_output);
        audio_adc_linein_close(&linein_demo->linein_ch);
        free(linein_demo);
        linein_demo = NULL;
    }
}

//开启audio_adc接口 用于测试BT干扰
void audio_adc_mic_dut_open(void)
{
    audio_adc_mic_demo_open(AUDIO_ADC_MIC_0, 10, 16000, 0);
}


#if AUDIO_DEMO_LP_REG_ENABLE
static u8 adc_demo_idle_query()
{
    return mic_demo ? 0 : 1;
}

REGISTER_LP_TARGET(adc_demo_lp_target) = {
    .name = "adc_demo",
    .is_idle = adc_demo_idle_query,
};

#endif/*AUDIO_DEMO_LP_REG_ENABLE*/
