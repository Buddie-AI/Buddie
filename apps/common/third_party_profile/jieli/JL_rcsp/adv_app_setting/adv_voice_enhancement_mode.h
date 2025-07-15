#ifndef __ADV_VOICE_ENHANCEMENT_MODE_H__
#define __ADV_VOICE_ENHANCEMENT_MODE_H__

#include "system/includes.h"

/**
 * @brief 获取人声增强模式开关
 *
 * @result bool
 */
bool get_voice_enhancement_mode_switch();

/**
 * @brief 设置人声增强模式开关
 */
void set_voice_enhancement_mode_switch(bool mode_switch);

#endif // __ADV_VOICE_ENHANCEMENT_MODE_H__
