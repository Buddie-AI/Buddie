/***********************************Jieli tech************************************************
  File : nor_fs.h
  By   : Huxi
  Email: xi_hu@zh-jieli.com
  date : 2016-11-30 14:30
********************************************************************************************/
#ifndef _NOR_FS_H_
#define _NOR_FS_H_

// #include "sdk_cfg.h"
#include "typedef.h"
// #include "system/includes.h"

// #define SPI_REC_EN  1

#define NORFS_DATA_LEN  16

#define REC_FILE_END 0xFE


//文件索引
typedef struct __RECF_INDEX_INFO {
    u16 index;  //文件索引号
    u16 sector; //文件所在扇区
} RECF_INDEX_INFO ;

#define FLASH_PAGE_SIZE 256
//文件系统句柄
typedef struct __RECFILESYSTEM {
    RECF_INDEX_INFO index;
    u8 buf[FLASH_PAGE_SIZE];
    u16 total_file;
    u16 first_sector;
    u16 last_sector;
    // u8 *buf;
    u8 sector_size;
    void (*eraser)(u32 address);
    s32(*read)(u8 *buf, u32 addr, u32 len);
    s32(*write)(u8 *buf, u32 addr, u32 len);
} RECFILESYSTEM, *PRECFILESYSTEM ;



//文件句柄
typedef struct __REC_FILE {
    RECF_INDEX_INFO index;
    RECFILESYSTEM *pfs;
    u32 addr;
    char priv_data[NORFS_DATA_LEN];
    u32 len;
    u32 w_len;
    u32 rw_p;
    u16 sr;
} REC_FILE;

enum {
    NOR_FS_SEEK_SET = 0,
    NOR_FS_SEEK_CUR = 0x01
};
// enum {
//     NOR_FS_SEEK_SET = 0x01,
//     NOR_FS_SEEK_CUR = 0x02
// };


typedef struct __nor_fs_hdl {
    u16 index;
    RECFILESYSTEM *recfs;
    REC_FILE *recfile;
} NOR_FS_HDL;


u8 recf_seek(REC_FILE *pfile, u8 type, int offsize);
u16 recf_read(REC_FILE *pfile, u8 *buff, u16 btr);
u16 recf_write(REC_FILE *pfile, u8 *buff, u16 btw);
u32 create_recfile(RECFILESYSTEM *pfs, REC_FILE *pfile);
u32 close_recfile(REC_FILE *pfile);
u32 open_recfile(u32 index, RECFILESYSTEM *pfs, REC_FILE *pfile);
void recf_save_sr(REC_FILE *pfile, u16 sr);

int music_flash_file_set_index(u8 file_sel, u32 index);
u32 recfs_scan(RECFILESYSTEM *pfs);
void init_nor_fs(RECFILESYSTEM *pfs, u16 sector_start, u16 sector_end, u8 sector_size);

u32 nor_fs_init(void);
int nor_fs_set_rec_capacity(int capacity); //需要先设置容量。
int nor_fs_ops_init(void);
int recfs_scan_ex();
u32 nor_get_capacity(void);
u32 flashinsize_rec_get_capacity(void);
int sdfile_rec_scan_ex();
void rec_clear_norfs_fileindex(void);
void clear_norfs_fileindex(void);
u32 _sdfile_rec_init(void);
int set_rec_capacity(int capacity); //需要先设置容量。
int sdfile_rec_ops_init(void);
u32 nor_get_index(void);
u32 flashinsize_rec_get_index(void);
int nor_set_offset_addr(int offset);

#endif
