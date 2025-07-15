#ifndef __DIAFX_H__
#define __DIAFX_H__


#ifndef __Q8P24__
#define __Q8P24__
#endif


#include<stdint.h>
#include<string.h>

#ifndef __FMATH_H__
#include "fmath.h"
#endif // __FMATH_H__

#ifdef __cplusplus
extern "C" {
#endif


extern int diafx_size();

// create a DIA handler
extern void *diafx_init_mode(int mode);

// clear buffer
extern void diafx_clear();

//render raw pcm into processed one.
extern uint32_t diafx_run_Q8P24(inumber  **inPcms, inumber **outPcms, int numFrames);
extern uint32_t diafx_run_Q8P24_interleave(inumber *inPcms, int numFrames);
uint32_t diafx_run_Q8P24_int16_interleave(int16_t *inPcms, int numFrames);
void diafx_enabled(int value);
void diafx_toggle_enabled();
uint32_t jl_platform_diafx_run(int16_t *inPcms, int numFrames);

#define diafx_init()	diafx_init_mode(0)

#ifdef __cplusplus
}
#endif

#endif

