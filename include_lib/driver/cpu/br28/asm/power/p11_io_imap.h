

//===============================================================================//
//
//      input IO define
//
//===============================================================================//
#define P11_PB0_IN  1
#define P11_PB1_IN  2
#define P11_PB2_IN  3
#define P11_PB3_IN  4
#define P11_PB4_IN  5
#define P11_PB5_IN  6
#define P11_PB6_IN  7
#define P11_PB7_IN  8
#define P11_PB8_IN  9
#define P11_PB9_IN  10
#define P11_PB10_IN  11
#define P11_PB11_IN  12

//===============================================================================//
//
//      function input select sfr
//
//===============================================================================//
typedef struct {
    __RW __u8 P11_FI_GP_ICH0;
    __RW __u8 P11_FI_GP_ICH1;
    __RW __u8 P11_FI_GP_ICH2;
    __RW __u8 P11_FI_UART0_RX;
    __RW __u8 P11_FI_SPI_DI;
    __RW __u8 P11_FI_IIC_SCL;
    __RW __u8 P11_FI_IIC_SDA;
    __RW __u8 P11_FI_DMIC_DAT;
} P11_IMAP_TypeDef;

#define P11_IMAP_BASE      (p11_sfr_base + map_adr(0x16, 0x00))
#define P11_IMAP           ((P11_IMAP_TypeDef   *)P11_IMAP_BASE)
