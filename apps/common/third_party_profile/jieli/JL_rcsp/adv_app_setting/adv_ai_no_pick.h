#ifndef __ADV_AI_NO_PICK_H__
#define __ADV_AI_NO_PICK_H__

#include "system/includes.h"

typedef enum {
    RCSP_AI_NO_PICK_SENSITIVITY_HIGH			= 0x00,
    RCSP_AI_NO_PICK_SENSITIVITY_LOW				= 0x01,
} RCSP_AI_NO_PICK_SENSITIVITY;

typedef enum {
    RCSP_AI_NO_PICK_AUTO_CLOSE_TIME_NONE		= 0x00,			// 不自动关闭
    RCSP_AI_NO_PICK_AUTO_CLOSE_TIME_5s			= 0x01,
    RCSP_AI_NO_PICK_AUTO_CLOSE_TIME_15s			= 0x02,
    RCSP_AI_NO_PICK_AUTO_CLOSE_TIME_30s			= 0x03,
} RCSP_AI_NO_PICK_AUTO_CLOSE_TIME;

/**
 * @brief 获取智能免摘开关
 *
 * @result bool
 */
bool get_ai_no_pick_switch();

/**
 * @brief 开启智能免摘
 */
void set_ai_no_pick_switch(bool p_switch);

/**
 * @brief 获取智能免摘敏感度
 *
 * @result RCSP_AI_NO_PICK_SENSITIVITY
 */
RCSP_AI_NO_PICK_SENSITIVITY get_ai_no_pick_sensitivity();

/**
 * @brief 设置智能免摘敏感度
 */
void set_ai_no_pick_sensitivity(RCSP_AI_NO_PICK_SENSITIVITY sensitivity);

/**
 * @brief 获取智能免摘自动关闭时间
 *
 * @result RCSP_AI_NO_PICK_AUTO_CLOSE_TIME
 */
RCSP_AI_NO_PICK_AUTO_CLOSE_TIME get_ai_no_pick_auto_close_time();

/**
 * @brief 设置智能免摘自动关闭时间
 */
void set_ai_no_pick_auto_close_time(RCSP_AI_NO_PICK_AUTO_CLOSE_TIME time_type);

#endif // __ADV_AI_NO_PICK_H__
