/**@file        usb_host.h
  * @brief      usb_host驱动头文件（做主机）
  * @details    结构体声明，功能函数声明
  * @author     jieli
  * @date       2021-8-1
  * @version    V1.0
  * @copyright  Copyright(c)2010-2021   珠海市杰理科技股份有限公司
  *********************************************************
  * @attention
  * 硬件平台：AC695N
  * SDK版本：AC695N_V1.0.0_SDK
  * @修改日志：
  * <table>
  * <tr><th>Date        <th>Version     <th>Author      <th>Description
  * <tr><td>2021-8-1    <td>1.0         <td>jieli       <td>创建初始版本
  * </table>
  *
  *********************************************************
  */
#ifndef  __USB_HOST_H__
#define  __USB_HOST_H__
#include "system/task.h"
#include "device/device.h"
#include "asm/usb.h"
#include "usb/ch9.h"
#include "usb/usb_phy.h"
// #include "usb_config.h"


#define USB_HUB      0

/**@struct  usb_private_data
  * @brief  usb_private_data私有数据结构体\n
  * 自定义一些私有数据信息存储在该结构体中
  */
struct usb_private_data {
    usb_dev usb_id; ///<USB的id号，如：0 : USB0 ; 1 : USB1
    u8 status; ///<当前状态,如：0:上线 ; 1:下线
    u8 devnum; ///<设备编号
    u8 ep0_max_packet_size;///<端点0最大包长，单位：byte
    /* ///<以下为保留信息，暂时未使用
    u8 speed; ///<传输速度
    u16 vendor_id; ///<供应商
    u16 product_id; ///<产品id号
    u16 language; ///<语言
    u8 manufacturer[64]; ///<制造商
    u8 product[64]; ///<产品序列
    */
};

struct usb_host_device;

/**@struct  interface_ctrl
  * @brief  interface_ctrl
  */
struct interface_ctrl {
    u8 interface_class;
    int (*set_power)(struct usb_host_device *host_dev, u32 value);
    int (*get_power)(struct usb_host_device *host_dev, u32 value);
    int (*ioctl)(struct usb_host_device *host_dev, u32 cmd, u32 arg);
};

/**@struct  usb_interface_info
  *@brief   usb_interface_info
  */
struct usb_interface_info {
    struct interface_ctrl *ctrl;
    union {
        struct mass_storage *disk;
        struct adb_device_t *adb;
        struct hid_device_t *hid;
        struct aoa_device_t *aoa;
        struct audio_device_t *audio;
        void *p;
    } dev;
};
#define     MAX_HOST_INTERFACE  4

/**@struct  usb_host_device
  *@brief   usb_host_device
  */
struct usb_host_device {
#if USB_HUB
    struct usb_host_device *father;
#endif
    OS_SEM *sem;
    struct usb_private_data private_data;
    const struct usb_interface_info *interface_info[MAX_HOST_INTERFACE];
};


#define     device_to_usbdev(device)	((struct usb_host_device *)((device)->private_data))

/**@brief   USB设备id号获取
  * @param[in]  usb_host_device定义的结构体指针
  * @return     USB设备的id号，如 0:USB0 ; 1:USB1
  * @par    示例：
  * @code
  * host_device2id(host_dev);   获取host_dev的USB设备id号
  * @encode
  */
u32 host_device2id(const struct usb_host_device *host_dev);

/**@brief   USB设备状态获取
  * @param[in]  usb_host_device定义的结构体指针
  * @return     USB设备的状态，如 0:下线 ; 1:上线
  * @par    示例：
  * @code
  * host_dev_status(host_dev);   获取host_dev的USB设备状态
  * @encode
  */
int host_dev_status(const struct usb_host_device *host_dev);

/**@brief   获取usb_host_device结构体的信息
  * @param[in]  USB的id号
  * @return     结构体信息的存储首地址
  * @par    示例：
  * @code
  * struct usb_host_device *host_dev = &host_devices[usb_id];
  * @encode
  */
const struct usb_host_device *host_id2device(const usb_dev id);

#define     check_usb_mount(ret)    \
    if(ret == -DEV_ERR_OFFLINE){\
        log_error("%s() @ %d DEV_ERR_OFFLINE\n", __func__, __LINE__);\
        goto __exit_fail;\
    } else if(ret){\
        log_error("%s() @ %d %x\n", __func__, __LINE__, ret);\
        continue;\
    }


typedef void(*usb_h_interrupt)(struct usb_host_device *, u32 ep);

/**@brief   USB_sem初始化
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * host_sem_init(host_dev);
  * @encode
  */
int usb_sem_init(struct usb_host_device *host_dev);

/**@brief   USB_sem申请一个信号量
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  timeout 超时时间设置,单位ms
  * @return
  * @par    示例：
  * @code
  * host_sem_pend(host_dev,1000);
  * @encode
  */
int usb_sem_pend(struct usb_host_device *host_dev, u32 timeout);

/**@brief   USB_sem释放一个信号量
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * host_sem_psot(host_dev);
  * @encode
  */
int usb_sem_post(struct usb_host_device *host_dev);

/**@brief   USB_sem删除
  * @param[in]  usb_host_device定义的结构体指针
  * @return
  * @par    示例：
  * @code
  * host_sem_del(host_dev);
  * @encode
  */
int usb_sem_del(struct usb_host_device *host_dev);

/**@brief   USB主机模式设置端点中断
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  ep 端点号
  * @param[in]  hander usb_h_interrupt定义的指针函数
  * @param[in]  *p
  * @return     无
  * @par    示例：
  * @code
  * usb_h_set_ep_isr(host_dev , 0 , func , host_dev);
  * @encode
  */
void usb_h_set_ep_isr(struct usb_host_device *host_dev, u32 ep, usb_h_interrupt hander, void *p);

/**@brief   USB主机设置中断处理函数
  * @param[in]  usb_id USB的id号
  * @param[in]  ep 端点号
  * @param[in]  hander  usb_h_interrupt定义的指针函数
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_h_set_intr_hander(usb_id , 0 , func);
  * @encode
  */
u32 usb_h_set_intr_hander(const usb_dev usb_id, u32 ep, usb_h_interrupt hander);

/**@brief   USB主机模式挂载
  * @param[in]  usb_id USB的id号
  * @param[in]  retry 主机挂载重试次数
  * @param[in]  reset_delay 复位等待延时 单位ms
  * @param[in]  mount_timeout 挂载超时时间 单位ms
  * @return
  * @par    示例：
  * @code
  * usb_host_mount(usb_id , 5 , 10 , 1000 );
  * @encode
  */
u32 usb_host_mount(const usb_dev usb_id, u32 retry, u32 reset_delay, u32 mount_timeout);

/**@brief   USB主机模式卸载
  * @param[in]  usb_id USB的id号
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_host_unmount(usb_id);
  * @encode
  */
u32 usb_host_unmount(const usb_dev usb_id);

/**@brief   USB主机模式重新挂载
  * @param[in]  usb_id USB的id号
  * @param[in]  retry 主机挂载重试次数
  * @param[in]  delay 复位等待延时 单位ms
  * @param[in]  ot 挂载超时时间 单位ms
  * @param[in]  notify 事件发送开关 1:开启 0:关闭
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_host_remount(usb_id , 5 , 10 , 1000 , 1);
  * @encode
  */
u32 usb_host_remount(const usb_dev usb_id, u32 retry, u32 delay, u32 ot, u8 notify);

/**@brief   USB主机模式挂起
  * @param[in]  usb_id USB的id号
  * @return     无
  * @par    示例：
  * @code
  * usb_host_suspend(usb_id);
  * @encode
  */
void usb_host_suspend(const usb_dev usb_id);

/**@brief   USB主机模式恢复
  * @param[in]  usb_id USB的id号
  * @return     无
  * @par    示例：
  * @code
  * usb_host_resume(usb_id);
  * @encode
  */
void usb_host_resume(const usb_dev usb_id);

int usb_host_force_reset(const usb_dev usb_id);

#endif  /*USB_HOST_H*/
