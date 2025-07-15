#ifndef __SFC_NORFLASH_API_H__
#define __SFC_NORFLASH_API_H__

#include "typedef.h"

int norflash_init(const struct dev_node *node, void *arg);
int norflash_open(const char *name, struct device **device, void *arg);
int norflash_read(struct device *device, void *buf, u32 len, u32 offset);
int norflash_write(struct device *device, void *buf, u32 len, u32 offset);
int norflash_ioctl(struct device *device, u32 cmd, u32 arg);


#endif
