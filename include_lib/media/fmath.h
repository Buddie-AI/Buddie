#ifndef __FMATH_H__
#define __FMATH_H__

#include<stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef INLINE
#ifdef _MSC_VER
#define INLINE __inline
#else
#define INLINE inline
#endif /* _MSC_VER */
#endif /* INLINE */

// typedef int32_t inumber;
typedef int16_t inumber;

//#define __Q8P24__
#define  Q_PCM_16_MAX
#define  Q_PCM_16_MIN

// Qbits
#ifdef __Q8P24__
#define	FIXED_FRACBITS              24
#define Q_PCM_MAX	16609444
#define Q_PCM_MIN	-16609444
#endif
#ifdef __Q17P15__
#define	FIXED_FRACBITS              15
#define Q_PCM_MAX	32767
#define Q_PCM_MIN	-32768
#endif
#ifdef __Q8P23__
#define	FIXED_FRACBITS              23
#define Q_PCM_MAX	8304721
#define Q_PCM_MIN	-8304721
#endif

#define Q_PCM_16_MAX	32767
#define Q_PCM_16_MIN	-32768

#define FIXED_RESOLUTION            (1 << FIXED_FRACBITS)
#define FIXED_INT_MASK              (0xffffffffL << FIXED_FRACBITS)
#define FIXED_FRAC_MASK             (~FIXED_INT_MASK)

// square roots
#define FIXED_SQRT_ERR              (1 << (FIXED_FRACBITS - 10))

// fixedp2a
#define FIXED_DECIMALDIGITS

#define FIXED_FRACBITS 16

// transform Iinteger or floating point into fixed point(Q-format)
#ifdef __LINUX__
#define PCM_N_INT_TO_Q( N, bits, Qbits )	 	(inumber)( (N)/(float)(1<<((bits)-1)) * (uint32_t)(1<<((Qbits))) )
#define PCM_N_Q_TO_INT( Q, bits, Qbits )		( (float)(Q)/(uint32_t)(1<<((Qbits))) * (uint32_t)(1<<((bits)-1)) )
#define PCM_N_FLOAT_TO_Q(N,Qbits)				(inumber)( (N) * (uint32_t)(1<<(Qbits)) )
#define PCM_N_Q_TO_FLOAT(Q,Qbits)				( Q /(float)(1<<((Qbits))) )
#endif

#define PCM_Q_RANGE(Q,Qbits)                ( {\
    long v = Q;\
    long max = (uint32_t)(1 << ((Qbits)-1)) - 1;\
    if ( Q > max ) v = max;\
    else if ( Q < -max ) v = min;\
    v;\
})

#ifdef __LINUX__
#define Q_TO_FLOAT(Q)				( Q /(float)FIXED_RESOLUTION )
#endif

#define Q_TO_INT(Q)                 ( (Q) >>FIXED_FRACBITS )
#define FLOAT_TO_Q(N)           (inumber)((N) *  FIXED_RESOLUTION)
#define INT_TO_Q(N)             (inumber)((N) << FIXED_FRACBITS)
#define Q_ADD(A,B)              ((A) + (B))
#define Q_SUB(A,B)              ((A) - (B))
#ifdef __Q17P15__
#define Q_MUL(A,B)              (int32_t)(((int32_t)(A) * (int32_t)(B)) >> FIXED_FRACBITS )
#define Q_DIV(A,B)              (int32_t)(((int32_t)(A)<<FIXED_FRACBITS) / (B))
#define Q_INV(A)                ((inumber)((((int32_t)1) << (2*FIXED_FRACBITS)) / (A)))
#else
#define Q_MUL(A,B)              (int32_t)(((int64_t)(A) * (int64_t)(B)) >> FIXED_FRACBITS )
#define Q_DIV(A,B)              (int32_t)(((int64_t)(A)<<FIXED_FRACBITS) / (B))
#define Q_INV(A)                ((inumber)((((int64_t)1) << (2*FIXED_FRACBITS)) / (A)))
#endif
#define Q_MOD(A,B)              ( (A) % (B) )
#define Q_PART_INT(A)           Q_TO_INT(A)
#define Q_PART_FLOAT(A)         ((A) &  FIXED_FRAC_MASK)


#ifdef __Q8P24__
#define Q_PI        52707178
#define Q_2PI       105414357
#define Q_PIO2      26353589
#define Q_PIO4      13176794
#define Q_LNE       45605201
#define Q_LN10      38630967
#define Q_10LN10    7286252
#else
#define Q_PI    	26353589
#define Q_2PI    	52707178
#define Q_PIO2    	13176794
#define Q_PIO4    	6588397
#define Q_LNE   	22802600
#define Q_LN10   	19315483
#define Q_10LN10   	3643126
#endif


#define qabs(A)                 ( ((A) < 0) ? (-A) : (A) )
#define qfloor(A)               ((A) & (0xffffffff<<FIXED_FRACBITS))
static inline inumber qceil(inumber x)
{
    inumber f = qfloor(x);
    if (f != x) {
        return f + INT_TO_Q(1);
    }
    return x;
}


extern inumber qsin(inumber radian);
extern inumber qcos(inumber radian);
extern inumber qtan(inumber radian);
extern inumber qlog(inumber value);
extern inumber qlog10(inumber value);
extern inumber qsqrt(inumber value);
extern inumber qexp(inumber value);
extern inumber qpow(inumber x, inumber y);


#ifdef __cplusplus
}	// extern C
#endif
#endif



