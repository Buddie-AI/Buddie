#include "app_config.h"
#include "includes.h"
#include "ui/ui_api.h"
#include "ui/ui.h"
#include "typedef.h"
#include "ui/ui_simple/ui_res.h"
#include "font/font_textout.h"
#include "ui/lcd_simple/ui_mainmenu.h"


#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_UI_SIMPLE))
#include "key_event_deal.h"
#include "app_task.h"
#include "font/language_list.h"

extern u8 LCDBuff[8][128];

#define UI_TASK_NAME "ui"



/* static const struct ui_dis_api *ui_dis_main[] = { */
/* #if TCFG_APP_BT_EN */
/* #endif */
/* #if TCFG_APP_MUSIC_EN */
/* #endif */
/* #if TCFG_APP_FM_EN */
/* #endif */
/* #if TCFG_APP_RECORD_EN */
/* #endif */
/* #if TCFG_APP_LINEIN_EN */
/* #endif */
/* #if TCFG_APP_RTC_EN */
/* #endif */
/* #if TCFG_APP_PC_EN */
/* #endif */
/* #if TCFG_APP_SPDIF_EN */
/* #endif */
/* }; */
/*  */


struct ui_display_env {
    u8 init;
    OS_SEM sem;//用来做模式初始化的信号量
    struct lcd_interface *lcd;
    int main_menu;//当前主页
    int this_menu;//当前子页面
};


static struct ui_display_env __ui_display_env = {0};

#define __ui_display (&__ui_display_env)
#define __this (__ui_display)

static int post_ui_msg(int *msg, u8 len)
{
    int count = 0;
    int err = 0;
    if (!__ui_display->init) {
        return -1;
    }

__retry:
    err =  os_taskq_post_type(UI_TASK_NAME, msg[0], len - 1, &msg[1]);
    if (cpu_in_irq() || cpu_irq_disabled()) {
        return err;
    }
    if (err) {

        if (!strcmp(os_current_task(), UI_TASK_NAME)) {
            return err;
        }

        if (count > 10) {
            return -1;
        }
        count++;
        os_time_dly(1);
        goto __retry;
    }
    return err;
}


// @brief: 应用往ui发送key消息，由ui控件分配
//=================================================================================//
int ui_simple_key_msg_post(int key, int value)
{
    int msg[8];

    /* if (key >= 0x80) { */
    /*     return -1; */
    /* } */
    /*  */
    msg[0] = Q_EVENT | SYS_KEY_EVENT;
    msg[1] = key;
    msg[2] = value;
    return post_ui_msg(msg, 3);
}




void clear_lcd()
{
    if (!__this->init) {
        return;
    }
    __this->lcd->clear_screen(0);
}


void draw_lcd(u8 start_page, u8 page_len)
{
    if (!__this->init) {
        return;
    }
    __this->lcd->draw_page(&(LCDBuff[start_page][0]), start_page, page_len);
}

void clear_lcd_area(u8 start_page, u8 end_page)
{
    if (end_page > 8) {
        end_page = 8;
    }
    for (; start_page < end_page; start_page++) {
        memset(&LCDBuff[start_page][0], 0x00, 128);
    }
}






extern u8 app_get_curr_task();

extern void demo_simple_ui_mode();

static void ui_task_loop()
{
    while (1) {
#if APP_CASE_EARPHONE
        demo_simple_ui_mode();
#else
        switch (app_get_curr_task()) {
        case APP_POWERON_TASK:
        case APP_POWEROFF_TASK:
        case APP_BT_TASK:
        case APP_MUSIC_TASK:
        case APP_FM_TASK:
        case APP_RECORD_TASK:
        case APP_LINEIN_TASK:
        case APP_RTC_TASK:
        case APP_PC_TASK:
        case APP_SPDIF_TASK:
        case APP_IDLE_TASK:
        case APP_SLEEP_TASK:
        case APP_SMARTBOX_ACTION_TASK:
            demo_simple_ui_mode();
            break;
        default:
            printf(" no task \n");
            os_time_dly(100);
            break;
        }
#endif
    }


}

static void test_ui()
{
    /* ResShowPic(2,0,0); */
    /* ResShowPic(2,0,16);  */
    /* ResShowMultiString(0,0,7); */
    struct font_info *info = font_open(NULL,  Chinese_Simplified);
    info->flags = FONT_SHOW_PIXEL;//显示
    info->text_height = 16;
    info->text_width = 128;//设置显示窗口大小
    font_textout_utf8(info, "杰理科技", strlen("杰理科技"), 24, 0);
    font_textout_utf8(info, "按ok按键开始", strlen("按ok按键开始"), 0, 16);
    font_textout_utf8(info, "测试", strlen("测试"), 24, 32);
    font_close(info);
    ResShowPic(2, 0, 48);
    ResShowPic(2, 110, 48);
    draw_lcd(0, 8);

}


static void __font_pix_copy(u8 *pix, struct rect *draw, int x, int y,
                            int height, int width, u8 color)
{

    int i, j, h;
    for (j = 0; j < (height + 7) / 8; j++) { /* 纵向8像素为1字节 */
        for (i = 0; i < width; i++) {
            if (((i + x) >= draw->left)
                && ((i + x) <= (draw->left + draw->width - 1))) { /* x在绘制区域，要绘制 */
                u8 pixel = pix[j * width + i];
                int hh = height - (j * 8);
                if (hh > 8) {
                    hh = 8;
                }
                for (h = 0; h < hh; h++) {
                    if (((y + j * 8 + h) >= draw->top)
                        && ((y + j * 8 + h) <= (draw->top + draw->height - 1))) { /* y在绘制区域，要绘制 */
                        u16 clr = pixel & BIT(h) ? color : !color;
                        if (clr) {
                            LCDBuff[j + (y + h) / 8][x + i] |= (BIT(h));
                        } else {
                            LCDBuff[j + (y + h) / 8][x + i] &= ~(BIT(h));
                        }
                    }
                } /* endof for h */

            }
        }/* endof for i */
    }/* endof for j */
}



void ui_simple_putchar(struct font_info *info, u8 *pixel, u16 width, u16 height, u16 x, u16 y)
{
    struct rect draw;
    u8 color = 1;
    draw.left = x;
    draw.top  = y;
    draw.width = info->text_width;
    draw.height = info->text_height;
    __font_pix_copy(pixel,
                    (struct rect *)&draw,
                    (s16)x,
                    (s16)y,
                    height,
                    width,
                    color);
}

static void half_second_tick()
{
    ui_simple_key_msg_post(MSG_HALF_SECOND, 0);
    //半秒的事件
}

static void ui_task(void *p)
{
    __ui_display->init = 1;

    struct lcd_info info = {0};
    __this->lcd = lcd_get_hdl();
    printf("ui_task_run_Check ...\n");
    ASSERT(__this->lcd);
    ASSERT(__this->lcd->init);
    ASSERT(__this->lcd->get_screen_info);
    ASSERT(__this->lcd->draw);
    ASSERT(__this->lcd->set_draw_area);
    ASSERT(__this->lcd->backlight_ctrl);
    ASSERT(__this->lcd->clear_screen);
    ASSERT(__this->lcd->draw_page);

    if (__this->lcd->init) {
        __this->lcd->init(p);
    }

    if (__this->lcd->backlight_ctrl) {
        __this->lcd->backlight_ctrl(true);
    }

    if (__this->lcd->get_screen_info) {
        __this->lcd->get_screen_info(&info);
    }

    printf("ui_platform_init :: [%d,%d,%d,%d]\n", 0, 0, info.width, info.height);
    os_sem_post(&__ui_display->sem);
#if APP_CASE_EARPHONE
    findtaskexist(3);
#else
    findtaskexist(APP_TASK_MAX_INDEX);
#endif
    ResOpen("mnt/sdfile/res/menu.res");
    test_ui();
    sys_timer_add(NULL, half_second_tick, 500); //制作半秒的时机
    while (1) {
        ui_task_loop();
    }
}






int lcd_ui_init(void *arg)
{
    int err = 0;
    os_sem_create(&__ui_display->sem, 0);
    err = task_create(ui_task, arg, UI_TASK_NAME);
    os_sem_pend(&__ui_display->sem, 0);
    return 0;
}



#else

int ui_simple_key_msg_post(int key, int value)
{
    return 0;
}

#endif
