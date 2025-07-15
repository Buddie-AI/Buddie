#ifndef UI_CAMERA_H
#define UI_CAMERA_H

#include "ui/control.h"
#include "ui/ui_core.h"






struct ui_camera {
    struct element elm; 	//must be first
    int fd;
    const struct ui_camera_info *info;
    const struct element_event_handler *handler;
};



#define ui_camera_for_id(id) \
	(struct ui_camera*)ui_core_get_element_by_id(id)



void register_ui_camera_handler(const struct element_event_handler *handler);

int ui_camera_set_rect(int id, struct rect *r);




#endif

