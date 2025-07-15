/**@file        usb_bulk_transfer.h
  * @brief      usb_bulk_transfer批量传输头文件
  * @details    功能函数声明
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
#ifndef __USB_BULK_TRANSFER_H__
#define __USB_BULK_TRANSFER_H__
#include "typedef.h"
#include "device/device.h"


/**@brief   批量传输只读取（异步模式）
  * @param[in] device定义的结构体指针
  * @param[in] host_ep 主机端点号
  * @param[in] rxmaxp 接收端点最大包长
  * @param[in] target_ep 目标端点号
  * @param[in] *pBuf BUFFER指针
  * @param[in] len 数据长度
  * @return
  * @par    示例：
  * @code
  * usb_bulk_only_receive_async(device, host_ep, rxmaxp, target_ep, pBuf, len);
  * @encode
  */
s32 usb_bulk_only_receive_async(struct device *device, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *pBuf, u32 len);

/**@brief   批量传输只读取（普通模式）
  * @param[in] device定义的结构体指针
  * @param[in] host_ep 主机端点号
  * @param[in] rxmaxp 接收端点最大包长
  * @param[in] ep 目标端点号
  * @param[in] *pBuf BUFFER指针
  * @param[in] len 数据长度
  * @return
  * @par    示例：
  * @code
  * usb_bulk_only_receive(device, host_ep, rxmaxp, ep, pBuf, len);
  * @encode
  */
s32 usb_bulk_only_receive(struct device *device, u8 host_ep, u16 rxmaxp, u8 ep, u8 *pBuf, u32 len);

/**@brief   批量传输只发送（异步模式）
  * @param[in] device定义的结构体指针
  * @param[in] host_ep 主机端点号
  * @param[in] txmaxp 发送端点最大包长
  * @param[in] target_ep 目标端点号
  * @param[in] *pBuf BUFFER指针
  * @param[in] len 数据长度
  * @return
  * @par    示例：
  * @code
  * usb_bulk_only_send_async(device, host_ep, txmaxp, target_ep, pBuf, len);
  * @encode
  */
s32 usb_bulk_only_send_async(struct device *device, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *pBuf, u32 len);

/**@brief   批量传输只发送（普通模式）
  * @param[in] device定义的结构体指针
  * @param[in] ep 主机端点号
  * @param[in] txmaxp 发送端点最大包长
  * @param[in] ep 目标端点号
  * @param[in] *pBuf BUFFER指针
  * @param[in] len 数据长度
  * @return
  * @par    示例：
  * @code
  * usb_bulk_only_send(device, host_ep, txmaxp, ep, pBuf, len);
  * @encode
  */
s32 usb_bulk_only_send(struct device *device, u8 host_ep, u16 txmaxp, u8 ep, const u8 *pBuf, u32 len);

/**@brief   获取异步模式当前状态
  * @param[in] 空
  * @return    当前状态
  * @par    示例：
  * @code
  * get_async_mode();
  * @encode
  */
u8 get_async_mode(void);

/**@brief   设置异步模式
  * @param[in] mode 需要设置的模式
  * @return    空
  * @par    示例：
  * @code
  * set_async_mode(BULK_ASYNC_MODE_EXIT);退出异步模式
  * @encode
  */
void set_async_mode(u8 mode);

/**@brief   批量传输异步模式读取并且不等待
  * @param[in] device定义的结构体指针
  * @param[in] ep 主机端点号
  * @param[in] rxmaxp 发送端点最大包长
  * @param[in] target_ep 目标端点号
  * @param[in] *pBuf BUFFER指针
  * @param[in] len 数据长度
  * @return
  * @par    示例：
  * @code
  * usb_bulk_only_send(device, host_ep, rxmaxp, ep, pBuf, len);
  * @encode
  */
s32 usb_bulk_receive_async_no_wait(struct device *device, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *pBuf, u32 len);
#endif
