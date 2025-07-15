#ifndef UI_CONTROL_H
#define UI_CONTROL_H

#include "ui/ui_core.h"

union ui_control_info;
struct layout_info;

#define CTRL_TYPE_WINDOW            2
#define CTRL_TYPE_LAYOUT            3
#define CTRL_TYPE_LAYER             4
#define CTRL_TYPE_GRID              5
#define CTRL_TYPE_LIST              6
#define CTRL_TYPE_BUTTON            7
#define CTRL_TYPE_PIC               8
#define CTRL_TYPE_BATTERY           9
#define CTRL_TYPE_TIME              10
#define CTRL_TYPE_CAMERA_VIEW       11
#define CTRL_TYPE_TEXT              12
#define CTRL_TYPE_ANIMATION         13
#define CTRL_TYPE_PLAYER            14
#define CTRL_TYPE_NUMBER            15

#define CTRL_TYPE_PROGRESS          20
#define CTRL_PROGRESS_CHILD_BEGIN   (CTRL_TYPE_PROGRESS + 1)
#define CTRL_PROGRESS_CHILD_HIGHLIGHT   (CTRL_PROGRESS_CHILD_BEGIN) //21
#define CTRL_PROGRESS_CHILD_END     (CTRL_PROGRESS_CHILD_BEGIN + 1)

#define CTRL_TYPE_MULTIPROGRESS          22
#define CTRL_MULTIPROGRESS_CHILD_BEGIN   (CTRL_TYPE_MULTIPROGRESS + 1)
#define CTRL_MULTIPROGRESS_CHILD_HIGHLIGHT   (CTRL_MULTIPROGRESS_CHILD_BEGIN)//23
#define CTRL_MULTIPROGRESS_CHILD_END     (CTRL_MULTIPROGRESS_CHILD_BEGIN + 1)

#define CTRL_TYPE_WATCH             24
#define CTRL_WATCH_CHILD_BEGIN      (CTRL_TYPE_WATCH + 1)
#define CTRL_WATCH_CHILD_HOUR       (CTRL_WATCH_CHILD_BEGIN)//25
#define CTRL_WATCH_CHILD_MIN        (CTRL_WATCH_CHILD_BEGIN+1)//26
#define CTRL_WATCH_CHILD_SEC        (CTRL_WATCH_CHILD_BEGIN+2)//27
#define CTRL_WATCH_CHILD_END        (CTRL_WATCH_CHILD_BEGIN+3)


#define CTRL_TYPE_SLIDER            28

#define SLIDER_CHILD_BEGIN          (CTRL_TYPE_SLIDER+1)
#define SLIDER_CHILD_UNSELECT_PIC   (SLIDER_CHILD_BEGIN)//29
#define SLIDER_CHILD_SELECTED_PIC   (SLIDER_CHILD_BEGIN+1)//30
#define SLIDER_CHILD_SLIDER_PIC     (SLIDER_CHILD_BEGIN+2)//31
#define SLIDER_CHILD_PERSENT_TEXT   (SLIDER_CHILD_BEGIN+3)//32
#define SLIDER_CHILD_END            (SLIDER_CHILD_BEGIN+4)


#define CTRL_TYPE_VSLIDER            33

#define VSLIDER_CHILD_BEGIN          (CTRL_TYPE_VSLIDER+1)
#define VSLIDER_CHILD_UNSELECT_PIC   (VSLIDER_CHILD_BEGIN)//34
#define VSLIDER_CHILD_SELECTED_PIC   (VSLIDER_CHILD_BEGIN+1)//35
#define VSLIDER_CHILD_SLIDER_PIC     (VSLIDER_CHILD_BEGIN+2)//36
#define VSLIDER_CHILD_PERSENT_TEXT   (VSLIDER_CHILD_BEGIN+3)//37
#define VSLIDER_CHILD_END            (VSLIDER_CHILD_BEGIN+4)


struct ui_ctrl_info_head {
    u8 		type;
    u8 		ctrl_num;
    u8 		css_num;
    u8 		len;
    u8 		page;
    u8 		rev[3];
    int 	id;
    struct 	element_css1 *css;
};

struct ui_image_list {
    u16 num;
    u16 image[0];
};

struct ui_text_list {
    u16 num;
    char str[0];
};


struct ui_image_list_t {
    u16 num;
    u16 image[64];
};

#define UI_TEXT_LIST_MAX_NUM    3
struct ui_text_list_t {
    u16 num;
    u16 str[50];
};

struct ui_button_info {
    struct ui_ctrl_info_head 	head;
    struct element_event_action *action;
};



struct ui_camera_info {
    struct ui_ctrl_info_head head;
    char device[8];
    struct element_event_action *action;
};

struct ui_player_info {
    struct ui_ctrl_info_head head;
    char device[8];
    struct element_event_action *action;
};

struct ui_time_info {
    struct ui_ctrl_info_head head;
    char source[8];
    u8 auto_cnt;
    u8 rev[3];
    char format[16];
    int color;
    int hi_color;
    u16 number[10];
    u16 delimiter[10];
    struct element_event_action *action;
};

struct ui_number_info {
    struct ui_ctrl_info_head head;
    char source[8];
    char format[16];
    int color;
    int hi_color;
    u16 number[10];
    u16 delimiter[10];
    u16 space[2];
    struct element_event_action *action;
};


struct ui_pic_info {
    struct ui_ctrl_info_head head;
    u8 highlight;
    u16 cent_x;
    u16 cent_y;
    struct ui_image_list *normal_img;
    struct ui_image_list *highlight_img;
    struct element_event_action *action;
};


struct ui_battery_info {
    struct ui_ctrl_info_head head;
    struct ui_image_list *normal_image;
    struct ui_image_list *charge_image;
    struct element_event_action *action;
};


struct ui_text_info {
    struct ui_ctrl_info_head head;
    char source[8];
    char code[8];
    int color;
    int highlight_color;
    struct ui_text_list *str;
    struct element_event_action *action;
};


struct ui_grid_info {
    struct ui_ctrl_info_head head;
    u8 page_mode;
    char highlight_index;
    struct element_event_action *action;
    struct layout_info *info;
};

struct ui_animation_info {
    struct ui_ctrl_info_head head;
    u16 loop_num;
    u32 interval;
    struct ui_image_list *img;
    struct element_event_action *action;
};

struct ui_slider_info {
    struct ui_ctrl_info_head head;
    u8 step;
    struct ui_ctrl_info_head *ctrl;
    // struct element_event_action *action;
};

struct ui_vslider_info {
    struct ui_ctrl_info_head head;
    u8 step;
    struct ui_ctrl_info_head *ctrl;
    // struct element_event_action *action;
};

struct ui_watch_info {
    struct ui_ctrl_info_head head;
    char source[8];
    struct element_event_action *action;
    struct ui_ctrl_info_head *ctrl;
};

struct ui_progress_info {
    struct ui_ctrl_info_head head;
    char source[8];
    struct element_event_action *action;
    struct ui_ctrl_info_head *ctrl;
};

struct ui_multiprogress_info {
    struct ui_ctrl_info_head head;
    char source[8];
    struct element_event_action *action;
    struct ui_ctrl_info_head *ctrl;
};

struct ui_browser_info {
    struct ui_ctrl_info_head head;
    u8 row;
    u8 column;
    u8 interval;
    u8 scroll;
    u8 auto_highlight;
    struct element_event_action *action;
    struct ui_ctrl_info_head *ctrl;
};

struct ui_fattrs_info {
    struct ui_ctrl_info_head head;
    struct element_event_action *action;
    struct ui_ctrl_info_head *ctrl;
};

struct layout_info {
    struct ui_ctrl_info_head head;
    struct element_event_action *action;
    union ui_control_info *ctrl;
};


struct layer_info {
    struct ui_ctrl_info_head head;
    u8 format;
    struct element_event_action *action;
    struct layout_info *layout;
};


union ui_control_info {
    struct ui_ctrl_info_head head;//16 bytes
    struct ui_button_info   button;//20 bytes
    struct ui_camera_info   camera;//28 bytes
    struct ui_time_info     time;//84 bytes
    struct ui_number_info   number;
    struct ui_pic_info      pic;//36 bytes
    struct ui_battery_info  battery;//28 bytes
    struct ui_text_info     text;//40 bytes
    struct ui_grid_info     grid;//28 bytes
    struct layer_info       layer;
    struct layout_info      layout;
    struct ui_watch_info    watch;
    struct ui_progress_info progress;
    struct ui_multiprogress_info multiprogress;
    struct ui_slider_info   slider;
    struct ui_vslider_info  vslider;
};//84 bytes

// union ui_control_info {
// struct ui_ctrl_info_head head;
// struct ui_button_info 	button;
// struct ui_camera_info 	camera;
// struct ui_time_info 	time;
// struct ui_number_info   number;
// struct ui_pic_info 		pic;
// struct ui_battery_info 	battery;
// struct ui_text_info 	text;
// struct ui_grid_info  	grid;
// };


struct window_info {
    u8 		type;
    u8 		ctrl_num;
    u8 		css_num;
    u8 		len;
    u8 		rev[4];
    struct rect rect;
    struct layer_info *layer;
//	struct element_event_action *action;
};

struct control_ops {
    int type;
    void *(*new)(const void *, struct element *);
    /*int (*delete)(void *);*/
};

extern const struct control_ops control_ops_begin[];
extern const struct control_ops control_ops_end[];


#define REGISTER_CONTROL_OPS(_type)  \
	static const struct control_ops control_ops_##_type sec(.control_ops) __attribute__((used)) = { \
		.type = _type,



#define get_control_ops_by_type(_type) \
		({  \
		 	const struct control_ops *ops, *ret=NULL; \
			for (ops = control_ops_begin; ops < control_ops_end; ops++) { \
		 			if (ops->type == _type) { \
		 				ret = ops; \
						break; \
		 			} \
		 	}\
		 	ret; \
		 })


#if 0
struct control_event_header {
    int id;
    int len;
};

extern struct control_event_header control_event_handler_begin[];
extern struct control_event_header control_event_handler_end[];


#define REGISTER_CONTROL_EVENT_HANDLER(control, _id) \
	static const struct control##_event_handler __##control##_event_handler_##_id \
			sec(.control_event_handler) = { \
				.header = { \
					.id = _id, \
					.len = sizeof(struct control##_event_handler), \
				}, \




static inline void *control_event_handler_for_id(int id)
{
    struct control_event_header *p;

    for (p = control_event_handler_begin; p < control_event_handler_end;) {
        if (p->id == id) {
            return p;
        }
        p = (u8 *)p + p->len;
    }

    return NULL;
}
#endif








#endif



