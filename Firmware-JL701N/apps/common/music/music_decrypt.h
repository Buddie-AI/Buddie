#ifndef __MUSIC_DECRYPT_H__
#define __MUSIC_DECRYPT_H__

#include "typedef.h"

typedef struct _CIPHER {
    u32 cipher_code;        ///>解密key
    u8  cipher_enable;      ///>解密读使能
} CIPHER;

void cryptanalysis_buff(CIPHER *pcipher, void *buf, u32 faddr, u32 len);

void cipher_check_decode_file(CIPHER *pcipher, void *file);

void cipher_init(CIPHER *pcipher, u32 key);
void cipher_close(CIPHER *pcipher);


#endif //__MUSIC_DECRYPT_H__
