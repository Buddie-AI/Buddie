

#ifndef _3TH_PROTOCOL_EVENT_H
#define _3TH_PROTOCOL_EVENT_H

//该文件只定义库里面和库外面都需要用得的第三方APP协议的消息和状态
#include<string.h>
#include <stdint.h>
#include "typedef.h"

#define DEMO_HANDLER_ID   0x300     /*作为一个使用的例子，同时也可作为客户自己添加协议的ID*/
#define GMA_HANDLER_ID    0x400     /*阿里天猫协议接口ID*/
#define MMA_HANDLER_ID    0x500     /*小米MMA协议接口ID*/
#define DMA_HANDLER_ID    0x600     /*百度DMA协议接口ID*/
#define TME_HANDLER_ID    0x700     /*腾讯酷狗TME协议接口ID*/
#define AMA_HANDLER_ID    0x800     /*亚马逊的AMA协议接口ID*/
#define GFPS_HANDLER_ID   0x900     /*谷歌快对的协议接口ID*/
//app protocol公共消息
enum {
    APP_PROTOCOL_COMMON_NOTICE      = 0,
    APP_PROTOCOL_CONNECTING,       /*保留，暂未使用*/
    APP_PROTOCOL_CONNECTED_BLE,    /*APP通过BLE连接成功状态更新*/
    APP_PROTOCOL_CONNECTED_SPP,    /*APP通过SPP连接成功状态更新*/
    APP_PROTOCOL_DISCONNECT,       /*APP连接断开状态更新*/
    APP_PROTOCOL_AUTH_PASS,        /*连接认证通过标识更新*/
    APP_PROTOCOL_SPEECH_ENCODER_TYPE,
    APP_PROTOCOL_SPEECH_START,     /*语音识别功能启动状态*/
    APP_PROTOCOL_SPEECH_STOP,      /*语音识别功能停止状态*/
    APP_PROTOCOL_SET_VOLUME,       /*app配置音量*/
    APP_PROTOCOL_GET_VOLUME,       /*app读取音量*/
    APP_PROTOCOL_GET_AUX_STATUS,   /*保留，暂未使用*/
    APP_PROTOCOL_LIB_TWS_DATA_SYNC,  /*需要更新给另一端tws数据*/
    APP_PROTOCOL_COMMON_NOTICE_END  = 0x14F,
};
//OTA消息
enum {
    APP_PROTOCOL_OTA_COMMON_NOTICE  = APP_PROTOCOL_COMMON_NOTICE_END + 1,
    APP_PROTOCOL_OTA_CHECK,
    APP_PROTOCOL_OTA_GET_APP_VERSION,
    APP_PROTOCOL_OTA_CHECK_CRC,
    APP_PROTOCOL_OTA_BEGIN,
    APP_PROTOCOL_OTA_TRANS_DATA,
    APP_PROTOCOL_OTA_PERCENT,
    APP_PROTOCOL_OTA_END,
    APP_PROTOCOL_OTA_SUCCESS,
    APP_PROTOCOL_OTA_FAIL,
    APP_PROTOCOL_OTA_CANCLE,
    APP_PROTOCOL_OTA_REBOOT,
    APP_PROTOCOL_OTA_COMMON_NOTICE_END      = 0x1FF,
};
//GMA私有消息
enum {
    APP_PROTOCOL_GMA_NOTICE_BEGIN           = GMA_HANDLER_ID,
    APP_PROTOCOL_GMA_FMTX_SETFRE,       /*样机支持FM功能的APP配置fm参数*/
    APP_PROTOCOL_GMA_FMTX_GETFRE,       /*app获取当前的fm配置参数*/
    APP_PROTOCOL_GMA_NOTICE_END             = GMA_HANDLER_ID + 0xFF,
};
//MMA私有消息
enum {
    APP_PROTOCOL_MMA_NOTICE                 = MMA_HANDLER_ID,
    APP_PROTOCOL_MMA_SAVE_INFO,           //库里面不直接访问VM接口，有些信息保存外面做
    APP_PROTOCOL_MMA_READ_INFO,
    APP_PROTOCOL_MMA_SAVE_ADV_COUNTER,
    APP_PROTOCOL_MMA_READ_ADV_COUNTER,
    APP_PROTOCOL_MMA_NOTICE_END             = MMA_HANDLER_ID + 0xFF,
};

//DMA私有消息
enum {
    APP_PROTOCOL_DMA_NOTICE                 = DMA_HANDLER_ID,
    APP_PROTOCOL_DMA_SAVE_RAND,          //库里面不直接访问VM接口，有些信息保存外面做
    APP_PROTOCOL_DMA_READ_RAND,
    APP_PROTOCOL_DMA_TWS_SNED_RAND,
    APP_PROTOCOL_DMA_TTS_TYPE,
    APP_PROTOCOL_DMA_SAVE_OTA_INFO,          //库里面不直接访问VM接口，有些信息保存外面做
    APP_PROTOCOL_DMA_READ_OTA_INFO,
    APP_PROTOCOL_DMA_NOTICE_END             = DMA_HANDLER_ID + 0xFF,
};

// GFPS私有消息
enum {
    APP_PROTOCOL_GFPS_RING_STOP_ALL                     =   GFPS_HANDLER_ID,
    APP_PROTOCOL_GFPS_RING_RIGHT,
    APP_PROTOCOL_GFPS_RING_LEFT,
    APP_PROTOCOL_GFPS_RING_ALL,
    APP_PROTOCOL_GFPS_HEARABLE_CONTROLS,
};

#define GFP_TWS_ENABLE  1

//APP_PROTOCOL获取电量的类型
#define APP_PROTOCOL_BAT_T_CHARGE_FLAG   0
#define APP_PROTOCOL_BAT_T_MAIN          1
#define APP_PROTOCOL_BAT_T_BOX           2
#define APP_PROTOCOL_BAT_T_TWS_LEFT      3
#define APP_PROTOCOL_BAT_T_TWS_RIGHT     4
#define APP_PROTOCOL_BAT_T_TWS_SIBLING   5
#define APP_PROTOCOL_BAT_T_LOW_POWER     6
#define APP_PROTOCOL_BAT_T_MAX           8

typedef struct _ota_frame_info_t {
    u16 max_pkt_len;
    u16 frame_crc;
    u32 frame_size;
} ota_frame_info;

//*****//
typedef enum {
    USER_NOTIFY_STATE_CONNECTED = 0,         /**< 手机APP与设备建立连接 */
    USER_NOTIFY_STATE_DISCONNECTED,          /**< 手机APP与设备的连接断开 */
    USER_NOTIFY_STATE_MOBILE_CONNECTED,      /**< 手机与设备建立BT连接（A2DP、HFP、AVRCP） */
    USER_NOTIFY_STATE_MOBILE_DISCONNECTED,   /**< 手机与设备BT连接（A2DP、HFP、AVRCP）断开 */
    USER_NOTIFY_STATE_SEND_PREPARE_DONE,     /**< 设备端进入可向手机发送数据的状态 */
    USER_NOTIFY_STATE_TWS_CONNECT,           /**< TWS类设备两端连接成功 */
    USER_NOTIFY_STATE_TWS_DISCONNECT,        /**< TWS类设备两端断开连接 */
    USER_NOTIFY_STATE_BOX_OPEN,              /**< 盒仓开启 */
    USER_NOTIFY_STATE_BOX_CLOSE,             /**< 盒仓关闭 */
    USER_NOTIFY_STATE_ROLE_SWITCH_START,     /**< TWS类设备开始进入主从切换流程 */
    USER_NOTIFY_STATE_ROLE_SWITCH_FINISH,    /**< TWS类设备主从切换完成 */
    USER_NOTIFY_STATE_ROLE_SWITCH_REQUEST,   /**< TWS类设备向APP发起主从切换请求 */
    USER_NOTIFY_STATE_DOUBLE_CLICK,          /**< 设备双击按键 */
    USER_NOTIFY_STATE_KEYWORD_DETECTED,      /**< 设备语音唤醒 */
    USER_NOTIFY_STATE_BATTERY_LEVEL_UPDATE,  /**< 通知耳机电量更新 */
    USER_NOTIFY_STATE_ONE_CLICK,             /**< 设备单击按键 */
} USER_NOTIFY_STATE;

typedef enum {
    /*尽量返回简单的1或者0*/
    CHECK_STATUS_TWS_MASTER,
    CHECK_STATUS_TWS_SLAVE,
    CHECK_STATUS_TWS_PAIR_STA, /*1是tws已经配对了，0是未配对*/
    CHECK_STATUS_TWS_SIDE,     /*0是单耳，1是左耳，2是右耳*/
    CHECK_STATUS_TWS_REV,
    CHECK_STATUS_TWS_REV1,
} CHECK_STATUS;
#endif
