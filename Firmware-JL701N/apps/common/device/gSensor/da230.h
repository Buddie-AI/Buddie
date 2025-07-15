#ifndef __da230_h
#define __da230_h

#include "typedef.h"
#include "mira_std.h"



#define LATCHED_MODE			LATCHED_1S	//interrupt latched mode
#define INTERRUPT_THRESHOLD 	15			//interrupt threshold 0~31 ,(threshold*125)mg
#define INTERRUPT_DURATION      5   		//interrupt duration 0~7 ,(50~700)ms 0:50ms 1:100ms 2:150ms 3:200ms 4:250ms 5:375ms 6:500ms 7:700ms
#define TAP_SAMPLE_LEVEL		1			//采样次数 = 25-TAP_SAMPLE_LEVEL*5
#define TAP_FILTER				30			//噪声大小，大于TAP_FILTER认为是噪声

#define I2C_ADDR_DA230_W 				0x4e
#define I2C_ADDR_DA230_R 				0x4f

extern u32 da230_register_read(u8 addr, u8 *data);
extern s8_m da230_register_write(u8_m addr, u8_m data);
extern u32 da230_register_write_bit(u8 addr, u8 start, u8 len, u8 data);

// extern u8 da230_int_event_detect(void);

#endif

