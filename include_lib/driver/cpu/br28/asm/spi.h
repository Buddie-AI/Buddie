#ifndef	_SPI_INTERFACE_H_
#define _SPI_INTERFACE_H_
#include "typedef.h"
#include "generic/ioctl.h"


/*enum spi_mode {
    SPI_2WIRE_MODE,
    SPI_ODD_MODE,
    SPI_DUAL_MODE,
    SPI_QUAD_MODE,
};*/
enum spi_mode {
    SPI_MODE_BIDIR_1BIT,    //支持SPIx(x=0,1,2)，全双工，di接收，do发送
    SPI_MODE_UNIDIR_1BIT,   //支持SPIx(x=0,1,2)，半双工，do分时发送/接收
    SPI_MODE_UNIDIR_2BIT,   //支持SPIx(x=0)，半双工，di & do共2bit分时发送/接收
    SPI_MODE_UNIDIR_4BIT,   //支持SPIx(x=0)，半双工，di & do & wp & hold 共4bit分时发送/接收
};

enum {
    SPI0,
    SPI1,
    SPI2,
    SPI_MAX_HW_NUM,
};

#define SPI_MAX_IO_GROUP            2

struct spi_io {
    u8 cs_pin;
    u8 di_pin;
    u8 do_pin;
    u8 clk_pin;
    u8 d2_pin;
    u8 d3_pin;
};

struct spi_io_mapping {
    u32 num;                                //可选端口数量
    struct spi_io io[SPI_MAX_IO_GROUP];
};

enum spi_role {
    SPI_ROLE_MASTER,
    SPI_ROLE_SLAVE,
};

struct spi_platform_data {
    u8 port[5];//CLK, DO, DI D2(wp) D3(hold)
    u8 mode;  //模式，选项为enum spi_mode中的枚举常量
    u8 role;  //角色，选项为enum spi_role中的枚举常量
    u32 clk;  //波特率
};

extern const struct spi_platform_data spi1_p_data;
extern const struct spi_platform_data spi2_p_data;

typedef const int spi_dev;

typedef struct {
    spi_dev spi_hdl;
    u8 spi_cs;
}spi_handle_t;



int spi_open(spi_dev spi);
int spi_dma_recv(spi_dev spi, void *buf, u32 len);
int spi_dma_send(spi_dev spi, const void *buf, u32 len);
void spi_dma_set_addr_for_isr(spi_dev spi, void *buf, u32 len, u8 rw);
void spi_set_ie(spi_dev spi, u8 en);
u8 spi_get_pending(spi_dev spi);
void spi_clear_pending(spi_dev spi);
void spi_set_bit_mode(spi_dev spi, int mode);
u8 spi_recv_byte(spi_dev spi, int *err);
u8 spi_recv_byte_for_isr(spi_dev spi);
int spi_send_byte(spi_dev spi, u8 byte);
void spi_send_byte_for_isr(spi_dev spi, u8 byte);
u8 spi_send_recv_byte(spi_dev spi, u8 byte, int *err);
int spi_set_baud(spi_dev spi, u32 baud);
u32 spi_get_baud(spi_dev spi);
void spi_close(spi_dev spi);

#endif
