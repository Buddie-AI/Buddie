#ifndef UI_PROGRESS_H
#define UI_PROGRESS_H


#include "ui/control.h"
#include "ui/ui_core.h"


#define PROGRESS_CHILD_NUM     (CTRL_PROGRESS_CHILD_END - CTRL_PROGRESS_CHILD_BEGIN)


struct progress_highlight_info {
    struct ui_ctrl_info_head head;
    u16 center_x;
    u16 center_y;
    u16 radius_big;
    u16 radius_small;
    u16 angle_begin;
    u16 angle_end;
    struct ui_image_list *img;
};

struct ui_progress {
    struct element elm;
    struct element child_elm[PROGRESS_CHILD_NUM];
    char source[8];
    u16 center_x;
    u16 center_y;
    u16 radius;
    u16 angle_begin;
    u16 angle_end;
    u8 ctrl_num;
    char percent;
    u8 *mask;
    u16 mask_len;
    void *timer;
    const struct layout_info *info;
    const struct progress_highlight_info *pic_info[PROGRESS_CHILD_NUM];
    const struct element_event_handler *handler;
};

void ui_progress_enable();
int ui_progress_set_persent_by_id(int id, int persent);
int ui_progress_set_persent(struct ui_progress *progress, int percent);

#endif


