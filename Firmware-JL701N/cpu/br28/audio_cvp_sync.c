#include "system/includes.h"
#include "media/includes.h"
#include "audio_config.h"

#if TCFG_AUDIO_CVP_SYNC

#if 0
#define cvp_sync_log	printf
#else
#define cvp_sync_log(...)
#endif

enum {
    CVP_SYNC_STA_NORMAL = 0,			//普通状态
    CVP_SYNC_STA_DOWNSAMPLING,			//下采样状态
    CVP_SYNC_STA_UPSAMPLING,			//上采样状态
};

#define AUDIO_CVP_SRCBUF_SIZE	672		//输入输出BUFFER长度
#define AUDIO_CVP_CATCH_LEN		AUDIO_CVP_SRCBUF_SIZE	//SRC 拼包BUF 长度, 注意每帧不能超过此长度
#define AUDIO_CVP_DETECT_TIME	1000	//定时检测周期

typedef struct {
    u8 ch_num;
    u8 src_break;
    u8 state;
    u8 det_flag;
    u8 cnt;
    u16 offset;
    u16 det_time_id;
    u32 target_sample_rate;				//目标采样率
    u32 sample_rate;					//采样率
    s16 src_inbuf[AUDIO_CVP_SRCBUF_SIZE / 2];
    s16 src_outbuf[AUDIO_CVP_SRCBUF_SIZE / 2];
    u8 frame_catch[AUDIO_CVP_CATCH_LEN];
    struct audio_src_handle *hw_src;	//SRC句柄
} audio_cvp_sync_t;

static audio_cvp_sync_t *hdl = NULL;

extern int lmp_private_get_esco_send_packet_len();
extern int audio_aec_sync_buffer_set(s16 *data, int len);
static int audio_cvp_sync_output_handler(void *priv, void *data, int len)
{
    if ((hdl->offset + len) > AUDIO_CVP_CATCH_LEN) {
        printf("cvp sync output buf full\n");
        hdl->src_break = 1;
    }
    //回调仅做拼包
    memcpy(hdl->frame_catch + hdl->offset, (u8 *)data, len);
    hdl->offset += len;
    /* if (hdl->cnt > 5) { */
    /* cvp_sync_log("sync len %d, offset %d\n", len, hdl->offset); */
    /* } */
    return len;
}

static void audio_cvp_sync_det_set(void *priv)
{
    if (hdl) {
        hdl->det_flag = 1;
    }
}

int audio_cvp_sync_open(u16 sr)
{
    if (!hdl) {
        hdl = zalloc(sizeof(audio_cvp_sync_t));
    }
    if (!hdl) {
        return 1;
    }
    hdl->hw_src = zalloc(sizeof(struct audio_src_handle));
    if (!hdl->hw_src) {
        return 1;
    }
    hdl->ch_num = 1;
    hdl->sample_rate = sr;
    hdl->target_sample_rate = sr;
    audio_hw_src_open(hdl->hw_src, hdl->ch_num, SRC_TYPE_RESAMPLE);
    audio_hw_src_set_output_buffer(hdl->hw_src, hdl->src_outbuf, AUDIO_CVP_SRCBUF_SIZE);
    audio_hw_src_set_input_buffer(hdl->hw_src, hdl->src_inbuf, AUDIO_CVP_SRCBUF_SIZE);
    audio_hw_src_set_rate(hdl->hw_src, hdl->sample_rate, hdl->target_sample_rate);
    audio_src_set_output_handler(hdl->hw_src, NULL, audio_cvp_sync_output_handler);
    hdl->det_time_id = sys_timer_add(NULL, audio_cvp_sync_det_set, AUDIO_CVP_DETECT_TIME);
    return 0;
}

int audio_cvp_sync_run(s16 *data, int len)
{
    int wlen = 0;
    int btlen = lmp_private_get_esco_send_packet_len();

    if (!hdl) {
        return 0;
    }
    if (hdl->det_flag) {
        if (btlen > 360) {
            if (hdl->state != CVP_SYNC_STA_UPSAMPLING) {
                hdl->state = CVP_SYNC_STA_UPSAMPLING;
                hdl->cnt = 0;
            }
            if (++hdl->cnt > 5) {
                audio_hw_src_set_rate(hdl->hw_src, hdl->sample_rate, hdl->target_sample_rate - 1);
                cvp_sync_log("cvp_src_upsr btlen %d, cnt %d, len %d\n", btlen, hdl->cnt, len);
            }
        } else if (btlen < 120) {
            if (hdl->state != CVP_SYNC_STA_DOWNSAMPLING) {
                hdl->state = CVP_SYNC_STA_DOWNSAMPLING;
                hdl->cnt = 0;
            }
            if (++hdl->cnt > 5) {
                audio_hw_src_set_rate(hdl->hw_src, hdl->sample_rate, hdl->target_sample_rate + 1);
                cvp_sync_log("cvp_src_downsr btlen %d, cnt %d, len %d\n", btlen, hdl->cnt, len);
            }
        } else {
            hdl->state = CVP_SYNC_STA_NORMAL;
            hdl->det_flag = 0;
            hdl->cnt = 0;
        }
    }

#if 0 //测试模块
    hdl->cnt = 6;
    static int max_btlen = 0;
    audio_hw_src_set_rate(hdl->hw_src, hdl->sample_rate, hdl->target_sample_rate + 10);
    if (btlen > max_btlen) {
        max_btlen = btlen;
        cvp_sync_log("btlen %d, len %d \n", max_btlen, len);
    }
#endif/*测试模块*/
    if (hdl->hw_src) {
        while (len) {
            hdl->offset = 0;
            wlen = audio_src_resample_write(hdl->hw_src, data, len);
            audio_resample_hw_push_data_out(hdl->hw_src);	//执行完则表示当前帧的数据全部输出完毕
            audio_aec_sync_buffer_set((s16 *)hdl->frame_catch, hdl->offset);	//输出到CVP
            data += wlen / 2;
            len -= wlen;
            if (hdl->src_break) {
                hdl->src_break = 0;
                break;
            }
        }
    }
    return len;
}

int audio_cvp_sync_close()
{
    if (hdl->hw_src) {
        audio_hw_src_stop(hdl->hw_src);
        audio_hw_src_close(hdl->hw_src);
        sys_timer_del(hdl->det_time_id);
        free(hdl->hw_src);
        free(hdl);
        hdl = NULL;
    }
    return 0;
}
#endif/*TCFG_AUDIO_CVP_SYNC*/
