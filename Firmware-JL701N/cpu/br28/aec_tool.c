#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_online_cfg.h"
#include "online_db_deal.h"
#include "math.h"
#include "config/config_target.h"
#include "config/config_interface.h"
#include "aec_user.h"

extern int aec_cfg_online_init();
extern int aec_cfg_online_update(int root_cmd, void *cfg);
extern int get_aec_config(u8 *buf, int version);

#if TCFG_AEC_TOOL_ONLINE_ENABLE

/* #define LOG_TAG_CONST    AEC_TOOL */
#define LOG_TAG     "[AEC-TOOL]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#include "debug.h"

const u8 audio_sdk_name[16] = "AC701N";
/* const u8 audio_aec_ver[4]       = {1, 7, 1, 1}; */
const u8 audio_aec_ver[4]       = {1, 7, 1, 2};

#define audio_eq_password       "000"
#define password_en   0

enum {
    AEC_ONLINE_CMD_GETVER = 0x5,
    AEC_ONLINE_CMD_GET_SOFT_SECTION,//br22专用
    AEC_ONLINE_CMD_GET_SECTION_NUM = 0x7,//工具查询 小机需要的eq段数
    AEC_ONLINE_CMD_GLOBAL_GAIN_SUPPORT_FLOAT = 0x8,


    AEC_ONLINE_CMD_PASSWORD = 0x9,
    AEC_ONLINE_CMD_VERIFY_PASSWORD = 0xA,
    AEC_ONLINE_CMD_FILE_SIZE = 0xB,
    AEC_ONLINE_CMD_FILE = 0xC,
    AEC_ONLINE_CMD_GET_SECTION_NUM_NEW = 0xD,//该命令新加与 0x7功能一样
    AEC_ONLINE_CMD_CHANGE_MODE = 0xE,//切模式

    AEC_ONLINE_CMD_MODE_COUNT = 0x100,//模式个数a  1
    AEC_ONLINE_CMD_MODE_NAME = 0x101,//模式的名字a eq
    AEC_ONLINE_CMD_MODE_GROUP_COUNT = 0x102,//模式下组的个数,4个字节 1
    AEC_ONLINE_CMD_MODE_GROUP_RANGE = 0x103,//模式下组的id内容  0x11
    AEC_ONLINE_CMD_EQ_GROUP_COUNT = 0x104,//eq组的id个数  1
    AEC_ONLINE_CMD_EQ_GROUP_RANGE = 0x105,//eq组的id内容 0x11
    AEC_ONLINE_CMD_MODE_SEQ_NUMBER = 0x106,//mode的编号  magic num


    AEC_ONLINE_CMD_MAX,//最后一个
};

typedef struct _aec_tool_cfg {
    u8 mode_index;       //模式序号
    u8 *mode_name;       //模式名
    u32 mode_seq;        //模式的seq,用于识别离线文件功能类型
    u8 fun_num;          //模式下有多少种功能
    u16 fun_type[6];     //模式下拥有哪些功能
} aec_tool_cfg;

typedef struct _aec_parm {
    u8 app: 1; //是否手机app在线调试
    u8 online_en: 1;
    aec_tool_cfg *aec_tool_tab;
} aec_adjust_parm;

typedef struct {
    u32 aec_updata;
    spinlock_t lock;

    u8 app: 1;
    u8 online_en: 1;
    u8 password_ok: 1;
    u8 parse_seq;
    aec_tool_cfg *aec_tool_tab;
} AEC_CFG;

static AEC_CFG *p_aec_cfg = NULL;


/*----------------------------------------------------------------------------*/
/**@brief    在线调试，应答接口
   @param    *packet
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void ci_send_packet_new(AEC_CFG *aec_cfg, u32 id, u8 *packet, int size)
{
    ASSERT(aec_cfg);
    if (aec_cfg->app) {
        if (AEC_CONFIG_ID == id) {
            app_online_db_ack(aec_cfg->parse_seq, packet, size);
            return;
        }
    }
#if TCFG_ONLINE_ENABLE
    ci_send_packet(id, packet, size);
#endif
}

/*
 *aec 在线调试，系数解析函数
 * */
static s32 aec_online_update(AEC_CFG *aec_cfg, void *_packet)
{
    typedef struct {
        int cmd;     			///<AEC_ONLINE_CMD
        int data[45];       	///<data
    } AEC_ONLINE_PACKET;

    AEC_ONLINE_PACKET *packet = _packet;
    int ret  = -1;

    log_d("aec_cmd:0x%x ", packet->cmd);
    spin_lock(&aec_cfg->lock);
    ret = aec_cfg_online_update(packet->cmd, packet->data);
    spin_unlock(&aec_cfg->lock);

    return ret;
}


/*
 * 打开工具时，相关的命令通讯
 * */
static int aec_online_nor_cmd(AEC_CFG *aec_cfg, void *_packet)
{
    typedef struct {
        int cmd;     			///<AEC_ONLINE_CMD
        int data[45];       	///<data
    } AEC_ONLINE_PACKET;

    AEC_ONLINE_PACKET *packet = _packet;

    aec_tool_cfg *aec_tool_tab = aec_cfg->aec_tool_tab;
    u32 id = AEC_CONFIG_ID;
    if (packet->cmd == AEC_ONLINE_CMD_GETVER) {
        struct eq_ver_info {
            char sdkname[16];
            u8 eqver[4];
        };
        struct eq_ver_info eq_ver_info = {0};
        memcpy(eq_ver_info.sdkname, audio_sdk_name, sizeof(audio_sdk_name));
        memcpy(eq_ver_info.eqver, audio_aec_ver, sizeof(audio_aec_ver));
        ci_send_packet_new(aec_cfg, id, (u8 *)&eq_ver_info, sizeof(struct eq_ver_info));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_GET_SECTION_NUM || (packet->cmd == AEC_ONLINE_CMD_GET_SECTION_NUM_NEW)) {
        struct _cmd {
            int id;
            int groupId;
        };
        struct _cmd cmd = {0};

        memcpy(&cmd, packet->data, sizeof(struct _cmd));
        uint8_t hw_section = 0;
        log_d("hw_section %d cmd.id:%x\n", hw_section, cmd.id);
        ci_send_packet_new(aec_cfg, id, (u8 *)&hw_section, sizeof(uint8_t));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_GLOBAL_GAIN_SUPPORT_FLOAT) {
        uint8_t support_float = 1;
        ci_send_packet_new(aec_cfg, id, (u8 *)&support_float, sizeof(uint8_t));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_PASSWORD) {
        uint8_t password = 0;
        if (password_en) {
            password = 1;
        }
        ci_send_packet_new(aec_cfg, id, (u8 *)&password, sizeof(uint8_t));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_VERIFY_PASSWORD) {
        //check password
        int len = 0;
        char pass[64];
        typedef struct password {
            int len;
            char pass[45];
        } PASSWORD;

        PASSWORD ps = {0};
        spin_lock(&aec_cfg->lock);
        memcpy(&ps, packet->data, sizeof(PASSWORD));
        memcpy(&ps, packet->data, sizeof(int) + ps.len);
        spin_unlock(&aec_cfg->lock);

        uint8_t password_ok = 0;
        if (!strcmp(ps.pass, audio_eq_password)) {
            password_ok = 1;
        } else {
            log_error("password verify failxx \n");
        }
        aec_cfg->password_ok = password_ok;
        ci_send_packet_new(aec_cfg, id, (u8 *)&password_ok, sizeof(uint8_t));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_FILE_SIZE) {
        y_printf("get aec file_size");
#if TCFG_AUDIO_DUAL_MIC_ENABLE
#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
        u32 cfg_file_size = sizeof(AEC_DMS_CONFIG);
#else/*TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE*/
        u32 cfg_file_size = sizeof(DMS_FLEXIBLE_CONFIG);
#endif/*TCFG_AUDIO_DMS_SEL*/
#else/*SINGLE MIC*/
        u32 cfg_file_size = sizeof(AEC_CONFIG);
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
        ci_send_packet_new(aec_cfg, id, (u8 *)&cfg_file_size, sizeof(u32));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_FILE) {
        y_printf("get aec file");
        put_buf(packet, 64);
#if TCFG_AUDIO_DUAL_MIC_ENABLE
#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
        AEC_DMS_CONFIG cfg_file;
#else/*TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE*/
        DMS_FLEXIBLE_CONFIG cfg_file;
#endif/*TCFG_AUDIO_DMS_SEL*/
#else/*SINGLE MIC*/
        AEC_CONFIG cfg_file;
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
        int version = 0;
        memcpy(&version, packet->data, 4);
        printf("version %x", version);
        int cfg_file_len = get_aec_config(&cfg_file, version);
        ci_send_packet_new(aec_cfg, id, (u8 *)&cfg_file, cfg_file_len);
        return 0;//-EINVAL;
    } else if (packet->cmd == AEC_ONLINE_CMD_MODE_COUNT) {
        //模式个数
        int mode_cnt = 1;//aec_cfg->mode_num;
        ci_send_packet_new(aec_cfg, id, (u8 *)&mode_cnt, sizeof(int));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_MODE_NAME) {
        //utf8编码得名字
        struct cmd {
            int id;
            int modeId;
        };
        struct cmd cmd;
        memcpy(&cmd, packet, sizeof(struct cmd));

        u8 tmp[32] = {0};
        u8 name_len =  strlen(aec_tool_tab[cmd.modeId].mode_name);
        memcpy(tmp, aec_tool_tab[cmd.modeId].mode_name, name_len);
        ci_send_packet_new(aec_cfg, id, (u8 *)tmp, name_len);
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_MODE_GROUP_COUNT) {
        struct cmd {
            int id;
            int modeId;
        };
        struct cmd cmd;
        memcpy(&cmd, packet, sizeof(struct cmd));
        int groups_num = aec_tool_tab[cmd.modeId].fun_num;
        ci_send_packet_new(aec_cfg, id, (u8 *)&groups_num, sizeof(int));
        return 0;

    } else if (packet->cmd == AEC_ONLINE_CMD_MODE_GROUP_RANGE) { //摸下是组的id
        struct cmd {
            int id;
            int modeId;
            int offset;
            int count;
        };
        struct cmd cmd;
        memcpy(&cmd, packet, sizeof(struct cmd));

        u16 *group_tmp = aec_tool_tab[cmd.modeId].fun_type;//groups[cmd.modeId];
        ci_send_packet_new(aec_cfg, id, (u8 *)&group_tmp[cmd.offset], cmd.count * sizeof(u16));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_EQ_GROUP_COUNT) {
        u32 eq_group_num = 0;
        ci_send_packet_new(aec_cfg, id, (u8 *)&eq_group_num, sizeof(u32));
        return 0;
    } else if (packet->cmd == AEC_ONLINE_CMD_EQ_GROUP_RANGE) {
    } else if (packet->cmd == AEC_ONLINE_CMD_CHANGE_MODE) {
    } else if (packet->cmd == AEC_ONLINE_CMD_MODE_SEQ_NUMBER) {
        struct cmd {
            int id;
            int modeId;
        };
        struct cmd cmd;
        memcpy(&cmd, packet, sizeof(struct cmd));
        ci_send_packet_new(aec_cfg, id, (u8 *)&aec_tool_tab[cmd.modeId].mode_seq, sizeof(u32));

        return 0;
    }
    return -EINVAL;
}
/*
 *注册到协议端的,数据回调接口
 * */
static void aec_online_callback(uint8_t *packet, uint16_t size)
{
    s32 res = 0;
    AEC_CFG *aec_cfg = p_aec_cfg;
    if (!aec_cfg) {
        return ;
    }
    //u8 *tmp_packet = malloc(size);
    //memcpy(tmp_packet, packet, size);
    u8 *tmp_packet = packet;
    ASSERT(((int)tmp_packet & 0x3) == 0, "buf %x size %d\n", tmp_packet, size);
    res = aec_online_update(aec_cfg, (void *)tmp_packet);

    u32 id = AEC_CONFIG_ID;

    if (res == 0) {
        log_d("Ack");
        ci_send_packet_new(aec_cfg, id, (u8 *)"OK", 2);
    } else {
        res = aec_online_nor_cmd(aec_cfg, (void *)tmp_packet);
        if (res == 0) {
            //free(tmp_packet);
            return ;
        }
        log_d("Nack");
        ci_send_packet_new(aec_cfg, id, (u8 *)"ER", 2);
    }
    //free(tmp_packet);
}



REGISTER_CONFIG_TARGET(aec_config_target) = {
    .id         = AEC_CONFIG_ID,
    .callback   = aec_online_callback,
};

//AEC在线调试不进power down
static u8 aec_online_idle_query(void)
{
    AEC_CFG *cfg = p_aec_cfg;
    if (!cfg) {
        return 1;
    }
    if (cfg->online_en) {
        return 0;
    }
    return 1;
}

REGISTER_LP_TARGET(aec_online_lp_target) = {
    .name = "aec_online",
    .is_idle = aec_online_idle_query,
};


/*----------------------------------------------------------------------------*/
/**@brief    手机app 在线调时，数据解析的回调
   @param    *packet
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int aec_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    AEC_CFG *aec_cfg = p_aec_cfg;
    ASSERT(aec_cfg);
    if (aec_cfg->online_en) {
        aec_cfg->parse_seq = ext_data[1];
        aec_online_callback(packet, size);
    } else {
        printf("AEC_ONLINE,not enable!\n");
    }
    return 0;
}

void *aec_cfg_open(aec_adjust_parm *parm)
{
    AEC_CFG *aec_cfg = NULL;
    if (aec_cfg == NULL) {
        aec_cfg = zalloc(sizeof(AEC_CFG));
        ASSERT(aec_cfg);
    }

    aec_cfg->aec_tool_tab = parm->aec_tool_tab;
    aec_cfg->app = parm->app;
    aec_cfg->online_en = parm->online_en;

    spin_lock_init(&aec_cfg->lock);
    p_aec_cfg = aec_cfg;
    return aec_cfg;

}

void aec_cfg_close(AEC_CFG *aec_cfg)
{
    if (!aec_cfg) {
        return ;
    }

    free(aec_cfg);
    aec_cfg = NULL;
}

aec_tool_cfg aec_tool_tab[] = {
    {0,	(u8 *)"AEC tool", AEC_CONFIG_ID, 1, {0x3000, 0x3100, 0x3200, 0x3300, 0x3400, 0x3500}},
};

/*----------------------------------------------------------------------------*/
/**@brief   设置CVP算法类型
   @param   cvp_mode 配置的算法类型
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void cvp_mode_set(u16 cvp_mode)
{
    printf("cvp_mode_set:0x%x\n", cvp_mode);
    aec_tool_tab[0].fun_type[0] = cvp_mode;
}

int aec_cfg_init(void)
{
#if TCFG_AUDIO_DUAL_MIC_ENABLE
#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    cvp_mode_set(0x3100);
#else/*CVP_DNS_MODE*/
    cvp_mode_set(0x3300);
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
#else/*TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE*/
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    cvp_mode_set(0x3400);
#else/*CVP_DNS_MODE*/
    cvp_mode_set(0x3500);
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
#endif/*TCFG_AUDIO_DMS_SEL*/
#else/*SINGLE MIC*/
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    cvp_mode_set(0x3000);
#else/*CVP_DNS_MODE*/
    cvp_mode_set(0x3200);
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
    aec_adjust_parm parm = {0};
    parm.online_en = 1;
#if APP_ONLINE_DEBUG
    parm.app = 1;
#endif
    parm.aec_tool_tab = aec_tool_tab;
    AEC_CFG *aec_cfg = aec_cfg_open(&parm);
    if (aec_cfg->app) {
        app_online_db_register_handle(DB_PKT_TYPE_AEC, aec_app_online_parse);
    }
    aec_cfg_online_init();

    return 0;
}
__initcall(aec_cfg_init);
#endif
