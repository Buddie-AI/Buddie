#ifndef HMAC_H
#define HMAC_H

#include <string.h>
#include <stddef.h>
#include <stdint.h>


#define HMAC_SELF_TEST  0

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief          Output = Generic_HMAC( hmac key, input buffer )
 *
 * \param md_info  message digest info
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key in bytes
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   Generic HMAC-result
 *
 * \returns        0 on success, MBEDTLS_ERR_MD_BAD_INPUT_DATA if parameter
 *                 verification fails.
 */
int32_t hmac_sha1_crypt(const uint8_t *key, uint32_t keylen, const uint8_t *input, uint32_t ilen, uint8_t *output);


int32_t hmac_sha256_crypt(const uint8_t *key, uint32_t keylen, const uint8_t *input, uint32_t ilen, uint8_t *output);


uint32_t sha256_hkdf(const uint8_t *key, uint32_t key_len, const uint8_t *salt, uint32_t salt_len, const uint8_t *info, uint32_t info_len,
                     uint8_t *out, uint32_t out_len);

#if (HMAC_SELF_TEST == 1)

void hkdf_test(void);

#endif /* HMAC_SELF_TEST */


#ifdef __cplusplus
}
#endif

#endif /* hmac.h */
