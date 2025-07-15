#ifndef UI_SLIDER_VERT_H
#define UI_SLIDER_VERT_H

#include "ui/ui_core.h"
#include "ui/control.h"

#define VSLIDER_CHILD_NUM     (VSLIDER_CHILD_END - VSLIDER_CHILD_BEGIN)

struct vslider_text_info {
    u8 move;
    int min_value;
    int max_value;
    int text_color;
};


struct ui_vslider {
    struct element elm;
    struct element child_elm[VSLIDER_CHILD_NUM];
    u8 step;
    char persent;
    s16 top;
    s16 height;
    u16 min_value;
    u16 max_value;
    u16 text_color;
    const struct ui_slider_info *info;
    const struct vslider_text_info *text_info;
    const struct element_event_handler *handler;
};


void ui_vslider_enable();
int ui_vslider_set_persent_by_id(int id, int persent);
int ui_vslider_set_persent(struct ui_vslider *vslider, int persent);

int vslider_touch_slider_move(struct ui_vslider *vslider, struct element_touch_event *e);//触摸滑动功能

int vslider_get_percent(struct ui_vslider *vslider);
#endif

