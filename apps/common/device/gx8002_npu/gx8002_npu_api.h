#ifndef __GX8002_NPU_API_H__
#define __GX8002_NPU_API_H__

#include "app_config.h"
#include "update_loader_download.h"

#if TCFG_GX8002_NPU_ENABLE

#define GX8002_UPGRADE_TOGGLE 		1

#if GX8002_UPGRADE_TOGGLE
#define GX8002_UPGRADE_SDFILE_TOGGLE 		0 //for test, 升级文件存内置flash升级gx8002
#define GX8002_UPGRADE_SPP_TOGGLE 			0 //使用spp协议升级gx8002

#define GX8002_UPGRADE_APP_TOGGLE 			1 //使用测试盒/手机APP传输数据, 通过ota流程升级gx8002
#if CONFIG_DOUBLE_BANK_ENABLE
#define GX8002_UPGRADE_APP_TWS_TOGGLE 		1 //TWS同步升级使能
#endif /* #if CONFIG_DOUBLE_BANK_ENABLE */
#endif

#endif /* #if TCFG_GX8002_NPU_ENABLE */

//语音事件列表
enum gx8002_voice_event {
    MAIN_WAKEUP_VOICE_EVENT = 100,  //"小度小度", "天猫精灵", "小爱同学"
    MUSIC_PAUSE_VOICE_EVENT = 102,  //"暂停播放"
    MUSIC_STOP_VOICE_EVENT  = 103,  //"停止播放"
    MUSIC_PLAY_VOICE_EVENT  = 104,  //"播放音乐"
    VOLUME_UP_VOICE_EVENT   = 105,  //"增大音量"
    VOLUME_DOWN_VOICE_EVENT = 106,  //"减小音量"
    MUSIC_PREV_VOICE_EVENT  = 112,  //"播放上一首"
    MUSIC_NEXT_VOICE_EVENT  = 113,  //"播放下一首"
    CALL_ANSWER_VOICE_EVENT = 114,  //"接听电话"
    CALL_HANG_UP_VOICE_EVENT = 115, //"挂掉电话"
};

enum GX8002_MSG {
    GX8002_MSG_BEGIN = ('8' << 24) | ('0' << 16) | ('0' << 8) | ('2' << 0),
    GX8002_MSG_WAKEUP,
    GX8002_MSG_VOICE_EVENT,
    GX8002_MSG_UPDATE,
    GX8002_MSG_UPDATE_END,
    GX8002_MSG_SUSPEND,
    GX8002_MSG_RESUMED,
    GX8002_MSG_SELF_TEST,
    GX8002_MSG_GET_VERSION,
};

enum GX8002_UPDATE_TYPE {
    GX8002_UPDATE_TYPE_SDFILE = 0x5A,
    GX8002_UPDATE_TYPE_SPP,
    GX8002_UPDATE_TYPE_APP,
};

void gx8002_npu_int_edge_wakeup_handle(u8 index, u8 gpio);
int gx8002_npu_init(void);
void gx8002_event_state_update(u8 voice_event);
//int gx8002_uart_ota_init(void);
int gx8002_uart_sdfile_ota_init(void);
int gx8002_uart_spp_ota_init(void);
int gx8002_uart_app_ota_init(void);
void gx8002_voice_event_handle(u8 voice_event);
void gx8002_uart_app_ota_update_register_handle(void);

void gx8002_cold_reset(void);
void gx8002_normal_cold_reset(void);
void gx8002_upgrade_cold_reset(void);
void gx8002_update_end_post_msg(u8 flag);
void gx8002_voice_event_post_msg(u8 msg);

#endif /* #ifndef __GX8002_NPU_API_H__ */
