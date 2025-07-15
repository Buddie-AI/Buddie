#ifndef SYS_SPINLOCK_H
#define SYS_SPINLOCK_H

#include "typedef.h"
#include "cpu.h"
#include "irq.h"


struct __spinlock {
    volatile u32 rwlock;
};

typedef struct __spinlock spinlock_t;

#if CPU_CORE_NUM > 1

#define preempt_disable() \
	__local_irq_disable()

#define preempt_enable() \
	__local_irq_enable()

#else

#define preempt_disable() \
	local_irq_disable()

#define preempt_enable() \
	local_irq_enable()




#endif


#if CPU_CORE_NUM > 1

#define spin_acquire(lock) 	\
	do { \
		arch_spin_lock(lock); \
	}while(0)

#define spin_release(lock) \
	do { \
		arch_spin_unlock(lock); \
	}while(0)

#else

#define spin_acquire(lock) 	\
	do { \
	}while(0)


#define spin_release(lock) \
	do { \
	}while(0)

#endif


#define DEFINE_SPINLOCK(x) \
	spinlock_t x = { .rwlock = 0 }


static inline void spin_lock_init(spinlock_t *lock)
{
    lock->rwlock = 0;
}
extern u32 spin_lock_cnt[];

#if 1
static inline void spin_lock(spinlock_t *lock)
{
    preempt_disable();
    /*ASSERT(spin_lock_cnt[current_cpu_id()] == 0);
    spin_lock_cnt[current_cpu_id()] = 1;*/
    spin_acquire(lock);
}


static inline void spin_unlock(spinlock_t *lock)
{
    /*spin_lock_cnt[current_cpu_id()] = 0;*/
    spin_release(lock);
    preempt_enable();
}

#else


#define spin_lock(lock) \
    do { \
        preempt_disable(); \
        if (!(T2_CON & (1<<0))) { \
            T2_CNT = 0; \
            T2_PRD = 120000000 / 10; \
            T2_CON = 1; \
        } \
        spin_lock_cnt[current_cpu_id()] = T2_CNT; \
        spin_acquire(lock); \
    } while (0)


#define spin_unlock(lock) \
    do { \
        u32 t = T2_CNT;\
        if(t < spin_lock_cnt[current_cpu_id()]) \
            t += T2_PRD - spin_lock_cnt[current_cpu_id()]; \
        else \
            t -= spin_lock_cnt[current_cpu_id()]; \
        spin_release(lock); \
        preempt_enable(); \
        if (t > 100000) { /*120000 == 1ms*/ \
            printf("???????spinlock: %d, %s\n", t, __func__); \
        } \
    } while(0)

#endif

/*#define spin_lock_irqsave(lock, flags) \
	do { \
		local_irq_save(flags); \
		spin_acquire((lock)); \
	}while(0)

#define spin_unlock_irqrestore(lock, flags) \
	do { \
		spin_release((lock)); \
		local_irq_restore(flags); \
	}while(0) */











#endif

