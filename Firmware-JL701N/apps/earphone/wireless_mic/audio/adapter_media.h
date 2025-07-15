#ifndef __ADAPTER_MEDIA_H__
#define __ADAPTER_MEDIA_H__

#include "app_config.h"
#include "generic/typedef.h"
#include "media/includes.h"
#include "adapter_idev.h"
#include "adapter_odev.h"


struct adapter_media {
    u8 status;
    //...
    struct idev *idev;
    struct odev *odev;
};


struct adapter_media *adapter_media_open(void *parm);
void adapter_media_close(struct adapter_media *media);
int adapter_media_start(struct adapter_media *media);
void adapter_media_stop(struct adapter_media *media);

#endif//__ADAPTER_MEDIA_H__

