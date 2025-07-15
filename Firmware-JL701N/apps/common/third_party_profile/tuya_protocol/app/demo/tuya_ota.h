#ifndef __TUYA_OTA_H__
#define __TUYA_OTA_H__

#include "typedef.h"

#define OTA_MAX_DATA_LEN    128

enum {
    TUYA_OTA_STATE_NORMAL = 0,
    TUYA_OTA_STATE_PID_NO_MATCH,
    TUYA_OTA_STATE_VER_LOW,
    TUYA_OTA_STATE_FILE_SIZE_TOO_LARGE,
};

enum {
    TUYA_OTA_DATA_SUCC = 0,
    TUYA_OTA_DATA_PKT_NUM_ERR,
    TUYA_OTA_DATA_LEN_ERR,
    TUYA_OTA_DATA_CRC_FAILED,
    TUYA_OTA_DATA_OTHER_ERR,
};

#define READ_BIG_U32(a)   ((*((u8*)(a)) <<24) + (*((u8*)(a)+1)<<16) + (*((u8*)(a)+2)<<8) + *((u8*)(a)+3))

typedef struct tuya_ota_req_response {
    u8  flag;
    u8  ota_version;
    u8  reserve;
    u32 frame_version;
    u16 max_pkt_len;
} tuya_ota_req_response_t;

typedef struct tuya_ota_file_info_response {
    u8  reserve;
    u8  state;
    u32 store_file_len;         //用于断点续传，目前不支持
    u32 store_crc;
    u8  md5[16];                //目前不使用
} tuya_ota_file_info_response_t;

typedef struct tuya_ota_file_offset_response {
    u8  reserve;
    u32 offset;
} tuya_ota_file_offset_response_t;

typedef struct tuya_ota_data_response {
    u8  reserve;
    u8  state;
} tuya_ota_data_response_t;

typedef struct tuya_ota_end_response {
    u8  reserve;
    u8  state;
} tuya_ota_end_response_t;

void tuya_ota_proc(u16 type, u8 *recv_data, u32 recv_len);

#endif
