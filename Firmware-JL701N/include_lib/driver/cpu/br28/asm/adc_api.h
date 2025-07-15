#ifndef __ADC_API_H__
#define __ADC_API_H__


//AD channel define
#define AD_CH_PA0    (0x0)
#define AD_CH_PA5    (0x1)
#define AD_CH_PA6    (0x2)
#define AD_CH_PA8    (0x3)
#define AD_CH_PC4    (0x4)
#define AD_CH_PC5    (0x5)
#define AD_CH_PB1    (0x6)
#define AD_CH_PB2    (0x7)
#define AD_CH_PB5    (0x8)
#define AD_CH_PB9    (0x9)
#define AD_CH_DP     (0xa)
#define AD_CH_DM     (0xb)
#define AD_CH_PG0    (0xc)
#define AD_CH_PG1    (0xd)
#define AD_CH_PG5    (0xe)
#define AD_CH_PG7    (0xf)

#define AD_CH_BT     (0x10)
#define AD_CH_PMU    (0x11)
#define AD_CH_AUDIO  (0x12)
#define AD_CH_LPCTM  (0x13)
#define AD_CH_X32K   (0x14)
#define AD_CH_PLL1   (0x15)

#define ADC_PMU_CH_VBG      (0x0<<16)//MVBG/WVBG
#define ADC_PMU_CH_VSW      (0x1<<16)
#define ADC_PMU_CH_PROGI    (0x2<<16)
#define ADC_PMU_CH_PROGF    (0x3<<16)
#define ADC_PMU_CH_VTEMP    (0x4<<16)
#define ADC_PMU_CH_VPWR     (0x5<<16) //1/4vpwr
#define ADC_PMU_CH_VBAT     (0x6<<16)  //1/4vbat
// #define ADC_PMU_CH_VBAT_2   (0x7<<16)
#define ADC_PMU_CH_VB17     (0x8<<16)
#define ADC_PMU_CH_IOVDD2   (0x9<<16)
#define ADC_PMU_CH_VDC15    (0xa<<16)
#define ADC_PMU_CH_DVDD     (0xb<<16)
#define ADC_PMU_CH_RVDD     (0xc<<16)
#define ADC_PMU_CH_TWVDD    (0xd<<16)
#define ADC_PMU_CH_PVDD     (0xe<<16)
#define ADC_PMU_CH_EVDD     (0xf<<16)


#define AD_CH_PMU_VBG   (AD_CH_PMU | ADC_PMU_CH_VBG)
#define AD_CH_VDC15     (AD_CH_PMU | ADC_PMU_CH_VDC15)
#define AD_CH_SYSVDD    (AD_CH_PMU | ADC_PMU_CH_DVDD)
#define AD_CH_DTEMP     (AD_CH_PMU | ADC_PMU_CH_VTEMP)
#define AD_CH_VBAT      (AD_CH_PMU | ADC_PMU_CH_VBAT)
#define AD_CH_LDO5V     (AD_CH_PMU | ADC_PMU_CH_VPWR)
#define AD_CH_WVDD      (AD_CH_PMU | ADC_PMU_CH_TWVDD)
#define AD_CH_PVDD      (AD_CH_PMU | ADC_PMU_CH_PVDD)


#define AD_CH_LDOREF    AD_CH_PMU_VBG


// #define AD_CH_BT_VBG    (AD_CH_BT |  (0x8<<11)) //WLA_CON0[14:11]= 0b1000

// #define AD_CH_LDOREF    AD_CH_BT_VBG

// #define AD_CH_PMU_VBG   (AD_CH_PMU | ADC_PMU_CH_VBG)
// #define AD_CH_VDC13     (AD_CH_PMU | ADC_PMU_CH_VDC13)
// #define AD_CH_SYSVDD    (AD_CH_PMU | ADC_PMU_CH_SYSVDD)
// #define AD_CH_DTEMP     (AD_CH_PMU | ADC_PMU_CH_DTEMP)
// #define AD_CH_VBAT      (AD_CH_PMU | ADC_PMU_CH_VBAT)
// #define AD_CH_LDO5V     (AD_CH_PMU | ADC_PMU_CH_LDO5V)
// #define AD_CH_WVDD      (AD_CH_PMU | ADC_PMU_CH_WVDD)

#define AD_AUDIO_VADADC     ((BIT(0))<<16)
#define AD_AUDIO_VCM        ((BIT(1))<<16)
#define AD_AUDIO_VBGLDO     ((BIT(2))<<16)
#define AD_AUDIO_HPVDD      ((BIT(3))<<16)
#define AD_AUDIO_RN         ((BIT(4))<<16)
#define AD_AUDIO_RP         ((BIT(5))<<16)
#define AD_AUDIO_LN         ((BIT(6))<<16)
#define AD_AUDIO_LP         ((BIT(7))<<16)
#define AD_AUDIO_DACVDD     ((BIT(8))<<16)

#define AD_CH_AUDIO_VADADC    (AD_CH_AUDIO | AD_AUDIO_VADADC)
#define AD_CH_AUDIO_VCM       (AD_CH_AUDIO | AD_AUDIO_VCM)
#define AD_CH_AUDIO_VBGLDO    (AD_CH_AUDIO | AD_AUDIO_VBGLDO)
#define AD_CH_AUDIO_HPVDD     (AD_CH_AUDIO | AD_AUDIO_HPVDD)
#define AD_CH_AUDIO_RN        (AD_CH_AUDIO | AD_AUDIO_RN)
#define AD_CH_AUDIO_RP        (AD_CH_AUDIO | AD_AUDIO_RP)
#define AD_CH_AUDIO_LN        (AD_CH_AUDIO | AD_AUDIO_LN)
#define AD_CH_AUDIO_LP        (AD_CH_AUDIO | AD_AUDIO_LP)
#define AD_CH_AUDIO_DACVDD    (AD_CH_AUDIO | AD_AUDIO_DACVDD)

#define     ADC_MAX_CH  10


extern void adc_init();
extern void adc_vbg_init();
//p33 define

extern void adc_pmu_ch_select(u32 ch);
extern void adc_pmu_detect_en(u32 ch);
extern void adc_vdc13_save();
extern void adc_vdc13_restore();

//
u32 adc_get_value(u32 ch);
u32 adc_add_sample_ch(u32 ch);
u32 adc_remove_sample_ch(u32 ch);
u32 adc_get_voltage(u32 ch);
u32 adc_check_vbat_lowpower();
u32 adc_set_sample_freq(u32 ch, u32 ms);
u32 adc_sample(u32 ch);

int get_hpvdd_voltage(void);

u32 get_vdd_voltage(u32 ch);
u32 get_vddio_voltage();

#endif
