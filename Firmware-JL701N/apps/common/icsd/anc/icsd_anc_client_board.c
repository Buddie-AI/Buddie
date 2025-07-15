
/*********************************************************************

 						 ANC 耳道自适应配置文件

*********************************************************************/


#if ADAPTIVE_CLIENT_BOARD == HT03_HYBRID_6G
const u8 cmp_iir_type[] = {
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 ff_iir_type[] = {
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
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;

#elif ADAPTIVE_CLIENT_BOARD == G96_HYBRID
const u8 cmp_iir_type[] = {
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 ff_iir_type[] = {
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
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;//ANC_FUN_TARGET_SYNC;

#elif ADAPTIVE_CLIENT_BOARD == P90_HYBRID
const u8 cmp_iir_type[] = {
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 ff_iir_type[] = {
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
const u8 ICSD_EP_TYPE = ICSD_HALF_INEAR;
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;

#elif ADAPTIVE_CLIENT_BOARD == D28_HYBRID
const u8 cmp_iir_type[] = {
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 ff_iir_type[] = {
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
const u8 FF_VERSION  =  2;
const u8 NEW_PRE_TREAT = 0;
const u8 ICSD_EP_TYPE = ICSD_HALF_INEAR;
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = ANC_FUN_TARGET_SYNC;

#elif ADAPTIVE_CLIENT_BOARD == ANC05_HYBRID
const u8 cmp_iir_type[] = {
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 ff_iir_type[] = {
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
const u8 ICSD_EP_TYPE = ICSD_HEADSET;
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;

#elif ADAPTIVE_CLIENT_BOARD == H3_HYBRID
const u8 cmp_iir_type[] = {
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
};

const u8 ff_iir_type[] = {
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
const u8 NEW_PRE_TREAT = 1;
const u8 ICSD_EP_TYPE = ICSD_HEADSET;
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;


#elif ADAPTIVE_CLIENT_BOARD == H3_TOZO_HYBIRD
const u8 cmp_iir_type[] = {
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
};

const u8 ff_iir_type[] = {
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
const u8 NEW_PRE_TREAT = 1;
const u8 ICSD_EP_TYPE = ICSD_HEADSET;
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;

#elif ADAPTIVE_CLIENT_BOARD == JH4006_HYBRID
const u8 cmp_iir_type[] = {
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_HIGH_SHELF,
};

const u8 ff_iir_type[] = {
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
const u8 NEW_PRE_TREAT = 1;
const u8 ICSD_EP_TYPE = ICSD_HEADSET;
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;

#else
const u8 cmp_iir_type[] = {
    ANC_IIR_LOW_SHELF,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
    ANC_IIR_BAND_PASS,
};

const u8 ff_iir_type[] = {
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
const u8 icsd_dcc[4] = {8, 4, 8, 1}; //ff1st,ff2nd,fb12t,fb2nd
const u16 ICSD_ANC_FUN = 0;

#endif

void icsd_xtmp_handler(float *xtmp, float gain_min, float gain_max)
{
#if ADAPTIVE_CLIENT_BOARD == H3_HYBRID
    if (xtmp[1] < 2.0) {
        xtmp[1] 		= (float)2.0;
    }
    if (xtmp[1] > 11.0) {
        xtmp[1] 		= (float)11.0;
    }
    if (xtmp[4] < 3.0) {
        xtmp[4] 		= (float)3.0;
    }
    if (xtmp[4] > 13.0) {
        xtmp[4] 		= (float)13.0;
    }

#elif ADAPTIVE_CLIENT_BOARD == H3_TOZO_HYBIRD
    if (xtmp[1] < 2.0) {
        xtmp[1] 		= (float)2.0;
    }
    if (xtmp[1] > 11.0) {
        xtmp[1] 		= (float)11.0;
    }
    if (xtmp[4] < 3.0) {
        xtmp[4] 		= (float)3.0;
    }
    if (xtmp[4] > 13.0) {
        xtmp[4] 		= (float)13.0;
    }

#else
    if (xtmp[1] < gain_min) {
        xtmp[1] 		= (float)gain_min;
    }
    if (xtmp[1] > gain_max) {
        xtmp[1] 		= (float)gain_max;
    }
#endif
}


void icsd_anc_board_config()
{
    icsd_anc_board_param_init();
    ANC_BOARD_PARAM->ff_yorder  = ANC_ADAPTIVE_FF_ORDER;
    ANC_BOARD_PARAM->fb_yorder  = ANC_ADAPTIVE_FB_ORDER;
    ANC_BOARD_PARAM->cmp_yorder = ANC_ADAPTIVE_CMP_ORDER;
    ANC_BOARD_PARAM->tool_target_sign = 1;

#if ADAPTIVE_CLIENT_BOARD == HT03_HYBRID_6G
#define PROJECT_098                 0
#define PROJECT_HT03                1
#define BOARD_PROJECT         		PROJECT_HT03
#if BOARD_PROJECT == PROJECT_HT03
    icsd_board_log("------------HT03_HYBRID_6G---------------\n");
#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				3
    extern const float HT03_DATA[];
    extern const s8 HT03_s8_DATA[];
    extern const float HT03_DOUBLE_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)HT03_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)HT03_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)HT03_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l = 120.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l = 0.4;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 11.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 18;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 1;
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
    ANC_BOARD_PARAM->cmp_type[0] = 10;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 1;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 1;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 1;
    ANC_BOARD_PARAM->cmp_thd_low = 3.0;
    ANC_BOARD_PARAM->cmp_thd_high = 10.0;

    ANC_BOARD_PARAM->IIR_NUM = 3;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 60;
    ANC_BOARD_PARAM->JT_MODE = 0;

#else
    icsd_board_log("------------ 098 -----------------\n");
#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				3

#endif

#elif ADAPTIVE_CLIENT_BOARD == G96_HYBRID

    icsd_board_log("------------G96_HYBRID---------------\n");
#define DEFAULT_FF_GAIN			    4
#define DEFAULT_FB_GAIN			    1
    extern const float G96_DATA[];
    extern const s8 G96_s8_DATA[];
    extern const float G96_DOUBLE_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)G96_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)G96_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)G96_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l = 120.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l = 18.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l = 0.4;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 15.0 - 12.5;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 10;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 2;
    ANC_BOARD_PARAM->idx_begin = 27;
    ANC_BOARD_PARAM->idx_end = 60;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 3;
    ANC_BOARD_PARAM->gain_min_offset = 10;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = -1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = -1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 0.7; //0.6

    ANC_BOARD_PARAM->tool_target_sign = 1;
    ANC_BOARD_PARAM->cmp_type[0] = 10;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 1;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 1;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 1;
    ANC_BOARD_PARAM->cmp_thd_low = 3.0;
    ANC_BOARD_PARAM->cmp_thd_high = 10.0;

    ANC_BOARD_PARAM->IIR_NUM = 3;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 45;
    ANC_BOARD_PARAM->JT_MODE = 1;

#elif ADAPTIVE_CLIENT_BOARD == G02_HYBRID_6G
    icsd_board_log("------------ G02 -----------------\n");
#define DEFAULT_FF_GAIN				4
#define DEFAULT_FB_GAIN				1
    extern const float G02_DATA[];
    extern const s8 G02_s8_DATA[];
    extern const float G02_DOUBLE_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)G02_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)G02_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)G02_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l = 130.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l = 14.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l = 0.5;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 22.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 10;
    ANC_BOARD_PARAM->gain_a_param = 12.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 1;
    ANC_BOARD_PARAM->idx_begin = 25;
    ANC_BOARD_PARAM->idx_end = 60;
    ANC_BOARD_PARAM->fb_w2r = 1200;
    ANC_BOARD_PARAM->FB_NFIX = 4;
    ANC_BOARD_PARAM->gain_min_offset = 22;
    ANC_BOARD_PARAM->gain_max_offset = 32;
    ANC_BOARD_PARAM->minvld = -30.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = -1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = -1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 0.6;//1.6;

    ANC_BOARD_PARAM->tool_target_sign = 1;
    ANC_BOARD_PARAM->cmp_type[0] = 10;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 1;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 1;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 1;
    ANC_BOARD_PARAM->cmp_thd_low = 3.0;
    ANC_BOARD_PARAM->cmp_thd_high = 10.0;

    ANC_BOARD_PARAM->IIR_NUM = 3;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 60;
    ANC_BOARD_PARAM->JT_MODE = 0;

#elif ADAPTIVE_CLIENT_BOARD == ANC05_HYBRID
    icsd_board_log("------------ ANC05_HYBRID -----------------\n");
#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				3
    extern const float ANC05_L_DATA[];
    extern const float ANC05_R_DATA[];
    extern const s8 ANC05_s8_DATA[];
    extern const double ANC05_L_DOUBLE_DATA[];
    extern const double ANC05_R_DOUBLE_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)ANC05_L_DATA;
    ANC_BOARD_PARAM->anc_data_r = (void *)ANC05_R_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)ANC05_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)ANC05_L_DOUBLE_DATA;
    ANC_BOARD_PARAM->anc_double_data_r = (void *)ANC05_R_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l      = 12.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->m_value_r  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_r      = 12.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_r     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 13.0 - 18.0;
    ANC_BOARD_PARAM->sen_offset_r = 13.0 - 18.0;
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
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = 1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 2.0;
    ANC_BOARD_PARAM->tool_target_sign = -1;//工具性能线反时需要调整该标志位

    ANC_BOARD_PARAM->cmp_type[0] = 10;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 0;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 1;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 1;
    ANC_BOARD_PARAM->cmp_thd_low = -40.0;
    ANC_BOARD_PARAM->cmp_thd_high = 40.0;

    ANC_BOARD_PARAM->IIR_NUM = 4;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 60;
    ANC_BOARD_PARAM->JT_MODE = 0;

#elif ADAPTIVE_CLIENT_BOARD == H3_HYBRID
    icsd_board_log("------------ H3_HYBRID -----------------\n");
#define DEFAULT_FF_GAIN				4
#define DEFAULT_FB_GAIN				1
    extern const float H3_L_DATA[];
    extern const float H3_R_DATA[];
    extern const s8 H3_s8_DATA[];
    extern const double H3_L_DOUBLE_DATA[];
    extern const double H3_R_DOUBLE_DATA[];

    ANC_BOARD_PARAM->anc_data_l = (void *)H3_L_DATA;
    ANC_BOARD_PARAM->anc_data_r = (void *)H3_L_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)H3_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)H3_L_DOUBLE_DATA;
    ANC_BOARD_PARAM->anc_double_data_r = (void *)H3_L_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l      = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->m_value_r  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_r      = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_r     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 13.0 - 18.0;
    ANC_BOARD_PARAM->sen_offset_r = 13.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 8;
    ANC_BOARD_PARAM->ff_target_fix_num_r = 8;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 2;
    ANC_BOARD_PARAM->idx_begin = 23;
    ANC_BOARD_PARAM->idx_end = 60;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 4;
    ANC_BOARD_PARAM->gain_min_offset = 11;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = 1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 2.0;

    ANC_BOARD_PARAM->tool_target_sign = -1;
    ANC_BOARD_PARAM->cmp_type[0] = 10;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 1;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 1;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 0;
    ANC_BOARD_PARAM->cmp_thd_low = -40.0;
    ANC_BOARD_PARAM->cmp_thd_high = 40.0;

    ANC_BOARD_PARAM->IIR_NUM = 4;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 50;//高声压SZ优先级
    ANC_BOARD_PARAM->JT_MODE = 0;

#elif ADAPTIVE_CLIENT_BOARD == H3_TOZO_HYBIRD
    icsd_board_log("------------ H3_TOZO_HYBIRD -----------------\n");
#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				1
    extern const float H3_TOZO_L_DATA[];
    extern const float H3_TOZO_R_DATA[];
    extern const s8 H3_TOZO_s8_DATA[];
    extern const double H3_TOZO_L_DOUBLE_DATA[];
    extern const double H3_TOZO_R_DOUBLE_DATA[];

    ANC_BOARD_PARAM->anc_data_l = (void *)H3_TOZO_L_DATA;
    ANC_BOARD_PARAM->anc_data_r = (void *)H3_TOZO_L_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)H3_TOZO_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)H3_TOZO_L_DOUBLE_DATA;
    ANC_BOARD_PARAM->anc_double_data_r = (void *)H3_TOZO_L_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l      = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->m_value_r  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_r      = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_r     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 13.0 - 18.0;
    ANC_BOARD_PARAM->sen_offset_r = 13.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 8;
    ANC_BOARD_PARAM->ff_target_fix_num_r = 8;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 2;
    ANC_BOARD_PARAM->idx_begin = 23;
    ANC_BOARD_PARAM->idx_end = 60;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 4;
    ANC_BOARD_PARAM->gain_min_offset = 11;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = 1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 2.0;

    ANC_BOARD_PARAM->tool_target_sign = -1;
    ANC_BOARD_PARAM->cmp_type[0] = 1;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 1;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 1;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 0;
    ANC_BOARD_PARAM->cmp_thd_low = -40.0;
    ANC_BOARD_PARAM->cmp_thd_high = 40.0;

    ANC_BOARD_PARAM->IIR_NUM = 4;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 50;//高声压SZ优先级
    ANC_BOARD_PARAM->JT_MODE = 0;

#elif ADAPTIVE_CLIENT_BOARD == P90_HYBRID

    icsd_board_log("------------P90_HYBRID---------------\n");
#define DEFAULT_FF_GAIN			    4
#define DEFAULT_FB_GAIN			    1
    extern const float P90_DATA[];
    extern const s8 P90_s8_DATA[];
    extern const float P90_DOUBLE_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)P90_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)P90_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)P90_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l = 120.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l = 18.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l = 0.4;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 15.0 - 12.5;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 10;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 3;
    ANC_BOARD_PARAM->idx_begin = 0;
    ANC_BOARD_PARAM->idx_end = 60;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 3;
    ANC_BOARD_PARAM->gain_min_offset = 10;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = 1;
    ANC_BOARD_PARAM->pz_max_times = 1;//PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = 1;//-1
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;//-1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 0.7; //0.6

    ANC_BOARD_PARAM->tool_target_sign = 1;
    ANC_BOARD_PARAM->cmp_type[0] = 1;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 10;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 0;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 1;
    ANC_BOARD_PARAM->cmp_thd_low = -40.0;
    ANC_BOARD_PARAM->cmp_thd_high = 40.0;

    ANC_BOARD_PARAM->IIR_NUM = 4;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 40;
    ANC_BOARD_PARAM->JT_MODE = 1;

#elif ADAPTIVE_CLIENT_BOARD == D28_HYBRID

    icsd_board_log("------------D28_HYBRID---------------\n");
#define DEFAULT_FF_GAIN			    4
#define DEFAULT_FB_GAIN			    1
    extern const float D28_DATA[];
    extern const s8 D28_s8_DATA[];
    extern const float D28_DOUBLE_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)D28_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)D28_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)D28_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l = 120.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l = 18.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l = 0.4;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 15.0 - 12.5;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 0;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 4;
    ANC_BOARD_PARAM->idx_begin = 0;
    ANC_BOARD_PARAM->idx_end = 60;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 3;
    ANC_BOARD_PARAM->gain_min_offset = 10;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = 1;
    ANC_BOARD_PARAM->pz_max_times = 1;//PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = 1;//-1
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;//-1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 0.7; //0.6

    ANC_BOARD_PARAM->tool_target_sign = 1;
    ANC_BOARD_PARAM->cmp_type[0] = 1;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 10;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 0;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 1;
    ANC_BOARD_PARAM->cmp_thd_low = -40.0;
    ANC_BOARD_PARAM->cmp_thd_high = 40.0;

    ANC_BOARD_PARAM->IIR_NUM = 7;
    ANC_BOARD_PARAM->IIR_NUM_FIX = ANC_ADAPTIVE_FF_ORDER - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 40;
    ANC_BOARD_PARAM->JT_MODE = 1;


#elif ADAPTIVE_CLIENT_BOARD == JH4006_HYBRID
    icsd_board_log("------------ JH4006_HYBRID -----------------\n");
#define DEFAULT_FF_GAIN				4
#define DEFAULT_FB_GAIN				1
    extern const float JH4006_L_DATA[];
    //extern const float JH4006_R_DATA[];
    extern const s8 JH4006_s8_DATA[];
    extern const double JH4006_L_DOUBLE_DATA[];
    extern const double JH4006_R_DOUBLE_DATA[];

    ANC_BOARD_PARAM->anc_data_l = (void *)JH4006_L_DATA;
    ANC_BOARD_PARAM->anc_data_r = (void *)JH4006_L_DATA;
    ANC_BOARD_PARAM->anc_db = (void *)JH4006_s8_DATA;
    ANC_BOARD_PARAM->anc_double_data_l = (void *)JH4006_L_DOUBLE_DATA;
    ANC_BOARD_PARAM->anc_double_data_r = (void *)JH4006_L_DOUBLE_DATA;
    ANC_BOARD_PARAM->m_value_l  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l      = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->m_value_r  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_r      = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_r     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 13.0 - 18.0;
    ANC_BOARD_PARAM->sen_offset_r = 13.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 8;
    ANC_BOARD_PARAM->ff_target_fix_num_r = 8;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 2;
    ANC_BOARD_PARAM->idx_begin = 23;
    ANC_BOARD_PARAM->idx_end = 60;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 4;
    ANC_BOARD_PARAM->gain_min_offset = 11;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = 1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 2.0;

    ANC_BOARD_PARAM->tool_target_sign = -1;
    ANC_BOARD_PARAM->cmp_type[0] = 10;
    ANC_BOARD_PARAM->cmp_type[1] = 1;
    ANC_BOARD_PARAM->cmp_type[2] = 1;
    ANC_BOARD_PARAM->cmp_type[3] = 1;
    ANC_BOARD_PARAM->cmp_type[4] = 1;
    ANC_BOARD_PARAM->cmp_type[5] = 1;
    ANC_BOARD_PARAM->cmp_type[6] = 1;
    ANC_BOARD_PARAM->cmp_type[7] = 0;
    ANC_BOARD_PARAM->cmp_thd_low = -40.0;
    ANC_BOARD_PARAM->cmp_thd_high = 40.0;

    ANC_BOARD_PARAM->IIR_NUM = 4;
    ANC_BOARD_PARAM->IIR_NUM_FIX = 8 - ANC_BOARD_PARAM->IIR_NUM;
    ANC_BOARD_PARAM->IIR_COEF = ANC_BOARD_PARAM->IIR_NUM * 3 + 1;

    ANC_BOARD_PARAM->sz_priority_thr = 50;//高声压SZ优先级
    ANC_BOARD_PARAM->JT_MODE = 0;

#else

#endif
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

