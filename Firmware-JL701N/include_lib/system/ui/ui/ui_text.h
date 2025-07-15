#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "ui/ui_core.h"
#include "ui/control.h"
#include "font/font_all.h"


struct ui_text {
    struct element elm;
    struct ui_text_attrs attrs;
    char source[8];
    u16 timer;
    u16 _str[UI_TEXT_LIST_MAX_NUM];
    char _format[7];
    u8 str_num;
    u8 index;
    // u8 str_num:4;
    // u8 css_num:4;
    // u32 css[2];
    // u16 attr_color;
    // u16 attr_highlight_color;
    // struct ui_text_attrs attrs;
    const struct ui_text_info *info;
    const struct element_event_handler *handler;
};



void ui_text_enable();
void *new_ui_text(const void *_info, struct element *parent);
/*api of format 'ascii'*/
int ui_text_set_str(struct ui_text *text, const char *format, const char *str, int strlen, u32 flags);
int ui_text_set_str_by_id(int id, const char *format, const char *str);
/*api of format 'strpic'*/
int ui_text_set_index(struct ui_text *text, int index);
int ui_text_show_index_by_id(int id, int index);
/*api of format 'text'*/
void ui_text_set_text_attrs(struct ui_text *text, const char *str, int strlen, u8 encode, u8 endian, u32 flags);
int ui_text_set_text_by_id(int id, const char *str, int strlen, u32 flags);
int ui_text_set_textw_by_id(int id, const char *str, int strlen, int endian, u32 flags);
int ui_text_set_textu_by_id(int id, const char *str, int strlen, u32 flags);

void text_release(struct ui_text *text);
/*
 * 注意：
 *   1.store_buf必须是全局或者静态，不能是局部,大小为index_num+1
 *   2.index_buf表示当前文本控件字符串id的序号，从0开始
 *   3.index_num表示有多少个字符串id拼起来
 * */
int ui_text_set_combine_index(struct ui_text *text, u16 *store_buf, u8 *index_buf, int index_num);


int ui_text_set_hide_by_id(int id, int hide);

#endif
