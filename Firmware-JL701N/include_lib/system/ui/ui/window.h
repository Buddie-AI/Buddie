#ifndef UI_WINDOW_H
#define UI_WINDOW_H


#include "ui/layer.h"
#include "ui/ui_core.h"
#include "ui/control.h"
#include "list.h"





struct window {
    struct element elm; 	//must be first
    u8 busy;
    u8 hide;
    u8 ctrl_num;
    struct list_head entry;
    struct layer *layer;
    const struct window_info *info;
    const struct element_event_handler *handler;
    void *private_data;
};


extern const struct window_info *window_table;



#define REGISTER_WINDOW_EVENT_HANDLER(id) \
	REGISTER_UI_EVENT_HANDLER(id)


int window_show(int);

int window_hide(int id);

int window_toggle(int id);

int window_ontouch(struct element_touch_event *e);

int window_onkey(struct element_key_event *e);


#endif


