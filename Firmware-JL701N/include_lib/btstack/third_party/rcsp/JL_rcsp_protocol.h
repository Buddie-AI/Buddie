#ifndef __JL_PROTOCOL_H__
#define __JL_PROTOCOL_H__

#include "typedef.h"
#include "JL_rcsp_packet.h"

#define ATTR_TYPE_PROTOCOL_VERSION  (0)
#define ATTR_TYPE_SYS_INFO          (1)
#define ATTR_TYPE_EDR_ADDR          (2)
#define ATTR_TYPE_PLATFORM          (3)
#define ATTR_TYPE_FUNCTION_INFO     (4)
#define ATTR_TYPE_DEV_VERSION       (5)
#define ATTR_TYPE_SDK_TYPE          (6)
#define ATTR_TYPE_UBOOT_VERSION     (7)
#define ATTR_TYPE_DOUBLE_PARITION   (8)
#define ATTR_TYPE_UPDATE_STATUS     (9)
#define ATTR_TYPE_DEV_VID_PID       (10)
#define ATTR_TYPE_DEV_AUTHKEY       (11)
#define ATTR_TYPE_DEV_PROCODE       (12)
#define ATTR_TYPE_DEV_MAX_MTU       (13)
#define ATTR_TEYP_BLE_ADDR			(17)
#define ATTR_TYPE_MD5_GAME_SUPPORT  (19)



#define COMMON_INFO_ATTR_BATTERY       (0)
#define COMMON_INFO_ATTR_VOL           (1)
#define COMMON_INFO_ATTR_DEV           (2)
#define COMMON_INFO_ATTR_ERR_REPORT    (3)
#define COMMON_INFO_ATTR_EQ            (4)
#define COMMON_INFO_ATTR_FILE_BROWSE_TYPE      (5)
#define COMMON_INFO_ATTR_FUN_TYPE      (6)
#define COMMON_INFO_ATTR_LIGHT     	   (7)
#define COMMON_INFO_ATTR_FMTX      	   (8)
#define COMMON_INFO_ATTR_HIGH_LOW_SET  (11)
#define COMMON_INFO_ATTR_PRE_FETCH_ALL_EQ_INFO (12)
#define COMMON_INFO_ATTR_ANC_VOICE     (13)
#define COMMON_INFO_ATTR_FETCH_ALL_ANC_VOICE  (14)
#define COMMON_INFO_ATTR_PHONE_SCO_STATE_INFO (15)
#define COMMON_INFO_ATTR_ASSISTED_HEARING  (20)
#define COMMON_INFO_ATTR_ADAPTIVE_NOISE_REDUCTION	(21)
#define COMMON_INFO_ATTR_AI_NO_PICK 			  	(22)
#define COMMON_INFO_ATTR_SCENE_NOISE_REDUCTION 		(23)
#define COMMON_INFO_ATTR_WIND_NOISE_DETECTION 		(24)
#define COMMON_INFO_ATTR_VOICE_ENHANCEMENT_MODE   	(25)



#define BT_INFO_ATTR_MUSIC_TITLE      	(0)
#define BT_INFO_ATTR_MUSIC_ARTIST      	(1)
#define BT_INFO_ATTR_MUSIC_ALBUM      	(2)
#define BT_INFO_ATTR_MUSIC_NUMBER      	(3)
#define BT_INFO_ATTR_MUSIC_TOTAL      	(4)
#define BT_INFO_ATTR_MUSIC_GENRE      	(5)
#define BT_INFO_ATTR_MUSIC_TIME      	(6)
#define BT_INFO_ATTR_MUSIC_STATE      	(7)
#define BT_INFO_ATTR_MUSIC_CURR_TIME    (8)

#define BT_INFO_ATTR_01                        (0)

#define MUSIC_INFO_ATTR_STATUS                 (0)
#define MUSIC_INFO_ATTR_FILE_NAME              (1)
#define MUSIC_INFO_ATTR_FILE_PLAY_MODE         (2)

#define RTC_INFO_ATTR_RTC_TIME                 (0)
#define RTC_INFO_ATTR_RTC_ALRAM                (1)

#define LINEIN_INFO_ATTR_STATUS                (0)


/***************************************/
/*     注意：不能在这个枚举里加代码    */
/*     注意：不能在这个枚举里加代码    */
/*     注意：不能在这个枚举里加代码    */
/***************************************/
enum {
    JL_OPCODE_DATA = 0x01,
    JL_OPCODE_GET_TARGET_FEATURE_MAP = 0x02,
    JL_OPCODE_GET_TARGET_FEATURE = 0x03,
    // JL_OPCODE_SWITCH_DEVICE = 0x04,
    JL_OPCODE_DISCONNECT_EDR = 0x06,
    JL_OPCODE_SYS_INFO_GET = 0x07,
    JL_OPCODE_SYS_INFO_SET = 0x08,
    JL_OPCODE_SYS_INFO_AUTO_UPDATE = 0x09,
    JL_OPCODE_CALL_REQUEST = 0xa,
    JL_OPCODE_SWITCH_DEVICE = 0x0b,
    JL_OPCODE_FILE_BROWSE_REQUEST_START = 0x0C,
    JL_OPCODE_FILE_BROWSE_REQUEST_STOP = 0x0D,
    JL_OPCODE_FUNCTION_CMD = 0x0E,

    JL_OPCODE_SYS_OPEN_BT_SCAN = 0x12,
    JL_OPCODE_SYS_UPDATE_BT_STATUS = 0X13,
    JL_OPCODE_SYS_STOP_BT_SCAN = 0X14,
    JL_OPCODE_SYS_BT_CONNECT_SPEC = 0X15,
    JL_OPCODE_SYS_EMITTER_BT_CONNECT_STATUS = 0XD3, // 临时调整， 被遗忘的指令
    JL_OPCODE_SYS_FIND_DEVICE = 0X19,

    JL_OPCODE_EXTRA_FLASH_OPT = 0X1A,

    //文件传输相关命令
    JL_OPCODE_FILE_TRANSFER_START = 0X1B,
    JL_OPCODE_FILE_TRANSFER_END = 0X1C,
    JL_OPCODE_FILE_TRANSFER = 0X1D,
    JL_OPCODE_FILE_TRANSFER_CANCEL = 0X1E,
    JL_OPCODE_FILE_DELETE = 0X1F,
    JL_OPCODE_FILE_RENAME = 0X20,

    JL_OPCODE_ACTION_PREPARE = 0X21,//APP操作预处理
    JL_OPCODE_DEVICE_FORMAT = 0X22,//设备格式化
    JL_OPCODE_ONE_FILE_DELETE = 0x23,//删除一个文件
    JL_OPCODE_ONE_FILE_TRANS_BACK = 0x24,//回传一个文件
    JL_OPCODE_ALARM_EXTRA = 0x25,
    JL_OPCODE_FILE_BLUK_TRANSFER = 0x26,//批量大文件前准备
    JL_OPCODE_DEVICE_PARM_EXTRA = 0x27,//设备操作，如文件传输、文件删除、设备格式化参数扩展

    JL_OPCODE_SIMPLE_FILE_TRANS = 0x28,

    JL_OPCODE_SPORTS_DATA_INFO_GET = 0xA0,
    JL_OPCODE_SPORTS_DATA_INFO_SET = 0xA1,
    JL_OPCODE_SPORTS_DATA_INFO_AUTO_UPDATE = 0xA2,
    JL_OPCODE_SENSOR_LOG_DATA_AUTO_UPDATE = 0xA3,
    JL_OPCODE_SENSOR_NFC_FUNCTION_OPT = 0xA4,
    JL_OPCODE_SPORTS_DATA_INFO_OPT = 0xA5,
    JL_OPCODE_SPORTS_DATA_SYNC = 0xA6,

    JL_OPCODE_NOTIFY_MTU = 0xD1,
    JL_OPCODE_GET_MD5 = 0xD4,
    JL_OPCODE_LOW_LATENCY_PARAM = 0xD5,
    JL_OPTCODE_EXTRA_FLASH_INFO = 0XD6,

    JL_OPCODE_CUSTOMER_USER = 0xFF,
};
/***************************************/
/***************************************/

enum {
    JL_NOT_NEED_RESPOND = 0,
    JL_NEED_RESPOND,
};

enum {
    JL_AUTH_NOTPASS = 0,
    JL_AUTH_PASS,
};

typedef enum __JL_ERR {
    JL_ERR_NONE = 0x0,
    JL_ERR_SEND_DATA_OVER_LIMIT,
    JL_ERR_SEND_BUSY,
    JL_ERR_SEND_NOT_READY,
    JL_ERR_EXIT,
} JL_ERR;

typedef enum __JL_PRO_STATUS {
    JL_PRO_STATUS_SUCCESS = 0x0,
    JL_PRO_STATUS_FAIL,
    JL_PRO_STATUS_UNKOWN_CMD,
    JL_PRO_STATUS_BUSY,
    JL_PRO_STATUS_NO_RESPONSE,
    JL_PRO_STATUS_CRC_ERR,
    JL_PRO_STATUS_ALL_DATA_CRC_ERR,
    JL_PRO_STATUS_PARAM_ERR,
    JL_PRO_STATUS_RESP_DATA_OVER_LIMIT,

} JL_PRO_STATUS;

///*< JL_CMD、JL_CMD_response、JL_DATA、JL_DATA_response packet send functions>*/
JL_ERR JL_CMD_send(u8 OpCode, u8 *data, u16 len, u8 request_rsp);
JL_ERR JL_CMD_response_send(u8 OpCode, u8 status, u8 sn, u8 *data, u16 len);
JL_ERR JL_DATA_send(u8 OpCode, u8 CMD_OpCode, u8 *data, u16 len, u8 request_rsp);
JL_ERR JL_DATA_response_send(u8 OpCode, u8 status, u8 sn, u8 CMD_OpCode, u8 *data, u16 len);

///*<JL_CMD、JL_CMD_response、JL_DATA、JL_DATA_response recieve>*/
typedef struct __JL_PRO_CB {
    /*send function callback, SPP or ble*/
    void *priv;
    bool (*fw_ready)(void *priv);
    s32(*fw_send)(void *priv, void *buf, u16 len);
    /*JL_CMD、JL_CMD_response、JL_DATA、JL_DATA_response packet recieve callback*/
    void (*CMD_resp)(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len);
    void (*DATA_resp)(void *priv, u8 OpCode_SN, u8 CMD_OpCode, u8 *data, u16 len);
    void (*CMD_no_resp)(void *priv, u8 OpCode, u8 *data, u16 len);
    void (*DATA_no_resp)(void *priv, u8 CMD_OpCode, u8 *data, u16 len);
    void (*CMD_recieve_resp)(void *priv, u8 OpCode, u8 status, u8 *data, u16 len);
    void (*DATA_recieve_resp)(void *priv, u8 status, u8 CMD_OpCode, u8 *data, u16 len);
    u8(*wait_resp_timeout)(void *priv, u8 OpCode, u8 counter);
    void (*auth_pass_callback)(void *priv);
} JL_PRO_CB;


extern u32 rcsp_fw_ready(void);
extern u32 rcsp_protocol_need_buf_size(void);

extern void JL_protocol_init(u8 *buf, u32 len);
extern void JL_protocol_exit(void);

extern void JL_protocol_dev_switch(const JL_PRO_CB *cb);

extern void JL_protocol_data_recieve(void *priv, void *buf, u16 len);
extern void JL_protocol_resume(void);

extern void JL_protocol_process(void);

extern void set_auth_pass(u8 auth_pass);

extern void JL_set_cur_tick(u16 tick);

extern bool rcsp_send_list_is_empty(void);


extern void JL_send_packet_process(void);
extern void JL_recieve_packet_parse_process(void);


#endif//__JL_PROTOCOL_H__


