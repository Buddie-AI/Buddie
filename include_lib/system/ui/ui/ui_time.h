#ifndef UI_TIME_H
#define UI_TIME_H


#include "ui/control.h"
#include "ui/ui_core.h"
#include "ui/p.h"

struct utime {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
};

struct ui_time {
    struct element_text text;
    char source[8];
    u16 year: 12;
    u16 month: 4;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
    u8 css_num;
    u8 auto_cnt;
    u32 css[2];
    int color;
    int hi_color;
    u16 buf[20];
    void *timer;
    const struct ui_time_info *info;
    const struct element_event_handler *handler;
};

void ui_time_enable();
void *new_ui_time(const void *_info, struct element *parent);

int ui_time_update(struct ui_time *time, struct utime *t);
int ui_time_update_by_id(int id, struct utime *time);

#endif
