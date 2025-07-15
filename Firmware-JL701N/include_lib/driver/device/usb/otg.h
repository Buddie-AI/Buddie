/**@file        otg.h
  * @brief      otg驱动头文件
  * @details    结构体声明，功能函数声明
  * @author     jieli
  * @date       2021-7-22
  * @version    V1.0
  * @copyright  Copyright(c)2010-2021   珠海市杰理科技股份有限公司
  *********************************************************************
  * @attention
  * 硬件平台：AC632N
  * SDK版本：AC632N_V1.0.0_SDK
  * @修改日志：
  * <table>
  * <tr><th>Date        <th>Version     <th>Author      <th>Description
  * <tr><td>2021-7-22   <td>1.0         <td>jieli       <td>创建初始版本
  * </table>
  *
  *********************************************************************
  */
#ifndef  __OTG_H__
#define  __OTG_H__

#include "asm/usb.h"

/**@enum usb_hotplug.state 或 usb_hotplug.last_state
  * @brief  otg当前所处模式 或 上一次所处模式
  */
enum {
    IDLE_MODE = 0, ///<空闲模式
    DISCONN_MODE = 1, ///<断连模式
    HOST_MODE = 2, ///<主机模式
    PRE_SLAVE_MODE, ///<成为从机模式前的一个中间模式
    SLAVE_MODE_WAIT_CONFIRMATION, ///<从机模式还需等待再次确认
    SLAVE_MODE, ///<从机模式
    CHARGE_MODE, ///<充电模式
    OTG_USER_MODE, ///<用户模式，暂时未具体定义
};

/**@enum    空
  * @brief  otg挂起时，所选操作模式
  */
enum {
    OTG_OP_NULL = 0, ///< ///<空，无意义
    OTG_UNINSTALL = 1, ///<OTG卸载
    OTG_KEEP_STATE, ///<OTG保持
    OTG_SUSPEND, ///< OTG挂起
    OTG_RESUME, ///< OTG恢复
};

#define     OTG_HOST_MODE      BIT(0)
#define     OTG_SLAVE_MODE     BIT(1)
#define     OTG_CHARGE_MODE    BIT(2)
#define     OTG_DET_DP_ONLY    BIT(3)
#define     OTG_DET_DM_ONLY    BIT(4)

/**@struct  otg_dev_data
  * @brief  otg_dev_data信息结构体 \n
  * 自定义的存储otg设备相关数据信息
  */
struct otg_dev_data {
    u8 usb_dev_en; ///<有哪几个otg设备使能，如USB0，USB1。
    u8 slave_online_cnt; ///<从机上线阈值
    u8 slave_offline_cnt; ///<从机下线阈值
    u8 host_online_cnt; ///<主机上线阈值
    u8 host_offline_cnt; ///<主机下线阈值
    u8 detect_mode; ///<otg可用模式配置
    u8 detect_time_interval; ///<检测时间间隔，单位 ms

    void *otg1; //需要使用双USB口独立配置时，在板级.c文件用户自定义一个otg信息的结构体，并指向它。
};


/**@brief USB设备当前模式获取
  * @param[in]  usb_id  USB接口的id号
  * @return     函数的执行结果
  * - IDLE_MODE
  * - DISCONN_MODE
  * - HOST_MODE
  * - PRE_SLAVE_MODE
  * - SLAVE_MODE_WAIT_CONFIRMATION
  * - SLAVE_MODE
  * - CHARGE_MODE
  * - OTG_USER_MODE
  * @par    示例：
  * @code
  * usb_otg_online(0);  获取USB0当前模式
  * @encode
  */
u32 usb_otg_online(const usb_dev usb_id);
// u32 usb_otg_init(u32 mode);

/**@brief 将DP/DM脚设为高阻
  * @param[in]  usb_id  USB接口的id号
  * @return     无
  * @par    示例：
  * @code
  * usb_otg_io_suspend(0);  将USB0的DP/DM脚设为高阻状态
  * @encode
  */
void usb_otg_io_suspend(usb_dev usb_id);

/**@brief 恢复DP/DM引脚的USB功能，并发起usb reset
  * @param[in]  usb_id  USB接口的id号
  * @return     无
  * @par    示例：
  * @code
  * usb_otg_io_resume(0);  将USB0的IO口功能恢复
  * @encode
  */
void usb_otg_io_resume(usb_dev usb_id);

/**@brief 将usb_otg设备挂起
  * @param[in]  usb_id  USB接口的id号
  * @param[in]  op_mode 选择挂起模式
  * @ref OTG_UNINSTALL  OTG卸载
  * @ref OTG_KEEP_STATE OTG保持原模式
  * @return     无
  * @par    示例：
  * @code
  * usb_otg_suspend(0,OTG_KEEP_STATE);  USB0保持原来的模式
  * @encode
  */
void usb_otg_suspend(usb_dev usb_id, u8 op_mode);

/**@brief 将usb_otg设备恢复
  * @param[in]  usb_id  USB接口的id号
  * @return     无
  * @par    示例：
  * @code
  * usb_otg_resume(0);  USB0恢复
  * @encode
  */
void usb_otg_resume(usb_dev usb_id);

void usb_otg_sof_check_register_hooks(void (*before)(), void (*after)());

extern const struct device_operations usb_dev_ops;


#endif  /*OTG_H*/
