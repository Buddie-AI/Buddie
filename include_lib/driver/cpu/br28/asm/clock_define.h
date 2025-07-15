

#ifndef _CPU_CLOCK_DEFINE__
#define _CPU_CLOCK_DEFINE__

///原生时钟源作系统时钟源
#define         SYS_CLOCK_INPUT_RC  0
#define         SYS_CLOCK_INPUT_BT_OSC  1          //BTOSC 双脚(12-26M)
#define         SYS_CLOCK_INPUT_RTOSCH  2
#define         SYS_CLOCK_INPUT_RTOSCL  3
#define         SYS_CLOCK_INPUT_PAT     4

///衍生时钟源作系统时钟源
#define         SYS_CLOCK_INPUT_PLL_BT_OSC  5
#define         SYS_CLOCK_INPUT_PLL_RTOSCH  6
#define         SYS_CLOCK_INPUT_PLL_PAT     7
#define         SYS_CLOCK_INPUT_PLL_RCL     8

///VAD时钟源
#define         VAD_CLOCK_USE_BTOSC                 0 //DVAD、ANALOG使用BTOSC
#define         VAD_CLOCK_USE_RC_AND_BTOSC          1 //DVAD使用RC、BTOSC直连ANALOG
#define         VAD_CLOCK_USE_PMU_STD12M            2 //DVAD使用BTOSC通过PMU配置的STD12M
#define         VAD_CLOCK_USE_LRC                   3 //DVAD使用LRC

//ANC时钟源
#define			ANC_CLOCK_USE_CLOSE					0 //ANC关闭，无需保持相关时钟
#define			ANC_CLOCK_USE_BTOSC					1 //ANC使用BTOSCX2时钟
#define			ANC_CLOCK_USE_PLL					2 //ANC使用PLL时钟
#endif

