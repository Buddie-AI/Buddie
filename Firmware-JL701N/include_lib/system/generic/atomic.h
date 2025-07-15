#ifndef ATOMIC_H
#define ATOMIC_H

#include "cpu.h"
#include "irq.h"

typedef struct {
    int counter;
} atomic_t;

static inline int atomic_add_return(int i, atomic_t *v)
{
    int val;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();

    val = v->counter;
    v->counter = val += i;

    CPU_CRITICAL_EXIT();

    return val;
}


static inline int atomic_sub_return(int i, atomic_t *v)
{
    int val;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();

    val = v->counter;
    v->counter = val -= i;

    CPU_CRITICAL_EXIT();

    return val;
}

static inline int atomic_set(atomic_t *v, int i)
{
    int val = 0;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();

    v->counter = i;

    CPU_CRITICAL_EXIT();

    return val;
}


#define DEFINE_ATOMIC(x) \
    atomic_t x = {.counter = 0}

#define atomic_add(i, v)	atomic_add_return(i, v)
#define atomic_sub(i, v)	atomic_sub_return(i, v)

#define atomic_read(v)		arch_atomic_read(v)
/*#define atomic_set(v,i)		(((v)->counter) = (i))*/

#define atomic_inc(v)		atomic_add(1, v)
#define atomic_dec(v)		atomic_sub(1, v)

#define atomic_inc_and_test(v)	(atomic_add_return(1, v) == 0)
#define atomic_dec_and_test(v)	(atomic_sub_return(1, v) == 0)
#define atomic_inc_return(v)    (atomic_add_return(1, v))
#define atomic_dec_return(v)    (atomic_sub_return(1, v))
#define atomic_sub_and_test(i, v) (atomic_sub_return(i, v) == 0)

#define atomic_add_negative(i,v) (atomic_add_return(i, v) < 0)




#endif

