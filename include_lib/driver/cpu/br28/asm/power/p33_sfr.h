#ifndef __BR28_P33__
#define __BR28_P33__
////////////////////////////////

#ifdef PMU_SYSTEM
#define P33_ACCESS(x) (*(volatile u32 *)(0xc000 + x*4))
#else
#define P33_ACCESS(x) (*(volatile u32 *)(0xf20000 + 0xc000 + x*4))
#endif

#ifdef PMU_SYSTEM
#define RTC_ACCESS(x) (*(volatile u32 *)(0xd000 + x*4))
#else
#define RTC_ACCESS(x) (*(volatile u32 *)(0xf20000 + 0xd000 + x*4))
#endif

//===========
//===============================================================================//
//
//
//
//===============================================================================//
//............. 0x0000 - 0x000f............
#define P3_IOV2_CON                       P33_ACCESS(0x00)

//............. 0x0010 - 0x001f............ for analog others
#define P3_OSL_CON                        P33_ACCESS(0x10)
#define P3_VLVD_CON                       P33_ACCESS(0x11)
#define P3_RST_SRC                        P33_ACCESS(0x12)
#define P3_LRC_CON0                       P33_ACCESS(0x13)
#define P3_LRC_CON1                       P33_ACCESS(0x14)
#define P3_RST_CON0                       P33_ACCESS(0x15)
#define P3_ANA_KEEP                       P33_ACCESS(0x16)
#define P3_VLD_KEEP                       P33_ACCESS(0x17)
#define P3_CLK_CON0                       P33_ACCESS(0x18)
#define P3_ANA_READ                       P33_ACCESS(0x19)
#define P3_CHG_CON0                       P33_ACCESS(0x1a)
#define P3_CHG_CON1                       P33_ACCESS(0x1b)
#define P3_CHG_CON2                       P33_ACCESS(0x1c)
#define P3_CHG_CON3                       P33_ACCESS(0x1d)

//............. 0x0020 - 0x002f............ for PWM LED
//#define P3_PWM_CON0                       P33_ACCESS(0x20)
//#define P3_PWM_CON1                       P33_ACCESS(0x21)
//#define P3_PWM_CON2                       P33_ACCESS(0x22)
//#define P3_PWM_CON3                       P33_ACCESS(0x23)
//#define P3_PWM_BRI_PRDL                   P33_ACCESS(0x24)
//#define P3_PWM_BRI_PRDH                   P33_ACCESS(0x25)
//#define P3_PWM_BRI_DUTY0L                 P33_ACCESS(0x26)
//#define P3_PWM_BRI_DUTY0H                 P33_ACCESS(0x27)
//#define P3_PWM_BRI_DUTY1L                 P33_ACCESS(0x28)
//#define P3_PWM_BRI_DUTY1H                 P33_ACCESS(0x29)
//#define P3_PWM_PRD_DIVL                   P33_ACCESS(0x2a)
//#define P3_PWM_DUTY0                      P33_ACCESS(0x2b)
//#define P3_PWM_DUTY1                      P33_ACCESS(0x2c)
//#define P3_PWM_DUTY2                      P33_ACCESS(0x2d)
//#define P3_PWM_DUTY3                      P33_ACCESS(0x2e)
//#define P3_PWM_CNT_RD                     P33_ACCESS(0x2f)

//............. 0x0030 - 0x003f............ for PMU manager
#define P3_PMU_CON0                       P33_ACCESS(0x30)

#define P3_SFLAG0                         P33_ACCESS(0x38)
#define P3_SFLAG1                         P33_ACCESS(0x39)
#define P3_SFLAG2                         P33_ACCESS(0x3a)
#define P3_SFLAG3                         P33_ACCESS(0x3b)
//#define P3_SFLAG4                         P33_ACCESS(0x3c)
//#define P3_SFLAG5                         P33_ACCESS(0x3d)
//#define P3_SFLAG6                         P33_ACCESS(0x3e)
//#define P3_SFLAG7                         P33_ACCESS(0x3f)

//............. 0x0040 - 0x004f............ for
#define P3_IVS_RD                         P33_ACCESS(0x40)
#define P3_IVS_SET                        P33_ACCESS(0x41)
#define P3_IVS_CLR                        P33_ACCESS(0x42)
#define P3_PVDD0_AUTO                     P33_ACCESS(0x43)
#define P3_PVDD1_AUTO                     P33_ACCESS(0x44)
#define P3_WKUP_DLY                       P33_ACCESS(0x45)
#define P3_VLVD_FLT                       P33_ACCESS(0x46)
#define P3_PINR_CON1                      P33_ACCESS(0x47)
#define P3_PINR_CON                       P33_ACCESS(0x48)
#define P3_PCNT_CON                       P33_ACCESS(0x49)
#define P3_PCNT_SET0                      P33_ACCESS(0x4a)
#define P3_PCNT_SET1                      P33_ACCESS(0x4b)
#define P3_PCNT_DAT0                      P33_ACCESS(0x4c)
#define P3_PCNT_DAT1                      P33_ACCESS(0x4d)
#define P3_PVDD2_AUTO                     P33_ACCESS(0x4e)

//............. 0x0050 - 0x005f............ for port wake up
#define P3_WKUP_EN0                       P33_ACCESS(0x50)
#define P3_WKUP_EN1                       P33_ACCESS(0x51)
#define P3_WKUP_EDGE0                     P33_ACCESS(0x52)
#define P3_WKUP_EDGE1                     P33_ACCESS(0x53)
#define P3_WKUP_LEVEL0                    P33_ACCESS(0x54)
#define P3_WKUP_LEVEL1                    P33_ACCESS(0x55)
#define P3_WKUP_PND0                      P33_ACCESS(0x56)
#define P3_WKUP_PND1                      P33_ACCESS(0x57)
#define P3_WKUP_CPND0                     P33_ACCESS(0x58)
#define P3_WKUP_CPND1                     P33_ACCESS(0x59)

//............. 0x0060 - 0x006f............ for
#define P3_AWKUP_EN                       P33_ACCESS(0x60)
#define P3_AWKUP_P_IE                     P33_ACCESS(0x61)
#define P3_AWKUP_N_IE                     P33_ACCESS(0x62)
#define P3_AWKUP_LEVEL                    P33_ACCESS(0x63)
#define P3_AWKUP_INSEL                    P33_ACCESS(0x64)
#define P3_AWKUP_P_PND                    P33_ACCESS(0x65)
#define P3_AWKUP_N_PND                    P33_ACCESS(0x66)
#define P3_AWKUP_P_CPND                   P33_ACCESS(0x67)
#define P3_AWKUP_N_CPND                   P33_ACCESS(0x68)

//............. 0x0070 - 0x007f............ for power gate
//#define P3_PGDR_CON0                      P33_ACCESS(0x70)
//#define P3_PGDR_CON1                      P33_ACCESS(0x71)
#define P3_PGSD_CON                       P33_ACCESS(0x72)
//#define P3_PGFS_CON                       P33_ACCESS(0x73)

//............. 0x0080 - 0x008f............ for
#define P3_AWKUP_FLT0                     P33_ACCESS(0x80)
#define P3_AWKUP_FLT1                     P33_ACCESS(0x81)
#define P3_AWKUP_FLT2                     P33_ACCESS(0x82)

#define P3_APORT_SEL0                     P33_ACCESS(0x88)
#define P3_APORT_SEL1                     P33_ACCESS(0x89)
#define P3_APORT_SEL2                     P33_ACCESS(0x8a)

//............. 0x0090 - 0x009f............ for analog control
#define P3_ANA_CON0                       P33_ACCESS(0x90)
#define P3_ANA_CON1                       P33_ACCESS(0x91)
#define P3_ANA_CON2                       P33_ACCESS(0x92)
#define P3_ANA_CON3                       P33_ACCESS(0x93)
#define P3_ANA_CON4                       P33_ACCESS(0x94)
#define P3_ANA_CON5                       P33_ACCESS(0x95)
#define P3_ANA_CON6                       P33_ACCESS(0x96)
#define P3_ANA_CON7                       P33_ACCESS(0x97)
#define P3_ANA_CON8                       P33_ACCESS(0x98)
#define P3_ANA_CON9                       P33_ACCESS(0x99)
#define P3_ANA_CON10                      P33_ACCESS(0x9a)
#define P3_ANA_CON11                      P33_ACCESS(0x9b)
#define P3_ANA_CON12                      P33_ACCESS(0x9c)
#define P3_ANA_CON13                      P33_ACCESS(0x9d)
#define P3_ANA_CON14                      P33_ACCESS(0x9e)
#define P3_ANA_CON15                      P33_ACCESS(0x9f)

//............. 0x00a0 - 0x00af............
#define P3_PR_PWR                         P33_ACCESS(0xa0)
#define P3_L5V_CON0                       P33_ACCESS(0xa1)
#define P3_L5V_CON1                       P33_ACCESS(0xa2)

#define P3_LS_P11                         P33_ACCESS(0xa4)

#define P3_RVD_CON                        P33_ACCESS(0xa7)
#define P3_WKUP_SRC                       P33_ACCESS(0xa8)

#define P3_PMU_DBG_CON                    P33_ACCESS(0xaa)
#define P3_ANA_KEEP1                      P33_ACCESS(0xab)

//............. 0x00b0 - 0x00bf............ for EFUSE
#define P3_EFUSE_CON0                     P33_ACCESS(0xb0)
#define P3_EFUSE_CON1                     P33_ACCESS(0xb1)
#define P3_EFUSE_RDAT                     P33_ACCESS(0xb2)

#define P3_FUNC_EN                        P33_ACCESS(0xb8)

//............. 0x00c0 - 0x00cf............ for port input select
#define P3_PORT_SEL0                      P33_ACCESS(0xc0)
#define P3_PORT_SEL1                      P33_ACCESS(0xc1)
#define P3_PORT_SEL2                      P33_ACCESS(0xc2)
#define P3_PORT_SEL3                      P33_ACCESS(0xc3)
#define P3_PORT_SEL4                      P33_ACCESS(0xc4)
#define P3_PORT_SEL5                      P33_ACCESS(0xc5)
#define P3_PORT_SEL6                      P33_ACCESS(0xc6)
#define P3_PORT_SEL7                      P33_ACCESS(0xc7)
#define P3_PORT_SEL8                      P33_ACCESS(0xc8)
#define P3_PORT_SEL9                      P33_ACCESS(0xc9)
#define P3_PORT_SEL10                     P33_ACCESS(0xca)
#define P3_PORT_SEL11                     P33_ACCESS(0xcb)
#define P3_PORT_SEL12                     P33_ACCESS(0xcc)
#define P3_PORT_SEL13                     P33_ACCESS(0xcd)
#define P3_PORT_SEL14                     P33_ACCESS(0xce)
#define P3_PORT_SEL15                     P33_ACCESS(0xcf)

//............. 0x00d0 - 0x00df............
#define P3_LS_IO_DLY                      P33_ACCESS(0xd0)    //TODO: check sync with verilog head file chip_def.v  LEVEL_SHIFTER
#define P3_LS_IO_ROM                      P33_ACCESS(0xd1)
#define P3_LS_ADC                         P33_ACCESS(0xd2)
#define P3_LS_AUDIO                       P33_ACCESS(0xd3)
#define P3_LS_RF                          P33_ACCESS(0xd4)
#define P3_LS_PLL                         P33_ACCESS(0xd5)


//===============================================================================//
//
//      p33 rtcvdd
//
//===============================================================================//
//#define RTC_SFR_BEGIN                     0x1000



//............. 0x0080 - 0x008f............ for RTC
#define R3_ALM_CON                        RTC_ACCESS((0x80))

#define R3_RTC_CON0                       RTC_ACCESS((0x84))
#define R3_RTC_CON1                       RTC_ACCESS((0x85))
#define R3_RTC_DAT0                       RTC_ACCESS((0x86))
#define R3_RTC_DAT1                       RTC_ACCESS((0x87))
#define R3_RTC_DAT2                       RTC_ACCESS((0x88))
#define R3_RTC_DAT3                       RTC_ACCESS((0x89))
#define R3_RTC_DAT4                       RTC_ACCESS((0x8a))
#define R3_ALM_DAT0                       RTC_ACCESS((0x8b))
#define R3_ALM_DAT1                       RTC_ACCESS((0x8c))
#define R3_ALM_DAT2                       RTC_ACCESS((0x8d))
#define R3_ALM_DAT3                       RTC_ACCESS((0x8e))
#define R3_ALM_DAT4                       RTC_ACCESS((0x8f))

//............. 0x0090 - 0x009f............ for PORT control
#define R3_WKUP_EN                        RTC_ACCESS((0x90))
#define R3_WKUP_EDGE                      RTC_ACCESS((0x91))
#define R3_WKUP_CPND                      RTC_ACCESS((0x92))
#define R3_WKUP_PND                       RTC_ACCESS((0x93))
#define R3_WKUP_FLEN                      RTC_ACCESS((0x94))
#define R3_PORT_FLT                       RTC_ACCESS((0x95))

#define R3_PR_IN                          RTC_ACCESS((0x98))
#define R3_PR_OUT                         RTC_ACCESS((0x99))
#define R3_PR_DIR                         RTC_ACCESS((0x9a))
#define R3_PR_DIE                         RTC_ACCESS((0x9b))
#define R3_PR_PU                          RTC_ACCESS((0x9c))
#define R3_PR_PD                          RTC_ACCESS((0x9d))
#define R3_PR_HD                          RTC_ACCESS((0x9e))

//............. 0x00a0 - 0x00af............ for system
#define R3_TIME_CON                       RTC_ACCESS((0xa0))
#define R3_TIME_CPND                      RTC_ACCESS((0xa1))
#define R3_TIME_PND                       RTC_ACCESS((0xa2))

#define R3_ADC_CON                        RTC_ACCESS((0xa4))
#define R3_OSL_CON                        RTC_ACCESS((0xa5))

#define R3_WKUP_SRC                       RTC_ACCESS((0xa8))
#define R3_RST_SRC                        RTC_ACCESS((0xa9))

#define R3_RST_CON                        RTC_ACCESS((0xab))
#define R3_CLK_CON                        RTC_ACCESS((0xac))

#endif


