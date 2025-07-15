
#ifndef ASM_CPU_H
#define ASM_CPU_H


#include "br28.h"
#include "csfr.h"
#include "cache.h"
#ifndef __ASSEMBLY__

typedef unsigned char   		u8, bool, BOOL;
typedef char            		s8;
typedef unsigned short  		u16;
typedef signed short    		s16;
typedef unsigned int    		u32;
typedef signed int      		s32;
typedef unsigned long long 		u64;
typedef u32						FOURCC;
typedef long long               s64;
typedef unsigned long long      u64;


#endif


#define ___trig        __asm__ volatile ("trigger")


#ifndef BIG_ENDIAN
#define BIG_ENDIAN 			0x3021
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 		0x4576
#endif
#define CPU_ENDIAN 			LITTLE_ENDIAN

#ifdef BT_DUT_INTERFERE
#define CPU_CORE_NUM     1      //dut interfere
#else
#define CPU_CORE_NUM     2      //sdk
#endif

#define OS_CORE_AFFINITY_ENABLE     1

extern const int CONFIG_CPU_UNMASK_IRQ_ENABLE;

///屏蔽的优先级, < N的优先级不可以响应
#define CPU_IRQ_IPMASK_LEVEL   				6

#define  CPU_TASK_CLR(a)
#define  CPU_TASK_SW(a) 		\
    do { \
        q32DSP(a)->ILAT_SET |= BIT(3-a); \
    } while (0)


#define  CPU_INT_NESTING 	2


#ifndef __ASSEMBLY__

#if CPU_CORE_NUM > 1
__attribute__((always_inline))
static int current_cpu_id()
{
    unsigned id;
    asm volatile("%0 = cnum" : "=r"(id) ::);
    return id ;
}

__attribute__((always_inline))
static int core_num(void)
{
    u32 num;
    asm volatile("%0 = cnum" : "=r"(num) :);
    return num;
}

#else
static inline int current_cpu_id()
{
    return 0;
}

static inline int core_num(void)
{
    return 0;
}
#endif

static inline int cpu_in_irq()
{
    int flag;
    __asm__ volatile("%0 = icfg" : "=r"(flag));
    return flag & 0xff;
}

extern int __cpu_irq_disabled(void);
static inline int cpu_irq_disabled()
{
    if (CONFIG_CPU_UNMASK_IRQ_ENABLE) {
        return __cpu_irq_disabled();
    } else {
        int flag;
        __asm__ volatile("%0 = icfg" : "=r"(flag));
        return ((flag & 0x300) != 0x300);
    }
}

#if 0
static inline int data_sat_s16(int ind)
{
    if (ind > 32767) {
        ind = 32767;
    } else if (ind < -32768) {
        ind = -32768;
    }
    return ind;
}

#else
static inline int data_sat_s16(int ind)
{
    __asm__ volatile(
        " %0 = sat16(%0)(s)  \t\n"
        : "=&r"(ind)
        : "0"(ind)
        :);
    return ind;
}
#endif


static inline u32 reverse_u32(u32 data32)
{
#if 0
    u8 *dataptr = (u8 *)(&data32);
    data32 = (((u32)dataptr[0] << 24) | ((u32)dataptr[1] << 16) | ((u32)dataptr[2] << 8) | (u32)dataptr[3]);
#else
    __asm__ volatile("%0 = rev8(%0) \t\n" : "=&r"(data32) : "0"(data32) :);
#endif
    return data32;
}

static inline u32 reverse_u16(u16 data16)
{
    u32 retv;
#if 0
    u8 *dataptr = (u8 *)(&data16);
    retv = (((u32)dataptr[0] << 8) | ((u32)dataptr[1]));
#else
    retv = ((u32)data16) << 16;
    __asm__ volatile("%0 = rev8(%0) \t\n" : "=&r"(retv) : "0"(retv) :);
#endif
    return retv;
}

static inline u32 rand32()
{
    return JL_RAND->R64L;
}

#define __asm_sine(s64, precision) \
    ({ \
        u64 ret; \
        u8 sel = 0; \
        __asm__ volatile ("%0 = copex(%1) (%2)" : "=r"(ret) : "r"(s64), "i"(sel)); \
        ret = ret>>32; \
        ret;\
    })

void p33_soft_reset(void);
static inline void cpu_reset(void)
{
    // JL_CLOCK->PWR_CON |= (1 << 4);
    p33_soft_reset();
}

#define __asm_csync() \
    do { \
		asm volatile("csync;"); \
    } while (0)

#include "asm/irq.h"
#include "generic/printf.h"
#include "system/generic/log.h"


#define arch_atomic_read(v)  \
	({ \
        __asm_csync(); \
		(*(volatile int *)&(v)->counter); \
	 })
#if 0
extern volatile int cpu_lock_cnt[];
extern volatile int irq_lock_cnt[];


static inline void local_irq_disable()
{
    __builtin_pi32v2_cli();
    irq_lock_cnt[current_cpu_id()]++;
}


static inline void local_irq_enable()
{
    if (--irq_lock_cnt[current_cpu_id()] == 0) {
        __builtin_pi32v2_sti();
    }
}
#else

extern   void __local_irq_disable() ;

extern   void __local_irq_enable() ;

extern void local_irq_disable();
extern void local_irq_enable();

#endif

#if(0 )

#define arch_spin_trylock(lock) \
	do { \
        __asm_csync(); \
		while ((lock)->rwlock); \
		(lock)->rwlock = 1; \
	}while(0)

#define arch_spin_lock(lock) \
	do { \
        int ret = false; \
        __asm_csync(); \
		if (!(lock)->rwlock) { \
            ret = true; \
		    (lock)->rwlock = 1; \
        } \
        if (ret) \
            break; \
	}while(1)

#define arch_spin_unlock(lock) \
	do { \
		(lock)->rwlock = 0; \
	}while(0)

#else



static inline void q32DSP_testset(u8 volatile *ptr)
{
    asm volatile(
        " 1:            \n\t "
        " testset b[%0] \n\t "
        " ifeq goto 1b  \n\t "
        :
        : "p"(ptr)
        : "memory"
    );
}

static inline void q32DSP_testclr(u8 volatile *ptr)
{
    asm volatile(
        " b[%0] = %1    \n\t "
        :
        : "p"(ptr), "r"(0)
        : "memory"
    );
}

#define arch_spin_trylock(lock) \
	do { \
	q32DSP_testset((u8 *)(&lock->rwlock));\
	}while(0)


#define arch_spin_lock(lock)  arch_spin_trylock(lock)

#define arch_spin_unlock(lock) \
	do{ \
	q32DSP_testclr((u8 *)(&lock->rwlock)) ;\
	}while(0)


#endif

#if 1 // CPU_CORE_NUM >1

extern volatile int cpu_lock_cnt[];
extern volatile int irq_lock_cnt[];
#if 0
#define	CPU_SR_ALLOC() 	\
//	int flags




#define CPU_CRITICAL_ENTER() \
     do { extern u8 volatile cpulock;\
         local_irq_disable(); \
         if(cpu_lock_cnt[current_cpu_id()]++ == 0) \
       q32DSP_testset(&cpulock);\
         __asm_csync(); \
     }while(0)

//	   	 asm volatile("lockset;");

#define CPU_CRITICAL_EXIT() \
     do {extern u8 volatile cpulock; \
         if (--cpu_lock_cnt[current_cpu_id()] == 0) \
 		q32DSP_testclr(&cpulock);\
 		 local_irq_enable();\
	 }while(0)

#endif


#define CPU_SR_ALLOC()  \
 //  int flags

#define CPU_CRITICAL_ENTER()  \
     do { \
         local_irq_disable(); \
     }while(0)


#define CPU_CRITICAL_EXIT() \
     do { \
         local_irq_enable(); \
     }while(0)

//             asm volatile("lockclr;");
#else


#define	CPU_SR_ALLOC() 	\
//	int flags

#define CPU_CRITICAL_ENTER()  \
	do { \
		local_irq_disable(); \
        __asm_csync(); \
	}while(0)


#define CPU_CRITICAL_EXIT() \
	do { \
		local_irq_enable(); \
	}while(0)

#endif

extern void cpu_assert_debug();
extern const int config_asser;
#define ASSERT(a,...)   \
		do { \
			if(config_asser){\
				if(!(a)){ \
					printf("cpu %d file:%s, line:%d",current_cpu_id(), __FILE__, __LINE__); \
					printf("ASSERT-FAILD: "#a" "__VA_ARGS__); \
					cpu_assert_debug(); \
				} \
			}else {\
				if(!(a)){ \
					cpu_reset(); \
				}\
			}\
		}while(0);



#endif //__ASSEMBLY__


#endif

