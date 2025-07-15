#include "generic/typedef.h"
#include "board_config.h"
#include "media/includes.h"
#include "audio_config.h"
#include "sound_device.h"
#include "audio_sidetone.h"
#include "system/task.h"

#if TCFG_SIDETONE_ENABLE

extern struct audio_dac_hdl dac_hdl;
#define SIDETONE_BUF_LEN        256
#define SIDETONE_READBUF_LEN    64                //中断点数设为32，则每次数据长度为64
static struct audio_sidetone_hdl {
    s16 *sidetone_buf;
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
    s16 *sidetone_buf_lr;
#endif/*(TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)*/
    cbuffer_t cbuf;
    OS_SEM sem;
    bool busy;              //检测任务是否在阻塞态
    bool suspend;           //暂停监听
    struct audio_dac_channel dac_ch;
    struct audio_dac_channel_attr attr;
};
static struct audio_sidetone_hdl *sidetone_hdl = NULL;

static inline void audio_pcm_mono_to_dual(s16 *dual_pcm, s16 *mono_pcm, int points)
{
    s16 *mono = mono_pcm;
    int i = 0;
    u8 j = 0;

    for (i = 0; i < points; i++, mono++) {
        *dual_pcm++ = *mono;
        *dual_pcm++ = *mono;
    }
}

static void audio_sidetone_task(void)
{
    if (!sidetone_hdl) {
        sidetone_hdl = zalloc(sizeof(struct audio_sidetone_hdl));
        if (!sidetone_hdl) {
            printf("zalloc sidetone_hdl err\n");
            return;
        }
    }
    sidetone_hdl->suspend = 1;
    sidetone_hdl->sidetone_buf = zalloc(SIDETONE_BUF_LEN);
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
    sidetone_hdl->sidetone_buf_lr = zalloc(SIDETONE_READBUF_LEN * 2);
#endif/*(TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)*/
    cbuf_init(&sidetone_hdl->cbuf, sidetone_hdl->sidetone_buf, SIDETONE_BUF_LEN);
    os_sem_create(&sidetone_hdl->sem, 0);

    audio_dac_new_channel(&dac_hdl, &sidetone_hdl->dac_ch);
    sidetone_hdl->attr.delay_time = 6;
    sidetone_hdl->attr.protect_time = 8;
    sidetone_hdl->attr.write_mode = WRITE_MODE_BLOCK;
    audio_dac_channel_set_attr(&sidetone_hdl->dac_ch, &sidetone_hdl->attr);
    sound_pcm_dev_start(&sidetone_hdl->dac_ch, 16000, app_audio_get_volume(APP_AUDIO_STATE_CALL));

    while (1) {
        sidetone_hdl->busy = 0;
        os_sem_pend(&sidetone_hdl->sem, 0);
        sidetone_hdl->busy = 1;
        u16 rlen = cbuf_read(&sidetone_hdl->cbuf, sidetone_hdl->sidetone_buf, SIDETONE_READBUF_LEN);
        if (rlen != SIDETONE_READBUF_LEN) {
            printf("rlen err : %d\n", rlen);
        }
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
        audio_pcm_mono_to_dual(sidetone_hdl->sidetone_buf_lr, sidetone_hdl->sidetone_buf, rlen >> 1);
        u16 wlen = sound_pcm_dev_write(&sidetone_hdl->dac_ch, sidetone_hdl->sidetone_buf_lr, rlen <<= 1);
#else
        u16 wlen = sound_pcm_dev_write(&sidetone_hdl->dac_ch, sidetone_hdl->sidetone_buf, rlen);
#endif
        if (wlen != rlen) {
            printf("wlen err : %d\n", wlen);
        }
    }
}

/*
*********************************************************************
*                  Audio Sidetone Inbuf
* Description: 通话监听数据流输入
* Arguments  : data 输入数据地址
*              len  输入数据长度
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_sidetone_inbuf(s16 *data, u16 len)
{
    if (sidetone_hdl && sidetone_hdl->suspend) {
        os_sem_post(&sidetone_hdl->sem);
        u16 wlen = cbuf_write(&sidetone_hdl->cbuf, data, len);
        if (wlen != len) {
            printf("wlen = %d, len = %d\n", wlen, len);
        }
    }
}

/*
*********************************************************************
*                  Audio Sidetone Open
* Description: 打开通话监听
* Arguments  : None.
* Return	 : 0成功 其他失败
* Note(s)    : None.
*********************************************************************
*/
int audio_sidetone_open(void)
{
    if (!sidetone_hdl) {
        task_create(audio_sidetone_task, NULL, "sidetone");//创建监听任务
        return 0;
    }
    return -1;
}

/*
*********************************************************************
*                  Audio Sidetone Close
* Description: 关闭通话监听
* Arguments  : None.
* Return	 : 0成功 其他失败
* Note(s)    : None.
*********************************************************************
*/
int audio_sidetone_close(void)
{
    if (!sidetone_hdl) {
        printf("sidetone already close\n");
        return -1;
    }
    if (!sidetone_hdl->busy) {                      //任务处于挂起态
        sound_pcm_dev_stop(&sidetone_hdl->dac_ch);  //关闭监听
        task_kill("sidetone");
        free(sidetone_hdl->sidetone_buf);
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
        free(sidetone_hdl->sidetone_buf_lr);
#endif/*(TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)*/
        free(sidetone_hdl);
        sidetone_hdl = NULL;
        return 0;
    }
    return -2;
}

/*
*********************************************************************
*                  Audio Sidetone Suspend
* Description: 暂停通话监听
* Arguments  : None.
* Return	 : 0成功 其他失败
* Note(s)    : None.
*********************************************************************
*/
int audio_sidetone_suspend(void)
{
    if (sidetone_hdl) {
        sidetone_hdl->suspend ? (sidetone_hdl->suspend = 0) : (sidetone_hdl->suspend = 1);
        return 0;
    }
    return -1;
}
#endif/*TCFG_SIDETONE_ENABLE*/
