#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H


/*
 *  板级配置选择
 */

// #define CONFIG_BOARD_JL701N_DEMO
// #define CONFIG_BOARD_JL701N_BTEMITTER
// #define CONFIG_BOARD_JL701N_ANC
#define CONFIG_BOARD_JL7016G_HYBRID
// #define CONFIG_BOARD_JL7018F_DEMO

#include "media/audio_def.h"
#include "board_jl701n_demo_cfg.h"
#include "board_jl701n_btemitter_cfg.h"
#include "board_jl701n_anc_cfg.h"
#include "board_jl7016g_hybrid_cfg.h"
#include "board_jl7018f_demo_cfg.h"

#define  DUT_AUDIO_DAC_LDO_VOLT   				DACVDD_LDO_1_25V

#ifdef CONFIG_NEW_CFG_TOOL_ENABLE
#define CONFIG_ENTRY_ADDRESS                    0x6000100
#endif

#endif
