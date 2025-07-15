#include "ui/ui.h"
#include "app_config.h"
#include "ui/ui_api.h"
#include "app_task.h"
#include "system/timer.h"
#include "key_event_deal.h"
#include "audio_config.h"
#include "jiffies.h"
#include "ui/ui_simple/ui_res.h"
#include "ui/lcd_simple/ui.h"
#include "app_msg.h"
#include "ui/lcd_simple/ui_mainmenu.h"

#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_UI_SIMPLE))

////FM任务下菜单////
MENULIST  FM_Main;             //FM任务主菜单
MENULIST  FM_Seek;
////FM任务下菜单ID////
const u16 FM_Main_ItemString[8] =
{fmenu16, sreplay1, fmenu1, fmenu15, fmenu2, fmenu3, fm_vol, mstop5};

const u16 FM_Seek_ItemString[3] =
{fm_manual, fmautoload, fm_switch};

static void DEMO_MenuInit(void)
{
    FM_Main.ItemSum = 8;
    FM_Main.ActiveItemNum = 1;    				//菜单选中时的图标ID
    FM_Main.IconID[0] = ACTIVE;   				//菜单选中时的图标ID
    FM_Main.IconID[1] = UNACTIVE; 				//菜单未选中时的图标ID
    FM_Main.TitleID = 0;//vstopm1;  		 	//菜单的标题字符串ID号,0代表无标题
    FM_Main.ItemString = FM_Main_ItemString;

    FM_Seek.ItemSum = 3;
    FM_Seek.ActiveItemNum = 1;    				//菜单选中时的图标ID
    FM_Seek.IconID[0] = ACTIVE;   				//菜单选中时的图标ID
    FM_Seek.IconID[1] = UNACTIVE; 				//菜单未选中时的图标ID
    FM_Seek.TitleID = 0;//vstopm1;  		 	//菜单的标题字符串ID号,0代表无标题
    FM_Seek.ItemString = FM_Seek_ItemString;
}

static void UI_MenuSelectOn(uint8 showitemnum)
{
    TurnPixelReverse_Rect(MENUICONWIDTH, showitemnum * 2, SCR_WIDTH - SCROLLBARWIDTH - 1, showitemnum * 2 + 1);
    ResShowPic(ACTIVE, 0, showitemnum * 16);
}

static u8 volume_adjust(u8 mode)
{
    static u8 test = 0;
    if (mode == UI_SLIDER_INC) {
        test ++;
    }

    if (mode == UI_SLIDER_DEC) {
        if (test) {
            test--;
        }
    }
    printf(">>>>%s %d %d<<<<\n", __func__, __LINE__, test);
    return test;
}

static void UpdateMenu(MENULIST *Mlist)
{
    u8 i;
    static u8 itembegin;
    if (Mlist->TitleID > 0) {
        itembegin = 1;
    } else {
        itembegin = 0;
    }

    if (CurrentScreenNum < NeedScreenSumViaItem - 1) {
        for (i = itembegin; i < MENUITEMSUMPERSCR; i++) {
            ResShowPic(Mlist->IconID[1], 0, i * MENUITEMHEIGHT);
            ResShowMultiString(MENUICONWIDTH, i * MENUITEMHEIGHT, Mlist->ItemString[(i - itembegin) + (CurrentScreenNum * (MENUITEMSUMPERSCR - itembegin))]);
        }
    } else { //到达最后一屏，只显示剩下的菜单项目
        for (i = itembegin; i < LaseScreenRemainMenuItem + itembegin; i++) {
            ResShowPic(Mlist->IconID[1], 0, i * MENUITEMHEIGHT);
            ResShowMultiString(MENUICONWIDTH, i * MENUITEMHEIGHT, Mlist->ItemString[(i - itembegin) + (CurrentScreenNum * (MENUITEMSUMPERSCR - itembegin))]);
        }
    }
}

static u8 SetFMArrow(uint8 Start_x, uint8 Start_y)
{
    uint8 FMArrowPixelBuf[7] = {0x10, 0x30, 0x5F, 0x81, 0x5F, 0x30, 0x10};
    uint8 x;

    if ((Start_x > SCR_WIDTH - 7) || (Start_y > LCDPAGE)) {
        return false;
    }

    for (x = Start_x; x < Start_x + 7; x++) {
        LCDBuff[Start_y][x] = FMArrowPixelBuf[x - Start_x];
    }
    return true;
}

//菜单//
static u8 DEMO_UI_MenuList(MENULIST *Mlist)
{
    uint8 autoruntime = MENUWAITTIME;
    uint8 i = 0;
    uint8 minusbase;
    int msg[16];
    bool redraw = true;

    memset(&LCDBuff[0][0], 0x00, (SCR_HEIGHT / 8)*SCR_WIDTH);

    if (Mlist->TitleID > 0) { //有菜单标题 菜单项目数为3
        minusbase = (Mlist->ActiveItemNum + (MENUITEMSUMPERSCR - 1) - 1) / (MENUITEMSUMPERSCR - 1) - 1;
        MenuItemSelectOnNum = Mlist->ActiveItemNum - minusbase * (MENUITEMSUMPERSCR - 1);
        NeedScreenSumViaItem = (Mlist->ItemSum + (MENUITEMSUMPERSCR - 1) - 1) / (MENUITEMSUMPERSCR - 1);
        LaseScreenRemainMenuItem = Mlist->ItemSum % (MENUITEMSUMPERSCR - 1);
        if (LaseScreenRemainMenuItem == 0) {
            LaseScreenRemainMenuItem = (MENUITEMSUMPERSCR - 1);
        }
        CurrentScreenNum = (Mlist->ActiveItemNum - 1) / (MENUITEMSUMPERSCR - 1);
        ResShowMultiString(0, 0, Mlist->TitleID);	 //显示菜单图标

        UpdateMenu(Mlist);
    } else { //没有菜单标题 菜单项目数为4
        MenuItemSelectOnNum = (Mlist->ActiveItemNum - 1) % (MENUITEMSUMPERSCR); //当前屏的活动菜单
        NeedScreenSumViaItem = (Mlist->ItemSum + MENUITEMSUMPERSCR - 1) / MENUITEMSUMPERSCR; //需要分几屏显示
        LaseScreenRemainMenuItem = Mlist->ItemSum % MENUITEMSUMPERSCR;

        if (LaseScreenRemainMenuItem == 0) {
            LaseScreenRemainMenuItem = (MENUITEMSUMPERSCR);
        }

        CurrentScreenNum = (Mlist->ActiveItemNum - 1) / MENUITEMSUMPERSCR;

        UpdateMenu(Mlist);
    }
    UI_MenuSelectOn(MenuItemSelectOnNum);

    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        switch (msg[0]) {
        /* case APP_MSG_SYS_EVENT: */
        case Q_EVENT:
            printf("%s demo key = %d value = %x\n", __func__, msg[1], msg[2]);
            switch (msg[1]) {
            case KEY_OK:
                return  Mlist->ItemString[Mlist->ActiveItemNum - 1];
                break;
            case KEY_MENU:
                break;
            case KEY_DOWN:
                autoruntime = MENUWAITTIME;
                Mlist->ActiveItemNum -= 1;
                if ((Mlist->ActiveItemNum <= 0) && (Mlist->TitleID > 0)) {
                    Mlist->ActiveItemNum = Mlist->ItemSum;
                    CurrentScreenNum = NeedScreenSumViaItem - 1;
                    MenuItemSelectOnNum = LaseScreenRemainMenuItem + 1;
                } else if ((Mlist->ActiveItemNum <= 0) && (Mlist->TitleID <= 0)) {
                    Mlist->ActiveItemNum = Mlist->ItemSum;
                    CurrentScreenNum = NeedScreenSumViaItem - 1;
                    MenuItemSelectOnNum = LaseScreenRemainMenuItem;
                }

                MenuItemSelectOnNum -= 1;
                if ((MenuItemSelectOnNum <= 0) && (Mlist->TitleID > 0)) {
                    MenuItemSelectOnNum = 3;
                    CurrentScreenNum -= 1;
                    if (CurrentScreenNum < 0) {
                        CurrentScreenNum = 0;
                    }
                } else if ((MenuItemSelectOnNum < 0) && (Mlist->TitleID <= 0)) {
                    MenuItemSelectOnNum = 3;
                    CurrentScreenNum -= 1;
                    if (CurrentScreenNum < 0) {
                        CurrentScreenNum = 0;
                    }
                }
                redraw = true;

                break;
            case KEY_UP:
                autoruntime = MENUWAITTIME;
                Mlist->ActiveItemNum += 1;
                if ((Mlist->ActiveItemNum > Mlist->ItemSum) && (Mlist->TitleID > 0)) {
                    Mlist->ActiveItemNum = 1;
                    CurrentScreenNum = 0;
                    MenuItemSelectOnNum = 0;
                } else if ((Mlist->ActiveItemNum > Mlist->ItemSum) && (Mlist->TitleID <= 0)) {
                    Mlist->ActiveItemNum = 1;
                    CurrentScreenNum = 0;
                    MenuItemSelectOnNum = -1;
                }


                MenuItemSelectOnNum += 1;
                if ((MenuItemSelectOnNum > 3) && (Mlist->TitleID > 0)) {
                    MenuItemSelectOnNum = 1;
                    CurrentScreenNum += 1;
                    if (CurrentScreenNum >= NeedScreenSumViaItem) {
                        CurrentScreenNum = 0;
                    }
                } else if ((MenuItemSelectOnNum > 3) && (Mlist->TitleID <= 0)) {
                    MenuItemSelectOnNum = 0;
                    CurrentScreenNum += 1;
                    if (CurrentScreenNum >= NeedScreenSumViaItem) {
                        CurrentScreenNum = 0;
                    }
                }
                redraw = true;

            case MSG_HALF_SECOND:
                autoruntime--;
                if (!autoruntime) {
                    return mstop5;
                }
                break;

            default:
                return 0xff;
                break;
            }
            break;
        }

        if (redraw) {
            redraw = false;
            memset(&LCDBuff[0][0], 0x00, (SCR_HEIGHT / 8)*SCR_WIDTH);
            if (Mlist->TitleID > 0) { //有菜单标题 菜单项目数为3
                ResShowMultiString(0, 0, Mlist->TitleID);
                UpdateMenu(Mlist);
            } else { //没有菜单标题 菜单项目数为4
                UpdateMenu(Mlist);
            }
            UI_MenuSelectOn(MenuItemSelectOnNum);

            SetScrollBar(SCR_WIDTH - SCROLLBARWIDTH); //滚动条
            //滑动块设置
            if (Mlist->ItemSum == 8) {
                SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, Mlist->ActiveItemNum - 1);
            } else if (Mlist->ItemSum < 8) {
                if (Mlist->ActiveItemNum == 1) {
                    SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, 0);
                } else if (Mlist->ActiveItemNum == Mlist->ItemSum) {
                    SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, 7);
                } else if (Mlist->ItemSum <= 4) {
                    SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, Mlist->ActiveItemNum * 2 - 1);
                } else if (Mlist->ItemSum == 5 && Mlist->ActiveItemNum == 2) {
                    SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, 2);
                } else {
                    SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, 8 * Mlist->ActiveItemNum / Mlist->ItemSum);
                }
            } else if (Mlist->ItemSum > 8) {
                if (Mlist->ActiveItemNum == 1) {
                    SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, 0);
                }
                if (Mlist->ActiveItemNum == Mlist->ItemSum) {
                    SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, 7);
                } else {
                    SetSlideBlock(SCR_WIDTH - SCROLLBARWIDTH + 1, 8 * Mlist->ActiveItemNum / Mlist->ItemSum);
                }
            }
            draw_lcd(0, LCDPAGE);
        }

    }
}







void demo_simple_ui_mode()
{
    int msg[16];
    DEMO_MenuInit();
    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        switch (msg[0]) {
        /* case APP_MSG_SYS_EVENT: */
        case Q_EVENT:
            printf("ui %s  key = %d value = %x\n", __func__, msg[1], msg[2]);
            switch (msg[1]) {
            case KEY_OK:
                /* UI_mainmenu(APP_FM_TASK); */
                UI_mainmenu(5);
                DEMO_UI_MenuList(&FM_Main);
                DEMO_UI_MenuList(&FM_Seek);
                /* UI_mainmenu(APP_RECORD_TASK); */
                ui_simple_key_msg_post(KEY_OK, 0);
                break;
            case KEY_MENU:
                break;
            case KEY_DOWN:
                break;
            case KEY_UP:
                break;
            default:
                break;
            }
            break;
        }
    }
}






#endif
