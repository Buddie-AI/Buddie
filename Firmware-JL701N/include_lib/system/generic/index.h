#ifndef INDEX_H
#define INDEX_H


#include "typedef.h"


#define TABLE(t)    t, ARRAY_SIZE(t)


int index_of_table8(u8 value, const u8 *table, int table_size);

int index_of_table16(u16 value, const u16 *table, int table_size);

int index_of_table32(u32 value, const u32 *table, int table_size);






#endif
