/*
 ****************************************************************
 *							AUDIO SMS(SingleMic System)
 * File  : audio_aec_sms.c
 * By    :
 * Notes : AEC回音消除 + 单mic降噪
 *
 ****************************************************************
 */
#include "aec_user.h"
#include "system/includes.h"
#include "media/includes.h"
#include "application/eq_config.h"
#include "application/audio_drc.h"
#include "application/audio_pitch.h"
#include "circular_buf.h"
#include "overlay_code.h"
#include "audio_aec_online.h"
//#include "audio_aec_debug.c"
#include "audio_config.h"
#include "amplitude_statistic.h"
#include "audio_gain_process.h"
#include "fft_and_pca.h"

#if TCFG_AUDIO_CVP_SYNC
#include "audio_cvp_sync.h"
#endif/*TCFG_AUDIO_CVP_SYNC*/

#if TCFG_AUDIO_CVP_DUT_ENABLE
#include "audio_cvp_dut.h"
#endif /*TCFG_AUDIO_CVP_DUT_ENABLE*/


int audio_cvp_code_load(void)
{
    overlay_load_code(OVERLAY_AEC);
    //printf("Audio_CVP_Code_Load succ\n");
    return 0;
}

#if !defined(TCFG_CVP_DEVELOP_ENABLE) || (TCFG_CVP_DEVELOP_ENABLE == 0)

#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif/* TCFG_USER_TWS_ENABLE */

#if (TCFG_AUDIO_DUAL_MIC_ENABLE == 0) && (TCFG_AUDIO_TRIPLE_MIC_ENABLE == 0)
#include "commproc.h"

#define LOG_TAG_CONST       AEC_USER
#define LOG_TAG             "[AEC_USER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define AEC_USER_MALLOC_ENABLE	1
#define AEC_TOGGLE				TCFG_AEC_ENABLE


#if TCFG_EQ_ENABLE
/*mic去直流滤波eq*/
#define AEC_DCCS_EN			TCFG_AEC_DCCS_EQ_ENABLE
/*mic普通eq*/
#define AEC_UL_EQ_EN		TCFG_AEC_UL_EQ_ENABLE
#else
#define AEC_DCCS_EN			0
#define AEC_UL_EQ_EN		0
#endif/*TCFG_EQ_ENABLE*/

#if TCFG_DRC_ENABLE
#define CVP_UL_DRC_EN       TCFG_CVP_UL_DRC_ENABLE
#else
#define CVP_UL_DRC_EN       0
#endif/*TCFG_DRC_ENABLE*/

extern const int  VC_KINDO_TVM;

/*省电容mic通过eq模块实现去直流滤波*/
#if TCFG_SUPPORT_MIC_CAPLESS
#if ((TCFG_EQ_ENABLE == 0) || (TCFG_AEC_DCCS_EQ_ENABLE == 0))
#error "MicCapless enable,Please enable TCFG_EQ_ENABLE and TCFG_AEC_DCCS_EQ_ENABLE"
#endif/*TCFG_EQ_ENABLE*/
#endif/*TCFG_SUPPORT_MIC_CAPLESS*/

extern struct adc_platform_data adc_data;
extern struct audio_dac_hdl dac_hdl;

#ifdef CONFIG_FPGA_ENABLE
const u8 CONST_AEC_ENABLE = 1;
#define  FPGA_AEC_CLK  (48 * 1000000L)
#else
const u8 CONST_AEC_ENABLE = 1;
#endif/*CONFIG_FPGA_ENABLE*/

#ifdef AUDIO_PCM_DEBUG
/*AEC串口数据导出*/
const u8 CONST_AEC_EXPORT = 1;
#else
const u8 CONST_AEC_EXPORT = 0;
#endif/*AUDIO_PCM_DEBUG*/

//*********************************************************************************//
//                                预处理配置(Pre-process Config)               	   //
//*********************************************************************************//
/*预增益配置*/
#define CVP_PRE_GAIN_ENABLE				0	//算法处理前预加数字增益放大

/*响度指示器*/
#define CVP_LOUDNESS_TRACE_ENABLE		0	//跟踪获取当前mic输入幅值

//*********************************************************************************//
//                                调试配置(Debug Config)                           //
//*********************************************************************************//
/*使能即可跟踪通话过程的内存情况*/
#define CVP_MEM_TRACE_ENABLE	0

/*
 *延时估计使能
 *点烟器需要做延时估计
 *其他的暂时不需要做
 */
const u8 CONST_AEC_DLY_EST = 0;

/*
 *ANS等级:0~2,
 *等级1比等级0多6k左右的ram
 *等级2比等级1多3k左右的ram:优化了连续说话变小声问题
 */
const u8 CONST_ANS_MODE = 2;

/*
 *ANS版本配置
 *ANS_V100:传统降噪
 *ANS_V200:AI降噪，需要更多的ram和mips
 **/
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
const u8 CONST_ANS_VERSION = ANS_V100;
#else
const u8 CONST_ANS_VERSION = ANS_V200;
#endif/*TCFG_AUDIO_CVP_NS_MODE*/

//*********************************************************************************//
//                                DNS配置                                          //
//*********************************************************************************//
/* SNR估计,可以实现场景适配 */
const u8 CONST_DNS_SNR_EST = 0;

/* DNS后处理 */
const u8 CONST_DNS_POST_ENHANCE = 0;

//*******************************DNS配置 end**************************************//

/*参考数据变采样处理*/
#if TCFG_USB_MIC_CVP_ENABLE
const u8 CONST_REF_SRC = 1;
#else
const u8 CONST_REF_SRC = 0;
#endif /*TCFG_USB_MIC_CVP_ENABLE*/

/*Splittingfilter模式：0 or 1
 *mode = 0:运算量和ram小，高频会跌下来
 *mode = 1:运算量和ram大，频响正常（过认证，选择模式1）
 */
const u8 CONST_SPLIT_FILTER_MODE = 0;

/*
 *AEC复杂等级，等级越高，ram和mips越大，适应性越好
 *回音路径不定/回音失真等情况才需要比较高的等级
 *音箱建议使用等级:5
 *耳机建议使用等级:2
 */
#define AEC_TAIL_LENGTH			2 /*range:2~10,default:2*/

#if TCFG_AEC_SIMPLEX
/*限幅器-噪声门限*/
const u8 CONST_AEC_SIMPLEX = 1;
#else
const u8 CONST_AEC_SIMPLEX = 0;
#endif/*TCFG_AEC_SIMPLEX*/
/*单工连续清0的帧数*/
#define AEC_SIMPLEX_TAIL 	15
/**远端数据大于CONST_AEC_SIMPLEX_THR,即清零近端数据
 *越小，回音限制得越好，同时也就越容易卡*/
#define AEC_SIMPLEX_THR		100000	/*default:260000*/

/*数据输出开头丢掉的数据包数*/
#define AEC_OUT_DUMP_PACKET		15
/*数据输出开头丢掉的数据包数*/
#define AEC_IN_DUMP_PACKET		1

__attribute__((weak))u32 usb_mic_is_running()
{
    return 0;
}

/*复用lmp rx buf(一般通话的时候复用)
 *rx_buf概率产生碎片，导致alloc失败，因此默认配0
 */
#define MALLOC_MULTIPLEX_EN		0
extern void *lmp_malloc(int);
extern void lmp_free(void *);
void *zalloc_mux(int size)
{
#if MALLOC_MULTIPLEX_EN
    void *p = NULL;
    do {
        p = lmp_malloc(size);
        if (p) {
            break;
        }
        printf("aec_malloc wait...\n");
        os_time_dly(2);
    } while (1);
    if (p) {
        memset(p, 0, size);
    }
    printf("[malloc_mux]p = 0x%x,size = %d\n", p, size);
    return p;
#else
    return zalloc(size);
#endif
}

void free_mux(void *p)
{
#if MALLOC_MULTIPLEX_EN
    printf("[free_mux]p = 0x%x\n", p);
    lmp_free(p);
#else
    free(p);
#endif
}

struct audio_aec_hdl {
    u8 start;				//aec模块状态
    u8 inbuf_clear_cnt;		//aec输入数据丢掉
    u8 output_fade_in;		//aec输出淡入使能
    u8 output_fade_in_gain;	//aec输出淡入增益
    u8 EnableBit;			//aec使能模块
    u8 input_clear;			//清0输入数据标志
    u16 dump_packet;		//前面如果有杂音，丢掉几包
    u8 output_buf[1000];	//aec数据输出缓存
    cbuffer_t output_cbuf;
#if AEC_UL_EQ_EN
    struct audio_eq *ul_eq;
#endif/*AEC_UL_EQ_EN*/
#if AEC_DCCS_EN
    struct audio_eq *dccs_eq;
#endif/*AEC_DCCS_EN*/
#if CVP_UL_DRC_EN
    struct audio_drc *ul_drc;
#endif/*CVP_UL_DRC_EN*/
    struct aec_s_attr attr;
#if AEC_PITCHSHIFTER_CONFIG
    s_pitch_hdl *pitch ; //变声
    u8 pitch_mode_L; //记录变声模式
    u8 pitch_mode_R; //记录变声模式
#endif/* AEC_PITCHSHIFTER_CONFIG */
};
#if AEC_USER_MALLOC_ENABLE
struct audio_aec_hdl *aec_hdl = NULL;
#else
struct audio_aec_hdl aec_handle;
struct audio_aec_hdl *aec_hdl = &aec_handle;
#endif/*AEC_USER_MALLOC_ENABLE*/

extern int esco_adc_mic_en();
void audio_aec_ref_start(u8 en)
{
    if (aec_hdl) {
        if (en != aec_hdl->attr.fm_tx_start) {
            if (esco_adc_mic_en() == 0) {
                aec_hdl->attr.fm_tx_start = en;
                y_printf("fm_tx_start:%d\n", en);
            }
        }
    }
}

#if AEC_DCCS_EN
//一段高通滤波器 可调中心截止频率、带宽
// 默认   /*freq:100*/   /*quality:0.7f*/
const struct eq_seg_info dccs_eq_tab_8k[] = {
    {0, EQ_IIR_TYPE_HIGH_PASS, 100,   0, 0.7f},
};
const struct eq_seg_info dccs_eq_tab_16k[] = {
    {0, EQ_IIR_TYPE_HIGH_PASS, 100,   0, 0.7f},
};

static float dccs_tab[5];
int aec_dccs_eq_filter(void *eq, int sr, struct audio_eq_filter_info *info)
{
    if (!sr) {
        sr = 16000;
    }
    u8 nsection = ARRAY_SIZE(dccs_eq_tab_8k);
    local_irq_disable();
    if (sr == 16000) {
        for (int i = 0; i < nsection; i++) {
            eq_seg_design((struct eq_seg_info *)&dccs_eq_tab_16k[i], sr, &dccs_tab[5 * i]);
        }
    } else {
        for (int i = 0; i < nsection; i++) {
            eq_seg_design((struct eq_seg_info *)&dccs_eq_tab_8k[i], sr, &dccs_tab[5 * i]);
        }
    }
    local_irq_enable();

    info->L_coeff = (float *)dccs_tab;
    info->R_coeff = (float *)dccs_tab;
    info->L_gain = 0;
    info->R_gain = 0;
    info->nsection = nsection;
    return 0;
}
#endif/*AEC_DCCS_EN*/

#if CVP_UL_DRC_EN
static float Gain_dB = 0;
int cvp_ul_drc_filter(struct audio_drc *drc, struct audio_drc_filter_info *info)
{
    static struct drc_ch wdrc_p = {0};
    struct threshold_group threshold[3] = {{0, 0}, {75.f, 75.f}, {140.f, 75.f}};    //这里减去90才是具体的db数
    /*倍数对应放大的dB数：
     * 20lg(2) = 6dB
     * 20lg(3) = 9.5dB
     * 20lg(4) = 12dB
     * 20lg(5) = 14dB
     * 20lg(6) = 15.5dB
     * 20lg(7) = 17dB
     * 20lg(8) = 18dB
     * 20lg(9) = 19dB
     * 20lg(10) = 20dB*/
    Gain_dB = 4.0f; /*drc压制后要放大的倍数*/

    wdrc_p.nband = 1;
    wdrc_p.type = 3;//wdrc

    //left
    int i = 0;
    wdrc_p._p.wdrc[i][0].attacktime = 1;
    wdrc_p._p.wdrc[i][0].releasetime = 200;
    memcpy(wdrc_p._p.wdrc[i][0].threshold, threshold, sizeof(threshold));
    wdrc_p._p.wdrc[i][0].threshold_num = ARRAY_SIZE(threshold);
    wdrc_p._p.wdrc[i][0].rms_time = 25;
    wdrc_p._p.wdrc[i][0].algorithm = 0;
    wdrc_p._p.wdrc[i][0].mode = 1;
    //right
    wdrc_p._p.wdrc[i][1].attacktime = 1;
    wdrc_p._p.wdrc[i][1].releasetime = 200;
    memcpy(wdrc_p._p.wdrc[i][1].threshold, threshold, sizeof(threshold));
    wdrc_p._p.wdrc[i][1].threshold_num = ARRAY_SIZE(threshold);
    wdrc_p._p.wdrc[i][1].rms_time = 25;
    wdrc_p._p.wdrc[i][1].algorithm = wdrc_p._p.wdrc[i][0].algorithm;
    wdrc_p._p.wdrc[i][1].mode = wdrc_p._p.wdrc[i][0].mode;
    info->R_pch = info->pch = &wdrc_p;
    return 0;
}
#endif/*CVP_UL_DRC_EN*/

/*
*********************************************************************
*                  Audio AEC Process_Probe
* Description: AEC模块数据前处理回调(预处理)
* Arguments  : talk_mic     主麦数据地址
*			   talk_ref_mic	单麦没有用到，为NULL
*			   fb_mic	    单麦没有用到，为NULL
*			   ref	参考数据地址
*			   len	数据长度(Unit:byte)
* Return	 : 0 成功 其他 失败
* Note(s)    : 在源数据经过CVP模块前，可以增加自定义处理
*********************************************************************
*/
static LOUDNESS_M_STRUCT mic_loudness;
static int audio_aec_probe(short *talk_mic, short *talk_ref_mic, short *fb_mic, short *ref, u16 len)
{
#if CVP_PRE_GAIN_ENABLE
    GainProcess_16Bit(talk_mic, talk_mic, 12.f, 1, 1, 1, len >> 1);
#endif/*CVP_PRE_GAIN_ENABLE*/

#if CVP_LOUDNESS_TRACE_ENABLE
    loudness_meter_short(&mic_loudness, talk_mic, len >> 1);
#endif/*CVP_LOUDNESS_TRACE_ENABLE*/

#if AEC_DCCS_EN
    if (aec_hdl->dccs_eq) {
        audio_eq_run(aec_hdl->dccs_eq, talk_mic, len);
    }
#endif/*AEC_DCCS_EN*/
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Process_Post
* Description: AEC模块数据后处理回调
* Arguments  : data 数据地址
*			   len	数据长度
* Return	 : 0 成功 其他 失败
* Note(s)    : 在数据处理完毕，可以增加自定义后处理
*********************************************************************
*/
static int audio_aec_post(s16 *data, u16 len)
{
#if AEC_UL_EQ_EN
    if (aec_hdl->ul_eq) {
        audio_eq_run(aec_hdl->ul_eq, data, len);
    }
#endif/*AEC_UL_EQ_EN*/
#if CVP_UL_DRC_EN
    if (aec_hdl->ul_drc) {
        audio_drc_run(aec_hdl->ul_drc, data, len);
        GainProcess_16Bit(data, data, Gain_dB, 1, 1, 1, len >> 1);
    }
#endif/*CVP_UL_DRC_EN*/
#if AEC_PITCHSHIFTER_CONFIG
    if (aec_hdl && aec_hdl->pitch) {
        pitch_run(aec_hdl->pitch, data, data, len, 1);
    }
#endif/* AEC_PITCHSHIFTER_CONFIG */
    return 0;
}

int audio_aec_update(u8 EnableBit)
{
#if (TCFG_KWS_VOICE_RECOGNITION_ENABLE == 1)
    extern u8 kws_is_running(void);
    if (kws_is_running()) { /*当是KWS调用的，使用默认参数的EnableBit恢复clk*/
        EnableBit = aec_hdl->attr.EnableBit;
    }
#endif /*TCFG_KWS_VOICE_RECOGNITION_ENABLE*/
    printf("aec_update,wideband:%d,EnableBit:%x", aec_hdl->attr.wideband, EnableBit);
    if (CONST_ANS_VERSION == ANS_V200) {
        clk_set("sys", CONFIG_BT_CALL_DNS_HZ);
        return 0;
    }
    if (aec_hdl->attr.wideband) {
        if ((EnableBit & 0x7) == AEC_MODE_ADVANCE) {
            clk_set("sys", BT_CALL_16k_ADVANCE_HZ);
        } else {
            clk_set("sys", BT_CALL_16k_HZ);
        }
    } else {
        if ((EnableBit & 0x7) == AEC_MODE_ADVANCE) {
            clk_set("sys", BT_CALL_ADVANCE_HZ);
        } else {
            clk_set("sys", BT_CALL_HZ);
        }
    }
    return 0;
}

/*跟踪系统内存使用情况:physics memory size xxxx bytes*/
static void sys_memory_trace(void)
{
    static int cnt = 0;
    if (cnt++ > 200) {
        cnt = 0;
        mem_stats();
    }
}

extern void esco_enc_resume(void);
#if TCFG_AUDIO_CVP_SYNC
int audio_aec_sync_buffer_set(s16 *data, int len)
{
    u16 wlen = cbuf_write(&aec_hdl->output_cbuf, data, len);
    /* printf("wlen:%d-%d\n",len,aec_hdl->output_cbuf.data_len); */
    esco_enc_resume();
    return wlen;
}
#endif/*TCFG_AUDIO_CVP_SYNC*/
/*
*********************************************************************
*                  Audio AEC Output Handle
* Description: AEC模块数据输出回调
* Arguments  : data 输出数据地址
*			   len	输出数据长度
* Return	 : 数据输出消耗长度
* Note(s)    : None.
*********************************************************************
*/
static int audio_aec_output(s16 *data, u16 len)
{
#if ((defined TCFG_KWS_VOICE_RECOGNITION_ENABLE) && TCFG_KWS_VOICE_RECOGNITION_ENABLE)
    //Voice Recognition get mic data here
    extern void kws_aec_data_output(void *priv, s16 * data, int len);
    kws_aec_data_output(NULL, data, len);
#endif/*TCFG_KWS_VOICE_RECOGNITION_ENABLE*/

#if CVP_MEM_TRACE_ENABLE
    sys_memory_trace();
#endif/*CVP_MEM_TRACE_ENABLE*/

    if (aec_hdl->dump_packet) {
        aec_hdl->dump_packet--;
        memset(data, 0, len);
    } else  {
        if (aec_hdl->output_fade_in) {
            s32 tmp_data;
            //printf("fade:%d\n",aec_hdl->output_fade_in_gain);
            for (int i = 0; i < len / 2; i++) {
                tmp_data = data[i];
                data[i] = tmp_data * aec_hdl->output_fade_in_gain >> 7;
            }
            aec_hdl->output_fade_in_gain += 12;
            if (aec_hdl->output_fade_in_gain >= 128) {
                aec_hdl->output_fade_in = 0;
            }
        }
    }
#if TCFG_AUDIO_CVP_SYNC
    audio_cvp_sync_run(data, len);
    return len;
#endif/*TCFG_AUDIO_CVP_SYNC*/

    u16 wlen = cbuf_write(&aec_hdl->output_cbuf, data, len);
    //printf("wlen:%d-%d\n",len,aec_hdl.output_cbuf.data_len);
    esco_enc_resume();
#if 1
    static u32 aec_output_max = 0;
    if (aec_output_max < aec_hdl->output_cbuf.data_len) {
        aec_output_max = aec_hdl->output_cbuf.data_len;
        y_printf("o_max:%d", aec_output_max);
    }
#endif
    if (wlen != len) {
        putchar('f');
    }
    return wlen;
}

/*
*********************************************************************
*                  Audio AEC Output Read
* Description: 读取aec模块的输出数据
* Arguments  : buf  读取数据存放地址
*			   len	读取数据长度
* Return	 : 数据读取长度
* Note(s)    : None.
*********************************************************************
*/
int audio_aec_output_read(s16 *buf, u16 len)
{
    //printf("rlen:%d-%d\n",len,aec_hdl.output_cbuf.data_len);
    local_irq_disable();
    if (!aec_hdl || !aec_hdl->start) {
        printf("audio_aec close now");
        local_irq_enable();
        return -EINVAL;
    }
    u16 rlen = cbuf_read(&aec_hdl->output_cbuf, buf, len);
    if (rlen == 0) {
        //putchar('N');
    }
    local_irq_enable();
    return rlen;
}

static const char *CVP_ModuleName[] = {
    "AEC", "NLP", "ANS", "ENC", "AGC"
};
static const char *CVP_ModuleStatus[] = {
    "Disable", "Enable"
};
static void dumpModulaStatus(int EnableBit)
{
    printf("EnableBitDebug = 0x%x\n", EnableBit);
    for (int i = 0; i < 5; i++) {
        int k = 1 << i;
        printf("%s : %s\n", CVP_ModuleName[i], CVP_ModuleStatus[((EnableBit & k) != 0)]);
    }
}

/*
*********************************************************************
*                  Audio AEC Parameters
* Description: AEC模块配置参数
* Arguments  : p	参数指针
* Return	 : None.
* Note(s)    : 读取配置文件成功，则使用配置文件的参数配置，否则使用默
*			   认参数配置
*********************************************************************
*/
static void audio_aec_param_init(struct aec_s_attr *p)
{
    int ret = 0;
    AEC_CONFIG cfg;
#if TCFG_AEC_TOOL_ONLINE_ENABLE
    ret = aec_cfg_online_update_fill(&cfg, sizeof(AEC_CONFIG));
#endif/*TCFG_AEC_TOOL_ONLINE_ENABLE*/
    if (ret == 0) {
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
        ret = syscfg_read(CFG_AEC_ID, &cfg, sizeof(AEC_CONFIG));
#else
        ret = syscfg_read(CFG_SMS_DNS_ID, &cfg, sizeof(AEC_CONFIG));
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
    }
    log_info("CVP_NS_MODE = %d\n", TCFG_AUDIO_CVP_NS_MODE);
    if (ret == sizeof(AEC_CONFIG)) {
        log_info("audio_aec read config ok\n");
        p->AGC_NDT_fade_in_step = cfg.ndt_fade_in;
        p->AGC_NDT_fade_out_step = cfg.ndt_fade_out;
        p->AGC_DT_fade_in_step = cfg.dt_fade_in;
        p->AGC_DT_fade_out_step = cfg.dt_fade_out;
        p->AGC_NDT_max_gain = cfg.ndt_max_gain;
        p->AGC_NDT_min_gain = cfg.ndt_min_gain;
        p->AGC_NDT_speech_thr = cfg.ndt_speech_thr;
        p->AGC_DT_max_gain = cfg.dt_max_gain;
        p->AGC_DT_min_gain = cfg.dt_min_gain;
        p->AGC_DT_speech_thr = cfg.dt_speech_thr;
        p->AGC_echo_present_thr = cfg.echo_present_thr;
        p->AEC_DT_AggressiveFactor = cfg.aec_dt_aggress;
        p->AEC_RefEngThr = cfg.aec_refengthr;
        p->ES_AggressFactor = cfg.es_aggress_factor;
        p->ES_MinSuppress = cfg.es_min_suppress;
        p->ES_Unconverge_OverDrive = cfg.es_min_suppress;
        p->ANS_AggressFactor = cfg.ans_aggress;
        p->ANS_MinSuppress = cfg.ans_suppress;
        p->DNS_OverDrive = cfg.ans_aggress;
        p->DNS_GainFloor = cfg.ans_suppress;
        p->DNS_highGain = 2.5f;
        p->DNS_rbRate = 0.3f;
        // dumpModulaStatus(cfg.aec_mode);
        p->EnableBit = (cfg.aec_mode & AEC_MODE_ADVANCE);
        if (p->EnableBit == 0) {
            p->toggle = 0;
            printf("cvp toggle off\n");
        } else {
            p->toggle = 1;
        }
#if (CVP_UL_DRC_EN == 1)/*开通话上行DRC时，不开AGC*/
        p->agc_en = 0;
#else
        p->agc_en = (cfg.aec_mode & AGC_EN) ? 1 : 0;
#endif/*CVP_UL_DRC_EN*/
        p->ul_eq_en = cfg.ul_eq_en;
        printf("aec_mode:%x,agc_en:%d,ul_eq_en:%d\n", cfg.aec_mode, p->agc_en, p->ul_eq_en);
        //aec_param_dump(p);
    } else {
        log_error("read audio_aec param err:%x", ret);
        p->toggle = 1;
        p->EnableBit = AEC_MODE_REDUCE;
        p->wideband = 0;
        p->ul_eq_en = 1;

        /*AGC*/
#if (CVP_UL_DRC_EN == 1)/*开通话上行DRC时，不开AGC*/
        p->agc_en = 0;
#else
        p->agc_en = 1;
#endif/*CVP_UL_DRC_EN*/
        p->AGC_NDT_fade_in_step = 1.3f;
        p->AGC_NDT_fade_out_step = 0.9f;
        p->AGC_DT_fade_in_step = 1.3f;
        p->AGC_DT_fade_out_step = 0.9f;
        p->AGC_NDT_max_gain = 12.f;
        p->AGC_NDT_min_gain = 0.f;
        p->AGC_NDT_speech_thr = -50.f;
        p->AGC_DT_max_gain = 12.f;
        p->AGC_DT_min_gain = 0.f;
        p->AGC_DT_speech_thr = -40.f;
        p->AGC_echo_present_thr = -70.f;

        /*AEC*/
        p->AEC_DT_AggressiveFactor = 1.f;	/*范围：1~5，越大追踪越好，但会不稳定,如破音*/
        p->AEC_RefEngThr = -70.f;

        /*ES*/
        p->ES_AggressFactor = -3.0f;
        p->ES_MinSuppress = 4.f;
        p->ES_Unconverge_OverDrive = p->ES_MinSuppress;

        /*ANS*/
        p->ANS_AggressFactor = 1.25f;	/*范围：1~2,动态调整,越大越强(1.25f)*/
        p->ANS_MinSuppress = 0.04f;	/*范围：0~1,静态定死最小调整,越小越强(0.09f)*/

        /*DNS*/
        p->DNS_GainFloor = 0.1f; /*增益最小值控制，范围：0~1.0，越小降噪越强，默认0.1，建议值:0~0.2*/
        p->DNS_OverDrive = 1.0f;   /*控制降噪强度，范围：0~3.0，越大降噪越强，正常降噪时为1.0，建议调节范围:0.3~3*/
        p->DNS_highGain = 2.5f; /*EQ强度, 范围：1.0f~3.5f,越大越强*/
        p->DNS_rbRate = 0.3f;   /*混响强度，范围：0~0.9f,越大越强*/
    }
    p->ANS_mode = 1;
    p->ANS_NoiseLevel = 2.2e3f;
    p->wn_gain = 331;
    p->SimplexTail = AEC_SIMPLEX_TAIL;
    p->SimplexThr = AEC_SIMPLEX_THR;
    p->dly_est = 0;
    p->dst_delay = 50;
    p->AGC_echo_look_ahead = 0;
    p->AGC_echo_hold = 0;
    p->packet_dump = 50;/*0~255(u8)*/
    p->aec_tail_length = AEC_TAIL_LENGTH;
}

#define CLEAR_FRAME  5    // 用来丢掉前几帧数据
/*
*********************************************************************
*                  Audio AEC Open
* Description: 初始化AEC模块
* Arguments  : sr 			采样率(8000/16000)
*			   enablebit	使能模块(AEC/NLP/AGC/ANS...)
*			   out_hdl		自定义回调函数，NULL则用默认的回调
* Return	 : 0 成功 其他 失败
* Note(s)    : 该接口是对audio_aec_init的扩展，支持自定义使能模块以及
*			   数据输出回调函数
*********************************************************************
*/
int audio_aec_open(u16 sample_rate, s16 enablebit, int (*out_hdl)(s16 *data, u16 len))
{
    struct aec_s_attr *aec_param;
    printf("audio_aec_open\n");
    mem_stats();
#if AEC_USER_MALLOC_ENABLE
    aec_hdl = zalloc(sizeof(struct audio_aec_hdl));
    if (aec_hdl == NULL) {
        log_error("aec_hdl malloc failed");
        return -ENOMEM;
    }
#endif/*AEC_USER_MALLOC_ENABLE*/

#if TCFG_AUDIO_CVP_SYNC
    audio_cvp_sync_open(sample_rate);
#endif/*TCFG_AUDIO_CVP_SYNC*/

#if CVP_LOUDNESS_TRACE_ENABLE
    loudness_meter_init(&mic_loudness, sample_rate, 50, 0);
#endif/*CVP_LOUDNESS_TRACE_ENABLE*/

    cbuf_init(&aec_hdl->output_cbuf, aec_hdl->output_buf, sizeof(aec_hdl->output_buf));
    aec_hdl->dump_packet = AEC_OUT_DUMP_PACKET;
    aec_hdl->inbuf_clear_cnt = AEC_IN_DUMP_PACKET;
    aec_hdl->output_fade_in = 1;
    aec_hdl->output_fade_in_gain = 0;
    aec_param = &aec_hdl->attr;
    aec_param->aec_probe = audio_aec_probe;
    aec_param->aec_post = audio_aec_post;
#if TCFG_AEC_TOOL_ONLINE_ENABLE
    aec_param->aec_update = audio_aec_update;
#endif/*TCFG_AEC_TOOL_ONLINE_ENABLE*/
    aec_param->output_handle = audio_aec_output;
    aec_param->far_noise_gate = 10;
    aec_param->ref_sr  = usb_mic_is_running();
    if (aec_param->ref_sr == 0) {
        aec_param->ref_sr = sample_rate;
    }

    aec_input_clear_enable(CLEAR_FRAME);
    audio_aec_param_init(aec_param);
    if (enablebit >= 0) {
        aec_param->EnableBit = enablebit;
    }
    if (out_hdl) {
        aec_param->output_handle = out_hdl;
    }

#if TCFG_AUDIO_OUTPUT_IIS
    aec_param->output_way = 1;
#endif

#if TCFG_AEC_SIMPLEX
    aec_param->wn_en = 1;
    aec_param.EnableBit = AEC_MODE_SIMPLEX;
    if (sr == 8000) {
        aec_param.SimplexTail = aec_param.SimplexTail / 2;
    }
#else
    aec_param->wn_en = 0;
#endif/*TCFG_AEC_SIMPLEX*/

    /*根据清晰语音处理模块配置，配置相应的系统时钟*/
    int aec_clock = BT_CALL_16k_HZ;
    if (sample_rate == 16000) {
        /*宽带wide-band*/
        aec_param->wideband = 1;
        aec_param->hw_delay_offset = 60;
        if (aec_param->EnableBit == AEC_MODE_ADVANCE) {
            aec_clock = BT_CALL_16k_ADVANCE_HZ;
        } else {
            aec_clock = BT_CALL_16k_HZ;
        }
    } else {
        /*窄带narrow-band*/
        aec_param->wideband = 0;
        aec_param->hw_delay_offset = 55;
        if (aec_param->EnableBit == AEC_MODE_ADVANCE) {
            aec_clock = BT_CALL_ADVANCE_HZ;
        } else {
            aec_clock = BT_CALL_HZ;
        }
    }

    if (CONST_ANS_VERSION == ANS_V200) {
        aec_clock = CONFIG_BT_CALL_DNS_HZ;
    }

#ifdef CONFIG_FPGA_ENABLE
    aec_clock = FPGA_AEC_CLK;
#endif/*CONFIG_FPGA_ENABLE*/
    clk_set("sys", aec_clock);

#if AEC_UL_EQ_EN
    if (aec_param->ul_eq_en) {
        struct audio_eq_param ul_eq_param = {0};
        ul_eq_param.sr = sample_rate;
        ul_eq_param.channels = 1;
        ul_eq_param.online_en = 1;
        ul_eq_param.mode_en = 0;
        ul_eq_param.remain_en = 0;
        ul_eq_param.max_nsection = EQ_SECTION_MAX;
        ul_eq_param.cb = aec_ul_eq_filter;
        ul_eq_param.eq_name = aec_eq_mode;
        aec_hdl->ul_eq = audio_dec_eq_open(&ul_eq_param);
    }
#endif/*AEC_UL_EQ_EN*/

#if AEC_DCCS_EN
    if (adc_data.mic_mode == AUDIO_MIC_CAPLESS_MODE) {
        struct audio_eq_param dccs_eq_param = {0};
        dccs_eq_param.sr = sample_rate;
        dccs_eq_param.channels = 1;
        dccs_eq_param.online_en = 0;
        dccs_eq_param.mode_en = 0;
        dccs_eq_param.remain_en = 0;
        dccs_eq_param.max_nsection = EQ_SECTION_MAX;
        dccs_eq_param.cb = aec_dccs_eq_filter;
        aec_hdl->dccs_eq = audio_dec_eq_open(&dccs_eq_param);
    }
#endif/*AEC_DCCS_EN*/

#if CVP_UL_DRC_EN
    printf("audio cvp ul drc open start\n");
    struct audio_drc_param ul_drc_param = {0};
    ul_drc_param.channels = 1;
    ul_drc_param.sr = sample_rate;
    ul_drc_param.out_32bit = 0;
    /* ul_drc_param.online_en = 0; */
    /* ul_drc_param.remain_en = 0; */
    /* ul_drc_param.stero_div = 0; */
    ul_drc_param.cb = cvp_ul_drc_filter;
    ul_drc_param.drc_name = 0;
    u8 index = 0;
    if (sample_rate == 8000) { //窄频
        index = 1;
    }
    /* ul_drc_param.wdrc = &phone_mode[index].drc_parm; */
    aec_hdl->ul_drc = audio_dec_drc_open(&ul_drc_param);
    if (aec_hdl->ul_drc) {
        printf("audio cvp ul drc open succ %x\n", aec_hdl->ul_drc);
    } else {
        printf("audio cvp ul drc open fail \n");
    }
#endif/*CVP_UL_DRC_EN*/

#if AEC_PITCHSHIFTER_CONFIG
    PITCH_SHIFT_PARM sparm = {0};
    sparm.sr = sample_rate;
    sparm.shiftv =  100;
    sparm.effect_v =  EFFECT_PITCH_SHIFT;
    sparm.formant_shift = 100;
    aec_hdl->pitch = open_pitch(&sparm);
    pause_pitch(aec_hdl->pitch, 1);
    aec_hdl->pitch_mode_L = 0;
    aec_hdl->pitch_mode_R = 0;
#endif/* AEC_PITCHSHIFTER_CONFIG */
    //aec_param_dump(aec_param);
    aec_hdl->EnableBit = aec_param->EnableBit;
#if ((defined TCFG_KWS_VOICE_RECOGNITION_ENABLE) && TCFG_KWS_VOICE_RECOGNITION_ENABLE)
    extern u8 kws_get_state(void);
    if (kws_get_state()) {
        aec_param->EnableBit = AEC_EN;
        aec_param->agc_en = 0;
        printf("kws open,aec_enablebit=%x", aec_param->EnableBit);
        //临时关闭aec, 对比测试
        //aec_param->toggle = 0;
    }
#endif/*TCFG_KWS_VOICE_RECOGNITION_ENABLE*/

#if AEC_TOGGLE
    int ret = aec_open(aec_param);
    ASSERT(ret != -EPERM, "Chip not support aec mode!!");
#endif/*AEC_TOGGLE*/
    aec_hdl->start = 1;
    mem_stats();
    printf("audio_aec_open succ\n");
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Init
* Description: 初始化AEC模块
* Arguments  : sample_rate 采样率(8000/16000)
* Return	 : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_aec_init(u16 sample_rate)
{
    return audio_aec_open(sample_rate, -1, NULL);
}

/*
*********************************************************************
*                  Audio AEC Reboot
* Description: AEC模块复位接口
* Arguments  : reduce 复位/恢复标志
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_aec_reboot(u8 reduce)
{
    if (aec_hdl) {
        printf("audio_aec_reboot:%x,%x,start:%d", aec_hdl->EnableBit, aec_hdl->attr.EnableBit, aec_hdl->start);
        if (aec_hdl->start) {
            if (reduce) {
                aec_hdl->attr.EnableBit = AEC_EN;
                aec_hdl->attr.agc_en = 0;
                aec_reboot(aec_hdl->attr.EnableBit);
            } else {
                if (aec_hdl->EnableBit != aec_hdl->attr.EnableBit) {
                    aec_hdl->attr.agc_en = 1;
                    aec_reboot(aec_hdl->EnableBit);
                }
            }
        }
    } else {
        printf("audio_aec close now\n");
    }
}

#if TCFG_SIDETONE_ENABLE
static s16 mic0_data[3][256];
static int mic0_data_offset = 0;
static int mic0_data_bulk = 0;
static s16 mic1_data[3][256];
static int mic1_data_offset = 0;
static int mic1_data_bulk = 0;
#endif/*TCFG_SIDETONE_ENABLE*/
/*
*********************************************************************
*                  Audio AEC Close
* Description: 关闭AEC模块
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_aec_close(void)
{
    printf("audio_aec_close:%x", (u32)aec_hdl);
    if (aec_hdl) {

        aec_hdl->start = 0;
#if AEC_TOGGLE
        aec_close();
#endif

#if TCFG_AUDIO_CVP_SYNC
        //在AEC关闭之后再关，否则还会跑CVP_SYNC_RUN,导致越界
        audio_cvp_sync_close();
#endif/*TCFG_AUDIO_CVP_SYNC*/

#if AEC_DCCS_EN
        if (aec_hdl->dccs_eq) {
            audio_dec_eq_close(aec_hdl->dccs_eq);
            aec_hdl->dccs_eq = NULL;
        }
#endif/*AEC_DCCS_EN*/
#if AEC_UL_EQ_EN
        if (aec_hdl->ul_eq) {
            audio_dec_eq_close(aec_hdl->ul_eq);
            aec_hdl->ul_eq = NULL;
        }
#endif/*AEC_UL_EQ_EN*/
#if CVP_UL_DRC_EN
        if (aec_hdl->ul_drc) {
            audio_dec_drc_close(aec_hdl->ul_drc);
            aec_hdl->ul_drc = NULL;
        }
#endif/*CVP_UL_DRC_EN*/

#if AEC_PITCHSHIFTER_CONFIG
        if (aec_hdl && aec_hdl->pitch) {
            close_pitch(aec_hdl->pitch);
            aec_hdl->pitch = NULL;
        }
#endif/* AEC_PITCHSHIFTER_CONFIG */
        local_irq_disable();
#if AEC_USER_MALLOC_ENABLE
        free(aec_hdl);
#endif/*AEC_USER_MALLOC_ENABLE*/
        aec_hdl = NULL;
        local_irq_enable();

#if TCFG_SIDETONE_ENABLE
        mic0_data_offset = 0;
        mic0_data_bulk = 0;
        mic1_data_offset = 0;
        mic1_data_bulk = 0;
#endif/*TCFG_SIDETONE_ENABLE*/
    }
}

/*
*********************************************************************
*                  Audio AEC Status
* Description: AEC模块当前状态
* Arguments  : None.
* Return	 : 0 关闭 其他 打开
* Note(s)    : None.
*********************************************************************
*/
u8 audio_aec_status(void)
{
    if (aec_hdl) {
        return aec_hdl->start;
    }
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Input
* Description: AEC源数据输入
* Arguments  : buf	输入源数据地址
*			   len	输入源数据长度
* Return	 : None.
* Note(s)    : 输入一帧数据，唤醒一次运行任务处理数据，默认帧长256点
*********************************************************************
*/
void audio_aec_inbuf(s16 *buf, u16 len)
{
    u16 data_len = len;
    if (aec_hdl && aec_hdl->start) {
        if (aec_hdl->input_clear) {
            aec_hdl->input_clear--;
            memset(buf, 0, len);
        }
#if TCFG_AUDIO_CVP_DUT_ENABLE
        if (cvp_dut_mode_get() == CVP_DUT_MODE_BYPASS) {
            audio_aec_output(buf, len);
            return;
        }
#endif/*TCFG_AUDIO_CVP_DUT_ENABLE*/
#if TCFG_SIDETONE_ENABLE
        if (mic0_data_bulk > 2) {
            mic0_data_bulk = 0;
        }
        if (mic0_data_offset > (512 - len)) {
            mic0_data_offset = 0;
        }
        memcpy((((u32)&mic0_data[mic0_data_bulk][0]) + mic0_data_offset), buf, len);
        data_len = 512;
        buf = (s16 *)(&mic0_data[mic0_data_bulk]);
        mic0_data_offset += len;
        if (mic0_data_offset <= (512 - len)) {
            return;
        }
        mic0_data_bulk++;

#endif/*TCFG_SIDETONE_ENABLE*/

#if AEC_TOGGLE
        if (aec_hdl->inbuf_clear_cnt) {
            aec_hdl->inbuf_clear_cnt--;
            memset(buf, 0, data_len);
        }

        int ret = aec_in_data(buf, data_len);
        if (ret == -1) {
        } else if (ret == -2) {
            log_error("aec inbuf full\n");
            printf("clk : %d\n", clk_get("sys"));
        }
#else	/*不经算法，直通到输出*/
        audio_aec_output(buf, data_len);
#endif/*AEC_TOGGLE*/
    }
}

/*
*********************************************************************
*                  Audio AEC Input Reference
* Description: AEC源参考数据输入
* Arguments  : buf	输入源数据地址
*			   len	输入源数据长度
* Return	 : None.
* Note(s)    : 双mic ENC的参考mic数据输入,单mic的无须调用该接口
*********************************************************************
*/
void audio_aec_inbuf_ref(s16 *buf, u16 len)
{
    u16 data_len = len;

#if TCFG_SIDETONE_ENABLE
    if (mic1_data_bulk > 2) {
        mic1_data_bulk = 0;
    }
    if (mic1_data_offset > (512 - len)) {
        mic1_data_offset = 0;
    }
    memcpy((((u32)&mic1_data[mic1_data_bulk][0]) + mic1_data_offset), buf, len);
    data_len = 512;
    buf = (s16 *)(&mic1_data[mic1_data_bulk]);
    mic1_data_offset += len;
    if (mic1_data_offset <= (512 - len)) {
        return;
    }
    mic1_data_bulk++;

#endif/*TCFG_SIDETONE_ENABLE*/

    if (aec_hdl && aec_hdl->start) {
        aec_in_data_ref(buf, data_len);
    }
}

/*
*********************************************************************
*                  Audio AEC Reference
* Description: AEC模块参考数据输入
* Arguments  : buf	输入参考数据地址
*			   len	输入参考数据长度
* Return	 : None.
* Note(s)    : 声卡设备是DAC，默认不用外部提供参考数据
*********************************************************************
*/
void audio_aec_refbuf(s16 *buf, u16 len)
{
    if (aec_hdl && aec_hdl->start) {
#if AEC_TOGGLE
        aec_ref_data(buf, len);
#endif/*AEC_TOGGLE*/
    }
}

/*
*********************************************************************
*                  			Audio CVP IOCTL
* Description: CVP功能配置
* Arguments  : cmd 		操作命令
*		       value 	操作数
*		       priv 	操作内存地址
* Return	 : 0 成功 其他 失败
* Note(s)    : (1)比如动态开关降噪NS模块:
*				audio_cvp_ioctl(CVP_NS_SWITCH,1,NULL);	//降噪开
*				audio_cvp_ioctl(CVP_NS_SWITCH,0,NULL);  //降噪关
*********************************************************************
*/
int audio_cvp_ioctl(int cmd, int value, void *priv)
{
    if (aec_hdl && aec_hdl->start) {
        return aec_ioctl(cmd, value, priv);
    } else {
        return -1;
    }
}

/*
*********************************************************************
*                  Audio AEC Toggle Set
* Description: AEC模块算法开关使能
* Arguments  : toggle 0 关闭算法 1 打开算法
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
int audio_aec_toggle_set(u8 toggle)
{
    if (aec_hdl) {
        aec_toggle(toggle);
        return 0;
    }
    return 1;
}

/**
 * 以下为用户层扩展接口
 */
//pbg profile use it,don't delete
void aec_input_clear_enable(u8 enable)
{
    if (aec_hdl) {
        aec_hdl->input_clear = enable;
        log_info("aec_input_clear_enable= %d\n", enable);
    }
}

#if AEC_PITCHSHIFTER_CONFIG

/*----------------------------------------------------------------------------*/
/**@brief    变声参数直接更新
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void set_pitch_para(u32 shiftv, u32 sr, u8 effect, u32 formant_shift)
{
    if (aec_hdl && aec_hdl->pitch) {
        PITCH_SHIFT_PARM parm = {0};

        parm.sr = sr;
        parm.shiftv = shiftv;
        parm.effect_v = effect;
        parm.formant_shift = formant_shift;

        //printf("\n\n\nshiftv[%d],sr[%d],effect[%d],formant_shift[%d] \n\n", parm.shiftv, parm.sr, parm.effect_v, parm.formant_shift);
        update_picth_parm(aec_hdl->pitch, &parm);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    变声效果设置
   @param  sound:原声SOUND_ORIGINAL,男声SOUND_UNCLE, 女声SOUND_GODDESS
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_aec_pitch_change(u32 sound)
{
    if (aec_hdl && aec_hdl->pitch) {
        switch (sound) {
        case SOUND_ORIGINAL:
            pause_pitch(aec_hdl->pitch, 1);
            log_info("pitch origin\n");
            break;
        case SOUND_UNCLE:
            set_pitch_para(136, aec_hdl->pitch->parm.sr, EFFECT_PITCH_SHIFT, 0);
            pause_pitch(aec_hdl->pitch, 0);
            log_info("pitch uncle\n");
            break;
        case SOUND_GODDESS:
            if (VC_KINDO_TVM) {
                set_pitch_para(56, aec_hdl->pitch->parm.sr, EFFECT_VOICECHANGE_KIN0, 150);
            } else {
                set_pitch_para(46, aec_hdl->pitch->parm.sr, EFFECT_VOICECHANGE_KIN2, 80);
            }

            /* set_pitch_para(66, aec_hdl->pitch->parm.sr, EFFECT_VOICECHANGE_KIN0, 150); */
            pause_pitch(aec_hdl->pitch, 0);
            log_info("pitch goddess\n");
            break;
        default:
            break;
        }
    }
}

struct _esco_eff {
    u16 eff_L;
    u16 eff_R;
};
struct _esco_eff esco_eff;
#define TWS_FUNC_ID_ESCO_EFF \
	((int)(('E' + 'S' + 'C' + 'O') << (2 * 8)) | \
	 (int)(('E' + 'F' + 'F') << (1 * 8)) | \
	 (int)(('S' + 'Y' + 'N' + 'C') << (0 * 8)))


/*----------------------------------------------------------------------------*/
/**@brief   变声控制
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_aec_pitch_change_ctrl()
{
    if (!aec_hdl) {
        return;
    }
#if TCFG_USER_TWS_ENABLE
    int state = tws_api_get_tws_state();
    if (state & TWS_STA_SIBLING_CONNECTED) {// tws链接时，左耳触发男声， 右耳触发女声
        enum audio_channel channel = tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R;
        if (channel == AUDIO_CH_L) {
            log_info("aec_hdl->pitch_mode_L %d\n", aec_hdl->pitch_mode_L);
            if (aec_hdl->pitch_mode_L != SOUND_UNCLE) {
                aec_hdl->pitch_mode_L = SOUND_UNCLE;
                aec_hdl->pitch_mode_R = SOUND_UNCLE;
            } else {
                aec_hdl->pitch_mode_L = SOUND_ORIGINAL;
                aec_hdl->pitch_mode_R = SOUND_ORIGINAL;
            }
        } else if (channel == AUDIO_CH_R) {
            log_info("aec_hdl->pitch_mode_R %d\n", aec_hdl->pitch_mode_R);
            if (aec_hdl->pitch_mode_R != SOUND_GODDESS) {
                aec_hdl->pitch_mode_R = SOUND_GODDESS;
                aec_hdl->pitch_mode_L = SOUND_GODDESS;
            } else {
                aec_hdl->pitch_mode_R = SOUND_ORIGINAL;
                aec_hdl->pitch_mode_L = SOUND_ORIGINAL;
            }
        }
        esco_eff.eff_L = aec_hdl->pitch_mode_L;
        esco_eff.eff_R = aec_hdl->pitch_mode_R;
        tws_api_send_data_to_sibling((u8 *)&esco_eff, sizeof(struct _esco_eff), TWS_FUNC_ID_ESCO_EFF);//发送左右耳的变声模式
    } else
#endif/*TCFG_USER_TWS_ENABLE*/
    {
        //tws未连接时，变声直接切换，男声->女声->原声
        if (aec_hdl->pitch_mode_L != SOUND_UNCLE) {
            aec_hdl->pitch_mode_L = SOUND_UNCLE;
        } else if (aec_hdl->pitch_mode_L == SOUND_UNCLE) {
            aec_hdl->pitch_mode_L = SOUND_GODDESS;
        } else {
            aec_hdl->pitch_mode_L = SOUND_ORIGINAL;
        }
        audio_aec_pitch_change(aec_hdl->pitch_mode_L);
    }
}


#if TCFG_USER_TWS_ENABLE
static void tws_esco_pitch_align(void *data, u16 len, bool rx)
{
    memcpy(&esco_eff, data, sizeof(struct _esco_eff));
    int state = tws_api_get_tws_state();
    enum audio_channel channel;
    if (state & TWS_STA_SIBLING_CONNECTED) {
        channel = tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R;
        if (channel == AUDIO_CH_L) {//左耳设置变声
            log_info("channel %d============= eff_L %d\n", channel, esco_eff.eff_L);
            audio_aec_pitch_change(esco_eff.eff_L);
            if (aec_hdl) {
                aec_hdl->pitch_mode_L =  esco_eff.eff_L;
            }
        } else {//右耳设置变声
            log_info("channel %d============= eff_R %d\n", channel, esco_eff.eff_R);
            audio_aec_pitch_change(esco_eff.eff_R);
            if (aec_hdl) {
                aec_hdl->pitch_mode_R =  esco_eff.eff_R;
            }
        }
    }
}

REGISTER_TWS_FUNC_STUB(esco_align_eff) = {
    .func_id = TWS_FUNC_ID_ESCO_EFF,
    .func    = tws_esco_pitch_align,
};

#endif/*TCFG_USER_TWS_ENABLE*/

#endif/* AEC_PITCHSHIFTER_CONFIG */


#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE == 0*/
#endif /*TCFG_CVP_DEVELOP_ENABLE*/

