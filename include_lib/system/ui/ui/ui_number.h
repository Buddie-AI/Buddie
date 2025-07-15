#ifndef UI_NUMBER_H
#define UI_NUMBER_H


#include "ui/control.h"
#include "ui/ui_core.h"
#include "ui/p.h"

enum {
    TYPE_NUM,
    TYPE_STRING,
};

struct unumber {
    u8 numbs;
    u8 type;
    u32 number[2];
    u8 *num_str;
};

struct ui_number {
    struct element_text text;
    char source[8];
    u16 number[2];
    u16 buf[20];

    int color;
    int hi_color;
    u8 css_num;
    u8 nums: 6;
    u8 type: 2;
    u32 css[2];
    u8 *num_str;
    const struct ui_number_info *info;
    const struct element_event_handler *handler;
};

void ui_number_enable();
void *new_ui_number(const void *_info, struct element *parent);
int ui_number_update(struct ui_number *number, struct unumber *n);
int ui_number_update_by_id(int id, struct unumber *n);

#endif

