#include "adv_adaptive_noise_reduction.h"
#include "syscfg_id.h"
#include "user_cfg_id.h"
#include "rcsp_adv_bluetooth.h"

#if (RCSP_ADV_EN && RCSP_ADV_ANC_VOICE && RCSP_ADV_ADAPTIVE_NOISE_REDUCTION)

#include "audio_anc.h"


/**
 * @brief 获取自适应降噪信息开关
 *
 * @result 1:自适应降噪开 0:自适应降噪关
 */
int get_adaptive_noise_reduction_switch()
{
    int sw_result = audio_anc_coeff_mode_get();
    return sw_result;
}

/**
 * @brief 开启自适应降噪
 */
void set_adaptive_noise_reduction_on()
{
    audio_anc_coeff_adaptive_set(1, 1, 1);
}

/**
 * @brief 关闭自适应降噪
 */
void set_adaptive_noise_reduction_off()
{
    audio_anc_coeff_adaptive_set(0, 1, 1);
}

static u8 adaptive_noise_reduction_reseting = 0; // 重新检测状态，1:进行中 0:结束
static u8 adaptive_noise_reduction_reset_result = 0; // 重新检测结果，1:失败 0:成功
/**
 * @brief 自适应降噪重新检测
 */
void set_adaptive_noise_reduction_reset()
{
    audio_anc_mode_ear_adaptive();
    adaptive_noise_reduction_reseting = 1;
}

/**
 * @brief 获取自适应降噪重新检测状态
 *
 * @result 1:进行中 0:结束
 */
u8 get_adaptive_noise_reduction_reset_status()
{
    return adaptive_noise_reduction_reseting;
}

/**
 * @brief 获取自适应降噪重新检测结果
 *
 * @result 1:失败 0:成功
 */
u8 get_adaptive_noise_reduction_reset_result()
{
    return adaptive_noise_reduction_reset_result;
}

/**
 * @brief 自适应降噪重新检测结果回调
 */
void set_adaptive_noise_reduction_reset_callback(u8 result)
{
    adaptive_noise_reduction_reseting = 0;
    if (result != 0) {
        // 成功
        adaptive_noise_reduction_reset_result = 0;
    } else {
        // 失败
        adaptive_noise_reduction_reset_result = 1;
    }
    // 通知手机APP
    /* JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_ANC_VOICE, NULL, 0); */
    JL_rcsp_event_to_user(DEVICE_EVENT_FROM_RCSP, MSG_JL_UPDATE_ADAPTIVE_NOISE_REDUCTION, NULL, 0);
}

#endif


