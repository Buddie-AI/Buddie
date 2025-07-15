#ifndef _UI_API_H_
#define _UI_API_H_

#include "app_config.h"
#include "ui/lcd_spi/lcd_drive.h"
#include "ui/ui.h"
#include "ui/ui_style.h"

#define GRT_CUR_MENU    (0)
#define GET_MAIN_MENU   (1)


enum ui_devices_type {
    LED_7,
    LCD_SEG3X9,
    TFT_LCD,//彩屏
    DOT_LCD,//点阵屏
};

//板级配置数据结构
struct ui_devices_cfg {
    enum ui_devices_type type;
    void *private_data;
};


struct touch_event {
    int event;
    int x;
    int y;
};


//led7 api
extern int  led7_ui_init(const struct ui_devices_cfg *ui_cfg);
extern void ui_set_main_menu(enum ui_menu_main menu);
extern void ui_menu_reflash(u8 break_in);//break_in 是否打断显示,例如显示设置过程中需要刷新新界面。是是否打断设置界面显示
extern u8   ui_get_app_menu(u8);
extern void ui_set_led(u8 app_menu, u8 on, u8 phase, u16 highlight, u16 period);
extern void ui_set_auto_reflash(u32 msec);//自动刷新主页
extern void ui_close_main_menu();
extern void ui_set_tmp_menu(u8 app_menu, u16 ret_time, s32 arg, void (*timeout_cb)(u8 menu));
extern void ui_common(void *hd, void *private, u8 menu, u32 arg);//公共显示


//lcd api
//
extern int lcd_ui_init(void *arg);
extern int ui_hide_main(int id);
extern int ui_show_main(int id);

extern int ui_server_msg_post(const char *msg, ...);
extern int ui_hide_curr_main();
extern int ui_touch_msg_post(struct touch_event *event);
extern int ui_key_msg_post(int msg);
extern void key_ui_takeover(u8 on);
extern int key_is_ui_takeover();
extern void ui_backlight_open(void);
extern void ui_backlight_close(void);

extern void ui_touch_timer_delete();
extern void ui_touch_timer_start();
extern void ui_auto_shut_down_modify(void);
extern void ui_auto_shut_down_enable(void);
extern void ui_auto_shut_down_disable(void);
extern int ui_simple_key_msg_post(int, int);





//=================================================================================//
//                        			UI API                    			   		   //
//=================================================================================//
#if (TCFG_UI_ENABLE)
#if(CONFIG_UI_STYLE == STYLE_JL_LED7)//led7 显示
#define UI_INIT(a)             led7_ui_init(a)
#define UI_SHOW_WINDOW(a)      ui_set_main_menu(a)
#define UI_HIDE_WINDOW(...)    ui_close_main_menu()
#define UI_HIDE_CURR_WINDOW()  ui_close_main_menu()
#define UI_GET_WINDOW_ID()     ui_get_app_menu(GET_MAIN_MENU)
#define UI_GET_CURR_MENU()     ui_get_app_menu(GRT_CUR_MENU)
#define UI_REFLASH_WINDOW(a)   ui_menu_reflash(a)
#define UI_SHOW_MENU           ui_set_tmp_menu
#define UI_MSG_POST(...)
#define UI_KEY_MSG_POST(...)


#elif(CONFIG_UI_STYLE == STYLE_UI_SIMPLE)//去框架显示显示

#define UI_INIT(a)            lcd_ui_init(a)
#define UI_SHOW_WINDOW(...)
#define UI_HIDE_WINDOW(...)
#define UI_GET_WINDOW_ID()    (0)
#define UI_HIDE_CURR_WINDOW()
#define UI_SHOW_MENU(...)
#define UI_MSG_POST(...)
#define UI_REFLASH_WINDOW(a)
#define UI_KEY_MSG_POST(a)

#else

#define UI_INIT(a)            lcd_ui_init(a)
#define UI_SHOW_WINDOW(a)     ui_show_main(a)
#define UI_HIDE_WINDOW(a)   ui_hide_main(a)
#define UI_HIDE_CURR_WINDOW() ui_hide_curr_main()
#define UI_GET_WINDOW_ID()    ui_get_current_window_id()
#define UI_MSG_POST           ui_server_msg_post
#define UI_SHOW_MENU(...)
#define UI_GET_CURR_MENU()
#define UI_REFLASH_WINDOW(a)
#define UI_KEY_MSG_POST(a)    ui_key_msg_post


#endif//if(CONFIG_UI_STYLE == STYLE_JL_LED7)

#else

//common api  lcd屏和led7 通用api
#define UI_INIT(...)
#define UI_SHOW_WINDOW(...)
#define UI_HIDE_WINDOW(...)
#define UI_GET_WINDOW_ID()
#define UI_HIDE_CURR_WINDOW()
#define UI_SHOW_MENU(...)
#define UI_MSG_POST(...)
#define UI_REFLASH_WINDOW(a)
#define UI_KEY_MSG_POST(...)
#endif /* #if TCFG_UI_ENABLE */

#endif
