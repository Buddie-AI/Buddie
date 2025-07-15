#ifndef UI_BROWSER_H
#define UI_BROWSER_H



#include "ui/ui_core.h"
#include "ui/control.h"




struct ui_browser {
    struct element elm;
    struct ui_file_browser *hdl;
    char order; // 1 表示 正序， 非1 表示反序
    u8 inited;
    u8 hide_byself;
    u8 item_num;
    u8 highlight;
    u8 show_mode;
    u16 cur_number;
    u16 file_number;
    struct ui_grid *grid;
    const char *path;
    const char *ftype;
    const struct ui_browser_info *info;
    const struct element_event_handler *handler;
};


#define ui_file_browser_cur_item(bro)   ui_grid_cur_item(((struct ui_browser *)bro)->grid)


int ui_file_browser_page_num(struct ui_browser *bro);

int ui_file_browser_cur_page(struct ui_browser *bro, int *file_num);

int ui_file_browser_set_page(struct ui_browser *bro, int page);

int ui_file_browser_set_page_by_id(int id, int page);

int ui_file_browser_next_page(struct ui_browser *bro);

int ui_file_browser_next_page_by_id(int id);

int ui_file_browser_prev_page(struct ui_browser *bro);

int ui_file_browser_prev_page_by_id(int id);

int ui_file_browser_set_dir(struct ui_browser *bro, const char *path, const char *ftype);

int ui_file_browser_set_dir_by_id(int id, const char *path, const char *ftype);

int ui_file_browser_get_file_attrs(struct ui_browser *bro, int item,
                                   struct ui_file_attrs *attrs);

int ui_file_browser_set_file_attrs(struct ui_browser *bro, int item,
                                   struct ui_file_attrs *attrs);

void *ui_file_browser_open_file(struct ui_browser *bro, int item);


int ui_file_browser_del_file(struct ui_browser *bro, int item);

int ui_file_browser_highlight_item(struct ui_browser *bro, int item, bool yes);

void *ui_file_browser_get_child_by_id(struct ui_browser *bro, int item, int id);
























#endif
