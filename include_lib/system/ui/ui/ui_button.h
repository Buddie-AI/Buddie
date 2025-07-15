#ifndef UI_BUTTON_H
#define UI_BUTTON_H


#include "ui/control.h"
#include "ui/ui_core.h"

struct button {
    struct element elm;
    u8 image_index;
    u8 css_num;
    u32 css[2];
    const struct ui_button_info *info;
    const struct element_event_handler *handler;
};

void ui_button_enable();

#endif

