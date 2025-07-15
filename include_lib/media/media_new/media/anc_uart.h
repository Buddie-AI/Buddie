#ifndef _ANC_DEBUG_H_
#define _ANC_DEBUG_H_

#include "generic/typedef.h"
#include "generic/circular_buf.h"
#include "system/includes.h"

#define ANC_CBUF_SIZE		8000
typedef struct {
    /* struct audio_adc_output_hdl adc_output; */
    /* struct adc_mic_ch mic_ch; */
    /* s16 adc_buf[ADC_DM_BUFS_SIZE];    //align 4Bytes */
    /* s16 mic_tmp_data[ADC_DM_IRQ_POINTS]; */
    u8 rx_num;
    u8 print_num;		//打印状态
    u8 tx_enable;		//打印总开关
    u8	tx_host;		//打印模式
    u32 tx_timeout;		//超时打印时间，单位秒
    int rx_vaule;

    char temp_strbuf[40];
    char temp_intbuf[12];
    u8 tx_buf[ANC_CBUF_SIZE];
    cbuffer_t tx_cb;
    int (*uart_write)(u8 *buf, u8 len);
} anc_debug_t;
extern anc_debug_t *anc_dbg;

void anc_uart_init(int (*uart_write_hdl)(u8 *buf, u8 len));
int anc_intlog(u32 int_para, u8 log_more);
int anc_strlog(char *str, u8 log_more);
void anc_uart_ctl();
int anc_uart_process(u8 *dat, int len);

#endif/*_ANC_DEBUG_H_*/
