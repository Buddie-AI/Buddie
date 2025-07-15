#ifndef RDEC_KEY_H
#define RDEC_KEY_H

#include "typedef.h"

#include "asm/rdec.h"

int rdec_key_init(const struct rdec_platform_data *rdec_key_data);
s8 get_rdec_rdat(int i);

#endif

