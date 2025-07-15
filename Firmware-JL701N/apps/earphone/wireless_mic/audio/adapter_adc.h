
#ifndef __ADAPTER_ADC_H___
#define __ADAPTER_ADC_H___

#include "generic/typedef.h"

void adapter_adc_init(void);
struct audio_adc_hdl *adapter_adc_get_handle(void);
int adapter_adc_mic_data_read(void *hdl, void *data, int len);
int adapter_adc_mic_open(u16 sample_rate, u8 gain, u16 irq_points);
void adapter_adc_mic_data_output_resume_register(void (*callback)(void *priv), void *priv);
void adapter_adc_mic_data_callback_register(void (*callback)(void *priv, u8 *data, u16 len), void *priv);
void adapter_adc_mic_close(void);
void adapter_adc_mic_start(void);

#endif//__ADAPTER_ADC_H___

