/*
 *********************************************************************************************************
 *                                                AC51
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

#include "file_bs_deal.h"
#include "uart.h"
#include "dev_manager.h"

#define FILE_BS_OPT_DBG
#ifdef  FILE_BS_OPT_DBG

#define file_bs_puts     puts
#define file_bs_printf   printf

#else
#define file_bs_puts(...)
#define file_bs_printf(...)

#endif

/*
 *********************************************************************************************************
 *
 * Description: 在指定数据中,搜索key(byte)的位置
 *
 * Arguments  : buf - 指定数据起始地址, buf_len-长度, key-搜索的key,
 *              find_type -搜索方向:0-从前往后搜索,1-从后往前搜索
 *
 * Returns    : key的位置
 *
 * Note:
 *********************************************************************************************************
 */
//查找byte的位置
static int find_byte_pos(u8 *buf, u16 buf_len, u8 key, u8 find_type)
{
    u8 *data = buf;

    if (0 == find_type) {
        //从前往后找
        while (buf_len > 0) {
            if (*data == key) {
                return (int)(data - buf);
            }
            data++;
            buf_len--;
        }
    } else {
        //从后往前找
        data += (buf_len - 1);
        while (buf_len > 0) {
            if (*data == key) {
                return (int)(data - buf);
            }
            data--;
            buf_len--;
        }
    }
    return -1;
}


/*
 *********************************************************************************************************
 *
 * Description: 转化路径格式
 *
 * Arguments  :
 *
 * Returns    : 0-为成功，其他值 失败
 *
 * Note:
 *********************************************************************************************************
 */
int file_comm_change_file_path(char *dest, char *src)
{
    u8 start_flag = 0, space_flag, val;

    while (1) {
        val = *src;
        if (val == '\0') {
            *dest = '\0';
            break;
        }
        switch (val) {
        case '/':
            if (start_flag != 0) {
                space_flag = 0;
            }
            start_flag++;
            *dest = val;
            break;

        case ' ':
            space_flag = 1;
            break;

        default:
            if (space_flag != 0) {
                *dest = '.';
                dest++;
                space_flag = 0;
            }
            *dest = val;
            break;
        }

        dest++;
        src++;
    }

    return 0;
}

/*
 *********************************************************************************************************
 *
 * Description: 修正从fs取等的长名
 *
 * Arguments  : str,len
 *
 * Returns    : 字节长度(不包含结束符)
 *
 * Note:    过滤0xffff值，自动填入结束符
 *********************************************************************************************************
 */
int file_comm_long_name_fix(u8 *str, u16 len)
{
    u8 *src = str;
    while (len > 1) {
        if ((*str == 0xff) && (*(str + 1) == 0xff)) {
            break;
        }

        if ((*str == 0x0) && (*(str + 1) == 0x0)) {
            break;
        }

        str += 2;
        len -= 2;
    }
    *str = 0x00;
    *(str + 1) = 0x00;
    return (int)(str - src);
}

/*
 *********************************************************************************************************
 *
 * Description: 转换8+3名字显示
 *
 * Arguments  : dest,src
 *
 * Returns    : 转换后名字的长度(不包含结束符)
 *
 * Note:      转换后自动填入结束符
 *********************************************************************************************************
 */
int file_comm_display_83name(u8 *dest, u8 *src)
{
    u8 offset;

    for (offset = 8; offset > 0; offset--) {
        if (src[offset - 1] != 0x20) {
            break;
        }
    }

    memcpy(dest, src, offset);

    if (src[8] != 0x20) {
        dest[offset++] = '.';
        memcpy(dest + offset, src + 8, 3);
        offset += 3;
    }

    dest[offset++] =  '\0';

    return offset; //name 's len
}

/*
 *********************************************************************************************************
 *
 * Description: 获取显示的文件夹或文件名字
 *
 * Arguments  :tpath,disp_file_name,disp_dir_name
 *
 * Returns    :
 *
 * Note:      转换后 lfn_cnt 为0为短名，lfn_cnt不为0 则是长名
 *********************************************************************************************************
 */
void file_comm_change_display_name(char *tpath, LONG_FILE_NAME *disp_file_name, LONG_FILE_NAME *disp_dir_name)
{
    u16 len;
    int pos;

    //取文件名字显示

    if (disp_file_name != NULL) {
        if (disp_file_name->lfn_cnt != 0) {
            //long name
            file_bs_puts("file long name  \n");
            //printf_buf(disp_file_name->lfn, 16);
            disp_file_name->lfn_cnt = file_comm_long_name_fix((void *)disp_file_name->lfn, disp_file_name->lfn_cnt); //增加结束符
        } else {
            //short name
            file_bs_puts("file short name\n");
            len = strlen((void *)tpath); //增加结束符
            pos = find_byte_pos((void *)tpath, len, 0x2f, 1); //
            if (pos == -1) {
                strcpy(disp_file_name->lfn, "----");
            } else {
                memcpy((void *)&disp_file_name->lfn[32], tpath + pos + 1, 11);
                file_comm_display_83name((void *)disp_file_name->lfn, (void *)&disp_file_name->lfn[32]);
            }

            disp_file_name->lfn_cnt = 0;
        }

        //printf_buf(disp_file_name->lfn, 32);

    }


    //取文件夹名字显示
    if (disp_dir_name != NULL) {
        if (disp_dir_name->lfn_cnt != 0) {
            //long name
            file_bs_puts("folder long name \n");
            //printf_buf(disp_dir_name->lfn, 16);
            disp_dir_name->lfn_cnt = file_comm_long_name_fix((void *)&disp_dir_name->lfn, disp_dir_name->lfn_cnt); //增加结束符
        } else {
            //short name
            file_bs_puts("folder short name \n");
            len = strlen((void *)tpath); //增加结束符
            pos = find_byte_pos((void *)tpath, len, 0x2f, 1); //

            if (pos != -1) {
                pos = find_byte_pos((void *)tpath, pos, 0x2f, 1); //
            }

            if (pos == -1) {
                strcpy(disp_dir_name->lfn, "ROOT");
            } else {
                memcpy(&disp_dir_name->lfn[32], tpath + pos + 1, 11);
                file_comm_display_83name((void *)disp_dir_name->lfn, (void *)&disp_dir_name->lfn[32]);
            }
            disp_dir_name->lfn_cnt = 0;
        }

        //printf_buf(disp_dir_name->lfn, 32);
    }
}
/*
 *********************************************************************************************************
 *
 * Description: 打开文件浏览器
 *
 * Arguments  : fil_bs
 *
 * Returns    : dir的总数
 *
 * Note:
 *********************************************************************************************************
 */
//返回根目录dir总数
u32 file_bs_open_handle(FILE_BS_DEAL *fil_bs, u8 *ext_name)
{
    u32 total_dir;

    if (fil_bs == NULL) {
        puts("*open hdl is null\n");
        return 0;
    }

    file_bs_puts("open bs hdl\n");
    file_bs_printf("bs_mapi:0x%x\n", (int)fil_bs);

    if (ext_name == NULL) {
        ext_name = "MP3";//"WAVWMAMP3FLA";
    }

    printf("fil_bs->dev = %x\n", fil_bs->dev);
    printf("%s, %s, %s, %d\n]", dev_manager_get_root_path(fil_bs->dev), dev_manager_get_logo(fil_bs->dev), __FUNCTION__, __LINE__);
    fset_ext_type(dev_manager_get_root_path(fil_bs->dev), ext_name);
    printf("%s, %d\n]", __FUNCTION__, __LINE__);

    total_dir = fopen_dir_info(dev_manager_get_root_path(fil_bs->dev), &fil_bs->file, 0);
    printf("%s, %d\n]", __FUNCTION__, __LINE__);
    if (fil_bs->file == 0) {
        file_bs_puts("open bs fail\n");
        return 0;
    }
    file_bs_printf("open bs hdl_out,root_dir = 0x%x\n", total_dir);
    return total_dir;
}

/*
 *********************************************************************************************************
 *
 * Description: 关闭文件浏览器
 *
 * Arguments  : fil_bs
 *
 * Returns    :
 *
 * Note:
 *********************************************************************************************************
 */
void file_bs_close_handle(FILE_BS_DEAL *fil_bs)
{
    file_bs_puts("close bs hdl\n");
    if (fil_bs->file) {
        fclose(fil_bs->file);
        fil_bs->file = NULL;
    }
    file_bs_puts("close bs hdl_out\n");
}

/*
 *********************************************************************************************************
 *
 * Description: 进入文件夹目录
 *
 * Arguments  : fil_bs,dir_info
 *
 * Returns    : dir的总数
 *
 * Note:
 *********************************************************************************************************
 */
u32 file_bs_entern_dir(FILE_BS_DEAL *fil_bs, FS_DIR_INFO *dir_info)
{
    u32 total_dir;
    file_bs_printf("bs_enter dir\n");
    total_dir = fenter_dir_info(fil_bs->file, dir_info); //使用open获得的file，无需重新申请。
    file_bs_printf("bs_enter_out,t = 0x%x\n", total_dir);
    return total_dir;
}

/*
 *********************************************************************************************************
 *
 * Description: 返回上传目录
 *
 * Arguments  : fil_bs
 *
 * Returns    : dir的总数
 *
 * Note:
 *********************************************************************************************************
 */
u32 file_bs_exit_dir(FILE_BS_DEAL *fil_bs)
{
    u32 total_dir;
    file_bs_printf("bs_exit dir\n");
    total_dir = fexit_dir_info(fil_bs->file);//
    file_bs_printf("bs_exit_out,total = 0x%x\n", total_dir);
    return total_dir;
}

/*
 *********************************************************************************************************
 *
 * Description: 获取指定位置的dir信息
 *
 * Arguments  : fil_bs,buf--被填入的buf,start_sn -获取的其实位置（从1开始）,get_cnt--获取dir的个数
 *
 * Returns    : 真正获取到的dir总数
 *
 * Note:
 *********************************************************************************************************
 */
u32 file_bs_get_dir_info(FILE_BS_DEAL *fil_bs, FS_DIR_INFO *buf, u16 start_sn, u16 get_cnt)
{
    u32 real_dir;
    /* file_bs_printf("bs_get dir_info\n"); */
    real_dir =  fget_dir_info(fil_bs->file, start_sn, get_cnt, buf);
    /* file_bs_printf("bs_get_out,t = 0x%x\n", real_dir); */
    return real_dir;
}


//////////////////////////////////////////////////////////////////////////
// test

#if 0

#define BS_GET_DIR_NUM		3
void file_bs_test(void)
{
    printf("\n\n\n\n %s,%d, ", __func__, __LINE__);
    FS_DIR_INFO *dir_buf = zalloc(sizeof(FS_DIR_INFO) * BS_GET_DIR_NUM);
    FILE_BS_DEAL *fil_bs = zalloc(sizeof(FILE_BS_DEAL));
    if (!fil_bs || !dir_buf) {
        printf("%s,%d, ", __func__, __LINE__);
        printf("malloc err");
        while (1);
    }
    fil_bs->dev = storage_dev_last();
    if (!fil_bs->dev) {
        printf("%s,%d, ", __func__, __LINE__);
        printf("have no dev");
        while (1);
    }

    /* void *fmnt = mount(fil_bs->dev->dev_name, fil_bs->dev->storage_path, fil_bs->dev->fs_type, 3, NULL); */
    /* if (!fmnt) { */
    /* printf("%s,%d, ", __func__, __LINE__); */
    /* printf("mount err"); */
    /* while(1); */
    /* } */

    int i, j, cnt, total;
    file_bs_open_handle(fil_bs, "MP3");//open完后需要close
    /* printf("\n root dir num: %d ", total); */
    /* if (!total) { */
    /*     printf("%s,%d, ", __func__, __LINE__); */
    /*     printf("total dir is zero"); */
    /*     goto __exit; */
    /* } */

    total = file_bs_entern_dir(fil_bs, dir_buf);
    /* printf("\n %s,%d, ", __func__, __LINE__); */
    /* printf("dir num: %d ", total); */
    if (!total) {
        printf("%s,%d, ", __func__, __LINE__);
        printf("total dir is zero");
        goto __exit;
    }

    u8 flag = 0;
    for (i = 1; i <= total; i += BS_GET_DIR_NUM) {
__again:
        cnt = file_bs_get_dir_info(fil_bs, dir_buf, i, BS_GET_DIR_NUM);
        /* printf("\n cnt:%d ", cnt); */
        for (j = 0; j < cnt; j++) {
            /* printf("j:%d, dirtype:%d, fntype:%d, sclust:0x%x ", i + j, dir_buf[j].dir_type, dir_buf[j].fn_type, dir_buf[j].sclust); */
            if (dir_buf[j].dir_type == BS_DIR_TYPE_FORLDER) {
                if (flag < 2) {
                    flag ++;
                    if (flag == 2) {
                        int sub_i, sub_j, sub_cnt, sub_total;
                        sub_total = file_bs_entern_dir(fil_bs, &dir_buf[j]);
                        /* printf("\n %s,%d, ", __func__, __LINE__); */
                        /* printf("dir num: %d ", sub_total); */
                        if (sub_total) {
                            for (sub_i = 1; sub_i <= sub_total; sub_i += BS_GET_DIR_NUM) {
                                sub_cnt = file_bs_get_dir_info(fil_bs, dir_buf, sub_i, BS_GET_DIR_NUM);
                                /* printf("sub_cnt:%d ", sub_cnt); */
                                for (sub_j = 0; sub_j < sub_cnt; sub_j++) {
                                    /* printf("sub_j:%d, dirtype:%d, fntype:%d, sclust:0x%x ", sub_i + sub_j, dir_buf[sub_j].dir_type, dir_buf[sub_j].fn_type, dir_buf[sub_j].sclust); */
                                }
                            }
                        }
                        file_bs_exit_dir(fil_bs);
                        goto __again;
                    }
                }
            }
        }
    }

__exit:
    /* printf("\n %s,%d, \n ", __func__, __LINE__); */

    file_bs_close_handle(fil_bs);

    /* unmount(fil_bs->dev->storage_path); */

    free(dir_buf);
    free(fil_bs);
}
#endif

