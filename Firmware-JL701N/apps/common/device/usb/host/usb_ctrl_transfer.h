/**@file        usb_ctrl_transfer.h
  * @brief      usb_ctrl_transfer控制传输头文件
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
#ifndef __USB_CTRL_TRANSFER_H__
#define __USB_CTRL_TRANSFER_H__

#include "usb/ch9.h"
#include "usb/usb_phy.h"
#include "device/device.h"
#include "usb_config.h"


/*
 * USB Packet IDs (PIDs)
 */
#define USB_PID_EXT			0xf0	/* USB 2.0 LPM ECN */
#define USB_PID_OUT			0xe1
#define USB_PID_ACK			0xd2
#define USB_PID_DATA0			0xc3
#define USB_PID_PING			0xb4	/* USB 2.0 */
#define USB_PID_SOF			0xa5
#define USB_PID_NYET			0x96	/* USB 2.0 */
#define USB_PID_DATA2			0x87	/* USB 2.0 */
#define USB_PID_SPLIT			0x78	/* USB 2.0 */
#define USB_PID_IN			0x69
#define USB_PID_NAK			0x5a
#define USB_PID_DATA1			0x4b
#define USB_PID_PREAMBLE		0x3c	/* Token mode */
#define USB_PID_ERR			0x3c	/* USB 2.0: handshake mode */
#define USB_PID_SETUP			0x2d
#define USB_PID_STALL			0x1e
#define USB_PID_MDATA			0x0f	/* USB 2.0 */



struct ctlXfer {
    struct usb_ctrlrequest setup; ///<控制请求
    void *buffer; ///<控制请求的data
    u8  stage; ///<当前状态
};

int usb_control_msg(struct usb_host_device *host_dev,
                    u8 request, u8 requesttype,
                    u16 value, u16 index,
                    void *data, u16 size);

/**@brief   USB清除或禁用特定的特性
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  ep 端点号
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_clear_feature(host_dev , ep);
  * @encode
  */
int usb_clear_feature(struct usb_host_device *usb_dev, u32 ep);

/**@brief   USB设置地址
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  devnum 设备编号
  * @return     0:成功
  * @par    示例：
  * @code
  * set_address(host_dev , ep);
  * @encode
  */
int set_address(struct usb_host_device *usb_dev, u8 devnum);

/**@brief   USB获取设备描述符
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  usb_device_descriptor定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_get_device_descriptor(host_dev , device_desc);
  * @encode
  */
int usb_get_device_descriptor(struct usb_host_device *usb_dev, struct usb_device_descriptor *desc);

/**@brief   USB获取字符串描述符
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  usb_device_descriptor定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_get_string_descriptor(host_dev , device_desc);
  * @encode
  */
int usb_get_string_descriptor(struct usb_host_device *usb_dev, struct usb_device_descriptor *desc);

/**@brief   USB设置相关配置
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * set_configuration(host_dev);
  * @encode
  */
int set_configuration(struct usb_host_device *usb_dev);

/**@brief   USB设置相关配置,添加相关参数
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  value 当前请求的参数
  * @return     0:成功
  * @par    示例：
  * @code
  * set_configuration_add_value(host_dev , value);
  * @encode
  */
int set_configuration_add_value(struct usb_host_device *host_dev, u16 value);

/**@brief   USB获取配置描述符
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  cfg_desc 自定义的配置描述符指针
  * @param[in]  len 长度
  * @return     0:成功
  * @par    示例：
  * @code
  * get_config_descriptor(host_dev , cfg_desc , len);
  * @encode
  */
int get_config_descriptor(struct usb_host_device *usb_dev, void *cfg_desc, u32 len);

/**@brief   USB获取配置描述符,添加相关参数
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  cfg_desc 自定义的配置描述符指针
  * @param[in]  len 长度
  * @param[in]  value_l 请求的参数
  * @return     0:成功
  * @par    示例：
  * @code
  * get_config_descriptor_add_value_l(host_dev , cfg_desc , len , value_l);
  * @encode
  */
int get_config_descriptor_add_value_l(struct usb_host_device *host_dev, void *cfg_desc, u32 len, u8 value_l);

/**@brief   USB获取大容量存储设备的最大逻辑单元号
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  lun 逻辑单元号
  * @return     0:成功
  * @par    示例：
  * @code
  * get_msd_max_lun(host_dev , lun);
  * @encode
  */
int get_msd_max_lun(struct usb_host_device *usb_dev, void *lun);

/**@brief   USB设置大容量存储设备复位
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * set_msd_reset(host_dev);
  * @encode
  */
int set_msd_reset(struct usb_host_device *usb_dev);

/**@brief   hid设置为空闲模式
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  id 指定接口或端点号
  * @return     0:成功
  * @par    示例：
  * @code
  * hid_set_idle(host_dev , id);
  * @encode
  */
int hid_set_idle(struct usb_host_device *usb_dev, u32 id);

/**@brief   hid获取报告描述符
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  report 自定义报告指针，指向报告内容
  * @param[in]  report_id 报告描述符id号
  * @param[in]  report_len 报告描述符长度
  * @return     0:成功
  * @par    示例：
  * @code
  * hid_get_report(host_dev , report , report_id , report_len);
  * @encode
  */
int hid_get_report(struct usb_host_device *usb_dev, u8 *report, u8 report_id, u16 report_len);

/**@brief   hid设置输出报告描述符
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  report 自定义报告指针，指向报告内容
  * @param[in]  report_id
  * @param[in]  report_len
  * @return     0:成功
  * @par    示例：
  * @code
  * hid_set_output_report(host_dev , report , report_id , report_len);
  * @encode
  */
int hid_set_output_report(struct usb_host_device *usb_dev, u8 *report, u8 report_id, u8 report_len);

/**@brief   USB设置远程唤醒
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_set_remote_wakeup(host_dev);
  * @encode
  */
int usb_set_remote_wakeup(struct usb_host_device *usb_dev);

/**@brief   USB获取设备状态
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * get_device_status(host_dev);
  * @encode
  */
int get_device_status(struct usb_host_device *usb_dev);

/**@brief   USB获取设备限定描述符
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  BUFFER 自定义的指针，指向限定描述符内容
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_get_device_qualifier(host_dev , buffer);
  * @encode
  */
int usb_get_device_qualifier(struct usb_host_device *usb_dev, u8 *buffer);

/**@brief   USB获取安卓aoa协议版本
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  version 自定义的指针，指向版本内容
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_get_aoa_version(host_dev , version);
  * @encode
  */
int usb_get_aoa_version(struct usb_host_device *host_dev, u16 *version);

/**@brief   USB设置证书信息
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  string 字符串指针，指向证书内容
  * @param[in]  index 传递的参数
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_set_credentials(host_dev , string , index);
  * @encode
  */
int usb_set_credentials(struct usb_host_device *host_dev, const char *string, int index);

/**@brief   USB设置aoa开关
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_switch2aoa(host_dev);
  * @encode
  */
int usb_switch2aoa(struct usb_host_device *host_dev);

/**@brief   USB设置从机模式开关
  * @param[in]  usb_host_device定义的结构体指针
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_switch2slave(host_dev);
  * @encode
  */
int usb_switch2slave(struct usb_host_device *host_dev);

/**@brief   USB hid设备注册
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  value 请求的参数
  * @param[in]  index 传递的参数
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_aoa_register_hid(host_dev , value , index);
  * @encode
  */
int usb_aoa_register_hid(struct usb_host_device *host_dev, u16 value, u16 index);

/**@brief   USB设置hid报告描述符
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  value 请求的参数
  * @param[in]  offset 偏移量参数
  * @param[in]  pbuf 存放数据的BUFFER指针
  * @param[in]  len 长度
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_aoa_set_hid_report_desc(host_dev , value , offset , *pbuf , len);
  * @encode
  */
int usb_aoa_set_hid_report_desc(struct usb_host_device *host_dev, u16 value, u16 offset, const char *pbuf, u32 len);

/**@brief   USB发送hid事件
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  value 请求的参数
  * @param[in]  pbuf 存放数据的BUFFER指针
  * @param[in]  len 长度
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_aoa_send_hid_event(host_dev , value , *pbuf , len);
  * @encode
  */
int usb_aoa_send_hid_event(struct usb_host_device *host_dev, u16 value, const u8 *pbuf, u32 len);

/**@brief   获取扩展的大容量存储设备（可兼容）id 号
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  BUFFER 存放数据的BUFFER
  * @return     0:成功
  * @par    示例:
  * @code
  * get_ms_extended_compat_id(host_dev , value , *pbuf , len);
  * @encode
  */
int get_ms_extended_compat_id(struct usb_host_device *host_dev,  u8 *buffer);

/**@brief   USB设置接口
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  interface 接口参数（请求的参数）
  * @param[in]  alternateSetting 交替设置（传递的参数）
  * @return     0:成功
  * @par    示例:
  * @code
  * usb_set_interface(host_dev , interface , alternateSetting);
  * @encode
  */
int usb_set_interface(struct usb_host_device *host_dev, u8 interface, u8 alternateSetting);

/**@brief   USB音频采样频率控制
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  ep 端点号
  * @param[in]  samp_rate 采样率
  * @return     0:成功
  * @par    示例:
  * @code
  * usb_audio_sampling_frequency_control(host_dev , ep , sampe_rate);
  * @encode
  */
int usb_audio_sampling_frequency_control(struct usb_host_device *host_dev, u32 ep, u32 sampe_rate);

/**@brief   USB音频音量控制
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  feature_id 特征值id号（端点或接口的id）（传递的参数）
  * @param[in]  channel_num 通道编号（请求的参数）
  * @paeam[in]  volume 音量
  * @return     0:成功
  * @par    示例:
  * @code
  * usb_audio_volume_control(host_dev , feature_id , channel_num , volume);
  * @encode
  */
int usb_audio_volume_control(struct usb_host_device *host_dev, u8 feature_id, u8 channel_num, u16 volume);

/**@brief   USB音频静音控制
  * @param[in]  usb_host_device定义的结构体指针
  * @param[in]  feature_id 特征值id号（端点或接口的id）（传递的参数）
  * @param[in]  mute 静音信号（静音标志）
  * @return     0:成功
  * @par    示例:
  * @code
  * usb_audio_mute_control(host_dev , feature_id , channel_num , volume);
  * @encode
  */
int usb_audio_mute_control(struct usb_host_device *host_dev, u8 feature_id, u8 mute);

#endif
