#ifndef __ATTR_H__
#define __ATTR_H__
#include "typedef.h"

typedef u32(*attr_get_func)(void *priv, u8 attr, u8 *buf, u16 buf_size, u32 offset);
typedef void (*attr_set_func)(void *priv, u8 attr, u8 *data, u16 len);

u16 add_one_attr_huge(u8 *buf, u16 max_len, u8 offset, u8 type, u8 *data, u16 size);
u8 add_one_attr(u8 *buf, u16 max_len, u8 offset, u8 type, u8 *data, u8 size);
u8 add_one_attr_ex(u8 *buf, u16 max_len, u8 offset, u8 type, u8 *data, u8 size, u8 att_size);
u8 add_one_attr_continue(u8 *buf, u16 max_len, u8 offset, u8 type, u8 *data, u8 size);
u32 attr_get(void *priv, u8 *buf, u16 buf_size, const attr_get_func *func_tab, u8 attr_max, u32 mask);
void attr_set(void *priv, u8 *data, u16 len, const attr_set_func *func_tab, u8 attr_max);

#endif//__ATTR_H__
