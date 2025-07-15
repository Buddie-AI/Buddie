#include "adapter_media.h"
#include "adapter_adc.h"
#include "adapter_wireless_dec.h"
#include "adapter_wireless_enc.h"

#if	(TCFG_WIRELESS_MIC_ENABLE)
struct adapter_media g_adapter_media = {0};

static void adapter_adc_mic_data_callback(void *priv, void *buf, int len)
{
    adapter_wireless_enc_write(buf, len);
}

struct adapter_media *adapter_media_open(void *parm)
{
    memset(&g_adapter_media, 0, sizeof(struct adapter_media));
    return &g_adapter_media;
}

void adapter_media_close(struct adapter_media *media)
{
    adapter_media_stop(media);
}

void adapter_media_stop(struct adapter_media *media)
{
    printf("adapter_media_stop\n");
    if (media) {
        if (media->idev && media->odev) {
            if (media->status) {
                if (media->idev->id == ADAPTER_IDEV_MIC && media->odev->id == ADAPTER_ODEV_BT)	{
                    printf("adapter_media_stop 1\n");
                    adapter_adc_mic_close();
                    adapter_wireless_enc_close();
                } else if (media->idev->id == ADAPTER_IDEV_BT && media->odev->id == ADAPTER_ODEV_DAC)	{
                    printf("adapter_media_stop 2\n");
                    adapter_wireless_dec_close();
                }
                media->status = 0;
            }
        }
    }
}

int adapter_media_start(struct adapter_media *media)
{
    if (media) {
        if (media->idev && media->odev) {
            if (media->status) {
                printf("media start aready\n");
                return 0;
            }
            //start编解码
            if (media->idev->id == ADAPTER_IDEV_MIC && media->odev->id == ADAPTER_ODEV_BT)	{
                //启动音频编码发射
                adapter_adc_mic_open(44100, 10, 120);
                adapter_wireless_enc_open();
                adapter_adc_mic_data_callback_register(adapter_adc_mic_data_callback, NULL);
                adapter_adc_mic_start();
            } else if (media->idev->id == ADAPTER_IDEV_BT && media->odev->id == ADAPTER_ODEV_DAC)	{
                //启动解码输出dac
                adapter_wireless_dec_open();
            }
            media->status = 1;
        }
    }
    return 0;
}
#endif//TCFG_WIRELESS_MIC_ENABLE



