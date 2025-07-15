#ifndef _AUD_MIC_DUT_H_
#define _AUD_MIC_DUT_H_

#include "generic/typedef.h"


#define MIC_DUT_DATA_SEND_BY_TIMER      0 /*1:使用定时器发spp数据，0:使用任务发数*/

/*定时器发送间隔*/
#define MIC_DATA_SEND_INTERVAL     		4

//MIC采样率bitmap
#define MIC_ADC_SR_8000     BIT(0)
#define MIC_ADC_SR_11025    BIT(1)
#define MIC_ADC_SR_12000    BIT(2)
#define MIC_ADC_SR_16000    BIT(3)
#define MIC_ADC_SR_22050    BIT(4)
#define MIC_ADC_SR_24000    BIT(5)
#define MIC_ADC_SR_32000    BIT(6)
#define MIC_ADC_SR_44100    BIT(7)
#define MIC_ADC_SR_48000    BIT(8)

typedef struct {
    u16 version;
    u16 amic_enable_bit;        /*模拟MIC使能位：AMIC0-AMIC15，默认使能模拟MIC最低使能位*/
    u16 dmic_enable_bit;        /*数字MIC使能位：DMIC0-DMIC15*/
    u8 channel;                 /*声道*/
    u8 bit_wide;                /*位宽，有符号*/
    u32 sr_enable_bit;          /*采样率支持列表使能位*/
    u32 sr_default;             /*采样率默认值：16000*/
    u16 mic_gain;               /*MIC增益范围*/
    u16 mic_gain_default;       /*MIC增益默认值*/
    u16 dac_gain;               /*DAC增益范围*/
    u16 dac_gain_default;       /*DAC增益默认值*/
} mic_dut_info_t;


/*获取当前设置的mic增益*/
int mic_dut_online_get_mic_gain(void);

/*获取当前设置的dac增益*/
int mic_dut_online_get_dac_gain(void);

/*获取当前设置的采样率*/
int mic_dut_online_get_sample_rate(void);

/*获取当前使能的模拟mic*/
u8 mic_dut_online_amic_enable_bit(void);

/*获取当前使能的数字mic*/
u8 mic_dut_online_dmic_enable_bit(void);

/*获取当前在线使用的mic*/
int mic_dut_online_get_mic_idx(void);

/*获取mic dut状态*/
int mic_dut_online_get_mic_state(void);

/*获取dut scan状态*/
int mic_dut_online_get_scan_state(void);


int aud_mic_dut_open(void);
int aud_mic_dut_close(void);


#endif/*_AUD_MIC_DUT_H_*/

