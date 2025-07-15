#include "includes.h"
#include "asm/lp_touch_key_alog.h"


#define ABS(a)                      ((a)>0?(a):-(a))
#define MIN(a,b)                    ((a)>(b)?(b):(a))
#define MAX(a,b)                    ((a)<(b)?(b):(a))
#define ELEM_SWAP_USHORT(a,b)       { unsigned short t=(a);(a)=(b);(b)=t; }

#define INIT_SIGMA                  (5)
#define MIN_SIGMA                   (5)
#define OUTLIER_ZSCORE              (6)
#define PRESSED_ZSCORE              (12)
#define PULSE_WIDTH                 (5)
#define MIN_VALLEY_SAMPLE           (3)
#define FIFO_BUF_SIZE               (8)
#define HISTORY_BUF_SIZE            (12)
#define UNBALANCED_TH               (2) //0.2
#define RANGE_TH_ALPHA              (5)
#define RANGE_TH_DOWN_ALPHA         (2)
#define RANGE_TH_DOWNWARD_EN        (0)
#define RANGE_STABLE_CHECK_EN       (1)
#define RANGE_STABLE_VALUE_TH       (3)     //0.3
#define RANGE_STABLE_COUNT_TH       (10)
#define RANGE_STABLE_ALPHA          (12)    //0.04296875

typedef struct {
    u32 count;

    //for median filter
    s16 med_dat0;
    s16 med_dat1;
    u8 med_sign;

    //for gradient
    s16 fifo_buf[FIFO_BUF_SIZE];
    u8  fifo_pos;
    u8  fifo_size;
    s32 grad_max;

    u8  key_state;

    //for sigma tracing
    s32 std_sum;
    s32 std_count;
    s32 sigma_iir_state;
    s16 sigma_iir_alpha;
    s16 sigma_iir_alpha_fast;

    //stable count
    s32 stable_count;
    u8  is_stable;

    //for valley tracing
    s16 valley_grad;
    s16 delayed_valley_grad;
    s16 peak_grad;
    s16 valley_val;
    s16 peak_max;
    s16 valley_duration;
    //down continued
    u8  down_cont;
    //up continued
    u8  up_cont;
    u8  delayed_keyup;

#if RANGE_TH_DOWNWARD_EN
    s32 high_undetect_count;
    s32 low_detect_count;

    u8  th_downward;
#endif

    //for delta filter
    u16 prev_val;

    u16 range;
    u16 range_median;
    u16 range_for_th;
    u16 range_list[HISTORY_BUF_SIZE];
    u16 range_list_median[HISTORY_BUF_SIZE];
    u8  range_size;
    u8  range_pos;
    u8  range_valid;

    u16 range_min;
    u16 range_max;

    u8  maybe_noise;

#if RANGE_STABLE_CHECK_EN
    s32 range_stable_iir_state;
    s32 range_stable_count;
    u8 range_isstable;
#endif
} TouchAlgo_t;

static TouchAlgo_t ta_ch[5];

static inline unsigned short kth_smallest_ushort(unsigned short a[], int n, int k)
{
    int i, j, l, m ;
    unsigned short x ;
    l = 0 ;
    m = n - 1 ;
    while (l < m) {
        x = a[k] ;
        i = l ;
        j = m ;
        do {
            while (a[i] < x) {
                i++ ;
            }
            while (x < a[j]) {
                j-- ;
            }
            if (i <= j) {
                ELEM_SWAP_USHORT(a[i], a[j]) ;
                i++ ;
                j-- ;
            }
        } while (i <= j) ;
        if (j < k) {
            l = i ;
        }
        if (k < i) {
            m = j ;
        }
    }
    return a[k] ;
}

static u16 medfilt(TouchAlgo_t *ta, u16 x)
{
    u16 ret = 0;
    if (ta->med_sign) {
        //dat0 <= dat1;
        if (x <= ta->med_dat0) {
            ret = ta->med_dat0;
            ta->med_sign = 0;
        } else if (x >= ta->med_dat1) {
            ret = ta->med_dat1;
            ta->med_sign = 1;
        } else {
            ret = x;
            ta->med_sign = 0;
        }
    } else {
        //da0 > dat1;
        if (x <= ta->med_dat1) {
            ret = ta->med_dat1;
            ta->med_sign = 0;
        } else if (x >= ta->med_dat0) {
            ret = ta->med_dat0;
            ta->med_sign = 1;
        } else {
            ret = x;
            ta->med_sign = 1;
        }
    }
    ta->med_dat0 = ta->med_dat1;
    ta->med_dat1 = x;
    return ret;
}

static s32 iir_filter(s32 *state, u16 x, s16 alpha)
{
    *state = (*state * (256 - alpha) + ((s32)x << 6) * alpha + 128) >> 8;
    return *state;
}

static s32 newton_sqrt(s32 in)
{
    int x = 1;
    int y = (x + in / x) >> 1;

    while (ABS(y - x) > 1) {
        x = y;
        y = (x + in / x) >> 1;
    }
    return y;
}

static void TouchAlgo_tracing(TouchAlgo_t *ta, u16 x)
{

    s32 sigma;
    s32 delta, delta_abs;
    s32 outlier_th;
    s32 pressed_th, sigma_pressed_th;
    s32 grad, grad_abs, grad_abs_max;
    s32  grad_sign_max;

    sigma = (ta->sigma_iir_state + 32) >> 6;

    sigma_pressed_th = sigma * PRESSED_ZSCORE;
    outlier_th = sigma * OUTLIER_ZSCORE;

    if (ta->range_valid || ta->range_size > 0) {
        pressed_th = ta->range_for_th / 2;

        if (pressed_th < sigma_pressed_th) {
            pressed_th = sigma_pressed_th;
        }
    } else {
        pressed_th = sigma_pressed_th;
    }

    //delta filter
    delta = (s32)x - (s32)ta->prev_val;
    ta->prev_val = x;

    delta_abs = delta;
    if (delta_abs < 0) {
        delta_abs = -delta_abs;
    }

    if (delta_abs < outlier_th) {
        ta->std_sum += delta * delta;
        ta->std_count += 1;

        if (ta->std_count == 128) {
            s32 var = newton_sqrt((ta->std_sum + ta->std_count / 2) / ta->std_count);
            ta->std_count = 0;
            ta->std_sum = 0;

            if (var < MIN_SIGMA) {
                var = MIN_SIGMA;
            }

            if (ta->count < 128 * 8) {
                iir_filter(&ta->sigma_iir_state, var, ta->sigma_iir_alpha_fast);
            } else {
                iir_filter(&ta->sigma_iir_state, var, ta->sigma_iir_alpha);
            }
        }

        ta->stable_count += 1;
        if (ta->stable_count > 10) {
            ta->is_stable = 1;
        }
    } else {
        ta->stable_count = 0;
        ta->is_stable = 0;
    }
    // printf("%d\n", sigma);

    //gradient
    grad_abs_max = delta_abs;
    grad_sign_max = delta >> 31;
    for (int i = 1; i < ta->fifo_size; i += 2) {
        int idx = (FIFO_BUF_SIZE + ta->fifo_pos - 1 - i) % FIFO_BUF_SIZE;

        grad = x - ta->fifo_buf[idx];
        grad_abs = ABS(grad);

        if (grad_abs > grad_abs_max) {
            grad_abs_max = grad_abs;
            grad_sign_max = grad >> 31;
        }
    }

    if (grad_sign_max == -1) {
        ta->grad_max = -grad_abs_max;
    } else {
        ta->grad_max = grad_abs_max;
    }

    //append to fifo
    ta->fifo_buf[ta->fifo_pos] = x;
    ta->fifo_pos = (ta->fifo_pos + 1) % FIFO_BUF_SIZE;
    ta->fifo_size += 1;
    if (ta->fifo_size > FIFO_BUF_SIZE) {
        ta->fifo_size = FIFO_BUF_SIZE;
    }

#if RANGE_TH_DOWNWARD_EN
    if (grad_abs_max <= pressed_th) {
        ta->high_undetect_count += 1;
        if (grad_abs_max > sigma_pressed_th) {
            ta->low_detect_count += 1;
        }

        if (ta->high_undetect_count >= 60000 && ta->low_detect_count > 0) { //10min
            ta->th_downward = 1;
        }

        if (ta->th_downward && ta->high_undetect_count % 100 == 0) {
            ta->range_for_th = (ta->range_for_th * (256 - RANGE_TH_DOWN_ALPHA) + sigma_pressed_th * 2 * RANGE_TH_DOWN_ALPHA + 128) >> 8;
        }
    } else {
        ta->high_undetect_count = 0;
        ta->low_detect_count = 0;

        ta->th_downward = 0;
    }
#endif

    if (ta->maybe_noise) {
        if (ta->is_stable) {
            ta->maybe_noise = 0;
        } else {
            return;
        }
    }

    //key state machine
    if (ta->key_state == 0) {
        if (grad_abs_max > pressed_th && grad_sign_max == 0) {
            //key up
            //nothing to do
            if (ta->up_cont == 0) {
                ta->maybe_noise = 1;
            }

            if (ta->up_cont) {
                if (ta->peak_max < x) {
                    ta->peak_max = x;
                    ta->peak_grad = ta->peak_max - ta->valley_val;
                }
            }

            ta->down_cont = 0;
            ta->up_cont = 1;
        } else if (grad_abs_max > pressed_th && grad_sign_max == -1) {
            //key down
            ta->key_state = 1;
            ta->valley_duration = 1;
            ta->valley_val = x;
            ta->valley_grad = grad_abs_max;

            ta->down_cont = 1;
            ta->up_cont = 0;
        } else {
            //nothing to do
            ta->down_cont = 0;
            ta->up_cont = 0;
        }
    } else {
        if (grad_abs_max > pressed_th && grad_sign_max == 0) {
            //key up
            ta->key_state = 0;

            ta->peak_max = x;
            ta->peak_grad = ta->peak_max - ta->valley_val;

            if (ta->valley_duration >= PULSE_WIDTH) {
                ta->delayed_keyup = 1;
                ta->delayed_valley_grad = ta->valley_grad;
            }

            ta->down_cont = 0;

            ta->up_cont = 1;

        } else if (grad_abs_max > pressed_th && grad_sign_max == -1) {
            //key down

            if (ta->down_cont) {
                ta->valley_duration += 1;
                if (x < ta->valley_val) {
                    ta->valley_grad += ta->valley_val - x;
                    ta->valley_val = x;
                }
            } else {
                //keydown  when it's already in keydown state
                //Discarding all history information, cause it's not reliable
                // ta->range_size = 0;
                // ta->range_pos = 0;
                // ta->range_valid = 0;
                // printf("keydown  when it's already in keydown state\n");

                //reset keydown state
                ta->key_state = 1;
                ta->valley_duration = 1;
                ta->valley_val = x;
                ta->valley_grad = grad_abs_max;
            }

            ta->down_cont = 1;
            ta->up_cont = 0;

        } else {
            ta->valley_duration += 1;
            if (x < ta->valley_val) {
                ta->valley_grad += ta->valley_val - x;
                ta->valley_val = x;
            }
            ta->down_cont = 0;
            ta->up_cont = 0;

        }

        if (ta->delayed_keyup && ta->up_cont == 0) {
            s16 peak_grad = ta->peak_grad;

            s16 min_grad = MIN(peak_grad, ta->delayed_valley_grad);
            s16 max_grad = MAX(peak_grad, ta->delayed_valley_grad);
            s32 stablized_range;
            s32 range_th;

            ta->delayed_keyup = 0;

#if RANGE_STABLE_CHECK_EN
            stablized_range = (ta->range_stable_iir_state + 32) >> 6;
            range_th = stablized_range * RANGE_STABLE_VALUE_TH / 10;

            if ((max_grad - min_grad) * 10 < max_grad * UNBALANCED_TH
                && min_grad > ta->range_min
                && max_grad < ta->range_max
                && ((ta->range_isstable && ABS(stablized_range - max_grad) < range_th) || ta->range_isstable == 0))
#else
            if ((max_grad - min_grad) * 10 < max_grad * UNBALANCED_TH
                && min_grad > ta->range_min
                && max_grad < ta->range_max)
#endif
            {
                ta->range_list[ta->range_pos] = max_grad;
                ta->range_pos = (ta->range_pos + 1) % HISTORY_BUF_SIZE;
                if (ta->range_size < HISTORY_BUF_SIZE) {
                    ta->range_size += 1;
                }

                ta->range = ta->range_list[0];
                for (int i = 1; i < ta->range_size; i++) {
                    if (ta->range < ta->range_list[i]) {
                        ta->range = ta->range_list[i];
                    }
                }


                memcpy(&ta->range_list_median, &ta->range_list, ta->range_size * sizeof(u16));
                ta->range_median = kth_smallest_ushort(ta->range_list_median, ta->range_size, ta->range_size / 2);

                // if(ta->range_size > HISTORY_BUF_SIZE/2)
                // {
                ta->range =  ta->range_median;
                // }

                if (ta->range_valid == 0) {
                    ta->range_for_th =  sigma_pressed_th * 2;
#if RANGE_STABLE_CHECK_EN
                    ta->range_stable_iir_state = ta->range << 6;
#endif
                } else {

                    ta->range_for_th = (ta->range_for_th * (256 - RANGE_TH_ALPHA) + ta->range_median * RANGE_TH_ALPHA + 128) >> 8;

#if RANGE_STABLE_CHECK_EN

                    iir_filter(&ta->range_stable_iir_state, ta->range, RANGE_STABLE_ALPHA);

                    if (ta->range > stablized_range - range_th
                        && ta->range < stablized_range  + range_th
                       ) {
                        ta->range_stable_count += 1;
                    } else {
                        ta->range_stable_count = 0;
                    }

                    if (ta->range_stable_count > RANGE_STABLE_COUNT_TH) {
                        ta->range_isstable = 1;
                    }
#endif
                }

                if (ta->range_size >= MIN_VALLEY_SAMPLE) {
                    ta->range_valid = 1;
                }
                if (ta->range_valid) {
                    // printf("range:%d, median:%d\n", ta->range, ta->range_median);
                }
            } else {
                printf("invalid touch value\n");
            }
        }
    }

    return;
}


void TouchAlgo_Init(u8 ch, u16 min, u16 max)
{
    TouchAlgo_t *ta = (TouchAlgo_t *)&ta_ch[ch];

    ta->count = 0;

    ta->fifo_pos = 0;
    ta->fifo_size = 0;

    ta->key_state = 0;

    ta->med_dat0 = ta->med_dat1 = 0;
    ta->med_sign = 1;

    ta->std_sum = 0;
    ta->std_count = 0;
    ta->sigma_iir_alpha = 5;
    ta->sigma_iir_alpha_fast = 32;
    ta->sigma_iir_state = INIT_SIGMA << 6;

    ta->stable_count = 0;
    ta->is_stable = 0;

    ta->valley_val = 0;
    ta->valley_duration = 0;
    ta->prev_val = 0;

    ta->delayed_keyup = 0;

#if RANGE_TH_DOWNWARD_EN
    ta->high_undetect_count = 0;
    ta->low_detect_count = 0;
    ta->th_downward = 0;
#endif
    ta->range_valid = 0;
    ta->range_median = 0;
    ta->range = 0;
    ta->range_size = 0;
    ta->range_pos = 0;

    ta->range_min = min;
    ta->range_max = max;

    ta->maybe_noise = 0;

#if RANGE_STABLE_CHECK_EN
    ta->range_stable_iir_state = 0;
    ta->range_stable_count = 0;
    ta->range_isstable = 0;
#endif
}

void TouchAlgo_Update(u8 ch, u16 x)
{
    TouchAlgo_t *ta = (TouchAlgo_t *)&ta_ch[ch];
    u16 dat0;

    dat0 = medfilt(ta, x);
    if (ta->count < 3) {
        ta->count++;

        ta->prev_val = dat0;
        return;
    }

    TouchAlgo_tracing(ta, dat0);

    ta->count++;
    return;
}

void TouchAlgo_Reset(u8 ch, u16 min, u16 max)
{
    TouchAlgo_Init(ch, min, max);
}

s32 TouchAlgo_GetSigma(u8 ch)
{
    TouchAlgo_t *ta = (TouchAlgo_t *)&ta_ch[ch];
    return ta->sigma_iir_state;
}

u16 TouchAlgo_GetRange(u8 ch, u8 *valid)
{
    TouchAlgo_t *ta = (TouchAlgo_t *)&ta_ch[ch];
    *valid = ta->range_valid;
    return ta->range_valid ? ta->range : 0;
}

void TouchAlgo_SetRange(u8 ch, u16 range)
{
    TouchAlgo_t *ta = (TouchAlgo_t *)&ta_ch[ch];
    ta->range = range;
    ta->range_valid = 1;

    ta->range_list[0] = range;
    ta->range_size = 1;
    ta->range_pos = 1;
}

void TouchAlgo_SetSigma(u8 ch, s32 sigma)
{
    TouchAlgo_t *ta = (TouchAlgo_t *)&ta_ch[ch];
    ta->sigma_iir_state = sigma;
}


