#ifndef _ICSD_ADT_H
#define _ICSD_ADT_H

#include "generic/typedef.h"
#include "os/os_type.h"
#include "os/os_api.h"
#include "classic/tws_api.h"
#include "math.h"

#if 0
#define _adt_printf printf                  //打开智能免摘库打印信息
#else
extern int printf_off(const char *format, ...);
#define _adt_printf printf_off
#endif/*log_en*/

#if 0
#define _wat_printf printf                  //打开广域点击库打印信息
#else
extern int printf_off(const char *format, ...);
#define _wat_printf printf_off
#endif/*log_en*/

#if 0
#define _wind_printf printf                 //打开风噪检测库打印信息
#else
extern int printf_off(const char *format, ...);
#define _wind_printf printf_off
#endif/*log_en*/

#if 0
#define _ein_printf printf                 //打开入耳检测库打印信息
#else
extern int printf_off(const char *format, ...);
#define _ein_printf printf_off
#endif/*log_en*/

#define ADT_CALIBRATION_EN  	    	0
#define ADT_USE_ADAPTIVE_SZ     	    0//使用自适应SZ
#define ADT_EAR_IN_EN                   0

#define ADT_VDT_EN					BIT(0) //智能免摘
#define ADT_WDT_EN					BIT(1) //风噪检测
#define ADT_WAT_EN					BIT(2) //广域点击
#define ADT_EIN_EN  				BIT(3) //入耳检测

#define ADT_PATH_VDT_ANCL_ON		BIT(0) //智能免摘使用ancl通路数据
#define ADT_PATH_WAT_EMIC_ON		BIT(1) //广域点击使用errmic数据通路
#define ADT_PATH_VAD_ON				BIT(2) //通过VAD检测降低功耗
#define ADT_PATH_DAC_SRC_EN         BIT(3)
#define ADT_PATH_3M_EN         		BIT(4)


#define ADT_INF_1					BIT(0)
#define ADT_INF_2					BIT(1)
#define ADT_INF_3					BIT(2)

extern const u8 ADT_PATH_CONFIG;
#define ICSD_TWS_STA_SIBLING_CONNECTED TWS_STA_SIBLING_CONNECTED//tws已连接

typedef struct {
    u8 voice_state;
    u8 wind_lvl;
    u8 wat_result;
    u8 ein_state;
} __adt_result;

struct icsd_acoustic_detector_infmt {
    u16 sample_rate;     //当前播放采样率
    u8 ff_gain;
    u8 fb_gain;
    u8 adt_mode;         // TWS: 0 HEADSET: 1
    u8 dac_mode;		 //低压0 高压1
    u8 mic0_type;        // MIC0 类型
    u8 mic1_type;        // MIC1 类型
    u8 mic2_type;        // MIC2 类型
    u8 mic3_type;        // MIC3 类型
    u8 sensitivity;      //智能免摘灵敏度设置
    void *lfb_coeff;
    void *lff_coeff;
    void *ltrans_coeff;
    void *ltransfb_coeff;
    float gains_l_fbgain;
    float gains_l_ffgain;
    float gains_l_transgain;
    float gains_l_transfbgain;
    u8    gain_sign;
    u8    lfb_yorder;
    u8    lff_yorder;
    u8    ltrans_yorder;
    u8    ltransfb_yorder;
    u32   trans_alogm;
    u32   alogm;

    void *alloc_ptr;    //外部申请的ram地址
    /*
        算法结果输出
        voice_state: 0 无讲话； 1 有讲话
        wind_lvl: 0-70 风噪等级
    */
    void (*output_hdl)(u8 voice_state, u8 wind_lvl);    //算法结果输出
    void (*anc_dma_post_msg)(void);     //ANC task dma 抄数回调接口
    void (*anc_dma_output_callback);    //ANC DMA下采样数据输出回调，用于写卡分析
};

struct icsd_acoustic_detector_libfmt {
    int adc_isr_len;     //ADC中断长度
    int adc_sr;           //ADC 采样率
    int lib_alloc_size;  //算法ram需求大小
    u8 mic_num;			 //需要打开的mic个数
};

enum {
    ICSD_ANC_LFF_MIC = 0,
    ICSD_ANC_LFB_MIC = 1,
    ICSD_ANC_RFF_MIC = 2,
    ICSD_ANC_RFB_MIC = 3,
    ICSD_ANC_TALK_MIC = 4,
    ICSD_ANC_MIC_NULL = 0XFF,   //对应的MIC没有数值,则表示这个MIC没有开启
};

typedef struct {
    void  *adt_float_data;
    float *sz_inptr;
    float *pz_inptr;
    s16 anc_in_pz_sum;
    s16 anc_in_sz_mean;
    s16 anc_in_fb_sum;
    s16 anc_out_pz_sum;
    s16 anc_out_sz_mean;
    s16 anc_out_fb_sum;

    s16 pnc_in_pz_sum;
    s16 pnc_in_sz_mean;
    s16 pnc_in_fb_sum;
    s16 pnc_out_pz_sum;
    s16 pnc_out_sz_mean;
    s16 pnc_out_fb_sum;

    s16 trans_in_pz_sum;
    s16 trans_in_sz_mean;
    s16 trans_in_fb_sum;
    s16 trans_out_pz_sum;
    s16 trans_out_sz_mean;
    s16 trans_out_fb_sum;
} __icsd_adt_parm;

enum {
    RESUME_ANCMODE = 0,
    RESUME_BYPASSMODE,
};
enum {
    ADT_DACMODE_LOW = 0,
    ADT_DACMODE_HIGH,
};
enum {
    ADT_TWS = 0,
    ADT_HEADSET,
};
enum {
    ADT_ANC_ON = 0,
    ADT_ANC_OFF,
};
enum {
    ADT_VOICE_WIND_MODE = 0,   //智能免摘 + 风噪
    ADT_WAT_MODE,          //广域点击
    ADT_VW_WAT_MODE,       //智能免摘 + 风噪检测 + 广域点击
};
enum {
    ADT_EIN_UNSUPPORT = 0,
    ADT_EIN_TWS_V0,
    ADT_EIN_HEADSET_V0,
};

extern const float wat_pn_gain;
extern const float wat_anc_gain;
extern const float wat_bypass_gain;
extern const u8 final_speech_even_thr;
extern const u8 VDT_TRAIN_EN;
extern const float ref_gain_sel;
extern const float err_gain_sel;
extern u8 pnc_ref_pwr_thr;
extern u8 pnc_err_pwr_thr;
extern u8 tpc_ref_pwr_thr;
extern u8 tpc_err_pwr_thr;
extern u8 anc_ref_pwr_thr;
extern u8 anc_err_pwr_thr;
extern const float wref[8];
extern const float wref_tran[8];
extern const float wref_ancs[8];
extern const float wdac_senc;
extern const float wdac[8];
extern const float fdac[8];
extern const u8 ADT_DET_VER;
extern const u8 ADT_TWS_MODE;
extern const u8 ADT_EIN_VER;
extern float anc_trans_lfb_gain;
extern float anc_trans_rfb_gain;
extern const double anc_trans_lfb_coeff[];
extern const double anc_trans_rfb_coeff[];

extern int (*adt_printf)(const char *format, ...);
extern int (*wat_printf)(const char *format, ...);
extern int (*ein_printf)(const char *format, ...);

extern void icsd_adt_tws_m2s(u8 cmd);
extern void icsd_adt_tws_s2m(u8 cmd);
//LIB调用的算术函数
extern float sin_float(float x);
extern float cos_float(float x);
extern float cos_hq(float x);
extern float sin_hq(float x);
extern float log10_float(float x);
extern float exp_float(float x);
extern float root_float(float x);
//extern float pow10(float n);
extern float adt_pow10(float n);
extern float angle_float(float x, float y);
//ADT调用的APP函数

extern int  audio_dac_read_anc_reset(void);
extern int  audio_acoustic_detector_updata();
extern void tws_tx_unsniff_req(void);
extern int  tws_api_get_role(void);
extern int  tws_api_get_tws_state();
extern u8   anc_dma_done_ppflag();
extern void anc_core_dma_ie(u8 en);
extern void anc_core_dma_stop(void);
extern int  os_task_create(void (*task_func)(void *p_arg), void *p_arg, u8 prio, u32 stksize, int qsize, const char *name);
extern int  os_task_del(const char *name);
extern int  os_taskq_post_msg(const char *name, int argc, ...);
extern int  os_taskq_pend(const char *fmt, int *argv, int argc);
extern void anc_dma_on_double(u8 out_sel, int *buf, int len);
extern int  audio_resample_hw_push_data_out(void *resample);
extern void audio_resample_hw_close(void *resample);
extern void *audio_resample_hw_open(u8 channel, int in_rate, int out_rate, u8 type);
extern int  audio_resample_hw_set_output_handler(void *resample, void *priv, int (*handler)(void *, void *, int));
extern int  audio_resample_hw_write(void *resample, void *data, int len);
extern void audio_anc_post_msg_adt(u8 cmd);
extern void hw_fft_run(unsigned int fft_config, const int *in, int *out);
extern unsigned int hw_fft_config(int N, int log2N, int is_same_addr, int is_ifft, int is_real);
//APP调用的ADT函数
extern void icsd_adt_start();
extern void icsd_adt_anctask_handle(u8 cmd);
//APP调用的LIB函数
extern void icsd_acoustic_detector_get_libfmt(struct icsd_acoustic_detector_libfmt *libfmt, u8 function);
extern void icsd_acoustic_detector_set_infmt(struct icsd_acoustic_detector_infmt *fmt);
extern void icsd_acoustic_detector_infmt_updata(struct icsd_acoustic_detector_infmt *fmt);
extern void icsd_acoustic_detector_open(void);
extern void icsd_acoustic_detector_close();
extern void icsd_acoustic_detector_resume(u8 mode, u8 anc_onoff);
extern void icsd_acoustic_detector_suspend();
extern void icsd_acoustic_detector_ancdma_done();//ancdma done回调
extern void icsd_acoustic_detector_mic_input_hdl(void *priv, s16 *buf, int len);
extern void icsd_adt_set_audio_sample_rate(u16 sample_rate);
//LIB调用的ADT函数
extern void icsd_adt_FFT_radix256(int *in_cur, int *out);
extern void icsd_adt_FFT_radix128(int *in_cur, int *out);
extern void icsd_adt_FFT_radix64(int *in_cur, int *out);
extern void icsd_adt_anc_dma_on(u8 out_sel, int *buf, int len);
extern void icsd_adt_hw_suspend();
extern void icsd_adt_hw_resume();
extern void icsd_post_adttask_msg(u8 cmd);
extern void icsd_post_srctask_msg(u8 cmd);
extern void icsd_adt_src_close(void *resample);
extern void *icsd_adt_src_init(int in_rate, int out_rate, int (*handler)(void *, void *, int));
extern void icsd_adt_src_push(void *resample);
extern void icsd_adt_src_write(void *data, int len, void *resample);
extern void icsd_adt_run_output(__adt_result *adt_result);
extern void icsd_post_anctask_msg(u8 cmd);
extern void icsd_adt_voice_board_data();
extern void icsd_adt_tx_unsniff_req();
extern u32  icsd_adt_get_tws_state();
extern u32  icsd_adt_get_role();
extern u8   icsd_get_ang_flag(float *_target_angle);
//ADT调用的LIB函数
extern void icsd_adt_version();
extern void icsd_adt_parm_set(__icsd_adt_parm *_ADT_PARM);
extern void icsd_adt_szout_set(float *anc_sz_data, float *anc_pz_data, u8 sz_out_exist);
extern void icsd_adt_task_handler(int msg);
extern void icsd_src_task_handler(int msg);
extern void icsd_adt_s2m_packet(s8 *data, u8 cmd);
extern void icsd_adt_m2s_packet(s8 *data, u8 cmd);

extern void icsd_adt_szout_init();
extern void icsd_adt_dma_done();
extern void icsd_adt_task_create();
extern void icsd_adt_task_kill();
extern void icsd_src_task_create();
extern void icsd_src_task_kill();
extern void icsd_adt_m2s_cb(void *_data, u16 len, bool rx);
extern void icsd_adt_s2m_cb(void *_data, u16 len, bool rx);

extern u8 audio_adt_talk_mic_analog_close();
extern u8 audio_adt_talk_mic_analog_open();
extern u8 talk_mic_analog_close;

extern u8 oral_function;
extern const u8 adt_errpxx_dB_thr;
extern const u8 adt_err_zcv_thr;
extern const u8 pwr_inside_even_flag0_thr;
extern const u8 angle_err_even_flag0_thr;
extern const u8 angle_talk_even_flag0_thr;
extern const u16 f1f2_angsum_thr;
extern const u16 ADT_DEBUG_INF;
#endif
