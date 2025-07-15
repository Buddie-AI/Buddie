/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) library
 *
 *  Copyright (C) 2008-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2004-2005  Henryk Ploetz <henryk@ploetzli.ch>
 *  Copyright (C) 2005-2006  Brad Midgley <bmidgley@xmission.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __SBC_ENC_H
#define __SBC_ENC_H

#ifdef __cplusplus
extern "C" {
#endif

//typedef unsigned char uint8 ,u8 ,U8 ,uint8_t;
//typedef unsigned short int uint16 ,UINT16 ,U16 ,u16 ,uint16_t;
//typedef unsigned int uint32 ,UINT32,U32 ,u32 ,uint32_t ,uintptr_t;
//typedef signed char sint8 ,s8 ,S8,int8 ,int8_t;
//typedef signed short int sint16 ,SINT16 ,S16 ,s16,int16,INT16,int16_t ;
//typedef signed int sint32 ,SINT32,S32 ,s32,int32 ,INT32;//,;
//typedef unsigned int  size_t,ssize_t;
//typedef int int32_t ;
//typedef unsigned __int64  int64_t;
//typedef __int64 uint64_t ;


//#include "typedef.h"
//#include <stdint.h>
//#include <sys/types.h>
#include <stddef.h>
#ifndef __GNUC__
#define  SBC_ENCODE
#endif
/* sampling frequency */
#define SBC_FREQ_16000		0x00
#define SBC_FREQ_32000		0x01
#define SBC_FREQ_44100		0x02
#define SBC_FREQ_48000		0x03


/* blocks */
#define SBC_BLK_4		0x00
#define SBC_BLK_8		0x01
#define SBC_BLK_12		0x02
#define SBC_BLK_16		0x03

/* channel mode */
#define SBC_MODE_MONO		0x00
#define SBC_MODE_DUAL_CHANNEL	0x01
#define SBC_MODE_STEREO		0x02
#define SBC_MODE_JOINT_STEREO	0x03

/* allocation method */
#define SBC_AM_LOUDNESS		0x00
#define SBC_AM_SNR		0x01

/* subbands */
#define SBC_SB_4		0x00
#define SBC_SB_8		0x01

/* Data endianness */
#define SBC_LE			0x00
#define SBC_BE			0x01

typedef struct sbc_struct {
    unsigned int flags;

    unsigned char frequency;
    unsigned char blocks;
    unsigned char subbands;
    unsigned char mode;
    unsigned char allocation;
    unsigned char bitpool;
    unsigned char endian;

    void *priv;
    void *priv_alloc_base;
} sbc_t;




unsigned int sbc_enc_query(void);
int sbc_init(sbc_t *sbc, unsigned long flags, void *priv);
int sbc_reinit(sbc_t *sbc, unsigned long flags);

unsigned int sbc_parse(sbc_t *sbc, const void *input, size_t input_len);

/* Decodes ONE input block into ONE output block */
unsigned int sbc_decode(sbc_t *sbc, const void *input, size_t input_len,
                        void *output, size_t output_len, size_t *written);

/* Encodes ONE input block into ONE output block */
unsigned int sbc_encode(sbc_t *sbc, const void *input, size_t input_len,
                        void *output, size_t output_len, unsigned int *written);

/* Returns the output block size in bytes */
//size_t sbc_get_frame_length(sbc_t *sbc);

/* Returns the time one input/output block takes to play in msec*/
unsigned sbc_get_frame_duration(sbc_t *sbc);

/* Returns the input block size in bytes */
//size_t sbc_get_codesize(sbc_t *sbc);

const char *sbc_get_implementation_info(sbc_t *sbc);
void sbc_finish(sbc_t *sbc);
//#define htons(x) ( ( (((unsigned short)(x)) >>8) & 0xff) | ((((unsigned short)(x)) & 0xff)<<8) )



#ifdef __cplusplus
}
#endif

struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};


#endif /* __SBC_ENC_H */


