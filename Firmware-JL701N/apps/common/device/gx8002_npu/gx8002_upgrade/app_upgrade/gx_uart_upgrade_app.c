#include "includes.h"
#include "../gx_uart_upgrade.h"
#include "../../gx8002_npu_api.h"
#include "../gx_uart_upgrade_porting.h"
//#include "../utils/gx_fifo.h"

#if GX8002_UPGRADE_APP_TOGGLE

typedef struct _gx8002_file_head_t {
    u16 head_crc;
    u16 data_crc;
    u32 addr;
    u32 len;
    u8 attr;
    u8 res;
    u16 index;
    char name[16];
} gx8002_file_head_t;

struct app_ota_info {
    int update_result;
    u32 ufw_addr;
    u8 cur_seek_file;
    u8 update_role;
    OS_SEM ota_sem;
    OS_SEM rx_sem;
    cbuffer_t *cbuf_handle;
    update_op_api_t *file_ops;
    gx8002_file_head_t file_boot_head;
    gx8002_file_head_t file_bin_head;
    int packet_buf[UPGRADE_PACKET_SIZE / sizeof(int)];
};


enum GX_APP_UPDATE_ERR {
    GX_APP_UPDATE_ERR_NONE = 0,
    GX_APP_UPDATE_ERR_NOMEM = -100,
    GX_APP_UPDATE_ERR_FILE_READ,
    GX_APP_UPDATE_ERR_FILE_DIR_VERIFY,
    GX_APP_UPDATE_ERR_FILE_HEAD_VERIFY,
    GX_APP_UPDATE_ERR_FILE_DATA_VERIFY,

    GX_APP_UPDATE_ERR_UPDATE_LOOP,
    GX_APP_UPDATE_ERR_UPDATE_STATE,
};

enum GX8002_UPDATE_ROLE {
    GX8002_UPDATE_ROLE_NORMAL = 0,
    GX8002_UPDATE_ROLE_TWS_MASTER,
    GX8002_UPDATE_ROLE_TWS_SLAVE,
};

static struct app_ota_info *ota_upgrade = NULL;
#define __this 			ota_upgrade

#define GX8002_UFW_VERIFY_BUF_LEN 				512
#define GX8002_UFW_FILE_DATA_VERIFY_ENABLE 		0

#define GX8002_OTA_UPGRADE_DEBUG_ENABLE 		1

#define ota_upgrade_info 		 printf

#if GX8002_OTA_UPGRADE_DEBUG_ENABLE
#define ota_upgrade_debug 	printf
#define ota_upgrade_put_buf  put_buf
#else
#define ota_upgrade_debug(...)
#define ota_upgrade_put_buf(...)
#endif /* #if GX8002_DEBUG_ENABLE */

//=================================================================//
//                        tws同步升级接口                          //
//=================================================================//
extern void gx8002_uart_app_ota_tws_update_register_handle(void);
extern int gx8002_uart_app_ota_tws_update_init(void *priv);
extern int gx8002_uart_app_ota_tws_update_sync_start(u8 *buf, u16 len);
extern int gx8002_uart_app_ota_tws_update_sync_pend(void);
extern int gx8002_uart_app_ota_tws_update_sync_post(void);
extern int gx8002_uart_app_ota_tws_update_sync_data(u8 *buf, u16 len);
extern int gx8002_uart_app_ota_tws_update_sync_respond(void);
extern int gx8002_uart_app_ota_tws_update_close(void);
extern int gx8002_uart_app_ota_tws_update_sync_result(int result);
extern int gx8002_uart_app_ota_tws_update_sync_result_get(void);

//=================================================================//
//                        使用fifo作数据缓存                       //
//=================================================================//
////////////// FW Stream Porting //////////////////
static u32 fw_stream_get_file_addr(u32 addr)
{
    return __this->ufw_addr + addr;
}

#define STREAM_FIFO_BLOCK_TOTAL    16
#define STREAM_FIFO_TIMEOUT_MS    (5000)

static int fw_stream_open(FW_IMAGE_TYPE img_type)
{
    if (__this->cbuf_handle == NULL) {
        return -1;
    }

    return 0;
}

static int fw_stream_close(FW_IMAGE_TYPE img_type)
{
    return 0;
}

//gx8002线程同步数据
static int fw_stream_read(FW_IMAGE_TYPE img_type, unsigned char *buf, unsigned int len)
{
    int ret = 0;

    static u32 read_cnt = 0;

    if (__this->cbuf_handle) {
        if (cbuf_get_data_len(__this->cbuf_handle) < len) {
            os_sem_set(&(__this->rx_sem), 0);
            os_sem_pend(&(__this->rx_sem), msecs_to_jiffies(STREAM_FIFO_TIMEOUT_MS));
        }
        ret = cbuf_read(__this->cbuf_handle, buf, len);
        if (ret < len) {
            ota_upgrade_info("read errm ret = 0x%x, len = 0x%x", ret, len);
            return -1;
        }
        if (__this->update_role == GX8002_UPDATE_ROLE_TWS_MASTER) {
            //TODO: tws sync data
            gx8002_uart_app_ota_tws_update_sync_data(buf, (u16)len);
            if (gx8002_uart_app_ota_tws_update_sync_pend()) {
                ota_upgrade_info("read timeout");
                ret = -1;
            }
        } else if (__this->update_role == GX8002_UPDATE_ROLE_TWS_SLAVE) {
            //TODO: tws respond
            gx8002_uart_app_ota_tws_update_sync_respond();
        }
    }

    return ret;
}

static int fw_stream_get_flash_img_info(flash_img_info_t *info)
{
    if (info == NULL) {
        return -1;
    }
    info->img_size = __this->file_bin_head.len;

    ota_upgrade_debug("%s, img_size = 0x%x", __func__, info->img_size);

    return 0;
}

static int gx8002_ota_data_receive(u8 *data, u32 size)
{
    gx8002_file_head_t *file_receive_table[] = {
        &(__this->file_boot_head),
        &(__this->file_bin_head)
    };

    u32 file_addr = 0;
    u32 remain_len = 0;
    u32 rlen = 0;
    u16 ret = 0;

    for (u32 i = 0; i < ARRAY_SIZE(file_receive_table); i++) {
        file_addr = fw_stream_get_file_addr((file_receive_table[i])->addr);
        __this->file_ops->f_seek(NULL, SEEK_SET, file_addr);
        remain_len = (file_receive_table[i])->len;
        ota_upgrade_debug("receive %s data, len = 0x%x", (file_receive_table[i])->name, remain_len);
        while (remain_len) {
            if (__this->update_result != GX_APP_UPDATE_ERR_NONE) {
                return __this->update_result;
            }
            rlen = (remain_len > size) ? size : remain_len;
            ret = __this->file_ops->f_read(NULL, data, rlen);

            if ((u16) - 1 == ret) {
                ota_upgrade_debug("f_read %s err", (file_receive_table[i])->name);
                return GX_APP_UPDATE_ERR_FILE_READ;
            }
            //TODO: TWS send data sync
            while (cbuf_is_write_able(__this->cbuf_handle, rlen) < rlen) {
                /* ota_upgrade_debug("aaa"); */
                os_time_dly(2);
                /* ota_upgrade_debug("bbb"); */
                if (__this->update_result != GX_APP_UPDATE_ERR_NONE) {
                    return __this->update_result;
                }
            }
            if (cbuf_write(__this->cbuf_handle, data, rlen) != rlen) {
                ota_upgrade_debug("cbuf full !!!");
            }
            os_sem_post(&(__this->rx_sem));

            remain_len -= rlen;
        }
    }

    return 0;
}


int gx8002_ota_data_passive_receive(u8 *data, u32 size)
{
    int ret = 0;
    if ((__this == NULL) || (__this->cbuf_handle == NULL)) {
        return -1;
    }

    if (__this->update_role != GX8002_UPDATE_ROLE_TWS_SLAVE) {
        return -1;
    }

    if (cbuf_is_write_able(__this->cbuf_handle, size) < size) {
        ota_upgrade_info("passive receive cbuf full !!!");
        return -1;
    }

    ret = cbuf_write(__this->cbuf_handle, data, size);
    os_sem_post(&(__this->rx_sem));

    return ret;
}


static int __gx8002_ufw_file_data_verify(u32 file_addr, u32 file_len, u16 target_crc, u16(*func_read)(void *fp, u8 *buff, u16 len))
{
    u8 *rbuf = malloc(GX8002_UFW_VERIFY_BUF_LEN);
    u16 crc_interval = 0;
    u16 r_len = 0;
    int ret = GX_APP_UPDATE_ERR_NONE;

    if (rbuf == NULL) {
        return GX_APP_UPDATE_ERR_NOMEM;
    }

    while (file_len) {
        r_len = (file_len > GX8002_UFW_VERIFY_BUF_LEN) ? GX8002_UFW_VERIFY_BUF_LEN : file_len;
        if (func_read) {
            func_read(NULL, rbuf, r_len);
        }

        crc_interval = CRC16_with_initval(rbuf, r_len, crc_interval);

        file_addr += r_len;
        file_len -= r_len;
    }

    if (crc_interval != target_crc) {
        ret = GX_APP_UPDATE_ERR_FILE_DATA_VERIFY;
    }

_file_data_verify_end:
    if (rbuf) {
        free(rbuf);
    }

    return ret;
}

static int __gx8002_ufw_file_head_verify(gx8002_file_head_t *file_head)
{
    int ret = GX_APP_UPDATE_ERR_NONE;
    u16 calc_crc = CRC16(&(file_head->data_crc), sizeof(gx8002_file_head_t) - 2);

    if (calc_crc != file_head->head_crc) {
        ret = GX_APP_UPDATE_ERR_FILE_DATA_VERIFY;
    }

    return ret;
}



static int gx8002_ufw_file_verify(u32 ufw_addr, update_op_api_t *file_ops)
{
    int ret = 0;
    u16 r_len = 0;
    u8 file_cnt = 0;
    gx8002_file_head_t *file_head;
    u8 *rbuf = (u8 *)malloc(GX8002_UFW_VERIFY_BUF_LEN);
    if (rbuf == NULL) {
        ret = GX_APP_UPDATE_ERR_NOMEM;
        goto _verify_end;
    }

    file_ops->f_seek(NULL, SEEK_SET, ufw_addr);
    r_len = file_ops->f_read(NULL, rbuf, GX8002_UFW_VERIFY_BUF_LEN);

    if ((u16) - 1 == r_len) {
        ret = GX_APP_UPDATE_ERR_FILE_READ;
        ota_upgrade_debug("f_read err");
        goto _verify_end;
    }

    file_head = (gx8002_file_head_t *)rbuf;
    ret = __gx8002_ufw_file_head_verify(file_head);
    if (ret != GX_APP_UPDATE_ERR_NONE) {
        ota_upgrade_debug("gx8002 ufw file head verify err");
        goto _verify_end;
    }

    if (strcmp(file_head->name, "gx8002")) {
        ret = GX_APP_UPDATE_ERR_FILE_HEAD_VERIFY;
        ota_upgrade_debug("gx8002 ufw file head name err");
        goto _verify_end;
    }

    ota_upgrade_info("find: %s", file_head->name);
    do {
        file_head++;
        ret = __gx8002_ufw_file_head_verify(file_head);
        if (ret != GX_APP_UPDATE_ERR_NONE) {
            break;
        }

        if (strcmp(file_head->name, "gx8002.boot") == 0) {
            ota_upgrade_info("find: %s", file_head->name);
            memcpy(&(__this->file_boot_head), file_head, sizeof(gx8002_file_head_t));
#if GX8002_UFW_FILE_DATA_VERIFY_ENABLE
            file_ops->f_seek(NULL, SEEK_SET, __this->ufw_addr + file_head->addr);
            ret = __gx8002_ufw_file_data_verify(__this->ufw_addr + file_head->addr, file_head->len, file_head->data_crc, file_ops->f_read);
            if (ret == GX_APP_UPDATE_ERR_NONE) {
                file_cnt++;
            } else {
                ota_upgrade_debug("gx8002.boot file head verify err");
                break;
            }
#else
            file_cnt++;
#endif /* #if GX8002_UFW_FILE_DATA_VERIFY_ENABLE */
        }
        if (strcmp(file_head->name, "mcu_nor.bin") == 0) {
            ota_upgrade_info("find: %s", file_head->name);
            memcpy(&(__this->file_bin_head), file_head, sizeof(gx8002_file_head_t));
#if GX8002_UFW_FILE_DATA_VERIFY_ENABLE
            file_ops->f_seek(NULL, SEEK_SET, __this->ufw_addr + file_head->addr);
            ret = __gx8002_ufw_file_data_verify(__this->ufw_addr + file_head->addr, file_head->len, file_head->data_crc, file_ops->f_read);
            if (ret == GX_APP_UPDATE_ERR_NONE) {
                file_cnt++;
            } else {
                ota_upgrade_debug("mcu_nor.bin file head verify err");
                break;
            }
#else
            file_cnt++;
#endif /* #if GX8002_UFW_FILE_DATA_VERIFY_ENABLE */
        }
    } while (file_head->index == 0);

    if ((ret != GX_APP_UPDATE_ERR_NONE) && (file_cnt != 2)) {
        ret = GX_APP_UPDATE_ERR_FILE_DATA_VERIFY;
        ota_upgrade_debug("file_cnt: %d err", file_cnt);
        goto _verify_end;
    }

    ota_upgrade_info("find file_cnt: %d, verify ok", file_cnt);

_verify_end:
    if (rbuf) {
        free(rbuf);
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
        ota_upgrade_info("status info: %s\n", reply_str);
    }
}

static int gx8002_upgrade_task(void *param)
{
    int ret = 0;

    ota_upgrade_info("---- gx8002 ota start ----");
    gx8002_upgrade_cold_reset();

    ret = gx_uart_upgrade_proc();

    gx8002_normal_cold_reset();
    ota_upgrade_info("---- gx8002 ota over ----");

    if (ret < 0) {
        gx8002_update_end_post_msg(0);
    } else {
        gx8002_update_end_post_msg(1);
    }

    return ret;
}


//===================================================================//
/*
  @breif: ota流程回调: 初始化函数, 公共流程
  @parma: addr: 访问外置芯片固件地址
  @parma: file_ops: 访问外置芯片固件文件操作接口
  @return: 1) GX_APP_UPDATE_ERR_NONE = 0: 初始化成功
  		   2) < 0: 初始化失败
 */
//===================================================================//
int gx8002_app_ota_start(int (*complete_callback)(void))
{
    int ret = GX_APP_UPDATE_ERR_NONE;

    ota_upgrade_debug("%s", __func__);

    if (__this->cbuf_handle == NULL) {
        __this->cbuf_handle = (cbuffer_t *)malloc(sizeof(cbuffer_t) + (UPGRADE_PACKET_SIZE * STREAM_FIFO_BLOCK_TOTAL));
        if (__this->cbuf_handle == NULL) {
            ret = GX_APP_UPDATE_ERR_NOMEM;
            return ret;
        }
        cbuf_init(__this->cbuf_handle, __this->cbuf_handle + 1, (UPGRADE_PACKET_SIZE * STREAM_FIFO_BLOCK_TOTAL));
    }

    os_sem_create(&(__this->rx_sem), 0);

    os_taskq_post_msg("gx8002", 2, GX8002_MSG_UPDATE, GX8002_UPDATE_TYPE_APP);

    if (complete_callback) {
        if (complete_callback()) {
            return GX_APP_UPDATE_ERR_UPDATE_LOOP;
        }
    }

    return ret;
}

//===================================================================//
/*
  @breif: ota流程回调: 释放资源
  @parma: void
  @return: 1) GX_APP_UPDATE_ERR_NONE = 0: 初始化成功
  		   2) < 0: 初始化失败
 */
//===================================================================//
static int gx8002_app_ota_close(void)
{
    if (__this->update_role == GX8002_UPDATE_ROLE_TWS_SLAVE) {
        //从机出口
        gx8002_uart_app_ota_tws_update_sync_result(__this->update_result);

        if (__this->cbuf_handle) {
            free(__this->cbuf_handle);
            __this->cbuf_handle = NULL;
        }

        if (__this) {
            free(__this);
            __this = NULL;
        }
        gx8002_uart_app_ota_tws_update_close();
    } else {
        os_sem_post(&(__this->ota_sem));
    }

    return 0;
}

//===================================================================//
/*
  @breif: 从机升级开始
  @parma: void
  @return: 1) GX_APP_UPDATE_ERR_NONE = 0: 初始化成功
  		   2) < 0: 初始化失败
 */
//===================================================================//
int gx8002_app_ota_passive_start(int (*complete_callback)(void), u8 *ota_data, u16 len)
{
    int ret = GX_APP_UPDATE_ERR_NONE;

    ota_upgrade_debug("%s", __func__);
    if (__this) {
        return GX_APP_UPDATE_ERR_UPDATE_STATE;
    }

    __this = zalloc(sizeof(struct app_ota_info));
    if (__this == NULL) {
        return GX_APP_UPDATE_ERR_NOMEM;
    }

    ASSERT(len == sizeof(gx8002_file_head_t));

    memcpy((u8 *) & (__this->file_bin_head), ota_data, sizeof(gx8002_file_head_t));

    __this->update_role = GX8002_UPDATE_ROLE_TWS_SLAVE;

    ret = gx8002_app_ota_start(complete_callback);

    if (ret) {
        gx8002_app_ota_close();
    }

    return ret;
}

//===================================================================//
/*
  @breif: ota流程回调: 初始化函数, 主机流程
  @parma: priv: 访问外置芯片固件地址
  @parma: file_ops: 访问外置芯片固件文件操作接口
  @return: 1) GX_APP_UPDATE_ERR_NONE = 0: 初始化成功
  		   2) < 0: 初始化失败
 */
//===================================================================//
static int gx8002_chip_update_init(void *priv, update_op_api_t *file_ops)
{
    int ret = GX_APP_UPDATE_ERR_NONE;

    if (__this) {
        return GX_APP_UPDATE_ERR_UPDATE_STATE;
    }

    user_chip_update_info_t *update_info = (user_chip_update_info_t *)priv;

    __this = zalloc(sizeof(struct app_ota_info));
    if (__this == NULL) {
        return GX_APP_UPDATE_ERR_NOMEM;
    }

    __this->ufw_addr = update_info->addr;
    __this->file_ops = file_ops;
    __this->update_role = GX8002_UPDATE_ROLE_NORMAL;
    __this->update_result = GX_APP_UPDATE_ERR_NONE;

    __this->cur_seek_file = FW_FLASH_MAX;

    ota_upgrade_info("%s: ufw_addr = 0x%x", __func__, __this->ufw_addr);

    ret = gx8002_ufw_file_verify(__this->ufw_addr, file_ops);

    ota_upgrade_info("%s, ret = %d", __func__, ret);

    return ret;
}

//===================================================================//
/*
  @breif: ota流程回调: 获取升级问文件长度, 用于计算升级进度, 主机调用
  @parma: void
  @return: 1) 升级长度(int)
  		   2) = 0: 升级状态出错
 */
//===================================================================//
static int gx8002_chip_update_get_len(void)
{
    int update_len = 0;

    if (__this == NULL) {
        return 0;
    }

    update_len = __this->file_boot_head.len + __this->file_bin_head.len;
    ota_upgrade_info("%s: boot_len = 0x%x, bin_len = 0x%x, update_len = 0x%x", __func__, __this->file_boot_head.len, __this->file_bin_head.len, update_len);

    return update_len;
}

//===================================================================//
/*
  @breif: ota流程回调: 升级主流程, 主机调用
  @parma: void *priv 用于tws同步升级接口(update_op_tws_api_t), 非tws同步升级设置未NULL即可
  @return: 1) = 0: 升级成功
  		   2) < 0: 升级失败
 */
//===================================================================//
static int gx8002_chip_update_loop(void *priv)
{
    int ret = 0;

    if (__this == NULL) {
        return 0;
    }

    ota_upgrade_debug("%s", __func__);

    int (*start_callback)(void) = NULL;

    if (priv) {
        __this->update_role = GX8002_UPDATE_ROLE_TWS_MASTER;

        ret = gx8002_uart_app_ota_tws_update_init(priv);
        if (ret) {
            goto __update_end;
        }
        gx8002_uart_app_ota_tws_update_sync_start((u8 *) & (__this->file_bin_head), sizeof(gx8002_file_head_t));

        start_callback = gx8002_uart_app_ota_tws_update_sync_pend;
    }

    os_sem_create(&(__this->ota_sem), 0);

    ret = gx8002_app_ota_start(start_callback);
    if (ret != GX_APP_UPDATE_ERR_NONE) {
        goto __update_end;
    }

    //update 线程接收数据
    ret = gx8002_ota_data_receive((u8 *)(__this->packet_buf), sizeof(__this->packet_buf));
    if (ret != GX_APP_UPDATE_ERR_NONE) {
        goto __update_end;
    }

    //等待升级完成
    os_sem_pend(&(__this->ota_sem), 0);

    if (__this->update_result != GX_APP_UPDATE_ERR_NONE) {
        ota_upgrade_info("gx8002 update failed.");
    } else {
        ota_upgrade_info("gx8002 update succ.");
    }

    ret = __this->update_result;

__update_end:
    if (priv) {
        ret |= gx8002_uart_app_ota_tws_update_sync_result_get();
        gx8002_uart_app_ota_tws_update_close();
    }
    if (__this->cbuf_handle) {
        free(__this->cbuf_handle);
        __this->cbuf_handle = NULL;
    }
    if (__this) {
        free(__this);
        __this = NULL;
    }

    return ret;
}


//===================================================================//
/*
  @breif: 需要升级流程提供注册接口
  @parma: user_chip_update_file_name: 外置芯片升级文件名, 包含在update.ufw文件中
  @parma: user_update_handle: 当update.ufw存在升级文件时回调该函数升级外置芯片固件
  			@parma: addr: 外置芯片固件在update.ufw升级文件中地址
  			@parma: file_ops: 访问外置芯片固件文件操作接口
 */
//===================================================================//
static const user_chip_update_t gx8002_user_chip_update_instance = {
    .file_name = "gx8002.bin",
    .update_init = gx8002_chip_update_init,
    .update_get_len = gx8002_chip_update_get_len,
    .update_loop = gx8002_chip_update_loop,
};

//===================================================================//
/*
  @breif: 注册ota流程回调, 提供给gx8002线程调用
  @parma: void
  @return: void
 */
//===================================================================//
void gx8002_uart_app_ota_update_register_handle(void)
{
    register_user_chip_update_handle(&gx8002_user_chip_update_instance);

#if GX8002_UPGRADE_APP_TWS_TOGGLE
    gx8002_uart_app_ota_tws_update_register_handle();
#endif /* #if GX8002_UPGRADE_APP_TWS_TOGGLE */
}

//===================================================================//
/*
  @breif: 外置芯片启动升级流程, 提供给gx8002线程调用
  @parma: void
  @return: 1) 0: 升级成功
           2) -1: 升级失败
 */
//===================================================================//
int gx8002_uart_app_ota_init(void)
{
    int ret = 0;
    upgrade_uart_t uart_ops;

    ota_upgrade_info("gx8002_uart_app_ota start >>>>");

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

    ret = gx8002_upgrade_task(NULL);

    if (ret < 0) {
        __this->update_result = GX_APP_UPDATE_ERR_UPDATE_LOOP;
    } else {
        __this->update_result = GX_APP_UPDATE_ERR_NONE;
    }

    gx8002_app_ota_close();

    return ret;
}

#else /* #if GX8002_UPGRADE_APP_TOGGLE */

int gx8002_uart_app_ota_init(void)
{
    return 0;
}

#endif /* #if GX8002_UPGRADE_APP_TOGGLE */

