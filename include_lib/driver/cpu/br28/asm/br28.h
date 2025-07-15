

#ifndef __BR28__
#define __BR28__

//===============================================================================//
//
//      sfr define
//
//===============================================================================//

#define hs_base            0xfe0000
#define ls_base            0xfd0000

#define __RW               volatile       // read write
#define __RO               volatile const // only read
#define __WO               volatile       // only write

#define __u8               unsigned int   // u8  to u32 special for struct
#define __u16              unsigned int   // u16 to u32 special for struct
#define __u32              unsigned int

#define __s8(x)            char(x); char(reserved_1_##x); char(reserved_2_##x); char(reserved_3_##x)
#define __s16(x)           short(x); short(reserved_1_##x)
#define __s32(x)           int(x)

#define map_adr(grp, adr)  ((64 * grp + adr) * 4)     // grp(0x0-0xff), adr(0x0-0x3f)

//===============================================================================//
//
//      high speed sfr address define
//
//===============================================================================//

//............. 0x0000 - 0x00ff............ for hemu
typedef struct {
    __RW __u32 WREN;
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 MSG0;
    __RW __u32 MSG1;
    __RW __u32 MSG2;
    __RW __u32 MSG3;
    __RO __u32 ID;
} JL_HEMU_TypeDef;

#define JL_HEMU_BASE                 (hs_base + map_adr(0x00, 0x00))
#define JL_HEMU                      ((JL_HEMU_TypeDef    *)JL_HEMU_BASE)


//............. 0x0100 - 0x02ff............ for sfc
typedef struct {
    __RW __u32 CON;
    __RW __u32 BAUD;
    __RW __u32 REDU_BAUD;
    __RW __u32 CODE;
    __RW __u32 BASE_ADR;
    __RW __u32 QUCNT;
    __RW __u8  ENC_CON;
    __RW __u16 ENC_KEY;
    __WO __u32 UNENC_ADRH;
    __WO __u32 UNENC_ADRL;
} JL_SFC_TypeDef;

#define JL_SFC1_BASE                    (hs_base + map_adr(0x01, 0x00))
#define JL_SFC1                         ((JL_SFC_TypeDef    *)JL_SFC1_BASE)

#define JL_SFC_BASE                     (hs_base + map_adr(0x02, 0x00))
#define JL_SFC                          ((JL_SFC_TypeDef    *)JL_SFC_BASE)

//............. 0x0300 - 0x03ff............ for psram
typedef struct {
    __RW __u32 PSRAM_CON0;
    __RW __u32 PSRAM_CON1;
    __RW __u32 PSRAM_CON2;
    __RW __u32 PSRAM_CFG0;
    __RW __u32 PSRAM_CFG1;
    __RW __u32 PSRAM_CMD;
    __RW __u32 PSRAM_DBG_CON;
    __RW __u32 PSRAM_DBG_START;
    __RW __u32 PSRAM_DBG_END;
    __RW __u32 PSRAM_DBG_ADR;
    __RW __u32 PSRAM_DBG_INFO;
} JL_PSRAM_TypeDef;

#define JL_PSRAM_BASE                   (hs_base + map_adr(0x03, 0x00))
#define JL_PSRAM                        ((JL_PSRAM_TypeDef  *)JL_PSRAM_BASE)

//............. 0x0400 - 0x04ff............ for anc
typedef struct {
    __RW __u32 SYS;
    __RW __u32 LEN;
    __RW __u32 ADR;
    __RW __u32 MCTL;
    __RW __u32 MDAT;
    __RW __u32 SYS_RESERVE0;
    __RW __u32 SYS_RESERVE1;
    __RW __u32 SYS_RESERVE2;
    __RW __u32 ANC_CON0;
    __RW __u32 ANC_CON1;
    __RW __u32 ANC_CON2;
    __RW __u32 ANC_CON3;
    __RW __u32 ANC_CON4;
    __RW __u32 ANC_CON5;
    __RW __u32 ANC_CON6;
    __RW __u32 ANC_CON7;
    __RW __u32 ANC_CON8;
    __RW __u32 ANC_CON9;
    __RW __u32 ANC_CON10;
    __RW __u32 ANC_CON11;
    __RW __u32 ANC_CON12;
    __RW __u32 ANC_CON13;
    __RW __u32 ANC_CON14;
    __RW __u32 ANC_CON15;
    __RW __u32 CORE_CON0;
    __RW __u32 CORE_CON1;
    __RW __u32 CORE_CON2;
    __RW __u32 CORE_CON3;
    __RW __u32 CORE_CON4;
    __RW __u32 CORE_CON5;
    __RW __u32 CORE_CON6;
    __RW __u32 CORE_CON7;
    __RW __u32 CORE_CON8;
    __RW __u32 CORE_CON9;
    __RW __u32 CORE_CON10;
    __RW __u32 CORE_CON11;
    __RW __u32 CORE_CON12;
    __RW __u32 CORE_CON13;
    __RW __u32 CORE_CON14;
    __RW __u32 CORE_CON15;
    __RW __u32 CORE_CON16;
    __RW __u32 CORE_CON17;
    __RW __u32 CORE_CON18;
    __RW __u32 CORE_CON19;
    __RW __u32 CORE_CON20;
    __RW __u32 CORE_CON21;
    __RW __u32 CORE_CON22;
    __RW __u32 CORE_CON23;
    __RW __u32 CORE_CON24;
    __RW __u32 CORE_CON25;
    __RW __u32 CORE_CON26;
    __RW __u32 CORE_CON27;
    __RW __u32 CORE_CON28;
    __RW __u32 CORE_CON29;
    __RW __u32 CORE_CON30;
    __RW __u32 CORE_CON31;
    __RW __u32 CORE_CON32;
    __RW __u32 CORE_CON33;
    __RW __u32 CORE_CON34;
    __RW __u32 CORE_CON35;
    __RW __u32 CORE_CON36;
    __RW __u32 CORE_PND0;
    __RW __u32 CORE_PND1;
} JL_ANC_TypeDef;

#define JL_ANC_BASE                   (hs_base + map_adr(0x04, 0x00))
#define JL_ANC                        ((JL_ANC_TypeDef  *)JL_ANC_BASE)

//............. 0x0500 - 0x05ff............ for audio_top
typedef struct {
    __RW __u32(DAC_CON);
    __RW __u32(DAC_ADR);
    __RW __u16(DAC_LEN);
    __RW __u16(DAC_PNS);
    __RW __u16(DAC_HRP);
    __RW __u16(DAC_SWP);
    __RW __u16(DAC_SWN);
    __RW __u32(DAC_VL0);
    __RO __u32 RESERVED8;
    __RW __u32(DAC_TM0);
    __RW __u32(DAC_TM1);
    __RW __u32(DAC_DB0);
    __RW __u32(DAC_DB1);
    __WO __u16(DAC_COP);
    __RW __u32(ADC_CON);
    __RW __u32(ADC_ADR);
    __RW __u16(ADC_LEN);
    __RW __u16(ADC_PNS);
    __RW __u16(ADC_HWP);
    __RW __u16(ADC_SRP);
    __RW __u16(ADC_SRN);
    __RW __u32(MBG_CON);
    __RW __u32(AUD_CON);
    __RW __u16(ADC_CON1);
    __RW __u32(DCC_CON0);
    __RW __u32(DCC_CON1);
    __RO __u32(DCC_CON2);
    __RO __u32(DCC_CON3);
} JL_AUDIO_TypeDef;

#define JL_AUDIO_BASE                (hs_base + map_adr(0x05, 0x00))
#define JL_AUDIO                     ((JL_AUDIO_TypeDef   *)JL_AUDIO_BASE)

//............. 0x0600 - 0x06ff............ for alnk
typedef struct {
    __RW __u16 CON0;
    __RW __u16 CON1;
    __RW __u8  CON2;
    __RW __u8  CON3;
    __WO __u32 ADR0;
    __WO __u32 ADR1;
    __WO __u32 ADR2;
    __WO __u32 ADR3;
    __WO __u16 LEN;
    __RW __u32 PNS;
    __RW __u32 HWPTR0;
    __RW __u32 HWPTR1;
    __RW __u32 HWPTR2;
    __RW __u32 HWPTR3;
    __RW __u32 SWPTR0;
    __RW __u32 SWPTR1;
    __RW __u32 SWPTR2;
    __RW __u32 SWPTR3;
    __RW __u32 SHN0;
    __RW __u32 SHN1;
    __RW __u32 SHN2;
    __RW __u32 SHN3;
} JL_ALNK_TypeDef;

#define JL_ALNK0_BASE                (hs_base + map_adr(0x06, 0x00))
#define JL_ALNK0                     ((JL_ALNK_TypeDef    *)JL_ALNK0_BASE)

//............. 0x0700 - 0x07ff............ for plnk
typedef struct {
    __RW __u16 CON;
    __RW __u8  SMR;
    __RW __u32 ADR;
    __RW __u32 LEN;
    __RW __u16 DOR;
    __RW __u32 CON1;

} JL_PLNK_TypeDef;

#define JL_PLNK_BASE                 (hs_base + map_adr(0x07, 0x00))
#define JL_PLNK                      ((JL_PLNK_TypeDef     *)JL_PLNK_BASE)

//............. 0x0800 - 0x08ff............ for vad
typedef struct {
    __RW __u32(VAD_CON0);
    __RW __u32(VAD_CON1);
    __RW __u32(VAD_CON2);
    __RW __u32(VAD_CON3);
    __RW __u32(VAD_CON4);
    __RW __u32(VAD_CON5);
    __RW __u32(VAD_CON6);
    __RW __u32(VAD_CON7);
    __RW __u32(VAD_CON8);
    __RW __u32(VAD_SIL_BADR);
    __RW __u32(VAD_VOICE_BADR);
    __RO __u32(VAD_VOICE_SADR);
    __RW __u32(DSF1_CON0);
    __RW __u32(DSF1_DCC);
    __RW __u32(AEC_CON0);
    __RW __u32(AEC_CON1);
    __RW __u32(AEC_CON2);
    __RW __u32(AEC_CON3);
    __RW __u32(AEC_CON4);
    __RW __u32(AEC_CON5);
    __RW __u32(AEC_CON6);
    __RW __u32(AEC_CON7);
    __RW __u32(AEC_CON8);
    __RW __u32(AEC_CON9);
    __RW __u32(AEC_CON10);
    __RW __u32(AEC_CON11);
    __RW __u32(AEC_CON12);
    __RW __u32(CLK_CON);
    __RW __u32(MIC_CON);
} JL_VAD_TypeDef;

#define JL_VAD_BASE                (hs_base + map_adr(0x08, 0x00))
#define JL_VAD                     ((JL_VAD_TypeDef   *)JL_VAD_BASE)

//............. 0x0900 - 0x09ff............ for ass_clkgen
typedef struct {
    __RW __u32 CLK_CON;
    __RW __u32 MIC_CTL0;
    __RW __u32 MIC_CTL1;
} JL_ASS_TypeDef;

#define JL_ASS_BASE                (hs_base + map_adr(0x09, 0x00))
#define JL_ASS                     ((JL_ASS_TypeDef  *)JL_ASS_BASE)

//............. 0x0a00 - 0x0aff............ for audio_ana
typedef struct {
    __RW __u32 DAA_CON0;
    __RW __u32 DAA_CON1;
    __RW __u32 DAA_CON2;
    __RW __u32 DAA_CON3;
    __RW __u32 DAA_CON4;
    __RO __u32 RESERVED5;
    __RO __u32 RESERVED6;
    __RO __u32 DAA_CON7;
    __RW __u32 ADA_CON0;
    __RW __u32 ADA_CON1;
    __RW __u32 ADA_CON2;
    __RW __u32 ADA_CON3;
    __RW __u32 ADA_CON4;
    __RW __u32 ADA_CON5;
    __RW __u32 ADA_CON6;
    __RW __u32 ADA_CON7;
    __RW __u32 ADA_CON8;
    __RW __u32 ADA_CON9;
    __RW __u32 ADDA_CON0;
    __RW __u32 ADDA_CON1;
} JL_ADDA_TypeDef;

#define JL_ADDA_BASE               (hs_base + map_adr(0x0a, 0x00))
#define JL_ADDA                    ((JL_ADDA_TypeDef       *)JL_ADDA_BASE)

//............. 0x0b00 - 0x0bff............ for ass_mbist
typedef struct {
    __RW __u32 CON;
    __RW __u32 SEL;
    __RW __u32 DP;
    __RW __u32 DAT_VLD0;
    __RW __u32 DAT_VLD1;
    __RW __u32 DAT_VLD2;
    __RW __u32 DAT_VLD3;
} JL_ASS_MBIST_TypeDef;

#define JL_ASS_MBIST_BASE                     (hs_base + map_adr(0x0b, 0x00))
#define JL_ASS_MBIST                          ((JL_ASS_MBIST_TypeDef  *)JL_ASS_MBIST_BASE)

//............. 0x1300 - 0x13ff............ for dc
typedef struct {
    __RW __u32 CON;
    __RW __u32 ADR;
} JL_DCP_TypeDef;

#define JL_DCP_BASE                     (hs_base + map_adr(0x13, 0x00))
#define JL_DCP                          ((JL_DCP_TypeDef  *)JL_DCP_BASE)


//............. 0x1500 - 0x15ff............ for eq
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 DATAI_ADR;
    __RW __u32 DATAO_ADR;
    __RW __u32 DATA_LEN;
    __RW __u32 FLT_ADR;

} JL_EQ_TypeDef;

#define JL_EQ_BASE                      (hs_base + map_adr(0x15, 0x00))
#define JL_EQ                           ((JL_EQ_TypeDef			*)JL_EQ_BASE)

//............. 0x1600 - 0x16ff............ for src
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 IDAT_ADR;
    __RW __u32 IDAT_LEN;
    __RW __u32 ODAT_ADR;
    __RW __u32 ODAT_LEN;
    __RW __u32 FLTB_ADR;
    __WO __u32 ODAT_ADR_START;
    __WO __u32 ODAT_ADR_END;
    __WO __u32 STOP_FLAG;
    __RW __u32 INSR;
    __RW __u32 OUTSR;
    __RW __u32 PHASE;
    __RW __u32 COEF;

} JL_SRC_TypeDef;

#define JL_SRC_BASE                     (hs_base + map_adr(0x16, 0x00))
#define JL_SRC                          ((JL_SRC_TypeDef			*)JL_SRC_BASE)

//............. 0x1800 - 0x18ff............


//............. 0x2000 - 0x20ff............ for jpg decode
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 WIDTH;
    __RW __u32 YADR;
    __RW __u32 UADR;
    __RW __u32 VADR;
    __RW __u32 BADDR;
    __RW __u32 BCNT;
    __RW __u32 MCUCNT;
    __RW __u32 HQTBLADR;
    __RW __u32 HQTBLDAT;
    __RW __u32 PIXEL_NUM;

} JL_JPG_TypeDef;

#define JL_JPG_BASE                  (hs_base + map_adr(0x20, 0x00))
#define JL_JPG                       ((JL_JPG_TypeDef *)JL_JPG_BASE)


//............. 0x2100 - 0x21ff............ for wireless
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON3;
    __RW __u32 LOFC_CON;
    __RW __u32 LOFC_RES;
    __RW __u32 DBG_CON;
} JL_WL_TypeDef;

#define JL_WL_BASE                      (hs_base + map_adr(0x21, 0x00))
#define JL_WL                           ((JL_WL_TypeDef     *)JL_WL_BASE)

//............. 0x2200 - 0x22ff............
typedef struct {
    __RW __u32 CON0;
} JL_WL_AUD_TypeDef;

#define JL_WL_AUD_BASE                  (hs_base + map_adr(0x22, 0x00))
#define JL_WL_AUD                       ((JL_WL_AUD_TypeDef *)JL_WL_AUD_BASE)

//............. 0x2300 - 0x23ff............

//............. 0x2400 - 0x24ff............

//............. 0x2500 - 0x25ff............ for audio_sync
typedef struct {
    __RW __u32 CON;
    __RW __u32 DABUF;
    __RO __u32 DASPACE;
    __RO __u32 OFFEND;
    __RO __u32 OFFSUM;
    __RO __u32 PCMNUM;
    __RO __u32 BTTIME;
    __WO __u32 CON1  ;
    __RW __u32 CON2  ;
    __WO __u32 BT_OFFSET;
    __WO __u32 MIN_SLOT;
    __RW __u32 OUTSR;
    __WO __u32 INSR;
} JL_SYNC_TypeDef;

#define JL_SYNC_BASE                 (hs_base + map_adr(0x25, 0x00))
#define JL_SYNC                      ((JL_SYNC_TypeDef *)JL_SYNC_BASE)

//............. 0x2600 - 0x26ff............ for src_sync
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 IDAT_ADR;
    __RW __u32 IDAT_LEN;
    __RW __u32 ODAT_ADR;
    __RW __u32 ODAT_LEN;
    __RW __u32 FLTB_ADR;
    __WO __u32 ODAT_ADR_START;
    __WO __u32 ODAT_ADR_END;
    __RW __u32 STOP_FLAG;
    __RW __u32 INSR;
    __RW __u32 OUTSR;
    __RW __u32 PHASE;
    __RW __u32 COEF;

} JL_SRC_SYNC_TypeDef;

#define JL_SRC_SYNC_BASE                     (hs_base + map_adr(0x26, 0x00))
#define JL_SRC_SYNC                          ((JL_SRC_SYNC_TypeDef *)JL_SRC_SYNC_BASE)

//............. 0x2700 - 0x27ff............
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 CON4;
    __RW __u32 CON5;
    __RW __u32 CON6;
    __RW __u32 CON7;
} JL_IMG_CP_TypeDef;

#define JL_IMG_CP_BASE                  (hs_base + map_adr(0x27, 0x00))
#define JL_IMG_CP                       ((JL_IMG_CP_TypeDef *)JL_IMG_CP_BASE)

//............. 0x2800 - 0x28ff............
typedef struct {
    __RW __u32 IMB_CON0;
    __RW __u32 IMB_CON1;
    __RW __u32 IMB_CON2;
    __RW __u32 IMB_CON3;
    __RW __u32 IMB_CON4;
    __RW __u32 IMB_CON5;
    __RW __u32 IMB_CON6;
    __RW __u32 IMB_CON7;
    __RW __u32 IMB_CON8;
    __RW __u32 IMB_CON9;
    __RW __u32 IMB_CON10;
    __RW __u32 IMB_CON11;
    __RW __u32 IMB_CON12;
    __RW __u32 IMB_CON13;
    __RW __u32 IMB_CON14;
    __RW __u32 IMB_CON15;
    __RW __u32 IMB_CON16;
} JL_IMB_TypeDef;

#define JL_IMB_BASE                  (hs_base + map_adr(0x28, 0x00))
#define JL_IMB                       ((JL_IMB_TypeDef *)JL_IMB_BASE)

//............. 0x2900 - 0x29ff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 SEL;
    __RW __u32 DP;
    __RW __u32 DAT_VLD0;
    __RW __u32 DAT_VLD1;
    __RW __u32 DAT_VLD2;
    __RW __u32 DAT_VLD3;
} JL_HMBIST_TypeDef;

#define JL_HMBIST_BASE               (hs_base + map_adr(0x29, 0x00))
#define JL_HMBIST                    ((JL_HMBIST_TypeDef *)JL_HMBIST_BASE)

//............. 0x2a00 - 0x2aff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 SEL;
    __RW __u32 DP;
    __RW __u32 DAT_VLD0;
    __RW __u32 DAT_VLD1;
    __RW __u32 DAT_VLD2;
    __RW __u32 DAT_VLD3;
} JL_WL_MBIST_TypeDef;

#define JL_WL_MBIST_BASE             (hs_base + map_adr(0x2a, 0x00))
#define JL_WL_MBIST                  ((JL_WL_MBIST_TypeDef *)JL_WL_MBIST_BASE)

//............. 0x2b00 - 0x2bff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 SEL;
    __RW __u32 DP;
    __RW __u32 DAT_VLD0;
    __RW __u32 DAT_VLD1;
    __RW __u32 DAT_VLD2;
    __RW __u32 DAT_VLD3;
} JL_JPG_MBIST_TypeDef;

#define JL_JPG_MBIST_BASE            (hs_base + map_adr(0x2b, 0x00))
#define JL_JPG_MBIST                 ((JL_JPG_MBIST_TypeDef *)JL_JPG_MBIST_BASE)

//............. 0x2c00 - 0x2cff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 SEL;
    __RW __u32 DP;
    __RW __u32 DAT_VLD0;
    __RW __u32 DAT_VLD1;
    __RW __u32 DAT_VLD2;
    __RW __u32 DAT_VLD3;
} JL_IMB_MBIST_TypeDef;

#define JL_IMB_MBIST_BASE            (hs_base + map_adr(0x2c, 0x00))
#define JL_IMB_MBIST                 ((JL_IMB_MBIST_TypeDef *)JL_IMB_MBIST_BASE)

//............. 0x2d00 - 0x2dff............
typedef struct {
    __RW __u32 IMD_CON0;
    __RW __u32 IMD_CON1;
    __RW __u32 IMD_CON2;
    __RW __u32 IMD_CON3;
    __RW __u32 IMD_LD_START_ADR;
    __RW __u32 IMD_DEBUG_PIXEL;
    __RW __u32 IMD_IO_CFG;
    __RW __u16 IMD_BAUD;
    __RW __u32 IMDSPI_CON0;
    __RW __u32 IMDSPI_BUF;
    __RW __u32 IMDPAP_CON0;
    __RW __u32 IMDPAP_PRD;
    __RW __u32 IMDPAP_BUF;
    __RW __u32 IMDRGB_CON0;
    __RW __u32 IMDRGB_HPRD;
    __RW __u32 IMDRGB_VPRD;
} JL_IMD_TypeDef;

#define JL_IMD_BASE                  (hs_base + map_adr(0x2d, 0x00))
#define JL_IMD                       ((JL_IMD_TypeDef *)JL_IMD_BASE)


//===============================================================================//
//
//      low speed sfr address define
//
//===============================================================================//

//............. 0x0000 - 0x00ff............ for syscfg
typedef struct {
    __RW __u32 RST_SRC;
} JL_RST_TypeDef;

#define JL_RST_BASE             (ls_base + map_adr(0x00, 0x00))
#define JL_RST                  ((JL_RST_TypeDef        *)JL_RST_BASE)

typedef struct {
    __RW __u32 PWR_CON;
    //__RW __u32 HTC_CON;
    __RW __u32 DCCK_CON0;
    __RW __u32 DCCK_CON1;
    __RW __u32 DCCK_CON2;
    __RW __u32 SYS_DIV;
    __RW __u32 CLK_CON0;
    __RW __u32 CLK_CON1;
    __RW __u32 CLK_CON2;
    __RW __u32 CLK_CON3;
} JL_CLOCK_TypeDef;

#define JL_CLOCK_BASE           (ls_base + map_adr(0x00, 0x01))
#define JL_CLOCK                ((JL_CLOCK_TypeDef      *)JL_CLOCK_BASE)

//............. 0x0100 - 0x01ff............
typedef struct {
    __RW __u32 MODE_CON;
} JL_MODE_TypeDef;

#define JL_MODE_BASE                  (ls_base + map_adr(0x01, 0x00))
#define JL_MODE                       ((JL_MODE_TypeDef     *)JL_MODE_BASE)

//............. 0x0200 - 0x02ff............
typedef struct {
    __WO __u32 CHIP_ID;
} JL_SYSTEM_TypeDef;

#define JL_SYSTEM_BASE                  (ls_base + map_adr(0x02, 0x00))
#define JL_SYSTEM                       ((JL_SYSTEM_TypeDef     *)JL_SYSTEM_BASE)

//............. 0x0300 - 0x03ff............
typedef struct {
    __RW __u32 WREN;
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 MSG0;
    __RW __u32 MSG1;
    __RW __u32 MSG2;
    __RW __u32 MSG3;
    __RO __u32 ID;
} JL_LEMU_TypeDef;

#define JL_LEMU_BASE                  (ls_base + map_adr(0x03, 0x00))
#define JL_LEMU                       ((JL_LEMU_TypeDef    *)JL_LEMU_BASE)

//............. 0x0400 - 0x09ff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 CNT;
    __RW __u32 PRD;
    __RW __u32 PWM;
} JL_TIMER_TypeDef;

#define JL_TIMER0_BASE                  (ls_base + map_adr(0x04, 0x00))
#define JL_TIMER0                       ((JL_TIMER_TypeDef     *)JL_TIMER0_BASE)

#define JL_TIMER1_BASE                  (ls_base + map_adr(0x05, 0x00))
#define JL_TIMER1                       ((JL_TIMER_TypeDef     *)JL_TIMER1_BASE)

#define JL_TIMER2_BASE                  (ls_base + map_adr(0x06, 0x00))
#define JL_TIMER2                       ((JL_TIMER_TypeDef     *)JL_TIMER2_BASE)

#define JL_TIMER3_BASE                  (ls_base + map_adr(0x07, 0x00))
#define JL_TIMER3                       ((JL_TIMER_TypeDef     *)JL_TIMER3_BASE)

#define JL_TIMER4_BASE                  (ls_base + map_adr(0x08, 0x00))
#define JL_TIMER4                       ((JL_TIMER_TypeDef     *)JL_TIMER4_BASE)

#define JL_TIMER5_BASE                  (ls_base + map_adr(0x09, 0x00))
#define JL_TIMER5                       ((JL_TIMER_TypeDef     *)JL_TIMER5_BASE)


//............. 0x1000 - 0x10ff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 VAL;
} JL_PCNT_TypeDef;

#define JL_PCNT_BASE                    (ls_base + map_adr(0x10, 0x00))
#define JL_PCNT                         ((JL_PCNT_TypeDef       *)JL_PCNT_BASE)

//............. 0x1100 - 0x11ff............
typedef struct {
    __RW __u32 CON;
    __RO __u32 NUM;
} JL_GPCNT_TypeDef;

#define JL_GPCNT_BASE                   (ls_base + map_adr(0x11, 0x00))
#define JL_GPCNT                        ((JL_GPCNT_TypeDef     *)JL_GPCNT_BASE)


//............. 0x1400 - 0x17ff............
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __WO __u32 CPTR;
    __WO __u32 DPTR;
    __RW __u32 CTU_CON;
    __WO __u32 CTU_CNT;
} JL_SD_TypeDef;

#define JL_SD0_BASE                     (ls_base + map_adr(0x14, 0x00))
#define JL_SD0                          ((JL_SD_TypeDef        *)JL_SD0_BASE)

//............. 0x1800 - 0x18ff............
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __WO __u32 EP0_CNT;
    __WO __u32 EP1_CNT;
    __WO __u32 EP2_CNT;
    __WO __u32 EP3_CNT;
    __WO __u32 EP4_CNT;
    __WO __u32 EP0_ADR;
    __WO __u32 EP1_TADR;
    __WO __u32 EP1_RADR;
    __WO __u32 EP2_TADR;
    __WO __u32 EP2_RADR;
    __WO __u32 EP3_TADR;
    __WO __u32 EP3_RADR;
    __WO __u32 EP4_TADR;
    __WO __u32 EP4_RADR;
    __RW __u32 TXDLY_CON;
    __RW __u32 EP1_RLEN;
    __RW __u32 EP2_RLEN;
    __RW __u32 EP3_RLEN;
    __RW __u32 EP4_RLEN;
    __RW __u32 EP1_MTX_PRD;
    __RW __u32 EP1_MRX_PRD;
    __RO __u32 EP1_MTX_NUM;
    __RO __u32 EP1_MRX_NUM;
} JL_USB_TypeDef;

#define JL_USB_BASE                     (ls_base + map_adr(0x18, 0x00))
#define JL_USB                          ((JL_USB_TypeDef       *)JL_USB_BASE)

//............. 0x1900 - 0x19ff............
typedef struct {
    __RW __u32 WLA_CON0 ;
    __RW __u32 WLA_CON1 ;
    __RW __u32 WLA_CON2 ;
    __RW __u32 WLA_CON3 ;
    __RW __u32 WLA_CON4 ;
    __RW __u32 WLA_CON5 ;
    __RW __u32 WLA_CON6 ;
    __RW __u32 WLA_CON7 ;
    __RW __u32 WLA_CON8 ;
    __RW __u32 WLA_CON9 ;
    __RW __u32 WLA_CON10;
    __RW __u32 WLA_CON11;
    __RW __u32 WLA_CON12;
    __RW __u32 WLA_CON13;
    __RW __u32 WLA_CON14;
    __RW __u32 WLA_CON15;
    __RW __u32 WLA_CON16;
    __RW __u32 WLA_CON17;
    __RW __u32 WLA_CON18;
    __RW __u32 WLA_CON19;
    __RW __u32 WLA_CON20;
    __RW __u32 WLA_CON21;
    __RW __u32 WLA_CON22;
    __RW __u32 WLA_CON23;
    __RW __u32 WLA_CON24;
    __RW __u32 WLA_CON25;
    __RW __u32 WLA_CON26;
    __RW __u32 WLA_CON27;
    __RW __u32 WLA_CON28;
    __RW __u32 WLA_CON29;
    __RO __u32 WLA_CON30;
    __RO __u32 WLA_CON31;
    __RO __u32 WLA_CON32;
    __RO __u32 WLA_CON33;
    __RO __u32 WLA_CON34;
    __RO __u32 WLA_CON35;
    __RO __u32 WLA_CON36;
    __RO __u32 WLA_CON37;
    __RO __u32 WLA_CON38;
    __RO __u32 WLA_CON39;
} JL_WLA_TypeDef;

#define JL_WLA_BASE                     (ls_base + map_adr(0x19, 0x00))
#define JL_WLA                          ((JL_WLA_TypeDef       *)JL_WLA_BASE)

//............. 0x1c00 - 0x1eff............
typedef struct {
    __RW __u32 CON;
    __WO __u32 BAUD;
    __RW __u32 BUF;
    __WO __u32 ADR;
    __WO __u32 CNT;
    __RW __u32 CON1;
} JL_SPI_TypeDef;

#define JL_SPI0_BASE                    (ls_base + map_adr(0x1c, 0x00))
#define JL_SPI0                         ((JL_SPI_TypeDef      *)JL_SPI0_BASE)

#define JL_SPI1_BASE                    (ls_base + map_adr(0x1d, 0x00))
#define JL_SPI1                         ((JL_SPI_TypeDef      *)JL_SPI1_BASE)

#define JL_SPI2_BASE                    (ls_base + map_adr(0x1e, 0x00))
#define JL_SPI2                         ((JL_SPI_TypeDef      *)JL_SPI2_BASE)

//............. 0x2000 - 0x22ff............
typedef struct {
    __RW __u16 CON0;
    __RW __u16 CON1;
    __WO __u16 BAUD;
    __RW __u8  BUF;
    __RW __u32 OTCNT;
    __RW __u32 TXADR;
    __WO __u16 TXCNT;
    __RW __u32 RXSADR;
    __RW __u32 RXEADR;
    __RW __u32 RXCNT;
    __RO __u16 HRXCNT;
    __RW __u16 CON2;
} JL_UART_TypeDef;

#define JL_UART0_BASE                   (ls_base + map_adr(0x20, 0x00))
#define JL_UART0                        ((JL_UART_TypeDef       *)JL_UART0_BASE)

#define JL_UART1_BASE                   (ls_base + map_adr(0x21, 0x00))
#define JL_UART1                        ((JL_UART_TypeDef       *)JL_UART1_BASE)

#define JL_UART2_BASE                   (ls_base + map_adr(0x22, 0x00))
#define JL_UART2                        ((JL_UART_TypeDef       *)JL_UART2_BASE)

//............. 0x2400 - 0x24ff............
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __WO __u16 BAUD;
    __RW __u8  BUF;
} JL_IIC_TypeDef;

#define JL_IIC_BASE                     (ls_base + map_adr(0x24, 0x00))
#define JL_IIC                          ((JL_IIC_TypeDef       *)JL_IIC_BASE)

//............. 0x2800 - 0x28ff............
typedef struct {
    __RW __u32 CON0;
    __RW __u32 WCLK_DUTY;
    __RW __u32 WCLK_PRD;
    __WO __u32 DMA_BASE_ADR;
    __WO __u32 DMA_BUF_LEN;
    __RW __u32 SLOT_EN;

} JL_TDM_TypeDef;

#define JL_TDM_BASE                   (ls_base + map_adr(0x28, 0x00))
#define JL_TDM                        ((JL_TDM_TypeDef *)JL_TDM_BASE)

//............. 0x2900 - 0x29ff............
typedef struct {
    __RW __u32 TMR0_CON;
    __RW __u32 TMR0_CNT;
    __RW __u32 TMR0_PR;
    __RW __u32 TMR1_CON;
    __RW __u32 TMR1_CNT;
    __RW __u32 TMR1_PR;
    __RW __u32 TMR2_CON;
    __RW __u32 TMR2_CNT;
    __RW __u32 TMR2_PR;
    __RW __u32 TMR3_CON;
    __RW __u32 TMR3_CNT;
    __RW __u32 TMR3_PR;
    __RW __u32 TMR4_CON;
    __RW __u32 TMR4_CNT;
    __RW __u32 TMR4_PR;
    __RW __u32 TMR5_CON;
    __RW __u32 TMR5_CNT;
    __RW __u32 TMR5_PR;
    __RW __u32 TMR6_CON;
    __RW __u32 TMR6_CNT;
    __RW __u32 TMR6_PR;
    __RW __u32 TMR7_CON;
    __RW __u32 TMR7_CNT;
    __RW __u32 TMR7_PR;
    __RW __u32 FPIN_CON;
    __RW __u32 CH0_CON0;
    __RW __u32 CH0_CON1;
    __RW __u32 CH0_CMPH;
    __RW __u32 CH0_CMPL;
    __RW __u32 CH1_CON0;
    __RW __u32 CH1_CON1;
    __RW __u32 CH1_CMPH;
    __RW __u32 CH1_CMPL;
    __RW __u32 CH2_CON0;
    __RW __u32 CH2_CON1;
    __RW __u32 CH2_CMPH;
    __RW __u32 CH2_CMPL;
    __RW __u32 CH3_CON0;
    __RW __u32 CH3_CON1;
    __RW __u32 CH3_CMPH;
    __RW __u32 CH3_CMPL;
    __RW __u32 CH4_CON0;
    __RW __u32 CH4_CON1;
    __RW __u32 CH4_CMPH;
    __RW __u32 CH4_CMPL;
    __RW __u32 CH5_CON0;
    __RW __u32 CH5_CON1;
    __RW __u32 CH5_CMPH;
    __RW __u32 CH5_CMPL;
    __RW __u32 CH6_CON0;
    __RW __u32 CH6_CON1;
    __RW __u32 CH6_CMPH;
    __RW __u32 CH6_CMPL;
    __RW __u32 CH7_CON0;
    __RW __u32 CH7_CON1;
    __RW __u32 CH7_CMPH;
    __RW __u32 CH7_CMPL;
    __RW __u32 MCPWM_CON0;
} JL_MCPWM_TypeDef;

#define JL_MCPWM_BASE                   (ls_base + map_adr(0x29, 0x00))
#define JL_MCPWM                        ((JL_MCPWM_TypeDef     *)JL_MCPWM_BASE)

//............. 0x2b00 - 0x2dff............
typedef struct {
    __RW __u8  CON;
    __RW __u8  DAT;
    __RW __u8  SMP;
} JL_RDEC_TypeDef;

#define JL_RDEC0_BASE                   (ls_base + map_adr(0x2b, 0x00))
#define JL_RDEC0                        ((JL_RDEC_TypeDef       *)JL_RDEC0_BASE)

#define JL_RDEC1_BASE                   (ls_base + map_adr(0x2c, 0x00))
#define JL_RDEC1                        ((JL_RDEC_TypeDef       *)JL_RDEC1_BASE)

#define JL_RDEC2_BASE                   (ls_base + map_adr(0x2d, 0x00))
#define JL_RDEC2                        ((JL_RDEC_TypeDef       *)JL_RDEC2_BASE)

//............. 0x3100 - 0x31ff............
typedef struct {
    __RW __u32 CON;
    __RO __u32 RES;
} JL_ADC_TypeDef;

#define JL_ADC_BASE                     (ls_base + map_adr(0x31, 0x00))
#define JL_ADC                          ((JL_ADC_TypeDef       *)JL_ADC_BASE)

//............. 0x3200 - 0x32ff............
typedef struct {
    __RW __u32 RFLT_CON;
} JL_IR_TypeDef;

#define JL_IR_BASE                      (ls_base + map_adr(0x32, 0x00))
#define JL_IR                           ((JL_IR_TypeDef         *)JL_IR_BASE)

//............. 0x3400 - 0x34ff............
typedef struct {
    __RW __u32 CON;
} JL_OSA_TypeDef;

#define JL_OSA_BASE                     (ls_base + map_adr(0x34, 0x00))
#define JL_OSA                          ((JL_OSA_TypeDef          *)JL_OSA_BASE)

//............. 0x3500 - 0x35ff............
typedef struct {
    __WO __u32 FIFO;
    __RW __u32 REG;
} JL_CRC_TypeDef;

#define JL_CRC_BASE                     (ls_base + map_adr(0x35, 0x00))
#define JL_CRC                          ((JL_CRC_TypeDef       *)JL_CRC_BASE)


//............. 0x3600 - 0x36ff............
typedef struct {
    __WO __u32 CON;
    __RW __u32 NUM;
} JL_LRCT_TypeDef;

#define JL_LRCT_BASE                    (ls_base + map_adr(0x36, 0x00))
#define JL_LRCT                         ((JL_LRCT_TypeDef     *)JL_LRCT_BASE)

//............. 0x3b00 - 0x3bff............
typedef struct {
    __RO __u32 R64L;
    __RO __u32 R64H;
} JL_RAND_TypeDef;

#define JL_RAND_BASE                    (ls_base + map_adr(0x3b, 0x00))
#define JL_RAND                         ((JL_RAND_TypeDef   *)JL_RAND_BASE)

//............. 0x3e00 - 0x3eff............ for p33
typedef struct {
    __RW __u32 PMU_CON;
    __RW __u32 PMU_STA;
} JL_PMU_TypeDef;

#define JL_PMU_BASE                 (ls_base + map_adr(0x3e, 0x00))
#define JL_PMU                      ((JL_PMU_TypeDef        *)JL_PMU_BASE)

//............. 0x4000 - 0x40ff............
typedef struct {
    __RW __u32 CON;
    __RW __u32 BASE;
    __RW __u32 ADC_CON;
    __RW __u32 ADC_CON1;
    __RW __u32 HF_CON0;
    __RW __u32 HF_CON1;
    __RW __u32 HF_CRAM;
    __RW __u32 HF_CRAM2;
    __RW __u32 HF_DRAM;
    __RW __u32 LF_CON;
    __RW __u32 LF_RES;
    __RW __u32 FMRX_CON4;
    __RW __u32 FMRX_CON5;

    __RW __u32 TX_CON0;
    __RW __u32 TX_CON1;
    __RW __u32 TX_PILOT;
    __RW __u32 TX_SYN_GAIN;
    __RW __u32 TX_MUL;
    __RW __u32 TX_ADR;
    __RW __u32 TX_LEN;
    __RW __u32 TX_FREQ;
    __RW __u32 TX_BASE_ADR;
} JL_FM_TypeDef;

#define JL_FM_BASE                      (ls_base + map_adr(0x40, 0x00))
#define JL_FM                           ((JL_FM_TypeDef			*)JL_FM_BASE)


//............. 0x4100 - 0x41ff............ for lsb peri(spi0/sd0)
typedef struct {
    __RW __u8  ENCCON ;
    __WO __u16 ENCKEY ;
    __WO __u16 ENCADR ;
} JL_PERIENC_TypeDef;

#define JL_PERIENC_BASE             (ls_base + map_adr(0x41, 0x00))
#define JL_PERIENC                  ((JL_PERIENC_TypeDef *)JL_PERIENC_BASE)

//............. 0x4200 - 0x42ff............ for sbc
typedef struct {
    __RW __u32 CON0;
    __WO __u32 DEC_SRC_ADR;
    __WO __u32 DEC_DST_ADR;
    __WO __u32 DEC_PCM_WCNT;
    __WO __u32 DEC_INBUF_LEN;
    __WO __u32 ENC_SRC_ADR;
    __WO __u32 ENC_DST_ADR;
    __RO __u32 DEC_DST_BASE;
    __WO __u32 DEC_WEIGHT_BASE;
    __WO __u32 DEC_WEIGHT_ADD;

} JL_SBC_TypeDef;

#define JL_SBC_BASE                   (ls_base + map_adr(0x42, 0x00))
#define JL_SBC                        ((JL_SBC_TypeDef *)JL_SBC_BASE)

//............. 0x4300 - 0x43ff............ for aes
typedef struct {
    __RW __u32 CON;
    __RW __u32 DATIN;
    __WO __u32 KEY;
    __RW __u32 ENCRES0;
    __RW __u32 ENCRES1;
    __RW __u32 ENCRES2;
    __RW __u32 ENCRES3;
    __WO __u32 NONCE;
    __WO __u8  HEADER;
    __WO __u32 SRCADR;
    __WO __u32 DSTADR;
    __WO __u32 CTCNT;
    __WO __u32 TAGLEN;
    __RO __u32 TAGRES0;
    __RO __u32 TAGRES1;
    __RO __u32 TAGRES2;
    __RO __u32 TAGRES3;
} JL_AES_TypeDef;

#define JL_AES_BASE               (ls_base + map_adr(0x43, 0x00))
#define JL_AES                    ((JL_AES_TypeDef *)JL_AES_BASE)

//............. 0x4600 - 0x46ff............ for pll0_ctl
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 CON4;
    __RW __u32 INTF0;
    __RW __u32 INTF1;
} JL_PLL0_TypeDef;

#define JL_PLL0_BASE                   (ls_base + map_adr(0x46, 0x00))
#define JL_PLL0                        ((JL_PLL0_TypeDef      *)JL_PLL0_BASE)


//............. 0x4700 - 0x47ff............ for pll1_ctl
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 CON4;
    __RW __u32 INTF0;
    __RW __u32 INTF1;
} JL_PLL1_TypeDef;

#define JL_PLL1_BASE                   (ls_base + map_adr(0x47, 0x00))
#define JL_PLL1                        ((JL_PLL1_TypeDef      *)JL_PLL1_BASE)


//............. 0x4a00 - 0x4fff............ for sie
typedef struct {
    __RW __u32 FADDR;
    __RW __u32 POWER;
    __RO __u32 INTRTX1;
    __RO __u32 INTRTX2;
    __RO __u32 INTRRX1;
    __RO __u32 INTRRX2;
    __RO __u32 INTRUSB;
    __RW __u32 INTRTX1E;
    __RW __u32 INTRTX2E;
    __RW __u32 INTRRX1E;
    __RW __u32 INTRRX2E;
    __RW __u32 INTRUSBE;
    __RO __u32 FRAME1;
    __RO __u32 FRAME2;
    __RO __u32 RESERVED14;
    __RW __u32 DEVCTL;
    __RO __u32 RESERVED10_0x16[0x16 - 0x10 + 1];

} JL_USB_SIE_TypeDef;

#define JL_USB_SIE_BASE                   (ls_base + map_adr(0x4a, 0x00))
#define JL_USB_SIE                        ((JL_USB_SIE_TypeDef *)JL_USB_SIE_BASE)

typedef struct {
    __RO __u32 RESERVED0;
    __RW __u32 CSR0;
    __RO __u32 RESERVED2_5[5 - 1];
    __RO __u32 COUNT0;

} JL_USB_EP0_TypeDef;

#define JL_USB_EP0_BASE                   (ls_base + map_adr(0x4a, 0x10))
#define JL_USB_EP0                        ((JL_USB_EP0_TypeDef *)JL_USB_EP0_BASE)


typedef struct {
    __RW __u32 TXMAXP;
    __RW __u32 TXCSR1;
    __RW __u32 TXCSR2;
    __RW __u32 RXMAXP;
    __RW __u32 RXCSR1;
    __RW __u32 RXCSR2;
    __RO __u32 RXCOUNT1;
    __RO __u32 RXCOUNT2;
    __RW __u32 TXTYPE;
    __RO __u32 TXINTERVAL;
    __RW __u32 RXTYPE;
    __RO __u32 RXINTERVAL;

} JL_USB_EP_TypeDef;

#define JL_USB_EP1_BASE                   (ls_base + map_adr(0x4b, 0x10))
#define JL_USB_EP1                        ((JL_USB_EP_TypeDef *)JL_USB_EP1_BASE)

#define JL_USB_EP2_BASE                   (ls_base + map_adr(0x4c, 0x10))
#define JL_USB_EP2                        ((JL_USB_EP_TypeDef *)JL_USB_EP2_BASE)

#define JL_USB_EP3_BASE                   (ls_base + map_adr(0x4d, 0x10))
#define JL_USB_EP3                        ((JL_USB_EP_TypeDef *)JL_USB_EP3_BASE)

#define JL_USB_EP4_BASE                   (ls_base + map_adr(0x4e, 0x10))
#define JL_USB_EP4                        ((JL_USB_EP_TypeDef *)JL_USB_EP4_BASE)


//............. 0x5000 - 0x57ff............ for port
typedef struct {
    __RW __u32 OUT;
    __RO __u32 IN;
    __RW __u32 DIR;
    __RW __u32 DIE;
    __RW __u32 PU;
    __RW __u32 PD;
    __RW __u32 HD0;
    __RW __u32 HD;
    __RW __u32 DIEH;
} JL_PORT_FLASH_TypeDef;

#define JL_PORTA_BASE                   (ls_base + map_adr(0x50, 0x00))
#define JL_PORTA                        ((JL_PORT_FLASH_TypeDef *)JL_PORTA_BASE)

#define JL_PORTB_BASE                   (ls_base + map_adr(0x51, 0x00))
#define JL_PORTB                        ((JL_PORT_FLASH_TypeDef *)JL_PORTB_BASE)

#define JL_PORTC_BASE                   (ls_base + map_adr(0x52, 0x00))
#define JL_PORTC                        ((JL_PORT_FLASH_TypeDef *)JL_PORTC_BASE)

#define JL_PORTD_BASE                   (ls_base + map_adr(0x53, 0x00))
#define JL_PORTD                        ((JL_PORT_FLASH_TypeDef *)JL_PORTD_BASE)

#define JL_PORTE_BASE                   (ls_base + map_adr(0x54, 0x00))
#define JL_PORTE                        ((JL_PORT_FLASH_TypeDef *)JL_PORTE_BASE)

#define JL_PORTG_BASE                   (ls_base + map_adr(0x55, 0x00))
#define JL_PORTG                        ((JL_PORT_FLASH_TypeDef *)JL_PORTG_BASE)

#define JL_PORTP_BASE                   (ls_base + map_adr(0x57, 0x00))
#define JL_PORTP                        ((JL_PORT_FLASH_TypeDef *)JL_PORTP_BASE)

//............. 0x6000 - 0x60ff............ for port others
typedef struct {
    __RW __u32 CON0;
} JL_USB_IO_TypeDef;

#define JL_USB_IO_BASE                  (ls_base + map_adr(0x60, 0x00))
#define JL_USB_IO                       ((JL_USB_IO_TypeDef    *)JL_USB_IO_BASE)

typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
} JL_WAKEUP_TypeDef;

#define JL_WAKEUP_BASE               (ls_base + map_adr(0x60, 0x01))
#define JL_WAKEUP                    ((JL_WAKEUP_TypeDef			*)JL_WAKEUP_BASE)

typedef struct {
    //__RW __u32 IOMC0;
    __RW __u32 CON0;
    __RW __u32 OCH_CON0;
    __RW __u32 OCH_CON1;
    __RW __u32 OCH_CON2;
    __RW __u32 ICH_CON0;
    __RW __u32 ICH_CON1;
    __RW __u32 ICH_CON2;
    __RW __u32 ICH_CON3;
} JL_IOMC_TypeDef;

#define JL_IOMC_BASE                   (ls_base + map_adr(0x60, 0x05))
#define JL_IOMC                        ((JL_IOMC_TypeDef      *)JL_IOMC_BASE)
#define JL_IOMAP                        ((JL_IOMC_TypeDef      *)JL_IOMC_BASE)

//............. 0x6100 - 0x61ff............ for led
typedef struct {
    __RW __u32 CON0;
    __RW __u32 CON1;
    __RW __u32 CON2;
    __RW __u32 CON3;
    __RW __u32 BRI_PRDL;
    __RW __u32 BRI_PRDH;
    __RW __u32 BRI_DUTY0L;
    __RW __u32 BRI_DUTY0H;
    __RW __u32 BRI_DUTY1L;
    __RW __u32 BRI_DUTY1H;
    __RW __u32 PRD_DIVL;
    __RW __u32 DUTY0;
    __RW __u32 DUTY1;
    __RW __u32 DUTY2;
    __RW __u32 DUTY3;
    __RO __u32 CNT_RD;
    __RW __u32 CON4;
    __WO __u32 CNT_SYNC;
    __RW __u32 CNT_DEC;
} JL_PLED_TypeDef;

#define JL_PLED_BASE                   (ls_base + map_adr(0x61, 0x00))
#define JL_PLED                        ((JL_PLED_TypeDef      *)JL_PLED_BASE)

#include "io_omap.h"
#include "io_imap.h"

#endif
