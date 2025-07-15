/*********************************************************************************************
    *   Filename        : log_config.c

    *   Description     : Optimized Code & RAM (编译优化Log配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 14:45

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "system/includes.h"

/**
 * @brief Bluetooth Controller Log
 */
/*-----------------------------------------------------------*/
const char log_tag_const_v_SETUP AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_SETUP AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_SETUP AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_d_SETUP AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_SETUP AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_BOARD AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_BOARD AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_BOARD AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_w_BOARD AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_BOARD AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_EARPHONE AT(.LOG_TAG_CONST)  = FALSE;
const char log_tag_const_i_EARPHONE AT(.LOG_TAG_CONST)  = TRUE;
const char log_tag_const_d_EARPHONE AT(.LOG_TAG_CONST)  = FALSE;
const char log_tag_const_w_EARPHONE AT(.LOG_TAG_CONST)  = TRUE;
const char log_tag_const_e_EARPHONE AT(.LOG_TAG_CONST)  = TRUE;

const char log_tag_const_v_UI AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_UI AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_UI AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_UI AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_UI AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_CHARGE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_CHARGE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_CHARGE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_CHARGE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_CHARGE AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_ANCBOX AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_ANCBOX AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_ANCBOX AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_ANCBOX AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_ANCBOX AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_KEY_EVENT_DEAL AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_CHARGESTORE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_CHARGESTORE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_CHARGESTORE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_CHARGESTORE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_CHARGESTORE AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_UMIDIGI_CHARGESTORE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_UMIDIGI_CHARGESTORE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_UMIDIGI_CHARGESTORE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_UMIDIGI_CHARGESTORE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_UMIDIGI_CHARGESTORE AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_CFG_TOOL AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_CFG_TOOL AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_CFG_TOOL AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_CFG_TOOL AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_CFG_TOOL AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_TESTBOX AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_TESTBOX AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_TESTBOX AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_TESTBOX AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_TESTBOX AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_IDLE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_IDLE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_IDLE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_IDLE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_IDLE AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_POWER AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_POWER AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_POWER AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_POWER AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_POWER AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_USER_CFG AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_USER_CFG AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_USER_CFG AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_USER_CFG AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_USER_CFG AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_TONE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_TONE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_TONE AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_TONE AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_TONE AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_BT_TWS AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_BT_TWS AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_BT_TWS AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_BT_TWS AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_BT_TWS AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_AEC_USER AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_AEC_USER AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_AEC_USER AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_AEC_USER AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_AEC_USER AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_BT_BLE AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_BT_BLE AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_d_BT_BLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_w_BT_BLE AT(.LOG_TAG_CONST) = 1;
const char log_tag_const_e_BT_BLE AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_EARTCH_EVENT_DEAL AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_EARTCH_EVENT_DEAL AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_EARTCH_EVENT_DEAL AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_EARTCH_EVENT_DEAL AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_EARTCH_EVENT_DEAL AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_ONLINE_DB AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_i_ONLINE_DB AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_d_ONLINE_DB AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_w_ONLINE_DB AT(.LOG_TAG_CONST) = 0;
const char log_tag_const_e_ONLINE_DB AT(.LOG_TAG_CONST) = 1;

const char log_tag_const_v_MUSIC AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_MUSIC AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_w_MUSIC AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_d_MUSIC AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_MUSIC AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_APP_MUSIC AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_APP_MUSIC AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_APP_MUSIC AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_w_APP_MUSIC AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_APP_MUSIC AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_PC AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_PC AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_w_PC AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_d_PC AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_PC AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_AUX AT(.LOG_TAG_CONST)  = FALSE;
const char log_tag_const_i_AUX AT(.LOG_TAG_CONST)  = TRUE;
const char log_tag_const_d_AUX AT(.LOG_TAG_CONST)  = FALSE;
const char log_tag_const_w_AUX AT(.LOG_TAG_CONST)  = TRUE;
const char log_tag_const_e_AUX AT(.LOG_TAG_CONST)  = TRUE;

const char log_tag_const_v_WIRELESSMIC AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_WIRELESSMIC AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_w_WIRELESSMIC AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_d_WIRELESSMIC AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_WIRELESSMIC AT(.LOG_TAG_CONST) = TRUE;

const char log_tag_const_v_KWS_VOICE_EVENT AT(.LOG_TAG_CONST) = FALSE;
const char log_tag_const_i_KWS_VOICE_EVENT AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_d_KWS_VOICE_EVENT AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_w_KWS_VOICE_EVENT AT(.LOG_TAG_CONST) = TRUE;
const char log_tag_const_e_KWS_VOICE_EVENT AT(.LOG_TAG_CONST) = TRUE;
