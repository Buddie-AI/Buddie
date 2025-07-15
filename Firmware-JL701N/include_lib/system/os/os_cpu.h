/***********************************Jieli tech************************************************
  File : os_cpu.h
  By   : Juntham
  date : 2014-07-03 09:06
********************************************************************************************/
#ifndef _OS_CPU_H
#define _OS_CPU_H

#include "asm/cpu.h"
#include "jiffies.h"


#ifndef __ASSEMBLY__
typedef unsigned short  QS;
typedef unsigned int    OS_STK;                  /* Each stack entry is 32-bit wide*/
typedef unsigned int    OS_CPU_SR;                   /* Unsigned 32 bit quantity */
typedef unsigned int    OS_CPU_DATA;                 /* Unsigned 32 bit quantity */
#endif

#define	OS_CPU_EXT      	extern
#define	OS_CPU_CORE     	CPU_CORE_NUM

#define OS_CPU_ID       	current_cpu_id()
#define OS_STK_GROWTH   	1         /* Stack grows from HIGH to LOW memory*/

#define OS_CPU_MMU          0

#define OS_CPU_VIRTUAL_MEM  1           //临时定义：区别于OS_CPU_MMU

#ifndef OS_CORE_AFFINITY_ENABLE
#define OS_CORE_AFFINITY_ENABLE     0
#endif

#define OS_TASK_CLR(a) 		CPU_TASK_CLR(a)
#define OS_TASK_SW(a)     	CPU_TASK_SW(a) /* 任务级任务切换函数*/
#define OS_INT_NESTING      CPU_INT_NESTING

#define CPU_SR_ALLOC()

#define OS_SR_ALLOC()

#define OS_ENTER_CRITICAL()  \
		CPU_CRITICAL_ENTER(); \

#define OS_EXIT_CRITICAL()  \
		CPU_CRITICAL_EXIT()




#ifndef __ASSEMBLY__

/*#include "system/spinlock.h"

extern spinlock_t os_lock;

#define OS_ENTER_CRITICAL()  \
	spin_lock(&os_lock)

#define OS_EXIT_CRITICAL()  \
	spin_unlock(&os_lock)*/


void OSCtxSw(void);

extern void EnableOtherCpu(void) ;

#define os_ctx_sw OSCtxSw

void OSInitTick(u32 hz);

void InstallOSISR(void);

void os_task_dead(const char *task_name);

//=======================================================//
//             系统进临界区多核同步类型                  //
//=======================================================//
enum CPU_SUSPEND_TYPE {
    CPU_SUSPEND_TYPE_NONE = 0,
    CPU_SUSPEND_TYPE_SFC = 0x55, //操作Flash
    CPU_SUSPEND_TYPE_PDOWN, 	 //系统进低功耗Pdown
    CPU_SUSPEND_TYPE_POFF, //系统进低功耗Pdown
    CPU_SUSPEND_TYPE_SOFF,
};

/* ---------------------------------------------------------------------------- */
/**
 * @brief 系统进入临界区用于多核同步
 */
/* ---------------------------------------------------------------------------- */
void cpu_suspend_other_core(enum CPU_SUSPEND_TYPE type);

/* ---------------------------------------------------------------------------- */
/**
 * @brief 系统退出临界区用于多核同步
 */
/* ---------------------------------------------------------------------------- */
void cpu_resume_other_core(enum CPU_SUSPEND_TYPE type);

#endif

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/


#define OS_CRITICAL_METHOD          3
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
//#define     CPU_SR_ALLOC()  OS_CPU_SR  cpu_sr
#endif


#endif                                           /*_OS_CPU_H                                            */
