#ifndef _UPDATE_H_
#define _UPDATE_H_

#include "typedef.h"

extern u32 UPDATA_BEG;

#define UPDATA_FLAG_ADDR		((void *)((u32)&UPDATA_BEG + 0x08))
#define BOOT_STATUS_ADDR		((void *)((u32)&UPDATA_BEG)) //预留8个bytes

#define UPDATA_MAGIC            (0x5A00)        //防止CRC == 0 的情况

typedef enum {
    UPDATA_NON = UPDATA_MAGIC,
    UPDATA_READY,
    UPDATA_SUCC,
    UPDATA_PARM_ERR,
    UPDATA_DEV_ERR,
    UPDATA_KEY_ERR,
} UPDATA_RESULT;

typedef enum {
    USB_UPDATA = UPDATA_MAGIC,      //0x5A00
    SD0_UPDATA,                     //0x5A01
    SD1_UPDATA,
    PC_UPDATA,
    UART_UPDATA,
    BT_UPDATA,
    BLE_APP_UPDATA,
    SPP_APP_UPDATA,
    DUAL_BANK_UPDATA,
    BLE_TEST_UPDATA,
    NORFLASH_UPDATA,
    //NOTE:以上的定义不要调整,新升级方式在此添加,注意加在USER_NORFLASH_UFW_UPDATA之前;
    USER_NORFLASH_UFW_UPDATA,

    NON_DEV = 0xFFFF,
} UPDATA_TYPE;

// sd
enum {
    SD_CONTROLLER_0 = 1,
    SD_CONTROLLER_1,
};
enum {
    SD0_IO_A = 1,
    SD0_IO_B,
    SD1_IO_A,
    SD1_IO_B,
    SD0_IO_C,
    SD0_IO_D,
    SD0_IO_E,
    SD0_IO_F,
};
typedef struct _UPDATA_SD {
    u8 control_type;
    u8 control_io;
    u8 online_check_way;
    u8 max_data_baud;
    u16 wDevTimeOutMax;
    u8 per_online_status;
    u8 hc_mode;
    u8(*io_det_func)(void);
    u8 power;
    u8 control_io_clk;
    u8 control_io_cmd;
    u8 control_io_dat;
} UPDATA_SD;

// uart
typedef struct _UPDATA_UART {
    u32  control_io_tx;      //<IO口对接
    u32  control_io_rx;      //<IO口对接
    u32  control_baud;        //<波特率
    u32  control_timeout;  //<超时，单位10ms
} UPDATA_UART; /*共12个bytes*/


#define UPDATE_PARAM_MAGIC		0x5441
#if (USE_SDFILE_NEW == 1)
typedef struct _UPDATA_PARM {
    u16 parm_crc;
    u16 parm_type;              //UPDATA_TYPE:sdk pass parm to uboot
    u16 parm_result;            //UPDATA_TYPE:uboot return result to sdk
    u16 magic;					//0x5441
    //8byte
    union {
        struct {
            u8  file_path[32];         //updata file path
        };
        struct {
            u8  file_patch[32];         //updata file path
        };
    };
    u8  parm_priv[32];          //sd updata
    //64byte
    u32 ota_addr;
    u16 ext_arg_len;
    u16 ext_arg_crc;
    //8 byte
} UPDATA_PARM;

enum EXT_ARG_TYPE {
    EXT_LDO_TRIM_RES = 0,
    EXT_JUMP_FLAG,
    EXT_TYPE_MAX = 0xff,
};

struct ext_arg_t {
    u8 type;
    u8 len;
    u8 *data;
};

//8+64+8+32 =112;
#else
typedef struct _UPDATA_PARM {
    u16 parm_crc;
    u16 parm_type;              //UPDATA_TYPE:sdk pass parm to uboot
    u16 parm_result;            //UPDATA_TYPE:uboot return result to sdk
    u8  file_patch[32];         //updata file patch
    u8  parm_priv[32];          //sd updata
} UPDATA_PARM;
#endif
#define UPDATE_PRIV_PARAM_LEN	32

void update_mode_api_v2(UPDATA_TYPE type, void (*priv_param_fill_hdl)(UPDATA_PARM *p), void (*priv_update_jump_handle)(int type));
void update_param_priv_fill(UPDATA_PARM *p, void *priv, u16 priv_len);
u16 update_result_get(void);
bool device_is_first_start();
int update_result_deal();
void update_result_set(u16 result);
void update_clear_result();
bool update_success_boot_check(void);
typedef u8(*update_handler_t)(void);

typedef enum _UPDATE_STATE_T {
    UPDATE_TASK_INIT,
    UPDATE_CH_INIT,
    UPDATE_CH_SUCESS_REPORT,
    UPDATE_CH_EXIT,
} UPDATE_STATE_T;

struct update_target {
    char *name;
    update_handler_t driver_close;
};

//void update_param_private_fill_handle_register(void (*handle)(UPDATA_PARM *p));

#define REGISTER_UPDATE_TARGET(target) \
        const struct update_target target sec(.update_target)


extern const struct update_target update_target_begin[];
extern const struct update_target update_target_end[];

#define list_for_each_update_target(p) \
    for (p = update_target_begin; p < update_target_end; p++)

#endif
