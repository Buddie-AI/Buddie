#include "init.h"
#include "app_config.h"
#include "system/includes.h"

#if (TCFG_ANC_TOOL_DEBUG_ONLINE && TCFG_AUDIO_ANC_ENABLE)

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".anc_box_bss")
#pragma data_seg(".anc_box_data")
#pragma const_seg(".anc_box_const")
#pragma code_seg(".anc_box_code")
#endif/*SUPPORT_MS_EXTENSIONS*/

#include "btstack/avctp_user.h"
#include "classic/hci_lmp.h"
#include "app_anctool.h"
#include "audio_anc.h"
#include "user_cfg.h"
#include "app_action.h"
#include "app_main.h"
#include "user_cfg.h"
#include "bt_tws.h"

#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
#include "icsd_adt_app.h"
#endif

#define anctool_printf  log_i
#define anctool_put_buf put_buf

#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
#define CONFIG_VERSION  "V2.1.1"
#else
#define CONFIG_VERSION  "V2.1.0"
#endif

extern void anc_cfg_btspp_update(u8 id, int data);
extern int anc_cfg_btspp_read(u8 id, int *data);
extern int anc_mode_change_tool(u8 dat);

enum {
    FILE_ID_COEFF = 0,  //系数文件ID号
    FILE_ID_MIC_SPK = 1,//mic spk文件ID号
    FILE_ID_GAIN = 2,   //增益ID
    FILE_ID_ADAPTIVE = 3, //自适应模式文件ID
    FILE_ID_ADAPTIVE_REF = 4,	//自适应模式金机曲线ID
    FILE_ID_SPKEQ = 5,     		//EQ数据
    FILE_ID_SZ_FFT = 6,     	//SZ频响数据
    FILE_ID_MIC_FFT = 7,     	//MIC声学频响数据
    FILE_ID_COEFF_FGQ = 8,     	//新系数文件-FGQ ID号
    FILE_ID_COEFF_FGQ_PART = 9,	//新部分系数文件-FGQ ID号
};

enum {
    ERR_NO = 0,
    ERR_EAR_OFFLINE = 1,//耳机不在线
    ERR_EAR_FAIL = 2,   //耳机返回失败
    ERR_MIC_BUSY = 3,   //表示当前处于训练阶段,mic联通状态不允许发送命令给耳机
    ERR_COEFF_NO = 4,   //读系数时耳机没有训练系数
    ERR_COEFF_LEN = 5,  //写系数时长度异常
    ERR_PARM_LEN = 6,   //参数长度异常
    ERR_VERSION = 7,    //版本匹配异常,请更新转接板固件
    ERR_READ_FILE = 8,  //读文件失败
    ERR_WRITE_FILE = 9, //写文件失败
    ERR_PAIR_KEY = 10,  //密码校验失败
};

enum {
    CMD_GET_VERSION     = 0x04, //获取版本号
    CMD_ANC_OFF         = 0x13, //关闭ANC
    CMD_ANC_ON          = 0x14, //开降噪
    CMD_ANC_PASS        = 0x15, //通透模式
    CMD_READ_START      = 0x20, //开始读
    CMD_READ_DATA       = 0x21, //读取数据
    CMD_WRITE_START     = 0x22, //开始写
    CMD_WRITE_DATA      = 0x23, //写入数据
    CMD_WRITE_END       = 0x24, //数据写入完成
    CMD_SET_ID          = 0x29, //设置ID对应的数据
    CMD_GET_ID          = 0x2A, //获取ID对应的数据
    CMD_TOOLS_SET_GAIN  = 0x2B, //工具设置增益(PC下来多少数据就转发多少给耳机)
    CMD_TOOLS_GET_GAIN  = 0x2C, //工具读取增益(耳机过来多少数据就转发多少给PC)
    CMD_ANC_BYPASS      = 0x2D, //BYPASS让耳机进入bypass模式,带一个u8的参数
    CMD_GET_ANC_MODE    = 0x2E, //获取耳机训练模式,(FF,FB,HYB...)
    CMD_SET_ANC_MODE    = 0x2F, //切换耳机训练模式,仅在耳机训练模式为混合溃时才能切换为单溃或者双馈
    CMD_GET_COEFF_SIZE  = 0x30, //获取系数内容大小(不会因为小机没有系数导致失败)
    CMD_MUTE_TRAIN_ONLY = 0x31, //静音训练,耳机只跑静音训练
    CMD_MUTE_TRAIN_ONLY_END = 0x32, //静音训练,耳机只跑静音训练,结束 ear->pc
    CMD_MUTE_TRAIN_GET_POW = 0x33, //静音训练结束后获取fz sz数据接口
    CMD_READ_FILE_START = 0x34, //读文件开始,携带ID号,返回文件大小
    CMD_READ_FILE_DATA  = 0x35, //读取文件数据,地址,长度
    CMD_WRITE_FILE_START = 0x36, //写文件开始,携带ID号,及文件长度
    CMD_WRITE_FILE_DATA = 0x37, //写入文件数据,地址+数据
    CMD_WRITE_FILE_END  = 0x38, //写入文件结束
    CMD_GET_CHIP_VERSION = 0x39, //获取耳机芯片ANC版本
    CMD_TRANS_MUTE_TRAIN = 0x3A, //通透训练
    CMD_TRANS_MUTE_TRAIN_END = 0x3B, //通透训练结束, EAR->PC
    CMD_TRIGER_CREATE_SPK_MIC = 0x3C, //触发生成DAT_SPK_MIC数据
    CMD_CREATE_SPK_MIC_END = 0x3D, //生成DAT_SPK_MIC数据完成
    CMD_GET_ANC_CH 		= 0x3E,	//获取耳机通道设置
    CMD_PAIR_KEY        = 0x3F,	//发送密码给小机
    CMD_PAIR_SUCC       = 0x40,	//小机绕过密码校验
    CMD_MIC_DATA_EXPORT_EN = 0X41,	//开启/关闭mic数据采集
    CMD_MIC_DATA_SEND	= 0X42,		//实时反馈mic数据
    CMD_GET_ANC_MAX_ORDER = 0x43,	//获取ANC最大滤波器阶数
    CMD_ANC_ADAPTIVE_MODE = 0x47,	//ANC耳道自适应模式
    CMD_ANC_IIR_SEL		  = 0x48,	//ANC 普通参数/自适应参数切换
    CMD_EAR_ROLE_SWITCH	  = 0x49,	//主从切换
    CMD_DEVELOP_MODE_SET  = 0X4A,	//开发者模式设置
    CMD_MUTL_SCENE_SET = 0X55, 	//ANC多场景设置
    CMD_FAIL            = 0xFE, //执行失败
    CMD_UNDEFINE        = 0xFF, //未定义
};

struct _anctool_info {
    anc_train_para_t *para;
    u8 mute_train_flag;
#if ANCTOOL_NEED_PAIR_KEY
    u8 pair_flag;
#endif
    u8 pack_flag;
    u8 sz_nadap_pow;
    u8 sz_adap_pow;
    u8 sz_mute_pow;
    u8 fz_nadap_pow;
    u8 fz_adap_pow;
    u8 fz_mute_pow;
    u8 mic_pow_type;
    u8 anc_designer;
    u8 *file_hdl;
    u32 file_len;
    u32 file_id;
    u8 connected_flag;
};
static struct _anctool_info anctool_info;
#define __this  (&anctool_info)

static void app_anctool_send_ack(u8 cmd2pc, u8 ret, u8 err_num)
{
    u8 cmd[2];
    if (ret == TRUE) {
        cmd[0] = cmd2pc;
        anctool_api_write(cmd, 1);
    } else {
        cmd[0] = CMD_FAIL;
        cmd[1] = err_num;
        anctool_api_write(cmd, 2);
    }
}

static void app_anctool_ack_get_version(void)
{
    u8 *cmd;
    u32 datalen;
    datalen = sizeof(CONFIG_VERSION) + 1;
    cmd = anctool_api_write_alloc(datalen);
    cmd[0] = CMD_GET_VERSION;
    memcpy(&cmd[1], CONFIG_VERSION, datalen - 1);
    anctool_api_write(cmd, datalen);
    anc_online_busy_set(1);
}

static void app_anctool_ack_read_start(u8 head)
{
    u8 *cmd;
    u32 data_len;
    __this->file_hdl = (u8 *)anc_coeff_read();		//先读头获取系数
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
    __this->file_len = anc_coeff_size_get(ANC_CFG_READ);
#else
    __this->file_len = anc_coeff_size_get();
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
    if (__this->file_hdl) {//判断耳机有没有系数
        data_len = __this->file_len;
        cmd = anctool_api_write_alloc(5);
        cmd[0] = head;
        memcpy(&cmd[1], (u8 *)&data_len, 4);
        anctool_api_write(cmd, 5);
    } else {
        app_anctool_send_ack(head, FALSE, ERR_COEFF_NO);
    }
}

static void app_anctool_ack_read_data(u8 head, u32 offset, u32 data_len)
{
    u8 *cmd = anctool_api_write_alloc(data_len + 1);
    cmd[0] = head;
    memcpy(&cmd[1], __this->file_hdl + offset, data_len);
    if (offset + data_len == __this->file_len) {
        __this->file_hdl = NULL;
    }
    anctool_api_write(cmd, data_len + 1);
}

static void app_anctool_ack_wirte_start(u8 cmd, u32 data_len)
{
    anctool_printf("CMD_WRITE_START\n");
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
    __this->file_len = data_len;
    anc_coeff_size_set(ANC_CFG_WRITE, data_len);			//设置长度写anc_coeff.bin
    __this->file_hdl = malloc(data_len);
    if (__this->file_hdl) {
#else
    __this->file_len = anc_coeff_size_get();
    __this->file_hdl = malloc(data_len);
    if ((data_len == __this->file_len) && __this->file_hdl) {
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        app_anctool_send_ack(cmd, TRUE, ERR_NO);
    } else {
        app_anctool_send_ack(cmd, FALSE, ERR_COEFF_LEN);
        if (__this->file_hdl) {
            free(__this->file_hdl);
            __this->file_hdl = NULL;
        }
    }
}

static void app_anctool_ack_wirte_data(u8 cmd, u32 offset, u8 *data, u32 data_len)
{
    memcpy(__this->file_hdl + offset, data, data_len);
    app_anctool_send_ack(cmd, TRUE, ERR_NO);
}

static void app_anctool_ack_write_end(u32 file_id, u8 cmd)
{
    int ret;
    switch (file_id) {
    case FILE_ID_COEFF:
        ret = anc_coeff_write((int *)__this->file_hdl, __this->file_len);
        break;
#if ANC_MULT_ORDER_ENABLE
    case FILE_ID_COEFF_FGQ:
        /* g_printf("coeff fgq write END!"); */
        /* put_buf(__this->file_hdl, __this->file_len); */
        ret = audio_anc_mult_coeff_write(ANC_MULT_COEFF_FILL_ALL, (int *)__this->file_hdl, __this->file_len);
        break;
    case FILE_ID_COEFF_FGQ_PART:
        /* g_printf("coeff fgq write part END!"); */
        /* put_buf(__this->file_hdl, __this->file_len); */
        ret = audio_anc_mult_coeff_write(ANC_MULT_COEFF_FILL_PART, (int *)__this->file_hdl, __this->file_len);
        break;
#endif/*ANC_MULT_ORDER_ENABLE*/
    }
    free(__this->file_hdl);
    __this->file_hdl = NULL;
    if (!ret) {
        app_anctool_send_ack(cmd, TRUE, ERR_NO);
    } else {	//滤波器格式出错则返回ERROR
        app_anctool_send_ack(CMD_SET_ID, FALSE, ERR_COEFF_LEN);
    }
}

static void app_anctool_ack_anc_mode(u8 cmd)
{
    /* anc_api_set_fade_en(0); */	//暂时不关闭淡入淡出
    if (!__this->anc_designer) {
        //产测模式-可以放置产测相关的特殊设置
        anctool_printf("ANC in Production mode\n");
#if ANC_EAR_ADAPTIVE_EN
        if (audio_anc_coeff_mode_get()) {
            audio_anc_coeff_adaptive_set(0, 0, 0);	//将ANC参数设置为默认
        }
#endif/*ANC_EAR_ADAPTIVE_EN*/
    } else {
        anctool_printf("ANC in Designer mode\n");
    }
    if (cmd == CMD_ANC_ON) {
        anc_mode_switch(ANC_ON, 0);
    } else if (cmd == CMD_ANC_OFF) {
        anc_mode_switch(ANC_OFF, 0);
    } else if (cmd == CMD_ANC_PASS) {
        anc_mode_switch(ANC_TRANSPARENCY, 0);
#if ANC_EAR_ADAPTIVE_EN
    } else if (cmd == CMD_ANC_ADAPTIVE_MODE) {
        if (audio_anc_mode_ear_adaptive(1)) {
            app_anctool_send_ack(cmd, TRUE, ERR_NO);
            return;
        }
#endif/*ANC_EAR_ADAPTIVE_EN*/
    }  else {
        anc_mode_switch(ANC_BYPASS, 0);
    }
    app_anctool_send_ack(cmd, TRUE, ERR_NO);
}

static void app_anctool_ack_set_id(u32 id, u32 value)
{
    anc_cfg_btspp_update((u8)id, (int)value);
    app_anctool_send_ack(CMD_SET_ID, TRUE, ERR_NO);
}

static void app_anctool_ack_get_id(u32 id)
{
    u8 *cmd;
    u32 value;
    if (anc_cfg_btspp_read((u8)id, (int *)&value) == 0) {
        cmd = anctool_api_write_alloc(4 + 1);
        cmd[0] = CMD_GET_ID;
        memcpy(&cmd[1], (u8 *)&value, 4);
        anctool_api_write(cmd, 4 + 1);
    } else {
        app_anctool_send_ack(CMD_SET_ID, FALSE, ERR_EAR_FAIL);
    }
}

static void app_anctool_ack_set_gain(u8 *buf, u32 len)
{
    u8 *cmd;
    u32 data_len;
    anc_gain_t *anc_gain = malloc(sizeof(anc_gain_t));
    if ((len != sizeof(anc_gain_t)) || (anc_gain == NULL)) {
        app_anctool_send_ack(CMD_TOOLS_SET_GAIN, FALSE, ERR_PARM_LEN);
    } else {
        memcpy(anc_gain, buf, len);
        anc_cfg_online_deal(ANC_CFG_WRITE, anc_gain);
        app_anctool_send_ack(CMD_TOOLS_SET_GAIN, TRUE, ERR_NO);
    }
    if (anc_gain) {
        free(anc_gain);
    }
}

static void app_anctool_ack_get_gain(void)
{
    u8 *cmd;
    u32 data_len;
    anc_gain_t *anc_gain = malloc(sizeof(anc_gain_t));
    if (anc_gain == NULL) {
        app_anctool_send_ack(CMD_TOOLS_GET_GAIN, FALSE, ERR_PARM_LEN);
        return;
    }
    data_len = sizeof(anc_gain_t) + 1;
    cmd = anctool_api_write_alloc(data_len);
    cmd[0] = CMD_TOOLS_GET_GAIN;
    anc_cfg_online_deal(ANC_CFG_READ, anc_gain);
    memcpy(&cmd[1], anc_gain, sizeof(anc_gain_t));
    anctool_api_write(cmd, data_len);
    free(anc_gain);
}

static void app_anctool_ack_get_chip_version(void)
{
    u8 *cmd = anctool_api_write_alloc(2);
    cmd[0] = CMD_GET_CHIP_VERSION;
#if ANC_MULT_ORDER_ENABLE
    cmd[1] = ANC_CHIP_VERSION | 0XA0;
#else
    cmd[1] = ANC_CHIP_VERSION;
#endif
    anctool_api_write(cmd, 2);
}

static void app_anctool_ack_get_train_mode(void)
{
    u8 *cmd = anctool_api_write_alloc(2);
    cmd[0] = CMD_GET_ANC_MODE;
    cmd[1] = ANC_TRAIN_MODE;
    anctool_api_write(cmd, 2);
}

static void app_anctool_ack_get_ch(void)
{
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
    u8 *cmd = anctool_api_write_alloc(2);
    cmd[0] = CMD_GET_ANC_CH;
    //开发者模式且打开耳道自适应功能，则返回工具立体声配置
    if (audio_anc_develop_get() && ANC_EAR_ADAPTIVE_EN) {
        cmd[1] = ANC_L_CH | ANC_R_CH;
    } else {
        cmd[1] = ANC_CH;
    }
    anctool_api_write(cmd, 2);
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
}

static void app_anctool_ack_get_anc_max_order(void)
{
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
    u8 *cmd = anctool_api_write_alloc(2);
    cmd[0] = CMD_GET_ANC_MAX_ORDER;
#if ANC_CHIP_VERSION == ANC_VERSION_BR28
    cmd[1] = 20;
#else
    cmd[1] = ANC_MAX_ORDER;
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR28*/
    anctool_api_write(cmd, 2);
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
}

static void app_anctool_ack_set_train_mode(u8 mode)
{
    anc_mode_change_tool(mode);
    app_anctool_send_ack(CMD_SET_ANC_MODE, TRUE, ERR_NO);
}

static void app_anctool_ack_get_coeff_size(void)
{
    u8 *cmd;
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
    u32 coeff_size = anc_coeff_size_get(ANC_CFG_READ);
#else
    u32 coeff_size = anc_coeff_size_get();
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
    cmd = anctool_api_write_alloc(5);
    cmd[0] = CMD_GET_COEFF_SIZE;
    memcpy(&cmd[1], &coeff_size, 4);
    anctool_api_write(cmd, 5);
}

void app_anctool_ack_mute_train_get_pow(void)
{
    u8 *cmd;
    cmd = anctool_api_write_alloc(7);
    cmd[0] = CMD_MUTE_TRAIN_GET_POW;
    cmd[1] = __this->sz_nadap_pow;
    cmd[2] = __this->sz_adap_pow;
    cmd[3] = __this->sz_mute_pow;
    cmd[4] = __this->fz_nadap_pow;
    cmd[5] = __this->fz_adap_pow;
    cmd[6] = __this->fz_mute_pow;
    anctool_api_write(cmd, 7);
}

#if ANC_CHIP_VERSION == ANC_VERSION_BR28
void app_anctool_ack_mic_power(void)
{
    u8 *cmd;
    u16 buf[4];
    cmd = anctool_api_write_alloc(3);
    audio_anc_pow_get(buf);
    cmd[0] = CMD_MIC_DATA_SEND;
    memcpy(cmd + 1, &buf[__this->mic_pow_type], 2);
    anctool_api_write(cmd, 3);
}
#endif

#if ANCTOOL_NEED_PAIR_KEY
void app_anctool_ack_pair_key(u8 *key)
{
    if (strcmp(ANCTOOL_PAIR_KEY, (const char *)key) == 0) {
        __this->pair_flag = 1;
        app_anctool_send_ack(CMD_PAIR_KEY, TRUE, ERR_NO);
    } else {
        __this->pair_flag = 0;
        app_anctool_send_ack(CMD_PAIR_KEY, FALSE, ERR_PAIR_KEY);
    }
}
#endif

static void anctool_ack_read_file_start(u32 id)
{
    u8 *cmd;
    __this->file_id = id;
    __this->file_len = 0;
    int *anc_debug_hdl;
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
#if ANCTOOL_NEED_PAIR_KEY
        if (!__this->pair_flag) {
            app_anctool_send_ack(CMD_READ_FILE_START, FALSE, ERR_PAIR_KEY);
        } else
#endif
        {
            app_anctool_ack_read_start(CMD_READ_FILE_START);
        }
        break;
    case FILE_ID_MIC_SPK:
        //这里判断是否有数据
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
        anc_debug_hdl = (u8 *)anc_debug_ctr(0);
        if (anc_debug_hdl) {
            __this->file_hdl = (u8 *)(anc_debug_hdl + 1); //数据指针
            __this->file_len = anc_debug_hdl[0];
            cmd = anctool_api_write_alloc(5);
            cmd[0] = CMD_READ_FILE_START;
            memcpy(&cmd[1], (u8 *)&__this->file_len, 4);
            anctool_api_write(cmd, 5);
        } else {
            app_anctool_send_ack(CMD_READ_FILE_START, FALSE, ERR_READ_FILE);
        }
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        break;
    case FILE_ID_GAIN:
        __this->file_len = sizeof(anc_gain_t);
        __this->file_hdl = (u8 *)zalloc(__this->file_len);//数据指针
        if (__this->file_hdl) {
            anc_cfg_online_deal(ANC_CFG_READ, (anc_gain_t *)__this->file_hdl);
            cmd = anctool_api_write_alloc(5);
            cmd[0] = CMD_READ_FILE_START;
            memcpy(&cmd[1], (u8 *)&__this->file_len, 4);
            anctool_api_write(cmd, 5);
        } else {
            app_anctool_send_ack(CMD_READ_FILE_START, FALSE, ERR_READ_FILE);
        }
        break;
#if ANC_EAR_ADAPTIVE_EN
    case FILE_ID_ADAPTIVE:
        extern anc_packet_data_t *anc_adaptive_data;
        if (anc_adaptive_data == NULL) {
            printf("packet is NULL, return!\n");
            app_anctool_send_ack(CMD_READ_FILE_START, FALSE, ERR_READ_FILE);
            return;
        }
        if (anc_adaptive_data->dat_len == 0) {
            printf("error: dat_len == 0\n");
            app_anctool_send_ack(CMD_READ_FILE_START, FALSE, ERR_READ_FILE);
            return;
        }
        __this->file_len = anc_adaptive_data->dat_len;
        __this->file_hdl = (u8 *)(anc_adaptive_data->dat);
        cmd = anctool_api_write_alloc(5);
        cmd[0] = CMD_READ_FILE_START;
        memcpy(&cmd[1], (u8 *)&__this->file_len, 4);
        anctool_api_write(cmd, 5);
        break;
#endif/*ANC_EAR_ADAPTIVE_EN*/
    }
}

static void anctool_ack_read_file_data(u32 offset, u32 data_len)
{
    u8 *cmd;
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
#if ANCTOOL_NEED_PAIR_KEY
        if (!__this->pair_flag) {
            app_anctool_send_ack(CMD_READ_FILE_DATA, FALSE, ERR_PAIR_KEY);
        } else
#endif
        {
            app_anctool_ack_read_data(CMD_READ_FILE_DATA, offset, data_len);
        }
        break;
    case FILE_ID_MIC_SPK:
#if (ANC_CHIP_VERSION == ANC_VERSION_BR36) || (ANC_CHIP_VERSION == ANC_VERSION_BR28)
        if (__this->file_hdl) {
            cmd = anctool_api_write_alloc(data_len + 1);
            cmd[0] = CMD_READ_FILE_DATA;
            memcpy(&cmd[1], __this->file_hdl + offset, data_len);
            if (offset + data_len == __this->file_len) {
                __this->file_hdl = (u8 *)anc_debug_ctr(1);
            }
            anctool_api_write(cmd, data_len + 1);
        } else {
            app_anctool_send_ack(CMD_READ_FILE_DATA, FALSE, ERR_READ_FILE);
        }
#endif/*ANC_CHIP_VERSION == ANC_VERSION_BR36*/
        break;
    case FILE_ID_GAIN:
        if (__this->file_hdl) {
            cmd = anctool_api_write_alloc(data_len + 1);
            cmd[0] = CMD_READ_FILE_DATA;
            memcpy(&cmd[1], __this->file_hdl + offset, data_len);
            if (offset + data_len == __this->file_len) {
                free(__this->file_hdl);
                __this->file_hdl = NULL;
            }
            anctool_api_write(cmd, data_len + 1);
        } else {
            app_anctool_send_ack(CMD_READ_FILE_DATA, FALSE, ERR_READ_FILE);
        }
        break;
#if ANC_EAR_ADAPTIVE_EN
    case FILE_ID_ADAPTIVE:
        if (__this->file_hdl) {
            cmd = anctool_api_write_alloc(data_len + 1);
            cmd[0] = CMD_READ_FILE_DATA;
            memcpy(&cmd[1], __this->file_hdl + offset, data_len);
            if (offset + data_len == __this->file_len) {
                /* free(__this->file_hdl); */
                __this->file_hdl = NULL;
            }
            anctool_api_write(cmd, data_len + 1);
        } else {
            app_anctool_send_ack(CMD_READ_FILE_DATA, FALSE, ERR_READ_FILE);
        }
        break;
#endif/*ANC_EAR_ADAPTIVE_EN*/
    }
}

static void anctool_ack_write_file_start(u32 id, u32 data_len)
{
    __this->file_id = id;
    __this->file_len = data_len;
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
    case FILE_ID_COEFF_FGQ_PART:
        app_anctool_ack_wirte_start(CMD_WRITE_FILE_START, data_len);
        break;
    case FILE_ID_GAIN:
        __this->file_len = sizeof(anc_gain_t);
        __this->file_hdl = (u8 *)malloc(__this->file_len);
        if ((__this->file_len != data_len) || (__this->file_hdl == NULL)) {
            anctool_printf("anc gain size err, %d != %d\n", data_len, __this->file_len);
            app_anctool_send_ack(CMD_WRITE_FILE_START, FALSE, ERR_WRITE_FILE);
            if (__this->file_hdl) {
                free(__this->file_hdl);
                __this->file_hdl = NULL;
            }
        } else {
            app_anctool_send_ack(CMD_WRITE_FILE_START, TRUE, ERR_NO);
        }
        break;
#if ANC_EAR_ADAPTIVE_EN
    case FILE_ID_ADAPTIVE_REF:
        //开始写金机曲线
        //y_printf("\n>>>>>> [FILE_ID_ADAPTIVE_REF] id : %d, data_len : %d\n\n", __this->file_id, __this->file_len);
        __this->file_hdl = (u8 *)malloc(__this->file_len);
        if (__this->file_hdl == NULL) {
            anctool_printf("anc write adaptive ref failed\n");
            app_anctool_send_ack(CMD_WRITE_FILE_START, FALSE, ERR_WRITE_FILE);
        } else {
            app_anctool_send_ack(CMD_WRITE_FILE_START, TRUE, ERR_NO);
        }
        break;
#endif/*ANC_EAR_ADAPTIVE_EN*/
    }
}

static void anctool_ack_write_file_data(u32 offset, u8 *data, u32 data_len)
{
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
    case FILE_ID_COEFF_FGQ_PART:
        app_anctool_ack_wirte_data(CMD_WRITE_FILE_DATA, offset, data, data_len);
        break;
    case FILE_ID_GAIN:
        if (__this->file_hdl == NULL || ((offset + data_len) > __this->file_len)) {
            app_anctool_send_ack(CMD_WRITE_FILE_DATA, FALSE, ERR_WRITE_FILE);
            if (__this->file_hdl) {
                free(__this->file_hdl);
                __this->file_hdl = NULL;
            }
        } else {
            memcpy(__this->file_hdl + offset, data, data_len);
            app_anctool_send_ack(CMD_WRITE_FILE_DATA, TRUE, ERR_NO);
        }
        break;
#if ANC_EAR_ADAPTIVE_EN
    case FILE_ID_ADAPTIVE_REF:
        //写金机曲线...
        //printf("\n==================================\n");
        //put_buf(data, data_len);
        if (__this->file_hdl == NULL || ((offset + data_len) > __this->file_len)) {
            app_anctool_send_ack(CMD_WRITE_FILE_DATA, FALSE, ERR_WRITE_FILE);
            if (__this->file_hdl) {
                free(__this->file_hdl);
                __this->file_hdl = NULL;
            }
        } else {
            memcpy(__this->file_hdl + offset, data, data_len);
            app_anctool_send_ack(CMD_WRITE_FILE_DATA, TRUE, ERR_NO);
        }
        break;
#endif/*ANC_EAR_ADAPTIVE_EN*/
    }
}

static void anctool_ack_write_file_end(void)
{
    switch (__this->file_id) {
    case FILE_ID_COEFF:
    case FILE_ID_COEFF_FGQ:
    case FILE_ID_COEFF_FGQ_PART:
        app_anctool_ack_write_end(__this->file_id, CMD_WRITE_FILE_END);
        break;
    case FILE_ID_GAIN:
        if (__this->file_hdl == NULL) {
            app_anctool_send_ack(CMD_WRITE_FILE_END, FALSE, ERR_WRITE_FILE);
        } else {
            anc_cfg_online_deal(ANC_CFG_WRITE, (anc_gain_t *)__this->file_hdl);
            free(__this->file_hdl);
            __this->file_hdl = NULL;
            app_anctool_send_ack(CMD_WRITE_FILE_END, TRUE, ERR_NO);
        }
        break;
#if ANC_EAR_ADAPTIVE_EN
    case FILE_ID_ADAPTIVE_REF:
        // 写金机曲线结束
        if (__this->file_hdl == NULL) {
            app_anctool_send_ack(CMD_WRITE_FILE_END, FALSE, ERR_WRITE_FILE);
        } else {
            //保存金机曲线数据
            anc_adaptive_ff_ref_data_save(__this->file_hdl, __this->file_len);
            free(__this->file_hdl);
            __this->file_hdl = NULL;
            app_anctool_send_ack(CMD_WRITE_FILE_END, TRUE, ERR_NO);
        }
        break;
#endif
    }
}

static void anctool_callback(u8 mode, u8 command)
{
    u8 cmd[2];
    anctool_printf("anctool_callback: %d, %d\n", mode, command);
    if (__this->mute_train_flag == 0) {
        g_printf("%s,%d\n", __func__, __LINE__);
        return;
    }
    if (__this->mute_train_flag == 1) {
        cmd[0] = CMD_MUTE_TRAIN_ONLY_END;
        cmd[1] = command;
    } else if (__this->mute_train_flag == 2) {
        cmd[0] = CMD_TRANS_MUTE_TRAIN_END;
        cmd[1] = command;
    } else if (__this->mute_train_flag == 3) {
        cmd[0] = CMD_CREATE_SPK_MIC_END;
        cmd[1] = command;
    } else {
        __this->mute_train_flag = 0;
        return;
    }
    anctool_api_set_active(1);
    anctool_api_write(cmd, 2);
    anctool_api_set_active(0);
    __this->mute_train_flag = 0;
}

static void anctool_pow_callback(anc_ack_msg_t *msg, u8 step)
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
    }
}

static void app_anctool_module_deal(u8 *data, u16 len)
{
    u8 cmd = data[0];
    u32 offset, data_len, id, value;
    __this->pack_flag = 1;
    anctool_printf("recv packet:\n");
    anctool_put_buf(data, len);
    __this->connected_flag = 1;
#if TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN
    /*关闭所有模块*/
    if (audio_icsd_adt_is_running()) {
        audio_icsd_adt_close_all();
    }
#endif
    switch (cmd) {
    case CMD_GET_VERSION:
        anctool_printf("CMD_GET_VERSION\n");
        app_anctool_ack_get_version();
        break;
    case CMD_READ_START:
        anctool_printf("CMD_READ_START\n");
#if ANCTOOL_NEED_PAIR_KEY
        if (!__this->pair_flag) {
            app_anctool_send_ack(CMD_READ_START, FALSE, ERR_PAIR_KEY);
        } else
#endif
        {
            app_anctool_ack_read_start(CMD_READ_START);
        }
        break;
    case CMD_READ_DATA:
        anctool_printf("CMD_READ_DATA\n");
        memcpy((u8 *)&offset, &data[1], 4);
        memcpy((u8 *)&data_len, &data[5], 4);
#if ANCTOOL_NEED_PAIR_KEY
        if (!__this->pair_flag) {
            app_anctool_send_ack(CMD_READ_DATA, FALSE, ERR_PAIR_KEY);
        } else
#endif
        {
            app_anctool_ack_read_data(CMD_READ_DATA, offset, data_len);
        }
        break;
    case CMD_WRITE_START:
        anctool_printf("CMD_WRITE_START\n");
        memcpy((u8 *)&data_len, &data[1], 4);
        app_anctool_ack_wirte_start(CMD_WRITE_START, data_len);
        break;
    case CMD_WRITE_DATA:
        anctool_printf("CMD_WRITE_DATA\n");
        memcpy((u8 *)&offset, &data[1], 4);
        app_anctool_ack_wirte_data(CMD_WRITE_DATA, offset, &data[5], len - 5);
        break;
    case CMD_WRITE_END:
        anctool_printf("CMD_WRITE_END\n");
        app_anctool_ack_write_end(FILE_ID_COEFF, CMD_WRITE_END);
        break;
    case CMD_ANC_OFF:
    case CMD_ANC_ON:
    case CMD_ANC_PASS:
    case CMD_ANC_BYPASS:
#if ANC_EAR_ADAPTIVE_EN
    case CMD_ANC_ADAPTIVE_MODE:
#endif/*ANC_EAR_ADAPTIVE_EN*/
        anctool_printf("CMD_ANC_MODE:%d\n", cmd);
        app_anctool_ack_anc_mode(cmd);
        break;
    case CMD_SET_ID:
        memcpy((u8 *)&offset, &data[1], 4);//id
        memcpy((u8 *)&data_len, &data[5], 4);//value
        anctool_printf("CMD_SET_ID: 0x%x, %d\n", offset, data_len);
        app_anctool_ack_set_id(offset, data_len);
        break;
    case CMD_GET_ID:
        anctool_printf("CMD_GET_ID\n");
        memcpy((u8 *)&offset, &data[1], 4);
        app_anctool_ack_get_id(offset);
        break;
    case CMD_TOOLS_SET_GAIN:
        anctool_printf("CMD_TOOLS_SET_GAIN: %d\n", len - 1);
        app_anctool_ack_set_gain(&data[1], len - 1);
        break;
    case CMD_TOOLS_GET_GAIN:
        anctool_printf("CMD_TOOLS_GET_GAIN\n");
        app_anctool_ack_get_gain();
        break;
    case CMD_GET_ANC_MODE://获取耳机结构
        anctool_printf("CMD_GET_ANC_MODE\n");
        app_anctool_ack_get_train_mode();
        break;
    case CMD_SET_ANC_MODE://设置结构
        anctool_printf("CMD_SET_ANC_MODE: %d\n", data[1]);
        app_anctool_ack_set_train_mode(data[1]);
        break;
    case CMD_GET_COEFF_SIZE:
        anctool_printf("MSG_PC_GET_COEFF_SIZE\n");
        app_anctool_ack_get_coeff_size();
        break;
    case CMD_MUTE_TRAIN_ONLY:
        anctool_printf("CMD_MUTE_TRAIN_ONLY\n");
        __this->mute_train_flag = 1;
        anc_api_set_callback(anctool_callback, anctool_pow_callback);
        anc_train_api_set(ANC_DEVELOPER_MODE, 1, __this->para);//设置为开发者模式
        anc_train_api_set(ANC_MUTE_TARIN, 1, __this->para);
        app_anctool_send_ack(CMD_MUTE_TRAIN_ONLY, TRUE, ERR_NO);
        break;
    case CMD_MUTE_TRAIN_GET_POW:
        anctool_printf("CMD_MUTE_TRAIN_GET_POW\n");
        app_anctool_ack_mute_train_get_pow();
        break;
    case CMD_READ_FILE_START:
        anctool_printf("CMD_READ_FILE_START\n");
        memcpy((u8 *)&id, &data[1], 4);
        anctool_ack_read_file_start(id);
        break;
    case CMD_READ_FILE_DATA:
        anctool_printf("CMD_READ_FILE_DATA\n");
        memcpy((u8 *)&offset, &data[1], 4);
        memcpy((u8 *)&data_len, &data[5], 4);
        anctool_ack_read_file_data(offset, data_len);
        break;
    case CMD_WRITE_FILE_START:
        anctool_printf("CMD_WRITE_FILE_START\n");
        memcpy((u8 *)&id, &data[1], 4);
        memcpy((u8 *)&data_len, &data[5], 4);
        anctool_ack_write_file_start(id, data_len);
        break;
    case CMD_WRITE_FILE_DATA:
        anctool_printf("CMD_WRITE_FILE_DATA\n");
        memcpy((u8 *)&offset, &data[1], 4);
        anctool_ack_write_file_data(offset, &data[5], len - 5);
        break;
    case CMD_WRITE_FILE_END:
        anctool_printf("CMD_WRITE_FILE_END\n");
        anctool_ack_write_file_end();
        break;
    //ear 2 pc cmd
    case CMD_MUTE_TRAIN_ONLY_END:
        anctool_printf("pc ack cmd=[%d]!\n", cmd);
        break;
    case CMD_GET_CHIP_VERSION:
        app_anctool_ack_get_chip_version();
        anctool_printf("CMD_GET_CHIP_VERSION\n");
        break;
    case CMD_TRANS_MUTE_TRAIN:
        anctool_printf("CMD_TOOLS_GET_GAIN\n");
        if (len > 1) {
            memcpy((u8 *)&value, &data[1], 4);
            anc_api_set_trans_aim_pow(value);
        }
        app_anctool_send_ack(CMD_TRANS_MUTE_TRAIN, TRUE, ERR_NO);
        __this->mute_train_flag = 2;
        anc_api_set_callback(anctool_callback, anctool_pow_callback);
        anc_train_api_set(ANC_DEVELOPER_MODE, 1, __this->para);//设置为开发者模式
        anc_train_api_set(ANC_TRANS_MUTE_TARIN, 0, __this->para);
        break;
#if ANC_EAR_ADAPTIVE_EN
    case CMD_ANC_IIR_SEL:
        anctool_printf("CMD_ANC_IIR_SEL\n");
        if (audio_anc_coeff_adaptive_set((u32)data[1], 1, 1)) {
            app_anctool_send_ack(CMD_ANC_IIR_SEL, FALSE, ERR_NO);
        } else {
            app_anctool_send_ack(CMD_ANC_IIR_SEL, TRUE, ERR_NO);
        }
        break;
#endif/*ANC_EAR_ADAPTIVE_EN*/
    case CMD_TRIGER_CREATE_SPK_MIC:
        //开始采集SPK MIC数据
        anctool_printf("CMD_TRIGER_CREATE_SPK_MIC\n");
        app_anctool_send_ack(CMD_TRIGER_CREATE_SPK_MIC, TRUE, ERR_NO);
        anc_api_set_callback(anctool_callback, NULL);
        anc_train_api_set(ANC_MUTE_TARIN, (u32)data[1], __this->para);
        __this->mute_train_flag = 3;
        //todo
        break;
#if TCFG_USER_TWS_ENABLE && (ANC_CH != (ANC_L_CH | ANC_R_CH))
    case CMD_EAR_ROLE_SWITCH:
        anctool_printf("CMD_EAR_ROLE_SWITCH\n");
        if (get_tws_sibling_connect_state()) {
            tws_conn_switch_role();
            app_anctool_send_ack(CMD_EAR_ROLE_SWITCH, TRUE, ERR_NO);
        } else {
            app_anctool_send_ack(CMD_EAR_ROLE_SWITCH, FALSE, ERR_NO);
        }
        break;
#endif
    case CMD_GET_ANC_CH://获取耳机ANC通道
        anctool_printf("CMD_GET_ANC_CH\n");
        app_anctool_ack_get_ch();
        break;
    case CMD_GET_ANC_MAX_ORDER:
        anctool_printf("CMD_GET_ANC_MAX_ORDER\n");
        app_anctool_ack_get_anc_max_order();
        break;
#if ANC_CHIP_VERSION == ANC_VERSION_BR28
    case CMD_MIC_DATA_EXPORT_EN://开启/关闭mic数据采集
        anctool_printf("CMD_MIC_DATA_EXPORT_EN: %d, mic_type: %d\n", data[1], data[2]);
        __this->mic_pow_type = data[2] - ANC_POW_SEL_R_DAC_REF;	//数组下标对齐
        audio_anc_pow_export_en(data[1], data[1]);
        app_anctool_send_ack(CMD_MIC_DATA_EXPORT_EN, TRUE, ERR_NO);
        break;
    case CMD_MIC_DATA_SEND://实时反馈mic数据
        anctool_printf("CMD_MIC_DATA_SEND\n");
        app_anctool_ack_mic_power();
        break;
#endif
#if ANCTOOL_NEED_PAIR_KEY
    case CMD_PAIR_KEY:
        data[len] = '\0';
        anctool_printf("CMD_PAIR_KEY: %s\n", (data + 1));
        app_anctool_ack_pair_key(data + 1);
        break;
    case CMD_PAIR_SUCC:
        anctool_printf("CMD_PAIR_SUCC\n");
        __this->pair_flag = 1;
        app_anctool_send_ack(CMD_PAIR_SUCC, TRUE, ERR_NO);
        break;
#endif
    case CMD_DEVELOP_MODE_SET:
        //ANC_DESIGNER 独有命令
        anctool_printf("CMD_DEVELOP_MODE_SET\n");
        __this->anc_designer = 1;
        audio_anc_develop_set(data[1]);
        app_anctool_send_ack(CMD_DEVELOP_MODE_SET, TRUE, ERR_NO);
        break;
#if ANC_MULT_ORDER_ENABLE
    case CMD_MUTL_SCENE_SET:
        anctool_printf("CMD_MUTL_SCENE_SET\n");
        audio_anc_mult_scene_set(data[1], 1);
        app_anctool_send_ack(CMD_MUTL_SCENE_SET, TRUE, ERR_NO);
        break;
#endif/*ANC_MULT_ORDER_ENABLE*/
    default:
        cmd = CMD_UNDEFINE;
        anctool_api_write(&cmd, 1);
        break;
    }
}

static void app_anctool_spp_tx_data(u8 *data, u16 size)
{
    user_send_cmd_prepare(USER_CTRL_SPP_SEND_DATA, size, data);
}

const struct anctool_data app_anctool_data = {
    .recv_packet = app_anctool_module_deal,
    .send_packet = app_anctool_spp_tx_data,
};

u8 app_anctool_spp_rx_data(u8 *packet, u16 size)
{
    __this->pack_flag = 0;
    anctool_api_rx_data(packet, size);
    return __this->pack_flag;
}

void app_anctool_spp_connect(void)
{
    anctool_printf("%s, spp_connect!\n", __FUNCTION__);
    anctool_api_init(&app_anctool_data);
    __this->para = anc_api_get_train_param();
}

void app_anctool_spp_disconnect(void)
{
    anctool_printf("%s, spp_disconnect!\n", __FUNCTION__);
    __this->anc_designer = 0;
    __this->connected_flag = 0;
#if ANCTOOL_NEED_PAIR_KEY
    __this->pair_flag = 0;
#endif
    anctool_api_uninit();
}

u8 get_app_anctool_spp_connected_flag()
{
    if (!__this) {
        return 0;
    }
    return __this->connected_flag;
}

#endif
