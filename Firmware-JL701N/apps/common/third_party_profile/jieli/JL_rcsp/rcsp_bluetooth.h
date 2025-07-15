#ifndef __JL_BLUETOOTH_H__
#define __JL_BLUETOOTH_H__

#include "typedef.h"
#include "le_common.h"
#include "spp_user.h"
#include "system/event.h"
#include "rcsp_msg.h"
#include "ble_user.h"
#if RCSP_BTMATE_EN
void rcsp_init();
void rcsp_dev_select(u8 type);
void function_change_inform(void);

bool common_msg_deal(u32 param, void *priv);
bool ble_msg_deal(u32 param);
bool music_msg_deal(u32 param, void *priv);
bool linein_msg_deal(u32 param);
bool rtc_msg_deal(u32 param);

void rcsp_exit(void);
u8 rcsp_get_asr_status(void);
u8 get_rcsp_support_new_reconn_flag(void);

// enum {
// RCSP_BLE,
// RCSP_SPP,
// };


enum {
    ANDROID,
    APPLE_IOS,
};

struct JL_AI_VAR {
    ble_state_e  JL_ble_status;
    struct ble_server_operation_t *rcsp_ble;
    u8 JL_spp_status;
    struct spp_operation_t *rcsp_spp;
    volatile u8 speech_state;
    u32 feature_mask;
    u8 device_type;
    u8 phone_platform;
    void (*start_speech)(void);
    void (*stop_speech)(void);
    u8 err_report;
    volatile u8 file_browse_lock_flag;
    u32 return_msg;
    u8 spec_mode;
    struct __rcsp_user_var *rcsp_user;
    volatile u8 rcsp_run_flag;
    u8 ffr_mode;
    u16 ffr_time;
    u16 rcsp_timer_hdl;
    volatile u8 wait_asr_end;
    u8 new_reconn_flag;            //是否支持新的回连方式(进行地址修改)
};

struct _SPEECH_OVER_DEAL {
    u8 last_task;
    u8 status;
};

extern struct JL_AI_VAR jl_ai_var;
extern struct _SPEECH_OVER_DEAL speech_deal_val;

enum RCSP_MSG_T {
    MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET = RCSP_MSG_END,
    MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE,
    MSG_JL_LOADER_DOWNLOAD_START,
    MSG_JL_UPDATE_START,
    MSG_JL_ENTER_UPDATE_MODE,
    MSG_JL_DEV_DISCONNECT,
    MSG_JL_BLE_UPDATE_START,
    MSG_JL_SPP_UPDATE_START,
};

bool rcsp_msg_post(RCSP_MSG msg, int argc, ...);

#define		SDK_TYPE_AC690X		0x0
#define		SDK_TYPE_AC692X		0x1
#define 	SDK_TYPE_AC693X		0x2
#define 	SDK_TYPE_AC695X 	0x3
#define		SDK_TYPE_AC697X 	0x4

#if   (defined CONFIG_CPU_BR21)
#define		RCSP_SDK_TYPE		SDK_TYPE_AC692X
#elif (defined CONFIG_CPU_BR22)
#define		RCSP_SDK_TYPE		SDK_TYPE_AC693X
#elif (defined CONFIG_CPU_BR23)
#define		RCSP_SDK_TYPE		SDK_TYPE_AC695X
#elif (defined CONFIG_CPU_BR30)
#define		RCSP_SDK_TYPE		SDK_TYPE_AC697X
#else
#define		RCSP_SDK_TYPE		SDK_TYPE_AC693X
#endif
#endif

#endif
