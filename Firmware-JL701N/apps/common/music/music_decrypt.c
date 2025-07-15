
#include "system/app_core.h"
#include "system/includes.h"
#include "app_config.h"
#include "music/music_decrypt.h"
#include "system/fs/fs.h"

#if (TCFG_DEC_DECRYPT_ENABLE)

#define LOG_TAG_CONST       APP_MUSIC
#define LOG_TAG             "[APP_MUSIC_CIPHER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

/*----------------------------------------------------------------------------*/
/**@brief  解密读使能开关
  @param  u8 ctl
  @return 无
  @note   void cipher_ctl(u8 ctl)
 */
/*----------------------------------------------------------------------------*/
static void cipher_ctl(CIPHER *pcipher, u8 ctl)
{
    pcipher->cipher_enable = ctl;
}

/*----------------------------------------------------------------------------*/
/**@brief  解密读文件数据的回调函数，用于底层的物理读
  @param  void* buf, u32 lba
  @return 无
  @note   void cryptanalysis_buff(void* buf, u32 faddr, u32 len)
 */
/*----------------------------------------------------------------------------*/
#define ALIN_SIZE	4
void cryptanalysis_buff(CIPHER *pcipher, void *buf, u32 faddr, u32 len)
{
    u32 i;
    u8 j;
    u8	head_rem;//
    u8  tail_rem;//
    u32 len_ali;

    u8  *buf_1b_ali;
    u8  *cipher_code;

    cipher_code = (u8 *)&pcipher->cipher_code;

    if (!pcipher->cipher_enable) {
        return;
    }
    /* log_info("----faddr = %d \n",faddr); */
    /* put_buf(buf,len); */
    /* log_info("buf_addr = %d \n", buf); */

    head_rem = ALIN_SIZE - (faddr % ALIN_SIZE);
    if (head_rem == ALIN_SIZE) {
        head_rem = 0;
    }
    if (head_rem > len) {
        head_rem = len;
    }

    if (len - head_rem) {
        tail_rem = (faddr + len) % ALIN_SIZE;
    } else {
        tail_rem = 0;
    }
    /* log_info("head_rem = %d tail_rem = %d \n", head_rem, tail_rem); */
    /* log_info("deal_head_buf\n"); */
    buf_1b_ali = buf;
    j = 3;
    for (i = head_rem; i > 0; i--) {
        buf_1b_ali[i - 1] ^= cipher_code[j--];
        /* log_info("i = %d \n", i - 1); */
        /* log_info("buf_1b_ali[i] = %x \n", buf_1b_ali[i - 1]); */
    }
    /* log_info("\n\n-----------TEST_HEAD-----------------"); */
    /* put_buf(buf_1b_ali, head_rem); */
    /* log_info("deal_main_buf\n"); */
    buf_1b_ali = buf;
    buf_1b_ali = (u8 *)(buf_1b_ali + head_rem);
    len_ali = len - head_rem - tail_rem;
    /* log_info("len_ali = %d \n", len_ali); */
    /* log_info("buf_1b_ali = %d \n", buf_1b_ali); */
    for (i = 0; i < (len_ali / 4); i++) {
        buf_1b_ali[0 + i * 4] ^= cipher_code[0];
        buf_1b_ali[1 + i * 4] ^= cipher_code[1];
        buf_1b_ali[2 + i * 4] ^= cipher_code[2];
        buf_1b_ali[3 + i * 4] ^= cipher_code[3];
    }
    /* log_info("\n\n-----------TEST_MAIN-----------------"); */
    /* put_buf(buf_1b_ali, len_ali); */

    /* log_info("deal_tail_buf\n"); */
    buf_1b_ali = buf;
    buf_1b_ali += len - tail_rem;
    j = 0;
    for (i = 0 ; i < tail_rem; i++) {
        buf_1b_ali[i] ^= cipher_code[j++];
    }
    /* log_info("\n\n-----------TEST_TAIL-----------------"); */
    /* put_buf(buf_1b_ali, tail_rem); */

    /* log_info("\n\n-----------TEST-----------------"); */
    /* put_buf(buf,len); */
}

/*----------------------------------------------------------------------------*/
/**@brief  解密判断
  @param  void *file：当前解码文件的句柄
  @return 无
  @note   void cipher_check_decode_file(void *file)
 */
/*----------------------------------------------------------------------------*/
void cipher_check_decode_file(CIPHER *pcipher, void *file)
{
    int rlen;
    u8 name[128];
    memset(name, 0, sizeof(name));
    rlen = fget_name(file, name, sizeof(name));
    /* log_info("rlen:%d \n", rlen); */
    /* put_buf(name, sizeof(name)); */
    u8 *ext_name = strrchr(name, '.');
    if (ext_name == NULL) {
        // 可能是83结构文件名
        int name_len = strlen(name);
        if (name_len > 3) {
            rlen = name_len - 4;
            goto __check_ext;
        }
    }
    while (rlen--) {
        if ((name[rlen] >= 'a') && (name[rlen] <= 'z')) {
            name[rlen] = name[rlen] - 'a' + 'A';
        }
        if (name[rlen] != '.') {
            continue;
        }
__check_ext:
        /* log_info("file exname : %s \n", &name[rlen+1]); */
        /* log_info("rlen:%d \n", rlen); */
        /* put_buf(name, sizeof(name)); */
        if (((name[rlen + 1] == 'S') && (name[rlen + 2] == 'M') && (name[rlen + 3] == 'P')) // assci
            || ((name[rlen + 2] == 'S') && (name[rlen + 4] == 'M') && (name[rlen + 6] == 'P')) // unicode
           ) {
            log_info("\n----It's a SMP FILE---\n");
            cipher_ctl(pcipher, 1);
        }
        return;
    }
    cipher_ctl(pcipher, 0);
}

/*----------------------------------------------------------------------------*/
/**@brief  解密读初始化
  @param  无
  @return 无
  @note   void cipher_init(u32 key)
 */
/*----------------------------------------------------------------------------*/
void cipher_init(CIPHER *pcipher, u32 key)
{
    pcipher->cipher_code = key;
    cipher_ctl(pcipher, 0);
}

/*----------------------------------------------------------------------------*/
/**@brief  解密读初关闭
  @param  无
  @return 无
  @note   void cipher_close(void)
 */
/*----------------------------------------------------------------------------*/
void cipher_close(CIPHER *pcipher)
{
    cipher_ctl(pcipher, 0);
}

#endif

