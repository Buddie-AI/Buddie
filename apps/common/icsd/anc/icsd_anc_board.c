#include "board_config.h"
#include "icsd_anc_app.h"
#include "tone_player.h"
#include "audio_anc.h"
#include "icsd_anc_user.h"

#if 1
#define icsd_board_log printf
#else
#define icsd_board_log(...)
#endif/*log_en*/

#if TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN

u8 anc_train_result = 0;	//ANC自适应训练结果返回值
u8 IIR_NUM;// 4     3
u8 IIR_NUM_FIX;// 4   8-IIR_NUM
u8 IIR_COEF; //(IIR_NUM * 3+1)

/*
typedef enum {
    ANC_IIR_HIGH_PASS = 0,
    ANC_IIR_LOW_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
    ANC_IIR_LOW_SHELF,
} ANC_iir_type;
*/

#ifndef ADAPTIVE_CLIENT_BOARD
const u8 cmp_iir_type[] = {
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 ff_iir_type[] = {
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 fb_iir_type[] = {
    ANC_IIR_HIGH_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 FF_VERSION  =  1;
const u8 NEW_PRE_TREAT = 0;
const u8 ICSD_EP_TYPE = ICSD_FULL_INEAR;
const u8 icsd_dcc[4] = {1, 4, 1, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;

void icsd_xtmp_handler(float *xtmp, float gain_min, float gain_max)
{
    if (xtmp[1] < gain_min) {
        xtmp[1] 		= (float)gain_min;
    }
    if (xtmp[1] > gain_max) {
        xtmp[1] 		= (float)gain_max;
    }
}

void icsd_anc_board_config()
{
    icsd_anc_board_param_init();
    ANC_BOARD_PARAM->ff_yorder  = ANC_ADAPTIVE_FF_ORDER;
    ANC_BOARD_PARAM->fb_yorder  = ANC_ADAPTIVE_FB_ORDER;
    ANC_BOARD_PARAM->cmp_yorder = ANC_ADAPTIVE_CMP_ORDER;

#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				3
    extern const float E108D_DATA[];
    extern const s8 E108D_s8_DATA[];
    extern const float E108D_DOUBLE_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)E108D_DATA;
    ANC_BOARD_PARAM->anc_data_r = (void *)E108D_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)E108D_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)E108D_DOUBLE_DATA;
    ANC_BOARD_PARAM->anc_double_data_r = (void *)E108D_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l = 120.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l = 0.4;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->m_value_r = 120.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_r = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_r = 0.4;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 11.0 - 18.0;
    ANC_BOARD_PARAM->sen_offset_r = 11.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 18;
    ANC_BOARD_PARAM->ff_target_fix_num_r = 18;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 0;
    ANC_BOARD_PARAM->idx_begin = 18;
    ANC_BOARD_PARAM->idx_end = 60;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 3;
    ANC_BOARD_PARAM->gain_min_offset = 11;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 2;
    ANC_BOARD_PARAM->tff_sign = 1;
    ANC_BOARD_PARAM->tfb_sign = -1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 0.6; //0.6

    ANC_BOARD_PARAM->tool_target_sign = -1;
    ANC_BOARD_PARAM->cmp_type[0] = 1;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 0;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 1;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 1;
    ANC_BOARD_PARAM->cmp_thd_low = -40.0;
    ANC_BOARD_PARAM->cmp_thd_high = 40.0;

    ANC_BOARD_PARAM->IIR_NUM = 3;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 60;
    ANC_BOARD_PARAM->JT_MODE = 0;

    if (ANC_BOARD_PARAM->pz_max_times < 3) {
        ANC_BOARD_PARAM->pz_max_times = 3;
    }

    if (ANC_BOARD_PARAM->bypass_max_times < 2) {
        ANC_BOARD_PARAM->pz_max_times = 2;
    }

    ANC_BOARD_PARAM->mode = ICSD_ANC_MODE;
    if (ANC_DFF_AFB_EN) {
        icsd_board_log("ANC_DFF_AFB_EN ON\n");
        ANC_BOARD_PARAM->FF_FB_EN |= ICSD_DFF_AFB_EN;//BIT(7);
    }
    if (ANC_AFF_DFB_EN) {
        icsd_board_log("ANC_AFF_DFB_EN ON\n");
        ANC_BOARD_PARAM->FF_FB_EN |= ICSD_AFF_DFB_EN;//BIT(6);
    }
    if (ANC_FB_TRAIN_OFF) {
        ANC_BOARD_PARAM->FF_FB_EN |= ICSD_FB_TRAIN_OFF;//BIT(5);
    }
    if (ANC_FB_TRAIN_NEW) {
        ANC_BOARD_PARAM->FF_FB_EN |= ICSD_FB_TRAIN_NEW;//BIT(4);
    }
    ANC_BOARD_PARAM->default_ff_gain = DEFAULT_FF_GAIN;
    ANC_BOARD_PARAM->default_fb_gain = DEFAULT_FB_GAIN;

    if (TCFG_AUDIO_DAC_MODE == DAC_MODE_H1_DIFF) {
        user_train_state |= ANC_DAC_MODE_H1_DIFF;
    } else {
        user_train_state &= ~ANC_DAC_MODE_H1_DIFF;
    }
}
#else

#include "icsd_anc_client_board.c"

#endif/*ADAPTIVE_CLIENT_BOARD*/

void icsd_anc_config_inf()
{
    icsd_board_log("\n=============ANC COMFIG INF================\n");

    icsd_anc_version();
    switch (ICSD_ANC_MODE) {
    case TWS_TONE_BYPASS_MODE:
        icsd_board_log("TWS_TONE_BYPASS_MODE\n");
        break;
    case TWS_TONE_MODE:
        icsd_board_log("TWS_TONE_MODE\n");
        break;
    case TWS_BYPASS_MODE:
        icsd_board_log("TWS_BYPASS_MODE\n");
        break;
    case HEADSET_TONE_BYPASS_MODE:
        icsd_board_log("HEADSET_TONE_BYPASS_MODE\n");
        break;
    case HEADSET_TONES_MODE:
        icsd_board_log("HEADSET_TONES_MODE\n");
        break;
    case HEADSET_BYPASS_MODE:
        icsd_board_log("HEADSET_BYPASS_MODE\n");
        break;
    default:
        icsd_board_log("NEW MODE\n");
        break;
    }
    icsd_board_log("TEST_ANC_COMBINATION:%d\n", TEST_ANC_COMBINATION);
    icsd_board_log("ANC_ADAPTIVE_CMP_EN:%d\n", ANC_ADAPTIVE_CMP_EN);
    icsd_board_log("ANC_EAR_RECORD_EN:%d\n", ANC_EAR_RECORD_EN);
    icsd_board_log("TEST_EAR_RECORD_EN:%d\n", TEST_EAR_RECORD_EN);
    icsd_board_log("BYPASS_TIMES MIN:2 MAX:%d\n", BYPASS_MAXTIME);
    icsd_board_log("PZ_TIMES MIN:3 MAX:%d\n", PZ_MAXTIME);
    icsd_board_log("\n");

#if	TEST_EAR_RECORD_EN
    icsd_board_log("EAR RECORD CURRENT RECORD\n");
    //icsd_anc_ear_record_icsd_board_log();
#endif
}

//训练结果获取API，用于通知应用层当前使用自适应参数还是默认参数
u8 icsd_anc_train_result_get(struct icsd_anc_tool_data *TOOL_DATA)
{
    u8 result = 0;
    if (TOOL_DATA) {
        switch (TOOL_DATA->anc_combination) {
        case TFF_TFB://使用自适应FF，自适应FB
            result = ANC_ADAPTIVE_RESULT_LFF | ANC_ADAPTIVE_RESULT_LFB | ANC_ADAPTIVE_RESULT_RFF | ANC_ADAPTIVE_RESULT_RFB;
            break;
        case TFF_DFB://使用自适应FF，默认FB
            result = ANC_ADAPTIVE_RESULT_LFF | ANC_ADAPTIVE_RESULT_RFF;
            break;
        case DFF_TFB://result:1 使用记忆FF，自适应FB   :3 使用默认FF，自适应FB
            result = ANC_ADAPTIVE_RESULT_LFB | ANC_ADAPTIVE_RESULT_RFB;
            break;
        case DFF_DFB://result:1 使用记忆FF，默认FB  :3 使用默认FF，默认FB
        default:
            break;
        }
        //使用耳道记忆的FF参数
        if (TOOL_DATA->result == ANC_USE_RECORD) {
            result |= (ANC_ADAPTIVE_RESULT_LFF | ANC_ADAPTIVE_RESULT_RFF);
        }
        //CMP成功，使用自适应CMP参数
        if (TOOL_DATA->cmp_result) {
            result |= (ANC_ADAPTIVE_RESULT_LCMP | ANC_ADAPTIVE_RESULT_RCMP);
        }
    }
    return result;
}

extern int tone_play_index(u8 index, u8 preemption);
void icsd_anc_htarget_data_end(u8 result)
{
    user_train_state &= ~ANC_USER_TRAIN_TOOL_DATA;
    ICSD_ANC.adaptive_run_busy = 0;
    /*
       训练结果播放提示音
       result = 1 训练成功
       result = 0 训练失败
    */
    anc_train_result = result;
    printf("ICSD ANC RESULT:%d================\n", result);

    switch (result) {
    case 1:
        anc_printf("FF USE RECORD\n");
        break;
    case 2:
        anc_printf("FF USE TRAIN\n");
        break;
    case 3:
        anc_printf("FF USE DEFAULT\n");
        break;
    default:
        break;
    }
    switch (TOOL_DATA->anc_combination) {
    case TFF_TFB://使用自适应FF，自适应FB
        anc_printf("TFF_TFB\n");
        break;
    case TFF_DFB://使用自适应FF，默认FB
        anc_printf("TFF_DFB\n");
#if ANC_DEVELOPER_MODE_EN
        icsd_adt_tone_play(ICSD_ADT_TONE_NUM2);
#endif/*ANC_DEVELOPER_MODE_EN*/
        break;
    case DFF_TFB://result:1 使用记忆FF，自适应FB   :3 使用默认FF，自适应FB
        anc_printf("DFF_TFB\n");
#if ANC_DEVELOPER_MODE_EN
        icsd_adt_tone_play(ICSD_ADT_TONE_NUM1);
#endif/*ANC_DEVELOPER_MODE_EN*/
        break;
    case DFF_DFB://result:1 使用记忆FF，默认FB  :3 使用默认FF，默认FB
        anc_printf("DFF_DFB\n");
#if ANC_DEVELOPER_MODE_EN
        icsd_adt_tone_play(ICSD_ADT_TONE_NUM0);
#endif/*ANC_DEVELOPER_MODE_EN*/
        break;
    default:
        break;
    }
    audio_anc_adaptive_fail_callback(TOOL_DATA->anc_err);
    icsd_board_log("train time tool data end:%d\n", (int)((jiffies - train_time) * 10));
    icsd_anc_htarget_data_send_end();
}

extern void audio_anc_adaptive_data_packet(struct icsd_anc_tool_data *TOOL_DATA);
void icsd_anc_htarget_data_send()
{

#if ANC_SZ_OUT_EN
    extern void icsd_anc_sz_out(float * hz_in);
    extern void icsd_anc_pz_out(float * hz_in);
    if (HEADSET_MODE) {
        if ((TOOL_DATA->result == 2) || (TOOL_DATA->anc_combination == TFF_DFB)) {
            icsd_anc_sz_out(TOOL_DATA->data_out1);
            icsd_anc_pz_out(TOOL_DATA->data_out2);
        }
    } else {
        if (TOOL_DATA->result == 2) {
            icsd_anc_sz_out(TOOL_DATA->data_out1);
            icsd_anc_pz_out(TOOL_DATA->data_out2);
        }
    }
#endif

    if (TOOL_DATA->cmp_result) {
        anc_printf("CMP RESULT:success\n");
    } else {
        anc_printf("CMP RESULT:fail\n");
    }

    icsd_board_log("train time tool data start:%d\n", (int)((jiffies - train_time) * 10));
    //TOOL_DATA->result  0:失败使用默认参数   1:耳道记忆参数  2:成功
    printf("save check:%d %d----------------\n", TOOL_DATA->result, HEADSET_MODE);
    if (HEADSET_MODE) {
        if ((TOOL_DATA->result == 2) || (TOOL_DATA->anc_combination == TFF_DFB)) {
            //成功时进行耳道记忆
            icsd_anc_save_with_idx(TOOL_DATA->save_idx);
        }
    } else {
        if (TOOL_DATA->result == 2) {
            //成功时进行耳道记忆
            icsd_anc_save_with_idx(TOOL_DATA->save_idx);
        }
    }
    audio_anc_adaptive_data_packet((struct icsd_anc_tool_data *)TOOL_DATA);
    icsd_anc_htarget_data_end(TOOL_DATA->result);
}

void icsd_anc_end(audio_anc_t *param)
{
    icsd_board_log("train time finish:%dms\n", (int)((jiffies - train_time) * 10));
    user_train_state &= ~ANC_USER_TRAIN_DMA_EN;
    if (icsd_time_out_hdl) {
        sys_timeout_del(icsd_time_out_hdl);
        icsd_time_out_hdl = 0;
    }
    icsd_anc_contral = 0;
    anc_user_train_cb(ANC_ON, anc_train_result, 0);
}

void icsd_anc_forced_exit_end()
{
    if (icsd_anc_contral & ICSD_ANC_TONE_END) {
        printf("icsd_anc_forced_exit_end cb-----------------------------\n");
        icsd_anc_contral &= ~ICSD_ANC_FORCED_EXIT;
        icsd_anc_contral &= ~ICSD_ANC_SUSPENDED;
        icsd_anc_contral &= ~ICSD_ANC_TONE_END;
        icsd_anc_contral &= ~ICSD_ANC_INITED;
        icsd_anc_contral &= ~ICSD_ANC_FORCED_BEFORE_INIT;
        if (icsd_time_out_hdl) {
            sys_timeout_del(icsd_time_out_hdl);
            icsd_time_out_hdl = 0;
        }
        anc_user_train_cb(ANC_ON, 0, 1);
    }
}

void anc_user_train_tone_play_cb()
{
    icsd_board_log("anc_user_train_tone_play_cb:%d========================================\n", (int)jiffies);
    train_time = jiffies;
#if ANC_USER_TRAIN_EN
    icsd_anc_contral |= ICSD_ANC_TONE_END;
    if (icsd_anc_contral & ICSD_ANC_SUSPENDED) {
        printf("FORCED EXIT END================================\n");
        user_train_state |= ANC_TRAIN_TONE_END;
        icsd_anc_forced_exit_end();
        return;
    }
#if ANC_USER_TRAIN_TONE_MODE
    icsd_board_log("TONE MODE tone play cb~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    if (user_train_state & ANC_TRAIN_TONE_FIRST) {
        user_train_state |= ANC_TRAIN_TONE_END;
    } else {
        icsd_board_log("second train at cb~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        user_train_state |= ANC_TRAIN_TONE_END;
        audio_anc_post_msg_icsd_anc_cmd(ANC_CMD_TRAIN_AFTER_TONE);
    }
#else
    extern void audio_anc_post_msg_user_train_init(u8 mode);
    audio_anc_post_msg_user_train_init(ANC_ON);
#endif/*ANC_USER_TRAIN_TONE_MODE*/

#else
    anc_user_train_cb(ANC_ON, anc_train_result, 0);
#endif/*ANC_USER_TRAIN_EN*/
}

#endif/*TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN*/
