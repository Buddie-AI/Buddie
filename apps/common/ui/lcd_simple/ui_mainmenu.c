#include "app_config.h"
#include "includes.h"
#include "system/includes.h"
#include "fs/fs.h"
#include "ui/ui_simple/ui_res.h"
#include "ui/lcd_simple/ui_mainmenu.h"
#include "ui/lcd_simple/ui.h"
#include "key_event_deal.h"
#include "app_task.h"
#include "app_msg.h"


#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_UI_SIMPLE))


/* APP_POWERON_TASK  = 1, */
/* APP_POWEROFF_TASK = 2, */
/* APP_BT_TASK       = 3, */
/* APP_MUSIC_TASK    = 4, */
/* APP_FM_TASK       = 5, */
/* APP_RECORD_TASK   = 6, */
/* APP_LINEIN_TASK   = 7, */
/* APP_RTC_TASK      = 8, */
/* APP_SLEEP_TASK    = 9, */
/* APP_IDLE_TASK	  = 10, */
/* APP_PC_TASK       = 11, */
/* APP_SPDIF_TASK    = 12, */
/* APP_WATCH_UPDATE_TASK     = 13, */
/* APP_SMARTBOX_ACTION_TASK  = 14, */
/*  */
const TASKSTRUCT taskbuf[TASKTOTLE] = {
    {"none", 0xffff,    0xffff,   0,},//0
    {"poweron", 0xffff,    0xffff,   0,},//1
    {"powerff", 0xffff,    0xffff,   0,},//2
    {"bt", FM,     t_fm,     1,},//3
    {"music", MUSIC1,    t_music,  1,},//4
    {"fm", FM,     t_fm,     1,},//5
    {"record", RECORD1,   t_record, 1,},//6
    {"linein", LINEIN1,   alarm1,   1,},//7
    {"rtctime", TOOLP,     fm_rtue,  1,},//8
    {"end", 0xffff,    0xffff,   0,},//必须 以end 结束
};

u8   taskindex[TASKTOTLE] = {0xff, 0, 1, 2, 3, 4, 5, 0xff, 0xff, 6, 7};
u8   taskshow[TASKTOTLE];

u8  g_taskshowsum;

#define  LCDPAGE  8
#define  LCDCOLUMN   128
#define  SCR_WIDTH         LCDCOLUMN
#define  SCR_HEIGHT        (LCDPAGE*8)


extern u8 LCDBuff[LCDPAGE][LCDCOLUMN];

void findtaskexist(u8 tasksum)
{
    u8 i;
    u8 ret;
    u8 n = 0;
    u8 appnumber = 0;
    u16 taskexist = 0x0001; //必有 main task
    u8 taskfile[16];

    memset(&taskindex, 0xff, sizeof(taskindex));
    for (i = 1; i < tasksum; i++) { //不用检查main task 0
        if (!strcmp(taskbuf[i].taskname, "end")) {
            break;
        }
        taskexist |= BIT(i);
        if (taskbuf[i].showflag == 1) { //该task需显性显示
            taskshow[n] = i;
            taskindex[i] = appnumber++;
            n++;
        }
    }
    g_taskshowsum = n;
}

u8 Set_LeftTriangle(u8 Start_x, u8 Start_y)    //左三角标
{
    u8   x, y;
    u8   LeftTrianglePixelBuf[2][3] = {0x80, 0xC0, 0xE0,
                                       0x00, 0x01, 0x03
                                      };

    if ((Start_x > SCR_WIDTH - 3) || (Start_y > LCDPAGE)) {
        return false;
    }

    for (x = Start_x; x < Start_x + 3; x++) {
        for (y = Start_y; y < Start_y + 2; y++) {
            LCDBuff[y][x] = LeftTrianglePixelBuf[y - Start_y][x - Start_x];
        }
    }
    return true;
}

u8 Set_RightTriangle(u8 Start_x, u8 Start_y)  //右三角标
{
    u8   x, y;
    u8   LeftTrianglePixelBuf[2][3] = {0xE0, 0xC0, 0x80,
                                       0x03, 0x01, 0x00
                                      };

    if ((Start_x > SCR_WIDTH - 3) || (Start_y > LCDPAGE)) {
        return false;
    }

    for (x = Start_x; x < Start_x + 3; x++) {
        for (y = Start_y; y < Start_y + 2; y++) {
            LCDBuff[y][x] = LeftTrianglePixelBuf[y - Start_y][x - Start_x];
        }
    }
    return true;
}
void  draw_mainmenu(s8 scr_index, s8 start_task_num)
{
    u8 n;
    u8 mem_start_task_num;
    mem_start_task_num = start_task_num;
    clear_lcd();
    for (n = 0; n < (g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM); n++) {
        if (n == scr_index) {
            clear_lcd_area(0, 1);
            ResShowMultiString(0, 0, taskbuf[taskshow[start_task_num]].strID);
            ResShowPic(taskbuf[taskshow[start_task_num]].bmpID + 1, n * 24 + 4, 2 * 8); //选中的图标
        } else {
            ResShowPic(taskbuf[taskshow[start_task_num]].bmpID + 0, n * 24 + 4, 2 * 8);    //未选中的图标
        }
        start_task_num++;
    }

    if (mem_start_task_num != (g_taskshowsum - (g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM))) {
        Set_RightTriangle((SCR_WIDTH - 1) - 3, 3);
    }
    if (mem_start_task_num != 0) {
        Set_LeftTriangle(0, 3);
    }
    draw_lcd(0, 8) ;
}

u8 UI_mainmenu(s8 apprun)
{
    u8 need_draw = 0 ;
    u8 autoruntime = AUTORUNTIMEMAX;
    int msg[16] = {0};

    s8 task_index;          //任务索引号
    s8 screen_index;        //当前屏幕显示高亮图标的索引号0-4
    s8 start_task_index;    //当前屏幕显示的第一个任务图标的索引号
    s8 mem_task_app;        //传递进来的任务号

    mem_task_app = apprun;//6  1
    task_index = taskindex[apprun]; //5  0
//  deg_string("\n task_index init:  ");
//  printf_u8(task_index);
    if (task_index <= (g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM)) {
        if (task_index == SCREENSHOWTASKSUM) {
            screen_index = task_index - 1;
            start_task_index = task_index - ((g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM) - 1);
            //task_index = task_index-1;
        } else {
            screen_index = task_index;
            start_task_index = 0;
        }
    } else {
        screen_index = (g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM) - 1; //4
        start_task_index = task_index - ((g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM) - 1); //1
    }
    draw_mainmenu(screen_index, start_task_index);
    while (1) {

        if (need_draw != 0) {
            draw_mainmenu(screen_index, start_task_index) ;
            need_draw = FALSE;
            draw_lcd(0, 8) ;
            autoruntime = AUTORUNTIMEMAX;
        }

        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        switch (msg[0]) {
#if APP_CASE_EARPHONE
        case Q_EVENT:
#else
        case APP_MSG_SYS_EVENT:
#endif
            printf("ui %s  key = %d value = %x\n", __func__, msg[1], msg[2]);
            switch (msg[1]) {
            case KEY_OK:
                return taskshow[task_index];
            case KEY_UP:
                task_index--;
                if (task_index < 0) {
                    task_index = 0;
                }

                screen_index--;
                if (screen_index < 0) {
                    start_task_index--;
                    if (start_task_index < 0) {
                        start_task_index = 0;
                    }
                }
                if (screen_index < 0) {
                    screen_index = 0;
                }

                //              deg_string("\n task_index:  ");
                //              printf_u8(task_index);
                //              deg_string("\n screen_index:  ");
                //              printf_u8(screen_index);
                need_draw = TRUE;
                break;


            case KEY_DOWN:
                task_index++;
                if (task_index > (g_taskshowsum - 1)) {
                    task_index = g_taskshowsum - 1 ;
                }

                screen_index++;
                if (screen_index > (g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM) - 1) {
                    start_task_index++;
                    if (start_task_index > g_taskshowsum - (g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM)) {
                        start_task_index = g_taskshowsum - (g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM);
                    }
                }
                if (screen_index > ((g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM) - 1)) {
                    screen_index = (g_taskshowsum < SCREENSHOWTASKSUM ? g_taskshowsum : SCREENSHOWTASKSUM) - 1;
                }


                //              deg_string("\n task_index:  ");
                //              printf_u8(task_index);
                //              deg_string("\n screen_index:  ");
                //              printf_u8(screen_index);
                need_draw = TRUE;
                break;

            case MSG_HALF_SECOND:
                autoruntime-- ;
                if (autoruntime == 0) {
                    //return taskshow[mem_task_app-1];
                    clear_lcd();
                    return 0xff;
                }
                break ;

            default:
                break;
            }
        }
    }
}
#endif
