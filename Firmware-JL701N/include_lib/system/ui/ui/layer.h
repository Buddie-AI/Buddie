#ifndef LAYER_H
#define LAYER_H


#include "ui/layout.h"
#include "ui/control.h"


struct layer {
    struct element elm; 		//must be first
    u8 hide;
    u8 inited;
    u8 highlight;
    u8 ctrl_num;
    u8 css_num;
    u32 css[2];
    struct draw_context dc;
    struct layout *layout;
    const struct layer_info *info;
    const struct element_event_handler *handler;
};


#define layer_for_id(id) \
		(struct layer *)ui_core_get_element_by_id(id);


struct layer *layer_new(struct layer_info *info, int num, struct element *parent);


void layer_delete_probe(struct layer *layer, int num);

void layer_delete(struct layer *layer, int num);

int layer_show(int id);

int layer_hide(int id);

int layer_toggle(int id);









#endif




