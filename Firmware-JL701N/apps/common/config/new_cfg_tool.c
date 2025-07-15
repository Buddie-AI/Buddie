#include "cfg_tool.h"
#include "event.h"
#include "usb/device/cdc.h"
#include "boot.h"
#include "ioctl_cmds.h"
#include "board_config.h"
#include "app_online_cfg.h"
#include "asm/crc16.h"
#include "app_config.h"
#include "config/config_target.h"
#if APP_ONLINE_DEBUG
#include "online_db_deal.h"
#endif/*APP_ONLINE_DEBUG*/

/*
 *调音工具在线保存功能由协议接管，通道使用0x12，不分发给调音处理
 *配置工具通道为0x12，调音工具在线保存功能相关数据包通道必须设置为0x12
 *new_cfg_tool.c适用于新耳机/音箱配置工具和调音工具在线保存功能
 *支持异步协议和蓝牙SPP协议的数据解析与通信
 */

#define LOG_TAG_CONST       APP_CFG_TOOL
#define LOG_TAG             "[APP_CFG_TOOL]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

struct cfg_tool_info {
    /*PC往小机发送的DATA*/
    R_QUERY_BASIC_INFO 		r_basic_info;
    R_QUERY_FILE_SIZE		r_file_size;
    R_QUERY_FILE_CONTENT	r_file_content;
    R_PREPARE_WRITE_FILE	r_prepare_write_file;
    R_READ_ADDR_RANGE		r_read_addr_range;
    R_ERASE_ADDR_RANGE      r_erase_addr_range;
    R_WRITE_ADDR_RANGE      r_write_addr_range;
    R_ENTER_UPGRADE_MODE    r_enter_upgrade_mode;

    /*小机返回PC发送的DATA*/
    S_QUERY_BASIC_INFO 		s_basic_info;
    S_QUERY_FILE_SIZE		s_file_size;
    S_PREPARE_WRITE_FILE    s_prepare_write_file;
};

static struct cfg_tool_info info = {
    .s_basic_info.protocolVer = PROTOCOL_VER_AT_OLD,
};

#define TEMP_BUF_SIZE	256

extern const char *sdk_version_info_get(void);
extern u8 *sdfile_get_burn_code(u8 *len);
extern int norflash_erase(u32 cmd, u32 addr);

static u8 local_packet[TEMP_BUF_SIZE];
const char error_return[] = "FA";	//表示失败
const char ok_return[] = "OK";		//表示成功
const char er_return[] = "ER";		//表示不能识别的命令
static u32 size_total_write = 0;

extern void doe(u16 k, void *pBuf, u32 lenIn, u32 addr);
extern int norflash_erase(u32 cmd, u32 addr);

#ifdef ALIGN
#undef ALIGN
#endif
#define ALIGN(a, b) \
	({ \
	 	int m = (u32)(a) & ((b)-1); \
		int ret = (u32)(a) + (m?((b)-m):0);	 \
		ret;\
	})

static u32 cfg_tool_encode_data_by_user_key(u16 key, u8 *buff, u16 size, u32 dec_addr, u8 dec_len)
{
    u16 key_addr;
    u16 r_len;

    while (size) {
        r_len = (size > dec_len) ? dec_len : size;
        key_addr = (dec_addr >> 2)^key;
        doe(key_addr, buff, r_len, 0);
        buff += r_len;
        dec_addr += r_len;
        size -= r_len;
    }

    return dec_addr;
}

static u8 parse_seq = 0;
static void ci_send_packet_new(u32 id, u8 *packet, int size)
{
#if APP_ONLINE_DEBUG
    app_online_db_ack(parse_seq, packet, size);
#endif/*APP_ONLINE_DEBUG*/
}

struct cfg_tool_event spp_packet;
int cfg_tool_online_parse(u8 *buf, u32 len)
{
    parse_seq = buf[2];
    u32 event = (buf[3] | (buf[4] << 8) | (buf[5] << 16) | (buf[6] << 24));
    spp_packet.event = event;
    spp_packet.packet = buf;
    spp_packet.size = len;
    return (app_cfg_tool_event_handler(&spp_packet));
}

void all_assemble_package_send_to_pc(u8 id, u8 sq, u8 *buf, u32 len)
{
    u8 *send_buf = NULL;
    u16 crc16_data;

    send_buf = (u8 *)malloc(TEMP_BUF_SIZE);
    if (send_buf == NULL) {
        log_error("send_buf malloc err!");
        return;
    }

    send_buf[0] = 0x5A;
    send_buf[1] = 0xAA;
    send_buf[2] = 0xA5;
    send_buf[5] = 2 + len;/*L*/
    send_buf[6] = id;/*T*/
    send_buf[7] = sq;/*SQ*/
    memcpy(send_buf + 8, buf, len);
    /*添加CRC16*/
    crc16_data = CRC16(&send_buf[5], len + 3);
    send_buf[3] = crc16_data & 0xff;
    send_buf[4] = (crc16_data >> 8) & 0xff;

    /* printf_buf(send_buf, len + 8); */

#if (TCFG_CFG_TOOL_ENABLE || TCFG_ONLINE_ENABLE)
#if (TCFG_COMM_TYPE == TCFG_SPP_COMM)
    ci_send_packet_new(INITIATIVE_STYLE, buf, len);
#elif (TCFG_COMM_TYPE == TCFG_USB_COMM)
    cdc_write_data(0, send_buf, len + 8);
#elif (TCFG_COMM_TYPE == TCFG_UART_COMM)
    ci_uart_write(send_buf, len + 8);
#endif
#endif

    free(send_buf);
}

/*6个byte的校验码数组转为字符串*/
void hex2text(u8 *buf, u8 *out)
{
    sprintf(out, "%02x%02x-%02x%02x%02x%02x", buf[5], buf[4], buf[3], buf[2], buf[1], buf[0]);
}

u32 packet_combined(u8 *packet, u8 num)
{
    u32 _packet = 0;
    _packet = (packet[num] | (packet[num + 1] << 8) | (packet[num + 2] << 16) | (packet[num + 3] << 24));
    return _packet;
}

FILE *cfg_open_file(u32 file_id)
{
    FILE *cfg_fp = NULL;
    if (file_id <= CFG_EQ_FILEID) {
        if ((file_id == CFG_TOOL_FILEID)) {
            cfg_fp = fopen(CFG_TOOL_FILE, "r");
            log_info("open cfg_tool.bin\n");
        } else if ((file_id == CFG_OLD_EQ_FILEID)) {
            cfg_fp = fopen(CFG_OLD_EQ_FILE, "r");
            log_info("open old eq_cfg_hw.bin\n");
        } else if ((file_id == CFG_OLD_EFFECT_FILEID)) {
            cfg_fp = fopen(CFG_OLD_EFFECT_FILE, "r");
            log_info("open effects_cfg.bin\n");
        } else if ((file_id == CFG_EQ_FILEID)) {
            cfg_fp = fopen(CFG_EQ_FILE, "r");
            log_info("open eq_cfg_hw.bin\n");
        }
    }
    return cfg_fp;
}

extern void nvram_set_boot_state(u32 state);
extern void hw_mmu_disable(void);
extern void ram_protect_close(void);
AT(.volatile_ram_code)
void cfg_tool_go_mask_usb_updata()
{
    local_irq_disable();
    ram_protect_close();
    hw_mmu_disable();
    nvram_set_boot_state(2);

    JL_CLOCK->PWR_CON |= (1 << 4);
    /* chip_reset(); */
    /* cpu_reset(); */
    while (1);
}

/*延时复位防止工具升级进度条显示错误*/
static void delay_cpu_reset(void *priv)
{
    extern void cpu_reset();
    cpu_reset();
}

int app_cfg_tool_event_handler(struct cfg_tool_event *cfg_tool_dev)
{
    u8 *buf = NULL;
    u8 *buf_temp = NULL;
    u8 *buf_temp_0 = NULL;
    u8 valid_cmd;
    u32 erase_cmd;
    int write_len;
    int send_len;
    u8 crc_temp_len, sdkname_temp_len;
    char proCrc_fw[32] = {0};
    char sdkName_fw[32] = {0};
    const struct tool_interface *p;
    struct vfs_attr attr;
    FILE *cfg_fp = NULL;

    /* printf_buf(cfg_tool_dev->packet, cfg_tool_dev->size); */

    buf = (u8 *)malloc(TEMP_BUF_SIZE);
    if (buf == NULL) {
        log_error("buf malloc err!");
        return 0;
    }
    buf_temp_0 = (u8 *)malloc(TEMP_BUF_SIZE);
    if (buf_temp_0 == NULL) {
        free(buf);
        log_error("buf_temp_0 malloc err!");
        return 0;
    }
    buf_temp_0 = (u8 *)ALIGN(buf_temp_0, 4);
    memset(buf_temp_0, 0, TEMP_BUF_SIZE);
    memcpy(buf_temp_0 + 1, cfg_tool_dev->packet, cfg_tool_dev->size);

    /*数据进行处理*/
    list_for_each_tool_interface(p) {
        if (p->id == cfg_tool_dev->packet[1]) {
            p->tool_message_deal(buf_temp_0 + 2, cfg_tool_dev->size - 1);
            free(buf_temp_0);
            free(buf);
            return 1;
        }
    }

    memset(buf, 0, TEMP_BUF_SIZE);

    switch (cfg_tool_dev->event) {
    case ONLINE_SUB_OP_QUERY_BASIC_INFO:
        valid_cmd = 1;
        /* log_info("event_ONLINE_SUB_OP_QUERY_BASIC_INFO\n"); */
        /*获取校验码*/
        u8 *p = sdfile_get_burn_code(&crc_temp_len);
        memcpy(info.s_basic_info.progCrc, p + 8, 6);
        /* printf_buf(info.s_basic_info.progCrc, 6); */
        hex2text(info.s_basic_info.progCrc, proCrc_fw);
        /* log_info("crc:%s\n", proCrc_fw); */

        /*获取固件版本信息*/
        sdkname_temp_len = strlen(sdk_version_info_get());
        memcpy(info.s_basic_info.sdkName, sdk_version_info_get(), sdkname_temp_len);
        memcpy(sdkName_fw, info.s_basic_info.sdkName, sdkname_temp_len);
        /* log_info("version:%s\n", sdk_version_info_get()); */

        struct flash_head flash_head_for_pid_vid;

        for (u8 i = 0; i < 5; i++) {
            norflash_read(NULL, (u8 *)&flash_head_for_pid_vid, 32, 0x1000 * i);
            doe(0xffff, (u8 *)&flash_head_for_pid_vid, 32, 0);
            if (flash_head_for_pid_vid.crc == 0xffff) {
                continue;
            } else {
                log_info("flash head addr = 0x%x\n", 0x1000 * i);
                break;
            }
        }

        struct flash_head _head;
        struct flash_head *temp_p = &_head;
        memcpy(temp_p, &flash_head_for_pid_vid, 32);

        memset(info.s_basic_info.pid, 0, sizeof(info.s_basic_info.pid));
        memcpy(info.s_basic_info.pid, temp_p->pid, sizeof(info.s_basic_info.pid));
        for (u8 i = 0; i < sizeof(info.s_basic_info.pid); i++) {
            if (~info.s_basic_info.pid[i] == 0x00) {
                info.s_basic_info.pid[i] = 0x00;
            }
        }
        /* printf_buf(info.s_basic_info.pid, 16); */

        memset(info.s_basic_info.vid, 0, sizeof(info.s_basic_info.vid));
        memcpy(info.s_basic_info.vid, temp_p->vid, 4);
        /* printf_buf(info.s_basic_info.vid, 16); */

        send_len = sizeof(info.s_basic_info.protocolVer) + sizeof(proCrc_fw) + sizeof(sdkName_fw) + 32;
        buf[0] = info.s_basic_info.protocolVer & 0xff;
        buf[1] = (info.s_basic_info.protocolVer >> 8) & 0xff;
        memcpy(buf + 2, proCrc_fw, sizeof(proCrc_fw));
        memcpy(buf + 2 + sizeof(proCrc_fw), sdkName_fw, sizeof(sdkName_fw));
        memcpy(buf + 2 + sizeof(proCrc_fw) + sizeof(sdkName_fw), info.s_basic_info.pid, 16);
        memcpy(buf + 2 + sizeof(proCrc_fw) + sizeof(sdkName_fw) + 16, info.s_basic_info.vid, 16);
        break;
    case ONLINE_SUB_OP_QUERY_FILE_SIZE:
        valid_cmd = 1;
        /* log_info("event_ONLINE_SUB_OP_QUERY_FILE_SIZE\n"); */
        info.r_file_size.file_id = packet_combined(cfg_tool_dev->packet, 7);
        cfg_fp = cfg_open_file(info.r_file_size.file_id);

        if (cfg_fp == NULL) {
            log_error("file open error!\n");
            goto _exit_;
        }
        fget_attrs(cfg_fp, &attr);

        /* log_info("file addr:%x,file size:%d\n", attr.sclust, attr.fsize); */
        info.s_file_size.file_size = attr.fsize;

        send_len = sizeof(info.s_file_size.file_size);//长度

        /*小端格式*/
        buf[3] = (info.s_file_size.file_size >> 24) & 0xff;
        buf[2] = (info.s_file_size.file_size >> 16) & 0xff;
        buf[1] = (info.s_file_size.file_size >> 8) & 0xff;
        buf[0] = info.s_file_size.file_size & 0xff;

        fclose(cfg_fp);
        break;
    case ONLINE_SUB_OP_QUERY_FILE_CONTENT:
        valid_cmd = 1;
        /* log_info("event_ONLINE_SUB_OP_QUERY_FILE_CONTENT\n"); */
        info.r_file_content.file_id = packet_combined(cfg_tool_dev->packet, 7);
        info.r_file_content.offset = packet_combined(cfg_tool_dev->packet, 11);
        info.r_file_content.size = packet_combined(cfg_tool_dev->packet, 15);

        cfg_fp = cfg_open_file(info.r_file_content.file_id);

        if (cfg_fp == NULL) {
            log_error("file open error!\n");
            goto _exit_;
        }
        fget_attrs(cfg_fp, &attr);

        /* log_info("file addr:%x,file size:%d\n", attr.sclust, attr.fsize); */
        if (info.r_file_content.size > attr.fsize) {
            fclose(cfg_fp);
            log_error("reading size more than actual size!\n");
            break;
        }

        /*逻辑地址转换成flash物理地址*/
        u32 flash_addr = sdfile_cpu_addr2flash_addr(attr.sclust);
        /* log_info("flash_addr:0x%x", flash_addr); */
        /*读取文件内容*/
        buf_temp = (char *)malloc(info.r_file_content.size);
        norflash_read(NULL, (void *)buf_temp, info.r_file_content.size, flash_addr + info.r_file_content.offset);
        /* printf_buf(buf_temp, info.r_file_content.size); */
        send_len = info.r_file_content.size;
        memcpy(buf, buf_temp, info.r_file_content.size);

        if (buf_temp) {
            free(buf_temp);
        }
        fclose(cfg_fp);
        break;
    case ONLINE_SUB_OP_PREPARE_WRITE_FILE:
        valid_cmd = 1;
        /* log_info("event_ONLINE_SUB_OP_PREPARE_WRITE_FILE\n"); */
        info.r_prepare_write_file.file_id = packet_combined(cfg_tool_dev->packet, 7);
        info.r_prepare_write_file.size = packet_combined(cfg_tool_dev->packet, 11);

        cfg_fp = cfg_open_file(info.r_prepare_write_file.file_id);
        if (cfg_fp == NULL) {
            log_error("file open error!\n");
            break;
        }
        fget_attrs(cfg_fp, &attr);

        /* log_info("file addr:%x,file size:%d\n", attr.sclust, attr.fsize); */
        if (info.r_prepare_write_file.size > attr.fsize) {
            //fclose(cfg_fp);
            //log_error("preparing to write size more than actual size!\n");
            //break;
        }

        info.s_prepare_write_file.file_size = attr.fsize;
        info.s_prepare_write_file.file_addr = sdfile_cpu_addr2flash_addr(attr.sclust);
        info.s_prepare_write_file.earse_unit = boot_info.vm.align * 256;

        send_len = sizeof(info.s_prepare_write_file.file_size) * 3;

        buf[3] = (info.s_prepare_write_file.file_addr >> 24) & 0xff;
        buf[2] = (info.s_prepare_write_file.file_addr >> 16) & 0xff;
        buf[1] = (info.s_prepare_write_file.file_addr >> 8) & 0xff;
        buf[0] = info.s_prepare_write_file.file_addr & 0xff;

        buf[7] = (info.s_prepare_write_file.file_size >> 24) & 0xff;
        buf[6] 	= (info.s_prepare_write_file.file_size >> 16) & 0xff;
        buf[5] 	= (info.s_prepare_write_file.file_size >> 8) & 0xff;
        buf[4] 	= info.s_prepare_write_file.file_size & 0xff;

        buf[11] = (info.s_prepare_write_file.earse_unit >> 24) & 0xff;
        buf[10] = (info.s_prepare_write_file.earse_unit >> 16) & 0xff;
        buf[9] = (info.s_prepare_write_file.earse_unit >> 8) & 0xff;
        buf[8] = info.s_prepare_write_file.earse_unit & 0xff;

        fclose(cfg_fp);
        break;
    case ONLINE_SUB_OP_READ_ADDR_RANGE:
        valid_cmd = 1;
        /* log_info("event_ONLINE_SUB_OP_READ_ADDR_RANGE\n"); */
        info.r_read_addr_range.addr = packet_combined(cfg_tool_dev->packet, 7);
        /* log_info("reading flash addr:0x%x\n", info.r_read_addr_range.addr); */
        info.r_read_addr_range.size = packet_combined(cfg_tool_dev->packet, 11);
        /* log_info("reading size = %d\n", info.r_read_addr_range.size); */
        buf_temp = (char *)malloc(info.r_read_addr_range.size);
        norflash_read(NULL, (void *)buf_temp, info.r_read_addr_range.size, info.r_read_addr_range.addr);
        /* printf_buf(buf_temp, info.r_read_addr_range.size); */

        send_len = info.r_read_addr_range.size;
        memcpy(buf, buf_temp, info.r_read_addr_range.size);

        if (buf_temp) {
            free(buf_temp);
        }
        break;
    case ONLINE_SUB_OP_ERASE_ADDR_RANGE:
        valid_cmd = 1;
        /* log_info("event_ONLINE_SUB_OP_ERASE_ADDR_RANGE\n"); */
        info.r_erase_addr_range.addr = packet_combined(cfg_tool_dev->packet, 7);
        /* log_info("erasing flash start addr:0x%x\n", info.r_erase_addr_range.addr); */
        /*要擦除的大小，会保证按earse_unit对齐，即总是erase_unit的倍数*/
        info.r_erase_addr_range.size = packet_combined(cfg_tool_dev->packet, 11);
        /* log_info("erasing size = %d\n", info.r_erase_addr_range.size); */
        /* log_info("earse_unit = %d\n", info.s_prepare_write_file.earse_unit); */
        switch (info.s_prepare_write_file.earse_unit) {
        case 256:
            erase_cmd = IOCTL_ERASE_PAGE;
            break;
        case (4*1024):
            erase_cmd = IOCTL_ERASE_SECTOR;
            break;
        case (64*1024):
            erase_cmd = IOCTL_ERASE_BLOCK;
            break;
defualt:
            memcpy(buf, error_return, sizeof(error_return));
            log_error("erase error!");
            break;
        }

        for (u8 i = 0; i < (info.r_erase_addr_range.size / info.s_prepare_write_file.earse_unit); i ++) {
            u8 ret = norflash_erase(erase_cmd, info.r_erase_addr_range.addr + (i * info.s_prepare_write_file.earse_unit));
            if (ret) {
                memcpy(buf, error_return, sizeof(error_return));
                log_error("erase error!");
            } else {
                memcpy(buf, ok_return, sizeof(ok_return));
                log_info("erase success");
            }
        }

        send_len = sizeof(error_return);
        break;
    case ONLINE_SUB_OP_WRITE_ADDR_RANGE:
        valid_cmd = 1;
        /* log_info("event_ONLINE_SUB_OP_WRITE_ADDR_RANGE\n"); */
        info.r_write_addr_range.addr = packet_combined(cfg_tool_dev->packet, 7);
        info.r_write_addr_range.size = packet_combined(cfg_tool_dev->packet, 11);
        /* log_info("writing flash start addr:0x%x\n", info.r_write_addr_range.addr); */
        /* log_info("writing size = %d\n", info.r_write_addr_range.size); */
        buf_temp = (char *)malloc(info.r_write_addr_range.size);
        memcpy(buf_temp, cfg_tool_dev->packet + 15, info.r_write_addr_range.size);
        /* printf_buf(buf_temp, info.r_write_addr_range.size); */

        cfg_tool_encode_data_by_user_key(boot_info.chip_id, buf_temp, info.r_write_addr_range.size, info.r_write_addr_range.addr - boot_info.sfc.sfc_base_addr, 0x20);

        write_len = norflash_write(NULL, buf_temp, info.r_write_addr_range.size, info.r_write_addr_range.addr);

        /* norflash_read(NULL, buf_temp, info.r_write_addr_range.size, info.r_write_addr_range.addr); */
        /* printf_buf(buf_temp, info.r_write_addr_range.size); */

        if (write_len != info.r_write_addr_range.size) {
            memcpy(buf, error_return, sizeof(error_return));
            log_error("write error!");
        } else {
            memcpy(buf, ok_return, sizeof(ok_return));
        }
        send_len = sizeof(error_return);

        if (buf_temp) {
            free(buf_temp);
        }

        if (info.r_prepare_write_file.file_id == CFG_TOOL_FILEID) {
            size_total_write += info.r_write_addr_range.size;
            /* log_info("size_total_write = %d\n", size_total_write); */
            /* log_info("erasing size = %d\n", info.r_erase_addr_range.size); */
            if (size_total_write >= info.r_erase_addr_range.size) {
                size_total_write = 0;
                log_info("cpu_reset\n");
                extern u16 sys_timeout_add(void *priv, void (*func)(void *priv), u32 msec);
                sys_timeout_add(NULL, delay_cpu_reset, 500);
            }
        }
        break;
    case ONLINE_SUB_OP_ENTER_UPGRADE_MODE:
        valid_cmd = 1;
        log_info("event_ONLINE_SUB_OP_ENTER_UPGRADE_MODE\n");
        cfg_tool_go_mask_usb_updata();
        break;
    default:
        valid_cmd = 0;
        log_error("invalid data\n");
        memcpy(buf, er_return, sizeof(er_return));//不认识的命令返回ER
        send_len = sizeof(er_return);
        break;
_exit_:
        memcpy(buf, error_return, sizeof(error_return));//文件打开失败返回FA
        send_len = sizeof(error_return);
        break;
    }

    all_assemble_package_send_to_pc(REPLY_STYLE, cfg_tool_dev->packet[2], buf, send_len);

    free(buf_temp_0);
    free(buf);
    return (valid_cmd);
}

void cfg_tool_event_to_user(u8 *packet, u32 type, u8 event, u8 size)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    if (packet != NULL) {
        if (size > sizeof(local_packet)) {
            return;
        }
        e.u.cfg_tool.packet = local_packet;
        memcpy(e.u.cfg_tool.packet, packet, size);
    }
    e.arg  = (void *)type;
    e.u.cfg_tool.event = event;
    e.u.cfg_tool.size = size;
    sys_event_notify(&e);
}

void app_cfg_tool_message_deal(u8 *buf, u32 len)
{
    u8 cmd = buf[8];
    switch (cmd) {
    case ONLINE_SUB_OP_QUERY_BASIC_INFO:
    case ONLINE_SUB_OP_QUERY_FILE_SIZE:
    case ONLINE_SUB_OP_QUERY_FILE_CONTENT:
    case ONLINE_SUB_OP_PREPARE_WRITE_FILE:
    case ONLINE_SUB_OP_READ_ADDR_RANGE:
    case ONLINE_SUB_OP_ERASE_ADDR_RANGE:
    case ONLINE_SUB_OP_WRITE_ADDR_RANGE:
    case ONLINE_SUB_OP_ENTER_UPGRADE_MODE:
        cfg_tool_event_to_user(&buf[5], DEVICE_EVENT_FROM_CFG_TOOL, cmd, buf[5] + 1);
        break;
    default:
        cfg_tool_event_to_user(&buf[5], DEVICE_EVENT_FROM_CFG_TOOL, DEFAULT_ACTION, buf[5] + 1);
        break;
    }
}

void online_cfg_tool_data_deal(void *buf, u32 len)
{
    u8 *data_buf = buf;
    u16 crc16_data;

    /* printf_buf(buf, len); */

    /*DATA前的固定字节包括 (5A AA A5 CRC L T SQ)共8个字节 */
    if (len < 8) {
        log_error("Data length is too short, receive an invalid message!\n");
        return;
    }

    if ((data_buf[0] != 0x5a) || (data_buf[1] != 0xaa) || (data_buf[2] != 0xa5)) {
        log_error("Header check error, receive an invalid message!\n");
        return;
    }

    /*CRC16校验，CRC16校验码位于数据包的buf[3]和buf[4]*/
    crc16_data = (data_buf[4] << 8) | data_buf[3];
    if (crc16_data != CRC16(data_buf + 5, len - 5)) {
        log_error("CRC16 check error, receive an invalid message!\n");
        return;
    }

    app_cfg_tool_message_deal(buf, len);
}
