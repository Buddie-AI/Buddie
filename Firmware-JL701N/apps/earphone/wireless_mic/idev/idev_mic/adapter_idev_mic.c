#include "adapter_idev_mic.h"
#include "adapter_process.h"
#include "adapter_adc.h"

#if TCFG_WIRELESS_MIC_ENABLE


#define ADAPTER_IDEV_MIC_DEBUG_ENABLE

#ifdef ADAPTER_IDEV_MIC_DEBUG_ENABLE
#define adapter_idev_mic_printf     printf
#define adapter_idev_mic_putchar    putchar
#define adapter_idev_mic_putbuf     put_buf
#else
#define adapter_idev_mic_printf (...)
#define adapter_idev_mic_putchar(...)
#define adapter_idev_mic_putbuf (...)
#endif  //ADAPTER_IDEV_MIC_DEBUG_ENABLE




#if (TCFG_AUDIO_ADC_ENABLE)


#define ADAPTER_IDEV_MIC_DEBUG_ENABLE

#ifdef ADAPTER_IDEV_MIC_DEBUG_ENABLE
#define adapter_idev_mic_printf     printf
#define adapter_idev_mic_putchar    putchar
#define adapter_idev_mic_putbuf     put_buf
#else
#define adapter_idev_mic_printf (...)
#define adapter_idev_mic_putchar(...)
#define adapter_idev_mic_putbuf (...)
#endif  //ADAPTER_IDEV_MIC_DEBUG_ENABLE




static int adapter_idev_mic_open(void *parm)
{
    //adc初始化
    adapter_adc_init();
    //事件通知主流程， idev设备初始化完成
    adapter_process_event_notify(ADAPTER_EVENT_IDEV_INIT_OK, 0);
    return 0;
}
static int adapter_idev_mic_close(void)
{
    //事件通知主流程停止媒体
    adapter_process_event_notify(ADAPTER_EVENT_IDEV_MEDIA_CLOSE, 0);
    return 0;
}

static int adapter_idev_mic_start(struct adapter_media *media)
{
    //idev设备启动，这里mic输入比较简单， 这里直接告知主流程请求启动音频媒体
    adapter_process_event_notify(ADAPTER_EVENT_IDEV_MEDIA_OPEN, 0);
    return 0;
}

static void adapter_idev_mic_stop(void)
{
}


static int adapter_idev_mic_event_handle(struct sys_event *e)
{
    //这里可以响应系统事件，根据设备情景， 将idev设备需要响应的事件在这里统一处理
    //可以响应按键事件、设备事件等， 根据实际情景自行解析
    int ret = 0;
    switch (e->type) {
    case SYS_DEVICE_EVENT:
        break;
    default:
        break;
    }
    return ret;
}

REGISTER_ADAPTER_IDEV(adapter_idev_mic) = {
    .id = ADAPTER_IDEV_MIC,
    .open = adapter_idev_mic_open,
    .close = adapter_idev_mic_close,
    .start = adapter_idev_mic_start,
    .stop = adapter_idev_mic_stop,
    .event_fun = adapter_idev_mic_event_handle,
};

#endif//TCFG_AUDIO_ADC_ENABLE
#endif
