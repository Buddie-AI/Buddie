#ifndef __FRAME_QUEQUE_H
#define __FRAME_QUEQUE_H
#include<string.h>
#include <stdint.h>
#include "typedef.h"


typedef  unsigned char      WORD8;
typedef  unsigned short int WORD16;
typedef  unsigned int	    WORD32;
typedef  unsigned long long WORD64;

typedef  signed char        SWORD8;
typedef  signed short int 	SWORD16;
typedef  signed int	    	SWORD32;
typedef  signed long long 	SWORD64;

typedef  unsigned char uint8_t;

#define FRAME_QUEQUE_MUTEX_ID           semlock_t
#define FRAME_QUEQUE_MUTEX_DEF          semlock_t
#define FRAME_QUEQUE_MUTEX_P(mutex_id)  frame_mute_p(mutex_id)
#define FRAME_QUEQUE_MUTEX_V(mutex_id)  frame_mute_v(mutex_id)

typedef enum {
    APP_DUEROS_VER,
    APP_DUEROS_SEND,
    APP_TWS_BLE_SLAVE_SPEECH_START,
    APP_SPEECH_START_FROM_TWS,
    APP_SPEECH_STOP_FROM_TWS,
    APP_TWS_DUEROS_RAND_SET,
    APP_TWS_BLE_DUEROS_CONNECT,
    APP_TWS_BLE_DUEROS_DISCONNECT,
} APP_CMD_TYPE ;


typedef struct {
    int counter;
} semlock_t;

typedef struct send_frame {
    void *buffer;
    WORD32 len;
    struct send_frame *next;
} _GNU_PACKED_  SEND_FRAME;

typedef struct __frame_queque {
    SEND_FRAME *head;
    SEND_FRAME *tail;
    unsigned int depth;
    FRAME_QUEQUE_MUTEX_ID mutex;
} _GNU_PACKED_  FRAME_QUEQUE;


int frame_queque_is_empty(FRAME_QUEQUE *queque);
int frame_push_queque(FRAME_QUEQUE *queque, SEND_FRAME *frame);
SEND_FRAME *frame_pop_queque(FRAME_QUEQUE *queque);
SEND_FRAME *frame_pop_queque_alloc(FRAME_QUEQUE *queque);
int frame_queque_clear(FRAME_QUEQUE *queque);
int frame_queque_init(FRAME_QUEQUE *queque);

void run_loop_register_for_app();
void app_remove_stack_run(void);

#endif
