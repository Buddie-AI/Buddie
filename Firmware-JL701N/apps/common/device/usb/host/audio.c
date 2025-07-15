#include "includes.h"
#include "asm/includes.h"
#include "app_config.h"
#include "system/timer.h"
#include "device/ioctl_cmds.h"
#include "device_drive.h"
#if TCFG_HOST_AUDIO_ENABLE
#include "usb/host/usb_host.h"
#include "usb_ctrl_transfer.h"
#include "usb_bulk_transfer.h"
#include "audio.h"
#include "usb_config.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[AUDIO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


/* #define USB_AUDIO_PLAY_TEST */

struct usb_audio_play {
    u32 sample_rate;
    u8 Cur_AlternateSetting;
    u8 src_channel;
    u8 play_state;
    u8 mute;
    u8 *send_buf;
    u8 *usb_audio_play_buf;
    u8 *usb_audio_play_buf2;
    cbuffer_t usb_audio_play_cbuf;
    u32 usb_audio_remain_len;
    OS_SEM sem;
    int (*put_buf)(void *ptr, u32 len);
};
struct usb_audio_mic {
    u8 Cur_AlternateSetting;
    u32 sample_rate;
    u8 record_state;
    u8 *usb_audio_record_buf;
    u8 *usb_audio_record_buf2;
    cbuffer_t usb_audio_record_cbuf;
    int *(*get_buf)(void *ptr, u32 len);
};
struct usb_audio_info {
    usb_dev usb_id;
    struct usb_audio_play player;
    struct usb_audio_mic microphone;
};

enum {
    AUDIO_PLAY_IDLE = 0,
    AUDIO_PLAY_START,
    AUDIO_PLAY_STOP,
    AUDIO_PLAY_PAUSE,
};
enum {
    AUDIO_RECORD_IDLE = 0,
    AUDIO_RECORD_START,
    AUDIO_RECORD_STOP,
    AUDIO_RECORD_PAUSE,
};

#define EP_MAX_PACKET_SIZE  (192)

struct usb_audio_info _usb_audio_info = {0};
#define __this   (&_usb_audio_info)

struct audio_device_t audio_device[USB_MAX_HW_NUM][MAX_HOST_INTERFACE];

static u8 ep_in_dma_buf[256]  __attribute__((aligned(4)));
static u8 ep_out_dma_buf[256]  __attribute__((aligned(4)));

static int set_power(struct usb_host_device *host_dev, u32 value)
{
    const usb_dev usb_id = host_device2id(host_dev);
    return DEV_ERR_NONE;
}

static int get_power(struct usb_host_device *host_dev, u32 value)
{
    return DEV_ERR_NONE;
}

static const struct interface_ctrl uac_ctrl = {
    .interface_class = USB_CLASS_AUDIO,
    .set_power = set_power,
    .get_power = get_power,
    .ioctl = NULL,
};

static const struct usb_interface_info _uac_if[USB_MAX_HW_NUM][MAX_HOST_INTERFACE] = {
    {
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][0],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][1],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][2],
        },
        {
            .ctrl = (struct interface_ctrl *) &uac_ctrl,
            .dev.audio = &audio_device[0][3],
        },
    },
};

int usb_audio_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf)
{
    struct usb_interface_descriptor *interface = (struct usb_interface_descriptor *)pBuf;
    const usb_dev usb_id = host_device2id(host_dev);

    const struct usb_interface_info *usb_if = &_uac_if[usb_id][interface_num];
    struct audio_streaming_t *as_t = NULL;
    memset(usb_if->dev.p, 0, sizeof(struct audio_device_t));
    host_dev->interface_info[interface_num] = usb_if;
    usb_if->dev.audio->parent = host_dev;

    if (interface->bInterfaceSubClass == USB_SUBCLASS_AUDIOCONTROL) {
        log_info("audio control interface : %d\n", interface_num);
        pBuf += sizeof(struct usb_interface_descriptor);
        usb_if->dev.audio->subclass = interface->bInterfaceSubClass;
        usb_if->dev.audio->interface_num = interface_num;

        return sizeof(struct usb_interface_descriptor);
    }

    if (interface->bInterfaceSubClass == USB_SUBCLASS_AUDIOSTREAMING) {
        usb_if->dev.audio->subclass = interface->bInterfaceSubClass;
        usb_if->dev.audio->interface_num = interface_num;
        if (interface->bNumEndpoints == 0) {
            pBuf += sizeof(struct usb_interface_descriptor);
            do {
                struct usb_interface_descriptor *as_interface = (struct usb_interface_descriptor *)pBuf;
                if (as_interface->bNumEndpoints == 0 || as_interface->bInterfaceClass != USB_CLASS_AUDIO) {
                    break;
                }
                log_info("audio streaming interface : %d  ep_num:%d Altersetting:%d", interface_num, as_interface->bNumEndpoints, as_interface->bAlternateSetting);
                as_t = &usb_if->dev.audio->as[as_interface->bAlternateSetting - 1];
                as_t->bNumEndpoints = as_interface->bNumEndpoints;
                pBuf += (USB_DT_INTERFACE_SIZE + UAC_DT_AS_HEADER_SIZE);
                //解析format
                struct uac_format_type_i_discrete_descriptor *uac_format_desc = (struct uac_format_type_i_discrete_descriptor *)pBuf;
                if (uac_format_desc->bDescriptorSubtype == UAC_FORMAT_TYPE) {
                    as_t->bFormatType = uac_format_desc->bFormatType;
                    as_t->bNrChannels = uac_format_desc->bNrChannels;
                    as_t->bSubframeSize = uac_format_desc->bSubframeSize;
                    as_t->bBitResolution = uac_format_desc->bBitResolution;
                    as_t->bSamFreqType = uac_format_desc->bSamFreqType;
                    for (u8 i = 0; i < as_t->bSamFreqType; i++) {
                        memcpy(&as_t->tSamFreq[i], &uac_format_desc->tSamFreq[i], 3);
                        log_info("as bNrChannels:%d bBitResolution:%d  tSamFreq : %d", as_t->bNrChannels, as_t->bBitResolution, as_t->tSamFreq[i]);
                    }
                    //Endpointdescriptor
                    pBuf += uac_format_desc->bLength;
                    /* for (int i = 0; i < as_t->bNumEndpoints; i++) { */
                    struct usb_endpoint_descriptor *endpoint = (struct usb_endpoint_descriptor *)pBuf;
                    if (endpoint->bDescriptorType == USB_DT_ENDPOINT) {
                        as_t->ep_Interval = endpoint->bInterval;
                        as_t->ep_max_packet_size = endpoint->wMaxPacketSize;
                        if (endpoint->bEndpointAddress & USB_DIR_IN) {
                            as_t->ep = endpoint->bEndpointAddress & 0xf;
                            log_info("ep in : %x\n", as_t->ep);
                            usb_if->dev.audio->support = MICROPHONE_SUPPORTED;
                        } else {
                            as_t->ep = endpoint->bEndpointAddress;
                            log_info("ep out : %x\n", as_t->ep);
                            usb_if->dev.audio->support = HEADPHONE_SUPPORTED;
                        }
                        pBuf += (USB_DT_ENDPOINT_AUDIO_SIZE + UAC_ISO_ENDPOINT_DESC_SIZE);
                    }
                    /* } */
                } else {
                    log_error("uac_format_desc->bDescriptorSubtype err!!\n");
                    goto __exit;
                }

            } while (1);
            /* log_info("lennnn:%d\n",pBuf - (u8 *)interface); */
            return pBuf - (u8 *)interface ;
        } else {
            log_info("audio streaming interface : %d  ep_num:%d Altersetting:%d\n", interface_num, interface->bNumEndpoints, interface->bAlternateSetting);
        }

    }

__exit:
    return USB_DT_INTERFACE_SIZE;

}

static struct audio_device_t *__find_microphone_interface(const struct usb_host_device *host_dev)
{
    struct audio_device_t *audio = NULL;
    for (u8 i = 0; i < MAX_HOST_INTERFACE; i++) {
        const struct usb_interface_info *usb_if = host_dev->interface_info[i];
        if (usb_if &&
            (usb_if->ctrl->interface_class == USB_CLASS_AUDIO)) {
            audio = usb_if->dev.audio;
            if (audio->subclass == USB_SUBCLASS_AUDIOSTREAMING &&
                audio->support == MICROPHONE_SUPPORTED) {
                // find microphone
                return audio;
            }
        }
    }

    return NULL;
}
static struct audio_device_t *__find_headphone_interface(const struct usb_host_device *host_dev)
{
    struct audio_device_t *audio = NULL;
    for (u8 i = 0; i < MAX_HOST_INTERFACE; i++) {
        const struct usb_interface_info *usb_if = host_dev->interface_info[i];
        if (usb_if &&
            (usb_if->ctrl->interface_class == USB_CLASS_AUDIO)) {
            audio = usb_if->dev.audio;
            if (audio->subclass == USB_SUBCLASS_AUDIOSTREAMING &&
                audio->support == HEADPHONE_SUPPORTED) {
                // find headphone
                return audio;
            }
        }
    }

    return NULL;
}
static u32 play_vol_convert(u16 v)
{
    //固定音量表,更换声卡需要修改音量表
    const u16 vol_table[] = {
        //0-100
        0xd300, //0
        0xd58f, 0xd7bf, 0xd9a8, 0xdb5b, 0xdce1, 0xde45, 0xdf8a, 0xe0b6, 0xe1cd, 0xe2d1,
        0xe3c5, 0xe4ab, 0xe583, 0xe651, 0xe714, 0xe7cd, 0xe87f, 0xe928, 0xe9ca, 0xea66,
        0xeafc, 0xeb8d, 0xec18, 0xec9e, 0xed20, 0xed9e, 0xee18, 0xee8e, 0xef00, 0xef6f,
        0xefdc, 0xf045, 0xf0ab, 0xf10f, 0xf171, 0xf1d0, 0xf22c, 0xf287, 0xf2e0, 0xf336,
        0xf38b, 0xf3de, 0xf42f, 0xf47e, 0xf4cc, 0xf518, 0xf563, 0xf5ad, 0xf5f5, 0xf63c,
        0xf681, 0xf6c6, 0xf709, 0xf74b, 0xf78c, 0xf7cb, 0xf80a, 0xf848, 0xf885, 0xf8c1,
        0xf8fc, 0xf936, 0xf96f, 0xf9a8, 0xf9df, 0xfa16, 0xfa4c, 0xfa81, 0xfab6, 0xfaea,
        0xfb1d, 0xfb50, 0xfb82, 0xfbb3, 0xfbe4, 0xfc14, 0xfc43, 0xfc72, 0xfca0, 0xfcce,
        0xfcfc, 0xfd28, 0xfd55, 0xfd80, 0xfdab, 0xfdd6, 0xfe01, 0xfe2a, 0xfe54, 0xfe7d,
        0xfea5, 0xfece, 0xfef5, 0xff1d, 0xff43, 0xff6a, 0xff90, 0xffb6, 0xffdb, 0x0000,
    };

    if (v <= 100) {
        return vol_table[v];
    }

    for (int i = 0; i < sizeof(vol_table) / 2; i++) {
        if (v <= vol_table[i]) {
            return i;
        }
    }

    return 0;
}

void set_usb_audio_play_volume(u16 vol)
{
    const struct usb_host_device *host_dev = host_id2device(__this->usb_id);
    u8 featureUnitID = 6;
    usb_audio_volume_control(host_dev, featureUnitID, 1, play_vol_convert(vol));
    usb_audio_volume_control(host_dev, featureUnitID, 2, play_vol_convert(vol));
    if (vol == 0) {
        __this->player.mute = 1;
        usb_audio_mute_control(host_dev, featureUnitID, __this->player.mute); //mute
    } else {
        if (__this->player.mute == 1) {
            __this->player.mute = 0;
            usb_audio_mute_control(host_dev, featureUnitID, __this->player.mute);
        }
    }
}

#ifdef USB_AUDIO_PLAY_TEST
static const s16 sin_48k[] = {
    0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
    14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
    14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
    0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
    -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
    -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
};
#endif
static void usb_audio_tx_isr(struct usb_host_device *host_dev, u32 ep)
{
    const usb_dev usb_id = host_device2id(host_dev);
    struct audio_device_t *audio = NULL;
    struct audio_streaming_t *as_t = NULL;
    u16 ep_max_packet_size = 0;
    u8 channel = 0;
    u32 rlen = 0;
    static u32 usb_audio_tx_len = 0;

    if (__this->player.play_state != AUDIO_PLAY_START) {
        return;
    }

    audio = __find_headphone_interface(host_dev);
    if (!audio) {
        log_error("no find headphone interface!");
        return;
    }

    as_t = &audio->as[__this->player.Cur_AlternateSetting - 1];
    /* ep_max_packet_size = as_t->ep_max_packet_size; */
    ep_max_packet_size = EP_MAX_PACKET_SIZE;
    channel = as_t->bNrChannels;

    //iso send
#ifdef USB_AUDIO_PLAY_TEST
    //For Test
    int tx_len = 0;
#if 1 // 单声道双声道输出
    s16 buf[240 / 2];
    for (u8 i = 0, j = 0; i < 240 / 2; i += 2) {
        buf[i] = sin_48k[j];
        buf[i + 1] = sin_48k[j];
        j++;
        if (j >= sizeof(sin_48k) / sizeof(sin_48k[0])) {
            j = 0;
        }
    }
#else
    //单声道直接输出
    u8 buf[248];
    do {
        memcpy(&buf[tx_len], sin_48k, sizeof(sin_48k));
        tx_len += sizeof(sin_48k);
    } while (tx_len < ep_max_packet_size);
#endif
    usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, buf, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
#else
    if (__this->player.usb_audio_remain_len == 0) {
        cbuf_read_alloc(&__this->player.usb_audio_play_cbuf, &__this->player.usb_audio_remain_len);
        usb_audio_tx_len = 0;
    }
    if (__this->player.usb_audio_remain_len) {
        if (usb_audio_tx_len == 0) {
            rlen = cbuf_read(&__this->player.usb_audio_play_cbuf, __this->player.usb_audio_play_buf2, __this->player.usb_audio_remain_len);
            if (!rlen) {
                __this->player.usb_audio_remain_len = 0;
                usb_audio_tx_len = 0;
                putchar('C');
                usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, NULL, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 1);
                os_sem_post(&__this->player.sem);
                return;
            }
            os_sem_post(&__this->player.sem);

        }
        u8 *send_buf = __this->player.send_buf;
        u8 *play_buf = __this->player.usb_audio_play_buf2;
        if (channel == 2) {
            if (__this->player.src_channel == 1) {
                //源数据是单声道数据,转双声道输出
                int j = 0;
                for (u8 i = 0; i < ep_max_packet_size; i += 4) {
                    //left
                    *(send_buf + i) = *(play_buf + (usb_audio_tx_len + j));
                    *(send_buf + i + 1) = *(play_buf + (usb_audio_tx_len + j + 1));
                    //right

                    *(send_buf + i + 2) = *(play_buf + (usb_audio_tx_len + j));
                    *(send_buf + i + 3) = *(play_buf + (usb_audio_tx_len + j + 1));
                    j += 2;
                }
                usb_audio_tx_len += j;
                usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, send_buf, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
            } else if (__this->player.src_channel == 2) {
                //源数据是双声道数据,直接双声道输出
                usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, play_buf + usb_audio_tx_len, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 0);
                usb_audio_tx_len += ep_max_packet_size;
            }
        } else if (channel == 1) {
        }
        if (usb_audio_tx_len >= __this->player.usb_audio_remain_len) {
            __this->player.usb_audio_remain_len = 0;
            usb_audio_tx_len = 0;
        }
    } else {
        //audio buf null ,send null packet
        putchar('E');
        usb_h_ep_write_async(usb_id, ep, ep_max_packet_size, as_t->ep, NULL, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 1);
    }

#endif
}
void set_vol_test(void *p)
{
    struct usb_host_device *host_dev = (struct usb_host_device *)p;
    static u16 vol = 100;
    set_usb_audio_play_volume(vol);
    /* static u8 f = 0; */
    /* void usb_audio_pause_play(void); */
    /* void usb_audio_resume_play(void); */
    /* if (!f) { */
    /* usb_audio_pause_play(); */
    /* } else { */
    /* usb_audio_resume_play(); */
    /* } */
    /* f = !f; */
    vol -= 10;
}
void audio_play_task(void *p)
{
    log_info(">>> Enter usb audio play task");
    struct usb_host_device *host_dev = (struct usb_host_device *)p;
    const usb_dev usb_id = host_device2id(host_dev);
    u8 *ptr = NULL;
    u32 wlen = 0;
    u32 ret = 0;
    struct audio_device_t *audio = NULL;
    audio = __find_headphone_interface(host_dev);
    struct audio_streaming_t *as_t = &audio->as[__this->player.Cur_AlternateSetting - 1];
    /* u32 ep_max_packet_size = as_t->ep_max_packet_size; */
    u32 ep_max_packet_size = EP_MAX_PACKET_SIZE;
    log_info("ep max packet : %d\n", ep_max_packet_size);
    if (__this->player.send_buf) {
        free(__this->player.send_buf);
        __this->player.send_buf = NULL;
    }
    __this->player.send_buf = zalloc(ep_max_packet_size);
    u32 usb_audio_buf_size = ep_max_packet_size * 5; //预留5个包的缓存

    /* sys_timer_add(host_dev,set_vol_test,5000); */
    os_sem_create(&__this->player.sem, 0);
    u32 host_ep = as_t->host_ep;
    __this->player.play_state = AUDIO_PLAY_START;
    //分配双缓存
    // 一个缓存保存读卡的数据,一个用于usb发送
    if (!__this->player.usb_audio_play_buf) {
        __this->player.usb_audio_play_buf = zalloc(usb_audio_buf_size);
        cbuf_init(&__this->player.usb_audio_play_cbuf, __this->player.usb_audio_play_buf, usb_audio_buf_size);
        usb_h_ep_write_async(usb_id, host_ep, ep_max_packet_size, as_t->ep, NULL, ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 1); //启动iso传输
    }
    if (!__this->player.usb_audio_play_buf2) {
        __this->player.usb_audio_play_buf2 = zalloc(usb_audio_buf_size);
    }
    while (1) {
        if (__this->player.Cur_AlternateSetting == 0 || __this->player.play_state != AUDIO_PLAY_START) {
            putchar('C');
            os_time_dly(50);
            continue;
        }
        ptr = cbuf_write_alloc(&__this->player.usb_audio_play_cbuf, &wlen);
        if (wlen) {
            putchar('R');
            ret = __this->player.put_buf(ptr, wlen);
            if (ret != wlen) {
                __this->player.play_state = AUDIO_PLAY_STOP;
                goto __task_exit;

            }
            cbuf_write_updata(&__this->player.usb_audio_play_cbuf, wlen);
        } else {
            log_w("usb audio play buf not enough!\n");
        }
__task_exit:
        os_sem_pend(&__this->player.sem, 0);
    }
}

void usb_audio_start_play(const usb_dev usb_id, u8 channel, u8 bit_reso, u32 sample_rate)
{
    log_info(" usb audio play\n");
    const struct usb_host_device *host_dev = host_id2device(usb_id);
    struct audio_device_t *audio = NULL;
    struct audio_streaming_t *as_t = NULL;
    u8 *ep_buffer = ep_out_dma_buf;
    u8 find_alternatesetting = 0;
    audio = __find_headphone_interface(host_dev);
    if (!audio) {
        log_error("no find headphone interface!");
        return;
    }
    for (u8 i = 0; i < ARRAY_SIZE(audio->as); i++) {
        as_t = &audio->as[i];
        if (as_t->bBitResolution == bit_reso) {
            for (u8 j = 0; j < as_t->bSamFreqType; j++) {
                if (as_t->tSamFreq[j] == sample_rate) {
                    find_alternatesetting = i + 1;
                    break;
                }
            }
        }
    }
    if (!find_alternatesetting) {
        log_e("can not find Alternatesetting,please check bit_reso and sample_rate\n");
        return;
    }

    __this->usb_id = usb_id;
    //端点分配
    u32 host_ep = usb_get_ep_num(usb_id, USB_DIR_OUT, USB_ENDPOINT_XFER_ISOC);
    ASSERT(host_ep != -1, "ep not enough");

    __this->player.Cur_AlternateSetting = find_alternatesetting; //选择Alternatesetting
    __this->player.sample_rate = sample_rate;   //选择采样率
    __this->player.src_channel = channel;
    as_t = &audio->as[find_alternatesetting - 1];
    u8 target_ep = as_t->ep;
    u8 ep_interval = as_t->ep_Interval;
    as_t->host_ep = host_ep;

    usb_set_interface(host_dev, audio->interface_num, find_alternatesetting); //interface   Alternatesetting
    usb_audio_sampling_frequency_control(host_dev, target_ep, sample_rate);//设置采样率
    //设置音量
    /* usb_audio_volume_control(host_dev, 6, 1, vol_convert(5)); */
    /* usb_audio_volume_control(host_dev, 6, 2, vol_convert(5)); */

    log_info("H2D ep: %x --> %x  interval: %d", host_ep, target_ep, ep_interval);
    usb_h_set_ep_isr(host_dev, host_ep, usb_audio_tx_isr, host_dev);
    usb_h_ep_config(usb_id,  host_ep, USB_ENDPOINT_XFER_ISOC, 1, ep_interval, ep_buffer, sizeof(ep_out_dma_buf));
    task_create(audio_play_task, host_dev, "uac_play");
}

void usb_audio_stop_play(const usb_dev usb_id)
{
    const struct usb_host_device *host_dev = host_id2device(usb_id);
    usb_h_set_ep_isr(NULL, 0, NULL, NULL);
    __this->player.put_buf(NULL, 0);
    __this->player.Cur_AlternateSetting = 0;
    __this->player.sample_rate = 0;
    __this->player.src_channel = 0;
    if (__this->player.usb_audio_play_buf) {
        free(__this->player.usb_audio_play_buf);
        __this->player.usb_audio_play_buf = NULL;
    }
    if (__this->player.usb_audio_play_buf2) {
        free(__this->player.usb_audio_play_buf2);
        __this->player.usb_audio_play_buf2 = NULL;
    }
    if (__this->player.send_buf) {
        free(__this->player.send_buf);
        __this->player.send_buf = NULL;
    }
    printf("\n[ debug ]--func=%s line=%d\n", __func__, __LINE__);
    task_kill("uac_play");
    printf("\n[ debug ]--func=%s line=%d\n", __func__, __LINE__);
}
void usb_audio_pause_play(void)
{
    __this->player.play_state = AUDIO_PLAY_PAUSE;
}
void usb_audio_resume_play(void)
{
    const struct usb_host_device *host_dev = host_id2device(__this->usb_id);
    struct audio_device_t *audio = __find_headphone_interface(host_dev);
    struct audio_streaming_t *as_t = &audio->as[__this->player.Cur_AlternateSetting - 1];
    __this->player.play_state = AUDIO_PLAY_START;
    usb_h_ep_write_async(__this->usb_id, as_t->host_ep, as_t->ep_max_packet_size, as_t->ep, NULL, as_t->ep_max_packet_size, USB_ENDPOINT_XFER_ISOC, 1); //重新启动传输
}


static u32 record_vol_convert(u16 v)
{
    //固定音量表,更换声卡需要修改音量表
    const u16 vol_table[] = {
        //0-100
        0xf400,
        0xf479, 0xf4ee, 0xf560, 0xf5cf, 0xf63a, 0xf6a3, 0xf709, 0xf76c, 0xf7cd, 0xf82b,
        0xf887, 0xf8e1, 0xf939, 0xf98f, 0xf9e4, 0xfa36, 0xfa87, 0xfad6, 0xfb23, 0xfb6f,
        0xfbba, 0xfc03, 0xfc4b, 0xfc91, 0xfcd6, 0xfd1a, 0xfd5d, 0xfd9f, 0xfde0, 0xfe1f,
        0xfe5e, 0xfe9b, 0xfed8, 0xff14, 0xff4e, 0xff88, 0xffc1, 0xfff9, 0x0003, 0x0069,
        0x00a3, 0x00de, 0x0119, 0x0155, 0x0193, 0x01d1, 0x0210, 0x0251, 0x0292, 0x02d5,
        0x0318, 0x035d, 0x03a3, 0x03eb, 0x0434, 0x047e, 0x04c9, 0x0517, 0x0565, 0x05b5,
        0x0607, 0x065b, 0x06b1, 0x0708, 0x0762, 0x07bd, 0x081b, 0x087c, 0x08de, 0x0943,
        0x09ab, 0x0a16, 0x0a84, 0x0af4, 0x0b69, 0x0be1, 0x0c5c, 0x0cdc, 0x0d60, 0x0de9,
        0x0e77, 0x0f0a, 0x0fa2, 0x1041, 0x10e7, 0x1195, 0x124a, 0x1308, 0x13d0, 0x14a3,
        0x1582, 0x166e, 0x176a, 0x1877, 0x1998, 0x1ad0, 0x1c24, 0x1d98, 0x1f33, 0x2100,
    };

    if (v <= 100) {
        return vol_table[v];
    }

    for (int i = 0; i < sizeof(vol_table) / 2; i++) {
        if (v <= vol_table[i]) {
            return i;
        }
    }

    return 0;
}

void set_usb_audio_record_volume(u16 vol)
{
    const struct usb_host_device *host_dev = host_id2device(__this->usb_id);
    u8 featureUnitID = 5;
    usb_audio_volume_control(host_dev, featureUnitID, 0, record_vol_convert(vol));
}

static u32 write_file_len = 0;
static void usb_audio_rx_isr(struct usb_host_device *host_dev, u32 ep)
{
    u8 buffer[192] = {0};
    u8 *ptr = NULL;
    int rlen, wlen = 0;
    usb_dev usb_id = host_device2id(host_dev);
    struct audio_device_t *audio = NULL;
    struct audio_streaming_t *as_t = NULL;
    audio = __find_microphone_interface(host_dev);
    if (!audio) {
        log_error("no find microphone interface!");
        return;
    }
    if (__this->microphone.record_state != AUDIO_RECORD_START) {
        return;
    }
    as_t = &audio->as[__this->microphone.Cur_AlternateSetting - 1];
    u8 channel = as_t->bNrChannels;

    u32 rx_len = usb_h_ep_read_async(usb_id, ep, as_t->ep, buffer, sizeof(buffer), USB_ENDPOINT_XFER_ISOC, 0);
    /* g_printf("RX:%d\n",rx_len); */
    /* printf_buf(buffer, rx_len); */
    cbuf_write(&__this->microphone.usb_audio_record_cbuf, buffer, rx_len);
    cbuf_write_alloc(&__this->microphone.usb_audio_record_cbuf, &wlen);
    if (wlen == 0) {
        putchar('O');
        if (write_file_len) {
            log_w("write againnnnn\n");
        }
        /* [> printf("R:%d  W:%d\n", rx_len,wlen); <] */
        cbuf_read_alloc(&__this->microphone.usb_audio_record_cbuf, &rlen);
        cbuf_read(&__this->microphone.usb_audio_record_cbuf, __this->microphone.usb_audio_record_buf2, rlen);
        write_file_len = rlen;
        os_taskq_post_msg("uac_record", 2, 0x01, rlen);
        /* return; */
    }

    usb_h_ep_read_async(usb_id, ep, as_t->ep, NULL, 0, USB_ENDPOINT_XFER_ISOC, 1); //触发下一个接收中断
}
void audio_record_task(void *p)
{
    log_info(">>> Enter usb audio record task");
    struct usb_host_device *host_dev = (struct usb_host_device *)p;
    const usb_dev usb_id = host_device2id(host_dev);
    u8 *ptr = NULL;
    u32 rlen = 0;
    u32 ret = 0;
    int msg[16];
    struct audio_device_t *audio = NULL;
    audio = __find_microphone_interface(host_dev);
    struct audio_streaming_t *as_t = &audio->as[__this->microphone.Cur_AlternateSetting - 1];
    /* u32 ep_max_packet_size = as_t->ep_max_packet_size; */
    u32 ep_max_packet_size = EP_MAX_PACKET_SIZE;
    log_info("ep max packet : %d\n", ep_max_packet_size);
    u32 usb_audio_buf_size = ep_max_packet_size * 50;

    u32 host_ep = as_t->host_ep;
    u8 target_ep = as_t->ep;
    //分配双缓存
    // 一个缓存写卡的数据,一个用于usb接收
    if (!__this->microphone.usb_audio_record_buf) {
        __this->microphone.usb_audio_record_buf = zalloc(usb_audio_buf_size);
        cbuf_init(&__this->microphone.usb_audio_record_cbuf, __this->microphone.usb_audio_record_buf, usb_audio_buf_size);

    }
    if (!__this->microphone.usb_audio_record_buf2) {
        __this->microphone.usb_audio_record_buf2 = zalloc(usb_audio_buf_size);
    }

    usb_h_ep_read_async(usb_id, host_ep, target_ep, NULL, 0, USB_ENDPOINT_XFER_ISOC, 1); //启动iso
    while (1) {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (ret == OS_TASKQ) {
            switch (msg[1]) {
            case 0x01:
                ptr = __this->microphone.usb_audio_record_buf2;
                rlen = msg[2];
                putchar('W');
                __this->microphone.get_buf(ptr, rlen);
                write_file_len = 0;
                break;
            }
        }
    }
}
void usb_audio_start_record(const usb_dev usb_id, u8 bit_reso, u32 sample_rate)
{
    log_info(" usb audio record\n");
    const struct usb_host_device *host_dev = host_id2device(usb_id);
    struct audio_device_t *audio = NULL;
    struct audio_streaming_t *as_t = NULL;
    u8 *ep_buffer = ep_in_dma_buf;
    u8 find_alternatesetting = 0;
    audio = __find_microphone_interface(host_dev);
    if (!audio) {
        log_error("no find microphone interface!");
        return;
    }
    for (u8 i = 0; i < ARRAY_SIZE(audio->as); i++) {
        as_t = &audio->as[i];
        if (as_t->bBitResolution == bit_reso) {
            for (u8 j = 0; j < as_t->bSamFreqType; j++) {
                if (as_t->tSamFreq[j] == sample_rate) {
                    find_alternatesetting = i + 1;
                    break;
                }
            }
        }
    }
    if (!find_alternatesetting) {
        log_e("can not find Alternatesetting,please check bit_reso and sample_rate\n");
        return;
    }
    //端点分配
    u32 host_ep = usb_get_ep_num(usb_id, USB_DIR_IN, USB_ENDPOINT_XFER_ISOC);
    ASSERT(host_ep != -1, "ep not enough");
    __this->usb_id = usb_id;
    host_ep = host_ep | USB_DIR_IN;

    __this->microphone.Cur_AlternateSetting = find_alternatesetting; //选择Alternatesetting
    __this->microphone.sample_rate = sample_rate;   //选择采样率
    as_t = &audio->as[find_alternatesetting - 1];
    u8 target_ep = as_t->ep;
    u8 ep_interval = as_t->ep_Interval;
    as_t->host_ep = host_ep;

    usb_set_interface(host_dev, audio->interface_num, find_alternatesetting); //interface 1  Alternatesetting 1
    usb_audio_sampling_frequency_control(host_dev, target_ep, sample_rate);//设置采样率
    //设置音量
    /* usb_audio_volume_control(host_dev, 6, 1, vol_convert(5)); */
    /* usb_audio_volume_control(host_dev, 6, 2, vol_convert(5)); */
    log_info("D2H ep: %x --> %x", target_ep, host_ep);
    usb_h_set_ep_isr(host_dev, host_ep, usb_audio_rx_isr, host_dev);
    usb_h_ep_config(usb_id,  host_ep, USB_ENDPOINT_XFER_ISOC, 1, ep_interval, ep_buffer, sizeof(ep_in_dma_buf));
    task_create(audio_record_task, host_dev, "uac_record");
    __this->microphone.record_state = AUDIO_RECORD_START;
}
void usb_audio_stop_record(const usb_dev usb_id)
{
    const struct usb_host_device *host_dev = host_id2device(usb_id);
    usb_h_set_ep_isr(NULL, 0, NULL, NULL);
    __this->microphone.get_buf(NULL, 0);
    __this->microphone.Cur_AlternateSetting = 0;
    __this->microphone.sample_rate = 0;
    if (__this->microphone.usb_audio_record_buf) {
        free(__this->microphone.usb_audio_record_buf);
        __this->microphone.usb_audio_record_buf = NULL;
    }
    if (__this->microphone.usb_audio_record_buf2) {
        free(__this->microphone.usb_audio_record_buf2);
        __this->microphone.usb_audio_record_buf2 = NULL;
    }
    printf("\n[ debug ]--func=%s line=%d\n", __func__, __LINE__);
    task_kill("uac_record");
    printf("\n[ debug ]--func=%s line=%d\n", __func__, __LINE__);
}
void usb_audio_pause_record(void)
{
    __this->microphone.record_state = AUDIO_RECORD_PAUSE;
}
void usb_audio_resume_record(void)
{
    const struct usb_host_device *host_dev = host_id2device(__this->usb_id);
    struct audio_device_t *audio = __find_microphone_interface(host_dev);
    struct audio_streaming_t *as_t = &audio->as[__this->microphone.Cur_AlternateSetting - 1];
    __this->microphone.record_state = AUDIO_RECORD_START;
    usb_h_ep_read_async(__this->usb_id, as_t->host_ep, as_t->ep, NULL, 0, USB_ENDPOINT_XFER_ISOC, 1); //重新启动接收
}

void usb_audio_start_process(u32 id)
{
    usb_audio_start_play(id, 1, 16, 48000); //开启headphone
    usb_audio_start_record(id, 16, 48000); //开启microphone
}
void usb_audio_stop_process(u32 id)
{
    usb_audio_stop_play(id);
    usb_audio_stop_record(id);
}


static void usb_audio_event_handler(struct sys_event *event, void *priv)
{
    const char *audio = NULL;
    switch (event->type) {
    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_USB_HOST) {
            if ((event->u.dev.event == DEVICE_EVENT_IN) ||
                (event->u.dev.event == DEVICE_EVENT_CHANGE)) {
                audio = (const char *)event->u.dev.value;
                usb_audio_start_process(audio[5] - '0');

            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
                log_error("device out %x", event->u.dev.value);
                usb_audio_stop_process(audio[5] - '0');
            }
            break;
        }
    }
}
void usb_host_audio_init(int (*put_buf)(void *ptr, u32 len), int *(*get_buf)(void *ptr, u32 len))
{
    memset(__this, 0, sizeof(struct usb_audio_info));
    __this->player.put_buf = put_buf;
    __this->microphone.get_buf = get_buf;
    register_sys_event_handler(SYS_DEVICE_EVENT, DEVICE_EVENT_FROM_USB_HOST, 2,
                               usb_audio_event_handler);
}
void usb_host_audio_exit()
{
    unregister_sys_event_handler(usb_audio_event_handler);
    __this->player.put_buf = NULL;
    __this->microphone.get_buf = NULL;
}

#endif
