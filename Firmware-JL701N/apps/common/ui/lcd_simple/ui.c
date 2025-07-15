#include "app_config.h"
#include "includes.h"
#include "system/includes.h"
#include "fs/fs.h"
#include "ui/ui_simple/ui_res.h"
#include "ui/lcd_simple/ui.h"
#include "key_event_deal.h"


#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_UI_SIMPLE))

s8  MenuItemSelectOnNum;		 //当前屏幕活动page序号:0--(MENUITEMSUMPERSCR-1)
s8  CurrentScreenNum;			 //当前所处屏幕数,从0序号开始
u8  NeedScreenSumViaItem;		 //菜单项目总数将占用屏幕的屏数
u8  LaseScreenRemainMenuItem;	 //最后一屏只需显示的菜单项目数

#define  LCDPAGE  8
#define  LCDCOLUMN   128
#define  SCR_WIDTH         LCDCOLUMN
#define  SCR_HEIGHT        (LCDPAGE*8)



extern u8 LCDBuff[8][128];
extern u8  LanguageMode ;	 //语言模式，ID号

//滑动块//
u8 SetSlider(u8 start_x, u8 start_y, u8 length)
{
    start_x = start_x;
    length = length;
//	u8   x;
//	if( (start_x>SCR_WIDTH-length+2) || (start_y>LCDPAGE) || (length>SLIDERLENGTH))
//		return false;
//
//	LCDBuff[start_y][start_x]=0x3C;
//	for(x=start_x+1; x<start_x+1+length+1; x++)
//	{
//		LCDBuff[start_y][x] = 0x24;
//	}
//	LCDBuff[start_y][start_x+length+1]=0x3C;
//	return true;
    if (ResShowPic(SLIDER, 0, start_y * 8)) {
        return true;
    } else {
        return false;
    }
}


u8 SetSlideBlock(u8 Start_x, u8 Start_y)
{
    u8   SlideBlockPixelBuf[4] = {0x7E, 0xFF, 0xFF, 0x7E};
    u8  x;

    if ((Start_x > SCR_WIDTH - 4) || (Start_y > LCDPAGE)) {
        return false;
    }

    for (x = Start_x; x < Start_x + 4; x++) {
        LCDBuff[Start_y][x] = SlideBlockPixelBuf[x - Start_x];
    }

    return true;
}

u8 SetSlideBlock1(u8 Start_x, u8 Start_y)
{
    if (ResShowPic(SLIDER_P, Start_x, Start_y * 8)) {
        return true;
    } else {
        return false;
    }
}

u8 draw_slider(s8 currentpos, u8 slider_y_pos, u8 sliderlength)
{
    if ((currentpos > sliderlength) || (slider_y_pos > LCDPAGE) || (sliderlength > SLIDERLENGTH)) { //判断滑动条是否超出屏幕范围
        return false;
    }

    if (currentpos <= 0) {    //界限判断
        currentpos = 0;
    } else if (currentpos > SLIDERLENGTH) {
        currentpos = SLIDERLENGTH;
    }

    //ClearLcdBuf_Page(slider_y_pos,1);
    clear_lcd_area(slider_y_pos, slider_y_pos + 1);
    SetSlider((SCR_WIDTH - sliderlength) / 2, slider_y_pos, sliderlength);
    SetSlideBlock1((SCR_WIDTH - sliderlength) / 2 + currentpos, slider_y_pos);
    draw_lcd(slider_y_pos, slider_y_pos + 1);
    return true;
}



u8 UI_Slider(FUN gslider)
{
    int msg[16] = {0};
    struct sys_event *event = NULL;
    u8  currentpos;
    u8  autoruntime = SLIDER_NOMOVE_TIMEEXIT;

    clear_lcd();
    currentpos = gslider(UI_SLIDER_CURR_VALUE);
    draw_slider(currentpos, 5, SLIDERLENGTH);
    draw_lcd(0, LCDPAGE);

    while (1) {
        int ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg)); //500ms_reflash
        if (ret != OS_TASKQ) {
            continue;
        }

        if (msg[0] != 0) {
            continue;
        }


        switch (msg[1]) { //action
        case KEY_OK:
            return 0x00;

        case KEY_UP: //HOLD
            autoruntime = SLIDER_NOMOVE_TIMEEXIT;
            clear_lcd_area(1, 4);
            currentpos = gslider(UI_SLIDER_DEC);
            draw_slider(currentpos, 5, SLIDERLENGTH);
            draw_lcd(0, 8);
            break;

        case KEY_DOWN:	//HOLD
            autoruntime = SLIDER_NOMOVE_TIMEEXIT;
            clear_lcd_area(1, 4);
            currentpos = gslider(UI_SLIDER_INC);
            draw_slider(currentpos, 5, SLIDERLENGTH);
            draw_lcd(0, 8);
            break;

        case MSG_HALF_SECOND:
            autoruntime-- ;
            if (autoruntime == 0) {
                return currentpos ;
                //return 0xff;
            }
            break;

        default:
            break;
        }
    }
}



//进度条//
u8 UI_Progress(u8 currentpos, u8 progress_y_pos, u8 progresslength)
{
    u8  i;
    if ((currentpos > progresslength) || (progress_y_pos > LCDPAGE) || (progresslength > 100)) {
        return false;
    }
    memset(&LCDBuff[0][0], 0x00, sizeof(LCDBuff));
    //while(1)
    {
        SetProgress((SCR_WIDTH - progresslength) / 2, progress_y_pos, progresslength);
        for (i = 1; i < currentpos + 1; i++) {
            LCDBuff[progress_y_pos][(SCR_WIDTH - progresslength) / 2 + i] = 0xFF;
        }
        draw_lcd(0, LCDPAGE);
    }
    return true;
}



u8 SetProgress(u8 start_x, u8 start_y, u8 length)
{
    u8  x;
    if ((start_x > SCR_WIDTH - length + 2) || (start_y > LCDPAGE) || (length > 100)) {
        return false;
    }

    LCDBuff[start_y][start_x] = 0x7E;
    for (x = start_x + 1; x < start_x + 1 + length + 1; x++) {
        LCDBuff[start_y][x] = 0x81;
    }
    LCDBuff[start_y][start_x + length + 1] = 0x7E;
    return true;
}

void Scrol_String(u16 StringID)
{
    res_infor_t  stringinfor;
    u8  yy;
    u32   sdimageaddr = 0;
    static u32   ScrolOffset = 0;
    //putchar(0x30+StringID);
    //u16 StringID = menulist.ItemString[menulist.ActiveItemNum-1];

    void *file = ResGetFp();
    if (!file) {
        return;
    }

    // if(!IsScrolString)  return;
    fseek(file, sizeof(res_head_t) + 2 * sizeof(res_entry_t) + BMPID_SUM * sizeof(res_infor_t) + ((StringID - 1)*LANGUAGEID_SUM + LanguageMode - 1)*sizeof(res_infor_t), SEEK_SET);
    fread(file, (u8 *)&stringinfor, sizeof(res_infor_t));

    stringinfor.dwOffset = ntohl(stringinfor.dwOffset); //res文件保存为大端模式,PC为小端模式,故加此三句,单片机下应该不加此三句
    stringinfor.wWidth   = ntoh(stringinfor.wWidth);

    sdimageaddr = stringinfor.dwOffset;

    if (stringinfor.wWidth > SCR_WIDTH - MENUICONWIDTH - SCROLLBARWIDTH) { //字符串图片宽度 > 屏幕所能显示最大宽度(减去菜单Icon占用的宽度)
        //需要进行滚屏处理
        for (yy = 0; yy < MENUITEMHEIGHT / 8 ; yy++) {
            fseek(file, sdimageaddr + ScrolOffset, SEEK_SET);
            fread(file, &LCDBuff[MenuItemSelectOnNum * 2 + yy][MENUICONWIDTH], (SCR_WIDTH - MENUICONWIDTH - SCROLLBARWIDTH));
            sdimageaddr += stringinfor.wWidth;
        }
        ScrolOffset += 1;
        if (ScrolOffset + (SCR_WIDTH - MENUICONWIDTH - SCROLLBARWIDTH) >= stringinfor.wWidth) {
            ScrolOffset = 0;
        }
        //TRACE("sdimageaddr: %d , ScrolOffset: %d \n", sdimageaddr, ScrolOffset );
        UI_MenuSelectOn(MenuItemSelectOnNum);
        draw_lcd(0, LCDPAGE);
    }
//    else
//        IsScrolString = false;
}






u8 SetScrollBar(u8 Start_x)
{
    u8   x, y;
    u8   ScrollBarPixelBuf[8][6] = { 0xFC, 0x02, 0x01, 0x01, 0x02, 0xFC,
                                     0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
                                     0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
                                     0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
                                     0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
                                     0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
                                     0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
                                     0x3F, 0x40, 0x80, 0x80, 0x40, 0x3F
                                   };
    if (Start_x > SCR_WIDTH - 6) { //6为滚动条的象ç´ 宽度
        return false;
    }

    for (x = Start_x; x < Start_x + 6; x++) {
        for (y = 0; y < 8; y++) {
            LCDBuff[y][x] = ScrollBarPixelBuf[y][x - Start_x];
        }
    }
    return true;
}


u8 TurnPixelReverse_Page(u8 startpage, u8 pagelen)
{
    u8   hnum, l;

    if ((startpage > 8) || (pagelen > 8)) {
        return false;
    }

    for (hnum = startpage; hnum < startpage + pagelen; hnum = hnum + 1) {
        for (l = 0; l < SCR_WIDTH; l++) {
            LCDBuff[hnum][l] = ~LCDBuff[hnum][l];
        }
    }
    return true;
}

u8 TurnPixelReverse_Rect(u8 left, u8 top, u8 right, u8 bottom)
{
    //left、right取值范围：0--(SCR_WIDTH-1)  ;  top、bottom取值范围：0--(SCR_HEIGHT/8-1)
    u8   hnum, l;
    if ((left > SCR_WIDTH) || (top > SCR_HEIGHT) || (left >= right) || (top >= bottom)) {
        return false;
    }

    for (hnum = top; hnum < bottom + 1; hnum = hnum + 1) {
        for (l = left; l < right + 1; l++) {
            LCDBuff[hnum][l] = ~LCDBuff[hnum][l];
        }
    }
    return true;
}

u8 ClearLcdBuf_Page(u8 startpage, u8 pagelen)
{
    u8   hnum, l;

    if ((startpage > 8) || (pagelen > 8)) {
        return false;
    }

    for (hnum = startpage; hnum < startpage + pagelen; hnum = hnum + 1) {
        for (l = 0; l < SCR_WIDTH; l++) {
            LCDBuff[hnum][l] = 0x00;
        }
    }
    return true;
}

#endif


