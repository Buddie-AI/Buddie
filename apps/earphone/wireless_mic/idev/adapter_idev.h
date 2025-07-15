#ifndef __ADAPTER_IDEV_H__
#define __ADAPTER_IDEV_H__

#include "generic/typedef.h"
#include "adapter_media.h"
#include "wireless_mic_test.h"
#include "app_config.h"


enum adapter_idev_type {
    ADAPTER_IDEV_USB = 0x0,
    ADAPTER_IDEV_MIC,
    ADAPTER_IDEV_BT,
};


struct idev {
    u16 id;
    int (*open)(void *parm);
    void (*close)(void);
    int (*start)(struct adapter_media *media);
    void (*stop)(void);
    int (*event_fun)(struct sys_event *);
    //其他操作
};

struct idev *adapter_idev_open(u16 id, void *parm);
void adapter_idev_close(struct idev *dev);
int adapter_idev_start(struct idev *dev, struct adapter_media *media);
void adapter_idev_stop(struct idev *dev);
int adapter_idev_event_deal(struct idev *dev, struct sys_event *event);

#define REGISTER_ADAPTER_IDEV(ops) \
        const struct idev ops sec(.adapter_idev)

extern const struct idev adapter_idev_begin[];
extern const struct idev adapter_idev_end[];

#define list_for_each_adapter_idev(p) \
    for (p = adapter_idev_begin; p < adapter_idev_end; p++)

#endif//__ADAPTER_IDEV_H__

