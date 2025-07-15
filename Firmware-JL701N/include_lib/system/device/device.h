#ifndef CHRDEV_H
#define CHRDEV_H


#include "generic/typedef.h"
#include "generic/list.h"
//#include "generic/ioctl.h"
#include "generic/atomic.h"
//#include "sys/task.h"
#include "device/ioctl_cmds.h"


struct dev_node;
struct device;


/**@struct  device_operations
  * @brief  device_operations结构体 \n
  * otg设备执行哪种类型的操作
  */
struct device_operations {
    bool (*online)(const struct dev_node *node); ///<设备在线状态查询
    int (*init)(const struct dev_node *node, void *); ///<设备初始化
    int (*open)(const char *name, struct device **device, void *arg); ///<设备开启
    int (*read)(struct device *device, void *buf, u32 len, u32); ///<读操作
    int (*write)(struct device *device, void *buf, u32 len, u32); ///<写操作
    int (*seek)(struct device *device, u32 offset, int orig); ///<设备搜索
    int (*ioctl)(struct device *device, u32 cmd, u32 arg); ///<I/O控制
    int (*close)(struct device *device); ///<设备关闭
};

struct dev_node {
    const char *name;
    const struct device_operations *ops;
    void *priv_data;
};


struct device {
    atomic_t ref;
    void *private_data;
    const struct device_operations *ops;
    void *platform_data;
    void *driver_data;
};


#define REGISTER_DEVICE(node) \
    const struct dev_node node sec(.device)

#define REGISTER_DEVICES(node) \
    const struct dev_node node[] sec(.device)


int devices_init();

bool dev_online(const char *name);

void *dev_open(const char *name, void *arg);


int dev_read(void *device, void *buf, u32 len);


int dev_write(void *device, void *buf, u32 len);


int dev_seek(void *device, u32 offset, int orig);


int dev_ioctl(void *device, int cmd, u32 arg);


int dev_close(void *device);


int dev_bulk_read(void *_device, void *buf, u32 offset, u32 len);

int dev_bulk_write(void *_device, void *buf, u32 offset, u32 len);

#endif


