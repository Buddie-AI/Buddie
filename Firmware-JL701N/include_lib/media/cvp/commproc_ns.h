#ifndef _COMMPROC_NS_H_
#define _COMMPROC_NS_H_

#include "generic/typedef.h"

typedef struct {
    char  wideband;
    char  mode;
    float AggressFactor;
    float MinSuppress;
    float NoiseLevel;
} noise_suppress_param;

int noise_suppress_frame_point_query(noise_suppress_param *param);
int noise_suppress_mem_query(noise_suppress_param *param);
int noise_suppress_open(noise_suppress_param *param);
int noise_suppress_close(void);
int noise_suppress_run(short *in, short *out, int npoint);

enum {
    NS_CMD_NOISE_FLOOR = 1,
    NS_CMD_LOWCUTTHR,
};
int noise_suppress_config(u32 cmd, int arg, void *priv);

#endif/*_COMMPROC_NS_H_*/
