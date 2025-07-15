#ifndef  __USB_STACK_H__
#define  __USB_STACK_H__
#include "typedef.h"
#include "asm/usb.h"
#include "usb/ch9.h"
#include "usb/usb_phy.h"
#include "usb/otg.h"


#define     MAX_INTERFACE_NUM       6
#define     USB_SUSPEND_RESUME      0
#define     USB_SUSPEND_RESUME_SYSTEM_NO_SLEEP  0
#define     USB_SETUP_SIZE         (512)

#if 0
#define     USB_ATTACHED        BIT(0)
#define     USB_POWERED         BIT(1)
#define     USB_DEFAULT         BIT(2)
#define     USB_ADDRESS         BIT(3)
#define     USB_CONFIGURED      BIT(4)
#define     USB_SUSPENDED       BIT(5)
#else
enum {
    USB_ATTACHED,
    USB_POWERED,
    USB_DEFAULT,
    USB_ADDRESS,
    USB_CONFIGURED,
    USB_SUSPENDED
};
#endif
struct usb_device_t {
    u8 baddr;
    u8 bsetup_phase;          //ep0 setup状态机
    u16 wDataLength;    //ep0 setup data stage数据长度

    u8 *setup_buffer;   //本次传输的bufer地址
    u8 *setup_ptr;      //当前传输的位置
    u32(*setup_hook)(struct usb_device_t *, struct usb_ctrlrequest *);
    u32(*setup_recv)(struct usb_device_t *, struct usb_ctrlrequest *);

    u8 bDeviceStates;
    u8 bDataOverFlag;    //ep0 0包标识
    u8 wDeviceClass;    // 设备类
    u8 bRemoteWakup: 1;
    u8 baddr_config: 1;
#if USB_MAX_HW_NUM == 2
    u8 usb_id: 1;
    u8 res: 5;
#else
    u8 res: 6;
#endif
};

typedef u32(*itf_hander)(struct usb_device_t *usb_device, struct usb_ctrlrequest *);
typedef void(*itf_reset_hander)(struct usb_device_t *, u32 itf);
typedef void(*usb_interrupt)(struct usb_device_t *, u32 ep);
typedef u32(*desc_config)(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num);

struct usb_setup_t {
    struct usb_device_t usb_device;
    struct usb_ctrlrequest request;
    itf_hander interface_hander[MAX_INTERFACE_NUM];
    itf_reset_hander reset_hander[MAX_INTERFACE_NUM];
} __attribute__((aligned(4)));

/**@brief 获取USB接口的id号
  * @param[in]  *usb_device  usb_device_t定义的结构体指针
  * @return     USB的id号
  * @par    示例：
  * @code
  * usb_device2id(usb_device);
  * @encode
  */
const usb_dev usb_device2id(const struct usb_device_t *usb_device);

/**@brief 获取usb_device_t定义的结构体地址
  * @param[in]  usb_id  USB接口的id号
  * @return     该结构的地址
  * @par    示例：
  * @code
  * usb_id2device(usb_device);
  * @encode
  */
struct usb_device_t *usb_id2device(const usb_dev usb_id);

/**@brief USB setup阶段控制传输
  * @param[in]  *usb_device  usb_device_t定义的结构体指针
  * @return     无
  * @par    示例：
  * @code
  * usb_control_transfer(usb_device);
  * @encode
  */
void usb_control_transfer(struct usb_device_t *usb_device);

/**@brief USB设置设备类
  * @param[in]  *usb_device  usb_device_t定义的结构体指针
  * @param[in]  class_config  设备类设置
  * @return     无
  * @par    示例：
  * @code
  * usb_device_set_class(usb_device,class_config);
  * @encode
  */
void usb_device_set_class(struct usb_device_t *usb_device, u32 class_config);
u32 usb_g_set_intr_hander(const usb_dev usb_id, u32 ep, usb_interrupt hander);

/**@brief USB设置接口服务函数
  * @param[in]  usb_id  USB接口的id号
  * @param[in]  itf_num 接口号
  * @param[in]  hander  自己定义的函数
  * @return  itf_num:成功   0:失败
  * @par    示例：
  * @code
  * usb_set_interface_hander(usb_id,itf_num,hander);
  * @encode
  */
u32 usb_set_interface_hander(const usb_dev usb_id, u32 itf_num, itf_hander hander);
void usb_add_desc_config(const usb_dev usb_id, u32 index, const desc_config desc);
const u8 *usb_get_config_desc();

/**@brief USB设置复位服务函数
  * @param[in]  usb_id  USB接口的id号
  * @param[in]  itf_num 接口号
  * @param[in]  hander  自己定义的函数
  * @return  itf_num:成功   0:失败
  * @par    示例：
  * @code
  * usb_set_reset_hander(usb_id,itf_num,hander);
  * @encode
  */
u32 usb_set_reset_hander(const usb_dev usb_id, u32 itf_num, itf_reset_hander hander);

/**@brief USB接口复位
  * @param[in]  *usb_device  usb_device_t定义的结构体指针
  * @return     无
  * @par    示例：
  * @code
  * usb_reset_interface(usb_device);
  * @encode
  */
void usb_reset_interface(struct usb_device_t *usb_device);
void usb_set_setup_recv(struct usb_device_t *usb_device, void *recv);
void usb_set_setup_hook(struct usb_device_t *usb_device, void *hook);
int usb_device_mode(const usb_dev usb_id, const u32 class);

/**@brief otg检测中sof初始化
  * @param[in]  usb_id  USB接口的id号
  * @return     1:等待sof信号
  * @par    示例：
  * @code
  * usb_otg_sof_check_init(usb_id);
  * @encode
  */
u32 usb_otg_sof_check_init(const usb_dev id);

/**@brief USB setup阶段初始化
  * @param[in]  usb_id  USB接口的id号
  * @param[in]  *ptr  usb_setup_t定义的结构体地址
  * @param[in]  *setup_buffer  setup_buffer的地址
  * @return     无
  * @par    示例：
  * @code
  * usb_setup_init(usb_id,ptr,setup_buffer);
  * @encode
  */
void usb_setup_init(const usb_dev usb_id, void *ptr, u8 *setup_buffer);

/**@brief USB setup阶段释放
  * @param[in]  usb_id  USB接口的id号
  * @return     0:成功
  * @par    示例：
  * @code
  * usb_setup_release(usb_id);
  * @encode
  */
u32 usb_setup_release(const usb_dev usb_id);

/**@brief USB设置数据载荷
  * @param[in]  *usb_device  usb_device_t定义的结构体指针
  * @param[in]  *req  usb_ctrlrequest定义的结构体指针
  * @param[in]  *data  存放数据指针
  * @param[in]  len  数据长度
  * @return     setup_buffer的地址
  * @par    示例：
  * @code
  * usb_set_data_payload(usb_device,req,tx_payload,len);
  * @encode
  */
u8 *usb_set_data_payload(struct usb_device_t *usb_device, struct usb_ctrlrequest *req, const void *data, u32 len);

/**@brief USB设置控制传输阶段
  * @param[in]  *usb_device  usb_device_t定义的结构体指针
  * @param[in]  setup_phase  需要设置成的阶段
  * @return     无
  * @par    示例：
  * @code
  * usb_set_setup_phase(usb_id);
  * @encode
  */
void usb_set_setup_phase(struct usb_device_t *usb_device, u8 setup_phase);
void dump_setup_request(const struct usb_ctrlrequest *request);
void user_setup_filter_install(struct usb_device_t *usb_device);
void usb_ep_enable(const usb_dev usb_id, u32 ep, u32 is_enable);

/**@brief USB获取本次传输的buffer地址
  * @param[in]  *usb_device  usb_device_t定义的结构体指针
  * @return     setup_buffer地址
  * @par    示例：
  * @code
  * usb_get_setup_buffer(usb_device);
  * @encode
  */
void *usb_get_setup_buffer(const struct usb_device_t *usb_device);
u32 usb_root2_testing();

extern void usb_start();
extern void usb_stop();
extern void usb_pause();
extern void usb_cdc_background_run();

/* #define usb_add_desc_config(fn) \                                    */
/*     const desc_config usb_desc_config##fn sec(.usb.desc_config) = fn */

#endif  /*USB_STACK_H*/
