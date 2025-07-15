#ifndef __ADAPTER_ODEV_H__
#define __ADAPTER_ODEV_H__

#include "generic/typedef.h"
#include "adapter_odev_bt.h"
#include "adapter_media.h"
#include "wireless_mic_test.h"
#include "event.h"

enum adapter_odev_type {
    ADAPTER_ODEV_BT = 0x0,
    ADAPTER_ODEV_DAC,
    ADAPTER_ODEV_USB,
};

enum {
    ADAPTER_ODEV_PAUSE = 0x0,
    ADAPTER_ODEV_PLAY,
};

struct odev {
    u16 id;
    int (*open)(void *);
    int (*start)(void *, struct adapter_media *);
    int (*stop)(void *);
    int (*close)(void *);
    int (*get_status)(void *);
    int (*media_pp)(u8);
    int (*media_prepare)(u8, int (*fun)(void *, u8, u8, void *), void *);
    int (*event_fun)(struct sys_event *);
    int (*config)(int cmd, void *parm);
    int (*output)(void *, u8 *, u16);
};

struct odev *adapter_odev_open(u16 id, void *parm);
void adapter_odev_start(struct odev *dev, struct adapter_media *media);
void adapter_odev_stop(struct odev *dev);
void adapter_odev_close(struct odev *dev);
int adapter_odev_get_status(struct odev *dev);
int adapter_odev_media_pp(struct odev *dev, u8 pp);
int adapter_odev_media_prepare(struct odev *dev, u8 mode, int (*fun)(void *, u8, u8, void *), void *priv);
int adapter_odev_event_deal(struct odev *dev, struct sys_event *event);
void adapter_odev_config(struct odev *dev, int cmd, void *priv);
int adapter_odev_output(struct odev *dev, void *priv, u8 *buf, u16 len);

#define REGISTER_ADAPTER_ODEV(ops) \
        const struct odev ops sec(.adapter_odev)

extern const struct odev adapter_odev_begin[];
extern const struct odev adapter_odev_end[];

#define list_for_each_adapter_odev(p) \
    for (p = adapter_odev_begin; p < adapter_odev_end; p++)

#endif//__ADAPTER_ODEV_H__

