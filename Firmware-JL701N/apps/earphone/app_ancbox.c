#include "init.h"
#include "app_config.h"
#include "system/includes.h"
#include "btstack/avctp_user.h"
#include "include/earphone.h"

#if TCFG_ANC_BOX_ENABLE

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".anc_box_bss")
#pragma data_seg(".anc_box_data")
#pragma const_seg(".anc_box_const")
#pragma code_seg(".anc_box_code")
#endif/*SUPPORT_MS_EXTENSIONS*/

#include "audio_anc.h"
#include "asm/charge.h"
#include "asm/chargestore.h"
#include "user_cfg.h"
#include "device/vm.h"
#include "app_action.h"
#include "app_main.h"
#include "app_ancbox.h"
#include "app_power_manage.h"
#include "app_chargestore.h"
#include "anc_btspp.h"
#include "user_cfg.h"
#include "audio_config.h"
#include "audio_codec_clock.h"

#if AUDIO_ENC_MPT_SELF_ENABLE
#include "audio_enc_mpt_self.h"
#endif/*AUDIO_ENC_MPT_SELF_ENABLE*/

#if TCFG_AUDIO_CVP_DUT_ENABLE
#include "audio_cvp_dut.h"
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#include "icsd_adt_app.h"
#endif

#if AUDIO_SPK_EQ_CONFIG
#include "audio_dec_eff.h"
#endif

#define LOG_TAG_CONST       APP_ANCBOX
#define LOG_TAG             "[APP_ANCBOX]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define ANC_VERSION             1//有必要时才更新该值(例如结构体数据改了)

enum {
    FILE_ID_COEFF = 0,  		//系数文件ID号
    FILE_ID_MIC_SPK = 1,		//mic spk文件ID号
    FILE_ID_GAIN = 2,   		//GAIN数据
    FILE_ID_ADAPTIVE = 3,		//自适应模式文件ID
    FILE_ID_ADAPTIVE_REF = 4,	//自适应模式金机曲线ID
    //FILE_ID_SPKEQ = 5,     		//EQ数据
    FILE_ID_SZ_FFT = 6,     	//SZ频响数据
    FILE_ID_MIC_FFT = 7,     	//MIC声学频响数据
    FILE_ID_COEFF_FGQ = 8,     	//新系数文件-FGQ ID号
    FILE_ID_COEFF_FGQ_PART = 9,	//新部分系数文件-FGQ ID号
    FILE_ID_SPKEQ_DATA_L = 10,	//SPK_EQ数据左耳
    FILE_ID_SPKEQ_DATA_R = 11,	//SPK_EQ数据右耳
};

struct _ear_parm {
    u8  ts;         //训练步进
    u8  mode;       //训练模式(普通/快速)
    u16 emmtv;      //误差mic下限阈值
    u16 rmmtv;      //参考mic下限阈值
    u8  sdt;        //静音检测时间
    u8  ntt;        //噪声训练时间
    u8  emstt;      //误差mic静音训练时间
    u8  rmstt;      //参考mic静音训练时间
    u8  c1tto;      //系数一次校准超时时间
    u8  c2tto;      //系数二次校准超时时间
    u8  gen;        //增益配置使能位
    u8  gdac;       //dac增益
    s8  grmic;      //参考mic增益
    s8  gemic;      //误差mic增益
    s16 ganc;       //降噪增益
    s16 gpass;      //通透增益
};

struct _ancbox_info {
    u8 ancbox_status;//anc测试盒在线状态
    u8 err_flag;
    u8 status;
    u8 test_flag;
    u8 baud_counter;
    u8 sz_nadap_pow;
    u8 sz_adap_pow;
    u8 sz_mute_pow;
    u8 fz_nadap_pow;
    u8 fz_adap_pow;
    u8 fz_mute_pow;
    u8 wz_nadap_pow;
    u8 wz_adap_pow;
    u8 train_step_succ;
#if ANCTOOL_NEED_PAIR_KEY
    u8 pair_flag;
#endif
    u8 audio_enc_mpt_fre_ch;
    u8 fft_file_busy;	//MIC_FFT SZ_FFT run状态获取: 1 正在run,  0 已停止
    anc_train_para_t *para;
    u8 *coeff;
    u32 coeff_ptr;
    u8 idle_flag;
    int anc_timer;
    int baud_timer;
    u8 *file_hdl;
    u32 file_id;
    u32 file_len;
    u32 coeff_len;
    u32 data_offset;
    u32 data_len;
    u8 *data_ptr;
    u32 type_id;
    u32 type_value;
    anc_gain_t gain;
};

#define DEFAULT_BAUDRATE            9600
enum {
    CMD_ANC_STATUS = 0x00,  	   //获取状态
    CMD_ANC_SET_START = 0x01,      //ANC训练开始
    CMD_ANC_MUTE_TRAIN = 0x02,     //静音训练
    CMD_ANC_SIN_TRAIN = 0x03,      //找步进训练
    CMD_ANC_NOISE_TRAIN = 0x04,    //噪声训练
    CMD_ANC_TEST_ANC_ON = 0x05,    //ANC ON
    CMD_ANC_TEST_ANC_OFF = 0x06,   //ANC OFF
    CMD_ANC_SET_STOP = 0x07,       //停止训练
    CMD_ANC_SET_PARM = 0x08,       //设置参数
    CMD_ANC_SYS_RESET = 0x09,      //复位系统
    CMD_ANC_SET_RUN_MODE = 0x0A,   //设置ANC运行模式
    CMD_ANC_TEST_PASS = 0x0B,      //设置为通透
    CMD_ANC_CHANGE_BAUD = 0x0C,    //切换波特率
    CMD_ANC_DEVELOPER_MODE = 0x0D, //开发者模式
    CMD_ANC_ADAP = 0x0E,           //开关自适应
    CMD_ANC_WZ_BREAK = 0x0F,       //退出噪声训练
    CMD_ANC_SET_TS = 0x10,         //单独设置步进
    CMD_ANC_TRAIN_STEP = 0x11,     //设置训练步进
    CMD_ANC_GET_TRAIN_STEP = 0x12, //获取训练步进
    CMD_ANC_GET_MUTE_TRAIN_POW = 0x13,//获取静音训练的收敛数据
    CMD_ANC_GET_NOISE_TRAIN_POW = 0x14,//获取噪声训练的收敛数据
    CMD_ANC_GET_TRAIN_POW = 0x15,  //用于手动训练时实时读取收敛值
    CMD_ANC_SET_ID = 0x16,         //根据ID设置参数
    CMD_ANC_GET_ID = 0x17,         //根据ID读取参数
    CMD_ANC_TEST_BYPASS = 0x18,    //设置bypass模式
    CMD_ANC_CHANGE_MODE = 0x19,    //切换ANC模式 = 0x01, FF/FB/HB
    CMD_ANC_GET_COEFF_SIZE = 0x1A, //获取系数大小
    CMD_ANC_MUTE_TRAIN_ONLY = 0x1B,//单独进行静音训练
    CMD_ANC_GET_VERSION = 0x1C,    //获取芯片版本号
    CMD_ANC_TRANS_TRAIN = 0x1D,		//单独进行通透训练
    CMD_ANC_CREATE_SPK_MIC = 0x1E, //触发生成SPK MIC数据
    CMD_ANC_GET_ANC_CH = 0x1F,     //获取耳机的通道 = 0x01,左声道/右声道/双声道
    CMD_ANC_PAIR_KEY = 0x20,       //和耳机进行密码验证 = 0x01,匹配才能读系数
    CMD_ANC_PAIR_SUCC = 0x21,      //绕过配对过程
    CMD_ANC_GET_MAX_ORDER = 0x22,	//获取ANC最大滤波器阶数
    CMD_ANC_GET_HEARAID_EN = 0x23,	//
    //透传指令START
    CMD_ANC_SZ_FFT_START = 0x51,	//SZ_FFT计算开始
    CMD_ANC_SZ_FFT_STOP = 0x52,		//SZ_FFT计算停止
    CMD_ANC_MIC_FFT_START = 0x53,	//MIC_FFT计算停止
    CMD_ANC_MIC_FFT_STOP = 0x54,	//MIC_FFT计算停止
    CMD_MUTL_SCENE_SET = 0X55, 		//ANC多场景设置
    CMD_FFT_FILE_BUSY_GET = 0X56,	//FFT flie busy状态获取
    //透传指令END

    CMD_ANC_GET_MAC = 0x80, //获取MAC地址
    CMD_ANC_TOOLS_SYNC,     //同步信号,同CMD_ANC_STATUS一样意思
    CMD_ANC_SET_LINKKEY,    //设置linkkey
    CMD_ANC_SW_TO_BT,       //切到蓝牙模式
    CMD_ANC_READ_COEFF,     //开始读取系数
    CMD_ANC_READ_COEFF_CONTINUE,//连续读取系数
    CMD_ANC_WRITE_COEFF,    //开始写系数
    CMD_ANC_WRITE_COEFF_CONTINUE,//连续写系数
    CMD_ANC_GET_RESULT,     //获取训练结果
    CMD_ANC_SET_GAIN,       //设置gain
    CMD_ANC_GET_GAIN,       //读取gain
    CMD_ANC_READ_FILE_START,//根据ID号读文件开始
    CMD_ANC_READ_FILE_DATA, //根据ID号读文件数据
    CMD_ANC_WRITE_FILE_START,//根据ID号写文件开始
    CMD_ANC_WRITE_FILE_DATA,//跟据ID号写文件数据
    CMD_ANC_WRITE_FILE_END, //根据ID号写文件结束

    CMD_ANC_EQ_DATA = 0x90, //eq数据传输

    //透传指令START
    CMD_ANC_SET_MUSIC_VOL,  //设置音量
    CMD_ANC_NEW_EQ_DATA,    //eq调试新指令

    CMD_ANC_CVP_DUT_DATA = 0xA0, //CVP_DUT数据传输
    //透传指令END

    CMD_ANC_FAIL = 0xFE,
};

#define ANC_POW_SIZE    15
static u8 anc_pow[ANC_POW_SIZE];
static u8 eq_buffer[32];
static u8 eq_len;
static int cvp_dut_len;
static u8 *cvp_dut_buffer;
static struct _ancbox_info ancbox_info;
#define __this  (&ancbox_info)
extern void sys_auto_shut_down_disable(void);
extern void sys_auto_shut_down_enable(void);
extern void bt_update_mac_addr(u8 *addr);
extern void set_temp_link_key(u8 *linkkey);
extern void chargestore_set_baudrate(u32 baudrate);
extern void anc_cfg_btspp_update(u8 id, int data);
extern int anc_cfg_btspp_read(u8 id, int *data);
extern int anc_mode_change_tool(u8 dat);

static void ancbox_callback(u8 mode, u8 command)
{
    log_info("ancbox_callback: %d, %d\n", mode, command);
    if (mode & 0x80) {//train step
        __this->err_flag = ANC_EXEC_SUCC;
        __this->train_step_succ = 1;
    } else {
        __this->err_flag = command;
    }

    if (__this->status == CMD_ANC_CREATE_SPK_MIC) {
        __this->status = CMD_ANC_SET_STOP;
    }
}

static void ancbox_pow_callback(anc_ack_msg_t *msg, u8 step)
{
    switch (step) {
    case ANC_TRAIN_SZ:
        __this->sz_nadap_pow = anc_powdat_analysis(msg->pow);
        __this->sz_adap_pow = anc_powdat_analysis(msg->temp_pow);
        __this->sz_mute_pow = anc_powdat_analysis(msg->mute_pow);
        break;
    case ANC_TRAIN_FZ:
        __this->fz_nadap_pow = anc_powdat_analysis(msg->pow);
        __this->fz_adap_pow = anc_powdat_analysis(msg->temp_pow);
        __this->fz_mute_pow = anc_powdat_analysis(msg->mute_pow);
        break;
    case ANC_TRAIN_WZ:
        __this->wz_nadap_pow = anc_powdat_analysis(msg->pow);
        __this->wz_adap_pow = anc_powdat_analysis(msg->temp_pow);
        if (msg->status == ANC_WZ_ADAP_STATUS) {
            for (u8 i = 0; i < (ANC_POW_SIZE - 1); i++) {
                anc_pow[i] = anc_pow[i + 1];
            }
            anc_pow[ANC_POW_SIZE - 1] = anc_powdat_analysis(msg->temp_pow);
        }
        break;
    }
}

static void anc_event_to_user(u8 event, u32 value)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg  = (void *)DEVICE_EVENT_FROM_ANC;
    e.u.ancbox.event = event;
    e.u.ancbox.value = value;
    sys_event_notify(&e);
}

static void anc_cmd_timeout_deal(void *priv)
{
    log_info("mabe takeup! sys reset!\n");
    cpu_reset();
}

static void anc_cmd_timeout_init(void)
{
    if (__this->anc_timer == 0) {
#if ANC_MIC_DMA_EXPORT
        __this->anc_timer = sys_timeout_add(NULL, anc_cmd_timeout_deal, 5000);
#else
        __this->anc_timer = sys_timeout_add(NULL, anc_cmd_timeout_deal, 1000);
#endif
    } else {
        sys_timer_modify(__this->anc_timer, 1000);
    }
}

static void anc_cmd_timeout_del(void)
{
    if (__this->anc_timer) {
        sys_timeout_del(__this->anc_timer);
        __this->anc_timer = 0;
    }
}

static void anc_baud_timer_deal(void *priv)
{
    __this->baud_counter++;
    if (__this->baud_counter > 20) {
        sys_timer_del(__this->baud_timer);
        __this->baud_timer = 0;
        __this->baud_counter = 0;
        log_info("timer out, set baud 9600!\n");
        chargestore_set_baudrate(DEFAULT_BAUDRATE);
    }
}

static void anc_wz_timeout_deal(void *priv)
{
    anc_train_api_set(ANC_TEST_WZ_BREAK, 0, __this->para);
}

static void anc_read_file_start(u8 *cmd, u32 id)
{
    u32 data_len;
    int *anc_debug_hdl;
    __this->file_id = id;
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
#if ANCTOOL_NEED_PAIR_KEY
        if (!__this->pair_flag) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
            return;
        }
#endif
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
        __this->file_len = anc_coeff_size_get(ANC_CFG_READ);
#else
        __this->file_len = anc_coeff_size_get();
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        __this->file_hdl = (u8 *)anc_coeff_read();
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_START;
        if (__this->file_hdl) {//判断耳机有没有系数
            memcpy(&cmd[2], (u8 *)&__this->file_len, 4);
            chargestore_api_write(cmd, 6);
            anc_cmd_timeout_del();
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
        break;
    case FILE_ID_MIC_SPK:
        //这里判断是否有数据
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
        anc_debug_hdl = (u8 *)anc_debug_ctr(0);
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_START;
        if (anc_debug_hdl) {
            __this->file_hdl = (u8 *)(anc_debug_hdl + 1); //数据指针
            __this->file_len = anc_debug_hdl[0];
            memcpy(&cmd[2], (u8 *)&__this->file_len, 4);
            chargestore_api_write(cmd, 6);
            anc_cmd_timeout_del();
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        break;
    case FILE_ID_GAIN:
        anc_cfg_online_deal(ANC_CFG_READ, &__this->gain);
        __this->file_len = sizeof(anc_gain_t);
        __this->file_hdl = (u8 *)&__this->gain;
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_START;
        memcpy(&cmd[2], (u8 *)&__this->file_len, 4);
        chargestore_api_write(cmd, 6);
        anc_cmd_timeout_del();
        break;
#if TCFG_ANC_SELF_DUT_GET_SZ
    case FILE_ID_SZ_FFT:
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_START;
        __this->file_len = audio_anc_sz_fft_outbuf_get((&__this->file_hdl));
        if (__this->file_hdl) {
            memcpy(&cmd[2], (u8 *)&__this->file_len, 4);
            chargestore_api_write(cmd, 6);
            anc_cmd_timeout_del();
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
        break;
#endif/*TCFG_ANC_SELF_DUT_GET_SZ*/

#if AUDIO_ENC_MPT_SELF_ENABLE
    case FILE_ID_MIC_FFT:
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_START;
        __this->file_len = audio_enc_mpt_fre_response_file_get((&__this->file_hdl));
        if (__this->file_hdl) {
            memcpy(&cmd[2], (u8 *)&__this->file_len, 4);
            chargestore_api_write(cmd, 6);
            anc_cmd_timeout_del();
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
        break;
#endif/*AUDIO_ENC_MPT_SELF_ENABLE*/
#if AUDIO_SPK_EQ_CONFIG
    case FILE_ID_SPKEQ_DATA_L:
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_START;
        __this->file_len = spk_eq_read_seg_l((&__this->file_hdl));
        if (__this->file_hdl) {
            memcpy(&cmd[2], (u8 *)&__this->file_len, 4);
            chargestore_api_write(cmd, 6);
            anc_cmd_timeout_del();
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
        break;
    case FILE_ID_SPKEQ_DATA_R:
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_START;
        __this->file_len = spk_eq_read_seg_r((&__this->file_hdl));
        if (__this->file_hdl) {
            memcpy(&cmd[2], (u8 *)&__this->file_len, 4);
            chargestore_api_write(cmd, 6);
            anc_cmd_timeout_del();
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
        break;
#endif
    }
}

static void anc_read_file_data(u8 *cmd, u32 offset, u32 data_len)
{
    u32 read_len;
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
#if ANCTOOL_NEED_PAIR_KEY
        if (!__this->pair_flag) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
            return;
        }
#endif
#if AUDIO_SPK_EQ_CONFIG
    case FILE_ID_SPKEQ_DATA_L:
    case FILE_ID_SPKEQ_DATA_R:
#endif/*AUDIO_SPK_EQ_CONFIG*/
    case FILE_ID_GAIN:
        if (__this->file_hdl == NULL) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        } else {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_READ_FILE_DATA;
            memcpy(&cmd[2], __this->file_hdl + offset, data_len);
            chargestore_api_write(cmd, data_len + 2);
            if (__this->file_len == offset + data_len) {
                __this->file_hdl = NULL;
            }
        }
        break;
    case FILE_ID_MIC_SPK:
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_DATA;
        if (__this->file_hdl) {
            memcpy(&cmd[2], __this->file_hdl + offset, data_len);
            chargestore_api_write(cmd, data_len + 2);
            if (__this->file_len == offset + data_len) {
                __this->file_hdl = (u8 *)anc_debug_ctr(1);
            }
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        break;
#if TCFG_ANC_SELF_DUT_GET_SZ
    case FILE_ID_SZ_FFT:
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_DATA;
        if (__this->file_hdl) {
            memcpy(&cmd[2], __this->file_hdl + offset, data_len);
            chargestore_api_write(cmd, data_len + 2);
            if (__this->file_len == offset + data_len) {
                audio_anc_sz_fft_outbuf_release();
                audio_codec_clock_del(ANC_DUT_SZ_FFT_MODE);
                __this->file_hdl = NULL;
            }
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
        break;
#endif/*TCFG_ANC_SELF_DUT_GET_SZ*/
#if AUDIO_ENC_MPT_SELF_ENABLE
    case FILE_ID_MIC_FFT:
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_FILE_DATA;
        if (__this->file_hdl) {
            memcpy(&cmd[2], __this->file_hdl + offset, data_len);
            chargestore_api_write(cmd, data_len + 2);
            if (__this->file_len == offset + data_len) {
                audio_enc_mpt_fre_response_release();
                __this->file_hdl = NULL;
            }
        } else {
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
        break;
#endif/*AUDIO_ENC_MPT_SELF_ENABLE*/
    }
}

static void anc_write_file_start(u8 *cmd, u32 id, u32 data_len)
{
    __this->file_id = id;
    if (__this->file_hdl) {
        free(__this->file_hdl);
        __this->file_hdl = NULL;
    }
    if (__this->data_ptr) {
        free(__this->data_ptr);
        __this->data_ptr = NULL;
    }

    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
    case FILE_ID_COEFF_FGQ_PART:
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
        cmd[0] = CMD_ANC_MODULE;
        anc_coeff_size_set(ANC_CFG_WRITE, data_len);			//设置长度写anc_coeff.bin
        __this->file_len = data_len;
#else
        __this->file_len = anc_coeff_size_get();
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        __this->file_hdl = (u8 *)malloc(__this->file_len);
        __this->data_ptr = (u8 *)malloc(32);
        if ((__this->file_len != data_len) || (__this->file_hdl == NULL) || (__this->data_ptr == NULL)) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
            log_info("anc coeff size err, %d != %d\n", data_len, __this->file_len);
            if (__this->file_hdl) {
                free(__this->file_hdl);
                __this->file_hdl = NULL;
            }
            if (__this->data_ptr) {
                free(__this->data_ptr);
                __this->data_ptr = NULL;
            }
            return;
        } else {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_WRITE_FILE_START;
            chargestore_api_write(cmd, 2);
            anc_cmd_timeout_del();
        }
        break;
    case FILE_ID_GAIN:
        __this->file_len = sizeof(anc_gain_t);
        __this->file_hdl = (u8 *)&__this->gain;
        __this->data_ptr = (u8 *)malloc(32);
        if ((__this->file_len != data_len) || (__this->data_ptr == NULL)) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
            log_info("anc gain size err, %d != %d\n", data_len, __this->file_len);
            if (__this->data_ptr) {
                free(__this->data_ptr);
                __this->data_ptr = NULL;
            }
        } else {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_WRITE_FILE_START;
            chargestore_api_write(cmd, 2);
            anc_cmd_timeout_del();
        }
        break;
    }
}

static void anc_write_file_data(u8 *cmd, u32 offset, u8 *data, u32 data_len)
{
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
    case FILE_ID_COEFF_FGQ_PART:
    case FILE_ID_GAIN:
        if (__this->file_hdl == NULL || ((offset + data_len) > __this->file_len)) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        } else {
            memcpy(__this->file_hdl + offset, data, data_len);
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_WRITE_FILE_DATA;
            chargestore_api_write(cmd, 2);
        }
        break;
    }
}

static int app_anc_write_file_end(u32 file_id)
{
    int ret;
    switch (file_id) {
    case FILE_ID_COEFF:
        ret = anc_coeff_write((int *)__this->file_hdl, __this->file_len);
        break;
#if ANC_MULT_ORDER_ENABLE
    case FILE_ID_COEFF_FGQ:
        ret = audio_anc_mult_coeff_write(ANC_MULT_COEFF_FILL_ALL, (int *)__this->file_hdl, __this->file_len);
        break;
    case FILE_ID_COEFF_FGQ_PART:
        ret = audio_anc_mult_coeff_write(ANC_MULT_COEFF_FILL_PART, (int *)__this->file_hdl, __this->file_len);
        break;
#endif/*ANC_MULT_ORDER_ENABLE*/
    }
    free(__this->file_hdl);
    free(__this->data_ptr);
    __this->file_hdl = NULL;
    __this->data_ptr = NULL;
    return ret;
}


static void anc_write_file_end(u8 *cmd)
{
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
    case FILE_ID_COEFF_FGQ_PART:
        if (__this->file_hdl == NULL) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        } else {
            anc_cmd_timeout_del();
            if (app_anc_write_file_end(__this->file_id)) {
                cmd[1] = CMD_ANC_FAIL;
            } else {
                cmd[1] = CMD_ANC_WRITE_FILE_END;
            }
            cmd[0] = CMD_ANC_MODULE;
            chargestore_api_write(cmd, 2);
        }
        break;
    case FILE_ID_GAIN:
        if (__this->file_hdl == NULL) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        } else {
            anc_cfg_online_deal(ANC_CFG_WRITE, &__this->gain);
            free(__this->data_ptr);
            __this->file_hdl = NULL;
            __this->data_ptr = NULL;
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_WRITE_FILE_END;
            chargestore_api_write(cmd, 2);
        }
        break;
    }
}

#if ANCTOOL_NEED_PAIR_KEY
static void anc_ack_pair_key(u8 *key)
{
    u8 cmd[2];
    if (strcmp(ANCTOOL_PAIR_KEY, (const char *)key) == 0) {
        __this->pair_flag = 1;
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_PAIR_KEY;
        chargestore_api_write(cmd, 2);
    } else {
        __this->pair_flag = 0;
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_FAIL;
        chargestore_api_write(cmd, 2);
    }
}
#endif

static void anc_ack_spk_eq(u8 seq, u8 *packet, u8 len)
{
    u8 cmd[32];
    if (len > 30) {
        log_error("anc_ack_spk_eq packet len err! %d\n", len);
        return;
    }
    cmd[0] = CMD_ANC_MODULE;
    cmd[1] = CMD_ANC_NEW_EQ_DATA;
    memcpy(cmd + 2, packet, len);
    chargestore_api_write(cmd, len + 2);
}

int app_ancbox_event_handler(struct ancbox_event *anc_dev)
{
    u8 cmd[32], mac_addr[6];
    u32 data_len;
    u32 anc_coeff_total_size;
    int ret;
    struct application *app = get_current_app();

    if (!get_total_connect_dev()) { //已经连接手机
        void sys_auto_shut_down_disable(void);
        void sys_auto_shut_down_enable(void);
        sys_auto_shut_down_disable();
        sys_auto_shut_down_enable();
    }

    switch (anc_dev->event) {
    case CMD_ANC_MUTE_TRAIN_ONLY://独立的训练模式
        log_info("event_CMD_ANC_MUTE_TRAIN_ONLY\n");
        if (__this->status != CMD_ANC_MUTE_TRAIN_ONLY) {
            __this->status = CMD_ANC_MUTE_TRAIN_ONLY;
            __this->err_flag = 0;
            sys_auto_shut_down_disable();
            anc_cmd_timeout_del();
            anc_train_api_set(ANC_MUTE_TARIN, 1, __this->para);
        }
        break;

    case CMD_ANC_TRANS_TRAIN://独立的训练模式
        log_info("event_CMD_ANC_TRANS_TRAIN\n");
        if (__this->status != CMD_ANC_TRANS_TRAIN) {
            __this->status = CMD_ANC_TRANS_TRAIN;
            __this->err_flag = 0;
            sys_auto_shut_down_disable();
            anc_cmd_timeout_del();
            anc_train_api_set(ANC_TRANS_MUTE_TARIN, 1, __this->para);
        }
        break;

    case CMD_ANC_CREATE_SPK_MIC:
        log_info("event_CMD_ANC_CREATE_SPK_MIC: %d\n", anc_dev->value);
        if (__this->status != CMD_ANC_CREATE_SPK_MIC) {
            __this->status = CMD_ANC_CREATE_SPK_MIC;
            __this->err_flag = 0;
            anc_train_api_set(ANC_MUTE_TARIN, anc_dev->value, __this->para);
            sys_auto_shut_down_disable();
            anc_cmd_timeout_del();
        }
        break;

    case CMD_ANC_SET_START:
        log_info("event_CMD_ANC_SET_STAR\n");
        __this->test_flag = 1;
        __this->status = CMD_ANC_SET_START;
        __this->err_flag = 0;
        sys_auto_shut_down_disable();
        anc_cmd_timeout_del();
        memset(anc_pow, 0, ANC_POW_SIZE);
        break;
    case CMD_ANC_MUTE_TRAIN:
        log_info("event_CMD_ANC_MUTE_TRAIN\n");
        if (__this->status != CMD_ANC_MUTE_TRAIN) {
            __this->status = CMD_ANC_MUTE_TRAIN;
            anc_train_api_set(ANC_MUTE_TARIN, 0, __this->para);
        }
        break;
    case CMD_ANC_SIN_TRAIN:
        log_info("event_CMD_ANC_SIN_TRAIN\n");
        if (__this->status != CMD_ANC_SIN_TRAIN) {
            __this->status = CMD_ANC_SIN_TRAIN;
            anc_train_api_set(ANC_TRAIN_STEP_1, 0, __this->para);
        }
        break;
    case CMD_ANC_NOISE_TRAIN:
        log_info("event_CMD_ANC_TRAIN_SPK_ON\n");
        if (__this->status != CMD_ANC_NOISE_TRAIN) {
            __this->status = CMD_ANC_NOISE_TRAIN;
            anc_train_api_set(ANC_NOISE_TARIN, 0, __this->para);
        }
        break;
    case CMD_ANC_TEST_ANC_ON:
        log_info("event_CMD_ANC_TEST_ANC_ON\n");
        if (__this->test_flag == 1) {
            __this->status = CMD_ANC_TEST_ANC_ON;
            anc_train_api_set(ANC_MODE_ON, 0, __this->para);
        } else {
            log_info("switch to ancon!\n");
            anc_api_set_fade_en(0);
            anc_mode_switch(ANC_ON, 0);
        }
        break;
    case CMD_ANC_TEST_ANC_OFF:
        log_info("event_CMD_ANC_TEST_ANC_OFF\n");
        if (__this->test_flag == 1) {
            __this->status = CMD_ANC_TEST_ANC_OFF;
            anc_train_api_set(ANC_MODE_OFF, 0, __this->para);
        } else {
            log_info("switch to ancoff!\n");
            anc_api_set_fade_en(0);
            anc_mode_switch(ANC_OFF, 0);
        }
        break;
    case CMD_ANC_TEST_PASS:
        log_info("event_CMD_ANC_TEST_PASS\n");
        if (__this->test_flag == 1) {
            __this->status = CMD_ANC_TEST_PASS;
            anc_train_api_set(ANC_PASS_MODE_ON, 0, __this->para);
        } else {
            log_info("switch to pass!\n");
            anc_api_set_fade_en(0);
            anc_mode_switch(ANC_TRANSPARENCY, 0);
        }
        break;
    case CMD_ANC_SET_STOP:
        log_info("event_CMD_ANC_SET_STOP\n");
        __this->status = CMD_ANC_SET_STOP;
        __this->err_flag = 0;
        if (__this->test_flag) {
            __this->test_flag = 0;
            anc_train_api_set(ANC_TRAIN_EXIT, 0, __this->para);
            sys_auto_shut_down_enable();
        }
        break;
    case CMD_ANC_SYS_RESET:
        log_info("event_CMD_ANC_SYS_RESET\n");
        os_time_dly(3);
        cpu_reset();
        break;
    case CMD_ANC_SET_RUN_MODE:
        log_info("event_CMD_ANC_SET_RUN_MODE = %d\n", anc_dev->value);
        anc_api_set_fade_en(0);
        if (anc_dev->value == 0) {
            anc_mode_switch(ANC_OFF, 0);
        } else if (anc_dev->value == 1) {
            anc_mode_switch(ANC_ON, 0);
        } else {
            anc_mode_switch(ANC_TRANSPARENCY, 0);
        }
        break;
    case CMD_ANC_TEST_BYPASS:
        log_info("event_CMD_ANC_TEST_BYPASS\n");
        anc_mode_switch(ANC_BYPASS, 0);
        //bypass,此处调用底层接口根据anc_dev->value需要切换到那个mic的bypass
        break;
    case CMD_ANC_CHANGE_MODE:
        log_info("event_CMD_ANC_CHANGE_MODE = %d\n", anc_dev->value);
        anc_mode_change_tool(anc_dev->value);
        //切换FF/FB,此处调用底层接口根据anc_dev->value需要切换到FF/FB
        break;
    case CMD_ANC_STATUS:
    case CMD_ANC_TOOLS_SYNC:
        putchar('S');
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
        /*关闭所有模块*/
        if (audio_icsd_adt_is_running()) {
            audio_icsd_adt_close_all();
        }
#endif
#if TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN
        //自适应还原为默认参数
        if (audio_anc_coeff_mode_get() == ANC_COEFF_MODE_ADAPTIVE) {
            audio_anc_coeff_adaptive_set(ANC_COEFF_MODE_NORMAL, 0, 0);
        }
#endif
#if TCFG_CHARGESTORE_PORT == LDOIN_BIND_IO
        if (app && strcmp(app->name, APP_NAME_IDLE)) {
            os_time_dly(1);
            log_info("not idle app, reset sys!\n");
            cpu_reset();
        } else {
            __this->idle_flag = 1;
        }
#endif
        anc_cmd_timeout_init();
        if (__this->para == NULL) {
            __this->para = anc_api_get_train_param();
            anc_api_set_callback(ancbox_callback, ancbox_pow_callback);
        }
        break;
    case CMD_ANC_SET_LINKKEY:
        log_info("CMD_ANC_SET_LINKKEY\n");
        if (app && strcmp(app->name, APP_NAME_BT)) {
            power_set_mode(TCFG_LOWPOWER_POWER_SEL);
            app_var.play_poweron_tone = 0;
            task_switch_to_bt();
            anc_cmd_timeout_del();
        }
        break;
    case CMD_ANC_SW_TO_BT:
        log_info("CMD_ANC_SW_TO_BT\n");
        bt_get_vm_mac_addr(mac_addr);
        bt_update_mac_addr(mac_addr);
        if (app && strcmp(app->name, APP_NAME_BT)) {
            power_set_mode(TCFG_LOWPOWER_POWER_SEL);
            app_var.play_poweron_tone = 0;
            task_switch_to_bt();
            anc_cmd_timeout_del();
        } else {
#if TCFG_CHARGESTORE_PORT != LDOIN_BIND_IO
            anc_cmd_timeout_del();
            //关闭回连
            EARPHONE_STATE_SET_PAGE_SCAN_ENABLE();
            user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
#endif
        }
        break;
    case CMD_ANC_READ_COEFF://read start
        log_info("CMD_ANC_READ_COEFF\n");
        __this->coeff = (u8 *)anc_coeff_read();
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_READ_COEFF;
        if (__this->coeff == NULL) {
            data_len = 0;
        } else {
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
            anc_coeff_total_size = anc_coeff_size_get(ANC_CFG_READ);
#else
            anc_coeff_total_size = anc_coeff_size_get();
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
            data_len = anc_coeff_total_size;
        }
        __this->coeff_ptr = 0;
        memcpy(&cmd[2], (u8 *)&data_len, 4);
        chargestore_api_write(cmd, 6);
        anc_cmd_timeout_del();
        break;
    case CMD_ANC_WRITE_COEFF://write start
        log_info("CMD_ANC_WRITE_COEFF\n");
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_WRITE_COEFF;
        __this->coeff_ptr = 0;
        if (__this->coeff == NULL) {
            __this->coeff = (u8 *)malloc(__this->coeff_len);
            if (__this->coeff == NULL) {
                cmd[1] = CMD_ANC_FAIL;
            }
        }
        chargestore_api_write(cmd, 2);
        anc_cmd_timeout_del();
        break;
    case CMD_ANC_WRITE_COEFF_CONTINUE:
        log_info("CMD_ANC_WRITE_COEFF_CONTINUE\n");
        if (__this->coeff) {
            anc_cmd_timeout_del();
            anc_coeff_write((int *)__this->coeff, __this->coeff_len);
            free(__this->coeff);
            __this->coeff = NULL;
            __this->coeff_ptr = 0;
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_WRITE_COEFF_CONTINUE;
            chargestore_api_write(cmd, 2);
        }
        log_info("CMD_ANC_WRITE_COEFF_CONTINUE--------end\n");
        break;
    case CMD_ANC_READ_FILE_START:
        anc_read_file_start(cmd, __this->file_id);
        break;
    case CMD_ANC_READ_FILE_DATA:
        anc_read_file_data(cmd, __this->data_offset, __this->data_len);
        break;
    case CMD_ANC_WRITE_FILE_START:
        anc_write_file_start(cmd, __this->file_id, __this->file_len);
        break;
    case CMD_ANC_WRITE_FILE_DATA:
        anc_write_file_data(cmd, __this->data_offset, __this->data_ptr, __this->data_len);
        break;
    case CMD_ANC_WRITE_FILE_END:
        anc_write_file_end(cmd);
        break;
    case CMD_ANC_SET_GAIN:
        log_info("CMD_ANC_SET_GAIN\n");
        anc_cmd_timeout_del();
        anc_cfg_online_deal(ANC_CFG_WRITE, &__this->gain);
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_SET_GAIN;
        chargestore_api_write(cmd, 2);
        break;
    case CMD_ANC_GET_GAIN:
        log_info("CMD_ANC_GET_GAIN\n");
        anc_cfg_online_deal(ANC_CFG_READ, &__this->gain);
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_GET_GAIN;
        memcpy(&cmd[2], (u8 *)&__this->gain, sizeof(anc_gain_t));
        chargestore_api_write(cmd, 2 + sizeof(anc_gain_t));
        break;
    case CMD_ANC_CHANGE_BAUD:
        log_info("CMD_ANC_CHANGE_BAUD: %d\n", anc_dev->value);
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_CHANGE_BAUD;
        chargestore_api_write(cmd, 2);
        chargestore_api_wait_complete();
        chargestore_set_baudrate(anc_dev->value);
        if (__this->baud_timer) {
            sys_timeout_del(__this->baud_timer);
            __this->baud_timer = 0;
        }
        if (anc_dev->value != DEFAULT_BAUDRATE) {
            __this->baud_timer = sys_timer_add(NULL, anc_baud_timer_deal, 100);
            __this->baud_counter = 0;
        }
        break;
    case CMD_ANC_WZ_BREAK:
        log_info("CMD_ANC_WZ_BREAK: %d s\n", anc_dev->value);
        sys_timeout_add(NULL, anc_wz_timeout_deal, anc_dev->value * 1000);
        break;
    case CMD_ANC_SET_ID:
        log_info("CMD_ANC_SET_ID: %d ,%d\n", __this->type_id, __this->type_value);
        anc_cmd_timeout_del();
        anc_cfg_btspp_update((u8)__this->type_id, (int)__this->type_value);
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_SET_ID;
        chargestore_api_write(cmd, 2);
        break;
    case CMD_ANC_GET_ID:
        log_info("CMD_ANC_GET_ID: %d\n", __this->type_id);
        if (anc_cfg_btspp_read((u8)__this->type_id, (int *)&__this->type_value) == 0) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_GET_ID;
            memcpy(&cmd[2], (u8 *)&__this->type_value, 4);
            chargestore_api_write(cmd, 6);
        } else {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        }
        break;
#if AUDIO_SPK_EQ_CONFIG
    case CMD_ANC_EQ_DATA:
        log_info("CMD_ANC_EQ_DATA\n");
        log_info_hexdump(eq_buffer, eq_len);
        ret = spk_eq_spp_rx_packet(eq_buffer, eq_len);
        if (ret == -1) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        } else {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_EQ_DATA;
            chargestore_api_write(cmd, 2);
        }
        break;
    case CMD_ANC_NEW_EQ_DATA:
        log_info("CMD_ANC_EQ_DATA\n");
        log_info_hexdump(eq_buffer, eq_len);
        spk_eq_set_send_data_handler(anc_ack_spk_eq);
        spk_eq_app_online_parse(eq_buffer, eq_len, NULL, 0);
        spk_eq_set_send_data_handler(NULL);
        break;
#endif
#if TCFG_AUDIO_CVP_DUT_ENABLE
    case CMD_ANC_CVP_DUT_DATA:
        log_info("CMD_ANC_CVP_DUT_DATA\n");
        log_info_hexdump(cvp_dut_buffer, cvp_dut_len);
        ret = cvp_dut_new_rx_packet(cvp_dut_buffer, cvp_dut_len);
        if (ret) {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_FAIL;
            chargestore_api_write(cmd, 2);
        } else {
            cmd[0] = CMD_ANC_MODULE;
            cmd[1] = CMD_ANC_CVP_DUT_DATA;
            chargestore_api_write(cmd, 2);
        }
        free(cvp_dut_buffer);
        break;
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/
    case CMD_ANC_SET_MUSIC_VOL:
        log_info("CMD_ANC_SET_MUSIC_VOL: %d\n", anc_dev->value);
        app_audio_set_volume(APP_AUDIO_STATE_MUSIC, (anc_dev->value * get_max_sys_vol()) / 0x64, 0);
        break;
#if TCFG_ANC_SELF_DUT_GET_SZ
    case CMD_ANC_SZ_FFT_STOP:
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_SZ_FFT_STOP;
        audio_anc_sz_fft_stop(NULL);			//SZ_FFT 停止计算
        anc_train_close();						//关闭ANC，模式设置为ANC_OFF
        __this->fft_file_busy = 0;
        /* chargestore_api_write(cmd, 2); */
        break;
#endif/*TCFG_ANC_SELF_DUT_GET_SZ*/

#if AUDIO_ENC_MPT_SELF_ENABLE
    case CMD_ANC_MIC_FFT_START:
        __this->fft_file_busy = 1;
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_MIC_FFT_START;
        chargestore_api_write(cmd, 2);	//先回复，再执行
        audio_enc_mpt_fre_response_open(__this->audio_enc_mpt_fre_ch);
        break;
    case CMD_ANC_MIC_FFT_STOP:
        cmd[0] = CMD_ANC_MODULE;
        cmd[1] = CMD_ANC_MIC_FFT_STOP;
        audio_enc_mpt_fre_response_close();
        __this->fft_file_busy = 0;
        break;
#endif/*AUDIO_ENC_MPT_SELF_ENABLE*/
    }
    return false;
}

static int app_ancbox_module_deal(u8 *buf, u8 len)
{
    if (buf[0] != CMD_ANC_MODULE) {
        //不是ANC测试指令
        return 0;
    }

    u8 cmd, i, j;
    u8 sendbuf[32];
    u32 read_len;
    u32 baudrate;
    u32 anc_coeff_total_size;
    u32 value;
    struct _ear_parm parm;
    sendbuf[0] = CMD_ANC_MODULE;
    sendbuf[1] = buf[1];
    __this->ancbox_status = 1;
    __this->baud_counter = 0;
    anc_online_busy_set(1);

    if (buf[1] != 129) {
        printf("cmd = 0x%x\n", buf[1]);
    }

    cmd = buf[1];
    switch (buf[1]) {
    case CMD_ANC_SET_PARM:
        memcpy((u8 *)&parm, &buf[2], sizeof(struct _ear_parm));
        chargestore_api_write(sendbuf, 2);
        if (parm.mode == 0) {
            anc_train_api_set(ANC_CHANGE_COMMAND, ANC_TRAIN_NOMA_MODE, __this->para);
        } else {
            anc_train_api_set(ANC_CHANGE_COMMAND, parm.mode, __this->para);
        }
        anc_train_api_set(ANC_SZ_LOW_THR_SET, parm.emmtv, __this->para);
        anc_train_api_set(ANC_FZ_LOW_THR_SET, parm.rmmtv, __this->para);
        anc_train_api_set(ANC_NONADAP_TIME_SET, parm.sdt, __this->para);
        anc_train_api_set(ANC_SZ_ADAP_TIME_SET, parm.emstt, __this->para);
        anc_train_api_set(ANC_FZ_ADAP_TIME_SET, parm.rmstt, __this->para);
        anc_train_api_set(ANC_WZ_TRAIN_TIME_SET, parm.ntt, __this->para);
        anc_train_api_set(ANC_TRAIN_STEP_SET, parm.ts, __this->para);
        if (parm.gen) {
#if (ANC_CHIP_VERSION != ANC_VERSION_BR36) && (ANC_CHIP_VERSION != ANC_VERSION_BR28)
            anc_param_fill(ANC_CFG_READ, &__this->gain);
            __this->gain.dac_gain = (u8)parm.gdac;
            __this->gain.ref_mic_gain = (u8)parm.grmic;
            __this->gain.err_mic_gain = (u8)parm.gemic;
            __this->gain.anc_gain = (int)parm.ganc;
            __this->gain.transparency_gain = (int)parm.gpass;
            anc_param_fill(ANC_CFG_WRITE, &__this->gain);
#endif
        }
        return 1;
    case CMD_ANC_SET_START:
    case CMD_ANC_SET_STOP:
    case CMD_ANC_MUTE_TRAIN:
    case CMD_ANC_MUTE_TRAIN_ONLY:
    case CMD_ANC_SIN_TRAIN:
    case CMD_ANC_NOISE_TRAIN:
    case CMD_ANC_TEST_ANC_ON:
    case CMD_ANC_TEST_ANC_OFF:
    case CMD_ANC_TEST_PASS:
    case CMD_ANC_SYS_RESET:
        chargestore_api_write(sendbuf, 2);
        break;
    case CMD_ANC_TRANS_TRAIN:
        if (len > 2) {
            memcpy((u8 *)&value, &buf[2], 4);
            anc_api_set_trans_aim_pow(value);
        }
        chargestore_api_write(sendbuf, 2);
        break;

    case CMD_ANC_SET_RUN_MODE:
    case CMD_ANC_TEST_BYPASS:
    case CMD_ANC_CHANGE_MODE:
    case CMD_ANC_CREATE_SPK_MIC:
        anc_event_to_user(cmd, (u32)buf[2]);
        chargestore_api_write(sendbuf, 2);
        return 1;
    case CMD_ANC_STATUS:
    case CMD_ANC_TOOLS_SYNC:
        //在idle
#if TCFG_CHARGESTORE_PORT == LDOIN_BIND_IO
        if (__this->idle_flag == 0) {
            sendbuf[1] = CMD_ANC_FAIL;
            chargestore_api_write(sendbuf, 2);
        } else
#endif
        {
            sendbuf[2] = ANC_VERSION;//版本
            sendbuf[3] = ANC_TRAIN_MODE;//结构
#if ANC_MULT_ORDER_ENABLE
            sendbuf[4] = ANC_CHIP_VERSION | 0XA0;	//芯片版本
#else
            sendbuf[4] = ANC_CHIP_VERSION;
#endif
            chargestore_api_write(sendbuf, 5);
        }
        break;
    case CMD_ANC_GET_MAC:
        bt_get_vm_mac_addr(&sendbuf[2]);
        chargestore_api_write(sendbuf, 8);
        return 1;
    case CMD_ANC_SET_LINKKEY:
        bt_update_mac_addr(&buf[2]);
        set_temp_link_key(&buf[8]);
        chargestore_api_write(sendbuf, 2);
        break;
    case CMD_ANC_SW_TO_BT:
        chargestore_api_write(sendbuf, 2);
        break;
    case CMD_ANC_READ_COEFF://read start
#if ANCTOOL_NEED_PAIR_KEY
        if (!__this->pair_flag) {
            sendbuf[1] = CMD_ANC_FAIL;
            chargestore_api_write(sendbuf, 2);
            return 1;
        }
#endif
        chargestore_api_set_timeout(50);
        break;
    case CMD_ANC_READ_COEFF_CONTINUE:
#if ANCTOOL_NEED_PAIR_KEY
        if (!__this->pair_flag) {
            sendbuf[1] = CMD_ANC_FAIL;
            chargestore_api_write(sendbuf, 2);
            return 1;
        }
#endif
        if (__this->coeff == NULL) {
            sendbuf[1] = CMD_ANC_FAIL;
            chargestore_api_write(sendbuf, 2);
        } else {
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
            anc_coeff_total_size = anc_coeff_size_get(ANC_CFG_READ);
#else
            anc_coeff_total_size = anc_coeff_size_get();
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
            read_len = anc_coeff_total_size - __this->coeff_ptr;
            if (read_len > 30) {
                read_len = 30;
            }
            memcpy(&sendbuf[2], __this->coeff + __this->coeff_ptr, read_len);
            chargestore_api_write(sendbuf, read_len + 2);
            __this->coeff_ptr += read_len;
            if (anc_coeff_total_size == __this->coeff_ptr) {
                __this->coeff = NULL;
                __this->coeff_ptr = 0;
            }
        }
        //读操作不需要推消息处理
        return 1;
    case CMD_ANC_WRITE_COEFF://write start
        chargestore_api_set_timeout(50);
        memcpy((u8 *)&read_len, buf + 2, 4);
        __this->coeff_len = read_len;
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
        anc_coeff_size_set(ANC_CFG_WRITE, read_len);
        anc_coeff_total_size = read_len;
#else
        anc_coeff_total_size = anc_coeff_size_get();
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        if (read_len != anc_coeff_total_size) {
            sendbuf[1] = CMD_ANC_FAIL;
            chargestore_api_write(sendbuf, 2);
            log_info("anc coeff size err, %d != %d\n", read_len, anc_coeff_total_size);
            return 1;
        }
        break;
    case CMD_ANC_WRITE_COEFF_CONTINUE:
        memcpy(__this->coeff + __this->coeff_ptr, buf + 2, len - 2);
        __this->coeff_ptr += (len - 2);
        if (__this->coeff_ptr < __this->coeff_len) {
            //未写完成时不推消息
            chargestore_api_write(sendbuf, 2);
            return 1;
        }
        chargestore_api_set_timeout(1000);
        break;
    case CMD_ANC_READ_FILE_START:
        memcpy((u8 *)&__this->file_id, buf + 2, 4);
        chargestore_api_set_timeout(1000);
        break;
    case CMD_ANC_READ_FILE_DATA:
        memcpy((u8 *)&__this->data_offset, buf + 2, 4);
        memcpy((u8 *)&__this->data_len, buf + 6, 4);
        chargestore_api_set_timeout(1000);
        break;
    case CMD_ANC_WRITE_FILE_START:
        memcpy((u8 *)&__this->file_id, buf + 2, 4);
        memcpy((u8 *)&__this->file_len, buf + 6, 4);
        chargestore_api_set_timeout(1000);
        break;
    case CMD_ANC_WRITE_FILE_DATA:
        memcpy((u8 *)&__this->data_offset, buf + 2, 4);
        memcpy(__this->data_ptr, buf + 6, len - 6);
        __this->data_len = len - 6;
        printf("write file data, offset:%d, datalen: %d\n", __this->data_offset, __this->data_len);
        chargestore_api_set_timeout(1000);
        break;
    case CMD_ANC_WRITE_FILE_END:
        chargestore_api_set_timeout(1000);
        break;
    case CMD_ANC_GET_RESULT:
        sendbuf[2] = __this->err_flag;
        sendbuf[3] = anc_api_get_train_step();
        chargestore_api_write(sendbuf, 4);
        return 1;//不需要app处理
    case CMD_ANC_SET_GAIN:
        if ((len - 2) != sizeof(anc_gain_t)) {
            sendbuf[1] = CMD_ANC_FAIL;
            chargestore_api_write(sendbuf, 2);
            return 1;
        } else {
            memcpy((u8 *)&__this->gain, &buf[2], sizeof(anc_gain_t));
            chargestore_api_set_timeout(1000);
        }
        break;
    case CMD_ANC_GET_GAIN:
        chargestore_api_set_timeout(50);
        break;
    case CMD_ANC_CHANGE_BAUD:
        chargestore_api_set_timeout(50);
        memcpy((u8 *)&baudrate, &buf[2], 4);
        anc_event_to_user(cmd, baudrate);
        return 1;
    case CMD_ANC_DEVELOPER_MODE:
        anc_train_api_set(ANC_DEVELOPER_MODE, (u32)buf[2], __this->para);
        chargestore_api_write(sendbuf, 2);
        return 1;
    case CMD_ANC_ADAP:
        if (buf[2] == 0) {
            anc_train_api_set(ANC_TEST_NADAP, 0, __this->para);
        } else {
            anc_train_api_set(ANC_TEST_ADAP, 0, __this->para);
        }
        chargestore_api_write(sendbuf, 2);
        return 1;
    case CMD_ANC_WZ_BREAK:
        anc_event_to_user(cmd, (u32)buf[2]);
        chargestore_api_write(sendbuf, 2);
        return 1;
    case CMD_ANC_SET_TS:
        anc_train_api_set(ANC_TRAIN_STEP_SET, (u32)buf[2], __this->para);
        chargestore_api_write(sendbuf, 2);
        return 1;
    case CMD_ANC_TRAIN_STEP:
        __this->train_step_succ = 0;
        anc_train_api_set(ANC_TRAIN_STEP_1, 0, __this->para);
        chargestore_api_write(sendbuf, 2);
        return 1;
    case CMD_ANC_GET_TRAIN_STEP:
        sendbuf[2] = 0;
        if (__this->train_step_succ) {
            sendbuf[2] = anc_api_get_train_step();
        } else {
            anc_train_api_set(ANC_TRAIN_STEP_CONNUTE, 0, __this->para);
        }
        chargestore_api_write(sendbuf, 3);
        return 1;
    case CMD_ANC_GET_MUTE_TRAIN_POW:
        sendbuf[2] = __this->sz_nadap_pow;
        sendbuf[3] = __this->sz_adap_pow;
        sendbuf[4] = __this->sz_mute_pow;
        sendbuf[5] = __this->fz_nadap_pow;
        sendbuf[6] = __this->fz_adap_pow;
        sendbuf[7] = __this->fz_mute_pow;
        chargestore_api_write(sendbuf, 8);
        return 1;
    case CMD_ANC_GET_NOISE_TRAIN_POW:
        sendbuf[2] = __this->wz_nadap_pow;
        sendbuf[3] = __this->wz_adap_pow;
        chargestore_api_write(sendbuf, 4);
        return 1;
    case CMD_ANC_GET_TRAIN_POW:
        for (i = (ANC_POW_SIZE / 3), j = 2; i < (ANC_POW_SIZE * 2 / 3); i++, j++) {
            sendbuf[j] = anc_pow[i];
        }
        chargestore_api_write(sendbuf, j);
        return 1;
    case CMD_ANC_SET_ID:
        memcpy((u8 *)&__this->type_id, &buf[2], 4);
        memcpy((u8 *)&__this->type_value, &buf[6], 4);
        chargestore_api_set_timeout(250);
        break;
    case CMD_ANC_GET_ID:
        memcpy((u8 *)&__this->type_id, &buf[2], 4);
        chargestore_api_set_timeout(250);
        break;
    case CMD_ANC_GET_COEFF_SIZE:
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
        anc_coeff_total_size = anc_coeff_size_get(ANC_CFG_READ);
#else
        anc_coeff_total_size = anc_coeff_size_get();
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        read_len = anc_coeff_total_size;
        memcpy(&sendbuf[2], (u8 *)&read_len, 4);
        chargestore_api_write(sendbuf, 6);
        return 1;
    case CMD_ANC_GET_VERSION:
#if ANC_MULT_ORDER_ENABLE
        sendbuf[2] = ANC_CHIP_VERSION | 0XA0;
#else
        sendbuf[2] = ANC_CHIP_VERSION;
#endif
        chargestore_api_write(sendbuf, 3);
        return 1;
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
    case CMD_ANC_GET_ANC_CH:
        sendbuf[2] = ANC_CH;
        chargestore_api_write(sendbuf, 3);
        return 1;
    case CMD_ANC_GET_MAX_ORDER:
#if ANC_CHIP_VERSION == ANC_VERSION_BR28
        sendbuf[2] = 20;
#else
        sendbuf[2] = ANC_MAX_ORDER;
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR28*/
        chargestore_api_write(sendbuf, 3);
        return 1;
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
#if ANCTOOL_NEED_PAIR_KEY
    case CMD_ANC_PAIR_KEY:
        buf[len] = '\0';
        log_info("CMD_ANC_PAIR_KEY: %s\n", (buf + 2));
        anc_ack_pair_key(buf + 2);
        return 1;
    case CMD_ANC_PAIR_SUCC:
        __this->pair_flag = 1;
        chargestore_api_write(sendbuf, 2);
        return 1;
#endif

    case CMD_ANC_EQ_DATA:
        eq_len = len - 2;
        memcpy(eq_buffer, buf + 2, eq_len);
        chargestore_api_set_timeout(100);
        break;
    case CMD_ANC_NEW_EQ_DATA:
        eq_len = len - 2;
        memcpy(eq_buffer, buf + 2, eq_len);
        chargestore_api_set_timeout(50);
        break;
#if TCFG_AUDIO_CVP_DUT_ENABLE
    case CMD_ANC_CVP_DUT_DATA:
        cvp_dut_len = len - 2;
        cvp_dut_buffer = malloc(cvp_dut_len);
        memcpy(cvp_dut_buffer, buf + 2, cvp_dut_len);
        chargestore_api_set_timeout(50);
        break;
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/

#if TCFG_ANC_SELF_DUT_GET_SZ
    case CMD_ANC_SZ_FFT_START:
        __this->fft_file_busy = 1;
        void audio_anc_post_msg_sz_fft_open(u8 sel);
        audio_anc_post_msg_sz_fft_open(buf[2]);
        chargestore_api_write(sendbuf, 2);
        return 1;
    case CMD_ANC_SZ_FFT_STOP:
        chargestore_api_set_timeout(50);	//透传命令固定50*2 ms验证
        chargestore_api_write(sendbuf, 2);
        break;
        /* chargestore_api_write(sendbuf, 2); */
        /* return 1; */
#endif/*TCFG_ANC_SELF_DUT_GET_SZ*/

    case CMD_ANC_SET_MUSIC_VOL:
        anc_event_to_user(cmd, (u32)buf[2]);
        chargestore_api_write(sendbuf, 2);
        return 1;

#if AUDIO_ENC_MPT_SELF_ENABLE
    case CMD_ANC_MIC_FFT_START:
        __this->audio_enc_mpt_fre_ch = buf[2];
        chargestore_api_set_timeout(50);	//透传命令固定50*2 ms验证
        break;
    case CMD_ANC_MIC_FFT_STOP:
        chargestore_api_set_timeout(50);	//透传命令固定50*2 ms验证
        chargestore_api_write(sendbuf, 2);
        break;
#endif/*AUDIO_ENC_MPT_SELF_ENABLE*/

#if ANC_MULT_ORDER_ENABLE
    case CMD_MUTL_SCENE_SET:
        audio_anc_mult_scene_set(buf[2], 1);
        chargestore_api_write(sendbuf, 2);
        return 1;
#endif/*ANC_MULT_ORDER_ENABLE*/
    case CMD_FFT_FILE_BUSY_GET:
        sendbuf[2] = __this->fft_file_busy;
        chargestore_api_write(sendbuf, 3);
        return 1;

    default:
        sendbuf[1] = CMD_ANC_FAIL;
        chargestore_api_write(sendbuf, 2);
        return 1;
    }
    anc_event_to_user(cmd, 0);
    return 1;
}

u8 ancbox_get_status(void)
{
    return __this->ancbox_status;
}

void ancbox_clear_status(void)
{
    __this->ancbox_status = 0;
}

CHARGESTORE_HANDLE_REG(ancbox, app_ancbox_module_deal);

#endif


