#ifndef _AUDIO_LLNS_H_
#define _AUDIO_LLNS_H_

#include "generic/typedef.h"

int audio_llns_heap_query(int *share_size, int *private_size, int sr);
void *audio_llns_init(char *private_buffer, int private_size, char *share_buffer, int share_size, int sr, float gainfloor, float suppress_level);
int audio_llns_run(void *hdl, short *inbuf, u16 inbuf_size, short *outbuf);
int audio_llns_close(void *hdl);


#endif/*_AUDIO_LLNS_H_*/
