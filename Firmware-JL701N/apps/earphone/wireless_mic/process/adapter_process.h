

#ifndef __ADAPTER_PROCESS_H__
#define __ADAPTER_PROCESS_H__

#include "app_config.h"
#include "generic/typedef.h"
#include "media/includes.h"
#include "stream/stream_entry.h"
#include "adapter_idev.h"
#include "adapter_odev.h"
//#include "adapter_media.h"

#define ADAPTER_INDEX_IDEV		0x0
#define ADAPTER_INDEX_ODEV		0x1

enum adapter_event {
    /* --- 适配事件定义 --- */
    ADAPTER_EVENT_IDEV_INIT_OK = 0x0,
    ADAPTER_EVENT_IDEV_MEDIA_OPEN,
    ADAPTER_EVENT_IDEV_MEDIA_CLOSE,


    ADAPTER_EVENT_ODEV_INIT_OK = 0x80,
    ADAPTER_EVENT_ODEV_MEDIA_OPEN,
    ADAPTER_EVENT_ODEV_MEDIA_CLOSE,


};

struct adapter_pro {
    /* --- 适配器process模块控制句柄 --- */
    u8 mode;										/*!< 跟音频媒体media_sel一样， 指的当前媒体选择 */
    u8 dev_status;									/*!< idev和odev 音频请求状态， idev&&odev都请求启动音频媒体，音频媒体才会正式启动*/
    u8 media_lock;									/*!< 音频媒体锁定状态标记 */
    struct idev *in;								/*!< idev控制句柄 */
    struct odev *out;								/*!< odev控制句柄 */
    struct adapter_media 	 *media;				/*!< 音频媒体控制句柄 */
    int (*event_handle)(struct sys_event *event);	/*!< 用户事件拦截回调函数 */
};

/**
 * @brief 适配器主流程创建
 *
 * @param  	in idev控制句柄
 * @param  	out odev控制句柄
 * @param  	media 音频媒体控制句柄
 * @param  	event 用户自定义事件拦截回调函数
 * @return	适配器主流程控制句柄
 */
struct adapter_pro *adapter_process_open(struct idev *in, struct odev *out, struct adapter_media *media, int (*event)(struct sys_event *event));
/**
 * @brief 适配器主流程关闭
 *
 * @param  	hdl 适配器主流程控制句柄双重指针
 */
void adapter_process_close(struct adapter_pro **hdl);
/**
 * @brief 适配器主流程关闭
 *
 * @param  	pro 适配器主流程控制句柄
 */
int adapter_process_run(struct adapter_pro *pro);
/**
 * @brief 适配器主流程事件通知接口
 *
 * @param  	event 事件
 * @param  	value 事件参数或者附带内容值
 */
void adapter_process_event_notify(u8 event, int value);

#endif//__ADAPTER_PROCESS_H__

