#ifndef __GX_UPGRADE_DEF_H__
#define __GX_UPGRADE_DEF_H__

#define UPGRADE_DATA_BLOCK_SIZE    256

#define UPGRADE_PACKET_SIZE    256
#define UPGRADE_BLOCK_SIZE    (1024 * 4)
#define UPGRADE_FLASH_BLOCK_SIZE    (1024 * 56) //56K

typedef enum {
    UPGRADE_STAGE_HANDSHAKE = 0,
    UPGRADE_STAGE_BOOT_HEADER,
    UPGRADE_STAGE_BOOT_S1,
    UPGRADE_STAGE_BOOT_S2,
    UPGRADE_STAGE_FLASH_IMG,
    UPGRADE_STAGE_NONE
} upgrade_stage_e;

typedef enum {
    UPGRADE_STATUS_START = 0,
    UPGRADE_STATUS_DOWNLOADING,
    UPGRADE_STATUS_OK,
    UPGRADE_STATUS_ERR,
    UPGRADE_STATUS_NONE
} upgrade_status_e;

typedef void (*upgrade_status_cb)(upgrade_stage_e stage, upgrade_status_e status);

typedef enum {
    FW_BOOT_IMAGE = 0, //"gx8002.boot"
    FW_FLASH_IMAGE,    //"mcu_nor.bin"

    FW_FLASH_MAX,
} FW_IMAGE_TYPE;

typedef struct {
    unsigned int img_size;
} flash_img_info_t;

typedef struct {
    int (*open)(FW_IMAGE_TYPE img_type);
    int (*close)(FW_IMAGE_TYPE img_type);
    int (*read)(FW_IMAGE_TYPE img_type, unsigned char *buf, unsigned int len);
    int (*get_flash_img_info)(flash_img_info_t *info);
} fw_stream_t;

#endif
