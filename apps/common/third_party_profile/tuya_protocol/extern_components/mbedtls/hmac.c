#include "sha1.h"
#include "sha256.h"
#include "hmac.h"


#define hmac_printf


#ifndef MIN
#define MIN(a,b)          ((a) < (b) ? (a) : (b))
#endif

#ifndef CEIL_DIV
#define CEIL_DIV(a, b)    (((a) + (b) - 1) / (b))
#endif


#define SHA1_DIGEST_SIZE  20
#define SHA1_BLOCK_SIZE   64

#define SHA256_DIGEST_SIZE 32
#define SHA256_BLOCK_SIZE  64


typedef struct {
    /** Digest-specific context */
    mbedtls_sha1_context *md_ctx;

    /** HMAC part of the context */
    uint8_t ipad[64];
    uint8_t opad[64];
} hmac_sha1_context_t;

typedef struct {
    /** Digest-specific context */
    mbedtls_sha256_context *md_ctx;

    /** HMAC part of the context */
    uint8_t ipad[64];
    uint8_t opad[64];
} hmac_sha256_context_t;



void hmac_sha1_init(hmac_sha1_context_t *ctx)
{
    memset(ctx, 0, sizeof(hmac_sha1_context_t));
}

void hmac_sha1_free(hmac_sha1_context_t *ctx)
{
    mbedtls_sha1_free((mbedtls_sha1_context *)ctx->md_ctx);
}


int32_t hmac_sha1_setup(hmac_sha1_context_t *ctx, int32_t hmac)
{
    return 0;
}

int hmac_sha1_starts(hmac_sha1_context_t *ctx, const uint8_t *key, uint32_t keylen)
{

    uint8_t sum[SHA1_DIGEST_SIZE];
    uint32_t i;

    if (keylen > (uint32_t) SHA1_BLOCK_SIZE) {
        mbedtls_sha1_starts((mbedtls_sha1_context *) ctx->md_ctx);
        mbedtls_sha1_update((mbedtls_sha1_context *) ctx->md_ctx, key, keylen);
        mbedtls_sha1_finish((mbedtls_sha1_context *) ctx->md_ctx, sum);

        keylen = SHA1_DIGEST_SIZE;
        key = sum;
    }


    memset(ctx->ipad, 0x36, SHA1_BLOCK_SIZE);
    memset(ctx->opad, 0x5C, SHA1_BLOCK_SIZE);

    for (i = 0; i < keylen; i++) {
        ctx->ipad[i] = (uint8_t)(ctx->ipad[i] ^ key[i]);
        ctx->opad[i] = (uint8_t)(ctx->opad[i] ^ key[i]);
    }

    memset(sum, 0, sizeof(sum));

    mbedtls_sha1_starts((mbedtls_sha1_context *) ctx->md_ctx);
    mbedtls_sha1_update((mbedtls_sha1_context *) ctx->md_ctx, ctx->ipad, SHA1_BLOCK_SIZE);

    return 0;

}

int32_t hmac_sha1_update(hmac_sha1_context_t *ctx, const uint8_t *input, uint32_t ilen)
{
    mbedtls_sha1_update((mbedtls_sha1_context *) ctx->md_ctx, input, ilen);
    return 0;
}

int32_t hmac_sha1_finish(hmac_sha1_context_t *ctx, uint8_t *output)
{
    uint8_t tmp[SHA1_DIGEST_SIZE];

    mbedtls_sha1_finish((mbedtls_sha1_context *) ctx->md_ctx, tmp);
    mbedtls_sha1_starts((mbedtls_sha1_context *) ctx->md_ctx);
    mbedtls_sha1_update((mbedtls_sha1_context *) ctx->md_ctx, ctx->opad, SHA1_BLOCK_SIZE);
    mbedtls_sha1_update((mbedtls_sha1_context *) ctx->md_ctx, tmp, SHA1_DIGEST_SIZE);
    mbedtls_sha1_finish((mbedtls_sha1_context *) ctx->md_ctx, output);

    return 0;
}

int32_t hmac_sha1_reset(hmac_sha1_context_t *ctx)
{

    mbedtls_sha1_starts((mbedtls_sha1_context *) ctx->md_ctx);
    mbedtls_sha1_update((mbedtls_sha1_context *) ctx->md_ctx, ctx->ipad, SHA1_BLOCK_SIZE);

    return 0 ;
}

int32_t hmac_sha1_crypt(const uint8_t *key, uint32_t keylen, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    hmac_sha1_context_t ctx;
    mbedtls_sha1_context sha_ctx;
    int32_t ret;

    hmac_sha1_init(&ctx);
    memset(&sha_ctx, 0, sizeof(sha_ctx));
    ctx.md_ctx = &sha_ctx;
    if ((ret = hmac_sha1_setup(&ctx, 1)) != 0) {
        return (ret);
    }

    hmac_sha1_starts(&ctx, key, keylen);
    hmac_sha1_update(&ctx, input, ilen);
    hmac_sha1_finish(&ctx, output);

    return (0);
}

//

void hmac_sha256_init(hmac_sha256_context_t *ctx)
{
    memset(ctx, 0, sizeof(hmac_sha256_context_t));
}

void hmac_sha256_free(hmac_sha256_context_t *ctx)
{
    mbedtls_sha256_free((mbedtls_sha256_context *)ctx->md_ctx);
}


int32_t hmac_sha256_setup(hmac_sha256_context_t *ctx, int32_t hmac)
{
    return 0;
}

int hmac_sha256_starts(hmac_sha256_context_t *ctx, const uint8_t *key, uint32_t keylen)
{

    uint8_t sum[SHA256_DIGEST_SIZE];
    uint32_t i;

    if (keylen > (uint32_t) SHA256_BLOCK_SIZE) {
        mbedtls_sha256_starts((mbedtls_sha256_context *) ctx->md_ctx, 0);
        mbedtls_sha256_update((mbedtls_sha256_context *) ctx->md_ctx, key, keylen);
        mbedtls_sha256_finish((mbedtls_sha256_context *) ctx->md_ctx, sum);

        keylen = SHA256_DIGEST_SIZE;
        key = sum;
    }


    memset(ctx->ipad, 0x36, SHA256_BLOCK_SIZE);
    memset(ctx->opad, 0x5C, SHA256_BLOCK_SIZE);

    for (i = 0; i < keylen; i++) {
        ctx->ipad[i] = (uint8_t)(ctx->ipad[i] ^ key[i]);
        ctx->opad[i] = (uint8_t)(ctx->opad[i] ^ key[i]);
    }

    memset(sum, 0, sizeof(sum));

    mbedtls_sha256_starts((mbedtls_sha256_context *) ctx->md_ctx, 0);
    mbedtls_sha256_update((mbedtls_sha256_context *) ctx->md_ctx, ctx->ipad, SHA256_BLOCK_SIZE);

    return 0;

}

int32_t hmac_sha256_update(hmac_sha256_context_t *ctx, const uint8_t *input, uint32_t ilen)
{
    mbedtls_sha256_update((mbedtls_sha256_context *) ctx->md_ctx, input, ilen);
    return 0;
}

int32_t hmac_sha256_finish(hmac_sha256_context_t *ctx, uint8_t *output)
{
    uint8_t tmp[SHA256_DIGEST_SIZE];

    mbedtls_sha256_finish((mbedtls_sha256_context *) ctx->md_ctx, tmp);
    mbedtls_sha256_starts((mbedtls_sha256_context *) ctx->md_ctx, 0);
    mbedtls_sha256_update((mbedtls_sha256_context *) ctx->md_ctx, ctx->opad, SHA256_BLOCK_SIZE);
    mbedtls_sha256_update((mbedtls_sha256_context *) ctx->md_ctx, tmp, SHA256_DIGEST_SIZE);
    mbedtls_sha256_finish((mbedtls_sha256_context *) ctx->md_ctx, output);

    return 0;
}

int32_t hmac_sha256_reset(hmac_sha256_context_t *ctx)
{

    mbedtls_sha256_starts((mbedtls_sha256_context *) ctx->md_ctx, 0);
    mbedtls_sha256_update((mbedtls_sha256_context *) ctx->md_ctx, ctx->ipad, SHA256_BLOCK_SIZE);

    return 0 ;
}

int32_t hmac_sha256_crypt(const uint8_t *key, uint32_t keylen, const uint8_t *input, uint32_t ilen, uint8_t *output)
{
    hmac_sha256_context_t ctx;
    mbedtls_sha256_context sha_ctx;
    int32_t ret;

    hmac_sha256_init(&ctx);
    memset(&sha_ctx, 0, sizeof(sha_ctx));
    ctx.md_ctx = &sha_ctx;
    if ((ret = hmac_sha256_setup(&ctx, 1)) != 0) {
        return (ret);
    }

    hmac_sha256_starts(&ctx, key, keylen);
    hmac_sha256_update(&ctx, input, ilen);
    hmac_sha256_finish(&ctx, output);

    return (0);
}

uint32_t sha256_hkdf(const uint8_t *key, uint32_t key_len,
                     const uint8_t *salt, uint32_t salt_len,
                     const uint8_t *info, uint32_t info_len,
                     uint8_t *out, uint32_t out_len)
{
    const uint8_t null_salt[32] = {0};
    uint8_t PRK[32];
    uint8_t T_n[32];
    uint32_t loop;
    uint32_t temp_len;

    // Step 1: HKDF-Extract(salt, IKM) -> PRK
    if (salt == NULL) {
        hmac_sha256_crypt(null_salt, 32, key, key_len, PRK);
    } else {
        hmac_sha256_crypt(salt, salt_len, key, key_len, PRK);
    }

    // Step 2: HKDF-Expand(PRK, info, L) -> OKM
    //T(0) = empty string (zero length)
    //T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
    //T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
    //T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)

    uint8_t temp[32 + info_len + 1];
    memset(temp, 0, 32 + info_len + 1);
    loop = CEIL_DIV(out_len, 32);

    for (int32_t i = 0; i < loop ; i++) {
        if (i == 0) {
            temp_len = 0;
        } else {
            memcpy(temp, T_n, 32);
            temp_len = 32;
        }

        memcpy(temp + temp_len, info, info_len);
        temp_len += info_len;

        temp[temp_len] = i + 1;
        temp_len += 1;

        hmac_sha256_crypt(PRK, 32, temp, temp_len, T_n);

        memcpy(out + 32 * i, T_n, MIN(32, out_len));
        out_len -= 32;
    }

    return 0;
}



/***************************************************this is for test************************************************/
#if (HMAC_SELF_TEST == 1)

///////////////////////////////////////////
//Hash = SHA-256
//IKM  = 0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b (22 octets)
//salt = 0x000102030405060708090a0b0c (13 octets)
//info = 0xf0f1f2f3f4f5f6f7f8f9 (10 octets)
//L    = 42
//
//PRK  = 0x077709362c2e32df0ddc3f0dc47bba63
//       90b6c73bb50f9c3122ec844ad7c2b3e5 (32 octets)
//OKM  = 0x3cb25f25faacd57a90434f64d0362f2a
//       2d2d0a90cf1a5a4c5db02d56ecc4c5bf
//       34007208d5b887185865 (42 octets)

#if 0
uint8_t salt[13] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
uint8_t info[10] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9};
uint8_t key_material[22] = {0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b};
#endif


void hkdf_test(void)
{
    uint8_t key_material[80] = {0x00,
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
                                0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                                0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
                                0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
                               };

    uint8_t salt[80] = {0x60,
                        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
                        0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80,
                        0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90,
                        0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0,
                        0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf
                       };

    uint8_t info[80] = {0xb0,
                        0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0,
                        0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
                        0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0,
                        0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0,
                        0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
                       };

    uint8_t expect[82] = {
        0xb1, 0x1e, 0x39, 0x8d, 0xc8, 0x03, 0x27, 0xa1, 0xc8, 0xe7, 0xf7, 0x8c, 0x59, 0x6a, 0x49, 0x34,
        0x4f, 0x01, 0x2e, 0xda, 0x2d, 0x4e, 0xfa, 0xd8, 0xa0, 0x50, 0xcc, 0x4c, 0x19, 0xaf, 0xa9, 0x7c,
        0x59, 0x04, 0x5a, 0x99, 0xca, 0xc7, 0x82, 0x72, 0x71, 0xcb, 0x41, 0xc6, 0x5e, 0x59, 0x0e, 0x09,
        0xda, 0x32, 0x75, 0x60, 0x0c, 0x2f, 0x09, 0xb8, 0x36, 0x77, 0x93, 0xa9, 0xac, 0xa3, 0xdb, 0x71,
        0xcc, 0x30, 0xc5, 0x81, 0x79, 0xec, 0x3e, 0x87, 0xc1, 0x4c, 0x01, 0xd5, 0xc1, 0xf3, 0x43, 0x4f,
        0x1d, 0x87
    };
    uint8_t out_key[82];

    sha256_hkdf(key_material, sizeof(key_material), salt, sizeof(salt), info, sizeof(info), out_key, 82);

    hmac_printf("SHA256-HKDF TEST: ");
    if (memcmp(expect, out_key, 82) == 0) {
        hmac_printf(" PASS\n");
    } else {
        hmac_printf(" FAIL\n");
    }

}


#endif

