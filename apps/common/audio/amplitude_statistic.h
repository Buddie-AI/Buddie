#ifndef _AMPLITUDE_STATISTIC_H_
#define _AMPLITUDE_STATISTIC_H_

#include "generic/typedef.h"

typedef  struct _LOUDNESS_M_STRUCT_ {
    int mutecnt;
    int rms;
    int counti;
    int maxval;
    int countperiod;
    int inv_counterpreiod;
    int errprintfcount0;
    short print_cnt;
    short print_dest;
    int dclevel;
    int rms_print;
    int maxval_print;
    int peak_val;
    u8 index;
} LOUDNESS_M_STRUCT;

void  loudness_meter_init(LOUDNESS_M_STRUCT *loud_obj, int sr, int print_dest, u8 index);
void  loudness_meter_short(LOUDNESS_M_STRUCT *loud_obj, short *data, int len);

#endif /*_AMPLITUDE_STATISTIC_H_*/
