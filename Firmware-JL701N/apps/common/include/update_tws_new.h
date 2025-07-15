#ifndef _UPRATE_TWS_NEW_H
#define _UPRATE_TWS_NEW_H

#include "app_config.h"

#if (OTA_TWS_SAME_TIME_ENABLE &&  OTA_TWS_SAME_TIME_NEW)//(RCSP_ADV_EN || AI_APP_PROTOCOL))

#define SYS_BT_OTA_EVENT_TYPE_STATUS (('O' << 24) | ('T' << 16) | ('A' << 8) | '\0')
#define TWS_FUNC_ID_OTA_TRANS	(('O' << (3 * 8)) | ('T' << (2 * 8)) | ('A' << (1 * 8)) | ('\0'))

#include "system/event.h"
#include "update_loader_download.h"

typedef int 		  sint32_t;

enum {
    OTA_OVER = 0,
    OTA_INIT,
    OTA_START,
    OTA_VERIFY_ING,
    OTA_VERIFY_END,
    OTA_SUCC,
};

enum {
    OTA_SINGLE_EARPHONE,
    OTA_TWS,
};

enum {
    OTA_START_UPDATE = 0,
    OTA_START_UPDATE_READY,
    OTA_START_VERIFY,
    OTA_UPDATE_OVER,
    OTA_UPDATE_ERR,
    OTA_UPDATE_SUCC,
};

enum {
    OTA_TYPE_SET = 0,
    OTA_TYPE_GET,
    OTA_STATUS_SET,
    OTA_STATUS_GET,
    OTA_REMOTE_STATUS_SET,
    OTA_REMOTE_STATUS_GET,
    OTA_RESULT_SET,
    OTA_RESULT_GET,
};


enum {
    OTA_STOP_APP_DISCONNECT,
    OTA_STOP_LINK_DISCONNECT,
    OTA_STOP_UPDATE_OVER_SUCC,
    OTA_STOP_UPDATE_OVER_ERR,
    OTA_STOP_PHONE,
};

enum {
    SYNC_CMD_START_UPDATE,
    SYNC_CMD_START_VERIFY,
    SYNC_CMD_UPDATE_OVER,
    SYNC_CMD_UPDATE_ERR,
};

//TWS OTA CMD
enum {
    OTA_TWS_INIT,
    OTA_TWS_START_UPDATE = 0x1,
    OTA_TWS_START_UPDATE_RSP,
    OTA_TWS_TRANS_UPDATE_DATA,
    OTA_TWS_TRANS_UPDATE_DATA_RSP,
    OTA_TWS_VERIFY,
    OTA_TWS_VERIFY_RSP,
    OTA_TWS_UPDATE_RESULT,
    OTA_TWS_WRITE_BOOT_INFO,
    OTA_TWS_WRITE_BOOT_INFO_RSP,
    OTA_TWS_USER_CHIP_UPDATE,
};

enum {
    OTA_TWS_CMD_SUCC = 0x1,
    OTA_TWS_CMD_ERR,
};

#define    MSG_OTA_TWS_START_UPDATE           0x1
#define    MSG_OTA_TWS_START_UPDATE_RSP       0x2
#define    MSG_OTA_TWS_TRANS_UPDATE_DATA      0x3
#define    MSG_OTA_TWS_TRANS_UPDATE_DATA_RSP  0x4
#define    MSG_OTA_TWS_VERIFY                 0x5
#define    MSG_OTA_TWS_VERIFY_RSP             0x6


int tws_ota_init(void);
int tws_ota_close(void);

int tws_ota_open(struct __tws_ota_para *para);
void tws_ota_stop(u8 reason);

u16 tws_ota_enter_verify(void *priv);
u16 tws_ota_exit_verify(u8 *res, u8 *up_flg);
u16 tws_ota_updata_boot_info_over(void *priv);

int tws_ota_err_callback(u8 reason);

int tws_ota_data_send_m_to_s(u8 *buf, u16 len);
int tws_ota_sync_cmd(int reason);
void tws_ota_app_event_deal(u8 evevt);
u8 dual_bank_update_burn_boot_info_callback(u8 ret);
int bt_ota_event_handler(struct bt_event *bt);
void tws_ota_event_post(u32 type, u8 event);

u8 tws_ota_control(int type, ...);

void tws_ota_send_data_to_sibling(u8 opcode, u8 *data, u8 len);
int tws_ota_get_data_from_sibling(u8 opcode, u8 *data, u8 len);

int tws_ota_result(u8 err);
int tws_ota_data_send_pend(void);
#endif   //(OTA_TWS_SAME_TIME_ENABLE && RCSP_ADV_EN)

#endif   //_RCSP_ADV_TWS_OTA_H

