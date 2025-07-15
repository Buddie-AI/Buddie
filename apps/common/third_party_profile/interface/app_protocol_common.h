

#ifndef _3TH_PROTOCOL_COMMON_H
#define _3TH_PROTOCOL_COMMON_H

#include <string.h>
#include <stdint.h>
#include "typedef.h"
#include "tone_player.h"
#include "audio_base.h"
#include "system/event.h"

#ifndef TONE_RES_ROOT_PATH
#if TCFG_NOR_FS && (TCFG_VIRFAT_FLASH_ENABLE == 0)
#define NOR_FLASH_RES_ROOT_PATH	 "storage/res_nor/C/"
#define TONE_RES_ROOT_PATH		 NOR_FLASH_RES_ROOT_PATH    //内置flash提示音根路径
#else
#define TONE_RES_ROOT_PATH		 SDFILE_RES_ROOT_PATH   	//内置flash提示音根路径
#endif//TCFG_NOR_FS
#endif

#define APP_PROTOCOL_TWS_SYNC_ID        0xFFFF321A
#define APP_PROTOCOL_TWS_SEND_ID        0xFFFF321B

enum {
    APP_PROTOCOL_TONE_CONNECTED_ALL_FINISH = 0,//所有连接完成【已连接，你可以按AI键来和我进行对话】 ok
    APP_PROTOCOL_TONE_PROTOCOL_CONNECTED,//APP已连接，经典蓝牙未连接【请在手机上完成蓝牙配对】 con
    APP_PROTOCOL_TONE_CONNECTED_NEED_OPEN_APP,//经典蓝牙已连接，app未连接【已配对，请打开app进行连接】 btcon
    APP_PROTOCOL_TONE_DISCONNECTED,//经典蓝牙已断开【蓝牙已断开，请在手机上完成蓝牙配对】 dis
    APP_PROTOCOL_TONE_DISCONNECTED_ALL,//经典蓝牙和【蓝牙未连接，请用手机蓝牙和我连接吧】alldis
    APP_RROTOCOL_TONE_OPEN_APP,//经典蓝牙已连接，APP未连接【请打开APP】 app
    APP_RROTOCOL_TONE_SPEECH_APP_START,//手机APP启动语音助手
    APP_RROTOCOL_TONE_SPEECH_KEY_START,//按键启动语音助手
    APP_RROTOCOL_TONE_MAX,//提示音最大数量
};

//发送给对耳的消息，格式为：opcode len data
enum {
    APP_PROTOCOL_TWS_FOR_LIB = 0,

    GMA_TWS_CMD_SYNC_LIC,
};

//对耳同步的命令，格式为cmd value
enum {
    APP_PROTOCOL_SYNC_TONE = 0,
};

//发送到系统事件处理的事件
enum {
    AI_EVENT_SPEECH_START = 0,
    AI_EVENT_SPEECH_STOP,
    AI_EVENT_APP_CONNECT,
    AI_EVENT_APP_DISCONNECT,
};

struct app_protocol_private_handle_t {
    void (*tws_rx_from_siblling)(u16 opcode, u8 *data, u16 len); //收到对耳数据的处理接口
    void (*tws_sync_func)(int cmd, int value); //对耳同步执行的接口
    int (*sys_event_handler)(struct sys_event *event); //系统事件
};

//提示音注册和播放
void app_protocol_tone_register(const char **tone_table);
const char *app_protocol_get_tone(int index);
void app_protocol_tone_play(int index, int tws_sync);
void app_speech_tone_play_end_callback(void *priv, int flag); //提示音播放完毕回调函数


extern int ai_mic_is_busy(void); //mic正在被使用
extern int ai_mic_rec_start(void); //启动mic和编码模块
extern int ai_mic_rec_close(void); //停止mic和编码模块
extern bool bt_is_sniff_close(void);

//语音识别接口
int mic_rec_pram_init(/* const char **name,  */u32 enc_type, u8 opus_type, u16(*speech_send)(u8 *buf, u16 len), u16 frame_num, u16 cbuf_size);
int __app_protocol_speech_start(void);
void __app_protocol_speech_stop(void);
int app_protocol_start_speech_by_app(u8 tone_en); //APP启动
int app_protocol_stop_speech_by_app(); //APP停止
int app_protocol_start_speech_by_key(struct sys_event *event); //按键启动
int app_protocol_stop_speech_by_key(void); //按键停止


//协议获取和设置一些设备信息接口
int app_protocol_set_volume(u8 vol);
u8 app_protocal_get_bat_by_type(u8 type);
u32 read_cfg_file(void *buf, u16 len, char *path);
const u8 *app_protocal_get_license_ptr(void); //获取三元组信息的头指针
int app_protocol_license2flash(const u8 *data, u16 len); //将三元组信息保存到flash里面

//事件处理函数指针列表注册和接口
void app_protocol_handle_register(struct app_protocol_private_handle_t *hd);
struct app_protocol_private_handle_t *app_protocol_get_handle();
void app_protocol_tws_sync_private_deal(int cmd, int value); //对耳同步事件私有协议处理函数
void app_protocol_tws_rx_data_private_deal(u16 opcode, u8 *data, u16 len); //私有协议处理来自对耳的数据函数
int app_protocol_sys_event_private_deal(struct sys_event *event); //私有协议处理系统事件函数

void app_protocol_post_bt_event(u8 event, void *priv); //发送到系统事件处理
int app_protocol_post_app_core_callback(int callback, void *priv); //放到app_core中处理
int app_protocol_tws_send_to_sibling(u16 opcode, u8 *data, u16 len); //发送命令数据给对耳
int app_protocol_tws_sync_send(int cmd, int value); //对耳同步执行命令

int app_protocol_key_event_handler(struct sys_event *event); //按键事件处理函数
int app_protocol_sys_event_handler(struct sys_event *event); //系统事件处理函数

#endif

