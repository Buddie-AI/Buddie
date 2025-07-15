/*
 ****************************************************************
 *							Amplitude Statistics
 *								幅值统计模块
 * File  : amplitude_statistic.c
 * By    :
 * Notes :
 ****************************************************************
 */

#include "amplitude_statistic.h"

#if 0
#define amplitude_log	printf
#else
#define amplitude_log(...)
#endif/*log_en*/

static  const unsigned short am_va_tab[] = {
    32768, 29205, 26029, 23198, 20675, 18427, 16423, 14637, 13045, 11627, 10362, 9235, 8231, 7336, 6538, 5827,
    5193, 4629, 4125, 3677, 3277, 2920, 2603, 2320, 2068, 1843, 1642, 1464, 1305, 1163, 1036, 0
};

static const int  rms_va_tab[] = {
    134217728, 106612931, 84685661, 67268212, 53433040, 42443372, 33713969, 26779957, 21272076, 16897011,
    13421773, 10661293, 8468566, 6726821, 5343304, 4244337, 3371397, 2677996, 2127208, 1689701,
    1342177, 1066129, 846857, 672682, 534330, 424434, 337140, 267800, 212721, 168970,
    134218, 0
};

/*
*********************************************************************
*                  Loudness meter init
* Description: 响度计算初始化
* Arguments  : loud_obj
*			   sr
*			   print_dest
*			   index
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
void loudness_meter_init(LOUDNESS_M_STRUCT *loud_obj, int sr, int print_dest, u8 index)
{
    memset(loud_obj, 0, sizeof(LOUDNESS_M_STRUCT));
    loud_obj->countperiod = sr / 50;
    loud_obj->inv_counterpreiod = 16777216 / loud_obj->countperiod;
    loud_obj->print_dest = print_dest;
    loud_obj->index = index;
}

/*
*********************************************************************
*                  Loudness meter short
* Description: 响度计算函数
* Arguments  : loud_obj
*			   data
*			   len
* Return	 : NULL
* Note(s)    : NULL
*********************************************************************
*/
void loudness_meter_short(LOUDNESS_M_STRUCT *loud_obj, short *data, int len)
{
    int i;
    int loudness_val = 0;

    for (i = 0; i < len; i++) {
        int tmp = data[i] * 256;
        loud_obj->dclevel = (loud_obj->dclevel * 511 + tmp) >> 9;

        int xabs = (data[i] > 0) ? data[i] : (-data[i]);
        int xabs2 = (xabs * xabs) >> 10;
        loud_obj->rms += xabs2;
        loud_obj->maxval = (loud_obj->maxval > xabs) ? loud_obj->maxval : xabs;

        if (xabs >= 32767) {
            loud_obj->errprintfcount0++;
        }

        loud_obj->counti++;
        if (loud_obj->counti > loud_obj->countperiod) {
            if (loud_obj->maxval_print < loud_obj->maxval) {
                loud_obj->maxval_print = loud_obj->maxval;
            }

            if (loud_obj->rms_print < loud_obj->rms) {
                loud_obj->rms_print = loud_obj->rms;
            }

            loud_obj->counti = 0;
            loud_obj->maxval = 0;
            loud_obj->rms = 0;

            if (loud_obj->errprintfcount0 > 2) {
                loud_obj->errprintfcount0 = 0;
                amplitude_log("[%d]overflow occur... \n", loud_obj->index);
            }

            loud_obj->print_cnt++;

            if (loud_obj->print_cnt >= loud_obj->print_dest) {

                int rmsval = ((__int64)loud_obj->rms_print * (__int64)loud_obj->inv_counterpreiod) >> (24 - (10 - 3));

                if (rmsval > 25837266) {
                    amplitude_log("[%d]energy  high... \n", loud_obj->index);
                }

                {
                    int compi = 0, found = 0;;
                    while (compi < 31) {
                        if (rmsval >= rms_va_tab[compi]) {
//							float  upval = (0 - compi) * 0.5;
//							float  dwonval = (0 - compi - 1) * 0.5;
//							amplitude_log("rms level: %f to %f dB\n", upval, dwonval);

                            int  upval = (0 - compi);
                            int  dwonval = (0 - compi - 1);

                            amplitude_log("[%d]rms level: %d to %d dB\n", loud_obj->index, upval, dwonval);

                            found = 1;
                            break;
                        }
                        compi++;
                    }

                    if (found == 0) {
                        amplitude_log("[%d]rms level < -30dB \n", loud_obj->index);
                    }
                }

                {
                    int compi = 0, found = 0;;
                    while (compi < 31) {
                        if (loud_obj->maxval_print >= am_va_tab[compi]) {
//							float  upval = (0 - compi) * 0.5;
//							float  dwonval = (0 - compi - 1) * 0.5;
//							amplitude_log("peak level: %f to %f dB\n", upval, dwonval);
                            int  upval = (0 - compi);
                            int  dwonval = (0 - compi - 1);
                            loud_obj->peak_val = dwonval;
                            amplitude_log("[%d]peak level: %d to %d dB\n", loud_obj->index, upval, dwonval);

                            found = 1;
                            break;
                        }

                        compi++;
                    }

                    if (found == 0) {
                        loud_obj->peak_val = -31;
                        amplitude_log("[%d]peak level < -30dB \n", loud_obj->index);
                    }
                }

                loud_obj->print_cnt = 0;
                loud_obj->maxval_print = 0;
                loud_obj->rms_print = 0;

                if (loud_obj->dclevel > (255 * 256)) {
                    amplitude_log("[%d] why ??? dc level : %d \n", loud_obj->index, loud_obj->dclevel >> 8);
                }
                amplitude_log("\n\n");
            }
        }
    }
}

#if 0
void loudness_meter_demo(void)
{
    loudness_meter_init(&loudness_adc, 16000, 1);
    loudness_meter_short(&loudness_adc, datain, points);
}
#endif


