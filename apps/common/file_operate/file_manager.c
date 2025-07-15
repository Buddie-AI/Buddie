#include "file_manager.h"
#include "app_config.h"

#define MODE_FIX 0
#if MODE_FIX
static int file_manager_mode_deal(struct vfscan *fs, int sel_mode, int *arg)
{

    int err;
    int fnum = 0;
    int file_start = 1;
    int file_end = fs->file_number;

    if (file_end == 0) {
        return -ENOENT;
    }
    struct ffolder folder = {0};
    fget_folder(fs, &folder);
    /* if ((scan->cycle_mode == FCYCLE_FOLDER) && (scan->ff_api.fileTotalInDir) */
    /*     && ((scan->ff_api.fileTotalOutDir + scan->ff_api.fileTotalInDir) <= scan->ff_api.totalFileNumber) */
    /*    ) { */

    if ((fs->cycle_mode == FCYCLE_FOLDER) && (folder.fileTotal)
        && ((folder.fileStart + folder.fileTotal - 1) <= file_end)
       ) {
        file_start = folder.fileStart;
        file_end = folder.fileStart + folder.fileTotal - 1;
    }
    switch (sel_mode) {
    case FSEL_LAST_FILE:
        fnum = fs->file_number;
        break;
    case FSEL_FIRST_FILE:
        fnum = 1;
        break;
    case FSEL_AUTO_FILE:
        /* if (scan->ff_api.fileNumber == 0) { */
        /*     return -EINVAL; */
        /* } */
        if (fs->cycle_mode == FCYCLE_ONE) {
            /* fnum = scan->ff_api.fileNumber; */
            fnum = fs->file_counter;
            break;
        }
    case FSEL_NEXT_FILE:
        /* if (scan->ff_api.fileNumber == 0) { */
        /*     return -EINVAL; */
        /* } */
        if (fs->cycle_mode == FCYCLE_RANDOM) {
            fnum = rand32() % (file_end - file_start + 1) + file_start;
            if (fnum == fs->file_counter) {
                fnum = fs->file_counter + 1;
            }
        } else {
            fnum = fs->file_counter + 1;
        }
        /* if (fnum > scan->last_file_number) { */
        if (fnum > fs->file_number) {
            if (fs->cycle_mode == FCYCLE_LIST) {
                return -ENOENT;
            } else if (fs->cycle_mode == FCYCLE_FOLDER) {
                fnum = 	file_start;
            } else {
                fnum = 1;
            }
        }
        if (fnum > file_end) {
            fnum = file_start;
        } else if (fnum < file_start) {
            fnum = file_end;
        }
        break;
    case FSEL_PREV_FILE:
        /* if (scan->ff_api.fileNumber == 0) { */
        /*     return -EINVAL; */
        /* } */
        if (fs->cycle_mode == FCYCLE_RANDOM) {
            fnum = rand32() % (file_end - file_start + 1) + file_start;
            if (fnum == fs->file_counter) {
                fnum = fs->file_counter + 1;
            }
        } else {
            fnum = fs->file_counter - 1;
        }
        /* if ((scan->ff_api.fileNumber | BIT(15)) != scan->cur_file_number) { */
        /*     fnum -= scan->last_file_number - scan->ff_api.totalFileNumber; */
        /* } */
        /* scan->last_file_number = scan->ff_api.totalFileNumber; */
        if (fs->cycle_mode == FCYCLE_LIST) {
            break;
        }
        if (fnum > file_end) {
            fnum = file_start;
        } else if (fnum < file_start) {
            fnum = file_end;
        }
        break;

    case FSEL_NEXT_FOLDER_FILE:
        /* fnum = scan->ff_api.fileTotalOutDir + scan->ff_api.fileTotalInDir + 1; */
        fnum = folder.fileStart + folder.fileTotal;
        if (fnum > fs->file_number) {
            fnum = 1;
        }
        break;
    case FSEL_PREV_FOLDER_FILE:
        /* if ((scan->ff_api.fileTotalOutDir + 1) > 1) { */
        if (folder.fileStart > 1) {
            fnum = folder.fileStart - 1;
        } else {
            fnum = fs->file_number;
        }
        break;
    default:
        return -EINVAL;
    }

    if ((sel_mode != FSEL_NEXT_FOLDER_FILE) && (sel_mode != FSEL_PREV_FOLDER_FILE)) {
        if (fnum < file_start) {
            fnum = file_start;
        } else if (fnum > file_end) {
            fnum = file_end;
        }
    }
    *arg = fnum;
    return 0;
}
#endif

FILE *file_manager_select(struct __dev *dev, struct vfscan *fs, int sel_mode, int arg, struct __scan_callback *callback)
{
    FILE *_file = NULL;
    //clock_add_set(SCAN_DISK_CLK);
    if (callback && callback->enter) {
        callback->enter(dev);//扫描前处理， 可以在注册的回调里提高系统时钟等处理
    }
#if MODE_FIX
    if ((sel_mode != FSEL_BY_SCLUST) && (sel_mode != FSEL_BY_PATH)) {
        if (file_manager_mode_deal(fs, sel_mode, &arg)) {
            return NULL;
        }
        sel_mode = FSEL_BY_NUMBER;
    }
#endif
    _file = fselect(fs, sel_mode, arg);
    //clock_remove_set(SCAN_DISK_CLK);
    if (callback && callback->exit) {
        callback->exit(dev);//扫描后处理， 可以在注册的回调里还原到enter前的状态
    }
    return _file;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief    文件删除统一处理
 *
 * @param path 扫描路径名
 * @param param 配置参数
 * @param dir_flag 是否删除文件夹标志
 * @note 加速处理:(删除文件的时候使用)从前往后依次删除. 文件夹必须从后往前删。
 *
 * @return  0成功， 其他失败
 */
/* ----------------------------------------------------------------------------*/
int file_manager_delete_deal(char *path, char *param, u8 dir_flag)
{
    u16 folder_total_file = 0;
    int d_err = 0;
    struct vfscan *fsn = NULL;
    FILE *d_f = NULL;

    fsn = fscan(path, param, 9);
    if (fsn == NULL) {
        r_printf(">>>[test]:err!!!!!! fsacn fsn fail\n");
        return 1;
    }
    folder_total_file = fsn->file_number;
    y_printf(">>>[test]:total = %d\n", folder_total_file);
    for (int i = folder_total_file; i >= 1; i--) {
        if (!dir_flag) {
            d_f = fselect(fsn, FSEL_BY_NUMBER, folder_total_file - i + 1); //加速处理，不用找到最后一个文件。
        } else {
            d_f = fselect(fsn, FSEL_BY_NUMBER, i);
        }
        if (d_f == NULL) {
            r_printf(">>>[test]:err!! select file err\n");
            return 1;
        }
        putchar('D');
        d_err = fdelete(d_f);
        if (d_err) {
            r_printf(">>>[test]:err!! delete file err\n");
            return 1;
        }
        d_f = NULL;
    }
    return 0;
}


/*----------------------------------------------------------------------------*/
/** @brief:文件夹删除处理
    @param:dev_logo :设备logo  folder:文件夹路径(短名) folder_len:文件夹路径长度
    @return:
    @author:phewlee
    @note:
    @date: 2021-05-21,10:16
*/
/*----------------------------------------------------------------------------*/
int file_manager_delete_dir(char *dev_logo, char *folder, u16 folder_len)
{
    int err = 0;
    struct __dev *dev;
    char path[128] = {0};

    static const u8 delete_file_param[] = "-t"
                                          "ALL"
                                          " -sn -r";

    static const u8 delete_folder_param[] = "-t"
                                            "ALL"
                                            " -sn -d -r";

    dev = dev_manager_find_spec(dev_logo, 0);
    if (dev == NULL) {
        r_printf(">>>[test]:errr!!!!!!!!! not find dev\n");
        return 1;
    }
    char *root_path = dev_manager_get_root_path(dev);
    memcpy(path, root_path, strlen(root_path));
    memcpy(path + strlen(root_path), folder, folder_len);
    r_printf(">>>[test]:path = %s\n", path);
    err = file_manager_delete_deal(path, delete_file_param, 0);
    if (err) {
        r_printf(">>>[test]:errr!!!!!!!!! delete file deal fail\n");
        return 1;
    }
    err = file_manager_delete_deal(path, delete_folder_param, 1);
    if (err) {
        r_printf(">>>[test]:errr!!!!!!!!! delete folder  deal fail\n");
        return 1;
    }
    FILE *folder_f = fopen(path, "r");
    if (folder_f == NULL) {
        r_printf(">>>[test]:err open folder\n");
        return 1;
    }
    err = fdelete(folder_f);
    return err;
}

/*----------------------------------------------------------------------------*/
/** @brief:文件插入处理
    @param:d_f: 源文件句柄， i_f:插入文件句柄,fptr:偏移量
    @return:
    @author:phewlee
    @note:
    @date: 2021-05-25,10:16
*/
/*----------------------------------------------------------------------------*/
int file_manager_insert_file(FILE *d_f, FILE *i_f, u32 fptr)
{
    int err = finsert_file(d_f, i_f, fptr);
    return err;
}

/*----------------------------------------------------------------------------*/
/** @brief:文件分割处理
    @param:f: 源文件句柄, dev_logo:设备logo , file_name : 文件名， name_len: 文件长度， fptr:偏移量, buf:头文件数据， buf_len:头文件数据长度, folder : 需要指定的路径，绝对路径，例如：/RECORD或者/1/2/3/4, 暂时只支持短名文件夹形式, folder 传NULL默认根目录。
    @return:
    @author:phewlee
    @note:
    @date: 2021-05-25,10:16
*/
/*----------------------------------------------------------------------------*/
int file_manager_division_file(FILE *f, char *dev_logo, char *file_name, u32 name_len, u32 fptr, char *buf, u32 buf_len, char *folder)
{
    struct __dev *dev;
    char path[128] = {0};
    char tmp_name[64] = {0};
    if ((dev_logo == NULL) || (file_name == NULL) || (buf == NULL)) {
        return -1;
    }

    ///////////////////////////////////////////////////
    dev = dev_manager_find_spec(dev_logo, 0);
    if (dev == NULL) {
        r_printf(">>>[test]:errr!!!!!!!!! not find dev\n");
        return -1;
    }
    char *root_path = dev_manager_get_root_path(dev);

    /****************分割文件*********************/
    if (folder) {
        memcpy(tmp_name, folder, strlen(folder));
    }
    strcat(tmp_name, "/");
    strcat(tmp_name, "jl_test.tmp");
    if (fdicvision_file(f, tmp_name, fptr)) {
        r_printf(">>>[test]:file division fail\n");
        goto __exit;
    }

    /****************打开分割出去的文件*********************/
    memcpy(path, root_path, strlen(root_path) - 1);
    memcpy(path + strlen(root_path) - 1, tmp_name, strlen(tmp_name));
    r_printf(">>>[test]:path1 = %s\n", path);
    FILE *i_f = fopen(path, "r");
    if (i_f == NULL) {
        r_printf(">>>[test]:open i_f fail\n");
        return -1;
    }
    ///////////////////////////////////////////////////

    /****************创建新文件，写入头数据*********************/
    memset(path, 0, sizeof(path));
    memcpy(path, root_path, strlen(root_path));
    if (folder) {
        memcpy(path + strlen(path) - 1, folder, strlen(folder));
        strcat(path, "/");
    }
    memcpy(path + strlen(path), file_name, name_len);
    r_printf(">>>[test]:path1 = %s\n", path);
    FILE *d_f = fopen(path, "w+");
    if (d_f == NULL) {
        r_printf(">>>[test]:open d_f fail\n");
        fclose(i_f);
        i_f = NULL;
        goto __exit;
    }
    int wlen = fwrite(d_f, buf, buf_len);
    if (wlen != buf_len) {
        r_printf(">>>[test]:err write!!!!!!!!wlen = %d, buf_len = %d\n", wlen, buf_len);
        fclose(i_f);
        i_f = NULL;
        fclose(d_f);
        d_f = NULL;
        goto __exit;
    }
    fclose(d_f);
    d_f = NULL;
    d_f = fopen(path, "r");
    if (d_f == NULL) {
        r_printf(">>>[test]:open d_f fail\n");
        fclose(i_f);
        i_f = NULL;
        goto __exit;
    }
    //////////////////////////////////////////////////////////

    int fsize = flen(d_f);
    int err = finsert_file(d_f, i_f, fsize);
    fclose(i_f);
    i_f = NULL;
    fclose(d_f);
    d_f = NULL;
    return err;

__exit:
    memset(path, 0, sizeof(path));
    memcpy(path, root_path, strlen(root_path) - 1);
    memcpy(path + strlen(root_path) - 1, tmp_name, strlen(tmp_name));
    r_printf(">>>[test]:error process: path1 = %s\n", path);
    FILE *f_tmp = fopen(path, "r");
    if (f_tmp) {
        fdelete(f_tmp);
        f_tmp = NULL;
    }
    return -1;
}

