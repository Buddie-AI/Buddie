#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))

#ifndef _EQ_CONFIG_H_
#define _EQ_CONFIG_H_

#include "application/audio_eq.h"
#include "application/audio_drc.h"
#include "application/audio_vbass.h"
#include "spinlock.h"
#include "math.h"

/*-----------------------------------------------------------*/
/*eq online cmd*/
enum {
    EQ_ONLINE_CMD_SECTION       = 1,
    EQ_ONLINE_CMD_GLOBAL_GAIN,
    EQ_ONLINE_CMD_LIMITER,
    EQ_ONLINE_CMD_INQUIRE,
    EQ_ONLINE_CMD_GETVER,
    EQ_ONLINE_CMD_GET_SOFT_SECTION,//br22专用
    EQ_ONLINE_CMD_GET_SECTION_NUM = 0x7,//工具查询 小机需要的eq段数
    EQ_ONLINE_CMD_GLOBAL_GAIN_SUPPORT_FLOAT = 0x8,


    EQ_ONLINE_CMD_PASSWORD = 0x9,
    EQ_ONLINE_CMD_VERIFY_PASSWORD = 0xA,
    EQ_ONLINE_CMD_FILE_SIZE = 0xB,
    EQ_ONLINE_CMD_FILE = 0xC,
    EQ_EQ_ONLINE_CMD_GET_SECTION_NUM = 0xD,//该命令新加与 0x7功能一样
    EQ_EQ_ONLINE_CMD_CHANGE_MODE = 0xE,//切模式

    EQ_ONLINE_CMD_MODE_COUNT = 0x100,//模式个数a  1
    EQ_ONLINE_CMD_MODE_NAME = 0x101,//模式的名字a eq
    EQ_ONLINE_CMD_MODE_GROUP_COUNT = 0x102,//模式下组的个数,4个字节 1
    EQ_ONLINE_CMD_MODE_GROUP_RANGE = 0x103,//模式下组的id内容  0x11
    EQ_ONLINE_CMD_EQ_GROUP_COUNT = 0x104,//eq组的id个数  1
    EQ_ONLINE_CMD_EQ_GROUP_RANGE = 0x105,//eq组的id内容 0x11
    EQ_ONLINE_CMD_MODE_SEQ_NUMBER = 0x106,//mode的编号  magic num


    EQ_ONLINE_CMD_PARAMETER_SEG = 0x11,
    EQ_ONLINE_CMD_PARAMETER_TOTAL_GAIN,
    EQ_ONLINE_CMD_PARAMETER_LIMITER,
    EQ_ONLINE_CMD_PARAMETER_DRC,
    EQ_ONLINE_CMD_PARAMETER_CHANNEL,//通道切换


    EQ_ONLINE_CMD_SONG_EQ_SEG = 0x2001,
    EQ_ONLINE_CMD_CALL_EQ_SEG = 0x2002,
    EQ_ONLINE_CMD_AEC_EQ_SEG = 0x2003,


    EQ_ONLINE_CMD_SONG_EQ_V1_SEG = 0x2011,// float gain, float q
    EQ_ONLINE_CMD_CALL_EQ_V1_SEG = 0x2012,
    EQ_ONLINE_CMD_AEC_EQ_V1_SEG = 0x2013,
    EQ_ONLINE_CMD_HEARING_AID_EQ_SEG    = 0x2014,// float gain, float q
    EQ_ONLINE_CMD_HEARING_AID_EQ_SEG2    = 0x2015,// float gain, float q


    EQ_ONLINE_CMD_SONG_DRC = 0x2101,
    EQ_ONLINE_CMD_SONG_WDRC = 0x2102,
    EQ_ONLINE_CMD_SONG_MULTI_WDRC = 0x2104,

    EQ_ONLINE_CMD_VIRTUAL_BASS = 0x2203,
//add xx here

    EQ_ONLINE_CMD_MAX,//最后一个
};

#define CONSTRAINT_OP_DRC_L_R_CH  0x01
#define CONSTRAINT_OP_DRC_WDRC_SINGLE_CH 0x02
#define CONSTRAINT_OP_DRC_WDRC_DUAL_CH 0x03


/*eq online packet*/
typedef struct {
    int cmd;     			///<EQ_ONLINE_CMD
    int data[45];       	///<data
} EQ_ONLINE_PACKET;


/*EQ_ONLINE_CMD_PARAMETER_SEG*/
typedef struct eq_seg_info EQ_ONLINE_PARAMETER_SEG;

/*-----------------------------------------------------------*/
typedef struct eq_seg_info EQ_CFG_SEG;

typedef struct {
    float global_gain;    //总增益
    int seg_num;          //eq效果文件存储的段数
    int enable_section;   //
} CFG_PARM;


#define SECTION_MAX 20
typedef struct {
    CFG_PARM par;
    EQ_CFG_SEG seg[SECTION_MAX];   //eq系数存储地址
} EQ_CFG_PARAMETER;


typedef struct {
    struct drc_ch drc;    //drc系数地址
} DRC_CFG_PARAMETER;


typedef struct {
    /* unsigned short crc; */
    unsigned short id;    //eq效果文件存储的标识
    unsigned short len;   //当前标识结构长度
} EQ_FILE_HEAD;

typedef struct {
    EQ_FILE_HEAD head;
    EQ_CFG_PARAMETER parm;
}  EQ_CFG_PARM;           //效果文件中，eq存储结构

typedef struct {
    EQ_FILE_HEAD head;
    DRC_CFG_PARAMETER parm;
}  DRC_CFG_PARM;          //效果文件中，drc存储结构

typedef struct {
    EQ_FILE_HEAD head;
    struct virtual_bass_tool_set parm;
}  VBASS_CFG_PARM;          //效果文件中，drc存储结构


typedef struct {
    EQ_CFG_PARM song_eq_parm;
    DRC_CFG_PARM drc_parm;
    VBASS_CFG_PARM vbass_parm;
    u8 cur_mode;
} EQ_FILE_PARM;           //eq drc效果系数，统一管理

typedef struct {
    u16 tmr;
    u8 *fade_stu;
    u32 *sr;
    EQ_CFG_SEG *seg;
} EQ_FADE_CFG;            //系数eq系数调节,支持淡入淡出，防止增益跳跃过大，造成哒哒音

typedef struct _eq_tool_cfg {
    u8 mode_index;       //模式序号
    u8 *mode_name;       //模式名
    u32 mode_seq;        //模式的seq,用于识别离线文件功能类型
    u8 section;          //每个模式下eq的段数
    u8 fun_num;          //模式下有多少种功能
    u16 fun_type[3];     //模式下拥有哪些功能
} eq_tool_cfg;           //调试工具支持的功能

typedef struct {
    u32 eq_type : 3;      //系数调试所处模式：在线模式、效果文件（离线）模式、默认系数表模式
    u32 *mode_updata;     //默认系数表切换更新标志
    u32 *drc_updata;      //drc系数更新标志
    u32 *online_updata;   //在线调试系数更新标志
    u32 *design_mask;     //在线调试，哪一段eq需要更新标志
    u32 *seg_num;         //当前模式(播歌、通话宽频、窄频上下行模式标号),所拥有的eq段数
    EQ_FILE_PARM *cfg_parm;//eq drc效果系数，统一管理
    u32 *cur_sr;          //当前模式(播歌、通话宽频、窄频上下行模式标号),记录的采样率
    spinlock_t lock;      //自旋锁
    u8 mode_id;           //当前模式(播歌、通话宽频、窄频上下行模式标号),在线调试时，内部使用

    u8 mode_num;          //模式总数(播歌、通话宽频、窄频上下行模式标号)
    u8 online_en: 1;      //是否支持在线调试
    u8 fade_en: 1;        //是否支持淡入淡出
    u8 stero: 1;          //左右声道效果是否支持拆分
    u8 drc: 1;            //是否支持drc调试
    u8 tws: 1;            //是否支持tws调试
    u8 app: 1;            //是否支持app调试
    u8 limit_zero: 1;     //是否将系数限制到0
    u8 eq_file_ver_err: 1;//记录效果文件版本是否匹配
    u8 eq_file_section_err : 1;//记录效果文件eq段数是否匹配
    u8 vbass;
    u8 vbass_file;
    u8 type_num;          //默认eq系数表总个数
    u8 section_max;       //eq最大段数
    u8 eq_mode;           //记录当前使用哪个eq系数表
    u8 parse_seq;         //记录app调试的seq
    uint8_t password_ok;  //在线调试记录密码是否正确
    float *eq_coeff_tab;    //eq在线多模式系数表,[MODE_NUM_MAX][SECTION_MAX * 5];

    void *tws_ci;         //tws收发句柄
    int tws_tmr;          //系数收发timer
    u16 *tws_pack;        //收发的数据包地址
    u8 pack_id;           //收发数据包标识

    EQ_FADE_CFG *fade_cfg;//系数淡入淡出句柄
    int *eq_type_tab;     //eq 默认系数表,计算前seg信息存储地址[EQ_MODE_MAX];
    float *type_gain_tab; //song_eq_mode eq 默认系数表的对应的总增益
    u8 *eq_mode_use_idx;  //10段内抽取部分段数
    eq_tool_cfg *eq_tool_tab;//在线调试工具支持的功能配置结构

    void *phone_eq_tab;//通话下行 eq系数表
    u8 phone_eq_tab_size;//通话下行eq系数表段数
    void *ul_eq_tab;//通话上行 eq系数表
    u8 ul_eq_tab_size;//通话上行eq系数表段数


    void *priv;
    int (*send_cmd)(void *priv, u32 id, u8 *packet, int size);//在线调试，命令应答
    u8 custom_mode_id;   //记录自定义eq系数表的数组所在下标号
    u16 timer;                      //总增益淡入timer
    float use_global_gain;          //总增益淡入目标值
    float global_gain_fade_step;    //总增益淡入步进
    u8 mode;                        //记录用于总增益淡入timer的mode
} EQ_CFG;

typedef struct _eq_parm {
    u8 mode_num;           //模式总数(播歌、通话宽频、窄频上下行模式标号)
    u8 online_en: 1;       //是否支持在线调试
    u8 fade_en: 1;         //是否支持淡入淡出
    u8 file_en: 1;         //是否支持效果文件解析
    u8 stero: 1;           //是否左右声道做不同eq
    u8 drc: 1;             //是否有drc
    u8 tws: 1;             //是否tws
    u8 app: 1;             //是否手机app在线调试
    u8 limit_zero: 1;      //是否将系数限制到0
    u8 type_num: 4;        //默认eq系数表总个数
    u8 section_max;        //eq最大段数
    void *eq_type_tab;     //eq 默认系数表,计算前seg信息存储地址[EQ_MODE_MAX];
    float *type_gain_tab;  //song_eq_mode eq 默认系数表的对应的总增益
    u8 *eq_mode_use_idx;   //10段内抽取部分段数
    eq_tool_cfg *eq_tool_tab;//在线调试工具支持的功能配置结构

    void *phone_eq_tab;    //通话下行 eq系数表
    u8 phone_eq_tab_size;  //通话下行eq系数表段数
    void *ul_eq_tab;       //通话上行 eq系数表
    u8 ul_eq_tab_size;     //通话上行eq系数表段数

} eq_adjust_parm;


/*模式序号*/
typedef enum {
    song_eq_mode  		 = 0,
    call_eq_mode  		 = 1,
    call_narrow_eq_mode  = 2,
    aec_eq_mode          = 3,
    aec_narrow_eq_mode   = 4,
    hearing_aid_mode     = 5,
    hearing_aid_mode2    = 6,
    fr_eq_mode    		 = 7,
    rl_eq_mode           = 8,
    rr_eq_mode           = 9,
    mic_eq_mode          = 10,
} MUSIC_MODE;

/*----------------------------------------------------------------------------*/
/**@brief   切换到指定的eq文件
   @param   index:数组中的文件序号
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void eq_file_set_by_index(u8 index);

/*----------------------------------------------------------------------------*/
/**@brief   根据eq_file_switch_list成员个数顺序切换eq文件
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void eq_file_switch();

/*----------------------------------------------------------------------------*/
/**@brief   eq系数表存放顺序
   @param   *eq_cfg:配置句柄
   @param   mode:播歌、通话宽频、窄频上下行模式标号
   @return
   @note  call_eq_mode|call_narrow_eq_mode|aec_eq_mode|aec_narrow_eq_mode|song_eq_mode|fr_eq_mode|rl_eq_mode|rr_eq_mode
*/
/*----------------------------------------------------------------------------*/
int *get_eq_coeff_tab(EQ_CFG *eq_cfg, MUSIC_MODE mode);
/*----------------------------------------------------------------------------*/
/**@brief    获取配置EQ_CFG *eq_cfg句柄
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void *get_eq_cfg_hdl();
/*----------------------------------------------------------------------------*/
/**@brief    seg值，限制到0
   @param
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void eq_seg_limit_zero(EQ_CFG_SEG *seg, u8 seg_num);
/*----------------------------------------------------------------------------*/
/**@brief    eq根据seg值，计算系数的并放到tar_tab
   @param    *eq_cfg:配置句柄
   @param    sr:采样率
   @param    mode:播歌、通话宽频、窄频上下行模式标号
   @param    *seg:计算前系数
   @param    *tar_tab:计算后系数存放地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void eq_coeff_set(EQ_CFG *eq_cfg, u32 sr, MUSIC_MODE mode, EQ_CFG_SEG *seg, int *tar_tab);

/*----------------------------------------------------------------------------*/
/**@brief    打开eq系数配置解析、在线调试配置等
   @param    *parm:系数解析、在线调试功能配置数据结构
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void *eq_cfg_open(eq_adjust_parm *parm);
/*----------------------------------------------------------------------------*/
/**@brief    手机app 在线调时，数据解析的回调
   @param    *packet
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int eq_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size);
/*----------------------------------------------------------------------------*/
/**@brief    设置 eq总增益
   @param    *eq_cfg:*eq_cfg:配置句柄
   @param    mode:播歌、通话宽频、窄频上下行模式标号
   @param   global_gain:总增益
   @return
   @note     call_eq_mode|call_narrow_eq_mode|aec_eq_mode|aec_narrow_eq_mode|song_eq_mode|fr_eq_mode|rl_eq_mode|rr_eq_mode
*/
/*----------------------------------------------------------------------------*/
void set_global_gain(EQ_CFG *eq_cfg, MUSIC_MODE mode, float global_gain);
/*----------------------------------------------------------------------------*/
/**@brief    获取 eq总增益
   @param    *eq_cfg:*eq_cfg:配置句柄
   @param    mode:播歌、通话宽频、窄频上下行模式标号
   @return
   @note     call_eq_mode|call_narrow_eq_mode|aec_eq_mode|aec_narrow_eq_mode|song_eq_mode|fr_eq_mode|rl_eq_mode|rr_eq_mode
*/
/*----------------------------------------------------------------------------*/
float get_glbal_gain(EQ_CFG *eq_cfg, MUSIC_MODE mode);

/*----------------------------------------------------------------------------*/
/**@brief   普通音频eq,无离线文件时，获取某eq效果模式的增益
   @param   mode:哪个模式
   @param   index:哪一段
   @return  增益
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
s8 eq_mode_get_gain(EQ_MODE mode, u16 index);
/*----------------------------------------------------------------------------*/
/**@brief   普通音频eq,无离线文件时，设置用户自定义eq效果模式时的增益
   @param   index:哪一段
   @param   gain:增益
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_param(u16 index, float gain);
/*----------------------------------------------------------------------------*/
/**@brief   普通音频eq,无离线文件时，获取用户自定义eq效果模式时的增益、频率
   @param   index:哪一段
   @param   freq:中心截止频率
   @param   gain:增益
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_info(u16 index, int freq, float gain);
/*----------------------------------------------------------------------------*/
/**@brief   普通音频eq,无离线文件时，用默认eq系数表的eq效果模式设置(设置模式,更新系数)
   @param   mode:EQ_MODE_NORMAL, EQ_MODE_ROCK,EQ_MODE_POP,EQ_MODE_CLASSIC,EQ_MODE_JAZZ,EQ_MODE_COUNTRY, EQ_MODE_CUSTOM
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int eq_mode_set(EQ_MODE mode);

/*----------------------------------------------------------------------------*/
/**@brief   eq模式切换
   @param
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int eq_mode_sw(void);

/*----------------------------------------------------------------------------*/
/**@brief   普通音频eq,无离线文件时，获取eq效果模式
   @param
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
EQ_MODE eq_mode_get_cur(void);



/*----------------------------------------------------------------------------*/
/**@brief    解析eq效果文件的系数
   @param    *eq_cfg:*eq_cfg:配置句柄
   @param    *path:eq效果文件路径
   @return
   @note  可用该接口切换eq效果文件，需使能宏TCFG_USE_EQ_FILE,以及在config_audio_eq_en上使能 EQ_FILE_SW_EN
*/
/*----------------------------------------------------------------------------*/
s32 eq_file_get_cfg(EQ_CFG *eq_cfg, u8 *path);

/*----------------------------------------------------------------------------*/
/**@brief    在线调试的回调
   @param    *packet:数据包
   @param    size:数据包长度
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void eq_online_callback(uint8_t *packet, uint16_t size);//
/*----------------------------------------------------------------------------*/
/**@brief   eq系数回调
   @param    *eq:eq句柄
   @param    sr:采样率
   @param    info:系数结构
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int eq_get_filter_info(void *_eq, int sr, struct audio_eq_filter_info *info);

/*----------------------------------------------------------------------------*/
/**@brief   drc系数回调
   @param    *drc:drc句柄
   @param    info:系数结构
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int drc_get_filter_info(void *_drc, struct audio_drc_filter_info *info);
/*----------------------------------------------------------------------------*/
/**@brief   aec eq系数 回调
   @param    *eq:eq句柄
   @param    sr:采样率
   @param    info:系数结构
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int aec_ul_eq_filter(void *_eq, int sr, struct audio_eq_filter_info *info);
/*----------------------------------------------------------------------------*/
/**@brief   通话eq系数 的回调函数
   @param    *eq:eq句柄
   @param    sr:采样率
   @param    info:系数结构
   @return
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int eq_phone_get_filter_info(void *_eq, int sr, struct audio_eq_filter_info *info);
/*----------------------------------------------------------------------------*/
/**@brief   drc系数更新检测
   @param    *drc:drc句柄
   @return
   @note    内部使用
*/
/*----------------------------------------------------------------------------*/
void drc_app_run_check(struct audio_drc *drc);

/*----------------------------------------------------------------------------*/
/**@brief   普通音频eq,无离线文件时，获取某eq效果模式的中心截止频率
   @param   mode:EQ_MODE_NORMAL, EQ_MODE_ROCK,EQ_MODE_POP,EQ_MODE_CLASSIC,EQ_MODE_JAZZ,EQ_MODE_COUNTRY, EQ_MODE_CUSTOM
   @param   index:哪一段
   @return  中心截止频率
   @note    外部使用
*/
/*----------------------------------------------------------------------------*/
int eq_mode_get_freq(EQ_MODE mode, u16 index);

/*----------------------------------------------------------------------------*/
/**@brief   eq系数更新检测
   @param    *eq:eq句柄
   @return
   @note    内部使用
*/
/*----------------------------------------------------------------------------*/
void eq_app_run_check(struct audio_eq *eq);



#if 0
static const struct eq_seg_info your_audio_out_eq_tab[] = {
#ifdef EQ_CORE_V1
    {0, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f)},
    {1, EQ_IIR_TYPE_BAND_PASS, 12000, 0, 0.3f)},
#else
    {0, EQ_IIR_TYPE_BAND_PASS, 125,   0 << 20, (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 12000, 0 << 20, (int)(0.3f * (1 << 24))},
#endif
};
float your_eq_coeff_tab[2][5];
/*----------------------------------------------------------------------------*/
/**@brief    用户自定义eq的系数回调
   @param    eq:句柄
   @param    sr:采样率
   @param    info: 系数地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_eq_get_filter_info_demo(void *_eq, int sr, struct audio_eq_filter_info *info)
{
    if (!sr) {
        sr = 44100;
    }
#if 0
    struct audio_eq *eq = (struct audio_eq *)_eq;
    local_irq_disable();
    u8 nsection = ARRAY_SIZE(your_audio_out_eq_tab);
    if (!eq->eq_seg_tab) {
        int size = sizeof(your_audio_out_eq_tab);
        eq->eq_seg_tab = zalloc(size);
        memcpy(eq->eq_seg_tab, your_audio_out_eq_tab, size);
    }
    if (!eq->eq_coeff_tab) {
        eq->eq_coeff_tab = zalloc(sizeof(int) * 5 * nsection);
    }
    for (int i = 0; i < nsection; i++) {
        eq_seg_design(&eq->eq_seg_tab[i], sr, &eq->eq_coeff_tab[5 * i]);
    }

    local_irq_enable();
    info->L_coeff = info->R_coeff = (void *)eq->eq_coeff_tab;
    info->L_gain = info->R_gain = 0;
    info->nsection = nsection;
#else
    local_irq_disable();
    u8 nsection = ARRAY_SIZE(your_audio_out_eq_tab);
    for (int i = 0; i < nsection; i++) {
        eq_seg_design(&your_audio_out_eq_tab[i], sr, &your_eq_coeff_tab[i]);
    }
    local_irq_enable();
    info->L_coeff = info->R_coeff = (void *)your_eq_coeff_tab;//系数指针赋值
    info->L_gain = info->R_gain = 0;//总增益填写，用户可修改（-20~20db）
    info->nsection = nsection;//eq段数，根据提供给的系数表来填写，例子是2

#endif
    return 0;
}

struct drc_ch drc_fliter = {0};
#define your_threshold  (0)
/*----------------------------------------------------------------------------*/
/**@brief    自定义限幅器系数回调
   @param    *drc: 句柄
   @param    *info: 系数结构地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int drc_get_filter_info_demo(void *drc, struct audio_drc_filter_info *info)
{
    int th = your_threshold;//-60 ~0db
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768); // 0db:32768, -60db:33

    drc_fliter.nband = 1;
    drc_fliter.type = 1;
    drc_fliter._p.limiter[0].attacktime = 5;
    drc_fliter._p.limiter[0].releasetime = 300;
    drc_fliter._p.limiter[0].threshold[0] = threshold;
    drc_fliter._p.limiter[0].threshold[1] = 32768;

    info->pch = info->R_pch = &drc_fliter;
    return 0;
}


struct drc_ch drc_fliter2 = {0};
#define your_threshold1  (0)
#define your_threshold2  (0)
/*----------------------------------------------------------------------------*/
/**@brief    自定义压缩器系数回调
   @param    *drc: 句柄
   @param    *info: 系数结构地址
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int drc_get_filter_info_demo2(void *drc, struct audio_drc_filter_info *info)
{
    int th1 = your_threshold1;//-60 ~0db
    int th2 = your_threshold2;//-60 ~0db

    int your_ratio0 = 800;//100~1000
    int your_ratio1 = 900;//100~1000


    if (th2 < th1) { //阈值2需大于等于阈值1
        th2 = th1;
    }
    if (your_ratio1 < your_ratio0) { //压缩比2需要大于等于压缩比1
        your_ratio1 = your_ratio0;
    }

    int threshold0 = roundf(powf(10.0f, th1 / 20.0f) * 32768); // 0db:32768, -60db:33
    int threshold1 = roundf(powf(10.0f, th2 / 20.0f) * 32768); // 0db:32768, -60db:33

    drc_fliter2.nband = 1;//全带
    drc_fliter2.type = 2;//压缩器
    drc_fliter2._p.compressor[0].attacktime = 5;
    drc_fliter2._p.compressor[0].releasetime = 300;
    drc_fliter2._p.compressor[0].threshold[0] = threshold0;
    drc_fliter2._p.compressor[0].threshold[1] = threshold1;
    drc_fliter2._p.compressor[0].threshold[2] = 32768;

    drc_fliter2._p.compressor[0].ratio[0] = 100;
    drc_fliter2._p.compressor[0].ratio[1] = your_ratio0;
    drc_fliter2._p.compressor[0].ratio[2] = your_ratio1;

    info->pch = info->R_pch = &drc_fliter2;
    return 0;
}



//修改自定义模式eq系数表，更新系数到eq 方法
// 板极头文件中 这两宏 配0
#define TCFG_EQ_ONLINE_ENABLE               0     //支持在线EQ调试,
#define TCFG_USE_EQ_FILE                    0    //离线eq使用配置文件还是默认系数表 1：使用文件  0 使用默认系数表


// 在sdk中实现以下接函数
/*----------------------------------------------------------------------------*/
/**@brief    设置用户自定义模式每段eq信息
   @param
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_info_user(struct eq_seg_info *seg)
{
    if (!config_audio_eq_file_en) {
        if (!seg) {
            return 0;
        }
        u16 index = seg->index;                       //第几段eq
        EQ_CFG *eq_cfg = get_eq_cfg_hdl();
        if (index >= eq_cfg->section_max) {
            return 0;
        }
        int *tmp = (int *)eq_cfg->eq_type_tab;
        EQ_CFG_SEG *tab_custom = (EQ_CFG_SEG *)tmp[EQ_MODE_CUSTOM];

        index = eq_cfg->eq_mode_use_idx[index];
        memcpy(&tab_custom[index], seg, sizeof(struct eq_seg_info));//更新参数到自定义模式系数表

        /* log_debug("idx:%d, iir:%d, freq:%d, gain:0x%x, q:0x%x ", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q); */
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief    获取某个模式eq表内，某一段eq的信息
   @param
   @param
   @return  返回eq信息
   @note
*/
/*----------------------------------------------------------------------------*/
struct eq_seg_info *eq_mode_get_custom_info_user(EQ_MODE mode, u8 index)
{
    if (!config_audio_eq_file_en) {

        EQ_CFG *eq_cfg = get_eq_cfg_hdl();
        if (index >= eq_cfg->section_max) {
            return 0;
        }
        int *tmp = (int *)eq_cfg->eq_type_tab;
        EQ_CFG_SEG *seg = (EQ_CFG_SEG *)tmp[mode];

        index = eq_cfg->eq_mode_use_idx[index];
        /* log_debug("idx:%d, iir:%d, freq:%d, gain:0x%x, q:0x%x ", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q); */
        return &seg[index];
    }
    return 0;
}

/*设置总增益后，需要设置更新*/
void update_global_gain_demo(float global_gain)
{
    EQ_CFG *eq_cfg = get_eq_cfg_hdl();
    set_global_gain(eq_cfg, song_eq_mode, global_gain);//设置总增益，
    eq_mode_set(EQ_MODE_CUSTOM);//设置自定义模式、更新系数以及总增益
}

/*设置参数后，需要设置更新*/
void update_eq_seg_info_demo(struct eq_seg_info *seg)
{
    eq_mode_set_custom_info_user(seg);//更新参数到自定义模式系数表
    eq_mode_set(EQ_MODE_CUSTOM);//设置自定义模式、更新系数以及总增益
}
void update_custom_info_demo2(u16 index, int freq, float gain)
{
    eq_mode_set_custom_info(index, freq, gain);/*改某一段eq的的中心截止频率 以及增益*/
    eq_mode_set(EQ_MODE_CUSTOM);//设置自定义模式、更新系数以及总增益
}

#endif

#endif
#endif//EQ_CORE_V1

