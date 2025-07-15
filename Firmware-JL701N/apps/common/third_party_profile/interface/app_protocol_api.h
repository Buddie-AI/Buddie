

#ifndef _3TH_PROTOCOL_API_H
#define _3TH_PROTOCOL_API_H

#include<string.h>
#include <stdint.h>
#include "typedef.h"
#include "os/os_api.h"
#include "btstack/third_party/app_protocol_event.h"
#include "app_protocol_common.h"

#define APP_PROTOCOL_TASK_NAME    "app_proto"

//定义这几个宏仅仅用于编译优化代码量。需要那个选哪个，flash够大，可以全部都打开的。
//不要在板级头文件重复定义。
#define APP_PROTOCOL_DEMO_CODE      0
#define APP_PROTOCOL_GMA_CODE       0
#define APP_PROTOCOL_AMA_CODE       0
#define APP_PROTOCOL_DMA_CODE       0
#define APP_PROTOCOL_TME_CODE       0
#define APP_PROTOCOL_MMA_CODE       0
#define APP_PROTOCOL_GFPS_CODE      1

#define APP_PROTOCOL_SPEECH_EN      0 //语音助手功能，若无此功能则关掉
#define APP_PROTOCOL_READ_CFG_EN    0 //从custom.dat中读取配置信息，若无此功能则关掉

#define DEMO_HANDLER_ID   0x300     /*作为一个使用的例子，同时也可作为客户自己添加协议的ID*/
#define GMA_HANDLER_ID    0x400     /*阿里天猫协议接口ID*/
#define MMA_HANDLER_ID    0x500     /*小米MMA协议接口ID*/
#define DMA_HANDLER_ID    0x600     /*百度DMA协议接口ID*/
#define TME_HANDLER_ID    0x700     /*腾讯酷狗TME协议接口ID*/
#define AMA_HANDLER_ID    0x800     /*亚马逊的AMA协议接口ID*/
#define GFPS_HANDLER_ID   0x900     /*谷歌快对的协议接口ID*/

//app os task message
enum {
    //Q_USER          =0x400000
    APP_PROTOCOL_RX_DATA_EVENT = (Q_USER + 100),
    APP_PROTOCOL_TX_DATA_EVENT,
    APP_PROTOCOL_TASK_EXIT,
};

//参数配置类接口(在库外面的文件common区)
//配置在init之前的参数接口
void app_protocol_set_product_id(u32 handler_id, u32 pid);
void app_protocol_set_vendor_id(u32 handler_id, u32 vid);
void app_protocol_set_local_version(u32 handler_id, u32 version);
void app_protocol_set_info_group(u32 handler_id, void *addr);   //如配置三元组
void app_protocol_tws_role_check_register(u8(*handler)(void)); /*注册获取tws状态的函数接口*/
void app_protocol_set_tws_sibling_mac(u8 *mac);


//公共类函数接口
/*这个接口主要是建立一个线程，注册一些协议需要用的公共接口，比如resume，message_handler等函数。建立了线程之后，初始化*/
void app_protocol_all_init();
void app_protocol_init(int handler_id);
/*主要是删除线程操作，在删除完之后再释放APP协议栈的资源*/
void app_protocol_exit(int handler_id);
/*开始语音识别功能*/
int app_protocol_start_speech_cmd();
/*手动停止语音识别功能*/
int app_protocol_stop_speech_cmd();
int app_protocol_left_start_speech_cmd();
int app_protocol_right_start_speech_cmd();
void app_protocol_update_battery(u8 main, u8 left, u8 right, u8 box);
void app_protocol_ble_adv_switch(int sw);
void app_protocol_ibeacon_switch(int sw);
void app_protocol_disconnect(void *addr);
void app_protocol_get_tws_data_for_lib(u8 *data, u32 len);
/*发送mic相应编码后的数据*/
int app_protocol_send_voice_data(uint8_t *voice_buf, uint16_t voice_len);
/*获取有没有链路连接完成状态*/
int app_protocol_check_connect_success();
bool is_tws_master_role();

u32 app_protocol_get_cur_handle_id();

int app_protocol_get_version_by_id(int id);

int app_protocol_reply_frame_check_result(int result);
void app_protocal_update_tws_state_to_lib(int state);

#endif
