/*****************************************************************
> file name : sound.h
>
*****************************************************************/
#ifndef _AUDIO_SOUND_H_
#define _AUDIO_SOUND_H_
#include "generic/ioctl.h"
#include "sound/pcm.h"
#include "os/os_api.h"

#define SOUND_PCM_DMA_CFIFO_MODE        0   /*FIFO与DMA地址映射关系方式*/
#define SOUND_PCM_DMA_PERIOD_MODE       1   /*DMA周期采样方式*/

/*
 * Sound声道分布定义
 */
#define SOUND_CHMAP_MONO                (1 << 0)
#define SOUND_CHMAP_FL                  SOUND_CHMAP_MONO
#define SOUND_CHMAP_FR                  (1 << 1)
#define SOUND_CHMAP_RL                  (1 << 2)
#define SOUND_CHMAP_RR                  (1 << 3)

struct sound_volume;
/*
 * Sound control IOCTL命令
 */
//#define SNDCTL_IOCTL_GET_GAIN_RANGE         _IOR('A', 1, sizeof(struct sound_volume))
#define SNDCTL_IOCTL_POWER_ON               _IOW('A', 1, sizeof(int))                   /* 控制器上电 */
#define SNDCTL_IOCTL_POWER_OFF              _IOW('A', 2, sizeof(int))                   /* 控制器断电 */
#define SNDCTL_IOCTL_SET_ANA_GAIN           _IOW('A', 5, sizeof(struct sound_volume))   /* 设置模拟增益 */
#define SNDCTL_IOCTL_GET_ANA_GAIN           _IOR('A', 6, sizeof(struct sound_volume))   /* 获取模拟增益 */
#define SNDCTL_IOCTL_SET_DIG_GAIN           _IOW('A', 7, sizeof(struct sound_volume))   /* 设置数字增益 */
#define SNDCTL_IOCTL_GET_DIG_GAIN           _IOR('A', 8, sizeof(struct sound_volume))   /* 获取数字增益 */
#define SNDCTL_IOCTL_SET_BIAS_TRIM          _IOW('A', 9, sizeof(int))                   /* TRIM */


struct sound_volume {
    u32 chmap;
    s16 volume[4];
};
/*
 * PCM设备软硬件平台配置
 */
struct sound_pcm_platform_data {
    void *dma_addr;             /*DMA 地址*/
    int  dma_bytes;             /*DMA 字节长度*/
    int	 fifo_bytes;            /*FIFO 长度*/
    void *private_data;         /*Soc私有数据*/
};

/*
 * 驱动控制器，由驱动实现
 */
struct sound_drv_controller {
    int (*power_on)(void *device);                          /*上电*/
    int (*power_off)(void *device);                         /*断电*/
    int (*ioctl)(void *device, int cmd, void *args);        /*Control控制函数*/
};

/*
 * Platform驱动，由驱动实现
 */
struct sound_platform_driver {
    const char *name;
    const struct sound_drv_controller *controller;
    const struct sound_pcm_ops *ops;
    int (*create)(void **device, struct sound_pcm_platform_data *data); /*创建新的设备*/
    void (*free)(void *device);                             /*关闭设备*/

};


/*
 * Platform挂载的子设备
 */
struct sound_platform_subdevice {
    char name[8];
    //const char *name;
    struct sound_platform_driver *driver;
    void *private_data;
    void *parent;
    struct list_head entry;
    OS_MUTEX mutex;
};

/*
 * Platform管理总结构
 */
struct sound_platform {
    struct list_head list;
};
/*
 *
 *
 */
int sound_platform_init(void);
int sound_platform_load(const char *name, struct sound_pcm_platform_data *data);
int sound_platform_power_on(const char *name);
int sound_platform_power_off(const char *name);
int sound_platform_free(const char *name);

int sound_platform_register(const struct sound_platform_driver *driver);

#define SOUND_PLATFORM_DRIVER(name) \
    const struct sound_platform_driver name sec(.sound_platform_driver)

/*
typedef int (*sound_register_t)(void);

#define __sound_platform_init(func)  \
	const sound_register_t __##func sec(.sound_platform_register) = func
*/
#endif
