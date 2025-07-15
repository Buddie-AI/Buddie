#ifndef __LOG_H
#define __LOG_H


#include "system/generic/printf.h"

#define   __LOG_VERB      0
#define   __LOG_DEBUG     1
#define   __LOG_INFO      2
#define   __LOG_WARN      3
#define   __LOG_ERROR     4
#define   __LOG_CHAR      5

struct logbuf {
    u16 len;
    u16 buf_len;
    char buf[0];
};

#define __LOG_ENABLE

#ifndef __LOG_LEVEL
#define __LOG_LEVEL 0
#endif

#ifdef CONFIG_RELEASE_ENABLE
#undef __LOG_LEVEL
#define __LOG_LEVEL 0xff
#endif

#if __LOG_LEVEL > __LOG_VERB
#define log_v(...) do {} while (0)
#elif defined __LOG_ENABLE
#define log_v(...) log_print(__LOG_VERB, NULL, __VA_ARGS__)
#else
#define log_v(...) printf(__VA_ARGS__)
#endif



#if __LOG_LEVEL > __LOG_DEBUG
#define log_d(...)  do {} while (0)
#elif defined __LOG_ENABLE
#define log_d(...)  log_print(__LOG_DEBUG, NULL, __VA_ARGS__);
#else
#define log_d(...) printf(__VA_ARGS__)
#endif

#if __LOG_LEVEL > __LOG_INFO
#define log_i(...)  do {} while (0)
#elif defined __LOG_ENABLE
#define log_i(...) log_print(__LOG_INFO, NULL, __VA_ARGS__);
#else
#define log_i(...) printf(__VA_ARGS__)
#endif

#if __LOG_LEVEL > __LOG_WARN
#define log_w(...)  do {} while (0)
#elif defined __LOG_ENABLE
#define log_w(...) log_print(__LOG_WARN, NULL, __VA_ARGS__);
#else
#define log_w(...) printf(__VA_ARGS__)
#endif

#if __LOG_LEVEL > __LOG_ERROR
#define log_e(...)  do {} while (0)
#elif defined __LOG_ENABLE
#define log_e(...) log_print(__LOG_ERROR, NULL, __VA_ARGS__);
#else
#define log_e(...) printf(__VA_ARGS__)
#endif

#if __LOG_LEVEL > __LOG_CHAR
#define log_c(x)  do {} while (0)
#elif defined   __LOG_ENABLE
#define log_c(x)  putchar(x)
#else
#define log_c(x)
#endif

#define r_printf(x, ...)  log_i("\e[31m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define g_printf(x, ...)  log_i("\e[32m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define y_printf(x, ...)  log_i("\e[33m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define r_f_printf(x, ...)  log_i("\e[31m\e[5m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define g_f_printf(x, ...)  log_i("\e[32m\e[5m\e[1m" x "\e[0m", ## __VA_ARGS__)
#define y_f_printf(x, ...)  log_i("\e[33m\e[5m\e[1m" x "\e[0m", ## __VA_ARGS__)

#ifndef __LOG_ENABLE
#define log_dump(a, b)              do {} while(0)
#define log_putchar()               do {} while(0)
#define log_early_init(a)           do {} while(0)
#define log_level(a)                do {} while(0)
#else

int log_output_lock();

void log_output_unlock();

void log_print_time();

void log_early_init(int buf_size);

void log_level(int level);

void log_print(int level, const char *tag, const char *format, ...);

void log_dump(const u8 *buf, int len);

struct logbuf *log_output_start(int len);

void log_output_end(struct logbuf *);

void log_putchar(struct logbuf *lb, char c);

void log_put_u8hex(struct logbuf *lb, unsigned char dat);

void log_putbyte(char);

void log_set_time_offset(int offset);

int log_get_time_offset();

#endif

void log_flush();

#endif
