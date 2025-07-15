#ifndef __UI_MAINMENU_H__
#define __UI_MAINMENU_H__

#define AUTORUNTIMEMAX      10
#define TASKTOTLE           24		//任务总数
#define SCREENSHOWTASKSUM   5		//屏幕需显示的任务图标
//#define SHOWTASKSUM  		8 		//需显示的任务图标总数

typedef struct {
    u8  taskname[16]; //任务名
    u16 bmpID;		  //该任务在主界面显示的图标
    u16 strID;		  //该任务在主界面显示的字符串
    u8  showflag;	  //标识该任务是否显式的显示任务图标
} TASKSTRUCT;

extern const TASKSTRUCT taskbuf[TASKTOTLE];
extern u8 taskshow[TASKTOTLE];

u8 UI_mainmenu(s8 apprun);
void findtaskexist(u8 tasksum);
#endif
