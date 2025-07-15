#include "adapter_odev.h"
#include "adapter_process.h"
#include "adapter_odev_dac.h"
#include "audio_config.h"
//#include "adapter_audio_stream.h"
#include "asm/dac.h"
#if TCFG_WIRELESS_MIC_ENABLE
#if (TCFG_AUDIO_DAC_ENABLE)
//struct audio_dac_channel default_dac = {0};
extern struct audio_dac_hdl dac_hdl;

static int adapter_odev_dac_open(void *parm)
{
    adapter_process_event_notify(ADAPTER_EVENT_ODEV_INIT_OK, 0);
    return 0;
}
static int adapter_odev_dac_start(void *priv, struct adapter_media *media)
{
    //通知主流程请求启动音频媒体
    adapter_process_event_notify(ADAPTER_EVENT_ODEV_MEDIA_OPEN, 0);
    return 0;
}


static int adapter_odev_dac_stop(void *priv)
{
    return 0;
}
static int adapter_odev_dac_close(void)
{
    //通知主流程请求停止音频媒体
    adapter_process_event_notify(ADAPTER_EVENT_ODEV_MEDIA_CLOSE, 0);
    return 0;
}


static int adapter_odev_dac_pp_ctl(u8 pp)
{
    return 0;
}


static int adapter_odev_dac_get_status(void *priv)
{
    return 0;
}


static int adapter_odev_dac_config(int cmd, void *parm)
{
    return 0;
}

REGISTER_ADAPTER_ODEV(adapter_odev_dac) = {
    .id     = ADAPTER_ODEV_DAC,
    .open   = adapter_odev_dac_open,
    .start  = adapter_odev_dac_start,
    .stop   = adapter_odev_dac_stop,
    .close  = adapter_odev_dac_close,
    .media_pp = NULL,
    .get_status = NULL,
    .media_prepare = NULL,
    .event_fun  = NULL,
    .config = NULL,
};
#endif
#endif//TCFG_AUDIO_DAC_ENABLE
