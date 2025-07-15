#if ADT_CLIENT_BOARD == HT03_HYBRID_6G
const u8 ADT_TWS_MODE = 1;
//const u8 ADT_DET_VER  = 2;
//const u8 ADT_PATH_CONFIG  = 0;
const u8 ADT_DET_VER  = 4;
const u8 ADT_PATH_CONFIG  = ADT_PATH_DAC_SRC_EN;
const u8 ADT_EIN_VER  = ADT_EIN_TWS_V0;
const float ref_gain_sel  = 3;
const float err_gain_sel  = 3;
u8 pnc_ref_pwr_thr = 160;
u8 pnc_err_pwr_thr = 100;
u8 tpc_ref_pwr_thr = 120;
u8 tpc_err_pwr_thr = 100;
u8 anc_ref_pwr_thr = 120;
u8 anc_err_pwr_thr = 100;
const float wat_pn_gain = 1;
const float wat_bypass_gain = 16;
const float wat_anc_gain = 1;
const u8 final_speech_even_thr = 4;
const float wdac_senc = 1;
const float wdac[8] = {1, 1, 0.92, 0.80, 0.67, 0.62, 0.7, 0.7};//播放一笑江湖DJ  /100
const float wref[8] = {6.78, 3.66, 0.58, 0.37, 0.31, 0.29, 0.28, 0.27};//播放外部噪声
const float wref_tran[8] = {1.26, 1.06, 1.06, 1.00, 1.00, 1.18, 1.16, 1.03 };
const float wref_ancs[8] = {2.06, 2.00, 0.011, 0.006, 0.009, 0.012, 0.014, 0.018 };
const float fdac[8] = {0.18, 0.051, 0.02, 0.02, 0.02, 0.02, 0.02, 0.05};//播放一笑江湖DJ /1000
u8 icsd_get_ang_flag(float *_target_angle)
{
    u8 af1 = _target_angle[1] > 15 || _target_angle[1] < -130;
    u8 af2 = _target_angle[2] > 15 || _target_angle[2] < -130;
    u8 af3 = _target_angle[3] > 15 || _target_angle[3] < -130;
    u8 ang_flag = af1 && af2 && af3;
    return ang_flag;
}

#elif ADT_CLIENT_BOARD == G96_HYBRID
const u8 ADT_TWS_MODE = 1;
//const u8 ADT_DET_VER  = 5;
//const u8 ADT_PATH_CONFIG  = 0;
const u8 ADT_DET_VER  = 4;
const u8 ADT_PATH_CONFIG  = ADT_PATH_DAC_SRC_EN;
const u8 ADT_EIN_VER  = ADT_EIN_TWS_V0;
const float ref_gain_sel = 4;
const float err_gain_sel = 1;
u8 pnc_ref_pwr_thr = 160;
u8 pnc_err_pwr_thr = 100;
u8 tpc_ref_pwr_thr = 120;
u8 tpc_err_pwr_thr = 100;
u8 anc_ref_pwr_thr = 120;
u8 anc_err_pwr_thr = 135;
const float wat_pn_gain = 2.4;
const float wat_bypass_gain = 16;
const float wat_anc_gain = 2.5;
const u8 final_speech_even_thr = 4;
const float wdac_senc = 1;
const float wdac[8] = {2.98, 2.29, 1.94, 1.95, 1.76, 1.54, 1.44, 1.29};
const float wref[8] = {6.76, 1.98, 1.57, 1.50, 1.41, 1.32, 1.16, 1.02};
const float wref_tran[8] = {4.26, 2.36, 2.36, 2.79, 3.00, 3.18, 3.16, 3.03 };
const float wref_ancs[8] = {2.06, 2.00, 0.11, 0.06, 0.09, 0.12, 0.14, 0.18 };
const float fdac[8] = {0.008, 0.008, 0.008, 0.008, 0.01, 0.01, 0.01, 0.01};
u8 icsd_get_ang_flag(float *_target_angle)
{
    u8 af1 = _target_angle[4] < 15 && _target_angle[4] > -150;
    u8 af2 = _target_angle[2] < 15 && _target_angle[2] > -150;
    u8 af3 = _target_angle[3] < 15 && _target_angle[3] > -150;
    u8 ang_flag = af1 && af2 && af3;
    return ang_flag;
}

#elif ADT_CLIENT_BOARD == G97_HYBRID
const u8 ADT_TWS_MODE = 1;
const u8 ADT_DET_VER  = 3;
const u8 ADT_PATH_CONFIG  = 0;
//const u8 ADT_DET_VER  = 4;
//const u8 ADT_PATH_CONFIG  = ADT_PATH_DAC_SRC_EN;
const u8 ADT_EIN_VER  = ADT_EIN_UNSUPPORT;
const float ref_gain_sel = 2;
const float err_gain_sel = 1;
u8 pnc_ref_pwr_thr = 160;
u8 pnc_err_pwr_thr = 100;
u8 tpc_ref_pwr_thr = 120;
u8 tpc_err_pwr_thr = 100;
u8 anc_ref_pwr_thr = 120;
u8 anc_err_pwr_thr = 135;
const float wat_pn_gain = 2.88;
const float wat_bypass_gain = 19.2;
const float wat_anc_gain = 3;
const u8 final_speech_even_thr = 4;
const float wdac_senc = 1;
const float wdac[8] = {10.43, 4.85, 1.36, 1.09, 0.87, 0.71, 0.63, 0.59};
const float wref[8] = {7.31, 1.90, 0.43, 0.33, 0.28, 0.25, 0.22, 0.20};
const float wref_tran[8] = {8.23, 3.33, 1.72, 1.64, 1.48, 1.30, 1.12, 1.04};
const float wref_ancs[8] = {4.62, 1.32, 0.09, 0.06, 0.07, 0.10, 0.13, 0.15 };
const float fdac[8] = {0.18, 0.051, 0.02, 0.02, 0.02, 0.02, 0.02, 0.05};
u8 icsd_get_ang_flag(float *_target_angle)
{
    u8 af1 = _target_angle[1] < -120 || _target_angle[1] > 20;
    u8 af2 = _target_angle[2] < -120 || _target_angle[2] > 20;
    u8 af3 = _target_angle[3] < -120 || _target_angle[3] > 20;
    u8 ang_flag = af1 && af2 && af3;

    printf("ang:%d %d %d af:%d %d %d\n", (int)_target_angle[1], (int)_target_angle[2], (int)_target_angle[3], af1, af2, af3);
    return ang_flag;
}

#elif ADT_CLIENT_BOARD == H3_HYBRID
const u8 ADT_TWS_MODE = 0;
const u8 ADT_DET_VER  = 1;
const u8 ADT_PATH_CONFIG  = 0;
const u8 ADT_EIN_VER  = ADT_EIN_HEADSET_V0;
const float ref_gain_sel = 3;
const float err_gain_sel = 3;
u8 pnc_ref_pwr_thr = 160;
u8 pnc_err_pwr_thr = 100;
u8 tpc_ref_pwr_thr = 120;
u8 tpc_err_pwr_thr = 100;
u8 anc_ref_pwr_thr = 120;
u8 anc_err_pwr_thr = 135;
const float wat_pn_gain = 1;
const float wat_bypass_gain = 16;
const float wat_anc_gain = 1;
const u8 final_speech_even_thr = 4;
const float wdac_senc = 1;
const float wref[8] = {6.78, 3.66, 0.58, 0.37, 0.31, 0.29, 0.28, 0.27};
const float wdac[8] = {1, 1, 0.92, 0.80, 0.67, 0.62, 0.7, 0.7};
const float wref_tran[8] = {4.26, 2.36, 2.36, 2.79, 3.00, 3.18, 3.16, 3.03 };
const float wref_ancs[8] = {2.06, 2.00, 0.011, 0.006, 0.009, 0.012, 0.014, 0.018 };
const float fdac[8] = {0.18, 0.051, 0.02, 0.02, 0.02, 0.02, 0.02, 0.05};

#elif ADT_CLIENT_BOARD == ANC05_HYBRID
const u8 ADT_TWS_MODE = 0;
const u8 ADT_DET_VER  = 1;
const u8 ADT_PATH_CONFIG  = 0;
const u8 ADT_EIN_VER  = ADT_EIN_UNSUPPORT;
const float ref_gain_sel = 3;
const float err_gain_sel = 3;
u8 pnc_ref_pwr_thr = 160;
u8 pnc_err_pwr_thr = 100;
u8 tpc_ref_pwr_thr = 120;
u8 tpc_err_pwr_thr = 100;
u8 anc_ref_pwr_thr = 120;
u8 anc_err_pwr_thr = 135;
const float wat_pn_gain = 1;
const float wat_bypass_gain = 16;
const float wat_anc_gain = 1;
const u8 final_speech_even_thr = 4;
const float wdac_senc = 1;
const float wref[8] = {6.78, 3.66, 0.58, 0.37, 0.31, 0.29, 0.28, 0.27};
const float wdac[8] = {1, 1, 0.92, 0.80, 0.67, 0.62, 0.7, 0.7};
const float wref_tran[8] = {4.26, 2.36, 2.36, 2.79, 3.00, 3.18, 3.16, 3.03 };
const float wref_ancs[8] = {2.06, 2.00, 0.011, 0.006, 0.009, 0.012, 0.014, 0.018 };
const float fdac[8] = {0.18, 0.051, 0.02, 0.02, 0.02, 0.02, 0.02, 0.05};

#else
const float wdac_senc = 1;
const u8 ADT_TWS_MODE = 1;
const u8 ADT_DET_VER  = 0;
const u8 ADT_PATH_CONFIG  = 0;
const u8 ADT_EIN_VER  = ADT_EIN_UNSUPPORT;
const float ref_gain_sel = 3;
const float err_gain_sel = 3;
u8 pnc_ref_pwr_thr = 160;
u8 pnc_err_pwr_thr = 100;
u8 tpc_ref_pwr_thr = 120;
u8 tpc_err_pwr_thr = 100;
u8 anc_ref_pwr_thr = 120;
u8 anc_err_pwr_thr = 135;
const float wat_pn_gain = 1;
const float wat_bypass_gain = 16;
const float wat_anc_gain = 1;
const u8 final_speech_even_thr = 4;
const float wref[8] = {6.78, 3.66, 0.58, 0.37, 0.31, 0.29, 0.28, 0.27};
const float wdac[8] = {1, 1, 0.92, 0.80, 0.67, 0.62, 0.7, 0.7};
const float wref_tran[8] = {4.26, 2.36, 2.36, 2.79, 3.00, 3.18, 3.16, 3.03 };
const float wref_ancs[8] = {2.06, 2.00, 0.011, 0.006, 0.009, 0.012, 0.014, 0.018 };
const float fdac[8] = {0.18, 0.051, 0.02, 0.02, 0.02, 0.02, 0.02, 0.05};

#endif

void icsd_adt_ein_parm_HT03(__icsd_adt_parm *_ADT_PARM)
{
    _ADT_PARM->anc_in_pz_sum = -80;
    _ADT_PARM->anc_out_pz_sum = 0;
    _ADT_PARM->anc_in_sz_mean = 1;
    _ADT_PARM->anc_out_sz_mean = -12;
    _ADT_PARM->anc_in_fb_sum = 60;
    _ADT_PARM->anc_out_fb_sum = 30;

    _ADT_PARM->pnc_in_pz_sum = -80;
    _ADT_PARM->pnc_out_pz_sum = 0;
    _ADT_PARM->pnc_in_sz_mean = 1;
    _ADT_PARM->pnc_out_sz_mean = -12;
    _ADT_PARM->pnc_in_fb_sum = 60;
    _ADT_PARM->pnc_out_fb_sum = 30;

    _ADT_PARM->trans_in_pz_sum = 2;
    _ADT_PARM->trans_out_pz_sum = 26;
    _ADT_PARM->trans_in_sz_mean = 2;
    _ADT_PARM->trans_out_sz_mean = -7;
    _ADT_PARM->trans_in_fb_sum = 20;
    _ADT_PARM->trans_out_fb_sum = -20;
}

void icsd_adt_ein_parm(__icsd_adt_parm *_ADT_PARM)
{
    _ADT_PARM->anc_in_pz_sum = -80;
    _ADT_PARM->anc_out_pz_sum = 0;
    _ADT_PARM->anc_in_sz_mean = 1;
    _ADT_PARM->anc_out_sz_mean = -12;
    _ADT_PARM->anc_in_fb_sum = 60;
    _ADT_PARM->anc_out_fb_sum = 30;

    _ADT_PARM->pnc_in_pz_sum = -80;
    _ADT_PARM->pnc_out_pz_sum = 0;
    _ADT_PARM->pnc_in_sz_mean = 1;
    _ADT_PARM->pnc_out_sz_mean = -12;
    _ADT_PARM->pnc_in_fb_sum = 60;
    _ADT_PARM->pnc_out_fb_sum = 30;

    _ADT_PARM->trans_in_pz_sum = 2;
    _ADT_PARM->trans_out_pz_sum = 26;
    _ADT_PARM->trans_in_sz_mean = 2;
    _ADT_PARM->trans_out_sz_mean = -7;
    _ADT_PARM->trans_in_fb_sum = 20;
    _ADT_PARM->trans_out_fb_sum = -20;
}

void icsd_adt_voice_board_data()
{
    __icsd_adt_parm *ADT_PARM;

    ADT_PARM = zalloc(sizeof(__icsd_adt_parm));
#if ADT_CLIENT_BOARD == HT03_HYBRID_6G
    adt_printf("ADT HT03\n");
    extern const float HT03_ADT_FLOAT_DATA[];
    extern const float HT03_sz_out_data[28];
    extern const float HT03_pz_out_data[28];
    ADT_PARM->adt_float_data = (void *)HT03_ADT_FLOAT_DATA;
    ADT_PARM->sz_inptr = (float *)HT03_sz_out_data;
    ADT_PARM->pz_inptr = (float *)HT03_pz_out_data;
    icsd_adt_ein_parm_HT03(ADT_PARM);

#elif ADT_CLIENT_BOARD == G96_HYBRID
    adt_printf("ADT G96\n");
    extern const float G96_ADT_FLOAT_DATA[];
    extern const float G96_sz_out_data[28];
    extern const float G96_pz_out_data[28];
    ADT_PARM->adt_float_data = (void *)G96_ADT_FLOAT_DATA;
    ADT_PARM->sz_inptr = (float *)G96_sz_out_data;
    ADT_PARM->pz_inptr = (float *)G96_pz_out_data;
    icsd_adt_ein_parm(ADT_PARM);

#elif ADT_CLIENT_BOARD == G97_HYBRID
    adt_printf("ADT G97\n");
    extern const float G97_ADT_FLOAT_DATA[];
    extern const float G97_sz_out_data[28];
    extern const float G97_pz_out_data[28];
    ADT_PARM->adt_float_data = (void *)G97_ADT_FLOAT_DATA;
    ADT_PARM->sz_inptr = (float *)G97_sz_out_data;
    ADT_PARM->pz_inptr = (float *)G97_pz_out_data;
    icsd_adt_ein_parm(ADT_PARM);

#elif ADT_CLIENT_BOARD == H3_HYBRID
    adt_printf("ADT H3\n");
    extern const float H3_ADT_FLOAT_DATA[];
    extern const float H3_sz_out_data[28];
    extern const float H3_pz_out_data[28];
    ADT_PARM->adt_float_data = (void *)H3_ADT_FLOAT_DATA;
    ADT_PARM->sz_inptr = (float *)H3_sz_out_data;
    ADT_PARM->pz_inptr = (float *)H3_pz_out_data;
    icsd_adt_ein_parm(ADT_PARM);

#elif ADT_CLIENT_BOARD == ANC05_HYBRID
    adt_printf("ADT ANC05\n");
    extern const float ANC05_ADT_FLOAT_DATA[];
    extern const float ANC05_sz_out_data[28];
    extern const float ANC05_pz_out_data[28];
    ADT_PARM->adt_float_data = (void *)ANC05_ADT_FLOAT_DATA;
    ADT_PARM->sz_inptr = (float *)ANC05_sz_out_data;
    ADT_PARM->pz_inptr = (float *)ANC05_pz_out_data;
    icsd_adt_ein_parm(ADT_PARM);

#else
    adt_printf("ADT XXX\n");
#endif

    icsd_adt_parm_set(ADT_PARM);
    free(ADT_PARM);

    icsd_adt_szout_init();
}






