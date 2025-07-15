
#include "system/includes.h"
#include "string.h"
#include "circular_buf.h"
#include "btstack/avctp_user.h"
#include "app_config.h"
#include "bt_tws.h"
#include "bt_common.h"
#include "syscfg_id.h"
#include "pbg_user.h"
#include "aec_user.h"
#include "tone_player.h"

#define PBG_DEMO_EN          0//enalbe pbg_demo

#if (PBG_DEMO_EN&&TCFG_USER_TWS_ENABLE)

#define log_info(x, ...)  printf("\n[###pbg_demo@@@]" x " ", ## __VA_ARGS__)

typedef struct {
    // linked list - assert: first field
    void *offset_item;

    // data is contained in same memory
    u32        service_record_handle;
    u8         *service_record;
} service_record_item_t;


#define SDP_RECORD_HANDLER_REGISTER(handler) \
	const service_record_item_t  handler \
	sec(.sdp_record_item)

//---------
enum {
    PBG_EVENT_CONNECT = 1,
    PBG_EVENT_DISCONNECT,
    PBG_EVENT_MONITOR_START,//监听开始
    PBG_EVENT_PACKET_HANDLER = 7,
};

enum {
    PBG_USER_ST_NULL = 0x0,
    PBG_USER_ST_CONNECT,
    PBG_USER_ST_DISCONN,
};

enum {
    PBG_USER_ERR_NONE = 0x0,
    PBG_USER_ERR_SEND_BUFF_BUSY,
    PBG_USER_ERR_SEND_OVER_LIMIT,
    PBG_USER_ERR_SEND_FAIL,
};

//--------------------------------------------------------------------------------------------------
//---------
u8 pbg_profile_support = 1; //enable libs profile success
//---------

#define USER_SEND_POOL_SIZE (256L)
static u8 user_send_pool[USER_SEND_POOL_SIZE];
static u8 user_send_busy;
static u8 user_state;

u32 pbg_user_send(void *priv, u8 *buf, u32 len);
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//示例添加SDP服务表，注意不要跟bt_profile_config.c的定义有重复和冲突！！！！！！

//标准sdp结构表,以PNP服务示例定义，表末端增加 0x00,0x00 为结束符
/* static const u8 sdp_pnp_service_data[] = { */
/* 0x36, 0x00, 0x34, 0x09, 0x00, 0x00, 0x0A, 0x50, 0x01, 0x10, 0x00, 0x09, 0x00, 0x01, 0x36, 0x00, */
/* 0x03, 0x19, 0x12, 0x00, 0x09, 0x02, 0x00, 0x09, 0x01, 0x03, 0x09, 0x02, 0x01, 0x09, 0x05, 0xD6, */
/* 0x09, 0x02, 0x02, 0x09, 0x00, 0x0A, 0x09, 0x02, 0x03, 0x09, 0x02, 0x40, 0x09, 0x02, 0x04, 0x28, */
/* 0x01, 0x09, 0x02, 0x05, 0x09, 0x00, 0x01, 0x00, 0x00 */
/* }; */

/* SDP_RECORD_HANDLER_REGISTER(pnp_sdp_record_item) = { */
/* .service_record = (u8 *)sdp_pnp_service_data, */
/* .service_record_handle = 0x50011000, */
/* }; */

//--------------------------------------------------------------------------------------------------

//发送数据接口，注意返回值;确定OK
//数据真正发送成功，以接口user_pbg_send_ok_callback 回调为准
u32 pbg_user_send(void *priv, u8 *buf, u32 len)
{
    if (user_state != PBG_USER_ST_CONNECT) {
        return PBG_USER_ERR_SEND_FAIL;
    }

    log_info("send_data(%d)\n", len);
    put_buf(buf, len);

    if (user_send_busy == 1) {
        log_info("ERR_SEND_BUFF_BUSY\n");
        return PBG_USER_ERR_SEND_BUFF_BUSY;
    }

    if (user_send_pool == NULL) {
        return PBG_USER_ERR_SEND_FAIL;
    }

    if (len) {
        if (len > USER_SEND_POOL_SIZE) {
            log_info("ERR_SEND_OVER_LIMIT\n");
            return PBG_USER_ERR_SEND_OVER_LIMIT;
        }

        user_send_busy = 1;
        memcpy(user_send_pool, buf, len);
        u32 ret = user_send_cmd_prepare(USER_CTRL_PBG_SEND_DATA, len, user_send_pool);
        if (ret) {
            user_send_busy = 0;
            return PBG_USER_ERR_SEND_FAIL;
        }
    }
    return PBG_USER_ERR_NONE;
}

void user_pbg_send_ok_callback(int err_code)
{
    user_send_busy = 0;
    log_info("send_ok\n");
}

static void user_pbg_packet_handler(u8 packet_type, u16 ch, u8 *packet, u16 size)
{
    u32 tmp32;

    switch (packet_type) {
    case PBG_EVENT_PACKET_HANDLER:
        log_info("pbg packet_data(%d):");
        put_buf(packet, size);
        /* test_pbg_cmd_send(packet);//for test */
        break;

    case PBG_EVENT_CONNECT:
        user_state = PBG_USER_ST_CONNECT;
        log_info("pbg connect #############\n");
        break;

    case PBG_EVENT_DISCONNECT:
        user_state = PBG_USER_ST_DISCONN;
        log_info("pbg disconnect #############\n");
        break;

    case PBG_EVENT_MONITOR_START:
        log_info("pbg monitor start #############\n");
        break;

    default:
        break;
    }

}

#define PSM_BrowseGroupDescriptor      (0x1001)
extern void pbg_profile_init(u16 psm);
extern void pbg_event_handler_register(void (*handler)(u8 packet_type, u16 channel, u8 *packet, u16 size));
void pbg_demo_init(void)
{
    log_info("pbg_demo_init\n");
    pbg_profile_init(PSM_BrowseGroupDescriptor);//input PSM ID
    pbg_event_handler_register(user_pbg_packet_handler);
    user_state = PBG_USER_ST_DISCONN;
    user_send_busy = 0;
}

#else

void pbg_demo_init(void)
{
    /* printf("pbg not enable...\n"); */
}

void pbg_user_battery_level_sync(u8 *dev_bat)
{
    //add code
}

void pbg_user_ear_pos_sync(u8 left, u8 right)
{
    //add code
}

bool pbg_user_key_vaild(u8 *key_msg, struct sys_event *event)
{
    //add code
    return false;
}

void pbg_user_event_deal(struct pbg_event *evt)
{
    //add code
}

void pbg_user_set_tws_state(u8 conn_flag)
{
    //add code
}

void pbg_user_mic_fixed_deal(u8 mode)
{
    //add code
}

void pbg_user_recieve_sync_info(u8 *sync_info)
{
    //add code
}

int pbg_user_is_connected(void)
{
    //add code
    //return PBG服务是否已连上，区分安卓和苹果,是否持续广播
    return 1;//不支持PBG服务,return 1
}

//----------------------------------------

#endif





