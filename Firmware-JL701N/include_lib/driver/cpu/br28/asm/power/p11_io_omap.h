

//===============================================================================//
//
//      output function define
//
//===============================================================================//
#define P11_FO_GP_OCH0        ((0 << 2)|BIT(1))
#define P11_FO_GP_OCH1        ((1 << 2)|BIT(1))
#define P11_FO_GP_OCH2        ((2 << 2)|BIT(1))
#define P11_FO_UART0_TX        ((3 << 2)|BIT(1)|BIT(0))
#define P11_FO_UART1_TX        ((4 << 2)|BIT(1)|BIT(0))
#define P11_FO_SPI_CLK        ((5 << 2)|BIT(1)|BIT(0))
#define P11_FO_SPI_DO         ((6 << 2)|BIT(1)|BIT(0))
#define P11_FO_IIC_SCL        ((7 << 2)|BIT(1)|BIT(0))
#define P11_FO_IIC_SDA        ((8 << 2)|BIT(1)|BIT(0))
#define P11_FO_DMIC_CLK        ((9 << 2)|BIT(1)|BIT(0))

//===============================================================================//
//
//      IO output select sfr
//
//===============================================================================//
typedef struct {
    __RW __u8 P11_PB0_OUT;
    __RW __u8 P11_PB1_OUT;
    __RW __u8 P11_PB2_OUT;
    __RW __u8 P11_PB3_OUT;
    __RW __u8 P11_PB4_OUT;
    __RW __u8 P11_PB5_OUT;
    __RW __u8 P11_PB6_OUT;
    __RW __u8 P11_PB7_OUT;
    __RW __u8 P11_PB8_OUT;
    __RW __u8 P11_PB9_OUT;
    __RW __u8 P11_PB10_OUT;
    __RW __u8 P11_PB11_OUT;
} P11_OMAP_TypeDef;

#define P11_OMAP_BASE      (p11_sfr_base + map_adr(0x15, 0x00))
#define P11_OMAP           ((P11_OMAP_TypeDef   *)P11_OMAP_BASE)
