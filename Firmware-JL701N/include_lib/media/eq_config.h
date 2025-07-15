#ifndef CONFIG_EFFECT_CORE_V2_ENABLE
#include "application/eq_config.h"
#else

#ifndef _EQ_CONFIG_H_
#define _EQ_CONFIG_H_

#include "media/audio_eq.h"
#include "media/audio_drc.h"
#include "spinlock.h"
#include "math.h"

/*----------------------------------------------------------------------------*/
/**@brief   用默认eq系数表的eq效果模式设置(设置模式,更新系数)
  @param   mode:EQ_MODE_NORMAL, EQ_MODE_ROCK,EQ_MODE_POP,EQ_MODE_CLASSIC,EQ_MODE_JAZZ,EQ_MODE_COUNTRY, EQ_MODE_CUSTOM
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/

int eq_mode_set(EQ_MODE mode);
/*----------------------------------------------------------------------------*/
/**@brief   eq模式切换
  @param
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_sw(void);
/*----------------------------------------------------------------------------*/
/**@brief   获取eq效果模式
  @param
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
EQ_MODE eq_mode_get_cur(void);
/*----------------------------------------------------------------------------*/
/**@brief   设置custom系数表的某一段系数
  @param   seg->index：第几段(从0开始)
  @param   seg->iir_type:滤波器类型(EQ_IIR_TYPE_HIGH_PASS, EQ_IIR_TYPE_LOW_PASS, EQ_IIR_TYPE_BAND_PASS, EQ_IIR_TYPE_HIGH_SHELF,EQ_IIR_TYPE_LOW_SHELF)
  @param   seg->freq:中心截止频率(20~22kHz)
  @param   seg->gain:总增益(-18~18)
  @param   seg->q : q值（0.3~30）
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_seg(struct eq_seg_info *seg);
/*----------------------------------------------------------------------------*/
/**@brief   获取custom系数表的增益、频率
  @param   index:哪一段
  @param   freq:中心截止频率
  @param   gain:增益
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_info(u16 index, int freq, float gain);
/*----------------------------------------------------------------------------*/
/**@brief   获取某eq系数表一段eq的增益
  @param   mode:哪个模式
  @param   index:哪一段
  @return  增益
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
s8 eq_mode_get_gain(EQ_MODE mode, u16 index);
/*----------------------------------------------------------------------------*/
/**@brief   获取某eq系数表一段eq的中心截止频率
  @param   mode:EQ_MODE_NORMAL, EQ_MODE_ROCK,EQ_MODE_POP,EQ_MODE_CLASSIC,EQ_MODE_JAZZ,EQ_MODE_COUNTRY, EQ_MODE_CUSTOM
  @param   index:哪一段
  @return  中心截止频率
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_get_freq(EQ_MODE mode, u16 index);

/*----------------------------------------------------------------------------*/
/**@brief   设置用custom系数表一段eq的增益
  @param   index:哪一段
  @param   gain:增益
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_param(u16 index, int gain);

/*----------------------------------------------------------------------------*/
/**@brief    设置播歌用的eq系数表的总增益
   @param   global_gain:总增益
   @param    mode:枚举型EQ_MODE
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void set_global_gain(EQ_MODE mode, float global_gain);
/*
 *mode:枚举型EQ_MODE
 *return 返回对应系数表的段数
 * */
u8 eq_get_table_nsection(EQ_MODE mode);

#endif
#endif /*CONFIG_EFFECT_CORE_V2_ENABLE*/
