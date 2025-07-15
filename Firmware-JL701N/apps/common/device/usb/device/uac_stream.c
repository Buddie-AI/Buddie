#include "app_config.h"
#include "system/includes.h"
#include "printf.h"
#include "usb/usb_config.h"
#include "usb/device/usb_stack.h"

#if TCFG_USB_SLAVE_AUDIO_ENABLE
#include "usb/device/uac_audio.h"
#include "uac_stream.h"
#include "audio_config.h"

/* #ifdef CONFIG_MEDIA_DEVELOP_ENABLE */
#include "audio_track.h"
/* #endif */


#define LOG_TAG_CONST       USB
#define LOG_TAG             "[UAC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define     UAC_DEBUG_ECHO_MODE 0

static volatile u8 speaker_stream_is_open = 0;
struct uac_speaker_handle {
    cbuffer_t cbuf;
    volatile u8 need_resume;
    u8 channel;
    u8 alive;
    void *buffer;
    void *audio_track;
    //void (*rx_handler)(int, void *, int);
};

static void (*uac_rx_handler)(int, void *, int) = NULL;

#if (SOUNDCARD_ENABLE)
#define UAC_BUFFER_SIZE     (4 * 1024)
#else
#define UAC_BUFFER_SIZE     (1 * 1024)
#endif

#define UAC_BUFFER_MAX		(UAC_BUFFER_SIZE * 50 / 100)

static struct uac_speaker_handle *uac_speaker = NULL;

#if USB_MALLOC_ENABLE
#else
static struct uac_speaker_handle uac_speaker_handle SEC(.uac_var);
static u8 uac_rx_buffer[UAC_BUFFER_SIZE] ALIGNED(4) SEC(.uac_rx);
#endif
u32 uac_speaker_stream_length()
{
    return UAC_BUFFER_SIZE;
}
u32 uac_speaker_stream_size()
{
    if (!speaker_stream_is_open) {
        return 0;
    }

    if (uac_speaker) {
        return cbuf_get_data_size(&uac_speaker->cbuf);
    }

    return 0;
}

u32 uac_speaker_get_alive()
{
    if (uac_speaker) {
        return uac_speaker->alive;
    }
    return 0;
}
void uac_speaker_set_alive(u8 alive)
{
    local_irq_disable();
    if (uac_speaker) {
        uac_speaker->alive = alive;
    }
    local_irq_enable();
}

void uac_speaker_stream_buf_clear(void)
{
    if (speaker_stream_is_open) {
        cbuf_clear(&uac_speaker->cbuf);
    }
}

void set_uac_speaker_rx_handler(void *priv, void (*rx_handler)(int, void *, int))
{
    uac_rx_handler = rx_handler;
    /* if (uac_speaker) { */
    /* uac_speaker->rx_handler = rx_handler; */
    /* } */
}

int uac_speaker_stream_sample_rate(void)
{
    /* #ifdef CONFIG_MEDIA_DEVELOP_ENABLE */
    if (uac_speaker && uac_speaker->audio_track) {
        int sr = audio_local_sample_track_rate(uac_speaker->audio_track);
        if ((sr < (SPK_AUDIO_RATE + 100)) && (sr > (SPK_AUDIO_RATE - 100))) {
            return sr;
        }
        /* printf("uac audio_track reset \n"); */
        /* local_irq_disable(); */
        /* audio_local_sample_track_close(uac_speaker->audio_track); */
        /* uac_speaker->audio_track = audio_local_sample_track_open(SPK_CHANNEL, SPK_AUDIO_RATE, 1000); */
        /* local_irq_enable(); */
    }
    /* #endif */
    return SPK_AUDIO_RATE;
}

void uac_speaker_stream_write(const u8 *obuf, u32 len)
{
    if (speaker_stream_is_open) {
        //write dac
        /* #ifdef CONFIG_MEDIA_DEVELOP_ENABLE */
        if (uac_speaker->audio_track) {
            audio_local_sample_track_in_period(uac_speaker->audio_track, (len >> 1) / uac_speaker->channel);
        }
        /* #endif */
        int wlen = cbuf_write(&uac_speaker->cbuf, (void *)obuf, len);
        if (wlen != len) {
            //putchar('W');
        }
        //if (uac_speaker->rx_handler) {
        if (uac_rx_handler) {
            /* if (uac_speaker->cbuf.data_len >= UAC_BUFFER_MAX) { */
            // 马上就要满了，赶紧取走
            uac_speaker->need_resume = 1; //2020-12-22注:无需唤醒
            /* } */
            if (uac_speaker->need_resume) {
                uac_speaker->need_resume = 0;
                uac_rx_handler(0, (void *)obuf, len);
                //uac_speaker->rx_handler(0, (void *)obuf, len);
            }
        }
        uac_speaker->alive = 0;
    }
}

int uac_speaker_read(void *priv, void *data, u32 len)
{
    int r_len;
    int err = 0;

    local_irq_disable();
    if (!speaker_stream_is_open) {
        local_irq_enable();
        return 0;
    }

    r_len = cbuf_get_data_size(&uac_speaker->cbuf);
    if (r_len) {
        r_len = r_len > len ? len : r_len;
        r_len = cbuf_read(&uac_speaker->cbuf, data, r_len);
        if (!r_len) {
            putchar('U');
        }
    }

    if (r_len == 0) {
        uac_speaker->need_resume = 1;
    }
    local_irq_enable();
    return r_len;
}

void uac_speaker_stream_open(u32 samplerate, u32 ch)
{
    if (speaker_stream_is_open) {
        return;
    }
    log_info("%s", __func__);

    if (!uac_speaker) {
#if USB_MALLOC_ENABLE

        uac_speaker = zalloc(sizeof(struct uac_speaker_handle));
        if (!uac_speaker) {
            return;
        }

        uac_speaker->buffer = malloc(UAC_BUFFER_SIZE);
        if (!uac_speaker->buffer) {
            free(uac_speaker);
            uac_speaker = NULL;
            goto __err;
        }


#else

        uac_speaker = &uac_speaker_handle;

        memset(uac_speaker, 0, sizeof(struct uac_speaker_handle));

        uac_speaker->buffer = uac_rx_buffer;
#endif
        uac_speaker->channel = ch;
        /* #ifdef CONFIG_MEDIA_DEVELOP_ENABLE */
        uac_speaker->audio_track = audio_local_sample_track_open(ch, samplerate, 1000);
        /* #endif */
    }


    //uac_speaker->rx_handler = NULL;

    cbuf_init(&uac_speaker->cbuf, uac_speaker->buffer, UAC_BUFFER_SIZE);
    speaker_stream_is_open = 1;
    struct sys_event event;
    event.type = SYS_DEVICE_EVENT;
    event.arg = (void *)DEVICE_EVENT_FROM_UAC;
    event.u.dev.event = USB_AUDIO_PLAY_OPEN;
    event.u.dev.value = (int)((ch << 24) | samplerate);

#if !UAC_DEBUG_ECHO_MODE
    sys_event_notify(&event);
#endif

    return;

__err:
    return;
}

void uac_speaker_stream_close()
{
    if (speaker_stream_is_open == 0) {
        return;
    }

    log_info("%s", __func__);
    speaker_stream_is_open = 0;

    if (uac_speaker) {
        /* #ifdef CONFIG_MEDIA_DEVELOP_ENABLE */
        audio_local_sample_track_close(uac_speaker->audio_track);
        /* #endif */
        uac_speaker->audio_track = NULL;
#if USB_MALLOC_ENABLE
        if (uac_speaker->buffer) {
            free(uac_speaker->buffer);
        }
        free(uac_speaker);
#endif
        uac_speaker = NULL;
    }
    struct sys_event event;
    event.type = SYS_DEVICE_EVENT;
    event.arg = (void *)DEVICE_EVENT_FROM_UAC;
    event.u.dev.event = USB_AUDIO_PLAY_CLOSE;
    event.u.dev.value = (int)0;

#if !UAC_DEBUG_ECHO_MODE
    sys_event_notify(&event);
#endif
}

int uac_get_spk_vol()
{
    int max_vol = get_max_sys_vol();
    int vol = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
    if (vol * 100 / max_vol < 100) {
        return vol * 100 / max_vol;
    } else {
        return 99;
    }
    return 0;
}
static u8 flag_uac_event_enable = 1;
void set_uac_event_flag(u8 en)
{
    flag_uac_event_enable = en;
}
u8 get_uac_event_flag(void)
{
    return flag_uac_event_enable;
}
static volatile u32 mic_stream_is_open;

u8 uac_get_mic_stream_status(void)
{
    return mic_stream_is_open;
}

void uac_mute_volume(u32 type, u32 l_vol, u32 r_vol)
{
    struct sys_event event;
    event.type = SYS_DEVICE_EVENT;
    event.arg = (void *)DEVICE_EVENT_FROM_UAC;

    static u32 last_spk_l_vol = (u32) - 1, last_spk_r_vol = (u32) - 1;
    static u32 last_mic_vol = (u32) - 1;

    if (!get_uac_event_flag()) {
        return;
    }
    switch (type) {
    case MIC_FEATURE_UNIT_ID: //MIC
        if (mic_stream_is_open == 0) {
            return ;
        }
        if (l_vol == last_mic_vol) {
            return;
        }
        last_mic_vol = l_vol;
        event.u.dev.event = USB_AUDIO_SET_MIC_VOL;
        break;
    case SPK_FEATURE_UNIT_ID: //SPK
        if (speaker_stream_is_open == 0) {
            return;
        }
        if (l_vol == last_spk_l_vol && r_vol == last_spk_r_vol) {
            return;
        }
        last_spk_l_vol = l_vol;
        last_spk_r_vol = r_vol;
        event.u.dev.event = USB_AUDIO_SET_PLAY_VOL;
        break;
    default:
        break;
    }

    event.u.dev.value = (int)(r_vol << 16 | l_vol);
#if !UAC_DEBUG_ECHO_MODE
    sys_event_notify(&event);
#endif
}


static int (*mic_tx_handler)(int, void *, int) = NULL;
int uac_mic_stream_read(u8 *buf, u32 len)
{
    if (mic_stream_is_open == 0) {
        return 0;
    }
#if 0//48K 1ksin
    const s16 sin_48k[] = {
        0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
        14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
        14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
        0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
        -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
        -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
    };
    u16 *l_ch = (u16 *)buf;
    u16 *r_ch = (u16 *)buf;
    r_ch++;
    for (int i = 0; i < len / 2; i++) {
        *l_ch = sin_48k[i];
        *r_ch = sin_48k[i];
        l_ch += 1;
        r_ch += 1;
    }
    return len;
#elif   UAC_DEBUG_ECHO_MODE
#if MIC_CHANNEL == 2
    uac_speaker_read(NULL, buf, len);
    const s16 sin_48k[] = {
        0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
        14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
        14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
        0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
        -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
        -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
    };
    u16 *r_ch = (u16 *)buf;
    r_ch++;
    for (int i = 0; i < len / 4; i++) {
        *r_ch = sin_48k[i];
        r_ch += 2;
    }
#else
    uac_speaker_read(NULL, buf, len * 2);
    u16 *r_ch = (u16 *)buf;
    for (int i = 0; i < len / 2; i++) {
        r_ch[i] = r_ch[i * 2];
    }

#endif
    return len;
#else
    if (mic_tx_handler) {
#if 1
        return mic_tx_handler(0, buf, len);
#else
        //16bit ---> 24bit
        int rlen = mic_tx_handler(0, tmp_buf, len / 3 * 2);
        rlen /= 2; //sampe point
        for (int i = 0 ; i < rlen ; i++) {
            buf[i * 3] = 0;
            buf[i * 3 + 1] = tmp_buf[i * 2];
            buf[i * 3 + 2] = tmp_buf[i * 2 + 1];
        }
#endif
    } else {
        //putchar('N');
    }
    return 0;
#endif
    return 0;
}

void set_uac_mic_tx_handler(void *priv, int (*tx_handler)(int, void *, int))
{
    mic_tx_handler = tx_handler;
}
static u32 mic_close_tid = 0;
static u8 mic_sw;
u32 uac_mic_stream_open(u32 samplerate, u32 frame_len, u32 ch)
{
    mic_sw = 1;
    if (mic_stream_is_open) {
        return 0;
    }

    /* mic_tx_handler = NULL; */
    log_info("%s", __func__);

    struct sys_event event;
    event.type = SYS_DEVICE_EVENT;
    event.arg = (void *)DEVICE_EVENT_FROM_UAC;
    event.u.dev.event = USB_AUDIO_MIC_OPEN;
    event.u.dev.value = (int)((ch << 24) | samplerate);
    mic_stream_is_open = 1;

#if !UAC_DEBUG_ECHO_MODE
    sys_event_notify(&event);
#endif
    return 0;
}

static void uac_mic_stream_close_delay()
{
    mic_close_tid = 0;
    if (!mic_sw) {

    } else {
        return ;
    }
    log_info("%s", __func__);
    struct sys_event event;
    event.type = SYS_DEVICE_EVENT;
    event.arg = (void *)DEVICE_EVENT_FROM_UAC;
    event.u.dev.event = USB_AUDIO_MIC_CLOSE;
    event.u.dev.value = (int)0;
    mic_stream_is_open = 0;

#if !UAC_DEBUG_ECHO_MODE
    sys_event_notify(&event);
#endif
}
void uac_mic_stream_close()
{
    mic_sw = 0;
    if (mic_stream_is_open == 0) {
        return ;
    }
    //FIXME:
    //未知原因出现频繁开关mic，导致出现audio或者蓝牙工作异常，
    //收到mic关闭命令后延时1s再发消息通知audio模块执行关闭动作
    //如果在1s之内继续收到usb下发的关闭命令，则继续推迟1s。
    if (mic_close_tid == 0) {
        mic_close_tid = sys_hi_timeout_add(NULL, uac_mic_stream_close_delay, 1000);
    } else {
        sys_hi_timeout_modify(mic_close_tid, 1000);
    }
}

_WEAK_
s8 app_audio_get_volume(u8 state)
{
    return 88;
}
_WEAK_
void usb_audio_demo_exit(void)
{

}
_WEAK_
int usb_audio_demo_init(void)
{
    return 0;
}
_WEAK_
u8 get_max_sys_vol(void)
{
    return 100;
}
_WEAK_
void *audio_local_sample_track_open(u8 channel, int sample_rate, int period)
{
    return NULL;
}
_WEAK_
int audio_local_sample_track_in_period(void *c, int samples)
{
    return 0;
}
_WEAK_
void audio_local_sample_track_close(void *c)
{
}
#endif
