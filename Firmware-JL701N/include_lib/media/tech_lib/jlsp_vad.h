/*****************************************************************
>file name : jlsp_vad.h
>create time : Fri 17 Dec 2021 02:18:47 PM CST
*****************************************************************/
#ifndef _JLSP_VAD_H_
#define _JLSP_VAD_H_

int JLSP_vad_get_heap_size(void *file_ptr, int *model_size);
void *JLSP_vad_init(char *heap_buffer, int heap_size, char *share_buffer, int share_size, void *file_ptr, int model_size);
int JLSP_vad_reset(void *m);
int JLSP_vad_process(void *m, short *pcm, int *out_flag);
int JLSP_vad_free(void *m);

#endif

