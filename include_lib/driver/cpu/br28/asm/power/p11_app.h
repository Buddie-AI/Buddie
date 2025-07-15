/*********************************************************************************************
    *   Filename        : p11_app.h

    *   Description     : 本文件基于p11.h文件封装应用的接口

    *   Author          : MoZhiYe

    *   Email           : mozhiye@zh-jieli.com

    *   Last modifiled  : 2021-04-19 09:00

    *   Copyright:(c)JIELI  2021-2029  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef __P11_APP_H__
#define __P11_APP_H__

#include "p11.h"

/*

 _______________ <-----P11 Message Acess End
| poweroff boot |
|_______________|
| m2p msg(0x40) |
|_______________|
| p2m msg(0x40) |
|_______________|<-----P11 Message Acess Begin
|               |
|    p11 use    |
|_______________|__

*/
#define P11_RAM_BASE 				0xF20000
#define P11_RAM_SIZE 				(0x8000)
#define P11_RAM_END 				(P11_RAM_BASE + P11_RAM_SIZE)

#define P11_POWEROFF_RAM_SIZE (0x14 + 0xc)
#define P11_POWEROFF_RAM_BEGIN  (P11_RAM_END - P11_POWEROFF_RAM_SIZE)

#define P2M_MESSAGE_SIZE 			0x40
#define M2P_MESSAGE_SIZE 			0x60

#define M2P_MESSAGE_RAM_BEGIN 		(P11_POWEROFF_RAM_BEGIN - M2P_MESSAGE_SIZE)
#define P2M_MESSAGE_RAM_BEGIN 		(M2P_MESSAGE_RAM_BEGIN - P2M_MESSAGE_SIZE)

#define P11_MESSAGE_RAM_BEGIN 		(P2M_MESSAGE_RAM_BEGIN)

#define P11_RAM_ACCESS(x)   		(*(volatile u8 *)(x))

#define P2M_MESSAGE_ACCESS(x)      	P11_RAM_ACCESS(P2M_MESSAGE_RAM_BEGIN + x)
#define M2P_MESSAGE_ACCESS(x)      	P11_RAM_ACCESS(M2P_MESSAGE_RAM_BEGIN + x)

//==========================================================//
//                       P11_VAD_RAM                        //
//==========================================================//
//-------------------------- VAD CBUF-----------------------//
#define VAD_POINT_PER_FRAME 				(160)
#define VAD_FRAME_SIZE 						(160 * 2)
#define VAD_CBUF_FRAME_CNT 					(6)
#define VAD_CBUF_TAG_SIZE 					(0)
#define VAD_CBUF_FRAME_SIZE 				(VAD_FRAME_SIZE +  VAD_CBUF_TAG_SIZE)
#define CONFIG_P11_CBUF_SIZE 				(VAD_CBUF_FRAME_SIZE * VAD_CBUF_FRAME_CNT)
#define VAD_CBUF_END 						(P11_MESSAGE_RAM_BEGIN - 0x20)
#define VAD_CBUF_BEGIN 						(VAD_CBUF_END - CONFIG_P11_CBUF_SIZE)

//------------------------ VAD CONFIG-----------------------//
#define CONFIG_P2M_AVAD_CONFIG_SIZE 		(20 * 4) //sizeof(int)
#define CONFIG_P2M_DVAD_CONFIG_SIZE 		(20 * 4) //sizeof(int)
#define CONFIG_VAD_CONFIG_SIZE 				(CONFIG_P2M_AVAD_CONFIG_SIZE + CONFIG_P2M_DVAD_CONFIG_SIZE)
#define VAD_AVAD_CONFIG_BEGIN 				(VAD_CBUF_BEGIN - CONFIG_P2M_AVAD_CONFIG_SIZE)
#define VAD_DVAD_CONFIG_BEGIN 				(VAD_AVAD_CONFIG_BEGIN - CONFIG_P2M_DVAD_CONFIG_SIZE)

#define P11_HEAP_BEGIN 						(P11_RAM_BASE + ((P2M_P11_HEAP_BEGIN_ADDR_H << 8) | P2M_P11_HEAP_BEGIN_ADDR_L))
#define P11_HEAP_SIZE 						((P2M_P11_HEAP_SIZE_H << 8) | P2M_P11_HEAP_SIZE_L)

#define P11_RAM_PROTECT_END 				(P11_HEAP_BEGIN)


#define P11_PWR_CON				   	P11_CLOCK->PWR_CON

/*
 *------------------- P11_CLOCK->CLK_CON
 */

#define P11_CLK_CON0 				P11_CLOCK->CLK_CON0
enum P11_SYS_CLK_TABLE {
    P11_SYS_CLK_RC250K = 0,
    P11_SYS_CLK_RC16M,
    P11_SYS_CLK_LRC_OSC,
    P11_SYS_CLK_BTOSC_24M,
    P11_SYS_CLK_BTOSC_48M,
    P11_SYS_CLK_PLL_SYS_CLK,
    P11_SYS_CLK_CLK_X2,
};
//#define P11_SYS_CLK_SEL(x) 			SFR(P11_CLOCK->CLK_CON0, 0, 3, x)
#define P11_SYS_CLK_SEL(x) 			(P11_CLOCK->CLK_CON0 = x)


//p11 btosc use d2sh
#define CLOCK_KEEP(en)							  \
	if(en){										  \
		P11_CLOCK->CLK_CON1 &= ~(3<<15);		  \
		P11_CLOCK->CLK_CON1 |= BIT(14);			  \
		P11_CLOCK->CLK_CON1 |= (2<<15);			  \
	}else{  									  \
		P11_CLOCK->CLK_CON1 &= ~(3<<15);		  \
		P11_CLOCK->CLK_CON1 &= ~BIT(14);		  \
	}

#define P11_P2M_CLK_CON0 			P11_SYSTEM->P2M_CLK_CON0
#define P11_SYSTEM_CON0 		   	P11_SYSTEM->P11_SYS_CON0
#define P11_SYSTEM_CON1 		   	P11_SYSTEM->P11_SYS_CON1
#define P11_P11_SYS_CON0  			P11_SYSTEM_CON0
#define P11_P11_SYS_CON1  			P11_SYSTEM_CON1
#define P11_RST_SRC					P11_CLOCK->RST_SRC

#define LED_CLK_SEL(x)             P11_SYSTEM->P2M_CLK_CON0 = ((P11_SYSTEM->P2M_CLK_CON0 & ~0xe0) | (x) << 5)
#define GET_LED_CLK_SEL(x)         (P11_SYSTEM->P2M_CLK_CON0 & 0xe0)

#define P11_WDT_CON 				P11_WDT->CON

#define P11_P2M_INT_IE 			   	P11_SYSTEM->P2M_INT_IE
#define P11_M2P_INT_IE 			   	P11_SYSTEM->M2P_INT_IE
#define P11_M2P_INT_SET 			P11_SYSTEM->M2P_INT_SET
#define P11_P2M_INT_SET 			P11_SYSTEM->P2M_INT_SET
#define P11_P2M_INT_CLR 			P11_SYSTEM->P2M_INT_CLR
#define P11_P2M_INT_PND 			P11_SYSTEM->P2M_INT_PND //?
#define P11_M2P_INT_PND 			P11_SYSTEM->M2P_INT_PND //?

#define P11_TMR0_CON0 			   P11_LPTMR0->CON0
#define P11_TMR0_CON1 			   P11_LPTMR0->CON1
#define P11_TMR0_CON2 			   P11_LPTMR0->CON2
#define P11_TMR0_CNT 			   P11_LPTMR0->CNT
#define P11_TMR0_PRD 			   P11_LPTMR0->PRD
#define P11_TMR0_RSC 			   P11_LPTMR0->RSC

#define P11_TMR1_CON0 			   P11_LPTMR1->CON0
#define P11_TMR1_CON1 			   P11_LPTMR1->CON1
#define P11_TMR1_CON2 			   P11_LPTMR1->CON2
#define P11_TMR1_CNT 			   P11_LPTMR1->CNT
#define P11_TMR1_PRD 			   P11_LPTMR1->PRD
#define P11_TMR1_RSC 			   P11_LPTMR1->RSC

#define P11_TMR2_CON0 			   P11_LPTMR2->CON0
#define P11_TMR2_CON1 			   P11_LPTMR2->CON1
#define P11_TMR2_CON2 			   P11_LPTMR2->CON2
#define P11_TMR2_CNT 			   P11_LPTMR2->CNT
#define P11_TMR2_PRD 			   P11_LPTMR2->PRD
#define P11_TMR2_RSC 			   P11_LPTMR2->RSC

#define P11_TMR3_CON0 			   P11_LPTMR3->CON0
#define P11_TMR3_CON1 			   P11_LPTMR3->CON1
#define P11_TMR3_CON2 			   P11_LPTMR3->CON2
#define P11_TMR3_CNT 			   P11_LPTMR3->CNT
#define P11_TMR3_PRD 			   P11_LPTMR3->PRD
#define P11_TMR3_RSC 			   P11_LPTMR3->RSC

#define GET_P11_SYS_RST_SRC()	   P11_RST_SRC

#define LP_PWR_IDLE(x)             SFR(P11_PWR_CON, 0, 1, x)
#define LP_PWR_STANDBY(x)          SFR(P11_PWR_CON, 1, 1, x)
#define LP_PWR_SLEEP(x)            SFR(P11_PWR_CON, 2, 1, x)
#define LP_PWR_SSMODE(x)           SFR(P11_PWR_CON, 3, 1, x)
#define LP_PWR_SOFT_RESET(x)       SFR(P11_PWR_CON, 4, 1, x)
#define LP_PWR_INIT_FLAG()         (P11_PWR_CON & BIT(5))
#define LP_PWR_RST_FLAG_CLR(x)     SFR(P11_PWR_CON, 6, 1, x)
#define LP_PWR_RST_FLAG()          (P11_PWR_CON & BIT(7))

#define P33_TEST_ENABLE()           P11_P11_SYS_CON0 |= BIT(5)
#define P33_TEST_DISABLE()          P11_P11_SYS_CON0 &= ~BIT(5)

#define P11_TX_DISABLE(x)           P11_SYSTEM->P11_SYS_CON1 |= BIT(2)
#define P11_TX_ENABLE(x)            P11_SYSTEM->P11_SYS_CON1 &= ~BIT(2)

#define MSYS_IO_LATCH_ENABLE()      P11_SYSTEM->P11_SYS_CON1 |= BIT(7)
#define MSYS_IO_LATCH_DISABLE()     P11_SYSTEM->P11_SYS_CON1 &= ~BIT(7)

#define LP_TMR0_EN(x)               SFR(P11_TMR0_CON0, 0, 1, x)
#define LP_TMR0_CTU(x)              SFR(P11_TMR0_CON0, 1, 1, x)
#define LP_TMR0_P11_WKUP_IE(x)      SFR(P11_TMR0_CON0, 2, 1, x)
#define LP_TMR0_P11_TO_IE(x)        SFR(P11_TMR0_CON0, 3, 1, x)
#define LP_TMR0_CLR_P11_WKUP(x)     SFR(P11_TMR0_CON0, 4, 1, x)
#define LP_TMR0_P11_WKUP(x)        (P11_TMR0_CON0 & BIT(5))
#define LP_TMR0_CLR_P11_TO(x)       SFR(P11_TMR0_CON0, 6, 1, x)
#define LP_TMR0_P11_TO(x)          (P11_TMR0_CON0 & BIT(7))

#define LP_TMR0_SW_KICK_START_EN(x)   SFR(P11_TMR0_CON1, 0, 1, x)
#define LP_TMR0_HW_KICK_START_EN(x)   SFR(P11_TMR0_CON1, 1, 1, x)
#define LP_TMR0_WKUP_IE(x)         SFR(P11_TMR0_CON1, 2, 1, x)
#define LP_TMR0_TO_IE(x)           SFR(P11_TMR0_CON1, 3, 1, x)
#define LP_TMR0_CLR_MSYS_WKUP(x)   SFR(P11_TMR0_CON1, 4, 1, x)
#define LP_TMR0_MSYS_WKUP(x)       (P11_TMR0_CON1 & BIT(5))
#define LP_TMR0_CLR_MSYS_TO(x)     SFR(P11_TMR0_CON1, 6, 1, x)
#define LP_TMR0_MSYS_TO(x)         (P11_TMR0_CON1 & BIT(7))

#define LP_TMR0_CLK_SEL(x)         SFR(P11_TMR0_CON2, 0, 4, x)
#define LP_TMR0_CLK_DIV(x)         SFR(P11_TMR0_CON2, 4, 4, x)
#define LP_TMR0_KST(x)             SFR(P11_TMR0_CON2, 8, 1, x)
#define LP_TMR0_RUN()              (P11_TMR0_CON2 & BIT(9))

#define P11_M2P_RESET_MASK(x)      SFR(P11_P11_SYS_CON1 , 4, 1, x)

//MEM_PWR_CON
#define	MEM_PWR_CPU_CON	BIT(0)
#define MEM_PWR_RAM0_RAM3_CON BIT(1)
#define MEM_PWR_RAM4_RAM5_CON BIT(2)
#define MEM_PWR_RAM6_RAM7_CON BIT(3)
#define MEM_PWR_RAM8_RAM9_CON BIT(4)
#define MEM_PWR_PERIPH_CON  BIT(5)

#define MEM_PWR_RAM_SET(a)   (((1 << a) - 1) - 1)

#define LRC_Hz_DEFAULT    (200 * 1000L)

#define LRC_CON0_INIT                                     \
        /*                               */     (0 << 7) |\
        /*                               */     (0 << 6) |\
        /*RC32K_RPPS_S1_33v              */     (1 << 5) |\
        /*RC32K_RPPS_S0_33v              */     (0 << 4) |\
        /*                               */     (0 << 3) |\
        /*                               */     (0 << 2) |\
        /*RC32K_RN_TRIM_33v              */     (0 << 1) |\
        /*RC32K_EN_33v                   */     (1 << 0)

#define LRC_CON1_INIT                                     \
        /*                               */     (0 << 7) |\
        /*RC32K_CAP_S2_33v               */     (1 << 6) |\
        /*RC32K_CAP_S1_33v               */     (0 << 5) |\
        /*RC32K_CAP_S0_33v               */     (0 << 4) |\
        /*                        2bit   */     (0 << 2) |\
        /*RC32K_RNPS_S1_33v              */     (0 << 1) |\
        /*RC32K_RNPS_S0_33v              */     (1 << 0)

void wdt_isr(void);
u8 p11_run_query(void);

#endif /* #ifndef __P11_APP_H__ */


