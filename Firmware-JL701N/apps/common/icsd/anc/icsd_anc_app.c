#include "icsd_anc_app.h"
#include "classic/tws_api.h"
#include "board_config.h"
#include "tone_player.h"
#include "adv_adaptive_noise_reduction.h"
#include "audio_anc.h"
#include "asm/audio_src.h"
#include "icsd_anc_user.h"

#if 1
#define icsd_anc_log	printf
#else
#define icsd_anc_log(...)
#endif/*log_en*/

#if TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN

#if (ICSD_ANC_MODE == TWS_TONE_BYPASS_MODE) || \
	(ICSD_ANC_MODE == TWS_TONE_MODE) || \
	(ICSD_ANC_MODE == TWS_BYPASS_MODE)
const u8 TWS_MODE = 1;
#else
const u8 TWS_MODE = 0;
#endif/**/
#if (ICSD_ANC_MODE == HEADSET_TONE_BYPASS_MODE) || \
	(ICSD_ANC_MODE == HEADSET_BYPASS_MODE) || \
	(ICSD_ANC_MODE == HEADSET_TONES_MODE)
const u8 HEADSET_MODE = 1;
#else
const u8 HEADSET_MODE = 0;
#endif/**/

const u8 ICSD_ANC_DEBUG = ICSD_ANC_DEBUG_TYPE;
u32 train_time;
struct icsd_anc_board_param *ANC_BOARD_PARAM = NULL;
volatile struct icsd_anc_tool_data *TOOL_DATA = NULL;
volatile struct icsd_anc_backup *CFG_BACKUP = NULL;
volatile struct icsd_anc ICSD_ANC;
volatile u8 icsd_anc_contral = 0;
volatile u8 icsd_anc_function = 0;
int *user_train_buf = 0;
volatile int user_train_state = 0;
u16 icsd_time_out_hdl = 0;
u8 icsd_anc_combination_test;
u8 icsd_anc_train_test;
int icsd_anc_id = 0;
u8 icsd_anc_tws_balance_en = 0;


int (*anc_printf)(const char *format, ...);
int anc_printf_off(const char *format, ...)
{
    return 0;
}

#define TWS_FUNC_ID_SDANC_M2S    TWS_FUNC_ID('I', 'C', 'M', 'S')
REGISTER_TWS_FUNC_STUB(icsd_anc_m2s) = {
    .func_id = TWS_FUNC_ID_SDANC_M2S,
    .func    = icsd_anc_m2s_cb,
};

#define TWS_FUNC_ID_SDANC_S2M    TWS_FUNC_ID('I', 'C', 'S', 'M')
REGISTER_TWS_FUNC_STUB(icsd_anc_s2m) = {
    .func_id = TWS_FUNC_ID_SDANC_S2M,
    .func    = icsd_anc_s2m_cb,
};

#define TWS_FUNC_ID_SDANC_MSYNC  TWS_FUNC_ID('I', 'C', 'M', 'N')
REGISTER_TWS_FUNC_STUB(icsd_anc_msync) = {
    .func_id = TWS_FUNC_ID_SDANC_MSYNC,
    .func    = icsd_anc_msync_cb,
};

#define TWS_FUNC_ID_SDANC_SSYNC  TWS_FUNC_ID('I', 'C', 'S', 'N')
REGISTER_TWS_FUNC_STUB(icsd_anc_ssync) = {
    .func_id = TWS_FUNC_ID_SDANC_SSYNC,
    .func    = icsd_anc_ssync_cb,
};

void icsd_anc_tws_msync(u8 cmd)
{
    struct icsd_anc_tws_packet packet;
    icsd_anc_msync_packet(&packet, cmd);
    anc_printf("msync:%d %d\n", packet.len, packet.data[0]);
    int ret = tws_api_send_data_to_sibling(packet.data, packet.len, TWS_FUNC_ID_SDANC_MSYNC);
}

void icsd_anc_tws_ssync(u8 cmd)
{
    struct icsd_anc_tws_packet packet;
    icsd_anc_ssync_packet(&packet, cmd);
    anc_printf("ssync:%d %d\n", packet.len, packet.data[0]);
    int ret = tws_api_send_data_to_sibling(packet.data, packet.len, TWS_FUNC_ID_SDANC_SSYNC);
}

void icsd_anc_tws_m2s(u8 cmd)
{
    s8 data[16];
    icsd_anc_m2s_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 16, TWS_FUNC_ID_SDANC_M2S);
}

void icsd_anc_tws_s2m(u8 cmd)
{
    s8 data[16];
    icsd_anc_s2m_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 16, TWS_FUNC_ID_SDANC_S2M);
}

float anc_pow10(float n)
{
    float pow10n = exp_float((float)n / log10_float(exp_float((float)1.0)));
    return pow10n;
}

void icsd_anc_fgq_printf(float *ptr)
{
    icsd_anc_log("icsd_anc_fgq\n");
    for (int i = 0; i < ANC_VMDATA_FF_RECORD_SIZE; i++) {
        icsd_anc_log("%d\n", (int)((*ptr++) * 1000));
    }
}

void icsd_anc_aabb_printf(double *iir_ab)
{
    icsd_anc_log("icsd_anc_aabb\n");
    for (int i = 0; i < ANC_BOARD_PARAM->fb_yorder; i++) {
        icsd_anc_log("AB:%d %d %d %d %d\n", (int)(iir_ab[i * 5] * 100000000),
                     (int)(iir_ab[i * 5 + 1] * 100000000),
                     (int)(iir_ab[i * 5 + 2] * 100000000),
                     (int)(iir_ab[i * 5 + 3] * 100000000),
                     (int)(iir_ab[i * 5 + 4] * 100000000));
    }
    //put_buf(iir_ab, 8 * 40);
}

void icsd_anc_forced_exit()
{
    if (!(icsd_anc_contral & ICSD_ANC_FORCED_EXIT)) {
        icsd_printf("icsd_anc_forced_exit----------------------------\n");
        if (icsd_anc_contral & ICSD_ANC_INITED) {
            icsd_anc_contral |= ICSD_ANC_FORCED_EXIT;
            audio_anc_post_msg_icsd_anc_cmd(ANC_CMD_FORCED_EXIT);
        } else {
            icsd_anc_contral |= ICSD_ANC_FORCED_BEFORE_INIT;
        }
    }
}

void icsd_anc_train_timeout()
{
    icsd_anc_log("icsd_anc_train_timeout\n");
    user_train_state &=	~ANC_USER_TRAIN_RUN;
    user_train_state |=	ANC_TRAIN_TIMEOUT;
    audio_anc_post_msg_user_train_timeout();
}

u32 icsd_anc_get_role()
{
    return tws_api_get_role();
}

u32 icsd_anc_get_tws_state()
{
    if (icsd_anc_tws_balance_en) {
        return tws_api_get_tws_state();
    }
    return 0;
}

void icsd_anc_set_alogm(void *_param, int alogm)
{
    audio_anc_t *param = _param;
    param->gains.alogm = alogm;
}

void icsd_anc_set_micgain(void *_param, int lff_gain, int lfb_gain)
{
    //0 ~ 15
    audio_anc_t *param = _param;
    icsd_anc_log("mic gain set:%d %d-->%d %d\n", param->gains.l_ffmic_gain, param->gains.l_fbmic_gain, lff_gain, lfb_gain);
    param->gains.l_ffmic_gain = lff_gain;
    param->gains.l_fbmic_gain = lfb_gain;
    audio_anc_mic_management(param);
    audio_anc_mic_gain(param->mic_param, 0);
}

void icsd_anc_fft(int *in, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(1024, 10, 1, 0, 1);
    hw_fft_run(fft_config, in, out);
}
void icsd_anc_fft256(int *in, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(256, 8, 1, 0, 1);
    hw_fft_run(fft_config, in, out);
}
void icsd_anc_fade(void *_param)
{
    audio_anc_t *param = _param;
    audio_anc_fade2(param->anc_fade_gain, param->anc_fade_en, 1, 0);
}

void icsd_anc_long_fade(void *_param)
{
    audio_anc_t *param = _param;
    audio_anc_fade2(0, param->anc_fade_en, 4, 0);	//增益先淡出到0
    os_time_dly(10);//10
}

void icsd_anc_user_train_dma_on(u8 out_sel, u32 len, int *buf)
{
    user_train_state |=	ANC_TRAIN_DMA_ON;
    anc_dma_on(out_sel, buf, len);
}

void icsd_anc_dma_done()
{
    if (ICSD_ANC.adaptive_run_busy) {
        if (user_train_state & ANC_USER_TRAIN_DMA_EN) {
            if (user_train_state & ANC_USER_TRAIN_DMA_READY) {
                user_train_state &= ~ANC_USER_TRAIN_DMA_READY;
                anc_core_dma_stop();
                audio_anc_post_msg_user_train_run();
            } else {
                user_train_state |= ANC_USER_TRAIN_DMA_READY;
            }
        }
    }
}


void *anc_ram_addr = 0;
void sd_anc_exit()
{
    if (anc_ram_addr) {
        free(anc_ram_addr);
        anc_ram_addr = 0;
    }
    printf("sd_anc_exit");
    mem_stats();
}
void sd_anc_init()
{
    anc_printf = _anc_printf;
    printf("sd anc init\n");
    mem_stats();
    struct icsd_anc_libfmt libfmt;
    icsd_anc_get_libfmt(&libfmt, SD_ANC_TWS);
    if (anc_ram_addr == 0) {
        anc_ram_addr = zalloc(libfmt.lib_alloc_size);
    }
    struct icsd_anc_infmt infmt;
    infmt.alloc_ptr = anc_ram_addr;
    icsd_anc_set_infmt(&infmt);
    printf("ram init:%d\n", libfmt.lib_alloc_size);
}

void icsd_anc_init(audio_anc_t *param, u8 mode, u8 seq, int tone_delay, u8 tws_balance_en)
{
    sd_anc_init();
    anc_printf = _anc_printf;
    icsd_anc_contral |= ICSD_ANC_INITED;
    if (icsd_anc_contral & ICSD_ANC_FORCED_BEFORE_INIT) {
        icsd_anc_contral &= ~ICSD_ANC_FORCED_BEFORE_INIT;
        icsd_anc_forced_exit();
    }
    if (icsd_anc_contral & ICSD_ANC_FORCED_EXIT) {
        icsd_printf("icsd_anc_init FORCED EXIT\n");
        return;
    }
    icsd_anc_id = seq;
    icsd_anc_tws_balance_en = tws_balance_en;
    icsd_anc_log("sd anc init:%d, seq %d, tone_delay %d, tws_balance %d ======================================\n", seq, tone_delay, icsd_anc_id, tws_balance_en);
    mem_stats();
    user_train_state = 0;
    icsd_anc_function = 0;
    if (mode == 1) {
        user_train_state |= ANC_USER_TRAIN_TOOL_DATA;
        icsd_anc_tool_data_init();
    }
    user_train_state |= ANC_TRAIN_TONE_PLAY;
    icsd_anc_combination_test = TEST_ANC_COMBINATION;
    icsd_anc_train_test = TEST_ANC_TRAIN;
#if ANC_USER_TRAIN_TONE_MODE
    user_train_state |=	ANC_USER_TRAIN_TONEMODE;
#endif
#if HEADSET_TONES_MODE_BYPASS_OFF
    user_train_state |=	ANC_TONESMODE_BYPASS_OFF;
#endif
#if ANC_ADAPTIVE_CMP_EN
    icsd_anc_function |= ANC_ADAPTIVE_CMP;
#endif
#if ANC_EARPHONE_CHECK_EN
    icsd_anc_function |= ANC_EARPHONE_CHECK;
#endif

    ICSD_ANC.param = param;
    ICSD_ANC.adaptive_run_busy = 1;
    ICSD_ANC.anc_fade_gain      = &param->anc_fade_gain;
    ICSD_ANC.gains_l_ffgain     = &param->gains.l_ffgain;
    ICSD_ANC.gains_l_fbgain     = &param->gains.l_fbgain;
    ICSD_ANC.gains_l_cmpgain    = &param->gains.l_cmpgain;
    ICSD_ANC.gains_r_ffgain     = &param->gains.r_ffgain;
    ICSD_ANC.gains_r_fbgain     = &param->gains.r_fbgain;
    ICSD_ANC.gains_r_cmpgain    = &param->gains.r_cmpgain;
    ICSD_ANC.lff_yorder         = &param->lff_yorder;
    ICSD_ANC.lfb_yorder         = &param->lfb_yorder;
    ICSD_ANC.lcmp_yorder        = &param->lcmp_yorder;
    ICSD_ANC.rff_yorder         = &param->rff_yorder;
    ICSD_ANC.rfb_yorder         = &param->rfb_yorder;
    ICSD_ANC.rcmp_yorder        = &param->rcmp_yorder;
    ICSD_ANC.lff_coeff          = &param->lff_coeff;
    ICSD_ANC.lfb_coeff      	= &param->lfb_coeff;
    ICSD_ANC.lcmp_coeff      	= &param->lcmp_coeff;
    ICSD_ANC.rff_coeff          = &param->rff_coeff;
    ICSD_ANC.rfb_coeff      	= &param->rfb_coeff;
    ICSD_ANC.rcmp_coeff      	= &param->rcmp_coeff;
    ICSD_ANC.gains_l_ffmic_gain = &param->gains.l_ffmic_gain;
    ICSD_ANC.gains_l_fbmic_gain = &param->gains.l_fbmic_gain;
    ICSD_ANC.gains_r_ffmic_gain = &param->gains.r_ffmic_gain;
    ICSD_ANC.gains_r_fbmic_gain = &param->gains.r_fbmic_gain;
    ICSD_ANC.gains_alogm        = (int *)(&param->gains.alogm);
    ICSD_ANC.ff_1st_dcc         = &param->gains.ff_1st_dcc;
    ICSD_ANC.fb_1st_dcc         = &param->gains.fb_1st_dcc;
    ICSD_ANC.ff_2nd_dcc         = &param->gains.ff_2nd_dcc;
    ICSD_ANC.fb_2nd_dcc         = &param->gains.fb_2nd_dcc;
    ICSD_ANC.gains_drc_en       = &param->gains.drc_en;

    ICSD_ANC.mode = ANC_USER_TRAIN_MODE;
    ICSD_ANC.train_index = 0;
    ICSD_ANC.adaptive_run_busy = 1;

    icsd_anc_board_config();
    user_train_state |=	ANC_USER_TRAIN_DMA_EN;
    TOOL_DATA->save_idx = 0xff;
    ANC_BOARD_PARAM->iir_mode = ANC_USER_TRAIN_MODE;
    ANC_BOARD_PARAM->tool_ffgain_sign = 1;
    ANC_BOARD_PARAM->tool_fbgain_sign = 1;
    if (param->gains.gain_sign & ANCL_FF_SIGN) {
        ANC_BOARD_PARAM->tool_ffgain_sign = -1;
    }
    if (param->gains.gain_sign & ANCL_FB_SIGN) {
        ANC_BOARD_PARAM->tool_fbgain_sign = -1;
    }

    //金机曲线功能==================================
#if 1
    icsd_anc_log("ref en:%d----------------------\n", param->gains.adaptive_ref_en);
    struct icsd_fb_ref *fb_parm = NULL;
    struct icsd_ff_ref *ff_parm = NULL;
    if (param->gains.adaptive_ref_en) {
        icsd_anc_log("get ref data\n");
        anc_adaptive_fb_ref_data_get((u8 **)(&fb_parm));
        anc_adaptive_ff_ref_data_get((u8 **)(&ff_parm));
        ANC_BOARD_PARAM->m_value_l = fb_parm->m_value;
        ANC_BOARD_PARAM->sen_l = fb_parm->sen;
        ANC_BOARD_PARAM->in_q_l = fb_parm->in_q;
        icsd_anc_log("fb_parm:%d %d %d\n", (int)(fb_parm->m_value * 1000), (int)(fb_parm->sen * 1000), (int)(fb_parm->in_q * 1000));
    }
    if (ff_parm) {
        icsd_anc_log("use gold curve==========================================================\n");
        ANC_BOARD_PARAM->gold_curve_en = param->gains.adaptive_ref_en;
        ANC_BOARD_PARAM->mse_tar = ff_parm->db;
    }
#endif
    //==============================================
    icsd_anc_config_inf();
    icsd_anc_lib_init();
    icsd_anc_set_alogm(param, ANC_USER_TRAIN_SR_SEL);
    ICSD_ANC.tone_delay = tone_delay;
    icsd_anc_mode_init(tone_delay);

}

void icsd_anc_init_cmd()
{
    icsd_anc_log("icsd_anc_init_cmd\n");
    extern void audio_anc_post_msg_user_train_init(u8 mode);
    audio_anc_post_msg_user_train_init(ANC_ON);
}


u32 icsd_slience_frames;
u16 icsd_sample_rate;
void icsd_anctone_dacon_handler()
{
    u32 slience_frames = icsd_slience_frames;
    u16 sample_rate = icsd_sample_rate;
    if (icsd_anc_contral & ICSD_ANC_FORCED_EXIT) {
        icsd_printf("icsd_anctone_dacon FORCED EXIT\n");
        return;
    }
    if (user_train_state & ANC_TRAIN_TONE_PLAY) {
        icsd_anc_board_param_init();
        ANC_BOARD_PARAM->mode = ICSD_ANC_MODE;
        icsd_anc_log("icsd_anctone_dacon 1:%d %d %d\n", (int)jiffies, slience_frames, sample_rate);
        user_train_state |= ANC_TONE_DAC_ON;
        user_train_state &= ~ANC_TRAIN_TONE_PLAY;
        u32 slience_ms = 0;
        if ((slience_frames != 0) && (sample_rate != 0)) {
            slience_ms = (slience_frames * 1000 / sample_rate) + 1;
            icsd_anc_log("icsd_anctone_dacon 2:%d %d %d\n", slience_frames, sample_rate, slience_ms);
        }
        ANC_BOARD_PARAM->tone_jiff = jiffies + (slience_ms / 10) + 1;
        ICSD_ANC.dac_on_jiff = jiffies;
        ICSD_ANC.dac_on_slience = slience_ms;
        if (user_train_state & ANC_TONE_ANC_READY) {
            icsd_anc_log("dac on set tonemode start timeout:%d %d\n", slience_ms, ANC_BOARD_PARAM->tone_jiff);
            switch (ANC_BOARD_PARAM->mode) {
            case HEADSET_TONES_MODE:
                sys_hi_timeout_add((void *)icsd_anc_id, icsd_anc_tonel_start, slience_ms + TONEL_DELAY);
                sys_hi_timeout_add((void *)icsd_anc_id, icsd_anc_toner_start, slience_ms + TONER_DELAY);
                sys_hi_timeout_add((void *)icsd_anc_id, icsd_anc_pzl_start, slience_ms + PZL_DELAY);
                sys_hi_timeout_add((void *)icsd_anc_id, icsd_anc_pzr_start, slience_ms + PZR_DELAY);
                break;
            default:
                sys_hi_timeout_add((void *)icsd_anc_id, icsd_anc_tonemode_start, slience_ms + ICSD_ANC.tone_delay);
                break;
            }
        } else {
            icsd_anc_log("dac on wait anc ready\n");
        }
    }
}

void icsd_anctone_dacon(u32 slience_frames, u16 sample_rate)
{
    icsd_slience_frames = slience_frames;
    icsd_sample_rate = sample_rate;
    audio_anc_post_msg_icsd_anc_cmd(ANC_CMD_TONE_DACON);
}

void icsd_anc_tone_play_start()
{
#if ANC_USER_TRAIN_TONE_MODE
    icsd_anc_log("icsd_anc_tone_play_start\n");
    user_train_state |= ANC_TRAIN_TONE_PLAY;
    icsd_anc_init_cmd();
    os_time_dly(20);
#endif
}

u8 icsd_anc_adaptive_busy_flag_get(void)
{
    return ICSD_ANC.adaptive_run_busy;
}

#if ANC_EAR_RECORD_EN
/* float vmdata_FL[EAR_RECORD_MAX][25]; */
/* float vmdata_FR[EAR_RECORD_MAX][25]; */
float (*vmdata_FL)[ANC_VMDATA_FF_RECORD_SIZE] = NULL;
float (*vmdata_FR)[ANC_VMDATA_FF_RECORD_SIZE] = NULL;
int *vmdata_num;

void icsd_anc_vmdata_init(float (*FL)[ANC_VMDATA_FF_RECORD_SIZE], float (*FR)[ANC_VMDATA_FF_RECORD_SIZE], int *num)
{
    //读取耳道记忆 VM数据地址
    vmdata_FL = FL;
    vmdata_FR = FR;
    vmdata_num = num;
    /* icsd_anc_log("vmdata_num %d\n", *vmdata_num); */
    /* put_buf((u8*)vmdata_FL, EAR_RECORD_MAX*ANC_VMDATA_FF_RECORD_SIZE*4); */
    /* put_buf((u8*)vmdata_FR, EAR_RECORD_MAX*ANC_VMDATA_FF_RECORD_SIZE*4); */
}

void icsd_anc_vmdata_num_reset()
{
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    *vmdata_num = 0;
    memset(vmdata_FL, 0, EAR_RECORD_MAX * 4 * ANC_VMDATA_FF_RECORD_SIZE);
}

u8 icsd_anc_get_vmdata_num()
{
    return *vmdata_num;
}

void icsd_anc_ear_record_printf()
{
    int i;
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    icsd_anc_log("vmdata_num:%d\n", *vmdata_num);
    for (i = 0; i < EAR_RECORD_MAX; i++) {
        icsd_anc_log("vmdata_FL[%d]\n", i);
        icsd_anc_fgq_printf(&vmdata_FL[i][0]);
    }
    for (i = 0; i < EAR_RECORD_MAX; i++) {
        icsd_anc_log("vmdata_FR[%d]\n", i);
        icsd_anc_fgq_printf(&vmdata_FR[i][0]);
    }
}

u8 icsd_anc_max_diff_idx()
{
    u8 max_diff_idx = 0;
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return max_diff_idx;
    }
    if (*vmdata_num > 0) {
        float diff;
        float max_diff = icsd_anc_vmdata_match(&vmdata_FL[0][0], vmdata_FL[0][0]);
        max_diff_idx = 0;
        for (int i = 1; i < *vmdata_num; i++) {
            diff = icsd_anc_vmdata_match(&vmdata_FL[i][0], vmdata_FL[i][0]);
            if (diff > max_diff) {
                max_diff = diff;
                max_diff_idx = i;
            }
        }
    }
    icsd_anc_log("max diff idx:%d\n", max_diff_idx);
    return max_diff_idx;
}

u8 icsd_anc_min_diff_idx()
{
    float diff;
    float min_diff;
    u8 min_diff_idx = 0xff;

    if ((!vmdata_FL) || (!vmdata_FR)) {
        return min_diff_idx;
    }
    if (*vmdata_num > 0) {
        min_diff = icsd_anc_vmdata_match(&vmdata_FL[0][0], vmdata_FL[0][0]);
        min_diff_idx = 0;
        icsd_anc_log("DIFF[0]:%d\n", (int)(min_diff * 1000));
        for (int i = 1; i < *vmdata_num; i++) {
            diff = icsd_anc_vmdata_match(&vmdata_FL[i][0], vmdata_FL[i][0]);
            if (diff < min_diff) {
                min_diff = diff;
                min_diff_idx = i;
            }
            icsd_anc_log("DIFF[%d]:%d %d %d\n", i, (int)(diff * 1000), min_diff_idx, (int)(min_diff * 1000));
        }
        diff = icsd_anc_default_match();
        icsd_anc_log("default diff:%d vmmin diff:%d\n", (int)(diff * 100), (int)(min_diff * 100));
        if (diff < min_diff) {
            icsd_anc_log("use default\n");
            min_diff_idx = 0xff;
        } else {
            icsd_anc_log("select vmdata_FL[%d]\n", min_diff_idx);
        }
    }
    return min_diff_idx;
}

u8 icsd_anc_get_save_idx()
{
    u8 save_idx = 0;
    if (*vmdata_num < EAR_RECORD_MAX) {
        save_idx = *vmdata_num;
    } else {
        save_idx = icsd_anc_max_diff_idx();
    }
    icsd_anc_log("icsd_anc_get_save_idx:%d\n", save_idx);
    return save_idx;
}

void icsd_anc_save_with_idx(u8 save_idx)
{
    anc_printf("save try:%d %d\n", save_idx, *vmdata_num);
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    if (save_idx > *vmdata_num) {
        return;
    }
    if ((save_idx >= 0) && (save_idx < EAR_RECORD_MAX)) {
        if (*vmdata_num < EAR_RECORD_MAX) {
            (*vmdata_num)++;
        }
        anc_printf("icsd_anc_save_with_idx:%d %d====================================================\n", save_idx, *vmdata_num);
        switch (ANC_BOARD_PARAM->mode) {
        case HEADSET_TONE_BYPASS_MODE:
        case HEADSET_TONES_MODE:
        case HEADSET_BYPASS_MODE:
            memcpy(&vmdata_FL[save_idx][0], TOOL_DATA->data_out5, 4 * ANC_VMDATA_FF_RECORD_SIZE);
            memcpy(&vmdata_FR[save_idx][0], TOOL_DATA->data_out10, 4 * ANC_VMDATA_FF_RECORD_SIZE);
            break;
        default:
            memcpy(&vmdata_FL[save_idx][0], TOOL_DATA->data_out5, 4 * ANC_VMDATA_FF_RECORD_SIZE);
            break;
        }
#if	TEST_EAR_RECORD_EN
        icsd_anc_log("EAR RECORD SAVE:%d\n", save_idx);
        icsd_anc_ear_record_printf();
#endif
    }
}

void icsd_anc_vmdata_by_idx(double *ff_ab, float *ff_gain, u8 idx)
{
    u8 num = icsd_anc_get_vmdata_num();
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    if ((num == 0) || (idx == 0xff) || ((num - 1) < idx)) {
        return;
    }
#if	TEST_EAR_RECORD_EN
    icsd_anc_log("EAR RECORD USED RECORD L\n");
    icsd_anc_fgq_printf(&vmdata_FL[idx][0]);
#endif
    ff_fgq_2_aabb(ff_ab, &vmdata_FL[idx][0]);
    *ff_gain = vmdata_FL[idx][0];
}

void icsd_anc_vmdatar_by_idx(double *ff_ab, float *ff_gain, u8 idx)
{
    u8 num = icsd_anc_get_vmdata_num();
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    if ((num == 0) || (idx == 0xff) || ((num - 1) < idx)) {
        return;
    }
#if	TEST_EAR_RECORD_EN
    icsd_anc_log("EAR RECORD USED RECORD R\n");
    icsd_anc_fgq_printf(&vmdata_FR[idx][0]);
#endif
    ff_fgq_2_aabb(ff_ab, &vmdata_FR[idx][0]);
    *ff_gain = vmdata_FR[idx][0];
}

u8 icsd_anc_tooldata_select_vmdata_headset(float *ff_fgq_l, float *ff_fgq_r)
{
    icsd_anc_log("headset train time vm select start:%d\n", (int)((jiffies - train_time) * 10));
    u8 min_idx = icsd_anc_min_diff_idx();
    extern void icsd_set_min_idx(u8 minidx);
    icsd_set_min_idx(min_idx);
    u8 num = icsd_anc_get_vmdata_num();
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return 0;
    }
    //icsd_anc_log("icsd_anc_tooldata_select_vmdata:%d %d\n", num, min_idx);
    if ((num == 0) || (min_idx == 0xff) || ((num - 1) < min_idx)) {
        icsd_anc_log("use default\n");
        return 0;
    }
    memcpy(ff_fgq_l, &vmdata_FL[min_idx][0], 4 * ANC_VMDATA_FF_RECORD_SIZE);
    memcpy(ff_fgq_r, &vmdata_FR[min_idx][0], 4 * ANC_VMDATA_FF_RECORD_SIZE);

#if	TEST_EAR_RECORD_EN
    icsd_anc_log("EAR RECORD TOOL_DATA RECORD\n");
    icsd_anc_fgq_printf(ff_fgq_l);
    icsd_anc_fgq_printf(ff_fgq_r);
#endif

    icsd_anc_log("train time vm select end:%d\n", (int)((jiffies - train_time) * 10));
    icsd_anc_log("headset use record:%d\n", min_idx);
    return 1;
}

u8 icsd_anc_tooldata_select_vmdata(float *ff_fgq)
{

    icsd_anc_log("train time vm select start:%d\n", (int)((jiffies - train_time) * 10));
    //u8 min_idx = icsd_anc_min_diff_idx();
    u8 min_idx = TOOL_DATA->use_idx;
    icsd_printf("use record idx:%d\n", TOOL_DATA->use_idx);
    extern void icsd_set_min_idx(u8 minidx);
    icsd_set_min_idx(min_idx);
    u8 num = icsd_anc_get_vmdata_num();
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return 0;
    }
    //icsd_anc_log("icsd_anc_tooldata_select_vmdata:%d %d\n", num, min_idx);
    if ((num == 0) || (min_idx == 0xff) || ((num - 1) < min_idx)) {
        icsd_anc_log("use default\n");
        return 0;
    }
    memcpy(ff_fgq, &vmdata_FL[min_idx][0], 4 * ANC_VMDATA_FF_RECORD_SIZE);

#if	TEST_EAR_RECORD_EN
    icsd_anc_log("EAR RECORD TOOL_DATA RECORD\n");
    icsd_anc_fgq_printf(ff_fgq);
#endif

    icsd_anc_log("train time vm select end:%d\n", (int)((jiffies - train_time) * 10));
    icsd_anc_log("use record:%d\n", min_idx);
    return 1;
}
#else
u8 icsd_anc_min_diff_idx()
{
    return 0xff;
}
u8 icsd_anc_get_save_idx()
{
    return 0xff;
}
u8 icsd_anc_get_vmdata_num()
{
    return 0;
}
void icsd_anc_vmdata_num_reset()
{}
void icsd_anc_save_with_idx(u8 save_idx)
{}
void icsd_anc_vmdata_by_idx(double *ff_ab, float *ff_gain, u8 idx)
{}
void icsd_anc_vmdatar_by_idx(double *ff_ab, float *ff_gain, u8 idx)
{}
void icsd_anc_ear_record_printf()
{}
u8 icsd_anc_tooldata_select_vmdata(float *ff_fgq)
{
    return 0;
}
u8 icsd_anc_tooldata_select_vmdata_headset(float *ff_fgq_l, float *ff_fgq_r)
{
    return 0;
}
#endif


#if ANC_SZ_OUT_EN

float icsd_anc_sz_data[TARLEN + TARLEN_L];
float icsd_anc_pz_data[TARLEN + TARLEN_L];
u8 sz_out_exist = 0;

void icsd_anc_sz_out(float *hz_in)
{
    printf("icsd_anc_sz_out\n");
    memcpy(icsd_anc_sz_data, hz_in, 4 * (TARLEN + TARLEN_L));
    sz_out_exist = 1;
    extern void icsd_aeq_demo();
    icsd_aeq_demo();
    icsd_printf("train time aeq end:%dms\n", (int)((jiffies - train_time) * 10));
}

void icsd_anc_pz_out(float *hz_in)
{
    printf("icsd_anc_pz_out\n");
    memcpy(icsd_anc_pz_data, hz_in, 4 * (TARLEN + TARLEN_L));
    sz_out_exist = 1;
}

#endif


#endif/*TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN*/





