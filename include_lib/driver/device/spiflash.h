#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__



#include "typedef.h"
#include "generic/list.h"
#include "generic/ioctl.h"
#include "device/device.h"
#include "system/task.h"


struct spi_device;

enum spiflash_bit_mode {
    SPI_2WIRE_MODE,
    SPI_ODD_MODE,
    SPI_DUAL_MODE,
    SPI_QUAD_MODE,
};

enum spiflash_read_mode {
    FAST_READ_OUTPUT_MODE,
    FAST_READ_IO_MODE,
    FAST_READ_IO_CONTINUOUS_READ_MODE,
};

enum sfc_run_mode {
    //1bit mode
    SFC_READ_DATA_MODE = (1 << 0),
    SFC_FAST_READ_MODE = (1 << 1),
    //2bit mode
    SFC_FAST_READ_DUAL_IO_NORMAL_READ_MODE  = (1 << 2),
    SFC_FAST_READ_DUAL_IO_CONTINUOUS_READ_MODE = (1 << 3),
    SFC_FAST_READ_DUAL_OUTPUT_MODE     = (1 << 4),
    //4bit mode
    SFC_FAST_READ_QUAD_IO_NORMAL_READ_MODE   = (1 << 5),
    SFC_FAST_READ_QUAD_IO_CONTINUOUS_READ_MODE = (1 << 6),
    SFC_FAST_READ_QUAD_OUTPUT_MODE     = (1 << 7),

};
struct spi_ops {
    int (*set_cs)(int);
    int (*init)(void *);
    u8(*read_byte)(int *err);
    int (*read)(u8 *, u32 len, u8 mode);
    int (*write_byte)(u8 cmd);
    int (*write_cmd)(u8 *cmd, u32 len);
    int (*write)(u8 *, u32 len);
    u8(*get_bit_mode)();
};


struct sf_info {
    u32 id;
    u16 page_size;     //byte
    u16 block_size;   //KByte
    u32 chip_size;   //KByte
};

enum sf_erase_type {
    SF_SECTOR_ERASE,
    SF_BLOCK_ERASE,
    SF_CHIP_ERASE,
};

//struct sf_erase {
//enum sf_erase_type type;
//u32 addr;
//};


//struct sf_wp {
//u8 enable;
//u8 cmd;
//};


struct spi_device {
    const char *name;
    const struct spi_ops *ops;
};

struct spiflash_platform_data {
    const char *name;
    enum spiflash_read_mode mode;
    enum sfc_run_mode  sfc_run_mode;
    void *private_data;
};


struct spiflash {
    struct list_head entry;
    void *device;
    struct device dev;
    struct sf_info info;
    const struct spiflash_platform_data *pd;
    const char *name;
    OS_MUTEX mutext;
    u8 inited;
    u8 read_mode;
    u8 read_cmd_mode;
    u8 write_cmd_mode;
    u8 continuous_read_mode;
};


#define REGISTER_SPIFLASH_DEVICE(dev) \
	static const struct spi_device dev sec(.spi_device)


extern struct spi_device spi_device_begin[];
extern struct spi_device spi_device_end[];




extern struct spiflash *__get_spiflash(const char *name);





#endif

