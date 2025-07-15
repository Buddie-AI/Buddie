#ifndef UI_ELEMENT_CORE_H
#define UI_ELEMENT_CORE_H

#include "typedef.h"
#include "rect.h"
#include "system/event.h"
// #include "fs/fs.h"
#include "res/resfile.h"


#define UI_CTRL_BUTTON  0

struct element;


#ifdef offsetof
#undef offsetof
#endif
#ifdef container_of
#undef container_of
#endif

#define offsetof(type, memb) \
((unsigned long)(&((type *)0)->memb))

#define container_of(ptr, type, memb) \
((type *)((char *)ptr - offsetof(type, memb)))

enum ui_direction {
    UI_DIR_UP,
    UI_DIR_DOWN,
    UI_DIR_LEFT,
    UI_DIR_RIGHT,
};

enum ui_align {
    UI_ALIGN_LEFT = 0,
    UI_ALIGN_CENTER,
    UI_ALIGN_RIGHT,
};


enum {
    POSITION_ABSOLUTE = 0,
    POSITION_RELATIVE = 1,
};

enum {
    ELM_EVENT_TOUCH_DOWN,
    ELM_EVENT_TOUCH_MOVE,
    ELM_EVENT_TOUCH_R_MOVE,
    ELM_EVENT_TOUCH_L_MOVE,
    ELM_EVENT_TOUCH_D_MOVE,
    ELM_EVENT_TOUCH_U_MOVE,
    ELM_EVENT_TOUCH_HOLD,
    ELM_EVENT_TOUCH_UP,
};


enum {
    ELM_EVENT_KEY_CLICK,
    ELM_EVENT_KEY_LONG,
    ELM_EVENT_KEY_HOLD,
};

enum {
    ELM_STA_INITED,
    //ELM_STA_SHOW_PROBE,
    //ELM_STA_SHOW_POST,
    ELM_STA_HIDE,
    ELM_STA_SHOW,
    ELM_STA_PAUSE,
};

enum {
    ELM_FLAG_NORMAL,
    ELM_FLAG_HEAD,
};

enum {
    DC_DATA_FORMAT_OSD8 = 0,
    DC_DATA_FORMAT_YUV420 = 1,
    DC_DATA_FORMAT_OSD16 = 2,
    DC_DATA_FORMAT_OSD8A = 3,
    DC_DATA_FORMAT_MONO = 4,
};


struct element_touch_event {
    int event;
    int xoffset;
    int yoffset;
    u8  hold_up;
    u8  onfocus;
    u8  move_dir;
    struct position pos;
    struct position mov;
    void *private_data;
};

struct element_key_event {
    u8 event;
    u8 value;
    void *private_data;
};

#define ELM_KEY_EVENT(e) 		(0x0000 | (e->event) | (e->value << 8))
#define ELM_TOUCH_EVENT(e) 		(0x1000 | (e->event))
#define ELM_CHANGE_EVENT(e) 	(0x2000 | (e->event))

enum element_change_event {
    ON_CHANGE_INIT_PROBE,
    ON_CHANGE_INIT,
    ON_CHANGE_TRY_OPEN_DC,
    ON_CHANGE_FIRST_SHOW,
    ON_CHANGE_SHOW_PROBE,
    ON_CHANGE_SHOW,
    ON_CHANGE_SHOW_POST,
    ON_CHANGE_HIDE,
    ON_CHANGE_HIGHLIGHT,
    ON_CHANGE_RELEASE_PROBE,
    ON_CHANGE_RELEASE,
    ON_CHANGE_ANIMATION_END,
    ON_CHANGE_SHOW_COMPLETED,
    ON_CHANGE_UPDATE_ITEM,
};


struct element_event_handler {
    int id;
    int (*ontouch)(void *, struct element_touch_event *);
    int (*onkey)(void *, struct element_key_event *);
    int (*onchange)(void *, enum element_change_event, void *);
};

struct jaction {
    u32 show;
    u32 hide;
};

enum {
    ELM_ACTION_HIDE = 0,
    ELM_ACTION_SHOW,
    ELM_ACTION_TOGGLE,
    ELM_ACTION_HIGHLIGHT,
};

struct event_action {
    u16 event;
    u16 action;
    int id;
    u8  argc;
    char argv[];
};

struct element_event_action {
    u16 num;
    struct event_action action[0];
};

struct image_preview {
    RESFILE *file;
    int id;
    int page;
};


struct image {
    int x;
    int y;
    int id;
    int page;
    int en;
};

struct draw_context {
    u8 ref;
    u8 alpha;
    u8 align;
    u8 data_format;
    u8 prj;
    u8 page;
    u8 buf_num;
    u32 background_color;
    void *handl;
    struct element *elm;
    struct rect rect;
    struct rect draw;
    void *dc;

    struct image_preview preview;

    struct rect need_draw;
    struct rect disp;
    u16 width;
    u16 height;
    u8 *fbuf;
    u32 fbuf_len;
    u8 *buf;
    u8 *buf0;
    u8 *buf1;
    u32 len;
    u16 lines;
    u8 col_align;
    u8 row_align;

    struct image draw_img;

    u8 *mask;
};

struct css_border {
    u16 left: 4;
    u16 top: 4;
    u16 right: 4;
    u16 bottom: 4;
    u16 color: 16;
};

struct css_border1 {
    u8 left;
    u8 top;
    u8 right;
    u8 bottom;
    int color: 24;
};

struct element_css {
    u8  align: 2;
    u8  invisible: 1;
    u8  z_order: 5;
    int left/* : 16 */;
    int top/* : 16 */;
    int width/* : 16 */;
    int height/* : 16 */;
    u32 background_color: 24;
    u32 alpha: 8;
    int background_image: 24;
    int image_quadrant: 8;
    struct css_border border;
};

struct element_css1 {
    u8  align;
    u8  invisible;
    u8  z_order;
    int left;
    int top;
    int width;
    int height;
    u32 background_color: 24;
    u32 alpha: 8;
    int background_image: 24;
    int image_quadrant: 8;
    struct css_border1 border;
};

struct element_ops {
    int (*show)(struct element *);
    int (*redraw)(struct element *, struct rect *);
};

struct element {
    u32 highlight: 1;
    u32 state: 2;
    u32 ref: 5;
    u32 prj: 3;
    u32 page: 21;
    // u32 alive;
    int id;
    struct element *parent;
    struct list_head sibling;
    struct list_head child;
    struct element *focus;
    struct element_css css;
    struct draw_context *dc;
    // const struct element_ops *ops;
    const struct element_event_handler *handler;
    // const struct element_event_action *action;
};

struct ui_style {
    const char *file;
    u32 version;
};

enum {
    UI_FTYPE_VIDEO = 0,
    UI_FTYPE_IMAGE,
    UI_FTYPE_AUDIO,
    UI_FTYPE_DIR,
    UI_FTYPE_UNKNOW = 0xff,
};

struct ui_file_attrs {
    char *format;
    char fname[128];
    struct vfs_attr attr;
    u8 ftype;
    u16 file_num;
    u32 film_len;
};

struct ui_image_attrs {
    u16 width;
    u16 height;
};

struct ui_text_attrs {
    const char *str;
    const char *format;
    int color;
    u16 strlen;
    u16 offset;
    u8  encode: 2;
    u8  endian: 1;
    u8  flags: 5;
    // u16  offset;
    u16  displen;
};

struct ui_file_browser {
    int file_number;
    u8  dev_num;
    void *private_data;
};

#define ELEMENT_ALIVE 		0x53547a7b

#define element_born(elm) \
		elm->alive = ELEMENT_ALIVE

#define element_alive(elm) \
		(elm->alive == ELEMENT_ALIVE)


#define list_for_each_child_element(p, elm) \
	list_for_each_entry(p, &(elm)->child, sibling)

#define list_for_each_child_element_reverse(p, n, elm) \
	list_for_each_entry_reverse_safe(p, n, &(elm)->child, sibling)

#define list_for_each_child_element_safe(p, n, elm) \
	list_for_each_entry_safe(p, n, &(elm)->child, sibling)

struct ui_platform_api {
    void *(*malloc)(int);
    void (*free)(void *);

    int (*load_style)(struct ui_style *);

    void *(*load_window)(int id);
    void (*unload_window)(void *);

    int (*open_draw_context)(struct draw_context *);
    int (*get_draw_context)(struct draw_context *);
    int (*put_draw_context)(struct draw_context *);
    int (*set_draw_context)(struct draw_context *);
    int (*close_draw_context)(struct draw_context *);

    int (*fill_rect)(struct draw_context *, u32 color);
    int (*draw_rect)(struct draw_context *, struct css_border *border);
    int (*draw_image)(struct draw_context *, u32 src, u8 quadrant, u8 *mask);
    int (*draw_point)(struct draw_context *, u16 x, u16 y, u32 color);
    u32(*read_point)(struct draw_context *dc, u16 x, u16 y);
    int (*invert_rect)(struct draw_context *, u32 color);

    void *(*load_widget_info)(void *_head, u8 page);
    void *(*load_css)(u8 page, void *_css);
    void *(*load_image_list)(u8 page, void *_list);
    void *(*load_text_list)(u8 page, void *__list);

    //int (*highlight)(struct draw_context *);
    int (*show_text)(struct draw_context *, struct ui_text_attrs *);
    int (*read_image_info)(struct draw_context *, u32, u8, struct ui_image_attrs *);

    int (*open_device)(struct draw_context *, const char *device);
    int (*close_device)(int);

    void *(*set_timer)(void *, void (*callback)(void *), u32 msec);
    int (*del_timer)(void *);

    struct ui_file_browser *(*file_browser_open)(struct rect *r,
            const char *path, const char *ftype, int show_mode);

    int (*get_file_attrs)(struct ui_file_browser *, struct ui_file_attrs *attrs);

    int (*set_file_attrs)(struct ui_file_browser *, struct ui_file_attrs *attrs);

    int (*clear_file_preview)(struct ui_file_browser *, struct rect *r);

    int (*show_file_preview)(struct ui_file_browser *, struct rect *r, struct ui_file_attrs *attrs);

    int (*flush_file_preview)(struct ui_file_browser *);

    void *(*open_file)(struct ui_file_browser *, struct ui_file_attrs *attrs);
    int (*delete_file)(struct ui_file_browser *, struct ui_file_attrs *attrs);

    int (*move_file_preview)(struct ui_file_browser *_bro, struct rect *dst, struct rect *src);

    void (*file_browser_close)(struct ui_file_browser *);

};

extern /* const */ struct ui_platform_api *platform_api;

extern /* const */ struct element_event_handler dumy_handler;

struct janimation {
    u8  persent[5];
    u8  direction;
    u8  play_state;
    u8  iteration_count;
    u16 delay;
    u16 duration;
    struct element_css css[0];
};


extern struct element_event_handler *elm_event_handler_begin;
extern struct element_event_handler *elm_event_handler_end;


#define ___REGISTER_UI_EVENT_HANDLER(style, _id) \
    static const struct element_event_handler element_event_handler_##_id \
			sec(.elm_event_handler_##style) __attribute__((used)) = { \
				.id = _id,

#define __REGISTER_UI_EVENT_HANDLER(style, _id) \
		___REGISTER_UI_EVENT_HANDLER(style, _id)

#define REGISTER_UI_EVENT_HANDLER(id) \
	__REGISTER_UI_EVENT_HANDLER(STYLE_NAME, id)



struct ui_style_info {
    const char *name;
    struct element_event_handler *begin;
    struct element_event_handler *end;
};

extern struct ui_style_info ui_style_begin[];
extern struct ui_style_info ui_style_end[];

#define __REGISTER_UI_STYLE(style_name) \
	extern struct element_event_handler elm_event_handler_begin_##style_name[]; \
	extern struct element_event_handler elm_event_handler_end_##style_name[]; \
    static const struct ui_style_info ui_style_##style_name sec(.ui_style) __attribute__((used)) = { \
		.name 	= #style_name, \
		.begin 	= elm_event_handler_begin_##style_name, \
		.end 	= elm_event_handler_end_##style_name, \
	};

#define REGISTER_UI_STYLE(style_name) \
	__REGISTER_UI_STYLE(style_name)


static inline struct element_event_handler *element_event_handler_for_id(u32 id)
{
    struct element_event_handler *p;

    for (p = elm_event_handler_begin; p < elm_event_handler_end; p++) {
        if (p->id == id) {
            return p;
        }
    }

    return NULL;
}




#define ui_core_get_element_css(elm)   \
	&((struct element *)(elm))->css

#define ui_core_element_invisable(elm, i)  \
		((struct element *)(elm))->css.invisible = i


int ui_core_init(const struct ui_platform_api *api, struct rect *rect);

int ui_core_set_style(const char *style);

void ui_core_set_rotate(int _rotate);

int ui_core_get_rotate();


void *ui_core_malloc(int size);

void ui_core_free(void *buf);

void ui_core_element_init(struct element *,
                          u32 id,
                          u8 page,
                          u8 prj,
                          /* const */ struct element_css1 *,
                          const struct element_event_handler *,
                          const struct element_event_action *);

void ui_core_get_element_abs_rect(struct element *elm, struct rect *rect);

void ui_core_append_child(void *_child);

struct element *ui_core_get_first_child();

void ui_core_remove_element(void *_child);


int ui_core_open_draw_context(struct draw_context *dc, struct element *elm);

int ui_core_close_draw_context(struct draw_context *dc);

int ui_core_show(void *_elm, int init);

int ui_core_hide(void *_elm);

struct element *get_element_by_id(struct element *elm, u32 id);

struct element *ui_core_get_element_by_id(u32 id);
int ui_core_get_disp_status_by_id(u32 id);

struct element *ui_core_get_up_element(struct element *elm);
struct element *ui_core_get_down_element(struct element *elm);
struct element *ui_core_get_left_element(struct element *elm);
struct element *ui_core_get_right_element(struct element *elm);

int ui_core_element_ontouch(struct element *, struct element_touch_event *e);

int ui_core_ontouch(struct element_touch_event *e);

int ui_core_element_onkey(struct element *elm, struct element_key_event *e);

int ui_core_onkey(struct element_key_event *e);

void ui_core_element_append_child(struct element *parent, struct element *child);

struct element_css *ui_core_set_element_css(void *_elm, const struct element_css1 *css);

int ui_core_invert_rect(struct draw_context *dc);

void ui_core_release_child_probe(struct element *elm);

void ui_core_release_child(struct element *elm);


int ui_core_redraw(void *_elm);

int ui_core_highlight_element(struct element *elm, int yes);

void ui_core_element_on_focus(struct element *elm, int yes);


void ui_core_ontouch_lose_focus(struct element *elm);

void ui_core_ontouch_lock(struct element *elm);

void ui_core_ontouch_unlock(struct element *elm);

int ui_core_get_draw_context(struct draw_context *dc, struct element *elm, struct rect *draw);
#endif



