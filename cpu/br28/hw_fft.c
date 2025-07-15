#include "includes.h"

/*FFT模块资源互斥方式配置*/
#define CRITICAL_NULL 		0
#define CRITICAL_IRQ 		1
#define CRITICAL_MUTEX		2
#define CRITICAL_SPIN_LOCK 	3
#define FFT_CRITICAL_MODE	CRITICAL_SPIN_LOCK

#if (FFT_CRITICAL_MODE == CRITICAL_MUTEX)
static OS_MUTEX fft_mutex;
#define FFT_CRITICAL_INIT() 	os_mutex_create(&fft_mutex)
#define FFT_ENTER_CRITICAL() 	os_mutex_pend(&fft_mutex, 0)
#define FFT_EXIT_CRITICAL() 	os_mutex_post(&fft_mutex)
#elif (FFT_CRITICAL_MODE == CRITICAL_IRQ)
#define FFT_CRITICAL_INIT(...)
#define FFT_ENTER_CRITICAL() 	local_irq_disable()
#define FFT_EXIT_CRITICAL() 	local_irq_enable()
#elif (FFT_CRITICAL_MODE == CRITICAL_SPIN_LOCK)
static spinlock_t fft_lock;
#define FFT_CRITICAL_INIT() 	spin_lock_init(&fft_lock)
#define FFT_ENTER_CRITICAL() 	spin_lock(&fft_lock)
#define FFT_EXIT_CRITICAL() 	spin_unlock(&fft_lock)

#endif /*FFT_CRITICAL_MODE*/

static volatile u8 fft_init = 0;

typedef struct {
    unsigned int fft_config;
    const int *in;
    int *out;
} pi32v2_hw_fft_ctx;

#define  FFT_ISR_IE     0
void hw_fft_wrap(pi32v2_hw_fft_ctx *ctx)
{
    if (fft_init == 0) {
        fft_init = 1;
        FFT_CRITICAL_INIT();
    }
    FFT_ENTER_CRITICAL();

    JL_FFT->CON = 0;
    JL_FFT->CON |= 1 << 8;
    JL_FFT->CADR = (unsigned int)ctx;
    JL_FFT->CON = (1 << 8) | (0 << 6) | (FFT_ISR_IE << 2) | (0 << 1) | (1 << 0);   //((1<<8)|(1<<0))
    while ((JL_FFT->CON & (1 << 7)) == 0);
    JL_FFT->CON |= (1 << 6);

    FFT_EXIT_CRITICAL();
}

static int REAL_FFT_CONFIG[6] = {
    809501970, 813696546, 822085426, 838862898, 872417602, 939526722,
};

/**** Real IFFT *******/
static int REAL_IFFT_CONFIG[6] = {
    809501958, 1082131974, 822085382, 1107298310, 872417542, 1207962118,
};

/****************************
 *
 * Complex Support points
 * 32\64\128\256\512\1024\2048
 *
 * *************************/
/**** Complex FFT *******/
static int COMPLEX_FFT_CONFIG[7] = {
    538969360, 541066784, 545261344, 553650224, 570427696, 603982400, 671091520,
};

/**** Complex IFFT *******/
static int COMPLEX_IFFT_CONFIG[7] = {
    1075840276, 809502212, 1082132244, 822085636, 1107298580, 872417796, 1207962388,
};



/*********************************************************************
*                  hw_fft_config
* Description: 根据配置生成 FFT_config
* Arguments  : N 运算数据量；
        	   log2N 运算数据量的对数值
        	   is_same_addr 输入输出是否同一个地址，0:否，1:是
        	   is_ifft 运算类型 0:FFT运算, 1:IFFT运算
        	   is_real 运算数据的类型  1:实数, 0:复数
* Return     : ConfgPars 写入FFT寄存器
* Note(s)    : None.
*********************************************************************/
unsigned int hw_fft_config(int N, int log2N, int is_same_addr, int is_ifft, int is_real)
{
    unsigned int ConfgPars;
    ConfgPars = 0;
    if (is_real == 1) {
        if (is_ifft == 0) {
            ConfgPars = REAL_FFT_CONFIG[log2N - 6];
            ConfgPars |= is_same_addr;
        } else {
            ConfgPars = REAL_IFFT_CONFIG[log2N - 6];
            ConfgPars |= is_same_addr;
        }
    } else {
        if (is_ifft == 0) {
            ConfgPars = COMPLEX_FFT_CONFIG[log2N - 5];
            ConfgPars |= is_same_addr;
        } else {
            ConfgPars = COMPLEX_IFFT_CONFIG[log2N - 5];
            ConfgPars |= is_same_addr;
        }
    }
    /* printf("ConfgPars%x\n",ConfgPars); */
    return ConfgPars;
}

/*********************************************************************
*                  hw_fft_run
* Description: fft/ifft运算函数
* Arguments  :fft_config FFT运算配置寄存器值
              in  输入数据地址(满足4byte对其，以及非const类型)
              out 输出数据地址
* Return   : void
* Note(s)    : None.
*********************************************************************/
void hw_fft_run(unsigned int fft_config, const int *in, int *out)
{
    pi32v2_hw_fft_ctx ctx;
    ctx.fft_config = fft_config;
    ctx.in = in;
    ctx.out = out;
    hw_fft_wrap(&ctx);
}

//////////////////////////////////////////////////////////////////////////
//							VECTOR
//////////////////////////////////////////////////////////////////////////

typedef struct {
    unsigned long  vector_con;
    unsigned long  vector_xadr;
    unsigned long  vector_yadr;
    unsigned long  vector_zadr;
    unsigned long  vector_config0;
    unsigned long  vector_config1;
    unsigned long  vector_config2;
    unsigned long  null;
} hwvec_ctx_t;

static hwvec_ctx_t g_vector_core_set __attribute__((aligned(16)));

//__attribute__ ((always_inline)) inline
void hwvec_exec(
    void *xptr,
    void *yptr,
    void *zptr,
    short x_inc,
    short y_inc,
    short z_inc,
    short nlen,
    short nloop,
    char q,
    long config,
    long const_dat
)
{

    if (fft_init == 0) {
        fft_init = 1;
        FFT_CRITICAL_INIT();
    }

    FFT_ENTER_CRITICAL();

    g_vector_core_set.vector_con = config;
    g_vector_core_set.vector_config0 = (q << 24) | (nloop << 12) | (nlen);
    g_vector_core_set.vector_config1 = (z_inc << 20) | (y_inc << 10) | x_inc;
    //printf("nlen0:%d,nloop:%d,q:%d,config:%d,%d\n",nlen,nloop,q,config,const_dat);

    g_vector_core_set.vector_xadr = (unsigned long)xptr;
    g_vector_core_set.vector_yadr = (unsigned long)yptr;
    g_vector_core_set.vector_zadr = (unsigned long)zptr;

    JL_FFT->CONST = const_dat;

    JL_FFT->CADR = (unsigned long)(&g_vector_core_set);


    // nu      clr    vector     ie      en
    JL_FFT->CON = (1 << 8) | (1 << 6) | (1 << 3) | (1 << 2) | (1 << 0);


    lp_waiting((int *)&JL_FFT->CON, BIT(7), BIT(6), 116);

    FFT_EXIT_CRITICAL();
}

u8 fft_wrap_for_ldac_enable(void)
{
    return 1;
}

int _FFT_wrap_(int cfg, int *in, int *out)
{
    //init pi32v2 fft hardware
    pi32v2_hw_fft_ctx ctx;
    if (in == out) {
        cfg |= 1;
    }

    ctx.fft_config = cfg;
    ctx.in = in;
    ctx.out = out;
    hw_fft_wrap(&ctx);

    return 0;
}



