#include "includes.h"
#include "../gx_uart_upgrade.h"
#include "../../gx8002_npu_api.h"
#include "../gx_uart_upgrade_porting.h"
#include "btstack/avctp_user.h"
#include "classic/hci_lmp.h"
#include "gx_fifo.h"

#if GX8002_UPGRADE_SPP_TOGGLE

#define spp_upgrade_info 		 g_printf

//=============== SPP Porting ================//
int gx8002_ota_recv(unsigned char *data, int size);
static void gx8002_spp_data_handler(u8 packet_type, u16 ch, u8 *packet, u16 size)
{
    switch (packet_type) {
    case 1:
        /* cbuf_init(&spp_buf_hdl, spp_buf, sizeof(spp_buf)); */
        /* spp_timer = sys_timer_add(NULL, spp_send_data_timer, 50); */
        /* spp_printf("spp connect\n"); */
        spp_upgrade_info("gx8002 spp connect\n");
        break;
    case 2:
        spp_upgrade_info("gx8002 spp disconnect\n");
        /* if (spp_timer) { */
        /* sys_timer_del(spp_timer); */
        /* spp_timer = 0; */
        /* } */
        break;
    case 7:
        //spp_upgrade_info("gx8002 spp_rx: %d", size);
        //put_buf(packet,size);
        gx8002_ota_recv(packet, size);
        break;
    }
}

static void gx8002_spp_send_data(u8 *data, u32 size)
{
    user_send_cmd_prepare(USER_CTRL_SPP_SEND_DATA, size, data);
}


void gx8002_spp_ota_init(void)
{
    spp_data_deal_handle_register(gx8002_spp_data_handler);
}

////////////// FW Stream Porting //////////////////
#define STREAM_FIFO_BLOCK_TOTAL    16
#define STREAM_FIFO_TIMEOUT_MS    (5000 / 10)

/* typedef int (*reply)(unsigned char *data, int size); */
static int ota_start = 0;
//static os_sem_t ota_sem;
static fifo_handle_t fw_fifo;

static int gx8002_ota_start(void)
{
    if (!ota_start) {
        os_taskq_post_msg("gx8002", 2, GX8002_MSG_UPDATE, GX8002_UPDATE_TYPE_SPP);
    }

    return 0;
}

int get_gx8002_ota_start(void)
{
    return ota_start;
}

int gx8002_ota_recv(unsigned char *data, int size)
{
    int i;

    if ((data == NULL) || (size == 0)) {
        return -1;
    }

    if (strncmp((char *)data, "startupgrade", strlen("startupgrade")) == 0) {
        gx8002_ota_start();
    } else {
        if (ota_start) {
            if (fw_fifo > 0) {
                int cnt = size / UPGRADE_PACKET_SIZE;
                int i;

                for (i = 0; i < cnt; i++) {
                    gx_fifo_write(fw_fifo, (const char *)&data[UPGRADE_PACKET_SIZE * i], UPGRADE_PACKET_SIZE);
                }

                if (size % UPGRADE_PACKET_SIZE) {
                    gx_fifo_write(fw_fifo, (const char *)&data[UPGRADE_PACKET_SIZE * i], size % UPGRADE_PACKET_SIZE);
                }
            }
        }
    }

    return 0;
}


static int fw_stream_open(FW_IMAGE_TYPE img_type)
{
    if (fw_fifo == 0) {
        if ((fw_fifo = gx_fifo_create(UPGRADE_PACKET_SIZE, STREAM_FIFO_BLOCK_TOTAL)) == 0) {
            return -1;
        }
    }

    return 0;
}

static int fw_stream_close(FW_IMAGE_TYPE img_type)
{
    int ret = 0;

    if (fw_fifo > 0) {
        ret = gx_fifo_destroy(fw_fifo);
        fw_fifo = 0;
    }

    return ret;
}

static int fw_stream_read(FW_IMAGE_TYPE img_type, unsigned char *buf, unsigned int len)
{
    int ret = 0;

    if (fw_fifo > 0) {
        ret = gx_fifo_read(fw_fifo, (char *)buf, len, STREAM_FIFO_TIMEOUT_MS);
    }

    return ret;
}

static int fw_stream_get_flash_img_info(flash_img_info_t *info)
{
    unsigned int size = 0;

    if (info == NULL) {
        return -1;
    }

    if (fw_fifo == 0) {
        return -1;
    }

    int ret = gx_fifo_read(fw_fifo, (char *)info, sizeof(flash_img_info_t), STREAM_FIFO_TIMEOUT_MS);

    return 0;
}

///////// status info /////////////
static const char *status_info_str[UPGRADE_STAGE_NONE][UPGRADE_STATUS_NONE] = {
    // UPGRADE_STAGE_HANDSHAKE
    {"handshake start\n", NULL, "handshake ok\n", "handshake err\n"},
    // UPGRADE_STAGE_BOOT_HEADER
    {"boot header start\n", NULL, "boot header ok\n", "boot header err\n"},
    // UPGRADE_STAGE_BOOT_S1
    {"boot stage1 start\n", "boot stage1 downloading\n", "boot stage1 ok\n", "boot stage1 err\n"},
    // UPGRADE_STAGE_BOOT_S2
    {"boot stage2 start\n", "boot stage2 downloading\n", "boot stage2 ok\n", "boot stage2 err\n"},
    // UPGRADE_STAGE_FLASH_IMG
    {"flash img start\n", "flash img downloading\n", "flash img ok\n", "flash img err\n"},
};

static void gx8002_upgrade_status_cb(upgrade_stage_e stage, upgrade_status_e status)
{
    //printf("---- [%s]: stage: %d, status: %d ----\n", __func__, stage, status);
    const char *reply_str = status_info_str[stage][status];
    if (reply_str != NULL) {
        spp_upgrade_info("status info: %s\n", reply_str);
        //user_spp_send_data((unsigned char *)reply_str, strlen(reply_str));
        gx8002_spp_send_data((u8 *)reply_str, strlen(reply_str));
    }
}



static int gx8002_upgrade_task(void *param)
{
    int ret = 0;

    spp_upgrade_info("---- gx8002 ota start ----");
    gx8002_upgrade_cold_reset();

    ret = gx_uart_upgrade_proc();

    gx8002_normal_cold_reset();
    spp_upgrade_info("---- gx8002 ota over ----");

    ota_start = 0;

    if (ret < 0) {
        gx8002_update_end_post_msg(0);
    } else {
        gx8002_update_end_post_msg(1);
    }

    return ret;
}


int gx8002_uart_spp_ota_init(void)
{
    upgrade_uart_t uart_ops;
    fw_stream_t fw_stream_ops;

    uart_ops.open = gx_upgrade_uart_porting_open;
    uart_ops.close = gx_upgrade_uart_porting_close;
    uart_ops.send = gx_upgrade_uart_porting_write;
    uart_ops.wait_reply = gx_uart_upgrade_porting_wait_reply;


    fw_stream_ops.open = fw_stream_open;
    fw_stream_ops.close = fw_stream_close;
    fw_stream_ops.read = fw_stream_read;
    fw_stream_ops.get_flash_img_info = fw_stream_get_flash_img_info;

    gx_uart_upgrade_init(&uart_ops, &fw_stream_ops, gx8002_upgrade_status_cb);

    ota_start = 1;

    gx8002_upgrade_task(NULL);

    return 0;
}

#else /* #if GX8002_UPGRADE_SPP_TOGGLE */

int gx8002_uart_spp_ota_init(void)
{
    return 0;
}


int get_gx8002_ota_start(void)
{
    return 0;
}

int gx8002_ota_recv(unsigned char *data, int size)
{
    return 0;
}

#endif /* #if GX8002_UPGRADE_SPP_TOGGLE */
