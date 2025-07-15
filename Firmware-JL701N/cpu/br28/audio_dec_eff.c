#include "audio_dec_eff.h"

#include "debug.h"
#include "system/syscfg_id.h"
#if TCFG_AUDIO_CVP_DUT_ENABLE
#include "audio_cvp_dut.h"
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/
extern struct audio_dac_hdl dac_hdl;
extern const int const_surround_en;
void user_sat16(s32 *in, s16 *out, u32 npoint);
void a2dp_surround_set(u8 eff);
int audio_out_eq_get_filter_info(void *eq, int sr, struct audio_eq_filter_info *info);
int audio_out_eq_spec_set_info(struct audio_eq *eq, u8 idx, int freq, float gain);
void *dec_eq_drc_setup_new(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en);
void eq_cfg_default_init(EQ_CFG *eq_cfg);
void drc_default_init(EQ_CFG *eq_cfg, u8 mode);


/*----------------------------------------------------------------------------*/
/**@brief    环绕音效切换测试例子
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void surround_switch_test(void *p)
{
#if 0
    if (!p) {
        return;
    }
    static u8 cnt_type = 0;
    if (EFFECT_OFF == cnt_type) {
        //中途关开测试
        static u8 en = 0;
        en = !en;
        audio_surround_parm_update(p, cnt_type, (surround_update_parm *)en);
    } else {
        //音效切换测试
        audio_surround_parm_update(p, cnt_type, NULL);
    }
    printf("cnt_type 0x%x\n", cnt_type);
    if (++cnt_type > EFFECT_OFF) {
        cnt_type = EFFECT_3D_PANORAMA;
    }
#endif
}

void *audio_surround_setup(u8 channel, u8 eff)
{
#if AUDIO_SURROUND_CONFIG
    struct dec_sur *sur = zalloc(sizeof(struct dec_sur));
    u8 nch = EFFECT_CH_L;
    if (channel == AUDIO_CH_L) {
        nch = EFFECT_CH_L;
    } else if (channel == AUDIO_CH_R) {
        nch = EFFECT_CH_R;
    } else if (channel == AUDIO_CH_LR) {
        nch = 2;
    } else if (channel == AUDIO_CH_DUAL_L) {
        nch = EFFECT_CH2_L;
    } else if (channel == AUDIO_CH_DUAL_R) {
        nch = EFFECT_CH2_R;
    } else {
        if (sur) {
            free(sur);
        }
        log_e("surround ch_type err %d\n", channel);
        return NULL;
    }
    surround_open_parm parm = {0};
    parm.channel = nch;
    parm.surround_effect_type = EFFECT_3D_PANORAMA;//打开时默认使用3d全景音,使用者，根据需求修改
    sur->surround = audio_surround_open(&parm);
    sur->surround_eff = eff;
    audio_surround_voice(sur, sur->surround_eff);//还原按键触发的音效
    //sur_test = sys_timer_add(sur->surround, surround_switch_test, 10000);
    return sur;
#else
    return NULL;
#endif//AUDIO_SURROUND_CONFIG

}

void audio_surround_free(void *sur)
{
#if AUDIO_SURROUND_CONFIG
    struct dec_sur *surh = (struct dec_sur *)sur;
    if (!surh) {
        return;
    }

    if (surh->surround) {
        audio_surround_close(surh->surround);
        surh->surround = NULL;
    }
    free(surh);
#endif//AUDIO_SURROUND_CONFIG

}

void audio_surround_set_ch(void *sur, u8 channel)
{
#if AUDIO_SURROUND_CONFIG
    struct dec_sur *surh = (struct dec_sur *)sur;
    if (!surh) {
        return;
    }

    u8 nch = EFFECT_CH_L;
    if (channel == AUDIO_CH_L) {
        nch = EFFECT_CH_L;
    } else if (channel == AUDIO_CH_R) {
        nch = EFFECT_CH_R;
    } else if (channel == AUDIO_CH_LR) {
        nch = 2;
    } else if (channel == AUDIO_CH_DUAL_L) {
        nch = EFFECT_CH2_L;
    } else if (channel == AUDIO_CH_DUAL_R) {
        nch = EFFECT_CH2_R;
    } else {
        log_e("surround ch_type err %d\n", channel);
        return ;
    }

    if (surh->surround) {
        audio_surround_switch_nch(surh->surround, nch);
    }
#endif//AUDIO_SURROUND_CONFIG

}

/*
 *环绕音效开关控制
 * */
void audio_surround_voice(void *sur, u8 en)
{
#if AUDIO_SURROUND_CONFIG
    struct dec_sur *surh = (struct dec_sur *)sur;
    surround_hdl *surround = (surround_hdl *)surh->surround;
    if (surround) {
        if (en) {
            if (const_surround_en & BIT(2)) {
                surround_update_parm parm = {0};
                parm.surround_type = EFFECT_3D_LRDRIFT2;//音效类型
                parm.rotatestep    = 4;   //建议1~6,环绕速度
                parm.damping = 40;//混响音量（0~70）,空间感
                parm.feedback = 100;//干声音量(0~100),清晰度
                parm.roomsize = 128;//无效参数
                audio_surround_parm_update(surround, EFFECT_SUR2, &parm);
            } else {
                audio_surround_parm_update(surround, EFFECT_3D_ROTATES, NULL);
            }
        } else {
            surround_update_parm parm = {0};
            audio_surround_parm_update(surround, EFFECT_OFF2, &parm);//关音效
        }
    }
#endif /* AUDIO_SURROUND_CONFIG */
}




#if AUDIO_VBASS_CONFIG
/*----------------------------------------------------------------------------*/
/**@brief    虚拟低音参数更新例子
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/


#define AEID_MUSIC_VBASS 1

vbass_hdl *audio_vbass_open_demo(u32 vbass_name, u32 sample_rate, u8 ch_num)
{
    VirtualBassParam parm = {0};
    u8 tar = 0;

    struct virtual_bass_tool_set *param = get_vbass_parm();
    u8 bypass  = 0;
    parm.ratio =  param->parm.ratio;
    parm.boost =  param->parm.boost;
    parm.fc    =  param->parm.fc;
    parm.channel = ch_num;
    parm.SampleRate = sample_rate;
    parm.pcm_info.IndataBit = DATA_INT_16BIT;
    parm.pcm_info.OutdataBit = DATA_INT_16BIT;
    parm.pcm_info.IndataInc = (ch_num == 2) ? 2 : 1;
    parm.pcm_info.OutdataInc = (ch_num == 2) ? 2 : 1;
    parm.pcm_info.Qval = 15;
    //printf("vbass ratio %d, boost %d, fc %d, channel %d, SampleRate %d\n", parm.ratio, parm.boost, parm.fc,parm.channel, parm.SampleRate);
    vbass_hdl *vbass = audio_vbass_open(vbass_name, &parm);
    vbass->lowfreq_en = 1; //1：保留低频；0：不保留
    audio_vbass_bypass(vbass_name, bypass);
    /* clock_add(DEC_VBASS_CLK); */
    return vbass;
}


void audio_vbass_close_demo(vbass_hdl *vbass)
{
    if (vbass) {
        audio_vbass_close(vbass);
        vbass = NULL;
    }
    /* clock_remove(DEC_VBASS_CLK); */
}



void audio_vbass_update_demo(u32 vbass_name, VirtualBassUdateParam *parm, u32 bypass)
{
    audio_vbass_parm_update(vbass_name, parm);
    audio_vbass_bypass(vbass_name, bypass);
}


void vbass_udate_parm_test(void *p)
{
#if 0
    //虚拟低音增益调节例子
    vbass_parm.ratio = 56;//比率(0~100),越大，低音强度越大
    vbass_parm.boost = 1;//(低音增强0, 1)
    vbass_parm.fc    = 100;//截止频率(30~300Hz)
    audio_vbass_parm_update(AEID_MUSIC_VBASS, &vbass_parm);
    audio_vbass_bypass(AEID_MUSIC_VBASS, 0);
#endif
#if 0
    //开关虚拟低音例子
    static u8 bypass = 0;
    /* bypass = 1;//关 */
    /* bypass = 0;//开 */
    bypass = !bypass;
    audio_vbass_bypass(AEID_MUSIC_VBASS, bypass);
#endif
}

vbass_hdl *audio_vbass_setup(u32 sample_rate, u8 channel)
{
    return audio_vbass_open_demo(AEID_MUSIC_VBASS, sample_rate, channel);
}

void audio_vbass_free(vbass_hdl *vbass)
{
    audio_vbass_close_demo(vbass);
}
#endif//AUDIO_VBASS_CONFIG



void eq_32bit_out(struct dec_eq_drc *eff)
{
    if (!config_eq_lite_en) {
        int wlen = 0;
        int point_len = 2;
#if defined(TCFG_AUDIO_DAC_24BIT_MODE) && TCFG_AUDIO_DAC_24BIT_MODE
        point_len = 4;
        s32 *out_tmp = eff->eq_out_buf;
#else
        s16 *out_tmp = eff->eq_out_buf;
#endif
        if (eff->priv && eff->out_cb) {
            wlen = eff->out_cb(eff->priv, &out_tmp[eff->eq_out_points], (eff->eq_out_total - eff->eq_out_points) * point_len);
        }
        eff->eq_out_points += wlen / point_len;
    }
}

static int eq_output(void *priv, void *buf, u32 len)
{
    if (!config_eq_lite_en) {
        int wlen = 0;
        int rlen = len;
        s16 *data = (s16 *)buf;
        struct dec_eq_drc *eff = priv;
        if (!eff->async) {
            return rlen;
        }

        if (eff->drc && eff->async) {
            if (eff->eq_out_buf && (eff->eq_out_points < eff->eq_out_total)) {
                eq_32bit_out(eff);
                if (eff->eq_out_points < eff->eq_out_total) {
                    return 0;
                }
            }

            audio_drc_run(eff->drc, data, len);

#if defined(TCFG_AUDIO_DAC_24BIT_MODE) && TCFG_AUDIO_DAC_24BIT_MODE
            if ((!eff->eq_out_buf) || (eff->eq_out_buf_len < len)) {
                if (eff->eq_out_buf) {
                    free(eff->eq_out_buf);
                }
                eff->eq_out_buf_len = len;
                eff->eq_out_buf = malloc(eff->eq_out_buf_len);
                ASSERT(eff->eq_out_buf);
            }
            memcpy(eff->eq_out_buf, data, len);
            eff->eq_out_points = 0;
            eff->eq_out_total = len / 4;
#else
            if ((!eff->eq_out_buf) || (eff->eq_out_buf_len < len / 2)) {
                if (eff->eq_out_buf) {
                    free(eff->eq_out_buf);
                }
                eff->eq_out_buf_len = len / 2;
                eff->eq_out_buf = malloc(eff->eq_out_buf_len);
                ASSERT(eff->eq_out_buf);
            }
            user_sat16((s32 *)data, (s16 *)eff->eq_out_buf, len / 4);
            eff->eq_out_points = 0;
            eff->eq_out_total = len / 4;
#endif

            eq_32bit_out(eff);
            return len;
        }

        int out_len = 0;
        if (eff->priv && eff->out_cb) {
            out_len = eff->out_cb(eff->priv, data, len);
        }
        return out_len;
    } else {
        return len;
    }
}



int wdrc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    static struct drc_ch wdrc_p = {0};
    struct threshold_group threshold[5] = {{-100 + 90.3f, -91.9f + 90.3f}, {-87 + 90.3f, -72.5f + 90.3f}, {-58.6f + 90.3f, -60.3f + 90.3f}, {-43.5f + 90.3f, -29.7f + 90.3f}, {0 + 90.3f, -10.9f + 90.3f}};
    /* struct threshold_group threshold[3] = {{0x42166666, 0x0}, {0x42580000, 0x42a40000},{0x42f00000, 0x42c50000}}; */

    wdrc_p.nband = 1;
    wdrc_p.type = 3;//wdrc

    //left
    int i = 0;
    wdrc_p._p.wdrc[i].attacktime = 1;
    wdrc_p._p.wdrc[i].releasetime = 500;
    memcpy(wdrc_p._p.wdrc[i].threshold, threshold, sizeof(threshold));
    wdrc_p._p.wdrc[i].threshold_num = ARRAY_SIZE(threshold);
    wdrc_p._p.wdrc[i].rms_time = 25;
    wdrc_p._p.wdrc[i].algorithm = 0;
    wdrc_p._p.wdrc[i].mode = 1;

    info->R_pch = info->pch = &wdrc_p;
    return 0;
}

u8 left_or_right_earphone()
{
#if 0//TCFG_USER_TWS_ENABLE
    if (config_wdrc_en) {
        u8 channel = 0;
        int state = 0;
        u8 dac_connect_mode;
        state = tws_api_get_tws_state();
        if (state & TWS_STA_SIBLING_CONNECTED) {
            dac_connect_mode = audio_dac_get_channel(&dac_hdl);
            if (dac_connect_mode == DAC_OUTPUT_LR) {
                if (tws_api_get_local_channel() == 'L') {
                    channel |= LL_wdrc;
                } else {
                    channel |= RR_wdrc;
                }
            } else {
                if (tws_api_get_local_channel() == 'L') {
                    channel |= L_wdrc;
                } else {
                    channel |= R_wdrc;
                }
            }
        }
        return channel;
    }
#endif
    return 0;
}

void *dec_eq_drc_setup(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en)
{
#if TCFG_EQ_ENABLE
    if (config_eq_lite_en) { //仅支持同步方式eq处理
        async = 0;//
    }
#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
    async = 0;
#endif
#if defined(AUDIO_GAME_EFFECT_CONFIG) && AUDIO_GAME_EFFECT_CONFIG
    if (drc_en & BIT(1)) { //game eff drc
        return dec_eq_drc_setup_new(priv, eq_output_cb, sample_rate, channel, async, drc_en);
    }
#endif/* AUDIO_GAME_EFFECT_CONFIG */

    struct dec_eq_drc *eff = zalloc(sizeof(struct dec_eq_drc));
    struct audio_eq_param eq_param = {0};
    eff->priv = priv;
    eff->out_cb = eq_output_cb;

    eq_param.channels = channel;
    eq_param.online_en = 1;
    eq_param.mode_en = 1;
    eq_param.remain_en = 1;
    eq_param.no_wait = async;
    if (drc_en) {
        eq_param.out_32bit = 1;
    }
    eq_param.max_nsection = EQ_SECTION_MAX;
    eq_param.cb = eq_get_filter_info;
    eq_param.eq_name = song_eq_mode;
    eq_param.sr = sample_rate;
    eq_param.priv = eff;
    eq_param.output = eq_output;
    eff->eq = audio_dec_eq_open(&eq_param);
    eff->async = async;
#if TCFG_DRC_ENABLE
    if (drc_en) {
        struct audio_drc_param drc_param = {0};
        drc_param.sr = sample_rate;
        drc_param.channels = channel | left_or_right_earphone();
        drc_param.online_en = 1;
        drc_param.remain_en = 1;
        drc_param.out_32bit = 1;
        drc_param.cb = drc_get_filter_info;
        drc_param.drc_name = song_eq_mode;
        eff->drc = audio_dec_drc_open(&drc_param);
    }
#endif//TCFG_DRC_ENABLE

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
    eff->spk_eq	= spk_eq_open(sample_rate, channel, drc_en ? 1 : 0);
#endif/*AUDIO_SPK_EQ_CONFIG*/


    return eff;
#else
    return NULL;
#endif//TCFG_EQ_ENABLE

}

void dec_eq_drc_free(void *eff)
{
#if TCFG_EQ_ENABLE
    struct dec_eq_drc *eff_hdl = (struct dec_eq_drc *)eff;
    if (!eff_hdl) {
        return;
    }
    u8 tmp = 0;
    if (eff_hdl->drc_bef_eq) {
        tmp = 1;
    }
    if (eff_hdl->drc_prev) {
        audio_dec_drc_close(eff_hdl->drc_prev);
        eff_hdl->drc_prev = NULL;
    }

    if (eff_hdl->eq) {
        audio_dec_eq_close(eff_hdl->eq);
        eff_hdl->eq = NULL;
    }

    if (eff_hdl->drc) {
        audio_dec_drc_close(eff_hdl->drc);
        eff_hdl->drc = NULL;
    }

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
    if (eff_hdl->spk_eq) {
        spk_eq_close(eff_hdl->spk_eq);
        eff_hdl->spk_eq = NULL;
    }
#endif

    if (eff_hdl->eq_out_buf) {
        free(eff_hdl->eq_out_buf);
        eff_hdl->eq_out_buf = NULL;
    }

#if AUDIO_EQ_FADE_EN
    if (eff_hdl->fade.tmr) {
        sys_hi_timer_del(eff_hdl->fade.tmr);
        eff_hdl->fade.tmr = 0;
    }
#endif
    free(eff_hdl);
    if (tmp) { //还原非游戏音效eq drc 效果
        EQ_CFG *eq_cfg = get_eq_cfg_hdl();
        eq_cfg->eq_type = EQ_TYPE_MODE_TAB;
        eq_cfg_default_init(get_eq_cfg_hdl());
        drc_default_init(get_eq_cfg_hdl(), song_eq_mode);
        if (!eq_file_get_cfg(get_eq_cfg_hdl(), (u8 *)SDFILE_RES_ROOT_PATH"eq_cfg_hw.bin")) {
            eq_cfg->eq_type = EQ_TYPE_FILE;
        }
    }
#endif//TCFG_EQ_ENABLE

}
struct drc_ch esco_drc_p = {0};
int esco_drc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    float th = -0.5f;//db -60db~0db,限幅器阈值
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768);
    esco_drc_p.nband = 1;
    esco_drc_p.type = 1;
    esco_drc_p._p.limiter[0].attacktime = 5;
    esco_drc_p._p.limiter[0].releasetime = 500;
    esco_drc_p._p.limiter[0].threshold[0] = threshold;
    esco_drc_p._p.limiter[0].threshold[1] = 32768;
    info->R_pch = info->pch = &esco_drc_p;
    return 0;
}
void *esco_eq_drc_setup(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en)
{
#if TCFG_EQ_ENABLE && TCFG_PHONE_EQ_ENABLE

    if (config_eq_lite_en) { //仅支持同步方式eq处理
        async = 0;//
    }

    struct dec_eq_drc *eff = zalloc(sizeof(struct dec_eq_drc));

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
    eff->spk_eq	= spk_eq_open(sample_rate, channel, drc_en ? 1 : 0);
#endif/*AUDIO_SPK_EQ_CONFIG*/


    struct audio_eq_param eq_param = {0};

    eff->priv = priv;
    eff->out_cb = eq_output_cb;
    eq_param.channels = channel;
    eq_param.online_en = 1;
    eq_param.mode_en = 0;
    eq_param.remain_en = 0;
    eq_param.no_wait = async;//a2dp_low_latency ? 0 : 1;
    if (drc_en) {
        eq_param.out_32bit = 1;
    }
    eq_param.max_nsection = EQ_SECTION_MAX;
    eq_param.cb = eq_phone_get_filter_info;
    eq_param.eq_name = call_eq_mode;
    eq_param.sr = sample_rate;
    eq_param.priv = eff;
    eq_param.output = eq_output;
    eff->eq = audio_dec_eq_open(&eq_param);

#if TCFG_DRC_ENABLE
    if (drc_en) {
        struct audio_drc_param drc_param = {0};
        drc_param.sr = sample_rate;
        drc_param.channels = channel;
        drc_param.online_en = 0;
        drc_param.remain_en = 0;
        drc_param.out_32bit = 1;
        drc_param.cb = esco_drc_get_filter_info;
        drc_param.drc_name = call_eq_mode;
        eff->drc = audio_dec_drc_open(&drc_param);
        eff->async = async;
    }
#endif//TCFG_DRC_ENABLE

    return eff;
#else
    return NULL;
#endif//TCFG_EQ_ENABLE && TCFG_PHONE_EQ_ENABLE

}
void esco_eq_drc_free(void *eff)
{
    dec_eq_drc_free(eff);
}
#define audio_out_drc_get_filter esco_drc_get_filter_info//可复用通话下行限幅器系数
void *audio_out_eq_drc_setup(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en)
{
#if TCFG_EQ_ENABLE &&AUDIO_OUT_EQ_USE_SPEC_NUM
    if (config_eq_lite_en) { //仅支持同步方式eq处理
        async = 0;//
    }

    struct dec_eq_drc *eff = zalloc(sizeof(struct dec_eq_drc));
    struct audio_eq_param eq_param = {0};

    eff->priv = priv;
    eff->out_cb = eq_output_cb;

    eq_param.channels = channel;
    eq_param.online_en = 0;
    eq_param.mode_en = 0;
    eq_param.remain_en = 0;
    eq_param.no_wait = async;
    if (drc_en) {
        eq_param.out_32bit = 1;
    }
    eq_param.max_nsection = AUDIO_OUT_EQ_USE_SPEC_NUM;
    eq_param.cb = audio_out_eq_get_filter_info;
    eq_param.eq_name = song_eq_mode;
    eq_param.sr = sample_rate;
    eq_param.priv = eff;
    eq_param.output = eq_output;
    eff->eq = audio_dec_eq_open(&eq_param);
#if TCFG_DRC_ENABLE
    if (drc_en) {
        struct audio_drc_param drc_param = {0};
        drc_param.sr = sample_rate;
        drc_param.channels = channel;
        drc_param.online_en = 0;
        drc_param.remain_en = 0;
        drc_param.out_32bit = 1;
        drc_param.cb = audio_out_drc_get_filter;
        drc_param.drc_name = song_eq_mode;
        eff->drc = audio_dec_drc_open(&drc_param);
        eff->async = async;
    }
#endif //TCFG_DRC_ENABLE

    return eff;
#else
    return NULL;
#endif//TCFG_EQ_ENABLE &&AUDIO_OUT_EQ_USE_SPEC_NUM

}
void audio_out_eq_drc_free(void *eff)
{
#if AUDIO_OUT_EQ_USE_SPEC_NUM
    dec_eq_drc_free(eff);
#endif//AUDIO_OUT_EQ_USE_SPEC_NUM

}

__attribute__((always_inline))
void sat16_to_sat32(s16 *in, s32 *out, u32 npoint, int factor)
{
#if 0
    for (int i = 0; i < npoint; i ++) {
        out[i] = in[i] * factor;
    }
#else
    s64 tmp;
    __asm__ volatile(
        "1:											\n\t"
        "rep %2 {									\n\t"        //循环
        "%3 = h[%1 ++= 2]*%4(s)                              \n\t"
        "[%0 ++= 4] = %3.l                            \n\t"   //%3.l意思是取tmp的低32位
        "}										    \n\t"
        "if(%2 != 0) goto 1b			            \n\t"
        :
        "=&r"(out),
        "=&r"(in),
        "=&r"(npoint),
        "=&r"(tmp)
        :
        "r"(factor),
        "0"(out),
        "1"(in),
        "2"(npoint),
        "3"(tmp)
        :
    );

#endif
}

int eq_drc_run(void *priv, void *data, u32 len)
{
#if TCFG_EQ_ENABLE
    struct dec_eq_drc *eff = (struct dec_eq_drc *)priv;
    if (!eff) {
        return 0;
    }

#if TCFG_AUDIO_CVP_DUT_ENABLE
    if (audio_dec_dut_en_get(1) == AUDIO_DEC_DUT_MODE_SPK_EQ) {
#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
        spk_eq_run(eff->spk_eq, data, data, len);
        return 0;
#endif
    }
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/

#if defined(TCFG_AUDIO_DAC_24BIT_MODE) && TCFG_AUDIO_DAC_24BIT_MODE
    if (!eff->async) {
        if (eff->eq_out_buf && (eff->eq_out_points < eff->eq_out_total)) {
            eq_32bit_out(eff);
            if (eff->eq_out_points < eff->eq_out_total) {
                return 0;
            }
        }
    }
#endif


#if TCFG_DRC_ENABLE
    if (eff->drc && !eff->async) {//同步32bit eq drc 处理
        if ((!eff->eq_out_buf) || (eff->eq_out_buf_len < len * 2)) {
            if (eff->eq_out_buf) {
                free(eff->eq_out_buf);
            }
            eff->eq_out_buf_len = len * 2;
            eff->eq_out_buf = malloc(eff->eq_out_buf_len);
            ASSERT(eff->eq_out_buf);
        }
        audio_eq_set_output_buf(eff->eq, eff->eq_out_buf, len);

        if (eff->drc_bef_eq && eff->drc_prev) {
            sat16_to_sat32((short *)data, (int *)eff->eq_out_buf, len >> 1, 1);
            audio_drc_run(eff->drc_prev, eff->eq_out_buf, len * 2);
        }
    }
#endif//TCFG_DRC_ENABLE

    int eqlen = 0;
    if (eff->drc_bef_eq) {
        eqlen = audio_eq_run(eff->eq, eff->eq_out_buf, len * 2);
    } else {
        eqlen = audio_eq_run(eff->eq, data, len);
    }
#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
    if (!eff->async) {
        if (eff->drc) {
            spk_eq_run(eff->spk_eq, eff->eq_out_buf, eff->eq_out_buf, len * 2);
        } else {
            spk_eq_run(eff->spk_eq, data, data, len);
        }
    }
#endif

#if TCFG_DRC_ENABLE
    if (eff->drc && !eff->async) {//同步32bit eq drc 处理
        audio_drc_run(eff->drc, eff->eq_out_buf, len * 2);
#if defined(TCFG_AUDIO_DAC_24BIT_MODE) && TCFG_AUDIO_DAC_24BIT_MODE
        eff->eq_out_points = 0;
        eff->eq_out_total = (len * 2) / 4;
        eq_32bit_out(eff);
        return len;//返回16bit位宽长度
#else
        user_sat16((s32 *)eff->eq_out_buf, (s16 *)data, (len * 2) / 4);
#endif
    }
#endif//TCFG_DRC_ENABLE

    if (eff->drc && !eff->async) {
        return len;
    }
    return eqlen;
#else
    return len;
#endif
}

#if AUDIO_OUT_EQ_USE_SPEC_NUM

static struct eq_seg_info audio_out_eq_tab[AUDIO_OUT_EQ_USE_SPEC_NUM] = {
#ifdef EQ_CORE_V1
    {0, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 12000, 0, 0.3f},
#else
    {0, EQ_IIR_TYPE_BAND_PASS, 125,   0 * (1 << 20), 0.7f * (1 << 24)},
    {1, EQ_IIR_TYPE_BAND_PASS, 12000, 0 * (1 << 20), 0.3f * (1 << 24)},
#endif
};



int audio_out_eq_get_filter_info(void *eq, int sr, struct audio_eq_filter_info *info)
{
    struct audio_eq *eq_hdl = (struct audio_eq *)eq;
    if (!eq_hdl) {
        return -1;
    }
    local_irq_disable();
    u8 nsection = ARRAY_SIZE(audio_out_eq_tab);
    if (!eq_hdl->eq_coeff_tab) {
        eq_hdl->eq_coeff_tab = zalloc(sizeof(int) * 5 * nsection);
    }
    for (int i = 0; i < nsection; i++) {
        eq_seg_design(&audio_out_eq_tab[i], sr, &eq_hdl->eq_coeff_tab[5 * i]);
    }

    local_irq_enable();
    info->L_coeff = info->R_coeff = (void *)eq_hdl->eq_coeff_tab;
    info->L_gain = info->R_gain = 0;
    info->nsection = nsection;
    return 0;
}
int audio_out_eq_spec_set_info(struct audio_eq *eq, u8 idx, int freq, float gain)
{
    if (idx >= AUDIO_OUT_EQ_USE_SPEC_NUM) {
        return false;
    }
    if (gain > 12) {
        gain = 12;
    }
    if (gain < -12) {
        gain = -12;
    }
    if (freq) {
        audio_out_eq_tab[idx].freq = freq;
    }
#ifdef EQ_CORE_V1
    audio_out_eq_tab[idx].gain = gain;
#else
    audio_out_eq_tab[idx].gain = gain * (1 << 20);
#endif

    /* printf("audio out eq, idx:%d, freq:%d,%d, gain:%d,%d \n", idx, freq, audio_out_eq_tab[idx].freq, gain, audio_out_eq_tab[idx].gain); */

#if !AUDIO_EQ_FADE_EN
    local_irq_disable();
    if (eq) {
        eq->updata = 1;
    }
    local_irq_enable();
#endif

    return true;
}

#if AUDIO_EQ_FADE_EN

void eq_fade_run(void *p)
{
    struct dec_eq_drc *eff = (struct dec_eq_drc *)p;
    struct eq_filter_fade *fade = &eff->fade;
    struct audio_eq *eq = (struct audio_eq *)eff->eq;


    u8 update = 0;
    u8 design = 0;
    for (int i = 0; i < ARRAY_SIZE(audio_out_eq_tab); i++) {
        if (fade->cur_gain[i] > fade->use_gain[i]) {
#ifdef EQ_CORE_V1
            fade->cur_gain[i] -= HIGH_BASS_EQ_FADE_STEP;
#else
            fade->cur_gain[i] -= HIGH_BASS_EQ_FADE_STEP * (1 << 20);
#endif
            if (fade->cur_gain[i] < fade->use_gain[i]) {
                fade->cur_gain[i] = fade->use_gain[i];
            }
            design = 1;
        } else if (fade->cur_gain[i] < fade->use_gain[i]) {
#ifdef EQ_CORE_V1
            fade->cur_gain[i] += HIGH_BASS_EQ_FADE_STEP;
#else
            fade->cur_gain[i] += HIGH_BASS_EQ_FADE_STEP * (1 << 20);
#endif
            if (fade->cur_gain[i] > fade->use_gain[i]) {
                fade->cur_gain[i] = fade->use_gain[i];
            }
            design = 1;
        }

        if (design) {
            design = 0;
            update = 1;
            /* audio_out_eq_spec_set_info(p, i, 0, fade->cur_gain[i]); */
            /* if (eq) { */
            audio_out_eq_tab[i].gain = fade->cur_gain[i];//gain;//gain << 20;
            /* } */

        }
    }

    if (update) {
        update = 0;
        local_irq_disable();
        if (eq) {
            eq->updata = 1;
        }
        local_irq_enable();
    }
}



#endif


int audio_out_eq_set_gain(void *eff, u8 idx, int gain)
{
    struct dec_eq_drc *eff_hdl = (struct dec_eq_drc *)eff;
    if (!eff_hdl) {
        return 0;
    }
    if (!idx) {
        idx = 0;
    } else {
        idx = 1;
    }

#if AUDIO_EQ_FADE_EN
    eff_hdl->fade.use_gain[idx] = gain;
    if (eff_hdl->fade.tmr == 0) {
        eff_hdl->fade.tmr = sys_hi_timer_add(eff_hdl, eq_fade_run, 20);
    }
#else
    if (!idx) {
        audio_out_eq_spec_set_info(eff_hdl->eq, 0, 0, gain);//低音
    } else {
        audio_out_eq_spec_set_info(eff_hdl->eq, 1, 0, gain);//高音
    }
#endif

    return true;
}
#endif /*AUDIO_OUT_EQ_USE_SPEC_NUM*/


static struct audio_drc *mix_out_drc = NULL;


static float mix_out_drc_threadhold = -0.5f;//db -60db~0db,限幅器阈值
static struct drc_ch mix_out_drc_p = {0};
int mix_out_drc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    float th = mix_out_drc_threadhold;//db -60db~0db,限幅器阈值
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768);
    mix_out_drc_p.nband = 1;
    mix_out_drc_p.type = 1;
    mix_out_drc_p._p.limiter[0].attacktime = 5;
    mix_out_drc_p._p.limiter[0].releasetime = 500;
    mix_out_drc_p._p.limiter[0].threshold[0] = threshold;
    mix_out_drc_p._p.limiter[0].threshold[1] = 32768;
    info->R_pch = info->pch = &mix_out_drc_p;
    return 0;
}

void mix_out_drc_open(u16 sample_rate)
{
    u8 ch_num;
#if TCFG_APP_FM_EMITTER_EN
    ch_num = 2;
#else
    u8 dac_connect_mode = audio_dac_get_channel(&dac_hdl);
    if (dac_connect_mode == DAC_OUTPUT_LR) {
        ch_num =  2;
    } else {
        ch_num =  1;
    }
#endif//TCFG_APP_FM_EMITTER_EN

#if TCFG_DRC_ENABLE
    struct audio_drc_param drc_param = {0};
    drc_param.sr = sample_rate;
    drc_param.channels = ch_num;
    drc_param.online_en = 0;
    drc_param.remain_en = 0;
    drc_param.out_32bit = 0;
    drc_param.cb = mix_out_drc_get_filter_info;
    drc_param.drc_name = song_eq_mode;
    mix_out_drc = audio_dec_drc_open(&drc_param);
#endif//TCFG_DRC_ENABLE

}

void mix_out_drc_close()
{
#if TCFG_DRC_ENABLE
    if (mix_out_drc) {
        audio_dec_drc_close(mix_out_drc);
        mix_out_drc = NULL;
    }
#endif//TCFG_DRC_ENABLE

}

void mix_out_drc_run(s16 *data, u32 len)
{
    u8 en = 1;
#if TCFG_DRC_ENABLE
#if TCFG_AUDIO_ANC_ENABLE
    en = anc_status_get();
#endif/*TCFG_AUDIO_ANC_ENABLE*/
    if (mix_out_drc && en) {
        audio_drc_run(mix_out_drc, data, len);
    }
#endif//TCFG_DRC_ENABLE

}

/*----------------------------------------------------------------------------*/
/**@brief    mix_out后限幅器系数更新
   @param    threadhold限幅器阈值，-60~0,单位db
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void mix_out_drc_threadhold_update(float threadhold)
{
#if TCFG_DRC_ENABLE
    mix_out_drc_threadhold = threadhold;

    local_irq_disable();
    if (mix_out_drc) {
        mix_out_drc->updata = 1;
    }
    local_irq_enable();
#endif//TCFG_DRC_ENABLE

}

struct audio_eq *audio_dec_eq_open_new(struct audio_eq_param *parm, struct eq_parm_new *par_new)
{
    struct audio_eq_param *eq_param = parm;
    struct audio_eq *eq = zalloc(sizeof(struct audio_eq) + sizeof(struct hw_eq_ch));
    if (eq) {
        eq->eq_ch = (struct hw_eq_ch *)((int)eq + sizeof(struct audio_eq));

        audio_eq_open(eq, eq_param);
        audio_eq_set_samplerate(eq, eq_param->sr);
        audio_eq_set_info_new(eq, eq_param->channels, par_new->in_mode, eq_param->out_32bit, par_new->run_mode, par_new->data_in_mode, par_new->data_out_mode);
        /* log_info("eq_param->sr %d, eq_param->channels %d\n", eq_param->sr,  eq_param->channels); */
        audio_eq_set_output_handle(eq, eq_param->output, eq_param->priv);
        audio_eq_start(eq);
        /* log_info("audio_dec_eq_open name %d\n", eq_param->eq_name); */
    }
    return eq;
}

#if defined(AUDIO_GAME_EFFECT_CONFIG) && AUDIO_GAME_EFFECT_CONFIG

struct drc_ch dec_drc_p = {0};
int dec_drc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    float th = -0.5f;//db -60db~0db,限幅器阈值
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768);
    dec_drc_p.nband = 1;
    dec_drc_p.type = 1;
    dec_drc_p._p.limiter[0].attacktime = 5;
    dec_drc_p._p.limiter[0].releasetime = 500;
    dec_drc_p._p.limiter[0].threshold[0] = threshold;
    dec_drc_p._p.limiter[0].threshold[1] = 32768;
    info->R_pch = info->pch = &dec_drc_p;
    return 0;
}


void *dec_eq_drc_setup_new(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en)
{
#if TCFG_EQ_ENABLE

    struct dec_eq_drc *eff = zalloc(sizeof(struct dec_eq_drc));
    if (config_eq_lite_en) { //仅支持同步方式eq处理
        async = 0;//
    }

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
    eff->spk_eq	= spk_eq_open(sample_rate, channel, drc_en ? 1 : 0);
#endif/*AUDIO_SPK_EQ_CONFIG*/

#if TCFG_DRC_ENABLE
    if (drc_en & BIT(1)) { //game eff drc
        if (!eq_file_get_cfg(get_eq_cfg_hdl(), (u8 *)SDFILE_RES_ROOT_PATH"eq_game_eff.bin")) { //加载游戏音效
            EQ_CFG *eq_cfg = get_eq_cfg_hdl();
            eq_cfg->eq_type = EQ_TYPE_FILE;
        }
        eff->drc_bef_eq = 1;
        async = 0;
        struct audio_drc_param drc_param = {0};
        drc_param.sr = sample_rate;
        drc_param.channels = channel;
        drc_param.online_en = 1;
        drc_param.remain_en = 1;
        drc_param.out_32bit = 1;
        drc_param.cb = drc_get_filter_info;
        drc_param.drc_name = song_eq_mode;
        eff->drc_prev = audio_dec_drc_open(&drc_param);
    }
#endif//TCFG_DRC_ENABLE

    eff->priv = priv;
    eff->out_cb = eq_output_cb;

    struct audio_eq_param eq_param = {0};
    eq_param.channels = channel;
    eq_param.online_en = 1;
    eq_param.mode_en = 1;
    eq_param.remain_en = 1;
    eq_param.no_wait = async;
    if (drc_en) {
        eq_param.out_32bit = 1;
    }
    eq_param.max_nsection = EQ_SECTION_MAX;
    eq_param.cb = eq_get_filter_info;
    eq_param.eq_name = song_eq_mode;
    eq_param.sr = sample_rate;
    eq_param.priv = eff;
    eq_param.output = eq_output;

    struct eq_parm_new par_new = {0};
    par_new.in_mode = DATI_INT;
    par_new.run_mode = NORMAL;
    par_new.data_in_mode = SEQUENCE_DAT_IN;
    par_new.data_out_mode = SEQUENCE_DAT_OUT;
    eff->eq = audio_dec_eq_open_new(&eq_param, &par_new);
    eff->async = async;
#if TCFG_DRC_ENABLE
    if (drc_en) {
        struct audio_drc_param drc_param = {0};
        drc_param.sr = sample_rate;
        drc_param.channels = channel;
        if (eff->drc_bef_eq) {
            drc_param.online_en = 0;
            drc_param.remain_en = 0;
            drc_param.out_32bit = 1;
            drc_param.cb = dec_drc_get_filter_info;
            drc_param.drc_name = mic_eq_mode;
        } else {
            drc_param.online_en = 1;
            drc_param.remain_en = 1;
            drc_param.out_32bit = 1;
            drc_param.cb = drc_get_filter_info;
            drc_param.drc_name = song_eq_mode;
        }
        eff->drc = audio_dec_drc_open(&drc_param);
    }
#endif//TCFG_DRC_ENABLE

    return eff;
#else
    return NULL;
#endif//TCFG_EQ_ENABLE

}
#endif /*AUDIO_GAME_EFFECT_CONFIG*/


#if TCFG_AUDIO_ESCO_LIMITER_ENABLE
#if !TCFG_DRC_ENABLE
#error "需打开宏TCFG_DRC_ENABLE"
#endif
#ifndef ESCO_DL_LIMITER_THR
#define  ESCO_DL_LIMITER_THR 0
#endif
static struct drc_ch esco_drc = {0};
int esco_limiter_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    float th = ESCO_DL_LIMITER_THR;//db -60db~0db,限幅器阈值
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768);
    esco_drc.nband = 1;
    esco_drc.type = 1;
    esco_drc._p.limiter[0].attacktime = 5;
    esco_drc._p.limiter[0].releasetime = 500;
    esco_drc._p.limiter[0].threshold[0] = threshold;
    esco_drc._p.limiter[0].threshold[1] = 32768;
    info->R_pch = info->pch = &esco_drc;
    return 0;
}

struct audio_drc *esco_limiter_open(u16 sample_rate, u16 ch_num)
{
#if TCFG_DRC_ENABLE

    struct audio_drc_param drc_param = {0};
    drc_param.sr = sample_rate;
    drc_param.channels = ch_num;
    drc_param.cb = esco_limiter_get_filter_info;
    drc_param.drc_name = call_eq_mode;
    return  audio_dec_drc_open(&drc_param);
#else
    return NULL;
#endif//TCFG_DRC_ENABLE
}

void esco_limiter_close(struct audio_drc *limiter)
{
#if TCFG_DRC_ENABLE
    if (limiter) {
        audio_dec_drc_close(limiter);
        limiter = NULL;
    }
#endif//TCFG_DRC_ENABLE

}

void esco_limiter_run(struct audio_drc *limiter, s16 *data, u32 len)
{
#if TCFG_DRC_ENABLE
    if (limiter) {
        audio_drc_run(limiter, data, len);
    }
#endif//TCFG_DRC_ENABLE
}
#endif/*TCFG_AUDIO_ESCO_LIMITER_ENABLE*/

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
#define AUDIO_SPK_EQ_STERO  1//支持左右声道独立的spk_eq


/*
 *spk_eq系数表
 * */
static struct eq_seg_info spk_eq_seg_tab[] = {
    /*L*/
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, 0.7f},
#if AUDIO_SPK_EQ_STERO
    /*R*/
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, 0.7f}
#endif
};
#if AUDIO_SPK_EQ_STERO
static struct eq_seg_info *spk_eq_seg_tab_R = &spk_eq_seg_tab[10];
#endif

/*
 *spk_eq系数表运算后存放的buf
 * */
static float spk_eq_coeff_tab[10][5];
static float spk_eq_coeff_tab_R[10][5];
/*
 *spk_eq 总增益
 * */
static float spk_eq_global_gain[2] = {0};//[0]：左声道   [1]:右声道
static u32 seg_sel = 0;
static u32 seg_sel_R = 0;
static u32 spk_eq_sample_rate = 0;
/*
 *spk_eq系数回调函数
 * */
int spk_eq_get_filter_info(void *_eq, int sr, struct audio_eq_filter_info *info)
{
    struct audio_eq *eq = _eq;
    if (!sr) {
        sr = spk_eq_sample_rate;
    }
    u8 ch_num;
#if TCFG_APP_FM_EMITTER_EN
    ch_num = 2;
#else
    u8 dac_connect_mode = audio_dac_get_channel(&dac_hdl);
    if (dac_connect_mode == DAC_OUTPUT_LR) {
        ch_num =  2;
    } else {
        ch_num =  1;
    }
#endif//TCFG_APP_FM_EMITTER_EN

    local_irq_disable();
    u8 cnt = 1;
#if AUDIO_SPK_EQ_STERO
    cnt = 2;
#endif
    u8 nsection = ARRAY_SIZE(spk_eq_seg_tab) / cnt;
    for (int i = 0; i < nsection; i++) {
        if (seg_sel & BIT(i)) {
            seg_sel &= ~BIT(i);
            eq_seg_design(&spk_eq_seg_tab[i], sr, &spk_eq_coeff_tab[i]);
            /* float *tar_buf = &spk_eq_coeff_tab[i]; */
            /* printf("cal coeff:0x%x, 0x%x, 0x%x, 0x%x, 0x%x " */
            /* , *(int *)&tar_buf[0] */
            /* , *(int *)&tar_buf[1] */
            /* , *(int *)&tar_buf[2] */
            /* , *(int *)&tar_buf[3] */
            /* , *(int *)&tar_buf[4] */
            /* ); */
        }
#if AUDIO_SPK_EQ_STERO
        if (ch_num == 2) {
            if (seg_sel_R & BIT(i)) {
                seg_sel_R &= ~BIT(i);
                eq_seg_design(&spk_eq_seg_tab_R[i], sr, &spk_eq_coeff_tab_R[i]);
            }
        }
#endif
    }
    /* printf("spk_eq_global_gain[0] %d\n",(int)spk_eq_global_gain[0]); */
    /* printf("spk_eq_global_gain[1] %d\n",(int)spk_eq_global_gain[1]); */
    local_irq_enable();
#if AUDIO_SPK_EQ_STERO
    if (ch_num == 2) {
        info->L_coeff  = (void *)spk_eq_coeff_tab;//系数指针赋值
        info->R_coeff  = (void *)spk_eq_coeff_tab_R;//系数指针赋值
        info->L_gain = spk_eq_global_gain[0];//总增益填写，用户可修改（-20~20db）
        info->R_gain = spk_eq_global_gain[1];//总增益填写，用户可修改（-20~20db）
    } else
#endif
    {
        info->L_coeff = info->R_coeff = (void *)spk_eq_coeff_tab;//系数指针赋值
        info->L_gain = info->R_gain = spk_eq_global_gain[0];//总增益填写，用户可修改（-20~20db）
    }
    info->nsection = nsection;//eq段数，根据提供给的系数表来填写，例子是2
    return 0;
}

static struct audio_eq *spk_eq = NULL;
/*
 *spk_eq 系数更新接口,更新第几段eq系数
 *parm: *seg
 *seg->index:第几段(0~9)
 *seg->iir_type:滤波器类型(EQ_IIR_TYPE)
 *seg->freq:中心截止频率(20~20kHz)
 *seg->gain:增益（-12~13dB）
 * */
void spk_eq_seg_update(struct eq_seg_info *seg)
{
    if (!seg) {
        log_e("spk_eq seg null\n");
        return ;
    }
    u8 i = seg->index;
    u8 cnt = 1;
#if AUDIO_SPK_EQ_STERO
    cnt = 2;
#endif

    if (i >= ARRAY_SIZE(spk_eq_seg_tab) / cnt) {
        log_e("spk_eq index err: %d >= %d\n", seg->index, ARRAY_SIZE(spk_eq_seg_tab) / cnt);
        return ;
    }
    local_irq_disable();
    memcpy(&spk_eq_seg_tab[i], seg, sizeof(struct eq_seg_info));
    seg_sel |= BIT(i);
    if (spk_eq) {
        struct audio_eq *eq = spk_eq;
        eq->updata = 1;//设置更新标志
    }
    local_irq_enable();
}

/*
 *spk_eq 右声道系数更新接口,更新第几段eq系数
 *parm: *seg
 *seg->index:第几段(0~9)
 *seg->iir_type:滤波器类型(EQ_IIR_TYPE)
 *seg->freq:中心截止频率(20~20kHz)
 *seg->gain:增益（-12~13dB）
 * */
void spk_eq_seg_update_R(struct eq_seg_info *seg)
{
    u8 cnt = 1;
#if AUDIO_SPK_EQ_STERO
    if (!seg) {
        log_e("spk_eq seg null\n");
        return ;
    }
    u8 i = seg->index;
    cnt = 2;

    if (i >= ARRAY_SIZE(spk_eq_seg_tab) / cnt) {
        log_e("spk_eq index err: %d >= %d\n", seg->index, ARRAY_SIZE(spk_eq_seg_tab) / cnt);
        return ;
    }
    local_irq_disable();
    memcpy(&spk_eq_seg_tab_R[i], seg, sizeof(struct eq_seg_info));
    seg_sel_R |= BIT(i);
    if (spk_eq) {
        struct audio_eq *eq = spk_eq;
        eq->updata = 1;//设置更新标志
    }
    local_irq_enable();
#endif
}

/*
 *spk_eq 总增益更新
 * */
void spk_eq_global_gain_udapte(float global_gain)
{
    spk_eq_global_gain[0] = global_gain;
    local_irq_disable();
    if (spk_eq) {
        /* printf("spk_eq_global_gain[L] %d\n",(int)spk_eq_global_gain[0]); */
        struct audio_eq *eq = spk_eq;
        eq->updata = 1;//设置更新标志
    }
    local_irq_enable();
}

void spk_eq_global_gain_udapte_R(float global_gain)
{
    spk_eq_global_gain[1] = global_gain;
    local_irq_disable();
    if (spk_eq) {
        /* printf("spk_eq_global_gain[R] %d\n",(int)spk_eq_global_gain[1]); */
        struct audio_eq *eq = spk_eq;
        eq->updata = 1;//设置更新标志
    }
    local_irq_enable();
}

/*
 *spk_eq 打开
 * */
void *spk_eq_open(u32 sample_rate, u8 ch_num, u8 bitwide)
{
#if TCFG_AUDIO_CVP_DUT_ENABLE
    //DEC_DUT模式默认仅保留spk_eq, 因此改为16bit位宽
    if (audio_dec_dut_en_get(1) == AUDIO_DEC_DUT_MODE_SPK_EQ) {
        bitwide = 0;
    }
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/
    spk_eq_sample_rate = sample_rate;
    seg_sel = 0xffff;
    seg_sel_R = 0xffff;
    struct audio_eq_param eq_param = {0};

    eq_param.channels = ch_num;
    eq_param.max_nsection = EQ_SECTION_MAX;
    eq_param.cb = spk_eq_get_filter_info;
    eq_param.eq_name = song_eq_mode;
    eq_param.sr = sample_rate;
    eq_param.out_32bit = bitwide ? 1 : 0;


    struct eq_parm_new par_new = {0};
    par_new.in_mode = bitwide ? DATI_INT : DATI_SHORT;
    par_new.run_mode = NORMAL;
    par_new.data_in_mode = SEQUENCE_DAT_IN;
    par_new.data_out_mode = SEQUENCE_DAT_OUT;
    void *eq = audio_dec_eq_open_new(&eq_param, &par_new);
    local_irq_disable();
    spk_eq = eq;//记录spk_eq 指针,用于系数更新调用
    local_irq_enable();
    /* sys_timer_add(NULL, spk_eq_global_gain_udpate_test, 5000); */
    /* sys_timer_add(NULL,spk_eq_seg_udpate_test , 5000); */
    return eq;
}

/*
 *spk_eq 关闭
 * */
void spk_eq_close(void *eq)
{
    if (eq) {
        local_irq_disable();
        spk_eq = NULL;
        local_irq_enable();
        audio_dec_eq_close(eq);
    }
}

/*
 *spk_eq 数据处理
 * */
void spk_eq_run(void *eq, void *in, void *out, u16 len)
{
    if (!eq) {
        return ;
    }

    audio_eq_set_output_buf(eq, out, len);
    audio_eq_run(eq, in, len);
}

/*
 *spk_eq 系数表保存到vm
 * */
int spk_eq_save_to_vm(void)
{
    int ret_tmp = 0;
    /* puts("====jj===========to save \n"); */
    /* printf("sizeof(spk_eq_seg_tab) %d, %d\n", sizeof(spk_eq_seg_tab), sizeof(float)); */
    /* printf("%d %d\n", CFG_SPK_EQ_SEG_SAVE, CFG_SPK_EQ_GLOBAL_GAIN_SAVE); */
    int ret = syscfg_write(CFG_SPK_EQ_SEG_SAVE, spk_eq_seg_tab, sizeof(spk_eq_seg_tab));
    if (ret <= 0) {
        printf("spk_eq tab write to vm err, ret %d\n", ret);
        ret_tmp = -1;
    }
    ret = syscfg_write(CFG_SPK_EQ_GLOBAL_GAIN_SAVE, spk_eq_global_gain, sizeof(spk_eq_global_gain));
    if (ret <= 0) {
        printf("spk_eq global gain write to vm err ret %d\n", ret);
        ret_tmp = -1;
    }
    /* puts("===============to save end\n"); */
    return ret_tmp;
}
/*
 *spk_eq 系数表从vm中读取
 * */
void spk_eq_read_from_vm()
{
    int ret = syscfg_read(CFG_SPK_EQ_SEG_SAVE, spk_eq_seg_tab, sizeof(spk_eq_seg_tab));
    if (ret <= 0) {
        printf("skp_eq read from vm err\n");
    }
    ret = syscfg_read(CFG_SPK_EQ_GLOBAL_GAIN_SAVE, spk_eq_global_gain, sizeof(spk_eq_global_gain));
    if (ret <= 0) {
        printf("spk_eq global gain read from vm err\n");
    }
}

/* typedef struct { */
/* u8 cmd; */
/* struct eq_seg_info seg; */
/* }spk_seg_info; */

/* typedef struct { */
/* u8 cmd; */
/* float global_gain; */
/* }spk_global_gain_info; */


typedef struct {
    u16 magic;     //0x3344
    u16 crc;       //data crc
    u8 data[32];   //data
} SPK_EQ_PACK;

#define CMD_SEG    0x1
#define CMD_GLOBAL 0x2
#define CMD_SAVE_PARM 0x3
#define CMD_RESET_PARM 0x4
/*
 *右声道更新命令
 * */
#define CMD_SEG_R        0x5
#define CMD_GLOBAL_R     0x6

#define CMD_SUPPORT_GLOBAL_GAIN 0x7


#define CMD_READ_SEG_L   0x8//左声道或者单声道获取系数表
#define CMD_READ_SEG_R   0x9//右声道获取系数表

#define CMD_READ_GLOBAL_L  0xa //左声道或者单声道获取总增益
#define CMD_READ_GLOBAL_R  0xb //右声道获取总增益

#define SPK_EQ_CRC_EN  0//是否使能crc校验

static u8 parse_seq = 0;
void (*send_data_handler)(u8 seq, u8 *packet, u8 size);

static spk_eq_ack_packet(u8 seq, u8 *packet, u8 size)
{
    if (send_data_handler) {
        send_data_handler(seq, packet, size);
    } else {
        app_online_db_ack(seq, packet, size);
    }
}

int spk_eq_spp_rx_packet(u8 *packet, u8 len)
{

    SPK_EQ_PACK pack = {0};//packet;
    memcpy(&pack, packet, len);
    if (pack.magic != 0x3344) {
        printf("magic err 0x%x\n", pack.magic);
        return -1;
    }
    struct eq_seg_info seg = {0};
    float global_gain = 0;
    u8 cmd = pack.data[0];
    u16 crc;
    printf("cmd %d\n", cmd);
    switch (cmd) {
    case CMD_SEG:
#if SPK_EQ_CRC_EN
        crc = CRC16(pack.data, len - 4);
        if (crc != pack.crc) {
            printf("spk_seg pack crc err %x, %x\n", crc, pack.crc);
            return -1;
        }
#endif
        memcpy(&seg, &pack.data[1], sizeof(struct eq_seg_info));
        spk_eq_seg_update(&seg);

        printf("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg.index, seg.iir_type, seg.freq, *(int *)&seg.gain, *(int *)&seg.q);
        break;
    case CMD_GLOBAL:
#if SPK_EQ_CRC_EN
        crc = CRC16(pack.data, len - 4);
        if (crc != pack.crc) {
            printf("spk global gain info pack crc err %x, %x\n", crc, pack.crc);
            return -1;
        }
#endif
        memcpy(&global_gain, &pack.data[1], sizeof(float));
        spk_eq_global_gain_udapte(global_gain);
        printf("global_gain 0x%x\n", *(int *)&global_gain);
        break;
    case CMD_SEG_R:
#if SPK_EQ_CRC_EN
        crc = CRC16(pack.data, len - 4);
        if (crc != pack.crc) {
            printf("spk_seg_r pack crc err %x, %x\n", crc, pack.crc);
            return -1;
        }
#endif
        memcpy(&seg, &pack.data[1], sizeof(struct eq_seg_info));
        spk_eq_seg_update_R(&seg);

        printf("R idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg.index, seg.iir_type, seg.freq, *(int *)&seg.gain, *(int *)&seg.q);
        break;
    case CMD_GLOBAL_R:
#if SPK_EQ_CRC_EN
        crc = CRC16(pack.data, len - 4);
        if (crc != pack.crc) {
            printf("spk global gain info pack crc err %x, %x\n", crc, pack.crc);
            return -1;
        }
#endif
        memcpy(&global_gain, &pack.data[1], sizeof(float));
        spk_eq_global_gain_udapte_R(global_gain);
        printf("R global_gain 0x%x\n", *(int *)&global_gain);
        break;
    case  CMD_SAVE_PARM:
        return spk_eq_save_to_vm();
        break;
    case CMD_RESET_PARM:
        local_irq_disable();
        u8 cnt = 1;
#if AUDIO_SPK_EQ_STERO
        cnt = 2;
#endif

        for (int i = 0; i < ARRAY_SIZE(spk_eq_seg_tab) / cnt; i++) {
            spk_eq_seg_tab[i].index = i;
            spk_eq_seg_tab[i].freq = 100;
            spk_eq_seg_tab[i].iir_type = EQ_IIR_TYPE_BAND_PASS;
            spk_eq_seg_tab[i].gain = 0.0f;
            spk_eq_seg_tab[i].q = 0.7f;
            seg_sel |= BIT(i);

#if AUDIO_SPK_EQ_STERO
            spk_eq_seg_tab_R[i].index = i;
            spk_eq_seg_tab_R[i].freq = 100;
            spk_eq_seg_tab_R[i].iir_type = EQ_IIR_TYPE_BAND_PASS;
            spk_eq_seg_tab_R[i].gain = 0.0f;
            spk_eq_seg_tab_R[i].q = 0.7f;
            seg_sel_R |= BIT(i);
#endif
        }
        local_irq_enable();
        spk_eq_global_gain_udapte(0);
        spk_eq_global_gain_udapte_R(0);

        return spk_eq_save_to_vm();
        break;
    case CMD_SUPPORT_GLOBAL_GAIN:
        //support global_gain
        break;
    case CMD_READ_SEG_L:
        spk_eq_ack_packet(parse_seq, spk_eq_seg_tab, sizeof(spk_eq_seg_tab) / 2);
        return 1;
    case CMD_READ_SEG_R:
        spk_eq_ack_packet(parse_seq, &spk_eq_seg_tab[10], sizeof(spk_eq_seg_tab) / 2);
        return 1;
    case CMD_READ_GLOBAL_L:
        spk_eq_ack_packet(parse_seq, &spk_eq_global_gain[0], 4);
        return 1;
    case CMD_READ_GLOBAL_R:
        spk_eq_ack_packet(parse_seq, &spk_eq_global_gain[1], 4);
        return 1;

    default:
        printf("crc %d\n", pack.crc);
        printf("cmd err %x\n", cmd);
        return -1;
    }
    return 0;
}

int spk_eq_read_seg_l(u8 **buf)
{
    *buf = (u8 *)(&spk_eq_seg_tab[0]);
    return sizeof(spk_eq_seg_tab) / 2;
}

int spk_eq_read_seg_r(u8 **buf)
{
#if AUDIO_SPK_EQ_STERO
    *buf = (u8 *)(&spk_eq_seg_tab[10]);
    return sizeof(spk_eq_seg_tab) / 2;
#else
    return 0;
#endif
}

int spk_eq_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    if (ext_data) {
        parse_seq = ext_data[1];
    } else {
        parse_seq = 0xff;
    }
    printf("parse_seq %x\n", parse_seq);
    int ret = spk_eq_spp_rx_packet(packet, size);
    if (!ret) {
        u8 ack[] = "OK";
        spk_eq_ack_packet(parse_seq, ack, sizeof(ack));
        /* spk_eq_ack_packet(parse_seq, packet, size); */
    } else if (ret != 1) {
        u8 ack[] = "ER";
        spk_eq_ack_packet(parse_seq, ack, sizeof(ack));
        /* spk_eq_ack_packet(parse_seq, packet, size); */
    }
    return 0;
}

void spk_eq_set_send_data_handler(void (*handler)(u8 seq, u8 *packet, u8 size))
{
    send_data_handler = handler;
}

int spk_eq_init(void)
{
#if APP_ONLINE_DEBUG
    app_online_db_register_handle(DB_PKT_TYPE_SPK_EQ, spk_eq_app_online_parse);
#endif
    return 0;
}
__initcall(spk_eq_init);


#endif/*AUDIO_SPK_EQ_CONFIG*/

#if defined(TCFG_AUDIO_BASS_BOOST) && TCFG_AUDIO_BASS_BOOST

#if TCFG_AUDIO_BASS_BOOST_TEST
static u8 bass_boost_run = 1;
u8 test_bass_boost = 1;
#else
static u8 bass_boost_run = 1;
#endif/*TCFG_AUDIO_BASS_BOOST_TEST*/

#if TCFG_AUDIO_BASS_BOOST_TEST
#define TWS_FUNC_ID_BASS_EFF \
	((int)(('A' + '2' + 'D' + 'P') << (2 * 8)) | \
	 (int)(('E' + 'F' + 'F') << (1 * 8)) | \
	 (int)(('S' + 'Y' + 'N' + 'C') << (0 * 8)))
void audio_bass_ctrl()
{
    int state = tws_api_get_tws_state();
    if (state & TWS_STA_SIBLING_CONNECTED) {
        test_bass_boost = !test_bass_boost;
        if (test_bass_boost) { //对耳链接，进入低音增强，播个提示音
            /* tone_play_index(TONE_NORMAL, 1); */
        } else {
        }

        tws_api_send_data_to_sibling((u8 *)&test_bass_boost, sizeof(int), TWS_FUNC_ID_BASS_EFF);
    } else {
        test_bass_boost = !test_bass_boost;
        bass_boost_run = test_bass_boost;

    }
}

static void tws_a2dp_eff_align_bass(void *data, u16 len, bool rx)
{
    int a2dp_eff;
    memcpy(&a2dp_eff, data, sizeof(int));
    bass_boost_run = a2dp_eff;
    test_bass_boost = bass_boost_run;
}

REGISTER_TWS_FUNC_STUB(a2dp_align_bass) = {
    .func_id = TWS_FUNC_ID_BASS_EFF,
    .func    = tws_a2dp_eff_align_bass,
};
#endif/*TCFG_AUDIO_BASS_BOOST_TEST*/

static int bass_boost_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
#if (TCFG_AUDIO_BASS_BOOST && TCFG_DRC_ENABLE)
    static struct drc_ch wdrc_p = {0};
    struct threshold_group low_threshold[] = {//低频
        {0, 0},
        {79, 79},
        {140, 80},
    };

    struct threshold_group mid_threshold[] = {//中频
        {0, 0},
        {82, 82},
        {140, 82}
    };

    wdrc_p.nband = 2;
    wdrc_p.type = 3;//wdrc
    wdrc_p.low_freq = 90; //低中分频点  （20~600hz）
    wdrc_p.high_freq = 2000;//中高分频点   (0~20000hz)
    wdrc_p.order = 2;////分频阶数 2或者4
    int i = 0;
    //低频
    wdrc_p._p.wdrc[0].attacktime = 10;
    wdrc_p._p.wdrc[0].releasetime = 100;
    memcpy(wdrc_p._p.wdrc[0].threshold, low_threshold, sizeof(low_threshold));
    wdrc_p._p.wdrc[0].threshold_num = ARRAY_SIZE(low_threshold);
    wdrc_p._p.wdrc[0].rms_time = 25;
    wdrc_p._p.wdrc[0].algorithm = 0;
    wdrc_p._p.wdrc[0].mode = 1;
    wdrc_p._p.wdrc[0].inputgain = 0;
    wdrc_p._p.wdrc[0].outputgain = 8;

    //高频
    //right
    wdrc_p._p.wdrc[1].attacktime = 10;
    wdrc_p._p.wdrc[1].releasetime = 100;
    memcpy(wdrc_p._p.wdrc[1].threshold, mid_threshold, sizeof(mid_threshold));
    wdrc_p._p.wdrc[1].threshold_num = ARRAY_SIZE(mid_threshold);
    wdrc_p._p.wdrc[1].rms_time = 25;
    wdrc_p._p.wdrc[1].algorithm = wdrc_p._p.wdrc[0].algorithm;
    wdrc_p._p.wdrc[1].mode = wdrc_p._p.wdrc[0].mode;
    wdrc_p._p.wdrc[1].inputgain = wdrc_p._p.wdrc[0].inputgain;
    wdrc_p._p.wdrc[1].outputgain = wdrc_p._p.wdrc[0].outputgain;


    info->R_pch = info->pch = &wdrc_p;
#endif/*TCFG_AUDIO_BASS_BOOST*/
    return 0;
}
void *audio_bass_boost_open(u32 sample_rate, u8 ch_num)
{
    struct audio_drc *bass_boost = NULL;
    struct audio_drc_param drc_param = {0};
    drc_param.sr = sample_rate;
    drc_param.channels = ch_num;
    drc_param.cb = bass_boost_get_filter_info;
    bass_boost = audio_dec_drc_open(&drc_param);
    return bass_boost;
}

void audio_bass_boost_run(void *hdl, s16 *data, int len)
{
    if (!hdl) {
        return;
    }
    if (bass_boost_run) {
        audio_drc_run(hdl, data, len);
    }
}


void audio_bass_boost_close(void *hdl)
{

    if (!hdl) {
        return;
    }
    audio_dec_drc_close(hdl);
}
#endif
