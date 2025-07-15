#ifndef UI_PIC_H
#define UI_PIC_H

#include "ui/ui_core.h"




struct ui_pic {
    struct element elm;
    u8 index;
    // u8 css_num:2;
    // u32 css[2];
    // u16 highlight_img;
    // u16 normal_img;
    // u16 highlight_img_num:8;
    // u16 normal_img_num:8;
    const struct ui_pic_info *info;
    const struct element_event_handler *handler;
};

void ui_pic_enable();
void *new_ui_pic(const void *_info, struct element *parent);
int ui_pic_show_image_by_id(int id, int index);
int ui_pic_set_image_index(struct ui_pic *pic, int index);
int ui_pic_get_normal_image_number_by_id(int id);
int ui_pic_get_highlgiht_image_number_by_id(int id);
int ui_pic_set_hide_by_id(int id, int hide);

#endif
