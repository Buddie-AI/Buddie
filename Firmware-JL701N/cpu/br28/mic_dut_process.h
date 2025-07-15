#ifndef _MIC_DUT_PROCESS_H_
#define _MIC_DUT_PROCESS_H_

#include "generic/typedef.h"
#include "online_debug/aud_mic_dut.h"

/*获取麦克风测试硬件数据*/
int audio_mic_dut_info_get(mic_dut_info_t *info);

/*设置mic增益*/
int audio_mic_dut_gain_set(u16 gain);

/*打开mic*/
int audio_mic_dut_start(void);

/*关闭mic*/
int audio_mic_dut_stop(void);

/*获取mic数据*/
int audio_mic_dut_data_get(s16 *data, int len);

/*获取mic缓存数据长度*/
int audio_mic_dut_get_data_len(void);

int audio_mic_dut_sample_rate_set(u32 sr);
int audio_mic_dut_amic_select(u8 idx);
int audio_mic_dut_dmic_select(u8 idx);

/*设置dac输出音量*/
int audio_mic_dut_dac_gain_set(u16 gain);

/*开始mic频响扫描*/
int audio_mic_dut_scan_start(void);

/*关闭mic频响扫描*/
int audio_mic_dut_scan_stop(void);

#endif /*_MIC_DUT_PROCESS_H_*/

