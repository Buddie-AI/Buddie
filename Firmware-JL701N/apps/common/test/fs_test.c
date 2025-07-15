#include "system/includes.h"
#include "app_config.h"

#define SD_DEV 	"sd0"
#define SD_MOUNT_PATH "storage/sd0"
#define SD_ROOT_PATH "storage/sd0/C/"
/**********************ac697n 添加SD卡写 配置*******************************
  1.配置板级xxx.h, 对比ad697n_demo.h文件，添加TCFG_SD0_ENABLE宏包住的内容。
  2.配置板级xxx.c, 对比ad697n_demo.c文件，添加TCFG_SD0_ENABLE宏包住的内容。注：sd_set_power 接口里面内容，如有需要自己配置。
  3.配置earphone/music.c ,避免编译不过。注：key_table内容如有需要自己配置。
  4.SD卡检测上线和挂载接口在default_event_handler.c文件中.dev_manager_mount("sd0")接口挂载SD卡。如果已经挂载了，可以不用测试demo中的mount接口。
  5.如下面测试demo，调用接口读写即可。
 ***********************************************************************/


struct imount *mount(const char *dev_name, const char *path,
                     const char *fs_type, int cache_num, void *dev_arg);
#if 1
int SD_test(void)
{
#define FILE_NAME SD_ROOT_PATH"abc.txt"
    log_d("SD driver test >>>> path: %s", FILE_NAME);
    void *mnt = mount(SD_DEV, SD_MOUNT_PATH, "fat", 3, NULL);
    if (mnt) {
        log_d("sd mount fat succ");
    } else {
        log_d("sd mount fat failed");
        return -1;
    }

    FILE *fp = NULL;
    u8 str[] = "This is a test string.";
    u8 buf[100];
    u8 len;

    fp = fopen(FILE_NAME, "w+");
    if (!fp) {
        log_d("open file ERR!");
        return -1;
    }

    len = fwrite(fp, str, sizeof(str));

    if (len != sizeof(str)) {
        log_d("write file ERR!");
        goto _end;
    }
#if 1
    fseek(fp, 0, SEEK_SET);

    len = fread(fp, buf, sizeof(str));

    if (len != sizeof(str)) {
        log_d("read file ERR!");
        goto _end;
    }

    put_buf(buf, sizeof(str));
#endif
    log_d("SD ok!");

_end:
    if (fp) {
        fclose(fp);
    }

    return 0;
}
#endif


#if 0
#define SDFILE_NEW_FILE1 	SDFILE_RES_ROOT_PATH"tone/bt.wtg"
#define SDFILE_NEW_FILE2 	SDFILE_RES_ROOT_PATH"cfg_tool.bin"
#if (USE_SDFILE_NEW == 1)
#define SDFILE_NEW_FILE3 	SDFILE_RES_ROOT_PATH"btif"
#else
#define SDFILE_NEW_FILE3 	SDFILE_RES_ROOT_PATH"RESERVED_CONFIG/btif"
#endif
#define SDFILE_NEW_FILE4 	SDFILE_RES_ROOT_PATH"cfg_tool.bin"
#define SDFILE_READ_LEN 	0x20

void sdfile_test(void)
{
    FILE *fp = NULL;
    u8 buf[SDFILE_READ_LEN];
    u32 len;
    int ret;

    printf("sdfile test >>>>>>>>");
    fp = fopen(SDFILE_NEW_FILE1, "r");

    if (!fp) {
        printf("file open fail");
    }

    printf("file open succ");

    ret = fread(fp, buf, SDFILE_READ_LEN);
    if (ret == SDFILE_READ_LEN) {
        printf("file read succ");
        put_buf(buf, SDFILE_READ_LEN);
    }

    fclose(fp);

    fp = fopen(SDFILE_NEW_FILE4, "r");

    if (!fp) {
        printf("file open fail");
    }

    printf("file open succ4444");

    ret = fread(fp, buf, SDFILE_READ_LEN);
    if (ret == SDFILE_READ_LEN) {
        printf("file read succ444");
        put_buf(buf, SDFILE_READ_LEN);
    }

    fclose(fp);

//////////////////////////////////
    fp = fopen(SDFILE_NEW_FILE2, "r");

    if (!fp) {
        printf("file open fail2");
    }

    ret = fread(fp, buf, SDFILE_READ_LEN);
    if (ret == SDFILE_READ_LEN) {
        printf("file read succ2");
        put_buf(buf, SDFILE_READ_LEN);
    }


    ret = fread(fp, buf, SDFILE_READ_LEN);
    if (ret == SDFILE_READ_LEN) {
        printf("file read succ2");
        put_buf(buf, SDFILE_READ_LEN);
    }

    fseek(fp, 0x200, SEEK_SET);

    ret = fread(fp, buf, SDFILE_READ_LEN);
    if (ret == SDFILE_READ_LEN) {
        printf("file read succ3");
        put_buf(buf, SDFILE_READ_LEN);
    }

    printf("file open succ2");
    fclose(fp);

//////////////////////////////////
    fp = fopen(SDFILE_NEW_FILE3, "r");

    if (!fp) {
        printf("file open fail3");
    }
    printf("file open succ3");

    ret = fread(fp, buf, SDFILE_READ_LEN);
    if (ret == SDFILE_READ_LEN) {
        printf("file read succ4");
        put_buf(buf, SDFILE_READ_LEN);
    }

    char str_buf[] = "test btif write";

    fseek(fp, 0, SEEK_SET);
    ret = fwrite(fp, str_buf, sizeof(str_buf));
    if (ret == sizeof(str_buf)) {
        printf("file write succ");
    }

    fseek(fp, 0, SEEK_SET);
    printf("file will read");
    ret = fread(fp, buf, sizeof(str_buf));
    if (ret == sizeof(str_buf)) {
        printf("file read succ5");
        put_buf(buf, sizeof(str_buf));
    }

    fclose(fp);
}

#endif


#if 0
void m_1() ;
void m_2() ;

void movefun_test()
{
    extern u32 moveable_addr ;
    extern u32 m_code_addr ;
    extern u32 m_code_size ;
    extern u32 moveable_slot_addr ;
    extern u32 moveable_slot_begin ;
    extern u32 moveable_slot_size ;
    u32 *a = &moveable_slot_addr ;
    u32 *b = &moveable_slot_begin ;
    for (int i =  0  ; i < (u32) &moveable_slot_size ; i += 4) {
        *a = *b - (u32)&m_code_addr + (u32) &moveable_addr ;
        printf("index %d = %x %x \r\n", i, *a, *b) ;
        a++ ;
        b++ ;
    }
    printf("sdram addr= %x flash addr=%x size =%x \r\n", (u32)&moveable_addr, (u32)&m_code_addr, (u32)&m_code_size);
    memcpy(&moveable_addr, &m_code_addr, (u32)&m_code_size) ;
    m_1() ;


}
void fun_test()
{
    m_1() ;
    m_2() ;
}
__attribute__((movable_region(2)))
void m_1() AT(.m.code)
{
    printf("movable_regin fun test 1") ;
    m_2() ;
}

__attribute__((movable_region(2)))
void m_2()AT(.m.code)
{
    printf("movable_regin fun test 2") ;

}
#endif
