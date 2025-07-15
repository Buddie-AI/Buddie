#include "adapter_odev.h"

#if TCFG_WIRELESS_MIC_ENABLE

struct odev *adapter_odev_open(u16 id, void *parm)
{
    g_printf("adapter_odev_open\n");
    struct odev *dev = NULL;
    list_for_each_adapter_odev(dev) {
        if (dev->id == id) {
            if (dev->open) {
                dev->open(parm);
            }
            return dev;
        }
    }
    return NULL;
}

void adapter_odev_start(struct odev *dev, struct adapter_media *media)
{
    g_printf("adapter_odev_start\n");
    if (dev && dev->start) {
        dev->start(NULL, media);
    }
    return;
}

void adapter_odev_stop(struct odev *dev)
{
    g_printf("adapter_odev_stop\n");
    if (dev && dev->stop) {
        dev->stop(NULL);
    }
    return;
}

void adapter_odev_close(struct odev *dev)
{
    g_printf("adapter_odev_close\n");
    if (dev && dev->close) {
        dev->close(NULL);
    }
    return;
}

int adapter_odev_get_status(struct odev *dev)
{
    g_printf("adapter_odev_get_status\n");
    if (dev && dev->get_status) {
        return (dev->get_status(NULL));
    }
    return 0;
}

int adapter_odev_media_pp(struct odev *dev, u8 pp)
{
    g_printf("adapter_odev_media_pp\n");
    if (dev && dev->media_pp) {
        return (dev->media_pp(pp));
    }
    return 0;
}

int adapter_odev_media_prepare(struct odev *dev, u8 mode, int (*fun)(void *, u8, u8, void *), void *priv)
{
    g_printf("adapter_odev_media_prepare\n");
    if (dev && dev->media_prepare) {
        return dev->media_prepare(mode, fun, priv);
    }
    return -1;
}

int adapter_odev_event_deal(struct odev *dev, struct sys_event *event)
{
    if (dev && dev->event_fun) {
        return dev->event_fun(event);
    }
    return 0;
}

void adapter_odev_config(struct odev *dev, int cmd, void *priv)
{
    g_printf("adapter_odev_config\n");
    if (dev && dev->config) {
        dev->config(cmd, priv);
    }
    return;
}

int adapter_odev_output(struct odev *dev, void *priv, u8 *buf, u16 len)
{
    int ret = 0;
    if (dev && dev->output) {
        ret = dev->output(priv, buf, len);
    }

    return ret;
}
#endif
