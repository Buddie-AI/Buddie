/**
 * \file tuya_ble_utils.c
 *
 * \brief
 */
/*
 *  Copyright (C) 2014-2019, Tuya Inc., All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of tuya ble sdk
 */
#include "tuya_ble_stdlib.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_port.h"
#include "tuya_ble_mem.h"


int32_t tuya_ble_count_bits(uint32_t data)
{
    int32_t i = 0;
    int32_t cnt = 0;

    for (i = 0; i < 32; i++) {
        if (data & 0x01) {
            cnt++;
        }

        data >>= 1;
    }

    return cnt;
}

int32_t tuya_ble_rand_number(int32_t min, int32_t max)
{
    static bool is_srand_init = false;
    int32_t num;

    if (!is_srand_init) {
        is_srand_init = true;
        //srand(time(NULL));
    }

    num = rand() % (max - min) + min;  // [min<=num<=max]

    return num;
}

void tuya_ble_inverted_array(uint8_t *array, uint16_t length)
{
    uint8_t temp;
    uint16_t i, j;
    for (i = 0, j = length - 1; i < j; i++, j--) {
        temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

bool tuya_ble_buffer_value_is_all_x(uint8_t *buffer, uint16_t len, uint8_t value)
{
    bool ret = true;
    for (uint16_t i = 0; i < len; i++) {
        if (buffer[i] != value) {
            ret = false;
            break;
        }
    }
    return ret;
}


uint8_t tuya_ble_check_sum(uint8_t *pbuf, uint16_t len)
{
    uint32_t i = 0, ck_sum = 0;

    for (i = 0; i < len ; i++) {
        ck_sum += pbuf[i];
    }
    return (uint8_t)ck_sum;
}

uint8_t tuya_ble_check_num(uint8_t *buf, uint8_t num)
{
    uint8_t i = 0;

    for (; i < buf[0]; i++) {
        if (buf[i + 1] == num) {
            return 1;
        }
    }
    return 0;
}

void tuya_ble_hextoascii(uint8_t *hexbuf, uint8_t len, uint8_t *ascbuf)
{
    uint8_t i = 0, j = 0, temp = 0;

    for (i = 0; i < len; i++) {
        temp = (hexbuf[i] >> 4) & 0xf;
        if (temp <= 9) {
            ascbuf[j] = temp + 0x30;
        } else {
            ascbuf[j] = temp + 87;
        }
        j++;
        temp = (hexbuf[i]) & 0xf;
        if (temp <= 9) {
            ascbuf[j] = temp + 0x30;
        } else {
            ascbuf[j] = temp + 87;
        }
        j++;
    }

}

void tuya_ble_hextostr(uint8_t *hexbuf, uint8_t len, uint8_t *strbuf)
{
    uint8_t i = 0, j = 0, temp = 0;

    for (i = 0; i < len; i++) {
        temp = (hexbuf[i] >> 4) & 0x0f;
        if (temp <= 9) {
            strbuf[j] = temp + 0x30;
        } else {
            strbuf[j] = temp + 87;
        }
        j++;
        temp = (hexbuf[i]) & 0x0f;
        if (temp <= 9) {
            strbuf[j] = temp + 0x30;
        } else {
            strbuf[j] = temp + 87;
        }
        j++;
    }
    strbuf[j] = 0x00;
}

void tuya_ble_asciitohex(uint8_t *ascbuf, uint8_t *hexbuf)
{
    uint8_t i = 0, j = 0;

    while (ascbuf[i]) {
        j++;
        if ((ascbuf[i] >= 0x30) && (ascbuf[i] <= 0x39)) {
            hexbuf[j] = ((ascbuf[i] - 0x30) << 4);
        } else if ((ascbuf[i] >= 65) && (ascbuf[i] <= 70)) {
            hexbuf[j] = ((ascbuf[i] - 55) << 4);
        } else if ((ascbuf[i] >= 97) && (ascbuf[i] <= 102)) {
            hexbuf[j] = ((ascbuf[i] - 87) << 4);
        }
        i++;
        if ((ascbuf[i] >= 0x30) && (ascbuf[i] <= 0x39)) {
            hexbuf[j] |= (ascbuf[i] - 0x30);
        } else if ((ascbuf[i] >= 65) && (ascbuf[i] <= 70)) {
            hexbuf[j] |= (ascbuf[i] - 55);
        } else if ((ascbuf[i] >= 97) && (ascbuf[i] <= 102)) {
            hexbuf[j] |= (ascbuf[i] - 87);
        }
        i++;
    }
    hexbuf[0] = j;
}



uint8_t tuya_ble_char_2_ascii(uint8_t data)
{
    uint8_t ret = 0xff;

    if ((data >= 48) && (data <= 57)) {
        ret = data - 48;
    } else if ((data >= 65) && (data <= 70)) {
        ret = data - 55;
    } else if ((data >= 97) && (data <= 102)) {
        ret = data - 87;
    }
    return ret;
}

void tuya_ble_str_to_hex(uint8_t *str_buf, uint8_t str_len, uint8_t *hex_buf)
{
    uint8_t data_tmp = 0, i = 0, j = 0;

    for (j = 0 ; j < str_len ; j++) {
        data_tmp = tuya_ble_char_2_ascii(str_buf[j]);
        if (data_tmp != 0xff) {
            hex_buf[i] = (data_tmp << 4);
        } else {
            return;
        }
        j++;
        data_tmp = tuya_ble_char_2_ascii(str_buf[j]);
        if (data_tmp != 0xff) {
            hex_buf[i] += data_tmp;
        } else {
            return;
        }
        i++;
    }
}

void tuya_ble_swap(int16_t *a, int16_t *b)
{
    int16_t    temp;
    temp = *a;
    *a = *b;
    *b = temp;
}


int32_t tuya_ble_hex2int(uint8_t mhex)
{
    switch (mhex) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    default:
        return -1;
    }
}
char tuya_ble_hexstr2int(uint8_t *hexstr, int32_t len, uint8_t *sum)
{
    *sum = 0;
    int32_t value;
    for (int32_t i = 0; i < len; i++) {
        value = tuya_ble_hex2int(hexstr[i]);
        if (value == -1) {
            return 0;
        }
        (*sum) = (*sum) << 4;
        (*sum) += value;
    }
    return 1;
}
char tuya_ble_hexstr2hex(uint8_t *hexstr, int32_t len, uint8_t *hex)
{
    for (uint8_t i = 0; i < len; i += 2) {
        if (tuya_ble_hexstr2int(&hexstr[i], 2, &hex[i / 2]) == 0) {
            return 0;
        }
    }
    return 1;
}


static void swapX(const uint8_t *src, uint8_t *dst, int32_t len)
{
    int32_t i;
    for (i = 0; i < len; i++) {
        dst[len - 1 - i] = src[i];
    }
}

void tuya_ble_swap24(uint8_t dst[3], const uint8_t src[3])
{
    swapX(src, dst, 3);
}

void tuya_ble_swap32(uint8_t dst[4], const uint8_t src[4])
{
    swapX(src, dst, 4);
}

void tuya_ble_swap48(uint8_t dst[7], const uint8_t src[7])
{
    swapX(src, dst, 6);
}

void tuya_ble_swap56(uint8_t dst[7], const uint8_t src[7])
{
    swapX(src, dst, 7);
}

void tuya_ble_swap64(uint8_t dst[8], const uint8_t src[8])
{
    swapX(src, dst, 8);
}

void tuya_ble_swap128(uint8_t dst[16], const uint8_t src[16])
{
    swapX(src, dst, 16);
}



uint16_t tuya_ble_crc16_compute(uint8_t *p_data, uint16_t size, uint16_t *p_crc)
{

    uint16_t poly[2] = {0, 0xa001};            //0x8005 <==> 0xa001
    uint16_t crc;
    int i, j;
    crc = (p_crc == NULL) ? 0xFFFF : *p_crc;

    for (j = size; j > 0; j--) {
        unsigned char ds = *p_data++;

        for (i = 0; i < 8; i++) {
            crc = (crc >> 1) ^ poly[(crc ^ ds) & 1];
            ds = ds >> 1;
        }
    }

    return crc;
}


uint32_t tuya_ble_crc32_compute(uint8_t const *p_data, uint32_t size, uint32_t const *p_crc)
{
    uint32_t crc;
    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++) {
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--) {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}



/**@brief Function for checking if a pointer value is aligned to a 4 byte boundary.
 *
 * @param[in]   p   Pointer value to be checked.
 *
 * @return      TRUE if pointer is aligned to a 4 byte boundary, FALSE otherwise.
 */
bool tuya_ble_is_word_aligned_tuya(void const *p)
{
    return (((uintptr_t)p & 0x03) == 0);
}



void tuya_ble_device_id_20_to_16(uint8_t *in, uint8_t *out)
{
    uint8_t i, j;
    uint8_t temp[4];
    for (i = 0; i < 5; i++) {
        for (j = i * 4; j < (i * 4 + 4); j++) {
            if ((in[j] >= 0x30) && (in[j] <= 0x39)) {
                temp[j - i * 4] = in[j] - 0x30;
            } else if ((in[j] >= 0x41) && (in[j] <= 0x5A)) {
                temp[j - i * 4] = in[j] - 0x41 + 36;
            } else if ((in[j] >= 0x61) && (in[j] <= 0x7A)) {
                temp[j - i * 4] = in[j] - 0x61 + 10;
            } else {

            }
        }

        out[i * 3] = temp[0] & 0x3F;
        out[i * 3] <<= 2;
        out[i * 3] |= ((temp[1] >> 4) & 0x03);

        out[i * 3 + 1] = temp[1] & 0x0F;
        out[i * 3 + 1] <<= 4;
        out[i * 3 + 1] |= ((temp[2] >> 2) & 0x0F);

        out[i * 3 + 2] = temp[2] & 0x03;
        out[i * 3 + 2] <<= 6;
        out[i * 3 + 2] |= temp[3] & 0x3F;

    }

    out[15] = 0xFF;
}

void tuya_ble_device_id_16_to_20(uint8_t *in, uint8_t *out)
{
    uint8_t i, j;
    uint8_t temp[4];
    for (i = 0; i < 5; i++) {
        j = i * 3;
        temp[j - i * 3] = (in[j] >> 2) & 0x3F;
        temp[j - i * 3 + 1] = in[j] & 0x03;
        temp[j - i * 3 + 1] <<= 4;
        temp[j - i * 3 + 1] |= (in[j + 1] >> 4) & 0x0F;
        temp[j - i * 3 + 2] = (in[j + 1] & 0x0F) << 2;
        temp[j - i * 3 + 2] |= ((in[j + 2] & 0xC0) >> 6) & 0x03;
        temp[j - i * 3 + 3] = in[j + 2] & 0x3F;

        for (j = i * 4; j < (i * 4 + 4); j++) {
            if ((temp[j - i * 4] >= 0) && (temp[j - i * 4] <= 9)) {
                out[j] = temp[j - i * 4] + 0x30;
            } else if ((temp[j - i * 4] >= 10) && (temp[j - i * 4] <= 35)) {
                out[j] = temp[j - i * 4] + 87;
            } else if ((temp[j - i * 4] >= 36) && (temp[j - i * 4] <= 61)) {
                out[j] = temp[j - i * 4] + 29;
            } else {

            }
        }


    }

}


/**
 * @brief   Function for search character/symbol index from input string
 *
 * @param
 *          [in]data     : pointer to input data
 *          [in]data     : Number of bytes of data
 *          [in]symbol   : Search symbol or character, Such as ':' or ','
 *          [out]index[] : Array of indexs
 *
 * @return  The number of matched character/symbol
 *
 * @note
 *
 * */
int32_t tuya_ble_search_symbol_index(char *data, uint16_t len, char symbol, uint8_t index[])
{
    int32_t i;
    uint8_t index_buf[64] = {0};
    uint8_t symbol_cnt = 0;

    if (data == NULL || len == 0 || index == NULL) {
        return symbol_cnt;
    }

    for (i = 0; i < len; i++) {
        if (data[i] == symbol) {
            index_buf[symbol_cnt] = i;
            symbol_cnt += 1;

            if (symbol_cnt >= sizeof(index_buf)) {
                /* error, too many symbols */
                break;
            }
        }
    }

    if (symbol_cnt != 0) {
        memcpy(index, index_buf, symbol_cnt);
    }

    return symbol_cnt;
}


int32_t tuya_ble_ascii_to_int(char *ascii, uint16_t len)
{
    int32_t i;
    int32_t num = 0;
    bool negative = false;

    if (ascii == NULL || len == 0) {
        return num;
    }

    if (ascii[0] == 0x2D) {
        negative = true;
    } else {
        num *= 10;
        num += ascii[0] - '0';
    }

    for (i = 1; i < len; i++) {
        num *= 10;
        num += ascii[i] - '0';
    }

    if (negative) {
        num = -1 * num;
    }

    return num;
}


static uint8_t base64_decode(const uint8_t *input, uint16_t inlen, uint8_t *output, uint16_t *outlen)
{
    const char *base64_tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8_t reverse_tbl[256] = {0};
    int32_t off = 0;
    int32_t i = 0;

    if (NULL == input) {
        return 1;
    }

    if (inlen == 0) {
        inlen = strlen((char *)input);
    }

    if (inlen == 0 || (inlen % 4 != 0)) {
        return 1;
    }

    for (i = 0; i < 64; i++) {
        reverse_tbl[base64_tbl[i]] = i;
    }

    for (i = 0; i < inlen - 4; i += 4) {
        output[off++] = (reverse_tbl[input[i]] << 2) | ((reverse_tbl[input[i + 1]] >> 4) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 1]] << 4) | ((reverse_tbl[input[i + 2]] >> 2) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 2]] << 6) | ((reverse_tbl[input[i + 3]]) & 0xFF);
    }

    if (input[i + 2] == '=') {
        output[off++] = (reverse_tbl[input[i]] << 2) | ((reverse_tbl[input[i + 1]] >> 4) & 0xFF);
    } else if (input[i + 3] == '=') {
        output[off++] = (reverse_tbl[input[i]] << 2) | ((reverse_tbl[input[i + 1]] >> 4) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 1]] << 4) | ((reverse_tbl[input[i + 2]] >> 2) & 0xFF);
    } else {
        output[off++] = (reverse_tbl[input[i]] << 2) | ((reverse_tbl[input[i + 1]] >> 4) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 1]] << 4) | ((reverse_tbl[input[i + 2]] >> 2) & 0xFF);
        output[off++] = (reverse_tbl[input[i + 2]] << 6) | ((reverse_tbl[input[i + 3]]) & 0xFF);
    }

    if (NULL != outlen) {
        *outlen = off;
    }

    return 0;
}

static int32_t delChar(char *s, uint16_t len, char match_char)
{
    int32_t i, j;
    int32_t counter = 0;

    for (i = 0; i < len; i++) {
        if (s[i] == match_char) {
            counter++;

            for (j = i; j < len; j++) {
                s[j] = s[j + 1];
                i--;
            }
        }
    }
    return counter;
}


/**
 * @brief   Function for pem format ecc key to hex foramt
 *
 * @param
 *          [in]pem     : pointer to pem data
 *          [out]key    : pointer to output data, hex format ecc key
 *          [out]key_len: Number of bytes of key.
 *
 * @return  0 - failed, 1 - success
 *
 * @note    developer could use tuya_ble_ecc_key_pem2hex_example() function to test
 *
 * */
int32_t tuya_ble_ecc_key_pem2hex(const char *pem, uint8_t *key, uint16_t *key_len)
{
    char buf1[256] = {0};
    char buf2[256] = {0};
    uint16_t inlen = 0;
    uint16_t i, j, k = 0;
    uint16_t len, len1, len2 = 0;

    if (NULL == pem) {
        return 0;
    }

    inlen = strlen(pem);

    if (inlen > 256) {
        return 0;
    }

    //head
    if ((pem[0] != '-') || (pem[1] != '-') || (pem[2] != '-') || (pem[3] != '-') || (pem[4] != '-')) {
        return 0;
    }

    //tail
    if ((pem[inlen - 1] != '-') || (pem[inlen - 2] != '-') || (pem[inlen - 3] != '-') || (pem[inlen - 4] != '-') || (pem[inlen - 5] != '-')) {
        return 0;
    }

    //find head end
    for (i = 5; i < inlen - 5; i++) {
        if (pem[i] == '-') {
            if ((pem[i + 1] != '-') || (pem[i + 2] != '-') || (pem[i + 3] != '-') || (pem[i + 4] != '-')) {
                return 0;
            }

            len1 = i + 5;
            break;
        }
    }

    //remove head
    memcpy(buf1, pem + len1, inlen - len1);

    //find tail
    for (i = 0; i < inlen - len1; i++) {
        if (buf1[i] == '-') {
            if ((buf1[i + 1] != '-') || (buf1[i + 2] != '-') || (buf1[i + 3] != '-') || (buf1[i + 4] != '-')) {
                return 0;
            }

            len2 = i;
            break;
        }
    }
    len = len2;

    //remove \n
    len1 = delChar(buf1, len, '\n');
    len = len - len1;
    //remove \r
    len1 = delChar(buf1, len, '\r');
    len = len - len1;

    //decode
    base64_decode((uint8_t *)buf1, len, (uint8_t *)buf2, (uint16_t *)&len2);

    //next is asn.1 decode
    if (buf2[0] != 0x30) { //0x30
        return 0;
    }

    len1 = buf2[1];//0x30 0x41
    if ((len1 + 2) != len2) {
        return 0;
    }

    if (buf2[2] != 0x02) { //0x30 0x41 0x20 0x01 0x00
        return 0;
    }

    if (buf2[5] != 0x30) {
        return 0;
    }

    len1 = buf2[6];

    if (buf2[7 + len1] != 0x04) { //0x04
        return 0;
    }

//	len2 = buf2[8+len1];//0x04 0x27

    if (buf2[9 + len1] != 0x30) { //0x30
        return 0;
    }

//	len = buf2[10+len1];//0x25

    if (buf2[11 + len1] != 0x02) { //0x02
        return 0;
    }

    if (buf2[14 + len1] != 0x04) { //0x04
        return 0;
    }

    *key_len = buf2[15 + len1];
    memcpy(key, &buf2[16 + len1], *key_len);

    return 1;
}


void tuya_ble_ecc_key_pem2hex_example(void)
{
    uint8_t key[64] = {0};
    uint16_t keylen = 0;

    char test_char[] =
        "-----BEGIN PRIVATE KEY-----MEECAQAwEwYHKoZIzj0CAQYIKoZIzj0DAQcEJzAlAgEBBCBX5s0E0DJowr3ibkG41jfvjRFgaoCO7v8fU/Wq/xLlDQ==-----END PRIVATE KEY-----";

    tuya_ble_ecc_key_pem2hex(test_char, key, &keylen);

    /* output hex format key */
    // 57 E6 CD 04 D0 32 68 C2 BD E2 6E 41 B8 D6 37 EF
    // 8D 11 60 6A 80 8E EE FF 1F 53 F5 AA FF 12 E5 0D
}


/**
 * @brief   Function for extract r+s from der from ecdsa sign
 *
 * @param
 *          [in]der        : pointer to der data
 *          [out]raw_rs    : pointer to output data, the raw data[r+s] of der
 *
 * @return  0 - failed, 1 - success
 *
 * @note
 *
 * */
int32_t tuya_ble_ecc_sign_secp256r1_extract_raw_from_der(const char *der, uint8_t *raw_rs)
{
    /* extract r + s from der */
    int pos = 0;
    uint8_t raw[64] = {0};

    if (der == NULL || raw_rs == NULL) {
        return 0;
    }

    if (der[3] != 0x20) {
        memcpy(raw, &der[5], 32);
        pos = 5 + 32;
    } else {
        memcpy(raw, &der[4], 32);
        pos = 4 + 32;
    }

    // 37
    if (der[pos + 1] != 0x20) {
        memcpy(&raw[32], &der[pos + 3], 32);
    } else {
        memcpy(&raw[32], &der[pos + 2], 32);
    }

    memcpy(raw_rs, raw, sizeof(raw));
    return 1;
}

