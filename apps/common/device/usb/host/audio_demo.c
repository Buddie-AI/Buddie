#include "includes.h"
#include "asm/includes.h"
#include "app_config.h"
#include "system/timer.h"
#include "device/ioctl_cmds.h"
#include "device_drive.h"
#if TCFG_HOST_AUDIO_ENABLE
#include "usb/host/usb_host.h"
#include "usb_ctrl_transfer.h"
#include "usb_bulk_transfer.h"
#include "audio.h"
#include "usb_config.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[AUDIO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#define TEST_FILE_ENABLE   (0)  //从sd卡读数据; 录制数据至sd卡
#if (TEST_FILE_ENABLE)
static FILE *play_file = NULL;
int usb_audio_play_put_buf(void *ptr, u32 len)
{
    int ret = 0;
    if (ptr == NULL && len == 0) {
        //err
        if (play_file) {
            fclose(play_file);
            play_file = NULL;
            return 0;
        }
    }
    if (!play_file) {
        play_file = fopen("storage/sd0/C/raw.pcm", "r"); //单声道
        /* play_file = fopen("storage/sd0/C/raw2.pcm", "r"); //双声道 */
        if (!play_file) {
            log_e("fopen play file faild!\n");
            return -1;
        }
    }
    //读sd卡数据到播放缓存中
    ret = fread(play_file, ptr, len);
    if (ret != len) {
        log_e(" file read buf err %d\n", ret);
        fclose(play_file);
        play_file = NULL;
        return -1;
    }

    return len;
}
static FILE *record_file = NULL;
int usb_audio_record_get_buf(void *ptr, u32 len)
{
#if (TEST_FILE_ENABLE)
    int ret = 0;
    static u32 cnt = 0;
    if (!record_file) {
        record_file = fopen("storage/sd0/C/record01.pcm", "w+");
        cnt = 0;
        if (!record_file) {
            log_e("fopen play file faild!\n");
            return -1;
        }
    }
    putchar('W');
    ret = fwrite(record_file, ptr, len);
    if (ret != len) {
        log_e(" file write buf err %d\n", ret);
        fclose(record_file);
        record_file = NULL;
        return -1;
    }
    //test
    if (cnt++ >= 800) {
        cnt = 0;
        log_info("stop record....\n");
        fclose(record_file);
        record_file = NULL;
    }
#endif

    return len;
}
#else

//将数据传入usb
//ptr:usb数据指针
//len:需要传入的数据长度
int usb_audio_play_put_buf(void *ptr, u32 len)
{
    return len;
}

//从usb读取数据
//ptr:usb数据指针
//len:读取的数据长度
int usb_audio_record_get_buf(void *ptr, u32 len)
{
    return len;
}


#endif

#endif
