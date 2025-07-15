#ifndef _LITE_DEBUG_H_
#define _LITE_DEBUG_H_


extern void puts_lite(const char *out);
extern void put_buf_lite(void *_buf, u32 len);
extern int printf_lite(const char *format, ...);

#endif /* #ifdef _LITE_DEBUG_H_ */

