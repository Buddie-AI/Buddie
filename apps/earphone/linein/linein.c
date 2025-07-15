#include "system/includes.h"
#include "app_action.h"
#include "key_event_deal.h"
#include "bt_background.h"
#include "default_event_handler.h"
#include "tone_player.h"
#include "user_cfg.h"
#include "app_task.h"
#include "earphone.h"
#include "app_charge.h"
#include "app_power_manage.h"
#include "app_main.h"
#include "media/includes.h"
#include "audio_config.h"
#include "asm/audio_adc.h"
#include "circular_buf.h"
#include "audio_dec_pcm.h"

#define LOG_TAG_CONST       AUX
#define LOG_TAG             "[AUX]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"
#include "log.h"

#if TCFG_APP_AUX_EN

#define POWER_OFF_CNT       6

extern void sys_enter_soft_poweroff(void *priv);
extern int app_power_event_handler(struct device_event *dev);
extern void app_task_switch_next(void);
extern struct audio_dac_hdl dac_hdl;
extern struct audio_adc_hdl adc_hdl;

#define AUX_CH_NUM        	4	/*支持的最大采样通道(max = 2)*/
#define AUX_BUF_NUM         2	/*采样buf数*/
#define AUX_IRQ_POINTS     64	/*采样中断点数*/
#define AUX_BUFS_SIZE      (AUX_CH_NUM * AUX_BUF_NUM * AUX_IRQ_POINTS)
#define AUX_CBUF_SIZE 	(AUX_BUFS_SIZE / 2)

struct linein_hdl {
    u8 adc_2_dac;
    u8 linein_idx;
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    struct adc_linein_ch linein_ch;
    cbuffer_t linein_cbuf;
    s16 adc_buf[AUX_BUFS_SIZE];
    u8 *cbuf;
    u8 linein_ch_num;
    u8 linein_ch_sel;
    u8 need_resume;
    u8 start;
};
struct app_aux_var {
    struct linein_hdl *linein_demo;
    u8 inited;
    u8 idle;
};

static u8 goto_poweroff_cnt = 0;
static u8 goto_poweroff_flag = 0;
static struct app_aux_var g_aux = {0};

static const u8 aux_key_table[][KEY_EVENT_MAX] = {
    // SHORT           LONG              HOLD
    // UP              DOUBLE            TRIPLE
    {
        KEY_MODE_SWITCH, KEY_POWEROFF,      KEY_POWEROFF_HOLD,
        KEY_NULL,       KEY_NULL,          KEY_NULL
    },
    {
        KEY_NULL,       KEY_NULL,          KEY_NULL,
        KEY_NULL,       KEY_NULL,          KEY_NULL
    },
    {
        KEY_NULL,       KEY_NULL,          KEY_NULL,
        KEY_NULL,       KEY_NULL,          KEY_NULL
    },
};

static u8 aux_idle_query()
{
    return !g_aux.idle;
}
REGISTER_LP_TARGET(aux_lp_target) = {
    .name = "aux",
    .is_idle = aux_idle_query,
};


// linein adc 回调函数
static void adc_linein_output_hdl(void *priv, s16 *data, int len)
{
    int wlen = 0;
    if (g_aux.linein_demo && g_aux.linein_demo->cbuf && g_aux.linein_demo->start == 1) {
        if (g_aux.linein_demo->linein_ch_num >= 2) {
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
            /*输出两个linein的数据到DAC两个通道,默认输出第一个和第二个采样通道的数据*/
            s16 temp[len * 2];
            for (int i = 0; i < len / 2; i++) {
                temp[2 * i] = data[g_aux.linein_demo->linein_ch_num * i];
                temp[2 * i + 1] = data[g_aux.linein_demo->linein_ch_num * i + 1];
            }
            /* wlen = audio_dac_write(&dac_hdl, temp, len * 2); */
            wlen = cbuf_write(&g_aux.linein_demo->linein_cbuf, temp, len * 2);
#else
            /*输出其中一个linein的数据到DAC一个通道*/
            s16 *mono_data = data;
            for (int i = 0; i < (len / 2); i++) {
                switch (g_aux.linein_demo->linein_ch_sel) {
                case 0:
                    mono_data[i] = data[i * g_aux.linein_demo->linein_ch_num];			/*第一路 linein采样的数据*/
                    break;
                case 1:
                    mono_data[i] = data[i * g_aux.linein_demo->linein_ch_num + 1];	     /*第一路 linein采样的数据*/
                    break;
                case 2:
                    mono_data[i] = data[i * g_aux.linein_demo->linein_ch_num + 2];		 /*第三路 linein采样的数据*/
                    break;
                case 3:
                    mono_data[i] = data[i * g_aux.linein_demo->linein_ch_num + 3];	     /*第四路 linein采样的数据*/
                    break;
                default:
                    break;
                }
            }
            /* wlen = audio_dac_write(&dac_hdl, mono_data, len); */
            wlen = cbuf_write(&g_aux.linein_demo->linein_cbuf, mono_data, len);
#endif/*TCFG_AUDIO_DAC_CONNECT_MODE*/
        } else {
            /*输出一个linein的数据到DAC一个通道*/
            /* wlen = audio_dac_write(&dac_hdl, data, len); */
            wlen = cbuf_write(&g_aux.linein_demo->linein_cbuf, data, len);
        }

        /* pcm_dec_resume(); */
        if (g_aux.linein_demo->need_resume == 1) {
            //激活解码
            pcm_dec_resume();
        }
    }
}

int linein_data_read(void *priv, void *data, int len)
{
    int rlen = 0;
    if (g_aux.linein_demo && g_aux.linein_demo->start == 1) {
        int read_len = cbuf_get_data_size(&g_aux.linein_demo->linein_cbuf);
        read_len = read_len > len ? len : read_len;
        rlen = cbuf_read(&g_aux.linein_demo->linein_cbuf, data, read_len);
        if (!rlen) {
            g_aux.linein_demo->need_resume = 1;
        }
    }
    return rlen;
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
*				audio_linein_open(AUDIO_ADC_LINE0,10,16000,1);
*				或者
*				audio_linein_open(AUDIO_ADC_LINE1,10,16000,1);
*			   (2)打开两个linein通道示例：
*				audio_linein_open(AUDIO_ADC_LINE1|AUDIO_ADC_LINE0,10,16000,1);
*			   (3)打开四个linein通道示例：
*				audio_linein_open(AUDIO_ADC_LINE3|AUDIO_ADC_LINE2|AUDIO_ADC_LINE1|AUDIO_ADC_LINE0,10,16000,1);
*********************************************************************
*/
void audio_linein_open(u8 linein_idx, u8 gain, u16 sr, u8 linein_2_dac)
{
    if (g_aux.linein_demo) {
        printf("Func:%s,Line:%d, audio linein is opened!\n", __func__, __LINE__);
    }
    printf("\naudio_adc_linein_demo open:%d,gain:%d,sr:%d,linein_2_dac:%d\n", linein_idx, gain, sr, linein_2_dac);
    g_aux.linein_demo = zalloc(sizeof(struct linein_hdl));
    if (g_aux.linein_demo) {
        //step0:打开linein通道，并设置增益
        if (linein_idx & AUDIO_ADC_LINE0) {
            audio_adc_linein_open(&g_aux.linein_demo->linein_ch, AUDIO_ADC_LINE0, &adc_hdl);
            audio_adc_linein_set_gain(&g_aux.linein_demo->linein_ch, gain);
            audio_adc_linein_0dB_en(1);
            g_aux.linein_demo->linein_ch_num += 1;
        }
        if (linein_idx & AUDIO_ADC_LINE1) {
            audio_adc_linein1_open(&g_aux.linein_demo->linein_ch, AUDIO_ADC_LINE1, &adc_hdl);
            audio_adc_linein1_set_gain(&g_aux.linein_demo->linein_ch, gain);
            audio_adc_linein1_0dB_en(1);
            g_aux.linein_demo->linein_ch_num += 1;
        }
        if (linein_idx & AUDIO_ADC_LINE2) {
            audio_adc_linein2_open(&g_aux.linein_demo->linein_ch, AUDIO_ADC_LINE2, &adc_hdl);
            audio_adc_linein2_set_gain(&g_aux.linein_demo->linein_ch, gain);
            audio_adc_linein2_0dB_en(1);
            g_aux.linein_demo->linein_ch_num += 1;

        }
        if (linein_idx & AUDIO_ADC_LINE3) {
            audio_adc_linein3_open(&g_aux.linein_demo->linein_ch, AUDIO_ADC_LINE3, &adc_hdl);
            audio_adc_linein3_set_gain(&g_aux.linein_demo->linein_ch, gain);
            audio_adc_linein3_0dB_en(1);
            g_aux.linein_demo->linein_ch_num += 1;
        }
        //初始化cbuffer
        if (!g_aux.linein_demo->cbuf) {
            g_aux.linein_demo->cbuf = malloc(AUX_CBUF_SIZE);
            if (g_aux.linein_demo->cbuf) {
                cbuf_init(&g_aux.linein_demo->linein_cbuf, g_aux.linein_demo->cbuf, AUX_CBUF_SIZE);
            } else {
                r_printf("Error, Func:%s, Line:%d\n", __func__, __LINE__);
                return;
            }
        }

        //step1:设置linein通道采样率
        audio_adc_linein_set_sample_rate(&g_aux.linein_demo->linein_ch, sr);
        //step2:设置linein采样buf
        audio_adc_linein_set_buffs(&g_aux.linein_demo->linein_ch, g_aux.linein_demo->adc_buf, AUX_IRQ_POINTS * 2, AUX_BUF_NUM);
        //step3:设置linein采样输出回调函数
        g_aux.linein_demo->adc_output.handler = adc_linein_output_hdl;
        audio_adc_add_output_handler(&adc_hdl, &g_aux.linein_demo->adc_output);
        //step4:启动linein通道采样
        audio_adc_linein_start(&g_aux.linein_demo->linein_ch);

        /*监听配置（可选）*/
        g_aux.linein_demo->adc_2_dac = linein_2_dac;
        g_aux.linein_demo->linein_idx = linein_idx;
#if 0
        if (linein_2_dac) {
            printf("max_sys_vol:%d\n", get_max_sys_vol());
            app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());
            printf("cur_vol:%d\n", app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
            audio_dac_set_volume(&dac_hdl, app_audio_get_volume(APP_AUDIO_STATE_MUSIC));
            audio_dac_set_sample_rate(&dac_hdl, sr);
            audio_dac_start(&dac_hdl);
        }
#endif
        g_aux.linein_demo->start = 1;
    }
    printf("audio_adc_linein_demo start succ\n");
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
void audio_linein_close(void)
{
    printf("\naudio_adc_linein_close\n");
    if (g_aux.linein_demo) {
        g_aux.linein_demo->start = 0;
        audio_adc_del_output_handler(&adc_hdl, &g_aux.linein_demo->adc_output);
        audio_adc_linein_close(&g_aux.linein_demo->linein_ch);
        local_irq_disable();
        if (g_aux.linein_demo->cbuf) {
            free(g_aux.linein_demo->cbuf);
            g_aux.linein_demo->cbuf = NULL;
        }
        free(g_aux.linein_demo);
        g_aux.linein_demo = NULL;
        local_irq_enable();
    }
}


static void app_aux_init()
{
    log_info("app_aux_init");
    audio_linein_open(AUDIO_ADC_LINE1 | AUDIO_ADC_LINE0, 10, 44100, 1);
    pcm_audio_play_open(44100, g_aux.linein_demo->linein_ch_num);
    pcm_dec_set_read_cb(linein_data_read);
}

static void app_aux_exit()
{
    log_info("app_aux_exit");
    audio_linein_close();
    pcm_audio_play_close();
    g_aux.idle = 0;
}


static int app_aux_key_event_handler(struct sys_event *event)
{
    struct key_event *key = &event->u.key;
    int key_event = aux_key_table[key->value][key->event];
    log_info("key_event:%d %d %d\n", key_event, key->value, key->event);
    switch (key_event) {
    case KEY_MODE_SWITCH:
        log_info("KEY_MODE_SWITCH");
        app_task_switch_next();
        break;

    case KEY_POWEROFF:
        goto_poweroff_cnt = 0;
        goto_poweroff_flag = 1;
        break;

    case KEY_POWEROFF_HOLD:
        log_info("poweroff flag:%d cnt:%d\n", goto_poweroff_flag, goto_poweroff_cnt);

        if (goto_poweroff_flag) {
            goto_poweroff_cnt++;
            if (goto_poweroff_cnt >= POWER_OFF_CNT) {
                goto_poweroff_cnt = 0;
                sys_enter_soft_poweroff(NULL);
            }
        }
        break;
    case KEY_ANC_SWITCH:
#if TCFG_AUDIO_ANC_ENABLE
        extern void anc_mode_next(void);
        anc_mode_next();
#endif
        break;
    default:
        break;
    }

    return false;
}

/*
 * 系统事件处理函数
 */
static int event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        /*
         * 普通按键消息处理
         */
        return app_aux_key_event_handler(event);
    case SYS_BT_EVENT:
        /*
         * 蓝牙事件处理
         */
        break;
    case SYS_DEVICE_EVENT:
        /*
         * 系统设备事件处理
         */
        if ((u32)event->arg == DEVICE_EVENT_FROM_OTG) {
            if (event->u.dev.event == DEVICE_EVENT_OUT) {
                /* app_task_switch_next(); */
                return true;
            }
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            /* app_power_event_handler(&event->u.dev); */
            return true;
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
#if TCFG_CHARGE_ENABLE
            /* app_charge_event_handler(&event->u.dev); */
#endif
            return true;
        }

        break;
    default:
        return false;
    }


    return false;
}

static int state_machine(struct application *app, enum app_state state,
                         struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        log_info("APP_STA_CREATE");
        break;
    case APP_STA_START:
        log_info("APP_STA_START");
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_AUX_MAIN:
            y_printf("\n>>>>>>> Enter Linein Mode!\n");
            log_info("ACTION_AUX_MAIN");
            ///按键使能
            sys_key_event_enable();
            //关闭自动关机
            sys_auto_shut_down_disable();
            app_aux_init();
            g_aux.idle = 1;		//防止进入低功耗
            break;
        }
        break;
    case APP_STA_PAUSE:
        log_info("APP_STA_PAUSE");
        app_aux_exit();
        break;
    case APP_STA_RESUME:
        log_info("APP_STA_RESUME");
        app_aux_init();
        break;
    case APP_STA_STOP:
        log_info("APP_STA_STOP");
        app_aux_exit();
        break;
    case APP_STA_DESTROY:
        log_info("APP_STA_DESTROY");
        break;
    }

    return 0;
}

static const struct application_operation app_aux_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

/*
 * 注册aux模式
 */
REGISTER_APPLICATION(app_aux) = {
    .name 	= "aux",
    .action	= ACTION_AUX_MAIN,
    .ops 	= &app_aux_ops,
    .state  = APP_STA_DESTROY,
};

#endif

