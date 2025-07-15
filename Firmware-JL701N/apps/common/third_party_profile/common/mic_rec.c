/*****************************************************************
>file name : mic_rec.c
>author : lichao
>create time : Wed 26 Jun 2019 04:31:50 PM CST
*****************************************************************/
#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "audio_config.h"
#include "classic/tws_local_media_sync.h"
#include "3th_profile_api.h"
#include "classic/tws_api.h"
#include "btstack/avctp_user.h"
#include "bt_tws.h"

#if (BT_MIC_EN)

#define LOG_TAG             "[MIC_REC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

enum {
    STANDARD_OPUS = 0 << 6,
    KUGOU_OPUS = 1 << 6,
};

enum {
    ENC_OPUS_16KBPS = 0,
    ENC_OPUS_32KBPS = 1,
    ENC_OPUS_64KBPS = 2,
};

typedef struct __ai_encode_info {
    const u8 *info;
    u32 enc_type;
    u8 opus_type;
    u16(*sender)(u8 *buf, u16 len);
} _ai_encode_info;

struct __speech_buf_ctl {
    cbuffer_t cbuffer;
    volatile u8 speech_init_flg;
    u16 cbuf_size;
    u8 *speech_cbuf;
};

struct __mic_rec_t {
    struct __speech_buf_ctl buf_ctl;
    struct __ai_encode_info ai_enc_info;
    u8 ai_mic_busy_flg;
    u8 init_ok;
    u16 frame_num;
    u16 frame_size;
    OS_MUTEX mutex_ai_mic;
};
static struct __mic_rec_t mic_rec;
#define __this (&mic_rec)

extern void bt_sniff_ready_clean(void);
bool get_tws_sibling_connect_state(void);
int a2dp_tws_dec_suspend(void *p);
void a2dp_tws_dec_resume(void);
int audio_mic_enc_open(int (*mic_output)(void *priv, void *buf, int len), u32 code_type, u8 ai_type);
int audio_mic_enc_close();
static void ai_mic_tws_stop_opus();

static void speech_cbuf_init(void)
{
    __this->buf_ctl.speech_init_flg = 1;

    ASSERT(__this->buf_ctl.speech_cbuf == NULL, "speech_cbuf is err\n");
    __this->buf_ctl.speech_cbuf = malloc(__this->buf_ctl.cbuf_size);
    ASSERT(__this->buf_ctl.speech_cbuf, "speech_cbuf is not ok\n");

    cbuf_init(&(__this->buf_ctl.cbuffer), __this->buf_ctl.speech_cbuf, __this->buf_ctl.cbuf_size);
}

static void speech_cbuf_exit(void)
{
    __this->buf_ctl.speech_init_flg = 0;
    cbuf_clear(&(__this->buf_ctl.cbuffer));

    free(__this->buf_ctl.speech_cbuf);
    __this->buf_ctl.speech_cbuf = NULL;
}

static u16 speech_data_send(u8 *buf, u16 len, u16(*send_data)(u8 *buf, u16 len))
{
    u16 res = 0;
    u16 send_len = __this->frame_num * len;
    u8 temp_buf[send_len];

    if (__this->buf_ctl.speech_init_flg == 0) {
        return 0;
    }

    if (cbuf_write(&(__this->buf_ctl.cbuffer), buf, len) != len) {
        res = (u16)(-1);
    }

    /* printf("cl %d\n",cbuf_get_data_size(&(__this->buf_ctl.cbuffer))); */

    while (cbuf_get_data_size(&(__this->buf_ctl.cbuffer)) >= send_len) {
        cbuf_read_alloc_len(&(__this->buf_ctl.cbuffer), temp_buf, send_len);
        if (send_data) {
            if (!send_data(temp_buf, send_len)) {
                putchar('S');
                cbuf_read_alloc_len_updata(&(__this->buf_ctl.cbuffer), send_len);
            } else {
                putchar('E');
                break;
            }
        }
    }

    return res;
}

#if TCFG_USER_TWS_ENABLE
#if 0
void tws_api_local_media_sync_rx_handler_notify()
{
    u8 *tws_buf = NULL;
    int len = 0;

    if (mic_get_data_source() == SINK_TYPE_SLAVE) {
        if (dma_tws_mic_pool) {
            tws_buf = tws_api_local_media_trans_pop(&len);
            /* log_info("sf %d %0x\n",len, tws_buf); */
            if (tws_buf) {
                tws_api_local_media_trans_free(tws_buf);
            }
        }
    }
}

static u16 tws_data_send_slave_to_master(u8 *buf, u16 len)
{
    u8 *tws_buf = NULL;

    if (dma_tws_mic_pool == 0) {
        return -1;
    }

    tws_buf = tws_api_local_media_trans_alloc(len);
    if (tws_buf == NULL) {
        return -1;
    }
    /* log_info("sd  %0x %d\n",tws_buf,len); */
    memcpy(tws_buf, buf, len);
    /* printf("tdlen\n"); */
    /* put_buf(tws_buf,len); */
    tws_api_local_media_trans_push(tws_buf, len);
    return 0;

}
#endif

enum {
    //send to sibling
    TWS_AI_A2DP_DROP_FRAME_CTL_V2 = 0,

    //tws sync deal
    TWS_AI_MIC_RESUME_A2DP = 0x80,
};

static int mic_rec_tws_send_cmd(u8 cmd)
{
    return tws_api_send_data_to_sibling(&cmd, sizeof(cmd), 0x3890AB12);
}

static void __mic_rec_tws_rx_cb_deal(int cmd)
{
    switch (cmd) { //tws sync deal
    case TWS_AI_A2DP_DROP_FRAME_CTL_V2:
        a2dp_tws_dec_suspend(NULL);
        break;
    case TWS_AI_MIC_RESUME_A2DP:
        ai_mic_tws_stop_opus();
        break;
    }
}

static void mic_rec_tws_rx_data(void *_data, u16 len, bool rx)
{
    int msg[4];
    int err = 0;
    u8 *data = (u8 *)_data;

    printf(">>>%s \n", __func__);
    printf("len :%d\n", len);
    put_buf(_data, len);

    if (!rx && data[0] < TWS_AI_MIC_RESUME_A2DP) { //not need deal
        return;
    }

    msg[0] = (int)__mic_rec_tws_rx_cb_deal;
    msg[1] = 1;
    msg[2] = *data;
    err = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
    if (err) {
        printf("tws rx post fail\n");
    }
}

REGISTER_TWS_FUNC_STUB(mic_rec_sync_stub) = {
    .func_id = 0x3890AB12,
    .func    = mic_rec_tws_rx_data,
};
#endif

u16 ai_mic_get_frame_size()
{
    return __this->frame_size;
}

static int rec_enc_output(void *priv, void *buf, int len)
{
    bt_sniff_ready_clean();

    u8 *send_buf = (u8 *)buf;
    int send_len = len;

    //printf("len:%d\n", len);
    __this->frame_size = len;
    if (speech_data_send(send_buf, send_len, __this->ai_enc_info.sender) == (u16)(-1)) {
        log_info("opus data miss !!! line:%d \n", __LINE__);
    }

    return 0;
}


_WEAK_ int a2dp_tws_dec_suspend(void *p)
{
    return 0;
}


_WEAK_ void a2dp_tws_dec_resume(void)
{
}

_WEAK_ void bt_sniff_ready_clean(void)
{
}

_WEAK_ void mic_rec_clock_set(void)
{
}

_WEAK_ void mic_rec_clock_recover(void)
{
}

int ai_mic_rec_start(void)
{
    if (0 == __this->init_ok) {
        printf("init err\n");
        return -1;
    }
    os_mutex_pend(&__this->mutex_ai_mic, 0);

    if (__this->ai_mic_busy_flg) {
        os_mutex_post(&__this->mutex_ai_mic);
        return -1;
    }

    user_send_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);

#if TCFG_USER_TWS_ENABLE
    if (get_tws_sibling_connect_state() && (tws_api_get_role() == TWS_ROLE_MASTER)) {
        mic_rec_tws_send_cmd(TWS_AI_A2DP_DROP_FRAME_CTL_V2);
    }
#endif

    int err = a2dp_tws_dec_suspend(NULL);
    if (err == 0) {
        printf("opus init \n");

        mic_rec_clock_set();

        speech_cbuf_init();

        printf("%s \n", __this->ai_enc_info.info);
        audio_mic_enc_open(rec_enc_output, __this->ai_enc_info.enc_type, __this->ai_enc_info.opus_type);
        __this->ai_mic_busy_flg = 1;
    }

    os_mutex_post(&__this->mutex_ai_mic);

    return 0;
}

int ai_mic_is_busy(void)
{
    return __this->ai_mic_busy_flg;
}

static void ai_mic_tws_stop_opus()
{
    if (__this->ai_mic_busy_flg == 0) {
        a2dp_tws_dec_resume();
    }
}

static int ai_mic_resume_a2dp(void)
{
    int err = 0;
    int msg[8];
    msg[0] = (int)ai_mic_tws_stop_opus;
    msg[1] = 1;
    msg[2] = 0;

    while (1) {
        err = os_taskq_post_type("app_core", Q_CALLBACK, 3, msg);
        if (err != OS_Q_FULL) {
            break;
        }
        os_time_dly(2);
    }

    return err;
}

int ai_mic_rec_close(void)
{
    os_mutex_pend(&__this->mutex_ai_mic, 0);

    if (__this->ai_mic_busy_flg) {
        printf(">>>>opus close: %d, %s\n", cpu_in_irq(), os_current_task());
        audio_mic_enc_close();

        speech_cbuf_exit();

        mic_rec_clock_recover();

        __this->ai_mic_busy_flg = 0;
    }

    os_mutex_post(&__this->mutex_ai_mic);

#if TCFG_USER_TWS_ENABLE
    if ((tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            mic_rec_tws_send_cmd(TWS_AI_MIC_RESUME_A2DP);
        }
    } else {
        /*a2dp_tws_dec_resume();*/
        ai_mic_resume_a2dp();
    }
#else
    ai_mic_resume_a2dp();
    /*a2dp_tws_dec_resume();*/
#endif

    return 0;
}

static int ai_mic_mutex_init(void)
{
    os_mutex_create(&__this->mutex_ai_mic);
    return 0;
}
late_initcall(ai_mic_mutex_init);

int mic_rec_pram_init(/* const char **name,  */u32 enc_type, u8 opus_type, u16(*speech_send)(u8 *buf, u16 len), u16 frame_num, u16 cbuf_size)
{
    /* __this->ai_enc_info.info = name; */
    __this->init_ok = 0;
    if (enc_type == AUDIO_CODING_OPUS && !TCFG_ENC_OPUS_ENABLE) {
        printf("please enable opus deceder");
        return -1;
    } else if (enc_type == AUDIO_CODING_SPEEX && !TCFG_ENC_SPEEX_ENABLE) {
        printf("please enable speex deceder");
        return -1;
    }
    __this->ai_enc_info.enc_type = enc_type;
    __this->ai_enc_info.opus_type = opus_type;
    __this->ai_enc_info.sender = speech_send;
    __this->frame_num = frame_num;
    __this->buf_ctl.cbuf_size = cbuf_size;
    __this->init_ok = 1;
    return 0;
}
#endif

