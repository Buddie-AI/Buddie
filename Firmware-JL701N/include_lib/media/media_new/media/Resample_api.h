#ifndef __RESAMPLE_API_H__
#define __RESAMPLE_API_H__

// #include "br18_dsp_Ecommon.h"

typedef struct _RS_IO_CONTEXT_ {
    void *priv;
    int(*output)(void *priv, void *data, int len);
} RS_IO_CONTEXT;

typedef struct _RS_PARA_STRUCT_ {
    unsigned int new_insample;
    unsigned int new_outsample;
    int nch;
} RS_PARA_STRUCT;

typedef struct  _RS_STUCT_API_ {
    unsigned int(*need_buf)();
    void (*open)(unsigned int *ptr, RS_PARA_STRUCT *rs_para);
    void (*set_sr)(unsigned int *ptr, int newsrI);
    int(*run)(unsigned int *ptr, short *inbuf, int len, short *obuf);  //len是n个点，返回n个点
} RS_STUCT_API;

RS_STUCT_API *get_rs16_context();
RS_STUCT_API *get_rsfast_context();

#endif
