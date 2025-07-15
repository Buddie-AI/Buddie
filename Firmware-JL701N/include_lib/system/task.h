#ifndef TASK_PRIORITY_H
#define TASK_PRIORITY_H


#include "os/os_api.h"


struct task_info {
    const char *name;
    u8 prio;
    u8 core;
    u16 stack_size;
    u16 qsize;
};



typedef OS_SEM  	sem_t;
typedef OS_MUTEX  	mutex_t;


int task_create(void (*task)(void *p), void *p, const char *name);


int task_exit(const char *name);

int task_delete(const char *name);

int task_kill(const char *name);










#endif


