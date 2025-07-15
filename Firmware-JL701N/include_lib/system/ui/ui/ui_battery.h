#ifndef UI_BATTERY_H
#define UI_BATTERY_H


#include "ui/control.h"
#include "list.h"






struct ui_battery {
    struct element elm;
    int src;
    u8 index;
    u16 charge_image;
    u16 normal_image;
    struct list_head entry;
    const struct ui_battery_info *info;
    const struct element_event_handler *handler;
};

void ui_battery_enable();
void ui_battery_level_change(int persent, int incharge);//改变所有电池控件
int ui_battery_set_level_by_id(int id, int persent, int incharge);//修改指定id
int ui_battery_set_level(struct ui_battery *battery, int persent, int incharge);//初始化使用

#endif
