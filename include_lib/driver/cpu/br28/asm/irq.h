#ifndef CPU_IRQ_H
#define CPU_IRQ_H


#include "asm/hwi.h"

#ifdef CONFIG_HWI_DEBUG
#define ___interrupt
#else
#define ___interrupt 	__attribute__((interrupt("")))
#endif




#endif




