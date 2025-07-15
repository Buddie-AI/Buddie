/**@file        usb_storage.h
  * @brief      usb_storage驱动头文件（做主机）
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
#ifndef __USB_STORAGE_H__
#define __USB_STORAGE_H__

#include "system/task.h"
#include "device/device.h"
#include "usb/scsi.h"
#include "usb_bulk_transfer.h"
#include "usb/host/usb_host.h"

/* u盘预读功能配置, 二选一
 * 当两种方式都不使能，则表示不开启预读 */
#define  UDISK_READ_BIGBLOCK_ASYNC_ENABLE    0   //使能大扇区预读方式(不需要额外buf,速度比512预读慢10%)
#define  UDISK_READ_512_ASYNC_ENABLE         0   //使能512Byte预读方式(需要额外的512byte buffer,速度比大扇区预读快10%)
/****************************/

#define UDISK_READ_ASYNC_BLOCK_NUM  (16) //预读扇区数

//****************************youning***********************************----->
#define UDISK_READ_AHEAD_ENABLE      0   //使能U盘预读功能
#define UDISK_READ_AHEAD_BLOCK_NUM  16  //U盘预读扇区数量
#define     check_usb_read_ahead(ret)    \
    if(ret < 0) {\
        log_error("func:%s,line:%d,lba:%d,ret:%d\n", __func__, __LINE__, lba, ret);\
        goto __exit;\
    }   //检查函数返回值，0:正确    非0:错误
//****************************youning***********************************<-----

/**@enum    usb_sta
  * @brief  USB设备当前状态
  */
typedef enum usb_sta {
    DEV_IDLE = 0, ///<空闲状态
    DEV_INIT, ///<初始化
    DEV_OPEN, ///<开启
    DEV_READ, ///<读操作
    DEV_WRITE, ///<写操作
    DEV_CLOSE, ///<关闭
    DEV_SUSPEND, ///<挂起
} USB_STA ;

/**@struct  udisk_end_desc
  * @brief  U盘端点描述结构体
  *
  */
struct udisk_end_desc {
    u8 host_epout; ///<主机端点输出
    u8 target_epout; ///<目标端点输出
    u8 host_epin; ///<主机端点输入
    u8 target_epin; ///<目标端点输入
#if defined(FUSB_MODE) && FUSB_MODE == 0
    u16 rxmaxp; ///<接收最大端点号
    u16 txmaxp; ///<发送最大端点号
#endif
};

#define ENABLE_DISK_HOTPLUG  0

/**@struct  mass_storage
  * @brief  mass_storage协议所使用的相关变量
  */
struct mass_storage {
    OS_MUTEX mutex; ///<互斥量

    struct usb_scsi_cbw cbw; ///<CBW指令结构体
    struct usb_scsi_csw csw; ///<CSW状态结构体
    struct request_sense_data sense; ///<请求数据检查结构体

    char *name; ///<设备名字
    struct read_capacity_data capacity[2]; ///<读取数据的能力，包含数据块的编号、大小
    u8 lun; ///<最大逻辑单元地址
    u8 curlun; ///<当前逻辑单元地址

    u8 dev_status; ///<设备状态
    u8 suspend_cnt; ///<挂起状态计数器
    u8 read_only; ///<只读标志位

    u32 remain_len; ///<剩余的包长度
    u32 prev_lba; ///<上一次扇区
#if (UDISK_READ_BIGBLOCK_ASYNC_ENABLE || UDISK_READ_512_ASYNC_ENABLE)
    u8 async_en; ///<异步模式使能
    u8 need_send_csw; ///<需要发送csw标志位
    u8 *udisk_512_buf; ///<U盘512K大小BUFFER指针
    u32 async_prev_lba; ///<异步模式上一次地址
#endif
#if UDISK_READ_AHEAD_ENABLE
    u8 *udisk_read_ahead_buf; ///<U盘512byte大小BUFFER指针
    u32 udisk_read_ahead_lba_last; ///<异步模式上一次地址
#endif
#if ENABLE_DISK_HOTPLUG
    u8 media_sta_cur; ///<当前媒介状态                //for card reader, card removable
    u8 media_sta_prev; ///<上次媒介状态

    int test_unit_ready_tick; ///<测试准备标记
#endif
};

enum usb_async_mode {
    BULK_ASYNC_MODE_EXIT = 0, ///<退出异步模式
    BULK_ASYNC_MODE_SEM_PEND, ///<异步预读等待信号量
};

#define MASS_LBA_INIT    (-2)

/**@brief   使用mass_storage协议，对大容量存储设备进行解析，端点配置
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  interface_num 接口号
  * @param[in]  *pBuf 指向BUFFER的指针
  * @return     len    BUFFER的长度
  * @par    示例：
  * @code
  * usb_msd_parser(host_dev,interface_num,pBuf);
  * @encode
  */
int usb_msd_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf);

/**@brief   异步模式等待信号量
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * _usb_stor_async_wait_sem(host_dev);
  * @encode
  */
int _usb_stor_async_wait_sem(struct usb_host_device *host_dev);

#endif
