/*********************************************************************************************
    *   Filename        : btcontroller_modules.h

    *   Description     : Lto 优化Macro 定义

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2018-12-19 16:38

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _BTCONTROLLER_H_
#define _BTCONTROLLER_H_

#include "hci_transport.h"
#include "btcontroller_mode.h"

#include "ble/hci_ll.h"

#include "classic/hci_lmp.h"


/* app 层修改蓝牙版本，可在BT_STATUS_INIT_OK case
  调用 set_bt_version 函数更改蓝牙版本号
*/
#define BLUETOOTH_CORE_SPEC_42  0x08
#define BLUETOOTH_CORE_SPEC_50  0x09
#define BLUETOOTH_CORE_SPEC_51  0x0a
#define BLUETOOTH_CORE_SPEC_52  0x0b
extern void set_bt_version(u8 version);


/*
 *-------------------Module SUPPORT
 *  brief : 运行时优化（LTO）下，代码空间优化；
 */
#define BT_MODULE_CLASSIC                   BIT(0)
#define BT_MODULE_LE                        BIT(1)

extern const int config_btctler_modules;
#define BT_MODULES_IS_SUPPORT(x)            (config_btctler_modules & (x))
/*-----------------------------------------------------------*/
extern const int config_stack_modules;
#define STACK_MODULES_IS_SUPPORT(x)         (config_stack_modules & (x))

/*
 *-------------------Mode SELECT
 *  brief : 运行时优化（LTO）下，代码空间优化；
 */
extern const int config_btctler_mode;
#define BT_MODE_IS(x)            (config_btctler_mode & (x))

/*-----------------------------------------------------------*/

extern const int config_btctler_hci_standard;
#define BT_HCI_STANDARD_IS_SUPPORT(x)        (config_btctler_hci_standard)


extern const int config_bt_function ;
#define BT_ENCTRY_TASK              BIT(0)
#define BT_MASTER_AFH               BIT(1)
#define BT_MASTER_QOS               BIT(2)


#define BT_FUNCTION_IS(x)           (config_bt_function & (x))


extern const int CONFIG_TEST_DUT_CODE;
extern const int CONFIG_TEST_FCC_CODE;
extern const int CONFIG_TEST_DUT_ONLY_BOX_CODE;
extern const int CONFIG_BREDR_INQUIRY;

extern const int CONFIG_INQUIRY_PAGE_OFFSET_ADJUST ;

extern const int CONFIG_LMP_NAME_REQ_ENABLE ;
extern const int CONFIG_LMP_PASSKEY_ENABLE ;
extern const int CONFIG_LMP_MASTER_ESCO_ENABLE ;
extern const int config_btctler_bredr_master ;
extern const int config_bredr_afh_user ;
extern const int config_bredr_master_afh ;
extern const int CONFIG_ESCO_MUX_RX_BULK_ENABLE ;
extern const int config_bt_temperature_pll_trim ;
extern const int CONFIG_WIFI_DETECT_ENABLE;
extern const int ESCO_FORWARD_ENABLE;
/********************************************************************************/
/*
 *                   API
 *
 */

/* --------------------------------------------------------------------------*/
/**
 * @brief rf_set_24g_hackable_coded
 *
 * @param coded                 2.4G 配对码
 */
/* ----------------------------------------------------------------------------*/
void rf_set_24g_hackable_coded(int coded);


/* --------------------------------------------------------------------------*/
/**
 * @brief bt_pll_para
 *
 * @param osc
 * @param sys
 * @param low_power
 * @param xosc
 */
/* ----------------------------------------------------------------------------*/
void bt_pll_para(u32 osc, u32 sys, u8 low_power, u8 xosc);


/* --------------------------------------------------------------------------*/
/**
 * @brief bt_production_test
 *
 * @param en
 */
/* ----------------------------------------------------------------------------*/
void bt_production_test(u8 en);

/* --------------------------------------------------------------------------*/
/**
 * @brief bt_set_rxtx_status_enable
 *
 * @param en
 *
 *

                TX     RX
       AI800x   PA13   PA12
       AC692x   PA13   PA12
       AC693x   PA8    PA9
       AC695x   PA9    PA10
       AC696x   PC1    PC2
       AC694x   PB1    PB2
       AC697x   PC2    PC3
       AC631x   PA7    PA8

 */
/* ----------------------------------------------------------------------------*/
void bt_set_rxtx_status_enable(u8 en);

/* --------------------------------------------------------------------------*/
/**
 * @brief bt_osc_offset_ext_save
 *
 * @param offset
 *
 * 更新并且保存频偏
 */
/* ----------------------------------------------------------------------------*/
void bt_osc_offset_ext_save(s32 offset);

/* --------------------------------------------------------------------------*/
/**
 * @brief bt_osc_offset_ext_updata
 *
 * @param offset
 *
 * 更新频偏
 */
/* ----------------------------------------------------------------------------*/
void bt_osc_offset_ext_updata(s32 offset);

/* --------------------------------------------------------------------------*/
/**
 * @brief 初始化配置蓝牙发射功率最大值范围
 *
 * @param pwr    edr 连接后发射功率(range:0~9)
 * @param pg_pwr edr page 可连接状态发射功率
 * @param iq_pwr edr inquiry 可发现状态发射功率
 * @param ble_pwr ble 发射功率
 */
/* ----------------------------------------------------------------------------*/
/*
蓝牙TX发射功率档位, 参考功率值(dbm) ,超过等级范围默认设置为最高档
BD29: rang(0~8)  {-18.3,  -14.6,  -12.1, -8.5,  -6.0,  -4.1,  -1.1,  +1.1,  +4.0,  +6.1}
BD19: rang(0~10) {-17.6,  -14.0,  -11.5, -9.6,  -6.6,  -4.4,  -0.79, +1.12, +3.8,  +5.65, +8.04}
BR23: rang(0~9)  {-15.7,  -12.5,  -10.0, -6.6,  -4.4,  -2.5,  -0.1,  +2.1,  +4.6,  +6.4}
BR25: rang(0~9)  {-15.7,  -12.5,  -10.0, -6.6,  -4.4,  -2.5,  -0.1,  +2.1,  +4.6,  +6.4}
BR30: rang(0~8)  {-17.48, -11.46, -7.96, -3.59, -0.79, +1.12, +3.8,  +6.5,  +8.44}
BR34: rang(0~10) {-17.6,  -14.0,  -11.5, -9.6,  -6.6,  -4.4,  -1.8,  0,     +2.1,  +4,    +6.3}
*/

void bt_max_pwr_set(u8 pwr, u8 pg_pwr, u8 iq_pwr, u8 ble_pwr);

/* --------------------------------------------------------------------------*/
/**
 * @brief bt_set_ldos
 *
 * @param mode
 */
/* ----------------------------------------------------------------------------*/
void bt_set_ldos(u8 mode);

/* --------------------------------------------------------------------------*/
/**
 * @brief ble_set_fix_pwr
 *
 * @param fix (0~max)
 * 动态调整BLE的发射功率
 */
/* ----------------------------------------------------------------------------*/
void ble_set_fix_pwr(u8 fix);


/* --------------------------------------------------------------------------*/
/**
 * @brief bredr_set_fix_pwr
 *
 * @param fix (0~max)
 * 动态调整EDR的发射功率
 */
/* ----------------------------------------------------------------------------*/
void bredr_set_fix_pwr(u8 fix);

/* --------------------------------------------------------------------------*/
/**
 * @brief ble_rf_vendor_fixed_channel
 *
 * @param channel_index: 指定信道定频:          range 0~39 fixed freq, or 0xff --close fixed,default 37、38、39
 * @param pktcnt:        adv方式,1次发包的个数: range 1~3
 * 配置ble 的 adv、scan、init 状态定频
 */
/* ----------------------------------------------------------------------------*/
bool ble_rf_vendor_fixed_channel(u8 channel_index, u8 pktcnt);

/* --------------------------------------------------------------------------*/
/**
 * @brief bredr_get_rssi_for_address
 * 获取已连接设备的rssi
 *
 * @param address 对方mac地址
 * @return rssi 值，range(-127 ~ +127)
 */
/* ----------------------------------------------------------------------------*/
s8 bredr_get_rssi_for_address(u8 *address);

/* --------------------------------------------------------------------------*/
/**
 * @brief  配置tx 是否支持包类型, (sdk默认支持)
 *
 * @param  packet_type
 * @param  support_en   0 or 1
 * @return true or false
 */
/* ----------------------------------------------------------------------------*/
typedef enum {
    PKT_TYPE_2DH5_EU = 0,
} pkt_type_eu;

bool bredr_link_vendor_support_packet_enable(pkt_type_eu packet_type, u8 support_en);

/* --------------------------------------------------------------------------*/
/**
 * @brief  配置ble 优先级锁定不低压ACL, (sdk 默认自动调节)
 *
 * @param  role:0--master,1--slave
 * @param  enalbe   0 or 1
 * @return null
 */
/* ----------------------------------------------------------------------------*/
void ble_vendor_set_hold_prio(u8 role, u8 enable);

void set_bt_afh_classs_enc(u8 afh_class);
void set_bt_enhanced_power_control(u8 en);
void set_bt_data_rate_acl_3mbs_mode(u8 en);

void set_bt_full_name_event(u8 en);

/* coexist between bt chips */
void bt_wl_coex_init(uint8_t state);
void bt_wl_coex_enable(bool enable);

#endif
