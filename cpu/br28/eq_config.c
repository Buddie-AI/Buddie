
#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_online_cfg.h"
#include "online_db/online_db_deal.h"
#include "config/config_interface.h"
#include "application/eq_config.h"
#include "audio_config.h"
#include "cfg_tool.h"

#ifdef CONFIG_EQ_APP_SEG_ENABLE
#pragma const_seg(	".eq_app_codec_const")
#pragma code_seg (	".eq_app_codec_code" )
#endif
const u8 audio_eq_sdk_name[16] 		= "AC701N";
#if defined(TCFG_AUDIO_MDRC_ENABLE) && TCFG_AUDIO_MDRC_ENABLE
const u8 audio_eq_ver[4] 			= {0, 8, 4, 0};//包含虚拟低音、多带drc、以及辅听耳机音效
#else
#if defined(TCFG_AUDIO_HEARING_AID_ENABLE) && TCFG_AUDIO_HEARING_AID_ENABLE
const u8 audio_eq_ver[4] 			= {0, 8, 3, 0};
#else
const u8 audio_eq_ver[4] 			= {0, 8, 0, 0};
#endif
#endif /*TCFG_AUDIO_HEARING_AID_ENABLE*/

#if TCFG_EQ_FILE_SWITCH_EN
const u8 *eq_file_switch_list[] = {
    (u8 *)SDFILE_RES_ROOT_PATH"eq_cfg_hw.bin",
    (u8 *)SDFILE_RES_ROOT_PATH"eq_cfg_hw1.bin",
    (u8 *)SDFILE_RES_ROOT_PATH"eq_cfg_hw2.bin",
};
#endif

//eq_cfg_hw.bin中播歌eq曲线，当作用户自定义模式，参与效果切换.
//通话宽频上下行eq曲线也对应放到phone_eq_tab_normal、ul_eq_tab_normal
//EQ_FILE_CP_TO_CUSTOM 1使能时，同时板极文件中 TCFG_USE_EQ_FILE 配 0
#define EQ_FILE_CP_TO_CUSTOM  0


/*************************非用户配置区*************************/
#if TCFG_EQ_ONLINE_ENABLE
#undef EQ_FILE_CP_TO_CUSTOM
#define EQ_FILE_CP_TO_CUSTOM  0
#endif
#ifndef TCFG_USER_EQ_MODE_NUM
#define TCFG_USER_EQ_MODE_NUM 0
#endif
#ifndef APP_ONLINE_DEBUG
#define APP_ONLINE_DEBUG 0
#endif


#ifndef TCFG_CALL_DL_EQ_SECTION
#define TCFG_CALL_DL_EQ_SECTION  3
#endif
#ifndef TCFG_CALL_UL_EQ_SECTION
#define TCFG_CALL_UL_EQ_SECTION  3
#endif
/*************************************************************/

#if (TCFG_EQ_ENABLE != 0)

#if (RCSP_ADV_EN)&&(JL_EARPHONE_APP_EN)&&(TCFG_DRC_ENABLE == 0)
#define AUDIO_EQ_Q          1.5f
#else
#define AUDIO_EQ_Q          0.7f
#endif


#if !TCFG_USE_EQ_FILE
const struct eq_seg_info eq_tab_normal[] = {
#if TCFG_USER_EQ_MODE_NUM > 0
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, AUDIO_EQ_Q},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, AUDIO_EQ_Q},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0, AUDIO_EQ_Q},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0, AUDIO_EQ_Q},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, AUDIO_EQ_Q},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0, AUDIO_EQ_Q},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0, AUDIO_EQ_Q},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, AUDIO_EQ_Q},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, AUDIO_EQ_Q},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, AUDIO_EQ_Q},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
#endif

#endif
};

const struct eq_seg_info eq_tab_rock[] = {

#if TCFG_USER_EQ_MODE_NUM > 1
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    -2, AUDIO_EQ_Q},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0, AUDIO_EQ_Q},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    2, AUDIO_EQ_Q},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4, AUDIO_EQ_Q},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -2, AUDIO_EQ_Q},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  -2, AUDIO_EQ_Q},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0, AUDIO_EQ_Q},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0, AUDIO_EQ_Q},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   4, AUDIO_EQ_Q},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4, AUDIO_EQ_Q},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
#endif

#endif
};

const struct eq_seg_info eq_tab_pop[] = {

#if TCFG_USER_EQ_MODE_NUM > 2
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     3, AUDIO_EQ_Q},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     1, AUDIO_EQ_Q},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0, AUDIO_EQ_Q},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   -2, AUDIO_EQ_Q},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -4, AUDIO_EQ_Q},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  -4, AUDIO_EQ_Q},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  -2, AUDIO_EQ_Q},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0, AUDIO_EQ_Q},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   1, AUDIO_EQ_Q},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  2, AUDIO_EQ_Q},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
#endif

#endif
};

const struct eq_seg_info eq_tab_classic[] = {

#if TCFG_USER_EQ_MODE_NUM > 3
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     0, AUDIO_EQ_Q},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     8, AUDIO_EQ_Q},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    8, AUDIO_EQ_Q},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4, AUDIO_EQ_Q},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    0, AUDIO_EQ_Q},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   0, AUDIO_EQ_Q},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0, AUDIO_EQ_Q},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0, AUDIO_EQ_Q},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   2, AUDIO_EQ_Q},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  2, AUDIO_EQ_Q},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
#endif

#endif
};

const struct eq_seg_info eq_tab_country[] = {
#if TCFG_USER_EQ_MODE_NUM > 5
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     -2, AUDIO_EQ_Q},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0, AUDIO_EQ_Q},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0, AUDIO_EQ_Q},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    2, AUDIO_EQ_Q},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    2, AUDIO_EQ_Q},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   0, AUDIO_EQ_Q},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0, AUDIO_EQ_Q},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0, AUDIO_EQ_Q},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   4, AUDIO_EQ_Q},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4, AUDIO_EQ_Q},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
#endif
#endif
};

const struct eq_seg_info eq_tab_jazz[] = {
#if TCFG_USER_EQ_MODE_NUM > 4
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     0, AUDIO_EQ_Q},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0, AUDIO_EQ_Q},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0, AUDIO_EQ_Q},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4, AUDIO_EQ_Q},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    4, AUDIO_EQ_Q},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   4, AUDIO_EQ_Q},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0, AUDIO_EQ_Q},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   2, AUDIO_EQ_Q},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   3, AUDIO_EQ_Q},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4, AUDIO_EQ_Q},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
#endif
#endif

};
struct eq_seg_info eq_tab_custom[] = {
#if TCFG_USER_EQ_MODE_NUM > 6
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, AUDIO_EQ_Q},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, AUDIO_EQ_Q},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0, AUDIO_EQ_Q},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0, AUDIO_EQ_Q},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, AUDIO_EQ_Q},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0, AUDIO_EQ_Q},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0, AUDIO_EQ_Q},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, AUDIO_EQ_Q},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, AUDIO_EQ_Q},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, AUDIO_EQ_Q},

#if (EQ_SECTION_MAX > 10)
    //10段之后频率值设置96k,目的是让10段之后的eq走直通
    {10, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {11, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {12, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {13, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {14, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {15, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {16, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {17, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {18, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
    {19, EQ_IIR_TYPE_BAND_PASS, 96000, 0, AUDIO_EQ_Q},
#endif

#endif
};


const EQ_CFG_SEG *eq_type_tab[EQ_MODE_MAX] = {
    eq_tab_normal, eq_tab_rock, eq_tab_pop, eq_tab_classic, eq_tab_jazz, eq_tab_country, eq_tab_custom
};
// 默认系数表，每个表对应的总增益,用户可修改
float type_gain_tab[EQ_MODE_MAX] = {0, 0, 0, 0, 0, 0, 0};
#endif

__attribute__((weak)) u32 get_eq_mode_tab(void)
{
#if !TCFG_USE_EQ_FILE
    return (u32)eq_type_tab;
#else
    return 0;
#endif
}

__attribute__((weak)) u8 get_eq_mode_max(void)
{
    return EQ_MODE_MAX;
}

#if (EQ_SECTION_MAX==9)
static const u8 eq_mode_use_idx[] = {
    0,	1,	2,	3,	4,	5,	/*6,*/	7,	8,	9
};
#elif (EQ_SECTION_MAX==8)
static const u8 eq_mode_use_idx[] = {
    0,	/*1,*/	2,	3,	4,	5,	6,	7,	/*8,*/	9
};
#elif (EQ_SECTION_MAX==7)
static const u8 eq_mode_use_idx[] = {
    0,	/*1,*/	2,	3,	4,	5,	/*6,*/	7,	/*8,*/	9
};
#elif (EQ_SECTION_MAX==6)
static const u8 eq_mode_use_idx[] = {
    0,	/*1,*/	2,	3,	4,	/*5,*/	/*6,*/	7,	/*8,*/	9
};
#elif (EQ_SECTION_MAX==5)
static const u8 eq_mode_use_idx[] = {
    /*0,*/	1,	/*2,*/	3,	/*4,*/	5,	/*6,*/	7,	/*8,*/	9
};
#else
static const u8 eq_mode_use_idx[] = {
    0,	1,	2,	3,	4,	5,	6,	7,	8,	9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};
#endif
/*
 *通话下行eq系数表
 * */
#if TCFG_EQ_ENABLE && TCFG_PHONE_EQ_ENABLE
#if EQ_FILE_CP_TO_CUSTOM
struct eq_seg_info phone_eq_tab_normal[] = {
#else
const struct eq_seg_info phone_eq_tab_normal[] = {
#endif
    {0, EQ_IIR_TYPE_HIGH_PASS, 200,   0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 300,   0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
#if TCFG_CALL_DL_EQ_SECTION > 3
    {3, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
#endif
};
#endif

/*
 *通话上行eq系数表
 * */
#if EQ_FILE_CP_TO_CUSTOM
struct eq_seg_info ul_eq_tab_normal[] = {
#else
const struct eq_seg_info ul_eq_tab_normal[] = {
#endif
    {0, EQ_IIR_TYPE_HIGH_PASS, 200,   0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 300,   0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
#if TCFG_CALL_UL_EQ_SECTION > 3
    {3, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 400,   0, 0.7f},
#endif

};



#define SONG_SECTION  EQ_SECTION_MAX
#define CALL_SECTION  TCFG_CALL_DL_EQ_SECTION//下行段数,小于等于SONG_SECTION
#define UL_SECTION    TCFG_CALL_UL_EQ_SECTION//上行段数,小于等于SONG_SECTION
/*
 *下行的宽频和窄频段数需一致，上行的宽频和窄频段数需要一致
 *表的每一项顺序不可修改
 * */
eq_tool_cfg eq_tool_tab[] = {
    {call_eq_mode, (u8 *)"通话宽频下行EQ", 0x3001, CALL_SECTION, 1, {EQ_ONLINE_CMD_CALL_EQ_V1_SEG, 0}},
    {call_narrow_eq_mode,	(u8 *)"通话窄频下行EQ", 0x3002, CALL_SECTION, 1, {EQ_ONLINE_CMD_CALL_EQ_V1_SEG, 0}},
    {aec_eq_mode, (u8 *)"通话宽频上行EQ", 0x3003, UL_SECTION,   1, {EQ_ONLINE_CMD_AEC_EQ_V1_SEG,  0}},
    {aec_narrow_eq_mode,	(u8 *)"通话窄频上行EQ", 0x3004, UL_SECTION,   1, {EQ_ONLINE_CMD_AEC_EQ_V1_SEG,  0}},
#if defined(TCFG_AUDIO_MDRC_ENABLE) && TCFG_AUDIO_MDRC_ENABLE
    {song_eq_mode, (u8 *)"普通音频EQ", 	0x3000, SONG_SECTION, 3, {EQ_ONLINE_CMD_SONG_EQ_V1_SEG, EQ_ONLINE_CMD_SONG_MULTI_WDRC, EQ_ONLINE_CMD_VIRTUAL_BASS}},//多带wdrc
    {hearing_aid_mode, (u8 *)"辅听EQ", 	0x3005, SONG_SECTION, 2, {EQ_ONLINE_CMD_HEARING_AID_EQ_SEG, EQ_ONLINE_CMD_SONG_WDRC}},
    {hearing_aid_mode2, (u8 *)"辅听EQ2", 	0x3006, SONG_SECTION, 1, {EQ_ONLINE_CMD_HEARING_AID_EQ_SEG2}},
#else
    {song_eq_mode, (u8 *)"普通音频EQ", 	0x3000, SONG_SECTION, 3, { EQ_ONLINE_CMD_SONG_EQ_V1_SEG, EQ_ONLINE_CMD_SONG_DRC, EQ_ONLINE_CMD_VIRTUAL_BASS}},
#if defined(TCFG_AUDIO_HEARING_AID_ENABLE) && TCFG_AUDIO_HEARING_AID_ENABLE
    {hearing_aid_mode, (u8 *)"辅听EQ",       0x3005, SONG_SECTION, 2, {EQ_ONLINE_CMD_HEARING_AID_EQ_SEG, EQ_ONLINE_CMD_SONG_WDRC}},
    {hearing_aid_mode2, (u8 *)"辅听EQ2", 	0x3006, SONG_SECTION, 1, {EQ_ONLINE_CMD_HEARING_AID_EQ_SEG2}},
#endif /*TCFG_AUDIO_HEARING_AID_ENABLE*/
#endif
};

/*----------------------------------------------------------------------------*/
/**@brief    eq 段数更新,需要在eq_init前就准备好
   @param    mode:call_eq_mode\call_narrow_eq_section等模式
   @param    section:段数最大为EQ_SECTION_MAX
   @return
   @note     下行的宽频和窄频段数需一致，上行的宽频和窄频段数需要一致
*/
/*----------------------------------------------------------------------------*/
void set_eq_tool_tab_section(u8 mode, u8 section)
{
    for (int i = 0; i < ARRAY_SIZE(eq_tool_tab); i++) {
        if (mode == eq_tool_tab[i].mode_index) {
            eq_tool_tab[i].section = section;
        }
    }
}

void eq_section_num_set(u8 song, u8 call_16k_8k, u8 aec_16k_8k)
{
    set_eq_tool_tab_section(song_eq_mode, song);
    set_eq_tool_tab_section(call_eq_mode, call_16k_8k);
    set_eq_tool_tab_section(call_narrow_eq_mode, call_16k_8k);
    set_eq_tool_tab_section(aec_eq_mode, aec_16k_8k);
    set_eq_tool_tab_section(aec_narrow_eq_mode, aec_16k_8k);
}
void drc_default_init(EQ_CFG *eq_cfg, u8 mode)
{
#if TCFG_DRC_ENABLE
    int i = mode;
    if (eq_cfg && eq_cfg->drc) {
        //限幅器的初始值
        int th = 0;//db -60db~0db
        int threshold = roundf(powf(10.0f, th / 20.0f) * 32768); // 0db:32768, -60db:33
        eq_cfg->cfg_parm[i].drc_parm.parm.drc.nband = 1;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc.type = 1;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc._p.limiter[0].attacktime = 5;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc._p.limiter[0].releasetime = 500;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc._p.limiter[0].threshold[0] = threshold;
        eq_cfg->cfg_parm[i].drc_parm.parm.drc._p.limiter[0].threshold[1] = 32768;
    }
#endif

}
#if EQ_FILE_CP_TO_CUSTOM
//eq_cfg_hw.bin中播歌eq曲线，当作用户自定义模式，参与效果切换.
//通话宽频上下行eq曲线也对应放到phone_eq_tab_normal、ul_eq_tab_normal
//EQ_FILE_CP_TO_CUSTOM 1使能时，同时板极文件中 TCFG_USE_EQ_FILE 配 0
static void eq_file_cp_to_custon_mode_fun(EQ_CFG *eq_cfg)
{
    if (eq_cfg->eq_type == EQ_TYPE_FILE) {
        eq_cfg->eq_type = EQ_TYPE_MODE_TAB;
        for (int i = 0; i < eq_cfg->mode_num; i++) {
            if ((i == song_eq_mode) || (i == call_eq_mode) || (i == aec_eq_mode)) { //播歌eq  通话宽频上下行eq
                u32 seg_num    = eq_cfg->cfg_parm[i].song_eq_parm.parm.par.seg_num;
                u8 index = 0;
                for (int j = 0; j < ARRAY_SIZE(eq_tool_tab); j++) {
                    if (i == eq_tool_tab[j].mode_index) {
                        index = j;
                    }
                }
                if (seg_num > eq_tool_tab[index].section) {
                    seg_num = eq_tool_tab[index].section;
                }
                void *tar = NULL;
                if (i == call_eq_mode) {
#if TCFG_PHONE_EQ_ENABLE
                    tar = eq_cfg->phone_eq_tab;
#endif
                } else if (i == aec_eq_mode) {
                    tar = eq_cfg->ul_eq_tab;
                } else if (i == song_eq_mode) {
                    tar = eq_tab_custom;
                    eq_cfg->type_gain_tab[EQ_MODE_MAX - 1] = eq_cfg->cfg_parm[i].song_eq_parm.parm.par.global_gain;
                }
                if (tar) {
                    memcpy(tar, eq_cfg->cfg_parm[i].song_eq_parm.parm.seg, seg_num * sizeof(EQ_CFG_SEG));
                    eq_cfg->seg_num[i] = seg_num;
                }
            }
        }
    }
}
#endif

__attribute__((weak))
s16 get_ci_tx_size()
{
    return 0x30;
}
/*----------------------------------------------------------------------------*/
static u8 reply_to_tool = 0;
static u8 reply_sq = 0;
/*
 *新调音回调
 * */
static void eq_online_callback_new_protocol(uint8_t *packet, uint16_t size)
{
    u8 *ptr = packet;
    reply_sq = ptr[1];
    reply_to_tool =  0;
    u8 *new_packet = (void *)&packet[2];
    eq_online_callback(new_packet, size);
}
//新调音注册
REGISTER_DETECT_TARGET(eq_adj_target) = {
    .id         = EQ_CONFIG_ID,
    .tool_message_deal   = eq_online_callback_new_protocol,
};


/*
/*----------------------------------------------------------------------------*/
int ci_send_cmd(void *priv, u32 id, u8 *packet, int size)
{
    EQ_CFG *eq_cfg = (EQ_CFG *)priv;
    if (!eq_cfg) {
        return 0;
    }
#if TCFG_ONLINE_ENABLE
#if defined(TCFG_EFFECT_TOOL_ENABLE) && TCFG_EFFECT_TOOL_ENABLE
    all_assemble_package_send_to_pc(reply_to_tool, reply_sq, packet, size);
#else
    ci_send_packet(id, packet, size);
#endif
#endif
    return 0;
}
struct virtual_bass_tool_set vbass_default_param = {
    .prev_gain = -3,     //dB预处理，降低数据增益，防止虚拟低音把数据调爆
    .parm = {
        .ratio = 10,     //比率(0~100),越大，低音强度越大
        .boost = 1,      //(低音自动增强0, 1)
        .fc = 100,       //截止频率(30~300Hz)
    },
};

struct virtual_bass_tool_set *get_vbass_parm()
{
    EQ_CFG *eq_cfg = get_eq_cfg_hdl();
    if (!eq_cfg) {
        return NULL;
    }
#if defined(TCFG_EFFECT_TOOL_ENABLE) && TCFG_EFFECT_TOOL_ENABLE
    return &eq_cfg->cfg_parm[song_eq_mode].vbass_parm.parm;
#else
    if (eq_cfg->vbass_file) {
        return &eq_cfg->cfg_parm[song_eq_mode].vbass_parm.parm;
    } else {
        return &vbass_default_param;
    }
#endif
}


int eq_init(void)
{
    eq_adjust_parm parm = {0};
#if TCFG_EQ_ONLINE_ENABLE
    parm.online_en = 1;
#endif
    if (config_filter_coeff_fade_en) {
        parm.fade_en = 1;
    }

#if TCFG_USE_EQ_FILE
    parm.file_en = 1;
#endif

#if EQ_FILE_CP_TO_CUSTOM
    parm.file_en = 1;
    parm.type_gain_tab = type_gain_tab;
#endif

#if TCFG_DRC_ENABLE
    parm.drc = 1;
#endif

#if TCFG_USER_TWS_ENABLE
    parm.tws = 1;
#endif


#if (RCSP_ADV_EN)&&(JL_EARPHONE_APP_EN)&&(TCFG_DRC_ENABLE == 0)
    /* parm.limit_zero = 1; */
#endif

    /* #if TCFG_EQ_DIVIDE_ENABLE */
    /* parm.stero = 1; */
    /* parm.mode_num = 8; */
    /* #endif */

    if (!parm.stero) {
#if defined(TCFG_AUDIO_MDRC_ENABLE) && TCFG_AUDIO_MDRC_ENABLE
        parm.mode_num = 7;//一共有多少个模式
#else
#if defined(TCFG_AUDIO_HEARING_AID_ENABLE) && TCFG_AUDIO_HEARING_AID_ENABLE
        parm.mode_num = 7;// 一共有多少个模式
#else
        parm.mode_num = 5;// 一共有多少个模式
#endif
#endif /*TCFG_AUDIO_HEARING_AID_ENABLE*/
        /* #ifdef DAC_OUTPUT_FRONT_LR_REAR_LR */
        /* #if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR) */
        /* parm.mode_num = 6; */
        /* #endif */
        /* #endif */
    }

#if TCFG_PHONE_EQ_ENABLE
    parm.phone_eq_tab = (void *)phone_eq_tab_normal;
    parm.phone_eq_tab_size = TCFG_CALL_DL_EQ_SECTION;//ARRAY_SIZE(phone_eq_tab_normal);
#endif

#if TCFG_AEC_UL_EQ_ENABLE
    parm.ul_eq_tab = (void *)ul_eq_tab_normal;
    parm.ul_eq_tab_size = TCFG_CALL_UL_EQ_SECTION;//ARRAY_SIZE(ul_eq_tab_normal);
#endif

    parm.eq_tool_tab = eq_tool_tab;
    parm.eq_mode_use_idx = (u8 *)eq_mode_use_idx;

    parm.eq_type_tab = (void *)get_eq_mode_tab();
#if TCFG_USER_EQ_MODE_NUM
    parm.type_num = TCFG_USER_EQ_MODE_NUM;//EQ_MODE_MAX;
#endif

    parm.section_max = EQ_SECTION_MAX;

    EQ_CFG *eq_cfg = eq_cfg_open(&parm);
    if (eq_cfg) {
        eq_cfg->priv = eq_cfg;
        eq_cfg->send_cmd = ci_send_cmd;
        eq_cfg->custom_mode_id = EQ_MODE_CUSTOM;
#if defined(AUDIO_VBASS_CONFIG)&&AUDIO_VBASS_CONFIG
        eq_cfg->vbass = 1;
#endif
#if EQ_FILE_CP_TO_CUSTOM
        eq_file_cp_to_custon_mode_fun(eq_cfg);
#else
        for (int i = 0; i < eq_cfg->mode_num; i++) {
            if (eq_cfg->eq_type == EQ_TYPE_MODE_TAB) {
                if (!eq_cfg->type_gain_tab) {//没指定总增益时，才配置默认总增益0
                    set_global_gain(eq_cfg, i, 0);
                }
                drc_default_init(eq_cfg, i);
            }
        }
#endif
    }

    int cpu_section = 0;
    //单声道或者立体声申请的cpu eq mem
    if (EQ_SECTION_MAX > 10) {
        u8 add_num = 0;
        if (hw_crossover_type0) {
            add_num = 4;//drc分频器使用eq硬件加速时、3段4阶使用最大eq段数24段(每段4个eq, 6声道)
        }
        cpu_section = EQ_SECTION_MAX - (int)&EQ_PRIV_SECTION_NUM + add_num;
    }
    audio_eq_init(cpu_section);
    return 0;
}
__initcall(eq_init);


#if TCFG_EQ_FILE_SWITCH_EN
//根据eq_file_switch_list切换到指定的eq文件
void eq_file_set_by_index(u8 index)
{
    if (index >= ARRAY_SIZE(eq_file_switch_list)) {
        printf("err, max index %d\n", ARRAY_SIZE(eq_file_switch_list));
        return;
    }
    EQ_CFG *eq_cfg = get_eq_cfg_hdl();
    if (!eq_cfg) {
        return;
    }
    int ret = eq_file_get_cfg(eq_cfg, eq_file_switch_list[index]);
    printf("eq_file_switch : %d, ret : %d", index, ret);
}

//根据eq_file_switch_list成员个数顺序切换eq文件
void eq_file_switch()
{
    static u8 index = 0;
    index++;
    if (index >= ARRAY_SIZE(eq_file_switch_list)) {
        index = 0;
    }
    eq_file_set_by_index(index);
}

#endif
#endif
