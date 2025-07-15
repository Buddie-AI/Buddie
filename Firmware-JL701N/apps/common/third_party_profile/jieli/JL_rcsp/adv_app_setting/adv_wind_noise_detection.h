#ifndef __ADV_WIND_NOISE_DETECTION_H__
#define __ADV_WIND_NOISE_DETECTION_H__

#include "system/includes.h"

/**
 * @brief 获取风噪检测开关
 *
 * @result bool
 */
bool get_wind_noise_detection_switch();

/**
 * @brief 设置风噪检测开关
 */
void set_wind_noise_detection_switch(bool detection_switch);

#endif // __ADV_WIND_NOISE_DETECTION_H__
