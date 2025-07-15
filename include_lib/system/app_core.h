#ifndef SYS_APPLICATION_H
#define SYS_APPLICATION_H

#include "typedef.h"
#include "generic/list.h"
#include "system/event.h"


#define ACTION_BACK 				0x0a1b2c00
#define ACTION_STOP 				0x0a1b2c01
#define ACTION_DO_NOTHING           0x0a1b2c02
#define ACTION_TONE_PLAY            0x0a1b2c03
#define ACTION_CLASS_MASK 			0xfffff000



enum app_state {
    APP_STA_CREATE,
    APP_STA_START,
    APP_STA_PAUSE,
    APP_STA_RESUME,
    APP_STA_STOP,
    APP_STA_DESTROY,
};

struct application;


struct intent {
    const char *name;
    int action;
    const char *data;
    u32 exdata;
};


struct application_operation {
    int (*state_machine)(struct application *, enum app_state, struct intent *);
    int (*event_handler)(struct application *, struct sys_event *);
};


struct application {
    u8 	state;
    int action;
    char *data;
    const char *name;
    struct list_head entry;
    void *private_data;
    const struct application_operation *ops;
};



#define REGISTER_APPLICATION(at) \
	static struct application at sec(.app)


#define init_intent(it) \
	do { \
		(it)->name = NULL; \
		(it)->action= 0; \
		(it)->data = NULL; \
		(it)->exdata = 0; \
	}while (0)



void register_app_event_handler(int (*handler)(struct sys_event *));

struct application *get_current_app();

struct application *get_prev_app();

void app_core_back_to_prev_app();

int start_app(struct intent *it);

int start_app_async(struct intent *it, void (*callback)(void *p, int err), void *p);





#endif

