#include "custom_cfg.h"
#include "app_config.h"

#if RCSP_BTMATE_EN
#include "rcsp_user_update.h"
#include "rcsp_bluetooth.h"
#elif RCSP_ADV_EN
#include "rcsp_adv_user_update.h"
#include "rcsp_adv_bluetooth.h"
#elif SMART_BOX_EN
#include "smartbox_adv_bluetooth.h"
#endif

#if AI_APP_PROTOCOL
#include "app_protocol_api.h"
#endif

#if RCSP_ADV_EN || RCSP_BTMATE_EN || SMART_BOX_EN || APP_PROTOCOL_READ_CFG_EN
#include "app_config.h"
#include "fs.h"
//#include "crc_api.h"
//#include "syd_file.h"
#include "uart.h"
#include <string.h>

#if 0
#pragma code_seg(".custom_cfg_code")
#pragma bss_seg(".custom_cfg_bss")
#pragma const_seg(".custom_cfg_const")
#endif

#define CFG_DEBUG_TAG "-[CFG]:"
//#define RES_CUSTOM_CFG_FILE "/config.***"
#define RES_CUSTOM_CFG_FILE	SDFILE_RES_ROOT_PATH"config.dat"

//配置:VM的接口采用692X还是693X
#define VM_API_AC692X	0
#define VM_API_AC693X	1
#define USE_VM_API_SEL VM_API_AC693X

//配置:是否在总是擦除EXIF区域然后重写,还是只有在EXIF信息有变化时才擦除
#define ALWAYS_ERASE_EXIF_AREA		0
#define ONLY_DIFF_ERASE_EXIF_AREA	1
#define EXIF_ERASE_CONFIG			ALWAYS_ERASE_EXIF_AREA

//配置:文件操作接口是用692X还是693X
#define SDFILE_API_AC692X	0
#define SDFILE_API_AC693X	1
#define USE_FS_API_SEL		SDFILE_API_AC693X


#define CUSTOM_CFG_DEBUG_EN 1
#if CUSTOM_CFG_DEBUG_EN
#define cfg_puts puts
#define cfg_printf printf
#define cfg_printf_buf put_buf
#else
#define cfg_puts(...)
#define cfg_printf(...)
#define cfg_printf_buf(...)
#endif

typedef struct _exif_info_t {
    u32 addr;
    u32 len;
} exif_info_t;

static exif_info_t exif_info;

#define MEMBER_OFFSET_OF_STRUCT(type, member)       ((u32)&(((type *)0)->member))
#define MEMBER_SIZE_OF_STRUCT(type,member)			((u16)sizeof(((type *)0)->member))

typedef struct _adv_data_t {
    u16 crc;
    u16 len;
    u8 data[31];
} adv_data_cfg_t;

typedef struct _ble_name_t {
    u16 crc;
    u16 len;
    u8 data[31 - 2];
} ble_name_t;

typedef struct _bt_name_t {
    u16 crc;
    u16 len;
    u8 data[31];
} bt_name_t;

typedef struct _bt_pin_code_t {
    u16 crc;
    u16 len;
    char data[4];
} bt_pin_code_t;

typedef struct _ver_info_cfg_t {
    u16 crc;
    u16 len;
    update_file_id_t data;
} ver_info_cfg_t;

typedef struct _reset_io_info_cfg_t {
    u16 crc;
    u16 len;
    u8 data;
} reset_io_info_cfg_t;

typedef struct _polit_lamp_io_info_cfg_t {
    u16 crc;
    u16 len;
    u8 data[4];
} pilot_lamp_io_info_cfg_t;

typedef struct _link_key_info_cfg_t {
    u16 crc;
    u16 len;
    update_file_link_key_t data;
} link_key_info_cfg_t;

typedef struct _last_device_connect_linkkey_cfg_t {
    u16 crc;
    u16 len;
    u8 data[16];
} last_device_connect_linkkey_cfg_t;

typedef struct LOW_POWER_VOLTAGE {
    u16 crc;
    u16 len;
    u8 data[2];
} low_power_voltage_t;

typedef struct _bt_addr_cfg_t {
    u16 crc;
    u16 len;
    u8 data[6];
} bt_addr_cfg_t;

typedef struct _gatt_profile_cfg_t {
    u16 crc;
    u16 len;
    u8 data[512 + 256];
} gatt_profile_cfg_t;

typedef struct _power_io_on_off_cfg_t {
    u16 crc;
    u16 len;
    u8 data[6];
} power_io_on_off_cfg_t;

#if VER_INFO_EXT_COUNT
typedef struct _ver_info_ext_cfg_t {
    u16 crc;
    u16 len;
    u8 data[VER_INFO_EXT_MAX_LEN];
} ver_info_ext_cfg_t;
#endif

typedef struct _read_write_uuid_cfg_t {
    u16 crc;
    u16 len;
    u8 data[4];
} read_write_uuid_cfg_t;

typedef struct _common_cfg_t {
    u16 crc;
    u16 len;
    u8 data[0];
} common_cfg_t;

typedef struct _pid_vid_cfg_t {
    u16 crc;
    u16 len;
    u8 data[4];
} pid_vid_cfg_t;

typedef struct _authkey_cfg_t {
    u16 crc;
    u16 len;
    u8 data[32];
} authkey_cfg_t;

typedef struct _project_code_cfg_t {
    u16 crc;
    u16 len;
    u8 data[32];
} project_code_cfg_t;

typedef struct _md5_cfg_t {
    u16 crc;
    u16 len;
    u8 data[32];
} md5_cfg_t;

typedef struct _sdk_type_cfg_t {
    u16 crc;
    u16 len;
    u8 data;
} sdk_type_cfg_t;

typedef struct _ex_cfg_t {
    adv_data_cfg_t adv_data_cfg;
    adv_data_cfg_t scan_rsp_cfg;
    ble_name_t ble_name_cfg;
    bt_name_t bt_name_cfg;
    bt_pin_code_t pin_code_cfg;
    ver_info_cfg_t ver_info_cfg;
    low_power_voltage_t low_power_voltage_cfg;
    bt_addr_cfg_t edr_addr_cfg;
    bt_addr_cfg_t ble_addr_cfg;
    gatt_profile_cfg_t gatt_profile_cfg;
    reset_io_info_cfg_t reset_io_info_cfg;
    pilot_lamp_io_info_cfg_t pilot_lamp_io_info_cfg;
    link_key_info_cfg_t link_key_info_cfg;
    power_io_on_off_cfg_t power_io_on_off_cfg;
    last_device_connect_linkkey_cfg_t last_device_connect_linkkey_cfg;
    read_write_uuid_cfg_t ble_read_write_uuid_cfg;
#if VER_INFO_EXT_COUNT
    ver_info_ext_cfg_t ver_info_authkey_cfg;
    ver_info_ext_cfg_t ver_info_procode_cfg;
#endif
    pid_vid_cfg_t       pid_vid_cfg;
    md5_cfg_t           md5_cfg;
    sdk_type_cfg_t      sdk_type_cfg;
} ex_cfg_t;

typedef union _ex_cfg_item_u {
    adv_data_cfg_t adv_data_cfg;
    adv_data_cfg_t scan_rsp_cfg;
    ble_name_t ble_name_cfg;
    bt_name_t bt_name_cfg;
    bt_pin_code_t pin_code_cfg;
    ver_info_cfg_t ver_info_cfg;
    low_power_voltage_t low_power_voltage_cfg;
    bt_addr_cfg_t edr_addr_cfg;
    bt_addr_cfg_t ble_addr_cfg;
    gatt_profile_cfg_t gatt_profile_cfg;
    reset_io_info_cfg_t reset_io_info_cfg;
    pilot_lamp_io_info_cfg_t pilot_lamp_io_info_cfg;
    link_key_info_cfg_t link_key_info_cfg;
    power_io_on_off_cfg_t power_io_on_off_cfg;
    last_device_connect_linkkey_cfg_t last_device_connect_linkkey_cfg;
    read_write_uuid_cfg_t ble_read_write_uuid_cfg;
#if VER_INFO_EXT_COUNT
    ver_info_ext_cfg_t ver_info_authkey_cfg;
    ver_info_ext_cfg_t ver_info_procode_cfg;
#endif
    common_cfg_t common_cfg;
    pid_vid_cfg_t       pid_vid_cfg;
    md5_cfg_t           md5_cfg;
    sdk_type_cfg_t      sdk_type_cfg;
} ex_cfg_item_u;

typedef struct _cfg_item_head_t {
    u16 index;
    u16 type;
    u32 addr;
    u32 len;
    u16 crc;
    u16 rfu;
    u8 name[16];
} cfg_item_head_t;

typedef struct _cfg_head_t {
    u8 flag[4];
    u16 self_crc;
    u16 item_head_crc;
    u32 len;
    u16 item_count;
    u16 rfu;
    u8 name[16];
} cfg_head_t;

typedef enum _cfg_err_type_t {
    CFG_ERR_NONE = 0,
    CFG_ERR_FILE_INDEX_ERR,
    CFG_ERR_FILE_SEEK_ERR,
    CFG_ERR_FILE_READ_ERR,
    CFG_ERR_ITEM_LEN_OVER,
    CFG_ERR_ITEM_NO_FOUND,
} cfg_err_type_t;

typedef struct _cfg_item_description {
    u8 *item_name;
    u8 *item_data;
    u16 item_len;
    u16 *real_len;
} cfg_item_description_t;

typedef struct _update_file_id_cfg {
    u16 len;
    update_file_id_t data;
} update_file_id_cfg_t;

static update_file_id_cfg_t update_id_cfg = {
    .len = 0,
};

typedef struct _update_file_reset_io_cfg {
    u16 len;
    update_file_reset_io_t data;
} update_file_reset_io_cfg_t;

static update_file_reset_io_cfg_t update_reset_io_cfg = {
    .len = 0,
};

typedef struct _update_file_pilot_lamp_io_cfg {
    u16 len;
    update_file_pilot_lamp_io_t data;
} update_file_pilot_lamp_io_cfg_t;

static update_file_pilot_lamp_io_cfg_t update_pilot_lamp_io_cfg = {
    .len = 0,
};

typedef struct _update_file_link_key_cfg {
    u16 len;
    update_file_link_key_t data;
} update_file_link_key_cfg_t;

static update_file_link_key_cfg_t update_link_cfg = {
    .len = 0,
};

typedef struct _update_file_power_io_on_off_cfg {
    u16 len;
    u8 data[16];
} update_file_power_io_on_off_cfg_t;

static update_file_power_io_on_off_cfg_t update_power_io_on_off_cfg = {
    .len = 0,
};

#if VER_INFO_EXT_COUNT
typedef struct _update_file_ver_info_ext_cfg {
    u16 len;
    u8 data[VER_INFO_EXT_COUNT * (VER_INFO_EXT_MAX_LEN + 1)];       // authkey + , + procode + '\0'
} update_file_ver_info_ext_cfg_t;

static update_file_ver_info_ext_cfg_t update_file_ver_info_ext_cfg = {
    .len = 0,
};
#endif

static const cfg_item_description_t cfg_item_description[] = {
    [0] = {
        (u8 *)"ver_info", (u8 *) &update_id_cfg.data, sizeof(update_id_cfg.data), &update_id_cfg.len
    },
    [1] = {
        (u8 *)"reset_io", (u8 *) &update_reset_io_cfg.data, sizeof(update_reset_io_cfg.data), &update_reset_io_cfg.len
    },
    [2] = {
        (u8 *)"pilot_lamp_io", (u8 *) &update_pilot_lamp_io_cfg.data, sizeof(update_pilot_lamp_io_cfg.data), &update_pilot_lamp_io_cfg.len
    },
    [3] = {
        (u8 *)"link_key", (u8 *) &update_link_cfg.data, sizeof(update_link_cfg.data), &update_link_cfg.len
    },
    [4] = {
        (u8 *)"power_io", (u8 *) &update_power_io_on_off_cfg.data, sizeof(update_power_io_on_off_cfg.data), &update_power_io_on_off_cfg.len
    },
#if VER_INFO_EXT_COUNT
    [5] = {
        (u8 *)"ver_info_ext", (u8 *) &update_file_ver_info_ext_cfg.data, sizeof(update_file_ver_info_ext_cfg.data), &update_file_ver_info_ext_cfg.len
    },
#endif
};

typedef enum _FLASH_ERASER {
    CHIP_ERASER,
    BLOCK_ERASER,
    SECTOR_ERASER,
} FLASH_ERASER;
#if (USE_VM_API_SEL == VM_API_AC692X)
#include "flash_api.h"
#elif (USE_VM_API_SEL == VM_API_AC693X)
extern bool sfc_erase(FLASH_ERASER cmd, u32 addr);
extern u32 sfc_write(const u8 *buf, u32 addr, u32 len);
extern u32 sfc_read(u8 *buf, u32 addr, u32 len);
static void vm_read_by_addr(u8 *buf, u32 addr, u32 len)
{
    sfc_read(buf, addr, len);
}

static u32 vm_write_by_addr(u8 *buf, u32 addr, u32 len)
{
    return sfc_write(buf, addr, len);
}

#endif



extern u32 fs_open_file_bypath_offest(void *dev_hdl, void *buf, u16 len, char *path, u32 offset);

u32 get_cfg_data_connect_by_name(u8 *name, u8 *data, u16 len, u16 *real_len)
{
    cfg_item_head_t cfg_item_head;
    cfg_head_t cfg_head;
    u32 err = CFG_ERR_NONE;
    u32 r_len;
    u16 i;
    *real_len = 0;
    u32 offset = 0;

#if (USE_FS_API_SEL == SDFILE_API_AC692X)
    DEV_HANDLE dev = cache;
    u32 file_err;
    file_err = fs_open_file_bypath_offest(dev, &cfg_head, sizeof(cfg_head_t), RES_CUSTOM_CFG_FILE, offset);
    if (0 == file_err) {
        err = CFG_ERR_FILE_READ_ERR;
        goto _ERR_RET;
    }
#elif (USE_FS_API_SEL == SDFILE_API_AC693X)
    FILE *fp = NULL;
    fp = fopen(RES_CUSTOM_CFG_FILE, "r");
    if (!fp) {
        cfg_puts("file open fail");
        err = CFG_ERR_FILE_READ_ERR;
        goto _ERR_RET;
    }
    r_len = fread(fp, (void *)&cfg_head, sizeof(cfg_head_t));
    if (r_len != sizeof(cfg_head_t)) {
        cfg_puts("file read cfg head fail\n");
        err = CFG_ERR_FILE_READ_ERR;
        goto _ERR_RET;
    }
#endif

    offset += sizeof(cfg_head_t);
    cfg_printf("read file req_len:%x ret_len:%x\n", sizeof(cfg_head_t), cfg_head.len);
    cfg_puts("cfg_head:\n");
    cfg_printf_buf((u8 *)&cfg_head, sizeof(cfg_head_t));

    for (i = 0; i < cfg_head.item_count; i++) {
#if (USE_FS_API_SEL == SDFILE_API_AC692X)
        file_err = fs_open_file_bypath_offest(dev, &cfg_item_head, sizeof(cfg_item_head_t), RES_CUSTOM_CFG_FILE, offset);
#elif (USE_FS_API_SEL == SDFILE_API_AC693X)
        fseek(fp, SEEK_SET, offset);
        r_len = fread(fp, (void *)&cfg_item_head, sizeof(cfg_item_head_t));
        if (r_len != sizeof(cfg_item_head_t)) {
            cfg_puts("file read cfg item head fail\n");
            err = CFG_ERR_FILE_READ_ERR;
            goto _ERR_RET;
        }
#endif
        offset += sizeof(cfg_item_head_t);
        if (0 == memcmp(cfg_item_head.name, name, strlen((const char *)name))) {
            cfg_printf("find item %d name:%s\n", i, cfg_item_head.name);
            cfg_printf_buf((u8 *)&cfg_item_head, sizeof(cfg_item_head_t));
            offset = cfg_item_head.addr;
            if (len < cfg_item_head.len) {
                err = CFG_ERR_ITEM_LEN_OVER;
                break;
            }
            *real_len = cfg_item_head.len;
#if (USE_FS_API_SEL == SDFILE_API_AC692X)
            fs_open_file_bypath_offest(dev, data, cfg_item_head.len, RES_CUSTOM_CFG_FILE, offset);
#elif (USE_FS_API_SEL == SDFILE_API_AC693X)
            fseek(fp, offset, SEEK_SET);
            r_len = fread(fp, data, cfg_item_head.len);
            if (r_len != cfg_item_head.len) {
                cfg_puts("read cfg item content fail\n");
                err = CFG_ERR_FILE_READ_ERR;
                goto _ERR_RET;
            }
#endif
            break;
        }
    }

    if (i == cfg_head.item_count) {
        err = CFG_ERR_ITEM_NO_FOUND;
    }

_ERR_RET:
    return err;
}

void ex_cfg_read_flash_by_addr(u8 *buf, u32 addr, u32 len)
{
    vm_read_by_addr(buf, exif_info.addr + addr, len);
}

extern u16 CRC16(const void *ptr, u32 len);
static u16 crc16(u8 *buf, u16 len)
{
    return CRC16(buf, len);
}

typedef struct _cfg_item_attr_t {
    u16 item_len;
    u16 data_len;
    u16 member_offset;
} cfg_item_attr_t;

static const cfg_item_attr_t cfg_item_attr_tab[] = {
    [CFG_ITEM_ADV_IND] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, adv_data_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, adv_data_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, adv_data_cfg),
    },
    [CFG_ITEM_SCAN_RSP] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, scan_rsp_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, scan_rsp_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, scan_rsp_cfg),
    },
    [CFG_ITEM_BLE_NAME] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_name_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_name_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ble_name_cfg),
    },
    [CFG_ITEM_BT_NAME] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, bt_name_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, bt_name_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, bt_name_cfg),
    },
    [CFG_ITEM_PIN_CODE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pin_code_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pin_code_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, pin_code_cfg),
    },
    [CFG_ITEM_VER_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ver_info_cfg),
    },
    [CFG_ITEM_LOW_POWER_VOLTAGE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, low_power_voltage_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, low_power_voltage_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, low_power_voltage_cfg),
    },
    [CFG_ITEM_EDR_ADDR] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, edr_addr_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, edr_addr_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, edr_addr_cfg),
    },
    [CFG_ITEM_BLE_ADDR] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_addr_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_addr_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ble_addr_cfg),
    },
    [CFG_ITEM_GATT_PROFILE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, gatt_profile_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, gatt_profile_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, gatt_profile_cfg),
    },
    [CFG_ITEM_RESET_IO_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, reset_io_info_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, reset_io_info_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, reset_io_info_cfg),
    },
    [CFG_ITEM_PILOT_LAMP_IO_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pilot_lamp_io_info_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pilot_lamp_io_info_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, pilot_lamp_io_info_cfg),
    },
    [CFG_ITEM_LINK_KEY_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, link_key_info_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, link_key_info_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, link_key_info_cfg),
    },
    [CFG_ITEM_POWER_IO_ON_OFF] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, power_io_on_off_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, power_io_on_off_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, power_io_on_off_cfg),
    },
    [CFG_ITEM_LAST_DEVICE_LINK_KEY_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, last_device_connect_linkkey_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, last_device_connect_linkkey_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, last_device_connect_linkkey_cfg),
    },
    [CFG_ITEM_BLE_READ_WRITE_UUID_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_read_write_uuid_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_read_write_uuid_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ble_read_write_uuid_cfg),
    },
#if VER_INFO_EXT_COUNT
    [CFG_ITEM_VER_INFO_AUTHKEY] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_authkey_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_authkey_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ver_info_authkey_cfg),
    },
    [CFG_ITEM_VER_INFO_PROCODE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_procode_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_procode_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ver_info_procode_cfg),
    },
#endif
    [CFG_ITEM_PVID] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pid_vid_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pid_vid_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, pid_vid_cfg),
    },
    [CFG_ITEM_MD5] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, md5_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, md5_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, md5_cfg),
    },
    [CFG_ITEM_SDK_TYPE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, sdk_type_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, sdk_type_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, sdk_type_cfg),
    }
};

static u32 custom_cfg_item_change_check(u8 type, u8 *new_data, u16 new_data_len, u8 *write_flag)
{
    ex_cfg_item_u item_u;
    u16 item_len = 0;
    ex_cfg_t *p_ex_cfg = NULL;
    u32 item_offset = 0;
    u32 exif_addr = exif_info.addr;

    u32 err = 0;

    if (type > sizeof(cfg_item_attr_tab) / sizeof(cfg_item_attr_tab[0])) {
        err = -2;
        goto _ERR_RET;
    }

    u32 data_len = cfg_item_attr_tab[type].data_len;

    if (item_len < new_data_len) {
        err = -1;
        goto _ERR_RET;
    }

    item_len = cfg_item_attr_tab[type].item_len;
    item_offset = cfg_item_attr_tab[type].member_offset;

    vm_read_by_addr((u8 *)&item_u.common_cfg, exif_addr + item_offset, item_len);

    if ((item_u.common_cfg.len == new_data_len) && \
        (0 == memcmp((u8 *)&item_u.common_cfg.data, new_data, new_data_len))) {
        *write_flag = 0;
    } else {
        *write_flag = 1;
    }

#if 0
    switch (type) {
    case CFG_ITEM_ADV_IND:
        item_len = sizeof(p_ex_cfg->adv_data_cfg.data);
        if (item_len < new_data_len) {
            err = -1;
            goto _ERR_RET;
        }
        item_offset = MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, adv_data_cfg);
        vm_read_by_addr((u8 *)&item_u.adv_data_cfg, exif_addr + item_offset, sizeof(adv_data_cfg_t));
        if ((item_u.adv_data_cfg.len == new_data_len) && \
            (0 == memcmp((u8 *)&item_u.adv_data_cfg.data, new_data, new_data_len))) {
            *write_flag = 0;
        } else {
            *write_flag = 1;
        }
        break;
    case CFG
        }
#endif
    _ERR_RET:
    return err;
}
static u32 custom_cfg_item_write(u8 type, u8 *data, u16 data_len)
{
    ex_cfg_item_u item_u;
    u16 item_len = 0;
    ex_cfg_t *p_ex_cfg = NULL;
    u32 item_offset = 0;
    u32 exif_addr = exif_info.addr;

    u32 err = 0;

    if (type > sizeof(cfg_item_attr_tab) / sizeof(cfg_item_attr_tab[0])) {
        err = -2;
        goto _ERR_RET;
    }

    u32 item_data_len = cfg_item_attr_tab[type].data_len;

    if (item_data_len < data_len) {
        err = -1;
        goto _ERR_RET;
    }

    item_len = cfg_item_attr_tab[type].item_len;
    item_offset = cfg_item_attr_tab[type].member_offset;

    vm_read_by_addr((u8 *)&item_u.common_cfg, exif_addr + item_offset, item_len);
    if ((0xffff != item_u.common_cfg.crc) && \
        (0xffff != item_u.common_cfg.len)) {
        err = -3;
        goto _ERR_RET;
    }
#if 0
    vm_read_by_addr((u8 *)&item_u.common_cfg, exif_addr + item_offset, item_len);

    if ((item_u.common_cfg.len == new_data_len) && \
        (0 == memcmp((u8 *)&item_u.common_cfg.data, new_data, new_data_len))) {
        *write_flag = 0;
    } else {
        *write_flag = 1;
    }
#else
    memset((u8 *)&item_u.common_cfg.data, 0x00, item_data_len);
    memcpy((u8 *)&item_u.common_cfg.data, data, data_len);
    item_u.common_cfg.len = data_len;
    item_u.common_cfg.crc = crc16((u8 *)&item_u.common_cfg.data, data_len);

    vm_write_by_addr((u8 *)&item_u.common_cfg, exif_addr + item_offset, item_len);
#endif

_ERR_RET:
    cfg_printf(">>>write item:%x err:%x\n", type, err);
    return err;
}

static u32 custom_cfg_fill(ex_cfg_t *user_ex_cfg, u8 *data, u16 data_len, u8 type, u8 *write_flag)
{
    u16 len = 0;
    switch (type) {
    case CFG_ITEM_ADV_IND:
        len = sizeof(user_ex_cfg->adv_data_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->adv_data_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->adv_data_cfg.data, data, len);
                user_ex_cfg->adv_data_cfg.len = len;
                user_ex_cfg->adv_data_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->adv_data_cfg.data, data, len);
            user_ex_cfg->adv_data_cfg.len = len;
            user_ex_cfg->adv_data_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_SCAN_RSP:
        len = sizeof(user_ex_cfg->scan_rsp_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->scan_rsp_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->scan_rsp_cfg.data, data, len);
                user_ex_cfg->scan_rsp_cfg.len = len;
                user_ex_cfg->scan_rsp_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->scan_rsp_cfg.data, data, len);
            user_ex_cfg->scan_rsp_cfg.len = len;
            user_ex_cfg->scan_rsp_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_BLE_NAME:
        len = sizeof(user_ex_cfg->ble_name_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->ble_name_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->ble_name_cfg.data, data, len);
                user_ex_cfg->ble_name_cfg.len = len;
                user_ex_cfg->ble_name_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->ble_name_cfg.data, data, len);
            user_ex_cfg->ble_name_cfg.len = len;
            user_ex_cfg->ble_name_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_BT_NAME:
        len = sizeof(user_ex_cfg->bt_name_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->bt_name_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->bt_name_cfg.data, data, len);
                user_ex_cfg->bt_name_cfg.len = len;
                user_ex_cfg->bt_name_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->bt_name_cfg.data, data, len);
            user_ex_cfg->bt_name_cfg.len = len;
            user_ex_cfg->bt_name_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_PIN_CODE:
        len = sizeof(user_ex_cfg->pin_code_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->pin_code_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->pin_code_cfg.data, data, len);
                user_ex_cfg->pin_code_cfg.len = len;
                user_ex_cfg->pin_code_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->pin_code_cfg.data, data, len);
            user_ex_cfg->pin_code_cfg.len = len;
            user_ex_cfg->pin_code_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_VER_INFO:
        len = sizeof(user_ex_cfg->ver_info_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp((u8 *)&user_ex_cfg->ver_info_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy((u8 *)&user_ex_cfg->ver_info_cfg.data, data, len);
                user_ex_cfg->ver_info_cfg.len = len;
                user_ex_cfg->ver_info_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy((u8 *)&user_ex_cfg->ver_info_cfg.data, data, len);
            user_ex_cfg->ver_info_cfg.len = len;
            user_ex_cfg->ver_info_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_RESET_IO_INFO:
        len = sizeof(user_ex_cfg->reset_io_info_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp((u8 *)&user_ex_cfg->reset_io_info_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy((u8 *)&user_ex_cfg->reset_io_info_cfg.data, data, len);
                user_ex_cfg->reset_io_info_cfg.len = len;
                user_ex_cfg->reset_io_info_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy((u8 *)&user_ex_cfg->reset_io_info_cfg.data, data, len);
            user_ex_cfg->reset_io_info_cfg.len = len;
            user_ex_cfg->reset_io_info_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_PILOT_LAMP_IO_INFO:
        len = sizeof(user_ex_cfg->pilot_lamp_io_info_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->pilot_lamp_io_info_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->pilot_lamp_io_info_cfg.data, data, len);
                user_ex_cfg->pilot_lamp_io_info_cfg.len = len;
                user_ex_cfg->pilot_lamp_io_info_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->pilot_lamp_io_info_cfg.data, data, len);
            user_ex_cfg->pilot_lamp_io_info_cfg.len = len;
            user_ex_cfg->pilot_lamp_io_info_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_POWER_IO_ON_OFF:
        len = sizeof(user_ex_cfg->power_io_on_off_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->power_io_on_off_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->power_io_on_off_cfg.data, data, len);
                user_ex_cfg->power_io_on_off_cfg.len = len;
                user_ex_cfg->power_io_on_off_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->power_io_on_off_cfg.data, data, len);
            user_ex_cfg->power_io_on_off_cfg.len = len;
            user_ex_cfg->power_io_on_off_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_LINK_KEY_INFO:
        len = sizeof(user_ex_cfg->link_key_info_cfg.data.link_key);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->link_key_info_cfg.data.link_key, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->link_key_info_cfg.data.link_key, data, len);
                user_ex_cfg->link_key_info_cfg.len = len;
                user_ex_cfg->link_key_info_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->link_key_info_cfg.data.link_key, data, len);
            user_ex_cfg->link_key_info_cfg.len = len;
            user_ex_cfg->link_key_info_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_LAST_DEVICE_LINK_KEY_INFO:
        len = sizeof(user_ex_cfg->last_device_connect_linkkey_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->last_device_connect_linkkey_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->last_device_connect_linkkey_cfg.data, data, len);
                user_ex_cfg->last_device_connect_linkkey_cfg.len = len;
                user_ex_cfg->last_device_connect_linkkey_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->last_device_connect_linkkey_cfg.data, data, len);
            user_ex_cfg->last_device_connect_linkkey_cfg.len = len;
            user_ex_cfg->last_device_connect_linkkey_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_LOW_POWER_VOLTAGE:
        len = sizeof(user_ex_cfg->low_power_voltage_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->low_power_voltage_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->low_power_voltage_cfg.data, data, len);
                user_ex_cfg->low_power_voltage_cfg.len = len;
                user_ex_cfg->low_power_voltage_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->low_power_voltage_cfg.data, data, len);
            user_ex_cfg->low_power_voltage_cfg.len = len;
            user_ex_cfg->low_power_voltage_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_EDR_ADDR:
        len = sizeof(user_ex_cfg->edr_addr_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->edr_addr_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->edr_addr_cfg.data, data, len);
                user_ex_cfg->edr_addr_cfg.len = len;
                user_ex_cfg->edr_addr_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->edr_addr_cfg.data, data, len);
            user_ex_cfg->edr_addr_cfg.len = len;
            user_ex_cfg->edr_addr_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_BLE_ADDR:
        len = sizeof(user_ex_cfg->ble_addr_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->ble_addr_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->ble_addr_cfg.data, data, len);
                user_ex_cfg->ble_addr_cfg.len = len;
                user_ex_cfg->ble_addr_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->ble_addr_cfg.data, data, len);
            user_ex_cfg->ble_addr_cfg.len = len;
            user_ex_cfg->ble_addr_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_GATT_PROFILE:
        len = sizeof(user_ex_cfg->gatt_profile_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->gatt_profile_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->gatt_profile_cfg.data, data, len);
                user_ex_cfg->gatt_profile_cfg.len = len;
                user_ex_cfg->gatt_profile_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->gatt_profile_cfg.data, data, len);
            user_ex_cfg->gatt_profile_cfg.len = len;
            user_ex_cfg->gatt_profile_cfg.crc = crc16(data, len);
        }
        break;
#if VER_INFO_EXT_COUNT
    case CFG_ITEM_VER_INFO_AUTHKEY:
        len = sizeof(user_ex_cfg->ver_info_authkey_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->ver_info_authkey_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->ver_info_authkey_cfg.data, data, len);
                user_ex_cfg->ver_info_authkey_cfg.len = len;
                user_ex_cfg->ver_info_authkey_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->ver_info_authkey_cfg.data, data, len);
            user_ex_cfg->ver_info_authkey_cfg.len = len;
            user_ex_cfg->ver_info_authkey_cfg.crc = crc16(data, len);
        }
        break;
    case CFG_ITEM_VER_INFO_PROCODE:
        len = sizeof(user_ex_cfg->ver_info_procode_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->ver_info_procode_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->ver_info_procode_cfg.data, data, len);
                user_ex_cfg->ver_info_procode_cfg.len = len;
                user_ex_cfg->ver_info_procode_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->ver_info_procode_cfg.data, data, len);
            user_ex_cfg->ver_info_procode_cfg.len = len;
            user_ex_cfg->ver_info_procode_cfg.crc = crc16(data, len);
        }
        break;
#endif
    case CFG_ITEM_BLE_READ_WRITE_UUID_INFO:
        len = sizeof(user_ex_cfg->ble_read_write_uuid_cfg.data);
        len = len > data_len ? data_len : len;
        if (!(*write_flag)) {
            if (memcmp(user_ex_cfg->ble_read_write_uuid_cfg.data, data, len)) {
                *write_flag = 1;
                memcpy(user_ex_cfg->ble_read_write_uuid_cfg.data, data, len);
                user_ex_cfg->ble_read_write_uuid_cfg.len = len;
                user_ex_cfg->ble_read_write_uuid_cfg.crc = crc16(data, len);
            }
        } else {
            memcpy(user_ex_cfg->ble_read_write_uuid_cfg.data, data, len);
            user_ex_cfg->ble_read_write_uuid_cfg.len = len;
            user_ex_cfg->ble_read_write_uuid_cfg.crc = crc16(data, len);
        }
        break;
    }
    return 0;
}
#if VER_INFO_EXT_COUNT
static u32 fill_authkey_procode(ex_cfg_t *user_ex_cfg, u8 *item_data, u16 item_len, u8 *write_flag)
{
    if (!item_len) {
        return 0;
    }
    u16 i = 0;
    u16 index = 0;
    u8 type = CFG_ITEM_VER_INFO_AUTHKEY;
    u8 separator = ',';
    for (; i < item_len && type < CFG_ITEM_VER_INFO_AUTHKEY + VER_INFO_EXT_COUNT; i++) {
        if (separator == item_data[i]) {
            custom_cfg_item_write(type, item_data + index, i - index);
            index = i + 1;
            type++;
        } else if (i == item_len - 1) {
            custom_cfg_item_write(type, item_data + index, item_len - index);
        }
    }
    return 0;
}
#endif

static u32 custom_cfg_file_fill(ex_cfg_t *user_ex_cfg, u8 *write_flag)
{
    u32 err;
    u16 i;
    u8 *item_data;
    u8 *item_name;
    u16 item_len;
    for (i = 0 ; i < sizeof(cfg_item_description) / sizeof(cfg_item_description[0]); i++) {
        err = get_cfg_data_connect_by_name(cfg_item_description[i].item_name, \
                                           cfg_item_description[i].item_data, \
                                           cfg_item_description[i].item_len, \
                                           cfg_item_description[i].real_len);
        if (CFG_ERR_NONE != err) {
            cfg_printf("cfg_item:%d read error:%x\n", i, err);
        } else {
            cfg_printf("cfg_name %s\n cfg_len%x\n", cfg_item_description[i].item_name, *(cfg_item_description[i].real_len));
            cfg_printf_buf((u8 *)cfg_item_description[i].item_data, *(cfg_item_description[i].real_len));
        }
        if (cfg_item_description[i].real_len != 0) {
            item_name = cfg_item_description[i].item_name;
            item_data = cfg_item_description[i].item_data;
            item_len = *cfg_item_description[i].real_len;
            if (0 == strcmp((const char *)item_name, "ver_info")) {
                custom_cfg_item_write(CFG_ITEM_VER_INFO, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "reset_io")) {
                custom_cfg_item_write(CFG_ITEM_RESET_IO_INFO, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "pilot_lamp_io")) {
                custom_cfg_item_write(CFG_ITEM_PILOT_LAMP_IO_INFO, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "link_key")) {
                custom_cfg_item_write(CFG_ITEM_LINK_KEY_INFO, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "power_io")) {
                custom_cfg_item_write(CFG_ITEM_POWER_IO_ON_OFF, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "ver_info_ext")) {
#if VER_INFO_EXT_COUNT
                custom_cfg_item_write(CFG_ITEM_VER_INFO, item_data, item_len);
#endif
            }
        }
    }
    return err;
}



extern int ble_at_get_address(u8 *addr);
//extern char *hook_get_host_name(void);
extern const char *bt_get_local_name();
extern const u8 *bt_get_mac_addr();
//extern char *hook_get_pin_code(void);
extern const char *bt_get_pin_code();
//extern void hook_get_mac_addr(u8 *btaddr);
extern u8 *ble_get_scan_rsp_ptr(u8 *len);
extern u8 *ble_get_adv_data_ptr(u8 *len);
extern int le_controller_get_mac(void *addr);
extern u8 *ble_get_gatt_profile_data(u16 *len);
extern const char *bt_get_local_name();
extern u8 *get_last_device_connect_linkkey(u16 *len);

static u32 ex_cfg_fill_content(ex_cfg_t *user_ex_cfg, u8 *write_flag)
{
    custom_cfg_file_fill(NULL, write_flag);

    //CFG_ITEM_BT_NAME
    u8 *host_name = (u8 *)bt_get_local_name();
    u16 host_name_len = strlen(bt_get_local_name());
    custom_cfg_item_write(CFG_ITEM_BT_NAME, host_name, host_name_len);

    //CFG_ITEM_EDR_ADDR
    u8 addr[6];
    //hook_get_mac_addr(addr);
    custom_cfg_item_write(CFG_ITEM_EDR_ADDR, bt_get_mac_addr(), sizeof(addr));

    //CFG_ITEM_PIN_CODE
#if (0 == BT_CONNECTION_VERIFY)
    u8 pin_code[] = {BT_CONNECTION_VERIFY, VER_INFO_EXT_COUNT, 0, 0};
    u16 pin_code_len = sizeof(pin_code);
#else
    u8 *pin_code = (u8 *)bt_get_pin_code();
    u16 pin_code_len = strlen(bt_get_pin_code());
#endif
    custom_cfg_item_write(CFG_ITEM_PIN_CODE, pin_code, pin_code_len);


    //CFG_ITEM_BLE_NAME
    host_name = bt_get_local_name();
    host_name_len = strlen(bt_get_local_name());
    custom_cfg_item_write(CFG_ITEM_BLE_NAME, host_name, host_name_len);

    //CFG_ITEM_BLE_ADDR
    le_controller_get_mac(addr);
    //CFG_ITEM_SCAN_RSP
    u16 len;
    u8 *item_data = ble_get_scan_rsp_ptr(&len);
    cfg_printf("get item_data\n");
    cfg_printf_buf(item_data, len);
    struct excfg_rsp_payload rsp_payload;

    //New Scan_rsp
    /*
     *  |      len(1 Byte)     |     type(1 Byte)     |     data(name_len)    |
     *        bt_name_len      |         0x9          |        name_str       |
     *  jl_payloader_len(14)   |         0xff         |      jl_payloader     |
    */
#if RCSP_ADV_EN || RCSP_BTMATE_EN
    if (get_rcsp_support_new_reconn_flag()) {
#else
    if (0) {
#endif
        u8 *rsp_data = malloc(31);
        u8 i, rsp_len = 0;
        if (rsp_data) {
            cfg_printf("[make new rsp data]\n");
            rsp_payload.vid = 0x05D6;
            memcpy(rsp_payload.logo, "JLOTA", sizeof("JLOTA"));
            for (i = 0; i < sizeof(rsp_payload.logo) / 2; i++) {
                rsp_payload.logo[i] ^= rsp_payload.logo[sizeof(rsp_payload.logo) - i - 1];
                rsp_payload.logo[sizeof(rsp_payload.logo) - i - 1] ^= rsp_payload.logo[i];
                rsp_payload.logo[i] ^= rsp_payload.logo[sizeof(rsp_payload.logo) - i - 1];
            }
            rsp_payload.version = 0;
            memcpy(rsp_payload.addr, addr, 6);

            while (i < len) {                           //如果rsp_data里有名字要把名字也拷贝出来
                if (*(item_data + 1) == 0x09) {         //find HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME:0x09
                    memcpy(rsp_data, item_data, *item_data + 1);
                    rsp_len = *item_data + 1;
                    break;
                }
                i += (1 + *item_data);
                item_data += (1 + *item_data);
            }

            if (rsp_len + sizeof(struct excfg_rsp_payload) + 2 > 31) {
                cfg_printf("rsp data overflow!!!\n");
            } else {
                *(rsp_data + rsp_len) = sizeof(struct excfg_rsp_payload) + 1;        //fill jlpayload
                *(rsp_data + rsp_len + 1) = 0xff;                                    // HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA
                memcpy(rsp_data + rsp_len + 2, &rsp_payload, sizeof(struct excfg_rsp_payload));
                rsp_len += (2 + sizeof(struct excfg_rsp_payload));
                addr[0] += 1;                                                        //修改地址，让手机重新发现服务, 这里地址的修改规则可以用户自行设置
                cfg_printf("new rsp_data:\n");
                cfg_printf_buf(rsp_data, rsp_len);
                custom_cfg_item_write(CFG_ITEM_SCAN_RSP, rsp_data, rsp_len);
            }

            //广播包里有0xff字段也要找出来去掉，小程序判断到adv和rsp有重复字段是会出错
            u8 new_adv_len = 0;
            i = 0;
            item_data = ble_get_adv_data_ptr(&len);
            while (i < len) {                           //找出不等于0xff的信息,拷贝到new_adv_data
                if (*(item_data + 1) != 0xff) {
                    //memcpy(rsp_data, item_data, *item_data + 1);
                    memcpy(rsp_data, item_data, *item_data + 1);
                    new_adv_len += *item_data + 1;
                }
                i += (1 + *item_data);
                item_data += (1 + *item_data);
            }
            cfg_printf("new adv_data:\n");
            cfg_printf_buf(rsp_data, new_adv_len);
            custom_cfg_item_write(CFG_ITEM_ADV_IND, rsp_data, new_adv_len);

            free(rsp_data);
        }

    } else {
        custom_cfg_item_write(CFG_ITEM_SCAN_RSP, item_data, len);
    }
    custom_cfg_item_write(CFG_ITEM_BLE_ADDR, addr, sizeof(addr));

    //CFG_ITEM_ADV_IND
    item_data = ble_get_adv_data_ptr(&len);
    custom_cfg_item_write(CFG_ITEM_ADV_IND, item_data, len);

    //CFG_ITEM_GATT_PROFILE
    item_data = ble_get_gatt_profile_data(&len);
    custom_cfg_item_write(CFG_ITEM_GATT_PROFILE, item_data, len);


    //CFG_ITEM_LAST_DEVICE_LINK_KEY_INFO
    item_data = get_last_device_connect_linkkey(&len);
    put_buf(item_data, len);
    custom_cfg_item_write(CFG_ITEM_LAST_DEVICE_LINK_KEY_INFO, item_data, len);

#if VER_INFO_EXT_COUNT
    u8 authkey_len = 0;
    u8 *local_authkey_data = NULL;
    get_authkey_procode_from_cfg_file(&local_authkey_data, &authkey_len, GET_AUTH_KEY_FROM_EX_CFG);
    custom_cfg_item_write(CFG_ITEM_VER_INFO_AUTHKEY, local_authkey_data, authkey_len);

    u8 procode_len = 0;
    u8 *local_procode_data = NULL;
    get_authkey_procode_from_cfg_file(&local_procode_data, &procode_len, GET_PRO_CODE_FROM_EX_CFG);

    custom_cfg_item_write(CFG_ITEM_VER_INFO_PROCODE, local_procode_data, procode_len);
#endif

    u16 pvid[2] = {0};
    pvid[0] =  get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);
    pvid[1] =  get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG);

    custom_cfg_item_write(CFG_ITEM_PVID, pvid, sizeof(pvid));

    u8 md5[32] = {0};
    FILE *fp = NULL;
#define RES_MD5_FILE	SDFILE_RES_ROOT_PATH"md5.bin"
    fp = fopen(RES_MD5_FILE, "r");
    if (fp) {
        fread(fp, (void *)md5, 32);
        fclose(fp);
    }
    custom_cfg_item_write(CFG_ITEM_MD5, md5, sizeof(md5));
    u8 sdk_type = 0;
#if RCSP_ADV_EN || RCSP_BTMATE_EN
    sdk_type = RCSP_SDK_TYPE;
#endif
    custom_cfg_item_write(CFG_ITEM_SDK_TYPE, &sdk_type, sizeof(sdk_type));
    return 0;
}

#define TEMP_BUF_LEN	0x20
static void ex_cfg_write_to_flash(ex_cfg_t *user_ex_cfg)
{
    u32 exif_addr = exif_info.addr;
    u32 exif_len = exif_info.len;
    u8 empty_flag = 1;
    u8 temp_buf[TEMP_BUF_LEN];
    u16 i;

    cfg_printf("exif_addr : 0x%x, exif_len : %d\n", exif_addr, exif_len);

    if (sizeof(ex_cfg_t) > exif_len) {
        cfg_printf("-EX_CFG_TOO_LONG:%x\n", sizeof(ex_cfg_t));
        return ;
    }

    u16 total_len = sizeof(ex_cfg_t);
    u16 read_len = sizeof(temp_buf);

    while (total_len) {
        u16 len = total_len > read_len ? read_len : total_len;
        vm_read_by_addr(temp_buf, exif_addr, len);

        for (i = 0; i < sizeof(temp_buf); i++) {
            if (temp_buf[i] != 0xff) {
                empty_flag = 0;
            }
        }
        total_len -= len;
        exif_addr += len;
    }

    if (empty_flag) {
        cfg_puts("EX_CFG_AREA EMPTY\n");
    } else {
        cfg_puts("EX_CFG_HAD_DATA\n");
        return;
    }
    vm_write_by_addr((u8 *)user_ex_cfg, exif_addr, sizeof(ex_cfg_t));
}

#define PRINT_EX_CFG_INFO	0
static void print_user_ex_cfg_info(ex_cfg_t *user_ex_cfg);
#define EXIF_CFG_PATH	SDFILE_APP_ROOT_PATH"exif"
__attribute__((noinline))
static void ex_cfg_get_addr_and_len(u32 *addr, u32 *len)
{
    FILE *fp = fopen(EXIF_CFG_PATH, "r");
    if (fp) {
        cfg_printf("open %s succ", EXIF_CFG_PATH);
        struct vfs_attr attr = {0};
        fget_attrs(fp, &attr);

        extern u32 sdfile_cpu_addr2flash_addr(u32 offset);
        *addr = sdfile_cpu_addr2flash_addr(attr.sclust);
        *len = attr.fsize;
    } else {

        cfg_printf("open %s fail", EXIF_CFG_PATH);
        *addr = 0;
        *len = 0;
    }

    cfg_printf(">>>exif addr:%x len:%x\n", *addr, *len);
}

u32 ex_cfg_get_start_addr(void)
{
    u32 start_addr;
    FILE *fp = fopen(EXIF_CFG_PATH, "r");
    if (fp) {
        cfg_printf("open %s succ", EXIF_CFG_PATH);
        struct vfs_attr attr = {0};
        fget_attrs(fp, &attr);

        extern u32 sdfile_cpu_addr2flash_addr(u32 offset);
        start_addr = sdfile_cpu_addr2flash_addr(attr.sclust);
    } else {

        cfg_printf("open %s fail", EXIF_CFG_PATH);
        start_addr = 0;
    }

    return start_addr;
}

#define SECOTR_SIZE	(4*1024L)
u32 ex_cfg_fill_content_api(void)
{
    ex_cfg_get_addr_and_len(&exif_info.addr, &exif_info.len);

    if (exif_info.addr) {
        //ex_cfg_read_flash_by_addr((u8 *)&user_ex_cfg, 0, sizeof(ex_cfg_t));
#if (EXIF_ERASE_CONFIG == ALWAYS_ERASE_EXIF_AREA)
        if (exif_info.len < SECOTR_SIZE) {
            puts("exif len less SECOTR_SiZE\n");
            while (1);
        }

        sfc_erase(SECTOR_ERASER, exif_info.addr);
#else
#error "To do ..."
#endif

        ex_cfg_fill_content(NULL, NULL);
    }

    return exif_info.addr;
}
u32 ex_cfg_fill_content_api_test(void)
{
    return 0;
}


static void print_user_ex_cfg_info(ex_cfg_t *user_ex_cfg)
{
#if PRINT_EX_CFG_INFO
    puts("adv_data_cfg:\n");
    printf_buf(user_ex_cfg->adv_data_cfg.data, user_ex_cfg->adv_data_cfg.len);
    puts("scan_rsp_cfg:\n");
    printf_buf(user_ex_cfg->scan_rsp_cfg.data, user_ex_cfg->scan_rsp_cfg.len);
    puts("ble_name_cfg:\n");
    printf_buf(user_ex_cfg->ble_name_cfg.data, user_ex_cfg->ble_name_cfg.len);
    puts("ble_addr_cfg:\n");
    printf_buf(user_ex_cfg->ble_addr_cfg.data, user_ex_cfg->ble_addr_cfg.len);
    puts("bt_name_cfg:\n");
    printf_buf(user_ex_cfg->bt_name_cfg.data, user_ex_cfg->bt_name_cfg.len);
    puts("pin_code_cfg:\n");
    printf_buf((u8 *)user_ex_cfg->pin_code_cfg.data, user_ex_cfg->pin_code_cfg.len);
    puts("ver_info_cfg:\n");
    printf_buf((u8 *)&user_ex_cfg->ver_info_cfg.data, user_ex_cfg->ver_info_cfg.len);
    puts("reset_io_info_cfg:\n");
    printf_buf((u8 *)&user_ex_cfg->reset_io_info_cfg.data, user_ex_cfg->reset_io_info_cfg.len);
    puts("pilot_lamp_io_info_cfg:\n");
    printf_buf(user_ex_cfg->pilot_lamp_io_info_cfg.data, user_ex_cfg->pilot_lamp_io_info_cfg.len);
    puts("power_io_on_off_cfg:\n");
    printf_buf(user_ex_cfg->power_io_on_off_cfg.data, user_ex_cfg->power_io_on_off_cfg.len);
    puts("link_key_info_cfg:\n");
    printf_buf(user_ex_cfg->link_key_info_cfg.data.link_key, user_ex_cfg->link_key_info_cfg.len);
    puts("last_device_connect_linkkey_cfg:\n");
    printf_buf(user_ex_cfg->last_device_connect_linkkey_cfg.data, user_ex_cfg->last_device_connect_linkkey_cfg.len);
    puts("ver_info_authkey_cfg:\n");
    printf("len is %x\n", user_ex_cfg->ver_info_authkey_cfg.len);
    if (0xffff != user_ex_cfg->ver_info_authkey_cfg.len) {
        printf_buf(user_ex_cfg->ver_info_authkey_cfg.data, user_ex_cfg->ver_info_authkey_cfg.len);
    }
    puts("ver_info_procode_cfg:\n");
    printf("len is %x\n", user_ex_cfg->ver_info_procode_cfg.len);
    if (0xffff != user_ex_cfg->ver_info_procode_cfg.len) {
        printf_buf(user_ex_cfg->ver_info_procode_cfg.data, user_ex_cfg->ver_info_procode_cfg.len);
    }
    puts("ble_read_write_uuid_cfg:\n");
    printf("len is %x\n", user_ex_cfg->ble_read_write_uuid_cfg.len);
    if (0xffff != user_ex_cfg->ble_read_write_uuid_cfg.len) {
        printf_buf(user_ex_cfg->ble_read_write_uuid_cfg.data, user_ex_cfg->ble_read_write_uuid_cfg.len);
    }
    puts("-------------------------\n");
    puts("new_ex_cfg_info:\n");
    printf_buf((u8 *)user_ex_cfg, sizeof(ex_cfg_t));
#endif
}

void ex_cfg_data_load(void)
{
    ex_cfg_t user_ex_cfg;
    u32 exif_addr = exif_info.addr;
    vm_read_by_addr((u8 *)&user_ex_cfg, exif_addr, sizeof(ex_cfg_t));
    cfg_puts("user_ex_cfg:\n");
    cfg_printf_buf((u8 *)&user_ex_cfg, sizeof(ex_cfg_t));
}

u16 get_vid_pid_ver_from_cfg_file(u8 type)
{
    u8 *item_name = cfg_item_description[0].item_name;
    u8 *item_data = cfg_item_description[0].item_data;
    u16 item_len = cfg_item_description[0].item_len;
    u16 *real_len = cfg_item_description[0].real_len;
    if (!(*real_len)) {
        get_cfg_data_connect_by_name(item_name, item_data, item_len, real_len);
    }
    switch (type) {
    case GET_VID_FROM_EX_CFG:
        return ((u16)item_data[0] << 8 | (u16)item_data[1]);
    case GET_PID_FROM_EX_CFG:
        return ((u16)item_data[2] << 8 | (u16)item_data[3]);
    case GET_VER_FROM_EX_CFG:
        return ((u16)item_data[4] << 8 | (u16)item_data[5]);
    }

    return ((u16) - 1);
}

#if VER_INFO_EXT_COUNT
u32 get_authkey_procode_from_cfg_file(u8 *data[], u8 *len, u8 type)
{
    u8 *item_name = cfg_item_description[5].item_name;
    u8 *item_data = cfg_item_description[5].item_data;
    u16 item_len = cfg_item_description[5].item_len;
    u16 *real_len = cfg_item_description[5].real_len;
    if (!(*real_len)) {
        get_cfg_data_connect_by_name(item_name, item_data, item_len, real_len);
    }
    u8 offset = (u8) - 1;
    u8 index = 0;
    u8 separator = ',';
    while (item_data[++offset]) {
        if (separator == item_data[offset]) {
            if (type - GET_AUTH_KEY_FROM_EX_CFG) {
                index = offset;
                type --;
            } else {
                break;
            }
        }
    }
    if (!item_data[offset]) {
        index ++;
    }
    *data = item_data + index;
    *len = offset - index;
    return 0;
}
#endif

#endif


