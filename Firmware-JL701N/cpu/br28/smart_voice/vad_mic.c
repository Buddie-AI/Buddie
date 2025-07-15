/*****************************************************************
>file name : vad_mic.c
>create time : Fri 15 Apr 2022 10:27:55 AM CST
*****************************************************************/
#include "smart_voice.h"
#include "vad_mic.h"
#include "asm/efuse.h"
#include "voice_mic_data.h"
#include "update/update.h"
struct low_power_vad_mic {
    void *priv;
    int (*dma_output)(void *, s16 *, int);
};

extern const int config_lp_vad_enable;
struct low_power_vad_mic *lp_vad = NULL;
static DEFINE_SPINLOCK(lp_vad_lock);
//===========================================================================//
//                               AUDIO_VAD                                   //
//===========================================================================//
#define AUDIO_VAD_CBUF_ADDR         VAD_CBUF_BEGIN

static void p11_vad_mic_dma_irq_handler(void)
{
    int buffered_frames = P11_LPVAD->DMA_SHN / VOICE_MIC_DATA_PERIOD_FRAMES * VOICE_MIC_DATA_PERIOD_FRAMES;
    int buffered_bytes = buffered_frames * VOICE_ADC_SAMPLE_CH * 2;
    u8 *read_ptr = (u8 *)((s16 *)AUDIO_VAD_CBUF_ADDR + P11_LPVAD->DMA_SPTR * VOICE_ADC_SAMPLE_CH);
    int write_len = 0;

    if (P11_LPVAD->DMA_SPTR + buffered_frames > P11_LPVAD->DMA_LEN) {
        int read_len = (P11_LPVAD->DMA_LEN - P11_LPVAD->DMA_SPTR) * VOICE_ADC_SAMPLE_CH * 2;
        spin_lock(&lp_vad_lock);
        if (lp_vad && lp_vad->dma_output) {
            lp_vad->dma_output(lp_vad->priv, (s16 *)read_ptr, read_len);
        }
        spin_unlock(&lp_vad_lock);
        read_ptr = (u8 *)AUDIO_VAD_CBUF_ADDR;
        buffered_bytes -= read_len;
    }
    spin_lock(&lp_vad_lock);
    if (lp_vad && lp_vad->dma_output) {
        lp_vad->dma_output(lp_vad->priv, (s16 *)read_ptr, buffered_bytes);
    }
    spin_unlock(&lp_vad_lock);
    /*更新P11 LPVAD读指针：通过p11传递，不可直接设置硬件*/
    AUDIO_VAD_DMA_READ_UPDATE(buffered_frames / VOICE_MIC_DATA_PERIOD_FRAMES);
}

/*static u8 p2m_active = 0;*/
/*
 * Event from IRQ
 *
 */
void audio_vad_coprocessor_event_handler(int event)
{
    if (!config_lp_vad_enable) {
        return;
    }

    int msg = SMART_VOICE_MSG_STANDBY;
    switch (event) {
    case P2M_VAD_TRIGGER_START:
        /*p2m_active = 1;*/
        msg = SMART_VOICE_MSG_WAKE;
        p11_vad_mic_dma_irq_handler();
        break;
    case P2M_VAD_TRIGGER_DMA:
        msg = SMART_VOICE_MSG_DMA;
        p11_vad_mic_dma_irq_handler();
        break;
    case P2M_VAD_TRIGGER_STOP:
        msg = SMART_VOICE_MSG_STANDBY;
        /*p2m_active = 0;*/
        break;
    default:
        break;
    }

    smart_voice_core_post_msg(1, msg);
}
/*
 * 由主系统P11响应中断调用
 */
void audio_vad_p2mevent_irq_handler(void)
{
    audio_vad_coprocessor_event_handler(P2M_MESSAGE_VAD_CMD);
}

static void lp_vad_mic_in_enable(struct vad_mic_platform_data *data)
{
    if (!config_lp_vad_enable) {
        return;
    }
    gpio_set_direction(IO_PORTA_01, 1);
    gpio_set_die(IO_PORTA_01, 0);
    gpio_set_pull_up(IO_PORTA_01, 0);
    gpio_set_pull_down(IO_PORTA_01, 0);
    if (data->mic_data.mic_mode == AUDIO_MIC_CAP_DIFF_MODE || data->mic_data.mic_bias_inside) {
        gpio_set_direction(IO_PORTA_02, 1);
        gpio_set_die(IO_PORTA_02, 0);
        gpio_set_pull_up(IO_PORTA_02, 0);
        gpio_set_pull_down(IO_PORTA_02, 0);
    }
}

int lp_vad_mic_data_init(struct vad_mic_platform_data *mic_data)
{
    struct vad_mic_platform_data *data = (struct vad_mic_platform_data *)(VAD_AVAD_CONFIG_BEGIN + sizeof(struct avad_config));

    memcpy(data, mic_data, sizeof(struct vad_mic_platform_data));

    u8 vbg_trim = get_vad_vbg_trim();
    if (vbg_trim != 0xf) {
        data->power_data.acm_select = get_vad_vbg_trim();
    }
    lp_vad_mic_in_enable(mic_data);
    return 0;
}

void *lp_vad_mic_open(void *priv, int (*dma_output)(void *priv, s16 *data, int len))
{
    if (!config_lp_vad_enable) {
        return NULL;
    }
    if (!lp_vad) {
        lp_vad = zalloc(sizeof(struct low_power_vad_mic));
        if (!lp_vad) {
            return NULL;
        }
    }
    P11_VAD_IRQ_ENABLE();
    /*
     * P11 VAD初始化
     */
    struct avad_config *avad_cfg = (struct avad_config *)VAD_AVAD_CONFIG_BEGIN;
    struct dvad_config *dvad_cfg = (struct dvad_config *)VAD_DVAD_CONFIG_BEGIN;

    //=================================//
    //       AVAD 效果参数配置         //
    //=================================//
    avad_cfg->avad_quantile_p = 3; //0.8
    avad_cfg->avad_gain_db = 10;
    avad_cfg->avad_compare_v = 3;

    //=================================//
    //       DVAD 效果参数配置         //
    //=================================//
    dvad_cfg->dvad_gain_id = 10;
    dvad_cfg->d2a_th_db = 20;
    dvad_cfg->d_frame_con = 100;
    dvad_cfg->d2a_frame_con = 100;
    dvad_cfg->d_stride1 = 3;//<<7
    dvad_cfg->d_stride2 = 5;//<<7
    dvad_cfg->d_low_con_th = 6;
    dvad_cfg->d_high_con_th = 3;

    printf("avad_cfg @ 0x%x, dvad_cfg @ 0x%x", (u32)avad_cfg, (u32)dvad_cfg);

    lp_vad->priv = priv;
    lp_vad->dma_output = dma_output;
    audio_vad_m2p_event_post(M2P_VAD_CMD_INIT);

    return lp_vad;
}

void lp_vad_mic_disable(void)
{
    audio_vad_m2p_event_post(M2P_VAD_CMD_CLOSE);
}

void lp_vad_mic_close(void *vad)
{
    if (!config_lp_vad_enable) {
        return;
    }
    if (!vad) {
        return;
    }
    lp_vad_mic_disable();
    spin_lock(&lp_vad_lock);
    if (vad) {
        free(vad);
    }
    lp_vad = NULL;
    spin_unlock(&lp_vad_lock);
}

void lp_vad_mic_test(void)
{
    audio_vad_m2p_event_post(M2P_VAD_CMD_TEST);
}

u8 vad_disable(void)
{
    lp_vad_mic_disable();
    return 0;
}

REGISTER_UPDATE_TARGET(vad_update_target) = {
    .name = "vad",
    .driver_close = vad_disable,
};

