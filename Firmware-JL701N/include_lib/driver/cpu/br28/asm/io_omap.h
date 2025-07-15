

//===============================================================================//
//
//      output function define
//
//===============================================================================//
#define FO_GP_OCH0        ((0 << 2)|BIT(1))
#define FO_GP_OCH1        ((1 << 2)|BIT(1))
#define FO_GP_OCH2        ((2 << 2)|BIT(1))
#define FO_GP_OCH3        ((3 << 2)|BIT(1))
#define FO_GP_OCH4        ((4 << 2)|BIT(1))
#define FO_GP_OCH5        ((5 << 2)|BIT(1))
#define FO_GP_OCH6        ((6 << 2)|BIT(1))
#define FO_GP_OCH7        ((7 << 2)|BIT(1))
#define FO_GP_OCH8        ((8 << 2)|BIT(1))
#define FO_GP_OCH9        ((9 << 2)|BIT(1))
#define FO_GP_OCH10        ((10 << 2)|BIT(1))
#define FO_GP_OCH11        ((11 << 2)|BIT(1))
#define FO_GP_OCH12        ((12 << 2)|BIT(1))
#define FO_GP_OCH13        ((13 << 2)|BIT(1))
#define FO_GP_OCH14        ((14 << 2)|BIT(1))
#define FO_GP_OCH15        ((15 << 2)|BIT(1))
#define FO_SPI0_CLK        ((16 << 2)|BIT(1)|BIT(0))
#define FO_SPI0_DA0        ((17 << 2)|BIT(1)|BIT(0))
#define FO_SPI0_DA1        ((18 << 2)|BIT(1)|BIT(0))
#define FO_SPI0_DA2        ((19 << 2)|BIT(1)|BIT(0))
#define FO_SPI0_DA3        ((20 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_CLK        ((21 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA0        ((22 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA1        ((23 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA2        ((24 << 2)|BIT(1)|BIT(0))
#define FO_SPI1_DA3        ((25 << 2)|BIT(1)|BIT(0))
#define FO_SPI2_CLK        ((26 << 2)|BIT(1)|BIT(0))
#define FO_SPI2_DA0        ((27 << 2)|BIT(1)|BIT(0))
#define FO_SPI2_DA1        ((28 << 2)|BIT(1)|BIT(0))
#define FO_SPI2_DA2        ((29 << 2)|BIT(1)|BIT(0))
#define FO_SPI2_DA3        ((30 << 2)|BIT(1)|BIT(0))
#define FO_SD0_CLK        ((31 << 2)|BIT(1)|BIT(0))
#define FO_SD0_CMD        ((32 << 2)|BIT(1)|BIT(0))
#define FO_SD0_DA0        ((33 << 2)|BIT(1)|BIT(0))
#define FO_SD0_DA1        ((34 << 2)|BIT(1)|BIT(0))
#define FO_SD0_DA2        ((35 << 2)|BIT(1)|BIT(0))
#define FO_SD0_DA3        ((36 << 2)|BIT(1)|BIT(0))
#define FO_IIC_SCL        ((37 << 2)|BIT(1)|BIT(0))
#define FO_IIC_SDA        ((38 << 2)|BIT(1)|BIT(0))
#define FO_UART0_TX        ((39 << 2)|BIT(1)|BIT(0))
#define FO_UART1_TX        ((40 << 2)|BIT(1)|BIT(0))
#define FO_UART1_RTS        ((41 << 2)|BIT(1)|BIT(0))
#define FO_UART2_TX        ((42 << 2)|BIT(1)|BIT(0))
#define FO_TDM_M_MCK        ((43 << 2)|BIT(1)|BIT(0))
#define FO_TDM_M_WCK        ((44 << 2)|BIT(1)|BIT(0))
#define FO_TDM_M_BCK        ((45 << 2)|BIT(1)|BIT(0))
#define FO_TDM_S_DA        ((46 << 2)|BIT(1)|BIT(0))
#define FO_ALNK0_MCLK        ((47 << 2)|BIT(1)|BIT(0))
#define FO_ALNK0_LRCK        ((48 << 2)|BIT(1)|BIT(0))
#define FO_ALNK0_SCLK        ((49 << 2)|BIT(1)|BIT(0))
#define FO_ALNK0_DAT0        ((50 << 2)|BIT(1)|BIT(0))
#define FO_ALNK0_DAT1        ((51 << 2)|BIT(1)|BIT(0))
#define FO_ALNK0_DAT2        ((52 << 2)|BIT(1)|BIT(0))
#define FO_ALNK0_DAT3        ((53 << 2)|BIT(1)|BIT(0))
#define FO_PLNK_SCLK        ((54 << 2)|BIT(1)|BIT(0))
#define FO_WL_ANT_ID0        ((55 << 2)|BIT(1)|BIT(0))
#define FO_WL_ANT_ID1        ((56 << 2)|BIT(1)|BIT(0))
#define FO_WL_ANT_ID2        ((57 << 2)|BIT(1)|BIT(0))
#define FO_WL_ANT_ID3        ((58 << 2)|BIT(1)|BIT(0))
#define FO_CHAIN_OUT0        ((59 << 2)|BIT(1)|BIT(0))
#define FO_CHAIN_OUT1        ((60 << 2)|BIT(1)|BIT(0))
#define FO_CHAIN_OUT2        ((61 << 2)|BIT(1)|BIT(0))
#define FO_CHAIN_OUT3        ((62 << 2)|BIT(1)|BIT(0))

//===============================================================================//
//
//      IO output select sfr
//
//===============================================================================//
typedef struct {
    __RW __u8 PA0_OUT;
    __RW __u8 PA1_OUT;
    __RW __u8 PA2_OUT;
    __RW __u8 PA3_OUT;
    __RW __u8 PA4_OUT;
    __RW __u8 PA5_OUT;
    __RW __u8 PA6_OUT;
    __RW __u8 PA7_OUT;
    __RW __u8 PA8_OUT;
    __RW __u8 PA9_OUT;
    __RW __u8 PA10_OUT;
    __RW __u8 PA11_OUT;
    __RW __u8 PA12_OUT;
    __RW __u8 PA13_OUT;
    __RW __u8 PA14_OUT;
    __RW __u8 PA15_OUT;
    __RW __u8 PB0_OUT;
    __RW __u8 PB1_OUT;
    __RW __u8 PB2_OUT;
    __RW __u8 PB3_OUT;
    __RW __u8 PB4_OUT;
    __RW __u8 PB5_OUT;
    __RW __u8 PB6_OUT;
    __RW __u8 PB7_OUT;
    __RW __u8 PB8_OUT;
    __RW __u8 PB9_OUT;
    __RW __u8 PB10_OUT;
    __RW __u8 PB11_OUT;
    __RW __u8 PC0_OUT;
    __RW __u8 PC1_OUT;
    __RW __u8 PC2_OUT;
    __RW __u8 PC3_OUT;
    __RW __u8 PC4_OUT;
    __RW __u8 PC5_OUT;
    __RW __u8 PC6_OUT;
    __RW __u8 PC7_OUT;
    __RW __u8 PC8_OUT;
    __RW __u8 PG0_OUT;
    __RW __u8 PG1_OUT;
    __RW __u8 PG2_OUT;
    __RW __u8 PG3_OUT;
    __RW __u8 PG4_OUT;
    __RW __u8 PG5_OUT;
    __RW __u8 PG6_OUT;
    __RW __u8 PG7_OUT;
    __RW __u8 PG8_OUT;
    __RW __u8 USBDP_OUT;
    __RW __u8 USBDM_OUT;
    __RW __u8 PP0_OUT;
} JL_OMAP_TypeDef;

#define JL_OMAP_BASE      (ls_base + map_adr(0x58, 0x00))
#define JL_OMAP           ((JL_OMAP_TypeDef   *)JL_OMAP_BASE)
