#ifndef _SDFILE_H_
#define _SDFILE_H_

#include "typedef.h"


//=======================================================//
//                    默认宏配置定义                     //
//=======================================================//
#ifndef SDFILE_STORAGE
#define SDFILE_STORAGE 			0
#endif /* #ifndef SDFILE_STORAGE */

#define NOR_SDFILE_ENABLE  1
/********* sdfile 文件头 **********/
#define SDFILE_NAME_LEN 			16

typedef struct sdfile_file_head {
    u16 head_crc;
    u16 data_crc;
    u32 addr;
    u32 len;
    u8 attr;
    u8 res;
    u16 index;
    char name[SDFILE_NAME_LEN];
} SDFILE_FILE_HEAD;

#if 0
////////////////////////////sdfile_file_head成员详细说明///////////////////////////////////////
typedef struct SDFILEJL_FILE_HEAD {
    u16 u16Crc;     // 结构体自身校验码
    u16 u16DataCrc; // 文件数据校验码
    u32 u32Address; // 数据存储地址
    u32 u32Length;  // 数据长度
    u8 u8Attribute; // 属性
    u8 u8Res;       // 保留数据
    u16 u16Index;   // 文件编号，从大到小排列，编号为0时，代表后面没有文件
    char szFileName[16];// 文件名
} JL_SDFILE_HEAD;

////////////////////////////flash_head成员详细说明///////////////////////////////////////
typedef struct SDFILEJL_FLASH_HEAD_V2 {
    u16 u16Crc;             // 结构体自身校验码
    u16 u16SizeForBurner;   // 为烧写器保留的空间大小
    char vid[4];    		// 存放VID信息，长度是4个byte
    u32 u32FlashSize;       // FLASH大小,由isd_download计算得出
    u8 u8FsVersion;         // flash类型：BR18(0)/BR22(1)
    u8 u8BlockAlingnModulus;// 对齐系数，对齐的值=对齐系数*256
    u8 u8Res;          		// 保留
    u8 u8SpecialOptFlag;    // 用于标记是否需要生成2种flash格式的标记位，目前只用1位
    char pid[16];    		// 存放PID信息，长度是16个byte
} SDFILEJL_FLASH_HEAD_V2;

#endif

struct sdfile_dir {
    u16 file_num;
    struct sdfile_file_head head;
};

enum sdfile_err_table {
    SDFILE_DIR_NOT_EXIST = -0xFF,
    SDFILE_FILE_NOT_EXIST,
    SDFILE_MALLOC_ERR,
    SDFILE_VM_NOT_FIND,
    SDFILE_DATA_CRC_ERR,
    SDFILE_WRITE_AREA_NEED_ERASE_ERR,
    SDFILE_SUSS = 0,
    SDFILE_END,
};

#if (VFS_ENABLE == 1)

#if SDFILE_STORAGE
#define SDFILE_MAX_DEEPTH 		2
struct sdfile_folder {
    u32 saddr;
    u32 addr;
    u32 len;
};
struct sdfile_scn {
    u8 subpath;
    u8 cycle_mode;
    u8 attr;
    u8 deepth;
    u16 dirCounter;
    u16 fileCounter;
    u16	fileNumber;			// 当前文件序号
    u16	totalFileNumber;
    u16	last_file_number;
    u16 fileTotalInDir;     // 当前目录的根下有效文件的个数
    u16 fileTotalOutDir;	// 当前目录前的文件总数，目录循环模式下，需要用它来计算文件序号
    u32 sclust_id;
    const char *ftypes;
    struct sdfile_file_head head;
    struct sdfile_folder folder[SDFILE_MAX_DEEPTH];
};
#endif

typedef struct sdfile_file_t {
    u32 fptr;
    struct sdfile_file_head head;
#if SDFILE_STORAGE
    struct sdfile_scn *pscn;
#endif
} SDFILE;

#else

typedef struct sdfile_file_t {
    u32 fptr;
    struct sdfile_file_head head;
} SDFILE, FILE;

struct vfs_attr {
    u8 attr;		//属性
    u32 fsize;		//文件大小
    u32 sclust;		//地址
};

SDFILE *sdfile_open(const char *path, const char *mode);
int sdfile_read(SDFILE *fp, void *buf, u32 len);
int sdfile_write(SDFILE *fp, void *buf, u32 len);
int sdfile_seek(SDFILE *fp, u32 offset, u32 fromwhere);
int sdfile_close(SDFILE *fp);
int sdfile_pos(SDFILE *fp);
int sdfile_len(SDFILE *fp);
int sdfile_get_name(SDFILE *fp, u8 *name, int len);
int sdfile_get_attrs(SDFILE *fp, struct vfs_attr *attr);

#if 0
#define fopen 	sdfile_open
#define fread 	sdfile_read
#define fseek 	sdfile_seek
#define fclose 	sdfile_close
#define fwrite 	sdfile_write
#define flen 	sdfile_len
#define fpos 	sdfile_pos
#define fget_name 	sdfile_get_name
#define fget_attrs 	sdfile_get_attrs
#else

static inline SDFILE *fopen(const char *path, const char *mode)
{
    return sdfile_open(path, mode);
}
static inline int fread(SDFILE *fp, void *buf, u32 len)
{
    return sdfile_read(fp, buf, len);
}
static inline int fwrite(SDFILE *fp, void *buf, u32 len)
{
    return sdfile_write(fp, buf, len);
}
static inline int fseek(SDFILE *fp, u32 offset, u32 fromwhere)
{
    return sdfile_seek(fp, offset, fromwhere);
}
static inline int fclose(SDFILE *fp)
{
    return sdfile_close(fp);
}
static inline int fpos(SDFILE *fp)
{
    return sdfile_pos(fp);
}
static inline int flen(SDFILE *fp)
{
    return sdfile_len(fp);
}
static inline int fget_name(SDFILE *fp, u8 *name, int len)
{
    return sdfile_get_name(fp, name, len);
}
static inline int fget_attrs(SDFILE *fp, struct vfs_attr *attr)
{
    return sdfile_get_attrs(fp, attr);
}

#endif

#endif  /* VFS_ENABLE */

int sdfile_delete_data(SDFILE *fp);
#define fdel_data   sdfile_delete_data

#ifndef SDFILE_MOUNT_PATH
#define SDFILE_MOUNT_PATH     	"mnt/sdfile"
#endif /* #ifndef SDFILE_MOUNT_PATH */

#if (USE_SDFILE_NEW)

#ifndef SDFILE_APP_ROOT_PATH
#define SDFILE_APP_ROOT_PATH       	SDFILE_MOUNT_PATH"/app/"  //app分区
#endif /* #ifndef SDFILE_APP_ROOT_PATH */

#ifndef SDFILE_RES_ROOT_PATH
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/res/"  //资源文件分区
#endif /* #ifndef SDFILE_RES_ROOT_PATH */

#else
#ifndef SDFILE_RES_ROOT_PATH
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/C/"
#endif /* #ifndef SDFILE_RES_ROOT_PATH */

#endif /* #if (USE_SDFILE_NEW) */

//获取flash物理大小, 单位: Byte
u32 sdfile_get_disk_capacity(void);
//flash addr  -> cpu addr
u32 sdfile_flash_addr2cpu_addr(u32 offset);
//cpu addr  -> flash addr
u32 sdfile_cpu_addr2flash_addr(u32 offset);

void sdfile_head_addr_get(char *name, u32 *addr, u32 *len);

u32 init_norsdfile_hdl(void);
int set_res_startaddr(int offset);

#endif /* _SDFILE_H_ */

