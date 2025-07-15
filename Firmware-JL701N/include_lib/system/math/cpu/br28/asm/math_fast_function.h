
#ifndef MATH_FAST_FUNCTION

#define MATH_FAST_FUNCTION

#ifndef MATH_USE_LIMIT
#define MATH_USE_LIMIT 1
#endif

struct data_q_struct {
    long data;
    char q;
};


extern  long  cos_fix(long x);
extern  float cos_float(float x);
extern  long  sin_fix(long x);
extern  float sin_float(float x);
extern  struct data_q_struct complex_abs_fix(long x, long y);
extern  struct data_q_struct complex_dqdt_fix(long x, long y);
extern  float  complex_abs_float(float x, float y);
extern  float  complex_dqdt_float(float x, float y);
extern  struct data_q_struct root_fix(struct data_q_struct x);
extern  float  root_float(float x);
extern  struct data_q_struct mul_fix(long x, long y);
extern  float  mul_float(float x, float y);
extern  struct data_q_struct div_fix(long x, long y);
extern  float  div_float(float x, float y);
extern  struct data_q_struct exp_fix(long x);
extern  float  exp_float(float x);
extern  struct data_q_struct ln_fix(struct data_q_struct x);
extern  float  ln_float(float x);
extern  struct data_q_struct angle_fix(long x, long y);
extern  float angle_float(float x, float y);
extern  struct data_q_struct atanh_fix(long x, long y);
extern  float atanh_float(float x, float y);
extern  long  cosh_fix(long x);
extern  float cosh_float(float x);
extern  long  sinh_fix(long x);
extern  float sinh_float(float x);
extern  struct data_q_struct log10_fix(struct data_q_struct x);
extern  float  log10_float(float x);
extern  struct data_q_struct sigmoid_fix(float x);
extern  float  sigmoid_float(float x);
extern  struct data_q_struct tanh_fix(float x);
extern  float  tanh_float(float x);

float dB_Convert_Mag(float x);

//==== add for limit input data range ====//
extern float atanh_float_limit(float x, float y);

#endif
