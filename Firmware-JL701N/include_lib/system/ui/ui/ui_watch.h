#ifndef UI_WATCH_H
#define UI_WATCH_H


#include "ui/control.h"
#include "ui/ui_core.h"


#define WATCH_CHILD_NUM 	(CTRL_WATCH_CHILD_END - CTRL_WATCH_CHILD_BEGIN)


struct watch_pic_info {
    struct ui_ctrl_info_head head;
    u16 cent_x;
    u16 cent_y;
    struct ui_image_list *img;
};

struct watch_css_info {
    int left: 16;
    int top: 16;
    int width: 16;
    int height: 16;
};

struct ui_watch {
    struct element elm;
    struct element child_elm[WATCH_CHILD_NUM];
    struct watch_css_info child_css[WATCH_CHILD_NUM];
    char source[8];
    u8 hour;
    u8 min;
    u8 sec;
    u8 last_hour;
    u8 last_min;
    u8 last_sec;
    u8 updata;
    u8 ctrl_num;
    void *timer;
    const struct layout_info *info;
    const struct watch_pic_info *pic_info[WATCH_CHILD_NUM];
    const struct element_event_handler *handler;
};


void ui_watch_enable();
int ui_watch_set_time_by_id(int id, int hour, int min, int sec);
int ui_watch_set_time(struct ui_watch *watch, int hour, int min, int sec);

#endif


