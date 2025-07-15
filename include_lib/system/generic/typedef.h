/*************************************************************
File: typedef.h
Author:Juntham
Discriptor:
    数据类型重定义
Version:
Date：
*************************************************************/
#ifndef _typedef_h_
#define _typedef_h_


#include "asm/cpu.h"


#if defined(__GNUC__)

///<locate code to x segment ever exist
#define SEC_USED(x)     __attribute__((section(#x),used))
///<locate code to x segment optimized by dependency
#define SEC(x)          __attribute__((section(#x)))
#define sec(x)          __attribute__((section(#x),used))
///<locate data to x segment
#define AT(x)           __attribute__((section(#x)))
#define SET(x)          __attribute__((x))
#define ALIGNED(x)	    __attribute__((aligned(x)))
#define _GNU_PACKED_	__attribute__((packed))
#define _NOINLINE_	    __attribute__((noinline))
#define _INLINE_	    __attribute__((always_inline))
#define _WEAK_	        __attribute__((weak))
#define _WEAKREF_	    __attribute__((weakref))
#define _NORETURN_      __attribute__((noreturn))
#define _NAKED_         __attribute__((naked))
#else

#define SEC_USED(x)
#define SEC(x)
#define AT(x)
#define SET(x)
#define ALIGNED(x)
#define _GNU_PACKED_
#define _NOINLINE_
#define _INLINE_
#define _WEAK_
#define _WEAKREF_
#define _NORETURN_
#define _NAKED_
#endif


#if CPU_ENDIAN == LITTLE_ENDIAN
//#define ntohl(x) (u32)((x>>24)|((x>>8)&0xff00)|(x<<24)|((x&0xff00)<<8))
//#define ntoh(x) (u16)((x>>8&0x00ff)|x<<8&0xff00)

//#define ntohl(x) (u32)((((u32)(x))>>24) | ((((u32)(x))>>8)&0xff00) | (((u32)(x))<<24) | ((((u32)(x))&0xff00)<<8))
//#define ntoh(x) (u16)((((u32)(x))>>8&0x00ff) | (((u32)(x))<<8&0xff00))

//#define NTOH(x) (x) = ntoh(x)
//#define NTOHL(x) (x) = ntohl(x)
#define LD_WORD(ptr)        (u16)(*(u16*)(u8*)(ptr))
#define LD_DWORD(ptr)        (u32)(*(u32*)(u8*)(ptr))
#define ST_WORD(ptr,val)    *(u16*)(u8*)(ptr)=(u16)(val)
#define ST_DWORD(ptr,val)    *(u32*)(u8*)(ptr)=(u32)(val)
#else
#define ntohl(x) (x)
#define ntoh(x) (x)
#define NTOH(x) (x) = ntoh(x)
#define NTOHL(x) (x) = ntohl(x)
#endif



#undef FALSE
#define FALSE    	0

#undef TRUE
#define TRUE    	1

#define false    	0
#define true    	1

#ifndef NULL
#define NULL    	(void *)0
#endif



#define     BIT(n)              (1UL << (n))
#define     BitSET(REG,POS)     ((REG) |= (1L << (POS)))
#define     BitCLR(REG,POS)     ((REG) &= (~(1L<< (POS))))
#define     BitXOR(REG,POS)     ((REG) ^= (~(1L << (POS))))
#define     BitCHK_1(REG,POS)   (((REG) & (1L << (POS))) == (1L << (POS)))
#define     BitCHK_0(REG,POS)   (((REG) & (1L << (POS))) == 0x00)
#define     testBit(REG,POS)    ((REG) & (1L << (POS)))

#define     clrBit(x,y)         (x) &= ~(1L << (y))
#define     setBit(x,y)         (x) |= (1L << (y))


#define readb(addr)   *((volatile unsigned char*)(addr))
#define readw(addr)   *((volatile unsigned short *)(addr))
#define readl(addr)   *((volatile unsigned long*)(addr))

#define writeb(addr, val)  *((volatile unsigned char*)(addr)) = (u8)(val)
#define writew(addr, val)  *((volatile unsigned short *)(addr)) = (u16)(val)
#define writel(addr, val)  *((volatile unsigned long*)(addr)) = (u32)(val)

#define ALIGN_4BYTE(size)   ((size+3)&0xfffffffc)

#if CPU_ENDIAN == BIG_ENDIAN
#define __cpu_u16(lo, hi)  ((lo)|((hi)<<8))
#elif CPU_ENDIAN == LITTLE_ENDIAN
#define __cpu_u16(lo, hi)  ((hi)|((lo)<<8))
#else
#error "undefine cpu eadin"
#endif


#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


#define ARRAY_SIZE(array)  (sizeof(array)/sizeof(array[0]))


#define likely(x) 	__builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define SFR(sfr, start, len, dat) \
	(sfr = (sfr & ~((~(0xffffffff << (len))) << (start))) | \
	 (((dat) & (~(0xffffffff << (len)))) << (start)))


#include "generic/errno-base.h"
#include "string.h"
#include "strings.h"
#include "system/malloc.h"


#ifdef offsetof
#undef offsetof
#endif

#ifdef container_of
#undef container_of
#endif

#define offsetof(type, memb) \
	((unsigned long)(&((type *)0)->memb))

#define container_of(ptr, type, memb) \
	((type *)((char *)(ptr) - offsetof(type, memb)))

void delay(unsigned int);

void delay_us(unsigned int);


#endif



