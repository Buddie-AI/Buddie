
#ifndef _PDM_LINK_H_
#define _PDM_LINK_H_

#include "generic/typedef.h"
#define PLNK_CH0_EN		BIT(0)
#define PLNK_CH1_EN		BIT(1)
#define PLNK_CH_EN		PLNK_CH0_EN //(PLNK_CH0_EN | PLNK_CH1_EN)
#define PLNK_SCLK_PIN	IO_PORTA_14
#define PLNK_DAT0_PIN	IO_PORTA_15
#define PLINK_BUF_LEN       256
#define PLNK_CLK		24000000 //48M
#define PLNK_SCLK		2400000 //1M ~ 4M
#define PLNK_CON_RESET()	do {JL_PLNK->CON = 0; JL_PLNK->CON1 = 0;} while(0)
#define PLNK_CH0_MD_SET(x)	SFR(JL_PLNK->CON, 2, 2, x)
#define PLNK_CH1_MD_SET(x)	SFR(JL_PLNK->CON, 6, 2, x)
#define PLNK_PND_CLR()		SFR(JL_PLNK->CON, 14, 1, 1); //CPND

/*通道0输入模式选择*/
typedef enum {
    CH0MD_CH0_SCLK_RISING_EDGE,
    CH0MD_CH0_SCLK_FALLING_EDGE,
    CH0MD_CH1_SCLK_RISING_EDGE,
    CH0MD_CH1_SCLK_FALLING_EDGE,
} PDM_LINK_CH0MD;

/*通道1输入模式选择*/
typedef enum {
    CH1MD_CH1_SCLK_RISING_EDGE,
    CH1MD_CH1_SCLK_FALLING_EDGE,
    CH1MD_CH0_SCLK_RISING_EDGE,
    CH1MD_CH0_SCLK_FALLING_EDGE,
} PDM_LINK_CH1MD;

typedef struct {
    u8 sclk_io;
    u8 ch_num;		/*使能多少个通道*/
    u8 ch0_io;
    u8 ch1_io;
    u8 ch0_mode;	/*通道0输入模式选择*/
    u8 ch1_mode;	/*通道1输入模式选择*/
    u16 sr;			/*采样率*/
    u16 buf_len;	/*一次采样点数*/
    s16 *buf;
    void (*output)(void *buf, u16 len);
} audio_plnk_t;

int audio_plnk_open(audio_plnk_t *hdl);
int audio_plnk_start(audio_plnk_t *hdl);
int audio_plnk_close(void);
void audio_plnk_test();

#endif/*_PDM_LINK_H_*/
