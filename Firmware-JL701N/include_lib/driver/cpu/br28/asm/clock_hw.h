#ifndef __CLOCK_HW_H__
#define __CLOCK_HW_H__

#include "typedef.h"

#define RC_EN(x)                SFR(JL_CLOCK->CLK_CON0,  0,  1,  x)
#define RCH_EN(x)				SFR(JL_CLOCK->CLK_CON0,  0,  1,  x)
//for MACRO - RCH_EN
enum {
    RCH_EN_250K = 0,
    RCH_EN_16M,
};

#define OSC_CLOCK_IN(x)         SFR(JL_CLOCK->CLK_CON0,  1,  3,  x)
//for MACRO - OSC_CLOCK_IN
enum {
    OSC_CLOCK_IN_BT_OSC = 0,
    OSC_CLOCK_IN_BT_OSC_X2,
    OSC_CLOCK_IN_STD_24M,
    OSC_CLOCK_IN_RTC_OSC,
    OSC_CLOCK_IN_LRC_CLK,
    OSC_CLOCK_IN_PAT,
};
#define     PLL_96M_SEL_END     6
#define PLL_96M_SEL_GET()       ((JL_CLOCK->CLK_CON0 >> 4) & 0xF)
#define PLL_96M_SEL(x)          SFR(JL_CLOCK->CLK_CON0, 4, 4,  x)
enum {
    PLL_96M_SEL_NULL = 0,

    PLL_96M_SEL_7DIV2, 	//f5
    PLL_96M_SEL_5DIV2,	//f4
    PLL_96M_SEL_4DIV2,	//f3
    PLL_96M_SEL_3DIV2,	//f2
    PLL_96M_SEL_1DIV1, 	//f1



    PLL1_96M_SEL_7DIV2, //f5
    PLL1_96M_SEL_5DIV2, //f4
    PLL1_96M_SEL_4DIV2, //f3
    PLL1_96M_SEL_3DIV2, //f2
    PLL1_96M_SEL_1DIV1, //f1

};

#define PLL_48M_SEL_GET()       ((JL_CLOCK->CLK_CON0 & BIT(8)) >> 8)
#define PLL_48M_SEL(x)          SFR(JL_CLOCK->CLK_CON0, 8, 1,  x)
enum {
    PLL_48M_SEL_DIV2 = 0,
    PLL_48M_SEL_DIV1,
};

#define STD_48M_SEL(x)          SFR(JL_CLOCK->CLK_CON0, 9, 1,  x)
#define PLL_48M_TO_STD_48M   0
#define BTOSCX2_TO_STD_48M   1

#define STD_48M_SEL_GET()		((JL_CLOCK->CLK_CON0 & BIT(9)) >> 9)

#define STD_24M_SEL(x)          SFR(JL_CLOCK->CLK_CON0, 10, 1,  x)
#define STD_48M_DIV2_TO_STD_24M     0
#define BTOSC_TO_STD_24M            1



#define PLL_SYS_SEL(x)          SFR(JL_CLOCK->CLK_CON1,  0,  4,  x)
#define PLL_SYS_SEL_GET()       ((JL_CLOCK->CLK_CON1 & 0xF))

#define    PLL_SYS_SEL_END      6
//for MACRO - PLL_SYS_SEL
enum {
    PLL_SYS_SEL_NULL = 0,
    PLL_SYS_SEL_7DIV2, //f5
    PLL_SYS_SEL_5DIV2, //f4
    PLL_SYS_SEL_4DIV2, //f3
    PLL_SYS_SEL_3DIV2, //f2
    PLL_SYS_SEL_1DIV1, //f1


    PLL1_SYS_SEL_7DIV2, //f5
    PLL1_SYS_SEL_5DIV2, //f4
    PLL1_SYS_SEL_4DIV2, //f3
    PLL1_SYS_SEL_3DIV2, //f2
    PLL1_SYS_SEL_1DIV1, //f1

};

#define PLL_SYS_DIV(x)          SFR(JL_CLOCK->CLK_CON1,  4,  4,  x)
//for MACRO - PLL_SYS_DIV
enum {
    PLL_SYS_DIV1 = 0,
    PLL_SYS_DIV3,
    PLL_SYS_DIV5,
    PLL_SYS_DIV7,

    PLL_SYS_DIV1X2 = 4,
    PLL_SYS_DIV3X2,
    PLL_SYS_DIV5X2,
    PLL_SYS_DIV7X2,

    PLL_SYS_DIV1X4 = 8,
    PLL_SYS_DIV3X4,
    PLL_SYS_DIV5X4,
    PLL_SYS_DIV7X4,

    PLL_SYS_DIV1X8 = 12,
    PLL_SYS_DIV3X8,
    PLL_SYS_DIV5X8,
    PLL_SYS_DIV7X8,
};
#define MAIN_CLOCK_SEL(x) 		SFR(JL_CLOCK->CLK_CON1,  8,  3,  x); \
									asm("csync")

//for MACRO - CLOCK_IN
enum {
    MAIN_CLOCK_IN_RC_250K = 0,
    MAIN_CLOCK_IN_PAT,
    MAIN_CLOCK_IN_RTC_OSC,
    MAIN_CLOCK_IN_RC,
    MAIN_CLOCK_IN_BTOSC,
    MAIN_CLOCK_IN_BTOSC_X2,
    MAIN_CLOCK_IN_PLL,
    MAIN_CLOCK_IN_RTOSC_L, //keep to fix make
};


#define SFR_MODE(x)             SFR(JL_CLOCK->CLK_CON1,  11,  1,  x)
enum {
    SFR_CLOCK_IDLE = 0,
    SFR_CLOCK_ALWAYS_ON,
};


#define BT_CLOCK_IN(x)          SFR(JL_CLOCK->CLK_CON1,  14,  2,  x)
//for MACRO - BT_CLOCK_IN
enum {
    BT_CLOCK_IN_PLL48M = 0,
    BT_CLOCK_IN_HSB,
    BT_CLOCK_IN_LSB,
    BT_CLOCK_IN_DISABLE,
};

#define PLL_ALNK0_SEL(x)        SFR(JL_CLOCK->CLK_CON2, 0, 3,  x)
#define PLL_ALNK0_DIV(x)        SFR(JL_CLOCK->CLK_CON2, 3, 2,  x)

#define PLL_ALNK_EN(x)         SFR(JL_CLOCK->CLK_CON2,  6,  1,  x)
#define PLL_ALNK_SEL(x)        SFR(JL_CLOCK->CLK_CON2,  7,  1,  x)
//for MACRO - PLL_ALNK_SEL
enum {
    PLL_ALNK_192M_DIV17 = 0,
    PLL_ALNK_480M_DIV39,
};

#define USB_CLOCK_IN(x)         SFR(JL_CLOCK->CLK_CON2, 8, 2,  x)
//for MACRO - USB_CLOCK_IN
enum {
    USB_CLOCK_IN_PLL48M = 0,
    USB_CLOCK_IN_OSC,
    USB_CLOCK_IN_LSB,
    USB_CLOCK_IN_DISABLE,
};

#define UART_CLOCK_IN(x)        SFR(JL_CLOCK->CLK_CON2,  10,  2,  x)
//for MACRO - UART_CLOCK_IN
enum {
    UART_CLOCK_IN_PLL24M = 0,
    UART_CLOCK_IN_OSC,
    UART_CLOCK_IN_LSB,
    UART_CLOCK_IN_DISABLE,
};

#define PLL_FM_SEL(x)	        SFR(JL_CLOCK->CLK_CON2,  12,  2,  x)
//for MACRO - PLL_APC_SEL
enum {
    PLL_APC_SEL_PLL192M = 0,
    PLL_APC_SEL_PLL137M,
    PLL_APC_SEL_PLL107M,
    PLL_APC_SEL_DISABLE,
};

#define PLL_FM_DIV(x)	        SFR(JL_CLOCK->CLK_CON2,  14,  4,  x)
//for MACRO - PLL_APC_DIV
enum {
    PLL_FM_DIV1 = 0,
    PLL_FM_DIV3,
    PLL_FM_DIV5,
    PLL_FM_DIV7,

    PLL_FM_DIV1X2 = 4,
    PLL_FM_DIV3X2,
    PLL_FM_DIV5X2,
    PLL_FM_DIV7X2,

    PLL_FM_DIV1X4 = 8,
    PLL_FM_DIV3X4,
    PLL_FM_DIV5X4,
    PLL_FM_DIV7X4,

    PLL_FM_DIV1X8 = 12,
    PLL_FM_DIV3X8,
    PLL_FM_DIV5X8,
    PLL_FM_DIV7X8,
};
#define GPCNT_CLOCK_IN(x)         SFR(JL_CLOCK->CLK_CON2,  28,  4,  x)
//for MACRO - DAC_CLOCK_IN
enum {
    GPCNT_CLOCK_IN_NULL = 0,
    GPCNT_CLOCK_IN_SRC,
    GPCNT_CLOCK_IN_ORG,
    GPCNT_CLOCK_IN_HSB,
    GPCNT_CLOCK_IN_XOSC_FSCK,
    GPCNT_CLOCK_IN_BTOSC_48M,
    GPCNT_CLOCK_IN_WL,
    GPCNT_CLOCK_IN_USB,
    GPCNT_CLOCK_IN_PMU_ANA,
    GPCNT_CLOCK_IN_VAD_VCON0,
    GPCNT_CLOCK_IN_VAD_VCOP0,
    GPCNT_CLOCK_IN_RC_16M,
    GPCNT_CLOCK_IN_P33_RC_250k,
    GPCNT_CLOCK_IN_LRC,
    GPCNT_CLOCK_IN_DPLL_OUT,
    GPCNT_CLOCK_IN_RTC_OSC,
};

#define PLL_IMD_SEL_GET( )    ((JL_CLOCK->CLK_CON3 >> 8) & 0xF)
#define PLL_IMD_SEL(x)        SFR(JL_CLOCK->CLK_CON3, 8,  4,  x)
#define PLL_IMD_DIV(x)        SFR(JL_CLOCK->CLK_CON3, 12, 4,  x)

#define PLL_TDM_SEL_GET(x)    ((JL_CLOCK->CLK_CON3 >> 24) & 0x7)
#define PLL_TDM_SEL(x)        SFR(JL_CLOCK->CLK_CON3, 24, 3,  x)
#define PLL_TDM_DIV(x)        SFR(JL_CLOCK->CLK_CON3, 27, 2,  x)


#define PLL_EN(x)         		SFR(JL_PLL0->CON0,  0,  1,  (x))
#define PLL_REST(x)             SFR(JL_PLL0->CON0,  1,  1,  (x))
#define PLL_MODE(x)             SFR(JL_PLL0->CON0,  2,  1,  (x))
#define PLL_PFDS(x)             SFR(JL_PLL0->CON0,  3,  2,  (x))
#define PLL_ICPS(x)             SFR(JL_PLL0->CON0,  5,  3,  (x))
#define PLL_LPFR2(x)            SFR(JL_PLL0->CON0,  8,  3,  (x))
#define PLL_IVCO(x)             SFR(JL_PLL0->CON0, 11,  3,  (x))
#define PLL_LDO12A(x)           SFR(JL_PLL0->CON0, 14,  3,  (x))
#define PLL_LDO12D(x)           SFR(JL_PLL0->CON0, 17,  3,  (x))
#define PLL_LDO_BYPASS(x)       SFR(JL_PLL0->CON0, 20,  1,  (x))
#define PLL_TSEL(x)         	SFR(JL_PLL0->CON0,  21, 2,  x)
#define PLL_TEST(x)         	SFR(JL_PLL0->CON0,  23, 1,  x)
#define PLL_VBG_TR(x)           SFR(JL_PLL0->CON0, 27,  4,  (x))



//REFDS
#define PLL_DIVn(x)          	SFR(JL_PLL0->CON1,  0, 7,  x)
#define PLL_REF_SEL0(x)        	SFR(JL_PLL0->CON1,  7,  2,  x)
//for MACRO - PLL_RSEL0
enum {
    PLL_RSEL_RCLK = 0, 	//
    PLL_RSEL_RCH,
    PLL_RSEL_DPLL_CLK,
    PLL_RSEL_PAT_CLK,
};

#define PLL_REF_SEL1(x)        	SFR(JL_PLL0->CON1,  9,  1,  x)
//for MACRO - PLL_REF_SEL1
enum {
    PLL_REF_SEL_BTOSC_DIFF = 0, 	//btosc_DIFF
    PLL_REF_SEL_RCLK,
};


#define PLL_DIVn_EN(x)         	SFR(JL_PLL0->CON1, 10, 2,  x)
//for MACRO - PLL_DIVn_EN
enum {
    PLL_DIVn_EN_X2 = 0,
    PLL_DIVn_DIS_DIV1,
    PLL_DIVn_EN_DIVn,
};

#define PLL_FBDS(x)             SFR(JL_PLL0->CON2, 0,  12,  x)

#define PLL_CLK_1DIV1_OE(x)       SFR(JL_PLL0->CON3, 0, 1,  x) //f1
#define PLL_CLK_3DIV2_OE(x)       SFR(JL_PLL0->CON3, 1, 1,  x) //f2
#define PLL_CLK_4DIV2_OE(x)       SFR(JL_PLL0->CON3, 2, 1,  x) //f3
#define PLL_CLK_5DIV2_OE(x)       SFR(JL_PLL0->CON3, 3, 1,  x) //f4
#define PLL_CLK_7DIV2_OE(x)       SFR(JL_PLL0->CON3, 4, 1,  x) //f5

#define PLL_CLK_EN(x)           SFR(JL_PLL0->CON3,  9, 1,  x)


#define PLL1_EN(x)         		SFR(JL_PLL1->CON0,  0,  1,  (x))
#define PLL1_REST(x)            SFR(JL_PLL1->CON0,  1,  1,  (x))
#define PLL1_MODE(x)            SFR(JL_PLL1->CON0,  2,  1,  (x))
#define PLL1_PFDS(x)            SFR(JL_PLL1->CON0,  3,  2,  (x))
#define PLL1_ICPS(x)            SFR(JL_PLL1->CON0,  5,  3,  (x))
#define PLL1_LPFR2(x)           SFR(JL_PLL1->CON0,  8,  3,  (x))
#define PLL1_IVCO(x)            SFR(JL_PLL1->CON0, 11,  3,  (x))
#define PLL1_LDO12A(x)          SFR(JL_PLL1->CON0, 14,  3,  (x))
#define PLL1_LDO12D(x)          SFR(JL_PLL1->CON0, 17,  3,  (x))
#define PLL1_LDO_BYPASS(x)      SFR(JL_PLL1->CON0, 20,  1,  (x))
#define PLL1_TSEL(x)         	SFR(JL_PLL1->CON0,  21, 2,  x)
#define PLL1_TEST(x)         	SFR(JL_PLL1->CON0,  23, 1,  x)
#define PLL1_VBG_TR(x)          SFR(JL_PLL1->CON0, 27,  4,  (x))

#define PLL1_DIVn(x)          	SFR(JL_PLL1->CON1,  0, 7,  x)
#define PLL1_REF_SEL0(x)        SFR(JL_PLL1->CON1,  7,  2,  x)
#define PLL1_REF_SEL1(x)        SFR(JL_PLL1->CON1,  9,  1,  x)
#define PLL1_DIVn_EN(x)         SFR(JL_PLL1->CON1, 10, 2,  x)

#define PLL1_FBDS(x)            SFR(JL_PLL1->CON2, 0,  12,  x)

#define PLL1_CLK_1DIV1_OE(x)       SFR(JL_PLL1->CON3, 0, 1,  x) //f1
#define PLL1_CLK_3DIV2_OE(x)       SFR(JL_PLL1->CON3, 1, 1,  x) //f2
#define PLL1_CLK_4DIV2_OE(x)       SFR(JL_PLL1->CON3, 2, 1,  x) //f3
#define PLL1_CLK_5DIV2_OE(x)       SFR(JL_PLL1->CON3, 3, 1,  x) //f4
#define PLL1_CLK_7DIV2_OE(x)       SFR(JL_PLL1->CON3, 4, 1,  x) //f5
#define PLL1_CLK_EN(x)             SFR(JL_PLL1->CON3,  9, 1,  x)


#define HSB_CLK_DIV(x)			SFR(JL_CLOCK->SYS_DIV,  0,  8,  x)
#define LSB_CLK_DIV(x)			SFR(JL_CLOCK->SYS_DIV,  8,  3,  x)
#define SFC_CLK_DIV(x)			SFR(JL_CLOCK->SYS_DIV,  12, 3,  x)

/********************************************************************************/
#define GPCNT_EN(x)             SFR(JL_GPCNT->CON,  0,  1,  x)
#define GPCNT_CSS(x)            SFR(JL_GPCNT->CON,  1,  3,  x)
//for MACRO - GPCNT_CSS
enum {
    GPCNT_CSS_LSB = 0,
    GPCNT_CSS_OSC,
    GPCNT_CSS_INPUT_CH2,
    GPCNT_CSS_INPUT_CH3,
    GPCNT_CSS_CLOCK_IN,
    GPCNT_CSS_RING,
    GPCNT_CSS_PLL,
    GPCNT_CSS_INTPUT_CH1,
};

#define GPCNT_CLR_PEND(x)       SFR(JL_GPCNT->CON,  6,  1,  x)
#define GPCNT_GTS(x)            SFR(JL_GPCNT->CON,  8,  4,  x)

#define GPCNT_GSS(x)            SFR(JL_GPCNT->CON,  12, 3,  x)
//for MACRO - GPCNT_CSS
enum {
    GPCNT_GSS_LSB = 0,
    GPCNT_GSS_OSC,
    GPCNT_GSS_INPUT_CH14,    //iomap con1[27:24]
    GPCNT_GSS_INPUT_CH15,    //iomap con1[31:28]
    GPCNT_GSS_CLOCK_IN,		 //CLK_CON2[31:28]
    GPCNT_GSS_RING,
    GPCNT_GSS_PLL,
    GPCNT_GSS_INPUT_CH13,   //iomap con1[23:20]
};

//wla_con16  xosc
#define XOSC_BGTR_10v_4(x)              ((x&0xf)<<15)

//wla_con8  xosc
#define XOSC_EN_10v_1(x)                ((x&0x1)<<0)
#define XOSCLDO_PAS_10v_1(x)            ((x&0x1)<<1)
#define XOSCLDO_S_10v_3(x)              ((x&0x7)<<2)
#define XOSC_CPTEST_EN_10v_1(x)         ((x&0x1)<<5)
#define XOSC_HCS_10v_5(x)               ((x&0x1f)<<6)
#define XOSC_CLS_10v_4(x)               ((x&0xf)<<11)
#define XOSC_CRS_10v_4(x)               ((x&0xf)<<15)
#define XOSC_BT_HDS_10v_2(x)            ((x&0x3)<<19)
#define XOSC_ANATEST_EN_10v_1(x)        ((x&0x1)<<21)
#define XOSC_ANATEST_S_10v_3(x)         ((x&0x7)<<22)
#define XOSC_BT_OE_10v_1(x)             ((x&0x1)<<25)
#define XOSC_SYS1_OE_10v_1(x)           ((x&0x1)<<26)
#define XOSC_CORE48M_OE_10v_1(x)        ((x&0x1)<<27)
#define XOSC_CORE48M_S_10v_3(x)         ((x&0x7)<<28)
#define XOSC_FSCK_OE_10v_1(x)           ((x&0x1)<<31)

//wla_con9 xosc
#define XOSC_FTCS_10v_3(x)              ((x&0x7)<<0)
#define XOSC_FTOE_10v_1(x)              ((x&0x1)<<3)
#define XOSC_FTIS_10v_5(x)              ((x&0x1f)<<4)
#define XOSC_CORE24M_OE_10v_1(x)        ((x&0x1)<<9)
#define XOSC_CMP_MODE_10v_1(x)          ((x&0x1)<<10)
#define XOSC_FTEN_10v_1(x)              ((x&0x1)<<11)
#define XOSC_PMU_HDS_10v_2(x)           ((x&0x3)<<12)
#define XOSC_PMU_OE_10v_1(x)            ((x&0x1)<<14)
#define XOSC_SYS1_HDS_10v_2(x)          ((x&0x3)<<15)
#define XOSC_DAC_HDS_10v_2(x)           ((x&0x3)<<17)
#define XOSC_DAC_OE_10v_1(x)            ((x&0x1)<<19)
#define XOSC_LFSREN_10v_1(x)            ((x&0x1)<<20)

//wla_con10
//PLL
#define XOSC_ADET_EN_10v_1(x)           ((x&0x1)<<0)
#define XOSC_ADET_S_10v_2(x)            ((x&0x3)<<1)
#define XOSC_VAD_DIVS_10v_4(x)          ((x&0xf)<<3)
#define XOSC_SYS0_HDS_10v_2(x)          ((x&0x3)<<7)
#define XOSC_SYS0_OE_10v_1(x)           ((x&0x1)<<9)
#define XOSC_VAD_HDS_10v_2(x)           ((x&0x3)<<10)
#define XOSC_VAD_OE_10v_1(x)            ((x&0x1)<<12)

//wla_con21
#define WLA_RD_MUX_4(x)                 ((x&0xf)<<0)

//wla_con30
#define XOSC_ADET_GET_RESULT()			((JL_WLA->WLA_CON30 >> 4) & 1)


//CORE
#define     ROM_MULTI_CYCLE(x)     SFR(JL_CMNG->CON0,3,2,(x))
#endif
