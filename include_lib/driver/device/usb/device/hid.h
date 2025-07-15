/**@file        hid.h
  * @brief      hid驱动头文件（做从机）
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
#ifndef __USB_HID_H__
#define __USB_HID_H__

#include "typedef.h"
#include "asm/usb.h"

//do not add brace to the macro outside
#define SHORT_ITEMS(prefix, _len, ...) \
    ((prefix) | (((_len) > 0) ? 1 << ((_len) - 1) : 0)), ##__VA_ARGS__

/*Main Items*/
#define INPUT(len, ...)              SHORT_ITEMS(0x80, len, ##__VA_ARGS__)
#define OUTPUT(len, ...)             SHORT_ITEMS(0x90, len, ##__VA_ARGS__)
#define COLLECTION(len, ...)         SHORT_ITEMS(0xA0, len, ##__VA_ARGS__)
#define FEATURE(len, ...)            SHORT_ITEMS(0xB0, len, ##__VA_ARGS__)
#define END_COLLECTION               0xC0

/*Golbal Items*/
#define USAGE_PAGE(len, ...)         SHORT_ITEMS(0x04, len, ##__VA_ARGS__)
#define LOGICAL_MIN(len, ...)        SHORT_ITEMS(0x14, len, ##__VA_ARGS__)
#define LOGICAL_MAX(len, ...)        SHORT_ITEMS(0x24, len, ##__VA_ARGS__)
#define PHYSICAL_MIN(len, ...)       SHORT_ITEMS(0x34, len, ##__VA_ARGS__)
#define PHYSICAL_MAX(len, ...)       SHORT_ITEMS(0x44, len, ##__VA_ARGS__)
#define UNIT_EXPONENT(len, ...)      SHORT_ITEMS(0x54, len, ##__VA_ARGS__)
#define UNIT(len, ...)               SHORT_ITEMS(0x64, len, ##__VA_ARGS__)
#define REPORT_SIZE(len, ...)        SHORT_ITEMS(0x74, len, ##__VA_ARGS__)
#define REPORT_ID(len, ...)          SHORT_ITEMS(0x84, len, ##__VA_ARGS__)
#define REPORT_COUNT(len, ...)       SHORT_ITEMS(0x94, len, ##__VA_ARGS__)
#define PUSH                         SHORT_ITEMS(0xA4, 0)
#define POP                          SHORT_ITEMS(0xB4, 0)

/*Local Items*/
#define USAGE(len, ...)              SHORT_ITEMS(0x08, len, ##__VA_ARGS__)
#define USAGE_MIN(len, ...)          SHORT_ITEMS(0x18, len, ##__VA_ARGS__)
#define USAGE_MAX(len, ...)          SHORT_ITEMS(0x28, len, ##__VA_ARGS__)
#define DESIGNATOR_INDEX(len, ...)   SHORT_ITEMS(0x38, len, ##__VA_ARGS__)
#define DESIGNATOR_MIN(len, ...)     SHORT_ITEMS(0x48, len, ##__VA_ARGS__)
#define DESIGNATOR_MAX(len, ...)     SHORT_ITEMS(0x58, len, ##__VA_ARGS__)
#define STRING_INDEX(len, ...)       SHORT_ITEMS(0x78, len, ##__VA_ARGS__)
#define STRING_MIN(len, ...)         SHORT_ITEMS(0x88, len, ##__VA_ARGS__)
#define STRING_MAX(len, ...)         SHORT_ITEMS(0x98, len, ##__VA_ARGS__)
#define DELIMITER(len, ...)          SHORT_ITEMS(0xA8, len, ##__VA_ARGS__)


/*Consumer Page*/
#define CONSUMER_PAGE           0x0C
#define CONSUMER_CONTROL        0x01
#define GENERIC_DESKTOP_CTRLS   0x01

/*Usage*/
#define POINTER                 0x01
#define MOUSE                   0x02
#define BUTTON                  0x09
#define X_AXIS                  0x30
#define Y_AXIS                  0x31

//Collection
#define PHYSICAL                0x00
#define APPLICATION             0x01
#define LOGICAL                 0x02
#define REPORT                  0x03

#define USB_HID_DT_HID   (USB_TYPE_CLASS | 0x01)
#define USB_HID_DT_REPORT    (USB_TYPE_CLASS | 0x02)
#define USB_HID_DT_PHYSICAL  (USB_TYPE_CLASS | 0x03)
/*
 *           * HID requests
 *            */
#define USB_REQ_GET_REPORT   0x01
#define USB_REQ_GET_IDLE     0x02
#define USB_REQ_GET_PROTOCOL     0x03
#define USB_REQ_SET_REPORT   0x09
#define USB_REQ_SET_IDLE     0x0A
#define USB_REQ_SET_PROTOCOL     0x0B




#define PLAY                    0xB0
#define PAUSE                   0xB1
#define RECORD                  0xB2
#define FAST_FORWARD            0xB3
#define REWIND                  0xB4
#define SCAN_NEXT_TRACK         0xB5
#define SCAN_PREV_TRACK         0xB6
#define STOP                    0xB7
#define FRAME_FORWARD           0xC0
#define FRAME_BACK              0xC1
#define TRACKING_INC            0xCA
#define TRACKING_DEC            0xCB
#define STOP_EJECT              0xCC
#define PLAY_PAUSE              0xCD
#define PLAY_SKIP               0xCE
#define VOLUME                  0xE0
#define BALANCE                 0xE1
#define MUTE                    0xE2
#define BASS                    0xE3
#define VOLUME_INC              0xE9
#define VOLUME_DEC              0xEA
#define BALANCE_LEFT            0x50, 0x01
#define BALANCE_RIGHT           0x51, 0x01
#define CHANNEL_LEFT            0x61, 0x01
#define CHANNEL_RIGHT           0x62, 0x01


//----------------------------------
// HID key for audio
//----------------------------------
#define USB_AUDIO_NONE          0
#define USB_AUDIO_VOLUP         BIT(0)
#define USB_AUDIO_VOLDOWN       BIT(1)
#define USB_AUDIO_MUTE          BIT(2)
#define USB_AUDIO_PP            BIT(3)
#define USB_AUDIO_NEXTFILE      BIT(4)
#define USB_AUDIO_PREFILE       BIT(5)
#define USB_AUDIO_FASTFORWARD   BIT(5)
#define USB_AUDIO_STOP          BIT(7)

#define USB_AUDIO_TRACKING_INC  BIT(8)
#define USB_AUDIO_TRACKING_DEC  BIT(9)
#define USB_AUDIO_STOP_EJECT    BIT(10)
#define USB_AUDIO_VOLUME        BIT(11)
#define USB_AUDIO_BALANCE_RIGHT BIT(12)
#define USB_AUDIO_BALANCE_LEFT  BIT(13)
#define USB_AUDIO_PLAY          BIT(14)
#define USB_AUDIO_PAUSE         BIT(15)

/**@brief   USB hid描述符配置
  * @param[in]  usb_id usb的id号
  * @param[in]  *ptr 指向HID描述符
  * @param[in]  *cur_itf_num 当前接口号
  * @return HID描述符长度，单位Byte
  * @par    示例：
  * @code
  * hid_desc_config(usb_id,ptr,cur_itf_num);
  * @encode
  */
u32 hid_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num);
u32 hid_second_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num);

/**@brief   hid按键处理函数
  * @param[in]  *usb_device usb_device_t定义的结构体指针
  * @param[in]  hid_key hid按键信息
  * @return 无
  * @par    示例：
  * @code
  * hid_key_handler(usb_device,hidkey);
  * @encode
  */
void hid_key_handler(struct usb_device_t *usb_device, u32 hid_key);

/**@brief   hid按键处理函数,用于特殊发送一个包的场景(正常hidkey有两个包)
  * @param[in]  *usb_device usb_device_t定义的结构体指针
  * @param[in]  hid_key hid按键信息
  * @return 无
  * @par    示例：
  * @code
  * hid_key_handler_send_one_packet(usb_device,hidkey);
  * @encode
  */
void hid_key_handler_send_one_packet(struct usb_device_t *usb_device, u32 hid_key);

/**@brief   hid发送数据
  * @param[in]  *p 数据指针，指向存放数据的地址
  * @param[in]  len 发送的数据长度
  * @return
  * @par    示例：
  * @code
  * hid_send_data(p,len);
  * @encode
  */
u32 hid_send_data(const void *p, u32 len);

/**@brief   hid注册
  * @param[in]  usb_id usb的id号
  * @return 0
  * @par    示例：
  * @code
  * hid_register(usb_id);
  * @encode
  */
u32 hid_register(const usb_dev usb_id);

/**@brief   hid释放（暂未使用）
  * @param[in]  usb_id usb的id号
  * @return 0
  * @par    示例：
  * @code
  * hid_release(usb_id);
  * @encode
  */
void hid_release(const usb_dev usb_id);
#endif
