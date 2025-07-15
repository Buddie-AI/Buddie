#ifndef __P11__
#define __P11__

//===============================================================================//
//
//      sfr define
//
//===============================================================================//

#ifdef PMU_SYSTEM
#define p11_base          0x000000
#define p11_ram_base      p11_base
#define p11_sfr_base      0x00a000
#else
#define p11_base          0xf20000
#define p11_ram_base      p11_base
#define p11_sfr_base      0xf2a000
#endif

#define __RW               volatile                 // read write
#define __RO               volatile const           // only read
#define __WO               volatile                 // only write

#define __u8               unsigned int             // u8  to u32 special for struct
#define __u16              unsigned int             // u16 to u32 special for struct
#define __u32              unsigned int

#define __s8(x)            char(x); char(reserved_1_##x); char(reserved_2_##x); char(reserved_3_##x)
#define __s16(x)           short(x); short(reserved_1_##x)
#define __s32(x)           int(x)

#define map_adr(grp, adr)  ((64 * grp + adr) * 4)   // grp(0x0-0xff), adr(0x0-0x3f)
#define P11_ACCESS(x)      (*(volatile u32 *)(p11_base + x))
#define P11_RAM(x)         (*(volatile u32 *)(p11_ram_base + x))

//===============================================================================//
//
//      sfr address define
//
//===============================================================================//

//............. 0x0000 - 0x03ff............ for cpu

// #include ../core/csfr.h

//............. 0x0400 - 0x04ff............ for clock
typedef struct {
    __RW __u32 PWR_CON;
    __RW __u32 RST_SRC;
    __RW __u32 WKUP_EN;
    __RW __u32 WKUP_SRC;
    __RW __u32 SYS_DIV;
    __RW __u32 CLK_CON0;
    __RW __u32 CLK_CON1;
    __RW __u32 CLK_CON2;
    __RW __u32 CLK_CON3;
} P11_CLOCK_TypeDef;

#define P11_CLOCK_BASE          (p11_sfr_base + map_adr(0x04, 0x00))
#define P11_CLOCK               ((P11_CLOCK_TypeDef    *)P11_CLOCK_BASE)

//............. 0x0500 - 0x05ff............ for memory control
typedef struct {
    __RW __u32 MPC0;
    __RW __u32 MPC1;
    __RW __u32 MSC;
} P11_MEM_CTL_TypeDef;

#define P11_MEM_CTL_BASE        (p11_sfr_base + map_adr(0x05, 0x00))
#define P11_MEM_CTL             ((P11_MEM_CTL_TypeDef    *)P11_MEM_CTL_BASE)

//............. 0x0600 - 0x06ff............ for system
typedef struct {
    __RW __u32 P2M_INT_IE;
    __RW __u32 P2M_INT_SET;
    __RW __u32 P2M_INT_CLR;
    __RO __u32 P2M_INT_PND;
    __RW __u32 P2M_CLK_CON0;
    __RW __u32 M2P_INT_IE;
    __RW __u32 M2P_INT_SET;
    __RW __u32 M2P_INT_CLR;
    __RO __u32 M2P_INT_PND;
    __RW __u32 P11_SYS_CON0;
    __RW __u32 P11_SYS_CON1;
    __RW __u32 MEM_PWR_CON;
} P11_SYSTEM_TypeDef;

#define P11_SYSTEM_BASE         (p11_sfr_base + map_adr(0x06, 0x00))
#define P11_SYSTEM              ((P11_SYSTEM_TypeDef    *)P11_SYSTEM_BASE)

//............. 0x0800 - 0x08ff............ for watch dog
typedef struct {
    __RW __u32 CON;
    __RW __u32 KEY;
    __RW __u32 DUMMY;
} P11_WDT_TypeDef;

#define P11_WDT_BASE            (p11_sfr_base + map_adr(0x08, 0x00))
#define P11_WDT                 ((P11_WDT_TypeDef    *)P11_WDT_BASE)

//............. 0x0900 - 0x0cff............ for lp timer
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 PRD;
    __RW __u32 RSC;
    __RO __u32 CNT;
} P11_LPTMR_TypeDef;

#define P11_LPTMR0_BASE         (p11_sfr_base + map_adr(0x09, 0x00))
#define P11_LPTMR1_BASE         (p11_sfr_base + map_adr(0x0a, 0x00))
#define P11_LPTMR2_BASE         (p11_sfr_base + map_adr(0x0b, 0x00))
#define P11_LPTMR3_BASE         (p11_sfr_base + map_adr(0x0c, 0x00))

#define P11_LPTMR0              ((P11_LPTMR_TypeDef    *)P11_LPTMR0_BASE)
#define P11_LPTMR1              ((P11_LPTMR_TypeDef    *)P11_LPTMR1_BASE)
#define P11_LPTMR2              ((P11_LPTMR_TypeDef    *)P11_LPTMR2_BASE)
#define P11_LPTMR3              ((P11_LPTMR_TypeDef    *)P11_LPTMR3_BASE)

//............. 0x0d00 - 0x0dff............ for irflt
typedef struct {
    __RW __u32 CON;
} P11_IRFLT_TypeDef;

#define P11_IRFLT_BASE         (p11_sfr_base + map_adr(0x0d, 0x00))
#define P11_IRFLT              ((P11_IRFLT_TypeDef    *)P11_IRFLT_BASE)

//............. 0x0e00 - 0x0eff............ for spi
typedef struct {
    __RW __u32 CON;
    __WO __u32 BAUD;
    __RW __u32 BUF;
    //__WO __u32 ADR;
    //__WO __u32 CNT;
} P11_SPI_TypeDef;

#define P11_SPI_BASE            (p11_sfr_base + map_adr(0x0e, 0x00))
#define P11_SPI                 ((P11_SPI_TypeDef    *)P11_SPI_BASE)

//............. 0x0f00 - 0x10ff............ for uart
typedef struct {
    __RW __u16 CON0;
    //__RW __u16 CON1;
    __WO __u16 BAUD;
    __RW __u8  BUF;
    __RW __u32 OTCNT;
    //__RW __u32 TXADR;
    //__WO __u16 TXCNT;
    //__RW __u32 RXSADR;
    //__RW __u32 RXEADR;
    //__RW __u32 RXCNT;
    //__RO __u16 HRXCNT;
    __RW __u16 CON2;
} P11_UART_TypeDef;

#define P11_UART0_BASE           (p11_sfr_base + map_adr(0x0f, 0x00))
#define P11_UART1_BASE           (p11_sfr_base + map_adr(0x10, 0x00))

#define P11_UART0                ((P11_UART_TypeDef    *)P11_UART0_BASE)
#define P11_UART1                ((P11_UART_TypeDef    *)P11_UART1_BASE)

//............. 0x1100 - 0x11ff............ for iic
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 BAUD;
    __RW __u32 BUF;
} P11_IIC_TypeDef;

#define P11_IIC_BASE           (p11_sfr_base + map_adr(0x11, 0x00))
#define P11_IIC                ((P11_IIC_TypeDef    *)P11_IIC_BASE)

//............. 0x1200 - 0x12ff............ for port
typedef struct {
    __RW __u32 OCH_CON0  ;
    __RW __u32 ICH_CON0  ;
    __RW __u32 P33_PORT  ;
    __RW __u32 PB_SEL    ;
    __RW __u32 PB_PU     ;
    __RW __u32 PB_PD     ;
    __RW __u32 PB_DIR    ;
    __RW __u32 PB_DIE    ;
    __RW __u32 PB_DIEH   ;
    __RW __u32 PB_OUT    ;
    __RO __u32 PB_IN     ;
} P11_PORT_TypeDef;

#define P11_PORT_BASE           (p11_sfr_base + map_adr(0x12, 0x00))
#define P11_PORT                ((P11_PORT_TypeDef    *)P11_PORT_BASE)

//............. 0x1300 - 0x13ff............ for lp ctmu
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 ANA0;
    __RW __u32 ANA1;
    __RW __u32 RES;
} P11_LPCTM_TypeDef;

#define P11_LPCTM_BASE          (p11_sfr_base + map_adr(0x13, 0x00))
#define P11_LPCTM               ((P11_LPCTM_TypeDef    *)P11_LPCTM_BASE)

//............. 0x1400 - 0x14ff............ for lpvad
typedef struct {
    __RW __u32 VAD_CON;
    __RW __u32 VAD_ACON0;
    __RW __u32 VAD_ACON1;
    __RW __u32 AVAD_CON;
    __RW __u32 AVAD_DATA;
    __RW __u32 DVAD_CON0;
    __RW __u32 DVAD_CON1;
    __RW __u32 DMA_BADR;
    __RW __u32 DMA_LEN;
    __RW __u32 DMA_HPTR;
    __RW __u32 DMA_SPTR;
    __RW __u32 DMA_SPN;
    __RW __u32 DMA_SHN;
} P11_LPVAD_TypeDef;

#define P11_LPVAD_BASE          (p11_sfr_base + map_adr(0x14, 0x00))
#define P11_LPVAD               ((P11_LPVAD_TypeDef    *)P11_LPVAD_BASE)

//............. 0x1500 - 0x16ff............ for crossbar
#include "p11_io_omap.h"
#include "p11_io_imap.h"

//............. 0x1700 - 0x18ff............ for gp timer
typedef struct {
    __RW __u32 CON;
    __RW __u32 CNT;
    __RW __u32 PRD;
    __RW __u32 PWM;
} P11_GPTMR_TypeDef;

#define P11_GPTMR0_BASE         (p11_sfr_base + map_adr(0x17, 0x00))
#define P11_GPTMR1_BASE         (p11_sfr_base + map_adr(0x18, 0x00))

#define P11_GPTMR0              ((P11_GPTMR_TypeDef    *)P11_GPTMR0_BASE)
#define P11_GPTMR1              ((P11_GPTMR_TypeDef    *)P11_GPTMR1_BASE)

#endif
