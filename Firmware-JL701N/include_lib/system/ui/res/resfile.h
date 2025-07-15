#ifndef RESFILE_H
#define RESFILE_H

#include "typedef.h"
#include "fs/fs.h"

// resfile共用文件句柄
#if (defined(CONFIG_CPU_BR23) && defined(CONFIG_APP_WATCH))
#define RESFILE_COMMON_HDL_EN		0
#else
#define RESFILE_COMMON_HDL_EN		0
#endif

#define FILE_TYPE_JPEG 	5
#define AT_UI_RAM             AT(.ui_ram)


//图像数据格式
enum {
    PIXEL_FMT_ARGB8888,
    PIXEL_FMT_RGB888,
    PIXEL_FMT_RGB565,
    PIXEL_FMT_L8,
    PIXEL_FMT_AL88,
    PIXEL_FMT_AL44,
    PIXEL_FMT_A8,
    PIXEL_FMT_L1,
    PIXEL_FMT_ARGB8565,
    PIXEL_FMT_OSD16,
    PIXEL_FMT_SOLID,
    PIXEL_FMT_JPEG,
    PIXEL_FMT_UNKNOW,
};

// #define EXTERN_PATH "storage/nor_ui/C/res/"
// #define EXTERN_PATH "storage/virfat_flash/C/uires/"
struct image_file {
    u8 format;
    u8 compress;
    u16 data_crc;
    u16 width;
    u16 height;
    u32 offset;
    u32 len;
};

typedef struct resfile {
    FILE *file;
#if RESFILE_COMMON_HDL_EN
    struct list_head entry;
    u32 offset;
    u32 size;
#endif
} RESFILE;

int open_resfile(const char *name);
void close_resfile();

int res_file_version_compare(int res_ver);

int open_str_file(const char *name);
void close_str_file();
int str_file_version_compare(int str_ver);

int open_style_file(const char *name);

int font_ascii_init(const char *name);
int open_image_by_id(RESFILE *specfile, struct image_file *f, int id, int page);
int read_image_data(struct image_file *f, u8 *data, int len);
int br23_read_image_data(RESFILE *specfile, struct image_file *f, u8 *data, int len, int offset);
int br25_read_image_data(RESFILE *specfile, struct image_file *f, u8 *data, int len, int offset);
u32 image_decode(const void *pSour, void *pDest, u32 SourLen, u32 DestLen, u8 compress);
int open_string_pic(struct image_file *file, int id);
int read_str_data(struct image_file *f, u8 *data, int len);
int br23_read_str_data(struct image_file *f, u8 *data, int len, int offset);
int br25_read_str_data(struct image_file *f, u8 *data, int len, int offset);
int load_pallet_table(int id, u32 *data);
int ui_language_set(int language);
int ui_language_get();

RESFILE *res_fopen(const char *path, const char *mode);
int res_fread(RESFILE *_file, void *buf, u32 len);
int res_fseek(RESFILE *_file, int offset, int fromwhere);
int res_flen(RESFILE *file);
int res_fclose(RESFILE *file);
int _norflash_read_watch(u8 *buf, u32 addr, u32 len, u8 wait);//加速读

struct ui_load_info {
    u8    pj_id;
    const char *path;
    RESFILE *file;
    RESFILE *res;
    RESFILE *str;
};

void *ui_load_res_by_pj_id(int pj_id);
void *ui_load_str_by_pj_id(int pj_id);
int ui_set_sty_path_by_pj_id(int pj_id, const u8 *path);
void *ui_load_sty_by_pj_id(int pj_id);

#endif
