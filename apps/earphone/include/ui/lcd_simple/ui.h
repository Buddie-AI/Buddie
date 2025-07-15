#ifndef __LCDSIMULATOR_H__
#define __LCDSIMULATOR_H__

#include "menu.h"
#include "font/language_list.h"
#include "typedef.h"

// #define USE_BLANK_FUNC   1
// #define  LCDPAGE  8
// #define  LCDCOLUMN   128
// #define        SCR_WIDTH         LCDCOLUMN
// #define        SCR_HEIGHT        (LCDPAGE*8)

#define  MENUICONWIDTH    12   //菜单项目左边图标的宽度(象素)
#define  MENUITEMHEIGHT   16   //菜单项目的高度(象素)
#define  SCROLLBARWIDTH    6   //垂直滚动条的宽度(象素)
#define  MENUITEMSUMPERSCR  (SCR_HEIGHT/MENUITEMHEIGHT)    //每屏可显示的最大菜单项
#define  MENUITEMPAGEHEIGHT   2    //每个菜单项所占的页数    2个LCDPAGE --> 2*8=16个像素

#define  RESFILESTARTADDRESS  0    //资源文件的起始地址


#define PROGRESSLENGTH  100    //进度条长度(象素)
#define SLIDERLENGTH    100	   //滑动条长度(象素)
#define SLIDERMOVESTEP  5      //滑动块的移动步长
#define SLIDER_NOMOVE_TIMEEXIT   10  //滑动条不动超过此时间自动退出,(*0.5S)

#define MENUWAITTIME   (8*2)   //菜单界面不操作超过MENUWAITTIME时间则自动返回

#define ntohl(x) (unsigned long)((x>>24)|((x>>8)&0xff00)|(x<<24)|((x&0xff00)<<8))
#define ntoh(x)  (unsigned short int )((x>>8&0x00ff)|x<<8&0xff00)

//#pragma pack(1)
typedef u8(* FUN)(u8 mode);

typedef struct _MENULIST {
    u8   ItemSum;        //菜单项目总数
    u8   ActiveItemNum;  //当前活动菜单项目序号:1--ItemSum
    u8   IconID[2];      //第一个为菜单选中时显示图标的ID号,第二个为菜单未选中时显示图标的ID号
    u16  TitleID;        //菜单的标题字符串ID号,没有标题此值为0
    u16   *ItemString;   //菜单项目对应的字符串ID号
//	FUN fun;
} MENULIST;

//////函数声明//////

u8 TurnPixelReverse_Page(u8 startpage, u8 pagelen);
u8 TurnPixelReverse_Rect(u8 left, u8 top, u8 right, u8 bottom);

void  UI_MenuSelectOn(u8 showitemnum);
void Scrol_String(u16 StringID);
u8 SetScrollBar(u8 Start_x);
u8 SetSlideBlock(u8 Start_x, u8 Start_y);

u8 UI_Slider(FUN gslider);
u8 UI_Progress(u8 currentpos, u8 progress_y_pos, u8 progresslength);
u8 SetSlider(u8 start_x, u8 start_y, u8 length);
u8 SetProgress(u8 start_x, u8 start_y, u8 length);
u8 ClearLcdBuf_Page(u8 startpage, u8 pagelen);
u8 ClearLcdBuf_Rect(u8 left, u8 top, u8 right, u8 bottom);
u8 draw_slider(s8 currentpos, u8 slider_y_pos, u8 sliderlength);
#define UI_SLIDER_CURR_VALUE	0
#define UI_SLIDER_DEC			1
#define UI_SLIDER_INC			2

extern s8   MenuItemSelectOnNum;		 //当前屏幕活动page序号:0--(MENUITEMSUMPERSCR-1)
extern s8   CurrentScreenNum;			 //当前所处屏幕数,从0序号开始
extern u8   NeedScreenSumViaItem;		 //菜单项目总数将占用屏幕的屏数
extern u8   LaseScreenRemainMenuItem;	 //最后一屏只需显示的菜单项目数
extern u8   LanguageMode;

extern void clear_lcd();
extern void draw_lcd(u8 start_page, u8 page_len);
extern void clear_lcd_area(u8 start_page, u8 end_page);

#endif
