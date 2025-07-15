/*****************************************************************
>file name : user_asr.h
>create time : Thu 23 Dec 2021 11:53:40 AM CST
*****************************************************************/
#ifndef _USER_ASR_H_
#define _USER_ASR_H_

int user_platform_asr_open(void);

void user_platform_asr_close(void);

int user_asr_core_handler(void *priv, int taskq_type, int *msg);

#endif

