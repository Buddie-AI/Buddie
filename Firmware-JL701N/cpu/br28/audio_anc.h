#ifndef AUDIO_ANC_H
#define AUDIO_ANC_H

#include "generic/typedef.h"
#include "asm/anc.h"
#include "anc_btspp.h"
#include "anc_uart.h"
#include "app_config.h"
#include "in_ear_detect/in_ear_manage.h"
#include "audio_anc_mult_scene.h"
#include "icsd_anc_app.h"
#include "audio_anc_fade_ctr.h"

/*******************ANC User Config***********************/
#define ANC_COEFF_SAVE_ENABLE		1	/*ANC滤波器表保存使能*/
#define ANC_INFO_SAVE_ENABLE		0	/*ANC信息记忆:保存上一次关机时所处的降噪模式等等*/
#define ANC_TONE_PREEMPTION			0	/*ANC提示音打断播放(1)还是叠加播放(0)*/
#define ANC_BOX_READ_COEFF			1	/*支持通过工具读取ANC训练系数*/
#define ANC_FADE_EN					1	/*ANC淡入淡出使能*/
#define ANC_MODE_SYSVDD_EN 			0	/*ANC模式提高SYSVDD，避免某些IC电压太低导致ADC模块工作不正常*/
#define ANC_TONE_END_MODE_SW		1	/*ANC提示音结束进行模式切换*/
#define ANC_MODE_EN_MODE_NEXT_SW	1	/*ANC提示音结束后才允许下一次模式切换*/
#define ANC_MODE_FADE_LVL			1	/*降噪模式淡入步进 * 53ms*/

/*
   ANC多场景滤波器配置
 */
#define ANC_MULT_ORDER_ENABLE				0	/*ANC多滤波器使能*/
#define ANC_MULT_ORDER_CMP_ONLY_USE_ID1		1	/*ANC多滤波器-CMP音乐补偿仅使用场景ID1的参数*/
#define ANC_MULT_ORDER_TRANS_ONLY_USE_ID1	0	/*ANC多滤波器-通透模式仅使用场景ID1的参数*/

#define ANC_MULT_ORDER_NORMAL_ID			1	/*ANC多滤波器-开机默认场景ID*/

//ANC多滤波器-耳道自适应ID匹配，选择0则跟随当前场景
#define ANC_MULT_ADPTIVE_TRAIN_USE_ID		1	/*耳道自适应-训练使用的场景ID*/
#define ANC_MULT_ADPTIVE_MATCH_USE_ID		1	/*耳道自适应-匹配使用的场景ID*/

/*-------------------耳道自适应开发者模式说明------------------
    开发者模式(支持工具在线修改)
    权限：
		 1、MIC/SPK数据获取，采样率跟随ANC
		 2、支持工具自适应数据读取,会占用3-4K ram
		 3、TWS区分左右参数
		 4、进入后不支持产测，关机自动退出开发者模式

    ANC_DEVELOPER_MODE_EN
    开发者强制模式(不支持工具修改, 开机默认开发者模式)
    新增权限：
			  5、每次自适应都保存数据，保存前会校验数据，确保开机转换数据正常，不会死机
   		 	  6、自适应失败播放提示音
 -------------------------------------------------------------*/
#define ANC_DEVELOPER_MODE_EN		0	/*开发者强制模式强制使能,启动后不支持产测, 优先级最高, 不受工具控制*/

#define ANC_EAR_ADAPTIVE_EN					TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN  /*ANC耳道自适应使能, 耳道是变量*/
#define ANC_POWEOFF_SAVE_ADAPTIVE_DATA		1							    /*保存耳道自适应数据 0 每次保存；1 关机保存*/
#define ANC_EAR_ADAPTIVE_CMP_EN				ANC_ADAPTIVE_CMP_EN				/*ANC耳道自适应音乐补偿使能*/
#define ANC_EAR_ADAPTIVE_EVERY_TIME			0								/*每次切ANC_ON都进行自适应*/

/*
   ANC场景增益自适应配置
   (场景是变量，与耳道自适应功能相互独立)
 */
#define ANC_ADAPTIVE_EN		    	0						/*ANC增益自适应使能*/
#define ANC_ADAPTIVE_MODE			ANC_ADAPTIVE_GAIN_MODE	/*ANC增益自适应模式*/
#define ANC_ADAPTIVE_TONE_EN		0						/*ANC增益自适应提示音使能*/

/*
   音乐响度ANC动态增益调节, 随着音乐响度增大而减小ANC增益, 开此功能可关闭AUDIO_DRC
 */
#define ANC_MUSIC_DYNAMIC_GAIN_EN					0		/*音乐动态ANC增益使能*/
#define ANC_MUSIC_DYNAMIC_GAIN_THR 					-12		/*音乐动态ANC增益触发阈值(单位dB), range: (-30 ~ 0]*/
#define ANC_MUSIC_DYNAMIC_GAIN_SINGLE_FB			1		/*动态增益单独调FB增益使能*/

#if ANC_TRAIN_MODE == ANC_FB_EN
#define ANC_MODE_ENABLE			ANC_OFF_BIT | ANC_ON_BIT
#else
#define ANC_MODE_ENABLE			ANC_OFF_BIT | ANC_ON_BIT | ANC_TRANS_BIT
#endif/*ANC_TRAIN_MODE*/

//************************************************************************//
//                 ICSD ADT 相关功能配置                                  //
//************************************************************************//
#define AUDIO_ANC_WIDE_AREA_TAP_EVENT_SYNC      0//广域点击左右耳同步使能

/*支持开免摘的anc模式*/
#define SPEAK_TO_CHAT_ANC_MODE_ENABLE			(ANC_ON_BIT)

//****************** ICSD ADT 相关功能配置 end ************************//

/*ANC工具配对码使能*/
#define ANCTOOL_NEED_PAIR_KEY   1
#define ANCTOOL_PAIR_KEY  		"123456" /*ANC工具默认配对码*/

/*******************ANC User Config End*******************/

/*ANC模式调试信息*/
static const char *anc_mode_str[] = {
    "NULL",			/*无效/非法*/
    "ANC_OFF",		/*关闭模式*/
    "ANC_ON",		/*降噪模式*/
    "Transparency",	/*通透模式*/
    "ANC_BYPASS",	/*BYPASS模式*/
    "ANC_TRAIN",	/*训练模式*/
    "ANC_TRANS_TRAIN",	/*通透训练模式*/
};

/*ANC状态调试信息*/
static const char *anc_state_str[] = {
    "anc_close",	/*关闭状态*/
    "anc_init",		/*初始化状态*/
    "anc_open",		/*打开状态*/
};

/*ANC MSG List*/
enum {
    ANC_MSG_TRAIN_OPEN = 0xA1,
    ANC_MSG_TRAIN_CLOSE,
    ANC_MSG_RUN,
    ANC_MSG_FADE_END,
    ANC_MSG_MODE_SYNC,
    ANC_MSG_TONE_SYNC,
    ANC_MSG_DRC_TIMER,
    ANC_MSG_DEBUG_OUTPUT,
    ANC_MSG_ADAPTIVE_SYNC,
    ANC_MSG_MUSIC_DYN_GAIN,
    ANC_MSG_ICSD_ANC_CMD,
    ANC_MSG_USER_TRAIN_INIT,
    ANC_MSG_USER_TRAIN_RUN,
    ANC_MSG_USER_TRAIN_SETPARAM,
    ANC_MSG_USER_TRAIN_TIMEOUT,
    ANC_MSG_ADT,
    ANC_MSG_USER_TRAIN_END,
    ANC_MSG_MIC_DATA_GET,
    ANC_MSG_RESET,
    ANC_MSG_SZ_FFT_OPEN,
    ANC_MSG_SZ_FFT_RUN,
    ANC_MSG_SZ_FFT_CLOSE,
    ANC_MSG_MODE_SWITCH_IN_ANCTASK,
};

/*ANC MIC动态增益调整状态*/
enum {
    ANC_MIC_DY_STA_INIT = 0,	/*准备状态*/
    ANC_MIC_DY_STA_START,		/*进行状态*/
    ANC_MIC_DY_STA_STOP,		/*停止状态*/
};

#define ANC_CONFIG_LFF_EN ((ANC_TRAIN_MODE & (ANC_HYBRID_EN | ANC_FF_EN)) && (ANC_CH & ANC_L_CH))
#define ANC_CONFIG_LFB_EN ((ANC_TRAIN_MODE & (ANC_HYBRID_EN | ANC_FB_EN)) && (ANC_CH & ANC_L_CH))
#define ANC_CONFIG_RFF_EN ((ANC_TRAIN_MODE & (ANC_HYBRID_EN | ANC_FF_EN)) && (ANC_CH & ANC_R_CH))
#define ANC_CONFIG_RFB_EN ((ANC_TRAIN_MODE & (ANC_HYBRID_EN | ANC_FB_EN)) && (ANC_CH & ANC_R_CH))

/*ANC记忆信息*/
typedef struct {
    u8 mode;		/*当前模式*/
    u8 mode_enable; /*使能的模式*/
#if INEAR_ANC_UI
    u8 inear_tws_mode;
#endif/*INEAR_ANC_UI*/
    //s32 coeff[488];
} anc_info_t;


#if ANC_EAR_ADAPTIVE_EN

typedef struct {
#if ANC_EAR_RECORD_EN
    float record_FL[EAR_RECORD_MAX][ANC_VMDATA_FF_RECORD_SIZE];
    float record_FR[EAR_RECORD_MAX][ANC_VMDATA_FF_RECORD_SIZE];
    int record_num;
    // u8 record_num;
#endif/*ANC_EAR_RECORD_EN*/
    u8 result;
#if ANC_CONFIG_LFF_EN
    float lff_gain;
    anc_fr_t lff[ANC_ADAPTIVE_FF_ORDER];
#endif/*ANC_CONFIG_LFF_EN*/
#if ANC_CONFIG_LFB_EN
    float lfb_gain;
    anc_fr_t lfb[ANC_ADAPTIVE_FB_ORDER];
#if ANC_EAR_ADAPTIVE_CMP_EN
    float lcmp_gain;
    anc_fr_t lcmp[ANC_ADAPTIVE_CMP_ORDER];
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CONFIG_LFB_EN*/
#if ANC_CONFIG_RFF_EN
    float rff_gain;
    anc_fr_t rff[ANC_ADAPTIVE_FF_ORDER];
#endif/*ANC_CONFIG_RFF_EN*/
#if ANC_CONFIG_RFB_EN
    float rfb_gain;
    anc_fr_t rfb[ANC_ADAPTIVE_FB_ORDER];
#if ANC_EAR_ADAPTIVE_CMP_EN
    float rcmp_gain;
    anc_fr_t rcmp[ANC_ADAPTIVE_CMP_ORDER];
#endif/*ANC_EAR_ADAPTIVE_CMP_EN*/
#endif/*ANC_CONFIG_RFB_EN*/
} _PACKED anc_adaptive_iir_t;

#endif/*ANC_EAR_ADAPTIVE_EN */


/*ANC初始化*/
void anc_init(void);

/*ANC训练模式*/
void anc_train_open(u8 mode, u8 debug_sel);

/*ANC关闭训练*/
void anc_train_close(void);

/*
 *ANC状态获取
 *return 0: idle(初始化)
 *return 1: busy(ANC/通透/训练)
 */
u8 anc_status_get(void);

/*获取anc当前模式*/
u8 anc_mode_get(void);

/*获取anc记录的最新的目标ANC模式*/
u8 anc_new_target_mode_get(void);

#define ANC_DAC_CH_L	0
#define ANC_DAC_CH_R	1
/*获取anc模式，dac左右声道的增益*/
u8 anc_dac_gain_get(u8 ch);

/*获取anc模式，ff_mic的增益*/
u8 audio_anc_ffmic_gain_get(void);

/*获取anc模式，fb_mic的增益*/
u8 audio_anc_fbmic_gain_get(void);

/*获取anc模式，指定mic的增益, mic_sel:目标MIC通道*/
u8 audio_anc_mic_gain_get(u8 mic_sel);

/*ANC模式切换(切换到指定模式)，并配置是否播放提示音*/
void anc_mode_switch(u8 mode, u8 tone_play);

/*在anc任务里面切换anc模式，
 *避免上一次切换没有完成，这次切换被忽略的情况*/
void anc_mode_switch_in_anctask(u8 mode, u8 tone_play);

/*ANC模式同步(tws模式)*/
void anc_mode_sync(u8 *data);

void anc_poweron(void);

/*ANC poweroff*/
void anc_poweroff(void);

/*ANC模式切换(下一个模式)*/
void anc_mode_next(void);

/*ANC通过ui菜单选择anc模式,处理快速切换的情景*/
void anc_ui_mode_sel(u8 mode, u8 tone_play);

/*设置ANC支持切换的模式*/
void anc_mode_enable_set(u8 mode_enable);

/*anc coeff 长度大小获取*/
int anc_coeff_size_get(u8 mode);

void anc_coeff_size_set(u8 mode, int len);

int *anc_debug_ctr(u8 free_en);
/*
 *查询当前ANC是否处于训练状态
 *@return 1:处于训练状态
 *@return 0:其他状态
 */
int anc_train_open_query(void);

/*ANC在线调试繁忙标志设置*/
void anc_online_busy_set(u8 en);

/*ANC在线调试繁忙标志获取*/
u8 anc_online_busy_get(void);

/*tws同步播放模式提示音，并且同步进入anc模式*/
void anc_tone_sync_play(int tone_name);

/*anc coeff读接口*/
int *anc_coeff_read(void);

/*anc coeff写接口*/
int anc_coeff_write(int *coeff, u16 len);

/*ANC挂起*/
void anc_suspend(void);

/*ANC恢复*/
void anc_resume(void);

/*通话动态MIC增益开始函数*/
void anc_dynamic_micgain_start(u8 audio_mic_gain);

/*通话动态MIC增益结束函数*/
void anc_dynamic_micgain_stop(void);

/*anc_gains参数读写接口*/
void anc_param_fill(u8 cmd, anc_gain_t *cfg);

/*ANC_DUT audio模块使能函数，用于分离功耗*/
void audio_anc_dut_enable_set(u8 enablebit);

/*ANC自适应TWS同步信息处理*/
void audio_anc_adap_sync(int ref_num, int err_num, int err);
void audio_anc_adap_num_compare(int ref_num, int err_num);

/*设置fb mic为复用mic*/
void audio_anc_mic_mana_fb_mult_set(u8 mult_flag);

//ANC MIC信息管理
void audio_anc_mic_management(audio_anc_t *param);

/*切换ANC自适应模式*/
void audio_anc_adaptive_mode_set(u8 mode, u8 lvl);

/*ANC增益调节 音乐响度检测*/
void audio_anc_music_dynamic_gain_det(s16 *data, int len);

/*ANC增益调节 音乐响度初始化*/
void audio_anc_music_dynamic_gain_init(int sr);

/*ANC增益调节 音乐响度复位*/
void audio_anc_music_dynamic_gain_reset(u8 effect_reset_en);

/*
   自适应模式-重新检测
   param: tws_sync_en	 	1 TWS同步自适应，支持TWS降噪平衡，需左右耳一起调用此接口
   							0 单耳自适应, 不支持TWS降噪平衡，可TWS状态下单耳自适应
 */
int audio_anc_mode_ear_adaptive(u8 tws_sync_en);

/*
   自适应/普通参数切换
	param:  mode 		0 使用普通参数; 1 使用自适应参数
			tone_play 	0 不播放提示音；1 播放提示音
			update_flag 0 不更新效果；	1 即时更新效果
 */
int audio_anc_coeff_adaptive_set(u32 mode, u8 tone_play, u8 update_flag);

/*ANC滤波器模式循环切换*/
int audio_anc_coeff_adaptive_switch();

/*判断当前是否处于训练中*/
u8 audio_anc_adaptive_busy_get(void);

/*当前ANC滤波器模式获取 0:普通参数 1:自适应参数*/
int audio_anc_coeff_mode_get(void);

/*强制停止ANC训练 - 并用回默认参数*/
void anc_ear_adaptive_forced_exit(void);

/*自适应训练失败处理回调函数*/
void audio_anc_adaptive_fail_callback(u8 anc_err);

void audio_anc_develop_set(u8 mode);

u8 audio_anc_develop_get(void);

/*自适应主从序号*/
u8 anc_ear_adaptive_seq_get(void);

void audio_anc_param_map(u8 coeff_en, u8 gain_en);

/*
   多滤波器-场景切换
	param:  scene 	    场景ID
			update_flag 非ANC_OFF时，即时更新效果: 0 不更新；1 更新
	Note: ID 从1 开始
 */
void audio_anc_mult_scene_set(u16 scene_id, u8 update_flag);

/*多滤波器-获取当前场景的滤波器*/
u8 audio_anc_mult_scene_get(void);

/*多滤波器-最大场景个数设置*/
void audio_anc_mult_scene_max_set(u8 scene_cnt);

/*多滤波器-最大场景个数获取*/
u8 audio_anc_mult_scene_max_get(void);

/*多滤波器-场景循环切换*/
void audio_anc_mult_scene_switch(u8 tone_flag);

/*
   ANC结构设置, 下一次切模式或复位ANC时生效
	param: enablebit ANC结构配置，如(ANC_FF_EN/ANC_FB_EN/ANC_HYBRID_EN)
		   update_flag 非ANC_OFF时，即时更新效果: 0 不更新；1 更新
 */
void audio_anc_enable_set(u8 enablebit, u8 update_flag);

//ANC产测/调试模式设置
void audio_anc_prodution_mode_set(u8 mode);

extern int anc_uart_write(u8 *buf, u8 len);
extern void ci_send_packet(u32 id, u8 *packet, int size);
extern void sys_auto_shut_down_enable(void);
extern void sys_auto_shut_down_disable(void);

#endif/*AUDIO_ANC_H*/
