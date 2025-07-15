#include "includes.h"
#include "boot.h"
#include "asm/sfc_norflash_api.h"
#include "cfg_private.h"
/* #include "sdfile.h" */

#define CFG_PRIVATE_FILE_NUM   5
SDFILE_FILE_HEAD head = {0};
static u8 cfg_private_create_flag = 0;
static u32 cfg_private_create_sclust = 0;


static FILE *cfg_private_create(const char *path, const char *mode)
{
    int i = 0;
    char *str = NULL;
    char *part_path = NULL;
    char *buf_temp = NULL;
    int file_head_len = 0;
    int align_size = 4096;
    if (boot_info.vm.align == 1) {
        align_size = 256;
    }

    str = strrchr(path, '/');
    /* r_printf(">>>[test]:file name str = %s\n", str + 1); */
    part_path = (char *)zalloc(strlen(path));
    memcpy(part_path, path, strlen(path) - strlen(str));
    /* r_printf(">>>[test]:part_path = %s\n", part_path); */
    FILE *file = fopen(part_path, "r");
    if (!file) {
        r_printf(">>>[test]:no apply part!!!!!!!\n");
        goto __exit;
    }
    struct vfs_attr attrs = {0};
    fget_attrs(file, &attrs);
    attrs.sclust = sdfile_cpu_addr2flash_addr(attrs.sclust);
    if (attrs.sclust % align_size) {
        fclose(file);
        file = NULL;
        r_printf(">>>[test]:part start addr is not alignment, sclust = %d, align_size = %d\n", attrs.sclust, align_size);
        goto __exit;
    }
    cfg_private_create_sclust = attrs.sclust;
    buf_temp = (char *)malloc(align_size);
    norflash_read(NULL, (void *)buf_temp, align_size, attrs.sclust);
    for (; i < CFG_PRIVATE_FILE_NUM; i++) {
        int addr = attrs.sclust + i * sizeof(SDFILE_FILE_HEAD);
        norflash_read(NULL, (void *)&head, sizeof(SDFILE_FILE_HEAD), addr);
        if (CRC16((u8 *)&head + 2, sizeof(SDFILE_FILE_HEAD) - 2) == head.head_crc) {
            file_head_len += head.len;
            continue;
        }
        memset(&head, 0, sizeof(SDFILE_FILE_HEAD));
        head.len = attrs.fsize - CFG_PRIVATE_FILE_NUM * sizeof(SDFILE_FILE_HEAD) - file_head_len;
        /* y_printf(">>>[test]:file len = %d\n", head.len); */
        head.addr = attrs.sclust + CFG_PRIVATE_FILE_NUM * sizeof(SDFILE_FILE_HEAD) + file_head_len;
        head.addr = sdfile_flash_addr2cpu_addr(head.addr);
        /* y_printf(">>>[test]:file addr = %d\n", head.addr); */
        memcpy(head.name, (char *)(str + 1), strlen(str + 1));
        head.attr = 0x02;
        head.head_crc = CRC16((u8 *)&head + 2, sizeof(SDFILE_FILE_HEAD) - 2);
        break;
    }
    if (i >= CFG_PRIVATE_FILE_NUM) {
        fclose(file);
        file = NULL;
        r_printf(">>>[test]:file head is over\n");
        goto __exit;
    }
    cfg_private_erase(attrs.sclust, align_size);
    memcpy(buf_temp + i * sizeof(SDFILE_FILE_HEAD), &head, sizeof(SDFILE_FILE_HEAD));
    norflash_write(NULL, (void *)buf_temp, align_size, attrs.sclust);
    /* put_buf(buf_temp, align_size); */
    fclose(file);
    file = NULL;
    file = fopen(path, mode);

__exit:
    if (part_path) {
        free(part_path);
    }
    if (buf_temp) {
        free(buf_temp);
    }
    return file;
}

FILE *cfg_private_open(const char *path, const char *mode)
{
    int res;
    FILE *file = fopen(path, mode);
    if (!file) {
        if (mode[0] == 'w' && mode[1] == '+') {
            file = cfg_private_create(path, mode);
            if (file) {
                cfg_private_create_flag = 1;
            }
        }
    }
    return file;
}

int cfg_private_read(FILE *file, void *buf, u32 len)
{
    return fread(file, buf, len);
}

void cfg_private_erase(u32 addr, u32 len)
{
    /* r_printf(">>>[test]:addr = 0x%x, len = %d\n", addr, len); */
    u32 erase_total_size = len;
    u32 erase_addr = addr;
    u32 erase_size = 4096;
    u32 erase_cmd = IOCTL_ERASE_SECTOR;
    //flash不同支持的最小擦除单位不同(page/sector)
    //boot_info.vm.align == 1: 最小擦除单位page;
    //boot_info.vm.align != 1: 最小擦除单位sector;
    if (boot_info.vm.align == 1) {
        erase_size = 256;
        erase_cmd = IOCTL_ERASE_PAGE;
    }
    while (erase_total_size) {
        //擦除区域操作
        norflash_ioctl(NULL, erase_cmd, erase_addr);
        erase_addr += erase_size;
        erase_total_size -= erase_size;
    }
}

int cfg_private_check(char *buf, int len)
{
    for (int i = 0; i < len; i++) {
        if (buf[i] != 0xff) {
            return 0;
        }
    }
    return 1;
}

int cfg_private_write(FILE *file, void *buf, u32 len)
{
    int align_size = 4096;
    if (boot_info.vm.align == 1) {
        align_size = 256;
    }
    struct vfs_attr attrs = {0};
    fget_attrs(file, &attrs);
    /* r_printf(">>>[test]:attrs.sclust = 0x%x\n", attrs.sclust); */
    attrs.sclust = sdfile_cpu_addr2flash_addr(attrs.sclust);
    u32 fptr = fpos(file);
    /* r_printf(">>>[test]:addr = %d, fsize = %d, fptr = %d, w_len = %d\n", attrs.sclust, attrs.fsize, fptr, len); */
    if (len + fptr > attrs.fsize) {
        r_printf(">>>[test]:error, write over!!!!!!!!\n");
        return -1;
    }
    char *buf_temp = (char *)malloc(align_size);
    int res = len;

    for (int wlen = 0; len > 0;) {
        u32 align_addr = (attrs.sclust + fptr) / align_size * align_size;
        u32 w_pos = attrs.sclust + fptr - align_addr;
        wlen = align_size - w_pos;
        /* y_printf(">>>[test]:wpos = %d, wlen = %d\n", w_pos, wlen); */
        if (wlen > len) {
            wlen = len;
        }
        norflash_read(NULL, (void *)buf_temp, align_size, align_addr);
        if (0 == cfg_private_check(buf_temp, align_size)) {
            cfg_private_erase(align_addr, align_size);
        }
        memcpy(buf_temp + w_pos, buf, wlen);
        /* put_buf(buf_temp, align_size); */
        norflash_write(NULL, (void *)buf_temp, align_size, align_addr);
        fptr += wlen;
        len -= wlen;
    }
__exit:
    free(buf_temp);
    fseek(file, fptr, SEEK_SET);
    return res;
}

int cfg_private_delete_file(FILE *file)
{
    int align_size = 4096;
    if (boot_info.vm.align == 1) {
        align_size = 256;
    }
    struct vfs_attr attrs = {0};
    fget_attrs(file, &attrs);
    attrs.sclust = sdfile_cpu_addr2flash_addr(attrs.sclust);
    int len = attrs.fsize;
    char *buf_temp = (char *)malloc(align_size);
    u32 fptr = 0;

    for (int wlen = 0; len > 0;) {
        u32 align_addr = (attrs.sclust + fptr) / align_size * align_size;
        u32 w_pos = attrs.sclust + fptr - align_addr;
        wlen = align_size - w_pos;
        if (wlen > len) {
            wlen = len;
        }
        norflash_read(NULL, (void *)buf_temp, align_size, align_addr);
        if (0 == cfg_private_check(buf_temp, align_size)) {
            cfg_private_erase(align_addr, align_size);
        }
        memset(buf_temp + w_pos, 0xff, wlen);
        norflash_write(NULL, (void *)buf_temp, align_size, align_addr);
        fptr += wlen;
        len -= wlen;
    }
    free(buf_temp);
    fclose(file);
    return 0;
}

int cfg_private_close(FILE *file)
{
    if (cfg_private_create_flag) {
        int i = 0;
        int align_size = 4096;
        if (boot_info.vm.align == 1) {
            align_size = 256;
        }
        char name[SDFILE_NAME_LEN];
        fget_name(file, name, SDFILE_NAME_LEN);
        char *buf_temp = (char *)malloc(align_size);
        norflash_read(NULL, (void *)buf_temp, align_size, cfg_private_create_sclust);
        for (; i < CFG_PRIVATE_FILE_NUM; i++) {
            int addr = cfg_private_create_sclust + i * sizeof(SDFILE_FILE_HEAD);
            norflash_read(NULL, (void *)&head, sizeof(SDFILE_FILE_HEAD), addr);
            if (CRC16((u8 *)&head + 2, sizeof(SDFILE_FILE_HEAD) - 2) != head.head_crc) {
                continue;
            }
            if (0 == memcmp(head.name, name, SDFILE_NAME_LEN)) {
                head.len = fpos(file);
                y_printf(">>>[test]:real file len = %d\n", head.len);
                head.head_crc = CRC16((u8 *)&head + 2, sizeof(SDFILE_FILE_HEAD) - 2);
                break;
            }
        }
        cfg_private_create_flag = 0;
        if (i < CFG_PRIVATE_FILE_NUM) {
            cfg_private_erase(cfg_private_create_sclust, align_size);
            memcpy(buf_temp + i * sizeof(SDFILE_FILE_HEAD), &head, sizeof(SDFILE_FILE_HEAD));
            norflash_write(NULL, (void *)buf_temp, align_size, cfg_private_create_sclust);
        }
        if (buf_temp) {
            free(buf_temp);
        }
    }
    return fclose(file);
}

int cfg_private_seek(FILE *file, int offset, int fromwhere)
{
    return fseek(file, offset, fromwhere);
}

void cfg_private_test_demo(void)
{
#if 0
#define N 256
    char test_buf[N] = {0};
    char r_buf[512] = {0};
    for (int i = 0; i < N; i++) {
        test_buf[i] = i & 0xff;
    }

    char path[64] = "mnt/sdfile/app/FATFSI/eq_cfg_hw.bin";
    char path2[64] = "mnt/sdfile/app/FATFSI/cfg_tool.bin";
    y_printf("\n >>>[test]:func = %s,line= %d\n", __FUNCTION__, __LINE__);
    FILE *fp = cfg_private_open(path, "w+");
    if (!fp) {
        r_printf(">>>[test]:open fail!!!!!\n");
        while (1);
    }
    /* cfg_private_delete_file(fp); */
    /* fp = cfg_private_open(path, "w+"); */
    /* cfg_private_read(fp, r_buf, 512); */
    /* put_buf(r_buf, 512); */
    /* cfg_private_seek(fp, 0, SEEK_SET); */

    cfg_private_write(fp, test_buf, N);

    cfg_private_close(fp);
    fp = cfg_private_open(path, "r");
    cfg_private_read(fp, r_buf, 512);
    put_buf(r_buf, 512);
    fclose(fp);
    y_printf("\n >>>[test]:func = %s,line= %d\n", __FUNCTION__, __LINE__);

    fp = cfg_private_open(path2, "w+");
    if (!fp) {
        r_printf(">>>[test]:open fail!!!!!\n");
        while (1);
    }
    for (int i = 0; i < N; i++) {
        test_buf[i] = (N - 1 - i) & 0xff;
    }
    cfg_private_write(fp, test_buf, N);
    cfg_private_close(fp);
    fp = cfg_private_open(path2, "r");
    cfg_private_read(fp, r_buf, 512);
    put_buf(r_buf, 512);
    fclose(fp);

#if 0
    FILE *file = cfg_private_open("mnt/sdfile/app/FATFSI", "w+");
    if (!file) {
        r_printf(">>>[test]:open fail!!!!!\n");
        while (1);
    }
    struct vfs_attr attrs = {0};
    fget_attrs(file, &attrs);
    y_printf(">>>[test]:in part addr = %d, fsize = %d\n", attrs.sclust, attrs.fsize);
    char *part = zalloc(attrs.fsize);
    cfg_private_delete_file(file);
    file = cfg_private_open("mnt/sdfile/app/FATFSI", "w+");
    cfg_private_read(file, part, attrs.fsize);
    put_buf(part, attrs.fsize);
    free(part);
#endif
    while (1);
#endif
}
