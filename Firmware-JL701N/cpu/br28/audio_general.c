/*
 ****************************************************************
 *							AUDIO General APIs
 * Brief : 实现一些依赖CPU的通用接口，适配所有case
 * Notes :
 ****************************************************************
 */
#include "system/includes.h"
#include "media/includes.h"
#include "audio_config.h"

void audio_adda_dump(void) //打印所有的dac,adc寄存器
{
    printf("JL_WL_AUD CON0:%x", JL_WL_AUD->CON0);
    printf("AUD_CON:%x", JL_AUDIO->AUD_CON);
    printf("DAC_CON:%x", JL_AUDIO->DAC_CON);
    printf("ADC_CON:%x", JL_AUDIO->ADC_CON);
    printf("ADC_CON1:%x", JL_AUDIO->ADC_CON1);
    printf("DAC_TM0:%x", JL_AUDIO->DAC_TM0);
    printf("DAA_CON 0:%x 	1:%x,	2:%x,	3:%x    4:%x\n", JL_ADDA->DAA_CON0, JL_ADDA->DAA_CON1, JL_ADDA->DAA_CON2, JL_ADDA->DAA_CON3, JL_ADDA->DAA_CON4);
    printf("ADA_CON 0:%x	1:%x	2:%x	3:%x	4:%x	5:%x\n", JL_ADDA->ADA_CON0, JL_ADDA->ADA_CON1, JL_ADDA->ADA_CON2, JL_ADDA->ADA_CON3, JL_ADDA->ADA_CON4, JL_ADDA->ADA_CON5);
}

void audio_gain_dump(void)
{
    int dac_again_max = 7;
    u8 dac_again_l = JL_ADDA->DAA_CON1 & 0xF;
    u8 dac_again_r = (JL_ADDA->DAA_CON1 >> 4) & 0xF;
    int dac_dgain_max = 16384;
    u32 dac_dgain_l = JL_AUDIO->DAC_VL0 & 0xFFFF;
    u32 dac_dgain_r = (JL_AUDIO->DAC_VL0 >> 16) & 0xFFFF;

    u8 mic0_0_6 = (JL_ADDA->ADA_CON4) & 0x1;
    u8 mic1_0_6 = (JL_ADDA->ADA_CON5) & 0x1;
    u8 mic2_0_6 = (JL_ADDA->ADA_CON6) & 0x1;
    u8 mic3_0_6 = (JL_ADDA->ADA_CON7) & 0x1;

    u8 mic0_gain = JL_ADDA->ADA_CON8 & 0x1F;
    u8 mic1_gain = (JL_ADDA->ADA_CON8 >> 5) & 0x1F;
    u8 mic2_gain = (JL_ADDA->ADA_CON8 >> 10) & 0x1F;
    u8 mic3_gain = (JL_ADDA->ADA_CON8 >> 15) & 0x1F;

    printf("L Mute:%d ,R Mute:%d\n", (JL_ADDA->DAA_CON2 >> 1) & 0x1, (JL_ADDA->DAA_CON2 >> 11) & 0x1); //PA MUTE
    printf("MIC_G:%d,%d,%d,%d,MIC_6dB_EN:%d,%d,%d,%d,DAC_AG_MAX:%d,DAC_AG:%d,%d,DAC_DG_MAX:%d,DAC_DG:%d,%d\n", mic0_gain, mic1_gain, mic2_gain, mic3_gain, mic0_0_6, mic1_0_6, mic2_0_6, mic3_0_6, dac_again_max, dac_again_l, dac_again_r, dac_dgain_max, dac_dgain_l, dac_dgain_r);

}
