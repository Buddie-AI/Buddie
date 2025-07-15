#include "includes.h"
#include "gx_uart_upgrade.h"
#include "../gx8002_npu_api.h"

#if GX8002_UPGRADE_TOGGLE

#define GX_UPGRADE_DEBUG  		0
#if GX_UPGRADE_DEBUG
//#define upgrade_debug(fmt, ...) y_printf("[gx_uart_upgrade]: " fmt, ##__VA_ARGS__)
#define upgrade_debug 			printf
#else
#define upgrade_debug(fmt, ...)
#endif

#define HANDSHAKE_BAUDRATE    576000
#define UART_REPLY_TIMEOUT_MS    5000

static upgrade_uart_t uart_ops;
static fw_stream_t fw_stream_ops;
static upgrade_status_cb status_callback;

typedef struct {
    unsigned short chip_id;
    unsigned char  chip_type;
    unsigned char  chip_version;

    unsigned short boot_delay;
    unsigned char  baud_rate;
    unsigned char  reserved_1;

    unsigned int   stage1_size;
    unsigned int   stage2_baud_rate;
    unsigned int   stage2_size;
    unsigned int   stage2_checksum;
    unsigned char  reserved[8];
} __attribute__((packed)) boot_header_t;

static boot_header_t boot_header;
static unsigned char data_buf[UPGRADE_PACKET_SIZE];
static upgrade_stage_e current_stage;
static int upgrade_initialized = 0;

static inline void set_upgrade_stage(upgrade_stage_e stage)
{
    current_stage = stage;
}

static inline upgrade_stage_e get_upgrade_stage(void)
{
    return current_stage;
}

static void status_report(upgrade_status_e status)
{
    if (status_callback) {
        status_callback(get_upgrade_stage(), status);
    }
}

static int upgrade_handshake(unsigned int timeout_ms, unsigned int retry_times)
{
    int ret = -1;

    upgrade_debug("start upgrade_handshake, timeout_ms: %d, retry_times: %d\n", timeout_ms, retry_times);

    set_upgrade_stage(UPGRADE_STAGE_HANDSHAKE);
    status_report(UPGRADE_STATUS_START);

    upgrade_debug("waiting 'M' ...\n");
    while (retry_times--) {
        upgrade_debug("handshake retry_times: %d", retry_times);
        unsigned char c = 0xef;
        uart_ops.send(&c, 1);
        if (!uart_ops.wait_reply((const unsigned char *)"M", 1, timeout_ms)) {
            ret = 0;
            break;
        }
    }

    if (ret == 0) {
        upgrade_debug("get 'M' !\n");
        status_report(UPGRADE_STATUS_OK);
    } else {
        upgrade_debug("wait 'M' err !\n");
        status_report(UPGRADE_STATUS_ERR);
    }

    upgrade_debug("upgrade_handshake over, ret: %d\n ", ret);

    return ret;
}

static int is_little_endian(void)
{
    int a = 0x11223344;
    unsigned char *p = (unsigned char *)&a;

    return (*p == 0x44) ? 1 : 0;
}

static unsigned int switch_endian(unsigned int v)
{
    return (((v >> 0) & 0xff) << 24) | (((v >> 8) & 0xff) << 16) | (((v >> 16) & 0xff) << 8) | (((v >> 24) & 0xff) << 0);
}

static unsigned int be32_to_cpu(unsigned int v)
{
    if (is_little_endian()) {
        return switch_endian(v);
    } else {
        return v;
    }
}

static int boot_info_baudrate[] = {300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
static void print_bootheader(const boot_header_t *header)
{
    upgrade_debug("chip_id %x\n", header->chip_id);
    upgrade_debug("chip_type %x\n", header->chip_type);
    upgrade_debug("chip_version %x\n", header->chip_version);
    upgrade_debug("boot_delay %x\n", header->boot_delay);
    upgrade_debug("baud_rate %d\n", boot_info_baudrate[header->baud_rate]);
    upgrade_debug("stage1_size %d\n", header->stage1_size);
    upgrade_debug("stage2_baud_rate %d\n", header->stage2_baud_rate);
    upgrade_debug("stage2_size %d\n", header->stage2_size);
    upgrade_debug("stage2_checksum %d\n", header->stage2_checksum);
}

int parse_bootimg_header(void)
{
    upgrade_debug("reading boot header ...\n");

    set_upgrade_stage(UPGRADE_STAGE_BOOT_HEADER);
    status_report(UPGRADE_STATUS_START);

    int ret = fw_stream_ops.read(FW_BOOT_IMAGE, (unsigned char *)&boot_header, sizeof(boot_header));
    if (ret < sizeof(boot_header)) {
        upgrade_debug("read boot header err !\n");
        goto header_err;
    }

    boot_header.stage1_size  = be32_to_cpu(boot_header.stage1_size);
    boot_header.stage2_baud_rate = be32_to_cpu(boot_header.stage2_baud_rate);
    boot_header.stage2_size  = be32_to_cpu(boot_header.stage2_size);
    boot_header.stage2_checksum = be32_to_cpu(boot_header.stage2_checksum);

    print_bootheader(&boot_header);
    status_report(UPGRADE_STATUS_OK);
    return 0;

header_err:
    status_report(UPGRADE_STATUS_ERR);
    return -1;
}

int download_bootimg_stage1(void)
{
    int wsize = 0;
    int len = 0;
    int size = 0;

    upgrade_debug("start boot stage1 ...\n");

    set_upgrade_stage(UPGRADE_STAGE_BOOT_S1);
    status_report(UPGRADE_STATUS_START);

    size = boot_header.stage1_size;
    size = size / 4;
    data_buf[0] = 'Y';
    data_buf[1] = (size >> 0) & 0xFF;
    data_buf[2] = (size >> 8) & 0xFF;
    data_buf[3] = (size >> 16) & 0xFF;
    data_buf[4] = (size >> 24) & 0xFF;
    uart_ops.send(data_buf, 5);

    if (boot_header.chip_type == 0x01) {
        size = size * 4;
    }

    upgrade_debug("download boot stage1 ...\n");
    while (wsize < size) {
        if (wsize % UPGRADE_BLOCK_SIZE == 0) {
            status_report(UPGRADE_STATUS_DOWNLOADING);
        }
        len = (boot_header.stage1_size - wsize >= UPGRADE_PACKET_SIZE) ? UPGRADE_PACKET_SIZE : (boot_header.stage1_size - wsize);
        len = fw_stream_ops.read(FW_BOOT_IMAGE, data_buf, len);
        if (len <= 0) {
            upgrade_debug("read data err !\n");
            goto stage1_err;
        }
        len = uart_ops.send(data_buf, len);
        if (len <= 0) {
            upgrade_debug("send data err !\n");
            goto stage1_err;
        }

        wsize += len;
    }

    upgrade_debug("download size: %d, waiting 'F' ...\n", wsize);
    if (uart_ops.wait_reply((const unsigned char *)"F", 1, UART_REPLY_TIMEOUT_MS)) {
        upgrade_debug("wait 'F' err !\n");
        goto stage1_err;
    }
    upgrade_debug("get 'F' !\n");

    ///// stage1 running, change baudrate /////
    upgrade_debug("change baudrate: %d\n", boot_header.stage2_baud_rate);

    uart_ops.close();
    uart_ops.open(boot_header.stage2_baud_rate, 8, 1, 0);

    upgrade_debug("wait \"GET\" ...\n");
    if (uart_ops.wait_reply((const unsigned char *)"GET", 3, UART_REPLY_TIMEOUT_MS)) {
        upgrade_debug("wait 'GET' err !\n");
        goto stage1_err;
    }
    upgrade_debug("get \"GET\" !\n");

    data_buf[0] = 'O';
    data_buf[1] = 'K';
    uart_ops.send(data_buf, 2);

    status_report(UPGRADE_STATUS_OK);
    upgrade_debug("boot stage1 ok !\n");

    return 0;

stage1_err:
    status_report(UPGRADE_STATUS_ERR);
    upgrade_debug("boot stage1 err !\n");

    return -1;
}

int download_bootimg_stage2(void)
{
    unsigned int checksum = 0;
    unsigned int stage2_size = 0;
    int len = 0;
    int wsize = 0;

    upgrade_debug("start boot stage2 ...\n");

    set_upgrade_stage(UPGRADE_STAGE_BOOT_S2);
    status_report(UPGRADE_STATUS_START);

    stage2_size = boot_header.stage2_size;
    checksum = boot_header.stage2_checksum;
    upgrade_debug("stage2_size = %u, checksum = %u\n", stage2_size, checksum);

    if (checksum  == 0 || stage2_size == 0) {
        upgrade_debug("stage2_size or checksum err !\n");
        goto stage2_err;
    }

    data_buf[0] = 'S';
    uart_ops.send(data_buf, 1);
    uart_ops.send((const unsigned char *)&checksum, 4);
    uart_ops.send((const unsigned char *)&stage2_size, 4);

    upgrade_debug("waiting \"ready\" ...\n");
    if (uart_ops.wait_reply((const unsigned char *)"ready", 5, UART_REPLY_TIMEOUT_MS)) {
        upgrade_debug("wait ready error !\n");
        goto stage2_err;
    }
    upgrade_debug("get \"ready\"\n");

    upgrade_debug("download boot stage2 ...\n");
    while (wsize < stage2_size) {
        if (wsize % UPGRADE_BLOCK_SIZE == 0) {
            status_report(UPGRADE_STATUS_DOWNLOADING);
        }
        len = (stage2_size - wsize >= UPGRADE_PACKET_SIZE) ? UPGRADE_PACKET_SIZE : (stage2_size - wsize);
        len = fw_stream_ops.read(FW_BOOT_IMAGE, data_buf, len);
        if (len <= 0) {
            upgrade_debug("read data err !\n");
            goto stage2_err;
        }

        len = uart_ops.send(data_buf, len);
        if (len <= 0) {
            upgrade_debug("send data err !\n");
            goto stage2_err;
        }

        wsize += len;
    }

    upgrade_debug("download size: %d, waiting 'O' ...\n", wsize);
    if (uart_ops.wait_reply((const unsigned char *)"O", 1, UART_REPLY_TIMEOUT_MS)) {
        upgrade_debug("wait 'O' err !\n");
        goto stage2_err;
    }
    upgrade_debug("get 'O' !\n");

    upgrade_debug("waiting \"boot>\" ...\n");

    if (uart_ops.wait_reply((const unsigned char *)"boot>", 5, UART_REPLY_TIMEOUT_MS)) {
        upgrade_debug("wait \"boot>\" err !\n");
        goto stage2_err;
    }
    upgrade_debug("get \"boot>\" !\n");

    status_report(UPGRADE_STATUS_OK);
    upgrade_debug("boot stage2 ok !\n");

    return 0;

stage2_err:
    status_report(UPGRADE_STATUS_ERR);
    upgrade_debug("boot stage2 err !\n");

    return -1;
}

static int download_bootimg(void)
{
    if (parse_bootimg_header() != 0) {
        return -1;
    }

    if (download_bootimg_stage1() != 0) {
        return -1;
    }

    if (download_bootimg_stage2() != 0) {
        return -1;
    }

    return 0;
}

static int download_flashimg(void)
{
    int wsize = 0;
    int len = 0;
    flash_img_info_t info = {0};

    upgrade_debug("start flash image ...\n");

    set_upgrade_stage(UPGRADE_STAGE_FLASH_IMG);
    status_report(UPGRADE_STATUS_START);
    fw_stream_ops.get_flash_img_info(&info);
    upgrade_debug("flash image size = %d\n", info.img_size);

    if (info.img_size == 0) {
        upgrade_debug("flash image size err !\n");
        goto flash_err;
    }

    memset(data_buf, 0, UPGRADE_PACKET_SIZE);
    len = sprintf((char *)data_buf, "serialdown %d %d %d\n", 0, info.img_size, UPGRADE_FLASH_BLOCK_SIZE);
    uart_ops.send(data_buf, len);

    upgrade_debug("waiting \"~sta~\" ...\n");
    if (uart_ops.wait_reply((const unsigned char *)"~sta~", 5, UART_REPLY_TIMEOUT_MS)) {
        upgrade_debug("wait ~sta~ err !\n");
        goto flash_err;
    }
    upgrade_debug("get \"~sta~\" !\n");

    upgrade_debug("download flash image ...\n");

    while (wsize < info.img_size) {
        if (wsize % UPGRADE_BLOCK_SIZE == 0) {
            status_report(UPGRADE_STATUS_DOWNLOADING);
        }

        len = (info.img_size - wsize >= UPGRADE_PACKET_SIZE) ? UPGRADE_PACKET_SIZE : (info.img_size - wsize);
        len = fw_stream_ops.read(FW_FLASH_IMAGE, data_buf, len);
        if (len <= 0) {
            upgrade_debug("read data err !\n");
            goto flash_err;
        }

        len = uart_ops.send(data_buf, len);
        if (len <= 0) {
            upgrade_debug("send data err !\n");
            goto flash_err;
        }
        wsize += len;

        if ((wsize % UPGRADE_FLASH_BLOCK_SIZE) == 0) {
            upgrade_debug("waiting \"~sta~\" ...\n");
            if (uart_ops.wait_reply((const unsigned char *)"~sta~", 5, UART_REPLY_TIMEOUT_MS)) {
                upgrade_debug("wait \"~sta~\" err !\n");
                goto flash_err;
            }
            upgrade_debug("get \"~sta~\" !\n");
        }
    }

    upgrade_debug("download size: %d, waiting \"~fin~\" ...\n", wsize);
    if (uart_ops.wait_reply((const unsigned char *)"~fin~", 5, UART_REPLY_TIMEOUT_MS)) {
        upgrade_debug("wait \"~fin~\" err !\n");
        goto flash_err;
    }
    upgrade_debug("get \"~fin~\" !\n");

    status_report(UPGRADE_STATUS_OK);
    upgrade_debug("flash image ok !\n");
    return 0;

flash_err:
    status_report(UPGRADE_STATUS_ERR);
    upgrade_debug("flash image err !\n");
    return -1;
}

int gx_uart_upgrade_proc(void)
{
    int ret = -1;

    if (!upgrade_initialized) {
        return -1;
    }

    set_upgrade_stage(UPGRADE_STAGE_NONE);

    if (uart_ops.open(HANDSHAKE_BAUDRATE, 8, 1, 0) < 0) {
        upgrade_debug("open uart err !\n");
        goto upgrade_done;
    }

    if (fw_stream_ops.open(FW_BOOT_IMAGE) < 0) {
        upgrade_debug("open boot img stream err !\n");
        goto upgrade_done;
    }

    if (fw_stream_ops.open(FW_FLASH_IMAGE) < 0) {
        upgrade_debug("open flash img stream err !\n");
        goto upgrade_done;
    }

    /* JL_PORTA->DIR &= ~BIT(4); */
    /* JL_PORTA->OUT |= BIT(4); */
    if (upgrade_handshake(10, 100) < 0) {
        goto upgrade_done;
    }

    if (download_bootimg() < 0) {
        goto upgrade_done;
    }

    if (download_flashimg() < 0) {
        goto upgrade_done;
    }

    ret = 0;

upgrade_done:
    uart_ops.close();
    fw_stream_ops.close(FW_BOOT_IMAGE);
    fw_stream_ops.close(FW_FLASH_IMAGE);
    set_upgrade_stage(UPGRADE_STAGE_NONE);

    return ret;
}

int gx_uart_upgrade_init(upgrade_uart_t *uart, fw_stream_t *fw_stream, upgrade_status_cb status_cb)
{
    if ((uart == NULL) || (fw_stream == NULL)) {
        return -1;
    }

    memcpy(&uart_ops, uart, sizeof(upgrade_uart_t));
    memcpy(&fw_stream_ops, fw_stream, sizeof(fw_stream_t));

    status_callback = status_cb;

    upgrade_initialized = 1;
    return 0;
}

int gx_uart_upgrade_deinit(void)
{
    upgrade_initialized = 0;
    return 0;
}

#endif /* #if GX8002_UPGRADE_TOGGLE */
