#ifndef JIFFIES_H
#define JIFFIES_H
/* timer interface */
/* Parameters used to convert the timespec values: */
#define HZ				100L
#define MSEC_PER_SEC	1000L
#define USEC_PER_MSEC	1000L
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define FSEC_PER_SEC	1000000000000000LL


#ifndef __ASSEMBLY__
extern volatile unsigned long jiffies;
extern unsigned long jiffies_msec();
extern unsigned long jiffies_half_msec();
#endif

#define JIFFIES_CIRCLE                  0x7FFFFFF

#define time_after(a,b)					((long)(b) - (long)(a) < 0)
#define time_before(a,b)				time_after(b,a)

extern unsigned char jiffies_unit;

#define msecs_to_jiffies(msec) 		    ((msec)/jiffies_unit)
#define jiffies_to_msecs(j) 		    ((j)*jiffies_unit)




#endif

