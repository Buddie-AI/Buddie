#ifndef SDMMC_MODULE_H
#define SDMMC_MODULE_H


#include "generic/typedef.h"
#include "generic/ioctl.h"


#define     SD_CMD_DECT 	0
#define     SD_CLK_DECT  	1
#define     SD_IO_DECT 		2

#define    SD_CLASS_0      0
#define    SD_CLASS_2      1
#define    SD_CLASS_4      2
#define    SD_CLASS_6      3
#define    SD_CLASS_10     4

#define SD_IOCTL_GET_CLASS  _IOR('S', 0, 4)


#define SD_IOCTL_SUPPORT_ERASE_OPERAT   1

typedef enum {
    SDMMC_NORMAL_ERASE,
    SDMMC_TRIM_ERASE,
    SDMMC_DISCARD,
    SDMMC_SECURE_ERASE,
    SDMMC_SECURE_TRIM_STEP_1,
    SDMMC_SECURE_TRIM_STEP_2,
} sdmmc_erase_type;

struct sdmmc_erase_arg_t {
    sdmmc_erase_type erase_type;
    u32 block_addr;
    u32 block_quantity;
};



#endif

