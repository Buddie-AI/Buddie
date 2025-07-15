//--- support functions for Lua

#include <stdlib.h>
#include "cpu.h"

#if 1
void abort()
{
    printf("lua aborted !!!!!\n");
    while (1);
}
#endif

char *_user_strerror(
    int errnum,
    int internal,
    int *errptr)
{
    /* prevent warning about unused parameters */
    (void)errnum;
    (void)internal;
    (void)errptr;

    return 0;
}

unsigned int luaP_makeseed(void)
{
    int rand = (JL_RAND->R64H << 32) | JL_RAND->R64L;
    return rand;
}

void luaP_writeline(void)
{
    putchar('\n');
}

void luaP_writestring(const char *p, int l)
{
    for (int i = 0; i < l; i++) {
        putchar(*p++);
    }
}


#if 1
void *my_realloc(void *ptr, size_t osize, size_t nsize)
{
    /* printf("realloc(%x, %d)\n", ptr, size); */
    if (!ptr) {
        return malloc(nsize);
    }
    if (nsize == 0) {
        free(ptr);
        ptr = NULL;
        return NULL;
    }

    void *p = malloc(nsize);
    if (p) {
        if (ptr != NULL) {
            // 照较小的buffer大小拷贝，防止溢出
            size_t copy_size = (osize > nsize) ? nsize : osize;
            memcpy(p, ptr, copy_size);
            free(ptr);
            ptr = NULL;
        }
        return p;
    }
    return NULL;
}
#endif

int my_sprintf(char *s, const char *fmt, ...)
{
    va_list args;
    double argv;
    int ret = 0;

    if (!strcmp(fmt, "%.7g")) {
        va_start(args, fmt);
        argv = va_arg(args, double);

        char format[9] = {0};
        int integer = 0;
        int trim = 0;
        if ((argv > -0.00000001) && (argv < 0.00000001)) {
            sprintf(format, "%%d.%%d");
        } else if (argv >= 0.00000001) {
            integer = (int)argv;
            int decimal = (int)(((__int64)((argv - integer) * 100000000 + 5)) % 100000000);
            decimal /= 10;
            trim = decimal;
            int bits = 7;
            while (bits) {
                int n = trim % 10;
                if (n != 0) {
                    break;
                } else {
                    trim /= 10;
                    bits--;
                }
            }
            if (bits) {
                sprintf(format, "%%d.%%0%dd", bits);
            } else {
                sprintf(format, "%%d.%%d");
            }
        } else {
            argv = -argv;
            integer = (int)argv;
            int decimal = (int)(((__int64)((argv - integer) * 100000000 + 5)) % 100000000);
            decimal /= 10;
            trim = decimal;
            int bits = 7;
            while (bits) {
                int n = trim % 10;
                if (n != 0) {
                    break;
                } else {
                    trim /= 10;
                    bits--;
                }
            }
            if (bits) {
                sprintf(format, "-%%d.%%0%dd", bits);
            } else {
                sprintf(format, "-%%d.%%d");
            }
        }
        ret = sprintf(s, format, integer, trim);
        va_end(args);
    } else {
        ASSERT(0, "format %s not support!", fmt);
    }

    return ret;
}


