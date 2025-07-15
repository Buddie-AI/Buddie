
#define IIS_TX_MIN_PNS      (JL_ALNK0->LEN / 4)
struct audio_iis_sync_node {
    void *hdl;
    struct list_head entry;
};
/***********************************************************
 * i2s 使用接口
 *
 ***********************************************************/
int audio_iis_buffered_frames(struct audio_iis_hdl *iis)
{
    return (JL_ALNK0->LEN - *ALNK0_SHN[iis->hw_ch] - 1);
}

int audio_iis_buffered_time(struct audio_iis_hdl *iis)
{
    if (!iis) {
        return 0;
    }

    int buffered_time = ((audio_iis_buffered_frames(iis) * 1000000) / iis->sample_rate) / 1000;

    return buffered_time;
}

int audio_iis_set_underrun_params(struct audio_iis_hdl *iis, int time, void *priv, void (*feedback)(void *))
{
    local_irq_disable();
    iis->underrun_time = time;
    iis->underrun_pns = 0;
    iis->underrun_data = priv;
    iis->underrun_feedback = feedback;
    local_irq_enable();

    return 0;
}

void audio_iis_syncts_update_frame(struct audio_iis_hdl *iis)
{
    struct audio_iis_sync_node *node;
    list_for_each_entry(node, &iis->sync_list, entry) {
        sound_pcm_enter_update_frame(node->hdl);
    }
}

void audio_iis_syncts_latch_trigger(struct audio_iis_hdl *iis)
{
    struct audio_iis_sync_node *node;

    list_for_each_entry(node, &iis->sync_list, entry) {
        sound_pcm_syncts_latch_trigger(node->hdl);
    }
}


void audio_iis_dma_update_to_syncts(struct audio_iis_hdl *iis, int frames)
{
    struct audio_iis_sync_node *node;
    u8 have_syncts = 0;

    list_for_each_entry(node, &iis->sync_list, entry) {
        sound_pcm_update_frame_num(node->hdl, frames);
        have_syncts = 1;
    }

    if (have_syncts) {
        u16 free_points = *ALNK0_SHN[iis->hw_ch];
        int timeout = (1000000 / iis->sample_rate) * (clk_get("sys") / 1000000);
        while (free_points == *ALNK0_SHN[iis->hw_ch] && (--timeout > 0));
    }
}

void audio_iis_add_syncts_handle(struct audio_iis_hdl *iis, void *syncts)
{
    struct audio_iis_sync_node *node = (struct audio_iis_sync_node *)zalloc(sizeof(struct audio_iis_sync_node));
    node->hdl = syncts;

    list_add(&node->entry, &iis->sync_list);

    if (iis->state == SOUND_PCM_STATE_RUNNING) {
        sound_pcm_syncts_latch_trigger(syncts);
    }
    ALINK_DA2BTSRC_SEL(ALINK0, iis->hw_ch);
}

void audio_iis_remove_syncts_handle(struct audio_iis_hdl *iis, void *syncts)
{
    struct audio_iis_sync_node *node;

    list_for_each_entry(node, &iis->sync_list, entry) {
        if (node->hdl == syncts) {
            goto remove_node;
        }
    }

    return;
remove_node:

    list_del(&node->entry);
    free(node);
}

static void audio_iis_tx_irq_handler(struct audio_iis_hdl *iis)
{
    if (iis->irq_trigger && iis->trigger_handler) {
        iis->trigger_handler(iis->trigger_data);
        iis->irq_trigger = 0;
    }

    if (iis->underrun_pns) {
        int unread_frames = JL_ALNK0->LEN - *ALNK0_SHN[iis->hw_ch] - 1;
        if (unread_frames <= iis->underrun_pns) {
            //TODO
        } else {
            ALINK_OPNS_SET(ALINK0, iis->underrun_pns);
            ALINK_CLR_CHx_PND(ALINK0, iis->hw_ch);
            return;
        }
    }

    ALINK_CHx_IE(ALINK0, iis->hw_ch, 0);
    ALINK_CLR_CHx_PND(ALINK0, iis->hw_ch);
}

extern ALINK_PARM alink0_platform_data;
int audio_iis_pcm_tx_open(struct audio_iis_hdl *iis, u8 ch, int sample_rate)
{
    memset(iis, 0x0, sizeof(struct audio_iis_hdl));
    INIT_LIST_HEAD(&iis->sync_list);
    iis->state = SOUND_PCM_STATE_IDLE;
    iis->sample_rate = sample_rate;
    iis->alink0_param = alink_init(&alink0_platform_data);
    alink_channel_init(iis->alink0_param, ch, ALINK_DIR_TX, iis, audio_iis_tx_irq_handler);
    /*alink_start();*/
    iis->hw_ch = ch;
}

void audio_iis_pcm_tx_close(struct audio_iis_hdl *iis)
{
    alink_channel_close(iis->hw_ch);
    iis->state = SOUND_PCM_STATE_IDLE;
}

int audio_iis_pcm_sample_rate(struct audio_iis_hdl *iis)
{
    return iis->sample_rate;
}

int audio_iis_set_delay_time(struct audio_iis_hdl *iis, int prepared_time, int delay_time)
{
    iis->prepared_time = prepared_time;
    iis->delay_time = delay_time;
    return 0;
}

int audio_iis_pcm_remapping(struct audio_iis_hdl *iis, u8 mapping)
{
    if (!iis->input_mapping) {
        iis->input_mapping = mapping;
    }
    if ((iis->input_mapping & SOUND_CHMAP_RL) || (iis->input_mapping & SOUND_CHMAP_RR)) {
        printf("Not support this channel map : 0x%x\n", mapping);
    }
    return 0;
}

int audio_iis_pcm_channel_num(struct audio_iis_hdl *iis)
{
    int num = 0;
    for (int i = 0; i < 2; i++) {
        if (iis->input_mapping & BIT(i)) {
            num++;
        }
    }
    return num;
}

#if 0
const unsigned char sin44K[88] ALIGNED(4) = {
    0x00, 0x00, 0x45, 0x0E, 0x41, 0x1C, 0xAA, 0x29, 0x3B, 0x36, 0xB2, 0x41, 0xD5, 0x4B, 0x6E, 0x54,
    0x51, 0x5B, 0x5A, 0x60, 0x70, 0x63, 0x82, 0x64, 0x8A, 0x63, 0x8E, 0x60, 0x9D, 0x5B, 0xD1, 0x54,
    0x4D, 0x4C, 0x3D, 0x42, 0xD5, 0x36, 0x50, 0x2A, 0xF1, 0x1C, 0xFB, 0x0E, 0xB7, 0x00, 0x70, 0xF2,
    0x6E, 0xE4, 0xFD, 0xD6, 0x60, 0xCA, 0xD9, 0xBE, 0xA5, 0xB4, 0xF7, 0xAB, 0xFC, 0xA4, 0xDA, 0x9F,
    0xAB, 0x9C, 0x7F, 0x9B, 0x5E, 0x9C, 0x3F, 0x9F, 0x19, 0xA4, 0xCE, 0xAA, 0x3D, 0xB3, 0x3A, 0xBD,
    0x92, 0xC8, 0x0A, 0xD5, 0x60, 0xE2, 0x50, 0xF0
};

int read_44k_sine_data(void *buf, int bytes, int offset, u8 channel)
{
    s16 *sine = (s16 *)sin44K;
    s16 *data = (s16 *)buf;
    int frame_len = (bytes >> 1) / channel;
    int sin44k_frame_len = sizeof(sin44K) / 2;
    int i, j;

    offset = offset % sin44k_frame_len;

    for (i = 0; i < frame_len; i++) {
        for (j = 0; j < channel; j++) {
            *data++ = sine[offset];
        }
        if (++offset >= sin44k_frame_len) {
            offset = 0;
        }
    }

    return i * 2 * channel;
}
static int frames_offset = 0;
#endif

static int __audio_iis_pcm_write(s16 *dst, s16 *src, int frames, int remapping)
{

    /* int frame_bytes = read_44k_sine_data(dst, frames * 2 * 2, frames_offset, 2); */
    /* putchar('k'); */
    /* frames_offset += (frame_bytes >> 1) / 2; */
    /* return frame_bytes; */

    if (remapping == 1) {
        for (int i = 0; i < frames; i++) {
            dst[i * 2] = src[i];
            dst[i * 2 + 1] = src[i];
        }
        return frames << 1;
    }

    if (remapping == 2) {
        memcpy(dst, src, (frames << 1) * 2);
        return frames << 2;
    }

    return frames;
}

#define CVP_REF_SRC_ENABLE
#ifdef CVP_REF_SRC_ENABLE
#define CVP_REF_SRC_TASK_NAME "RefSrcTask"
#include "Resample_api.h"
#include "aec_user.h"

#define CVP_REF_SRC_FRAME_SIZE  512
typedef struct {
    volatile u8 state;
    volatile u8 busy;
    RS_STUCT_API *sw_src_api;
    u8 *sw_src_buf;
    u16 input_rate;
    u16 output_rate;
    s16 ref_tmp_buf[CVP_REF_SRC_FRAME_SIZE / 2];
    cbuffer_t cbuf;
    u8 ref_buf[CVP_REF_SRC_FRAME_SIZE * 3];
} aec_ref_src_t;
static aec_ref_src_t *aec_ref_src = NULL;

extern void audio_aec_ref_src_get_output_rate(u16 *input_rate, u16 *output_rate);
extern u8 bt_phone_dec_is_running();

extern struct audio_iis_hdl iis_hdl;
void audio_aec_ref_src_run(s16 *data, int len)
{
    u16 ref_len = 0;
    if (aec_ref_src) {
        if (iis_hdl.input_mapping != SOUND_CHMAP_MONO) {
            /*双变单*/
            for (int i = 0; i < (len >> 2); i++) {
                aec_ref_src->ref_tmp_buf[i] =  data[2 * i];
            }
            len >>= 1;
        }
        /* audio_aec_ref_src_get_output_rate(&aec_ref_src->input_rate, &aec_ref_src->output_rate); */
        /* printf("%d %d \n",input_rate,output_rate); */
        if (aec_ref_src->sw_src_api) {
            /* aec_ref_src->sw_src_api->set_sr(aec_ref_src->sw_src_buf, aec_ref_src->output_rate); */
            ref_len = aec_ref_src->sw_src_api->run(aec_ref_src->sw_src_buf, aec_ref_src->ref_tmp_buf, len >> 1, aec_ref_src->ref_tmp_buf);
            ref_len <<= 1;
        }
        /* printf("ref_len %d", ref_len); */
        audio_aec_refbuf(aec_ref_src->ref_tmp_buf, ref_len);
    }
}

static void audio_aec_ref_src_task(void *p)
{
    int res;
    int msg[16];
    while (1) {

        res = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));

        if (aec_ref_src && aec_ref_src->state) {
            s16 *data = (int)msg[1];
            int len = msg[2];
            int rlen = 0;

            aec_ref_src->busy = 1;
            if (cbuf_get_data_len(&aec_ref_src->cbuf) >= CVP_REF_SRC_FRAME_SIZE) {
                cbuf_read(&aec_ref_src->cbuf, aec_ref_src->ref_tmp_buf, CVP_REF_SRC_FRAME_SIZE);
                audio_aec_ref_src_run(aec_ref_src->ref_tmp_buf, CVP_REF_SRC_FRAME_SIZE);
            }
            aec_ref_src->busy = 0;
        }
    }
}

int audio_aec_ref_src_data_fill(void *p, s16 *data, int len)
{
    int ret = 0;
    if (aec_ref_src && aec_ref_src->state) {
        audio_aec_ref_start(1);
        if (0 == cbuf_write(&aec_ref_src->cbuf, data, len)) {
            /* cbuf_clear(&aec_ref_src->cbuf); */
            printf("ref src cbuf wfail!!");
        }
        if (cbuf_get_data_len(&aec_ref_src->cbuf) >= CVP_REF_SRC_FRAME_SIZE) {
            ret = os_taskq_post_msg(CVP_REF_SRC_TASK_NAME, 2, (int)data, len);
        }
    }
    return ret;
}

int audio_aec_ref_src_open(u32 insr, u32 outsr)
{
    if (aec_ref_src) {
        printf("aec_ref_src alreadly open !!!");
        return -1;
    }
    aec_ref_src = zalloc(sizeof(aec_ref_src_t));
    if (aec_ref_src == NULL) {
        printf("aec_ref_src malloc fail !!!");
        return -1;
    }

    cbuf_init(&aec_ref_src->cbuf, aec_ref_src->ref_buf, sizeof(aec_ref_src->ref_buf));
    int err = os_task_create(audio_aec_ref_src_task, NULL, 4, 256, 128, CVP_REF_SRC_TASK_NAME);
    if (err != OS_NO_ERR) {
        printf("task create error!");
        free(aec_ref_src);
        aec_ref_src = NULL;
        return -1;
    }

    /* audio_aec_ref_src_get_output_rate(&aec_ref_src->input_rate, &aec_ref_src->output_rate); */
    aec_ref_src->input_rate = insr;
    aec_ref_src->output_rate = outsr;
    aec_ref_src->sw_src_api = get_rs16_context();
    printf("sw_src_api:0x%x\n", aec_ref_src->sw_src_api);
    ASSERT(aec_ref_src->sw_src_api);
    int sw_src_need_buf = aec_ref_src->sw_src_api->need_buf();
    printf("sw_src_buf:%d\n", sw_src_need_buf);
    aec_ref_src->sw_src_buf = zalloc(sw_src_need_buf);
    ASSERT(aec_ref_src->sw_src_buf, "sw_src_buf zalloc fail");
    RS_PARA_STRUCT rs_para_obj;
    rs_para_obj.nch = 1;
    if (insr == 44100) {
        rs_para_obj.new_insample = 44117;
    } else {
        rs_para_obj.new_insample = insr;
    }
    rs_para_obj.new_outsample = outsr;
    printf("sw src,ch = %d, in = %d,out = %d\n", rs_para_obj.nch, rs_para_obj.new_insample, rs_para_obj.new_outsample);
    aec_ref_src->sw_src_api->open(aec_ref_src->sw_src_buf, &rs_para_obj);

    aec_ref_src->state = 1;
    return 0;
}

void audio_aec_ref_src_close()
{
    if (aec_ref_src) {
        aec_ref_src->state = 0;
        while (aec_ref_src->busy) {
            putchar('w');
            os_time_dly(1);
        }
        int err = os_task_del(CVP_REF_SRC_TASK_NAME);
        if (err) {
            log_i("kill task %s: err=%d\n", CVP_REF_SRC_TASK_NAME, err);
        }
        if (aec_ref_src->sw_src_api) {
            aec_ref_src->sw_src_api = NULL;
        }
        if (aec_ref_src->sw_src_buf) {
            free(aec_ref_src->sw_src_buf);
            aec_ref_src->sw_src_buf = NULL;
        }
        free(aec_ref_src);
        aec_ref_src = NULL;
    }
}
#endif/*CVP_REF_SRC_ENABLE*/

int audio_iis_pcm_write(struct audio_iis_hdl *iis, void *data, int len)
{
    if (iis->state != SOUND_PCM_STATE_RUNNING) {
        return 0;
    }
    int remapping_ch = 2;
    int frames = 0;
    if (iis->input_mapping == SOUND_CHMAP_MONO) {
        frames = len >> 1;
        remapping_ch = 1;
    } else if (iis->input_mapping & SOUND_CHMAP_FR) {
        frames = len >> 2;
        remapping_ch = 2;
    } else {

    }

    s16 *ref_data = (s16 *)data;
    int swp = *ALNK0_SWPTR[iis->hw_ch];
    int free_frames = *ALNK0_SHN[iis->hw_ch] - iis->reserved_frames;

    /* printf("free : %d, %d\n", *ALNK0_SHN[iis->hw_ch], free_frames); */
    if (free_frames <= 0) {
        return 0;
    }

    if (free_frames > frames) {
        free_frames = frames;
    }

    frames = 0;
    if (swp + free_frames > JL_ALNK0->LEN) {
        frames = JL_ALNK0->LEN - swp;
        __audio_iis_pcm_write((s16 *)(*ALNK0_BUF_ADR[iis->hw_ch] + swp * 2 * 2), (s16 *)data, frames, remapping_ch);
        free_frames -= frames;
        data = (s16 *)data + frames * remapping_ch;
        swp = 0;
    }

    __audio_iis_pcm_write((s16 *)(*ALNK0_BUF_ADR[iis->hw_ch] + swp * 2 * 2), (s16 *)data, free_frames, remapping_ch);
    frames += free_frames;

    audio_iis_syncts_update_frame(iis);
    *ALNK0_SHN[iis->hw_ch] = frames;
    __asm_csync();
    audio_iis_dma_update_to_syncts(iis, frames);

    /*printf("frames : %d\n", frames);*/
    /*printf("CLK CON2 : %d, %d\n", JL_CLOCK->CLK_CON2 & 0x3, (JL_CLOCK->CLK_CON2 >> 2) & 0x3);*/

#ifdef CVP_REF_SRC_ENABLE
    if (bt_phone_dec_is_running()) {
        audio_aec_ref_src_data_fill(iis, ref_data, (frames << 1) * remapping_ch);
    }
#endif
    return (frames << 1) * remapping_ch;
}

static void audio_iis_dma_fifo_start(struct audio_iis_hdl *iis)
{
    if (iis->state != SOUND_PCM_STATE_PREPARED) {
        return;
    }

    if (iis->prepared_frames) {
        *ALNK0_SHN[iis->hw_ch] = iis->prepared_frames;
        __asm_csync();
    }

    local_irq_disable();
    iis->state = SOUND_PCM_STATE_RUNNING;
    ALINK_CHx_IE(ALINK0, iis->hw_ch, 1);
    ALINK_CLR_CHx_PND(ALINK0, iis->hw_ch);
    local_irq_enable();
    audio_iis_syncts_latch_trigger(iis);
}

static int audio_iis_fifo_set_delay(struct audio_iis_hdl *iis)
{
    iis->prepared_frames = iis->prepared_time * iis->sample_rate / 1000;
    int delay_frames = (iis->delay_time * iis->sample_rate) / 1000 / 2 * 2;
    if (iis->prepared_frames >= JL_ALNK0->LEN) {
        iis->prepared_frames = JL_ALNK0->LEN / 2;
    }
    if (delay_frames < JL_ALNK0->LEN) {
        iis->reserved_frames = JL_ALNK0->LEN - delay_frames;
    } else {
        iis->reserved_frames = 0;
    }
    return 0;
}

int audio_iis_trigger_interrupt(struct audio_iis_hdl *iis, int time_ms, void *priv, void (*callback)(void *))
{
    if (iis->state != SOUND_PCM_STATE_RUNNING) {
        return -EINVAL;
    }

    int irq_frames = time_ms * iis->sample_rate / 1000;
    int pns = audio_iis_buffered_frames(iis) - irq_frames;
    if (pns < irq_frames) {
        sys_hi_timeout_add(priv, callback, time_ms);
        return -EINVAL;
    }
    local_irq_disable();
    if (pns > ALINK_OPNS(ALINK0)) {
        ALINK_OPNS_SET(ALINK0, pns);
    }

    iis->irq_trigger = 1;
    iis->trigger_handler = callback;
    iis->trigger_data = priv;
    ALINK_CHx_IE(ALINK0, iis->hw_ch, 1);
    local_irq_enable();

    return 0;
}

int audio_iis_dma_start(struct audio_iis_hdl *iis)
{
    if (iis->state == SOUND_PCM_STATE_RUNNING) {
        return 0;
    }

    audio_iis_fifo_set_delay(iis);
    iis->underrun_pns = iis->underrun_time ? (iis->underrun_time * iis->sample_rate / 1000) : 0;
    ALINK_OPNS_SET(ALINK0, (iis->underrun_pns ? iis->underrun_pns : IIS_TX_MIN_PNS));
    iis->state = SOUND_PCM_STATE_PREPARED;
    /* printf("DMA : %d, %d, %d\n", *ALNK0_SHN[iis->hw_ch], *ALNK0_SWPTR[iis->hw_ch], *ALNK0_HWPTR[iis->hw_ch]); */
    audio_iis_dma_fifo_start(iis);
    alink_start(iis->alink0_param);
    /* printf("[iis dma start]... %d, free : %d, prepared_frames : %d, reserved_frames : %d\n", JL_ALNK0->LEN, *ALNK0_SHN[iis->hw_ch], iis->prepared_frames, iis->reserved_frames); */
}

int audio_iis_dma_stop(struct audio_iis_hdl *iis)
{
    if (iis->state != SOUND_PCM_STATE_RUNNING) {
        return 0;
    }
    /*audio_iis_buffered_frames_fade_out(iis, JL_ALNK0->LEN - *ALNK0_SHN[iis->hw_ch] - 1);*/
    ALINK_CHx_IE(ALINK0, iis->hw_ch, 0);
    ALINK_CLR_CHx_PND(ALINK0, iis->hw_ch);
    /* alink_uninit(iis->alink0_param); */

    iis->input_mapping = 0;
    iis->state = SOUND_PCM_STATE_SUSPENDED;
    return 0;
}


