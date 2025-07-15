//*********************************************************************************//
// Module name : csfr.h                                                            //
// Description : q32small core sfr define                                          //
// By Designer : zequan_liu                                                        //
// Dat changed :                                                                   //
//*********************************************************************************//

#ifndef __P11_Q32S_CSFR__
#define __P11_Q32S_CSFR__

#define __RW      volatile       // read write
#define __RO      volatile const // only read
#define __WO      volatile       // only write

#define __u8      unsigned int   // u8  to u32 special for struct
#define __u16     unsigned int   // u16 to u32 special for struct
#define __u32     unsigned int

//---------------------------------------------//
// q32small define
//---------------------------------------------//

#ifdef PMU_SYSTEM
#define p11_q32s_sfr_base     0x00a000
#define p11_q32s_sfr_offset   0x000000  // multi_core used
#else
#define p11_q32s_sfr_base     0xf2a000
#define p11_q32s_sfr_offset   0x000000  // multi_core used
#endif

#define p11_q32s_cpu_base    (p11_q32s_sfr_base + 0x00)
#define p11_q32s_mpu_base    (p11_q32s_sfr_base + 0x80)

#define p11_q32s(n)          ((JL_TypeDef_p11_q32s     *)(p11_q32s_sfr_base + p11_q32s_sfr_offset*n))
#define p11_q32s_mpu(n)      ((JL_TypeDef_p11_q32s_MPU *)(p11_q32s_mpu_base + p11_q32s_sfr_offset*n))

//---------------------------------------------//
// q32small core sfr
//---------------------------------------------//

typedef struct {
    /* 00 */  __RO __u32 DR00;
    /* 01 */  __RO __u32 DR01;
    /* 02 */  __RO __u32 DR02;
    /* 03 */  __RO __u32 DR03;
    /* 04 */  __RO __u32 DR04;
    /* 05 */  __RO __u32 DR05;
    /* 06 */  __RO __u32 DR06;
    /* 07 */  __RO __u32 DR07;
    /* 08 */  __RO __u32 DR08;
    /* 09 */  __RO __u32 DR09;
    /* 0a */  __RO __u32 DR10;
    /* 0b */  __RO __u32 DR11;
    /* 0c */  __RO __u32 DR12;
    /* 0d */  __RO __u32 DR13;
    /* 0e */  __RO __u32 DR14;
    /* 0f */  __RO __u32 DR15;

    /* 10 */  __RO __u32 RETI;
    /* 11 */  __RO __u32 RETE;
    /* 12 */  __RO __u32 RETX;
    /* 13 */  __RO __u32 RETS;
    /* 14 */  __RO __u32 SR04;
    /* 15 */  __RO __u32 PSR;
    /* 16 */  __RO __u32 CNUM;
    /* 17 */  __RO __u32 SR07;
    /* 18 */  __RO __u32 SR08;
    /* 19 */  __RO __u32 SR09;
    /* 1a */  __RO __u32 SR10;
    /* 1b */  __RO __u32 ICFG;
    /* 1c */  __RO __u32 USP;
    /* 1d */  __RO __u32 SSP;
    /* 1e */  __RO __u32 SP;
    /* 1f */  __RO __u32 PCRS;

    /* 20 */  __RW __u32 BPCON;
    /* 21 */  __RW __u32 BSP;
    /* 22 */  __RW __u32 BP0;
    /* 23 */  __RW __u32 BP1;
    /* 24 */  __RW __u32 BP2;
    /* 25 */  __RW __u32 BP3;
    /* 26 */  __WO __u32 CMD_PAUSE;
    /*    */  __RO __u32 REV_30_26[0x30 - 0x26 - 1];

    /* 30 */  __RW __u32 PMU_CON;
    /*    */  __RO __u32 REV_3b_30[0x3b - 0x30 - 1];
    /* 3b */  __RW __u8  TTMR_CON;
    /* 3c */  __RW __u32 TTMR_CNT;
    /* 3d */  __RW __u32 TTMR_PRD;
    /* 3e */  __RW __u32 BANK_CON;
    /* 3f */  __RW __u32 BANK_NUM;

    /* 40 */  __RW __u32 ICFG00;
    /* 41 */  __RW __u32 ICFG01;
    /* 42 */  __RW __u32 ICFG02;
    /* 43 */  __RW __u32 ICFG03;
    /* 44 */  __RW __u32 ICFG04;
    /* 45 */  __RW __u32 ICFG05;
    /* 46 */  __RW __u32 ICFG06;
    /* 47 */  __RW __u32 ICFG07;
    /* 48 */  __RW __u32 ICFG08;
    /* 49 */  __RW __u32 ICFG09;
    /* 4a */  __RW __u32 ICFG10;
    /* 4b */  __RW __u32 ICFG11;
    /* 4c */  __RW __u32 ICFG12;
    /* 4d */  __RW __u32 ICFG13;
    /* 4e */  __RW __u32 ICFG14;
    /* 4f */  __RW __u32 ICFG15;

    /* 50 */  __RW __u32 ICFG16;
    /* 51 */  __RW __u32 ICFG17;
    /* 52 */  __RW __u32 ICFG18;
    /* 53 */  __RW __u32 ICFG19;
    /* 54 */  __RW __u32 ICFG20;
    /* 55 */  __RW __u32 ICFG21;
    /* 56 */  __RW __u32 ICFG22;
    /* 57 */  __RW __u32 ICFG23;
    /* 58 */  __RW __u32 ICFG24;
    /* 59 */  __RW __u32 ICFG25;
    /* 5a */  __RW __u32 ICFG26;
    /* 5b */  __RW __u32 ICFG27;
    /* 5c */  __RW __u32 ICFG28;
    /* 5d */  __RW __u32 ICFG29;
    /* 5e */  __RW __u32 ICFG30;
    /* 5f */  __RW __u32 ICFG31;

    /* 60 */  __RO __u32 IPND0;
    /* 61 */  __RO __u32 IPND1;
    /* 62 */  __RO __u32 IPND2;
    /* 63 */  __RO __u32 IPND3;
    /* 64 */  __RO __u32 IPND4;
    /* 65 */  __RO __u32 IPND5;
    /* 66 */  __RO __u32 IPND6;
    /* 67 */  __RO __u32 IPND7;
    /* 68 */  __WO __u32 ILAT_SET;
    /* 69 */  __WO __u32 ILAT_CLR;
    /* 6a */  __RW __u32 IPMASK;
    /* 6b */  __RW __u32 GIEMASK;
    /* 6c */  __RW __u32 IWKUP_NUM;
    /*    */  __RO __u32 REV_70_6c[0x70 - 0x6c - 1];

    /* 70 */  __RW __u32 ETM_CON;
    /* 71 */  __RO __u32 ETM_PC0;
    /* 72 */  __RO __u32 ETM_PC1;
    /* 73 */  __RO __u32 ETM_PC2;
    /* 74 */  __RO __u32 ETM_PC3;
    /* 75 */  __RW __u32 WP0_ADRH;
    /* 76 */  __RW __u32 WP0_ADRL;
    /* 77 */  __RW __u32 WP0_DATH;
    /* 78 */  __RW __u32 WP0_DATL;
    /* 79 */  __RW __u32 WP0_PC;

    /*    */  __RO __u32 REV_80_79[0x80 - 0x79 - 1];
    /* 80 */  __RW __u32 EMU_CON;
    /* 81 */  __RW __u32 EMU_MSG;
    /* 82 */  __RO __u32 EMU_SSP_H;
    /* 83 */  __RO __u32 EMU_SSP_L;
    /* 84 */  __RO __u32 EMU_USP_H;
    /* 85 */  __RO __u32 EMU_USP_L;
} JL_TypeDef_p11_q32s;

#undef __RW
#undef __RO
#undef __WO

#undef __u8
#undef __u16
#undef __u32

#endif

//*********************************************************************************//
//                                                                                 //
//                               end of this module                                //
//                                                                                 //
//*********************************************************************************//
