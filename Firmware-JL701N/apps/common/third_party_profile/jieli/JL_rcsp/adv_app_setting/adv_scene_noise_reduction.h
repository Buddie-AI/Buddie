#ifndef __ADV_SCENE_NOISE_REDUCTION_H__
#define __ADV_SCENE_NOISE_REDUCTION_H__

#include "system/includes.h"

typedef enum {
    RCSP_SCENE_NOISE_REDUCTION_TYPE_INTELLIGENT = 0x00,
    RCSP_SCENE_NOISE_REDUCTION_TYPE_MILD		= 0x01,
    RCSP_SCENE_NOISE_REDUCTION_TYPE_BALANCE		= 0x02,
    RCSP_SCENE_NOISE_REDUCTION_TYPE_DEEPNESS	= 0x03,
} RCSP_SCENE_NOISE_REDUCTION_TYPE;

/**
 * @brief 获取场景降噪类型
 *
 * @result RCSP_SCENE_NOISE_REDUCTION_TYPE
 */
RCSP_SCENE_NOISE_REDUCTION_TYPE get_scene_noise_reduction_type();

/**
 * @brief 设置场景降噪类型
 */
void set_scene_noise_reduction_type(RCSP_SCENE_NOISE_REDUCTION_TYPE type);



#endif // __ADV_SCENE_NOISE_REDUCTION_H__
