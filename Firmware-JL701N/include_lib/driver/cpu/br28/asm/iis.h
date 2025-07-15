#ifndef __IIS_H__
#define __IIS_H__

#include "generic/typedef.h"


#define ALINK_SEL(module, reg)             (((JL_ALNK_TypeDef    *)(((u8 *)JL_ALNK0)))->reg)

#define ALNK_CON_RESET(module)	do {ALINK_SEL(module, CON0) = 0; ALINK_SEL(module, CON1) = 0; ALINK_SEL(module, CON2) = 0; ALINK_SEL(module, CON3) = 0;} while(0)
#define ALNK_HWPTR_RESET(module)	do {ALINK_SEL(module, HWPTR0) = 0; ALINK_SEL(module, HWPTR1) = 0; ALINK_SEL(module, HWPTR2) = 0; ALINK_SEL(module, HWPTR3) = 0;} while(0)
#define ALNK_SWPTR_RESET(module)	do {ALINK_SEL(module, SWPTR0) = 0; ALINK_SEL(module, SWPTR1) = 0; ALINK_SEL(module, SWPTR2) = 0; ALINK_SEL(module, SWPTR3) = 0;} while(0)
#define ALNK_SHN_RESET(module)	do {ALINK_SEL(module, SHN0) = 0; ALINK_SEL(module, SHN1) = 0; ALINK_SEL(module, SHN2) = 0; ALINK_SEL(module, SHN3) = 0;} while(0)
#define ALNK_ADR_RESET(module)	do {ALINK_SEL(module, ADR0) = 0; ALINK_SEL(module, ADR1) = 0; ALINK_SEL(module, ADR2) = 0; ALINK_SEL(module, ADR3) = 0;} while(0)
#define ALNK_PNS_RESET(module)	do {ALINK_SEL(module, PNS) = 0;} while(0)

#define	ALINK_DA2BTSRC_SEL(module, x)	SFR(ALINK_SEL(module, CON0), 0, 2, x)
#define	ALINK_DMA_MODE_SEL(module, x)	SFR(ALINK_SEL(module, CON0), 2, 1, x)
#define ALINK_DSPE(module, x)			SFR(ALINK_SEL(module, CON0), 6, 1, x)
#define ALINK_SOE(module, x) 			SFR(ALINK_SEL(module, CON0), 7, 1, x)
#define ALINK_MOE(module, x) 			SFR(ALINK_SEL(module, CON0), 8, 1, x)
#define F32_EN(module, x)    		    SFR(ALINK_SEL(module, CON0), 9, 1, x)
#define SCLKINV(module, x)   		   	SFR(ALINK_SEL(module, CON0),10, 1, x)
#define ALINK_EN(module, x)  		    SFR(ALINK_SEL(module, CON0),11, 1, x)
#define ALINK_24BIT_MODE(module)	(ALINK_SEL(module, CON1) |= (BIT(2) | BIT(6) | BIT(10) | BIT(14)))
#define ALINK_16BIT_MODE(module)	(ALINK_SEL(module, CON1) &= (~(BIT(2) | BIT(6) | BIT(10) | BIT(14))))


#define ALINK_CHx_DIR_MODE(module, ch, x)  	 SFR(ALINK_SEL(module, CON1), 3 + 4 * ch, 1, x)
#define ALINK_CHx_MODE_SEL(module, ch, x)  	 SFR(ALINK_SEL(module, CON1), 4 * ch, 2, x)
#define ALINK_CHx_CLOSE(module, ch)  	 SFR(ALINK_SEL(module, CON1), 4 * ch, 2, 0)


#define ALINK_CLR_ALL_PND(module)				(ALINK_SEL(module, CON2) |= BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define ALINK_CLR_CHx_PND(module, ch)			(ALINK_SEL(module, CON2) |= BIT(ch))
#define ALINK_CHx_IE(module, ch, x)			SFR(ALINK_SEL(module, CON2), ch + 12, 1, x)

#define ALINK_MSRC(module, x)		SFR(ALINK_SEL(module, CON3), 0, 2, x)
#define ALINK_MDIV(module, x)		SFR(ALINK_SEL(module, CON3), 2, 3, x)
#define ALINK_LRDIV(module, x)		SFR(ALINK_SEL(module, CON3), 5, 3, x)

#define ALINK_OPNS(module)            (ALINK_SEL(module, PNS) >> 16)
#define ALINK_IPNS(module)            (ALINK_SEL(module, PNS) & 0xffff)
#define ALINK_OPNS_SET(module, x)		SFR(ALINK_SEL(module, PNS), 16, 16, x)
#define ALINK_IPNS_SET(module, x)		SFR(ALINK_SEL(module, PNS), 0, 16, x)

#define ALINK_LEN_SET(module, x)		(ALINK_SEL(module, LEN) = x)
#define ALINK_FIFO_LEN(module)          (ALINK_SEL(module, LEN))
#endif
