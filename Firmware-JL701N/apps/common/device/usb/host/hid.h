/**@file        hid.h
  * @brief      hid驱动头文件（做主机）
  * @details    结构体声明，功能函数声明
  * @author     jieli
  * @date       2021-9-1
  * @version    V1.0
  * @copyright  Copyright(c)2010-2021   珠海市杰理科技股份有限公司
  *********************************************************
  * @attention
  * 硬件平台：AC632N
  * SDK版本：AC632N_V1.0.0_SDK
  * @修改日志：
  * <table>
  * <tr><th>Date        <th>Version     <th>Author      <th>Description
  * <tr><td>2021-9-1    <td>1.0         <td>jieli       <td>创建初始版本
  * </table>
  *
  *********************************************************
  */
#ifndef  __HID_H__
#define  __HID_H__

#include "system/task.h"
#include "device/device.h"
#include "usb/scsi.h"
#include "usb_bulk_transfer.h"
#include "usb/host/usb_host.h"

/**@struct  report_info_t
  * @brief  报告描述符包含的信息结构体\n
  * 自定义一些私有数据信息存储在该结构体中
  */
struct report_info_t {
    u8 report_id; ///<id号，不同hid设备对应不同id
    u8 usage; ///<描述该数据信息的用途

    u8 btn_start_bit; ///<鼠标按键数据起始位
    u8 btn_width; ///<鼠标按键数据宽度

    u8 xy_start_bit; ///<鼠标平移数据起始位
    u8 xy_width; ///<鼠标平移数据宽度

    u8 wheel_start_bit; ///<鼠标滚轮数据起始位
    u8 wheel_width; ///<鼠标滚轮数据宽度
};

#define     MAX_REPORT_COUNT    4

/**@struct  hid_device_t
  * @brief  报告描述符包含的信息结构体\n
  * 自定义一些私有数据信息存储在该结构体中
  */
struct hid_device_t {
    void *parent; ///<定义的parent指针，指向该hid的所属设备
    u8 ep_pair[4]; ///<端点对数组，数组下标存放主机端点，对应的元素存放目标端点
    u8 report_count; ///<报告描述符中item计数器
    u8 bNumEndpoints; ///<端点数量
    struct report_info_t report_list[MAX_REPORT_COUNT]; ///<报告描述符结构体
};

/**@brief   USB hid设备解析
  * @param[in]  host_dev usb_host_device定义的结构体指针
  * @param[in]  interface_num 接口号
  * @param[in]  *pBuf 数据指针,指向数据存放的地址
  * @return     接口描述符长度
  * @par    示例：
  * @code
  * usb_hid_parser(host_dev,interface_num,pBuf);
  * @encode
  */
int usb_hid_parser(struct usb_host_device *host_dev, u8 interface_num, const u8 *pBuf);

/**@brief   USB hid设备信息处理
  * @param[in]  id usb设备id号
  * @return 无
  * @par    示例：
  * @code
  * hid_process(id);
  * @encode
  */
void hid_process(u32 id);

#endif  /*HID_H*/
