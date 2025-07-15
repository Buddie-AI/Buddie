#include "audio_aec_online.h"
#include "app_online_cfg.h"
#include "config/config_target.h"
#include "system/includes.h"
#include "online_db_deal.h"
#include "timer.h"
#include "aec_user.h"
#include "audio_adc.h"

#if 1
extern void put_float(double fv);
#define AEC_ONLINE_LOG	y_printf
#define AEC_ONLINE_FLOG	put_float
#else
#define AEC_ONLINE_LOG(...)
#define AEC_ONLINE_FLOG(...)
#endif

#if TCFG_AEC_TOOL_ONLINE_ENABLE
extern int esco_enc_mic_gain_set(u8 gain);
extern int esco_enc_mic1_gain_set(u8 gain);
extern int esco_enc_mic2_gain_set(u8 gain);
extern int esco_enc_mic3_gain_set(u8 gain);
extern int esco_dec_dac_gain_set(u8 gain);
enum {
    AEC_UPDATE_CLOSE,
    AEC_UPDATE_INIT,
    AEC_UPDATE_ONLINE,
};

typedef struct {
    u8 update;
    u8 reserved;
    u16 timer;
#if TCFG_AUDIO_DUAL_MIC_ENABLE
#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
    AEC_DMS_CONFIG cfg;
#else/*TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE*/
    DMS_FLEXIBLE_CONFIG cfg;
#endif/*TCFG_AUDIO_DMS_SEL*/
#else/*SINGLE MIC*/
    AEC_CONFIG cfg;
    u8 agc_en;
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
} aec_update_t;
aec_update_t *aec_update = NULL;

static void aec_update_timer_deal(void *priv)
{
    AEC_ONLINE_LOG("aec_update_timer_deal");
#if TCFG_AUDIO_DUAL_MIC_ENABLE
    aec_dms_cfg_update(&aec_update->cfg);
#else
    aec_cfg_update(&aec_update->cfg);
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
    sys_timer_del(aec_update->timer);
    aec_update->timer = 0;
}

int aec_cfg_online_update(int root_cmd, void *packet)
{
    if ((root_cmd != 0x3000) && (root_cmd != 0x3100) &&
        (root_cmd != 0x3200) && (root_cmd != 0x3300) &&
        (root_cmd != 0x3400) && (root_cmd != 0x3500)) {
        return -1;
    }
    if (aec_update->update == AEC_UPDATE_CLOSE) {
        return 0;
    }
    u8 update = 1;
    aec_online_t *cfg = packet;
    int id = cfg->id;
    //AEC_ONLINE_LOG("AEC_TYPE[0x30xx:SMS 0x31xx:DMS]:%x",root_cmd);
    //AEC_ONLINE_LOG("aec_cfg_id:%x,val:%d", cfg->id, (int)cfg->val_int);
    //AEC_ONLINE_FLOG(cfg->val_float);
    if (id >= ENC_Process_MaxFreq) {
#if TCFG_AUDIO_DUAL_MIC_ENABLE
        AEC_ONLINE_LOG("ENC cfg update\n");
        switch (id) {
#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
        case ENC_Process_MaxFreq:
            AEC_ONLINE_LOG("ENC_Process_MaxFreq:%d\n", cfg->val_int);
            aec_update->cfg.enc_process_maxfreq = cfg->val_int;
            break;
        case ENC_Process_MinFreq:
            AEC_ONLINE_LOG("ENC_Process_MinFreq:%d\n", cfg->val_int);
            aec_update->cfg.enc_process_minfreq = cfg->val_int;
            break;
        case ENC_SIR_MaxFreq:
            AEC_ONLINE_LOG("ENC_SIR_MaxFreq:%d\n", cfg->val_int);
            aec_update->cfg.sir_maxfreq = cfg->val_int;
            break;
        case ENC_MIC_Distance:
            AEC_ONLINE_LOG("ENC_MIC_Distance:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.mic_distance = cfg->val_float;
            break;
        case ENC_Target_Signal_Degradation:
            AEC_ONLINE_LOG("ENC_Target_Signal_Degradation:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.target_signal_degradation = cfg->val_float;
            break;
        case ENC_AggressFactor:
            AEC_ONLINE_LOG("ENC_AggressFactor:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.enc_aggressfactor = cfg->val_float;
            break;
        case ENC_MinSuppress:
            AEC_ONLINE_LOG("ENC_MinSuppress:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.enc_minsuppress = cfg->val_float;
            break;
#else/*TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE*/
        case ENC_Suppress_Pre:
            AEC_ONLINE_LOG("ENC_Suppress_Pre:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.enc_suppress_pre = cfg->val_float;
            break;
        case ENC_Suppress_Post:
            AEC_ONLINE_LOG("ENC_Suppress_Post:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.enc_suppress_post = cfg->val_float;
            break;
        case ENC_MinSuppress:
            AEC_ONLINE_LOG("ENC_MinSuppress:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.enc_minsuppress = cfg->val_float;
            break;
        case ENC_Disconverge_Thr:
            AEC_ONLINE_LOG("ENC_Disconverge_Thr:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.enc_disconverge_erle_thr = cfg->val_float;
            break;

#endif/*TCFG_AUDIO_DMS_SEL*/
        default:
            AEC_ONLINE_LOG("enc param default:%x\n", id, cfg->val_int);
            AEC_ONLINE_FLOG(cfg->val_float);
            break;
        }
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
    } else if (id >= AGC_NDT_FADE_IN) {
        AEC_ONLINE_LOG("AGC cfg update\n");
        switch (id) {
        case AGC_NDT_FADE_IN:
            AEC_ONLINE_LOG("AGC_NDT_FADE_IN:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ndt_fade_in = cfg->val_float;
            break;
        case AGC_NDT_FADE_OUT:
            AEC_ONLINE_LOG("AGC_NDT_FADE_OUT:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ndt_fade_out = cfg->val_float;
            break;
        case AGC_DT_FADE_IN:
            AEC_ONLINE_LOG("AGC_DT_FADE_IN:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.dt_fade_in = cfg->val_float;
            break;
        case AGC_DT_FADE_OUT:
            AEC_ONLINE_LOG("AGC_DT_FADE_OUT:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.dt_fade_out = cfg->val_float;
            break;
        case AGC_NDT_MAX_GAIN:
            AEC_ONLINE_LOG("AGC_NDT_MAX_GAIN:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ndt_max_gain = cfg->val_float;
            break;
        case AGC_NDT_MIN_GAIN:
            AEC_ONLINE_LOG("AGC_NDT_MIN_GAIN:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ndt_min_gain = cfg->val_float;
            break;
        case AGC_NDT_SPEECH_THR:
            AEC_ONLINE_LOG("AGC_NDT_SPEECH_THR:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ndt_speech_thr = cfg->val_float;
            break;
        case AGC_DT_MAX_GAIN:
            AEC_ONLINE_LOG("AGC_DT_MAX_GAIN:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.dt_max_gain = cfg->val_float;
            break;
        case AGC_DT_MIN_GAIN:
            AEC_ONLINE_LOG("AGC_DT_MIN_GAIN:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.dt_min_gain = cfg->val_float;
            break;
        case AGC_DT_SPEECH_THR:
            AEC_ONLINE_LOG("AGC_DT_SPEECH_THR:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.dt_speech_thr = cfg->val_float;
            break;
        case AGC_ECHO_PRESENT_THR:
            AEC_ONLINE_LOG("AGC_ECHO_PRESENT_THR:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.echo_present_thr = cfg->val_float;
            break;
        }
    } else if (id >= ANS_AggressFactor) {
        AEC_ONLINE_LOG("ANS cfg update\n");
        switch (id) {
#if TCFG_AUDIO_DUAL_MIC_ENABLE
        case ANS_AggressFactor:
            AEC_ONLINE_LOG("ANS_AggressFactor:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.aggressfactor = cfg->val_float;
            break;
        case ANS_MinSuppress:
            AEC_ONLINE_LOG("ANS_MinSuppress:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.minsuppress = cfg->val_float;
            break;
        case ANS_MicNoiseLevel:
            AEC_ONLINE_LOG("ANS_MicNoiseLevel:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.init_noise_lvl = cfg->val_float;
            break;
#else
        case ANS_AGGRESS:
            AEC_ONLINE_LOG("ANS_AGGRESS:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ans_aggress = cfg->val_float;
            break;
        case ANS_SUPPRESS:
            AEC_ONLINE_LOG("ANS_SUPPRESS:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ans_suppress = cfg->val_float;
            break;
        case DNS_GainFloor:
            AEC_ONLINE_LOG("DNS_GainFloor:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ans_suppress = cfg->val_float;
            break;
        case DNS_OverDrive:
            AEC_ONLINE_LOG("DNS_OverDrive:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.ans_aggress = cfg->val_float;
            break;
#endif
        }
    } else if (id >= NLP_ProcessMaxFreq) {
        AEC_ONLINE_LOG("NLP cfg update\n");
        switch (id) {
#if TCFG_AUDIO_DUAL_MIC_ENABLE
        case NLP_ProcessMaxFreq:
            AEC_ONLINE_LOG("NLP_ProcessMaxFreq:%d\n", cfg->val_int);
            aec_update->cfg.nlp_process_maxfrequency = cfg->val_int;
            break;
        case NLP_ProcessMinFreq:
            AEC_ONLINE_LOG("NLP_ProcessMinFreq:%d\n", cfg->val_int);
            aec_update->cfg.nlp_process_minfrequency = cfg->val_int;
            break;
        case NLP_OverDrive:
            AEC_ONLINE_LOG("NLP_OverDrive:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.overdrive = cfg->val_float;
            break;
#else/*SINGLE MIC*/
        case NLP_AGGRESS_FACTOR:
            AEC_ONLINE_LOG("NLP_AGGRESS_FACTOR:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.es_aggress_factor = cfg->val_float;
            break;
        case NLP_MIN_SUPPRESS:
            AEC_ONLINE_LOG("NLP_MIN_SUPPRESS:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.es_min_suppress = cfg->val_float;
            break;
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
        }
    } else if (id >= AEC_ProcessMaxFreq) {
        switch (id) {
#if TCFG_AUDIO_DUAL_MIC_ENABLE
        case AEC_ProcessMaxFreq:
            AEC_ONLINE_LOG("AEC_ProcessMaxFreq:%d\n", cfg->val_int);
            aec_update->cfg.aec_process_maxfrequency = cfg->val_int;
            break;
        case AEC_ProcessMinFreq:
            AEC_ONLINE_LOG("AEC_ProcessMinFreq:%d\n", cfg->val_int);
            aec_update->cfg.aec_process_minfrequency = cfg->val_int;
            break;
        case AEC_AF_Lenght:
            AEC_ONLINE_LOG("AEC_AF_Lenght:%d\n", cfg->val_int);
            aec_update->cfg.af_length = cfg->val_int;
            break;
#else/*SINGLE MIC*/
        case AEC_DT_AGGRESS:
            AEC_ONLINE_LOG("AEC_DT_AGGRESS:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.aec_dt_aggress = cfg->val_float;
        case AEC_REFENGTHR:
            AEC_ONLINE_LOG("AEC_REFENGTHR:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.aec_refengthr = cfg->val_float;
            break;
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
        }
    } else {
        AEC_ONLINE_LOG("General cfg update\n");
        switch (id) {
        case GENERAL_DAC:
            AEC_ONLINE_LOG("DAC_Gain:%d\n", cfg->val_int);
            aec_update->cfg.dac_again = cfg->val_int;
            esco_dec_dac_gain_set(cfg->val_int);
            update = 0;
            break;
        case GENERAL_MIC:
            AEC_ONLINE_LOG("Mic_Gain:%d\n", cfg->val_int);
            aec_update->cfg.mic_again = cfg->val_int;
#if (TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_0)
            esco_enc_mic_gain_set(cfg->val_int);
#endif/*TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_0*/
#if (TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_1)
            esco_enc_mic1_gain_set(cfg->val_int);
#endif/*(TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_1)*/
#if (TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_2)
            esco_enc_mic2_gain_set(cfg->val_int);
#endif/*(TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_2)*/
#if (TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_3)
            esco_enc_mic3_gain_set(cfg->val_int);
#endif/*(TCFG_AUDIO_ADC_MIC_CHA & AUDIO_ADC_MIC_3)*/
            update = 0;
            break;
#if (TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE)
        case GENERAL_MIC_1:
            AEC_ONLINE_LOG("Mic1_Gain:%d\n", cfg->val_int);
            aec_update->cfg.mic1_again = cfg->val_int;
            esco_enc_mic1_gain_set(cfg->val_int);
            update = 0;
            break;
#endif/*TCFG_AUDIO_DMS_SEL*/
        case GENERAL_ModuleEnable:
#if TCFG_AUDIO_DUAL_MIC_ENABLE
            AEC_ONLINE_LOG("DMS EnableBit:%x", cfg->val_int);
            aec_update->cfg.enable_module = cfg->val_int;
#else
            AEC_ONLINE_LOG("SMS EnableBit:%x", cfg->val_int);
            aec_update->cfg.aec_mode = cfg->val_int;

            /*使用新配置工具后，兼容app在线调试*/
            if (aec_update->cfg.aec_mode == 2) {
                aec_update->cfg.aec_mode &= ~AEC_MODE_ADVANCE;
                aec_update->cfg.aec_mode |= AEC_MODE_ADVANCE;
            } else if (aec_update->cfg.aec_mode == 1) {
                aec_update->cfg.aec_mode &= ~AEC_MODE_ADVANCE;
                aec_update->cfg.aec_mode |= AEC_MODE_REDUCE;
            } else {
                aec_update->cfg.aec_mode &= ~AEC_MODE_ADVANCE;
            }
            if (aec_update->cfg.aec_mode) {
                aec_update->cfg.aec_mode |= AGC_EN;
            }
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
            break;
        case GENERAL_PC_ModuleEnable:
#if TCFG_AUDIO_DUAL_MIC_ENABLE
            AEC_ONLINE_LOG("DMS EnableBit:%x", cfg->val_int);
            aec_update->cfg.enable_module = cfg->val_int;
#else
            AEC_ONLINE_LOG("SMS EnableBit:%x", cfg->val_int);
            aec_update->cfg.aec_mode = cfg->val_int;
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
            break;
        case GENERAL_UL_EQ:
            AEC_ONLINE_LOG("UL_EQ_EN:%d\n", cfg->val_int);
            aec_update->cfg.ul_eq_en = cfg->val_int;
            break;
#if TCFG_AUDIO_DUAL_MIC_ENABLE
#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
        case GENERAL_Global_MinSuppress:
            AEC_ONLINE_LOG("GlobalMinSuppress:");
            AEC_ONLINE_FLOG(cfg->val_float);
            aec_update->cfg.global_minsuppress = cfg->val_float;
            break;
#endif/*TCFG_AUDIO_DMS_SEL*/
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
        }
    }
    aec_update->update = AEC_UPDATE_ONLINE;
    if (update && audio_aec_status()) {
        if (aec_update->timer) {
            sys_timer_modify(aec_update->timer, 500);
        } else {
            aec_update->timer = sys_timer_add(NULL, aec_update_timer_deal, 500);
        }
    }
    return 0;
}

int aec_cfg_online_init()
{
    int ret = 0;
    aec_update = zalloc(sizeof(aec_update_t));
#if TCFG_AUDIO_DUAL_MIC_ENABLE
#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    ret = syscfg_read(CFG_DMS_ID, &aec_update->cfg, sizeof(AEC_DMS_CONFIG));
#else/*CVP_DNS_MODE*/
    ret = syscfg_read(CFG_DMS_DNS_ID, &aec_update->cfg, sizeof(AEC_DMS_CONFIG));
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
    if (ret == sizeof(AEC_DMS_CONFIG)) {
        aec_update->update = AEC_UPDATE_INIT;
    }
#else/*TCFG_AUDIO_DMS_SEL == DMS_FLEXIBLE*/
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    ret = syscfg_read(CFG_DMS_FLEXIBLE_ID, &aec_update->cfg, sizeof(DMS_FLEXIBLE_CONFIG));
#else/*CVP_DNS_MODE*/
    ret = syscfg_read(CFG_DMS_DNS_FLEXIBLE_ID, &aec_update->cfg, sizeof(DMS_FLEXIBLE_CONFIG));
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
    if (ret == sizeof(DMS_FLEXIBLE_CONFIG)) {
        aec_update->update = AEC_UPDATE_INIT;
    }
#endif/*TCFG_AUDIO_DMS_SEL*/
#else/*SINGLE MIC*/
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
    ret = syscfg_read(CFG_AEC_ID, &aec_update->cfg, sizeof(AEC_CONFIG));
#else/*CVP_DNS_MODE*/
    ret = syscfg_read(CFG_SMS_DNS_ID, &aec_update->cfg, sizeof(AEC_CONFIG));
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
    if (ret == sizeof(AEC_CONFIG)) {
        aec_update->update = AEC_UPDATE_INIT;
    }
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
    return 0;
}

/*return 1：有在线更新数据*/
/*return 0：没有在线更新数据*/
int aec_cfg_online_update_fill(void *cfg, u16 len)
{
    if (aec_update && aec_update->update) {
        memcpy(cfg, &aec_update->cfg, len);

        return len;
    }
    return 0;
}

int aec_cfg_online_exit()
{
    if (aec_update) {
        free(aec_update);
        aec_update = NULL;
    }
    return 0;
}

int get_aec_config(u8 *buf, int version)
{
#if 1/*每次获取update的配置*/
    if (aec_update) {
        printf("cfg_size:%d\n", sizeof(aec_update->cfg));
        memcpy(buf, &aec_update->cfg, sizeof(aec_update->cfg));

#if (TCFG_AUDIO_DUAL_MIC_ENABLE == 0)
        AEC_CONFIG *cfg = (AEC_CONFIG *)buf;
        /*单麦aec_mode: app(旧)和pc端(兼容) */
        if (version == 0x01) {
            printf("APP version %d", version);
            if (aec_update->cfg.aec_mode & AGC_EN) {
                aec_update->agc_en = 1;
            } else {
                aec_update->agc_en = 0;
            }
            if ((aec_update->cfg.aec_mode & AEC_MODE_ADVANCE) == AEC_MODE_ADVANCE) {
                cfg->aec_mode = 2;
            } else if ((aec_update->cfg.aec_mode & AEC_MODE_ADVANCE) == AEC_MODE_REDUCE) {
                cfg->aec_mode = 1;
            } else {
                cfg->aec_mode = 0;
            }
        } else if (version == 0x02) {
            printf("PC version %d", version);
        }
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/

        return sizeof(aec_update->cfg);
    }
    return 0;
#else/*每次获取原始配置*/
#if TCFG_AUDIO_DUAL_MIC_ENABLE
    AEC_DMS_CONFIG cfg;
    int ret = syscfg_read(CFG_DMS_ID, &cfg, sizeof(AEC_DMS_CONFIG));
    if (ret == sizeof(AEC_DMS_CONFIG)) {
        memcpy(buf, &cfg, sizeof(AEC_DMS_CONFIG));
        return sizeof(AEC_DMS_CONFIG);
    } else {
        return 0;
    }
#else
    AEC_CONFIG cfg;
    int ret = syscfg_read(CFG_AEC_ID, &cfg, sizeof(AEC_CONFIG));
    if (ret == sizeof(AEC_CONFIG)) {
        memcpy(buf, &cfg, sizeof(AEC_CONFIG));
        return sizeof(AEC_CONFIG);
    } else {
        return 0;
    }
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/
#endif
}
#endif /*TCFG_AEC_TOOL_ONLINE_ENABLE*/

/*
 ***********************************************************************
 *					导出通话过程的数据
 *
 *
 ***********************************************************************
 */
#if (TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP)
enum {
    AEC_RECORD_COUNT = 0x200,
    AEC_RECORD_START,
    AEC_RECORD_STOP,
    ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH,
};

enum {
    AEC_ST_INIT,
    AEC_ST_START,
    AEC_ST_STOP,
};

#define AEC_RECORD_CH		2
#define AEC_RECORD_MTU		200
#define RECORD_CH0_LENGTH		644
#define RECORD_CH1_LENGTH		644
#define RECORD_CH2_LENGTH		644

typedef struct {
    u8 state;
    u8 ch;	/*export data ch num*/
    u16 send_timer;
    u8 packet[256];
} aec_record_t;
aec_record_t *aec_rec = NULL;

typedef struct {
    int cmd;
    int data;
} rec_cmd_t;

extern int audio_capture_start(void);
extern void audio_capture_stop(void);
static int aec_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    int res_data = 0;
    rec_cmd_t rec_cmd;
    int err = 0;
    u8 parse_seq = ext_data[1];
    //AEC_ONLINE_LOG("aec_spp_rx,seq:%d,size:%d\n", parse_seq, size);
    //put_buf(packet, size);
    memcpy(&rec_cmd, packet, sizeof(rec_cmd_t));
    switch (rec_cmd.cmd) {
    case AEC_RECORD_COUNT:
        res_data = aec_rec->ch;
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4);
        AEC_ONLINE_LOG("query record_ch num:%d\n", res_data);
        break;
    case AEC_RECORD_START:
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 1); //该命令随便ack一个byte即可
        aec_rec->state = AEC_ST_START;
        audio_capture_start();
        AEC_ONLINE_LOG("record_start\n");
        break;
    case AEC_RECORD_STOP:
        AEC_ONLINE_LOG("record_stop\n");
        audio_capture_stop();
        aec_rec->state = AEC_ST_STOP;
        app_online_db_ack(parse_seq, (u8 *)&res_data, 1); //该命令随便ack一个byte即可
        break;
    case ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH:
        if (rec_cmd.data == 0) {
            res_data = RECORD_CH0_LENGTH;
        } else if (rec_cmd.data == 1) {
            res_data = RECORD_CH1_LENGTH;
        } else {
            res_data = RECORD_CH2_LENGTH;
        }
        AEC_ONLINE_LOG("query record ch%d packet length:%d\n", rec_cmd.data, res_data);
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4); //回复对应的通道数据长度
        break;
    }
    return 0;
}

static void aec_export_timer(void *priv)
{
    int err = 0;
    if (aec_rec->state) {
        putchar('.');
#if 1
        static u8 data = 0;
        memset(aec_rec->packet, data, 128);
        data++;
#endif
        err = app_online_db_send(DB_PKT_TYPE_DAT_CH0, aec_rec->packet, AEC_RECORD_MTU);
        if (err) {
            printf("w0_err:%d", err);
        }
        err = app_online_db_send(DB_PKT_TYPE_DAT_CH1, aec_rec->packet, AEC_RECORD_MTU);
        if (err) {
            printf("w1_err:%d", err);
        }
    } else {
        //putchar('S');
    }
}

int spp_data_export(u8 ch, u8 *buf, u16 len)
{
    u8 data_ch;
    if (aec_rec->state == AEC_ST_START) {
        putchar('.');
        if (ch == 0) {
            data_ch = DB_PKT_TYPE_DAT_CH0;
        } else if (ch == 1) {
            data_ch = DB_PKT_TYPE_DAT_CH1;
        } else {
            data_ch = DB_PKT_TYPE_DAT_CH2;
        }
        int err = app_online_db_send_more(data_ch, buf, len);
        if (err) {
            r_printf("tx_err:%d", err);
            //return -1;
        }
        return len;
    } else {
        //putchar('x');
        return 0;
    }
}

int aec_data_export_init(u8 ch)
{
    aec_rec = zalloc(sizeof(aec_record_t));
    //aec_rec->send_timer = sys_timer_add(NULL, aec_export_timer, 16);
    aec_rec->ch = ch;
    app_online_db_register_handle(DB_PKT_TYPE_EXPORT, aec_online_parse);
    //app_online_db_register_handle(DB_PKT_TYPE_MIC_DUT, mic_dut_online_parse);
    return 0;
}

int aec_data_export_exit()
{
    if (aec_rec) {
        if (aec_rec->send_timer) {
            sys_timer_del(aec_rec->send_timer);
            aec_rec->send_timer = 0;
        }
        free(aec_rec);
        aec_rec = NULL;
    }
    return 0;
}
#endif /*TCFG_AUDIO_DATA_EXPORT_ENABLE*/

#if 0
static u8 aec_online_idle_query(void)
{
    //return ((aec_rec == NULL) ? 1 : 0);
    return 0;
}

REGISTER_LP_TARGET(aec_online_lp_target) = {
    .name = "aec_online",
    .is_idle = aec_online_idle_query,
};
#endif
