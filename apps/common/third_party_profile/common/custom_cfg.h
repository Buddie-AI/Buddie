#ifndef _CUSTOM_CFG_H_
#define _CUSTOM_CFG_H_

#include "typedef.h"
//#include "hw_cpu.h"
#include <string.h>
#include "app_config.h"

#define BT_CONNECTION_VERIFY        0   // 0是校验,1是不校验

#if (JL_EARPHONE_APP_EN && RCSP_UPDATE_EN)
#define VER_INFO_EXT_COUNT          2   //2
#else
#define VER_INFO_EXT_COUNT          0   //2
#endif
#define VER_INFO_EXT_MAX_LEN        24

enum {
    CFG_ITEM_ADV_IND = 0,
    CFG_ITEM_SCAN_RSP,
    CFG_ITEM_BLE_NAME,
    CFG_ITEM_BT_ADDR,

    CFG_ITEM_BT_NAME = 4,
    CFG_ITEM_PIN_CODE,
    CFG_ITEM_VER_INFO,
    CFG_ITEM_LOW_POWER_VOLTAGE,

    CFG_ITEM_EDR_ADDR = 8,
    CFG_ITEM_BLE_ADDR,
    CFG_ITEM_GATT_PROFILE,
    CFG_ITEM_RESET_IO_INFO,

    CFG_ITEM_PILOT_LAMP_IO_INFO = 12,
    CFG_ITEM_LINK_KEY_INFO,
    CFG_ITEM_POWER_IO_ON_OFF,
    CFG_ITEM_LAST_DEVICE_LINK_KEY_INFO,

    CFG_ITEM_BLE_READ_WRITE_UUID_INFO = 16,
#if VER_INFO_EXT_COUNT
    CFG_ITEM_VER_INFO_AUTHKEY,
    CFG_ITEM_VER_INFO_PROCODE,
#endif
    CFG_ITEM_PVID,           //供loader使用
    CFG_ITEM_MD5,
    CFG_ITEM_SDK_TYPE,
};

enum {
    EX_CFG_ERR_NONE = 0,
    EX_CFG_INDEX_ERR,
    EX_CFG_LEN_ERR,
};

enum {
    GET_VID_FROM_EX_CFG = 0,
    GET_PID_FROM_EX_CFG,
    GET_VER_FROM_EX_CFG,
#if VER_INFO_EXT_COUNT
    GET_AUTH_KEY_FROM_EX_CFG,
    GET_PRO_CODE_FROM_EX_CFG,
#endif
};

typedef struct _update_file_id {
    u8 vid[2];
    u8 pid[2];
    u8 ver[2];
    u8 len[4];
    u8 reserve[4];
    u8 crc[2];
} update_file_id_t;

typedef struct _update_file_ext_id {
    update_file_id_t update_file_id_info;
#if VER_INFO_EXT_COUNT
    u8 ext[VER_INFO_EXT_COUNT * (VER_INFO_EXT_MAX_LEN * 2)];
#endif
} update_file_ext_id_t;

typedef struct _update_file_reset_io {
    u8 io_num;
    u8 resever[15];
} update_file_reset_io_t;

typedef struct _update_file_pilot_lamp_io {
    u8 pilot_lamp_io[16];
} update_file_pilot_lamp_io_t;

typedef struct _update_file_link_key {
    u8 link_key[16];
}   update_file_link_key_t;

struct excfg_rsp_payload {
    u16 vid;
    u8 logo[5];
    u8 version;
    u8 addr[6];
};
//u32 custom_cfg_file_init(void);
//void ex_cfg_write_to_flash(void);
//u32 ex_cfg_fill_content_api(u8 cfg_index, u8 *data, u16 len);
u32 ex_cfg_fill_content_api(void);
u16 get_vid_pid_ver_from_cfg_file(u8 type);
#if VER_INFO_EXT_COUNT
u32 get_authkey_procode_from_cfg_file(u8 *data[], u8 *len, u8 type);
#endif

#if RCSP_UPDATE_EN
extern int rcsp_update_msg[10];
#endif

#endif

