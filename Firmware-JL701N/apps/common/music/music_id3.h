#ifndef __MUSIC_ID3_H__
#define  __MUSIC_ID3_H__
#include "typedef.h"

typedef struct __MP3_ID3_OBJ {
    u8 *id3_buf;
    u32 id3_len;
} MP3_ID3_OBJ;

void id3_obj_post(MP3_ID3_OBJ **obj);
MP3_ID3_OBJ *id3_v1_obj_get(void *file);
MP3_ID3_OBJ *id3_v2_obj_get(void *file);

#endif// __MUSIC_ID3_H__

