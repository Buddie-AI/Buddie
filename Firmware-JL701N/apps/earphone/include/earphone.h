#ifndef EARPHONE_H
#define EARPHONE_H

#include "system/includes.h"
///搜索完整结束的事件
#define HCI_EVENT_INQUIRY_COMPLETE                            0x01
///连接完成的事件
#define HCI_EVENT_CONNECTION_COMPLETE                         0x03
///断开的事件,会有一些错误码区分不同情况的断开
#define HCI_EVENT_DISCONNECTION_COMPLETE                      0x05
///连接中请求pin code的事件,给这个事件上来目前是做了一个确认的操作
#define HCI_EVENT_PIN_CODE_REQUEST                            0x16
///连接设备发起了简易配对加密的连接
#define HCI_EVENT_IO_CAPABILITY_REQUEST                       0x31
///这个事件上来目前是做了一个连接确认的操作
#define HCI_EVENT_USER_CONFIRMATION_REQUEST                   0x33
///这个事件是需要输入6个数字的连接消息，一般在键盘应用才会有
#define HCI_EVENT_USER_PASSKEY_REQUEST                        0x34
///<可用于显示输入passkey位置 value 0:start  1:enrer  2:earse   3:clear  4:complete
#define HCI_EVENT_USER_PRESSKEY_NOTIFICATION			      0x3B
///杰理自定义的事件，上电回连的时候读不到VM的地址。
#define HCI_EVENT_VENDOR_NO_RECONN_ADDR                       0xF8
///杰理自定义的事件，有测试盒连接的时候会有这个事件
#define HCI_EVENT_VENDOR_REMOTE_TEST                          0xFE
///杰理自定义的事件，可以认为是断开之后协议栈资源释放完成的事件
#define BTSTACK_EVENT_HCI_CONNECTIONS_DELETE                  0x6D


#define ERROR_CODE_SUCCESS                                    0x00
///<回连超时退出消息
#define ERROR_CODE_PAGE_TIMEOUT                               0x04
///<连接过程中linkkey错误
#define ERROR_CODE_AUTHENTICATION_FAILURE                     0x05
///<连接过程中linkkey丢失，手机删除了linkkey，回连就会出现一次
#define ERROR_CODE_PIN_OR_KEY_MISSING                         0x06
///<连接超时，一般拿远就会断开有这个消息
#define ERROR_CODE_CONNECTION_TIMEOUT                         0x08
///<连接个数超出了资源允许
#define ERROR_CODE_SYNCHRONOUS_CONNECTION_LIMIT_TO_A_DEVICE_EXCEEDED  0x0A
///<回连的时候发现设备还没释放我们这个地址，一般直接断电开机回连会出现
#define ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS                      0x0B
///<需要回连的设备资源不够。有些手机连了一个设备之后就会用这个拒绝。或者其它的资源原因
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES       0x0D
///<有些可能只允许指定地址连接的，可能会用这个去拒绝连接。或者我们设备的地址全0或者有问题
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR    0x0F
///<连接的时间太长了，手机要断开了。这种容易复现可以联系我们分析
#define ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED         0x10
///<经常用的连接断开消息。就认为断开就行
#define ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION          0x13
///<正常的断开消息，本地L2CAP资源释放之后就会发这个值上来了
#define ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST        0x16
///<回连的时候发现设备两次role switch失败了，发断开在重新连接
#define ERROR_CODE_ROLE_SWITCH_FAILED                         0x35
///杰理自定义，在回连的过程中使用page cancel命令主动取消成功会有这个命令
#define CUSTOM_BB_AUTO_CANCEL_PAGE                            0xFD  //// app cancle page
///杰理自定义，库里面有些功能需要停止page的时候会有。比如page的时候来了电话
#define BB_CANCEL_PAGE                                        0xFE  //// bb cancle page


#if (TCFG_USER_BLE_ENABLE && (CONFIG_BT_MODE == BT_NORMAL))

#if RCSP_ADV_EN

int rcsp_earphone_state_init();
int rcsp_earphone_state_set_page_scan_enable();
int rcsp_earphone_state_get_connect_mac_addr();
int rcsp_earphone_state_cancel_page_scan();
int rcsp_earphone_state_enter_soft_poweroff();
int rcsp_earphone_state_tws_init(int paired);
int rcsp_earphone_state_tws_connected(int first_pair, u8 *comm_addr);
int rcsp_sys_event_handler_specific(struct sys_event *event);

#define EARPHONE_STATE_INIT()                   rcsp_earphone_state_init()
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   rcsp_earphone_state_set_page_scan_enable()
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   rcsp_earphone_state_get_connect_mac_addr()
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       rcsp_earphone_state_cancel_page_scan()
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    rcsp_earphone_state_enter_soft_poweroff()
#define EARPHONE_STATE_TWS_INIT(a)              rcsp_earphone_state_tws_init(a)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      rcsp_earphone_state_tws_connected(a,b)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           rcsp_sys_event_handler_specific(a)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)

#elif TRANS_DATA_EN

int trans_data_earphone_state_init();
int trans_data_earphone_state_set_page_scan_enable();
int trans_data_earphone_state_get_connect_mac_addr();
int trans_data_earphone_state_cancel_page_scan();
int trans_data_earphone_state_enter_soft_poweroff();
int trans_data_earphone_state_tws_init(int paired);
int trans_data_earphone_state_tws_connected(int first_pair, u8 *comm_addr);
int trans_data_sys_event_handler_specific(struct sys_event *event);

#define EARPHONE_STATE_INIT()                   trans_data_earphone_state_init()
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   trans_data_earphone_state_set_page_scan_enable()
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   trans_data_earphone_state_get_connect_mac_addr()
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       trans_data_earphone_state_cancel_page_scan()
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    trans_data_earphone_state_enter_soft_poweroff()
#define EARPHONE_STATE_TWS_INIT(a)              trans_data_earphone_state_tws_init(a)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      trans_data_earphone_state_tws_connected(a,b)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           trans_data_sys_event_handler_specific(a)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)

#elif BLE_HID_EN

int ble_hid_earphone_state_init();
int ble_hid_earphone_state_set_page_scan_enable();
int ble_hid_earphone_state_get_connect_mac_addr();
int ble_hid_earphone_state_cancel_page_scan();
int ble_hid_earphone_state_enter_soft_poweroff();
int ble_hid_earphone_state_tws_init(int paired);
int ble_hid_earphone_state_tws_connected(int first_pair, u8 *comm_addr);
int ble_hid_sys_event_handler_specific(struct sys_event *event);

#define EARPHONE_STATE_INIT()                   ble_hid_earphone_state_init()
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   ble_hid_earphone_state_set_page_scan_enable()
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   ble_hid_earphone_state_get_connect_mac_addr()
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       ble_hid_earphone_state_cancel_page_scan()
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    ble_hid_earphone_state_enter_soft_poweroff()
#define EARPHONE_STATE_TWS_INIT(a)              ble_hid_earphone_state_tws_init(a)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      ble_hid_earphone_state_tws_connected(a,b)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           ble_hid_sys_event_handler_specific(a)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)



#elif LL_SYNC_EN

int ll_sync_earphone_state_init();
int ll_sync_earphone_state_set_page_scan_enable();
int ll_sync_earphone_state_get_connect_mac_addr();
int ll_sync_earphone_state_cancel_page_scan();
int ll_sync_earphone_state_enter_soft_poweroff();
int ll_sync_earphone_state_tws_init(int paired);
int ll_sync_earphone_state_tws_connected(int first_pair, u8 *comm_addr);
int ll_sync_sys_event_handler_specific(struct sys_event *event);

#define EARPHONE_STATE_INIT()                   ll_sync_earphone_state_init()
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   ll_sync_earphone_state_set_page_scan_enable()
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   ll_sync_earphone_state_get_connect_mac_addr()
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       ll_sync_earphone_state_cancel_page_scan()
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    ll_sync_earphone_state_enter_soft_poweroff()
#define EARPHONE_STATE_TWS_INIT(a)              ll_sync_earphone_state_tws_init(a)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      ll_sync_earphone_state_tws_connected(a,b)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           ll_sync_sys_event_handler_specific(a)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)

#elif TUYA_DEMO_EN
int tuya_earphone_state_init();
int tuya_earphone_state_set_page_scan_enable();
int tuya_earphone_state_get_connect_mac_addr();
int tuya_earphone_state_cancel_page_scan();
int tuya_earphone_state_enter_soft_poweroff();
int tuya_earphone_state_tws_init(int paired);
int tuya_earphone_state_tws_connected(int first_pair, u8 *comm_addr);
int tuya_sys_event_handler_specific(struct sys_event *event);

#define EARPHONE_STATE_INIT()                   tuya_earphone_state_init()
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   tuya_earphone_state_set_page_scan_enable()
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   tuya_earphone_state_get_connect_mac_addr()
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       tuya_earphone_state_cancel_page_scan()
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    tuya_earphone_state_enter_soft_poweroff()
#define EARPHONE_STATE_TWS_INIT(a)              tuya_earphone_state_tws_init(a)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      tuya_earphone_state_tws_connected(a,b)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           tuya_sys_event_handler_specific(a)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)


#elif (AI_APP_PROTOCOL | LE_AUDIO_EN)

int app_protocol_sys_event_handler(struct sys_event *event);
#define EARPHONE_STATE_INIT()                   do { } while(0)
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   do { } while(0)
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   do { } while(0)
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       do { } while(0)
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    do { } while(0)
#define EARPHONE_STATE_TWS_INIT(a)              do { } while(0)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      do { } while(0)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           app_protocol_sys_event_handler(a)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)

#elif TCFG_WIRELESS_MIC_ENABLE
#define EARPHONE_STATE_INIT()                   do { } while(0)
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   do { } while(0)
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   do { } while(0)
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       do { } while(0)
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    do { } while(0)
#define EARPHONE_STATE_TWS_INIT(a)              do { } while(0)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      do { } while(0)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           do { } while(0)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)

#else
int adv_earphone_state_init();
int adv_earphone_state_set_page_scan_enable();
int adv_earphone_state_get_connect_mac_addr();
int adv_earphone_state_cancel_page_scan();
int adv_earphone_state_enter_soft_poweroff();
int adv_earphone_state_tws_init(int paired);
int adv_earphone_state_tws_connected(int first_pair, u8 *comm_addr);
int adv_sys_event_handler_specific(struct sys_event *event);
int adv_earphone_state_sniff(u8 state);
int adv_earphone_state_role_switch(u8 role);

#define EARPHONE_STATE_INIT()                   adv_earphone_state_init()
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   adv_earphone_state_set_page_scan_enable()
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   adv_earphone_state_get_connect_mac_addr()
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       adv_earphone_state_cancel_page_scan()
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    adv_earphone_state_enter_soft_poweroff()
#define EARPHONE_STATE_TWS_INIT(a)              adv_earphone_state_tws_init(a)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      adv_earphone_state_tws_connected(a,b)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           adv_sys_event_handler_specific(a)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)                 adv_earphone_state_sniff(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)           adv_earphone_state_role_switch(a)

#endif


#else

#define EARPHONE_STATE_INIT()                   do { } while(0)
#define EARPHONE_STATE_SET_PAGE_SCAN_ENABLE()   do { } while(0)
#define EARPHONE_STATE_GET_CONNECT_MAC_ADDR()   do { } while(0)
#define EARPHONE_STATE_CANCEL_PAGE_SCAN()       do { } while(0)
#define EARPHONE_STATE_ENTER_SOFT_POWEROFF()    do { } while(0)
#define EARPHONE_STATE_TWS_INIT(a)              do { } while(0)
#define EARPHONE_STATE_TWS_CONNECTED(a, b)      do { } while(0)
#define SYS_EVENT_HANDLER_SPECIFIC(a)           do { } while(0)
#define SYS_EVENT_REMAP(a) 				        0
#define EARPHONE_STATE_SNIFF(a)
#define EARPHONE_STATE_ROLE_SWITCH(a)
#endif




extern void sys_auto_shut_down_enable(void);

extern void sys_auto_shut_down_disable(void);

extern int bt_in_background_event_handler(struct sys_event *event);

extern u8 bt_app_exit_check(void);













#endif
