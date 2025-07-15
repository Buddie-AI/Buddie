#include "adapter_process.h"
//#include "adapter_media.h"
#include "app_task.h"
#include "adapter_idev.h"
#include "adapter_odev.h"

#if TCFG_WIRELESS_MIC_ENABLE
#include "clock_cfg.h"

static int adapter_process_media_start_callback(void *priv, u8 mode, u8 status, void *parm)
{
    struct adapter_pro *pro = (struct adapter_pro *)priv;


    if (pro == NULL || pro->media == NULL)	{
        return 0;
    }

    adapter_media_stop(pro->media);

    pro->media->idev = pro->in;
    pro->media->odev = pro->out;
    adapter_media_start(pro->media);

    pro->media_lock = 0;
    return 0;
}
struct adapter_pro *adapter_process_open(struct idev *in, struct odev *out, struct adapter_media *media, int (*event)(struct sys_event *event))
{
    struct adapter_pro *pro = zalloc(sizeof(struct adapter_pro));
    if (pro == NULL) {
        return NULL;
    }

    pro->in = in;
    pro->out = out;
    pro->media = media;
    pro->event_handle = event;

    clock_add_set(ADAPTER_PROCESS_CLK);

    return pro;
}

void adapter_process_close(struct adapter_pro **hdl)
{
    if (hdl == NULL || *hdl == NULL)		{
        return ;
    }
    struct adapter_pro *pro = *hdl;

    //其他模块关闭
    adapter_media_stop(pro->media);

    adapter_idev_stop(pro->in);
    adapter_odev_stop(pro->out);

    //释放句柄
    local_irq_disable();
    free(pro);
    *hdl = NULL;
    local_irq_enable();
    clock_remove_set(ADAPTER_PROCESS_CLK);
}


static int adapter_device_event_parse(struct adapter_pro *pro, struct sys_event *e)
{
    u8 event = e->u.dev.event;
    int value = e->u.dev.value;

    switch (event) {
    //初始化完成
    case ADAPTER_EVENT_IDEV_INIT_OK:
        printf("ADAPTER_EVENT_IDEV_INIT_OK\n");
        adapter_idev_start(pro->in, pro->media);
        break;
    case ADAPTER_EVENT_ODEV_INIT_OK:
        printf("ADAPTER_EVENT_ODEV_INIT_OK\n");
        adapter_odev_start(pro->out, pro->media);
        break;

    //媒体相关事件
    case ADAPTER_EVENT_IDEV_MEDIA_OPEN:
        printf("ADAPTER_EVENT_IDEV_MEDIA_OPEN\n");
        pro->mode = (u8)value;
        pro->dev_status |= BIT(ADAPTER_INDEX_IDEV);
        if ((pro->dev_status & 0x3) == 0x3) {
            //prepare media
            if (pro->media_lock == 0) {
                pro->media_lock = 1;
                if (adapter_odev_media_prepare(pro->out, pro->mode, adapter_process_media_start_callback, (void *)pro)) {
                    //prepare 返回值为非0， 需要主动启动媒体
                    printf("adapter_odev_media_prepare null\n");
                    adapter_process_media_start_callback(pro, pro->mode, 1, NULL);
                }
            } else {
                g_f_printf("%s, %d, pro->media_lock!!!!!\n", __FUNCTION__, __LINE__);
            }
        }
        break;
    case ADAPTER_EVENT_IDEV_MEDIA_CLOSE:
        pro->mode = 0xff;
        pro->dev_status &= ~BIT(ADAPTER_INDEX_IDEV);
        adapter_odev_media_pp(pro->out, 0);
        adapter_media_stop(pro->media);
        break;
    case ADAPTER_EVENT_ODEV_MEDIA_OPEN:
        printf("==================ADAPTER_EVENT_ODEV_MEDIA_OPEN\n");
        pro->dev_status |= BIT(ADAPTER_INDEX_ODEV);
        if ((pro->dev_status & 0x3) == 0x3) {
            //prepare media
            if (pro->media_lock == 0) {
                pro->media_lock = 1;
                if (adapter_odev_media_prepare(pro->out, pro->mode, adapter_process_media_start_callback, (void *)pro)) {
                    //prepare 返回值为非0， 需要主动启动媒体
                    printf("adapter_odev_media_prepare null\n");
                    adapter_process_media_start_callback(pro, pro->mode, 1, NULL);
                }
            } else {
                g_f_printf("%s, %d, pro->media_lock!!!!!\n", __FUNCTION__, __LINE__);
            }
        }
        break;
    case ADAPTER_EVENT_ODEV_MEDIA_CLOSE:
        pro->media_lock = 0;
        pro->dev_status &= ~BIT(ADAPTER_INDEX_ODEV);
        adapter_media_stop(pro->media);
        break;

    default:
        break;
    }
    return 0;
}

static int adapter_process_event_parse(struct adapter_pro *pro, struct sys_event *e)
{
    int ret = 0;

    //输入设备事件处理
    ret = adapter_idev_event_deal(pro->in, e);
    if (ret) {
        return ret;
    }

    //输出设备事件处理
    ret = adapter_odev_event_deal(pro->out, e);
    if (ret) {
        return ret;
    }

    //公共事件处理
    switch (e->type) {
    case SYS_KEY_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        switch ((u32)e->arg) {
        case DEVICE_EVENT_FROM_ADAPTER:
            ret = adapter_device_event_parse(pro, e);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return ret;
}

int adapter_process_run(struct adapter_pro *pro)
{
    if (pro == NULL) {
        return false;
    }

    int msg[32];
    int ret = 0;
    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        if (adapter_process_event_parse(pro, (struct sys_event *)&msg[1]) == 0) {
            if (pro->event_handle) {
                ret = pro->event_handle((struct sys_event *)(&msg[1]));
            }
        }
        if (ret) {
            break;
        }
    }
    return ret;
}

void adapter_process_event_notify(u8 event, int value)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg = (void *)DEVICE_EVENT_FROM_ADAPTER;
    e.u.dev.event = event;
    e.u.dev.value = value;

    sys_event_notify(&e);
}
#endif
