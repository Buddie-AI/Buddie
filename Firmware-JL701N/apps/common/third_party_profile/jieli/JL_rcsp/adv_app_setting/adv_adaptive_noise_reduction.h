#ifndef __ADV_ADAPTIVE_NOISE_REDUCTION_H__
#define __ADV_ADAPTIVE_NOISE_REDUCTION_H__

#include "le_rcsp_adv_module.h"

/**
 * @brief 获取自适应降噪信息开关
 *
 * @result 1:自适应降噪开 0:自适应降噪关
 */
int get_adaptive_noise_reduction_switch();

/**
 * @brief 开启自适应降噪
 */
void set_adaptive_noise_reduction_on();

/**
 * @brief 关闭自适应降噪
 */
void set_adaptive_noise_reduction_off();

/**
 * @brief 自适应降噪重新检测
 */
void set_adaptive_noise_reduction_reset();

/**
 * @brief 获取自适应降噪重新检测状态
 *
 * @result 1:进行中 0:结束
 */
u8 get_adaptive_noise_reduction_reset_status();

/**
 * @brief 获取自适应降噪重新检测结果
 *
 * @result 1:失败 0:成功
 */
u8 get_adaptive_noise_reduction_reset_result();

/**
 * @brief 自适应降噪重新检测结果回调
 */
void set_adaptive_noise_reduction_reset_callback(u8 result);

#endif

