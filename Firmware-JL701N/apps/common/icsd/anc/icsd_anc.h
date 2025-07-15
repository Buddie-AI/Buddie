#ifndef _SD_ANC_LIB_H
#define _SD_ANC_LIB_H

#include "asm/anc.h"
#include "timer.h"
#include "math.h"

#if 0
#define _anc_printf printf                  //打开自适应ANC参数调试信息
#else
extern int anc_printf_off(const char *format, ...);
#define _anc_printf anc_printf_off
#endif

#if 0
#define icsd_printf printf
#else
#define icsd_printf(...)
#endif

#define ANC_TRAIN_TIMEOUT_S         5//10//秒
#define TWS_STA_SIBLING_CONNECTED   0x00000002//tws已连接

#define ANC_USER_TRAIN_DMA_LEN 		16384

#define TEST_JT_OFF                 0

#define ANC_DEBUG_OFF               0
#define ANC_DEBUG_TONEL_SZ_DATA     1
#define ANC_DEBUG_TONER_SZ_DATA     2
#define ANC_DEBUG_PZ_DATA           3
#define ANC_DEBUG_BYPASS_SIGN       4
extern  const u8 ICSD_ANC_DEBUG;

#define ANC_FUN_TARGET_SYNC          BIT(0)
extern const u16 ICSD_ANC_FUN;

//FF_FB_EN
#define ICSD_DFF_AFB_EN             BIT(7)
#define ICSD_AFF_DFB_EN             BIT(6)
#define ICSD_FB_TRAIN_OFF           BIT(5)
#define ICSD_FB_TRAIN_NEW           BIT(4)
//TEST_ANC_COMBINATION
#define TEST_ANC_COMBINATION_OFF    0x00
#define TEST_TFF_TFB_TONE			0x01          // bypassl强制失败
#define TEST_TFF_TFB_BYPASS			0x02          // tonel强制失败
#define TEST_TFF_DFB				0x03          // FB训练强制失败
#define TEST_DFF_TFB				0x04          // PZ强制失败
#define TEST_DFF_DFB				0x05          // tonel,bypassl,PZ强制失败
#define TEST_DFF_DFB_2              0x06          // tonel,PZ强制失败
//TEST_ANC_TRAIN
#define TEST_ANC_TRAIN_OFF          0x01
#define TEST_FF_TRAIN_PASS          0x02          // 强制每次训练都成功
#define TEST_TRAIN_FAIL             0x03          // 强制每次训练都失败

#define TWS_TONE_BYPASS_MODE  		1
#define TWS_BYPASS_MODE      		2
#define HEADSET_TONE_BYPASS_MODE 	3
#define HEADSET_BYPASS_MODE  		4
#define HEADSET_TONES_MODE          5
#define TWS_TONE_MODE               6
//user_train_state------------------------
#define ANC_USER_TRAIN_DMA_READY	BIT(0)
#define ANC_USER_TRAIN_TOOL_DATA 	BIT(1)
#define ANC_DAC_MODE_H1_DIFF        BIT(2)
#define ANC_USER_TRAIN_DMA_EN       BIT(3)
#define ANC_USER_TRAIN_TONEMODE     BIT(4)
#define ANC_TRAIN_TONE_PLAY			BIT(5)
#define ANC_TRAIN_TONE_END			BIT(6)
#define ANC_TRAIN_TONE_FIRST 		BIT(7)
#define ANC_TRAIN_IIR_READY 		BIT(8)
#define ANC_MASTER_RX_PZSYNC 		BIT(9)
#define ANC_FB_IIR_AB_DONE 			BIT(10)
#define ANC_FF_IIR_AB_DONE 			BIT(11)
#define ANC_PZOUT_INITED 			BIT(12)
#define ANC_SZOUT_INITED 			BIT(13)
#define ANC_USER_TRAIN_RUN 			BIT(14)
#define ANC_TRAIN_TIMEOUT 			BIT(15)
#define ANC_TRAIN_DMA_ON			BIT(16)
#define ANC_TONE_DAC_ON				BIT(17)
#define ANC_TONE_ANC_READY			BIT(18)
#define ANC_TONESMODE_BYPASS_OFF    BIT(19)
#define ANC_RX_TARGET               BIT(20)
#define ANC_TARGET_DONE             BIT(21)
//icsd_anc_contral
#define ICSD_ANC_FORCED_EXIT        BIT(0)
#define ICSD_ANC_SUSPENDED          BIT(1)
#define ICSD_ANC_FADE_GAIN_RESUME   BIT(2)
#define ICSD_ANC_TONE_END           BIT(3)
#define ICSD_ANC_INITED             BIT(4)
#define ICSD_ANC_FORCED_BEFORE_INIT BIT(5)
//anctask cmd
enum {
    ANC_CMD_FORCED_EXIT = 0,
    ANC_CMD_TONE_MODE_START,
    ANC_CMD_TONEL_START,
    ANC_CMD_TONER_START,
    ANC_CMD_PZL_START,
    ANC_CMD_PZR_START,
    ANC_CMD_TONE_DACON,
    ANC_CMD_TRAIN_AFTER_TONE,
    ANC_CMD_TARGET_END,
    ANC_CMD_TARGET_HANDLE,
};
//SZPZ->anc_err---------------------------
#define ANC_ERR_LR_BALANCE          BIT(0)//左右耳不平衡
#define ANC_ERR_SOUND_PRESSURE      BIT(1)//低声压
#define ANC_ERR_SHAKE               BIT(2)//抖动
//icsd_anc_function-----------------------
#define ANC_ADAPTIVE_CMP            BIT(0)
#define ANC_EARPHONE_CHECK          BIT(1)

#define TARLEN    					120
#define TARLEN_L					40
#define DRPPNT   					10
#define ANC_FBANC_OUT_SEL_L         13
#define ANC_FFANC_OUT_SEL_L         12



struct icsd_anc_tws_packet {
    s8  *data;
    u16 len;
};

struct icsd_anc_board_param {
    u8    iir_mode;
    void  *anc_db;
    void  *anc_data_l;
    void  *anc_data_r;
    void  *anc_double_data_l;
    void  *anc_double_data_r;
    u8 default_ff_gain;
    u8 default_fb_gain;
    s8 tool_ffgain_sign;
    s8 tool_fbgain_sign;
    s8 tool_target_sign;
    s8 tfb_sign;
    s8 tff_sign;
    s8 bfb_sign;
    s8 bff_sign;
    s8 bypass_sign;
    float m_value_l;			//80 ~ 200   //135  最深点位置中心值
    float sen_l;     			//12 ~ 22    //18   最深点深度
    float in_q_l;  				//0.4 ~ 1.2  //0.6	最深点降噪宽度
    float m_value_r;			//80 ~ 200   //135  最深点位置中心值
    float sen_r;     			//12 ~ 22    //18   最深点深度
    float in_q_r;  				//0.4 ~ 1.2  //0.6	最深点降噪宽度
    float *mse_tar;
    int   gold_curve_en;
    float gain_a_param;
    u8    gain_a_en;
    u8    cmp_abs_en;
    u8    jt_en;
    int   idx_begin;
    int   idx_end;
    u8  FB_NFIX;
    u16 fb_w2r;
    float sen_offset_l;
    float sen_offset_r;
    u8  gain_min_offset;
    u8  gain_max_offset;
    u8  ff_target_fix_num_l;
    u8  ff_target_fix_num_r;
    u8	pz_max_times;
    u8	bypass_max_times;
    u8	ff_yorder;
    u8	fb_yorder;
    u8	cmp_yorder;
    int  cmp_type[8];
    float cmp_thd_low;
    float cmp_thd_high;
    float bypass_volume;
    float minvld;
    //ctl
    u8 mode;
    u8 FF_FB_EN;
    u8 dma_belong_to;
    u32 tone_jiff;
    int IIR_NUM;// 4     3
    int IIR_NUM_FIX;// 4   8-IIR_NUM
    int IIR_COEF; //(IIR_NUM * 3+1)
    u8 	JT_MODE;
    float *anc_szl_out;
    float *anc_szr_out;
    float sz_priority_thr;
};
extern struct icsd_anc_board_param *ANC_BOARD_PARAM;

struct icsd_anc_backup {
    int gains_alogm;
    u8 gains_l_ffmic_gain;
    u8 gains_l_fbmic_gain;
    u8 gains_r_ffmic_gain;
    u8 gains_r_fbmic_gain;
    float gains_l_ffgain;
    float gains_l_fbgain;
    float gains_r_ffgain;
    float gains_r_fbgain;
    double *lff_coeff;
    double *lfb_coeff;
    double *rff_coeff;
    double *rfb_coeff;
    u8 lff_yorder;
    u8 lfb_yorder;
    u8 rff_yorder;
    u8 rfb_yorder;
    u8 ff_1st_dcc;
    u8 fb_1st_dcc;
    u8 ff_2nd_dcc;
    u8 fb_2nd_dcc;
    u8 gains_drc_en;
};
extern volatile struct icsd_anc_backup *CFG_BACKUP;

struct icsd_anc {
    audio_anc_t *param;
    u16	*anc_fade_gain;
    u8 mode;
    u8 train_index;
    u8 adaptive_run_busy;			//自适应训练中
    int	*gains_alogm;
    u8 *gains_l_ffmic_gain;
    u8 *gains_l_fbmic_gain;
    u8 *gains_r_ffmic_gain;
    u8 *gains_r_fbmic_gain;
    float *gains_l_ffgain;
    float *gains_l_fbgain;
    float *gains_l_cmpgain;
    float *gains_r_ffgain;
    float *gains_r_fbgain;
    float *gains_r_cmpgain;
    double **lff_coeff;
    double **lfb_coeff;
    double **lcmp_coeff;
    double **rff_coeff;
    double **rfb_coeff;
    double **rcmp_coeff;
    u8 *lff_yorder;
    u8 *lfb_yorder;
    u8 *lcmp_yorder;
    u8 *rff_yorder;
    u8 *rfb_yorder;
    u8 *rcmp_yorder;
    u8 *ff_1st_dcc;
    u8 *fb_1st_dcc;
    u8 *ff_2nd_dcc;
    u8 *fb_2nd_dcc;
    u8 *gains_drc_en;
    void *src_hdl;
    u32 dac_on_jiff;
    u32 dac_on_slience;
    int tone_delay;
};
extern volatile struct icsd_anc ICSD_ANC;

struct icsd_anc_tool_data {
    int h_len;
    int yorderb;//int fb_yorder;
    int yorderf;//int ff_yorder;
    int yorderc;//int cmp_yorder;
    float *h_freq;
    float *data_out1;//float *hszpz_out_l;
    float *data_out2;//float *hpz_out_l;
    float *data_out3;//float *htarget_out_l;
    float *data_out4;//float *fb_fgq_l;
    float *data_out5;//float *ff_fgq_l;
    float *data_out6;//float *hszpz_out_r;
    float *data_out7;//float *hpz_out_r;
    float *data_out8;//float *htarget_out_r;
    float *data_out9;//float *fb_fgq_r;,
    float *data_out10;//float *ff_fgq_r;
    float *data_out11;//float *cmp_fgq_l;
    float *data_out12;//float *cmp_fgq_r;
    float *data_out13;//float *tool_target_out_l;
    float *data_out14;//float *tool_target_out_r;
    float *wz_temp;
    u8 result;
    u8 anc_err;
    u8 save_idx;
    u8 use_idx;
    u8 tws_use_idx;
    u8 anc_combination;
    u8 cmp_result;
};
extern volatile struct icsd_anc_tool_data *TOOL_DATA;

enum {
    TFF_TFB = 0,
    TFF_DFB,
    DFF_TFB,
    DFF_DFB,
};

enum {
    ICSD_FULL_INEAR = 0,
    ICSD_HALF_INEAR,
    ICSD_HEADSET,
};
extern const u8 ICSD_EP_TYPE;

extern int (*anc_printf)(const char *format, ...);
extern int icsd_anc_id;
extern volatile u8 icsd_anc_function;
extern volatile u8 icsd_anc_contral;
extern volatile int user_train_state;
extern u32 train_time;
extern u16 icsd_time_out_hdl;
extern u8 icsd_anc_combination_test;
extern u8 icsd_anc_train_test;
extern const u8 TWS_MODE;
extern const u8 HEADSET_MODE;
extern const u8 NEW_PRE_TREAT;
extern const u8 icsd_dcc[4];
extern const u8 FF_VERSION;
extern const u8 cmp_iir_type[];
extern const u8 ff_iir_type[];
extern const u8 fb_iir_type[];
//LIB调用的算术函数
extern float complex_abs_float(float x, float y);
extern float sin_float(float x);
extern float cos_float(float x);
extern float cos_hq(float x);
extern float sin_hq(float x);
extern float log10_float(float x);
extern float exp_float(float x);
extern float root_float(float x);
extern float anc_pow10(float n);
extern float angle_float(float x, float y);
extern void icsd_anc_save_with_idx(u8 save_idx);
extern void audio_adc_mic_demo_close(void);
extern void icsd_anc_board_config();
//SDK调用的SDANC APP函数

extern void icsd_anc_dma_done();
extern void icsd_anc_end(audio_anc_t *param);
extern void icsd_anc_run();
extern void	icsd_anc_setparam();
extern void icsd_anc_timeout_handler();
extern void icsd_anc_init(audio_anc_t *param, u8 mode, u8 seq, int tone_delay, u8 tws_balance_en);
extern u16  sys_timeout_add(void *priv, void (*func)(void *priv), u32 msec);
extern void icsd_anc_tone_play_start();
extern void icsd_anctone_dacon(u32 slience_frames, u16 sample_rate);
extern u8 icsd_anc_train_result_get(struct icsd_anc_tool_data *TOOL_DATA);

//SDANC APP调用的库函数
extern void icsd_anc_cmd_packet(s8 *data, u8 cmd);
extern void icsd_anc_tws_sync_cmd(void *_data, u16 len, bool rx);
extern void icsd_anc_version();
extern void icsd_anc_lib_init();
extern void icsd_anc_htarget_data_send_end();
extern void icsd_anc_mode_init(int tone_delay);
extern void icsd_anc_train_after_tone();
extern void icsd_anc_m2s_packet(s8 *data, u8 cmd);
extern void icsd_anc_s2m_packet(s8 *data, u8 cmd);
extern void icsd_anc_msync_packet(struct icsd_anc_tws_packet *packet, u8 cmd);
extern void icsd_anc_ssync_packet(struct icsd_anc_tws_packet *packet, u8 cmd);
extern void icsd_anc_m2s_cb(void *_data, u16 len, bool rx);
extern void icsd_anc_s2m_cb(void *_data, u16 len, bool rx);
extern void icsd_anc_msync_cb(void *_data, u16 len, bool rx);
extern void icsd_anc_ssync_cb(void *_data, u16 len, bool rx);
extern void cal_wz(double *ab, float gain, int tap, float *freq, float fs, float *wz, int len);
extern void ff_fgq_2_aabb(double *iir_ab, float *ff_fgq);
extern float icsd_anc_vmdata_match(float *_vmdata, float gain);
extern void icsd_anc_tonemode_start();
extern float icsd_anc_default_match();
extern void icsd_anc_anctask_cmd_handle(u8 cmd);
extern void icsd_anc_board_param_init();
extern void icsd_anc_tool_data_init();
//SDANC APP调用的SDK函数

extern void icsd_biquad2ab_out(float gain, float f, float fs, float q, double *a0, double *a1, double *a2, double *b0, double *b1, double *b2, int type);
extern void biquad2ab_double(float gain, float f, float q, double *a0, double *a1, double *a2, double *b0, double *b1, double *b2, int type);
extern int  tws_api_get_role(void);
extern int  tws_api_get_tws_state();
extern void anc_dma_on(u8 out_sel, int *buf, int len);
extern void audio_anc_fade2(int gain, u8 en, u8 step, u8 slow);
extern void anc_user_train_process(audio_anc_t *param);
extern void audio_anc_post_msg_user_train_run(void);
extern void audio_anc_post_msg_user_train_setparam(void);
extern void audio_anc_post_msg_user_train_timeout(void);
extern unsigned int hw_fft_config(int N, int log2N, int is_same_addr, int is_ifft, int is_real);
extern void hw_fft_run(unsigned int fft_config, const int *in, int *out);
extern void anc_user_train_cb(u8 mode, u8 result, u8 forced_exit);
//库调用的SDANC APP函数
extern void icsd_anc_set_alogm(void *_param, int alogm);
extern void icsd_anc_set_micgain(void *_param, int lff_gain, int lfb_gain);
extern void icsd_anc_fade(void *_param);
extern void icsd_anc_long_fade(void *_param);
extern void icsd_anc_user_train_dma_on(u8 out_sel, u32 len, int *buf);
extern void icsd_anc_htarget_data_send();
extern void icsd_anc_fft(int *in, int *out);
extern u32  icsd_anc_get_role();
extern u32  icsd_anc_get_tws_state();
extern void icsd_anc_tws_m2s(u8 cmd);
extern void icsd_anc_tws_s2m(u8 cmd);
extern void icsd_anc_tws_msync(u8 cmd);
extern void icsd_anc_tws_ssync(u8 cmd);
extern void icsd_anc_train_timeout();
extern void icsd_anc_src_init(int in_rate, int out_rate, int (*handler)(void *, void *, int));
extern void icsd_anc_src_write(void *data, int len);
extern u8   icsd_anc_get_save_idx();
extern void icsd_anc_vmdata_num_reset();
extern u8   icsd_anc_get_vmdata_num();
extern u8 	icsd_anc_min_diff_idx();
extern void icsd_anc_fgq_printf(float *ptr);
extern void icsd_anc_aabb_printf(double *iir_ab);
extern void icsd_anc_vmdata_by_idx(double *ff_ab, float *ff_gain, u8 idx);
extern void icsd_anc_vmdatar_by_idx(double *ff_ab, float *ff_gain, u8 idx);
extern void icsd_anc_config_inf();
extern void icsd_anc_ear_record_printf();
extern u8   icsd_anc_tooldata_select_vmdata(float *ff_fgq);
extern u8   icsd_anc_tooldata_select_vmdata_headset(float *ff_fgq_l, float *ff_fgq_r);
extern void icsd_audio_adc_mic_open(u8 mic_idx, u8 gain, u16 sr, u8 mic_2_dac);
extern void icsd_audio_adc_mic_close();
extern void sd_anc_exit();

extern void icsd_anc_tonel_start(void *priv);
extern void icsd_anc_toner_start(void *priv);
extern void icsd_anc_pzl_start(void *priv);
extern void icsd_anc_pzr_start(void *priv);
extern void anc_dmadata_debug();
extern void icsd_anc_fft256(int *in, int *out);
extern void icsd_anc_src_push();
extern void anc_core_dma_stop(void);
extern void anc_core_dma_ie(u8 en);
extern void icsd_anc_forced_exit();
extern void audio_anc_post_msg_icsd_anc_cmd(u8 cmd);
extern void icsd_anctone_dacon_handler();
extern void icsd_anc_forced_exit_end();
extern float anc_pow10(float n);


enum {
    SD_ANC_TWS = 0,
    SD_ANC_HEADSET,
};

struct icsd_anc_libfmt {
    int lib_alloc_size; //算法ram需求大小
};

struct icsd_anc_infmt {
    void *alloc_ptr;    //外部申请的ram地址
};

extern void icsd_anc_set_infmt(struct icsd_anc_infmt *fmt);
extern void icsd_anc_get_libfmt(struct icsd_anc_libfmt *libfmt, u8 type);

extern const float THD_JT;
extern const float THD_JT_SAVE;
extern const float THD_JT2;
extern const float spl_tb [21];
extern const float spl_freq_tb [21];
extern const float target_cmp_dat[];

#endif
