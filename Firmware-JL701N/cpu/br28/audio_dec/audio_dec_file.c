/*
 ****************************************************************
 *File : audio_dec_file.c
 *Note :
 *
 ****************************************************************
 */
//////////////////////////////////////////////////////////////////////////////
#include "asm/includes.h"
#include "media/includes.h"
#include "media/file_decoder.h"
#include "system/includes.h"
#include "effectrs_sync.h"
#include "app_config.h"
#include "audio_config.h"
#include "audio_dec.h"
#include "app_config.h"
#include "app_main.h"
#include "classic/tws_api.h"
#include "audio_dec/audio_dec_file.h"
#include "media/audio_stream.h"
#include "music/music_decrypt.h"
#include "music/music_id3.h"
#include "application/eq_config.h"
#include "audio_dec_eff.h"
#include "bt_tws.h"
#include "media/bt_audio_timestamp.h"
#include "media/audio_syncts.h"
#include "audio_codec_clock.h"
#include "sound_device.h"
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
#include "audio_dvol.h"
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

#if TCFG_APP_MUSIC_EN
#define FILE_DEC_PICK_EN			0 // 本地解码拆包转发

#define FILE_DEC_ALIGN              0
#define FILE_DEC_START              1
#define FILE_DEC_INVITE             2

#if (!TCFG_DEC2TWS_ENABLE)
#undef FILE_DEC_PICK_EN
#define FILE_DEC_PICK_EN			0
#endif

#ifndef BREAKPOINT_DATA_LEN
#define BREAKPOINT_DATA_LEN			32
#endif

const int FILE_DEC_ONCE_OUT_NUM	= ((512 * 4) * 2);	// 一次最多输出长度。避免多解码叠加时卡住其他解码太长时间

const int FILE_DEC_PP_FADE_MS = 50;	// pp淡入淡出时长。0-不使用淡入淡出

struct file_dec_hdl *file_dec = NULL;	// 文件解码句柄
u8 file_dec_start_pause = 0;	// 启动解码后但不马上开始播放
extern struct audio_dac_hdl dac_hdl;


//////////////////////////////////////////////////////////////////////////////

void file_eq_drc_open(struct file_dec_hdl *hdl, u16 sample_rate, u8 ch_num);
void file_eq_drc_close(struct file_dec_hdl *hdl);

extern void *local_tws_dec_sync_open(u8 channel, u16 sample_rate, u16 output_rate);
extern void local_tws_sync_no_check_data_buf(u8 no_check);
extern void audio_mix_ch_event_handler(void *priv, int event);
extern void bt_audio_sync_nettime_select(u8 base);

int file_dec_repeat_set(u8 repeat_num);

//////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------------------------------------*/
/**@brief    获取文件解码hdl
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void *get_file_dec_hdl()
{
    return file_dec;
}

static enum audio_channel file_dec_get_output_channel()
{
    enum audio_channel channel;

#if TCFG_USER_TWS_ENABLE
    int state = tws_api_get_tws_state();
    if (state & TWS_STA_SIBLING_CONNECTED) {
        channel = tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R;
    } else {
        u8 dac_connect_mode = audio_dac_get_channel(&dac_hdl);
        if (dac_connect_mode == DAC_OUTPUT_LR) {
            channel = AUDIO_CH_LR;
        } else {
            channel = AUDIO_CH_DIFF;
        }
    }
#else
    u8 dac_connect_mode = audio_dac_get_channel(&dac_hdl);
    if (dac_connect_mode == DAC_OUTPUT_LR) {
        channel = AUDIO_CH_LR;
    } else {
        channel = AUDIO_CH_DIFF;
    }
#endif
    return channel;
}

static u8 file_dec_output_channel_num(void)
{
    u8 dac_connect_mode = audio_dac_get_channel(&dac_hdl);
    if (dac_connect_mode == AUDIO_CH_LR) {
        return 2;
    }
    return 1;
}
/*----------------------------------------------------------------------------*/
/**@brief    读取文件数据
   @param    *decoder: 解码器句柄
   @param    *buf: 数据
   @param    len: 数据长度
   @return   >=0：读到的数据长度
   @return   <0：错误
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_fread(struct audio_decoder *decoder, void *buf, u32 len)
{
    int rlen;
    struct file_dec_hdl *dec = file_dec;

#if TCFG_DEC_DECRYPT_ENABLE
    u32 addr;
    addr = fpos(dec->file);
    rlen = fread(dec->file, buf, len);
    if (rlen && (rlen <= len)) {
        cryptanalysis_buff(&dec->mply_cipher, buf, addr, rlen);
    }
#else
    rlen = fread(dec->file, buf, len);
#endif
    if (rlen > len) {
        // putchar('r');
        if (rlen == (-1)) {
            //file err
            dec->read_err = 1;
        } else {
            //dis err
            dec->read_err = 2;
        }
        rlen = 0;
    } else {
        // putchar('R');
        dec->read_err = 0;
    }
    return rlen;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件指针定位
   @param    *decoder: 解码器句柄
   @param    offset: 定位偏移
   @param    seek_mode: 定位类型
   @return   0：成功
   @return   非0：错误
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_fseek(struct audio_decoder *decoder, u32 offset, int seek_mode)
{
    struct file_dec_hdl *dec = file_dec;

    return fseek(dec->file, offset, seek_mode);
}

/*----------------------------------------------------------------------------*/
/**@brief    读取文件长度
   @param    *decoder: 解码器句柄
   @return   文件长度
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_flen(struct audio_decoder *decoder)
{
    int len = 0;
    struct file_dec_hdl *dec = file_dec;

    len = flen(dec->file);
    return len;
}

static const u32 file_input_coding_more[] = {
#if TCFG_DEC_WAV_ENABLE
    AUDIO_CODING_WAV,
#endif
#if TCFG_DEC_MP3_ENABLE
    AUDIO_CODING_MP3,
#endif
    0,
};

static const struct audio_dec_input file_input = {
    .coding_type = 0
#if TCFG_DEC_WMA_ENABLE
    | AUDIO_CODING_WMA
#endif

#if TCFG_DEC_FLAC_ENABLE
    | AUDIO_CODING_FLAC
#endif
#if TCFG_DEC_APE_ENABLE
    | AUDIO_CODING_APE
#endif
#if TCFG_DEC_M4A_ENABLE
    | AUDIO_CODING_M4A
#endif
#if TCFG_DEC_ALAC_ENABLE
    | AUDIO_CODING_ALAC
#endif
#if TCFG_DEC_AMR_ENABLE
    | AUDIO_CODING_AMR
#endif
#if TCFG_DEC_DTS_ENABLE
    | AUDIO_CODING_DTS
#endif
#if TCFG_DEC_G726_ENABLE
    | AUDIO_CODING_G726
#endif

    ,
    .p_more_coding_type = (u32 *)file_input_coding_more,
    .data_type   = AUDIO_INPUT_FILE,
    .ops = {
        .file = {
            .fread = file_fread,
            .fseek = file_fseek,
            .flen  = file_flen,
        }
    }
};

#if TCFG_DEC2TWS_ENABLE

static inline u16 local_tws_audio_seqn(void *buf)
{
    u16 seqn;
    /*Get local tws audio sequence.*/
    if (!buf) {
        return 0;
    }

    memcpy(&seqn, buf, sizeof(seqn));
    return seqn;
}

static inline u16 local_tws_audio_timestamp(void *buf)
{
    u32 timestamp;
    /*Get local tws audio sequence.*/
    if (!buf) {
        return 0;
    }

    memcpy(&timestamp, buf, sizeof(timestamp));
    return timestamp;
}

/*static int frame_fread(struct audio_decoder *decoder, u8 **frame)*/
static int frame_fread(struct audio_decoder *decoder, void *buf, u32 len)
{
    int offset = 0;
    int frame_len;
    struct file_dec_hdl *dec = file_dec;

    while (len) {
        int clen = MIN(dec->frame_remain_len, len);
        if (clen) {
            memcpy((u8 *)buf + offset,
                   dec->frame + dec->frame_len - dec->frame_remain_len, clen);
            len     -= clen;
            offset  += clen;
            dec->frame_remain_len -= clen;
            break;
        } else {
            if (dec->frame) {
                tws_api_data_trans_free(dec->tws_channel, dec->frame);
            }
            dec->frame = tws_api_data_trans_pop(dec->tws_channel, &frame_len);
            if (!dec->frame) {
                dec->frame_remain_len = 0;
                break;
            }
#if 0
            u16 seqn = local_tws_audio_seqn(dec->frame);
            if ((u16)(seqn - dec->seqn) > 1) {
                log_w("Local tws audio sequence error : %d, %d.\n", seqn, dec->seqn);
            }
            dec->seqn = seqn;
#else
            u32 timestamp = local_tws_audio_timestamp(dec->frame);
            audio_syncts_next_pts(dec->syncts, timestamp);
            dec->mix_ch_event_params[2] = timestamp;
#endif
            dec->frame_len = frame_len;
            dec->frame_remain_len = frame_len - sizeof(dec->seqn);
        }
    }

    if (offset == 0) {
//        r_printf("no_data\n");
        if (dec->trans_dec_end) {
            return -EINVAL;
        }
    }

    return offset;
}

static int frame_fseek(struct audio_decoder *decoder, u32 offset, int seek_mode)
{
    return 0;
}

static int frame_flen(struct audio_decoder *decoder)
{
    return 0x7fffffff;
}

static const struct audio_dec_input frame_input = {
    .coding_type = 0,
    .data_type   = AUDIO_INPUT_FILE,
    .ops = {
        .file = {
            .fread = frame_fread,
            .fseek = frame_fseek,
            .flen  = frame_flen,
        }
    }
};
#endif



/*----------------------------------------------------------------------------*/
/**@brief    文件解码释放
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void file_dec_release(void)
{
    struct file_dec_hdl *dec = file_dec;

#if TCFG_DEC_ID3_V1_ENABLE
    if (dec->p_mp3_id3_v1) {
        id3_obj_post(&dec->p_mp3_id3_v1);
    }
#endif
#if TCFG_DEC_ID3_V2_ENABLE
    if (dec->p_mp3_id3_v2) {
        id3_obj_post(&dec->p_mp3_id3_v2);
    }
#endif

    audio_decoder_task_del_wait(&decode_task, &dec->wait);

    audio_codec_clock_del(AUDIO_FILE_MODE);

#if TCFG_DEC2TWS_ENABLE
    if (dec->tws_channel) {
        tws_api_data_trans_close(dec->tws_channel);
    }
#endif

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_close(MUSIC_DVOL);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/


    local_irq_disable();
    if (file_dec->dec_bp) {
        free(file_dec->dec_bp);
        file_dec->dec_bp = NULL;
    }
    free(file_dec);
    file_dec = NULL;
    local_irq_enable();
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码事件处理
   @param    *decoder: 解码器句柄
   @param    argc: 参数个数
   @param    *argv: 参数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void file_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    printf("file_dec_event_handler: %d\n", argv[0]);

    switch (argv[0]) {
    case AUDIO_DEC_EVENT_END:
        log_i("AUDIO_DEC_EVENT_END\n");
        if (!file_dec) {
            log_i("file_dec handle err ");
            break;
        }
#if TCFG_DEC2TWS_ENABLE
        if (decoder == &file_dec->trans_dec.decoder) {
            file_dec->trans_dec_end = 1;
            break;
        }
#endif
        if (decoder != &file_dec->file_dec.decoder) {
            break;
        }

        // 有回调，让上层close，避免close后上层读不到断点等
        if (file_dec->evt_cb) {
            /* file_dec->evt_cb(file_dec->evt_priv, argc, argv); */
            int msg[2];
            msg[0] = argv[0];
            msg[1] = file_dec->read_err;
            /* log_i("read err0:%d ", file_dec->read_err); */
            file_dec->evt_cb(file_dec->evt_priv, 2, msg);
        } else {
            file_dec_close();
        }
        break;
    default:
        break;
    }
}

static int file_decoder_output_after_syncts(void *priv, void *data, int len)
{
    struct file_dec_hdl *dec = (struct file_dec_hdl *)priv;
#if TCFG_EQ_ENABLE&&TCFG_MUSIC_MODE_EQ_ENABLE
    if (dec->eq_drc && dec->eq_drc->async) {
        return eq_drc_run(dec->eq_drc, data, len);//异步eq
    }
#endif//TCFG_MUSIC_MODE_EQ_ENABLE

    int wlen = audio_mixer_ch_write(&dec->mix_ch, data, len);
    return wlen;
}

static int file_decoder_output_hander(struct file_decoder *decoder, s16 *data, int len)
{
    int wlen = 0;

    struct file_dec_hdl *dec = container_of(decoder, struct file_dec_hdl, file_dec);

    if (!dec->remain) {
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
        audio_digital_vol_run(MUSIC_DVOL, data, len);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

#if TCFG_EQ_ENABLE&&TCFG_MUSIC_MODE_EQ_ENABLE
        if (dec->eq_drc && !dec->eq_drc->async) {
            eq_drc_run(dec->eq_drc, data, len);//同步eq
        }
#endif
    }
#if TCFG_DEC2TWS_ENABLE
    if (dec->syncts) {
        int wlen = audio_syncts_frame_filter(dec->syncts, data, len);
        if (wlen < len) {
            audio_syncts_trigger_resume(dec->syncts, (void *)decoder, (void (*)(void *))audio_decoder_resume);
        }
        dec->remain = wlen < len ? 1 : 0;
        return wlen;
    }
#endif
    wlen = file_decoder_output_after_syncts(dec, data, len);
    dec->remain = wlen < len ? 1 : 0;
    return  wlen;
}

static int tws_local_media_trans_handler(struct file_decoder *decoder, s16 *data, int len)
{
    /*int wlen = tws_api_local_media_trans_bulk_push((u8 *)data, len);*/
    struct file_dec_hdl *dec = container_of(decoder, struct file_dec_hdl, file_dec);
    int wlen = 0;
#if TCFG_DEC2TWS_ENABLE
    /*wlen = tws_api_local_media_push_with_sequence((u8 *)data, len, file_dec->tx_seqn);*/
    u32 timestamp = 0;
    if (dec->ts_handle) {
        timestamp = file_audio_timestamp_update(dec->ts_handle, dec->pcm_num);
    }
    wlen = tws_api_local_media_push_with_timestamp((u8 *)data, len, timestamp);
    if (wlen) {
        u16 pcm_num = 0;
        memcpy(&pcm_num, data, sizeof(pcm_num));
        /*printf("Num : %d x 32\n", pcm_num);*/
        dec->pcm_num += (pcm_num * 32);
        /*file_dec->tx_seqn++;*/
    }
#endif
    /*printf("trans: %d, %d\n", wlen, len);*/
    return wlen;
}

static int tws_file_decoder_syncts_setup(struct file_dec_hdl *dec)
{
    int err = 0;
#if TCFG_DEC2TWS_ENABLE
    struct audio_syncts_params params = {0};
    params.nch = file_dec_output_channel_num();
    params.pcm_device = PCM_INSIDE_DAC;
    params.network = AUDIO_NETWORK_BT2_1;
    params.rin_sample_rate = dec->file_dec.sample_rate;
    params.rout_sample_rate = dec->file_dec.sample_rate;
    params.priv = dec;
    params.factor = TIME_US_FACTOR;
    params.output = file_decoder_output_after_syncts;

    bt_audio_sync_nettime_select(1);
    err = audio_syncts_open(&dec->syncts, &params);
    if (!err) {
        dec->mix_ch_event_params[0] = (u32)&dec->mix_ch;
        dec->mix_ch_event_params[1] = (u32)dec->syncts;
        audio_mixer_ch_set_event_handler(&dec->mix_ch, (void *)dec->mix_ch_event_params, audio_mix_ch_event_handler);
    }
#endif
    return err;
}



static void tws_file_decoder_syncts_free(struct file_dec_hdl *dec)
{
#if TCFG_DEC2TWS_ENABLE
    if (dec->syncts) {
        audio_syncts_close(dec->syncts);
        dec->syncts = NULL;
    }
#endif
}
static int tws_data_trans_handler(struct file_decoder *dec, s16 *data, int len)
{
    int wlen = 0;

#if TCFG_DEC2TWS_ENABLE
    u8 *buf = tws_api_data_trans_buf_alloc(file_dec->tws_channel, len + 2);
    if (!buf) {
        return 0;
    }

    memcpy(buf, &file_dec->tx_seqn, 2);
    memcpy(buf + 2, data, len);
    tws_api_data_trans_push(file_dec->tws_channel, buf, len + 2);
    file_dec->tx_seqn++;
#endif
    /*printf("trans: %d, %d\n", wlen, len);*/
    return len;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码开始
   @param
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
static int file_dec_start()
{
    int err;
    enum audio_channel channel;
    struct file_dec_hdl *dec = file_dec;
    struct audio_mixer *p_mixer = &mixer;

    if (!dec) {
        return -EINVAL;
    }

    log_i("file_dec_start: in\n");

    // 打开file解码器
#if TCFG_DEC2TWS_ENABLE
    err = frame_decoder_open(&dec->file_dec, &frame_input, &decode_task);
#else
    err = file_decoder_open(&dec->file_dec, &file_input, &decode_task, dec->bp, 0);
#endif
    if (err) {
        goto __err1;
    }
    dec->file_dec.output_handler = file_decoder_output_hander;
    file_decoder_set_event_handler(&dec->file_dec, file_dec_event_handler, dec->id);

    channel = file_dec_get_output_channel();
    audio_decoder_set_output_channel(&dec->file_dec.decoder, channel);

#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
    audio_digital_vol_open(MUSIC_DVOL, app_audio_get_volume(APP_AUDIO_STATE_MUSIC), MUSIC_DVOL_MAX, MUSIC_DVOL_FS, -1);
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/

    // 设置叠加功能
    audio_mixer_ch_open(&dec->mix_ch, p_mixer);
    audio_mixer_ch_set_sample_rate(&dec->mix_ch, dec->file_dec.sample_rate);
    int ch_num = 1;
    if (channel == AUDIO_CH_LR) {
        ch_num = 2;
    }
    file_eq_drc_open(dec, dec->file_dec.sample_rate, ch_num);

    sound_pcm_dev_channel_mapping(ch_num);

#if TCFG_DEC2TWS_ENABLE
    tws_file_decoder_syncts_setup(dec);
#endif

    app_audio_state_switch(APP_AUDIO_STATE_MUSIC, get_max_sys_vol());

    // 设置时钟
    /*clk_set("sys", 120000000);*/
    audio_codec_clock_set(AUDIO_FILE_MODE, dec->file_dec.coding_type, dec->wait.preemption);

    dec->file_dec.status = FILE_DEC_STATUS_PLAY;
    if (dec->evt_cb) {
        int msg[2];
        msg[0] = AUDIO_DEC_EVENT_START;
        dec->evt_cb(dec->evt_priv, 2, msg);
    }

    err = audio_decoder_start(&dec->file_dec.decoder);
    if (err) {
        goto __err2;
    }

    return 0;

__err2:
    dec->file_dec.status = 0;
    audio_mixer_ch_close(&dec->mix_ch);
    file_eq_drc_close(dec);
    file_decoder_close(&dec->file_dec);

__err1:
    if (dec->evt_cb && (dec->wait_add)) {
        int msg[2];
        msg[0] = AUDIO_DEC_EVENT_ERR;
        dec->evt_cb(dec->evt_priv, 2, msg);
    }
    file_dec_release();
    /*clk_set("sys", 24000000);*/
    audio_codec_clock_del(AUDIO_FILE_MODE);

    return err;
}

#if TCFG_DEC2TWS_ENABLE
static void tws_file_trans_timestamp_free(struct file_dec_hdl *dec)
{
    if (dec->ts_handle) {
        file_audio_timestamp_close(dec->ts_handle);
        dec->ts_handle = NULL;
    }
}

static void tws_file_trans_timestamp_create(struct file_dec_hdl *dec)
{
    tws_file_trans_timestamp_free(dec);
    bt_audio_sync_nettime_select(1);
    dec->ts_handle = file_audio_timestamp_create(0, dec->trans_dec.sample_rate, bt_audio_sync_lat_time(), 250, TIME_US_FACTOR);
    dec->pcm_num = 0;
}
static void send_data_trans_dec_start_cmd();
static int tws_file_dec_trans_start(void)
{
    int err;
    u8 channel = AUDIO_CH_LR;
    struct file_dec_hdl *dec = file_dec;

    if (!dec) {
        return -EINVAL;
    }

    log_i("file_dec_trans_start: in\n");

    // 打开file解码器
    err = file_decoder_open(&dec->trans_dec, &file_input, &decode_task, dec->bp, 1);
    if (err) {
        goto __err1;
    }
    dec->tx_seqn = 1;
    dec->trans_dec.output_handler = tws_data_trans_handler;
    file_decoder_set_event_handler(&dec->trans_dec, file_dec_event_handler, dec->id);
    audio_decoder_set_output_channel(&dec->trans_dec.decoder, channel);
    audio_decoder_set_run_max(&dec->trans_dec.decoder, 5);

    dec->file_dec.sample_rate = dec->trans_dec.sample_rate;
    dec->file_dec.coding_type = dec->trans_dec.coding_type & 0x0fffffff;

    log_i("total_time : %d\n", dec->trans_dec.dec_total_time);

    // 设置时钟
    /*clk_set("sys", 120000000);*/
    audio_codec_clock_set(AUDIO_FILE_MODE, dec->file_dec.coding_type, dec->wait.preemption);

    dec->tws_channel = tws_api_data_trans_open(dec->tws_channel,
                       TWS_DTC_LOCAL_MEDIA, 10 * 1024);

    send_data_trans_dec_start_cmd();

    int ch_num = 1;
    if (channel == AUDIO_CH_LR) {
        ch_num = 2;
    }

    /* file_eq_drc_open(dec, dec->file_dec.sample_rate, ch_num); */
    tws_file_trans_timestamp_create(dec);
    dec->trans_dec_end = 0;
    dec->trans_dec.status = FILE_DEC_STATUS_PLAY;
    err = audio_decoder_start(&dec->trans_dec.decoder);
    if (err) {
        goto __err2;
    }

    return 0;

__err2:
    dec->file_dec.status = 0;
    file_decoder_close(&dec->file_dec);
    tws_file_trans_timestamp_free(dec);
    tws_api_data_trans_stop(dec->tws_channel);
    file_eq_drc_close(dec);

__err1:
    file_dec_release();
    /*clk_set("sys", 24000000);*/
    audio_codec_clock_del(AUDIO_FILE_MODE);

    return err;
}
#endif
/*----------------------------------------------------------------------------*/
/**@brief    文件解码资源等待
   @param    *wait: 句柄
   @param    event: 事件
   @return   0：成功
   @note     用于多解码打断处理
*/
/*----------------------------------------------------------------------------*/
static int file_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;

    log_i("file_wait_res_handler, event:%d, status:%d ", event,
          file_dec->file_dec.status);

    if (event == AUDIO_RES_GET) {
        // 启动解码
        if (file_dec->file_dec.status == 0) {
            err = file_dec_start();
        } else if (file_dec->file_dec.tmp_pause) {
            file_dec->file_dec.tmp_pause = 0;
            if (file_dec->file_dec.status == FILE_DEC_STATUS_PLAY) {
                audio_mixer_ch_open(&file_dec->mix_ch, &mixer);
#if TCFG_DEC2TWS_ENABLE
                tws_api_auto_drop_frame_enable(0);
                tws_file_decoder_syncts_setup(file_dec);
#endif
                err = audio_decoder_start(&file_dec->file_dec.decoder);
                audio_decoder_resume_all(&decode_task);
            } else if (file_dec->file_dec.status == FILE_DEC_STATUS_PAUSE) {
                audio_mixer_ch_open(&file_dec->mix_ch, &mixer);
            }
        }
    } else if (event == AUDIO_RES_PUT) {
        // 被打断
        if (file_dec->file_dec.status) {
            if (file_dec->file_dec.status == FILE_DEC_STATUS_PLAY || \
                file_dec->file_dec.status == FILE_DEC_STATUS_PAUSE) {
                audio_decoder_resume_all(&decode_task);
                err = audio_decoder_pause(&file_dec->file_dec.decoder);
                audio_mixer_ch_close(&file_dec->mix_ch);
#if TCFG_DEC2TWS_ENABLE
                tws_file_decoder_syncts_free(file_dec);
                tws_api_auto_drop_frame_enable(1);
#endif
            }
            file_dec->file_dec.tmp_pause = 1;
        }
    }

    return err;
}

#if TCFG_DEC2TWS_ENABLE

static void send_data_trans_dec_start_cmd()
{
    u8 args[8];

    if (file_dec->file_dec.status) {
        u32 data[1];
        data[0] = FILE_DEC_INVITE;
        tws_api_send_data_to_sibling(data, sizeof(data), TWS_FUNC_ID_FILE_DEC);
    }

    if (file_dec->file_dec.coding_type == AUDIO_CODING_MP3) {
        args[0] = 1;
    } else if (file_dec->file_dec.coding_type == AUDIO_CODING_WMA) {
        args[0] = 2;
    }
    args[1] = file_dec->file_dec.sample_rate;
    args[2] = file_dec->file_dec.sample_rate >> 8;
    args[3] = file_dec->id & 0xff;

    tws_api_data_trans_start(file_dec->tws_channel, args, 4);
}


static int tws_file_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;

    log_i("tws_file_wait_res_handler, event:%d, status:%d ", event,
          file_dec->trans_dec.status);

    if (event == AUDIO_RES_GET) {
        // 启动解码
        if (file_dec->trans_dec.status == 0) {
            err = tws_file_dec_trans_start();
            if (err) {
                return err;
            }
        } else if (file_dec->trans_dec.tmp_pause) {
            file_dec->trans_dec.tmp_pause = 0;
            if (file_dec->trans_dec.status == FILE_DEC_STATUS_PLAY) {
                err = audio_decoder_start(&file_dec->trans_dec.decoder);
            }
        }
    } else if (event == AUDIO_RES_PUT) {
        // 被打断
        if (file_dec->trans_dec.status) {
            if (file_dec->trans_dec.status == FILE_DEC_STATUS_PLAY || \
                file_dec->trans_dec.status == FILE_DEC_STATUS_PAUSE) {
                err = audio_decoder_pause(&file_dec->trans_dec.decoder);
            }
            file_dec->trans_dec.tmp_pause = 1;
        }
    }

    file_wait_res_handler(wait, event);

    return err;
}

void send_local_media_dec_open_cmd()
{
    puts("send_local_media_dec_open_cmd\n");

    if (!file_dec) {
        return;
    }
    if (file_dec->trans_dec.status) {
        puts("---trans_open\n");
        file_dec->tws_channel = tws_api_data_trans_open(file_dec->tws_channel,
                                TWS_DTC_LOCAL_MEDIA, 10 * 1024);
    }
    if (file_dec->file_dec.status) {
        puts("---dec_open\n");
        send_data_trans_dec_start_cmd();
    }
}

#endif
/*----------------------------------------------------------------------------*/
/**@brief    file解码pp处理
   @param    play: 1-播放。0-暂停
   @return
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
static void file_dec_pp_ctrl(u8 play)
{
    if (!file_dec) {
        return ;
    }
#if TCFG_DEC2TWS_ENABLE
    if (play) {
        // 播放前处理
        if (!file_dec->pick_flag) {
            audio_mixer_ch_pause(&file_dec->mix_ch, 0);
        }
    } else {
        // 暂停后处理
        if (!file_dec->pick_flag) {
            audio_mixer_ch_pause(&file_dec->mix_ch, 1);
        }
        audio_decoder_resume_all_by_sem(&decode_task, 2); // 超时等待解码task运行一轮
    }
#else
    if (play) {
        // 播放前处理
        if (!file_dec->pick_flag) {
            audio_mixer_ch_open(&file_dec->mix_ch, &mixer);
        }
    } else {
        // 暂停后处理
        if (!file_dec->pick_flag) {
            audio_mixer_ch_close(&file_dec->mix_ch);
        }
        audio_decoder_resume_all_by_sem(&decode_task, 2); // 超时等待解码task运行一轮
    }

#endif
}

/*----------------------------------------------------------------------------*/
/**@brief    创建一个文件解码
   @param    *priv: 事件回调私有参数
   @param    *handler: 事件回调句柄
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_create(void *priv, void (*handler)(void *, int argc, int *argv))
{
    struct file_dec_hdl *dec;
    if (file_dec) {
        file_dec_close();
    }

    dec = zalloc(sizeof(*dec));
    if (!dec) {
        return -ENOMEM;
    }

    file_dec = dec;
    file_dec->evt_cb = handler;
    file_dec->evt_priv = priv;

    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief    设置解码数据流设置回调接口
   @param    *dec: 解码句柄
   @param    *stream_handler: 数据流设置回调
   @param    *stream_priv: 数据流设置回调私有句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void file_dec_set_stream_set_hdl(struct file_dec_hdl *dec,
                                 void (*stream_handler)(void *priv, int event, struct file_dec_hdl *),
                                 void *stream_priv)
{
    if (dec) {
        dec->stream_handler = stream_handler;
        dec->stream_priv = stream_priv;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    打开文件解码
   @param    *file: 文件句柄
   @param    *bp: 断点信息
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_open(void *file, struct audio_dec_breakpoint *bp)
{
    int err;
    struct file_dec_hdl *dec = file_dec;

    log_i("file_dec_open: in, 0x%x, bp:0x%x \n", file, bp);

    if ((!dec) || (!file)) {
        return -EPERM;
    }
    dec->file = file;
    dec->bp = bp;
    dec->id = rand32();

    dec->file_dec.ch_type = AUDIO_CH_MAX;
    dec->file_dec.output_ch_num = 2;//audio_output_channel_num();

#if TCFG_DEC_DECRYPT_ENABLE
    cipher_init(&dec->mply_cipher, TCFG_DEC_DECRYPT_KEY);
    cipher_check_decode_file(&dec->mply_cipher, file);
#endif
    dec->wait_add = 1;
#if TCFG_DEC2TWS_ENABLE
    dec->wait.priority = 1;
    dec->wait.preemption = 1;
    dec->wait.handler = tws_file_wait_res_handler;
    err = audio_decoder_task_add_wait(&decode_task, &dec->wait);
#else
    dec->wait.priority = 1;
    dec->wait.preemption = 1;
    dec->wait.handler = file_wait_res_handler;
    err = audio_decoder_task_add_wait(&decode_task, &dec->wait);
#endif
    if (file_dec) {
        file_dec->wait_add = 0;
    }

    return err;
}

int tws_local_media_dec_open(u8 channel, u8 *arg)
{
    int err;
    struct file_dec_hdl *dec = file_dec;

    log_i("tws_local_media_dec_open\n");

    if (!dec) {
        dec = zalloc(sizeof(*dec));
        if (!dec) {
            return -EPERM;
        }
        file_dec = dec;
    }

    if (arg[0] == 1) {
        dec->file_dec.coding_type = AUDIO_CODING_MP3;
    } else if (arg[0] == 2) {
        dec->file_dec.coding_type = AUDIO_CODING_WMA;
    }
    dec->file_dec.sample_rate = (arg[2] << 8) | arg[1];
    dec->id = arg[3];
#if TCFG_USER_TWS_ENABLE
    dec->tws_channel = channel;
#endif

    dec->wait.priority = 1;
    dec->wait.preemption = 1;
    dec->wait.handler = file_wait_res_handler;
    err = audio_decoder_task_add_wait(&decode_task, &dec->wait);

    return err;
}

int tws_local_media_dec_state()
{
    if (!file_dec) {
        return 0;
    }
    if (file_dec->file_dec.status == FILE_DEC_STATUS_PLAY) {
        return 1;
    }
    return 0;
}

static void __file_dec_close(void)
{
    if (!file_dec) {
        return;
    }

    if (file_dec->file_dec.status) {
        file_dec->file_dec.status = 0;
        audio_mixer_ch_pause(&file_dec->mix_ch, 1);
        file_decoder_close(&file_dec->file_dec);
        file_eq_drc_close(file_dec);
        tws_file_decoder_syncts_free(file_dec);
        audio_mixer_ch_close(&file_dec->mix_ch);

        if (file_dec->stream_handler) {
            file_dec->stream_handler(file_dec->stream_priv, FILE_DEC_STREAM_CLOSE,
                                     file_dec);
        }
#if TCFG_DEC2TWS_ENABLE
        if (file_dec->frame) {
            tws_api_data_trans_free(file_dec->tws_channel, file_dec->frame);
            file_dec->frame = NULL;
        }
#endif
    }

    file_dec_release();

    log_i("file_dec_close: exit\n");
}

/*----------------------------------------------------------------------------*/
/**@brief    关闭文件解码
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void file_dec_close(void)
{
#if TCFG_DEC2TWS_ENABLE
    if (!file_dec) {
        return;
    }
    if (file_dec->trans_dec.status) {
        file_dec->trans_dec.status = 0;
        file_decoder_close(&file_dec->trans_dec);
        tws_api_data_trans_stop(file_dec->tws_channel);
        file_eq_drc_close(file_dec);
    }

    tws_file_trans_timestamp_free(file_dec);
    tws_api_data_trans_clear(file_dec->tws_channel);
#endif
    __file_dec_close();

}

#if TCFG_DEC2TWS_ENABLE
void tws_local_media_dec_close()
{
    file_dec_close();
}
#endif

/*----------------------------------------------------------------------------*/
/**@brief    获取file_dec句柄
   @param
   @return   file_dec句柄
   @note
*/
/*----------------------------------------------------------------------------*/
struct file_decoder *file_dec_get_file_decoder_hdl(void)
{
    if (!file_dec) {
        return NULL;
    }

#if TCFG_DEC2TWS_ENABLE
    return &file_dec->trans_dec;
#else
    return &file_dec->file_dec;
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief    获取file_dec状态
   @param
   @return   解码状态
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_get_status(void)
{
    struct file_decoder *dec = file_dec_get_file_decoder_hdl();
    if (dec) {
        return dec->status;
    }
    return FILE_DEC_STATUS_STOP;
}

int file_dec_get_source()
{
    if (!file_dec) {
        return -EINVAL;
    }
#if TCFG_DEC2TWS_ENABLE
    if (file_dec->trans_dec.status) {
        return FILE_FROM_LOCAL;
    }
    if (file_dec->file_dec.status) {
        return FILE_FROM_TWS;
    }
#else
    if (file_dec->file_dec.status) {
        return FILE_FROM_LOCAL;
    }
#endif
    return -EINVAL;
}

/*----------------------------------------------------------------------------*/
/**@brief    文件解码重新开始
   @param    id: 文件解码id
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_restart(int id)
{
    if ((!file_dec) || (id != file_dec->id)) {
        return -1;
    }
    if (file_dec->bp == NULL) {
        if (file_dec->dec_bp == NULL) {
            file_dec->dec_bp = zalloc(sizeof(struct audio_dec_breakpoint) +
                                      BREAKPOINT_DATA_LEN);
            ASSERT(file_dec->dec_bp);
            file_dec->dec_bp->data_len = BREAKPOINT_DATA_LEN;
        }
        file_dec->bp = file_dec->dec_bp;
    }
    if (file_dec->file_dec.status && file_dec->bp) {
        audio_decoder_get_breakpoint(&file_dec->file_dec.decoder, file_dec->bp);
    }

    void *file = file_dec->file;
    void *bp = file_dec->bp;
    void *evt_cb = file_dec->evt_cb;
    void *evt_priv = file_dec->evt_priv;
    int err;
    void *dec_bp = file_dec->dec_bp; // 先保存一下，避免close被释放
    file_dec->dec_bp = NULL;

    file_dec_close();

    err = file_dec_create(evt_priv, evt_cb);
    if (!err) {
        file_dec->dec_bp = dec_bp; // 还原回去
        err = file_dec_open(file, bp);
    } else {
        if (dec_bp) {
            free(dec_bp); // 失败，释放
        }
    }
    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    推送文件解码重新开始命令
   @param
   @return   true：成功
   @return   false：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int file_dec_push_restart(void)
{
    if (!file_dec) {
        return false;
    }
    int argv[3];
    argv[0] = (int)file_dec_restart;
    argv[1] = 1;
    argv[2] = (int)file_dec->id;
    os_taskq_post_type(os_current_task(), Q_CALLBACK, ARRAY_SIZE(argv), argv);
    return true;
}

/*----------------------------------------------------------------------------*/
/**@brief    file decoder pp处理
   @param    *dec: file解码句柄
   @param    play: 1-播放。0-暂停
   @return
   @note     弱函数重定义
*/
/*----------------------------------------------------------------------------*/
void file_decoder_pp_ctrl(struct file_decoder *dec, u8 play)
{
    if (file_dec && (&file_dec->file_dec == dec)) {
        file_dec_pp_ctrl(play);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    音乐模式 eq drc 打开
   @param    sample_rate:采样率
   @param    ch_num:通道个数
   @return   句柄
   @note
*/
/*----------------------------------------------------------------------------*/
void file_eq_drc_open(struct file_dec_hdl *hdl, u16 sample_rate, u8 ch_num)
{
#if TCFG_EQ_ENABLE&&TCFG_MUSIC_MODE_EQ_ENABLE
    u8 drc_en = 0;
#if TCFG_DRC_ENABLE&&TCFG_MUSIC_MODE_DRC_ENABLE
    drc_en = 1;
#endif//TCFG_MUSIC_MODE_DRC_ENABLE

    hdl->eq_drc = dec_eq_drc_setup(&hdl->mix_ch, (eq_output_cb)audio_mixer_ch_write, sample_rate, ch_num, 1, drc_en);
#endif//TCFG_MUSIC_MODE_EQ_ENABLE

}
/*----------------------------------------------------------------------------*/
/**@brief    音乐模式 eq drc 关闭
   @param    句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void file_eq_drc_close(struct file_dec_hdl *hdl)
{
#if TCFG_EQ_ENABLE&&TCFG_MUSIC_MODE_EQ_ENABLE
    if (hdl->eq_drc) {
        dec_eq_drc_free(hdl->eq_drc);
        hdl->eq_drc = NULL;
    }
#endif//TCFG_MUSIC_MODE_EQ_ENABLE

}


#endif /*TCFG_APP_MUSIC_EN*/

