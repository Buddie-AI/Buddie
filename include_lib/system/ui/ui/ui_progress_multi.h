#ifndef UI_PROGRESS_MULTI_H
#define UI_PROGRESS_MULTI_H


#include "ui/control.h"
#include "ui/ui_core.h"


#define MULTIPROGRESS_CHILD_NUM     (CTRL_MULTIPROGRESS_CHILD_END - CTRL_MULTIPROGRESS_CHILD_BEGIN)

struct multiprogress_highlight_info {
    struct ui_ctrl_info_head head;
    u16 number;
    u16 center_x;
    u16 center_y;
    u16 radius0_big;
    u16 radius0_small;
    u16 radius1_big;
    u16 radius1_small;
    u16 radius2_big;
    u16 radius2_small;
    u16 angle_begin;
    u16 angle_end;
    struct ui_image_list *img;
};

struct ui_multiprogress {
    struct element elm;
    struct element child_elm[MULTIPROGRESS_CHILD_NUM];
    char source[8];
    u16 center_x;
    u16 center_y;
    u16 radius;
    u16 angle_begin;
    u16 angle_end;
    u8 ctrl_num;
    char percent[3];
    u8 circle_num;
    u8 index;
    u8 *mask;
    u16 mask_len;
    void *timer;
    const struct layout_info *info;
    const struct multiprogress_highlight_info *pic_info[MULTIPROGRESS_CHILD_NUM];
    const struct element_event_handler *handler;
};

void ui_multiprogress_enable();
int ui_multiprogress_set_persent_by_id(int id, int persent);
int ui_multiprogress_set_second_persent_by_id(int id, int percent);
int ui_multiprogress_set_third_persent_by_id(int id, int percent);

int ui_multiprogress_set_persent(struct ui_multiprogress *multiprogress, int percent);
int ui_multiprogress_set_second_persent(struct ui_multiprogress *multiprogress, int percent);
int ui_multiprogress_set_third_persent(struct ui_multiprogress *multiprogress, int percent);

#endif


