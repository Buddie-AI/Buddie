/*******************************************************************************************
 File Name: ap_res.h

 Version: 1.00

 Discription:

 Author:yulin deng

 Email :flowingfeeze@163.com

 Date:星期三, 六月 22 2011

 Copyright:(c) 2011  @ , All Rights Reserved.
*******************************************************************************************/
#ifndef __ap_res_h
#define __ap_res_h
#include "typedef.h"

/******************************************************************
 ************************资源头的数据结构***************************
 资源结构:
 资源头        res_head
 ------------------------------------------
 资源目录项	res_head_Item1(picture Item head)
 res_head_Item2(string Item head)
 res_head_Item3(multi-string Item head)
 ------------------------------------------
 资源索引表	res_entry1(picture 1)
 ................
 res_entryN(picture N)
 res_entry1(lang 1)
 ................
 res_entryN(lang N)
 --------------------------------------------
 资源数据      picture data 1
 ................
 picture data N
 Lang ID 1 entry
 ................
 Lang ID N entry
 Lang 1 string data
 ................
 Lang N string data
 ******************************************************************/
#define   RESHEADITEM    16 		   //各个entry长度，统一为16uint8s
#define   GROUPDEFINE   6
#define   ROW_COUNT_DEF   6
#define   TYPE_DIR        0
#define   TYPE_FILE       1


#ifndef uint8
#define uint8 u8
#endif
#ifndef uint16
#define uint16 u16
#endif
#ifndef uint32
#define uint32 u32
#endif
#ifndef int32
#define int32 int
#endif




typedef struct {
    uint8 magic[4]; //'R', 'U', '2', 0x19
    uint16 counts; //资源的个数
} res_head_t; //6 uint8s


/*资源类型索引表的数据结构*/
typedef struct {
    uint32 dwOffset; //资源内容索引表的偏移
    uint16 wCount; //资源类型总数
    uint8 bItemType; //'P'--PIC Table,'S'--String Table,'X' -- XML File
    uint8 type;
} res_entry_t;

/*资源内容信息索引的数据结构*/
typedef struct {
    uint16 wWidth; //若是图片，则代表图片宽，若是字符串，则代表ID总数
    uint16 wHeight; //若是图片，则代表图片长，若是字符串，则代表该语言的ID.
    uint32 bType; //资源类型,0x01--language string ,0x02--PIC
    uint32 dwOffset; //图片数据区在文件内偏移,4 uint8s
    uint32 dwLength; //资源长度, 最大 4G，4 uint8s
} res_infor_t; //13 uint8s


/*多国语言资源ID索引表的数据结构*/
typedef struct {
    uint32 dwOffset; // 字符ID号对应字符串编码在文件内的偏移
    uint16 dwLength; //  字符串长度.即unicode编码字符串的字节数
} res_langid_entry_t; // 6 uint8s

typedef struct {
    uint8 filetype; //文件类型 0-- 目录  1  文件
    char name[12];
    int32 DirEntry;
} file_record_m;

// extern res_entry_t  res_entry;
u8 ResOpen(const char *filename);
void *ResGetFp();
uint16 ResShowPic(uint16 pic_id, uint8 x, uint8 y);
uint16 ResShowMultiString(uint8 x, uint8 y, uint16 StringID);
void ResClose();
u8 InitRes();




#define  LCDPAGE  8
#define  LCDCOLUMN   128
#define  SCR_WIDTH         LCDCOLUMN
#define  SCR_HEIGHT        (LCDPAGE*8)

#define  MENUICONWIDTH    12   //菜单项目左边图标的宽度(象素)
#define  MENUITEMHEIGHT   16   //菜单项目的高度(象素)
#define  SCROLLBARWIDTH    6   //垂直滚动条的宽度(象素)

extern u8 LCDBuff[LCDPAGE][LCDCOLUMN];


#endif
