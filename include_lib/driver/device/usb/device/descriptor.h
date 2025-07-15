/**@file        descriptor.h
  * @brief      各种描述符头文件
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
#ifndef  __DESCRIPTOR_H__
#define  __DESCRIPTOR_H__

#include "asm/usb.h"

/**@brief USB获取设备描述符
  * @param[in]  *ptr  存放设备描述符的地址
  * @return     无
  * @par    示例：
  * @code
  * get_device_descriptor(ptr);
  * @encode
  */
void get_device_descriptor(u8 *ptr);

/**@brief USB获取语言字符串描述符
  * @param[in]  *ptr  存放语言字符串描述符的地址
  * @return     无
  * @par    示例：
  * @code
  * get_language_str(ptr);
  * @encode
  */
void get_language_str(u8 *ptr);

/**@brief USB获取生产商字符串描述符
  * @param[in]  *ptr  存放生产商字符串描述符的地址
  * @return     无
  * @par    示例：
  * @code
  * get_manufacture_str(ptr);
  * @encode
  */
void get_manufacture_str(u8 *ptr);

/**@brief USB获取产品字符串描述符
  * @param[in]  *ptr  存放产品字符串描述符的地址
  * @return     无
  * @par    示例：
  * @code
  * get_product_str(ptr);
  * @encode
  */
void get_product_str(u8 *ptr);

/**@brief USB获取序列号字符串描述符
  * @param[in]  *ptr  存放序列号字符串描述符的地址
  * @return     无
  * @par    示例：
  * @code
  * get_iserialnumber_str(ptr);
  * @encode
  */
void get_iserialnumber_str(u8 *ptr);

/**@brief USB获取***字符串描述符
  * @param[in]  *ptr  存放***字符串描述符的地址
  * @return     无
  * @par    示例：
  * @code
  * get_string_ee(ptr);
  * @encode
  */
void get_string_ee(u8 *ptr);

/**@brief USB设置描述符
  * @param[in]  usb_id  USB接口的id号
  * @param[in]  class_config  类配置
  * @param[in]  *p  存放描述符的地址
  * @param[in]  max_len  最大长度
  * @return
  * @par    示例：
  * @code
  * set_descriptor(usb_id,class_config,p,max_len);
  * @encode
  */
u32 set_descriptor(const usb_dev usb_id, u32 class_config, u8 *p, u32 max_len);


#endif  /*DESCRIPTOR_H*/
