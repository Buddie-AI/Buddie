#ifndef DEVICE_TOUCH_KEY_H
#define DEVICE_TOUCH_KEY_H

#include "typedef.h"
#include "asm/plcnt.h"

/* =========== touch key API ============= */
//触摸按键初始化
int touch_key_init(const struct touch_key_platform_data *touch_key_data);

//获取触摸按键键值
u8 touch_key_get_value(void);


#endif

