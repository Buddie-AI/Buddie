/*
 ******************************************************************
 *			Limiter_NoiseGate Module(限幅器/噪声门限模块)
 *1. Limiter:限制最大幅度
 *2. NoiseGate:
 *(1)启动时间
 *(2)释放时间
 *(3)噪声阈值
 *(4)噪声增益
 *3. Usage:
 *(1)LIMITER_THR:限幅器参数，限制信号的最大值
 *(2)LIMITER_NOISE_GATE:噪声门限的阈值，配合LIMITER_NOISE_GAIN使用
 *(3)LIMITER_NOISE_GAIN:当信号小于噪声门限LIMITER_NOISE_GATE的时候
 *信号增益为LIMITER_NOISE_GAIN，范围是0到1，增益0为静音，增益1为直通
 *效果。噪声门限用来优化声音的底噪
 ******************************************************************
 */

#include "system/includes.h"
#include "os/os_api.h"
#include "limiter_noiseGate_api.h"
#include "audio_noise_gate.h"


//#define NG_LOG_ENABLE
#ifdef NG_LOG_ENABLE
#define NG_LOG 		y_printf
#else
#define NG_LOG(...)
#endif/*NG_LOG_ENABLE*/

/*限幅器上限*/
#define LIMITER_THR	 -10000 /*-12000 = -12dB,放大1000倍,(-10000参考)*/
/*小于CONST_NOISE_GATE的当成噪声处理,防止清0近端声音*/
#define LIMITER_NOISE_GATE  -40000 /*-12000 = -12dB,放大1000倍,(-40000参考)*/
/*低于噪声门限阈值的增益 */
#define LIMITER_NOISE_GAIN  (0 << 30) /*(0~1)*2^30*/

enum {
    NG_STA_CLOSE = 0,
    NG_STA_OPEN,
    NG_STA_RUN,
};

typedef struct {
    u8 state;
    OS_MUTEX mutex;
    u8 *run_buf;
} audio_noise_gate_t;
audio_noise_gate_t NoiseGate;

int audio_noise_gate_open(u16 sample_rate, int limiter_thr, int noise_gate, int noise_gain)
{
    NG_LOG("audio_noise_gate_open");
    NoiseGate.run_buf = malloc(need_limiter_noiseGate_buf(1));
    NG_LOG("Limiter_noisegate_buf size:%d\n", need_limiter_noiseGate_buf(1));
    if (NoiseGate.run_buf) {
        //限幅器启动因子 int32(exp(-0.65/(16000 * 0.005))*2^30)   16000为采样率  0.005 为启动时间(s)
        int limiter_attfactor = 1065053018;
        //限幅器释放因子 int32(exp(-0.15/(16000 * 0.1))*2^30)     16000为采样率  0.1   为释放时间(s)
        int limiter_relfactor = 1073641165;
        //限幅器阈值(mdb)
        //int limiter_threshold = CONST_LIMITER_THR;

        //噪声门限启动因子 int32(exp(-1/(16000 * 0.1))*2^30)       16000为采样率  0.1   为启动时间(s)
        /*
         *import math
         *int(math.exp(-1.0/(16000*0.1))*2**30)
         */
        int noiseGate_attfactor = 1073070945;
        //噪声门限释放因子 int32(exp(-1/(16000 * 0.005))*2^30)     16000为采样率  0.005 为释放时间(s)
        int noiseGate_relfactor = 1060403589;
        //噪声门限(mdb)
        //int noiseGate_threshold = -25000;
        //低于噪声门限阈值的增益 (0~1)*2^30
        //int noise
        //Gate_low_thr_gain = 0 << 30;

        if (sample_rate == 8000) {
            limiter_attfactor = 1056434522;
            limiter_relfactor =  1073540516;
            noiseGate_attfactor = 1072400485;
            noiseGate_relfactor =  1047231044;
        }

        limiter_noiseGate_init(NoiseGate.run_buf,
                               limiter_attfactor,
                               limiter_relfactor,
                               noiseGate_attfactor,
                               noiseGate_relfactor,
                               limiter_thr,
                               noise_gate,
                               noise_gain,
                               sample_rate, 1);
        os_mutex_create(&NoiseGate.mutex);
        NoiseGate.state = NG_STA_OPEN;
        NG_LOG("audio_noise_gate_open succ");
        return 0;
    }
    return -ENOMEM;
}

void audio_noise_gate_run(void *in, void *out, u16 len)
{
    if (NoiseGate.state == NG_STA_CLOSE) {
        return;
    }
    os_mutex_pend(&NoiseGate.mutex, 0);
    if (NoiseGate.run_buf) {
        limiter_noiseGate_run(NoiseGate.run_buf, in, out, len / 2);
    }
    os_mutex_post(&NoiseGate.mutex);
}

void audio_noise_gate_close()
{
    NG_LOG("audio_noise_gate_close");
    os_mutex_pend(&NoiseGate.mutex, 0);
    NoiseGate.state = NG_STA_CLOSE;
    if (NoiseGate.run_buf) {
        free(NoiseGate.run_buf);
    }
    os_mutex_post(&NoiseGate.mutex);
    NG_LOG("audio_noise_gate_close succ");
}

