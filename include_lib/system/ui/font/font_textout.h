#ifndef __FONT_OUT_H__
#define __FONT_OUT_H__

#include "generic/typedef.h"
#include "font/font_all.h"

/**
 * @brief  打开字库
 *
 * @param info:字库信息
 * @param language:语言
 *
 * @returns   TRUE:打开成功 FALSE:打开失败
 */
struct font_info *font_open(struct font_info *info, u8 language);
/**
 * @brief 获取字符宽度
 *
 * @param info
 * @param str
 * @param strlen
 *
 * @returns
 */
u16 font_text_width(struct font_info *info, u8 *str, u16 strlen);
u16 font_textw_width(struct font_info *info, u8 *str, u16 strlen);
u16 font_textu_width(struct font_info *info, u8 *str, u16 strlen);

/**
 * @brief  字库内码显示接口
 *
 * @param info
 * @param str
 * @param strlen
 *
 * @returns
 */
u16 font_textout(struct font_info *info, u8 *str, u16 strlen, u16 x, u16 y);
/**
 * @brief  字库unicode显示接口
 *
 * @param info
 * @param str
 * @param strlen
 * @param x
 * @param y
 *
 * @returns
 */
u16 font_textout_unicode(struct font_info *info, u8 *str, u16 strlen, u16 x, u16 y);
/**
 * @brief  字库utf8显示接口
 *
 * @param info
 * @param str
 * @param strlen
 * @param x
 * @param y
 *
 * @returns
 */
u16 font_textout_utf8(struct font_info *info, u8 *str, u16 strlen, u16 x, u16 y);
/**
 * @brief  utf8转内码
 *
 * @param info
 * @param utf8
 * @param utf8len
 * @param ansi
 *
 * @returns
 */
u16 font_utf8toansi(struct font_info *info, u8 *utf8, u16 utf8len, u8 *ansi);
/**
 * @brief utf16转内码
 *
 * @param info
 * @param utf
 * @param len
 * @param ansi
 *
 * @returns
 */
u16 font_utf16toansi(struct font_info *info, u8 *utf, u16 len, u8 *ansi);
/**
 * @brief utf8转utf16
 *
 * @param info
 * @param utf8
 * @param utf8len
 * @param utf16
 *
 * @returns
 */
u16 font_utf8toutf16(struct font_info *info, u8 *utf8, u16 utf8len, u16 *utf16);
/**
 * @brief  字库关闭
 *
 * @param info
 */
void font_close(struct font_info *info);

#endif
