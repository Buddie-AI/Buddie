#ifndef __UI_ROTATE_H__
#define __UI_ROTATE_H__

#include "rect.h"

void rotate_0(unsigned char *src, unsigned char *src1, int sw, int sh, int cx, int cy, unsigned char *dst, int dw, int dh, int dx, int dy, struct rect *rect, int angle);
void rotate_1(unsigned char *tmp, unsigned char *src, int sw, int sh, int cx, int cy, unsigned char *dst, int dw, int dh, int dx, int dy, struct rect *rect, int angle);
void rotate_map(int sw, int sh, int scx, int scy, int *dw, int *dh, int dcx, int dcy, int angle);

#endif

