#ifndef __POWER_PORT_H__
#define __POWER_PORT_H__

#define NO_CONFIG_PORT						(-1)
enum {
    PORTA_GROUP = 0,
    PORTB_GROUP,
    PORTC_GROUP,
    PORTD_GROUP,
    PORTE_GROUP,
    PORTG_GROUP,
    PORTP_GROUP,
};

struct gpio_value {
    u16 gpioa;
    u16 gpiob;
    u16 gpioc;
    u16 gpiod;
    u16 gpioe;
    u16 gpiog;
    u16 gpiop;
    u16  gpiousb;
};

#define     _PORT(p)            JL_PORT##p
#define     SPI_PORT(p)         _PORT(p)



// | func\port |  A   |  B   |  C   |
// |-----------|------|------|------|
// | CS        | PD3  | PG4  | PC3  |
// | CLK       | PD0  | PD0  | PC1  |
// | DO(D0)    | PD1  | PD1  | PC2  |
// | DI(D1)    | PD2  | PG3  | PC4  |
// | WP(D2)    | PD5  | PG2  | PC5  |
// | HOLD(D3)  | PD6  | PD6  | PC0  |

#define     PORT_SPI0_PWRA      D
#define     SPI0_PWRA           4

#define     PORT_SPI0_CSA       D
#define     SPI0_CSA            3

#define     PORT_SPI0_CLKA      D
#define     SPI0_CLKA           0

#define     PORT_SPI0_DOA       D
#define     SPI0_DOA            1

#define     PORT_SPI0_DIA       D
#define     SPI0_DIA            2

#define     PORT_SPI0_D2A       D
#define     SPI0_D2A         	5

#define     PORT_SPI0_D3A       D
#define     SPI0_D3A            6

#define		SPI0_PWR_A		IO_PORTD_04
#define		SPI0_CS_A		IO_PORTD_03
#define 	SPI0_CLK_A		IO_PORTD_00
#define 	SPI0_DO_D0_A	IO_PORTD_01
#define 	SPI0_DI_D1_A	IO_PORTD_02
#define 	SPI0_WP_D2_A	IO_PORTD_05
#define 	SPI0_HOLD_D3_A	IO_PORTD_06


////////////////////////////////////////////////////////////////////////////////
#define     PORT_SPI0_PWRB      D
#define     SPI0_PWRB           4

#define     PORT_SPI0_CSB       G
#define     SPI0_CSB            4

#define     PORT_SPI0_CLKB      D
#define     SPI0_CLKB           0

#define     PORT_SPI0_DOB       D
#define     SPI0_DOB            1

#define     PORT_SPI0_DIB       G
#define     SPI0_DIB            3

#define     PORT_SPI0_D2B       G
#define     SPI0_D2B   		 	2

#define     PORT_SPI0_D3B       D
#define     SPI0_D3B            6

#define		SPI0_PWR_B		IO_PORTD_04
#define		SPI0_CS_B		IO_PORTG_04
#define 	SPI0_CLK_B		IO_PORTD_00
#define 	SPI0_DO_D0_B	IO_PORTD_01
#define 	SPI0_DI_D1_B	IO_PORTG_03
#define 	SPI0_WP_D2_B	IO_PORTG_02
#define 	SPI0_HOLD_D3_B	IO_PORTD_06

////////////////////////////////////////////////////////////////////////////////
#define     PORT_SPI0_PWRC      C
#define     SPI0_PWRC           8

#define     PORT_SPI0_CSC       C
#define     SPI0_CSC            3

#define     PORT_SPI0_CLKC      C
#define     SPI0_CLKC           1

#define     PORT_SPI0_DOC       C
#define     SPI0_DOC            2

#define     PORT_SPI0_DIC       C
#define     SPI0_DIC            4

#define     PORT_SPI0_D2C       C
#define     SPI0_D2C   		 	5

#define     PORT_SPI0_D3C       C
#define     SPI0_D3C            0

#define		SPI0_PWR_C			IO_PORTC_08
#define		SPI0_CS_C			IO_PORTC_03
#define 	SPI0_CLK_C			IO_PORTC_01
#define 	SPI0_DO_D0_C		IO_PORTC_02
#define 	SPI0_DI_D1_C		IO_PORTC_04
#define 	SPI0_WP_D2_C		IO_PORTC_05
#define 	SPI0_HOLD_D3_C		IO_PORTC_00

////////////////////////////////////////////////////////////////////////

#define		PSRAM_D0A		IO_PORTE_00
#define		PSRAM_D1A		IO_PORTE_01
#define		PSRAM_D2A		IO_PORTE_02
#define		PSRAM_D3A		IO_PORTE_05

u32 get_sfc_port(void);
u8 get_sfc_bit_mode();
u8 get_sfc1_bit_mode();
void port_init(void);
void port_protect(u16 *port_group, u32 port_num);

u8 WSIG_to_PANA(u8 wsig);
u8 PANA_to_WSIG(u8 iomap);

void soff_gpio_protect(u32 gpio);
void board_set_soft_poweroff_common(void *priv);

void sleep_gpio_protect(u32 gpio);
void sleep_enter_callback_common(void *priv);
void sleep_exit_callback_common(void *priv);

#endif

