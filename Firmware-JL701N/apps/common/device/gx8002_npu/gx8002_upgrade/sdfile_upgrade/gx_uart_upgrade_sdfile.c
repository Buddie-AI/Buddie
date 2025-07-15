#include "includes.h"
#include "../gx_uart_upgrade.h"
#include "../../gx8002_npu_api.h"
#include "../gx_uart_upgrade_porting.h"

#if GX8002_UPGRADE_SDFILE_TOGGLE

#define STREAM_FIFO_BLOCK_TOTAL    20
#define STREAM_FIFO_TIMEOUT_MS    7000

typedef int (*reply)(unsigned char *data, int size);
static int ota_start = 0;

struct sdfile_fifo {
    FILE *boot_fp;
    FILE *bin_fp;
};

static struct sdfile_fifo sdfile_upgrade = {0};
#define __this 			(&sdfile_upgrade)

#define GX8002_SDFILE_UPGRADE_DEBUG_ENABLE 			1

#define sdfile_upgrade_info 		 g_printf

#if GX8002_SDFILE_UPGRADE_DEBUG_ENABLE
#define sdfile_upgrade_debug 	r_printf
#define sdfile_upgrade_put_buf  put_buf
#else
#define sdfile_upgrade_debug(...)
#define sdfile_upgrade_put_buf(...)
#endif /* #if GX8002_DEBUG_ENABLE */

////////////// FW Stream Porting //////////////////
static int fw_stream_open(FW_IMAGE_TYPE img_type)
{
#define BOOT_IMAGE_PATH				SDFILE_RES_ROOT_PATH"gx8002.boot"
#define BIN_IMAGE_PATH				SDFILE_RES_ROOT_PATH"mcu_nor.bin"
    FILE *fp = NULL;
    if (img_type == FW_BOOT_IMAGE) {
        fp = fopen(BOOT_IMAGE_PATH, "r");
        if (fp) {
            sdfile_upgrade_info("open %s succ", BOOT_IMAGE_PATH);
            __this->boot_fp = fp;
        } else {
            sdfile_upgrade_info("open %s failed!", BOOT_IMAGE_PATH);
            return -1;
        }
    } else if (img_type == FW_FLASH_IMAGE) {
        fp = fopen(BIN_IMAGE_PATH, "r");
        if (fp) {
            sdfile_upgrade_info("open %s succ", BIN_IMAGE_PATH);
            __this->bin_fp = fp;
        } else {
            sdfile_upgrade_info("open %s failed!", BIN_IMAGE_PATH);
            return -1;
        }
    }

    return 0;
}

static int fw_stream_close(FW_IMAGE_TYPE img_type)
{
    int ret = 0;

    if (img_type == FW_BOOT_IMAGE) {
        if (__this->boot_fp) {
            fclose(__this->boot_fp);
            __this->boot_fp = NULL;
        }
    } else if (img_type == FW_FLASH_IMAGE) {
        if (__this->bin_fp) {
            fclose(__this->bin_fp);
            __this->bin_fp = NULL;
        }
    }

    return ret;
}

static int fw_stream_read(FW_IMAGE_TYPE img_type, unsigned char *buf, unsigned int len)
{
    int ret = 0;
    FILE *fp = NULL;

    if (img_type == FW_BOOT_IMAGE) {
        fp = __this->boot_fp;
    } else if (img_type == FW_FLASH_IMAGE) {
        fp = __this->bin_fp;
    }

    if (fp) {
        ret = fread(fp, buf, len);
    }

    return ret;
}

static int fw_stream_get_flash_img_info(flash_img_info_t *info)
{
    int ret = 0;

    if (info == NULL) {
        return -1;
    }

    if (__this->bin_fp) {
        info->img_size = flen(__this->bin_fp);
    }

    return ret;
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
    const char *reply_str = status_info_str[stage][status];
    if (reply_str != NULL) {
        sdfile_upgrade_info("status info: %s\n", reply_str);
    }
}

static int gx8002_upgrade_task(void *param)
{
    int ret = 0;

    sdfile_upgrade_info("---- gx8002 ota start ----");
    gx8002_upgrade_cold_reset();

    ret = gx_uart_upgrade_proc();

    gx8002_normal_cold_reset();
    sdfile_upgrade_info("---- gx8002 ota over ----");

    if (ret < 0) {
        gx8002_update_end_post_msg(0);
    } else {
        gx8002_update_end_post_msg(1);
    }

    return ret;
}


int gx8002_uart_sdfile_ota_init(void)
{
    upgrade_uart_t uart_ops;

    uart_ops.open = gx_upgrade_uart_porting_open;
    uart_ops.close = gx_upgrade_uart_porting_close;
    uart_ops.send = gx_upgrade_uart_porting_write;
    uart_ops.wait_reply = gx_uart_upgrade_porting_wait_reply;

    fw_stream_t fw_stream_ops;
    fw_stream_ops.open = fw_stream_open;
    fw_stream_ops.close = fw_stream_close;
    fw_stream_ops.read = fw_stream_read;
    fw_stream_ops.get_flash_img_info = fw_stream_get_flash_img_info;

    gx_uart_upgrade_init(&uart_ops, &fw_stream_ops, gx8002_upgrade_status_cb);

    gx8002_upgrade_task(NULL);

    return 0;
}

#else /* #if GX8002_UPGRADE_SDFILE_TOGGLE */

int gx8002_uart_sdfile_ota_init(void)
{
    return 0;
}

#endif /* #if GX8002_UPGRADE_SDFILE_TOGGLE */

