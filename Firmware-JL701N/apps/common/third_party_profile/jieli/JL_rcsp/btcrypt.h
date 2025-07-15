#ifndef TOMCRYPT_H_
#define TOMCRYPT_H_
// #include <stdio.h>
#include <string.h>


#define  HARDWARE_METHOD  0

#ifndef WIN32
// #include "hw_cpu.h"
#undef   HARDWARE_METHOD
#define  HARDWARE_METHOD 1
#endif


#ifdef __cplusplus
extern "C" {
#endif

/*
@param [in]  pt[*1]      The plaintext
@param ptlen       The length of the plaintext(octets)    range: 1~32
@param [in]  key[*1]     The key for encrypt
@param keylen[*1]  The length of the key(octets)          range: 1~32
@param [out] mac[*1]     output : The Message Authentication Code    16 bytes.
*/
extern void btcon_hash(unsigned char *pt, int ptlen, unsigned char *key, int keylen, unsigned char *mac);

#if HARDWARE_METHOD
extern int yf_aes_start_enc(unsigned char key[16], unsigned char plaintext[16], unsigned char encrypt[16]);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TOMCRYPT_H_ */


/* $Source$ */
/* $Revision$ */
/* $Date$ */
