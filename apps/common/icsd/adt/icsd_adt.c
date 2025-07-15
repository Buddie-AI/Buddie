#include "app_config.h"
#if ((defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN && \
	 TCFG_AUDIO_ANC_ENABLE)
#include "system/task.h"
#include "icsd_adt.h"
#include "icsd_adt_app.h"
#include "classic/tws_api.h"
#include "icsd_anc.h"
#include "tone_player.h"
#include "icsd_anc_user.h"

#define ICSD_ADT_TASK_NAME  "icsd_adt"

const u8 VDT_TRAIN_EN = 0;

u8 adt_task = 0;
u8 src_task = 0;
int (*adt_printf)(const char *format, ...);
int (*wat_printf)(const char *format, ...);
int (*ein_printf)(const char *format, ...);
int printf_off(const char *format, ...)
{
    return 0;
}

#define ADT_DATA_DEBUG  0
void icsd_adt_run_output(__adt_result *adt_result)
{
    u8 voice_state = adt_result->voice_state;
    u8 wind_lvl    = adt_result->wind_lvl;
    u8 wat_result  = adt_result->wat_result;
    u8 ein_state   = adt_result->ein_state;

#if ADT_DATA_DEBUG
    extern void icsd_adt_output_demo(__adt_result * adt_result);
    icsd_adt_output_demo(adt_result);
#else
    void audio_acoustic_detector_output_hdl(u8 voice_state, u8 wind_lvl, u8 wat_result);
    audio_acoustic_detector_output_hdl(voice_state, wind_lvl, wat_result);
#endif
}

#define AUDIO_ADC_CH0   BIT(0)
#define AUDIO_ADC_CH1   BIT(1)
#define AUDIO_ADC_CH2   BIT(2)
#define AUDIO_ADC_CH3   BIT(3)
void icsd_adt_mic_ch_analog_open(u32 ch)
{
    if (ch &  AUDIO_ADC_CH0) {
        /*close mic0 analog config*/
        SFR(JL_ADDA->ADA_CON4, 15,  1,  0);        // AUDADC_MICA_BUFP_EN_10v
        SFR(JL_ADDA->ADA_CON4, 14,  1,  0);        // AUDADC_MICA_BUFN_EN_10v
        SFR(JL_ADDA->ADA_CON4,  4,  1,  1);        // AUDADC_MICA_AMP_EN_10v
        SFR(JL_ADDA->ADA_CON1,  8,  1,  1);        // AUDADC_CHA_S2_RSTB_10v
        SFR(JL_ADDA->ADA_CON1,  7,  1,  1);        // AUDADC_CHA_EN_10v
        SFR(JL_ADDA->ADA_CON1,  6,  1,  1);        // AUDADC_CHA_BIAS_EN_10v
    }
    if (ch &  AUDIO_ADC_CH1) {
        /*close mic1 analog config*/
        SFR(JL_ADDA->ADA_CON5, 15,  1,  0);        // AUDADC_MICB_BUFP_EN_10v
        SFR(JL_ADDA->ADA_CON5, 14,  1,  0);        // AUDADC_MICB_BUFN_EN_10v
        SFR(JL_ADDA->ADA_CON5,  4,  1,  1);        // AUDADC_MICB_AMP_EN_10v
        SFR(JL_ADDA->ADA_CON1, 11,  1,  1);        // AUDADC_CHB_S2_RSTB_10v
        SFR(JL_ADDA->ADA_CON1, 10,  1,  1);        // AUDADC_CHB_EN_10v
        SFR(JL_ADDA->ADA_CON1,  9,  1,  1);        // AUDADC_CHB_BIAS_EN_10v
    }
    if (ch &  AUDIO_ADC_CH2) {
        /*close mic2 analog config*/
        SFR(JL_ADDA->ADA_CON6, 15,  1,  0);        // AUDADC_MICC_BUFP_EN_10v
        SFR(JL_ADDA->ADA_CON6, 14,  1,  0);        // AUDADC_MICC_BUFN_EN_10v
        SFR(JL_ADDA->ADA_CON6,  4,  1,  1);        // AUDADC_MICC_AMP_EN_10v
        SFR(JL_ADDA->ADA_CON1, 14,  1,  1);        // AUDADC_CHC_S2_RSTB_10v
        SFR(JL_ADDA->ADA_CON1, 13,  1,  1);        // AUDADC_CHC_EN_10v
        SFR(JL_ADDA->ADA_CON1, 12,  1,  1);        // AUDADC_CHC_BIAS_EN_10v
    }
    if (ch &  AUDIO_ADC_CH3) {
        /*close mic3 analog config*/
        SFR(JL_ADDA->ADA_CON7, 15,  1,  0);        // AUDADC_MICD_BUFP_EN_10v
        SFR(JL_ADDA->ADA_CON7, 14,  1,  0);        // AUDADC_MICD_BUFN_EN_10v
        SFR(JL_ADDA->ADA_CON7,  4,  1,  1);        // AUDADC_MICD_AMP_EN_10v
        SFR(JL_ADDA->ADA_CON1, 17,  1,  1);        // AUDADC_CHD_S2_RSTB_10v
        SFR(JL_ADDA->ADA_CON1, 16,  1,  1);        // AUDADC_CHD_EN_10v
        SFR(JL_ADDA->ADA_CON1, 15,  1,  1);        // AUDADC_CHD_BIAS_EN_10v
    }
}

void icsd_adt_mic_ch_analog_close(u32 ch)
{
    if (ch &  AUDIO_ADC_CH0) {
        /*close mic0 analog config*/
        SFR(JL_ADDA->ADA_CON4, 15,  1,  0);        // AUDADC_MICA_BUFP_EN_10v
        SFR(JL_ADDA->ADA_CON4, 14,  1,  0);        // AUDADC_MICA_BUFN_EN_10v
        SFR(JL_ADDA->ADA_CON4,  4,  1,  0);        // AUDADC_MICA_AMP_EN_10v
        SFR(JL_ADDA->ADA_CON1,  8,  1,  0);        // AUDADC_CHA_S2_RSTB_10v
        SFR(JL_ADDA->ADA_CON1,  7,  1,  0);        // AUDADC_CHA_EN_10v
        SFR(JL_ADDA->ADA_CON1,  6,  1,  0);        // AUDADC_CHA_BIAS_EN_10v
    }
    if (ch &  AUDIO_ADC_CH1) {
        /*close mic1 analog config*/
        SFR(JL_ADDA->ADA_CON5, 15,  1,  0);        // AUDADC_MICB_BUFP_EN_10v
        SFR(JL_ADDA->ADA_CON5, 14,  1,  0);        // AUDADC_MICB_BUFN_EN_10v
        SFR(JL_ADDA->ADA_CON5,  4,  1,  0);        // AUDADC_MICB_AMP_EN_10v
        SFR(JL_ADDA->ADA_CON1, 11,  1,  0);        // AUDADC_CHB_S2_RSTB_10v
        SFR(JL_ADDA->ADA_CON1, 10,  1,  0);        // AUDADC_CHB_EN_10v
        SFR(JL_ADDA->ADA_CON1,  9,  1,  0);        // AUDADC_CHB_BIAS_EN_10v
    }
    if (ch &  AUDIO_ADC_CH2) {
        /*close mic2 analog config*/
        SFR(JL_ADDA->ADA_CON6, 15,  1,  0);        // AUDADC_MICC_BUFP_EN_10v
        SFR(JL_ADDA->ADA_CON6, 14,  1,  0);        // AUDADC_MICC_BUFN_EN_10v
        SFR(JL_ADDA->ADA_CON6,  4,  1,  0);        // AUDADC_MICC_AMP_EN_10v
        SFR(JL_ADDA->ADA_CON1, 14,  1,  0);        // AUDADC_CHC_S2_RSTB_10v
        SFR(JL_ADDA->ADA_CON1, 13,  1,  0);        // AUDADC_CHC_EN_10v
        SFR(JL_ADDA->ADA_CON1, 12,  1,  0);        // AUDADC_CHC_BIAS_EN_10v
    }
    if (ch &  AUDIO_ADC_CH3) {
        /*close mic3 analog config*/
        SFR(JL_ADDA->ADA_CON7, 15,  1,  0);        // AUDADC_MICD_BUFP_EN_10v
        SFR(JL_ADDA->ADA_CON7, 14,  1,  0);        // AUDADC_MICD_BUFN_EN_10v
        SFR(JL_ADDA->ADA_CON7,  4,  1,  0);        // AUDADC_MICD_AMP_EN_10v
        SFR(JL_ADDA->ADA_CON1, 17,  1,  0);        // AUDADC_CHD_S2_RSTB_10v
        SFR(JL_ADDA->ADA_CON1, 16,  1,  0);        // AUDADC_CHD_EN_10v
        SFR(JL_ADDA->ADA_CON1, 15,  1,  0);        // AUDADC_CHD_BIAS_EN_10v
    }
}

u8 talk_mic_analog_close = 0;
u8 audio_adt_talk_mic_analog_close()
{
    if (talk_mic_analog_close == 0) {
        printf("-----------audio_adt_talk_mic_analog_close\n");
        u8 talk_mic_ch = icsd_get_talk_mic_ch();
        icsd_adt_mic_ch_analog_close(talk_mic_ch);
        talk_mic_analog_close = 1;
        return 1;
    }
    return 0;
}

u8 audio_adt_talk_mic_analog_open()
{
    if (talk_mic_analog_close) {
        printf("-----------audio_adt_talk_mic_analog_open\n");
        u8 talk_mic_ch = icsd_get_talk_mic_ch();
        icsd_adt_mic_ch_analog_open(talk_mic_ch);
        talk_mic_analog_close = 0;
        return 1;
    }
    return 0;
}

float adt_pow10(float n)
{
    float pow10n = exp_float((float)n / log10_float(exp_float((float)1.0)));
    return pow10n;
}

u32 icsd_adt_get_role()
{
    return tws_api_get_role();
}

u32 icsd_adt_get_tws_state()
{
    return tws_api_get_tws_state();
}

#define TWS_FUNC_ID_SDADT_M2S    TWS_FUNC_ID('A', 'D', 'T', 'M')
REGISTER_TWS_FUNC_STUB(icsd_adt_m2s) = {
    .func_id = TWS_FUNC_ID_SDADT_M2S,
    .func    = icsd_adt_m2s_cb,
};
#define TWS_FUNC_ID_SDADT_S2M    TWS_FUNC_ID('A', 'D', 'T', 'S')
REGISTER_TWS_FUNC_STUB(icsd_adt_s2m) = {
    .func_id = TWS_FUNC_ID_SDADT_S2M,
    .func    = icsd_adt_s2m_cb,
};

void icsd_adt_tws_m2s(u8 cmd)
{
    local_irq_disable();
    s8 data[8];
    icsd_adt_m2s_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 8, TWS_FUNC_ID_SDADT_M2S);
    local_irq_enable();
}

void icsd_adt_tws_s2m(u8 cmd)
{
    local_irq_disable();
    s8 data[8];
    icsd_adt_s2m_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 8, TWS_FUNC_ID_SDADT_S2M);
    local_irq_enable();
}

void icsd_adt_anc_dma_on(u8 out_sel, int *buf, int len)
{
    anc_dma_on_double(out_sel, buf, len);
}

void icsd_adt_dma_done()
{
    icsd_acoustic_detector_ancdma_done();
}

void icsd_adt_hw_suspend()
{
    anc_core_dma_ie(0);
    anc_core_dma_stop();
}

void icsd_adt_hw_resume()
{
    audio_acoustic_detector_updata();
    icsd_adt_szout_init();
}

void icsd_adt_FFT_radix256(int *in_cur, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(256, 8, 1, 0, 1);
    hw_fft_run(fft_config, in_cur, out);
}

void icsd_adt_FFT_radix128(int *in_cur, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(128, 7, 1, 0, 1);
    hw_fft_run(fft_config, in_cur, out);
}

void icsd_adt_FFT_radix64(int *in_cur, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(64, 6, 1, 0, 1);
    hw_fft_run(fft_config, in_cur, out);
}

void icsd_post_anctask_msg(u8 cmd)
{
    audio_anc_post_msg_adt(cmd);
}

void icsd_post_adttask_msg(u8 cmd)
{
    if (adt_task == 1) {
        os_taskq_post_msg("icsd_adt", 1, cmd);
    }
}

void icsd_post_srctask_msg(u8 cmd)
{
    if (src_task == 1) {
        os_taskq_post_msg("icsd_src", 1, cmd);
    }
}

void icsd_adt_task(void *priv)
{
    int res = 0;
    int msg[30];
    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            icsd_adt_task_handler(msg[1]);
        }
    }
}

void icsd_adt_task_create()
{
    if (adt_task == 0) {
        task_create(icsd_adt_task, NULL, ICSD_ADT_TASK_NAME);
        adt_task = 1;
    }
}

void icsd_adt_task_kill()
{
    if (adt_task == 1) {
        task_kill(ICSD_ADT_TASK_NAME);
        adt_task = 0;
    }
}

void icsd_src_task(void *priv)
{
    int res = 0;
    int msg[30];
    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            icsd_src_task_handler(msg[1]);
        }
    }
}

void icsd_src_task_create()
{
    if (src_task == 0) {
        task_create(icsd_src_task, NULL, "icsd_src");
        src_task = 1;
    }
}

void icsd_src_task_kill()
{
    if (src_task == 1) {
        task_kill("icsd_src");
        src_task = 0;
    }
}

void icsd_adt_start()
{
    adt_printf = _adt_printf;
    wat_printf = _wat_printf;
    ein_printf = _ein_printf;
    printf("icsd_adt_start====================================\n");
}

void icsd_adt_szout_init()
{
#if ADT_USE_ADAPTIVE_SZ
    extern float icsd_anc_sz_data[120 + 40];
    extern float icsd_anc_pz_data[120 + 40];
    extern u8 sz_out_exist;//需要打开自适应ANC库中的 自适应SZ输出使能:ANC_SZ_OUT_EN
    icsd_adt_szout_set(icsd_anc_sz_data, icsd_anc_pz_data, sz_out_exist);
#else
    icsd_adt_szout_set(0, 0, 0);
#endif
}
const u16 ADT_DEBUG_INF = 0;//ADT_INF_1;//state
//state = 1, pwr = 7, dov = 8, angle_err = 7, angle_tlk = 3
const u8 adt_errpxx_dB_thr = 80;//60;
const u8 adt_err_zcv_thr = 10;//12;
const u8 pwr_inside_even_flag0_thr = 5; //pwr
const u8 angle_err_even_flag0_thr = 3;//4;  //angle_err
const u8 angle_talk_even_flag0_thr = 2; //angle_tlk
const u16 f1f2_angsum_thr = 200;
#ifndef ADT_CLIENT_BOARD
const u8 final_speech_even_thr = 4;     //dov


const u8 ADT_TWS_MODE = 1;
const u8 ADT_DET_VER  = 6;
const u8 ADT_PATH_CONFIG  = ADT_PATH_DAC_SRC_EN | ADT_PATH_3M_EN;
const u8 ADT_EIN_VER  = ADT_EIN_TWS_V0;
const float ref_gain_sel  = 3;
const float err_gain_sel  = 3;
u8 pnc_ref_pwr_thr = 160;
u8 pnc_err_pwr_thr = 100;
u8 tpc_ref_pwr_thr = 120;
u8 tpc_err_pwr_thr = 100;
u8 anc_ref_pwr_thr = 120;
u8 anc_err_pwr_thr = 100;

const float wat_pn_gain     = 0.22;
const float wat_bypass_gain = 16;
const float wat_anc_gain    = 0.22;

const float wdac_senc = 0.7; //越小播音乐时灵敏度越高
const float wdac[8] = { 0.35, 0.54, 0.57, 0.48, 0.37, 0.22, 0.38, 0.37};//播放音乐/100
const float wref[8] = {1.62, 0.90, 0.71, 0.60, 0.54, 0.50, 0.47, 0.46};//播放外部噪声
const float wref_tran[8] = {1.82, 1.80, 1.58, 1.30, 1.16, 1.12, 1.11, 1.12};
const float wref_ancs[8] = {1.92, 0.48, 0.07, 0.03, 0.05, 0.06, 0.06, 0.04 };
const float fdac[8] = {0.18, 0.051, 0.02, 0.02, 0.02, 0.02, 0.02, 0.05};//播放音乐/1000
u8 icsd_get_ang_flag(float *_target_angle)
{
    u8 af1 = _target_angle[1] > 15 || _target_angle[1] < -130;
    u8 af2 = _target_angle[2] > 15 || _target_angle[2] < -130;
    u8 af3 = _target_angle[3] > 15 || _target_angle[3] < -130;
    u8 ang_flag = af1 && af2 && af3;
    /* printf("get ang flag:%d\n", ang_flag); */
    return ang_flag;
}

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

void icsd_adt_voice_board_data()
{
    __icsd_adt_parm *ADT_PARM;
    ADT_PARM = zalloc(sizeof(__icsd_adt_parm));

    extern const float HT03_ADT_FLOAT_DATA[];
    extern const float HT03_sz_out_data[28];
    extern const float HT03_pz_out_data[28];
    ADT_PARM->adt_float_data = (void *)HT03_ADT_FLOAT_DATA;
    ADT_PARM->sz_inptr = (float *)HT03_sz_out_data;
    ADT_PARM->pz_inptr = (float *)HT03_pz_out_data;
    icsd_adt_ein_parm_HT03(ADT_PARM);

    icsd_adt_parm_set(ADT_PARM);
    free(ADT_PARM);

    icsd_adt_szout_init();
}
#else

#include "icsd_adt_client_board.c"

#endif/*ADC_CLIENT_BOARD*/


//=======================================================================================================

#if ADT_DATA_DEBUG

void icsd_play_num0()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NUM0);
}
void icsd_play_num1()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NUM1);
}
void icsd_play_num2()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NUM2);
}
void icsd_play_num3()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NUM3);
}
void icsd_play_normal()
{
    icsd_adt_tone_play(ICSD_ADT_TONE_NORMAL);
}

void icsd_adt_output_demo(__adt_result *adt_result)
{
#if ADT_EAR_IN_EN
    static u8 hold = 0;
    static u8 hold_flg = 0;
    static int hold_time = 0;
    if (adt_result->ein_state == 0) {
        hold = 1;
        hold_flg = 1;
        hold_time = 0;
        printf("------------------------------------------------------------------ein state:0\n");
    } else {
        if (hold_flg) {
            hold_flg = 0;
            hold_time = jiffies;
            printf("set hold time:%d\n", hold_time);
        }
        if ((hold_time > 0) && (hold)) {
            if (jiffies > (hold_time + 100)) {
                printf("hold time out:%d\n", jiffies);
                hold = 0;
            }
        }
        printf("--------------------------------ein state:1\n");
    }
    if (hold) {
        //return;
    }
#endif

    if (adt_result->wind_lvl) {
        printf("------wind lvl:%d\n", adt_result->wind_lvl);
    }
    if (adt_result->voice_state) {
        icsd_play_normal();
    }
    switch (adt_result->wat_result) {
    case 2:
        icsd_play_num2();
        break;
    case 3:
        icsd_play_num3();
        break;
    default:
        break;
    }
}
#endif /*ADT_DATA_DEBUG*/

#endif /*(defined TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN) && TCFG_AUDIO_ANC_ACOUSTIC_DETECTOR_EN*/

