/*
 *********************************************************************************************************
 *                                                AC46
 *                                      fs browser select
 *                                             CODE
 *
 *                          (c) Copyright 2015-2016, ZHUHAI JIELI
 *                                           All Rights Reserved
 *
 * File : *
 * By   : jamin.li
 * DATE : 2015-10-15 build this file
 *********************************************************************************************************
 */

#ifndef _FILE_BS_DEAL_H_
#define _FILE_BS_DEAL_H_

#include "fs/fs.h"
#include "storage_dev/storage_dev.h"

#define BS_DIR_TYPE_FORLDER   	0
#define BS_DIR_TYPE_FILE   		1

#define BS_FNAME_TYPE_SHORT   	0
#define BS_FNAME_TYPE_LONG   	1

typedef struct _FILE_BS_DEAL_ {
    struct __dev *dev;
    FILE *file;
} FILE_BS_DEAL;


extern int file_comm_change_file_path(char *dest, char *src);
extern int file_comm_long_name_fix(u8 *str, u16 len);
extern int file_comm_display_83name(u8 *dest, u8 *src);

extern u32 file_bs_open_handle(FILE_BS_DEAL *fil_bs, u8 *ext_name);
extern void file_bs_close_handle(FILE_BS_DEAL *fil_bs);
extern u32 file_bs_entern_dir(FILE_BS_DEAL *fil_bs, FS_DIR_INFO *dir_info);
extern u32 file_bs_exit_dir(FILE_BS_DEAL *fil_bs);
extern u32 file_bs_get_dir_info(FILE_BS_DEAL *fil_bs, FS_DIR_INFO *buf, u16 start_sn, u16 get_cnt);

extern void file_comm_change_display_name(char *tpath, LONG_FILE_NAME *disp_file_name, LONG_FILE_NAME *disp_dir_name);

#endif/*_FILE_BS_DEAL_H_*/

