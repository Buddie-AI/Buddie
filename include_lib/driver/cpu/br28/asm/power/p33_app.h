#ifndef __P33_APP_H__
#define __P33_APP_H__

//ROM
u8 p33_buf(u8 buf);

// void p33_xor_1byte(u16 addr, u8 data0);
#define p33_xor_1byte(addr, data0)      (*((volatile u8 *)&addr + 0x300*4)  = data0)
// #define p33_xor_1byte(addr, data0)      addr ^= (data0)

// void p33_and_1byte(u16 addr, u8 data0);
#define p33_and_1byte(addr, data0)      (*((volatile u8 *)&addr + 0x100*4)  = (data0))
//#define p33_and_1byte(addr, data0)      addr &= (data0)

// void p33_or_1byte(u16 addr, u8 data0);
#define p33_or_1byte(addr, data0)       (*((volatile u8 *)&addr + 0x200*4)  = data0)
// #define p33_or_1byte(addr, data0)       addr |= (data0)

// void p33_tx_1byte(u16 addr, u8 data0);
#define p33_tx_1byte(addr, data0)       addr = data0

// u8 p33_rx_1byte(u16 addr);
#define p33_rx_1byte(addr)              addr

#define P33_CON_SET(sfr, start, len, data)  (sfr = (sfr & ~((~(0xff << (len))) << (start))) | \
	 (((data) & (~(0xff << (len)))) << (start)))

#define P33_CON_GET(sfr)    sfr

#define P33_ANA_CHECK(reg) (((reg & reg##_MASK) == reg##_RV) ? 1:0)


#if 1

#define p33_fast_access(reg, data, en)           \
{ 												 \
    if (en) {                                    \
		p33_or_1byte(reg, (data));               \
    } else {                                     \
		p33_and_1byte(reg, ~(data));             \
    }                                            \
}

#else

#define p33_fast_access(reg, data, en)         \
{                                              \
	if (en) {                                  \
       	reg |= (data);                         \
	} else {                                   \
		reg &= ~(data);                        \
    }                                          \
}

#endif

//
//
//					for p33_analog.doc
//
//
//
/************************P3_ANA_CON0*****************************/
#define VDD13TO12_SYS_EN(en)    	P33_CON_SET(P3_ANA_CON0, 0, 1, en)

#define VDD13TO12_RVD_EN(en)    	P33_CON_SET(P3_ANA_CON0, 1, 1, en)

#define LDO13_EN(en)            	P33_CON_SET(P3_ANA_CON0, 2, 1, en)

#define DCDC13_EN(en)           	P33_CON_SET(P3_ANA_CON0, 3, 1, en)

#define GET_DCDC13_EN()				((P33_CON_GET(P3_ANA_CON0) & BIT(3)) ? 1:0)

#define PVDD_EN(en)             	P33_CON_SET(P3_ANA_CON0, 4, 1, en)

#define MVIO_VBAT_EN(en)        	P33_CON_SET(P3_ANA_CON0, 5, 1, en)

#define MVIO_VPWR_EN(en)        	P33_CON_SET(P3_ANA_CON0, 6, 1, en)

#define MBG_EN(en)					P33_CON_SET(P3_ANA_CON0, 7, 1, en)

#define P3_ANA_CON0_MASK 			0b11110011
#define P3_ANA_CON0_RV	 			0b11110011

/************************P3_ANA_KEEP*****************************/
#define CLOSE_ANA_KEEP()			P33_CON_SET(P3_ANA_KEEP, 0, 8, 0)

#define P3_ANA_KEEP_MASK			0b11111111
#define P3_ANA_KEEP_RV				0b00000000

/************************P3_ANA_KEEP1****************************/
#define VIO2_EN_KEEP(en)			P33_CON_SET(P3_ANA_KEEP1, 0, 1, en)

#define P3_ANA_KEEP1_MASK			0b00000001
#define P3_ANA_KEEP1_RV				0b00000000

/**************************P3_ANA_CON1*********************************/
#define RVDD_BYPASS_EN(en)      	P33_CON_SET(P3_ANA_CON1, 0, 1, en)

#define WVDD_SHORT_RVDD(en)     	P33_CON_SET(P3_ANA_CON1, 1, 1, en)

#define WVDD_SHORT_SVDD(en)     	P33_CON_SET(P3_ANA_CON1, 2, 1, en)

#define WLDO06_EN(en)           	P33_CON_SET(P3_ANA_CON1, 3, 1, en)

#define WLDO06_OE(en)           	P33_CON_SET(P3_ANA_CON1, 4, 1, en)

#define EVD_EN(en)       			P33_CON_SET(P3_ANA_CON1, 5, 1, en)

#define EVD_SHORT_PB8(en)       	P33_CON_SET(P3_ANA_CON1, 6, 1, en)

#define PVD_SHORT_PB8(en)       	P33_CON_SET(P3_ANA_CON1, 7, 1, en)

#define P3_ANA_CON1_MASK			0b10011111
#define P3_ANA_CON1_RV				0b00000100

/************************P3_ANA_CON2*****************************/
#define VCM_DET_EN(en)          	P33_CON_SET(P3_ANA_CON2, 3, 1, en)

#define MVIO_VBAT_ILMT_EN(en)   	P33_CON_SET(P3_ANA_CON2, 4, 1, en)

#define MVIO_VPWR_ILMT_EN(en)   	P33_CON_SET(P3_ANA_CON2, 5, 1, en)

#define DCVD_ILMT_EN(en)        	P33_CON_SET(P3_ANA_CON2, 6, 1, en)

#define CURRENT_LIMIT_DISABLE() 	(P3_ANA_CON2 &= ~(BIT(4) | BIT(5) | BIT(6)))

#define P3_ANA_CON2_MASK			0b01110000
#define P3_ANA_CON2_RV				0b01110000

/************************P3_ANA_CON3*****************************/
#define MVBG_SEL(en)     			P33_CON_SET(P3_ANA_CON3, 0, 4, en)

#define MVBG_GET()     		    	(P33_CON_GET(P3_ANA_CON3) & 0x0f)

#define WVBG_SEL(en)           		P33_CON_SET(P3_ANA_CON3, 4, 4, en)

#define P3_ANA_CON3_MASK  			0b00000000
#define P3_ANA_CON3_RV				0b00000000

/************************P3_ANA_CON4*****************************/
#define PMU_DET_EN(en)          	P33_CON_SET(P3_ANA_CON4, 0, 1, en)

#define ADC_CHANNEL_SEL(ch)     	P33_CON_SET(P3_ANA_CON4, 1, 4, ch)

#define PMU_DET_BG_BUF_EN(en)   	P33_CON_SET(P3_ANA_CON4, 5, 1, en)

#define VBG_TEST_EN(en)   			P33_CON_SET(P3_ANA_CON4, 6, 1, en)

#define VBG_TEST_SEL(en)   			P33_CON_SET(P3_ANA_CON4, 7, 1, en)

#define P3_ANA_CON4_MASK			0b11100001
#define P3_ANA_CON4_RV				0b11100001

/************************P3_ANA_CON5*****************************/
//vddiom_lev
enum {
    VDDIOM_VOL_20V = 0,
    VDDIOM_VOL_22V,
    VDDIOM_VOL_24V,
    VDDIOM_VOL_26V,
    VDDIOM_VOL_28V,
    VDDIOM_VOL_30V, //default
    VDDIOM_VOL_32V,
    VDDIOM_VOL_34V,
};

#define VDDIOM_VOL_SEL(lev)     	P33_CON_SET(P3_ANA_CON5, 0, 3, lev)

#define GET_VDDIOM_VOL()        	(P33_CON_GET(P3_ANA_CON5) & 0x7)

//vddiow_lev
enum {
    VDDIOW_VOL_20V = 0,
    VDDIOW_VOL_22V,
    VDDIOW_VOL_24V,
    VDDIOW_VOL_26V,
    VDDIOW_VOL_28V,
    VDDIOW_VOL_30V,
    VDDIOW_VOL_32V,
    VDDIOW_VOL_34V,
};

#define VDDIOW_VOL_SEL(lev)     	P33_CON_SET(P3_ANA_CON5, 3, 3, lev)

#define GET_VDDIOW_VOL()        	(P33_CON_GET(P3_ANA_CON5)>>3 & 0x7)

#define VDDIO_HD_SEL(cur)       	P33_CON_SET(P3_ANA_CON5, 6, 2, cur)

#define P3_ANA_CON5_MASK			0b11000000
#define P3_ANA_CON5_RV				0b01000000

/************************P3_ANA_CON6*****************************/
#define VDC13_VOL_SEL(sel)      	P33_CON_SET(P3_ANA_CON6, 0, 4, sel)
//Macro for VDC13_VOL_SEL
enum {
    VDC13_VOL_SEL_100V = 0,
    VDC13_VOL_SEL_105V,
    VDC13_VOL_SEL_1075V,
    VDC13_VOL_SEL_110V,
    VDC13_VOL_SEL_1125V,
    VDC13_VOL_SEL_115V,
    VDC13_VOL_SEL_1175V,
    VDC13_VOL_SEL_120V,
    VDC13_VOL_SEL_1225V,
    VDC13_VOL_SEL_125V,
    VDC13_VOL_SEL_1275V,
    VDC13_VOL_SEL_130V,
    VDC13_VOL_SEL_1325V,
    VDC13_VOL_SEL_135V,
    VDC13_VOL_SEL_1375V,
    VDC13_VOL_SEL_140V,
};

#define VD13_DEFAULT_VOL			VDC13_VOL_SEL_125V

#define GET_VD13_VOL_SEL()      	(P33_CON_GET(P3_ANA_CON6) & 0xf)

#define VD13_HD_SEL(sel)        	P33_CON_SET(P3_ANA_CON6, 4, 2, sel)

#define VD13_CAP_EN(en)         	P33_CON_SET(P3_ANA_CON6, 6, 1, en)

#define VD13_DESHOT_EN(en)      	P33_CON_SET(P3_ANA_CON6, 7, 1, en)

#define	P3_ANA_CON6_MASK			0b11111111
#define P3_ANA_CON6_RV				0b11011010

/************************P3_ANA_CON7*****************************/
#define BTDCDC_PFM_MODE(en)     	P33_CON_SET(P3_ANA_CON7, 0, 1, en)

#define GET_BTDCDC_PFM_MODE()   	(P33_CON_GET(P3_ANA_CON7) & BIT(0) ? 1 : 0)

#define BTDCDC_RAMP_SHORT(en)      	P33_CON_SET(P3_ANA_CON7, 1, 1, en)

#define BTDCDC_V17_TEST_OE(en)		P33_CON_SET(P3_ANA_CON7, 2, 1, en);

#define BTDCDC_DUTY_SEL(sel)    	P33_CON_SET(P3_ANA_CON7, 3, 2, sel)

#define BTDCDC_OSC_SEL(sel)     	P33_CON_SET(P3_ANA_CON7, 5, 3, sel)
//Macro for BTDCDC_OSC_SEL
enum {
    BTDCDC_OSC_SEL0520KHz = 0,
    BTDCDC_OSC_SEL0762KHz,
    BTDCDC_OSC_SEL0997KHz,
    BTDCDC_OSC_SEL1220KHz,
    BTDCDC_OSC_SEL1640KHz,
    BTDCDC_OSC_SEL1840KHz,
    BTDCDC_OSC_SEL2040KHz,
    BTDCDC_OSC_SEL2220MHz,
};

#define P3_ANA_CON7_MASK 			0b11111111
#define P3_ANA_CON7_RV   			0b01001001

/************************P3_ANA_CON8*****************************/
#define BTDCDC_V21_RES_S(sel) 	P33_CON_SET(P3_ANA_CON8, 0, 2, sel)

#define BTDCDC_DT_S(sel)          	P33_CON_SET(P3_ANA_CON8, 2, 2, sel)

#define BTDCDC_ISENSE_HD(sel) 	P33_CON_SET(P3_ANA_CON8, 4, 2, sel)

#define BTDCDC_COMP_HD(sel)     	P33_CON_SET(P3_ANA_CON8, 6, 2, sel)

#define P3_ANA_CON8_MASK 			0b11111111
#define P3_ANA_CON8_RV	 			0b01100110

/************************P3_ANA_CON9*****************************/
#define BTDCDC_NMOS_S(sel)    	P33_CON_SET(P3_ANA_CON9, 1, 3, sel)

#define BTDCDC_PMOS_S(sel)    	P33_CON_SET(P3_ANA_CON9, 5, 3, sel)

#define P3_ANA_CON9_MASK 			0b11101110
#define P3_ANA_CON9_RV   			0b01101110

/************************P3_ANA_CON10*****************************/

#define BTDCDC_OSC_TEST_OE(en)  	P33_CON_SET(P3_ANA_CON10, 7, 1, en)

#define BTDCDC_HD_BIAS_SEL(sel) 	P33_CON_SET(P3_ANA_CON10, 5, 2, sel)

#define BTDCDC_CLK_SEL(sel)     	P33_CON_SET(P3_ANA_CON10, 4, 1, sel)

#define GET_BTDCDC_CLK_SEL()    	(P33_CON_GET(P3_ANA_CON10) & BIT(4) ? 1 : 0)

#define BTDCDC_ZCD_RES(sel)     	P33_CON_SET(P3_ANA_CON10, 2, 2, sel)

#define BTDCDC_ZCD_EN(en)       	P33_CON_SET(P3_ANA_CON10, 0, 1, en)

#define P3_ANA_CON10_MASK 			0b11111101
#define P3_ANA_CON10_RV	  			0b00110001

/************************P3_ANA_CON11*****************************/
#define SYSVDD_VOL_SEL(sel)     	P33_CON_SET(P3_ANA_CON11, 0, 4, sel)
//Macro for SYSVDD_VOL_SEL
enum {
    SYSVDD_VOL_SEL_081V = 0,
    SYSVDD_VOL_SEL_084V,
    SYSVDD_VOL_SEL_087V,
    SYSVDD_VOL_SEL_090V,
    SYSVDD_VOL_SEL_093V,
    SYSVDD_VOL_SEL_096V,
    SYSVDD_VOL_SEL_099V,
    SYSVDD_VOL_SEL_102V,
    SYSVDD_VOL_SEL_105V,
    SYSVDD_VOL_SEL_108V,
    SYSVDD_VOL_SEL_111V,
    SYSVDD_VOL_SEL_114V,
    SYSVDD_VOL_SEL_117V,
    SYSVDD_VOL_SEL_120V,
    SYSVDD_VOL_SEL_123V,
    SYSVDD_VOL_SEL_126V,
};

#define SYSVDD_DEFAULT_VOL			SYSVDD_VOL_SEL_105V

#define GET_SYSVDD_VOL_SEL()     	(P33_CON_GET(P3_ANA_CON11) & 0xf)

#define SYSVDD_VOL_HD_SEL(sel)  	P33_CON_SET(P3_ANA_CON11, 4, 2, sel)

#define SYSVDD_CAP_EN(en)       	P33_CON_SET(P3_ANA_CON11, 6, 1, en)

#define P3_ANA_CON11_MASK 			0b01110000
#define P3_ANA_CON11_RV	  			0b00010000

/************************P3_ANA_CON12*****************************/
#define RVDD_VOL_SEL(sel)       	P33_CON_SET(P3_ANA_CON12, 0, 4, sel)
//Macro for SYSVDD_VOL_SEL
enum {
    RVDD_VOL_SEL_081V = 0,
    RVDD_VOL_SEL_084V,
    RVDD_VOL_SEL_087V,
    RVDD_VOL_SEL_090V,
    RVDD_VOL_SEL_093V,
    RVDD_VOL_SEL_096V,
    RVDD_VOL_SEL_099V,
    RVDD_VOL_SEL_102V,
    RVDD_VOL_SEL_105V,
    RVDD_VOL_SEL_108V,
    RVDD_VOL_SEL_111V,
    RVDD_VOL_SEL_114V,
    RVDD_VOL_SEL_117V,
    RVDD_VOL_SEL_120V,
    RVDD_VOL_SEL_123V,
    RVDD_VOL_SEL_126V,
};

#define RVDD_DEFAULT_VOL			RVDD_VOL_SEL_105V

#define GET_RVDD_VOL_SEL()      	(P33_CON_GET(P3_ANA_CON12) & 0xf)

#define RVDD_VOL_HD_SEL(en)     	P33_CON_SET(P3_ANA_CON12, 4, 2, en)

#define RVDD_CAP_EN(en)         	P33_CON_SET(P3_ANA_CON12, 6, 1, en)

#define P3_ANA_CON12_MASK			0b01110000
#define P3_ANA_CON12_RV				0b00010000

/************************P3_ANA_CON13*****************************/
#define WVDD_VOL_SEL(sel)       	P33_CON_SET(P3_ANA_CON13, 0, 4, sel)
//Macro for WVDD_VOL_SEL
enum {
    WVDD_VOL_SEL_050V = 0,
    WVDD_VOL_SEL_055V,
    WVDD_VOL_SEL_060V,
    WVDD_VOL_SEL_065V,
    WVDD_VOL_SEL_070V,
    WVDD_VOL_SEL_075V,
    WVDD_VOL_SEL_080V,
    WVDD_VOL_SEL_085V,
    WVDD_VOL_SEL_090V,
    WVDD_VOL_SEL_095V,
    WVDD_VOL_SEL_100V,
    WVDD_VOL_SEL_105V,
    WVDD_VOL_SEL_110V,
    WVDD_VOL_SEL_115V,
    WVDD_VOL_SEL_120V,
    WVDD_VOL_SEL_125V,
};

#define WVDD_VOL_MIN				500
#define VWDD_VOL_MAX				1250
#define WVDD_VOL_TRIM				800//mv
#define WVDD_VOL_STEP				50
#define WVDD_LEVEL_MAX    		    0xf
#define WVDD_LEVEL_ERR      		0xff

#define WVDD_VOL_TRIM_LED   850

#define WVDD_LEVEL_DEFAULT  ((WVDD_VOL_TRIM-WVDD_VOL_MIN)/WVDD_VOL_STEP + 2)

#define WVDD_LOAD_EN(en)        	P33_CON_SET(P3_ANA_CON13, 4, 1, en)

#define WVDDIO_FBRES_AUTO(en)   	P33_CON_SET(P3_ANA_CON13, 6, 1, en)

#define WVDDIO_FBRES_SEL_W(en)  	P33_CON_SET(P3_ANA_CON13, 7, 1, en)

#define P3_ANA_CON13_MASK 			0b11110000
#define P3_ANA_CON13_RV				0b10000000

/************************P3_ANA_CON14*****************************/
#define RVD2PVD_SHORT_EN(en)      	P33_CON_SET(P3_ANA_CON14, 4, 1, en)

#define GET_RVD2PVD_SHORT_EN()		(P33_CON_GET(P3_ANA_CON14) & BIT(4) ? 1:0)

#define PVD_DEUDSHT_EN(en)      	P33_CON_SET(P3_ANA_CON14, 3, 1, en)

#define GET_PVD_DEUDST_EN()			((P33_CON_GET(P3_ANA_CON14) & BIT(3)) ? 1:0)

#define PVD_HD_SEL(sel)         	P33_CON_SET(P3_ANA_CON14, 0, 3, sel)

#define GET_PVD_HD_SEL()			(P33_CON_GET(P3_ANA_CON14) & 0x7)

#define P3_ANA_CON14_MASK			0b00011111
#define P3_ANA_CON14_RV				0b00000100

/************************P3_ANA_CON15*****************************/
#define EVD_VOL_SEL(sel)       		P33_CON_SET(P3_ANA_CON15, 0, 2, sel)
enum {
    EVD_VOL_SEL_100V = 0,
    EVD_VOL_SEL_105V,
    EVD_VOL_SEL_110V,
    EVD_VOL_SEL_115V,
};

#define EVD_HD_SEL(sel)         	P33_CON_SET(P3_ANA_CON15, 2, 2, sel)

#define EVD_CAP_EN(en)          	P33_CON_SET(P3_ANA_CON15, 4, 1, en)

#define P3_ANA_CON15_MASK   		0b00001100
#define P3_ANA_CON15_RV				0b00000100

/************************P3_PVDD0_AUTO*****************************/
#define PVDD_LEVEL_LOW(sel)			P33_CON_SET(P3_PVDD0_AUTO, 0, 4, sel)

#define GET_PVDD_LEVEL_LOW()		(P33_CON_GET(P3_PVDD0_AUTO) & 0xf)

#define PVDD_LEVEL_AUTO(en)			P33_CON_SET(P3_PVDD0_AUTO, 4, 1, en)

#define GET_PVDD_LEVEL_AUTO()		((P33_CON_GET(P3_PVDD0_AUTO) & BIT(4)) ? 1:0)

#define PVDD_AUTO_PRD(sel)			P33_CON_SET(P3_PVDD0_AUTO, 5, 3, sel)

#define GET_PVDD_AUTO_PRD()			((P33_CON_GET(P3_PVDD0_AUTO) & (0x7<<5)) >> 5)

enum {
    PVDD_VOL_SEL_050V = 0,
    PVDD_VOL_SEL_055V,
    PVDD_VOL_SEL_060V,
    PVDD_VOL_SEL_065V,
    PVDD_VOL_SEL_070V,
    PVDD_VOL_SEL_075V,
    PVDD_VOL_SEL_080V,
    PVDD_VOL_SEL_085V,
    PVDD_VOL_SEL_090V,
    PVDD_VOL_SEL_095V,
    PVDD_VOL_SEL_100V,
    PVDD_VOL_SEL_105V,
    PVDD_VOL_SEL_110V,
    PVDD_VOL_SEL_115V,
    PVDD_VOL_SEL_120V,
    PVDD_VOL_SEL_125V,
};

#define PVDD_VOL_MIN        500
#define PVDD_VOL_MAX		1250
#define PVDD_VOL_STEP       50
#define PVDD_LEVEL_MAX      0xf
#define PVDD_LEVEL_ERR		0xff
#define PVDD_LEVEL_DEFAULT  0xc

#define PVDD_LEVEL_REF			  	PVDD_VOL_SEL_115V
#define PVDD_VOL_REF                1150//mV

#define PVDD_VOL_CLOCK_SET			1200

#define PVDD_VOL_HIGH_NOW        1100

#ifdef CONFIG_WATCH_CASE_ENABLE
#define PVDD_LEVEL_SLEEP      PVDD_VOL_SEL_090V
#define PVDD_VOL_SLEEP        900

#else
#define PVDD_LEVEL_SLEEP      PVDD_VOL_SEL_095V
#define PVDD_VOL_SLEEP        950

#endif

#define P3_PVDD0_AUTO_MASK			0b11110000
#define P3_PVDD0_AUTO_RV			0b01110000

/************************P3_PVDD1_AUTO*****************************/
#define PVDD_LEVEL_HIGH_NOW(sel)	P33_CON_SET(P3_PVDD1_AUTO, 0, 8, (sel<<4)|sel);

#define PVDD_LEVEL_HIGH(sel)		P33_CON_SET(P3_PVDD1_AUTO, 4, 4, sel)

#define GET_PVDD_LEVEL_HIGH()		((P33_CON_GET(P3_PVDD1_AUTO) & 0xf0)>>4)

#define PVDD_LEVEL_NOW(sel)			P33_CON_SET(P3_PVDD1_AUTO, 0, 4, sel)

#define GET_PVDD_LEVEL_NOW()		(P33_CON_GET(P3_PVDD1_AUTO) & 0x0f)

#define P3_PVDD1_AUTO_MASK			0b00000000
#define P3_PVDD1_AUTO_RV			0b00000000

/************************P3_PVDD2_AUTO*****************************/
#define GET_PVDD_DRV_AUTO()			(P33_CON_GET(P3_PVDD2_AUTO) & BIT(0))

#define P3_PVDD2_AUTO_MASK			0b00000001
#define P3_PVDD2_AUTO_RV			0b00000001

/************************P3_CHG_CON0*****************************/
#define CHARGE_EN(en)           	P33_CON_SET(P3_CHG_CON0, 0, 1, en)

#define CHGGO_EN(en)            	P33_CON_SET(P3_CHG_CON0, 1, 1, en)

#define IS_CHARGE_EN()				((P33_CON_GET(P3_CHG_CON0) & BIT(0)) ? 1: 0 )

#define CHG_HV_MODE(mode)       	P33_CON_SET(P3_CHG_CON0, 2, 1, mode)

#define CHG_TRICKLE_EN(en)          P33_CON_SET(P3_CHG_CON0, 3, 1, en)

#define CHG_CCLOOP_EN(en)           P33_CON_SET(P3_CHG_CON0, 4, 1, en)

#define CHG_VILOOP_EN(en)           P33_CON_SET(P3_CHG_CON0, 5, 1, en)

#define CHG_VINLOOP_SLT(sel)        P33_CON_SET(P3_CHG_CON0, 6, 1, sel)

#define CHG_SEL_CHG_FULL            0
#define CHG_SEL_VBAT_DET            1
#define CHG_SSEL(sel)               P33_CON_SET(P3_CHG_CON0, 7, 1, sel)

#define P3_CHG_CON0_MASK			0
#define P3_CHG_CON0_RV				0

/************************P3_CHG_CON1*****************************/
#define CHARGE_FULL_V_SEL(a)		P33_CON_SET(P3_CHG_CON1, 0, 4, a)

#define CHARGE_mA_SEL(a)			P33_CON_SET(P3_CHG_CON1, 4, 4, a)

#define P3_CHG_CON1_MASK			0
#define P3_CHG_CON1_RV				0

/************************P3_CHG_CON2*****************************/
#define CHARGE_FULL_mA_SEL(a)		P33_CON_SET(P3_CHG_CON2, 4, 3, a)

enum {
    CHARGE_DET_VOL_365V,
    CHARGE_DET_VOL_375V,
    CHARGE_DET_VOL_385V,
    CHARGE_DET_VOL_395V,
};
#define CHARGE_DET_VOL(a)	P33_CON_SET(P3_CHG_CON2, 1, 2, a)

#define CHARGE_DET_EN(en)	P33_CON_SET(P3_CHG_CON2, 0, 1, en)

#define P3_CHG_CON2_MASK			0
#define P3_CHG_CON2_RV				0

/************************P3_L5V_CON0*****************************/
#define L5V_LOAD_EN(a)		    	P33_CON_SET(P3_L5V_CON0, 0, 1, a)

#define L5V_IO_MODE(a)              P33_CON_SET(P3_L5V_CON0, 2, 1, a)

#define IS_L5V_LOAD_EN()        ((P33_CON_GET(P3_L5V_CON0) & BIT(0)) ? 1: 0 )

#define GET_L5V_RES_DET_S_SEL() (P33_CON_GET(P3_L5V_CON1) & 0x03)

#define P3_L5V_CON0_MASK			0
#define P3_L5V_CON0_RV				0

/************************P3_L5V_CON1*****************************/
#define L5V_RES_DET_S_SEL(a)		P33_CON_SET(P3_L5V_CON1, 0, 2, a)

#define P3_L5V_CON1_MASK			0
#define P3_L5V_CON1_RV				0

/************************P3_VLVD_CON*****************************/
#define P33_VLVD_EN(en)         	P33_CON_SET(P3_VLVD_CON, 0, 1, en)

#define GET_VLVD_EN()				(P33_CON_GET(P3_VLVD_CON) & BIT(0))

#define P33_VLVD_PS(en)         	P33_CON_SET(P3_VLVD_CON, 1, 1, en)

#define P33_VLVD_OE(en)         	P33_CON_SET(P3_VLVD_CON, 2, 1, en)

#define GET_VLVD_OE()            ((P33_CON_GET(P3_VLVD_CON) & BIT(2)) ? 1:0)

#define VLVD_SEL(lev)           	P33_CON_SET(P3_VLVD_CON, 3, 3, lev)

#define GET_VLVD_SEL()           	((P33_CON_GET(P3_VLVD_CON) & (0x7<<3))>>3)
//Macro for VLVD_SEL
enum {
    VLVD_SEL_18V = 0,
    VLVD_SEL_19V,
    VLVD_SEL_20V,
    VLVD_SEL_21V,
    VLVD_SEL_22V,
    VLVD_SEL_23V,
    VLVD_SEL_24V,
    VLVD_SEL_25V,
};

#define VLVD_PND_CLR()       		P33_CON_SET(P3_VLVD_CON, 6, 1, 1)

#define VLVD_PND()          		((P33_CON_GET(P3_VLVD_CON) & BIT(7)) ? 1 : 0)

#define P3_VLVD_CON_MASK 			0
#define P3_VLVD_CON_RV 				0

/************************P3_VLVD_FLT*****************************/
#define VLVD_FLT(sel)				P33_CON_SET(P3_VLVD_FLT, 0, 2, sel);

#define P3_VLVD_FLT_MASK			0b00000011
#define P3_VLVD_FLT_RV				0b00000010

/************************P3_RST_CON0*****************************/
#define DPOR_MASK(en)           	P33_CON_SET(P3_RST_CON0, 0, 1, en)

#define VLVD_RST_EN(en)         	P33_CON_SET(P3_RST_CON0, 2, 1, en)

#define VLVD_WKUP_EN(en)        	P33_CON_SET(P3_RST_CON0, 3, 1, en)

#define PPOR_MASK(en)           	P33_CON_SET(P3_RST_CON0, 4, 1, en)

#define P11_TO_P33_RST_MASK(en) 	P33_CON_SET(P3_RST_CON0, 5, 1, en)

#define DVDDOK_OE(en) 				P33_CON_SET(P3_RST_CON0, 6, 1, en)

#define PVDDOK_OE(en) 				P33_CON_SET(P3_RST_CON0, 7, 1, en)

#define P3_RST_CON0_MASK			0b11111101
#define P3_RST_CON0_RV				0b00000100

/************************P3_LRC_CON0*****************************/
#define RC32K_EN(en)            	P33_CON_SET(P3_LRC_CON0, 0, 1, en)

#define RC32K_RN_TRIM(en)       	P33_CON_SET(P3_LRC_CON0, 1, 1, en)

#define RC32K_RPPS_SEL(sel)     	P33_CON_SET(P3_LRC_CON0, 4, 2, sel)

#define P3_LRC_CON0_MASK			0b00110011
#define P3_LRC_CON0_RV				0b00100001

/************************P3_LRC_CON1*****************************/
#define RC32K_PNPS_SEL(sel)     	P33_CON_SET(P3_LRC_CON1, 0, 2, sel)

#define RC32K_CAP_SEL(sel)      	P33_CON_SET(P3_LRC_CON1, 4, 3, sel)

#define CLOSE_LRC()					P33_CON_SET(P3_LRC_CON0, 0, 8, 0);\
									P33_CON_SET(P3_LRC_CON1, 0, 8, 0)

#define P3_LRC_CON1_MASK			0b01110011
#define P3_LRC_CON1_RV				0b01000001

/************************P3_CLK_CON0*****************************/
#define RC_250K_EN(a)          		P33_CON_SET(P3_CLK_CON0, 0, 1, a)

#define P3_CLK_CON0_MASK			0b00000001
#define P3_CLK_CON0_RV				0b00000000

/************************P3_VLD_KEEP*****************************/
#define RTC_WKUP_KEEP(a)        	P33_CON_SET(P3_VLD_KEEP, 1, 1, a)

#define P33_WKUP_P11_EN(a)          P33_CON_SET(P3_VLD_KEEP, 2, 1, a)

#define P3_VLD_KEEP_MASK			0b00000110
#define P3_VLD_KEEP_RV				0b00000110

/************************P3_PMU_CON0*****************************/
#define GET_P33_SYS_POWER_FLAG() 	((P33_CON_GET(P3_PMU_CON0) & BIT(7)) ? 1 : 0)

#define P33_SYS_POWERUP_CLEAR()  	P33_CON_SET(P3_PMU_CON0, 6, 1, 1);

#define P3_PMU_CON0_MASK			0b00000000
#define P3_PMU_CON0_RV				0b00000000

/************************P3_PMU_DBG_CON*****************************/
#define P3_PMU_DBG_CON_MASK			0b00000000
#define P3_PMU_DBG_CON_RV			0b00000000

/************************P3_IOV2_CON*****************************/
#define GET_IOV2_VOL()				((P33_CON_GET(P3_IOV2_CON) & (0x7<<3)) >> 3)

#define GET_IOV2_IFULL()			((P33_CON_GET(P3_IOV2_CON) & BIT(2)) ? 1:0)

#define GET_IOV2_BYPASS()			((P33_CON_GET(P3_IOV2_CON) & BIT(1)) ? 1:0)

#define GET_IOV2_EN()				((P33_CON_GET(P3_IOV2_CON) & BIT(0)) ? 1:0)

#define P3_IOV2_CON_MASK			0
#define P3_IOV2_CON_RV				0

/************************P3_RVD_CON*****************************/
#define RVDD_CMP_EN(en)				P33_CON_SET(P3_RVD_CON, 4, 1, en)

#define PVDD_DCDC_LEV_SEL(sel)		P33_CON_SET(P3_RVD_CON, 0, 4, sel)

#define GET_PVDD_DCDC_LEV_SEL()		(P33_CON_GET(P3_RVD_CON) & 0xf)

#define P3_RVD_CON_MASK				0b00000000
#define P3_RVD_CON_RV				0b00000000

/************************P3_CHG_WKUP*****************************/
#define CHARGE_LEVEL_DETECT_EN(a)	P33_CON_SET(P3_CHG_WKUP, 0, 1, a)

#define CHARGE_EDGE_DETECT_EN(a)	P33_CON_SET(P3_CHG_WKUP, 1, 1, a)

#define CHARGE_WKUP_SOURCE_SEL(a)	P33_CON_SET(P3_CHG_WKUP, 2, 2, a)

#define CHARGE_WKUP_EN(a)			P33_CON_SET(P3_CHG_WKUP, 4, 1, a)

#define CHARGE_WKUP_EDGE_SEL(a)		P33_CON_SET(P3_CHG_WKUP, 5, 1, a)

#define CHARGE_WKUP_PND_CLR()		P33_CON_SET(P3_CHG_WKUP, 6, 1, 1)

/************************P3_AWKUP_LEVEL*****************************/
#define CHARGE_FULL_FILTER_GET()	((P33_CON_GET(P3_AWKUP_LEVEL) & BIT(2)) ? 1: 0)

#define LVCMP_DET_FILTER_GET()      ((P33_CON_GET(P3_AWKUP_LEVEL) & BIT(1)) ? 1: 0)

#define LDO5V_DET_FILTER_GET()      ((P33_CON_GET(P3_AWKUP_LEVEL) & BIT(0)) ? 1: 0)

/************************P3_ANA_READ*****************************/
#define CHARGE_FULL_FLAG_GET()		((P33_CON_GET(P3_ANA_READ) & BIT(0)) ? 1: 0 )

#define LVCMP_DET_GET()			    ((P33_CON_GET(P3_ANA_READ) & BIT(1)) ? 1: 0 )

#define LDO5V_DET_GET()			    ((P33_CON_GET(P3_ANA_READ) & BIT(2)) ? 1: 0 )
//
//
//			for ANA_control.doc
//
//
//
/************************P3_LS_XX*****************************/
#define PVDD_ANA_LAT_EN(en)  \
	if(en){				  	 \
		P3_LS_P11 = 0x01; 	 \
		P3_LS_P11 = 0x03; 	 \
	}else{				  	 \
		P3_LS_P11 = 0x0;  	 \
	}

#define DVDD_ANA_LAT_EN(en)      \
	if(en){					  	 \
		P3_LS_IO_DLY      = 0x1; \
        P3_LS_IO_ROM      = 0x1; \
        P3_LS_ADC         = 0x1; \
		P3_LS_AUDIO       = 0x1; \
        P3_LS_RF          = 0x1; \
        P3_LS_PLL         = 0x1; \
        P3_LS_IO_DLY      = 0x3; \
        P3_LS_IO_ROM      = 0x3; \
        P3_LS_ADC         = 0x3; \
		P3_LS_AUDIO       = 0x3; \
        P3_LS_RF          = 0x3; \
        P3_LS_PLL          = 0x3; \
	}else{						 \
        P3_LS_IO_DLY      = 0x0; \
        P3_LS_IO_ROM      = 0x0; \
        P3_LS_ADC         = 0x0; \
		P3_LS_AUDIO       = 0x0; \
        P3_LS_RF          = 0x0; \
        P3_LS_PLL          = 0x0;\
	}

#define DVDD_ANA_ROM_LAT_EN(en)    \
	if(en){                        \
		P3_LS_IO_ROM  = 1;		   \
        P3_LS_PLL     = 1;		   \
    	P3_LS_RF      = 1;         \
		P3_LS_IO_ROM  = 3;		   \
        P3_LS_PLL     = 3;		   \
    	P3_LS_RF      = 3;         \
	}else{                         \
		P3_LS_IO_ROM  = 0;		   \
        P3_LS_PLL     = 0;		   \
    	P3_LS_RF      = 0;         \
	}
//
//
//			for reset_source.doc
//
//
//
/************************P3_PR_PWR*****************************/
#define	P3_SOFT_RESET()				P33_CON_SET(P3_PR_PWR, 4, 2, 3)

/************************P3_IVS_CLR*****************************/
#define	P33_SF_KICK_START()			P33_CON_SET(P3_IVS_CLR, 0, 8, 0b10101000)

/************************P3_RST_SRC*****************************/
#define GET_P33_SYS_RST_SRC()		P33_CON_GET(P3_RST_SRC)


#endif
