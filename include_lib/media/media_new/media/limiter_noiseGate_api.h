#ifndef LIMITER_NOISEGATE_API_H
#define LIMITER_NOISEGATE_API_H

extern int need_limiter_noiseGate_buf(int x);
extern void limiter_noiseGate_init(void *work_buf, int limiter_attfactor, int limiter_relfactor, int noiseGate_attfactor, int noiseGate_relfactor, int limiter_threshold, int noiseGate_threshold, int noiseGate_low_thr_gain, int sample_rate, int channel);
extern int limiter_noiseGate_run(void *work_buf, void *in_buf, void *out_buf, int point_per_channel);

#endif // !LIMITER_NOISEGATE_API_H

