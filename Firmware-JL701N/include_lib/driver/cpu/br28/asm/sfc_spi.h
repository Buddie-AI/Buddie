#ifndef	_SFC1_INTERFACE_H_
#define _SFC1_INTERFACE_H_
#include "typedef.h"
#include "generic/ioctl.h"

enum SFC_DATA_WIDTH {
    SFC_DATA_WIDTH_2 = 2,
    SFC_DATA_WIDTH_4 = 4,
};

enum SFC_READ_MODE {
    SFC_RD_OUTPUT = 0,
    SFC_RD_IO,
    SFC_RD_IO_CONTINUE,
};

struct sfc_spi_platform_data {
    u8 spi_hw_index;
    enum SFC_DATA_WIDTH sfc_data_width;
    enum SFC_READ_MODE sfc_read_mode;
    u8 sfc_encry; //是否加密
    u16 sfc_clk_div; //时钟分频: sfc_fre = sys_clk / div;
    u32 unencry_start_addr;         //不加密起始地址
    u32 unencry_size;               //不加密大小
};

#define SFC_SPI_PLATFORM_DATA_BEGIN(data) \
		const struct sfc_spi_platform_data data = {

#define SFC_SPI_PLATFORM_DATA_END()  \
};


//sfc1 API:
int sfc_spi_init(struct sfc_spi_platform_data *sfc_spi_data);
int sfc_spi_open(void *sfc_spi_data);
int sfc_spi_close(void);
u32 sfc_spi_read_id(void);
int sfc_spi_read(u32 addr, void *buf, u32 len);
int sfc_spi_write_pages(u32 addr, void *buf, u32 len);
int sfc_spi_eraser(u32 cmd, u32 addr);
u32 sfc1_flash_addr2cpu_addr(u32 offset);
u32 sfc1_cpu_addr2flash_addr(u32 offset);

void sfc_suspend(u32 enable_spi);
void sfc1_suspend();
#endif /* #ifndef	_SFC1_INTERFACE_H_ */
