#ifndef _ANC_BTSPP_H_
#define _ANC_BTSPP_H_

#include "generic/typedef.h"
#include "asm/anc.h"

//前期训练 建议先做喇叭测试，再做MIC测试。这样内部可以通过喇叭判断MIC好不好
//-------------------ANC SPP 协议 -------------------//

//父命令
#define	ANC_COMMAND_RESPO			0x00		//命令回复
#define ANC_TRAIN_NOMA_MODE			0x01 		//普通训练模式
#define ANC_TRAIN_FAST_MODE			0x02 		//快速训练模式

//训练工具子命令1
#define ANC_CHANGE_COMMAND			0x00		//父命令模式切换
#define	ANC_MUTE_TARIN				0x01		//静音训练
#define	ANC_NOISE_TARIN   			0x02 		//噪声训练
#define ANC_TARIN_AGAIN    			0x03		//继续训练
#define ANC_MODE_OFF				0x04		//ANC关
#define ANC_MODE_ON					0x05		//ANC降噪模式
#define ANC_TRAIN_EXIT				0x06		//ANC训练结束

#define ANC_PASS_MODE_ON			0x07		//ANC通透模式
#define ANC_TRAIN_STEP_1			0x08		//ANC训练步进1
#define ANC_TRAIN_STEP_2			0x09		//ANC训练步进2


#define ANC_TRAIN_STEP_SET          0x15		//设置ANC训练步进
#define ANC_SZ_LOW_THR_SET          0x16        //设置SZ MIC下限能量阈值
#define ANC_FZ_LOW_THR_SET          0x17        //设置FZ MIC下限能量阈值
#define ANC_SZ_ADAP_THR_SET			0x18		//设置SZ 自适应收敛阈值
#define ANC_FZ_ADAP_THR_SET			0x19		//设置FZ 自适应收敛阈值
#define ANC_AUTO_TS_EN_SET			0x1a		//设置步进自适应微调使能

#define ANC_NONADAP_TIME_SET		0x1b		//设置非自适应收敛时间
#define ANC_SZ_ADAP_TIME_SET		0x1c		//设置SZ自适应收敛时间
#define ANC_FZ_ADAP_TIME_SET		0x1d		//设置FZ自适应收敛时间
#define ANC_WZ_TRAIN_TIME_SET		0x1e		//设置WZ训练时间


//以下为开发者测试指令
#define ANC_DEVELOPER_MODE			0x20		//开发者模式
#define ANC_TEST_NADAP				0x21		//关闭ANC自适应收敛
#define ANC_TEST_ADAP				0x22		//开启ANC自适应收敛
#define ANC_TEST_WZ_BREAK			0x23		//退出噪声训练
#define ANC_CHANGE_MODE				0x24		//模式设置指令

#define ANC_TRAIN_STEP_CONNUTE      0x30        //继续找步进
#define ANC_MUTE_TRAIN_GET_DATA     0x31        //获取静音训练的SPK MIC数据
#define ANC_TRANS_MUTE_TARIN 	  	0x32		//通透模式训练
#define ANC_MIC_DMA_EXPORT_EN		0x50		//设置输出ANC 的MIC数据

#define ANC_REF_MIC_GAIN_SET		0x11		//设置参考MIC的增益
#define ANC_ERR_MIC_GAIN_SET		0x12		//设置误差MIC的增益
#define ANC_DAC_GAIN_SET			0x13		//设置DAC的模拟增益
#define ANC_FFGAIN_SET				0x14		//设置ANC的数字增益
#define ANC_FBGAIN_SET				0x1f		//设置ANC的FBGAIN增益

#define ANC_SAMPLE_RATE_SET         0x40        //设置ANC采样率
#define ANC_ORDER_SET               0x41        //设置ANC阶数
#define ANC_TRANS_HPF_SET           0x42        //设置通透高通滤波器
#define ANC_TRANS_LPF_SET           0x43        //设置通透低通滤波器
#define ANC_TRANS_GAIN_SET          0x44        //设置通透增益
#define ANC_TRANS_SAMPLE_RATE_SET   0x45        //设置通透采样率
#define ANC_TRANS_ORDER_SET         0x46        //设置通透阶数

#define ANC_CMP_EN_SET				0x51		//设置CMP使能
#define ANC_DRC_EN_SET				0x52		//设置DRC使能
#define ANC_AHS_EN_SET				0x53		//设置AHS使能
#define ANC_DCC_SEL_SET				0x54		//设置DCC档位
#define ANC_GAIN_SIGN_SET			0x55		//设置ANC各类增益的符号
#define ANC_NOISE_LVL_SET			0x56		//设置训练噪声等级
#define ANC_CMP_GAIN_SET			0x57		//设置ANC左声道CMP增益
#define ANC_RFF_GAIN_SET			0x58		//设置ANC右声道FF增益
#define ANC_RFB_GAIN_SET			0x59		//设置ANC右声道FB增益
#define ANC_RTRANS_GAIN_SET			0x5A		//设置ANC右声道通透增益
#define ANC_RCMP_GAIN_SET			0x5B		//设置ANC右声道CMP增益
#define ANC_DRCFF_ZERO_DET_SET		0x5C		//设置DRCFF过零检测使能
#define ANC_DRCFF_DAT_MODE_SET		0x5D		//设置DRCFF_DAT模式
#define ANC_DRCFF_LPF_SEL_SET		0x5E		//设置DRCFF_LPF档位
#define ANC_DRCFB_ZERO_DET_SET		0x5F		//设置DRCFB过零检测使能
#define ANC_DRCFB_DAT_MODE_SET		0x60		//设置DRCFB_DAT模式
#define ANC_DRCFB_LPF_SEL_SET		0x61		//设置DRCFB_LPF档位
#define ANC_DRCFF_LTHR_SET			0x62		//设置DRCFF_LOW阈值
#define ANC_DRCFF_HTHR_SET			0x63		//设置DRCFF_HIGH阈值
#define ANC_DRCFF_LGAIN_SET			0x64		//设置DRCFF_LOW增益
#define ANC_DRCFF_HGAIN_SET			0x65		//设置DRCFF_HIGH增益
#define ANC_DRCFF_NORGAIN_SET		0x66		//设置DRCFF_NOR增益
#define ANC_DRCFB_LTHR_SET			0x67		//设置DRCFB_LOW阈值
#define ANC_DRCFB_HTHR_SET			0x68		//设置DRCFB_HIGH阈值
#define ANC_DRCFB_LGAIN_SET			0x69		//设置DRCFB_LOW增益
#define ANC_DRCFB_HGAIN_SET			0x6A		//设置DRCFB_HIGH增益
#define ANC_DRCFB_NORGAIN_SET		0x6B		//设置DRCFB_NOR增益
#define ANC_DRCTRANS_LTHR_SET		0x6C		//设置DRCTRANS_LOW阈值
#define ANC_DRCTRANS_HTHR_SET		0x6D		//设置DRCTRANS_HIGH阈值
#define ANC_DRCTRANS_LGAIN_SET		0x6E		//设置DRCTRANS_LOW增益
#define ANC_DRCTRANS_HGAIN_SET		0x6F		//设置DRCTRANS_HIGH增益
#define ANC_DRCTRANS_NORGAIN_SET	0x70		//设置DRCTRANS_NOR增益
#define ANC_AHS_DLY_SET				0x71		//设置啸叫抑制_DLY
#define ANC_AHS_TAP_SET             0x72		//设置啸叫抑制_TAP
#define ANC_AHS_WN_SHIFT_SET		0x73		//设置啸叫抑制_WN_SHIFT
#define ANC_AHS_WN_SUB_SET			0x74		//设置啸叫抑制_WN_SUB
#define ANC_AHS_SHIFT_SET			0x75		//设置啸叫抑制_SHIFT
#define ANC_AHS_U_SET				0x76		//设置啸叫抑制步进
#define ANC_AHS_GAIN_SET			0x77		//设置啸叫抑制增益
#define ANC_FADE_STEP_SET			0x78		//设置淡入淡出步进
#define ANC_AUDIO_DRC_THR_SET		0x79		//设置AUDIO DRC阈值
#define ANC_AHS_NLMS_SEL_SET	    0x7A		//设置啸叫抑制NLMS
#define ANC_R_FFMIC_GAIN_SET	    0x7B		//设置ANCR_FFMIC增益
#define ANC_R_FBMIC_GAIN_SET	    0x7C		//设置ANCR_FBMIC增益
#define ANC_FB_1ST_DCC_SET	    	0x7D		//设置FB1阶DCC
#define ANC_FF_2ND_DCC_SET	    	0x7E		//设置FF2阶DCC
#define ANC_FB_2ND_DCC_SET	    	0x7F		//设置FB2阶DCC
#define ANC_DRC_FF_2DCC_SET	    	0x80		//设置DRCFF动态DCC目标档位
#define ANC_DRC_FB_2DCC_SET	    	0x81		//设置DRCFB动态DCC目标档位
#define ANC_DRC_DCC_DET_TIME_SET	0x82		//设置DRC动态DCC检测时间
#define ANC_DRC_DCC_RES_TIME_SET	0x83		//设置DRC动态DCC恢复时间
//0x84-0x8A 36啸叫抑制参数使用
#define ANC_ADAPTIVE_REF_FB_FRE		0x8B		//设置自适应FB参考 Fre
#define ANC_ADAPTIVE_REF_FB_G		0x8C		//设置自适应FB参考 G
#define ANC_ADAPTIVE_REF_FB_Q		0x8D 		//设置自适应FB参考 Q

#define ANC_ADAPTIVE_REF_EN			0x8E 		//金机曲线使能控制

#define ANC_LFF_GAIN_S_SET			0x8F		//设置ANC左声道FF增益与符号
#define ANC_LFB_GAIN_S_SET			0x90		//设置ANC左声道FB增益与符号
#define ANC_LTRANS_GAIN_S_SET		0x91		//设置ANC左声道通透增益与符号
#define ANC_LCMP_GAIN_S_SET			0x92		//设置ANC左声道CMP增益与符号

#define ANC_RFF_GAIN_S_SET			0x93		//设置ANC右声道FF增益与符号
#define ANC_RFB_GAIN_S_SET			0x94		//设置ANC右声道FB增益与符号
#define ANC_RTRANS_GAIN_S_SET		0x95		//设置ANC右声道通透增益与符号
#define ANC_RCMP_GAIN_S_SET			0x96		//设置ANC右声道CMP增益与符号



//----------------耳机反馈子命令1---------------------//
#define	ANC_EXEC_SUCC				0x01		//执行成功
#define ANC_EXEC_FAIL				0x02		//执行失败
#define ANC_TRIM_MIC_WORK_FAIL		0x80|BIT(0)	//MIC1工作不正常
#define ANC_MIC_WORK_FAIL   		0x80|BIT(1)	//MIC0工作不正常
#define ANC_TRIM_MIC_SNR_FAIL   	0x80|BIT(2)	//MIC1信噪比差
#define ANC_MIC_SNR_FAIL   			0x80|BIT(3) //MIC0信噪比差
#define ANC_TRIM_MIC_NOISE_FAIL  	0x80|BIT(4) //MIC1底噪高
#define ANC_MIC_NOISE_FAIL   		0x80|BIT(5) //MIC0底噪高
//多种错误信息组合为|的形式,如MIC0/MIC1工作都不正常，则返回0x80|BIT(0)|BIT(1)

//---------------------spk_mic_dat param--------------//
#define ANC_R_SPK_L_SPK				0x01		//高16bit R spk数据, 低16bit L spk 数据
#define ANC_R_SPK_R_ERRMIC			0x02
#define ANC_R_SPK_R_REFMIC			0x03
#define ANC_L_SPK_L_ERRMIC			0x04
#define ANC_L_SPK_L_REFMIC			0x05
#define ANC_R_ERRMIC_R_REFMIC		0x06
#define ANC_L_ERRMIC_L_REFMIC		0x07

//注意FF/FB/Hybrid/只能选择一种

#define ANC_MUTE_EN					BIT(0)		//通透静音训练使能
#define ANC_NOISE_EN				BIT(1)		//通透噪声训练使能


#define ANC_SPP_PACK_NUM		10		//数据包长度

#define ANC_SPP_MAGIC			0x55AA

enum {
    ANC_SZ_MUTE_STATUS = 0,					//SZ非自适应（DAC静音）状态
    ANC_SZ_NADAP_STATUS,					//SZ非自适应状态
    ANC_SZ_ADAP_STATUS,						//SZ自适应状态
    ANC_FZ_MUTE_STATUS,				    	//FZ非自适应（DAC静音）状态
    ANC_FZ_NADAP_STATUS,					//FZ非自适应状态
    ANC_FZ_ADAP_STATUS,						//FZ自适应状态
    ANC_WZ_NADAP_STATUS,					//WZ非自适应状态
    ANC_WZ_ADAP_STATUS,						//WZ自适应状态
    ANC_WZ_END_STATUS,						//WZ训练结束
};

enum {
    ANC_TRAIN_SZ = 0,
    ANC_TRAIN_FZ,
    ANC_TRAIN_WZ,
};

typedef struct {
    u16 magic;
    u16 crc;
    u16 len;
    u8 dat[4];
} anc_spp_data_t;



typedef struct {
    anc_train_para_t *para;
    audio_anc_t *param;
    u8 developer_mode;		//开发者模式
    u8 noise_exit;			//退出噪声训练
    u8 pow_dat[10];
    u8 status;
    u8 train_step_num;
    int *anc_gain;
    int *anc_fbgain;
    anc_spp_data_t rx_buf;
    anc_spp_data_t tx_buf;

} anc_spp_t;

typedef struct {
    u8 mode;
    u8 ff_en;
    u8 fb_en;
    u16 fb0_gain;
    u16 fb1_gain;
} anc_train_reg_t;



void anc_debug_init(audio_anc_t *param);
void anc_debug_uninit(void);
int anc_spp_event_deal(u8 *dat);
void anc_train_api_set(u8 cmd, u32 data, anc_train_para_t *para);
int anc_spp_rx_packet(u8 *dat, u8 len);
int anc_spp_tx_packet(u8 mode, u8 command, u16 command_dat);
u8 anc_powdat_analysis(u32 pow);
void anc_coeff_max_get(s32 *coeff, s32 *dat, u16 len, u8 type);

void anc_btspp_display_pow(anc_ack_msg_t *anc_ack);
void anc_btspp_status_set(u8 status);

#endif/*_ANC_BTSPP_H_*/

