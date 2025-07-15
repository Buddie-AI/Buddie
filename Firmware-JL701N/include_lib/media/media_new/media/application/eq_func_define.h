#ifndef _EQ_FUNC_DEFINE_H
#define _EQ_FUNC_DEFINE_H

#include "generic/typedef.h"

extern u32 EQ_PRIV_SECTION_NUM;
extern u32 EQ_SECTION_NUM;
extern const int const_eq_debug;
extern const int config_audio_drc_en;
extern const int config_audio_eq_en;
enum {
    EQ_EN = BIT(0),//eq模式使能
    EQ_SUPPORT_OLD_VER_EN = BIT(1),//ac700N,使能该bit可版本0.7.1.0， 0.7.1.1
    EQ_LITE_VER_EN = BIT(2),//精简版版本eq驱动，不支持异步，不支持默认效果切换接口，仅支持文件解析
    EQ_ONLINE_EN = BIT(3),  //在线调试模块使能
    EQ_FILE_EN = BIT(4),//打开支持文件解析,关闭则只能使用默认效果表做eq
    EQ_FILE_SWITCH_EN = BIT(5),//打开支持eq_cfg_hw.bin文件切换更新效果
    EQ_HIGH_BASS_EN  = BIT(6),//eq内部集成的高低音接口使能
    EQ_HIGH_BASS_FADE_EN = BIT(7),//eq内部集成高低音淡入淡出效果方式使能,配合config_audio_eq_fade_step使用
    EQ_FILTER_COEFF_FADE_EN = BIT(8),//eq默认系数表效果切换，或者在线调试效果更新，使用淡入淡出方式，避免增益跳跃引起的杂音问题
    EQ_FILTER_COEFF_LIMITER_ZERO_EN = BIT(9),//
    EQ_HW_UPDATE_COEFF_ONLY_EN = BIT(10),// 有空闲的段可以使用，就不需要切换系数 */
    EQ_HW_LR_ALONE = BIT(11),//// 左右声道分开处理与bit(10),同时使能或关闭
    EQ_SUPPORT_32BIT_SYNC_EN = BIT(12),//// 支持同步方式32biteq
    EQ_SUPPORT_MULIT_CHANNEL_EN = BIT(13),//AC699N、AC700N eq是否支持多声道（3~8） 打开:支持  关闭：仅支持1~2声道
    EQ_HW_CROSSOVER_TYPE0_EN  = BIT(14),//支持硬件分频器,且分频器使用序列进序列出，需使能BIT(13)
    EQ_HW_CROSSOVER_TYPE1_EN  = BIT(15),//硬件分频器使用使用块出方式，会增加mem(该方式仅支持单声道处理)
    EQ_LR_DIVIDE_EN = BIT(16),//eq左右声道效果拆分,四声道时可能会使用
    EQ_ONLINE_FILE_SAVE = BIT(17),//eq在线调试保存到vm使能
};

#define config_audio_eq_online_en      (config_audio_eq_en & EQ_ONLINE_EN)
#define config_audio_eq_file_en        (config_audio_eq_en & EQ_FILE_EN)
#define config_audio_eq_file_sw_en     (config_audio_eq_en & EQ_FILE_SWITCH_EN)
#define config_filter_coeff_fade_en    (config_audio_eq_en & EQ_FILTER_COEFF_FADE_EN)
#define config_high_bass_en            (config_audio_eq_en & EQ_HIGH_BASS_EN)
#define config_audio_eq_fade_en        (config_audio_eq_en & EQ_HIGH_BASS_FADE_EN)
#define config_filter_coeff_limit_zero (config_audio_eq_en & EQ_FILTER_COEFF_LIMITER_ZERO_EN)
#define HW_EQ_UPDATE_COEFF_ONLY_EN     (config_audio_eq_en & EQ_HW_UPDATE_COEFF_ONLY_EN)
#define HW_EQ_LR_ALONE                 (config_audio_eq_en & EQ_HW_LR_ALONE)
#define SUPPORT_32BIT_SYNC_EQ          (config_audio_eq_en & EQ_SUPPORT_32BIT_SYNC_EN)
#define hw_eq_support_multi_channels   (config_audio_eq_en & EQ_SUPPORT_MULIT_CHANNEL_EN)
#define hw_crossover_type0             (config_audio_eq_en & EQ_HW_CROSSOVER_TYPE0_EN)
#define hw_crossover_type1             (config_audio_eq_en & EQ_HW_CROSSOVER_TYPE1_EN)
#define config_eq_lite_en              (config_audio_eq_en & EQ_LITE_VER_EN)
#define config_eq_support_old_ver_en   (config_audio_eq_en & EQ_SUPPORT_OLD_VER_EN)
#define config_divide_en               (config_audio_eq_en & EQ_LR_DIVIDE_EN)
#define config_eq_online_file_save     (config_audio_eq_en & EQ_ONLINE_FILE_SAVE)

extern const int config_audio_drc_en;
enum {
    DRC_EN = BIT(0),   //drc 使能
    DRC_NBAND_MERGING_ASM_EN = BIT(1),//drc 多带处理完后，多带合并使用汇编加速
    DRC_NBAND_DIS = BIT(2),//关闭drc多带
    DRC_LIMITER_DIS = BIT(3),//关闭drc限幅器
    DRC_COMPRESSOF_DIS = BIT(4),//关闭drc压缩器
    WDRC_TYPE_EN = BIT(5),//wdrc使能
};

#define config_drc_limiter_en         (!(config_audio_drc_en & DRC_LIMITER_DIS))
#define config_drc_compressor_en      (!(config_audio_drc_en & DRC_COMPRESSOF_DIS))
#define config_drc_nband_en           (!(config_audio_drc_en & DRC_NBAND_DIS))
#define config_drc_nband_merg_asm_en  (!(config_audio_drc_en & DRC_NBAND_MERGING_ASM_EN))
#define config_wdrc_en                (config_audio_drc_en & WDRC_TYPE_EN)


#endif
