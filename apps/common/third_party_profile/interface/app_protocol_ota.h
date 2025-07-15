#ifndef __APP_PROTOCOL_OTA_H__
#define __APP_PROTOCOL_OTA_H__

#include "typedef.h"

#define NO_SUPPORT_OTA          0xff
#define INITIATIVE_OTA          0x01
#define PASSIVE_OTA             0x02

typedef struct app_protocol_ota_type_map_t {
    u32 protocol_id;
    u8  ota_type;
} app_protocol_ota_type_map;

typedef struct app_protocol_ota_api_t {
    u32(*ota_request_data)(void *priv, u32 offset, u16 len);
    u32(*ota_report_result)(u8 result);
    void (*ota_notify_file_size)(u32 file_size);
    void (*ota_crc_init_hdl)(void);
    u16(*ota_crc_calc_hdl)(u16 init_crc, u8 *data, u32 len);
} app_protocol_ota_api;

int app_protocol_ota_message_handler(int id, int opcode, u8 *data, u32 len);
void app_protocol_ota_init(app_protocol_ota_api  *api, u8 need_wait_app_reboot);
int app_protocol_ota_interrupt_by_disconnect(int id);
int app_protocol_update_success_flag(void);

#endif //__APP_PROTOCOL_OTA_H__
