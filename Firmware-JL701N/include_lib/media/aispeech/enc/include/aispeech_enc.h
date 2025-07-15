#ifndef _AISPEECH_NR_H_
#define _AISPEECH_NR_H_

#ifdef __cplusplus
extern "C" {
#endif

int Aispeech_NR_getmemsize(short sample_rate);
int Aispeech_NR_init(char *pcMemPool, unsigned int  memPoolLen, short sample_rate);
int Aispeech_NR_run(short *mic0, short *mic1, short *mic2, short *ref, short *out, short points);
int Aispeech_NR_deinit(void);
#ifdef __cplusplus
};

#endif
#endif

