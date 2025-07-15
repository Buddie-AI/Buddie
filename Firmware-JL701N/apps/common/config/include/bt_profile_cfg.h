
#ifndef _BT_PROFILE_CFG_H_
#define _BT_PROFILE_CFG_H_

#include "app_config.h"
#include "btcontroller_modules.h"


#if (TRANS_DATA_EN || RCSP_BTMATE_EN || RCSP_ADV_EN || SMART_BOX_EN || ANCS_CLIENT_EN || LL_SYNC_EN || TUYA_DEMO_EN)
#ifndef BT_FOR_APP_EN
#define    BT_FOR_APP_EN             1
#endif
#else
#ifndef BT_FOR_APP_EN
#define    BT_FOR_APP_EN             0
#endif
#ifndef AI_APP_PROTOCOL
#define    AI_APP_PROTOCOL             0
#endif
#endif


///---sdp service record profile- 用户选择支持协议--///
#if (BT_FOR_APP_EN || APP_ONLINE_DEBUG || AI_APP_PROTOCOL)
#if (LL_SYNC_EN || TUYA_DEMO_EN)
#undef USER_SUPPORT_PROFILE_SPP
#define USER_SUPPORT_PROFILE_SPP    0
#else
#undef USER_SUPPORT_PROFILE_SPP
#define USER_SUPPORT_PROFILE_SPP    1
#endif
#endif

//ble demo的例子
#define DEF_BLE_DEMO_NULL                 0 //ble 没有使能
#define DEF_BLE_DEMO_ADV                  1 //only adv,can't connect
#define DEF_BLE_DEMO_TRANS_DATA           2 //
#define DEF_BLE_DEMO_RCSP_DEMO            4 //
#define DEF_BLE_DEMO_ADV_RCSP             5
#define DEF_BLE_DEMO_CLIENT               7 //
#define DEF_BLE_ANCS_ADV				  9
#define DEF_BLE_DEMO_MULTI                11 //
#define DEF_BLE_DEMO_LL_SYNC              13 //
#define DEF_BLE_DEMO_WIRELESS_MIC_SERVER  14 //
#define DEF_BLE_DEMO_WIRELESS_MIC_CLIENT  15 //
#define DEF_BLE_DEMO_TUYA                 16 //
#define DEF_BLE_WL_MIC_1T1_TX             17
#define DEF_BLE_WL_MIC_1T1_RX             18
#define DEF_BLE_WL_MIC_1TN_TX             19
#define DEF_BLE_WL_MIC_1TN_RX             20
#define DEF_LE_AUDIO_CENTRAL              21
#define DEF_LE_AUDIO_PERIPHERAL           22
#define DEF_LE_AUDIO_BROADCASTER          23
#define DEF_BLE_DEMO_HOGP                 24

#define    LE_AUDIO_EN                    0  //DEF_LE_AUDIO_CENTRAL

//配置选择的demo
#if TCFG_USER_BLE_ENABLE

#if (SMART_BOX_EN | RCSP_BTMATE_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_RCSP_DEMO

#elif TRANS_DATA_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_TRANS_DATA

#elif LL_SYNC_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_LL_SYNC

#elif TUYA_DEMO_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_TUYA

#elif RCSP_ADV_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_ADV_RCSP

#elif BLE_CLIENT_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_CLIENT

#elif TRANS_MULTI_BLE_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_MULTI

#elif ANCS_CLIENT_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_ANCS_ADV

#elif AI_APP_PROTOCOL
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_NULL

#elif (BLE_WIRELESS_CLIENT_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_WIRELESS_MIC_CLIENT

#elif (BLE_WIRELESS_SERVER_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_WIRELESS_MIC_SERVER

#elif (BLE_WIRELESS_1T1_TX_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_WL_MIC_1T1_TX

#elif (BLE_WIRELESS_1T1_RX_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_WL_MIC_1T1_RX

#elif (BLE_WIRELESS_1TN_TX_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_WL_MIC_1TN_TX

#elif (BLE_WIRELESS_1TN_RX_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_WL_MIC_1TN_RX

#elif (LE_AUDIO_EN)
#define TCFG_BLE_DEMO_SELECT          LE_AUDIO_EN

#elif (BLE_HID_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_HOGP

#else
#define TCFG_BLE_DEMO_SELECT          DEF_BLE5_DEMO
#endif

#else
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_NULL//ble is closed
#endif

//delete 2021-09-24;删除公共配置，放到各个profile自己配置
// #define TCFG_BLE_SECURITY_EN          0 /*是否发请求加密命令*/



#endif
