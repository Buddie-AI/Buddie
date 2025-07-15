#ifndef __FS_FILE_NAME_H__
#define __FS_FILE_NAME_H__

#include "generic/typedef.h"

#define D_LFN_MAX_SIZE 		512

#define LFN_MAX_SIZE 	D_LFN_MAX_SIZE   	//不能超过512		//必须4byte对齐

typedef struct _LONG_FILE_NAME {
    u16     lfn_cnt;
    char    lfn[LFN_MAX_SIZE];             //长文件名buffer
} LONG_FILE_NAME; //整理后的长文件名

typedef struct _FS_DIR_INFO {
    u32 sclust;					//dir sclust
    u16 dir_type;            	// 0-folder,1-file
    u16 fn_type;            	// 0-sfn,1-lfn
    LONG_FILE_NAME lfn_buf;     //long file name
} FS_DIR_INFO;

typedef struct _FS_DISP_INFO {
    char tpath[128];
    LONG_FILE_NAME file_name;
    LONG_FILE_NAME dir_name;
    u8 update_flag;
} FS_DISP_INFO;

typedef struct _FLASH_FAT_ADDRINFO {
    u32 saddr; //当前连续簇块文件起始实际物理地址
    u32 end_addr; //当前连续簇块结束位置
    u32 file_len; //文件长度,如 JL.res文件的
} FLASH_FAT_ADDRINFO;
#endif  /* __FS_FILE_NAME_H__ */

