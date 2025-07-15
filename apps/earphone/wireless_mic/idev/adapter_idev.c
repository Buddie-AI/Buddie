#include "adapter_idev.h"
#include "app_config.h"

#if TCFG_WIRELESS_MIC_ENABLE

struct idev *adapter_idev_open(u16 id, void *parm)
{
    struct idev *dev = NULL;
    list_for_each_adapter_idev(dev) {
        if (dev->id == id) {
            if (dev->open) {
                dev->open(parm);
            }
            return dev;
        }
    }

    return NULL;
}

void adapter_idev_close(struct idev *dev)
{
    if (dev && dev->close) {
        dev->close();
    }
}

int adapter_idev_start(struct idev *dev, struct adapter_media *media)
{
    if (dev && dev->start) {
        return dev->start(media);
    }
    return 0;
}

void adapter_idev_stop(struct idev *dev)
{
    if (dev && dev->stop) {
        dev->stop();
    }
}


int adapter_idev_event_deal(struct idev *dev, struct sys_event *event)
{
    if (dev && dev->event_fun) {
        return dev->event_fun(event);
    }
    return 0;
}

#endif
